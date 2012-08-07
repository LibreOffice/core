/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_DISCOVERYSERVICE_HXX
#define _SD_IMPRESSREMOTE_DISCOVERYSERVICE_HXX

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <osl/socket.hxx>
#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>

namespace css = ::com::sun::star;

/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT_DISCOVERY 1598
#define BUFFER_SIZE 200

#define CHARSET RTL_TEXTENCODING_UTF8

struct sockaddr_in;

namespace sd
{



    class DiscoveryService : public salhelper::Thread
    {
        public:
            static void setup();

        private:
            DiscoveryService();
            ~DiscoveryService();

            static DiscoveryService *spService;
            void execute();

//             osl::DatagramSocket mSocket;
            int mSocket;

    };
}

#endif // _SD_IMPRESSREMOTE_DISCOVERYSERVICE_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */