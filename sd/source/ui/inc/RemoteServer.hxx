/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// SERVER
#include <memory>
#include <vector>

#include <osl/socket_decl.hxx>
#include <salhelper/thread.hxx>

#include <sddllapi.h>

namespace osl { class Mutex; }
namespace com::sun::star::presentation { class XSlideShowController; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }

/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT 1599

namespace sd
{
    class Communicator;

    struct ClientInfo
    {
        OUString mName;

        bool mbIsAlreadyAuthorised;

        ClientInfo( const OUString& rName,
                    const bool bIsAlreadyAuthorised ) :
            mName( rName ),
            mbIsAlreadyAuthorised( bIsAlreadyAuthorised ) {}

        virtual ~ClientInfo() {};
    };

    struct ClientInfoInternal;

    class RemoteServer : public salhelper::Thread
    {
        public:
            // Internal setup
            static void setup();

            // For slideshowimpl to inform us.
            static void presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController );
            static void presentationStopped();

            // For the control dialog
            SD_DLLPUBLIC static std::vector< std::shared_ptr< ClientInfo > > getClients();
            SD_DLLPUBLIC static bool connectClient( const std::shared_ptr< ClientInfo >& pClient,
                                                    std::u16string_view aPin );
            SD_DLLPUBLIC static void deauthoriseClient( const std::shared_ptr< ClientInfo >& pClient );

            /// ensure that discoverability (eg. for Bluetooth) is enabled
            SD_DLLPUBLIC static void ensureDiscoverable();
            /// restore the state of discoverability from before ensureDiscoverable
            SD_DLLPUBLIC static void restoreDiscoverable();

            // For the communicator
            static void removeCommunicator( Communicator const * pCommunicator );
        private:
            RemoteServer();
            virtual ~RemoteServer() override;
            static RemoteServer *spServer;
            static ::osl::Mutex sDataMutex;
            static ::std::vector<Communicator*> sCommunicators;
            osl::AcceptorSocket mSocket;

            ::std::vector< std::shared_ptr< ClientInfoInternal > > mAvailableClients;

            void execute() override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
