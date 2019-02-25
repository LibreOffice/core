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

#include <string.h>
#include <errno.h>

#ifdef FREEBSD
#include <sys/types.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>

#include <osl/endian.h>
#include <sal/log.hxx>

#include <tools/helpers.hxx>
#include <tools/debug.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salbtype.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salbmp.h>
#include <unx/salinst.h>
#include <unx/x11/xlimits.hxx>

#include <o3tl/safeint.hxx>
#include <opengl/salbmp.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

#include <memory>


std::shared_ptr<SalBitmap> X11SalInstance::CreateSalBitmap()
{
    if (OpenGLHelper::isVCLOpenGLEnabled())
        return std::make_shared<OpenGLSalBitmap>();
    else
        return std::make_shared<X11SalBitmap>();
}

ImplSalBitmapCache* X11SalBitmap::mpCache = nullptr;
unsigned int        X11SalBitmap::mnCacheInstCount = 0;

X11SalBitmap::X11SalBitmap()
    : mbGrey( false )
{
}

X11SalBitmap::~X11SalBitmap()
{
    Destroy();
}

void X11SalBitmap::ImplCreateCache()
{
    if( !mnCacheInstCount++ )
        mpCache = new ImplSalBitmapCache;
}

void X11SalBitmap::ImplDestroyCache()
{
    SAL_WARN_IF( !mnCacheInstCount, "vcl", "X11SalBitmap::ImplDestroyCache(): underflow" );

    if( mnCacheInstCount && !--mnCacheInstCount )
    {
        delete mpCache;
        mpCache = nullptr;
    }
}

void X11SalBitmap::ImplRemovedFromCache()
{
    mpDDB.reset();
}

#if defined HAVE_VALGRIND_HEADERS
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

std::unique_ptr<BitmapBuffer> X11SalBitmap::ImplCreateDIB(
    const Size& rSize,
    sal_uInt16 nBitCount,
    const BitmapPalette& rPal)
{
    DBG_ASSERT(
           nBitCount ==  1
        || nBitCount ==  4
        || nBitCount ==  8
        || nBitCount == 16
        || nBitCount == 24
        , "Unsupported BitCount!"
    );

    std::unique_ptr<BitmapBuffer> pDIB;

    if( !rSize.Width() || !rSize.Height() )
        return nullptr;

    try
    {
        pDIB.reset(new BitmapBuffer);
    }
    catch (const std::bad_alloc&)
    {
        return nullptr;
    }

    const sal_uInt16 nColors = ( nBitCount <= 8 ) ? ( 1 << nBitCount ) : 0;

    pDIB->mnFormat = ScanlineFormat::NONE;

    switch( nBitCount )
    {
        case 1: pDIB->mnFormat |= ScanlineFormat::N1BitMsbPal; break;
        case 4: pDIB->mnFormat |= ScanlineFormat::N4BitMsnPal; break;
        case 8: pDIB->mnFormat |= ScanlineFormat::N8BitPal; break;
#ifdef OSL_BIGENDIAN
        case 16:
        {
            pDIB->mnFormat|= ScanlineFormat::N16BitTcMsbMask;
            ColorMaskElement aRedMask(0xf800);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x07e0);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x001f);
            aBlueMask.CalcMaskShift();
            pDIB->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
#else
        case 16:
        {
            pDIB->mnFormat|= ScanlineFormat::N16BitTcLsbMask;
            ColorMaskElement aRedMask(0xf800);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(0x07e0);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(0x001f);
            aBlueMask.CalcMaskShift();
            pDIB->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
#endif
        default:
            nBitCount = 24;
            [[fallthrough]];
        case 24:
            pDIB->mnFormat |= ScanlineFormat::N24BitTcBgr;
        break;
    }

    pDIB->mnWidth = rSize.Width();
    pDIB->mnHeight = rSize.Height();
    long nScanlineBase;
    bool bFail = o3tl::checked_multiply<long>(pDIB->mnWidth, nBitCount, nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl.gdi", "checked multiply failed");
        return nullptr;
    }
    pDIB->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (pDIB->mnScanlineSize < nScanlineBase/8)
    {
        SAL_WARN("vcl.gdi", "scanline calculation wraparound");
        return nullptr;
    }
    pDIB->mnBitCount = nBitCount;

    if( nColors )
    {
        pDIB->maPalette = rPal;
        pDIB->maPalette.SetEntryCount( nColors );
    }

    try
    {
        pDIB->mpBits = new sal_uInt8[ pDIB->mnScanlineSize * pDIB->mnHeight ];
#if defined HAVE_VALGRIND_HEADERS
        if (RUNNING_ON_VALGRIND)
            blankExtraSpace(pDIB.get());
#endif
    }
    catch (const std::bad_alloc&)
    {
        return nullptr;
    }

    return pDIB;
}

