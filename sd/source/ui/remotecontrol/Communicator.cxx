/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <vector>

#include <comphelper/processfactory.hxx>

#include "Communicator.hxx"
#include "ImagePreparer.hxx"
#include "Listener.hxx"
#include "Receiver.hxx"
#include "RemoteServer.hxx"

using namespace sd;
using namespace std;
using namespace com::sun::star;
using namespace osl;

Communicator::Communicator( BufferedStreamSocket *pSocket ):
    Thread( "CommunicatorThread" ),
    mpSocket( pSocket ),
    pTransmitter( 0 ),
    mListener( 0 )
{
}

Communicator::~Communicator()
{
}

// Run as a thread
void Communicator::execute()
{
    pTransmitter = new Transmitter( mpSocket );
    pTransmitter->launch();

    pTransmitter->addMessage( "LO_SERVER_SERVER_PAIRED\n\n",
                              Transmitter::PRIORITY_HIGH );
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
        else
        {
            pTransmitter->addMessage( "slideshow_finished\n\n",
                                      Transmitter::PRIORITY_HIGH );
        }
    }
    catch (uno::RuntimeException &)
    {
    }

    sal_uInt64 aRet;
    vector<OString> aCommand;
    while ( true )
    {
        OString aLine;
        aRet = mpSocket->readLine( aLine );
        if ( aRet == 0 )
        {
            break; // I.e. transmission finished.
        }
        if ( aLine.getLength() )
        {
            aCommand.push_back( aLine );
        }
        else
        {
            aReceiver.parseCommand( aCommand );
            aCommand.clear();
        }
    }
    disposeListener();

    pTransmitter->notifyFinished();
    pTransmitter->join();
    pTransmitter = NULL;

    delete mpSocket;

    RemoteServer::removeCommunicator( this );
}

void Communicator::informListenerDestroyed()
{
    pTransmitter->addMessage( "slideshow_finished\n\n",
                               Transmitter::PRIORITY_HIGH );
    mListener.clear();
}

void Communicator::presentationStarted( const css::uno::Reference<
     css::presentation::XSlideShowController > &rController )
{
    if ( pTransmitter )
    {
        mListener = rtl::Reference<Listener>( new Listener( this, pTransmitter ) );
        mListener->init( rController );
    }
}

void Communicator::disposeListener()
{
    if ( mListener.is() )
    {
        mListener->disposing();
        mListener = NULL;
    }
}

Transmitter* Communicator::getTransmitter()
{
    return pTransmitter;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
