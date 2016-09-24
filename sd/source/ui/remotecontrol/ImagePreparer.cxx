/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ImagePreparer.hxx"

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/strbuf.hxx>
#include <unotools/streamwrap.hxx>

#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace ::sd;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

ImagePreparer::ImagePreparer(
    const uno::Reference<presentation::XSlideShowController>& rxController,
    Transmitter *aTransmitter )
 :  xController( rxController ),
    pTransmitter( aTransmitter )
{
    SAL_INFO( "sdremote", "ImagePreparer - start" );
    SetTimeout( 50 );
    mnSendingSlide = 0;
    Start();
}

ImagePreparer::~ImagePreparer()
{
    SAL_INFO( "sdremote", "ImagePreparer - stop" );
    Stop();
}

void ImagePreparer::Invoke()
{
    sal_uInt32 aSlides = xController->getSlideCount();
    SAL_INFO( "sdremote", "ImagePreparer " << xController->isRunning() <<
              " sending slide " << mnSendingSlide << " of " << aSlides );
    if ( xController->isRunning() && // not stopped/disposed of.
         mnSendingSlide < aSlides )
    {
        sendPreview( mnSendingSlide );
        sendNotes( mnSendingSlide );
        mnSendingSlide++;
        Start();
    }
    else
        Stop();
}

void ImagePreparer::sendPreview( sal_uInt32 aSlideNumber )
{
    sal_uInt64 aSize;
    uno::Sequence<sal_Int8> aImageData = preparePreview( aSlideNumber, 320, 240,
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

    aBuffer.append( OString::number( aSlideNumber ).getStr() );
    aBuffer.append( "\n" );

    aBuffer.append( aEncodedShortString.getStr() );
    aBuffer.append( "\n\n" );
    pTransmitter->addMessage( aBuffer.makeStringAndClear(),
        Transmitter::PRIORITY_LOW );

}

uno::Sequence<sal_Int8> ImagePreparer::preparePreview(
    sal_uInt32 aSlideNumber, sal_uInt32 aWidth, sal_uInt32 aHeight,
    sal_uInt64 &rSize )
{
    OUString aFileURL;
    FileBase::createTempFile( nullptr, nullptr, &aFileURL );

    uno::Reference< drawing::XGraphicExportFilter > xFilter =
        drawing::GraphicExportFilter::create( ::comphelper::getProcessComponentContext() );

    if ( !xController->isRunning() )
        return uno::Sequence<sal_Int8>();

    uno::Reference< lang::XComponent > xSourceDoc(
        xController->getSlideByIndex( aSlideNumber ),
        uno::UNO_QUERY_THROW );

    xFilter->setSourceDocument( xSourceDoc );

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

void ImagePreparer::sendNotes( sal_uInt32 aSlideNumber )
{

    OString aNotes = prepareNotes( aSlideNumber );

    if ( aNotes.isEmpty() )
        return;

//     OUStringBuffer aStrBuffer;
//     ::sax::Converter::encodeBase64( aStrBuffer, aTemp );

//     OString aNotes = OUStringToOString(
//         aStrBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );

    if ( !xController->isRunning() )
        return;

    // Start the writing
    OStringBuffer aBuffer;

    aBuffer.append( "slide_notes\n" );

    aBuffer.append( OString::number( aSlideNumber ).getStr() );
    aBuffer.append( "\n" );

    aBuffer.append( "<html><body>" );
    aBuffer.append( aNotes );
    aBuffer.append( "</body></html>" );
    aBuffer.append( "\n\n" );
    pTransmitter->addMessage( aBuffer.makeStringAndClear(),
        Transmitter::PRIORITY_LOW );
}

// Code copied from sdremote/source/presenter/PresenterNotesView.cxx
OString ImagePreparer::prepareNotes( sal_uInt32 aSlideNumber )
{
    OUStringBuffer aRet;

    if ( !xController->isRunning() )
        return "";

    uno::Reference<css::drawing::XDrawPage> aNotesPage;
    uno::Reference< drawing::XDrawPage > xSourceDoc(
        xController->getSlideByIndex( aSlideNumber ),
        uno::UNO_QUERY_THROW );
    uno::Reference<presentation::XPresentationPage> xPresentationPage(
        xSourceDoc, UNO_QUERY);
    if (xPresentationPage.is())
        aNotesPage = xPresentationPage->getNotesPage();
    else
        return "";

    static const OUString sNotesShapeName (
        "com.sun.star.presentation.NotesShape" );
    static const OUString sTextShapeName (
        "com.sun.star.drawing.TextShape" );

    uno::Reference<container::XIndexAccess> xIndexAccess ( aNotesPage, UNO_QUERY);
    if (xIndexAccess.is())
    {

        // Iterate over all shapes and find the one that holds the text.
        sal_Int32 nCount (xIndexAccess->getCount());
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {

            uno::Reference<lang::XServiceName> xServiceName (
                xIndexAccess->getByIndex(nIndex), UNO_QUERY);
            if (xServiceName.is()
                && xServiceName->getServiceName().equals(sNotesShapeName))
            {
                uno::Reference<text::XTextRange> xText (xServiceName, UNO_QUERY);
                if (xText.is())
                {
                    aRet.append(xText->getString());
                    aRet.append("<br/>");
                }
            }
            else
            {
                uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor (
                    xIndexAccess->getByIndex(nIndex), UNO_QUERY);
                if (xShapeDescriptor.is())
                {
                    OUString sType (xShapeDescriptor->getShapeType());
                    if (sType.equals(sNotesShapeName) || sType.equals(sTextShapeName))
                    {
                        uno::Reference<text::XTextRange> xText (
                            xIndexAccess->getByIndex(nIndex), UNO_QUERY);
                        if (xText.is())
                        {
                            aRet.append(xText->getString());
                            aRet.append("<br/>");
                        }
                    }
                }
            }
        }
    }
    // Replace all newlines with <br\> tags
    for ( sal_Int32 i = 0; i < aRet.getLength(); i++ )
    {
        if ( aRet[i] == '\n' )
        {
            aRet[i]=  '<';
            aRet.insert( i+1, "br/>" );
        }
    }
    return OUStringToOString(
        aRet.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
