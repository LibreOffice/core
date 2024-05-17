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
#include "Transmitter.hxx"

#include <comphelper/base64.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <utility>

using namespace ::sd;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

ImagePreparer::ImagePreparer(
    uno::Reference<presentation::XSlideShowController> _xController,
    Transmitter *aTransmitter )
 :  Timer("sd ImagePreparer"),
    xController(std::move( _xController )),
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
    ::comphelper::Base64::encode( aStrBuffer, aImageData );

    OString aEncodedShortString = OUStringToOString(
        aStrBuffer, RTL_TEXTENCODING_UTF8 );

    // Start the writing
    OString aBuffer =  "slide_preview\n" +
        OString::number(aSlideNumber) +
        "\n" + aEncodedShortString + "\n\n";
    pTransmitter->addMessage( aBuffer,
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

    uno::Sequence< beans::PropertyValue > aFilterData{
        comphelper::makePropertyValue(u"PixelWidth"_ustr, aWidth),
        comphelper::makePropertyValue(u"PixelHeight"_ustr, aHeight),
        comphelper::makePropertyValue(u"ColorMode"_ustr, sal_Int32(0)) // 0: Color, 1: B&W
    };

    uno::Sequence< beans::PropertyValue > aProps{
        comphelper::makePropertyValue(u"MediaType"_ustr, u"image/png"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, aFileURL),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData)
    };

    xFilter->filter( aProps );

    File aFile(aFileURL);
    if (aFile.open(0) != osl::File::E_None)
        return uno::Sequence<sal_Int8>();

    sal_uInt64 aRead;
    rSize = 0;
    aFile.getSize( rSize );
    uno::Sequence<sal_Int8> aContents( rSize );

    aFile.read( aContents.getArray(), rSize, aRead );
    if (aRead != rSize)
        aContents.realloc(aRead);

    aFile.close();
    File::remove( aFileURL );
    return aContents;

}

void ImagePreparer::sendNotes( sal_uInt32 aSlideNumber )
{

    OString aNotes = prepareNotes( aSlideNumber );

    if ( aNotes.isEmpty() )
        return;

    if ( !xController->isRunning() )
        return;

    // Start the writing
    OString aBuffer =
        "slide_notes\n" +
        OString::number( static_cast<sal_Int32>(aSlideNumber) ) +
        "\n"
        "<html><body>" +
        aNotes +
        "</body></html>"
        "\n\n";
    pTransmitter->addMessage( aBuffer,
        Transmitter::PRIORITY_LOW );
}

// Code copied from sdremote/source/presenter/PresenterNotesView.cxx
OString ImagePreparer::prepareNotes( sal_uInt32 aSlideNumber )
{
    OUStringBuffer aRet;

    if ( !xController->isRunning() )
        return ""_ostr;

    uno::Reference<css::drawing::XDrawPage> aNotesPage;
    uno::Reference< drawing::XDrawPage > xSourceDoc(
        xController->getSlideByIndex( aSlideNumber ),
        uno::UNO_SET_THROW );
    uno::Reference<presentation::XPresentationPage> xPresentationPage(
        xSourceDoc, UNO_QUERY);
    if (xPresentationPage.is())
        aNotesPage = xPresentationPage->getNotesPage();
    else
        return ""_ostr;

    static constexpr OUString sNotesShapeName (
        u"com.sun.star.presentation.NotesShape"_ustr );
    static constexpr OUStringLiteral sTextShapeName (
        u"com.sun.star.drawing.TextShape" );

    if (aNotesPage.is())
    {

        // Iterate over all shapes and find the one that holds the text.
        sal_Int32 nCount (aNotesPage->getCount());
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {

            uno::Reference<lang::XServiceName> xServiceName (
                aNotesPage->getByIndex(nIndex), UNO_QUERY);
            if (xServiceName.is()
                && xServiceName->getServiceName() == sNotesShapeName)
            {
                uno::Reference<text::XTextRange> xText (xServiceName, UNO_QUERY);
                if (xText.is())
                {
                    aRet.append(xText->getString() + "<br/>");
                }
            }
            else
            {
                uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor (
                    aNotesPage->getByIndex(nIndex), UNO_QUERY);
                if (xShapeDescriptor.is())
                {
                    OUString sType (xShapeDescriptor->getShapeType());
                    if (sType == sNotesShapeName || sType == sTextShapeName)
                    {
                        uno::Reference<text::XTextRange> xText (
                            aNotesPage->getByIndex(nIndex), UNO_QUERY);
                        if (xText.is())
                        {
                            aRet.append(xText->getString() + "<br/>");
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
    return OUStringToOString( aRet, RTL_TEXTENCODING_UTF8 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
