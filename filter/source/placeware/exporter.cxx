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

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <sax/tools/converter.hxx>
#include <osl/diagnose.h>

#include <vector>

#include "exporter.hxx"
#include "zip.hxx"
#include "tempfile.hxx"

using rtl::OUString;
using rtl::OString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::std;

using com::sun::star::beans::PropertyValue;
using com::sun::star::beans::XPropertySet;
using com::sun::star::presentation::XPresentationPage;
using com::sun::star::task::XStatusIndicator;

// -----------------------------------------------------------------------------

PlaceWareExporter::PlaceWareExporter(const Reference< XMultiServiceFactory > &rxMSF)
:   mxMSF( rxMSF )
{
}

// -----------------------------------------------------------------------------

PlaceWareExporter::~PlaceWareExporter()
{
}

// -----------------------------------------------------------------------------
class PageEntry
{
private:
    TempFile maTempFile;
    rtl::OUString maName;
    rtl::OUString maTitle;
    rtl::OUString maNotes;
    rtl::OUString maURL;

public:
    PageEntry();
    ~PageEntry();

    OUString getTempURL() { return maTempFile.getFileURL(); }

    void setName( const rtl::OUString& rName ) { maName = rName; }
    const rtl::OUString& getName() const { return maName; }

    void setTitle( const rtl::OUString& rTitle ) { maTitle = rTitle; }
    const rtl::OUString& getTitle() const { return maTitle; }

    void setNotes( const rtl::OUString& rNotes ) { maNotes = rNotes; }
    const rtl::OUString& getNotes() const { return maNotes; }

    void setURL( const rtl::OUString& rURL ) { maURL = rURL; }
    const rtl::OUString& getURL() const { return maURL; }
};

PageEntry::PageEntry()
: maTempFile( TempFile::createTempFileURL() )
{
}

PageEntry::~PageEntry()
{
}


static void encodeFile( osl::File& rSourceFile, Reference< XOutputStream >& xOutputStream ) throw( ::com::sun::star::uno::Exception )
{
    if( xOutputStream.is() )
    {
        sal_uInt64 nTemp( 0 );

        osl::File::RC nRC = rSourceFile.setPos( osl_Pos_End, 0  );
        if( osl::File::E_None == nRC )
        {
            nRC = rSourceFile.getPos( nTemp );
            if( osl::File::E_None == nRC )
            {
                nRC = rSourceFile.setPos( osl_Pos_Absolut, 0 );
            }
        }

        sal_Int32 nLen = static_cast<sal_Int32>(nTemp);

        if( osl::File::E_None != nRC )
            throw IOException();

        sal_Int32 nBufferSize = 3*1024; // !!! buffer size must be a factor of 3 for base64 to work
        Sequence< sal_Int8 > aInBuffer( nBufferSize < nLen ? nBufferSize : nLen );
        void* pInBuffer = aInBuffer.getArray();

        Sequence< sal_Int8 > aOutBuffer;
        sal_Int32 nRead;
        while( nLen )
        {
            nRC = rSourceFile.read( pInBuffer, aInBuffer.getLength(), nTemp );

            if( (nRC != osl::File::E_None) || (0 == nTemp) )
                throw IOException();

            nRead = static_cast<sal_Int32>( nTemp );

            if( nRead < aInBuffer.getLength() )
            {
                aInBuffer.realloc( nRead );
                pInBuffer = aInBuffer.getArray();
            }

            nLen -= nRead;

            rtl::OUStringBuffer aStrBuffer;
            ::sax::Converter::encodeBase64(aStrBuffer, aInBuffer);

            sal_Int32 nCount = aStrBuffer.getLength();

            if( aOutBuffer.getLength() != nCount )
                aOutBuffer.realloc( nCount );

            sal_Int8* pBytes = aOutBuffer.getArray();
            const sal_Unicode* pUnicode = aStrBuffer.getStr();

            while( nCount-- )
            {
                // since base64 is always ascii, we can cast safely
                *pBytes++ = static_cast<sal_Int8>(*pUnicode++);
            }

            xOutputStream->writeBytes( aOutBuffer );
        }
    }
}

static OString convertString( OUString aInput )
{
    OString aRet( aInput.getStr(), aInput.getLength(), RTL_TEXTENCODING_ASCII_US );
    aRet = aRet.replace( '\r', ' ' );
    aRet = aRet.replace( '\n', ' ' );

    return aRet;
}

