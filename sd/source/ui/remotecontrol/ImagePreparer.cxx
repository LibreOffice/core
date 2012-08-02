/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "ImagePreparer.hxx"

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <xmlsec/base64.h>
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
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace ::sd;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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
    fprintf( stderr, "Preparing slide notes\n" );
    for ( sal_uInt32 i = 0; i < aSlides; i++ )
    {
        if ( !xController->isRunning() ) // stopped/disposed of.
        {
            break;
        }
        sendNotes( i );
    }
//      notesToHtml( 0 );
    mRef.clear();
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

void ImagePreparer::sendNotes( sal_uInt32 aSlideNumber )
{

    OString aNotes = prepareNotes( aSlideNumber );

    if ( aNotes.getLength() == 0 )
        return;

//     OUStringBuffer aStrBuffer;
//     ::sax::Converter::encodeBase64( aStrBuffer, aTemp );
//
//     OString aNotes = OUStringToOString(
//         aStrBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );

    if ( !xController->isRunning() )
        return;

    // Start the writing
    OStringBuffer aBuffer;

    aBuffer.append( "slide_notes\n" );

    aBuffer.append( OString::valueOf( sal_Int32( aSlideNumber ) ).getStr() );
    aBuffer.append( "\n" );

    aBuffer.append( "<html><body>" );
    aBuffer.append( aNotes );
    aBuffer.append( "</html></body>" );
    aBuffer.append( "\n\n" );
    pTransmitter->addMessage( aBuffer.makeStringAndClear(),
        Transmitter::Priority::LOW );
}

sal_Bool ExportTo( uno::Reference< drawing::XDrawPage>& aNotesPage, String aUrl );
OString ImagePreparer::notesToHtml( sal_uInt32 aSlideNumber )
{
    OString aRet("");

    OUString aFileURL;
    FileBase::createTempFile( 0, 0, &aFileURL );
    fprintf( stderr, OUStringToOString( aFileURL, RTL_TEXTENCODING_UTF8).getStr() );

    if ( !xController->isRunning() )
        return "";




    // Get the page
    uno::Reference< drawing::XDrawPage > xNotesPage;
    uno::Reference< drawing::XDrawPage > xSourceDoc(
        xController->getSlideByIndex( aSlideNumber ),
        uno::UNO_QUERY_THROW );

    uno::Reference<presentation::XPresentationPage> xPresentationPage(
        xSourceDoc, UNO_QUERY);
    if (xPresentationPage.is())
        xNotesPage = uno::Reference< drawing::XDrawPage >(
            xPresentationPage->getNotesPage(), uno::UNO_QUERY_THROW );
    else
        return "";


    ExportTo( xNotesPage, aFileURL );

    // FIXME: error handling.

//     File aFile( aFileURL );
//     aFile.open(0);
//     sal_uInt64 aRead;
//     rSize = 0;
//     aFile.getSize( rSize );
//     uno::Sequence<sal_Int8> aContents( rSize );

//     aFile.read( aContents.getArray(), rSize, aRead );
//     aFile.close();
//     File::remove( aFileURL );
//     return aContents;

    return aRet;
}

// Code copied from sdremote/source/presenter/PresenterNotesView.cxx
OString ImagePreparer::prepareNotes( sal_uInt32 aSlideNumber )
{
    OUString aRet("");

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


    static const ::rtl::OUString sNotesShapeName (
        "com.sun.star.presentation.NotesShape" );
    static const ::rtl::OUString sTextShapeName (
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
                    aRet += xText->getString();
                }
            }
            else
            {
                uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor (
                    xIndexAccess->getByIndex(nIndex), UNO_QUERY);
                if (xShapeDescriptor.is())
                {
                    ::rtl::OUString sType (xShapeDescriptor->getShapeType());
                    if (sType.equals(sNotesShapeName) || sType.equals(sTextShapeName))
                    {
                        uno::Reference<text::XTextRange> xText (
                            xIndexAccess->getByIndex(nIndex), UNO_QUERY);
                        if (xText.is())
                        {
                            aRet += xText->getString();
                        }
                    }
                }
            }
        }
    }
    return OUStringToOString(
        aRet, RTL_TEXTENCODING_UTF8 );
}

