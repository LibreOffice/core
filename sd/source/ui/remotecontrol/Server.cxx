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

RemoteServer::RemoteServer()
:  Thread( "RemoteServerThread" ), mSocket()
{
}

RemoteServer::~RemoteServer()
{
}

// Run as a thread
void RemoteServer::listenThread()
{
    pTransmitter = new Transmitter( mStreamSocket );
    pTransmitter->launch();
    Receiver aReceiver( pTransmitter );
    try {
        uno::Reference< lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference< frame::XFramesSupplier > xFramesSupplier( xServiceManager->createInstance(
        "com.sun.star.frame.Desktop" ) , uno::UNO_QUERY_THROW );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_QUERY_THROW );
        uno::Reference<presentation::XPresentationSupplier> xPS ( xFrame->getController()->getModel(), uno::UNO_QUERY_THROW);
        uno::Reference<presentation::XPresentation2> xPresentation(
            xPS->getPresentation(), uno::UNO_QUERY_THROW);
        if ( xPresentation->isRunning() )
        {
            presentationStarted( xPresentation->getController() );
        }
    }
    catch ( com::sun::star::uno::RuntimeException &e )
    {
    }

    sal_uInt64 aRet, aRead;
    vector<char> aBuffer;
    vector<OString> aCommand;
    aRead = 0;
    while ( true )
    {
        aBuffer.resize( aRead + 100 );
        aRet = mStreamSocket.recv( &aBuffer[aRead], 100 );
        if ( aRet == 0 )
        {
            break; // I.e. transmission finished.
        }
        aRead += aRet;
        vector<char>::iterator aIt;
        while ( (aIt = find( aBuffer.begin(), aBuffer.end(), '\n' ))
            != aBuffer.end() )
        {
            sal_uInt64 aLocation = aIt - aBuffer.begin();

            aCommand.push_back( OString( &(*aBuffer.begin()), aLocation ) );
            if ( aIt == aBuffer.begin() )
            {
                aReceiver.parseCommand( aCommand );
                aCommand.clear();
            }
            aBuffer.erase( aBuffer.begin(), aIt + 1 ); // Also delete the empty line
            aRead -= (aLocation + 1);
        }
    }
    // TODO: deal with transmision errors gracefully.
    presentationStopped();

    delete pTransmitter;
    pTransmitter = NULL;
    fprintf( stderr, "Finished listening\n" );
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
        fprintf( stderr, "Awaiting a connection.\n" );
        mSocket.acceptConnection( mStreamSocket );
        fprintf( stderr, "Accepted a connection!\n" );
        listenThread();
    }

}

void RemoteServer::informListenerDestroyed()
{
    mListener.clear();
}

void RemoteServer::presentationStarted( const css::uno::Reference<
     css::presentation::XSlideShowController > &rController )
{
    if ( pTransmitter )
    {
        mListener = rtl::Reference<Listener>( new Listener( spServer, pTransmitter ) );
        mListener->init( rController );
    }
}

void RemoteServer::presentationStopped()
{
    if ( mListener.is() )
    {
        mListener->disposing();
        mListener = NULL;
    }
}

RemoteServer *sd::RemoteServer::spServer = NULL;
Transmitter *sd::RemoteServer::pTransmitter = NULL;
rtl::Reference<Listener> sd::RemoteServer::mListener = NULL;

void RemoteServer::setup()
{
  if (spServer)
    return;

  spServer = new RemoteServer();
  spServer->launch();
}

void SdDLL::RegisterRemotes()
{
  fprintf( stderr, "Register our remote control goodness\n" );
  sd::RemoteServer::setup();
  sd::DiscoveryService::setup();

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
