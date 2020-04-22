#include <netinet/in.h>
#include "packet_processor.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"


PacketProcessor::PacketProcessor() :
        stop(false),
        m_thread([this]() { this->worker(); }) {}

PacketProcessor::~PacketProcessor() {
    std::lock_guard<std::mutex> lockg(mutex);
    stop = true;
    m_thread.join();
}

void PacketProcessor::update(pk_buff pkt) {
    pkt_queue.push(std::move(pkt));

}

void PacketProcessor::worker() {
    while (!stop || !pkt_queue.empty()) {
        if (pkt_queue.empty())
            continue;

        pk_buff pkb;
        pkt_queue.wait_and_pop(pkb);

        auto *eth = eth_hdr(pkb.data);
        eth->type = htons(eth->type);

        switch (eth->type) {
            case ETH_P_ARP:
                _ARP()->recv(std::move(pkb));
                break;
            case ETH_P_IP:
                _IP()->recv(std::move(pkb));
                break;
            default:
                break;
        }
    }
}