std::unique_ptr<BitmapBuffer> X11SalBitmap::ImplCreateDIB(
    Drawable aDrawable,
    SalX11Screen nScreen,
    long nDrawableDepth,
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    bool bGrey
) {
    std::unique_ptr<BitmapBuffer> pDIB;

    if( aDrawable && nWidth && nHeight && nDrawableDepth )
    {
        SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericUnixSalData());
        Display*    pXDisp = pSalDisp->GetDisplay();

        // do not die on XError here
        // alternatively one could check the coordinates for being offscreen
        // but this call can actually work on servers with backing store
        // defaults even if the rectangle is offscreen
        // so better catch the XError
        GetGenericUnixSalData()->ErrorTrapPush();
        XImage* pImage = XGetImage( pXDisp, aDrawable, nX, nY, nWidth, nHeight, AllPlanes, ZPixmap );
        bool bWasError = GetGenericUnixSalData()->ErrorTrapPop( false );

        if( ! bWasError && pImage && pImage->data )
        {
            const SalTwoRect        aTwoRect = { 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight };
            BitmapBuffer            aSrcBuf;
            const BitmapPalette*    pDstPal = nullptr;

            aSrcBuf.mnFormat = ScanlineFormat::TopDown;
            aSrcBuf.mnWidth = nWidth;
            aSrcBuf.mnHeight = nHeight;
            aSrcBuf.mnBitCount = pImage->bits_per_pixel;
            aSrcBuf.mnScanlineSize = pImage->bytes_per_line;
            aSrcBuf.mpBits = reinterpret_cast<sal_uInt8*>(pImage->data);

            pImage->red_mask = pSalDisp->GetVisual( nScreen ).red_mask;
            pImage->green_mask = pSalDisp->GetVisual( nScreen ).green_mask;
            pImage->blue_mask = pSalDisp->GetVisual( nScreen ).blue_mask;

            switch( aSrcBuf.mnBitCount )
            {
                case 1:
                {
                    aSrcBuf.mnFormat |= ( LSBFirst == pImage->bitmap_bit_order
                                            ? ScanlineFormat::N1BitLsbPal
                                            : ScanlineFormat::N1BitMsbPal
                                        );
                }
                break;

                case 4:
                {
                    aSrcBuf.mnFormat |= ( LSBFirst == pImage->bitmap_bit_order
                                            ? ScanlineFormat::N4BitLsnPal
                                            : ScanlineFormat::N4BitMsnPal
                                        );
                }
                break;

                case 8:
                {
                    aSrcBuf.mnFormat |= ScanlineFormat::N8BitPal;
                }
                break;

                case 16:
                {
                    ColorMaskElement aRedMask(pImage->red_mask);
                    aRedMask.CalcMaskShift();
                    ColorMaskElement aGreenMask(pImage->green_mask);
                    aGreenMask.CalcMaskShift();
                    ColorMaskElement aBlueMask(pImage->blue_mask);
                    aBlueMask.CalcMaskShift();
                    aSrcBuf.maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);

                    if( LSBFirst == pImage->byte_order )
                    {
                        aSrcBuf.mnFormat |= ScanlineFormat::N16BitTcLsbMask;
                    }
                    else
                    {
                        aSrcBuf.mnFormat |= ScanlineFormat::N16BitTcMsbMask;
                    }
                }
                break;

                case 24:
                {
                    if( ( LSBFirst == pImage->byte_order ) && ( pImage->red_mask == 0xFF ) )
                        aSrcBuf.mnFormat |= ScanlineFormat::N24BitTcRgb;
                    else
                        aSrcBuf.mnFormat |= ScanlineFormat::N24BitTcBgr;
                }
                break;

                case 32:
                {
                    if( LSBFirst == pImage->byte_order )
                        aSrcBuf.mnFormat |= (  pSalDisp->GetVisual(nScreen).red_mask == 0xFF
                                                ? ScanlineFormat::N32BitTcRgba
                                                : ScanlineFormat::N32BitTcBgra
                                            );
                    else
                        aSrcBuf.mnFormat |= (  pSalDisp->GetVisual(nScreen).red_mask == 0xFF
                                                ? ScanlineFormat::N32BitTcAbgr
                                                : ScanlineFormat::N32BitTcArgb
                                            );
                }
                break;
            }

            BitmapPalette& rPal = aSrcBuf.maPalette;

            if( aSrcBuf.mnBitCount == 1 )
            {
                rPal.SetEntryCount( 2 );
                pDstPal = &rPal;

                rPal[ 0 ] = COL_BLACK;
                rPal[ 1 ] = COL_WHITE;
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
                const sal_uInt16 nCols = std::min(static_cast<sal_uLong>(rColMap.GetUsed()),
                                                  sal_uLong(1) << nDrawableDepth);

                rPal.SetEntryCount( nCols );
                pDstPal = &rPal;

                for( sal_uInt16 i = 0; i < nCols; i++ )
                {
                    const Color  nColor( rColMap.GetColor( i ) );
                    BitmapColor&    rBmpCol = rPal[ i ];

                    rBmpCol.SetRed( nColor.GetRed() );
                    rBmpCol.SetGreen( nColor.GetGreen() );
                    rBmpCol.SetBlue( nColor.GetBlue() );
                }
            }

            pDIB = StretchAndConvert( aSrcBuf, aTwoRect, aSrcBuf.mnFormat,
                pDstPal, &aSrcBuf.maColorMask );
            XDestroyImage( pImage );
        }
    }

    return pDIB;
}

