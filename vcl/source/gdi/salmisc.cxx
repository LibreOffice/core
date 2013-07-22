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

#include <vcl/bmpacc.hxx>
#include <vcl/salbtype.hxx>
#include <bmpfast.hxx>

#define IMPL_CASE_GET_FORMAT( Format )                          \
case( BMP_FORMAT##Format ):                                 \
    pFncGetPixel = BitmapReadAccess::GetPixelFor##Format;       \
break

#define IMPL_CASE_SET_FORMAT( Format, BitCount )                \
case( BMP_FORMAT##Format ):                                 \
{                                                               \
    pFncSetPixel = BitmapReadAccess::SetPixelFor##Format;       \
    pDstBuffer->mnBitCount = BitCount;                          \
}                                                               \
break

#define DOUBLE_SCANLINES()                                                      \
while( ( nActY < nHeight1 ) && ( pMapY[ nActY + 1 ] == nMapY ) )                \
{                                                                               \
    memcpy( pDstScanMap[ nActY + 1L ], pDstScan, rDstBuffer.mnScanlineSize );   \
    nActY++;                                                                    \
}

#define TC_TO_PAL_COLORS    4096

static long ImplIndexFromColor( const BitmapColor& rCol )
{
#if TC_TO_PAL_COLORS == 4096

    return( ( ( (long) rCol.GetBlue() >> 4L) << 8L ) |
            ( ( (long) rCol.GetGreen() >> 4L ) << 4L ) |
            ( (long) rCol.GetRed() >> 4L ) );

#elif TC_TO_PAL_COLORS == 32768

    return( ( ( (long) rCol.GetBlue() >> 3L) << 10L ) |
            ( ( (long) rCol.GetGreen() >> 3L ) << 5L ) |
            ( (long) rCol.GetRed() >> 3L ) );

#endif
}

static void ImplPALToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                          FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                          Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nWidth = rDstBuffer.mnWidth, nHeight = rDstBuffer.mnHeight, nHeight1 = nHeight - 1;
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

    for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
    {
        Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

        for( long nX = 0L; nX < nWidth; nX++ )
            pFncSetPixel( pDstScan, nX, pColMapBuf[ pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ).GetIndex() ], rDstMask );

        DOUBLE_SCANLINES();
    }
}

static void ImplPALToTC( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nWidth = rDstBuffer.mnWidth, nHeight = rDstBuffer.mnHeight, nHeight1 = nHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    const BitmapColor*  pColBuf = rSrcBuffer.maPalette.ImplGetColorBuffer();

    if( BMP_SCANLINE_FORMAT( rSrcBuffer.mnFormat ) == BMP_FORMAT_1BIT_MSB_PAL )
    {
        const BitmapColor   aCol0( pColBuf[ 0 ] );
        const BitmapColor   aCol1( pColBuf[ 1 ] );
        long                nMapX;

        for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
        {
            Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

            for( long nX = 0L; nX < nWidth; )
            {
                nMapX = pMapX[ nX ];
                pFncSetPixel( pDstScan, nX++,
                              pSrcScan[ nMapX >> 3 ] & ( 1 << ( 7 - ( nMapX & 7 ) ) ) ? aCol1 : aCol0,
                              rDstMask );
            }

            DOUBLE_SCANLINES();
        }
    }
    else if( BMP_SCANLINE_FORMAT( rSrcBuffer.mnFormat ) == BMP_FORMAT_4BIT_MSN_PAL )
    {
        long nMapX;

        for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
        {
            Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

            for( long nX = 0L; nX < nWidth; )
            {
                nMapX = pMapX[ nX ];
                pFncSetPixel( pDstScan, nX++,
                              pColBuf[ ( pSrcScan[ nMapX >> 1 ] >> ( nMapX & 1 ? 0 : 4 ) ) & 0x0f ],
                              rDstMask );
            }

            DOUBLE_SCANLINES();
        }
    }
    else if( BMP_SCANLINE_FORMAT( rSrcBuffer.mnFormat ) == BMP_FORMAT_8BIT_PAL )
    {
        for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
        {
            Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

            for( long nX = 0L; nX < nWidth; nX++ )
                pFncSetPixel( pDstScan, nX, pColBuf[ pSrcScan[ pMapX[ nX ] ] ], rDstMask );

            DOUBLE_SCANLINES();
        }
    }
    else
    {
        for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
        {
            Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

            for( long nX = 0L; nX < nWidth; nX++ )
                pFncSetPixel( pDstScan, nX, pColBuf[ pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ).GetIndex() ], rDstMask );

            DOUBLE_SCANLINES();
        }
    }
}

