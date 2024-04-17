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

#include <vcl/BitmapReadAccess.hxx>
#include <vcl/salgtype.hxx>
#include <bitmap/bmpfast.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/helpers.hxx>
#include <memory>

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

constexpr int TC_TO_PAL_COLORS = 4096;

static tools::Long ImplIndexFromColor( const BitmapColor& rCol )
{
    return  ( ( static_cast<tools::Long>(rCol.GetBlue()) >> 4) << 8 ) |
            ( ( static_cast<tools::Long>(rCol.GetGreen()) >> 4 ) << 4 ) |
            ( static_cast<tools::Long>(rCol.GetRed()) >> 4 );
}

static void ImplPALToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                          FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                          Scanline* pSrcScanMap, Scanline* pDstScanMap, sal_Int32 const * pMapX, const sal_Int32* pMapY )
{
    const tools::Long          nHeight1 = rDstBuffer.mnHeight - 1;
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

    for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
    {
        tools::Long nMapY = pMapY[nActY];
        Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

        for (tools::Long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
            pFncSetPixel( pDstScan, nX, pColMapBuf[ pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ).GetIndex() ], rDstMask );

        DOUBLE_SCANLINES();
    }
}

static void ImplPALToTC( const BitmapBuffer& rSrcBuffer, BitmapBuffer const & rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, sal_Int32 const * pMapX, const sal_Int32* pMapY )
{
    const tools::Long          nHeight1 = rDstBuffer.mnHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    const BitmapColor*  pColBuf = rSrcBuffer.maPalette.ImplGetColorBuffer();

    if (rSrcBuffer.meFormat == ScanlineFormat::N1BitMsbPal)
    {
        const BitmapColor   aCol0( pColBuf[ 0 ] );
        const BitmapColor   aCol1( pColBuf[ 1 ] );
        tools::Long                nMapX;

        for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            tools::Long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (tools::Long nX = 0; nX < rDstBuffer.mnWidth;)
            {
                nMapX = pMapX[ nX ];
                pFncSetPixel( pDstScan, nX++,
                              pSrcScan[ nMapX >> 3 ] & ( 1 << ( 7 - ( nMapX & 7 ) ) ) ? aCol1 : aCol0,
                              rDstMask );
            }

            DOUBLE_SCANLINES();
        }
    }
    else if (rSrcBuffer.meFormat == ScanlineFormat::N8BitPal)
    {
        for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            tools::Long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (tools::Long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
                pFncSetPixel( pDstScan, nX, pColBuf[ pSrcScan[ pMapX[ nX ] ] ], rDstMask );

            DOUBLE_SCANLINES();
        }
    }
    else
    {
        for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            tools::Long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (tools::Long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
                pFncSetPixel( pDstScan, nX, pColBuf[ pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ).GetIndex() ], rDstMask );

            DOUBLE_SCANLINES();
        }
    }
}

static void ImplTCToTC( const BitmapBuffer& rSrcBuffer, BitmapBuffer const & rDstBuffer,
                        FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                        Scanline* pSrcScanMap, Scanline* pDstScanMap, sal_Int32 const * pMapX, const sal_Int32* pMapY )
{
    const tools::Long          nHeight1 = rDstBuffer.mnHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;

    if (rSrcBuffer.meFormat == ScanlineFormat::N24BitTcBgr)
    {
        BitmapColor aCol;
        sal_uInt8* pPixel = nullptr;

        for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            tools::Long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (tools::Long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
            {
                pPixel = pSrcScan + pMapX[ nX ] * 3;
                aCol.SetBlue( *pPixel++ );
                aCol.SetGreen( *pPixel++ );
                aCol.SetRed( *pPixel );
                pFncSetPixel( pDstScan, nX, aCol, rDstMask );
            }

            DOUBLE_SCANLINES()
        }
    }
    else
    {
        for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
        {
            tools::Long nMapY = pMapY[nActY];
            Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

            for (tools::Long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
                pFncSetPixel( pDstScan, nX, pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ), rDstMask );

            DOUBLE_SCANLINES();
        }
    }
}

static void ImplTCToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer const & rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, sal_Int32 const * pMapX, const sal_Int32* pMapY )
{
    const tools::Long          nHeight1 = rDstBuffer.mnHeight- 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    std::unique_ptr<sal_uInt8[]> pColToPalMap(new sal_uInt8[ TC_TO_PAL_COLORS ]);
    BitmapColor         aIndex( 0 );

    for( tools::Long nR = 0; nR < 16; nR++ )
    {
        for( tools::Long nG = 0; nG < 16; nG++ )
        {
            for( tools::Long nB = 0; nB < 16; nB++ )
            {
                BitmapColor aCol( sal::static_int_cast<sal_uInt8>(nR << 4),
                                  sal::static_int_cast<sal_uInt8>(nG << 4),
                                  sal::static_int_cast<sal_uInt8>(nB << 4) );
                pColToPalMap[ ImplIndexFromColor( aCol ) ] = static_cast<sal_uInt8>(rDstBuffer.maPalette.GetBestIndex( aCol ));
            }
        }
    }

    for (tools::Long nActY = 0; nActY < rDstBuffer.mnHeight; ++nActY)
    {
        tools::Long nMapY = pMapY[nActY];
        Scanline pSrcScan(pSrcScanMap[nMapY]), pDstScan(pDstScanMap[nActY]);

        for (tools::Long nX = 0; nX < rDstBuffer.mnWidth; ++nX)
        {
            aIndex.SetIndex( pColToPalMap[ ImplIndexFromColor( pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ) ) ] );
            pFncSetPixel( pDstScan, nX, aIndex, rDstMask );
        }

        DOUBLE_SCANLINES();
    }
}

