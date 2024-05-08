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

#include "imgprod.hxx"

#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <utility>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/ImageStatus.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <svtools/imageresourceaccess.hxx>
#include <comphelper/processfactory.hxx>


ImageProducer::ImageProducer()
    : mpStm(nullptr)
    , mnTransIndex(0)
    , mbConsInit(false)
{
    moGraphic.emplace();
}

ImageProducer::~ImageProducer()
{
}


// XInterface
css::uno::Any ImageProducer::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::lang::XInitialization* >(this),
                                        static_cast< css::lang::XServiceInfo* >(this),
                                        static_cast< css::awt::XImageProducer* >(this) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}


void ImageProducer::addConsumer( const css::uno::Reference< css::awt::XImageConsumer >& rxConsumer )
{
    DBG_ASSERT( rxConsumer.is(), "::AddConsumer(...): No consumer referenced!" );
    if( rxConsumer.is() )
        maConsList.push_back( rxConsumer );
}


void ImageProducer::removeConsumer( const css::uno::Reference< css::awt::XImageConsumer >& rxConsumer )
{
    ConsumerList_t::reverse_iterator riter = std::find(maConsList.rbegin(),maConsList.rend(),rxConsumer);

    if (riter != maConsList.rend())
        maConsList.erase(riter.base()-1);
}


void ImageProducer::SetImage( const OUString& rPath )
{
    maURL = rPath;
    moGraphic->Clear();
    mbConsInit = false;
    mxOwnedStream.reset();
    mpStm = nullptr;

    if ( ::svt::GraphicAccess::isSupportedURL( maURL ) )
    {
        mxOwnedStream = ::svt::GraphicAccess::getImageStream( ::comphelper::getProcessComponentContext(), maURL );
        mpStm = mxOwnedStream.get();
    }
    else if( !maURL.isEmpty() )
    {
        mxOwnedStream = ::utl::UcbStreamHelper::CreateStream( maURL, StreamMode::STD_READ );
        mpStm = mxOwnedStream.get();
    }
}


void ImageProducer::SetImage( SvStream& rStm )
{
    maURL.clear();
    moGraphic->Clear();
    mbConsInit = false;
    mxOwnedStream.reset();
    mpStm = &rStm;
}


void ImageProducer::setImage( css::uno::Reference< css::io::XInputStream > const & rInputStmRef )
{
    maURL.clear();
    moGraphic->Clear();
    mbConsInit = false;
    mxOwnedStream.reset();
    mpStm = nullptr;

    if( rInputStmRef.is() )
    {
        mxOwnedStream = utl::UcbStreamHelper::CreateStream( rInputStmRef );
        mpStm = mxOwnedStream.get();
    }
}


void ImageProducer::NewDataAvailable()
{
    if( ( GraphicType::NONE == moGraphic->GetType() ) || moGraphic->GetReaderContext() )
        startProduction();
}


void ImageProducer::startProduction()
{
    if( maConsList.empty() && !maDoneHdl.IsSet() )
        return;

    bool bNotifyEmptyGraphics = false;

    // valid stream or filled graphic? => update consumers
    if( mpStm || ( moGraphic->GetType() != GraphicType::NONE ) )
    {
        // if we already have a graphic, we don't have to import again;
        // graphic is cleared if a new Stream is set
        if( ( moGraphic->GetType() == GraphicType::NONE ) || moGraphic->GetReaderContext() )
        {
            if ( ImplImportGraphic( *moGraphic ) )
                maDoneHdl.Call( &*moGraphic );
        }

        if( moGraphic->GetType() != GraphicType::NONE )
            ImplUpdateData( *moGraphic );
        else
            bNotifyEmptyGraphics = true;
    }
    else
        bNotifyEmptyGraphics = true;

    if ( !bNotifyEmptyGraphics )
        return;

    // reset image
    // create temporary list to hold interfaces
    ConsumerList_t aTmp = maConsList;

    // iterate through interfaces
    for (auto const& elem : aTmp)
    {
        elem->init( 0, 0 );
        elem->complete( css::awt::ImageStatus::IMAGESTATUS_STATICIMAGEDONE, this );
    }

    maDoneHdl.Call( nullptr );
}


