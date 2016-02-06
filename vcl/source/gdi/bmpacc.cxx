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
#include <vcl/bitmapaccess.hxx>

#include <impbmp.hxx>

#include <osl/diagnose.h>

#include <string.h>

BitmapInfoAccess::BitmapInfoAccess( Bitmap& rBitmap, BitmapAccessMode nMode ) :
            mpBuffer        ( nullptr ),
            mnAccessMode    ( nMode )
{
    ImplCreate( rBitmap );
}

BitmapInfoAccess::BitmapInfoAccess( Bitmap& rBitmap ) :
            mpBuffer        ( nullptr ),
            mnAccessMode    ( BITMAP_INFO_ACCESS )
{
    ImplCreate( rBitmap );
}

BitmapInfoAccess::~BitmapInfoAccess()
{
    ImplDestroy();
}

void BitmapInfoAccess::ImplCreate( Bitmap& rBitmap )
{
    ImpBitmap* pImpBmp = rBitmap.ImplGetImpBitmap();

    DBG_ASSERT( pImpBmp, "Forbidden Access to empty bitmap!" );

    if( pImpBmp )
    {
        if( mnAccessMode == BITMAP_WRITE_ACCESS && !maBitmap.ImplGetImpBitmap() )
        {
            rBitmap.ImplMakeUnique();
            pImpBmp = rBitmap.ImplGetImpBitmap();
        }
        else
        {
            DBG_ASSERT( mnAccessMode != BITMAP_WRITE_ACCESS ||
                        pImpBmp->ImplGetRefCount() == 2,
                        "Unpredictable results: bitmap is referenced more than once!" );
        }

        mpBuffer = pImpBmp->ImplAcquireBuffer( mnAccessMode );

        if( !mpBuffer )
        {
            ImpBitmap* pNewImpBmp = new ImpBitmap;

            if( pNewImpBmp->ImplCreate( *pImpBmp, rBitmap.GetBitCount()  ) )
            {
                pImpBmp = pNewImpBmp;
                rBitmap.ImplSetImpBitmap( pImpBmp );
                mpBuffer = pImpBmp->ImplAcquireBuffer( mnAccessMode );
            }
            else
                delete pNewImpBmp;
        }

        maBitmap = rBitmap;
    }
}

void BitmapInfoAccess::ImplDestroy()
{
    ImpBitmap* pImpBmp = maBitmap.ImplGetImpBitmap();

    if( mpBuffer && pImpBmp )
    {
        pImpBmp->ImplReleaseBuffer( mpBuffer, mnAccessMode );
        mpBuffer = nullptr;
    }
}

sal_uInt16 BitmapInfoAccess::GetBestPaletteIndex( const BitmapColor& rBitmapColor ) const
{
    return( HasPalette() ? mpBuffer->maPalette.GetBestIndex( rBitmapColor ) : 0 );
}

BitmapReadAccess::BitmapReadAccess( Bitmap& rBitmap, BitmapAccessMode nMode ) :
            BitmapInfoAccess( rBitmap, nMode ),
            mpScanBuf       ( nullptr ),
            mFncGetPixel    ( nullptr ),
            mFncSetPixel    ( nullptr )
{
    ImplInitScanBuffer( rBitmap );
}

BitmapReadAccess::BitmapReadAccess( Bitmap& rBitmap ) :
            BitmapInfoAccess( rBitmap, BITMAP_READ_ACCESS ),
            mpScanBuf       ( nullptr ),
            mFncGetPixel    ( nullptr ),
            mFncSetPixel    ( nullptr )
{
    ImplInitScanBuffer( rBitmap );
}

BitmapReadAccess::~BitmapReadAccess()
{
    ImplClearScanBuffer();
}

void BitmapReadAccess::ImplInitScanBuffer( Bitmap& rBitmap )
{
    if (!mpBuffer)
        return;

    ImpBitmap* pImpBmp = rBitmap.ImplGetImpBitmap();
    if (!pImpBmp)
        return;

    maColorMask = mpBuffer->maColorMask;

    bool bOk(true);
    const long nHeight = mpBuffer->mnHeight;
    Scanline pTmpLine = mpBuffer->mpBits;
    try
    {
        mpScanBuf = new Scanline[ nHeight ];
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
        bOk = ImplSetAccessPointers(BMP_SCANLINE_FORMAT(mpBuffer->mnFormat));
    }
    catch (const std::bad_alloc&)
    {
        bOk = false;
    }

    if (!bOk)
    {
        delete[] mpScanBuf;
        mpScanBuf = nullptr;

        pImpBmp->ImplReleaseBuffer( mpBuffer, mnAccessMode );
        mpBuffer = nullptr;
    }
}

void BitmapReadAccess::ImplClearScanBuffer()
{
    delete[] mpScanBuf;
    mpScanBuf = nullptr;
}

