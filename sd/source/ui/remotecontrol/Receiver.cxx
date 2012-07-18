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
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

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
    mTransmitter = aTransmitter;
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
            // FIXME: remove later, this is just to test functionality
    sendPreview( 0, xSlideShowController, mTransmitter );

}

void sendPreview(sal_uInt32 aSlideNumber,
                 uno::Reference<presentation::XSlideShowController> xSlideShowController, Transmitter *aTransmitter )
{

    sal_uInt64 aSize; // Unused
    uno::Sequence<sal_Int8> aImageData = preparePreview( aSlideNumber, xSlideShowController, 320, 240, aSize );
    rtl::OUStringBuffer aStrBuffer;
    ::sax::Converter::encodeBase64( aStrBuffer, aImageData );

    OString aEncodedShortString = rtl::OUStringToOString(
        aStrBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );

    // Start the writing
    rtl::OStringBuffer aBuffer;

    aBuffer.append( "slide_preview\n" );

    rtl::OString aSlideNumberString( rtl::OString::valueOf( sal_Int32(2) ) ); // FIXME get number
    aBuffer.append( aSlideNumberString.getStr() );
    aBuffer.append( "\n" );

    aBuffer.append( aEncodedShortString.getStr() );
    aBuffer.append( "\n\n" );
    aTransmitter->addMessage( aBuffer.makeStringAndClear(), Transmitter::Priority::LOW );

}

uno::Sequence<sal_Int8>
preparePreview(sal_uInt32 aSlideNumber,
               const uno::Reference<presentation::XSlideShowController> &xSlideShowController,
               sal_uInt32 aWidth, sal_uInt32 aHeight, sal_uInt64 &rSize )
{
    (void)aWidth; (void)aHeight; // FIXME: remove me when I'm used
    // Create temp file
    OUString aFileURL;
    FileBase::createTempFile( 0, 0, &aFileURL );

    uno::Reference< lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );

    uno::Reference< document::XFilter > xFilter( xServiceManager->createInstance(
        "com.sun.star.drawing.GraphicExportFilter"  ) , uno::UNO_QUERY_THROW );

    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY_THROW );

    uno::Reference< lang::XComponent > xSourceDoc(
        xSlideShowController->getSlideByIndex( aSlideNumber ) , uno::UNO_QUERY_THROW );

    xExporter->setSourceDocument( xSourceDoc );

    uno::Sequence< beans::PropertyValue > aFilterData(3);
    aFilterData[0].Name = "PixelWidth";
    aFilterData[0].Value <<= sal_Int32(2000);
    aFilterData[1].Name = "PixelHeight";
    aFilterData[1].Value <<= sal_Int32(2000);

    // Add quality if jpg "Quality" [1-100]
    // FIXME: is setting color mode needed.
    aFilterData[2].Name = "ColorMode";
    aFilterData[2].Value <<= sal_Int32(0); // Color

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = "MediaType";
    aProps[0].Value <<= OUString( "image/png" );

    aProps[1].Name = "URL";
    aProps[1].Value <<= aFileURL;

    aProps[2].Name = "FilterData";
    aProps[2].Value <<= aFilterData;

    xFilter->filter( aProps );

    // FIXME: error handling.

    File aFile( aFileURL );
    aFile.open(0);
    sal_uInt64 aRead;
    rSize = 0;
    aFile.getSize( rSize );
    uno::Sequence<sal_Int8> aContents( rSize );

    aFile.read( aContents.getArray(), rSize, aRead );
    aFile.close();
    File::remove( aFileURL );
    return aContents;

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
