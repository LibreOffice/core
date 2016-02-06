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
#include "services.hxx"

#include <vcl/bitmapaccess.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "svtools/imageresourceaccess.hxx"
#include <comphelper/processfactory.hxx>

// - ImgProdLockBytes -


class ImgProdLockBytes : public SvLockBytes
{
    css::uno::Reference< css::io::XInputStream >      xStmRef;
    css::uno::Sequence<sal_Int8>                      maSeq;

public:

    ImgProdLockBytes( SvStream* pStm, bool bOwner );
    explicit ImgProdLockBytes( css::uno::Reference< css::io::XInputStream > & rStreamRef );
    virtual             ~ImgProdLockBytes();

    virtual ErrCode     ReadAt( sal_uInt64 nPos, void* pBuffer, sal_Size nCount, sal_Size * pRead ) const override;
    virtual ErrCode     WriteAt( sal_uInt64 nPos, const void* pBuffer, sal_Size nCount, sal_Size * pWritten ) override;
    virtual ErrCode     Flush() const override;
    virtual ErrCode     SetSize( sal_uInt64 nSize ) override;
    virtual ErrCode     Stat( SvLockBytesStat*, SvLockBytesStatFlag ) const override;
};



ImgProdLockBytes::ImgProdLockBytes( SvStream* pStm, bool bOwner ) :
        SvLockBytes( pStm, bOwner )
{
}



ImgProdLockBytes::ImgProdLockBytes( css::uno::Reference< css::io::XInputStream > & rStmRef ) :
        xStmRef( rStmRef )
{
    if( xStmRef.is() )
    {
        const sal_uInt32    nBytesToRead = 65535;
        sal_uInt32          nRead;

        do
        {
            css::uno::Sequence< sal_Int8 > aReadSeq;

            nRead = xStmRef->readSomeBytes( aReadSeq, nBytesToRead );

            if( nRead )
            {
                const sal_uInt32 nOldLength = maSeq.getLength();
                maSeq.realloc( nOldLength + nRead );
                memcpy( maSeq.getArray() + nOldLength, aReadSeq.getConstArray(), aReadSeq.getLength() );
            }
        }
        while( nBytesToRead == nRead );
    }
}



ImgProdLockBytes::~ImgProdLockBytes()
{
}

ErrCode ImgProdLockBytes::ReadAt(sal_uInt64 const nPos,
        void* pBuffer, sal_Size nCount, sal_Size * pRead) const
{
    if( GetStream() )
    {
        const_cast<SvStream*>(GetStream())->ResetError();
        const ErrCode nErr = SvLockBytes::ReadAt( nPos, pBuffer, nCount, pRead );
        const_cast<SvStream*>(GetStream())->ResetError();
        return nErr;
    }
    else
    {
        const sal_Size nSeqLen = maSeq.getLength();
        ErrCode nErr = ERRCODE_NONE;

        if( nPos < nSeqLen )
        {
            if( ( nPos + nCount ) > nSeqLen )
                nCount = nSeqLen - nPos;

            memcpy( pBuffer, maSeq.getConstArray() + nPos, nCount );
            *pRead = nCount;
        }
        else
            *pRead = 0UL;

        return nErr;
    }
}

ErrCode ImgProdLockBytes::WriteAt(sal_uInt64 const nPos,
        const void* pBuffer, sal_Size nCount, sal_Size * pWritten)
{
    if( GetStream() )
        return SvLockBytes::WriteAt( nPos, pBuffer, nCount, pWritten );
    else
    {
        DBG_ASSERT( xStmRef.is(), "ImgProdLockBytes::WriteAt: xInputStream has no reference..." );
        return ERRCODE_IO_CANTWRITE;
    }
}



ErrCode ImgProdLockBytes::Flush() const
{
    return ERRCODE_NONE;
}



