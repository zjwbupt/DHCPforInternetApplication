/* dhcp.h */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef DHCP_H
#define DHCP_H

#define DHCP_HADDR_LEN      16
#define DHCP_SNAME_LEN      64
#define DHCP_FILE_LEN       128

#define DHCP_MAX_OPTION_LEN  (512 - 282)

typedef struct dhcp_packet {
    u_int8_t  op;       /* 0: Message opcode/type */
    u_int8_t  htype;    /* 1: Hardware addr type (net/if_types.h) */
    u_int8_t  hlen;     /* 2: Hardware addr length */
    u_int8_t  hops;     /* 3: Number of relay agent hops from client */
    u_int32_t xid;      /* 4: Transaction ID */
    u_int16_t secs;     /* 8: Seconds since client started looking */
    u_int16_t flags;    /* 10: Flag bits */
    struct in_addr ciaddr;   /* 12: Client IP address (if already in use) */
    struct in_addr yiaddr;   /* 16: Client IP address */
    struct in_addr siaddr;   /* 18: IP address of next server to talk to */
    struct in_addr giaddr;   /* 20: DHCP relay agent IP address */
    unsigned char chaddr[DHCP_HADDR_LEN];   /* 24: Client hardware address */
    char sname[DHCP_SNAME_LEN];             /* 40: Server name */
    char file[DHCP_FILE_LEN];               /* 104: Boot filename */
    u_int32_t magic_cookie;                 /* 212: Magic Cookie */
    unsigned char options[DHCP_MAX_OPTION_LEN];  /* 216: Optional parameters (actual length dependent on MTU). */
} packet;

/* DHCP Option codes */
#define DHO_SUBNET_MASK                  1
#define DHO_ROUTERS                      3
#define DHO_DOMAIN_NAME_SERVERS          6
#define DHO_DHCP_LEASE_TIME              51
#define DHO_DHCP_MESSAGE_TYPE            53
#define DHO_DHCP_SERVER_IDENTIFIER       54
#define DHO_DHCP_PARAMETER_REQUEST_LIST  55
#define DHO_DHCP_RENEWAL_TIME            58
#define DHO_DHCP_REBINDING_TIME          59
#define DHO_VENDOR_CLASS_IDENTIFIER      60
#define DHO_END                          255

/* DHCP message types */
#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7
#define DHCPINFORM    8


/* BOOTP (rfc951) message types */
#define	BOOTREQUEST	1
#define BOOTREPLY	2

#endif /* DHCP_H */

