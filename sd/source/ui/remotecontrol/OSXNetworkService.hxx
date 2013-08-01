/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef OSX_NETWORK_SERVICE_HXX
#define OSX_NETWORK_SERVICE_HXX


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>
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
        OSXBonjourService *osxservice;
    public:
        OSXNetworkService(const std::string& aname = "", unsigned int aport = 1599)
            : ZeroconfService(aname, aport){}

        void clear() {
            [osxservice dealloc];
        }
        void setup() {
            osxservice = [[OSXBonjourService alloc] init];
            [osxservice publishImpressRemoteServiceOnLocalNetworkWithName: @""];
        };
    };
}
#endif