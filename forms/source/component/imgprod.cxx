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


#include "imgprod.hxx"

#include <vcl/bmpacc.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include "svtools/imageresourceaccess.hxx"
#include <comphelper/processfactory.hxx>

// --------------------
// - ImgProdLockBytes -
// --------------------

class ImgProdLockBytes : public SvLockBytes
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >      xStmRef;
    ::com::sun::star::uno::Sequence<sal_Int8>       maSeq;

                        ImgProdLockBytes() {};

public:

                        ImgProdLockBytes( SvStream* pStm, sal_Bool bOwner );
                        ImgProdLockBytes( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > & rStreamRef );
    virtual             ~ImgProdLockBytes();

    virtual ErrCode     ReadAt( sal_Size nPos, void* pBuffer, sal_Size nCount, sal_Size* pRead ) const;
    virtual ErrCode     WriteAt( sal_Size nPos, const void* pBuffer, sal_Size nCount, sal_Size* pWritten );
    virtual ErrCode     Flush() const;
    virtual ErrCode     SetSize( sal_Size nSize );
    virtual ErrCode     Stat( SvLockBytesStat*, SvLockBytesStatFlag ) const;
};

// ------------------------------------------------------------------------

ImgProdLockBytes::ImgProdLockBytes( SvStream* pStm, sal_Bool bOwner ) :
        SvLockBytes( pStm, bOwner )
{
}

// ------------------------------------------------------------------------

ImgProdLockBytes::ImgProdLockBytes( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > & rStmRef ) :
        xStmRef( rStmRef )
{
    if( xStmRef.is() )
    {
        const sal_uInt32    nBytesToRead = 65535;
        sal_uInt32          nRead;

        do
        {
            ::com::sun::star::uno::Sequence< sal_Int8 > aReadSeq;

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

// ------------------------------------------------------------------------

ImgProdLockBytes::~ImgProdLockBytes()
{
}

// ------------------------------------------------------------------------

ErrCode ImgProdLockBytes::ReadAt( sal_Size nPos, void* pBuffer, sal_Size nCount, sal_Size* pRead ) const
{
    if( GetStream() )
    {
        ( (SvStream*) GetStream() )->ResetError();
        const ErrCode nErr = SvLockBytes::ReadAt( nPos, pBuffer, nCount, pRead );
        ( (SvStream*) GetStream() )->ResetError();
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

// ------------------------------------------------------------------------

ErrCode ImgProdLockBytes::WriteAt( sal_Size nPos, const void* pBuffer, sal_Size nCount, sal_Size* pWritten )
{
    if( GetStream() )
        return SvLockBytes::WriteAt( nPos, pBuffer, nCount, pWritten );
    else
    {
        DBG_ASSERT( xStmRef.is(), "ImgProdLockBytes::WriteAt: xInputStream has no reference..." );
        return ERRCODE_IO_CANTWRITE;
    }
}

// ------------------------------------------------------------------------

ErrCode ImgProdLockBytes::Flush() const
{
    return ERRCODE_NONE;
}

// ------------------------------------------------------------------------

ErrCode ImgProdLockBytes::SetSize( sal_Size nSize )
{
    if( GetStream() )
        return SvLockBytes::SetSize( nSize );
    else
    {
        OSL_FAIL( "ImgProdLockBytes::SetSize not supported for xInputStream..." );
        return ERRCODE_IO_CANTWRITE;
    }
}

// ------------------------------------------------------------------------

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

// -----------------
// - ImageProducer -
// -----------------

ImageProducer::ImageProducer() :
    mpStm       ( NULL ),
    mbConsInit  ( sal_False )
{
    mpGraphic = new Graphic;
    DBG_ASSERT( Application::GetFilterHdl().IsSet(), "ImageProducer::ImageProducer(): No filter handler set" );
}

// ------------------------------------------------------------

ImageProducer::~ImageProducer()
{
    delete mpGraphic;
    mpGraphic = NULL;

    delete mpStm;
    mpStm = NULL;
}

// ------------------------------------------------------------

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any ImageProducer::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::lang::XInitialization* >(this)),
                                        (static_cast< ::com::sun::star::awt::XImageProducer* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ------------------------------------------------------------

void ImageProducer::addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& rxConsumer ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_ASSERT( rxConsumer.is(), "::AddConsumer(...): No consumer referenced!" );
    if( rxConsumer.is() )
        maConsList.push_back( new ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer > ( rxConsumer ));
}

// ------------------------------------------------------------

void ImageProducer::removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& rxConsumer ) throw(::com::sun::star::uno::RuntimeException)
{
    ConsumerList_t::reverse_iterator riter = std::find(maConsList.rbegin(),maConsList.rend(),rxConsumer);

    if (riter != maConsList.rend())
        maConsList.erase(riter.base()-1);
}

// ------------------------------------------------------------

void ImageProducer::SetImage( const ::rtl::OUString& rPath )
{
    maURL = rPath;
    mpGraphic->Clear();
    mbConsInit = sal_False;
    delete mpStm;

    if ( ::svt::GraphicAccess::isSupportedURL( maURL ) )
    {
        mpStm = ::svt::GraphicAccess::getImageStream( ::comphelper::getProcessServiceFactory(), maURL );
    }
    else if( !maURL.isEmpty() )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( maURL, STREAM_STD_READ );
        mpStm = pIStm ? new SvStream( new ImgProdLockBytes( pIStm, sal_True ) ) : NULL;
    }
    else
        mpStm = NULL;
}

// ------------------------------------------------------------

void ImageProducer::SetImage( SvStream& rStm )
{
    maURL = ::rtl::OUString();
    mpGraphic->Clear();
    mbConsInit = sal_False;

    delete mpStm;
    mpStm = new SvStream( new ImgProdLockBytes( &rStm, sal_False ) );
}

// ------------------------------------------------------------

void ImageProducer::setImage( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > & rInputStmRef )
{
    maURL = ::rtl::OUString();
    mpGraphic->Clear();
    mbConsInit = sal_False;
    delete mpStm;

    if( rInputStmRef.is() )
        mpStm = new SvStream( new ImgProdLockBytes( rInputStmRef ) );
    else
        mpStm = NULL;
}

// ------------------------------------------------------------

void ImageProducer::NewDataAvailable()
{
    if( ( GRAPHIC_NONE == mpGraphic->GetType() ) || mpGraphic->GetContext() )
        startProduction();
}

// ------------------------------------------------------------

void ImageProducer::startProduction() throw(::com::sun::star::uno::RuntimeException)
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
                if ( ImplImportGraphic( *mpGraphic ) && maDoneHdl.IsSet() )
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
                (*iter)->complete( ::com::sun::star::awt::ImageStatus::IMAGESTATUS_STATICIMAGEDONE, this );
            }

            if ( maDoneHdl.IsSet() )
                maDoneHdl.Call( NULL );
        }
    }
}

