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

#include <vcl/bitmapaccess.hxx>
#include <vcl/salbtype.hxx>
#include <bmpfast.hxx>
#include <osl/diagnose.h>
#include <memory>

#define IMPL_CASE_GET_FORMAT( Format )                          \
case( ScanlineFormat::Format ):                                 \
    pFncGetPixel = BitmapReadAccess::GetPixelFor##Format;       \
break

#define IMPL_CASE_SET_FORMAT( Format, BitCount )                \
case( ScanlineFormat::Format ):                                 \
{                                                               \
    pFncSetPixel = BitmapReadAccess::SetPixelFor##Format;       \
    pDstBuffer->mnBitCount = BitCount;                          \
}                                                               \
break

#define DOUBLE_SCANLINES()                                                      \
while( ( nActY < nHeight1 ) && ( pMapY[ nActY + 1 ] == nMapY ) )                \
{                                                                               \
    memcpy( pDstScanMap[ nActY + 1 ], pDstScan, rDstBuffer.mnScanlineSize );   \
    nActY++;                                                                    \
}

#define TC_TO_PAL_COLORS    4096

static long ImplIndexFromColor( const BitmapColor& rCol )
{
#if TC_TO_PAL_COLORS == 4096

    return( ( ( (long) rCol.GetBlue() >> 4) << 8 ) |
            ( ( (long) rCol.GetGreen() >> 4 ) << 4 ) |
            ( (long) rCol.GetRed() >> 4 ) );

#elif TC_TO_PAL_COLORS == 32768

    return( ( ( (long) rCol.GetBlue() >> 3) << 10 ) |
            ( ( (long) rCol.GetGreen() >> 3 ) << 5 ) |
            ( (long) rCol.GetRed() >> 3 ) );

#endif
}

static void ImplPALToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                          FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                          Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nHeight1 = rDstBuffer.mnHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    BitmapPalette       aColMap( rSrcBuffer.maPalette.GetEntryCount() );
    BitmapColor*        pColMapBuf = aColMap.ImplGetColorBuffer();
    BitmapColor         aIndex( 0 );

    for( sal_uInt16 i = 0, nSrcCount = aColMap.GetEntryCount(), nDstCount = rDstBuffer.maPalette.GetEntryCount(); i < nSrcCount; i++ )
    {
        if( ( i < nDstCount ) && ( rSrcBuffer.maPalette[ i ] == rDstBuffer.maPalette[ i ] ) )
            aIndex.SetIndex( sal::static_int_cast<sal_uInt8>(i) );
        else
            aIndex.SetIndex( sal::static_int_cast<sal_uInt8>(rDstBuffer.maPalette.GetBestIndex( rSrcBuffer.maPalette[ i ] )) );

        pColMapBuf[ i ] = aIndex;
    }

    for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
    {
        long nMapY = pMapY[nActY];
        Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

        for (long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
            pFncSetPixel( pDstScan, nX, pColMapBuf[ pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ).GetIndex() ], rDstMask );

        DOUBLE_SCANLINES();
    }
}

static void ImplPALToTC( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nHeight1 = rDstBuffer.mnHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    const BitmapColor*  pColBuf = rSrcBuffer.maPalette.ImplGetColorBuffer();

    if( RemoveScanline( rSrcBuffer.mnFormat ) == ScanlineFormat::N1BitMsbPal )
    {
        const BitmapColor   aCol0( pColBuf[ 0 ] );
        const BitmapColor   aCol1( pColBuf[ 1 ] );
        long                nMapX;

        for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (long nX = 0; nX < rDstBuffer.mnWidth;)
            {
                nMapX = pMapX[ nX ];
                pFncSetPixel( pDstScan, nX++,
                              pSrcScan[ nMapX >> 3 ] & ( 1 << ( 7 - ( nMapX & 7 ) ) ) ? aCol1 : aCol0,
                              rDstMask );
            }

            DOUBLE_SCANLINES();
        }
    }
    else if( RemoveScanline( rSrcBuffer.mnFormat ) == ScanlineFormat::N4BitMsnPal )
    {
        long nMapX;

        for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (long nX = 0; nX < rDstBuffer.mnWidth;)
            {
                nMapX = pMapX[ nX ];
                pFncSetPixel( pDstScan, nX++,
                              pColBuf[ ( pSrcScan[ nMapX >> 1 ] >> ( nMapX & 1 ? 0 : 4 ) ) & 0x0f ],
                              rDstMask );
            }

            DOUBLE_SCANLINES();
        }
    }
    else if( RemoveScanline( rSrcBuffer.mnFormat ) == ScanlineFormat::N8BitPal )
    {
        for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
                pFncSetPixel( pDstScan, nX, pColBuf[ pSrcScan[ pMapX[ nX ] ] ], rDstMask );

            DOUBLE_SCANLINES();
        }
    }
    else
    {
        for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
                pFncSetPixel( pDstScan, nX, pColBuf[ pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ).GetIndex() ], rDstMask );

            DOUBLE_SCANLINES();
        }
    }
}