bool BitmapReadAccess::ImplSetAccessPointers( sal_uLong nFormat )
{
    bool bRet = true;

    switch( nFormat )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
        {
            mFncGetPixel = GetPixelFor_1BIT_MSB_PAL;
            mFncSetPixel = SetPixelFor_1BIT_MSB_PAL;
        }
        break;
        case BMP_FORMAT_1BIT_LSB_PAL:
        {
            mFncGetPixel = GetPixelFor_1BIT_LSB_PAL;
            mFncSetPixel = SetPixelFor_1BIT_LSB_PAL;
        }
        break;
        case BMP_FORMAT_4BIT_MSN_PAL:
        {
            mFncGetPixel = GetPixelFor_4BIT_MSN_PAL;
            mFncSetPixel = SetPixelFor_4BIT_MSN_PAL;
        }
        break;
        case BMP_FORMAT_4BIT_LSN_PAL:
        {
            mFncGetPixel = GetPixelFor_4BIT_LSN_PAL;
            mFncSetPixel = SetPixelFor_4BIT_LSN_PAL;
        }
        break;
        case BMP_FORMAT_8BIT_PAL:
        {
            mFncGetPixel = GetPixelFor_8BIT_PAL;
            mFncSetPixel = SetPixelFor_8BIT_PAL;
        }
        break;
        case BMP_FORMAT_8BIT_TC_MASK:
        {
            mFncGetPixel = GetPixelFor_8BIT_TC_MASK;
            mFncSetPixel = SetPixelFor_8BIT_TC_MASK;
        }
        break;
        case BMP_FORMAT_16BIT_TC_MSB_MASK:
        {
            mFncGetPixel = GetPixelFor_16BIT_TC_MSB_MASK;
            mFncSetPixel = SetPixelFor_16BIT_TC_MSB_MASK;
        }
        break;
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
        {
            mFncGetPixel = GetPixelFor_16BIT_TC_LSB_MASK;
            mFncSetPixel = SetPixelFor_16BIT_TC_LSB_MASK;
        }
        break;
        case BMP_FORMAT_24BIT_TC_BGR:
        {
            mFncGetPixel = GetPixelFor_24BIT_TC_BGR;
            mFncSetPixel = SetPixelFor_24BIT_TC_BGR;
        }
        break;
        case BMP_FORMAT_24BIT_TC_RGB:
        {
            mFncGetPixel = GetPixelFor_24BIT_TC_RGB;
            mFncSetPixel = SetPixelFor_24BIT_TC_RGB;
        }
        break;
        case BMP_FORMAT_24BIT_TC_MASK:
        {
            mFncGetPixel = GetPixelFor_24BIT_TC_MASK;
            mFncSetPixel = SetPixelFor_24BIT_TC_MASK;
        }
        break;
        case BMP_FORMAT_32BIT_TC_ABGR:
        {
            mFncGetPixel = GetPixelFor_32BIT_TC_ABGR;
            mFncSetPixel = SetPixelFor_32BIT_TC_ABGR;
        }
        break;
        case BMP_FORMAT_32BIT_TC_ARGB:
        {
            mFncGetPixel = GetPixelFor_32BIT_TC_ARGB;
            mFncSetPixel = SetPixelFor_32BIT_TC_ARGB;
        }
        break;
        case BMP_FORMAT_32BIT_TC_BGRA:
        {
            mFncGetPixel = GetPixelFor_32BIT_TC_BGRA;
            mFncSetPixel = SetPixelFor_32BIT_TC_BGRA;
        }
        break;
        case BMP_FORMAT_32BIT_TC_RGBA:
        {
            mFncGetPixel = GetPixelFor_32BIT_TC_RGBA;
            mFncSetPixel = SetPixelFor_32BIT_TC_RGBA;
        }
        break;
        case BMP_FORMAT_32BIT_TC_MASK:
        {
            mFncGetPixel = GetPixelFor_32BIT_TC_MASK;
            mFncSetPixel = SetPixelFor_32BIT_TC_MASK;
        }
        break;

        default:
            bRet = false;
        break;
    }

    return bRet;
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

BitmapWriteAccess::BitmapWriteAccess(Bitmap& rBitmap)
    : BitmapReadAccess(rBitmap, BITMAP_WRITE_ACCESS)
    , mpLineColor()
    , mpFillColor()
{
}

BitmapWriteAccess::~BitmapWriteAccess()
{
}

void BitmapWriteAccess::CopyScanline( long nY, const BitmapReadAccess& rReadAcc )
{
    assert(nY >= 0 && nY < mpBuffer->mnHeight && "y-coordinate in destination out of range!");
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

    assert(nY >= 0 && nY < mpBuffer->mnHeight && "y-coordinate in destination out of range!");
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
                    pFncGetPixel = nullptr;
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
