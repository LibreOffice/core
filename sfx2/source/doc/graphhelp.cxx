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


#ifdef WNT

#undef WB_LEFT
#undef WB_RIGHT

#include <tools/presys.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tools/postsys.h>

#endif
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XStream.hpp>


#include <osl/thread.h>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>

#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>


#include <sfx2/sfxresid.hxx>
#include "graphhelp.hxx"
#include "doc.hrc"

using namespace ::com::sun::star;

SvMemoryStream* GraphicHelper::getFormatStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, sal_uInt32 nFormat )
{
    SvMemoryStream* pResult = NULL;
    if ( pGDIMeta )
    {
        SvMemoryStream* pStream = new SvMemoryStream( 65535, 65535 );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            if ( GraphicConverter::Export( *pStream, aGraph, nFormat ) == 0 )
                pResult = pStream;
            else
                delete pStream;
        }
    }

    return pResult;
}


// static
void* GraphicHelper::getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta )
{
    (void)pGDIMeta;  // unused
    void* pResult = NULL;

#ifdef WNT
    if ( pGDIMeta )
    {
        OUString const aStr(".emf");
        ::utl::TempFile aTempFile( OUString(),
                                   &aStr,
                                   NULL,
                                   sal_False );

        OUString aMetaFile = aTempFile.GetFileName();
        OUString aMetaURL = aTempFile.GetURL();
        OString aWinFile = OUStringToOString( aMetaFile, osl_getThreadTextEncoding() );

        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aMetaURL, STREAM_STD_READWRITE );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            sal_Bool bFailed = (sal_Bool)GraphicConverter::Export( *pStream, aGraph, CVT_EMF );
            pStream->Flush();
            delete pStream;

            if ( !bFailed )
                pResult = GetEnhMetaFileA( aWinFile.getStr() );
        }
    }
#endif

    return pResult;
}


// static
void* GraphicHelper::getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize )
{
    (void)pGDIMeta;  // unused
    (void)aMetaSize; // unused
    void* pResult = NULL;

#ifdef WNT
    if ( pGDIMeta )
    {
        SvMemoryStream* pStream = new SvMemoryStream( 65535, 65535 );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            sal_Bool bFailed = (sal_Bool)GraphicConverter::Export( *pStream, aGraph, CVT_WMF );
            pStream->Flush();
            if ( !bFailed )
            {
                sal_Int32 nLength = pStream->Seek( STREAM_SEEK_TO_END );
                if ( nLength > 22 )
                {
                    HMETAFILE hMeta = SetMetaFileBitsEx( nLength - 22,
                                    ( reinterpret_cast< const unsigned char*>( pStream->GetData() ) ) + 22 );

                    if ( hMeta )
                    {
                        HGLOBAL hMemory = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, sizeof( METAFILEPICT ) );

                        if ( hMemory )
                        {
                               METAFILEPICT* pMF = (METAFILEPICT*)GlobalLock( hMemory );

                               pMF->hMF = hMeta;
                               pMF->mm = MM_ANISOTROPIC;

                            MapMode aMetaMode = pGDIMeta->GetPrefMapMode();
                            MapMode aWinMode( MAP_100TH_MM );

                            if ( aWinMode == pGDIMeta->GetPrefMapMode() )
                            {
                                pMF->xExt = aMetaSize.Width();
                                pMF->yExt = aMetaSize.Height();
                            }
                            else
                            {
                                Size aWinSize = OutputDevice::LogicToLogic( Size( aMetaSize.Width(), aMetaSize.Height() ),
                                                                            pGDIMeta->GetPrefMapMode(),
                                                                            aWinMode );
                                pMF->xExt = aWinSize.Width();
                                pMF->yExt = aWinSize.Height();
                            }

                            GlobalUnlock( hMemory );
                            pResult = (void*)hMemory;
                        }
                        else
                               DeleteMetaFile( hMeta );
                    }
                }
            }

            delete pStream;
        }
    }

#endif


    return pResult;
}


// static
sal_Bool GraphicHelper::supportsMetaFileHandle_Impl()
{
#ifdef WNT
    return sal_True;
#else
    return sal_False;
#endif
}