bool ImageProducer::ImplImportGraphic( Graphic& rGraphic )
{
    if (!mpStm)
        return false;

    if( ERRCODE_IO_PENDING == mpStm->GetError() )
        mpStm->ResetError();

    mpStm->Seek( 0 );

    bool bRet = GraphicConverter::Import( *mpStm, rGraphic ) == ERRCODE_NONE;

    if( ERRCODE_IO_PENDING == mpStm->GetError() )
        mpStm->ResetError();

    return bRet;
}


void ImageProducer::ImplUpdateData( const Graphic& rGraphic )
{
    ImplInitConsumer( rGraphic );

    if( mbConsInit && !maConsList.empty() )
    {
        // create temporary list to hold interfaces
        ConsumerList_t aTmp = maConsList;

        ImplUpdateConsumer( rGraphic );
        mbConsInit = false;

        // iterate through interfaces
        for (auto const& elem : aTmp)
            elem->complete( css::awt::ImageStatus::IMAGESTATUS_STATICIMAGEDONE, this );
    }
}


void ImageProducer::ImplInitConsumer( const Graphic& rGraphic )
{
    sal_uInt32 nRMask = 0;
    sal_uInt32 nGMask = 0;
    sal_uInt32 nBMask = 0;
    sal_uInt32 nAMask = 0;
    sal_uInt32 nWidth = 0;
    sal_uInt32 nHeight = 0;
    sal_uInt8 nBitCount = 0;
    css::uno::Sequence< sal_Int32 > aRGBPal;
    rGraphic.GetBitmapEx().GetColorModel(aRGBPal, nRMask, nGMask, nBMask, nAMask, mnTransIndex, nWidth, nHeight, nBitCount);

    // create temporary list to hold interfaces
    ConsumerList_t aTmp = maConsList;

    // iterate through interfaces
    for (auto const& elem : aTmp)
    {
        elem->init( nWidth, nHeight );
        elem->setColorModel( nBitCount,aRGBPal, nRMask, nGMask, nBMask, nAMask );
    }

    mbConsInit = true;
}