static void ImplTCToTC( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                        FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                        Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nHeight1 = rDstBuffer.mnHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;

    if( RemoveScanline( rSrcBuffer.mnFormat ) == ScanlineFormat::N24BitTcBgr )
    {
        BitmapColor aCol;
        sal_uInt8* pPixel = nullptr;

        for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
            {
                aCol.SetBlue( *( pPixel = ( pSrcScan + pMapX[ nX ] * 3 ) )++ );
                aCol.SetGreen( *pPixel++ );
                aCol.SetRed( *pPixel );
                pFncSetPixel( pDstScan, nX, aCol, rDstMask );
            }

            DOUBLE_SCANLINES()
        }
    }
    else
    {
        for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
                pFncSetPixel( pDstScan, nX, pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ), rDstMask );

            DOUBLE_SCANLINES();
        }
    }
}

static void ImplTCToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nHeight1 = rDstBuffer.mnHeight- 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    BitmapPalette       aColMap( rSrcBuffer.maPalette.GetEntryCount() );
    std::unique_ptr<sal_uInt8[]> pColToPalMap(new sal_uInt8[ TC_TO_PAL_COLORS ]);
    BitmapColor         aIndex( 0 );

    for( long nR = 0; nR < 16; nR++ )
    {
        for( long nG = 0; nG < 16; nG++ )
        {
            for( long nB = 0; nB < 16; nB++ )
            {
                BitmapColor aCol( sal::static_int_cast<sal_uInt8>(nR << 4),
                                  sal::static_int_cast<sal_uInt8>(nG << 4),
                                  sal::static_int_cast<sal_uInt8>(nB << 4) );
                pColToPalMap[ ImplIndexFromColor( aCol ) ] = (sal_uInt8) rDstBuffer.maPalette.GetBestIndex( aCol );
            }
        }
    }

    for (long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
    {
        long nMapY = pMapY[nActY];
        Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

        for (long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
        {
            aIndex.SetIndex( pColToPalMap[ ImplIndexFromColor( pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ) ) ] );
            pFncSetPixel( pDstScan, nX, aIndex, rDstMask );
        }

        DOUBLE_SCANLINES();
    }
}

