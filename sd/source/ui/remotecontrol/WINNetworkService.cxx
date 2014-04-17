#include <string>
#include <iostream>
#include "WINNetworkService.hxx"
#include "mDNSResponder/dns_sd.h"

void sd::WINNetworkService::setup()
{
    DNSServiceErrorType err = DNSServiceRegister(&client, 0, 0, NULL, kREG_TYPE, "local", NULL, 1599, 1, "", NULL, this );

    // Fail silently
}

void sd::WINNetworkService::clear()
{
    DNSServiceRefDeallocate(client);
}
