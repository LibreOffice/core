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
            mnAccessMode    ( BitmapAccessMode::Info )
{
    ImplCreate( rBitmap );
}

BitmapInfoAccess::~BitmapInfoAccess()
{
    ImplDestroy();
}

void BitmapInfoAccess::ImplCreate( Bitmap& rBitmap )
{
    std::shared_ptr<ImpBitmap> xImpBmp = rBitmap.ImplGetImpBitmap();

    SAL_WARN_IF( !xImpBmp, "vcl", "Forbidden Access to empty bitmap!" );

    if( xImpBmp )
    {
        if( mnAccessMode == BitmapAccessMode::Write && !maBitmap.ImplGetImpBitmap() )
        {
            rBitmap.ImplMakeUnique();
            xImpBmp = rBitmap.ImplGetImpBitmap();
        }
        else
        {
            DBG_ASSERT( mnAccessMode != BitmapAccessMode::Write ||
                        xImpBmp.use_count() == 2,
                        "Unpredictable results: bitmap is referenced more than once!" );
        }

        mpBuffer = xImpBmp->ImplAcquireBuffer( mnAccessMode );

        if( !mpBuffer )
        {
            std::shared_ptr<ImpBitmap> xNewImpBmp(new ImpBitmap);
            if (xNewImpBmp->ImplCreate(*xImpBmp, rBitmap.GetBitCount()))
            {
                xImpBmp = xNewImpBmp;
                rBitmap.ImplSetImpBitmap( xImpBmp );
                mpBuffer = xImpBmp->ImplAcquireBuffer( mnAccessMode );
            }
        }

        maBitmap = rBitmap;
    }
}