static void ImplTCToTC( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                        FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                        Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nWidth = rDstBuffer.mnWidth, nHeight = rDstBuffer.mnHeight, nHeight1 = nHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;

    if( BMP_SCANLINE_FORMAT( rSrcBuffer.mnFormat ) == BMP_FORMAT_24BIT_TC_BGR )
    {
        BitmapColor aCol;
        sal_uInt8* pPixel = NULL;

        for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
        {
            Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

            for( long nX = 0L; nX < nWidth; nX++ )
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
        for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
        {
            Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

            for( long nX = 0L; nX < nWidth; nX++ )
                pFncSetPixel( pDstScan, nX, pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ), rDstMask );

            DOUBLE_SCANLINES();
        }
    }
}

static void ImplTCToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nWidth = rDstBuffer.mnWidth, nHeight = rDstBuffer.mnHeight, nHeight1 = nHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    BitmapPalette       aColMap( rSrcBuffer.maPalette.GetEntryCount() );
    sal_uInt8*              pColToPalMap = new sal_uInt8[ TC_TO_PAL_COLORS ];
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

    for( long nActY = 0, nMapY; nActY < nHeight; nActY++ )
    {
        Scanline pSrcScan( pSrcScanMap[ nMapY = pMapY[ nActY ] ] ), pDstScan( pDstScanMap[ nActY ] );

        for( long nX = 0L; nX < nWidth; nX++ )
        {
            aIndex.SetIndex( pColToPalMap[ ImplIndexFromColor( pFncGetPixel( pSrcScan, pMapX[ nX ], rSrcMask ) ) ] );
            pFncSetPixel( pDstScan, nX, aIndex, rDstMask );
        }

        DOUBLE_SCANLINES();
    }

    delete[] pColToPalMap;
}