XImage* X11SalBitmap::ImplCreateXImage(
    SalDisplay const *pSalDisp,
    SalX11Screen nScreen,
    long nDepth,
    const SalTwoRect& rTwoRect
) const
{
    XImage* pImage = nullptr;

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
                               nDepth, ( 1 == nDepth ) ? XYBitmap :ZPixmap, 0, nullptr,
                               nWidth, nHeight, 32, 0 );

        if( pImage )
        {
            std::unique_ptr<BitmapBuffer> pDstBuf;
            ScanlineFormat       nDstFormat = ScanlineFormat::TopDown;
            std::unique_ptr<BitmapPalette> xPal;
            std::unique_ptr<ColorMask> xMask;

            switch( pImage->bits_per_pixel )
            {
                case 1:
                    nDstFormat |=   ( LSBFirst == pImage->bitmap_bit_order
                                        ? ScanlineFormat::N1BitLsbPal
                                        : ScanlineFormat::N1BitMsbPal
                                    );
                break;

                case 4:
                    nDstFormat |=   ( LSBFirst == pImage->bitmap_bit_order
                                        ? ScanlineFormat::N4BitLsnPal
                                        : ScanlineFormat::N4BitMsnPal
                                    );
                break;

                case 8:
                    nDstFormat |= ScanlineFormat::N8BitPal;
                break;

                case 16:
                {
                    #ifdef OSL_BIGENDIAN

                    if( MSBFirst == pImage->byte_order )
                        nDstFormat |= ScanlineFormat::N16BitTcMsbMask;
                    else
                        nDstFormat |= ScanlineFormat::N16BitTcLsbMask;

                    #else /* OSL_LITENDIAN */

                    nDstFormat |= ScanlineFormat::N16BitTcLsbMask;
                    if( MSBFirst == pImage->byte_order )
                        pImage->byte_order = LSBFirst;

                    #endif

                    ColorMaskElement aRedMask(pImage->red_mask);
                    aRedMask.CalcMaskShift();
                    ColorMaskElement aGreenMask(pImage->green_mask);
                    aGreenMask.CalcMaskShift();
                    ColorMaskElement aBlueMask(pImage->blue_mask);
                    aBlueMask.CalcMaskShift();
                    xMask.reset(new ColorMask(aRedMask, aGreenMask, aBlueMask));
                }
                break;

                case 24:
                {
                    if( ( LSBFirst == pImage->byte_order ) && ( pImage->red_mask == 0xFF ) )
                        nDstFormat |= ScanlineFormat::N24BitTcRgb;
                    else
                        nDstFormat |= ScanlineFormat::N24BitTcBgr;
                }
                break;

                case 32:
                {
                    if( LSBFirst == pImage->byte_order )
                        nDstFormat |=   ( pImage->red_mask == 0xFF
                                            ? ScanlineFormat::N32BitTcRgba
                                            : ScanlineFormat::N32BitTcBgra
                                        );
                    else
                        nDstFormat |=   ( pImage->red_mask == 0xFF
                                            ? ScanlineFormat::N32BitTcAbgr
                                            : ScanlineFormat::N32BitTcArgb
                                        );
                }
                break;
            }

            if( pImage->depth == 1 )
            {
                xPal.reset(new BitmapPalette( 2 ));
                (*xPal)[ 0 ] = COL_BLACK;
                (*xPal)[ 1 ] = COL_WHITE;
            }
            else if( pImage->depth == 8 && mbGrey )
            {
                xPal.reset(new BitmapPalette( 256 ));

                for( sal_uInt16 i = 0; i < 256; i++ )
                {
                    BitmapColor&    rBmpCol = (*xPal)[ i ];

                    rBmpCol.SetRed( i );
                    rBmpCol.SetGreen( i );
                    rBmpCol.SetBlue( i );
                }

            }
            else if( pImage->depth <= 8 )
            {
                const SalColormap& rColMap = pSalDisp->GetColormap( nScreen );
                const sal_uInt16 nCols = std::min( static_cast<sal_uLong>(rColMap.GetUsed())
                                            , static_cast<sal_uLong>(1 << pImage->depth)
                                            );

                xPal.reset(new BitmapPalette( nCols ));

                for( sal_uInt16 i = 0; i < nCols; i++ )
                {
                    const Color  nColor( rColMap.GetColor( i ) );
                    BitmapColor&    rBmpCol = (*xPal)[ i ];

                    rBmpCol.SetRed( nColor.GetRed() );
                    rBmpCol.SetGreen( nColor.GetGreen() );
                    rBmpCol.SetBlue( nColor.GetBlue() );
                }
            }

            pDstBuf = StretchAndConvert( *mpDIB, rTwoRect, nDstFormat, xPal.get(), xMask.get() );
            xPal.reset();
            xMask.reset();

            if( pDstBuf && pDstBuf->mpBits )
            {
#if defined HAVE_VALGRIND_HEADERS
                if (RUNNING_ON_VALGRIND)
                    blankExtraSpace(pDstBuf.get());
#endif
                // set data in buffer as data member in pImage
                pImage->data = reinterpret_cast<char*>(pDstBuf->mpBits);
            }
            else
            {
                XDestroyImage( pImage );
                pImage = nullptr;
            }

            // note that pDstBuf it deleted here, but that doesn't destroy allocated data in buffer
        }
    }

    return pImage;
}

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
        mpDDB.reset(new ImplSalDDB( aDrawable, nScreen, nDrawableDepth, nX, nY, nWidth, nHeight ));

    return( mpDDB != nullptr );
}

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

            mpDDB.reset();
        }

        if( mpCache )
            mpCache->ImplRemove( this );

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
                return nullptr; // this would be a really mad case

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

        XImage* pImage = ImplCreateXImage( vcl_sal::getSalDisplay(GetGenericUnixSalData()), nXScreen,
                                           nDrawableDepth, aTwoRect );

        if( pImage )
        {
            mpDDB.reset(new ImplSalDDB( pImage, aDrawable, nXScreen, aTwoRect ));
            delete[] pImage->data;
            pImage->data = nullptr;
            XDestroyImage( pImage );

            if( mpCache )
                mpCache->ImplAdd( const_cast<X11SalBitmap*>(this) );
        }
    }

    return mpDDB.get();
}

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
        mpDDB->ImplDraw( aDrawable, rTwoRect, rGC );
}