ErrCode ImgProdLockBytes::SetSize(sal_uInt64 const nSize)
{
    if( GetStream() )
        return SvLockBytes::SetSize( nSize );
    else
    {
        OSL_FAIL( "ImgProdLockBytes::SetSize not supported for xInputStream..." );
        return ERRCODE_IO_CANTWRITE;
    }
}



ErrCode ImgProdLockBytes::Stat( SvLockBytesStat* pStat, SvLockBytesStatFlag eFlag ) const
{
    if( GetStream() )
        return SvLockBytes::Stat( pStat, eFlag );
    else
    {
        DBG_ASSERT( xStmRef.is(), "ImgProdLockBytes::Stat: xInputStream has no reference..." );
        pStat->nSize = maSeq.getLength();
        return ERRCODE_NONE;
    }
}

// - ImageProducer -
ImageProducer::ImageProducer()
    : mpStm(nullptr)
    , mnTransIndex(0)
    , mbConsInit(false)
{
    mpGraphic = new Graphic;
}

ImageProducer::~ImageProducer()
{
    delete mpGraphic;
    mpGraphic = nullptr;

    delete mpStm;
    mpStm = nullptr;
}

// css::uno::XInterface
css::uno::Any ImageProducer::queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception)
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< css::lang::XInitialization* >(this)),
                                        (static_cast< css::awt::XImageProducer* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}



void ImageProducer::addConsumer( const css::uno::Reference< css::awt::XImageConsumer >& rxConsumer )
    throw(css::uno::RuntimeException,
          std::exception)
{
    DBG_ASSERT( rxConsumer.is(), "::AddConsumer(...): No consumer referenced!" );
    if( rxConsumer.is() )
        maConsList.push_back( rxConsumer );
}



void ImageProducer::removeConsumer( const css::uno::Reference< css::awt::XImageConsumer >& rxConsumer ) throw(css::uno::RuntimeException, std::exception)
{
    ConsumerList_t::reverse_iterator riter = std::find(maConsList.rbegin(),maConsList.rend(),rxConsumer);

    if (riter != maConsList.rend())
        maConsList.erase(riter.base()-1);
}



void ImageProducer::SetImage( const OUString& rPath )
{
    maURL = rPath;
    mpGraphic->Clear();
    mbConsInit = false;
    delete mpStm;

    if ( ::svt::GraphicAccess::isSupportedURL( maURL ) )
    {
        mpStm = ::svt::GraphicAccess::getImageStream( ::comphelper::getProcessComponentContext(), maURL );
    }
    else if( !maURL.isEmpty() )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( maURL, STREAM_STD_READ );
        mpStm = pIStm ? new SvStream( new ImgProdLockBytes( pIStm, true ) ) : nullptr;
    }
    else
        mpStm = nullptr;
}



void ImageProducer::SetImage( SvStream& rStm )
{
    maURL.clear();
    mpGraphic->Clear();
    mbConsInit = false;

    delete mpStm;
    mpStm = new SvStream( new ImgProdLockBytes( &rStm, false ) );
}



void ImageProducer::setImage( css::uno::Reference< css::io::XInputStream > & rInputStmRef )
{
    maURL.clear();
    mpGraphic->Clear();
    mbConsInit = false;
    delete mpStm;

    if( rInputStmRef.is() )
        mpStm = new SvStream( new ImgProdLockBytes( rInputStmRef ) );
    else
        mpStm = nullptr;
}



void ImageProducer::NewDataAvailable()
{
    if( ( GRAPHIC_NONE == mpGraphic->GetType() ) || mpGraphic->GetContext() )
        startProduction();
}



