#ifndef OSX_NETWORK_SERVICE_HXX
#define OSX_NETWORK_SERVICE_HXX


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <thread>
#include <iostream>

#include <osl/conditn.hxx> // Include this early to avoid error as check() gets defined by some SDK header to empty
#include <premac.h>
#import <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#import "OSXNetworkService.h"

#include "ZeroconfService.hxx"

namespace sd {
    class OSXNetworkService : public ZeroconfService
    {
    private:
        OSXNetworkService osxservice;
    public:
        OSXNetworkService(const string& aname = "", uint aport = 1599)
            : ZeroconfService(aname, aport){}

        void clear() {
            [osxservice dealloc];
        }
        void setup() {
            osxservice = [[OSXNetworkService alloc] init];
            [osxservice publishImpressRemoteServiceOnLocalNetworkWithName: @""];
        };
    };
}
#endif