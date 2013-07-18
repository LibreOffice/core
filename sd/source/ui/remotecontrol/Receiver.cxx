/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "Receiver.hxx"
#include <string.h>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>


#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

using namespace sd;
using namespace ::com::sun::star;
using namespace ::osl;
using namespace std;

Receiver::Receiver( Transmitter *aTransmitter )
{
    pTransmitter = aTransmitter;
    SetTimeout( 0 );
}

Receiver::~Receiver()
{
}

// Bounce the commands to the main thread to avoid threading woes
void Receiver::pushCommand( const std::vector<OString> &rCommand )
{
    SolarMutexGuard aGuard;
    maExecQueue.push_back( rCommand );
    Start();
}

void Receiver::Timeout()
{
    if( maExecQueue.size() )
    {
        std::vector< OString > aCommands( maExecQueue.front() );
        maExecQueue.pop_front();
        if( !aCommands.empty() )
            executeCommand( aCommands );
        Start();
    }
    else
        Stop();
}

void Receiver::executeCommand( const std::vector<OString> &aCommand )
{
    uno::Reference<presentation::XSlideShowController> xSlideShowController;
    uno::Reference<presentation::XPresentation2> xPresentation;
    try {
        uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_QUERY_THROW );
        uno::Reference<presentation::XPresentationSupplier> xPS ( xFrame->getController()->getModel(), uno::UNO_QUERY_THROW);
        xPresentation = uno::Reference<presentation::XPresentation2>(
            xPS->getPresentation(), uno::UNO_QUERY_THROW);
        // Throws an exception if now slideshow running
        xSlideShowController =  uno::Reference<presentation::XSlideShowController>(
           xPresentation->getController(), uno::UNO_QUERY_THROW );
    }
    catch (uno::RuntimeException &)
    {
    }

    if ( aCommand[0].equals( "transition_next" ) )
    {
        if ( xSlideShowController.is() )
            xSlideShowController->gotoNextEffect();
    }
    else if ( aCommand[0].equals( "transition_previous" ) )
    {
        if ( xSlideShowController.is() )
            xSlideShowController->gotoPreviousEffect();
    }
    else if ( aCommand[0].equals( "goto_slide" ) )
    {
        // FIXME: if 0 returned, then not a valid number
        sal_Int32 aSlide = aCommand[1].toInt32();
        if ( xSlideShowController.is() &&
            xSlideShowController->getCurrentSlideIndex() != aSlide )
        {
            xSlideShowController->gotoSlideIndex( aSlide );
        }
    }
    else if ( aCommand[0].equals( "presentation_start" ) )
    {
        if ( xPresentation.is() )
            xPresentation->start();
    }
    else if ( aCommand[0].equals( "presentation_stop" ) )
    {
        if ( xPresentation.is() )
            xPresentation->end();
    }
    else if ( aCommand[0].equals( "presentation_blank_screen" ) )
    {
        sal_Int32 aColour = 0; // Default is black
        if ( aCommand.size() > 1 )
        {
//             aColour = FIXME: get the colour in some format from this string
//              Determine the formatting first.
        }
        if ( xSlideShowController.is() )
        {
            xSlideShowController->blankScreen( aColour );
        }
    }
    // pointer_coordination
    // x
    // y
    else if (aCommand[0].equals( "pointer_coordination" ))
    {
        float x = aCommand[1].toFloat();
        float y = aCommand[2].toFloat();
        std::cerr << "("<<x<<","<<y<<")"<< std::endl;
    }
    else if ( aCommand[0].equals( "presentation_resume" ) )
    {
        if ( xSlideShowController.is() )
        {
            xSlideShowController->resume();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