bool X11SalBitmap::Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal )
{
    Destroy();
    mpDIB = ImplCreateDIB( rSize, nBitCount, rPal );

    return( mpDIB != nullptr );
}

bool X11SalBitmap::Create( const SalBitmap& rSSalBmp )
{
    Destroy();

    const X11SalBitmap& rSalBmp = static_cast<const X11SalBitmap&>( rSSalBmp );

    if( rSalBmp.mpDIB )
    {
        // TODO: reference counting...
        mpDIB.reset(new BitmapBuffer( *rSalBmp.mpDIB ));
        // TODO: get rid of this when BitmapBuffer gets copy constructor
        try
        {
            mpDIB->mpBits = new sal_uInt8[ mpDIB->mnScanlineSize * mpDIB->mnHeight ];
#if defined HAVE_VALGRIND_HEADERS
            if (RUNNING_ON_VALGRIND)
                blankExtraSpace(mpDIB.get());
#endif
        }
        catch (const std::bad_alloc&)
        {
            mpDIB.reset();
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
            ( rSalBmp.mpDIB && ( mpDIB != nullptr ) ) ||
            ( rSalBmp.mpDDB && ( mpDDB != nullptr ) ) );
}

bool X11SalBitmap::Create( const SalBitmap&, SalGraphics* )
{
    return false;
}

bool X11SalBitmap::Create( const SalBitmap&, sal_uInt16 )
{
    return false;
}

bool X11SalBitmap::Create(
    const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
    Size& rSize,
    bool bMask
) {
    css::uno::Reference< css::beans::XFastPropertySet > xFastPropertySet( rBitmapCanvas, css::uno::UNO_QUERY );

    if( xFastPropertySet.get() ) {
        sal_Int32 depth;
        css::uno::Sequence< css::uno::Any > args;

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
                                    rSize.Width(),
                                    rSize.Height()
                                );
                bool bFreePixmap = false;
                if( bSuccess && (args[0] >>= bFreePixmap) && bFreePixmap )
                    XFreePixmap( vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetDisplay(), pixmapHandle );

                return bSuccess;
            }
        }
    }

    return false;
}

