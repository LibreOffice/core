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


#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bmpacc.hxx>

#include <impbmp.hxx>

#include <string.h>

BitmapReadAccess::BitmapReadAccess( Bitmap& rBitmap, sal_Bool bModify ) :
            mpBuffer        ( NULL ),
            mpScanBuf       ( NULL ),
            mFncGetPixel    ( NULL ),
            mFncSetPixel    ( NULL ),
            mbModify        ( bModify )
{
    ImplCreate( rBitmap );
}

BitmapReadAccess::BitmapReadAccess( Bitmap& rBitmap ) :
            mpBuffer        ( NULL ),
            mpScanBuf       ( NULL ),
            mFncGetPixel    ( NULL ),
            mFncSetPixel    ( NULL ),
            mbModify        ( sal_False )
{
    ImplCreate( rBitmap );
}

BitmapReadAccess::~BitmapReadAccess()
{
    ImplDestroy();
}

void BitmapReadAccess::ImplCreate( Bitmap& rBitmap )
{
    ImpBitmap* pImpBmp = rBitmap.ImplGetImpBitmap();

    DBG_ASSERT( pImpBmp, "Forbidden Access to empty bitmap!" );

    if( pImpBmp )
    {
        if( mbModify && !maBitmap.ImplGetImpBitmap() )
        {
            rBitmap.ImplMakeUnique();
            pImpBmp = rBitmap.ImplGetImpBitmap();
        }
        else
        {
            DBG_ASSERT( !mbModify || pImpBmp->ImplGetRefCount() == 2,
                        "Unpredictable results: bitmap is referenced more than once!" );
        }

        mpBuffer = pImpBmp->ImplAcquireBuffer( !mbModify );

        if( !mpBuffer )
        {
            ImpBitmap* pNewImpBmp = new ImpBitmap;

            if( pNewImpBmp->ImplCreate( *pImpBmp, rBitmap.GetBitCount()  ) )
            {
                pImpBmp = pNewImpBmp;
                rBitmap.ImplSetImpBitmap( pImpBmp );
                mpBuffer = pImpBmp->ImplAcquireBuffer( !mbModify );
            }
            else
                delete pNewImpBmp;
        }

        if( mpBuffer )
        {
            const long  nHeight = mpBuffer->mnHeight;
            Scanline    pTmpLine = mpBuffer->mpBits;

            mpScanBuf = new Scanline[ nHeight ];
            maColorMask = mpBuffer->maColorMask;

            if( BMP_SCANLINE_ADJUSTMENT( mpBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
            {
                for( long nY = 0L; nY < nHeight; nY++, pTmpLine += mpBuffer->mnScanlineSize )
                    mpScanBuf[ nY ] = pTmpLine;
            }
            else
            {
                for( long nY = nHeight - 1; nY >= 0; nY--, pTmpLine += mpBuffer->mnScanlineSize )
                    mpScanBuf[ nY ] = pTmpLine;
            }

            if( !ImplSetAccessPointers( BMP_SCANLINE_FORMAT( mpBuffer->mnFormat ) ) )
            {
                delete[] mpScanBuf;
                mpScanBuf = NULL;

                pImpBmp->ImplReleaseBuffer( mpBuffer, !mbModify );
                mpBuffer = NULL;
            }
            else
                maBitmap = rBitmap;
        }
    }
}

void BitmapReadAccess::ImplDestroy()
{
    ImpBitmap* pImpBmp = maBitmap.ImplGetImpBitmap();

    delete[] mpScanBuf;
    mpScanBuf = NULL;

    if( mpBuffer && pImpBmp )
    {
        pImpBmp->ImplReleaseBuffer( mpBuffer, !mbModify );
        mpBuffer = NULL;
    }
}

sal_Bool BitmapReadAccess::ImplSetAccessPointers( sal_uLong nFormat )
{
    sal_Bool bRet = sal_True;

    switch( nFormat )
    {
        CASE_FORMAT( _1BIT_MSB_PAL )
        CASE_FORMAT( _1BIT_LSB_PAL )
        CASE_FORMAT( _4BIT_MSN_PAL )
        CASE_FORMAT( _4BIT_LSN_PAL )
        CASE_FORMAT( _8BIT_PAL )
        CASE_FORMAT( _8BIT_TC_MASK )
        CASE_FORMAT( _16BIT_TC_MSB_MASK )
        CASE_FORMAT( _16BIT_TC_LSB_MASK )
        CASE_FORMAT( _24BIT_TC_BGR )
        CASE_FORMAT( _24BIT_TC_RGB )
        CASE_FORMAT( _24BIT_TC_MASK )
        CASE_FORMAT( _32BIT_TC_ABGR )
        CASE_FORMAT( _32BIT_TC_ARGB )
        CASE_FORMAT( _32BIT_TC_BGRA )
        CASE_FORMAT( _32BIT_TC_RGBA )
        CASE_FORMAT( _32BIT_TC_MASK )

        default:
            bRet = sal_False;
        break;
    }

    return bRet;
}

void BitmapReadAccess::ImplZeroInitUnusedBits()
{
    const sal_uInt32 nWidth = Width(), nHeight = Height(), nScanSize = GetScanlineSize();

    if( nWidth && nHeight && nScanSize && GetBuffer() )
    {
        sal_uInt32 nBits;
        bool       bMsb;

        const sal_uLong nScanlineFormat = GetScanlineFormat();
        switch( nScanlineFormat )
        {
            case( BMP_FORMAT_1BIT_MSB_PAL ):
                nBits = 1;
                bMsb = true;
                break;

            case( BMP_FORMAT_1BIT_LSB_PAL ):
                nBits = 1;
                bMsb = false;
                break;

            case( BMP_FORMAT_4BIT_MSN_PAL ):
                nBits = 4;
                bMsb = true;
                break;

            case( BMP_FORMAT_4BIT_LSN_PAL ):
                nBits = 4;
                bMsb = false;
                break;

            case( BMP_FORMAT_8BIT_PAL ):
            case( BMP_FORMAT_8BIT_TC_MASK ):
                bMsb = true;
                nBits = 8;
            break;

            case( BMP_FORMAT_16BIT_TC_MSB_MASK ):
            case( BMP_FORMAT_16BIT_TC_LSB_MASK ):
                bMsb = true;
                nBits = 16;
            break;

            case( BMP_FORMAT_24BIT_TC_BGR ):
            case( BMP_FORMAT_24BIT_TC_RGB ):
            case( BMP_FORMAT_24BIT_TC_MASK ):
                bMsb = true;
                nBits = 24;
            break;

            case( BMP_FORMAT_32BIT_TC_ABGR ):
            case( BMP_FORMAT_32BIT_TC_ARGB ):
            case( BMP_FORMAT_32BIT_TC_BGRA ):
            case( BMP_FORMAT_32BIT_TC_RGBA ):
            case( BMP_FORMAT_32BIT_TC_MASK ):
                bMsb = true;
                nBits = 32;
            break;

            default:
            {
                OSL_FAIL( "BitmapWriteAccess::ZeroInitUnusedBits: Unsupported pixel format");
                nBits = 0;
                bMsb = true;
            }
            break;
        }

        nBits *= nWidth;
        if( nScanSize % 4 || !bMsb )
        {
            DBG_ASSERT( 8*nScanSize >= nBits,
                        "BitmapWriteAccess::ZeroInitUnusedBits: span size smaller than width?!");
            const sal_uInt32 nLeftOverBits = 8*sizeof(sal_uInt8)*nScanSize - nBits;
            if( nLeftOverBits != 0 ) // else there is really nothing to do
            {
                const sal_uInt32 nBytes = (nLeftOverBits + 7U) >> 3U;
                sal_uInt8        nMask;

                if( bMsb )
                    nMask = static_cast<sal_uInt8>(0xffU << (nLeftOverBits & 3UL));
                else
                    nMask = static_cast<sal_uInt8>(0xffU >> (nLeftOverBits & 3UL));

                sal_uInt8* pLastBytes = (sal_uInt8*)GetBuffer() + ( nScanSize - nBytes );
                for( sal_uInt32 i = 0; i < nHeight; i++, pLastBytes += nScanSize )
                {
                    *pLastBytes &= nMask;
                    for( sal_uInt32 j = 1; j < nBytes; j++ )
                        pLastBytes[j] = 0;
                }
            }
        }
        else if( nBits & 0x1f )
        {
            sal_uInt32  nMask = 0xffffffff << ( ( nScanSize << 3 ) - nBits );
            sal_uInt8*      pLast4Bytes = (sal_uInt8*) GetBuffer() + ( nScanSize - 4 );

#ifdef OSL_LITENDIAN
            nMask = OSL_SWAPDWORD( nMask );
#endif
            for( sal_uInt32 i = 0; i < nHeight; i++, pLast4Bytes += nScanSize )
                ( *(sal_uInt32*) pLast4Bytes ) &= nMask;
        }
    }
}

sal_uInt16 BitmapReadAccess::GetBestPaletteIndex( const BitmapColor& rBitmapColor ) const
{
    return( HasPalette() ? mpBuffer->maPalette.GetBestIndex( rBitmapColor ) : 0 );
}

BitmapColor BitmapReadAccess::GetInterpolatedColorWithFallback( double fY, double fX, const BitmapColor& rFallback ) const
{
    // ask directly doubles >= 0.0 here to avoid rounded values of 0 at small negative
    // double values, e.g. static_cast< sal_Int32 >(-0.25) is 0, not -1, but *has* to be outside (!)
    if(mpBuffer && fX >= 0.0 && fY >= 0.0)
    {
        const sal_Int32 nX(static_cast< sal_Int32 >(fX));
        const sal_Int32 nY(static_cast< sal_Int32 >(fY));

        if(nX < mpBuffer->mnWidth && nY < mpBuffer->mnHeight)
        {
            // get base-return value from inside pixel
            BitmapColor aRetval(GetColor(nY, nX));

            // calculate deltas and indices for neighbour accesses
            sal_Int16 nDeltaX((fX - (nX + 0.5)) * 255.0); // [-255 .. 255]
            sal_Int16 nDeltaY((fY - (nY + 0.5)) * 255.0); // [-255 .. 255]
            sal_Int16 nIndX(0);
            sal_Int16 nIndY(0);

            if(nDeltaX > 0)
            {
                nIndX = nX + 1;
            }
            else
            {
                nIndX = nX - 1;
                nDeltaX = -nDeltaX;
            }

            if(nDeltaY > 0)
            {
                nIndY = nY + 1;
            }
            else
            {
                nIndY = nY - 1;
                nDeltaY = -nDeltaY;
            }

            // get right/left neighbour
            BitmapColor aXCol(rFallback);

            if(nDeltaX && nIndX >= 0 && nIndX < mpBuffer->mnWidth)
            {
                aXCol = GetColor(nY, nIndX);
            }

            // get top/bottom neighbour
            BitmapColor aYCol(rFallback);

            if(nDeltaY && nIndY >= 0 && nIndY < mpBuffer->mnHeight)
            {
                aYCol = GetColor(nIndY, nX);
            }

            // get one of four edge neighbours
            BitmapColor aXYCol(rFallback);

            if(nDeltaX && nDeltaY && nIndX >=0 && nIndY >= 0 && nIndX < mpBuffer->mnWidth && nIndY < mpBuffer->mnHeight)
            {
                aXYCol = GetColor(nIndY, nIndX);
            }

            // merge return value with right/left neighbour
            if(aXCol != aRetval)
            {
                aRetval.Merge(aXCol, 255 - nDeltaX);
            }

            // merge top/bottom neighbour with edge
            if(aYCol != aXYCol)
            {
                aYCol.Merge(aXYCol, 255 - nDeltaX);
            }

            // merge return value with already merged top/bottom neighbour
            if(aRetval != aYCol)
            {
                aRetval.Merge(aYCol, 255 - nDeltaY);
            }

            return aRetval;
        }
    }

    return rFallback;
}

BitmapColor BitmapReadAccess::GetColorWithFallback( double fY, double fX, const BitmapColor& rFallback ) const
{
    // ask directly doubles >= 0.0 here to avoid rounded values of 0 at small negative
    // double values, e.g. static_cast< sal_Int32 >(-0.25) is 0, not -1, but *has* to be outside (!)
    if(mpBuffer && fX >= 0.0 && fY >= 0.0)
    {
        const sal_Int32 nX(static_cast< sal_Int32 >(fX));
        const sal_Int32 nY(static_cast< sal_Int32 >(fY));

        if(nX < mpBuffer->mnWidth && nY < mpBuffer->mnHeight)
        {
            return GetColor(nY, nX);
        }
    }

    return rFallback;
}

BitmapColor BitmapReadAccess::GetColorWithFallback( long nY, long nX, const BitmapColor& rFallback ) const
{
    if(mpBuffer)
    {
        if(nX >= 0 && nY >= 0 && nX < mpBuffer->mnWidth && nY < mpBuffer->mnHeight)
        {
            return GetColor(nY, nX);
        }
    }

    return rFallback;
}

BitmapWriteAccess::BitmapWriteAccess( Bitmap& rBitmap ) :
            BitmapReadAccess( rBitmap, sal_True ),
            mpLineColor     ( NULL ),
            mpFillColor     ( NULL )
{
}

BitmapWriteAccess::~BitmapWriteAccess()
{
    delete mpLineColor;
    delete mpFillColor;
}

void BitmapWriteAccess::CopyScanline( long nY, const BitmapReadAccess& rReadAcc )
{
    DBG_ASSERT( ( nY >= 0 ) && ( nY < mpBuffer->mnHeight ), "y-coordinate in destination out of range!" );
    DBG_ASSERT( nY < rReadAcc.Height(), "y-coordinate in source out of range!" );
    DBG_ASSERT( ( HasPalette() && rReadAcc.HasPalette() ) || ( !HasPalette() && !rReadAcc.HasPalette() ), "No copying possible between palette bitmap and TC bitmap!" );

    if( ( GetScanlineFormat() == rReadAcc.GetScanlineFormat() ) &&
        ( GetScanlineSize() >= rReadAcc.GetScanlineSize() ) )
    {
        memcpy( mpScanBuf[ nY ], rReadAcc.GetScanline( nY ), rReadAcc.GetScanlineSize() );
    }
    else
        // TODO: use fastbmp infrastructure
        for( long nX = 0L, nWidth = std::min( mpBuffer->mnWidth, rReadAcc.Width() ); nX < nWidth; nX++ )
            SetPixel( nY, nX, rReadAcc.GetPixel( nY, nX ) );
}


void BitmapWriteAccess::CopyScanline( long nY, ConstScanline aSrcScanline,
                                      sal_uLong nSrcScanlineFormat, sal_uLong nSrcScanlineSize )
{
    const sal_uLong nFormat = BMP_SCANLINE_FORMAT( nSrcScanlineFormat );

    DBG_ASSERT( ( nY >= 0 ) && ( nY < mpBuffer->mnHeight ), "y-coordinate in destination out of range!" );
    DBG_ASSERT( ( HasPalette() && nFormat <= BMP_FORMAT_8BIT_PAL ) ||
                ( !HasPalette() && nFormat > BMP_FORMAT_8BIT_PAL ),
                "No copying possible between palette and non palette scanlines!" );

    const sal_uLong nCount = std::min( GetScanlineSize(), nSrcScanlineSize );

    if( nCount )
    {
        if( GetScanlineFormat() == BMP_SCANLINE_FORMAT( nSrcScanlineFormat ) )
            memcpy( mpScanBuf[ nY ], aSrcScanline, nCount );
        else
        {
            DBG_ASSERT( nFormat != BMP_FORMAT_8BIT_TC_MASK &&
                        nFormat != BMP_FORMAT_16BIT_TC_MSB_MASK && nFormat != BMP_FORMAT_16BIT_TC_LSB_MASK &&
                        nFormat != BMP_FORMAT_24BIT_TC_MASK && nFormat != BMP_FORMAT_32BIT_TC_MASK,
                        "No support for pixel formats with color masks yet!" );

            // TODO: use fastbmp infrastructure
            FncGetPixel pFncGetPixel;

            switch( nFormat )
            {
                case( BMP_FORMAT_1BIT_MSB_PAL ):    pFncGetPixel = GetPixelFor_1BIT_MSB_PAL; break;
                case( BMP_FORMAT_1BIT_LSB_PAL ):    pFncGetPixel = GetPixelFor_1BIT_LSB_PAL; break;
                case( BMP_FORMAT_4BIT_MSN_PAL ):    pFncGetPixel = GetPixelFor_4BIT_MSN_PAL; break;
                case( BMP_FORMAT_4BIT_LSN_PAL ):    pFncGetPixel = GetPixelFor_4BIT_LSN_PAL; break;
                case( BMP_FORMAT_8BIT_PAL ):        pFncGetPixel = GetPixelFor_8BIT_PAL; break;
                case( BMP_FORMAT_8BIT_TC_MASK ):    pFncGetPixel = GetPixelFor_8BIT_TC_MASK; break;
                case( BMP_FORMAT_16BIT_TC_MSB_MASK ):   pFncGetPixel = GetPixelFor_16BIT_TC_MSB_MASK; break;
                case( BMP_FORMAT_16BIT_TC_LSB_MASK ):   pFncGetPixel = GetPixelFor_16BIT_TC_LSB_MASK; break;
                case( BMP_FORMAT_24BIT_TC_BGR ):    pFncGetPixel = GetPixelFor_24BIT_TC_BGR; break;
                case( BMP_FORMAT_24BIT_TC_RGB ):    pFncGetPixel = GetPixelFor_24BIT_TC_RGB; break;
                case( BMP_FORMAT_24BIT_TC_MASK ):   pFncGetPixel = GetPixelFor_24BIT_TC_MASK; break;
                case( BMP_FORMAT_32BIT_TC_ABGR ):   pFncGetPixel = GetPixelFor_32BIT_TC_ABGR; break;
                case( BMP_FORMAT_32BIT_TC_ARGB ):   pFncGetPixel = GetPixelFor_32BIT_TC_ARGB; break;
                case( BMP_FORMAT_32BIT_TC_BGRA ):   pFncGetPixel = GetPixelFor_32BIT_TC_BGRA; break;
                case( BMP_FORMAT_32BIT_TC_RGBA ):   pFncGetPixel = GetPixelFor_32BIT_TC_RGBA; break;
                case( BMP_FORMAT_32BIT_TC_MASK ):   pFncGetPixel = GetPixelFor_32BIT_TC_MASK; break;

                default:
                    pFncGetPixel = NULL;
                break;
            }

            if( pFncGetPixel )
            {
                const ColorMask aDummyMask;

                for( long nX = 0L, nWidth = mpBuffer->mnWidth; nX < nWidth; nX++ )
                    SetPixel( nY, nX, pFncGetPixel( aSrcScanline, nX, aDummyMask ) );
            }
        }
    }
}

void BitmapWriteAccess::CopyBuffer( const BitmapReadAccess& rReadAcc )
{
    DBG_ASSERT( ( HasPalette() && rReadAcc.HasPalette() ) || ( !HasPalette() && !rReadAcc.HasPalette() ), "No copying possible between palette bitmap and TC bitmap!" );

    if( ( GetScanlineFormat() == rReadAcc.GetScanlineFormat() ) &&
        ( GetScanlineSize() == rReadAcc.GetScanlineSize() ) )
    {
        const long  nHeight = std::min( mpBuffer->mnHeight, rReadAcc.Height() );
        const sal_uLong nCount = nHeight * mpBuffer->mnScanlineSize;

        memcpy( mpBuffer->mpBits, rReadAcc.GetBuffer(), nCount );
    }
    else
        for( long nY = 0L, nHeight = std::min( mpBuffer->mnHeight, rReadAcc.Height() ); nY < nHeight; nY++ )
            CopyScanline( nY, rReadAcc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