void ImageProducer::startProduction() throw(css::uno::RuntimeException, std::exception)
{
    if( !maConsList.empty() || maDoneHdl.IsSet() )
    {
        bool bNotifyEmptyGraphics = false;

        // valid stream or filled graphic? => update consumers
        if( mpStm || ( mpGraphic->GetType() != GRAPHIC_NONE ) )
        {
            // if we already have a graphic, we don't have to import again;
            // graphic is cleared if a new Stream is set
            if( ( mpGraphic->GetType() == GRAPHIC_NONE ) || mpGraphic->GetContext() )
            {
                if ( ImplImportGraphic( *mpGraphic ) )
                    maDoneHdl.Call( mpGraphic );
            }

            if( mpGraphic->GetType() != GRAPHIC_NONE )
                ImplUpdateData( *mpGraphic );
            else
                bNotifyEmptyGraphics = true;
        }
        else
            bNotifyEmptyGraphics = true;

        if ( bNotifyEmptyGraphics )
        {
            // reset image
            // create temporary list to hold interfaces
            ConsumerList_t aTmp = maConsList;

            // iterate through interfaces
            for( ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter )
            {
                (*iter)->init( 0, 0 );
                (*iter)->complete( css::awt::ImageStatus::IMAGESTATUS_STATICIMAGEDONE, this );
            }

            maDoneHdl.Call( nullptr );
        }
    }
}



bool ImageProducer::ImplImportGraphic( Graphic& rGraphic )
{
    if (!mpStm)
        return false;

    if( ERRCODE_IO_PENDING == mpStm->GetError() )
        mpStm->ResetError();

    mpStm->Seek( 0UL );

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
        for( ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter )
            (*iter)->complete( css::awt::ImageStatus::IMAGESTATUS_STATICIMAGEDONE, this );
    }
}



void ImageProducer::ImplInitConsumer( const Graphic& rGraphic )
{
    Bitmap              aBmp( rGraphic.GetBitmapEx().GetBitmap() );
    BitmapReadAccess*   pBmpAcc = aBmp.AcquireReadAccess();

    if( pBmpAcc )
    {
        sal_uInt16       nPalCount = 0;
        sal_uInt32       nRMask = 0;
        sal_uInt32       nGMask = 0;
        sal_uInt32       nBMask = 0;
        sal_uInt32       nAMask = 0;
        css::uno::Sequence< sal_Int32 >    aRGBPal;

        if( pBmpAcc->HasPalette() )
        {
            nPalCount = pBmpAcc->GetPaletteEntryCount();

            if( nPalCount )
            {
                aRGBPal = css::uno::Sequence< sal_Int32 >( nPalCount + 1 );

                sal_Int32* pTmp = aRGBPal.getArray();

                for( sal_uInt32 i = 0; i < nPalCount; i++, pTmp++ )
                {
                    const BitmapColor& rCol = pBmpAcc->GetPaletteColor( (sal_uInt16) i );

                    *pTmp = ( (sal_Int32) rCol.GetRed() ) << (sal_Int32)(24L);
                    *pTmp |= ( (sal_Int32) rCol.GetGreen() ) << (sal_Int32)(16L);
                    *pTmp |= ( (sal_Int32) rCol.GetBlue() ) << (sal_Int32)(8L);
                    *pTmp |= (sal_Int32)(0x000000ffL);
                }

                if( rGraphic.IsTransparent() )
                {
                    // append transparent entry
                    *pTmp = (sal_Int32)(0xffffff00L);
                    mnTransIndex = nPalCount;
                    nPalCount++;
                }
                else
                    mnTransIndex = 0;

            }
        }
        else
        {
            nRMask = 0xff000000UL;
            nGMask = 0x00ff0000UL;
            nBMask = 0x0000ff00UL;
            nAMask = 0x000000ffUL;
        }

        // create temporary list to hold interfaces
        ConsumerList_t aTmp = maConsList;

        // iterate through interfaces
        for( ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter)
        {
            (*iter)->init( pBmpAcc->Width(), pBmpAcc->Height() );
            (*iter)->setColorModel( pBmpAcc->GetBitCount(),aRGBPal, nRMask, nGMask, nBMask, nAMask );
        }

        Bitmap::ReleaseAccess( pBmpAcc );
        mbConsInit = true;
    }
}



