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


#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef FREEBSD
#include <sys/types.h>
#endif

#include <osl/endian.h>

#include <vcl/bitmap.hxx>
#include <vcl/salbtype.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <tools/prex.h>
#include "unx/Xproto.h"
#include <tools/postx.h>
#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salbmp.h>
#include <unx/salinst.h>
#include <unx/x11/xlimits.hxx>

#if defined(HAVE_MEMCHECK_H)
#include <memcheck.h>
#endif

// -------------
// - SalBitmap -
// -------------

SalBitmap* X11SalInstance::CreateSalBitmap()
{
    return new X11SalBitmap();
}

ImplSalBitmapCache* X11SalBitmap::mpCache = NULL;
sal_uLong           X11SalBitmap::mnCacheInstCount = 0;

// -----------------------------------------------------------------------------

X11SalBitmap::X11SalBitmap()
    : mpDIB( NULL )
    , mpDDB( NULL )
    , mbGrey( false )
{
}

// -----------------------------------------------------------------------------

X11SalBitmap::~X11SalBitmap()
{
    Destroy();
}

// -----------------------------------------------------------------------------

void X11SalBitmap::ImplCreateCache()
{
    if( !mnCacheInstCount++ )
        mpCache = new ImplSalBitmapCache;
}

// -----------------------------------------------------------------------------

void X11SalBitmap::ImplDestroyCache()
{
    DBG_ASSERT( mnCacheInstCount, "X11SalBitmap::ImplDestroyCache(): underflow" );

    if( mnCacheInstCount && !--mnCacheInstCount )
        delete mpCache, mpCache = NULL;
}

// -----------------------------------------------------------------------------

void X11SalBitmap::ImplRemovedFromCache()
{
    if( mpDDB )
        delete mpDDB, mpDDB = NULL;
}

#if defined(HAVE_MEMCHECK_H)
namespace
{
    void blankExtraSpace(BitmapBuffer* pDIB)
    {
        size_t nExtraSpaceInScanLine = pDIB->mnScanlineSize - pDIB->mnWidth * pDIB->mnBitCount / 8;
        if (nExtraSpaceInScanLine)
        {
            for (long i = 0; i < pDIB->mnHeight; ++i)
            {
                sal_uInt8 *pRow = pDIB->mpBits + (i * pDIB->mnScanlineSize);
                memset(pRow + (pDIB->mnScanlineSize - nExtraSpaceInScanLine), 0, nExtraSpaceInScanLine);
            }
        }
    }
}
#endif

// -----------------------------------------------------------------------------

BitmapBuffer* X11SalBitmap::ImplCreateDIB(
    const Size& rSize,
    sal_uInt16 nBitCount,
    const BitmapPalette& rPal
) {
    DBG_ASSERT(
           nBitCount ==  1
        || nBitCount ==  4
        || nBitCount ==  8
        || nBitCount == 16
        || nBitCount == 24
        , "Unsupported BitCount!"
    );

    BitmapBuffer* pDIB = NULL;

    if( rSize.Width() && rSize.Height() )
    {
        try
        {
            pDIB = new BitmapBuffer;
        }
        catch (const std::bad_alloc&)
        {
            pDIB = NULL;
        }

        if( pDIB )
        {
            const sal_uInt16 nColors = ( nBitCount <= 8 ) ? ( 1 << nBitCount ) : 0;

            pDIB->mnFormat = BMP_FORMAT_BOTTOM_UP;

            switch( nBitCount )
            {
                case( 1 ): pDIB->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL; break;
                case( 4 ): pDIB->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL; break;
                case( 8 ): pDIB->mnFormat |= BMP_FORMAT_8BIT_PAL; break;
#ifdef OSL_BIGENDIAN
                case(16 ):
                    pDIB->mnFormat|= BMP_FORMAT_16BIT_TC_MSB_MASK;
                    pDIB->maColorMask = ColorMask( 0xf800, 0x07e0, 0x001f );
                    break;
#else
                case(16 ):
                    pDIB->mnFormat|= BMP_FORMAT_16BIT_TC_LSB_MASK;
                    pDIB->maColorMask = ColorMask( 0xf800, 0x07e0, 0x001f );
                    break;
#endif
                default:
                    nBitCount = 24;
                    //fall through
                case 24:
                    pDIB->mnFormat |= BMP_FORMAT_24BIT_TC_BGR;
                break;
            }

            pDIB->mnWidth = rSize.Width();
            pDIB->mnHeight = rSize.Height();
            pDIB->mnScanlineSize = AlignedWidth4Bytes( pDIB->mnWidth * nBitCount );
            pDIB->mnBitCount = nBitCount;

            if( nColors )
            {
                pDIB->maPalette = rPal;
                pDIB->maPalette.SetEntryCount( nColors );
            }

            try
            {
                pDIB->mpBits = new sal_uInt8[ pDIB->mnScanlineSize * pDIB->mnHeight ];
#if defined(HAVE_MEMCHECK_H)
                if (RUNNING_ON_VALGRIND)
                    blankExtraSpace(pDIB);
#endif
            }
            catch (const std::bad_alloc&)
            {
                delete pDIB;
                pDIB = NULL;
            }
        }
    }
    else
        pDIB = NULL;

    return pDIB;
}

