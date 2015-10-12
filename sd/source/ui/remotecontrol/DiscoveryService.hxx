/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_DISCOVERYSERVICE_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_DISCOVERYSERVICE_HXX

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
            virtual ~DiscoveryService();

            /**
             * Networking related setup -- must be run within our own thread
             * to prevent the application blocking (fdo#75328).
             */
            void setupSockets();

            static DiscoveryService *spService;
            virtual void SAL_CALL run() override;
            int mSocket;

            ZeroconfService * zService;
    };
}

#endif // INCLUDED_SD_SOURCE_UI_REMOTECONTROL_DISCOVERYSERVICE_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