void BitmapInfoAccess::ImplDestroy()
{
    std::shared_ptr<ImpBitmap> xImpBmp = maBitmap.ImplGetImpBitmap();

    if (mpBuffer && xImpBmp)
    {
        xImpBmp->ImplReleaseBuffer( mpBuffer, mnAccessMode );
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
            BitmapInfoAccess( rBitmap, BitmapAccessMode::Read ),
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

    std::shared_ptr<ImpBitmap> xImpBmp = rBitmap.ImplGetImpBitmap();
    if (!xImpBmp)
        return;

    maColorMask = mpBuffer->maColorMask;

    bool bOk(true);
    const long nHeight = mpBuffer->mnHeight;
    Scanline pTmpLine = mpBuffer->mpBits;
    try
    {
        mpScanBuf = new Scanline[ nHeight ];
        if( mpBuffer->mnFormat & ScanlineFormat::TopDown )
        {
            for( long nY = 0; nY < nHeight; nY++, pTmpLine += mpBuffer->mnScanlineSize )
                mpScanBuf[ nY ] = pTmpLine;
        }
        else
        {
            for( long nY = nHeight - 1; nY >= 0; nY--, pTmpLine += mpBuffer->mnScanlineSize )
                mpScanBuf[ nY ] = pTmpLine;
        }
        bOk = ImplSetAccessPointers(RemoveScanline(mpBuffer->mnFormat));
    }
    catch (const std::bad_alloc&)
    {
        bOk = false;
    }

    if (!bOk)
    {
        delete[] mpScanBuf;
        mpScanBuf = nullptr;

        xImpBmp->ImplReleaseBuffer( mpBuffer, mnAccessMode );
        mpBuffer = nullptr;
    }
}

void BitmapReadAccess::ImplClearScanBuffer()
{
    delete[] mpScanBuf;
    mpScanBuf = nullptr;
}

bool BitmapReadAccess::ImplSetAccessPointers( ScanlineFormat nFormat )
{
    bool bRet = true;

    switch( nFormat )
    {
        case ScanlineFormat::N1BitMsbPal:
        {
            mFncGetPixel = GetPixelForN1BitMsbPal;
            mFncSetPixel = SetPixelForN1BitMsbPal;
        }
        break;
        case ScanlineFormat::N1BitLsbPal:
        {
            mFncGetPixel = GetPixelForN1BitLsbPal;
            mFncSetPixel = SetPixelForN1BitLsbPal;
        }
        break;
        case ScanlineFormat::N4BitMsnPal:
        {
            mFncGetPixel = GetPixelForN4BitMsnPal;
            mFncSetPixel = SetPixelForN4BitMsnPal;
        }
        break;
        case ScanlineFormat::N4BitLsnPal:
        {
            mFncGetPixel = GetPixelForN4BitLsnPal;
            mFncSetPixel = SetPixelForN4BitLsnPal;
        }
        break;
        case ScanlineFormat::N8BitPal:
        {
            mFncGetPixel = GetPixelForN8BitPal;
            mFncSetPixel = SetPixelForN8BitPal;
        }
        break;
        case ScanlineFormat::N8BitTcMask:
        {
            mFncGetPixel = GetPixelForN8BitTcMask;
            mFncSetPixel = SetPixelForN8BitTcMask;
        }
        break;
        case ScanlineFormat::N16BitTcMsbMask:
        {
            mFncGetPixel = GetPixelForN16BitTcMsbMask;
            mFncSetPixel = SetPixelForN16BitTcMsbMask;
        }
        break;
        case ScanlineFormat::N16BitTcLsbMask:
        {
            mFncGetPixel = GetPixelForN16BitTcLsbMask;
            mFncSetPixel = SetPixelForN16BitTcLsbMask;
        }
        break;
        case ScanlineFormat::N24BitTcBgr:
        {
            mFncGetPixel = GetPixelForN24BitTcBgr;
            mFncSetPixel = SetPixelForN24BitTcBgr;
        }
        break;
        case ScanlineFormat::N24BitTcRgb:
        {
            mFncGetPixel = GetPixelForN24BitTcRgb;
            mFncSetPixel = SetPixelForN24BitTcRgb;
        }
        break;
        case ScanlineFormat::N32BitTcAbgr:
        {
            mFncGetPixel = GetPixelForN32BitTcAbgr;
            mFncSetPixel = SetPixelForN32BitTcAbgr;
        }
        break;
        case ScanlineFormat::N32BitTcArgb:
        {
            mFncGetPixel = GetPixelForN32BitTcArgb;
            mFncSetPixel = SetPixelForN32BitTcArgb;
        }
        break;
        case ScanlineFormat::N32BitTcBgra:
        {
            mFncGetPixel = GetPixelForN32BitTcBgra;
            mFncSetPixel = SetPixelForN32BitTcBgra;
        }
        break;
        case ScanlineFormat::N32BitTcRgba:
        {
            mFncGetPixel = GetPixelForN32BitTcRgba;
            mFncSetPixel = SetPixelForN32BitTcRgba;
        }
        break;
        case ScanlineFormat::N32BitTcMask:
        {
            mFncGetPixel = GetPixelForN32BitTcMask;
            mFncSetPixel = SetPixelForN32BitTcMask;
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
    : BitmapReadAccess(rBitmap, BitmapAccessMode::Write)
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
    SAL_WARN_IF( nY >= rReadAcc.Height(), "vcl", "y-coordinate in source out of range!" );
    SAL_WARN_IF( ( !HasPalette() || !rReadAcc.HasPalette() ) && ( HasPalette() || rReadAcc.HasPalette() ), "vcl", "No copying possible between palette bitmap and TC bitmap!" );

    if( ( GetScanlineFormat() == rReadAcc.GetScanlineFormat() ) &&
        ( GetScanlineSize() >= rReadAcc.GetScanlineSize() ) )
    {
        memcpy( mpScanBuf[ nY ], rReadAcc.GetScanline( nY ), rReadAcc.GetScanlineSize() );
    }
    else
        // TODO: use fastbmp infrastructure
        for( long nX = 0, nWidth = std::min( mpBuffer->mnWidth, rReadAcc.Width() ); nX < nWidth; nX++ )
            SetPixel( nY, nX, rReadAcc.GetPixel( nY, nX ) );
}

void BitmapWriteAccess::CopyScanline( long nY, ConstScanline aSrcScanline,
                                      ScanlineFormat nSrcScanlineFormat, sal_uLong nSrcScanlineSize )
{
    const ScanlineFormat nFormat = RemoveScanline( nSrcScanlineFormat );

    assert(nY >= 0 && nY < mpBuffer->mnHeight && "y-coordinate in destination out of range!");
    DBG_ASSERT( ( HasPalette() && nFormat <= ScanlineFormat::N8BitPal ) ||
                ( !HasPalette() && nFormat > ScanlineFormat::N8BitPal ),
                "No copying possible between palette and non palette scanlines!" );

    const sal_uLong nCount = std::min( GetScanlineSize(), nSrcScanlineSize );

    if( nCount )
    {
        if( GetScanlineFormat() == RemoveScanline( nSrcScanlineFormat ) )
            memcpy( mpScanBuf[ nY ], aSrcScanline, nCount );
        else
        {
            DBG_ASSERT( nFormat != ScanlineFormat::N8BitTcMask &&
                        nFormat != ScanlineFormat::N16BitTcMsbMask && nFormat != ScanlineFormat::N16BitTcLsbMask &&
                        nFormat != ScanlineFormat::N32BitTcMask,
                        "No support for pixel formats with color masks yet!" );

            // TODO: use fastbmp infrastructure
            FncGetPixel pFncGetPixel;

            switch( nFormat )
            {
                case ScanlineFormat::N1BitMsbPal:    pFncGetPixel = GetPixelForN1BitMsbPal; break;
                case ScanlineFormat::N1BitLsbPal:    pFncGetPixel = GetPixelForN1BitLsbPal; break;
                case ScanlineFormat::N4BitMsnPal:    pFncGetPixel = GetPixelForN4BitMsnPal; break;
                case ScanlineFormat::N4BitLsnPal:    pFncGetPixel = GetPixelForN4BitLsnPal; break;
                case ScanlineFormat::N8BitPal:        pFncGetPixel = GetPixelForN8BitPal; break;
                case ScanlineFormat::N8BitTcMask:    pFncGetPixel = GetPixelForN8BitTcMask; break;
                case ScanlineFormat::N16BitTcMsbMask:   pFncGetPixel = GetPixelForN16BitTcMsbMask; break;
                case ScanlineFormat::N16BitTcLsbMask:   pFncGetPixel = GetPixelForN16BitTcLsbMask; break;
                case ScanlineFormat::N24BitTcBgr:    pFncGetPixel = GetPixelForN24BitTcBgr; break;
                case ScanlineFormat::N24BitTcRgb:    pFncGetPixel = GetPixelForN24BitTcRgb; break;
                case ScanlineFormat::N32BitTcAbgr:   pFncGetPixel = GetPixelForN32BitTcAbgr; break;
                case ScanlineFormat::N32BitTcArgb:   pFncGetPixel = GetPixelForN32BitTcArgb; break;
                case ScanlineFormat::N32BitTcBgra:   pFncGetPixel = GetPixelForN32BitTcBgra; break;
                case ScanlineFormat::N32BitTcRgba:   pFncGetPixel = GetPixelForN32BitTcRgba; break;
                case ScanlineFormat::N32BitTcMask:   pFncGetPixel = GetPixelForN32BitTcMask; break;

                default:
                    pFncGetPixel = nullptr;
                break;
            }

            if( pFncGetPixel )
            {
                const ColorMask aDummyMask;

                for( long nX = 0, nWidth = mpBuffer->mnWidth; nX < nWidth; nX++ )
                    SetPixel( nY, nX, pFncGetPixel( aSrcScanline, nX, aDummyMask ) );
            }
        }
    }
}

void BitmapWriteAccess::CopyBuffer( const BitmapReadAccess& rReadAcc )
{
    SAL_WARN_IF( ( !HasPalette() || !rReadAcc.HasPalette() ) && ( HasPalette() || rReadAcc.HasPalette() ), "vcl", "No copying possible between palette bitmap and TC bitmap!" );

    if( ( GetScanlineFormat() == rReadAcc.GetScanlineFormat() ) &&
        ( GetScanlineSize() == rReadAcc.GetScanlineSize() ) )
    {
        const long  nHeight = std::min( mpBuffer->mnHeight, rReadAcc.Height() );
        const sal_uLong nCount = nHeight * mpBuffer->mnScanlineSize;

        memcpy( mpBuffer->mpBits, rReadAcc.GetBuffer(), nCount );
    }
    else
        for( long nY = 0, nHeight = std::min( mpBuffer->mnHeight, rReadAcc.Height() ); nY < nHeight; nY++ )
            CopyScanline( nY, rReadAcc );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
