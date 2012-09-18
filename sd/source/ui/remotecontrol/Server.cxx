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
#include "officecfg/Office/Impress.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>

#include "sddll.hxx"

#include "DiscoveryService.hxx"
#include "ImagePreparer.hxx"
#include "Listener.hxx"
#include "Receiver.hxx"
#include "RemoteServer.hxx"
#include "BluetoothServer.hxx"

using namespace std;
using namespace sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using rtl::OString;
using namespace ::osl;
using namespace ::comphelper;

namespace sd {
    /**
     * Used to keep track of clients that have attempted to connect, but haven't
     * yet been approved.
     */
    struct ClientInfoInternal:
        ClientInfo
    {
        BufferedStreamSocket *mpStreamSocket;
        rtl::OUString mPin;

        ClientInfoInternal( const rtl::OUString rName,
                            const rtl::OUString rAddress,
                            BufferedStreamSocket *pSocket, rtl::OUString rPin ):
                ClientInfo( rName, rAddress ),
                mpStreamSocket( pSocket ),
                mPin( rPin ) {}
    };
}

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
        return;
    }

    if ( !mSocket.listen(3) )
    {
        // Error listening
        return;
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

            SocketAddr aClientAddr;
            pSocket->getPeerAddr( aClientAddr );
            OUString aAddress = aClientAddr.getHostname();

            MutexGuard aGuard( mDataMutex );
            ClientInfoInternal* pClient = new ClientInfoInternal(
                    OStringToOUString( aName, RTL_TEXTENCODING_UTF8 ),
                    aAddress, pSocket, OStringToOUString( aPin,
                    RTL_TEXTENCODING_UTF8 ) );
            mAvailableClients.push_back( pClient );

            // Read off any additional non-empty lines
            // We know that we at least have the empty termination line to read.
            do
            {
                pSocket->readLine( aLine );
            }
            while ( aLine.getLength() > 0 );

            // Check if we already have this server.
            Reference< XNameAccess > xConfig = officecfg::Office::Impress::Misc::AuthorisedRemotes::get();
            Sequence< OUString > aNames = xConfig->getElementNames();
            bool aFound = false;
            for ( int i = 0; i < aNames.getLength(); i++ )
            {
                if ( aNames[i].equals( pClient->mName ) )
                {
                    Reference<XNameAccess> xSetItem( xConfig->getByName(aNames[i]), UNO_QUERY );
                    Any axPin(xSetItem->getByName("PIN"));
                    OUString sPin;
                    axPin >>= sPin;

                    if ( sPin.equals( pClient->mPin ) ) {
                        connectClient( pClient, sPin );
                        aFound = true;
                        break;
                    }
                }

            }
            // Pin not found so inform the client.
            if ( !aFound )
                pSocket->write( "LO_SERVER_VALIDATING_PIN\n\n",
                            strlen( "LO_SERVER_VALIDATING_PIN\n\n" ) );
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

    sd::BluetoothServer::setup( &(spServer->mCommunicators) );
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
    std::vector<ClientInfo*> aClients;
    if ( !spServer )
        return aClients;

    MutexGuard aGuard( spServer->mDataMutex );
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
        // Save in settings first
        boost::shared_ptr< ConfigurationChanges > aChanges = ConfigurationChanges::create();
        Reference< XNameContainer > xConfig = officecfg::Office::Impress::Misc::AuthorisedRemotes::get( aChanges );

        Reference<XSingleServiceFactory> xChildFactory (
            xConfig, UNO_QUERY);
        Reference<XNameReplace> xChild( xChildFactory->createInstance(), UNO_QUERY);
                Any aValue;
        if (xChild.is())
        {
            // Check whether the client is already saved
            bool aSaved = false;
            Sequence< OUString > aNames = xConfig->getElementNames();
            for ( int i = 0; i < aNames.getLength(); i++ )
            {
                if ( aNames[i].equals( apClient->mName ) )
                {
                    xConfig->replaceByName( apClient->mName, makeAny( xChild ) );
                    aSaved = true;
                    break;
                }
            }
            if ( !aSaved )
                xConfig->insertByName( apClient->mName, makeAny( xChild ) );
            aValue <<= OUString( apClient->mPin );
            xChild->replaceByName("PIN", aValue);
            aChanges->commit();
        }

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

bool RemoteServer::isBluetoothDiscoverable()
{
    return BluetoothServer::isDiscoverable();
}

void RemoteServer::setBluetoothDiscoverable( bool aDiscoverable )
{
    BluetoothServer::setDiscoverable( aDiscoverable );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