sal_Bool ExportTo( uno::Reference< drawing::XDrawPage>& aNotesPage, String aUrl )
{
    ::rtl::OUString aFilterName( "XHTML Draw File" );
    uno::Reference< document::XExporter > xExporter;

    {
        uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );

        uno::Sequence < beans::PropertyValue > aProps;
        uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
        if ( xFilters->hasByName( aFilterName ) )
            xFilters->getByName( aFilterName ) >>= aProps;
        else
            fprintf( stderr, "Couldn't find by name.\n" );

        ::rtl::OUString aFilterImplName;
        sal_Int32 nFilterProps = aProps.getLength();
        for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
        {
            const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
            if ( rFilterProp.Name.compareToAscii("FilterService") == 0 )
            {
                rFilterProp.Value >>= aFilterImplName;
                break;
            }
        }

        fprintf( stderr, "aName%s\n", OUStringToOString(aFilterImplName, RTL_TEXTENCODING_UTF8).getStr() );
        if ( !aFilterImplName.isEmpty() )
        {
            try{
            xExporter = uno::Reference< document::XExporter >
                ( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
            }catch(const uno::Exception&)
                {
                    xExporter.clear();
                    fprintf( stderr, "Couldn't create instance of filter.\n" );
                }
        }
    }

    if (xExporter.is())
        fprintf( stderr, "Is!\n" );
    else
        fprintf( stderr, "Isn't\n" );

    if ( xExporter.is() )
    {
        try{
        uno::Reference< lang::XComponent >  xComp( aNotesPage, uno::UNO_QUERY_THROW );
        uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );

        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aOldArgs ( 2 );
        aOldArgs[0].Name = "FileName";
        aOldArgs[0].Value <<= OUString( aUrl );
        aOldArgs[1].Name = "FilterName";
        aOldArgs[1].Value <<= OUString("com.sun.star.documentconversion.XSLTFilter");

        SfxMedium rMedium( aUrl , STREAM_STD_WRITE  );

        const com::sun::star::beans::PropertyValue * pOldValue = aOldArgs.getConstArray();
        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aArgs ( aOldArgs.getLength() );
        com::sun::star::beans::PropertyValue * pNewValue = aArgs.getArray();


        // put in the REAL file name, and copy all PropertyValues
        const OUString sOutputStream ( RTL_CONSTASCII_USTRINGPARAM ( "OutputStream" ) );
        const OUString sStream ( RTL_CONSTASCII_USTRINGPARAM ( "StreamForOutput" ) );
        sal_Bool bHasOutputStream = sal_False;
        sal_Bool bHasStream = sal_False;
        sal_Bool bHasBaseURL = sal_False;
        sal_Int32 i;
        sal_Int32 nEnd = aOldArgs.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue[i].Name == "FileName" )
                pNewValue[i].Value <<= OUString ( rMedium.GetName() );
            else if ( pOldValue[i].Name == sOutputStream )
                bHasOutputStream = sal_True;
            else if ( pOldValue[i].Name == sStream )
                bHasStream = sal_True;
            else if ( pOldValue[i].Name == "DocumentBaseURL" )
                bHasBaseURL = sal_True;
        }

        if ( !bHasOutputStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sOutputStream;
            aArgs[nEnd-1].Value <<= uno::Reference < io::XOutputStream > ( new utl::OOutputStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        // add stream as well, for OOX export and maybe others
        if ( !bHasStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sStream;
            aArgs[nEnd-1].Value <<= com::sun::star::uno::Reference < com::sun::star::io::XStream > ( new utl::OStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "DocumentBaseURL" ) );
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL( sal_True );
        }

        return xFilter->filter( aArgs );
        }catch(const uno::Exception&)
        {}
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */