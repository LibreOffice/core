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

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>

#include <bmpfast.hxx>
#include <salgdi.hxx>
#include <impbmp.hxx>
#include <image.h>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <boost/scoped_array.hpp>


void OutputDevice::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    const Size aSizePix( rBitmap.GetSizePixel() );
    DrawBitmap( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, META_BMP_ACTION );
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    DrawBitmap( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, META_BMPSCALE_ACTION );
}


void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                   const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                   const Bitmap& rBitmap, const sal_uLong nAction )
{
    if( ImplIsRecordLayout() )
        return;

    if ( ( mnDrawMode & DRAWMODE_NOBITMAP ) )
    {
        return;
    }
    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    Bitmap aBmp( rBitmap );

    if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP |
                             DRAWMODE_GRAYBITMAP  | DRAWMODE_GHOSTEDBITMAP ) )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP ) )
        {
            sal_uInt8 cCmpVal;

            if ( mnDrawMode & DRAWMODE_BLACKBITMAP )
                cCmpVal = ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 0x80 : 0;
            else
                cCmpVal = 255;

            Color aCol( cCmpVal, cCmpVal, cCmpVal );
            Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            SetLineColor( aCol );
            SetFillColor( aCol );
            DrawRect( Rectangle( rDestPt, rDestSize ) );
            Pop();
            return;
        }
        else if( !!aBmp )
        {
            if ( mnDrawMode & DRAWMODE_GRAYBITMAP )
                aBmp.Convert( BMP_CONVERSION_8BIT_GREYS );

            if ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP )
                aBmp.Convert( BMP_CONVERSION_GHOSTED );
        }
    }

    if ( mpMetaFile )
    {
        switch( nAction )
        {
            case( META_BMP_ACTION ):
                mpMetaFile->AddAction( new MetaBmpAction( rDestPt, aBmp ) );
            break;

            case( META_BMPSCALE_ACTION ):
                mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
            break;

            case( META_BMPSCALEPART_ACTION ):
                mpMetaFile->AddAction( new MetaBmpScalePartAction(
                    rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp ) );
            break;
        }
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
        if ( !AcquireGraphics() )
            return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if( !aBmp.IsEmpty() )
    {
        SalTwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
            const sal_uLong nMirrFlags = AdjustTwoRect( aPosAry, aBmp.GetSizePixel() );

            if ( nMirrFlags )
                aBmp.Mirror( nMirrFlags );

            if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            {
                if ( nAction == META_BMPSCALE_ACTION )
                    ScaleBitmap (aBmp, aPosAry);

                mpGraphics->DrawBitmap( aPosAry, *aBmp.ImplGetImpBitmap()->ImplGetSalBitmap(), this );
            }
        }
    }

    if( mpAlphaVDev )
    {
        // #i32109#: Make bitmap area opaque
        mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
    }
}

Bitmap OutputDevice::GetDownsampledBitmap( const Size& rDstSz,
                                           const Point& rSrcPt, const Size& rSrcSz,
                                           const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    Bitmap aBmp( rBmp );

    if( !aBmp.IsEmpty() )
    {
        Point           aPoint;
        const Rectangle aBmpRect( aPoint, aBmp.GetSizePixel() );
        Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if necessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmp.Crop( aSrcRect );
            else
                aBmp.SetEmpty();
        }

        if( !aBmp.IsEmpty() )
        {
            // do downsampling if necessary
            Size aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );

            // #103209# Normalize size (mirroring has to happen outside of this method)
            aDstSizeTwip = Size( labs(aDstSizeTwip.Width()), labs(aDstSizeTwip.Height()) );

            const Size      aBmpSize( aBmp.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmp.Scale( aNewBmpSize );
                else
                    aBmp.SetEmpty();
            }
        }
    }

    return aBmp;
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt,
                                 const BitmapEx& rBitmapEx )
{
    if( ImplIsRecordLayout() )
        return;

    if( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rBitmapEx.GetBitmap() );
    }
    else
    {
        const Size aSizePix( rBitmapEx.GetSizePixel() );
        DrawBitmapEx( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmapEx, META_BMPEX_ACTION );
    }
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const BitmapEx& rBitmapEx )
{
    if( ImplIsRecordLayout() )
        return;

    if ( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rDestSize, rBitmapEx.GetBitmap() );
    }
    else
    {
        DrawBitmapEx( rDestPt, rDestSize, Point(), rBitmapEx.GetSizePixel(), rBitmapEx, META_BMPEXSCALE_ACTION );
    }
}