BitmapBuffer* StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    ScanlineFormat nDstBitmapFormat, const BitmapPalette* pDstPal, const ColorMask* pDstMask )
{
    FncGetPixel     pFncGetPixel;
    FncSetPixel     pFncSetPixel;
    BitmapBuffer*   pDstBuffer = new BitmapBuffer;

    // set function for getting pixels
    switch( RemoveScanline( rSrcBuffer.mnFormat ) )
    {
        IMPL_CASE_GET_FORMAT( N1BitMsbPal );
        IMPL_CASE_GET_FORMAT( N1BitLsbPal );
        IMPL_CASE_GET_FORMAT( N4BitMsnPal );
        IMPL_CASE_GET_FORMAT( N4BitLsnPal );
        IMPL_CASE_GET_FORMAT( N8BitPal );
        IMPL_CASE_GET_FORMAT( N8BitTcMask );
        IMPL_CASE_GET_FORMAT( N16BitTcMsbMask );
        IMPL_CASE_GET_FORMAT( N16BitTcLsbMask );
        IMPL_CASE_GET_FORMAT( N24BitTcBgr );
        IMPL_CASE_GET_FORMAT( N24BitTcRgb );
        IMPL_CASE_GET_FORMAT( N32BitTcAbgr );
        IMPL_CASE_GET_FORMAT( N32BitTcArgb );
        IMPL_CASE_GET_FORMAT( N32BitTcBgra );
        IMPL_CASE_GET_FORMAT( N32BitTcRgba );
        IMPL_CASE_GET_FORMAT( N32BitTcMask );

        default:
            // should never come here
            // initialize pFncGetPixel to something valid that is
            // least likely to crash
            pFncGetPixel = BitmapReadAccess::GetPixelForN1BitMsbPal;
            OSL_FAIL( "unknown read format" );
        break;
    }

    // set function for setting pixels
    const ScanlineFormat nDstScanlineFormat = RemoveScanline( nDstBitmapFormat );
    switch( nDstScanlineFormat )
    {
        IMPL_CASE_SET_FORMAT( N1BitMsbPal, 1 );
        IMPL_CASE_SET_FORMAT( N1BitLsbPal, 1 );
        IMPL_CASE_SET_FORMAT( N4BitMsnPal, 1 );
        IMPL_CASE_SET_FORMAT( N4BitLsnPal, 4 );
        IMPL_CASE_SET_FORMAT( N8BitPal, 8 );
        IMPL_CASE_SET_FORMAT( N8BitTcMask, 8 );
        IMPL_CASE_SET_FORMAT( N16BitTcMsbMask, 16 );
        IMPL_CASE_SET_FORMAT( N16BitTcLsbMask, 16 );
        IMPL_CASE_SET_FORMAT( N24BitTcBgr, 24 );
        IMPL_CASE_SET_FORMAT( N24BitTcRgb, 24 );
        IMPL_CASE_SET_FORMAT( N32BitTcAbgr, 32 );
        IMPL_CASE_SET_FORMAT( N32BitTcArgb, 32 );
        IMPL_CASE_SET_FORMAT( N32BitTcBgra, 32 );
        IMPL_CASE_SET_FORMAT( N32BitTcRgba, 32 );
        IMPL_CASE_SET_FORMAT( N32BitTcMask, 32 );

        default:
            // should never come here
            // initialize pFncSetPixel to something valid that is
            // least likely to crash
            pFncSetPixel = BitmapReadAccess::SetPixelForN1BitMsbPal;
            pDstBuffer->mnBitCount = 1;
            OSL_FAIL( "unknown write format" );
        break;
    }

    // fill destination buffer
    pDstBuffer->mnFormat = nDstBitmapFormat;
    pDstBuffer->mnWidth = rTwoRect.mnDestWidth;
    pDstBuffer->mnHeight = rTwoRect.mnDestHeight;
    pDstBuffer->mnScanlineSize = AlignedWidth4Bytes( pDstBuffer->mnBitCount * pDstBuffer->mnWidth );
    try
    {
        pDstBuffer->mpBits = new sal_uInt8[ pDstBuffer->mnScanlineSize * pDstBuffer->mnHeight ];
    }
    catch( const std::bad_alloc& )
    {
        // memory exception, clean up
        pDstBuffer->mpBits = nullptr;
        delete pDstBuffer;
        return nullptr;
    }

    // do we need a destination palette or color mask?
    if( ( nDstScanlineFormat == ScanlineFormat::N1BitMsbPal ) ||
        ( nDstScanlineFormat == ScanlineFormat::N1BitLsbPal ) ||
        ( nDstScanlineFormat == ScanlineFormat::N4BitMsnPal ) ||
        ( nDstScanlineFormat == ScanlineFormat::N4BitLsnPal ) ||
        ( nDstScanlineFormat == ScanlineFormat::N8BitPal ) )
    {
        assert(pDstPal && "destination buffer requires palette");
        if (!pDstPal)
        {
            delete pDstBuffer;
            return nullptr;
        }
        pDstBuffer->maPalette = *pDstPal;
    }
    else if( ( nDstScanlineFormat == ScanlineFormat::N8BitTcMask ) ||
             ( nDstScanlineFormat == ScanlineFormat::N16BitTcMsbMask ) ||
             ( nDstScanlineFormat == ScanlineFormat::N16BitTcLsbMask ) ||
             ( nDstScanlineFormat == ScanlineFormat::N32BitTcMask ) )
    {
        assert(pDstMask && "destination buffer requires color mask");
        if (!pDstMask)
        {
            delete pDstBuffer;
            return nullptr;
        }
        pDstBuffer->maColorMask = *pDstMask;
    }

    // short circuit the most important conversions
    bool bFastConvert = ImplFastBitmapConversion( *pDstBuffer, rSrcBuffer, rTwoRect );
    if( bFastConvert )
        return pDstBuffer;

    std::unique_ptr<Scanline[]> pSrcScan;
    std::unique_ptr<Scanline[]> pDstScan;
    std::unique_ptr<long[]>     pMapX;
    std::unique_ptr<long[]>     pMapY;

    try
    {
        pSrcScan.reset(new Scanline[rSrcBuffer.mnHeight]);
        pDstScan.reset(new Scanline[pDstBuffer->mnHeight]);
        pMapX.reset(new long[pDstBuffer->mnWidth]);
        pMapY.reset(new long[pDstBuffer->mnHeight]);
    }
    catch( const std::bad_alloc& )
    {
        // memory exception, clean up
        // remark: the buffer ptr causing the exception
        // is still NULL here
        delete pDstBuffer;
        return nullptr;
    }

    // horizontal mapping table
    if( (pDstBuffer->mnWidth != rTwoRect.mnSrcWidth) && (pDstBuffer->mnWidth != 0) )
    {
        const double fFactorX = (double)rTwoRect.mnSrcWidth / pDstBuffer->mnWidth;

        for (long i = 0; i < pDstBuffer->mnWidth; ++i)
            pMapX[ i ] = rTwoRect.mnSrcX + static_cast<int>( i * fFactorX );
    }
    else
    {
        for (long i = 0, nTmp = rTwoRect.mnSrcX ; i < pDstBuffer->mnWidth; ++i)
            pMapX[ i ] = nTmp++;
    }

    // vertical mapping table
    if( (pDstBuffer->mnHeight != rTwoRect.mnSrcHeight) && (pDstBuffer->mnHeight != 0) )
    {
        const double fFactorY = (double)rTwoRect.mnSrcHeight / pDstBuffer->mnHeight;

        for (long i = 0; i < pDstBuffer->mnHeight; ++i)
            pMapY[ i ] = rTwoRect.mnSrcY + static_cast<int>( i * fFactorY );
    }
    else
    {
        for (long i = 0, nTmp = rTwoRect.mnSrcY; i < pDstBuffer->mnHeight; ++i)
            pMapY[ i ] = nTmp++;
    }

    // source scanline buffer
    Scanline pTmpScan;
    long nOffset;
    if( rSrcBuffer.mnFormat & ScanlineFormat::TopDown )
    {
        pTmpScan = rSrcBuffer.mpBits;
        nOffset = rSrcBuffer.mnScanlineSize;
    }
    else
    {
        pTmpScan = rSrcBuffer.mpBits + ( rSrcBuffer.mnHeight - 1 ) * rSrcBuffer.mnScanlineSize;
        nOffset = -rSrcBuffer.mnScanlineSize;
    }

    for (long i = 0; i < rSrcBuffer.mnHeight; i++, pTmpScan += nOffset)
        pSrcScan[ i ] = pTmpScan;

    // destination scanline buffer
    if( pDstBuffer->mnFormat & ScanlineFormat::TopDown )
    {
        pTmpScan = pDstBuffer->mpBits;
        nOffset = pDstBuffer->mnScanlineSize;
    }
    else
    {
        pTmpScan = pDstBuffer->mpBits + ( pDstBuffer->mnHeight - 1 ) * pDstBuffer->mnScanlineSize;
        nOffset = -pDstBuffer->mnScanlineSize;
    }

    for (long i = 0; i < pDstBuffer->mnHeight; i++, pTmpScan += nOffset)
        pDstScan[ i ] = pTmpScan;

    // do buffer scaling and conversion
    if( rSrcBuffer.mnBitCount <= 8 && pDstBuffer->mnBitCount <= 8 )
    {
        ImplPALToPAL( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                      pSrcScan.get(), pDstScan.get(), pMapX.get(), pMapY.get() );
    }
    else if( rSrcBuffer.mnBitCount <= 8 && pDstBuffer->mnBitCount > 8 )
    {
        ImplPALToTC( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                     pSrcScan.get(), pDstScan.get(), pMapX.get(), pMapY.get() );
    }
    else if( rSrcBuffer.mnBitCount > 8 && pDstBuffer->mnBitCount > 8 )
    {
        ImplTCToTC( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                    pSrcScan.get(), pDstScan.get(), pMapX.get(), pMapY.get() );
    }
    else
    {
        ImplTCToPAL( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                     pSrcScan.get(), pDstScan.get(), pMapX.get(), pMapY.get() );
    }

    return pDstBuffer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