// static
sal_Bool GraphicHelper::mergeBitmaps_Impl( const BitmapEx& rBmpEx, const BitmapEx& rOverlay,
                   const Rectangle& rOverlayRect, BitmapEx& rReturn )
{
    // the implementation is provided by KA

    Point           aNullPt;
    Rectangle       aBmpRect( aNullPt, rBmpEx.GetSizePixel() );
    VirtualDevice   aVDev;

    if( !rReturn.IsEmpty() )
        rReturn.SetEmpty();

    if( !rBmpEx.IsEmpty() && aVDev.SetOutputSizePixel( aBmpRect.GetSize() ) )
    {
        Rectangle aOverlayRect( rOverlayRect );

        aOverlayRect.Intersection( aBmpRect );

        if( rOverlay.IsEmpty() || rOverlayRect.IsEmpty() )
            rReturn = rBmpEx;
        else
        {
            aVDev.DrawBitmap( aNullPt, aVDev.GetOutputSizePixel(), rBmpEx.GetBitmap() );
            aVDev.DrawBitmapEx( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), rOverlay );

            Bitmap aBmp( aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );
            aBmp.Convert( BMP_CONVERSION_24BIT );

            if( !rBmpEx.IsTransparent() )
                rReturn = aBmp;
            else
            {
                aVDev.DrawBitmap( aNullPt, aVDev.GetOutputSizePixel(), rBmpEx.GetMask() );
                Bitmap aOverlayMergeBmp( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ) );

                if( rOverlay.IsTransparent() )
                    aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), rOverlay.GetMask() );
                else
                {
                    aVDev.SetLineColor( COL_BLACK );
                    aVDev.SetFillColor( COL_BLACK );
                    aVDev.DrawRect( aOverlayRect);
                }

                aOverlayMergeBmp.CombineSimple( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ), BMP_COMBINE_AND );
                aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), aOverlayMergeBmp );
                rReturn = BitmapEx( aBmp, aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );
            }
        }
    }

    return !rReturn.IsEmpty();
}

// static
sal_Bool GraphicHelper::getThumbnailFormatFromGDI_Impl( GDIMetaFile* pMetaFile,
                                                        const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;
    SvStream* pStream = NULL;

    if ( xStream.is() )
        pStream = ::utl::UcbStreamHelper::CreateStream( xStream );

    if ( pMetaFile && pStream && !pStream->GetError() )
    {
        BitmapEx aResultBitmap;

        bResult = pMetaFile->CreateThumbnail(aResultBitmap);

        if ( bResult )
            bResult = ( !aResultBitmap.IsEmpty()
                        && GraphicConverter::Export( *pStream, aResultBitmap, CVT_PNG ) == 0
                        && ( pStream->Flush(), !pStream->GetError() ) );

        delete pStream;
    }

    return bResult;
}

// static
sal_Bool GraphicHelper::getThumbnailReplacement_Impl( sal_Int32 nResID, const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;
    if ( nResID && xStream.is() )
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        try
        {
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(graphic::GraphicProvider::create(xContext));
            OUString aURL("private:resource/sfx/bitmapex/");
            aURL += OUString::number( nResID );

            uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
            aMediaProps[0].Name = "URL";
            aMediaProps[0].Value <<= aURL;

            uno::Reference< graphic::XGraphic > xGraphic = xGraphProvider->queryGraphic( aMediaProps );
            if ( xGraphic.is() )
            {
                uno::Sequence< beans::PropertyValue > aStoreProps( 2 );
                aStoreProps[0].Name = "OutputStream";
                aStoreProps[0].Value <<= xStream;
                aStoreProps[1].Name = "MimeType";
                aStoreProps[1].Value <<= OUString("image/png");

                xGraphProvider->storeGraphic( xGraphic, aStoreProps );
                bResult = sal_True;
            }
        }
        catch(const uno::Exception&)
        {
        }
    }

    return bResult;
}


// static
sal_uInt16 GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl( const OUString& aFactoryShortName, sal_Bool /*bIsTemplate*/ )
{
    sal_uInt16 nResult = 0;

    if ( aFactoryShortName == "scalc" )
    {
        nResult = BMP_128X128_CALC_DOC;
    }
    else if ( aFactoryShortName == "sdraw" )
    {
        nResult = BMP_128X128_DRAW_DOC;
    }
    else if ( aFactoryShortName == "simpress" )
    {
        nResult = BMP_128X128_IMPRESS_DOC;
    }
    else if ( aFactoryShortName == "smath" )
    {
        nResult = BMP_128X128_MATH_DOC;
    }
    else if ( aFactoryShortName == "swriter" || aFactoryShortName.startsWith("swriter/") )
    {
        nResult = BMP_128X128_WRITER_DOC;
    }

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