void X11SalBitmap::Destroy()
{
    if( mpDIB )
    {
        delete[] mpDIB->mpBits;
        mpDIB.reset();
    }

    mpDDB.reset();

    if( mpCache )
        mpCache->ImplRemove( this );
}

Size X11SalBitmap::GetSize() const
{
    Size aSize;

    if( mpDIB )
    {
        aSize.setWidth( mpDIB->mnWidth );
        aSize.setHeight( mpDIB->mnHeight );
    }
    else if( mpDDB )
    {
        aSize.setWidth( mpDDB->ImplGetWidth() );
        aSize.setHeight( mpDDB->ImplGetHeight() );
    }

    return aSize;
}

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

BitmapBuffer* X11SalBitmap::AcquireBuffer( BitmapAccessMode /*nMode*/ )
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

    return mpDIB.get();
}

void X11SalBitmap::ReleaseBuffer( BitmapBuffer*, BitmapAccessMode nMode )
{
    if( nMode == BitmapAccessMode::Write )
    {
        mpDDB.reset();

        if( mpCache )
            mpCache->ImplRemove( this );
        InvalidateChecksum();
    }
}

bool X11SalBitmap::GetSystemData( BitmapSystemData& rData )
{
    if( mpDDB )
    {
        // Rename/retype pDummy to your likings (though X11 Pixmap is
        // prolly not a good idea, since it's accessed from
        // non-platform aware code in vcl/bitmap.hxx)
        rData.aPixmap = reinterpret_cast<void*>(mpDDB->ImplGetPixmap());
        rData.mnWidth = mpDDB->ImplGetWidth ();
        rData.mnHeight = mpDDB->ImplGetHeight ();
        return true;
    }

    return false;
}

bool X11SalBitmap::ScalingSupported() const
{
    return false;
}

