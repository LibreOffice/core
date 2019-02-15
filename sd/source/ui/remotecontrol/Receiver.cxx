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
#include <com/sun/star/uno/RuntimeException.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

using namespace sd;
using namespace ::osl;
using namespace std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::beans;

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

void Receiver::Invoke()
{
    if( !maExecQueue.empty() )
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
    uno::Reference<presentation::XSlideShow> xSlideShow;
    try {
        uno::Reference< frame::XDesktop2 > xFramesSupplier = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_QUERY_THROW );
        uno::Reference<presentation::XPresentationSupplier> xPS ( xFrame->getController()->getModel(), uno::UNO_QUERY_THROW);
        xPresentation.set( xPS->getPresentation(), uno::UNO_QUERY_THROW);
        // Throws an exception if now slideshow running
        xSlideShowController.set( xPresentation->getController(), uno::UNO_QUERY_THROW );
        xSlideShow.set( xSlideShowController->getSlideShow(), uno::UNO_QUERY_THROW );
    }
    catch (uno::RuntimeException &)
    {
    }

    if ( aCommand[0] ==  "transition_next" )
    {
        if ( xSlideShowController.is() )
            xSlideShowController->gotoNextEffect();
    }
    else if ( aCommand[0] == "transition_previous" )
    {
        if ( xSlideShowController.is() )
            xSlideShowController->gotoPreviousEffect();
    }
    else if ( aCommand[0] == "goto_slide" )
    {
        // FIXME: if 0 returned, then not a valid number
        sal_Int32 aSlide = aCommand[1].toInt32();
        if ( xSlideShowController.is() &&
            xSlideShowController->getCurrentSlideIndex() != aSlide )
        {
            xSlideShowController->gotoSlideIndex( aSlide );
        }
    }
    else if ( aCommand[0] == "presentation_start" )
    {
        if ( xPresentation.is() )
            xPresentation->start();
    }
    else if ( aCommand[0] == "presentation_stop" )
    {
        if ( xPresentation.is() )
            xPresentation->end();
    }
    else if ( aCommand[0] == "presentation_blank_screen" )
    {
        if ( aCommand.size() > 1 )
        {
//             aColour = FIXME: get the colour in some format from this string
//              Determine the formatting first.
        }
        if ( xSlideShowController.is() )
        {
            xSlideShowController->blankScreen( 0 ); // Default is black
        }
    }
    else if (aCommand[0] == "pointer_started" )
    {
        // std::cerr << "pointer_started" << std::endl;
        float x = aCommand[1].toFloat();
        float y = aCommand[2].toFloat();
        SolarMutexGuard aSolarGuard;

        const css::geometry::RealPoint2D pos(x,y);
        // std::cerr << "Pointer at ("<<pos.X<<","<<pos.Y<<")" << std::endl;

        if (xSlideShow.is()) try
        {
            // std::cerr << "pointer_coordination in the is" << std::endl;
            xSlideShow->setProperty(
                        beans::PropertyValue( "PointerPosition" ,
                            -1,
                            makeAny( pos ),
                            beans::PropertyState_DIRECT_VALUE ) );
        }
        catch ( Exception& )
        {
            SAL_WARN( "sdremote", "sd::SlideShowImpl::setPointerPosition(), "
                "exception caught: " << exceptionToString( cppu::getCaughtException() ));
        }

        if (xSlideShow.is()) try
        {
            xSlideShow->setProperty(
                        beans::PropertyValue( "PointerVisible" ,
                            -1,
                            makeAny( true ),
                            beans::PropertyState_DIRECT_VALUE ) );
        }
        catch ( Exception& )
        {
            SAL_WARN( "sdremote", "sd::SlideShowImpl::setPointerMode(), "
                "exception caught: " << exceptionToString( cppu::getCaughtException() ));
        }

        SAL_INFO( "sdremote", "Pointer started, we display the pointer on screen" );
    }
    else if (aCommand[0] == "pointer_dismissed" )
    {
        SolarMutexGuard aSolarGuard;
        if (xSlideShow.is()) try
        {
            xSlideShow->setProperty(
                        beans::PropertyValue( "PointerVisible" ,
                            -1,
                            makeAny( false ),
                            beans::PropertyState_DIRECT_VALUE ) );
        }
        catch ( Exception& )
        {
            SAL_WARN( "sdremote", "sd::SlideShowImpl::setPointerMode(), "
                "exception caught: " << exceptionToString( cppu::getCaughtException() ));
        }

        SAL_INFO( "sdremote", "Pointer dismissed, we hide the pointer on screen" );
    }
    else if (aCommand[0] == "pointer_coordination" )
    {
        float x = aCommand[1].toFloat();
        float y = aCommand[2].toFloat();

        SAL_INFO( "sdremote", "Pointer at ("<<x<<","<<y<<")" );
        const css::geometry::RealPoint2D pos(x,y);

        SolarMutexGuard aSolarGuard;
        if (xSlideShow.is()) try
        {
            xSlideShow->setProperty(
                        beans::PropertyValue( "PointerPosition" ,
                            -1,
                            makeAny( pos ),
                            beans::PropertyState_DIRECT_VALUE ) );
        }
        catch ( Exception& )
        {
            SAL_WARN( "sdremote", "sd::SlideShowImpl::setPointerPosition(), "
                "exception caught: " << exceptionToString( cppu::getCaughtException() ));
        }
    }
    else if ( aCommand[0] == "presentation_resume" )
    {
        if ( xSlideShowController.is() )
        {
            xSlideShowController->resume();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
