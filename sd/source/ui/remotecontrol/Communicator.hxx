/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_COMMUNICATOR_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_COMMUNICATOR_HXX

#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <vector>

#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>

#include <com/sun/star/presentation/XSlideShowController.hpp>

#include "IBluetoothSocket.hxx"

#define CHARSET RTL_TEXTENCODING_UTF8
namespace sd
{

    class Transmitter;
    class Listener;

    /** Class used for communication with one single client, dealing with all
     * tasks specific to this client.
     *
     * Needs to be created, then started using launch(), disposes itself.
     */
    class Communicator : public salhelper::Thread
    {
        public:
            explicit Communicator( IBluetoothSocket *pSocket );
            virtual ~Communicator();

            void presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController );
            void informListenerDestroyed();
            void disposeListener();
            void forceClose();

        private:
            void execute() override;
            IBluetoothSocket *mpSocket;

            Transmitter *pTransmitter;
            rtl::Reference<Listener> mListener;
    };
}

#endif // INCLUDED_SD_SOURCE_UI_REMOTECONTROL_COMMUNICATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