// -----------------------------------------------------------------------------

BitmapBuffer* X11SalBitmap::ImplCreateDIB(
    Drawable aDrawable,
    SalX11Screen nScreen,
    long nDrawableDepth,
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    bool bGrey
) {
    BitmapBuffer* pDIB = NULL;

    if( aDrawable && nWidth && nHeight && nDrawableDepth )
    {
        SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
        Display*    pXDisp = pSalDisp->GetDisplay();

        // do not die on XError here
        // alternatively one could check the coordinates for being offscreen
        // but this call can actually work on servers with backing store
        // defaults even if the rectangle is offscreen
        // so better catch the XError
        GetGenericData()->ErrorTrapPush();
        XImage* pImage = XGetImage( pXDisp, aDrawable, nX, nY, nWidth, nHeight, AllPlanes, ZPixmap );
        bool bWasError = GetGenericData()->ErrorTrapPop( false );

        if( ! bWasError && pImage && pImage->data )
        {
            const SalTwoRect        aTwoRect = { 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight };
            BitmapBuffer            aSrcBuf;
            sal_uLong               nDstFormat = BMP_FORMAT_BOTTOM_UP;
            const BitmapPalette*    pDstPal = NULL;

            aSrcBuf.mnFormat = BMP_FORMAT_TOP_DOWN;
            aSrcBuf.mnWidth = nWidth;
            aSrcBuf.mnHeight = nHeight;
            aSrcBuf.mnBitCount = pImage->bits_per_pixel;
            aSrcBuf.mnScanlineSize = pImage->bytes_per_line;
            aSrcBuf.mpBits = (sal_uInt8*) pImage->data;

            pImage->red_mask = pSalDisp->GetVisual( nScreen ).red_mask;
            pImage->green_mask = pSalDisp->GetVisual( nScreen ).green_mask;
            pImage->blue_mask = pSalDisp->GetVisual( nScreen ).blue_mask;

            switch( aSrcBuf.mnBitCount )
            {
                case( 1 ):
                {
                    aSrcBuf.mnFormat |= ( LSBFirst == pImage->bitmap_bit_order
                                            ? BMP_FORMAT_1BIT_LSB_PAL
                                            : BMP_FORMAT_1BIT_MSB_PAL
                                        );
                    nDstFormat |= BMP_FORMAT_1BIT_MSB_PAL;
                }
                break;

                case( 4 ):
                {
                    aSrcBuf.mnFormat |= ( LSBFirst == pImage->bitmap_bit_order
                                            ? BMP_FORMAT_4BIT_LSN_PAL
                                            : BMP_FORMAT_4BIT_MSN_PAL
                                        );
                    nDstFormat |= BMP_FORMAT_4BIT_MSN_PAL;
                }
                break;

                case( 8 ):
                {
                    aSrcBuf.mnFormat |= BMP_FORMAT_8BIT_PAL;
                    nDstFormat |= BMP_FORMAT_8BIT_PAL;
                }
                break;

                case( 16 ):
                {
                    nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                    aSrcBuf.maColorMask = ColorMask( pImage->red_mask, pImage->green_mask, pImage->blue_mask );

                    if( LSBFirst == pImage->byte_order )
                    {
                        aSrcBuf.mnFormat |= BMP_FORMAT_16BIT_TC_LSB_MASK;
                    }
                    else
                    {
                        aSrcBuf.mnFormat |= BMP_FORMAT_16BIT_TC_MSB_MASK;
                    }
                }
                break;

                case( 24 ):
                {
                    if( ( LSBFirst == pImage->byte_order ) && ( pImage->red_mask == 0xFF ) )
                        aSrcBuf.mnFormat |= BMP_FORMAT_24BIT_TC_RGB;
                    else
                        aSrcBuf.mnFormat |= BMP_FORMAT_24BIT_TC_BGR;

                    nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                }
                break;

                case( 32 ):
                {
                    if( LSBFirst == pImage->byte_order )
                        aSrcBuf.mnFormat |= (  pSalDisp->GetVisual(nScreen).red_mask == 0xFF
                                                ? BMP_FORMAT_32BIT_TC_RGBA
                                                : BMP_FORMAT_32BIT_TC_BGRA
                                            );
                    else
                        aSrcBuf.mnFormat |= (  pSalDisp->GetVisual(nScreen).red_mask == 0xFF
                                                ? BMP_FORMAT_32BIT_TC_ABGR
                                                : BMP_FORMAT_32BIT_TC_ARGB
                                            );
                    nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                }
                break;
            }

            BitmapPalette& rPal = aSrcBuf.maPalette;

            if( aSrcBuf.mnBitCount == 1 )
            {
                rPal.SetEntryCount( 2 );
                pDstPal = &rPal;

                rPal[ 0 ] = Color( COL_BLACK );
                rPal[ 1 ] = Color( COL_WHITE );
            }
            else if( pImage->depth == 8 && bGrey )
            {
                rPal.SetEntryCount( 256 );
                pDstPal = &rPal;

                for( sal_uInt16 i = 0; i < 256; i++ )
                {
                    BitmapColor&    rBmpCol = rPal[ i ];

                    rBmpCol.SetRed( i );
                    rBmpCol.SetGreen( i );
                    rBmpCol.SetBlue( i );
                }

            }
            else if( aSrcBuf.mnBitCount <= 8 )
            {
                const SalColormap& rColMap = pSalDisp->GetColormap( nScreen );
                const sal_uInt16 nCols = Min( (sal_uLong)rColMap.GetUsed()
                                            , (sal_uLong)(1 << nDrawableDepth)
                                            );

                rPal.SetEntryCount( nCols );
                pDstPal = &rPal;

                for( sal_uInt16 i = 0; i < nCols; i++ )
                {
                    const SalColor  nColor( rColMap.GetColor( i ) );
                    BitmapColor&    rBmpCol = rPal[ i ];

                    rBmpCol.SetRed( SALCOLOR_RED( nColor ) );
                    rBmpCol.SetGreen( SALCOLOR_GREEN( nColor ) );
                    rBmpCol.SetBlue( SALCOLOR_BLUE( nColor ) );
                }
            }

            nDstFormat = aSrcBuf.mnFormat;
            pDIB = StretchAndConvert( aSrcBuf, aTwoRect, nDstFormat,
                const_cast<BitmapPalette*>(pDstPal), &aSrcBuf.maColorMask );
            XDestroyImage( pImage );
        }
    }

    return pDIB;
}