void ImageProducer::ImplUpdateConsumer( const Graphic& rGraphic )
{
    BitmapEx            aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap              aBmp( aBmpEx.GetBitmap() );
    BitmapScopedReadAccess pBmpAcc(aBmp);

    if( !pBmpAcc )
        return;

    AlphaMask              aMask( aBmpEx.GetAlphaMask() );
    BitmapScopedReadAccess pMskAcc;
    if (!aMask.IsEmpty())
        pMskAcc = aMask;
    const tools::Long          nWidth = pBmpAcc->Width();
    const tools::Long          nHeight = pBmpAcc->Height();
    const tools::Long          nStartX = 0;
    const tools::Long          nEndX = nWidth - 1;
    const tools::Long          nStartY = 0;
    const tools::Long          nEndY = nHeight - 1;
    const tools::Long          nPartWidth = nEndX - nStartX + 1;
    const tools::Long          nPartHeight = nEndY - nStartY + 1;

    if( !pMskAcc )
    {
        aMask = AlphaMask(aBmp.GetSizePixel());
        aMask.Erase( 0 );
        pMskAcc = aMask;
    }

    // create temporary list to hold interfaces
    ConsumerList_t aTmp = maConsList;

    if( pBmpAcc->HasPalette() )
    {
        const BitmapColor aWhite( pMskAcc->GetBestMatchingColor( COL_ALPHA_TRANSPARENT ) );

        if( mnTransIndex < 256 )
        {
            css::uno::Sequence<sal_Int8>   aData( nPartWidth * nPartHeight );
            sal_Int8*                                   pTmp = aData.getArray();

            for( tools::Long nY = nStartY; nY <= nEndY; nY++ )
            {
                Scanline pScanlineMask = pMskAcc->GetScanline( nY );
                Scanline pScanline = pBmpAcc->GetScanline( nY );
                for( tools::Long nX = nStartX; nX <= nEndX; nX++ )
                {
                    if( pMskAcc->GetPixelFromData( pScanlineMask, nX ) == aWhite )
                        *pTmp++ = sal::static_int_cast< sal_Int8 >(
                            mnTransIndex );
                    else
                        *pTmp++ = pBmpAcc->GetPixelFromData( pScanline, nX ).GetIndex();
                }
            }

            // iterate through interfaces
            for (auto const& elem : aTmp)
                elem->setPixelsByBytes( nStartX, nStartY, nPartWidth, nPartHeight, aData, 0UL, nPartWidth );
        }
        else
        {
            css::uno::Sequence<sal_Int32>  aData( nPartWidth * nPartHeight );
            sal_Int32*                                  pTmp = aData.getArray();

            for( tools::Long nY = nStartY; nY <= nEndY; nY++ )
            {
                Scanline pScanlineMask = pMskAcc->GetScanline( nY );
                Scanline pScanline = pBmpAcc->GetScanline( nY );
                for( tools::Long nX = nStartX; nX <= nEndX; nX++ )
                {
                    if( pMskAcc->GetPixelFromData( pScanlineMask, nX ) == aWhite )
                        *pTmp++ = mnTransIndex;
                    else
                        *pTmp++ = pBmpAcc->GetPixelFromData( pScanline, nX ).GetIndex();
                }
            }

            // iterate through interfaces
            for (auto const& elem : aTmp)
                elem->setPixelsByLongs( nStartX, nStartY, nPartWidth, nPartHeight, aData, 0UL, nPartWidth );
        }
    }
    else
    {
        css::uno::Sequence<sal_Int32>  aData( nPartWidth * nPartHeight );
        const BitmapColor                           aWhite( pMskAcc->GetBestMatchingColor( COL_WHITE ) );
        sal_Int32*                                  pTmp = aData.getArray();

        for( tools::Long nY = nStartY; nY <= nEndY; nY++ )
        {
            Scanline pScanlineMask = pMskAcc->GetScanline( nY );
            Scanline pScanline = pBmpAcc->GetScanline( nY );
            for( tools::Long nX = nStartX; nX <= nEndX; nX++, pTmp++ )
            {
                const BitmapColor aCol( pBmpAcc->GetPixelFromData( pScanline, nX ) );

                *pTmp = static_cast<sal_Int32>(aCol.GetRed()) << 24;
                *pTmp |= static_cast<sal_Int32>(aCol.GetGreen()) << 16;
                *pTmp |= static_cast<sal_Int32>(aCol.GetBlue()) << 8;

                if( pMskAcc->GetPixelFromData( pScanlineMask, nX ) != aWhite )
                    *pTmp |= 0x000000ffUL;
            }
        }

        // iterate through interfaces
        for (auto const& elem : aTmp)
            elem->setPixelsByLongs( nStartX, nStartY, nPartWidth, nPartHeight, aData, 0UL, nPartWidth );
    }
}


void ImageProducer::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    if ( aArguments.getLength() == 1 )
    {
        css::uno::Any aArg = aArguments.getConstArray()[0];
        OUString aURL;
        if ( aArg >>= aURL )
        {
            SetImage( aURL );
        }
    }
}

OUString ImageProducer::getImplementationName() {
    return u"com.sun.star.form.ImageProducer"_ustr;
}

sal_Bool ImageProducer::supportsService(OUString const & ServiceName) {
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> ImageProducer::getSupportedServiceNames() {
    return {u"com.sun.star.awt.ImageProducer"_ustr};
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_ImageProducer_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ImageProducer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
