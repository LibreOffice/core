/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_SERVER_HXX
#define _SD_IMPRESSREMOTE_SERVER_HXX

// SERVER
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <osl/socket.hxx>
#include <salhelper/thread.hxx>

#include <com/sun/star/presentation/XSlideShowController.hpp>

namespace css = ::com::sun::star;

/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT 1599

#define CHARSET RTL_TEXTENCODING_UTF8

namespace sd
{

    class Transmitter;

    class Server : public salhelper::Thread
    {
        public:
            static void setup();
            static void presentationStarted( css::uno::Reference<
                css::presentation::XSlideShowController > rController );
        private:
            Server();
            ~Server();
            static Server *spServer;
            osl::AcceptorSocket mSocket;
            osl::StreamSocket mStreamSocket;
            void listenThread();
            void execute();
            static Transmitter *mTransmitter;
    };
}

#endif // _SD_IMPRESSREMOTE_SERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */