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


#include <sfx2/sfxresid.hxx>
#include "graphhelp.hxx"
#include "doc.hrc"

#include <memory>

using namespace css;

SvMemoryStream* GraphicHelper::getFormatStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, ConvertDataFormat nFormat )
{
    SvMemoryStream* pResult = nullptr;
    if ( pGDIMeta )
    {
        SvMemoryStream* pStream = new SvMemoryStream( 65535, 65535 );
        Graphic aGraph( *pGDIMeta );
        if ( GraphicConverter::Export( *pStream, aGraph, nFormat ) == 0 )
            pResult = pStream;
        else
            delete pStream;
    }

    return pResult;
}


// static
void* GraphicHelper::getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta )
{
    (void)pGDIMeta;  // unused
    void* pResult = nullptr;

#ifdef _WIN32
    if ( pGDIMeta )
    {
        OUString const aStr(".emf");
        ::utl::TempFile aTempFile( OUString(), true, &aStr );

        OUString aMetaFile = aTempFile.GetFileName();
        OUString aMetaURL = aTempFile.GetURL();
        OString aWinFile = OUStringToOString( aMetaFile, osl_getThreadTextEncoding() );

        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aMetaURL, StreamMode::STD_READWRITE );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            bool bFailed = GraphicConverter::Export( *pStream, aGraph, ConvertDataFormat::EMF );
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
    void* pResult = nullptr;

#ifdef _WIN32
    if ( pGDIMeta )
    {
        SvMemoryStream pStream( 65535, 65535 );
        Graphic aGraph( *pGDIMeta );
        bool bFailed = GraphicConverter::Export( pStream, aGraph, ConvertDataFormat::WMF );
        pStream.Flush();
        if ( !bFailed )
        {
            sal_Int32 nLength = pStream.Seek( STREAM_SEEK_TO_END );
            if ( nLength > 22 )
            {
                HMETAFILE hMeta = SetMetaFileBitsEx( nLength - 22,
                                ( static_cast< const unsigned char*>( pStream.GetData() ) ) + 22 );

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
#endif


    return pResult;
}


// static
bool GraphicHelper::getThumbnailFormatFromGDI_Impl(GDIMetaFile* pMetaFile, const uno::Reference<io::XStream>& xStream)
{
    bool bResult = false;

    if (!pMetaFile || !xStream.is())
        return false;

    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xStream));

    if (pStream->GetError())
        return false;

    BitmapEx aResultBitmap;

    bResult = pMetaFile->CreateThumbnail(aResultBitmap, 256, BmpConversion::N8BitColors, BmpScaleFlag::Default);

    if (!bResult || aResultBitmap.IsEmpty())
        return false;

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    if (rFilter.compressAsPNG(aResultBitmap, *pStream.get()) != GRFILTER_OK)
        return false;

    pStream->Flush();

    return !pStream->GetError();
}

// static
bool GraphicHelper::getThumbnailReplacement_Impl( sal_Int32 nResID, const uno::Reference< io::XStream >& xStream )
{
    bool bResult = false;
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
sal_uInt16 GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl( const OUString& aFactoryShortName, bool /*bIsTemplate*/ )
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