static void createSlideFile( Reference< XComponent > xDoc, ZipFile& rZipFile, const rtl::OUString& rURL, vector< PageEntry* >& rPageEntries  ) throw( ::com::sun::star::uno::Exception )
{
    OString aInfo;

    const OString aNewLine( "\r\n" );
    OUString aTemp;

    Reference< XDocumentPropertiesSupplier > xDPS( xDoc, UNO_QUERY );
    Reference< XDocumentProperties > xDocProps( xDPS->getDocumentProperties() );

    aTemp = xDocProps->getTitle();
    if( aTemp.isEmpty() )
    {
        sal_Int32 nPos1 = rURL.lastIndexOf( (sal_Unicode)'/' );
        if( -1 != nPos1 )
        {
            sal_Int32 nPos2 = rURL.lastIndexOf( (sal_Unicode)'.' );
            if( nPos2 > nPos1 )
            {
                aTemp = rURL.copy( nPos1 + 1, nPos2 - nPos1 - 1 );
            }
            else
            {
                aTemp = rURL.copy( nPos1 + 1 );
            }
        }
        else
        {
            aTemp = rURL;
        }
    }

    aInfo += OString( "SlideSetName: " );
    aInfo += convertString( aTemp );
    aInfo += aNewLine;

    aTemp = xDocProps->getAuthor();

    if( !aTemp.isEmpty() )
    {
        aInfo += OString( "PresenterName: " );
        aInfo += convertString( aTemp );
        aInfo += aNewLine;
    }

    vector< PageEntry* >::iterator aIter( rPageEntries.begin() );
    vector< PageEntry* >::iterator aEnd( rPageEntries.end() );
    while( aIter != aEnd )
    {
        PageEntry* pEntry = (*aIter++);

        aInfo += OString( "slide: " );
        if( !pEntry->getTitle().isEmpty() )
        {
            aInfo += convertString( pEntry->getTitle() );
        }
        else
        {
            aInfo += convertString( pEntry->getName() );
        }
        aInfo += aNewLine;

        aInfo += OString( "type: gif");
        aInfo += aNewLine;

        aInfo += OString( "url: " );
        aInfo += convertString( pEntry->getURL() );
        aInfo += aNewLine;


        if( !pEntry->getNotes().isEmpty() )
        {
            aInfo += OString( "notes: " );
            aInfo += convertString( pEntry->getNotes() );
            aInfo += aNewLine;
        }
    }

    TempFile aInfoFile( TempFile::createTempFileURL() );

    osl::File::RC nRC;
    sal_uInt64 nTemp;

    nRC = aInfoFile.open( osl_File_OpenFlag_Write );
    if( osl::File::E_None == nRC )
    {
        nRC = aInfoFile.write( aInfo.getStr(), aInfo.getLength(), nTemp );
        if( osl::File::E_None == nRC )
        {
            nRC = aInfoFile.setPos( osl_Pos_Absolut, 0 );
            if( osl::File::E_None == nRC )
            {
                nRC = aInfoFile.close();
            }
        }
    }

    if( (osl::File::E_None != nRC) || !rZipFile.addFile( aInfoFile, OString( RTL_CONSTASCII_STRINGPARAM("slides.txt") ) ))
        throw IOException();
}

//#define PLACEWARE_DEBUG 1

