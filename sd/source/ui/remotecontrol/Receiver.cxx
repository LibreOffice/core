/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "Receiver.hxx"
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/svapp.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::beans;

Receiver::Receiver( Transmitter *aTransmitter ) : Timer("sd Receiver")
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
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_SET_THROW );
        uno::Reference<presentation::XPresentationSupplier> xPS ( xFrame->getController()->getModel(), uno::UNO_QUERY_THROW);
        xPresentation.set( xPS->getPresentation(), uno::UNO_QUERY_THROW);
        // Throws an exception if no slideshow running
        xSlideShowController.set( xPresentation->getController(), uno::UNO_SET_THROW );
        xSlideShow.set( xSlideShowController->getSlideShow(), uno::UNO_SET_THROW );
    }
    catch (uno::RuntimeException &)
    {
    }

    if (aCommand.empty())
    {
        SAL_WARN("sdremote", "Receiver::executeCommand: no command");
        return;
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
        if (aCommand.size() < 2)
        {
            SAL_WARN("sdremote", "Receiver::executeCommand: invalid goto_slide");
            return;
        }
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
        if (aCommand.size() < 3)
        {
            SAL_WARN("sdremote", "Receiver::executeCommand: invalid pointer_started");
            return;
        }
        // std::cerr << "pointer_started" << std::endl;
        float x = aCommand[1].toFloat();
        float y = aCommand[2].toFloat();
        SolarMutexGuard aSolarGuard;

        const css::geometry::RealPoint2D pos(x,y);
        // std::cerr << "Pointer at ("<<pos.X<<","<<pos.Y<<")" << std::endl;

        if (xSlideShow.is())
        {
            try
            {
                // std::cerr << "pointer_coordination in the is" << std::endl;
                xSlideShow->setProperty(beans::PropertyValue("PointerPosition", -1, Any(pos),
                                                             beans::PropertyState_DIRECT_VALUE));
            }
            catch (Exception&)
            {
                TOOLS_WARN_EXCEPTION("sdremote", "sd::SlideShowImpl::setPointerPosition()");
            }

            try
            {
                xSlideShow->setProperty(beans::PropertyValue("PointerVisible", -1, Any(true),
                                                             beans::PropertyState_DIRECT_VALUE));
            }
            catch (Exception&)
            {
                TOOLS_WARN_EXCEPTION("sdremote", "sd::SlideShowImpl::setPointerMode()");
            }
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
                            Any( false ),
                            beans::PropertyState_DIRECT_VALUE ) );
        }
        catch ( Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sdremote", "sd::SlideShowImpl::setPointerMode()" );
        }

        SAL_INFO( "sdremote", "Pointer dismissed, we hide the pointer on screen" );
    }
    else if (aCommand[0] == "pointer_coordination" )
    {
        if (aCommand.size() < 3)
        {
            SAL_WARN("sdremote", "Receiver::executeCommand: invalid pointer_coordination");
            return;
        }
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
                            Any( pos ),
                            beans::PropertyState_DIRECT_VALUE ) );
        }
        catch ( Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sdremote", "sd::SlideShowImpl::setPointerPosition()" );
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
