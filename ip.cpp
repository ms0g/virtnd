#include <iostream>
#include <netinet/in.h>
#include "utils.h"
#include "ip.h"
#include "icmp.h"

/*
 * rfc 791
 * https://tools.ietf.org/html/rfc791
 */


IP *IP::instance() {
    static IP ins;
    return &ins;
}

void IP::recv(pk_buff *pkb, uint8_t hwaddr[]) {
    struct iphdr *iph;
    struct eth_frame *eth;

    eth = eth_hdr(pkb->data);
    iph = emit_hdr(eth);

    if (iph->version != IPv4) {
        std::cerr << "Version is not IPv4\n";
        return;
    }

    if(pkb->len < sizeof(eth_frame)+ sizeof(iphdr)) {
        std::cerr << "IPv4 packet is too small";
        return;
    }

    if (iph->ihl < 5) {
        std::cerr << "IPv4 header length must be at least 5\n";
        return;
    }

    if (iph->ihl * 4 != sizeof(iphdr)) {
        std::cerr << "IPv4 header is too small\n";
        return;
    }

    if (iph->ttl == 0) {
        std::cerr << "The Datagram must be destroyed\n";
        return;
    }

    auto cksum = checksum(iph, iph->ihl * 4, 0);
    if (cksum != 0) {
        std::cerr << "IP Invalid Checksum\n";
        return;
    }

    // Check IP Fragment

    iph->saddr = ntohl(iph->saddr);
    iph->daddr = ntohl(iph->daddr);
    iph->len = ntohs(iph->len);
    iph->id = ntohs(iph->id);

    switch (iph->pro) {
        case ICMPv4:
            icmp->recv(pkb, hwaddr);
            break;
        case IP_TCP:
            break;
        default:
            break;
    }
}

IP *ip = IP::instance();

