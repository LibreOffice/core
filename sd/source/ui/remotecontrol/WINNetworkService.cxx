#include <string>
#include <iostream>
#include "WINNetworkService.hxx"
#include <sal/log.hxx>

void sd::WINNetworkService::setup()
{
    DNSServiceErrorType err = DNSServiceRegister(&client, 0, 0, NULL, kREG_TYPE, "local", NULL, 1599, 1, "", NULL, this );

    if (kDNSServiceErr_NoError != err)
        SAL_WARN("sdremote.wifi", "DNSServiceRegister failed: " << err);

    // Fail silently
}

void sd::WINNetworkService::clear()
{
    DNSServiceRefDeallocate(client);
}