BitmapBuffer* StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    sal_uLong nDstBitmapFormat, const BitmapPalette* pDstPal, const ColorMask* pDstMask )
{
    FncGetPixel     pFncGetPixel;
    FncSetPixel     pFncSetPixel;
    BitmapBuffer*   pDstBuffer = new BitmapBuffer;
    long            i;

    // set function for getting pixels
    switch( BMP_SCANLINE_FORMAT( rSrcBuffer.mnFormat ) )
    {
        IMPL_CASE_GET_FORMAT( _1BIT_MSB_PAL );
        IMPL_CASE_GET_FORMAT( _1BIT_LSB_PAL );
        IMPL_CASE_GET_FORMAT( _4BIT_MSN_PAL );
        IMPL_CASE_GET_FORMAT( _4BIT_LSN_PAL );
        IMPL_CASE_GET_FORMAT( _8BIT_PAL );
        IMPL_CASE_GET_FORMAT( _8BIT_TC_MASK );
        IMPL_CASE_GET_FORMAT( _16BIT_TC_MSB_MASK );
        IMPL_CASE_GET_FORMAT( _16BIT_TC_LSB_MASK );
        IMPL_CASE_GET_FORMAT( _24BIT_TC_BGR );
        IMPL_CASE_GET_FORMAT( _24BIT_TC_RGB );
        IMPL_CASE_GET_FORMAT( _24BIT_TC_MASK );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_ABGR );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_ARGB );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_BGRA );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_RGBA );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_MASK );

        default:
            // should never come here
            // initialize pFncGetPixel to something valid that is
            // least likely to crash
            pFncGetPixel = BitmapReadAccess::GetPixelFor_1BIT_MSB_PAL;
            OSL_FAIL( "unknown read format" );
        break;
    }

    // set function for setting pixels
    const sal_uLong nDstScanlineFormat = BMP_SCANLINE_FORMAT( nDstBitmapFormat );
    switch( nDstScanlineFormat )
    {
        IMPL_CASE_SET_FORMAT( _1BIT_MSB_PAL, 1 );
        IMPL_CASE_SET_FORMAT( _1BIT_LSB_PAL, 1 );
        IMPL_CASE_SET_FORMAT( _4BIT_MSN_PAL, 1 );
        IMPL_CASE_SET_FORMAT( _4BIT_LSN_PAL, 4 );
        IMPL_CASE_SET_FORMAT( _8BIT_PAL, 8 );
        IMPL_CASE_SET_FORMAT( _8BIT_TC_MASK, 8 );
        IMPL_CASE_SET_FORMAT( _16BIT_TC_MSB_MASK, 16 );
        IMPL_CASE_SET_FORMAT( _16BIT_TC_LSB_MASK, 16 );
        IMPL_CASE_SET_FORMAT( _24BIT_TC_BGR, 24 );
        IMPL_CASE_SET_FORMAT( _24BIT_TC_RGB, 24 );
        IMPL_CASE_SET_FORMAT( _24BIT_TC_MASK, 24 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_ABGR, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_ARGB, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_BGRA, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_RGBA, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_MASK, 32 );

        default:
            // should never come here
            // initialize pFncSetPixel to something valid that is
            // least likely to crash
            pFncSetPixel = BitmapReadAccess::SetPixelFor_1BIT_MSB_PAL;
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
        pDstBuffer->mpBits = NULL;
        delete pDstBuffer;
        return NULL;
    }

    // do we need a destination palette or color mask?
    if( ( nDstScanlineFormat == BMP_FORMAT_1BIT_MSB_PAL ) ||
        ( nDstScanlineFormat == BMP_FORMAT_1BIT_LSB_PAL ) ||
        ( nDstScanlineFormat == BMP_FORMAT_4BIT_MSN_PAL ) ||
        ( nDstScanlineFormat == BMP_FORMAT_4BIT_LSN_PAL ) ||
        ( nDstScanlineFormat == BMP_FORMAT_8BIT_PAL ) )
    {
        DBG_ASSERT( pDstPal, "destination buffer requires palette" );
        pDstBuffer->maPalette = *pDstPal;
    }
    else if( ( nDstScanlineFormat == BMP_FORMAT_8BIT_TC_MASK ) ||
             ( nDstScanlineFormat == BMP_FORMAT_16BIT_TC_MSB_MASK ) ||
             ( nDstScanlineFormat == BMP_FORMAT_16BIT_TC_LSB_MASK ) ||
             ( nDstScanlineFormat == BMP_FORMAT_24BIT_TC_MASK ) ||
             ( nDstScanlineFormat == BMP_FORMAT_32BIT_TC_MASK ) )
    {
        DBG_ASSERT( pDstMask, "destination buffer requires color mask" );
        pDstBuffer->maColorMask = *pDstMask;
    }

    // short circuit the most important conversions
    bool bFastConvert = ImplFastBitmapConversion( *pDstBuffer, rSrcBuffer, rTwoRect );
    if( bFastConvert )
        return pDstBuffer;

    const long      nSrcX = rTwoRect.mnSrcX, nSrcY = rTwoRect.mnSrcY;
    const long      nSrcDX = rTwoRect.mnSrcWidth, nSrcDY = rTwoRect.mnSrcHeight;
    const long      nDstDX = rTwoRect.mnDestWidth, nDstDY = rTwoRect.mnDestHeight;
    Scanline*       pSrcScan = NULL;
    Scanline*       pDstScan = NULL;
    long*           pMapX = NULL;
    long*           pMapY = NULL;
    long            nTmp, nOffset;

    try
    {
        pSrcScan = new Scanline[ rSrcBuffer.mnHeight ];
        pDstScan = new Scanline[ nDstDY ];
        pMapX = new long[ nDstDX ];
        pMapY = new long[ nDstDY ];
    }
    catch( const std::bad_alloc& )
    {
        // memory exception, clean up
        // remark: the buffer ptr causing the exception
        // is still NULL here
        delete[] pSrcScan;
        delete[] pDstScan;
        delete[] pMapX;
        delete[] pMapY;
        delete pDstBuffer;
        return NULL;
    }

    // horizontal mapping table
    if( (nDstDX != nSrcDX) && (nDstDX != 0) )
    {
        const double fFactorX = (double)nSrcDX / nDstDX;

        for( i = 0L; i < nDstDX; i++ )
            pMapX[ i ] = nSrcX + static_cast<int>( i * fFactorX );
    }
    else
    {
        for( i = 0L, nTmp = nSrcX; i < nDstDX; i++ )
            pMapX[ i ] = nTmp++;
    }

    // vertical mapping table
    if( (nDstDY != nSrcDY) && (nDstDY != 0) )
    {
        const double fFactorY = (double)nSrcDY / nDstDY;

        for( i = 0L; i < nDstDY; i++ )
            pMapY[ i ] = nSrcY + static_cast<int>( i * fFactorY );
    }
    else
    {
        for( i = 0L, nTmp = nSrcY; i < nDstDY; i++ )
            pMapY[ i ] = nTmp++;
    }

    // source scanline buffer
    Scanline pTmpScan;
    if( BMP_SCANLINE_ADJUSTMENT( rSrcBuffer.mnFormat ) == BMP_FORMAT_TOP_DOWN )
        pTmpScan = rSrcBuffer.mpBits, nOffset = rSrcBuffer.mnScanlineSize;
    else
    {
        pTmpScan = rSrcBuffer.mpBits + ( rSrcBuffer.mnHeight - 1 ) * rSrcBuffer.mnScanlineSize;
        nOffset = -rSrcBuffer.mnScanlineSize;
    }

    for( i = 0L; i < rSrcBuffer.mnHeight; i++, pTmpScan += nOffset )
        pSrcScan[ i ] = pTmpScan;

    // destination scanline buffer
    if( BMP_SCANLINE_ADJUSTMENT( pDstBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
        pTmpScan = pDstBuffer->mpBits, nOffset = pDstBuffer->mnScanlineSize;
    else
    {
        pTmpScan = pDstBuffer->mpBits + ( nDstDY - 1 ) * pDstBuffer->mnScanlineSize;
        nOffset = -pDstBuffer->mnScanlineSize;
    }

    for( i = 0L; i < nDstDY; i++, pTmpScan += nOffset )
        pDstScan[ i ] = pTmpScan;

    // do buffer scaling and conversion
    if( rSrcBuffer.mnBitCount <= 8 && pDstBuffer->mnBitCount <= 8 )
    {
        ImplPALToPAL( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                      pSrcScan, pDstScan, pMapX, pMapY );
    }
    else if( rSrcBuffer.mnBitCount <= 8 && pDstBuffer->mnBitCount > 8 )
    {
        ImplPALToTC( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                     pSrcScan, pDstScan, pMapX, pMapY );
    }
    else if( rSrcBuffer.mnBitCount > 8 && pDstBuffer->mnBitCount > 8 )
    {
        ImplTCToTC( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                    pSrcScan, pDstScan, pMapX, pMapY );
    }
    else
    {
        ImplTCToPAL( rSrcBuffer, *pDstBuffer, pFncGetPixel, pFncSetPixel,
                     pSrcScan, pDstScan, pMapX, pMapY );
    }

    // cleanup
    delete[] pSrcScan;
    delete[] pDstScan;
    delete[] pMapX;
    delete[] pMapY;

    return pDstBuffer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
