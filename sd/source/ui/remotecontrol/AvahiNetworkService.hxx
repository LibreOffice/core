/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
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
