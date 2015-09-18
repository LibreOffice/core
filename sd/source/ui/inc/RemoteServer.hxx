/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_INC_REMOTESERVER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_REMOTESERVER_HXX

// SERVER
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <memory>
#include <vector>

#include <osl/mutex.hxx>
#include <osl/socket.hxx>
#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>

#include <com/sun/star/presentation/XSlideShowController.hpp>

#include "sddllapi.h"

/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT 1599

#define CHARSET RTL_TEXTENCODING_UTF8

namespace sd
{
    class Communicator;
    class BufferedStreamSocket;

    struct ClientInfo
    {
        OUString mName;
        OUString mAddress;

        bool mbIsAlreadyAuthorised;

        enum PROTOCOL { NETWORK = 1, BLUETOOTH };
        ClientInfo( const OUString& rName,
                    const OUString& rAddress,
                    const bool bIsAlreadyAuthorised ) :
            mName( rName ),
            mAddress( rAddress ),
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
            SD_DLLPUBLIC static bool connectClient( std::shared_ptr< ClientInfo > pClient,
                                                        const OUString& aPin );
            SD_DLLPUBLIC static void deauthoriseClient( std::shared_ptr< ClientInfo > pClient );

            /// ensure that discoverability (eg. for Bluetooth) is enabled
            SD_DLLPUBLIC static void ensureDiscoverable();
            /// restore the state of discoverability from before ensureDiscoverable
            SD_DLLPUBLIC static void restoreDiscoverable();

            // For the communicator
            static void removeCommunicator( Communicator* pCommunicator );
        private:
            RemoteServer();
            virtual ~RemoteServer();
            static RemoteServer *spServer;
            static ::osl::Mutex sDataMutex;
            static ::std::vector<Communicator*> sCommunicators;
            osl::AcceptorSocket mSocket;

            ::std::vector< std::shared_ptr< ClientInfoInternal > > mAvailableClients;

            void execute() SAL_OVERRIDE;
    };
}

#endif // INCLUDED_SD_SOURCE_UI_INC_REMOTESERVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
