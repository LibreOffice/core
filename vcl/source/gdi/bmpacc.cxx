/*************************************************************************
 *
 *  $RCSfile: bmpacc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#define _SV_BMPACC_CXX

#ifdef W31
#include <tools/svwin.h>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_IMPBMP_HXX
#include <impbmp.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#include <string.h>

// --------------------
// - BitmapReadAccess -
// --------------------

BitmapReadAccess::BitmapReadAccess( Bitmap& rBitmap, BOOL bModify ) :
            mpBuffer        ( NULL ),
            mpScanBuf       ( NULL ),
            mbModify        ( bModify ),
            mFncGetPixel    ( NULL ),
            mFncSetPixel    ( NULL )
{
    ImplCreate( rBitmap );
}

// ------------------------------------------------------------------

BitmapReadAccess::BitmapReadAccess( Bitmap& rBitmap ) :
            mpBuffer        ( NULL ),
            mpScanBuf       ( NULL ),
            mbModify        ( FALSE ),
            mFncGetPixel    ( NULL ),
            mFncSetPixel    ( NULL )
{
    ImplCreate( rBitmap );
}

// ------------------------------------------------------------------

BitmapReadAccess::~BitmapReadAccess()
{
    ImplDestroy();
}

// ------------------------------------------------------------------

void BitmapReadAccess::ImplCreate( Bitmap& rBitmap )
{
    ImpBitmap* pImpBmp = rBitmap.ImplGetImpBitmap();

    BMP_ASSERT( pImpBmp, "Forbidden Access to empty bitmap!" );

    if( pImpBmp )
    {
        if( mbModify && !maBitmap.ImplGetImpBitmap() )
        {
            rBitmap.ImplMakeUnique();
            pImpBmp = rBitmap.ImplGetImpBitmap();
        }
        else
        {
            BMP_ASSERT( !mbModify || pImpBmp->ImplGetRefCount() == 2,
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

// ------------------------------------------------------------------

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

// ------------------------------------------------------------------

BOOL BitmapReadAccess::ImplSetAccessPointers( ULONG nFormat )
{
    BOOL bRet = TRUE;

    switch( nFormat )
    {
        CASE_FORMAT( _1BIT_MSB_PAL )
        CASE_FORMAT( _1BIT_LSB_PAL )
        CASE_FORMAT( _4BIT_MSN_PAL )
        CASE_FORMAT( _4BIT_LSN_PAL )
        CASE_FORMAT( _8BIT_PAL )
        CASE_FORMAT( _8BIT_TC_MASK )
        CASE_FORMAT( _16BIT_TC_MASK )
        CASE_FORMAT( _24BIT_TC_BGR )
        CASE_FORMAT( _24BIT_TC_RGB )
        CASE_FORMAT( _24BIT_TC_MASK )
        CASE_FORMAT( _32BIT_TC_ABGR )
        CASE_FORMAT( _32BIT_TC_ARGB )
        CASE_FORMAT( _32BIT_TC_BGRA )
        CASE_FORMAT( _32BIT_TC_RGBA )
        CASE_FORMAT( _32BIT_TC_MASK )

        default:
            bRet = FALSE;
        break;
    }

    return bRet;
}

// ------------------------------------------------------------------

void BitmapReadAccess::ImplZeroInitUnusedBits()
{
    const sal_uInt32 nWidth = Width(), nHeight = Height(), nScanSize = GetScanlineSize();

    if( nWidth && nHeight && nScanSize && GetBuffer() )
    {
        DBG_ASSERT( !( nScanSize % 4 ), "BitmapWriteAccess::ZeroInitUnusedBits: Unsupported scanline alignment" );

        sal_uInt32 nBits;

        switch( GetScanlineFormat() )
        {
            case( BMP_FORMAT_1BIT_MSB_PAL ):
                nBits = 1;
            break;

            case( BMP_FORMAT_4BIT_MSN_PAL ):
                nBits = 4;
            break;

            case( BMP_FORMAT_8BIT_PAL ):
            case( BMP_FORMAT_8BIT_TC_MASK ):
                nBits = 8;
            break;

            case( BMP_FORMAT_16BIT_TC_MASK ):
                nBits = 16;
            break;

            case( BMP_FORMAT_24BIT_TC_BGR ):
            case( BMP_FORMAT_24BIT_TC_RGB ):
            case( BMP_FORMAT_24BIT_TC_MASK ):
                nBits = 24;
            break;

            case( BMP_FORMAT_32BIT_TC_ABGR ):
            case( BMP_FORMAT_32BIT_TC_ARGB ):
            case( BMP_FORMAT_32BIT_TC_BGRA ):
            case( BMP_FORMAT_32BIT_TC_RGBA ):
            case( BMP_FORMAT_32BIT_TC_MASK ):
                nBits = 32;
            break;

            default:
            {
                DBG_ERROR( "BitmapWriteAccess::ZeroInitUnusedBits: Unsupported pixel format" );
                nBits = 0;
            }
            break;
        }

        if( ( nBits *= nWidth ) & 0x1f )
        {
            sal_uInt32  nMask = 0xffffffff << ( ( nScanSize << 3 ) - nBits );
            BYTE*       pLast4Bytes = (BYTE*) GetBuffer() + ( nScanSize - 4 );

#ifdef __LITTLEENDIAN
            nMask = SWAPLONG( nMask );
#endif
            for( long i = 0; i < nHeight; i++, pLast4Bytes += nScanSize )
                ( *(long*) pLast4Bytes ) &= nMask;
        }
    }
}

// ------------------------------------------------------------------

void BitmapReadAccess::Flush()
{
    ImplDestroy();
}

// ------------------------------------------------------------------

void BitmapReadAccess::ReAccess( BOOL bModify )
{
    const ImpBitmap* pImpBmp = maBitmap.ImplGetImpBitmap();

    BMP_ASSERT( !mpBuffer, "No ReAccess possible while bitmap is being accessed!" );
    BMP_ASSERT( pImpBmp && ( pImpBmp->ImplGetRefCount() > 1UL ), "Accessed bitmap does not exist anymore!" );

    if( !mpBuffer && pImpBmp && ( pImpBmp->ImplGetRefCount() > 1UL ) )
    {
        mbModify = bModify;
        ImplCreate( maBitmap );
    }
}

// ------------------------------------------------------------------

USHORT BitmapReadAccess::GetBestPaletteIndex( const BitmapColor& rBitmapColor ) const
{
    return( HasPalette() ? mpBuffer->maPalette.GetBestIndex( rBitmapColor ) : 0 );
}

// ---------------------
// - BitmapWriteAccess -
// ---------------------

BitmapWriteAccess::~BitmapWriteAccess()
{
}

// ------------------------------------------------------------------

void BitmapWriteAccess::ImplInitDraw()
{
    if( HasPalette() )
    {
        if( !maLineColor.IsIndex() )
            maLineColor = (BYTE) GetBestPaletteIndex( maLineColor );

        if( !maFillColor.IsIndex() )
            maFillColor = (BYTE) GetBestPaletteIndex( maFillColor );
    }
}

// ------------------------------------------------------------------

void BitmapWriteAccess::CopyScanline( long nY, const BitmapReadAccess& rReadAcc )
{
    BMP_ASSERT( ( nY >= 0 ) && ( nY < mpBuffer->mnHeight ), "y-coordinate in destination out of range!" );
    BMP_ASSERT( nY < rReadAcc.Height(), "y-coordinate in source out of range!" );
    BMP_ASSERT( ( HasPalette() && rReadAcc.HasPalette() ) || ( !HasPalette() && !rReadAcc.HasPalette() ), "No copying possible between palette bitmap and TC bitmap!" );

    if( ( GetScanlineFormat() == rReadAcc.GetScanlineFormat() ) &&
        ( GetScanlineSize() >= rReadAcc.GetScanlineSize() ) )
    {
        HMEMCPY( mpScanBuf[ nY ], rReadAcc.GetScanline( nY ), rReadAcc.GetScanlineSize() );
    }
    else
        for( long nX = 0L, nWidth = Min( mpBuffer->mnWidth, rReadAcc.Width() ); nX < nWidth; nX++ )
            SetPixel( nY, nX, rReadAcc.GetPixel( nY, nX ) );
}

// ------------------------------------------------------------------

void BitmapWriteAccess::CopyScanline( long nY, const Scanline aSrcScanline,
                                      ULONG nSrcScanlineFormat, ULONG nSrcScanlineSize )
{
    const ULONG nFormat = BMP_SCANLINE_FORMAT( nSrcScanlineFormat );

    BMP_ASSERT( ( nY >= 0 ) && ( nY < mpBuffer->mnHeight ), "y-coordinate in destination out of range!" );
    BMP_ASSERT( ( HasPalette() && nFormat <= BMP_FORMAT_8BIT_PAL ) ||
                ( !HasPalette() && nFormat > BMP_FORMAT_8BIT_PAL ),
                "No copying possible between palette and non palette scanlines!" );

    const ULONG nCount = Min( GetScanlineSize(), nSrcScanlineSize );

    if( nCount )
    {
        if( GetScanlineFormat() == BMP_SCANLINE_FORMAT( nSrcScanlineFormat ) )
            HMEMCPY( mpScanBuf[ nY ], aSrcScanline, nCount );
        else
        {
            BMP_ASSERT( nFormat != BMP_FORMAT_8BIT_TC_MASK && nFormat != BMP_FORMAT_16BIT_TC_MASK &&
                        nFormat != BMP_FORMAT_24BIT_TC_MASK && nFormat != BMP_FORMAT_32BIT_TC_MASK,
                        "No support for pixel formats with color masks yet!" );

            FncGetPixel pFncGetPixel;

            switch( nFormat )
            {
                case( BMP_FORMAT_1BIT_MSB_PAL ):    pFncGetPixel = GetPixelFor_1BIT_MSB_PAL; break;
                case( BMP_FORMAT_1BIT_LSB_PAL ):    pFncGetPixel = GetPixelFor_1BIT_LSB_PAL; break;
                case( BMP_FORMAT_4BIT_MSN_PAL ):    pFncGetPixel = GetPixelFor_4BIT_MSN_PAL; break;
                case( BMP_FORMAT_4BIT_LSN_PAL ):    pFncGetPixel = GetPixelFor_4BIT_LSN_PAL; break;
                case( BMP_FORMAT_8BIT_PAL ):        pFncGetPixel = GetPixelFor_8BIT_PAL; break;
                case( BMP_FORMAT_8BIT_TC_MASK ):    pFncGetPixel = GetPixelFor_8BIT_TC_MASK; break;
                case( BMP_FORMAT_16BIT_TC_MASK ):   pFncGetPixel = GetPixelFor_16BIT_TC_MASK; break;
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


// ------------------------------------------------------------------

void BitmapWriteAccess::CopyBuffer( const BitmapReadAccess& rReadAcc )
{
    BMP_ASSERT( ( HasPalette() && rReadAcc.HasPalette() ) || ( !HasPalette() && !rReadAcc.HasPalette() ), "No copying possible between palette bitmap and TC bitmap!" );

    if( ( GetScanlineFormat() == rReadAcc.GetScanlineFormat() ) &&
        ( GetScanlineSize() == rReadAcc.GetScanlineSize() ) )
    {
        const long  nHeight = Min( mpBuffer->mnHeight, rReadAcc.Height() );
        const ULONG nCount = nHeight * mpBuffer->mnScanlineSize;

        HMEMCPY( mpBuffer->mpBits, rReadAcc.GetBuffer(), nCount );
    }
    else
        for( long nY = 0L, nHeight = Min( mpBuffer->mnHeight, rReadAcc.Height() ); nY < nHeight; nY++ )
            CopyScanline( nY, rReadAcc );
}
