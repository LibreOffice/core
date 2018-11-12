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
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <osl/socket.hxx>

#include <sddll.hxx>

#include "DiscoveryService.hxx"
#include "Listener.hxx"
#include "Receiver.hxx"
#include <RemoteServer.hxx>
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
        BufferedStreamSocket * const mpStreamSocket;
        OUString const mPin;

        ClientInfoInternal( const OUString& rName,
                            BufferedStreamSocket *pSocket,
                            const OUString& rPin ):
                ClientInfo( rName, false ),
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
        spServer = nullptr;
        return;
    }
    osl::SocketAddr aAddr( "0.0.0.0", PORT );
    if ( !mSocket.bind( aAddr ) )
    {
        SAL_WARN( "sdremote", "bind failed" << mSocket.getErrorAsString() );
        spServer = nullptr;
        return;
    }

    if ( !mSocket.listen(3) )
    {
        SAL_WARN( "sdremote", "listen failed" << mSocket.getErrorAsString() );
        spServer = nullptr;
        return;
    }
    while ( true )
    {
        StreamSocket aSocket;
        SAL_INFO( "sdremote", "waiting on accept" );
        if ( mSocket.acceptConnection( aSocket ) == osl_Socket_Error )
        {
            SAL_WARN( "sdremote", "accept failed" << mSocket.getErrorAsString() );
            spServer = nullptr;
            return; // Closed, or other issue.
        }
        BufferedStreamSocket *pSocket = new BufferedStreamSocket( aSocket);
        OString aLine;
        if ( pSocket->readLine( aLine)
            && aLine == "LO_SERVER_CLIENT_PAIR"
            && pSocket->readLine( aLine ) )
        {
            OString aName( aLine );

            if ( ! pSocket->readLine( aLine ) )
            {
                delete pSocket;
                continue;
            }
            OString aPin( aLine );

            SocketAddr aClientAddr;
            pSocket->getPeerAddr( aClientAddr );

            MutexGuard aGuard( sDataMutex );
            std::shared_ptr< ClientInfoInternal > pClient(
                new ClientInfoInternal(
                    OStringToOUString( aName, RTL_TEXTENCODING_UTF8 ),
                    pSocket, OStringToOUString( aPin, RTL_TEXTENCODING_UTF8 ) ) );
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
                if ( aNames[i] == pClient->mName )
                {
                    Reference<XNameAccess> xSetItem( xConfig->getByName(aNames[i]), UNO_QUERY );
                    Any axPin(xSetItem->getByName("PIN"));
                    OUString sPin;
                    axPin >>= sPin;

                    if ( sPin == pClient->mPin ) {
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
    spServer = nullptr; // Object is destroyed when Thread::execute() ends.
}

RemoteServer *sd::RemoteServer::spServer = nullptr;
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
    for ( const auto& rpCommunicator : sCommunicators )
    {
        rpCommunicator->presentationStarted( rController );
    }
}
void RemoteServer::presentationStopped()
{
    if ( !spServer )
        return;
    MutexGuard aGuard( sDataMutex );
    for ( const auto& rpCommunicator : sCommunicators )
    {
        rpCommunicator->disposeListener();
    }
}

void RemoteServer::removeCommunicator( Communicator const * mCommunicator )
{
    if ( !spServer )
        return;
    MutexGuard aGuard( sDataMutex );
    auto aIt = std::find(sCommunicators.begin(), sCommunicators.end(), mCommunicator);
    if (aIt != sCommunicators.end())
        sCommunicators.erase( aIt );
}

std::vector< std::shared_ptr< ClientInfo > > RemoteServer::getClients()
{
    SAL_INFO( "sdremote", "RemoteServer::getClients() called" );
    std::vector< std::shared_ptr< ClientInfo > > aClients;
    if ( spServer )
    {
        MutexGuard aGuard( sDataMutex );
        aClients.assign( spServer->mAvailableClients.begin(),
                         spServer->mAvailableClients.end() );
    }
    else
    {
        SAL_INFO( "sdremote", "No remote server instance => no remote clients" );
    }
    // We also need to provide authorised clients (no matter whether or not
    // they are actually available), so that they can be de-authorised if
    // necessary. We specifically want these to be at the end of the list
    // since the user is more likely to be trying to connect a new remote
    // than removing an existing remote.
    // We can also be sure that pre-authorised clients will not be on the
    // available clients list, as they get automatically connected if seen.
    // TODO: we should probably add some sort of extra labelling to mark
    // authorised AND connected client.
    Reference< XNameAccess > const xConfig = officecfg::Office::Impress::Misc::AuthorisedRemotes::get();
    Sequence< OUString > aNames = xConfig->getElementNames();
    for ( int i = 0; i < aNames.getLength(); i++ )
    {
        aClients.push_back( std::make_shared< ClientInfo > ( aNames[i], true ) );
    }

    return aClients;
}

bool RemoteServer::connectClient( const std::shared_ptr< ClientInfo >& pClient, const OUString& aPin )
{
    SAL_INFO( "sdremote", "RemoteServer::connectClient called" );
    if ( !spServer )
        return false;

    ClientInfoInternal* apClient = dynamic_cast< ClientInfoInternal* >( pClient.get() );
    if ( !apClient )
    // could happen if we try to "connect" an already authorised client
    {
        return false;
    }

    if ( apClient->mPin == aPin )
    {
        // Save in settings first
        std::shared_ptr< ConfigurationChanges > aChanges = ConfigurationChanges::create();
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
                if ( aNames[i] == apClient->mName )
                {
                    xConfig->replaceByName( apClient->mName, makeAny( xChild ) );
                    aSaved = true;
                    break;
                }
            }
            if ( !aSaved )
                xConfig->insertByName( apClient->mName, makeAny( xChild ) );
            aValue <<= apClient->mPin;
            xChild->replaceByName("PIN", aValue);
            aChanges->commit();
        }

        Communicator* pCommunicator = new Communicator( std::unique_ptr<IBluetoothSocket>(apClient->mpStreamSocket) );
        MutexGuard aGuard( sDataMutex );

        sCommunicators.push_back( pCommunicator );

        auto aIt = std::find(spServer->mAvailableClients.begin(), spServer->mAvailableClients.end(), pClient);
        if (aIt != spServer->mAvailableClients.end())
            spServer->mAvailableClients.erase( aIt );
        pCommunicator->launch();
        return true;
    }
    else
    {
        return false;
    }
}

