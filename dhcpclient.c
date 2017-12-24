#include "dhcp.h"
#include <arpa/inet.h>
#include <net/if.h>    // for struct ifreq
#include <sys/types.h>
#include <sys/socket.h> // for setsockopt()
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRANS_LEN 512
#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67
#define DHCP_MAGIC_COOKIE 0x63538263
#define DEV "eth1"

int get_mac_address(unsigned char *mac)
{
    struct ifreq s;
    int fd;
    int flag;
    //unsigned char arp[6];

    if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Socket Error");  
        exit(1); 
    }
    strcpy(s.ifr_name, DEV);
    flag = ioctl(fd, SIOCGIFHWADDR, &s);
    close(fd);

    if (flag != 0)
        return -1;

    memcpy((void *)mac, s.ifr_addr.sa_data, 6);
    return 0;
}

int fill_dhcp_option(unsigned char *options, u_int8_t type, u_int8_t *data, u_int8_t length)
{
    options[0] = type;
    options[1] = length;
    memcpy(&options[2], data, length);

    return length + (sizeof(u_int8_t) * 2);
}

void fill_client_commen_field(packet *p)
{
    p->op = BOOTREQUEST;
    p->htype = 1;
    p->hlen = 6;
    p->hops = 0;

    p->yiaddr.s_addr = inet_addr("0.0.0.0");
    p->siaddr.s_addr = inet_addr("0.0.0.0");
    p->giaddr.s_addr = inet_addr("0.0.0.0");

    // memset(p->chaddr, 0, sizeof(p->chaddr));
    // memset(p->sname, 0, sizeof(p->sname));
    // memset(p->file, 0, sizeof(p->file));
    // p->chaddr = {'0'};
    // p->sname  = {'0'};
    // p->file   = {'0'};

    p->magic_cookie = DHCP_MAGIC_COOKIE;
}

// void send_release()
// {
//     packet *release_packet;
// 	memset(&release_packet, 0, sizeof(struct packet));
//     fill_client_commen_field(release_packet);

//     release->xid   = 0x11fd2561;
//     release->secs  = 0;
//     release->flags = 0;
//     get_mac_address(release->ciaddr);

// }

void fill_discover(packet *discover_packet)
{   
    int len = 0;
    u_int8_t message_data[] = {DHCPDISCOVER};
    u_int8_t data = 0;
    u_int8_t parameter_req_list[] = {DHO_SUBNET_MASK, DHO_ROUTERS, DHO_DOMAIN_NAME_SERVERS};

    memset(discover_packet, 0, sizeof(packet));
    fill_client_commen_field(discover_packet);

    discover_packet->xid   = 0x780E73E4;   // student ID  2014213092
    discover_packet->secs  = 0;
    discover_packet->flags = 0x0080;
    get_mac_address(discover_packet->chaddr);

    len += fill_dhcp_option(&discover_packet->options[len], DHO_DHCP_MESSAGE_TYPE, (u_int8_t *)&message_data, sizeof(message_data));
    len += fill_dhcp_option(&discover_packet->options[len], DHO_DHCP_PARAMETER_REQUEST_LIST, (u_int8_t *)&parameter_req_list, sizeof(parameter_req_list));
    len += fill_dhcp_option(&discover_packet->options[len], DHO_END, &data, sizeof(data));
}

/* send_to(sd, &discover_msg, sizeof discover_msg, 0, (struct sockaddr*)&skt,sktlen) */
/* sendto(sock, echoString, echoStringLen, 0,(struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) */
void send_to(int socket, const void* buffer, int buffer_length, int flags, const struct sockaddr* dest_addr, socklen_t dest_len)
{
    int res;
    if ((res = sendto(socket, buffer, buffer_length, 0, dest_addr, dest_len)) < 0) {
        perror("sendto failed. ");
        exit(1);
    }
    printf("Sending packet to: %s(%d)\n",inet_ntoa(((struct sockaddr_in*)dest_addr)->sin_addr),ntohs(((struct sockaddr_in*)dest_addr)->sin_port));
}

void recv_from(int socket, void* buffer, int buffer_length, int flags, struct sockaddr* address, socklen_t* address_len)
{
    int res;
    if ((res = recvfrom(socket, buffer, buffer_length, flags, address, address_len)) < 0) {
        perror("recvfrom failed. ");
        exit(1);
    }
    printf("Recieve msg from: %s(%d)\n", inet_ntoa(((struct sockaddr_in*)address)->sin_addr), ntohs(((struct sockaddr_in*)address)->sin_port));

}

int main(int argc, char **argv) 
{   
    int sock; /* Socket descriptor */
    struct ifreq netcard;
    struct sockaddr_in clntAddr;
    struct sockaddr_in servAddr;
    struct in_addr wanted_addr; 
    packet p;
    char *commond;
    int clntPort; /* client port */
    int servPort; /* server port */
    int i = 1;

    if ((argc < 2) || (argc > 4)) /* Test for correct number of arguments */
    {
        printf("Usage: %s <commond> [desired_address]\n", argv[0]);
        exit(1);
    }
    commond = argv[1];
    if (argc == 3)
    {
        wanted_addr.s_addr = inet_addr(argv[2]);
    }

    clntPort = DHCP_CLIENT_PORT;
    servPort = DHCP_SERVER_PORT;

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket() failed.\n");
        exit(1);
    }

    if (strcmp(commond, "--default") == 0)
    {
        strcpy(netcard.ifr_name, DEV);
        socklen_t len = sizeof(i);
        /* Allow socket to broadcast */
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &i, len);


        /* Set socket to interface DEV */
        if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&netcard, sizeof(netcard)) < 0)
        {
            printf("bind socket to %s error\n", DEV);
        }

        /* Zero out structure */
        memset(&clntAddr, 0, sizeof(clntAddr));
        clntAddr.sin_family = AF_INET;
        clntAddr.sin_port   = htons(clntPort);
        clntAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if((bind(sock, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0)
        {
            printf("bind() failed.\n");
            exit(1);
        }

        /* Construct the server address structure */
        /*Zero out structure*/
        memset(&servAddr, 0, sizeof(servAddr));
        /* Internet addr family */
        servAddr.sin_family = AF_INET; 
        /*Server IP address*/
        servAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
        /* Server port */
        servAddr.sin_port = htons(servPort);
        

        fill_discover(&p);
        send_to(sock, &p, sizeof(packet), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));
    }
    else if (commond == "")
    {
        ;
    }
    else if (commond == "")
    {
        ;
    }
    else if (commond == "")
    {
        ;
    }
    else
    {
        printf("Bad Commond.\n");
        exit(1);
    }
}