void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const BitmapEx& rBitmapEx, const sal_uLong nAction )
{

    if( ImplIsRecordLayout() )
        return;

    if( TRANSPARENT_NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx.GetBitmap() );
    }
    else
    {
        if ( mnDrawMode & DRAWMODE_NOBITMAP )
            return;

        if ( ROP_INVERT == meRasterOp )
        {
            DrawRect( Rectangle( rDestPt, rDestSize ) );
            return;
        }

        BitmapEx aBmpEx( rBitmapEx );

        if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP |
                                 DRAWMODE_GRAYBITMAP | DRAWMODE_GHOSTEDBITMAP ) )
        {
            if ( mnDrawMode & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP ) )
            {
                Bitmap  aColorBmp( aBmpEx.GetSizePixel(), ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 4 : 1 );
                sal_uInt8   cCmpVal;

                if ( mnDrawMode & DRAWMODE_BLACKBITMAP )
                    cCmpVal = ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP ) ? 0x80 : 0;
                else
                    cCmpVal = 255;

                aColorBmp.Erase( Color( cCmpVal, cCmpVal, cCmpVal ) );

                if( aBmpEx.IsAlpha() )
                {
                    // Create one-bit mask out of alpha channel, by
                    // thresholding it at alpha=0.5. As
                    // DRAWMODE_BLACK/WHITEBITMAP requires monochrome
                    // output, having alpha-induced grey levels is not
                    // acceptable.
                    Bitmap aMask( aBmpEx.GetAlpha().GetBitmap() );
                    aMask.MakeMono( 129 );
                    aBmpEx = BitmapEx( aColorBmp, aMask );
                }
                else
                {
                    aBmpEx = BitmapEx( aColorBmp, aBmpEx.GetMask() );
                }
            }
            else if( !!aBmpEx )
            {
                if ( mnDrawMode & DRAWMODE_GRAYBITMAP )
                    aBmpEx.Convert( BMP_CONVERSION_8BIT_GREYS );

                if ( mnDrawMode & DRAWMODE_GHOSTEDBITMAP )
                    aBmpEx.Convert( BMP_CONVERSION_GHOSTED );
            }
        }

        if ( mpMetaFile )
        {
            switch( nAction )
            {
                case( META_BMPEX_ACTION ):
                    mpMetaFile->AddAction( new MetaBmpExAction( rDestPt, aBmpEx ) );
                break;

                case( META_BMPEXSCALE_ACTION ):
                    mpMetaFile->AddAction( new MetaBmpExScaleAction( rDestPt, rDestSize, aBmpEx ) );
                break;

                case( META_BMPEXSCALEPART_ACTION ):
                    mpMetaFile->AddAction( new MetaBmpExScalePartAction( rDestPt, rDestSize,
                                                                         rSrcPtPixel, rSrcSizePixel, aBmpEx ) );
                break;
            }
        }

        if ( !IsDeviceOutputNecessary() )
            return;

        if ( !mpGraphics )
            if ( !AcquireGraphics() )
                return;

        if ( mbInitClipRegion )
            InitClipRegion();

        if ( mbOutputClipped )
            return;

        DrawDeviceBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmpEx );
    }
}

Bitmap OutputDevice::GetBitmap( const Point& rSrcPt, const Size& rSize ) const
{
    Bitmap  aBmp;
    long    nX = ImplLogicXToDevicePixel( rSrcPt.X() );
    long    nY = ImplLogicYToDevicePixel( rSrcPt.Y() );
    long    nWidth = ImplLogicWidthToDevicePixel( rSize.Width() );
    long    nHeight = ImplLogicHeightToDevicePixel( rSize.Height() );

    if ( mpGraphics || AcquireGraphics() )
    {
        if ( nWidth > 0 && nHeight  > 0 && nX <= (mnOutWidth + mnOutOffX) && nY <= (mnOutHeight + mnOutOffY))
        {
            Rectangle   aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            bool        bClipped = false;

            // X-Coordinate outside of draw area?
            if ( nX < mnOutOffX )
            {
                nWidth -= ( mnOutOffX - nX );
                nX = mnOutOffX;
                bClipped = true;
            }

            // Y-Coordinate outside of draw area?
            if ( nY < mnOutOffY )
            {
                nHeight -= ( mnOutOffY - nY );
                nY = mnOutOffY;
                bClipped = true;
            }

            // Width outside of draw area?
            if ( (nWidth + nX) > (mnOutWidth + mnOutOffX) )
            {
                nWidth  = mnOutOffX + mnOutWidth - nX;
                bClipped = true;
            }

            // Height outside of draw area?
            if ( (nHeight + nY) > (mnOutHeight + mnOutOffY) )
            {
                nHeight = mnOutOffY + mnOutHeight - nY;
                bClipped = true;
            }

            if ( bClipped )
            {
                // If the visible part has been clipped, we have to create a
                // Bitmap with the correct size in which we copy the clipped
                // Bitmap to the correct position.
                VirtualDevice aVDev( *this );

                if ( aVDev.SetOutputSizePixel( aRect.GetSize() ) )
                {
                    if ( ((OutputDevice*)&aVDev)->mpGraphics || ((OutputDevice*)&aVDev)->AcquireGraphics() )
                    {
                        SalTwoRect aPosAry;

                        aPosAry.mnSrcX = nX;
                        aPosAry.mnSrcY = nY;
                        aPosAry.mnSrcWidth = nWidth;
                        aPosAry.mnSrcHeight = nHeight;
                        aPosAry.mnDestX = ( aRect.Left() < mnOutOffX ) ? ( mnOutOffX - aRect.Left() ) : 0L;
                        aPosAry.mnDestY = ( aRect.Top() < mnOutOffY ) ? ( mnOutOffY - aRect.Top() ) : 0L;
                        aPosAry.mnDestWidth = nWidth;
                        aPosAry.mnDestHeight = nHeight;

                        if ( (nWidth > 0) && (nHeight > 0) )
                        {
                            (((OutputDevice*)&aVDev)->mpGraphics)->CopyBits( aPosAry, mpGraphics, this, this );
                        }
                        else
                        {
                            OSL_ENSURE(false, "CopyBits with negative width or height (!)");
                        }

                        aBmp = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );
                     }
                     else
                        bClipped = false;
                }
                else
                    bClipped = false;
            }

            if ( !bClipped )
            {
                SalBitmap* pSalBmp = mpGraphics->GetBitmap( nX, nY, nWidth, nHeight, this );

                if( pSalBmp )
                {
                    ImpBitmap* pImpBmp = new ImpBitmap;
                    pImpBmp->ImplSetSalBitmap( pSalBmp );
                    aBmp.ImplSetImpBitmap( pImpBmp );
                }
            }
        }
    }

    return aBmp;
}

BitmapEx OutputDevice::GetBitmapEx( const Point& rSrcPt, const Size& rSize ) const
{

    // #110958# Extract alpha value from VDev, if any
    if( mpAlphaVDev )
    {
        Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( rSrcPt, rSize ) );

        // ensure 8 bit alpha
        if( aAlphaBitmap.GetBitCount() > 8 )
            aAlphaBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );

        return BitmapEx(GetBitmap( rSrcPt, rSize ), AlphaMask( aAlphaBitmap ) );
    }
    else
        return GetBitmap( rSrcPt, rSize );
}

