/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <utility>
#include <vector>

#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Security.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <osl/socket.hxx>

#include <sddll.hxx>

#include "DiscoveryService.hxx"
#include "Listener.hxx"
#include <RemoteServer.hxx>
#include "BluetoothServer.hxx"
#include "Communicator.hxx"
#include "BufferedStreamSocket.hxx"

using namespace sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
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

        ClientInfoInternal( const OUString& rName,
                            BufferedStreamSocket *pSocket,
                            OUString aPin ):
                ClientInfo( rName, false ),
                mpStreamSocket( pSocket ),
                mPin(std::move( aPin )) {}
    };
}

IPRemoteServer::IPRemoteServer()
    : Thread("IPRemoteServerThread")
{
    SAL_INFO("sdremote", "Instantiated IPRemoteServer");
}

IPRemoteServer::~IPRemoteServer()
{
}

void IPRemoteServer::execute()
{
    SAL_INFO("sdremote", "IPRemoteServer::execute called");
    osl::SocketAddr aAddr( u"0.0.0.0"_ustr, PORT );
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
        handleAcceptedConnection( pSocket );
    }
    SAL_INFO("sdremote", "shutting down IPRemoteServer");
    spServer = nullptr; // Object is destroyed when Thread::execute() ends.
}

void IPRemoteServer::handleAcceptedConnection( BufferedStreamSocket *pSocket )
{
    OString aLine;
    if ( ! ( pSocket->readLine( aLine)
             && aLine == "LO_SERVER_CLIENT_PAIR"
             && pSocket->readLine( aLine ) ) )
    {
        SAL_INFO( "sdremote", "client failed to send LO_SERVER_CLIENT_PAIR, ignoring" );
        delete pSocket;
        return;
    }

    OString aName( aLine );

    if ( ! pSocket->readLine( aLine ) )
    {
        delete pSocket;
        return;
    }
    OString aPin( aLine );

    SocketAddr aClientAddr;
    pSocket->getPeerAddr( aClientAddr );

    do
    {
        // Read off any additional non-empty lines
        // We know that we at least have the empty termination line to read.
        if ( ! pSocket->readLine( aLine ) ) {
            delete pSocket;
            return;
        }
    }
    while ( aLine.getLength() > 0 );

    MutexGuard aGuard(RemoteServer::sDataMutex);
    std::shared_ptr< ClientInfoInternal > pClient =
        std::make_shared<ClientInfoInternal>(
            OStringToOUString( aName, RTL_TEXTENCODING_UTF8 ),
            pSocket, OStringToOUString( aPin, RTL_TEXTENCODING_UTF8 ) );
    mAvailableClients.push_back( pClient );

    // Check if we already have this server.
    Reference< XNameAccess > const xConfig = officecfg::Office::Impress::Misc::AuthorisedRemotes::get();
    const Sequence< OUString > aNames = xConfig->getElementNames();
    for ( const auto& rName : aNames )
    {
        if ( rName == pClient->mName )
        {
            Reference<XNameAccess> xSetItem( xConfig->getByName(rName), UNO_QUERY );
            Any axPin(xSetItem->getByName(u"PIN"_ustr));
            OUString sPin;
            axPin >>= sPin;

            if ( sPin == pClient->mPin ) {
                SAL_INFO( "sdremote", "client found on validated list -- connecting" );
                connectClient( pClient, sPin );
                return;
            }
        }
    }

    // Pin not found so inform the client.
    SAL_INFO( "sdremote", "client not found on validated list" );
    pSocket->write( "LO_SERVER_VALIDATING_PIN\n\n",
                    strlen( "LO_SERVER_VALIDATING_PIN\n\n" ) );
}

IPRemoteServer *sd::IPRemoteServer::spServer = nullptr;
::osl::Mutex sd::RemoteServer::sDataMutex;
::std::vector<Communicator*> sd::RemoteServer::sCommunicators;

void IPRemoteServer::setup()
{
    if (spServer)
        return;

    spServer = new IPRemoteServer();
    spServer->launch();
}

void RemoteServer::presentationStarted( const css::uno::Reference<
                css::presentation::XSlideShowController > &rController )
{
    // note this can be invoked even when there is no IPRemoteServer instance
    // but there are communicators belonging to a BluetoothServer
    MutexGuard aGuard( sDataMutex );
    for ( const auto& rpCommunicator : sCommunicators )
    {
        rpCommunicator->presentationStarted( rController );
    }
}
void RemoteServer::presentationStopped()
{
    MutexGuard aGuard( sDataMutex );
    for ( const auto& rpCommunicator : sCommunicators )
    {
        rpCommunicator->disposeListener();
    }
}

void RemoteServer::removeCommunicator( Communicator const * mCommunicator )
{
    MutexGuard aGuard( sDataMutex );
    auto aIt = std::find(sCommunicators.begin(), sCommunicators.end(), mCommunicator);
    if (aIt != sCommunicators.end())
        sCommunicators.erase( aIt );
}

std::vector<std::shared_ptr<ClientInfo>> IPRemoteServer::getClients()
{
    SAL_INFO( "sdremote", "IPRemoteServer::getClients() called" );
    std::vector< std::shared_ptr< ClientInfo > > aClients;
    if ( spServer )
    {
        MutexGuard aGuard(RemoteServer::sDataMutex);
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
    const Sequence< OUString > aNames = xConfig->getElementNames();
    std::transform(aNames.begin(), aNames.end(), std::back_inserter(aClients),
        [](const OUString& rName) -> std::shared_ptr<ClientInfo> {
            return std::make_shared<ClientInfo>(rName, true); });

    return aClients;
}

bool IPRemoteServer::connectClient(const std::shared_ptr<ClientInfo>& pClient, std::u16string_view aPin)
{
    SAL_INFO("sdremote", "IPRemoteServer::connectClient called");
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
            Sequence< OUString > aNames = xConfig->getElementNames();
            if (comphelper::findValue(aNames, apClient->mName) != -1)
                xConfig->replaceByName( apClient->mName, Any( xChild ) );
            else
                xConfig->insertByName( apClient->mName, Any( xChild ) );
            aValue <<= apClient->mPin;
            xChild->replaceByName(u"PIN"_ustr, aValue);
            aChanges->commit();
        }

        Communicator* pCommunicator = new Communicator( std::unique_ptr<IBluetoothSocket>(apClient->mpStreamSocket) );
        MutexGuard aGuard(RemoteServer::sDataMutex);

        RemoteServer::sCommunicators.push_back( pCommunicator );

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

void IPRemoteServer::deauthoriseClient(const std::shared_ptr<ClientInfo>& pClient)
{
    // TODO: we probably want to forcefully disconnect at this point too?
    // But possibly via a separate function to allow just disconnecting from
    // the UI.

    SAL_INFO("sdremote", "IPRemoteServer::deauthoriseClient called");

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

    if ( !officecfg::Office::Impress::Misc::Start::EnableSdremote::get() )
        return;

#ifdef ENABLE_SDREMOTE_BLUETOOTH
    sd::BluetoothServer::setup( &RemoteServer::sCommunicators );
#endif

    if (!officecfg::Office::Security::Net::AllowInsecureImpressRemoteWiFi::get())
    {
        SAL_WARN("desktop", "Impress remote WiFi is disabled by configuration");
        return;
    }

    // this is the IP/WiFi server
    sd::IPRemoteServer::setup();
    // assumption is that BluetoothServer doesn't need DiscoveryService
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
