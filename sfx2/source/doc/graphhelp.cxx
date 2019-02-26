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


#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
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
#include <vcl/graphicfilter.hxx>

#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include "graphhelp.hxx"
#include <bitmaps.hlst>

#include <memory>

using namespace css;

std::unique_ptr<SvMemoryStream> GraphicHelper::getFormatStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, ConvertDataFormat nFormat )
{
    std::unique_ptr<SvMemoryStream> pResult;
    if ( pGDIMeta )
    {
        std::unique_ptr<SvMemoryStream> pStream(new SvMemoryStream( 65535, 65535 ));
        Graphic aGraph( *pGDIMeta );
        if ( GraphicConverter::Export( *pStream, aGraph, nFormat ) == ERRCODE_NONE )
            pResult = std::move(pStream);
    }

    return pResult;
}


// static
void* GraphicHelper::getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta )
{
    void* pResult = nullptr;

#ifdef _WIN32
    if ( pGDIMeta )
    {
        OUString const aStr(".emf");
        ::utl::TempFile aTempFile( OUString(), true, &aStr );

        OUString aMetaFile = aTempFile.GetFileName();
        OUString aMetaURL = aTempFile.GetURL();

        std::unique_ptr<SvStream> pStream = ::utl::UcbStreamHelper::CreateStream( aMetaURL, StreamMode::STD_READWRITE );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            ErrCode nFailed = GraphicConverter::Export( *pStream, aGraph, ConvertDataFormat::EMF );
            pStream->Flush();
            pStream.reset();

            if ( !nFailed )
                pResult = GetEnhMetaFileW( o3tl::toW(aMetaFile.getStr()) );
        }
    }
#else
    (void)pGDIMeta;  // unused
#endif

    return pResult;
}


// static
void* GraphicHelper::getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize )
{
    void* pResult = nullptr;

#ifdef _WIN32
    if ( pGDIMeta )
    {
        SvMemoryStream pStream( 65535, 65535 );
        Graphic aGraph( *pGDIMeta );
        ErrCode nFailed = GraphicConverter::Export( pStream, aGraph, ConvertDataFormat::WMF );
        pStream.Flush();
        if ( !nFailed )
        {
            sal_Int32 nLength = pStream.TellEnd();
            if ( nLength > 22 )
            {
                HMETAFILE hMeta = SetMetaFileBitsEx( nLength - 22,
                                static_cast< const unsigned char*>( pStream.GetData() ) + 22 );

                if ( hMeta )
                {
                    HGLOBAL hMemory = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, sizeof( METAFILEPICT ) );

                    if ( hMemory )
                    {
                        METAFILEPICT* pMF = static_cast<METAFILEPICT*>(GlobalLock( hMemory ));

                        pMF->hMF = hMeta;
                        pMF->mm = MM_ANISOTROPIC;

                        MapMode aMetaMode = pGDIMeta->GetPrefMapMode();
                        MapMode aWinMode( MapUnit::Map100thMM );

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
                        pResult = static_cast<void*>(hMemory);
                    }
                    else
                           DeleteMetaFile( hMeta );
                }
            }
        }
    }
#else
    (void)pGDIMeta;  // unused
    (void)aMetaSize; // unused
#endif


    return pResult;
}


// static
bool GraphicHelper::getThumbnailFormatFromGDI_Impl(GDIMetaFile const * pMetaFile, const uno::Reference<io::XStream>& xStream)
{
    bool bResult = false;

    if (!pMetaFile || !xStream.is())
        return false;

    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xStream));

    if (pStream->GetError())
        return false;

    BitmapEx aResultBitmap;

    bResult = pMetaFile->CreateThumbnail(aResultBitmap, BmpConversion::N8BitColors, BmpScaleFlag::Default);

    if (!bResult || aResultBitmap.IsEmpty())
        return false;

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    if (rFilter.compressAsPNG(aResultBitmap, *pStream) != ERRCODE_NONE)
        return false;

    pStream->Flush();

    return !pStream->GetError();
}

// static
bool GraphicHelper::getThumbnailReplacement_Impl(const OUString& rResID, const uno::Reference< io::XStream >& xStream )
{
    bool bResult = false;
    if (!rResID.isEmpty() && xStream.is())
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        try
        {
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(graphic::GraphicProvider::create(xContext));
            const OUString aURL{"private:graphicrepository/" + rResID};

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
                bResult = true;
            }
        }
        catch(const uno::Exception&)
        {
        }
    }

    return bResult;
}

// static
OUString GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl( const OUString& aFactoryShortName )
{
    OUString sResult;

    if ( aFactoryShortName == "scalc" )
    {
        sResult = BMP_128X128_CALC_DOC;
    }
    else if ( aFactoryShortName == "sdraw" )
    {
        sResult = BMP_128X128_DRAW_DOC;
    }
    else if ( aFactoryShortName == "simpress" )
    {
        sResult = BMP_128X128_IMPRESS_DOC;
    }
    else if ( aFactoryShortName == "smath" )
    {
        sResult = BMP_128X128_MATH_DOC;
    }
    else if ( aFactoryShortName == "swriter" || aFactoryShortName.startsWith("swriter/") )
    {
        sResult = BMP_128X128_WRITER_DOC;
    }

    return sResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