bool X11SalBitmap::Scale( const double& /*rScaleX*/, const double& /*rScaleY*/, BmpScaleFlag /*nScaleFlag*/ )
{
    return false;
}

bool X11SalBitmap::Replace( const Color& /*rSearchColor*/, const Color& /*rReplaceColor*/, sal_uInt8 /*nTol*/ )
{
    return false;
}


ImplSalDDB::ImplSalDDB( XImage* pImage, Drawable aDrawable,
                        SalX11Screen nXScreen, const SalTwoRect& rTwoRect )
    : maPixmap    ( 0 )
    , maTwoRect   ( rTwoRect )
    , mnDepth     ( pImage->depth )
    , mnXScreen   ( nXScreen )
{
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericUnixSalData());
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
            aValues.foreground = 1;
            aValues.background = 0;
        }

        aGC = XCreateGC( pXDisp, maPixmap, nValues, &aValues );
        XPutImage( pXDisp, maPixmap, aGC, pImage, 0, 0, 0, 0, maTwoRect.mnDestWidth, maTwoRect.mnDestHeight );
        XFreeGC( pXDisp, aGC );
    }
}

ImplSalDDB::ImplSalDDB(
    Drawable aDrawable,
    SalX11Screen nXScreen,
    long nDrawableDepth,
    long nX,
    long nY,
    long nWidth,
    long nHeight
)   : maTwoRect(0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight)
    , mnDepth( nDrawableDepth )
    , mnXScreen( nXScreen )
{
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericUnixSalData());
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
            aValues.foreground = 1;
            aValues.background = 0;
        }

        aGC = XCreateGC( pXDisp, maPixmap, nValues, &aValues );
        ImplDraw( aDrawable, nDrawableDepth, maPixmap,
                  nX, nY, nWidth, nHeight, 0, 0, aGC );
        XFreeGC( pXDisp, aGC );
    }
    else
    {
        maTwoRect.mnSrcWidth = maTwoRect.mnDestWidth = 0;
        maTwoRect.mnSrcHeight = maTwoRect.mnDestHeight = 0;
    }
}

ImplSalDDB::~ImplSalDDB()
{
    if( maPixmap && ImplGetSVData() )
        XFreePixmap( vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetDisplay(), maPixmap );
}

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
            // absolutely identically
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

void ImplSalDDB::ImplDraw(
    Drawable aDrawable,
    const SalTwoRect& rTwoRect,
    const GC& rGC
) const
{
    ImplDraw( maPixmap, mnDepth, aDrawable,
              rTwoRect.mnSrcX - maTwoRect.mnSrcX, rTwoRect.mnSrcY - maTwoRect.mnSrcY,
              rTwoRect.mnDestWidth, rTwoRect.mnDestHeight,
              rTwoRect.mnDestX, rTwoRect.mnDestY, rGC );
}

void ImplSalDDB::ImplDraw(
    Drawable aSrcDrawable,
    long nSrcDrawableDepth,
    Drawable aDstDrawable,
    long nSrcX,
    long nSrcY,
    long nDestWidth,
    long nDestHeight,
    long nDestX,
    long nDestY,
    const GC& rGC
) {
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericUnixSalData());
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


ImplSalBitmapCache::ImplSalBitmapCache()
{
}

ImplSalBitmapCache::~ImplSalBitmapCache()
{
    ImplClear();
}

void ImplSalBitmapCache::ImplAdd( X11SalBitmap* pBmp )
{
    for(auto pObj : maBmpList)
    {
        if( pObj == pBmp )
            return;
    }
    maBmpList.push_back( pBmp );
}

void ImplSalBitmapCache::ImplRemove( X11SalBitmap const * pBmp )
{
    auto it = std::find(maBmpList.begin(), maBmpList.end(), pBmp);
    if( it != maBmpList.end() )
    {
        (*it)->ImplRemovedFromCache();
        maBmpList.erase( it );
    }
}

void ImplSalBitmapCache::ImplClear()
{
    for(auto pObj : maBmpList)
    {
        pObj->ImplRemovedFromCache();
    }
    maBmpList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
