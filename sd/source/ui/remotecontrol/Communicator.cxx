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
#include <config_version.h>
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
    pTransmitter( nullptr ),
    mListener( nullptr )
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

    OStringBuffer aServerInformationBuffer;
    aServerInformationBuffer.append( "LO_SERVER_INFO\n" LIBO_VERSION_DOTTED "\n\n" );

    pTransmitter->addMessage( aServerInformationBuffer.makeStringAndClear(), Transmitter::PRIORITY_HIGH );

    Receiver aReceiver( pTransmitter );
    try {
        uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_QUERY );

        uno::Reference<presentation::XPresentationSupplier> xPS;
        if( xFrame.is() )
            xPS.set( xFrame->getController()->getModel(), uno::UNO_QUERY );
        uno::Reference<presentation::XPresentation2> xPresentation;
        if( xPS.is() )
            xPresentation.set( xPS->getPresentation(), uno::UNO_QUERY );
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
        aBuffer
          .append( "slideshow_info\n" )
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
    pTransmitter = nullptr;

    mpSocket->close();
    delete mpSocket;
    mpSocket = nullptr;

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
        mListener.set( new Listener( this, pTransmitter ) );
        mListener->init( rController );
    }
}

void Communicator::disposeListener()
{
    if ( mListener.is() )
    {
        mListener->disposing();
        mListener = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
