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
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>


#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <xmlsec/base64.h>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/strbuf.hxx>

using namespace sd;
using namespace ::com::sun::star;
using rtl::OUString;
using rtl::OString;
using namespace ::osl;
using namespace std;

Receiver::Receiver( Transmitter *aTransmitter )
{
    pTransmitter = aTransmitter;
}

Receiver::~Receiver()
{
}

void Receiver::parseCommand( std::vector<OString> aCommand )
{
    uno::Reference<presentation::XSlideShowController> xSlideShowController;
    uno::Reference<presentation::XPresentation2> xPresentation;
    try {
        uno::Reference< lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference< frame::XFramesSupplier > xFramesSupplier( xServiceManager->createInstance(
        "com.sun.star.frame.Desktop" ) , uno::UNO_QUERY_THROW );
        uno::Reference< frame::XFrame > xFrame ( xFramesSupplier->getActiveFrame(), uno::UNO_QUERY_THROW );
        uno::Reference<presentation::XPresentationSupplier> xPS ( xFrame->getController()->getModel(), uno::UNO_QUERY_THROW);
        xPresentation = uno::Reference<presentation::XPresentation2>(
            xPS->getPresentation(), uno::UNO_QUERY_THROW);
        // Throws an exception if now slideshow running
        xSlideShowController =  uno::Reference<presentation::XSlideShowController>(
           xPresentation->getController(), uno::UNO_QUERY_THROW );
    }
    catch ( com::sun::star::uno::RuntimeException &e )
    {
        fprintf( stderr, "Error in retrieving Controller\n" );
        //return;
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
        if ( xSlideShowController.is() )
            xSlideShowController->gotoSlideIndex( aSlide );
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
    else if ( aCommand[0].equals( "presentation_resume" ) )
    {
        if ( xSlideShowController.is() )
        {
            xSlideShowController->resume();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