std::optional<BitmapBuffer> StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    ScanlineFormat nDstBitmapFormat, std::optional<BitmapPalette> pDstPal, const ColorMask* pDstMask )
{
    FncGetPixel     pFncGetPixel;
    FncSetPixel     pFncSetPixel;
    std::optional<BitmapBuffer> pDstBuffer(std::in_place);


    pDstBuffer->meDirection = rSrcBuffer.meDirection;
    // set function for getting pixels
    pFncGetPixel = BitmapReadAccess::GetPixelFunction(rSrcBuffer.meFormat);
    if( !pFncGetPixel )
    {
        // should never come here
        // initialize pFncGetPixel to something valid that is
        // least likely to crash
        pFncGetPixel = BitmapReadAccess::GetPixelForN1BitMsbPal;
        OSL_FAIL( "unknown read format" );
    }

    // set function for setting pixels
    switch (nDstBitmapFormat)
    {
        IMPL_CASE_SET_FORMAT( N1BitMsbPal, 1 );
        IMPL_CASE_SET_FORMAT( N8BitPal, 8 );
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
    pDstBuffer->meFormat = nDstBitmapFormat;
    pDstBuffer->mnWidth = rTwoRect.mnDestWidth;
    pDstBuffer->mnHeight = rTwoRect.mnDestHeight;
    tools::Long nScanlineBase;
    bool bFail = o3tl::checked_multiply<tools::Long>(pDstBuffer->mnBitCount, pDstBuffer->mnWidth, nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl.gdi", "checked multiply failed");
        pDstBuffer->mpBits = nullptr;
        return std::nullopt;
    }
    pDstBuffer->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (pDstBuffer->mnScanlineSize < nScanlineBase/8)
    {
        SAL_WARN("vcl.gdi", "scanline calculation wraparound");
        pDstBuffer->mpBits = nullptr;
        return std::nullopt;
    }
    try
    {
        pDstBuffer->mpBits = new sal_uInt8[ pDstBuffer->mnScanlineSize * pDstBuffer->mnHeight ];
    }
    catch( const std::bad_alloc& )
    {
        // memory exception, clean up
        pDstBuffer->mpBits = nullptr;
        return std::nullopt;
    }

    // do we need a destination palette or color mask?
    if (nDstBitmapFormat == ScanlineFormat::N1BitMsbPal ||
        nDstBitmapFormat == ScanlineFormat::N8BitPal)
    {
        assert(pDstPal && "destination buffer requires palette");
        if (!pDstPal)
        {
            return std::nullopt;
        }
        pDstBuffer->maPalette = *pDstPal;
    }
    else if (nDstBitmapFormat == ScanlineFormat::N32BitTcMask)
    {
        assert(pDstMask && "destination buffer requires color mask");
        if (!pDstMask)
        {
            return std::nullopt;
        }
        pDstBuffer->maColorMask = *pDstMask;
    }

    // short circuit the most important conversions
    bool bFastConvert = ImplFastBitmapConversion( *pDstBuffer, rSrcBuffer, rTwoRect );
    if( bFastConvert )
        return pDstBuffer;

    std::unique_ptr<Scanline[]> pSrcScan;
    std::unique_ptr<Scanline[]> pDstScan;
    std::unique_ptr<sal_Int32[]> pMapX;
    std::unique_ptr<sal_Int32[]> pMapY;

    try
    {
        pSrcScan.reset(new Scanline[rSrcBuffer.mnHeight]);
        pDstScan.reset(new Scanline[pDstBuffer->mnHeight]);
        pMapX.reset(new sal_Int32[pDstBuffer->mnWidth]);
        pMapY.reset(new sal_Int32[pDstBuffer->mnHeight]);
    }
    catch( const std::bad_alloc& )
    {
        // memory exception, clean up
        // remark: the buffer ptr causing the exception
        // is still NULL here
        return std::nullopt;
    }

    // horizontal mapping table
    if( (pDstBuffer->mnWidth != rTwoRect.mnSrcWidth) && (pDstBuffer->mnWidth != 0) )
    {
        const double fFactorX = static_cast<double>(rTwoRect.mnSrcWidth) / pDstBuffer->mnWidth;

        for (tools::Long i = 0; i < pDstBuffer->mnWidth; ++i)
            pMapX[ i ] = rTwoRect.mnSrcX + static_cast<int>( i * fFactorX );
    }
    else
    {
        for (tools::Long i = 0, nTmp = rTwoRect.mnSrcX ; i < pDstBuffer->mnWidth; ++i)
            pMapX[ i ] = nTmp++;
    }

    // vertical mapping table
    if( (pDstBuffer->mnHeight != rTwoRect.mnSrcHeight) && (pDstBuffer->mnHeight != 0) )
    {
        const double fFactorY = static_cast<double>(rTwoRect.mnSrcHeight) / pDstBuffer->mnHeight;

        for (tools::Long i = 0; i < pDstBuffer->mnHeight; ++i)
            pMapY[ i ] = rTwoRect.mnSrcY + static_cast<int>( i * fFactorY );
    }
    else
    {
        for (tools::Long i = 0, nTmp = rTwoRect.mnSrcY; i < pDstBuffer->mnHeight; ++i)
            pMapY[ i ] = nTmp++;
    }

    // source scanline buffer
    Scanline pTmpScan;
    tools::Long nOffset;
    if (rSrcBuffer.meDirection == ScanlineDirection::TopDown)
    {
        pTmpScan = rSrcBuffer.mpBits;
        nOffset = rSrcBuffer.mnScanlineSize;
    }
    else
    {
        pTmpScan = rSrcBuffer.mpBits + ( rSrcBuffer.mnHeight - 1 ) * rSrcBuffer.mnScanlineSize;
        nOffset = -rSrcBuffer.mnScanlineSize;
    }

    for (tools::Long i = 0; i < rSrcBuffer.mnHeight; i++, pTmpScan += nOffset)
        pSrcScan[ i ] = pTmpScan;

    // destination scanline buffer
    if (pDstBuffer->meDirection == ScanlineDirection::TopDown)
    {
        pTmpScan = pDstBuffer->mpBits;
        nOffset = pDstBuffer->mnScanlineSize;
    }
    else
    {
        pTmpScan = pDstBuffer->mpBits + ( pDstBuffer->mnHeight - 1 ) * pDstBuffer->mnScanlineSize;
        nOffset = -pDstBuffer->mnScanlineSize;
    }

    for (tools::Long i = 0; i < pDstBuffer->mnHeight; i++, pTmpScan += nOffset)
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