void ImageProducer::ImplUpdateConsumer( const Graphic& rGraphic )
{
    BitmapEx            aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap              aBmp( aBmpEx.GetBitmap() );
    BitmapReadAccess*   pBmpAcc = aBmp.AcquireReadAccess();

    if( pBmpAcc )
    {
        Bitmap              aMask( aBmpEx.GetMask() );
        BitmapReadAccess*   pMskAcc = !!aMask ? aMask.AcquireReadAccess() : nullptr;
        const long          nWidth = pBmpAcc->Width();
        const long          nHeight = pBmpAcc->Height();
        const long          nStartX = 0L;
        const long          nEndX = nWidth - 1L;
        const long          nStartY = 0L;
        const long          nEndY = nHeight - 1L;
        const long          nPartWidth = nEndX - nStartX + 1;
        const long          nPartHeight = nEndY - nStartY + 1;

        if( !pMskAcc )
        {
            aMask = Bitmap( aBmp.GetSizePixel(), 1 );
            aMask.Erase( COL_BLACK );
            pMskAcc = aMask.AcquireReadAccess();
        }

        // create temporary list to hold interfaces
        ConsumerList_t aTmp = maConsList;

        if( pBmpAcc->HasPalette() )
        {
            const BitmapColor aWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

            if( mnTransIndex < 256 )
            {
                css::uno::Sequence<sal_Int8>   aData( nPartWidth * nPartHeight );
                sal_Int8*                                   pTmp = aData.getArray();

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        if( pMskAcc->GetPixel( nY, nX ) == aWhite )
                            *pTmp++ = sal::static_int_cast< sal_Int8 >(
                                mnTransIndex );
                        else
                            *pTmp++ = pBmpAcc->GetPixel( nY, nX ).GetIndex();
                    }
                }

                // iterate through interfaces
                for (ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter)
                    (*iter)->setPixelsByBytes( nStartX, nStartY, nPartWidth, nPartHeight, aData, 0UL, nPartWidth );
            }
            else
            {
                css::uno::Sequence<sal_Int32>  aData( nPartWidth * nPartHeight );
                sal_Int32*                                  pTmp = aData.getArray();

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        if( pMskAcc->GetPixel( nY, nX ) == aWhite )
                            *pTmp++ = mnTransIndex;
                        else
                            *pTmp++ = pBmpAcc->GetPixel( nY, nX ).GetIndex();
                    }
                }

                // iterate through interfaces
                for (ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter)
                    (*iter)->setPixelsByLongs( nStartX, nStartY, nPartWidth, nPartHeight, aData, 0UL, nPartWidth );
            }
        }
        else
        {
            css::uno::Sequence<sal_Int32>  aData( nPartWidth * nPartHeight );
            const BitmapColor                           aWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
            sal_Int32*                                  pTmp = aData.getArray();

            for( long nY = nStartY; nY <= nEndY; nY++ )
            {
                for( long nX = nStartX; nX <= nEndX; nX++, pTmp++ )
                {
                    const BitmapColor aCol( pBmpAcc->GetPixel( nY, nX ) );

                    *pTmp = ( (sal_Int32) aCol.GetRed() ) << (sal_Int32)(24L);
                    *pTmp |= ( (sal_Int32) aCol.GetGreen() ) << (sal_Int32)(16L);
                    *pTmp |= ( (sal_Int32) aCol.GetBlue() ) << (sal_Int32)(8L);

                    if( pMskAcc->GetPixel( nY, nX ) != aWhite )
                        *pTmp |= 0x000000ffUL;
                }
            }

            // iterate through interfaces
                for (ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter)
                    (*iter)->setPixelsByLongs( nStartX, nStartY, nPartWidth, nPartHeight, aData, 0UL, nPartWidth );
        }

        Bitmap::ReleaseAccess( pBmpAcc );
        Bitmap::ReleaseAccess( pMskAcc );
    }
}

void ImageProducer::initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_ImageProducer_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ImageProducer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
