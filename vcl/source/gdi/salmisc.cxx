/*************************************************************************
 *
 *  $RCSfile: salmisc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#include <bmpacc.hxx>
#include <salbtype.hxx>

// -----------
// - Defines -
// -----------

#define IMPL_CASE_GET_FORMAT( Format )                          \
case( BMP_FORMAT##Format ):                                 \
    pFncGetPixel = BitmapReadAccess::GetPixelFor##Format;       \
break

// -----------------------------------------------------------------------------

#define IMPL_CASE_SET_FORMAT( Format, BitCount )                \
case( BMP_FORMAT##Format ):                                 \
{                                                               \
    pFncSetPixel = BitmapReadAccess::SetPixelFor##Format;       \
    pDstBuffer->mnBitCount = BitCount;                          \
}                                                               \
break

// -----------------------------------------------------------------------------

#define DOUBLE_SCANLINES()                                                      \
while( ( nActY < nHeight1 ) && ( pMapY[ nActY + 1 ] == nMapY ) )                \
{                                                                               \
    HMEMCPY( pDstScanMap[ nActY + 1L ], pDstScan, rDstBuffer.mnScanlineSize );  \
    nActY++;                                                                    \
}

// -----------
// - Inlines -
// -----------

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


#define COLOR_TO_INDEX( _def_rCol )

// ------------------------
// - conversion functions -
// ------------------------

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

    for( USHORT i = 0, nCount = aColMap.GetEntryCount(); i < nCount; i++ )
    {
        if( rSrcBuffer.maPalette[ i ] == rDstBuffer.maPalette[ i ] )
            aIndex.SetIndex( i );
        else
            aIndex.SetIndex( rDstBuffer.maPalette.GetBestIndex( rSrcBuffer.maPalette[ i ] ) );

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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
        BYTE*       pPixel;

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

// -----------------------------------------------------------------------------

static void ImplTCToPAL( const BitmapBuffer& rSrcBuffer, BitmapBuffer& rDstBuffer,
                         FncGetPixel pFncGetPixel, FncSetPixel pFncSetPixel,
                         Scanline* pSrcScanMap, Scanline* pDstScanMap, long* pMapX, long* pMapY )
{
    const long          nWidth = rDstBuffer.mnWidth, nHeight = rDstBuffer.mnHeight, nHeight1 = nHeight - 1;
    const ColorMask&    rSrcMask = rSrcBuffer.maColorMask;
    const ColorMask&    rDstMask = rDstBuffer.maColorMask;
    BitmapPalette       aColMap( rSrcBuffer.maPalette.GetEntryCount() );
    BYTE*               pColToPalMap = new BYTE[ TC_TO_PAL_COLORS ];
    BitmapColor         aIndex( 0 );

    for( long nR = 0; nR < 16; nR++ )
    {
        for( long nG = 0; nG < 16; nG++ )
        {
            for( long nB = 0; nB < 16; nB++ )
            {
                BitmapColor aCol( nR << 4, nG << 4, nB << 4 );
                pColToPalMap[ ImplIndexFromColor( aCol ) ] = (BYTE) rDstBuffer.maPalette.GetBestIndex( aCol );
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

// ---------------------
// - StretchAndConvert -
// ---------------------

BitmapBuffer* StretchAndConvert( const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
                                 ULONG nDstBitmapFormat, BitmapPalette* pDstPal, ColorMask* pDstMask )
{
    FncGetPixel     pFncGetPixel;
    FncSetPixel     pFncSetPixel;
    BitmapBuffer*   pDstBuffer = new BitmapBuffer;
    const ULONG     nDstScanlineFormat = BMP_SCANLINE_FORMAT( nDstBitmapFormat );
    const long      nSrcX = rTwoRect.mnSrcX, nSrcY = rTwoRect.mnSrcY;
    const long      nSrcDX = rTwoRect.mnSrcWidth, nSrcDY = rTwoRect.mnSrcHeight;
    const long      nDstDX = rTwoRect.mnDestWidth, nDstDY = rTwoRect.mnDestHeight;
    Scanline*       pSrcScan = new Scanline[ rSrcBuffer.mnHeight ];
    Scanline*       pDstScan = new Scanline[ nDstDY ];
    long*           pMapX = new long[ nDstDX ];
    long*           pMapY = new long[ nDstDY ];
    Scanline        pTmpScan;
    long            i, nTmp, nOffset;

    // set function for getting pixels
    switch( BMP_SCANLINE_FORMAT( rSrcBuffer.mnFormat ) )
    {
        IMPL_CASE_GET_FORMAT( _1BIT_MSB_PAL );
        IMPL_CASE_GET_FORMAT( _1BIT_LSB_PAL );
        IMPL_CASE_GET_FORMAT( _4BIT_MSN_PAL );
        IMPL_CASE_GET_FORMAT( _4BIT_LSN_PAL );
        IMPL_CASE_GET_FORMAT( _8BIT_PAL );
        IMPL_CASE_GET_FORMAT( _8BIT_TC_MASK );
        IMPL_CASE_GET_FORMAT( _16BIT_TC_MASK );
        IMPL_CASE_GET_FORMAT( _24BIT_TC_BGR );
        IMPL_CASE_GET_FORMAT( _24BIT_TC_RGB );
        IMPL_CASE_GET_FORMAT( _24BIT_TC_MASK );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_ABGR );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_ARGB );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_BGRA );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_RGBA );
        IMPL_CASE_GET_FORMAT( _32BIT_TC_MASK );

        default:
            DBG_ERROR( "unknown read format" );
        break;
    }

    // set function for setting pixels
    switch( nDstScanlineFormat )
    {
        IMPL_CASE_SET_FORMAT( _1BIT_MSB_PAL, 1 );
        IMPL_CASE_SET_FORMAT( _1BIT_LSB_PAL, 1 );
        IMPL_CASE_SET_FORMAT( _4BIT_MSN_PAL, 1 );
        IMPL_CASE_SET_FORMAT( _4BIT_LSN_PAL, 4 );
        IMPL_CASE_SET_FORMAT( _8BIT_PAL, 8 );
        IMPL_CASE_SET_FORMAT( _8BIT_TC_MASK, 8 );
        IMPL_CASE_SET_FORMAT( _16BIT_TC_MASK, 16 );
        IMPL_CASE_SET_FORMAT( _24BIT_TC_BGR, 24 );
        IMPL_CASE_SET_FORMAT( _24BIT_TC_RGB, 24 );
        IMPL_CASE_SET_FORMAT( _24BIT_TC_MASK, 24 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_ABGR, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_ARGB, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_BGRA, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_RGBA, 32 );
        IMPL_CASE_SET_FORMAT( _32BIT_TC_MASK, 32 );

        default:
            DBG_ERROR( "unknown write format" );
        break;
    }

    // fill destination buffer
    pDstBuffer->mnFormat = nDstBitmapFormat;
    pDstBuffer->mnWidth = nDstDX;
    pDstBuffer->mnHeight = nDstDY;
    pDstBuffer->mnScanlineSize = AlignedWidth4Bytes( pDstBuffer->mnBitCount * nDstDX );
    pDstBuffer->mpBits = new BYTE[ pDstBuffer->mnScanlineSize * nDstDY ];
    rtl_zeroMemory( pDstBuffer->mpBits, pDstBuffer->mnScanlineSize * nDstDY );

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
             ( nDstScanlineFormat == BMP_FORMAT_16BIT_TC_MASK ) ||
             ( nDstScanlineFormat == BMP_FORMAT_24BIT_TC_MASK ) ||
             ( nDstScanlineFormat == BMP_FORMAT_32BIT_TC_MASK ) )
    {
        DBG_ASSERT( pDstMask, "destination buffer requires color mask" );
        pDstBuffer->maColorMask = *pDstMask;
    }

    // horizontal mapping table
    if( nDstDX != nSrcDX )
    {
        const double fFactorX = ( nDstDX > 1 ) ? (double) ( nSrcDX - 1 ) / ( nDstDX - 1 ) : 0.0;

        for( i = 0L; i < nDstDX; i++ )
            pMapX[ i ] = nSrcX + FRound( i * fFactorX );
    }
    else
    {
        for( i = 0L, nTmp = nSrcX; i < nDstDX; i++ )
            pMapX[ i ] = nTmp++;
    }

    // vertical mapping table
    if( nDstDY != nSrcDY )
    {
        const double fFactorY = ( nDstDY > 1 ) ? (double) ( nSrcDY - 1 ) / ( nDstDY - 1 ) : 0.0;

        for( i = 0L; i < nDstDY; i++ )
            pMapY[ i ] = nSrcY + FRound( i * fFactorY );
    }
    else
    {
        for( i = 0L, nTmp = nSrcY; i < nDstDY; i++ )
            pMapY[ i ] = nTmp++;
    }

    // source scanline buffer
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