void OutputDevice::DrawDeviceBitmap( const Point& rDestPt, const Size& rDestSize,
                                     const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                     BitmapEx& rBitmapEx )
{
    if (rBitmapEx.IsAlpha())
    {
        DrawDeviceAlphaBitmap(rBitmapEx.GetBitmap(), rBitmapEx.GetAlpha(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel);
    }
    else if (!!rBitmapEx)
    {
        SalTwoRect aPosAry;

        aPosAry.mnSrcX = rSrcPtPixel.X();
        aPosAry.mnSrcY = rSrcPtPixel.Y();
        aPosAry.mnSrcWidth = rSrcSizePixel.Width();
        aPosAry.mnSrcHeight = rSrcSizePixel.Height();
        aPosAry.mnDestX = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth = ImplLogicWidthToDevicePixel( rDestSize.Width() );
        aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

        const sal_uLong nMirrFlags = AdjustTwoRect(aPosAry, rBitmapEx.GetSizePixel());

        if (aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight)
        {

            if (nMirrFlags)
                rBitmapEx.Mirror(nMirrFlags);

            const SalBitmap* pSalSrcBmp = rBitmapEx.ImplGetBitmapImpBitmap()->ImplGetSalBitmap();
            const ImpBitmap* pMaskBmp = rBitmapEx.ImplGetMaskImpBitmap();

            if (pMaskBmp)
            {
                SalBitmap* pSalAlphaBmp = pMaskBmp->ImplGetSalBitmap();
                bool bTryDirectPaint(pSalSrcBmp && pSalAlphaBmp);

                if (bTryDirectPaint)
                {
                    // only paint direct when no scaling and no MapMode, else the
                    // more expensive conversions may be done for short-time Bitmap/BitmapEx
                    // used for buffering only
                    if(!IsMapMode() && aPosAry.mnSrcWidth == aPosAry.mnDestWidth && aPosAry.mnSrcHeight == aPosAry.mnDestHeight)
                    {
                        bTryDirectPaint = false;
                    }
                }

                if (bTryDirectPaint && mpGraphics->DrawAlphaBitmap(aPosAry, *pSalSrcBmp, *pSalAlphaBmp, this))
                {
                    // tried to paint as alpha directly. If tis worked, we are done (except
                    // alpha, see below)
                }
                else
                {
                    // #4919452# reduce operation area to bounds of
                    // cliprect. since masked transparency involves
                    // creation of a large vdev and copying the screen
                    // content into that (slooow read from framebuffer),
                    // that should considerably increase performance for
                    // large bitmaps and small clippings.

                    // Note that this optimization is a workaround for a
                    // Writer peculiarity, namely, to decompose background
                    // graphics into myriads of disjunct, tiny
                    // rectangles. That otherwise kills us here, since for
                    // transparent output, SAL always prepares the whole
                    // bitmap, if aPosAry contains the whole bitmap (and
                    // it's _not_ to blame for that).

                    // Note the call to ImplPixelToDevicePixel(), since
                    // aPosAry already contains the mnOutOff-offsets, they
                    // also have to be applied to the region
                    Rectangle aClipRegionBounds( ImplPixelToDevicePixel(maRegion).GetBoundRect() );

                    // TODO: Also respect scaling (that's a bit tricky,
                    // since the source points have to move fractional
                    // amounts (which is not possible, thus has to be
                    // emulated by increases copy area)
                    // const double nScaleX( aPosAry.mnDestWidth / aPosAry.mnSrcWidth );
                    // const double nScaleY( aPosAry.mnDestHeight / aPosAry.mnSrcHeight );

                    // for now, only identity scales allowed
                    if (!aClipRegionBounds.IsEmpty() &&
                        aPosAry.mnDestWidth == aPosAry.mnSrcWidth &&
                        aPosAry.mnDestHeight == aPosAry.mnSrcHeight)
                    {
                        // now intersect dest rect with clip region
                        aClipRegionBounds.Intersection(Rectangle(aPosAry.mnDestX,
                                                                 aPosAry.mnDestY,
                                                                 aPosAry.mnDestX + aPosAry.mnDestWidth - 1,
                                                                 aPosAry.mnDestY + aPosAry.mnDestHeight - 1));

                        // Note: I could theoretically optimize away the
                        // DrawBitmap below, if the region is empty
                        // here. Unfortunately, cannot rule out that
                        // somebody relies on the side effects.
                        if (!aClipRegionBounds.IsEmpty())
                        {
                            aPosAry.mnSrcX += aClipRegionBounds.Left() - aPosAry.mnDestX;
                            aPosAry.mnSrcY += aClipRegionBounds.Top() - aPosAry.mnDestY;
                            aPosAry.mnSrcWidth = aClipRegionBounds.GetWidth();
                            aPosAry.mnSrcHeight = aClipRegionBounds.GetHeight();

                            aPosAry.mnDestX = aClipRegionBounds.Left();
                            aPosAry.mnDestY = aClipRegionBounds.Top();
                            aPosAry.mnDestWidth = aClipRegionBounds.GetWidth();
                            aPosAry.mnDestHeight = aClipRegionBounds.GetHeight();
                        }
                    }

                    mpGraphics->DrawBitmap(aPosAry, *pSalSrcBmp,
                                           *pMaskBmp->ImplGetSalBitmap(),
                                           this);
                }

                // #110958# Paint mask to alpha channel. Luckily, the
                // black and white representation of the mask maps to
                // the alpha channel

                // #i25167# Restrict mask painting to _opaque_ areas
                // of the mask, otherwise we spoil areas where no
                // bitmap content was ever visible. Interestingly
                // enough, this can be achieved by taking the mask as
                // the transparency mask of itself
                if (mpAlphaVDev)
                    mpAlphaVDev->DrawBitmapEx(rDestPt,
                                              rDestSize,
                                              BitmapEx(rBitmapEx.GetMask(),
                                                       rBitmapEx.GetMask()));
            }
            else
            {
                mpGraphics->DrawBitmap(aPosAry, *pSalSrcBmp, this);

                if (mpAlphaVDev)
                {
                    // #i32109#: Make bitmap area opaque
                    mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
                }
            }
        }
    }
}

void OutputDevice::DrawDeviceAlphaBitmap( const Bitmap& rBmp, const AlphaMask& rAlpha,
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aOutPt( LogicToPixel( rDestPt ) );
    Size        aOutSz( LogicToPixel( rDestSize ) );
    Rectangle   aDstRect( Point(), GetOutputSizePixel() );
    const bool  bHMirr = aOutSz.Width() < 0;
    const bool  bVMirr = aOutSz.Height() < 0;

    ClipToPaintRegion(aDstRect);

    if( bHMirr )
    {
        aOutSz.Width() = -aOutSz.Width();
        aOutPt.X() -= ( aOutSz.Width() - 1L );
    }

    if( bVMirr )
    {
        aOutSz.Height() = -aOutSz.Height();
        aOutPt.Y() -= ( aOutSz.Height() - 1L );
    }

    if( !aDstRect.Intersection( Rectangle( aOutPt, aOutSz ) ).IsEmpty() )
    {
        bool bNativeAlpha = false;
        static const char* pDisableNative = getenv( "SAL_DISABLE_NATIVE_ALPHA");
        // #i83087# Naturally, system alpha blending cannot work with
        // separate alpha VDev
        bool bTryDirectPaint(!mpAlphaVDev && !pDisableNative && !bHMirr && !bVMirr);

#ifdef WNT
        if(bTryDirectPaint)
        {
            // only paint direct when no scaling and no MapMode, else the
            // more expensive conversions may be done for short-time Bitmap/BitmapEx
            // used for buffering only
            if(!IsMapMode() && rSrcSizePixel.Width() == aOutSz.Width() && rSrcSizePixel.Height() == aOutSz.Height())
            {
                bTryDirectPaint = false;
            }
        }
#endif

        if(bTryDirectPaint)
        {
            Point aRelPt = aOutPt + Point( mnOutOffX, mnOutOffY );
            SalTwoRect aTR = {
                rSrcPtPixel.X(), rSrcPtPixel.Y(),
                rSrcSizePixel.Width(), rSrcSizePixel.Height(),
                aRelPt.X(), aRelPt.Y(),
                aOutSz.Width(), aOutSz.Height()
            };
            SalBitmap* pSalSrcBmp = rBmp.ImplGetImpBitmap()->ImplGetSalBitmap();
            SalBitmap* pSalAlphaBmp = rAlpha.ImplGetImpBitmap()->ImplGetSalBitmap();
            bNativeAlpha = mpGraphics->DrawAlphaBitmap( aTR, *pSalSrcBmp, *pSalAlphaBmp, this );
        }

        VirtualDevice* pOldVDev = mpAlphaVDev;

        Rectangle aBmpRect( Point(), rBmp.GetSizePixel() );
        if( !bNativeAlpha
                &&  !aBmpRect.Intersection( Rectangle( rSrcPtPixel, rSrcSizePixel ) ).IsEmpty() )
        {
            // The scaling in this code path produces really ugly results - it
            // does the most trivial scaling with no smoothing.

            GDIMetaFile*    pOldMetaFile = mpMetaFile;
            const bool      bOldMap = mbMap;
            mpMetaFile = NULL; // fdo#55044 reset before GetBitmap!
            mbMap = false;
            Bitmap          aBmp( GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );

            // #109044# The generated bitmap need not necessarily be
            // of aDstRect dimensions, it's internally clipped to
            // window bounds. Thus, we correct the dest size here,
            // since we later use it (in nDstWidth/Height) for pixel
            // access)
            // #i38887# reading from screen may sometimes fail
            if( aBmp.ImplGetImpBitmap() )
                aDstRect.SetSize( aBmp.GetSizePixel() );

            BitmapColor     aDstCol;
            const long      nSrcWidth = aBmpRect.GetWidth(), nSrcHeight = aBmpRect.GetHeight();
            const long      nDstWidth = aDstRect.GetWidth(), nDstHeight = aDstRect.GetHeight();
            const long      nOutWidth = aOutSz.Width(), nOutHeight = aOutSz.Height();
            // calculate offset in original bitmap
            // in RTL case this is a little more complicated since the contents of the
            // bitmap is not mirrored (it never is), however the paint region and bmp region
            // are in mirrored coordinates, so the intersection of (aOutPt,aOutSz) with these
            // is content wise somewhere else and needs to take mirroring into account
            const long      nOffX = IsRTLEnabled()
                                    ? aOutSz.Width() - aDstRect.GetWidth() - (aDstRect.Left() - aOutPt.X())
                                    : aDstRect.Left() - aOutPt.X(),
                            nOffY = aDstRect.Top() - aOutPt.Y();
            long            nX, nOutX, nY, nOutY;
            long            nMirrOffX = 0;
            long            nMirrOffY = 0;
            boost::scoped_array<long> pMapX(new long[ nDstWidth ]);
            boost::scoped_array<long> pMapY(new long[ nDstHeight ]);

            // create horizontal mapping table
            if( bHMirr )
                nMirrOffX = ( aBmpRect.Left() << 1 ) + nSrcWidth - 1;

            for( nX = 0L, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
            {
                pMapX[ nX ] = aBmpRect.Left() + nOutX * nSrcWidth / nOutWidth;
                if( bHMirr )
                    pMapX[ nX ] = nMirrOffX - pMapX[ nX ];
            }

            // create vertical mapping table
            if( bVMirr )
                nMirrOffY = ( aBmpRect.Top() << 1 ) + nSrcHeight - 1;

            for( nY = 0L, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                pMapY[ nY ] = aBmpRect.Top() + nOutY * nSrcHeight / nOutHeight;

                if( bVMirr )
                    pMapY[ nY ] = nMirrOffY - pMapY[ nY ];
            }

            BitmapReadAccess*   pP = ( (Bitmap&) rBmp ).AcquireReadAccess();
            BitmapReadAccess*   pA = ( (AlphaMask&) rAlpha ).AcquireReadAccess();

            DBG_ASSERT( pA->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                        pA->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                        "OutputDevice::ImplDrawAlpha(): non-8bit alpha no longer supported!" );

            // #i38887# reading from screen may sometimes fail
            if( aBmp.ImplGetImpBitmap() )
            {
                Bitmap aTmp;

                if( mpAlphaVDev )
                {
                    aTmp = BlendBitmapWithAlpha(
                        aBmp,pP,pA,
                        aDstRect,
                        nOffY,nDstHeight,
                        nOffX,nDstWidth,
                        pMapX.get(),pMapY.get() );
                }
                else
                {
                    aTmp = BlendBitmap(
                        aBmp,pP,pA,
                        nOffY,nDstHeight,
                        nOffX,nDstWidth,
                        aBmpRect,aOutSz,
                        bHMirr,bVMirr,
                        pMapX.get(),pMapY.get() );
                }

                // #110958# Disable alpha VDev, we're doing the necessary
                // stuff explicitly furher below
                if( mpAlphaVDev )
                    mpAlphaVDev = NULL;

                DrawBitmap( aDstRect.TopLeft(),
                            aTmp );

                // #110958# Enable alpha VDev again
                mpAlphaVDev = pOldVDev;
            }

            ( (Bitmap&) rBmp ).ReleaseAccess( pP );
            ( (AlphaMask&) rAlpha ).ReleaseAccess( pA );

            mbMap = bOldMap;
            mpMetaFile = pOldMetaFile;
        }
    }
}

void OutputDevice::ScaleBitmap (Bitmap &rBmp, SalTwoRect &rPosAry)
{
    const double nScaleX = rPosAry.mnDestWidth  / static_cast<double>( rPosAry.mnSrcWidth );
    const double nScaleY = rPosAry.mnDestHeight / static_cast<double>( rPosAry.mnSrcHeight );

    // If subsampling, use Bitmap::Scale for subsampling for better quality.
    if ( nScaleX < 1.0 || nScaleY < 1.0 )
    {
        rBmp.Scale ( nScaleX, nScaleY );
        rPosAry.mnSrcWidth = rPosAry.mnDestWidth;
        rPosAry.mnSrcHeight = rPosAry.mnDestHeight;
    }
}

bool OutputDevice::DrawTransformBitmapExDirect(
        const basegfx::B2DHomMatrix& aFullTransform,
        const BitmapEx& rBitmapEx)
{
    bool bDone = false;

    // try to paint directly
    const basegfx::B2DPoint aNull(aFullTransform * basegfx::B2DPoint(0.0, 0.0));
    const basegfx::B2DPoint aTopX(aFullTransform * basegfx::B2DPoint(1.0, 0.0));
    const basegfx::B2DPoint aTopY(aFullTransform * basegfx::B2DPoint(0.0, 1.0));
    SalBitmap* pSalSrcBmp = rBitmapEx.GetBitmap().ImplGetImpBitmap()->ImplGetSalBitmap();
    SalBitmap* pSalAlphaBmp = 0;

    if(rBitmapEx.IsTransparent())
    {
        if(rBitmapEx.IsAlpha())
        {
            pSalAlphaBmp = rBitmapEx.GetAlpha().ImplGetImpBitmap()->ImplGetSalBitmap();
        }
        else
        {
            pSalAlphaBmp = rBitmapEx.GetMask().ImplGetImpBitmap()->ImplGetSalBitmap();
        }
    }

    bDone = mpGraphics->DrawTransformedBitmap(
        aNull,
        aTopX,
        aTopY,
        *pSalSrcBmp,
        pSalAlphaBmp,
        this);

    return bDone;
};

bool OutputDevice::TransformAndReduceBitmapExToTargetRange(
        const basegfx::B2DHomMatrix& aFullTransform,
        basegfx::B2DRange &aVisibleRange,
        double &fMaximumArea)
{
    // limit TargetRange to existing pixels (if pixel device)
    // first get discrete range of object
    basegfx::B2DRange aFullPixelRange(aVisibleRange);

    aFullPixelRange.transform(aFullTransform);

    if(basegfx::fTools::equalZero(aFullPixelRange.getWidth()) || basegfx::fTools::equalZero(aFullPixelRange.getHeight()))
    {
        // object is outside of visible area
        return false;
    }

    // now get discrete target pixels; start with OutDev pixel size and evtl.
    // intersect with active clipping area
    basegfx::B2DRange aOutPixel(
        0.0,
        0.0,
        GetOutputSizePixel().Width(),
        GetOutputSizePixel().Height());

    if(IsClipRegion())
    {
        const Rectangle aRegionRectangle(GetActiveClipRegion().GetBoundRect());

        aOutPixel.intersect( // caution! Range from rectangle, one too much (!)
            basegfx::B2DRange(
                aRegionRectangle.Left(),
                aRegionRectangle.Top(),
                aRegionRectangle.Right() + 1,
                aRegionRectangle.Bottom() + 1));
    }

    if(aOutPixel.isEmpty())
    {
        // no active output area
        return false;
    }

    // if aFullPixelRange is not completely inside of aOutPixel,
    // reduction of target pixels is possible
    basegfx::B2DRange aVisiblePixelRange(aFullPixelRange);

    if(!aOutPixel.isInside(aFullPixelRange))
    {
        aVisiblePixelRange.intersect(aOutPixel);

        if(aVisiblePixelRange.isEmpty())
        {
            // nothing in visible part, reduces to nothing
            return false;
        }

        // aVisiblePixelRange contains the reduced output area in
        // discrete coordinates. To make it useful everywhere, make it relative to
        // the object range
        basegfx::B2DHomMatrix aMakeVisibleRangeRelative;

        aVisibleRange = aVisiblePixelRange;
        aMakeVisibleRangeRelative.translate(
            -aFullPixelRange.getMinX(),
            -aFullPixelRange.getMinY());
        aMakeVisibleRangeRelative.scale(
            1.0 / aFullPixelRange.getWidth(),
            1.0 / aFullPixelRange.getHeight());
        aVisibleRange.transform(aMakeVisibleRangeRelative);
    }

    // for pixel devices, do *not* limit size, else OutputDevice::DrawDeviceAlphaBitmap
    // will create another, badly scaled bitmap to do the job. Nonetheless, do a
    // maximum clipping of something big (1600x1280x2). Add 1.0 to avoid rounding
    // errors in rough estimations
    const double fNewMaxArea(aVisiblePixelRange.getWidth() * aVisiblePixelRange.getHeight());

    fMaximumArea = std::min(4096000.0, fNewMaxArea + 1.0);

    return true;
}

void OutputDevice::DrawTransformedBitmapEx(
    const basegfx::B2DHomMatrix& rTransformation,
    const BitmapEx& rBitmapEx)
{
    if( ImplIsRecordLayout() )
        return;

    if(rBitmapEx.IsEmpty())
        return;

    if ( mnDrawMode & DRAWMODE_NOBITMAP )
        return;

    // decompose matrix to check rotation and shear
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
    const bool bRotated(!basegfx::fTools::equalZero(fRotate));
    const bool bSheared(!basegfx::fTools::equalZero(fShearX));
    const bool bMirroredX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirroredY(basegfx::fTools::less(aScale.getY(), 0.0));
    static bool bForceToOwnTransformer(false);

    if(!bForceToOwnTransformer && !bRotated && !bSheared && !bMirroredX && !bMirroredY)
    {
        // with no rotation, shear or mirroring it can be mapped to DrawBitmapEx
        // do *not* execute the mirroring here, it's done in the fallback
        // #i124580# the correct DestSize needs to be calculated based on MaxXY values
        const Point aDestPt(basegfx::fround(aTranslate.getX()), basegfx::fround(aTranslate.getY()));
        const Size aDestSize(
            basegfx::fround(aScale.getX() + aTranslate.getX()) - aDestPt.X(),
            basegfx::fround(aScale.getY() + aTranslate.getY()) - aDestPt.Y());

        DrawBitmapEx(aDestPt, aDestSize, rBitmapEx);
        return;
    }

    // we have rotation,shear or mirror, check if some crazy mode needs the
    // created transformed bitmap
    const bool bInvert(ROP_INVERT == meRasterOp);
    const bool bBitmapChangedColor(mnDrawMode & (DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP | DRAWMODE_GRAYBITMAP | DRAWMODE_GHOSTEDBITMAP));
    const bool bMetafile(mpMetaFile);
    bool bDone(false);
    const basegfx::B2DHomMatrix aFullTransform(GetViewTransformation() * rTransformation);
    const bool bTryDirectPaint(!bInvert && !bBitmapChangedColor && !bMetafile );

    if(!bForceToOwnTransformer && bTryDirectPaint)
    {
        bDone = DrawTransformBitmapExDirect(aFullTransform, rBitmapEx);
    }

    if(!bDone)
    {
        // take the fallback when no rotate and shear, but mirror (else we would have done this above)
        if(!bForceToOwnTransformer && !bRotated && !bSheared)
        {
            // with no rotation or shear it can be mapped to DrawBitmapEx
            // do *not* execute the mirroring here, it's done in the fallback
            // #i124580# the correct DestSize needs to be calculated based on MaxXY values
            const Point aDestPt(basegfx::fround(aTranslate.getX()), basegfx::fround(aTranslate.getY()));
            const Size aDestSize(
                basegfx::fround(aScale.getX() + aTranslate.getX()) - aDestPt.X(),
                basegfx::fround(aScale.getY() + aTranslate.getY()) - aDestPt.Y());

            DrawBitmapEx(aDestPt, aDestSize, rBitmapEx);
            return;
        }

        // fallback; create transformed bitmap the hard way (back-transform
        // the pixels) and paint
        basegfx::B2DRange aVisibleRange(0.0, 0.0, 1.0, 1.0);

        // limit maximum area to something looking good for non-pixel-based targets (metafile, printer)
        // by using a fixed minimum (allow at least, but no need to utilize) for good smooting and an area
        // dependent of original size for good quality when e.g. rotated/sheared. Still, limit to a maximum
        // to avoid crashes/ressource problems (ca. 1500x3000 here)
        const Size& rOriginalSizePixel(rBitmapEx.GetSizePixel());
        const double fOrigArea(rOriginalSizePixel.Width() * rOriginalSizePixel.Height() * 0.5);
        const double fOrigAreaScaled(bSheared || bRotated ? fOrigArea * 1.44 : fOrigArea);
        double fMaximumArea(std::min(4500000.0, std::max(1000000.0, fOrigAreaScaled)));

        if(!bMetafile)
        {
            if ( !TransformAndReduceBitmapExToTargetRange( aFullTransform, aVisibleRange, fMaximumArea ) )
                return;
        }

        if(!aVisibleRange.isEmpty())
        {
            static bool bDoSmoothAtAll(true);
            BitmapEx aTransformed(rBitmapEx);

            // #122923# when the result needs an alpha channel due to being rotated or sheared
            // and thus uncovering areas, add these channels so that the own transformer (used
            // in getTransformed) also creates a transformed alpha channel
            if(!aTransformed.IsTransparent() && (bSheared || bRotated))
            {
                // parts will be uncovered, extend aTransformed with a mask bitmap
                const Bitmap aContent(aTransformed.GetBitmap());

                AlphaMask aMaskBmp(aContent.GetSizePixel());
                aMaskBmp.Erase(0);

                aTransformed = BitmapEx(aContent, aMaskBmp);
            }

            aTransformed = aTransformed.getTransformed(
                aFullTransform,
                aVisibleRange,
                fMaximumArea,
                bDoSmoothAtAll);
            basegfx::B2DRange aTargetRange(0.0, 0.0, 1.0, 1.0);

            // get logic object target range
            aTargetRange.transform(rTransformation);

            // get from unified/relative VisibleRange to logoc one
            aVisibleRange.transform(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aTargetRange.getRange(),
                    aTargetRange.getMinimum()));

            // extract point and size; do not remove size, the bitmap may have been prepared reduced by purpose
            // #i124580# the correct DestSize needs to be calculated based on MaxXY values
            const Point aDestPt(basegfx::fround(aVisibleRange.getMinX()), basegfx::fround(aVisibleRange.getMinY()));
            const Size aDestSize(
                basegfx::fround(aVisibleRange.getMaxX()) - aDestPt.X(),
                basegfx::fround(aVisibleRange.getMaxY()) - aDestPt.Y());

            DrawBitmapEx(aDestPt, aDestSize, aTransformed);
        }
    }
}

namespace
{
    BitmapEx makeDisabledBitmap(const Bitmap &rBitmap)
    {
        const Size aTotalSize( rBitmap.GetSizePixel() );
        Bitmap aGrey( aTotalSize, 8, &Bitmap::GetGreyPalette( 256 ) );
        AlphaMask aGreyAlphaMask( aTotalSize );
        BitmapReadAccess*  pBmp = const_cast<Bitmap&>(rBitmap).AcquireReadAccess();
        BitmapWriteAccess* pGrey = aGrey.AcquireWriteAccess();
        BitmapWriteAccess* pGreyAlphaMask = aGreyAlphaMask.AcquireWriteAccess();

        if( pBmp && pGrey && pGreyAlphaMask )
        {
            BitmapColor aGreyVal( 0 );
            BitmapColor aGreyAlphaMaskVal( 0 );
            const int nLeft = 0, nRight = aTotalSize.Width();
            const int nTop = 0, nBottom = nTop + aTotalSize.Height();

            for( int nY = nTop; nY < nBottom; ++nY )
            {
                for( int nX = nLeft; nX < nRight; ++nX )
                {
                    aGreyVal.SetIndex( pBmp->GetLuminance( nY, nX ) );
                    pGrey->SetPixel( nY, nX, aGreyVal );

                    aGreyAlphaMaskVal.SetIndex( static_cast< sal_uInt8 >( 128ul ) );
                    pGreyAlphaMask->SetPixel( nY, nX, aGreyAlphaMaskVal );
                }
            }
        }

        const_cast<Bitmap&>(rBitmap).ReleaseAccess( pBmp );
        aGrey.ReleaseAccess( pGrey );
        aGreyAlphaMask.ReleaseAccess( pGreyAlphaMask );
        return BitmapEx( aGrey, aGreyAlphaMask );
    }
}

void OutputDevice::DrawImage( const Point& rPos, const Image& rImage, sal_uInt16 nStyle )
{
    DrawImage( rPos, Size(), rImage, nStyle );
}

void OutputDevice::DrawImage( const Point& rPos, const Size& rSize,
                              const Image& rImage, sal_uInt16 nStyle )
{
    bool bIsSizeValid = (rSize.getWidth() == 0 || rSize.getHeight() == 0) ? false : true;

    if( rImage.mpImplData && !ImplIsRecordLayout() )
    {
        switch( rImage.mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
            {
                const Bitmap &rBitmap = *static_cast< Bitmap* >( rImage.mpImplData->mpData );
                if( nStyle & IMAGE_DRAW_DISABLE )
                {
                    if ( bIsSizeValid )
                        DrawBitmapEx( rPos, rSize, makeDisabledBitmap(rBitmap) );
                    else
                        DrawBitmapEx( rPos, makeDisabledBitmap(rBitmap) );
                }
                else
                {
                    if ( bIsSizeValid )
                        DrawBitmap( rPos, rSize, rBitmap );
                    else
                        DrawBitmap( rPos, rBitmap );
                }
            }
            break;

            case IMAGETYPE_IMAGE:
            {
                ImplImageData* pData = static_cast< ImplImageData* >( rImage.mpImplData->mpData );

                if ( !pData->mpImageBitmap )
                {
                    const Size aSize( pData->maBmpEx.GetSizePixel() );

                    pData->mpImageBitmap = new ImplImageBmp;
                    pData->mpImageBitmap->Create( pData->maBmpEx, aSize.Width(), aSize.Height(), 1 );
                }

                if ( bIsSizeValid )
                    pData->mpImageBitmap->Draw( 0, this, rPos, nStyle, &rSize );
                else
                    pData->mpImageBitmap->Draw( 0, this, rPos, nStyle );
            }
            break;

            default:
            break;
        }
    }
}

namespace
{
    // Co = Cs + Cd*(1-As) premultiplied alpha -or-
    // Co = (AsCs + AdCd*(1-As)) / Ao
    inline sal_uInt8 CalcColor( const sal_uInt8 nSourceColor, const sal_uInt8 nSourceAlpha,
                                const sal_uInt8 nDstAlpha, const sal_uInt8 nResAlpha, const sal_uInt8 nDestColor )
    {
        int c = nResAlpha ? ( (int)nSourceAlpha*nSourceColor + (int)nDstAlpha*nDestColor -
                              (int)nDstAlpha*nDestColor*nSourceAlpha/255 ) / (int)nResAlpha : 0;
        return sal_uInt8( c );
    }

    inline BitmapColor AlphaBlend( int nX,               int nY,
                                   const long            nMapX,
                                   const long            nMapY,
                                   BitmapReadAccess*     pP,
                                   BitmapReadAccess*     pA,
                                   BitmapReadAccess*     pB,
                                   BitmapWriteAccess*    pAlphaW,
                                   sal_uInt8&            nResAlpha )
    {
        BitmapColor aDstCol,aSrcCol;
        aSrcCol = pP->GetColor( nMapY, nMapX );
        aDstCol = pB->GetColor( nY, nX );

        // vcl stores transparency, not alpha - invert it
        const sal_uInt8 nSrcAlpha = 255 - pA->GetPixelIndex( nMapY, nMapX );
        const sal_uInt8 nDstAlpha = 255 - pAlphaW->GetPixelIndex( nY, nX );

        // Perform porter-duff compositing 'over' operation

        // Co = Cs + Cd*(1-As)
        // Ad = As + Ad*(1-As)
        nResAlpha = (int)nSrcAlpha + (int)nDstAlpha - (int)nDstAlpha*nSrcAlpha/255;

        aDstCol.SetRed( CalcColor( aSrcCol.GetRed(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetRed() ) );
        aDstCol.SetBlue( CalcColor( aSrcCol.GetBlue(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetBlue() ) );
        aDstCol.SetGreen( CalcColor( aSrcCol.GetGreen(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetGreen() ) );

        return aDstCol;
    }
}

Bitmap OutputDevice::BlendBitmapWithAlpha(
            Bitmap&             aBmp,
            BitmapReadAccess*   pP,
            BitmapReadAccess*   pA,
            const Rectangle&    aDstRect,
            const sal_Int32     nOffY,
            const sal_Int32     nDstHeight,
            const sal_Int32     nOffX,
            const sal_Int32     nDstWidth,
            const long*         pMapX,
            const long*         pMapY )

{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;
    sal_uInt8   nResAlpha;

    SAL_WARN_IF( !mpAlphaVDev, "vcl.gdi", "BlendBitmapWithAlpha(): call me only with valid alpha VirtualDevice!" );

    bool bOldMapMode( mpAlphaVDev->IsMapModeEnabled() );
    mpAlphaVDev->EnableMapMode(false);

    Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );
    BitmapWriteAccess*  pAlphaW = aAlphaBitmap.AcquireWriteAccess();

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
        BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

        if( pB && pP && pA && pW && pAlphaW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4L;
                int nOutX;

                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB, pAlphaW, nResAlpha );

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                    pW->SetPixel( nY, nX, aIndex );

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] +
                                                   nVCLGLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] +
                                                   nVCLBLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] ) );
                    pAlphaW->SetPixel( nY, nX, aIndex );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        aDither.ReleaseAccess( pW );
        res = aDither;
    }
    else
    {
        BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();
        if( pP && pA && pB )
        {
            for( nY = 0; nY < nDstHeight; nY++ )
            {
                const long  nMapY = pMapY[ nY ];

                for( nX = 0; nX < nDstWidth; nX++ )
                {
                    const long nMapX = pMapX[ nX ];
                    aDstCol = AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB, pAlphaW, nResAlpha );

                    pB->SetPixel( nY, nX, aDstCol );
                    pAlphaW->SetPixel( nY, nX, Color(255L-nResAlpha, 255L-nResAlpha, 255L-nResAlpha) );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        res = aBmp;
    }

    aAlphaBitmap.ReleaseAccess( pAlphaW );
    mpAlphaVDev->DrawBitmap( aDstRect.TopLeft(), aAlphaBitmap );
    mpAlphaVDev->EnableMapMode( bOldMapMode );

    return res;
}

Bitmap OutputDevice::BlendBitmap(
            Bitmap&             aBmp,
            BitmapReadAccess*   pP,
            BitmapReadAccess*   pA,
            const sal_Int32     nOffY,
            const sal_Int32     nDstHeight,
            const sal_Int32     nOffX,
            const sal_Int32     nDstWidth,
            const Rectangle&    aBmpRect,
            const Size&         aOutSz,
            const bool          bHMirr,
            const bool          bVMirr,
            const long*         pMapX,
            const long*         pMapY )
{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
        BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

        if( pB && pP && pA && pW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4L;
                int nOutX;

                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = pB->GetColor( nY, nX );
                    aDstCol.Merge( pP->GetColor( nMapY, nMapX ), pA->GetPixelIndex( nMapY, nMapX ) );
                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                    pW->SetPixel( nY, nX, aIndex );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        aDither.ReleaseAccess( pW );
        res = aDither;
    }
    else
    {
        BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();

        bool bFastBlend = false;
        if( pP && pA && pB )
        {
            SalTwoRect aTR;
            aTR.mnSrcX      = aBmpRect.Left();
            aTR.mnSrcY      = aBmpRect.Top();
            aTR.mnSrcWidth  = aBmpRect.GetWidth();
            aTR.mnSrcHeight = aBmpRect.GetHeight();
            aTR.mnDestX     = nOffX;
            aTR.mnDestY     = nOffY;
            aTR.mnDestWidth = aOutSz.Width();
            aTR.mnDestHeight= aOutSz.Height();

            if( !bHMirr && !bVMirr )
                bFastBlend = ImplFastBitmapBlending( *pB,*pP,*pA, aTR );
        }

        if( pP && pA && pB && !bFastBlend )
        {
            switch( pP->GetScanlineFormat() )
            {
                case( BMP_FORMAT_8BIT_PAL ):
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long  nMapY = pMapY[ nY ];
                            Scanline    pPScan = pP->GetScanline( nMapY );
                            Scanline    pAScan = pA->GetScanline( nMapY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long nMapX = pMapX[ nX ];
                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetPaletteColor( pPScan[ nMapX ] ),
                                                                     pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                case( BMP_FORMAT_24BIT_TC_BGR ):
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long  nMapY = pMapY[ nY ];
                            Scanline    pPScan = pP->GetScanline( nMapY );
                            Scanline    pAScan = pA->GetScanline( nMapY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long  nMapX = pMapX[ nX ];
                                Scanline    pTmp = pPScan + nMapX * 3;

                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pTmp[ 2 ], pTmp[ 1 ], pTmp[ 0 ],
                                                                     pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                case( BMP_FORMAT_24BIT_TC_RGB ):
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long  nMapY = pMapY[ nY ];
                            Scanline    pPScan = pP->GetScanline( nMapY );
                            Scanline    pAScan = pA->GetScanline( nMapY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long  nMapX = pMapX[ nX ];
                                Scanline    pTmp = pPScan + nMapX * 3;

                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pTmp[ 0 ], pTmp[ 1 ], pTmp[ 2 ],
                                                                     pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                default:
                {
                    for( nY = 0; nY < nDstHeight; nY++ )
                    {
                        const long  nMapY = pMapY[ nY ];
                        Scanline    pAScan = pA->GetScanline( nMapY );

                        for( nX = 0; nX < nDstWidth; nX++ )
                        {
                            const long nMapX = pMapX[ nX ];
                            aDstCol = pB->GetPixel( nY, nX );
                            pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetColor( nMapY, nMapX ),
                                                                 pAScan[ nMapX ] ) );
                        }
                    }
                }
                break;
            }
        }

        aBmp.ReleaseAccess( pB );
        res = aBmp;
    }

    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