// ------------------------------------------------------------

sal_Bool ImageProducer::ImplImportGraphic( Graphic& rGraphic )
{
    if( ERRCODE_IO_PENDING == mpStm->GetError() )
        mpStm->ResetError();

    mpStm->Seek( 0UL );

    sal_Bool bRet = GraphicConverter::Import( *mpStm, rGraphic ) == ERRCODE_NONE;

    if( ERRCODE_IO_PENDING == mpStm->GetError() )
        mpStm->ResetError();

    return bRet;
}

// ------------------------------------------------------------

void ImageProducer::ImplUpdateData( const Graphic& rGraphic )
{
    ImplInitConsumer( rGraphic );

    if( mbConsInit && !maConsList.empty() )
    {
        // create temporary list to hold interfaces
        ConsumerList_t aTmp = maConsList;

        ImplUpdateConsumer( rGraphic );
        mbConsInit = sal_False;

        // iterate through interfaces
        for( ConsumerList_t::iterator iter = aTmp.begin(); iter != aTmp.end(); ++iter )
            (*iter)->complete( ::com::sun::star::awt::ImageStatus::IMAGESTATUS_STATICIMAGEDONE, this );
    }
}

// ------------------------------------------------------------

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
        ::com::sun::star::uno::Sequence< sal_Int32 >    aRGBPal;

        if( pBmpAcc->HasPalette() )
        {
            nPalCount = pBmpAcc->GetPaletteEntryCount();

            if( nPalCount )
            {
                aRGBPal = ::com::sun::star::uno::Sequence< sal_Int32 >( nPalCount + 1 );

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

        aBmp.ReleaseAccess( pBmpAcc );
        mbConsInit = sal_True;
    }
}

// ------------------------------------------------------------

void ImageProducer::ImplUpdateConsumer( const Graphic& rGraphic )
{
    BitmapEx            aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap              aBmp( aBmpEx.GetBitmap() );
    BitmapReadAccess*   pBmpAcc = aBmp.AcquireReadAccess();

    if( pBmpAcc )
    {
        Bitmap              aMask( aBmpEx.GetMask() );
        BitmapReadAccess*   pMskAcc = !!aMask ? aMask.AcquireReadAccess() : NULL;
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
                ::com::sun::star::uno::Sequence<sal_Int8>   aData( nPartWidth * nPartHeight );
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
                ::com::sun::star::uno::Sequence<sal_Int32>  aData( nPartWidth * nPartHeight );
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
            ::com::sun::star::uno::Sequence<sal_Int32>  aData( nPartWidth * nPartHeight );
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

        aBmp.ReleaseAccess( pBmpAcc );
        aMask.ReleaseAccess( pMskAcc );
    }
}

void ImageProducer::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if ( aArguments.getLength() == 1 )
    {
        ::com::sun::star::uno::Any aArg = aArguments.getConstArray()[0];
        rtl::OUString aURL;
        if ( aArg >>= aURL )
        {
            SetImage( aURL );
        }
    }
}

namespace frm
{
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL ImageProducer_CreateInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
{
    return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >(
        ( ::cppu::OWeakObject* ) new ImageProducer );
}
} // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