// -----------------------------------------------------------------------------

XImage* X11SalBitmap::ImplCreateXImage(
    SalDisplay *pSalDisp,
    SalX11Screen nScreen,
    long nDepth,
    const SalTwoRect& rTwoRect
) const
{
    XImage* pImage = NULL;

    if( !mpDIB && mpDDB )
    {
        const_cast<X11SalBitmap*>(this)->mpDIB =
            ImplCreateDIB( mpDDB->ImplGetPixmap(),
                           mpDDB->ImplGetScreen(),
                           mpDDB->ImplGetDepth(),
                           0, 0,
                           mpDDB->ImplGetWidth(),
                           mpDDB->ImplGetHeight(),
                           mbGrey );
    }

    if( mpDIB && mpDIB->mnWidth && mpDIB->mnHeight )
    {
        Display*    pXDisp = pSalDisp->GetDisplay();
        long        nWidth = rTwoRect.mnDestWidth;
        long        nHeight = rTwoRect.mnDestHeight;

        if( 1 == GetBitCount() )
            nDepth = 1;

        pImage = XCreateImage( pXDisp, pSalDisp->GetVisual( nScreen ).GetVisual(),
                               nDepth, ( 1 == nDepth ) ? XYBitmap :ZPixmap, 0, NULL,
                               nWidth, nHeight, 32, 0 );

        if( pImage )
        {
            BitmapBuffer*   pDstBuf;
            sal_uLong           nDstFormat = BMP_FORMAT_TOP_DOWN;
            BitmapPalette*  pPal = NULL;
            ColorMask*      pMask = NULL;

            switch( pImage->bits_per_pixel )
            {
                case( 1 ):
                    nDstFormat |=   ( LSBFirst == pImage->bitmap_bit_order
                                        ? BMP_FORMAT_1BIT_LSB_PAL
                                        : BMP_FORMAT_1BIT_MSB_PAL
                                    );
                break;

                case( 4 ):
                    nDstFormat |=   ( LSBFirst == pImage->bitmap_bit_order
                                        ? BMP_FORMAT_4BIT_LSN_PAL
                                        : BMP_FORMAT_4BIT_MSN_PAL
                                    );
                break;

                case( 8 ):
                    nDstFormat |= BMP_FORMAT_8BIT_PAL;
                break;

                case( 16 ):
                {
                    #ifdef OSL_BIGENDIAN

                    if( MSBFirst == pImage->byte_order )
                        nDstFormat |= BMP_FORMAT_16BIT_TC_MSB_MASK;
                    else
                        nDstFormat |= BMP_FORMAT_16BIT_TC_LSB_MASK;

                    #else /* OSL_LITENDIAN */

                    nDstFormat |= BMP_FORMAT_16BIT_TC_LSB_MASK;
                    if( MSBFirst == pImage->byte_order )
                        pImage->byte_order = LSBFirst;

                    #endif

                    pMask = new ColorMask( pImage->red_mask, pImage->green_mask, pImage->blue_mask );
                }
                break;

                case( 24 ):
                {
                    if( ( LSBFirst == pImage->byte_order ) && ( pImage->red_mask == 0xFF ) )
                        nDstFormat |= BMP_FORMAT_24BIT_TC_RGB;
                    else
                        nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                }
                break;

                case( 32 ):
                {
                    if( LSBFirst == pImage->byte_order )
                        nDstFormat |=   ( pImage->red_mask == 0xFF
                                            ? BMP_FORMAT_32BIT_TC_RGBA
                                            : BMP_FORMAT_32BIT_TC_BGRA
                                        );
                    else
                        nDstFormat |=   ( pImage->red_mask == 0xFF
                                            ? BMP_FORMAT_32BIT_TC_ABGR
                                            : BMP_FORMAT_32BIT_TC_ARGB
                                        );
                }
                break;
            }

            if( pImage->depth == 1 )
            {
                pPal = new BitmapPalette( 2 );
                (*pPal)[ 0 ] = Color( COL_BLACK );
                (*pPal)[ 1 ] = Color( COL_WHITE );
            }
            else if( pImage->depth == 8 && mbGrey )
            {
                pPal = new BitmapPalette( 256 );

                for( sal_uInt16 i = 0; i < 256; i++ )
                {
                    BitmapColor&    rBmpCol = (*pPal)[ i ];

                    rBmpCol.SetRed( i );
                    rBmpCol.SetGreen( i );
                    rBmpCol.SetBlue( i );
                }

            }
            else if( pImage->depth <= 8 )
            {
                const SalColormap& rColMap = pSalDisp->GetColormap( nScreen );
                const sal_uInt16 nCols = Min( (sal_uLong)rColMap.GetUsed()
                                            , (sal_uLong)(1 << pImage->depth)
                                            );

                pPal = new BitmapPalette( nCols );

                for( sal_uInt16 i = 0; i < nCols; i++ )
                {
                    const SalColor  nColor( rColMap.GetColor( i ) );
                    BitmapColor&    rBmpCol = (*pPal)[ i ];

                    rBmpCol.SetRed( SALCOLOR_RED( nColor ) );
                    rBmpCol.SetGreen( SALCOLOR_GREEN( nColor ) );
                    rBmpCol.SetBlue( SALCOLOR_BLUE( nColor ) );
                }
            }

            pDstBuf = StretchAndConvert( *mpDIB, rTwoRect, nDstFormat, pPal, pMask );
            delete pPal;
            delete pMask;

            if( pDstBuf && pDstBuf->mpBits )
            {
#if defined(HAVE_MEMCHECK_H)
                if (RUNNING_ON_VALGRIND)
                    blankExtraSpace(pDstBuf);
#endif
                // set data in buffer as data member in pImage
                pImage->data = (char*) pDstBuf->mpBits;

                // destroy buffer; don't destroy allocated data in buffer
                delete pDstBuf;
            }
            else
            {
                XDestroyImage( pImage );
                pImage = NULL;
            }
        }
    }

    return pImage;
}

