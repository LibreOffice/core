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
#include <rtl/ref.hxx>
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
    class Listener;
    class ImagePreparer;

    class RemoteServer : public salhelper::Thread
    {
        public:
            static void setup();
            static void presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController );
            static void presentationStopped();
            void informListenerDestroyed();
        private:
            RemoteServer();
            ~RemoteServer();
            static RemoteServer *spServer;
            osl::AcceptorSocket mSocket;
            osl::StreamSocket mStreamSocket;
            void pairClient();
            void listenThread();
            void execute();
            static Transmitter *pTransmitter;
            static rtl::Reference<Listener> mListener;
    };
}

#endif // _SD_IMPRESSREMOTE_SERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */