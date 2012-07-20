/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ImagePreparer.hxx"

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <xmlsec/base64.h>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>

using namespace sd;
using namespace rtl;
using namespace osl;
using namespace ::com::sun::star;

ImagePreparer::ImagePreparer(
    const uno::Reference<presentation::XSlideShowController>& rxController,
    Transmitter *aTransmitter, rtl::Reference<ImagePreparer>& rRef )
 :  Thread( "ImagePreparer Thread" ),
    xController( rxController ),
    pTransmitter( aTransmitter ),
    mRef( rRef )
{
}

ImagePreparer::~ImagePreparer()
{
}

void ImagePreparer::execute()
{
    fprintf( stderr, "ImagePreparer running\n" );
    sal_uInt32 aSlides = xController->getSlideCount();
    for ( sal_uInt32 i = 0; i < aSlides; i++ )
    {
        if ( !xController->isRunning() ) // stopped/disposed of.
        {
            break;
        }
        sendPreview( i );
    }
    fprintf( stderr, "ImagePreparer done\n" );
    mRef.clear();
}

void ImagePreparer::sendPreview( sal_uInt32 aSlideNumber )
{
    sal_uInt64 aSize;
    uno::Sequence<sal_Int8> aImageData = preparePreview( aSlideNumber, 160, 120,
        aSize );
    if ( !xController->isRunning() )
        return;

    OUStringBuffer aStrBuffer;
    ::sax::Converter::encodeBase64( aStrBuffer, aImageData );

    OString aEncodedShortString = OUStringToOString(
        aStrBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );

    // Start the writing
    OStringBuffer aBuffer;

    aBuffer.append( "slide_preview\n" );

    aBuffer.append( OString::valueOf( sal_Int32( aSlideNumber ) ).getStr() );
    aBuffer.append( "\n" );

    aBuffer.append( aEncodedShortString.getStr() );
    aBuffer.append( "\n\n" );
    pTransmitter->addMessage( aBuffer.makeStringAndClear(),
        Transmitter::Priority::LOW );

}

uno::Sequence<sal_Int8> ImagePreparer::preparePreview(
    sal_uInt32 aSlideNumber, sal_uInt32 aWidth, sal_uInt32 aHeight,
    sal_uInt64 &rSize )
{
    OUString aFileURL;
    FileBase::createTempFile( 0, 0, &aFileURL );

    uno::Reference< lang::XMultiServiceFactory > xServiceManager(
        ::comphelper::getProcessServiceFactory(),
        uno::UNO_QUERY_THROW );

    uno::Reference< document::XFilter > xFilter(
        xServiceManager->createInstance(
        "com.sun.star.drawing.GraphicExportFilter" ) ,
        uno::UNO_QUERY_THROW );

    uno::Reference< document::XExporter > xExporter( xFilter,
        uno::UNO_QUERY_THROW );

    if ( !xController->isRunning() )
        return uno::Sequence<sal_Int8>();
    uno::Reference< lang::XComponent > xSourceDoc(
        xController->getSlideByIndex( aSlideNumber ),
        uno::UNO_QUERY_THROW );

    xExporter->setSourceDocument( xSourceDoc );

    uno::Sequence< beans::PropertyValue > aFilterData(3);

    aFilterData[0].Name = "PixelWidth";
    aFilterData[0].Value <<= aWidth;

    aFilterData[1].Name = "PixelHeight";
    aFilterData[1].Value <<= aHeight;

    aFilterData[2].Name = "ColorMode";
    aFilterData[2].Value <<= sal_Int32(0); // 0: Color, 1: B&W

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