// -----------------------------------------------------------------------------
bool X11SalBitmap::ImplCreateFromDrawable(
    Drawable aDrawable,
    SalX11Screen nScreen,
    long nDrawableDepth,
    long nX,
    long nY,
    long nWidth,
    long nHeight
) {
    Destroy();

    if( aDrawable && nWidth && nHeight && nDrawableDepth )
        mpDDB = new ImplSalDDB( aDrawable, nScreen, nDrawableDepth, nX, nY, nWidth, nHeight );

    return( mpDDB != NULL );
}
// -----------------------------------------------------------------------------

ImplSalDDB* X11SalBitmap::ImplGetDDB(
    Drawable          aDrawable,
    SalX11Screen      nXScreen,
    long              nDrawableDepth,
    const SalTwoRect& rTwoRect
) const
{
    if( !mpDDB || !mpDDB->ImplMatches( nXScreen, nDrawableDepth, rTwoRect ) )
    {
        if( mpDDB )
        {
            // do we already have a DIB? if not, create aDIB from current DDB first
            if( !mpDIB )
            {
                const_cast<X11SalBitmap*>(this)->mpDIB = ImplCreateDIB( mpDDB->ImplGetPixmap(),
                                                                        mpDDB->ImplGetScreen(),
                                                                        mpDDB->ImplGetDepth(),
                                                                        0, 0,
                                                                        mpDDB->ImplGetWidth(),
                                                                        mpDDB->ImplGetHeight(),
                                                                        mbGrey );
            }

            delete mpDDB, const_cast<X11SalBitmap*>(this)->mpDDB = NULL;
        }

        if( mpCache )
            mpCache->ImplRemove( const_cast<X11SalBitmap*>(this) );

        SalTwoRect aTwoRect( rTwoRect );
        if( aTwoRect.mnSrcX < 0 )
        {
            aTwoRect.mnSrcWidth += aTwoRect.mnSrcX;
            aTwoRect.mnSrcX = 0;
        }
        if( aTwoRect.mnSrcY < 0 )
        {
            aTwoRect.mnSrcHeight += aTwoRect.mnSrcY;
            aTwoRect.mnSrcY = 0;
        }

        // create new DDB from DIB
        const Size aSize( GetSize() );
        if( aTwoRect.mnSrcWidth == aTwoRect.mnDestWidth &&
            aTwoRect.mnSrcHeight == aTwoRect.mnDestHeight )
        {
            aTwoRect.mnSrcX = aTwoRect.mnSrcY = aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
            aTwoRect.mnSrcWidth = aTwoRect.mnDestWidth = aSize.Width();
            aTwoRect.mnSrcHeight = aTwoRect.mnDestHeight = aSize.Height();
        }
        else if( aTwoRect.mnSrcWidth+aTwoRect.mnSrcX > aSize.Width() ||
                 aTwoRect.mnSrcHeight+aTwoRect.mnSrcY > aSize.Height() )
        {
            // #i47823# this should not happen at all, but does nonetheless
            // because BitmapEx allows for mask bitmaps of different size
            // than image bitmap (broken)
            if( aTwoRect.mnSrcX >= aSize.Width() ||
                aTwoRect.mnSrcY >= aSize.Height() )
                return NULL; // this would be a really mad case

            if( aTwoRect.mnSrcWidth+aTwoRect.mnSrcX > aSize.Width() )
            {
                aTwoRect.mnSrcWidth = aSize.Width()-aTwoRect.mnSrcX;
                if( aTwoRect.mnSrcWidth < 1 )
                {
                    aTwoRect.mnSrcX = 0;
                    aTwoRect.mnSrcWidth = aSize.Width();
                }
            }
            if( aTwoRect.mnSrcHeight+aTwoRect.mnSrcY > aSize.Height() )
            {
                aTwoRect.mnSrcHeight = aSize.Height() - aTwoRect.mnSrcY;
                if( aTwoRect.mnSrcHeight < 1 )
                {
                    aTwoRect.mnSrcY = 0;
                    aTwoRect.mnSrcHeight = aSize.Height();
                }
            }
        }

        XImage* pImage = ImplCreateXImage( GetGenericData()->GetSalDisplay(), nXScreen,
                                           nDrawableDepth, aTwoRect );

        if( pImage )
        {
            const_cast<X11SalBitmap*>(this)->mpDDB = new ImplSalDDB( pImage, aDrawable, nXScreen, aTwoRect );
            delete[] pImage->data, pImage->data = NULL;
            XDestroyImage( pImage );

            if( mpCache )
                mpCache->ImplAdd( const_cast<X11SalBitmap*>(this), mpDDB->ImplGetMemSize() );
        }
    }

    return mpDDB;
}

// -----------------------------------------------------------------------------

void X11SalBitmap::ImplDraw(
    Drawable           aDrawable,
    SalX11Screen       nXScreen,
    long               nDrawableDepth,
    const SalTwoRect&  rTwoRect,
    const GC&          rGC
) const
{
    ImplGetDDB( aDrawable, nXScreen, nDrawableDepth, rTwoRect );
    if( mpDDB )
        mpDDB->ImplDraw( aDrawable, nDrawableDepth, rTwoRect, rGC );
}

// -----------------------------------------------------------------------------

bool X11SalBitmap::Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal )
{
    Destroy();
    mpDIB = ImplCreateDIB( rSize, nBitCount, rPal );

    return( mpDIB != NULL );
}

// -----------------------------------------------------------------------------

bool X11SalBitmap::Create( const SalBitmap& rSSalBmp )
{
    Destroy();

    const X11SalBitmap& rSalBmp = static_cast<const X11SalBitmap&>( rSSalBmp );

    if( rSalBmp.mpDIB )
    {
        // TODO: reference counting...
        mpDIB = new BitmapBuffer( *rSalBmp.mpDIB );
        // TODO: get rid of this when BitmapBuffer gets copy constructor
        try
        {
            mpDIB->mpBits = new sal_uInt8[ mpDIB->mnScanlineSize * mpDIB->mnHeight ];
#if defined(HAVE_MEMCHECK_H)
            if (RUNNING_ON_VALGRIND)
                blankExtraSpace(mpDIB);
#endif
        }
        catch (const std::bad_alloc&)
        {
            delete mpDIB;
            mpDIB = NULL;
        }

        if( mpDIB )
            memcpy( mpDIB->mpBits, rSalBmp.mpDIB->mpBits, mpDIB->mnScanlineSize * mpDIB->mnHeight );
    }
    else if(  rSalBmp.mpDDB )
        ImplCreateFromDrawable( rSalBmp.mpDDB->ImplGetPixmap(),
                                rSalBmp.mpDDB->ImplGetScreen(),
                                rSalBmp.mpDDB->ImplGetDepth(),
                                0, 0, rSalBmp.mpDDB->ImplGetWidth(), rSalBmp.mpDDB->ImplGetHeight() );

    return( ( !rSalBmp.mpDIB && !rSalBmp.mpDDB ) ||
            ( rSalBmp.mpDIB && ( mpDIB != NULL ) ) ||
            ( rSalBmp.mpDDB && ( mpDDB != NULL ) ) );
}

