/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_COMMUNICATOR_HXX
#define _SD_IMPRESSREMOTE_COMMUNICATOR_HXX

// SERVER
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <vector>

#include <osl/socket.hxx>
#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>

#include <com/sun/star/presentation/XSlideShowController.hpp>

#define CHARSET RTL_TEXTENCODING_UTF8
namespace css = ::com::sun::star;

namespace sd
{

    class Transmitter;
    class Listener;
    class ImagePreparer;

    /** Class used for communication with one single client, dealing with all
     * tasks specific to this client.
     *
     * Needs to be created, then started using launch(), disposes itself.
     */
    class Communicator : public salhelper::Thread
    {
        public:
            Communicator( osl::StreamSocket &aSocket );
            ~Communicator();

            Transmitter* getTransmitter();
            void presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController );
            void informListenerDestroyed();
            void disposeListener();

        private:
            void execute();
            osl::StreamSocket mSocket;

            Transmitter *pTransmitter;
            rtl::Reference<Listener> mListener;
    };
}

#endif // _SD_IMPRESSREMOTE_COMMUNICATOR_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
