/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include "officecfg/Office/Common.hxx"
#include <comphelper/processfactory.hxx>

#include "sddll.hxx"

#include "DiscoveryService.hxx"
#include "ImagePreparer.hxx"
#include "Listener.hxx"
#include "Receiver.hxx"
#include "RemoteServer.hxx"

using namespace std;
using namespace sd;
using namespace ::com::sun::star;
using rtl::OString;
using namespace ::osl;

// struct ClientInfoInternal:
//     ClientInfo
// {
//     osl::StreamSocket mStreamSocket;
//     rtl::OUString mPin;
//     ClientInfoInternal( const rtl::OUString rName,
//                         const rtl::OUString rAddress,
//                         osl::StreamSocket &rSocket, rtl::OUString rPin ):
//             ClientInfo( rName, rAddress ),
//             mStreamSocket( rSocket ),
//             mPin( rPin ) {}
// };

RemoteServer::RemoteServer() :
    Thread( "RemoteServerThread" ),
    mSocket(),
    mDataMutex(),
    mCommunicators(),
    mAvailableClients()
{
}

RemoteServer::~RemoteServer()
{
}

void RemoteServer::execute()
{
    osl::SocketAddr aAddr( "0", PORT );
    if ( !mSocket.bind( aAddr ) )
    {
        // Error binding
    }

    if ( !mSocket.listen(3) )
    {
        // Error listening
    }
    while ( true )
    {
        StreamSocket aSocket;
        if ( mSocket.acceptConnection( aSocket ) == osl_Socket_Error ) {
            MutexGuard aGuard( mDataMutex );
            // FIXME: read one line in, parse the data.
            mAvailableClients.push_back( new ClientInfoInternal( "A name",
                                        "An address", aSocket, "0000" ) );
        }
    }

}

RemoteServer *sd::RemoteServer::spServer = NULL;

void RemoteServer::setup()
{
    if (spServer)
        return;

    spServer = new RemoteServer();
    spServer->launch();
}


void RemoteServer::presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController )
{
    if ( !spServer )
        return;
    MutexGuard aGuard( spServer->mDataMutex );
    for ( vector<Communicator*>::const_iterator aIt = spServer->mCommunicators.begin();
         aIt < spServer->mCommunicators.end(); aIt++ )
    {
        (*aIt)->presentationStarted( rController );
    }
}
void RemoteServer::presentationStopped()
{
    if ( !spServer )
        return;
    MutexGuard aGuard( spServer->mDataMutex );
    for ( vector<Communicator*>::const_iterator aIt = spServer->mCommunicators.begin();
         aIt < spServer->mCommunicators.end(); aIt++ )
    {
        (*aIt)->disposeListener();
    }
}

void RemoteServer::removeCommunicator( Communicator* mCommunicator )
{
    if ( !spServer )
        return;
    MutexGuard aGuard( spServer->mDataMutex );
    for ( vector<Communicator*>::iterator aIt = spServer->mCommunicators.begin();
         aIt < spServer->mCommunicators.end(); aIt++ )
    {
        if ( mCommunicator == *aIt )
        {
            spServer->mCommunicators.erase( aIt );
            break;
        }
    }
}

std::vector<ClientInfo*> RemoteServer::getClients()
{
    if ( !spServer )
        std::vector<ClientInfo*>();
    MutexGuard aGuard( spServer->mDataMutex );
    std::vector<ClientInfo*> aClients;
    aClients.assign( spServer->mAvailableClients.begin(),
                     spServer->mAvailableClients.end() );
    return aClients;
}

void RemoteServer::connectClient( ClientInfo aClient, rtl::OString aPin )
{
    (void) aClient;
    (void) aPin;
}

void SdDLL::RegisterRemotes()
{
    // Disable unless in experimental mode for now
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if (!xContext.is() || !officecfg::Office::Common::Misc::ExperimentalMode::get(xContext))
        return;

    sd::RemoteServer::setup();
    sd::DiscoveryService::setup();

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