// -----------------------------------------------------------------------------

bool X11SalBitmap::Create( const SalBitmap&, SalGraphics* )
{
    return false;
}

// -----------------------------------------------------------------------------

bool X11SalBitmap::Create( const SalBitmap&, sal_uInt16 )
{
    return false;
}

// -----------------------------------------------------------------------------

bool X11SalBitmap::Create(
    const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > xBitmapCanvas,
    Size& rSize,
    bool bMask
) {
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >
        xFastPropertySet( xBitmapCanvas, ::com::sun::star::uno::UNO_QUERY );

    if( xFastPropertySet.get() ) {
        sal_Int32 depth;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > args;

        if( xFastPropertySet->getFastPropertyValue(bMask ? 2 : 1) >>= args ) {
            long pixmapHandle;
            if( ( args[1] >>= pixmapHandle ) && ( args[2] >>= depth ) ) {

                mbGrey = bMask;
                bool bSuccess = ImplCreateFromDrawable(
                                    pixmapHandle,
                                    // FIXME: this seems multi-screen broken to me
                                    SalX11Screen( 0 ),
                                    depth,
                                    0,
                                    0,
                                    (long) rSize.Width(),
                                    (long) rSize.Height()
                                );
                bool bFreePixmap = false;
                if( bSuccess && (args[0] >>= bFreePixmap) && bFreePixmap )
                    XFreePixmap( GetGenericData()->GetSalDisplay()->GetDisplay(), pixmapHandle );

                return bSuccess;
            }
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

void X11SalBitmap::Destroy()
{
    if( mpDIB )
    {
        delete[] mpDIB->mpBits;
        delete mpDIB, mpDIB = NULL;
    }

    if( mpDDB )
        delete mpDDB, mpDDB = NULL;

    if( mpCache )
        mpCache->ImplRemove( this );
}

// -----------------------------------------------------------------------------

Size X11SalBitmap::GetSize() const
{
    Size aSize;

    if( mpDIB )
        aSize.Width() = mpDIB->mnWidth, aSize.Height() = mpDIB->mnHeight;
    else if( mpDDB )
        aSize.Width() = mpDDB->ImplGetWidth(), aSize.Height() = mpDDB->ImplGetHeight();

    return aSize;
}

// -----------------------------------------------------------------------------

sal_uInt16 X11SalBitmap::GetBitCount() const
{
    sal_uInt16 nBitCount;

    if( mpDIB )
        nBitCount = mpDIB->mnBitCount;
    else if( mpDDB )
        nBitCount = mpDDB->ImplGetDepth();
    else
        nBitCount = 0;

    return nBitCount;
}

// -----------------------------------------------------------------------------

BitmapBuffer* X11SalBitmap::AcquireBuffer( bool )
{
    if( !mpDIB && mpDDB )
    {
        mpDIB = ImplCreateDIB(
                    mpDDB->ImplGetPixmap(),
                    mpDDB->ImplGetScreen(),
                    mpDDB->ImplGetDepth(),
                    0, 0,
                    mpDDB->ImplGetWidth(),
                    mpDDB->ImplGetHeight(),
                    mbGrey
                );
    }

    return mpDIB;
}

// -----------------------------------------------------------------------------

void X11SalBitmap::ReleaseBuffer( BitmapBuffer*, bool bReadOnly )
{
    if( !bReadOnly )
    {
        if( mpDDB )
            delete mpDDB, mpDDB = NULL;

        if( mpCache )
            mpCache->ImplRemove( this );
    }
}

// -----------------------------------------------------------------------------

bool X11SalBitmap::GetSystemData( BitmapSystemData& rData )
{
    if( mpDDB )
    {
        // Rename/retype pDummy to your likings (though X11 Pixmap is
        // prolly not a good idea, since it's accessed from
        // non-platform aware code in vcl/bitmap.hxx)
        rData.aPixmap = (void*)mpDDB->ImplGetPixmap();
        rData.mnWidth = mpDDB->ImplGetWidth ();
        rData.mnHeight = mpDDB->ImplGetHeight ();
        return true;
    }

    return false;
}

// --------------
// - ImplSalDDB -
// --------------

ImplSalDDB::ImplSalDDB( XImage* pImage, Drawable aDrawable,
                        SalX11Screen nXScreen, const SalTwoRect& rTwoRect )
    : maPixmap    ( 0 )
    , maTwoRect   ( rTwoRect )
    , mnDepth     ( pImage->depth )
    , mnXScreen   ( nXScreen )
{
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    Display*    pXDisp = pSalDisp->GetDisplay();

    if( (maPixmap = limitXCreatePixmap( pXDisp, aDrawable, ImplGetWidth(), ImplGetHeight(), ImplGetDepth() )) )
    {
        XGCValues   aValues;
        GC          aGC;
        int         nValues = GCFunction;

        aValues.function = GXcopy;

        if( 1 == mnDepth )
        {
            nValues |= ( GCForeground | GCBackground );
            aValues.foreground = 1, aValues.background = 0;
        }

        aGC = XCreateGC( pXDisp, maPixmap, nValues, &aValues );
        XPutImage( pXDisp, maPixmap, aGC, pImage, 0, 0, 0, 0, maTwoRect.mnDestWidth, maTwoRect.mnDestHeight );
        XFreeGC( pXDisp, aGC );
    }
}

// -----------------------------------------------------------------------------

ImplSalDDB::ImplSalDDB(
    Drawable aDrawable,
    SalX11Screen nXScreen,
    long nDrawableDepth,
    long nX,
    long nY,
    long nWidth,
    long nHeight
)   : mnDepth( nDrawableDepth )
    , mnXScreen( nXScreen )
{
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    Display*    pXDisp = pSalDisp->GetDisplay();

    if( (maPixmap = limitXCreatePixmap( pXDisp, aDrawable, nWidth, nHeight, nDrawableDepth )) )
    {
        XGCValues   aValues;
        GC          aGC;
        int         nValues = GCFunction;

        aValues.function = GXcopy;

        if( 1 == mnDepth )
        {
            nValues |= ( GCForeground | GCBackground );
            aValues.foreground = 1, aValues.background = 0;
        }

        aGC = XCreateGC( pXDisp, maPixmap, nValues, &aValues );
        ImplDraw( aDrawable, nDrawableDepth, maPixmap, mnDepth,
                  nX, nY, nWidth, nHeight, 0, 0, aGC );
        XFreeGC( pXDisp, aGC );

        maTwoRect.mnSrcX = maTwoRect.mnSrcY = maTwoRect.mnDestX = maTwoRect.mnDestY = 0;
        maTwoRect.mnSrcWidth = maTwoRect.mnDestWidth = nWidth;
        maTwoRect.mnSrcHeight = maTwoRect.mnDestHeight = nHeight;
    }
}

// -----------------------------------------------------------------------------

ImplSalDDB::~ImplSalDDB()
{
    if( maPixmap && ImplGetSVData() )
        XFreePixmap( GetGenericData()->GetSalDisplay()->GetDisplay(), maPixmap );
}

// -----------------------------------------------------------------------------

bool ImplSalDDB::ImplMatches( SalX11Screen nXScreen, long nDepth, const SalTwoRect& rTwoRect ) const
{
    bool bRet = false;

    if( ( maPixmap != 0 ) && ( ( mnDepth == nDepth ) || ( 1 == mnDepth ) ) && nXScreen == mnXScreen)
    {
        if (  rTwoRect.mnSrcX       == maTwoRect.mnSrcX
           && rTwoRect.mnSrcY       == maTwoRect.mnSrcY
           && rTwoRect.mnSrcWidth   == maTwoRect.mnSrcWidth
           && rTwoRect.mnSrcHeight  == maTwoRect.mnSrcHeight
           && rTwoRect.mnDestWidth  == maTwoRect.mnDestWidth
           && rTwoRect.mnDestHeight == maTwoRect.mnDestHeight
           )
        {
            // absolutely indentically
            bRet = true;
        }
        else if(  rTwoRect.mnSrcWidth   == rTwoRect.mnDestWidth
               && rTwoRect.mnSrcHeight  == rTwoRect.mnDestHeight
               && maTwoRect.mnSrcWidth  == maTwoRect.mnDestWidth
               && maTwoRect.mnSrcHeight == maTwoRect.mnDestHeight
               && rTwoRect.mnSrcX       >= maTwoRect.mnSrcX
               && rTwoRect.mnSrcY       >= maTwoRect.mnSrcY
               && ( rTwoRect.mnSrcX + rTwoRect.mnSrcWidth  ) <= ( maTwoRect.mnSrcX + maTwoRect.mnSrcWidth  )
               && ( rTwoRect.mnSrcY + rTwoRect.mnSrcHeight ) <= ( maTwoRect.mnSrcY + maTwoRect.mnSrcHeight )
               )
        {
            bRet = true;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void ImplSalDDB::ImplDraw(
    Drawable aDrawable,
    long nDrawableDepth,
    const SalTwoRect& rTwoRect,
    const GC& rGC
) const
{
    ImplDraw( maPixmap, mnDepth, aDrawable, nDrawableDepth,
              rTwoRect.mnSrcX - maTwoRect.mnSrcX, rTwoRect.mnSrcY - maTwoRect.mnSrcY,
              rTwoRect.mnDestWidth, rTwoRect.mnDestHeight,
              rTwoRect.mnDestX, rTwoRect.mnDestY, rGC );
}

// -----------------------------------------------------------------------------

void ImplSalDDB::ImplDraw(
    Drawable aSrcDrawable,
    long nSrcDrawableDepth,
    Drawable aDstDrawable,
    long,
    long nSrcX,
    long nSrcY,
    long nDestWidth,
    long nDestHeight,
    long nDestX,
    long nDestY,
    const GC& rGC
) {
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    Display*    pXDisp = pSalDisp->GetDisplay();

    if( 1 == nSrcDrawableDepth )
    {
        XCopyPlane( pXDisp, aSrcDrawable, aDstDrawable, rGC,
                    nSrcX, nSrcY, nDestWidth, nDestHeight, nDestX, nDestY, 1 );
    }
    else
    {
        XCopyArea( pXDisp, aSrcDrawable, aDstDrawable, rGC,
                   nSrcX, nSrcY, nDestWidth, nDestHeight, nDestX, nDestY );
    }
}

// ----------------------
// - ImplSalBitmapCache -
// ----------------------

struct ImplBmpObj
{
    X11SalBitmap*   mpBmp;
    sal_uLong       mnMemSize;
    sal_uLong       mnFlags;

                ImplBmpObj( X11SalBitmap* pBmp, sal_uLong nMemSize, sal_uLong nFlags ) :
                    mpBmp( pBmp ), mnMemSize( nMemSize ), mnFlags( nFlags ) {}
};

// -----------------------------------------------------------------------------

ImplSalBitmapCache::ImplSalBitmapCache() :
    mnTotalSize( 0UL )
{
}

// -----------------------------------------------------------------------------

ImplSalBitmapCache::~ImplSalBitmapCache()
{
    ImplClear();
}

// -----------------------------------------------------------------------------

void ImplSalBitmapCache::ImplAdd( X11SalBitmap* pBmp, sal_uLong nMemSize, sal_uLong nFlags )
{
    ImplBmpObj* pObj = NULL;
    bool        bFound = false;

    for(
        BmpList_impl::iterator it = maBmpList.begin();
        (it != maBmpList.end() ) && !bFound ;
        ++it
    ) {
        pObj = *it;
        if( pObj->mpBmp == pBmp )
            bFound = true;
    }

    mnTotalSize += nMemSize;

    if( bFound )
    {
        mnTotalSize -= pObj->mnMemSize;
        pObj->mnMemSize = nMemSize, pObj->mnFlags = nFlags;
    }
    else
        maBmpList.push_back( new ImplBmpObj( pBmp, nMemSize, nFlags ) );
}

// -----------------------------------------------------------------------------

void ImplSalBitmapCache::ImplRemove( X11SalBitmap* pBmp )
{
    for(
        BmpList_impl::iterator it = maBmpList.begin();
        it != maBmpList.end();
        ++it
    ) {
        if( (*it)->mpBmp == pBmp )
        {
            (*it)->mpBmp->ImplRemovedFromCache();
            mnTotalSize -= (*it)->mnMemSize;
            delete *it;
            maBmpList.erase( it );
            break;
        }
    }
}

// -----------------------------------------------------------------------------

void ImplSalBitmapCache::ImplClear()
{
    for(
        BmpList_impl::iterator it = maBmpList.begin();
        it != maBmpList.end();
        ++it
    ) {
        (*it)->mpBmp->ImplRemovedFromCache();
        delete *it;
    }
    maBmpList.clear();
    mnTotalSize = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