sal_Bool PlaceWareExporter::doExport( Reference< XComponent > xDoc, Reference < XOutputStream > xOutputStream,
                                        const rtl::OUString& rURL, Reference < XInterface > /* xHandler */, Reference < XStatusIndicator >& xStatusIndicator )
{
    sal_Bool bRet = sal_False;

    mxGraphicExporter = Reference< XExporter >::query( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicExportFilter") ) ) );
    Reference< XDrawPagesSupplier > xDrawPagesSupplier(xDoc, UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        return sal_False;

    Reference< XIndexAccess > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );
    if(!xDrawPages.is())
        return sal_False;

    if(xStatusIndicator.is())
    {
        xStatusIndicator->start(OUString(  RTL_CONSTASCII_USTRINGPARAM( "PlaceWare:" )),xDrawPages->getCount());
    }

    Reference< XDrawPage > xDrawPage;

    osl::File::RC nRC;

#ifndef PLACEWARE_DEBUG
    TempFile aTempFile( TempFile::createTempFileURL() );
    nRC = aTempFile.open( osl_File_OpenFlag_Write|osl_File_OpenFlag_Read );
    OUString aURL( aTempFile.getFileURL() );
#else
    OUString aURL( RTL_CONSTASCII_USTRINGPARAM("file:///e:/test.zip") );
    osl::File::remove( aURL );
    osl::File aTempFile( aURL );
    nRC = aTempFile.open( osl_File_OpenFlag_Create|osl_File_OpenFlag_Write|osl_File_OpenFlag_Read );
#endif

    if( osl::File::E_None != nRC )
        return sal_False;

    vector< PageEntry* > aPageEntries;

    // Create new package...
    try
    {
        ZipFile aZipFile(aTempFile);

        // export slides as gifs and collect information for slides

        const sal_Int32 nPageCount = xDrawPages->getCount();
        sal_Int32 nPage;

        for( nPage = 0; nPage < nPageCount; nPage++)
        {
            xDrawPages->getByIndex(nPage) >>= xDrawPage;

            if( !xDrawPage.is() )
                continue;

            PageEntry* pEntry = exportPage( xDrawPage );
            aPageEntries.push_back( pEntry );

            OUString aName( RTL_CONSTASCII_USTRINGPARAM("i") );
            aName += OUString::valueOf( nPage );
            aName += OUString( RTL_CONSTASCII_USTRINGPARAM(".gif") );
            pEntry->setURL( aName );

            if(xStatusIndicator.is())
            {
                xStatusIndicator->setValue( nPage + 1 );
            }
        }

        // create the slide.txt file

        createSlideFile( xDoc, aZipFile, rURL, aPageEntries );

        // add gifs to zip
        vector< PageEntry* >::iterator aIter( aPageEntries.begin() );
        vector< PageEntry* >::iterator aEnd( aPageEntries.end() );
        while( aIter != aEnd )
        {
            PageEntry* pEntry = (*aIter++);

            osl::File aFile(pEntry->getTempURL() );
            const OUString aTemp( pEntry->getURL() );

            if( (osl::File::E_None != nRC) || !aZipFile.addFile( aFile, OString( aTemp.getStr(), aTemp.getLength(), RTL_TEXTENCODING_ASCII_US ) ) )
                throw IOException();
        }

        if(!aZipFile.close())
            throw IOException();

        encodeFile( aTempFile, xOutputStream );

        bRet = sal_True;
    }
    catch ( RuntimeException const & )
    {
    }
    catch ( Exception const & )
    {
    }

    vector< PageEntry* >::iterator aIter( aPageEntries.begin() );
    vector< PageEntry* >::iterator aEnd( aPageEntries.end() );
    while( aIter != aEnd )
    {
        delete (*aIter++);
    }

    if( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bRet;
}

// -----------------------------------------------------------------------------

PageEntry* PlaceWareExporter::exportPage( Reference< XDrawPage >&xDrawPage )
{
    Reference< XComponent > xComp( xDrawPage, UNO_QUERY );

    PageEntry* pEntry = new PageEntry();

    // get page name
    Reference< XNamed > xNamed( xDrawPage, UNO_QUERY );
    if( xNamed.is() )
        pEntry->setName( xNamed->getName() );

    // get title text from title presentation shape if available
    const OUString szTitleTextShape( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TitleTextShape") );
    const OUString szIsEmptyPresObj( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") );

    sal_Int32 nShapeCount = xDrawPage->getCount();
    sal_Int32 nShape;
    for( nShape = 0; nShape < nShapeCount; nShape++ )
    {
        Reference< XShape > xShape;
        xDrawPage->getByIndex( nShape ) >>= xShape;

        if( xShape.is() && xShape->getShapeType() == szTitleTextShape )
        {
            Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
            if( xPropSet.is() )
            {
                sal_Bool bIsEmpty = true;
                xPropSet->getPropertyValue( szIsEmptyPresObj ) >>= bIsEmpty;

                if( !bIsEmpty )
                {
                    Reference< XText > xText( xShape, UNO_QUERY );
                    if( xText.is() )
                    {
                        pEntry->setTitle( xText->getString() );
                    }
                }
            }
            break;
        }
    }

    // get notes text if available
    Reference< XPresentationPage > xPresPage( xDrawPage, UNO_QUERY );
    if( xPresPage.is() )
    {
        Reference< XDrawPage > xNotesPage( xPresPage->getNotesPage() );

        const OUString szNotesShape( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.NotesShape") );

        nShapeCount = xNotesPage->getCount();
        for( nShape = 0; nShape < nShapeCount; nShape++ )
        {
            Reference< XShape > xShape;
            xNotesPage->getByIndex( nShape ) >>= xShape;

            if( xShape.is() && (xShape->getShapeType() == szNotesShape) )
            {
                Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
                if( xPropSet.is() )
                {
                    sal_Bool bIsEmpty = true;
                    xPropSet->getPropertyValue( szIsEmptyPresObj ) >>= bIsEmpty;

                    if( !bIsEmpty )
                    {
                        Reference< XText > xText( xShape, UNO_QUERY );
                        if( xText.is() )
                        {
                            pEntry->setNotes( xText->getString() );
                        }
                    }
                }
                break;
            }
        }
    }

    // create the gif
    Reference< XFilter > xFilter( mxGraphicExporter, UNO_QUERY );

    Sequence< PropertyValue > aFilterData( 2 );
    aFilterData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Width") );
    aFilterData[0].Value <<= (sal_Int32)704;
    aFilterData[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Translucent") );
    aFilterData[1].Value <<= (sal_Bool)sal_False;

    Sequence< PropertyValue > aDescriptor( 3 );
    aDescriptor[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FilterName") );
    aDescriptor[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM("GIF") );
    aDescriptor[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("URL") );
    aDescriptor[1].Value <<= OUString( pEntry->getTempURL() );
    aDescriptor[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FilterData") );
    aDescriptor[2].Value <<= aFilterData;
    mxGraphicExporter->setSourceDocument( xComp );
    xFilter->filter( aDescriptor );

    return pEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
