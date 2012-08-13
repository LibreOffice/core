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
        if ( mSocket.acceptConnection( aSocket ) == osl_Socket_Error )
        {
            return; // Closed, or other issue.
        }
        BufferedStreamSocket *pSocket = new BufferedStreamSocket( aSocket);
        OString aLine;
        if ( pSocket->readLine( aLine)
            && aLine.equals( "LO_SERVER_CLIENT_PAIR" ) &&
            pSocket->readLine( aLine ) )
        {
            OString aName( aLine );

            if ( ! pSocket->readLine( aLine ) ) delete pSocket;
            OString aPin( aLine );
            fprintf( stderr, "Pin:%s\n", aPin.getStr() );

            SocketAddr aClientAddr;
            pSocket->getPeerAddr( aClientAddr );
            OUString aAddress = aClientAddr.getHostname();

            MutexGuard aGuard( mDataMutex );
            mAvailableClients.push_back( new ClientInfoInternal(
                    OStringToOUString( aName, RTL_TEXTENCODING_UTF8 ),
                    aAddress, pSocket, OStringToOUString( aPin,
                    RTL_TEXTENCODING_UTF8 ) ) );

            // Read off any additional non-empty lines
            do
            {
                pSocket->readLine( aLine );
            }
            while ( aLine.getLength() > 0 );
        } else {
            delete pSocket;
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

sal_Bool RemoteServer::connectClient( ClientInfo* pClient, rtl::OUString aPin )
{
    if ( !spServer )
        return false;

    ClientInfoInternal *apClient = (ClientInfoInternal*) pClient;
    if ( apClient->mPin.equals( aPin ) )
    {
        Communicator* pCommunicator = new Communicator( apClient->mpStreamSocket );
        MutexGuard aGuard( spServer->mDataMutex );

        spServer->mCommunicators.push_back( pCommunicator );

        for ( vector<ClientInfoInternal*>::iterator aIt = spServer->mAvailableClients.begin();
            aIt < spServer->mAvailableClients.end(); aIt++ )
        {
            if ( pClient == *aIt )
            {
                spServer->mAvailableClients.erase( aIt );
            break;
            }
        }
        pCommunicator->launch();
        return true;
    }
    else
    {
        return false;
    }
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
