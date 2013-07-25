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

#include <osl/socket.hxx>
#include <osl/thread.hxx>

#include "ZeroconfService.hxx"

struct sockaddr_in;

namespace sd
{
    class DiscoveryService : public osl::Thread
    {
        public:
            static void setup();

        private:
            DiscoveryService();
            ~DiscoveryService();

            static DiscoveryService *spService;
            virtual void SAL_CALL run();
            int mSocket;

            ZeroconfService * zService;
    };
}

#endif // _SD_IMPRESSREMOTE_DISCOVERYSERVICE_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
