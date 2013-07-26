#ifndef AVAHI_NETWORK_SERVICE_H
#define AVAHI_NETWORK_SERVICE_H

#include <string>
#include "ZeroconfService.hxx"

namespace sd {

    class AvahiNetworkService : public ZeroconfService
    {
    public:
        AvahiNetworkService(const std::string& aname = "", unsigned int aport = 1599)
            : ZeroconfService(aname, aport){}
        virtual ~AvahiNetworkService(){}

        void clear();
        void setup();
    };
}
#endif
