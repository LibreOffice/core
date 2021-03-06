/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>

#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>

namespace com::sun::star::uno { template <typename > class Reference; }
namespace com::sun::star::presentation { class XSlideShowController; }
namespace sd { struct IBluetoothSocket; }

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
            explicit Communicator( std::unique_ptr<IBluetoothSocket> pSocket );
            virtual ~Communicator() override;

            void presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController );
            void informListenerDestroyed();
            void disposeListener();
            void forceClose();

        private:
            void execute() override;
            std::unique_ptr<IBluetoothSocket> mpSocket;

            std::unique_ptr<Transmitter> pTransmitter;
            rtl::Reference<Listener> mListener;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
