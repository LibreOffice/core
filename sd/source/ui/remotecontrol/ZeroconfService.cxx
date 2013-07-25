#include "ZeroconfService.hxx"
#include <string>


ZeroconfService::ZeroconfService(string aname = "", uint aport = 1599)
    :port(aport), name(aname)
{
    // Defined in subclasses
    this->setup();

    mSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    sockaddr_in aAddr;
    memset(&aAddr, 0, sizeof(aAddr));
    aAddr.sin_family = AF_INET;
    aAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    aAddr.sin_port = htons( PORT_DISCOVERY );

    int rc = bind( mSocket, (sockaddr*) &aAddr, sizeof(sockaddr_in) );

    if (rc)
    {
        SAL_WARN("sd", "DiscoveryService: bind failed: " << errno);
        return; // would be better to throw, but unsure if caller handles that
    }

    struct ip_mreq multicastRequest;

    multicastRequest.imr_multiaddr.s_addr = inet_addr( "239.0.0.1" );
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    rc = setsockopt( mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
  #ifdef WNT
        (const char*)
  #endif
        &multicastRequest, sizeof(multicastRequest));

    if (rc)
    {
        SAL_WARN("sd", "DiscoveryService: setsockopt failed: " << errno);
        return; // would be better to throw, but unsure if caller handles that
    }

    SAL_WARN("sd", "ZeroconfService created: " << name << " port:"<< port);
}