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

#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Impress.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <sal/log.hxx>

#include "sddll.hxx"

#include "DiscoveryService.hxx"
#include "Listener.hxx"
#include "Receiver.hxx"
#include "RemoteServer.hxx"
#include "BluetoothServer.hxx"
#include "Communicator.hxx"
#include "BufferedStreamSocket.hxx"

using namespace std;
using namespace sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
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
        OUString mPin;

        ClientInfoInternal( const OUString rName,
                            const OUString rAddress,
                            BufferedStreamSocket *pSocket, OUString rPin ):
                ClientInfo( rName, rAddress ),
                mpStreamSocket( pSocket ),
                mPin( rPin ) {}
    };
}

RemoteServer::RemoteServer() :
    Thread( "RemoteServerThread" ),
    mSocket(),
    mAvailableClients()
{
    SAL_INFO( "sdremote", "Instantiated RemoteServer" );
}

RemoteServer::~RemoteServer()
{
}

void RemoteServer::execute()
{
    SAL_INFO( "sdremote", "RemoteServer::execute called" );
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if (!xContext.is()/* || !officecfg::Office::Common::Misc::ExperimentalMode::get(xContext)*/)
    {
        // SAL_INFO("sdremote", "not in experimental mode, disabling TCP server");
        spServer = NULL;
        return;
    }
    osl::SocketAddr aAddr( "0", PORT );
    if ( !mSocket.bind( aAddr ) )
    {
        SAL_WARN( "sdremote", "bind failed" << mSocket.getErrorAsString() );
        spServer = NULL;
        return;
    }

    if ( !mSocket.listen(3) )
    {
        SAL_WARN( "sdremote", "listen failed" << mSocket.getErrorAsString() );
        spServer = NULL;
        return;
    }
    while ( true )
    {
        StreamSocket aSocket;
        SAL_INFO( "sdremote", "waiting on accept" );
        if ( mSocket.acceptConnection( aSocket ) == osl_Socket_Error )
        {
            SAL_WARN( "sdremote", "accept failed" << mSocket.getErrorAsString() );
            spServer = NULL;
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

            MutexGuard aGuard( sDataMutex );
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
            Reference< XNameAccess > const xConfig = officecfg::Office::Impress::Misc::AuthorisedRemotes::get();
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
                        SAL_INFO( "sdremote", "client found on validated list -- connecting" );
                        connectClient( pClient, sPin );
                        aFound = true;
                        break;
                    }
                }

            }
            // Pin not found so inform the client.
            if ( !aFound )
            {
                SAL_INFO( "sdremote", "client not found on validated list" );
                pSocket->write( "LO_SERVER_VALIDATING_PIN\n\n",
                            strlen( "LO_SERVER_VALIDATING_PIN\n\n" ) );
            }
        } else {
            SAL_INFO( "sdremote", "client failed to send LO_SERVER_CLIENT_PAIR, ignoring" );
            delete pSocket;
        }
    }
    SAL_INFO( "sdremote", "shutting down RemoteServer" );
    spServer = NULL; // Object is destroyed when Thread::execute() ends.
}

RemoteServer *sd::RemoteServer::spServer = NULL;
::osl::Mutex sd::RemoteServer::sDataMutex;
::std::vector<Communicator*> sd::RemoteServer::sCommunicators;

void RemoteServer::setup()
{
    if (spServer)
        return;

    spServer = new RemoteServer();
    spServer->launch();

#ifdef ENABLE_SDREMOTE_BLUETOOTH
    sd::BluetoothServer::setup( &sCommunicators );
#endif
}


void RemoteServer::presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController )
{
    if ( !spServer )
        return;
    MutexGuard aGuard( sDataMutex );
    for ( vector<Communicator*>::const_iterator aIt = sCommunicators.begin();
         aIt != sCommunicators.end(); ++aIt )
    {
        (*aIt)->presentationStarted( rController );
    }
}
void RemoteServer::presentationStopped()
{
    if ( !spServer )
        return;
    MutexGuard aGuard( sDataMutex );
    for ( vector<Communicator*>::const_iterator aIt = sCommunicators.begin();
         aIt != sCommunicators.end(); ++aIt )
    {
        (*aIt)->disposeListener();
    }
}

void RemoteServer::removeCommunicator( Communicator* mCommunicator )
{
    if ( !spServer )
        return;
    MutexGuard aGuard( sDataMutex );
    for ( vector<Communicator*>::iterator aIt = sCommunicators.begin();
         aIt != sCommunicators.end(); ++aIt )
    {
        if ( mCommunicator == *aIt )
        {
            sCommunicators.erase( aIt );
            break;
        }
    }
}

std::vector<ClientInfo*> RemoteServer::getClients()
{
    SAL_INFO( "sdremote", "RemoteServer::getClients() called" );
    std::vector<ClientInfo*> aClients;
    if ( !spServer )
    {
        SAL_INFO( "sdremote", "No remote server instance => no clients" );
        return aClients;
    }

    MutexGuard aGuard( sDataMutex );
    aClients.assign( spServer->mAvailableClients.begin(),
                     spServer->mAvailableClients.end() );
    return aClients;
}

sal_Bool RemoteServer::connectClient( ClientInfo* pClient, OUString aPin )
{
    SAL_INFO( "sdremote", "RemoteServer::connectClient called" );
    if ( !spServer )
        return false;

    ClientInfoInternal *apClient = (ClientInfoInternal*) pClient;
    if ( apClient->mPin.equals( aPin ) )
    {
        // Save in settings first
        boost::shared_ptr< ConfigurationChanges > aChanges = ConfigurationChanges::create();
        Reference< XNameContainer > const xConfig = officecfg::Office::Impress::Misc::AuthorisedRemotes::get( aChanges );

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
        MutexGuard aGuard( sDataMutex );

        sCommunicators.push_back( pCommunicator );

        for ( vector<ClientInfoInternal*>::iterator aIt = spServer->mAvailableClients.begin();
            aIt != spServer->mAvailableClients.end(); ++aIt )
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
    SAL_INFO( "sdremote", "SdDLL::RegisterRemotes called" );
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if ( xContext.is()  && !officecfg::Office::Impress::Misc::Start::EnableSdremote::get( xContext ) )
        return;

    sd::RemoteServer::setup();
    sd::DiscoveryService::setup();
}

void RemoteServer::ensureDiscoverable()
{
    // FIXME: we could also enable listening on our WiFi
    // socket here to significantly reduce the attack surface.
#ifdef ENABLE_SDREMOTE_BLUETOOTH
    BluetoothServer::ensureDiscoverable();
#endif
}

void RemoteServer::restoreDiscoverable()
{
#ifdef ENABLE_SDREMOTE_BLUETOOTH
    BluetoothServer::restoreDiscoverable();
#endif
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
