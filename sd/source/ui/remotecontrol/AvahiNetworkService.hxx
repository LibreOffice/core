#ifndef AVAHI_NETWORK_SERVICE_H
#define AVAHI_NETWORK_SERVICE_H


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <thread>
#include <iostream>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/thread-watch.h>

#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

#include "ZeroconfService.hxx"

namespace sd {

    class AvahiNetworkService : public ZeroconfService
    {
    public:
        AvahiNetworkService(const std::string& aname = "", uint aport = 1599)
            : ZeroconfService(aname, aport){}
        ~AvahiNetworkService();

        void clear();
        void setup();
    };
}
#endif
