/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef ZEROCONF_SERVICE
#define ZEROCONF_SERVICE

#include <string>
#include <stdio.h>
#include <stdlib.h>

/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT_DISCOVERY 1598
#define BUFFER_SIZE 200

#define CHARSET RTL_TEXTENCODING_UTF8

struct sockaddr_in;

typedef unsigned int uint;

namespace sd{

    class ZeroconfService
    {
    protected:
        std::string name;
        uint port;

    public:
        explicit ZeroconfService(const std::string& aname, uint aport)
            :name(aname), port(aport){}
        virtual ~ZeroconfService(){}

        std::string getName() const {return name;}
        void setName(const char * n) {name = n;}
        uint getPort() const {return port;}

        // Clean up the service when closing
        virtual void clear() = 0;
        // Bonjour for OSX, Avahi for Linux
        virtual void setup() = 0;
    };

}
#endif
