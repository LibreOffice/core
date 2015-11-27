#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_WINNETWORKSERVICE_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_WINNETWORKSERVICE_HXX

#include <string>
#undef WB_LEFT
#undef WB_RIGHT
#include <dns_sd.h>
#include "ZeroconfService.hxx"

namespace sd{
    class WINNetworkService : public ZeroconfService
    {
    private:
        DNSServiceRef client;

    public:
        WINNetworkService(const std::string& aname = "", unsigned int aport = 1599)
            : ZeroconfService(aname, aport), client(0) {}
        virtual ~WINNetworkService(){}

        void clear();
        void setup();

    };
}
#endif
