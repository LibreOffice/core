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

#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentinfo.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include "Communicator.hxx"
#include "Listener.hxx"
#include "Receiver.hxx"
#include "RemoteServer.hxx"

using namespace sd;
using namespace std;
using namespace com::sun::star;
using namespace osl;

Communicator::Communicator( IBluetoothSocket *pSocket ):
    Thread( "CommunicatorThread" ),
    mpSocket( pSocket ),
    pTransmitter( 0 ),
    mListener( 0 )
{
}

Communicator::~Communicator()
{
}

/// Close the underlying socket from another thread to force
/// an early exit / termination
void Communicator::forceClose()
{
    if( mpSocket )
        mpSocket->close();
}

// Run as a thread
void Communicator::execute()
{
    pTransmitter = new Transmitter( mpSocket );
    pTransmitter->create();

    pTransmitter->addMessage( "LO_SERVER_SERVER_PAIRED\n\n",
                              Transmitter::PRIORITY_HIGH );
    Receiver aReceiver( pTransmitter );
    try {
        uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_QUERY );

        uno::Reference<presentation::XPresentationSupplier> xPS;
        if( xFrame.is() )
            xPS = uno::Reference<presentation::XPresentationSupplier>( xFrame->getController()->getModel(), uno::UNO_QUERY );
        uno::Reference<presentation::XPresentation2> xPresentation;
        if( xPS.is() )
            xPresentation = uno::Reference<presentation::XPresentation2>( xPS->getPresentation(), uno::UNO_QUERY );
        if ( xPresentation.is() && xPresentation->isRunning() )
        {
            presentationStarted( xPresentation->getController() );
        }
        else
        {
            pTransmitter->addMessage( "slideshow_finished\n\n",
                                      Transmitter::PRIORITY_HIGH );
        }

        OStringBuffer aBuffer;
            aBuffer.append( "slideshow_info\n" )
                .append( OUStringToOString( ::comphelper::DocumentInfo::getDocumentTitle( xFrame->getController()->getModel() ), RTL_TEXTENCODING_UTF8 ) )
               .append("\n\n");

        pTransmitter->addMessage( aBuffer.makeStringAndClear(), Transmitter::PRIORITY_LOW );
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
            aReceiver.pushCommand( aCommand );
            aCommand.clear();
        }
    }

    SAL_INFO ("sdremote", "Exiting transmission loop\n");

    disposeListener();

    pTransmitter->notifyFinished();
    pTransmitter->join();
    pTransmitter = NULL;

    delete mpSocket;


    RemoteServer::removeCommunicator( this );
}

void Communicator::informListenerDestroyed()
{
    if ( pTransmitter )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