void RemoteServer::deauthoriseClient( const std::shared_ptr< ClientInfo >& pClient )
{
    // TODO: we probably want to forcefully disconnect at this point too?
    // But possibly via a separate function to allow just disconnecting from
    // the UI.

    SAL_INFO( "sdremote", "RemoteServer::deauthoriseClient called" );

    if ( !pClient->mbIsAlreadyAuthorised )
    // We can't remove unauthorised clients from the authorised list...
    {
        return;
    }

    std::shared_ptr< ConfigurationChanges > aChanges = ConfigurationChanges::create();
    Reference< XNameContainer > const xConfig =
        officecfg::Office::Impress::Misc::AuthorisedRemotes::get( aChanges );

    xConfig->removeByName( pClient->mName );
    aChanges->commit();
}

void SdDLL::RegisterRemotes()
{
    SAL_INFO( "sdremote", "SdDLL::RegisterRemotes called" );

    // The remote server is likely of no use in headless mode. And as only
    // one instance of the server can actually own the appropriate ports its
    // probably best to not even try to do so from our headless instance
    // (i.e. as to avoid blocking expected usage).
    // It could perhaps be argued that we would still need the remote
    // server for tiled rendering of presentations, but even then this
    // implementation would not be of much use, i.e. would be controlling
    // the purely imaginary headless presentation -- instead we'd need
    // to have some sort of mechanism of plugging in our tiled rendering
    // client to be controlled by the remote server, or provide an
    // alternative implementation.
    if ( Application::IsHeadlessModeEnabled() )
        return;

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
