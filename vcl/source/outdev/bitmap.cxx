/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <cassert>
#include <cstdlib>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapFilterStackBlur.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <config_features.h>
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/bmpfast.hxx>
#include <salgdi.hxx>
#include <salbmp.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <memory>
#include <comphelper/lok.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/helpers.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <o3tl/unit_conversion.hxx>

#include <vcl/dibtools.hxx>
#include <tools/stream.hxx>

void OutputDevice::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    assert(!is_double_buffered_window());

    const Size aSizePix( rBitmap.GetSizePixel() );
    DrawBitmap( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmap, MetaActionType::BMP );
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    assert(!is_double_buffered_window());

    DrawBitmap( rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, MetaActionType::BMPSCALE );
}


void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                   const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                   const Bitmap& rBitmap, const MetaActionType nAction )
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if ( RasterOp::Invert == meRasterOp )
    {
        DrawRect( tools::Rectangle( rDestPt, rDestSize ) );
        return;
    }

    Bitmap aBmp( rBitmap );

    if ( mnDrawMode & ( DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap |
                             DrawModeFlags::GrayBitmap ) )
    {
        if ( mnDrawMode & ( DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap ) )
        {
            sal_uInt8 cCmpVal;

            if ( mnDrawMode & DrawModeFlags::BlackBitmap )
                cCmpVal = 0;
            else
                cCmpVal = 255;

            Color aCol( cCmpVal, cCmpVal, cCmpVal );
            Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
            SetLineColor( aCol );
            SetFillColor( aCol );
            DrawRect( tools::Rectangle( rDestPt, rDestSize ) );
            Pop();
            return;
        }
        else if( !!aBmp )
        {
            if ( mnDrawMode & DrawModeFlags::GrayBitmap )
                aBmp.Convert( BmpConversion::N8BitGreys );
        }
    }

    if ( mpMetaFile )
    {
        switch( nAction )
        {
            case MetaActionType::BMP:
                mpMetaFile->AddAction( new MetaBmpAction( rDestPt, aBmp ) );
            break;

            case MetaActionType::BMPSCALE:
                mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
            break;

            case MetaActionType::BMPSCALEPART:
                mpMetaFile->AddAction( new MetaBmpScalePartAction(
                    rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp ) );
            break;

            default: break;
        }
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if( !aBmp.IsEmpty() )
    {
        SalTwoRect aPosAry(rSrcPtPixel.X(), rSrcPtPixel.Y(), rSrcSizePixel.Width(), rSrcSizePixel.Height(),
                           ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                           ImplLogicWidthToDevicePixel(rDestSize.Width()),
                           ImplLogicHeightToDevicePixel(rDestSize.Height()));

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
        {
            const BmpMirrorFlags nMirrFlags = AdjustTwoRect( aPosAry, aBmp.GetSizePixel() );

            if ( nMirrFlags != BmpMirrorFlags::NONE )
                aBmp.Mirror( nMirrFlags );

            if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            {
                if (nAction == MetaActionType::BMPSCALE && CanSubsampleBitmap())
                {
                    const double nScaleX = aPosAry.mnDestWidth  / static_cast<double>(aPosAry.mnSrcWidth);
                    const double nScaleY = aPosAry.mnDestHeight / static_cast<double>(aPosAry.mnSrcHeight);

                    // If subsampling, use Bitmap::Scale() for subsampling of better quality.
                    if ( nScaleX < 1.0 || nScaleY < 1.0 )
                    {
                        aBmp.Scale(nScaleX, nScaleY);
                        aPosAry.mnSrcWidth = aPosAry.mnDestWidth;
                        aPosAry.mnSrcHeight = aPosAry.mnDestHeight;
                    }
                }

                mpGraphics->DrawBitmap( aPosAry, *aBmp.ImplGetSalBitmap(), *this );
            }
        }
    }

    if( mpAlphaVDev )
    {
        // #i32109#: Make bitmap area opaque
        mpAlphaVDev->ImplFillOpaqueRectangle( tools::Rectangle(rDestPt, rDestSize) );
    }
}

Bitmap OutputDevice::GetDownsampledBitmap( const Size& rDstSz,
                                           const Point& rSrcPt, const Size& rSrcSz,
                                           const Bitmap& rBmp, tools::Long nMaxBmpDPIX, tools::Long nMaxBmpDPIY )
{
    Bitmap aBmp( rBmp );

    if( !aBmp.IsEmpty() )
    {
        const tools::Rectangle aBmpRect( Point(), aBmp.GetSizePixel() );
        tools::Rectangle       aSrcRect( rSrcPt, rSrcSz );

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
            Size aDstSizeTwip( PixelToLogic(LogicToPixel(rDstSz), MapMode(MapUnit::MapTwip)) );

            // #103209# Normalize size (mirroring has to happen outside of this method)
            aDstSizeTwip = Size( std::abs(aDstSizeTwip.Width()), std::abs(aDstSizeTwip.Height()) );

            const Size      aBmpSize( aBmp.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double fMaxPixelX
                = o3tl::convert<double>(aDstSizeTwip.Width(), o3tl::Length::twip, o3tl::Length::in)
                  * nMaxBmpDPIX;
            const double fMaxPixelY
                = o3tl::convert<double>(aDstSizeTwip.Height(), o3tl::Length::twip, o3tl::Length::in)
                  * nMaxBmpDPIY;

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
                    aNewBmpSize.setWidth( FRound( fMaxPixelY * fBmpWH ) );
                    aNewBmpSize.setHeight( FRound( fMaxPixelY ) );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.setWidth( FRound( fMaxPixelX ) );
                    aNewBmpSize.setHeight( FRound( fMaxPixelX / fBmpWH) );
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
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if( TransparentType::NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rBitmapEx.GetBitmap() );
    }
    else
    {
        const Size aSizePix( rBitmapEx.GetSizePixel() );
        DrawBitmapEx( rDestPt, PixelToLogic( aSizePix ), Point(), aSizePix, rBitmapEx, MetaActionType::BMPEX );
    }
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const BitmapEx& rBitmapEx )
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if ( TransparentType::NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rDestSize, rBitmapEx.GetBitmap() );
    }
    else
    {
        DrawBitmapEx( rDestPt, rDestSize, Point(), rBitmapEx.GetSizePixel(), rBitmapEx, MetaActionType::BMPEXSCALE );
    }
}


void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const BitmapEx& rBitmapEx, const MetaActionType nAction )
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if( TransparentType::NONE == rBitmapEx.GetTransparentType() )
    {
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx.GetBitmap() );
    }
    else
    {
        if ( RasterOp::Invert == meRasterOp )
        {
            DrawRect( tools::Rectangle( rDestPt, rDestSize ) );
            return;
        }

        BitmapEx aBmpEx( rBitmapEx );

        if ( mnDrawMode & ( DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap |
                                 DrawModeFlags::GrayBitmap ) )
        {
            if ( mnDrawMode & ( DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap ) )
            {
                Bitmap  aColorBmp( aBmpEx.GetSizePixel(), 1 );
                sal_uInt8   cCmpVal;

                if ( mnDrawMode & DrawModeFlags::BlackBitmap )
                    cCmpVal = 0;
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
                    BitmapEx aMaskEx(aBmpEx.GetAlpha().GetBitmap());
                    BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(129));
                    aBmpEx = BitmapEx(aColorBmp, aMaskEx.GetBitmap());
                }
                else
                {
                    aBmpEx = BitmapEx( aColorBmp, aBmpEx.GetMask() );
                }
            }
            else if( !!aBmpEx )
            {
                if ( mnDrawMode & DrawModeFlags::GrayBitmap )
                    aBmpEx.Convert( BmpConversion::N8BitGreys );
            }
        }

        if ( mpMetaFile )
        {
            switch( nAction )
            {
                case MetaActionType::BMPEX:
                    mpMetaFile->AddAction( new MetaBmpExAction( rDestPt, aBmpEx ) );
                break;

                case MetaActionType::BMPEXSCALE:
                    mpMetaFile->AddAction( new MetaBmpExScaleAction( rDestPt, rDestSize, aBmpEx ) );
                break;

                case MetaActionType::BMPEXSCALEPART:
                    mpMetaFile->AddAction( new MetaBmpExScalePartAction( rDestPt, rDestSize,
                                                                         rSrcPtPixel, rSrcSizePixel, aBmpEx ) );
                break;

                default: break;
            }
        }

        if ( !IsDeviceOutputNecessary() )
            return;

        if ( !mpGraphics && !AcquireGraphics() )
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
    tools::Long    nX = ImplLogicXToDevicePixel( rSrcPt.X() );
    tools::Long    nY = ImplLogicYToDevicePixel( rSrcPt.Y() );
    tools::Long    nWidth = ImplLogicWidthToDevicePixel( rSize.Width() );
    tools::Long    nHeight = ImplLogicHeightToDevicePixel( rSize.Height() );

    if ( mpGraphics || AcquireGraphics() )
    {
        assert(mpGraphics);

        if ( nWidth > 0 && nHeight  > 0 && nX <= (mnOutWidth + mnOutOffX) && nY <= (mnOutHeight + mnOutOffY))
        {
            tools::Rectangle   aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
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
                ScopedVclPtrInstance< VirtualDevice > aVDev(  *this  );

                if ( aVDev->SetOutputSizePixel( aRect.GetSize() ) )
                {
                    if ( aVDev->mpGraphics || aVDev->AcquireGraphics() )
                    {
                        if ( (nWidth > 0) && (nHeight > 0) )
                        {
                            SalTwoRect aPosAry(nX, nY, nWidth, nHeight,
                                              (aRect.Left() < mnOutOffX) ? (mnOutOffX - aRect.Left()) : 0L,
                                              (aRect.Top() < mnOutOffY) ? (mnOutOffY - aRect.Top()) : 0L,
                                              nWidth, nHeight);
                            aVDev->mpGraphics->CopyBits(aPosAry, *mpGraphics, *this, *this);
                        }
                        else
                        {
                            OSL_ENSURE(false, "CopyBits with zero or negative width or height");
                        }

                        aBmp = aVDev->GetBitmap( Point(), aVDev->GetOutputSizePixel() );
                    }
                    else
                        bClipped = false;
                }
                else
                    bClipped = false;
            }

            if ( !bClipped )
            {
                std::shared_ptr<SalBitmap> pSalBmp = mpGraphics->GetBitmap( nX, nY, nWidth, nHeight, *this );

                if( pSalBmp )
                {
                    aBmp.ImplSetSalBitmap(pSalBmp);
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
            aAlphaBitmap.Convert( BmpConversion::N8BitNoConversion );

        return BitmapEx(GetBitmap( rSrcPt, rSize ), AlphaMask( aAlphaBitmap ) );
    }
    else
        return BitmapEx(GetBitmap( rSrcPt, rSize ));
}

void OutputDevice::DrawDeviceBitmap( const Point& rDestPt, const Size& rDestSize,
                                     const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                     BitmapEx& rBitmapEx )
{
    assert(!is_double_buffered_window());

    if (rBitmapEx.IsAlpha())
    {
        DrawDeviceAlphaBitmap(rBitmapEx.GetBitmap(), rBitmapEx.GetAlpha(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel);
    }
    else if (!!rBitmapEx)
    {
        SalTwoRect aPosAry(rSrcPtPixel.X(), rSrcPtPixel.Y(), rSrcSizePixel.Width(), rSrcSizePixel.Height(),
                           ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                           ImplLogicWidthToDevicePixel(rDestSize.Width()),
                           ImplLogicHeightToDevicePixel(rDestSize.Height()));

        const BmpMirrorFlags nMirrFlags = AdjustTwoRect(aPosAry, rBitmapEx.GetSizePixel());

        if (aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight)
        {

            if (nMirrFlags != BmpMirrorFlags::NONE)
                rBitmapEx.Mirror(nMirrFlags);

            const SalBitmap* pSalSrcBmp = rBitmapEx.ImplGetBitmapSalBitmap().get();
            std::shared_ptr<SalBitmap> xMaskBmp = rBitmapEx.ImplGetMaskSalBitmap();

            if (xMaskBmp)
            {
                bool bTryDirectPaint(pSalSrcBmp);

                if (bTryDirectPaint && mpGraphics->DrawAlphaBitmap(aPosAry, *pSalSrcBmp, *xMaskBmp, *this))
                {
                    // tried to paint as alpha directly. If this worked, we are done (except
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
                    tools::Rectangle aClipRegionBounds( ImplPixelToDevicePixel(maRegion).GetBoundRect() );

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
                        aClipRegionBounds.Intersection(tools::Rectangle(aPosAry.mnDestX,
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

                    mpGraphics->DrawBitmap(aPosAry, *pSalSrcBmp, *xMaskBmp, *this);
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
                mpGraphics->DrawBitmap(aPosAry, *pSalSrcBmp, *this);

                if (mpAlphaVDev)
                {
                    // #i32109#: Make bitmap area opaque
                    mpAlphaVDev->ImplFillOpaqueRectangle( tools::Rectangle(rDestPt, rDestSize) );
                }
            }
        }
    }
}

void OutputDevice::DrawDeviceAlphaBitmap( const Bitmap& rBmp, const AlphaMask& rAlpha,
                                    const Point& rDestPt, const Size& rDestSize,
                                    const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    assert(!is_double_buffered_window());

    Point     aOutPt(LogicToPixel(rDestPt));
    Size      aOutSz(LogicToPixel(rDestSize));
    tools::Rectangle aDstRect(Point(), GetOutputSizePixel());

    const bool bHMirr = aOutSz.Width() < 0;
    const bool bVMirr = aOutSz.Height() < 0;

    ClipToPaintRegion(aDstRect);

    BmpMirrorFlags mirrorFlags = BmpMirrorFlags::NONE;
    if (bHMirr)
    {
        aOutSz.setWidth( -aOutSz.Width() );
        aOutPt.AdjustX( -(aOutSz.Width() - 1) );
        mirrorFlags |= BmpMirrorFlags::Horizontal;
    }

    if (bVMirr)
    {
        aOutSz.setHeight( -aOutSz.Height() );
        aOutPt.AdjustY( -(aOutSz.Height() - 1) );
        mirrorFlags |= BmpMirrorFlags::Vertical;
    }

    if (aDstRect.Intersection(tools::Rectangle(aOutPt, aOutSz)).IsEmpty())
        return;

    {
        Point aRelPt = aOutPt + Point(mnOutOffX, mnOutOffY);
        SalTwoRect aTR(
            rSrcPtPixel.X(), rSrcPtPixel.Y(),
            rSrcSizePixel.Width(), rSrcSizePixel.Height(),
            aRelPt.X(), aRelPt.Y(),
            aOutSz.Width(), aOutSz.Height());

        Bitmap bitmap(rBmp);
        AlphaMask alpha(rAlpha);
        if(bHMirr || bVMirr)
        {
            bitmap.Mirror(mirrorFlags);
            alpha.Mirror(mirrorFlags);
        }
        SalBitmap* pSalSrcBmp = bitmap.ImplGetSalBitmap().get();
        SalBitmap* pSalAlphaBmp = alpha.ImplGetSalBitmap().get();

        // #i83087# Naturally, system alpha blending (SalGraphics::DrawAlphaBitmap) cannot work
        // with separate alpha VDev

        // try to blend the alpha bitmap with the alpha virtual device
        if (mpAlphaVDev)
        {
            Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( aRelPt, aOutSz ) );
            if (SalBitmap* pSalAlphaBmp2 = aAlphaBitmap.ImplGetSalBitmap().get())
            {
                if (mpGraphics->BlendAlphaBitmap(aTR, *pSalSrcBmp, *pSalAlphaBmp, *pSalAlphaBmp2, *this))
                {
                    mpAlphaVDev->BlendBitmap(aTR, rAlpha);
                    return;
                }
            }
        }
        else
        {
            if (mpGraphics->DrawAlphaBitmap(aTR, *pSalSrcBmp, *pSalAlphaBmp, *this))
                return;
        }

        // we need to make sure Skia never reaches this slow code path
        assert(!SkiaHelper::isVCLSkiaEnabled());
    }

    tools::Rectangle aBmpRect(Point(), rBmp.GetSizePixel());
    if (!aBmpRect.Intersection(tools::Rectangle(rSrcPtPixel, rSrcSizePixel)).IsEmpty())
    {
        Point     auxOutPt(LogicToPixel(rDestPt));
        Size      auxOutSz(LogicToPixel(rDestSize));

        // HACK: The function is broken with alpha vdev and mirroring, mirror here.
        Bitmap bitmap(rBmp);
        AlphaMask alpha(rAlpha);
        if(mpAlphaVDev && (bHMirr || bVMirr))
        {
            bitmap.Mirror(mirrorFlags);
            alpha.Mirror(mirrorFlags);
            auxOutPt = aOutPt;
            auxOutSz = aOutSz;
        }
        DrawDeviceAlphaBitmapSlowPath(bitmap, alpha, aDstRect, aBmpRect, auxOutSz, auxOutPt);
    }
}

namespace
{

struct LinearScaleContext
{
    std::unique_ptr<tools::Long[]> mpMapX;
    std::unique_ptr<tools::Long[]> mpMapY;

    std::unique_ptr<tools::Long[]> mpMapXOffset;
    std::unique_ptr<tools::Long[]> mpMapYOffset;

    LinearScaleContext(tools::Rectangle const & aDstRect, tools::Rectangle const & aBitmapRect,
                 Size const & aOutSize, tools::Long nOffX, tools::Long nOffY)

        : mpMapX(new tools::Long[aDstRect.GetWidth()])
        , mpMapY(new tools::Long[aDstRect.GetHeight()])
        , mpMapXOffset(new tools::Long[aDstRect.GetWidth()])
        , mpMapYOffset(new tools::Long[aDstRect.GetHeight()])
    {
        const tools::Long nSrcWidth = aBitmapRect.GetWidth();
        const tools::Long nSrcHeight = aBitmapRect.GetHeight();

        generateSimpleMap(
            nSrcWidth,  aDstRect.GetWidth(), aBitmapRect.Left(),
            aOutSize.Width(),  nOffX, mpMapX.get(), mpMapXOffset.get());

        generateSimpleMap(
            nSrcHeight, aDstRect.GetHeight(), aBitmapRect.Top(),
            aOutSize.Height(), nOffY, mpMapY.get(), mpMapYOffset.get());
    }

private:

    static void generateSimpleMap(tools::Long nSrcDimension, tools::Long nDstDimension, tools::Long nDstLocation,
                                  tools::Long nOutDimension, tools::Long nOffset, tools::Long* pMap, tools::Long* pMapOffset)
    {

        const double fReverseScale = (std::abs(nOutDimension) > 1) ? (nSrcDimension - 1) / double(std::abs(nOutDimension) - 1) : 0.0;

        tools::Long nSampleRange = std::max(tools::Long(0), nSrcDimension - 2);

        for (tools::Long i = 0; i < nDstDimension; i++)
        {
            double fTemp = std::abs((nOffset + i) * fReverseScale);

            pMap[i] = MinMax(nDstLocation + tools::Long(fTemp), 0, nSampleRange);
            pMapOffset[i] = static_cast<tools::Long>((fTemp - pMap[i]) * 128.0);
        }
    }

public:
    bool blendBitmap(
            const BitmapWriteAccess* pDestination,
            const BitmapReadAccess*  pSource,
            const BitmapReadAccess*  pSourceAlpha,
            const tools::Long nDstWidth,
            const tools::Long nDstHeight)
    {
        if (pSource && pSourceAlpha && pDestination)
        {
            ScanlineFormat nSourceFormat = pSource->GetScanlineFormat();
            ScanlineFormat nDestinationFormat = pDestination->GetScanlineFormat();

            switch (nSourceFormat)
            {
                case ScanlineFormat::N24BitTcRgb:
                case ScanlineFormat::N24BitTcBgr:
                {
                    if ( (nSourceFormat == ScanlineFormat::N24BitTcBgr && nDestinationFormat == ScanlineFormat::N32BitTcBgra)
                      || (nSourceFormat == ScanlineFormat::N24BitTcRgb && nDestinationFormat == ScanlineFormat::N32BitTcRgba))
                    {
                        blendBitmap24(pDestination, pSource, pSourceAlpha, nDstWidth, nDstHeight);
                        return true;
                    }
                }
                break;
                default: break;
            }
        }
        return false;
    }

    void blendBitmap24(
            const BitmapWriteAccess*  pDestination,
            const BitmapReadAccess*   pSource,
            const BitmapReadAccess*   pSourceAlpha,
            const tools::Long nDstWidth,
            const tools::Long nDstHeight)
    {
        Scanline pLine0, pLine1;
        Scanline pLineAlpha0, pLineAlpha1;
        Scanline pColorSample1, pColorSample2;
        Scanline pDestScanline;

        tools::Long nColor1Line1, nColor2Line1, nColor3Line1;
        tools::Long nColor1Line2, nColor2Line2, nColor3Line2;
        tools::Long nAlphaLine1, nAlphaLine2;

        sal_uInt8 nColor1, nColor2, nColor3, nAlpha;

        for (tools::Long nY = 0; nY < nDstHeight; nY++)
        {
            const tools::Long nMapY  = mpMapY[nY];
            const tools::Long nMapFY = mpMapYOffset[nY];

            pLine0 = pSource->GetScanline(nMapY);
            // tdf#95481 guard nMapY + 1 to be within bounds
            pLine1 = (nMapY + 1 < pSource->Height()) ? pSource->GetScanline(nMapY + 1) : pLine0;

            pLineAlpha0 = pSourceAlpha->GetScanline(nMapY);
            // tdf#95481 guard nMapY + 1 to be within bounds
            pLineAlpha1 = (nMapY + 1 < pSourceAlpha->Height()) ? pSourceAlpha->GetScanline(nMapY + 1) : pLineAlpha0;

            pDestScanline = pDestination->GetScanline(nY);

            for (tools::Long nX = 0; nX < nDstWidth; nX++)
            {
                const tools::Long nMapX = mpMapX[nX];
                const tools::Long nMapFX = mpMapXOffset[nX];

                pColorSample1 = pLine0 + 3 * nMapX;
                pColorSample2 = (nMapX + 1 < pSource->Width()) ? pColorSample1 + 3 : pColorSample1;
                nColor1Line1 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor2Line1 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor3Line1 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1 = pLine1 + 3 * nMapX;
                pColorSample2 = (nMapX + 1 < pSource->Width()) ? pColorSample1 + 3 : pColorSample1;
                nColor1Line2 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor2Line2 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor3Line2 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1 = pLineAlpha0 + nMapX;
                pColorSample2 = (nMapX + 1 < pSourceAlpha->Width()) ? pColorSample1 + 1 : pColorSample1;
                nAlphaLine1 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                pColorSample1 = pLineAlpha1 + nMapX;
                pColorSample2 = (nMapX + 1 < pSourceAlpha->Width()) ? pColorSample1 + 1 : pColorSample1;
                nAlphaLine2 = (static_cast<tools::Long>(*pColorSample1) << 7) + nMapFX * (static_cast<tools::Long>(*pColorSample2) - *pColorSample1);

                nColor1 = (nColor1Line1 + nMapFY * ((nColor1Line2 >> 7) - (nColor1Line1 >> 7))) >> 7;
                nColor2 = (nColor2Line1 + nMapFY * ((nColor2Line2 >> 7) - (nColor2Line1 >> 7))) >> 7;
                nColor3 = (nColor3Line1 + nMapFY * ((nColor3Line2 >> 7) - (nColor3Line1 >> 7))) >> 7;

                nAlpha  = (nAlphaLine1  + nMapFY * ((nAlphaLine2  >> 7) - (nAlphaLine1 >> 7))) >> 7;

                *pDestScanline = color::ColorChannelMerge(*pDestScanline, nColor1, nAlpha);
                pDestScanline++;
                *pDestScanline = color::ColorChannelMerge(*pDestScanline, nColor2, nAlpha);
                pDestScanline++;
                *pDestScanline = color::ColorChannelMerge(*pDestScanline, nColor3, nAlpha);
                pDestScanline++;
                pDestScanline++;
            }
        }
    }
};

struct TradScaleContext
{
    std::unique_ptr<tools::Long[]> mpMapX;
    std::unique_ptr<tools::Long[]> mpMapY;

    TradScaleContext(tools::Rectangle const & aDstRect, tools::Rectangle const & aBitmapRect,
                 Size const & aOutSize, tools::Long nOffX, tools::Long nOffY)

        : mpMapX(new tools::Long[aDstRect.GetWidth()])
        , mpMapY(new tools::Long[aDstRect.GetHeight()])
    {
        const tools::Long nSrcWidth = aBitmapRect.GetWidth();
        const tools::Long nSrcHeight = aBitmapRect.GetHeight();

        const bool bHMirr = aOutSize.Width() < 0;
        const bool bVMirr = aOutSize.Height() < 0;

        generateSimpleMap(
            nSrcWidth, aDstRect.GetWidth(), aBitmapRect.Left(),
            aOutSize.Width(), nOffX, bHMirr, mpMapX.get());

        generateSimpleMap(
            nSrcHeight, aDstRect.GetHeight(), aBitmapRect.Top(),
            aOutSize.Height(), nOffY, bVMirr, mpMapY.get());
    }

private:

    static void generateSimpleMap(tools::Long nSrcDimension, tools::Long nDstDimension, tools::Long nDstLocation,
                                  tools::Long nOutDimension, tools::Long nOffset, bool bMirror, tools::Long* pMap)
    {
        tools::Long nMirrorOffset = 0;

        if (bMirror)
            nMirrorOffset = (nDstLocation << 1) + nSrcDimension - 1;

        for (tools::Long i = 0; i < nDstDimension; ++i, ++nOffset)
        {
            pMap[i] = nDstLocation + nOffset * nSrcDimension / nOutDimension;
            if (bMirror)
                pMap[i] = nMirrorOffset - pMap[i];
        }
    }
};


} // end anonymous namespace

void OutputDevice::DrawDeviceAlphaBitmapSlowPath(const Bitmap& rBitmap,
    const AlphaMask& rAlpha, tools::Rectangle aDstRect, tools::Rectangle aBmpRect, Size const & aOutSize, Point const & aOutPoint)
{
    assert(!is_double_buffered_window());

    VirtualDevice* pOldVDev = mpAlphaVDev;

    const bool  bHMirr = aOutSize.Width() < 0;
    const bool  bVMirr = aOutSize.Height() < 0;

    // The scaling in this code path produces really ugly results - it
    // does the most trivial scaling with no smoothing.
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    const bool   bOldMap = mbMap;

    mpMetaFile = nullptr; // fdo#55044 reset before GetBitmap!
    mbMap = false;

    Bitmap aBmp(GetBitmap(aDstRect.TopLeft(), aDstRect.GetSize()));

    // #109044# The generated bitmap need not necessarily be
    // of aDstRect dimensions, it's internally clipped to
    // window bounds. Thus, we correct the dest size here,
    // since we later use it (in nDstWidth/Height) for pixel
    // access)
    // #i38887# reading from screen may sometimes fail
    if (aBmp.ImplGetSalBitmap())
    {
        aDstRect.SetSize(aBmp.GetSizePixel());
    }

    const tools::Long nDstWidth = aDstRect.GetWidth();
    const tools::Long nDstHeight = aDstRect.GetHeight();

    // calculate offset in original bitmap
    // in RTL case this is a little more complicated since the contents of the
    // bitmap is not mirrored (it never is), however the paint region and bmp region
    // are in mirrored coordinates, so the intersection of (aOutPt,aOutSz) with these
    // is content wise somewhere else and needs to take mirroring into account
    const tools::Long nOffX = IsRTLEnabled()
                            ? aOutSize.Width() - aDstRect.GetWidth() - (aDstRect.Left() - aOutPoint.X())
                            : aDstRect.Left() - aOutPoint.X();

    const tools::Long nOffY = aDstRect.Top() - aOutPoint.Y();

    TradScaleContext aTradContext(aDstRect, aBmpRect, aOutSize, nOffX, nOffY);

    Bitmap::ScopedReadAccess pBitmapReadAccess(const_cast<Bitmap&>(rBitmap));
    AlphaMask::ScopedReadAccess pAlphaReadAccess(const_cast<AlphaMask&>(rAlpha));

    DBG_ASSERT( pAlphaReadAccess->GetScanlineFormat() == ScanlineFormat::N8BitPal,
                "OutputDevice::ImplDrawAlpha(): non-8bit alpha no longer supported!" );

    // #i38887# reading from screen may sometimes fail
    if (aBmp.ImplGetSalBitmap())
    {
        Bitmap aNewBitmap;

        if (mpAlphaVDev)
        {
            aNewBitmap = BlendBitmapWithAlpha(
                            aBmp, pBitmapReadAccess.get(), pAlphaReadAccess.get(),
                            aDstRect,
                            nOffY, nDstHeight,
                            nOffX, nDstWidth,
                            aTradContext.mpMapX.get(), aTradContext.mpMapY.get() );
        }
        else
        {
            LinearScaleContext aLinearContext(aDstRect, aBmpRect, aOutSize, nOffX, nOffY);

            if (aLinearContext.blendBitmap( BitmapScopedWriteAccess(aBmp).get(), pBitmapReadAccess.get(), pAlphaReadAccess.get(),
                    nDstWidth, nDstHeight))
            {
                aNewBitmap = aBmp;
            }
            else
            {
                aNewBitmap = BlendBitmap(
                            aBmp, pBitmapReadAccess.get(), pAlphaReadAccess.get(),
                            nOffY, nDstHeight,
                            nOffX, nDstWidth,
                            aBmpRect, aOutSize,
                            bHMirr, bVMirr,
                            aTradContext.mpMapX.get(), aTradContext.mpMapY.get() );
            }
        }

        // #110958# Disable alpha VDev, we're doing the necessary
        // stuff explicitly further below
        if (mpAlphaVDev)
            mpAlphaVDev = nullptr;

        DrawBitmap(aDstRect.TopLeft(), aNewBitmap);

        // #110958# Enable alpha VDev again
        mpAlphaVDev = pOldVDev;
    }

    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

bool OutputDevice::DrawTransformBitmapExDirect(
        const basegfx::B2DHomMatrix& aFullTransform,
        const BitmapEx& rBitmapEx)
{
    assert(!is_double_buffered_window());

    bool bDone = false;

    // try to paint directly
    const basegfx::B2DPoint aNull(aFullTransform * basegfx::B2DPoint(0.0, 0.0));
    const basegfx::B2DPoint aTopX(aFullTransform * basegfx::B2DPoint(1.0, 0.0));
    const basegfx::B2DPoint aTopY(aFullTransform * basegfx::B2DPoint(0.0, 1.0));
    SalBitmap* pSalSrcBmp = rBitmapEx.GetBitmap().ImplGetSalBitmap().get();
    Bitmap aAlphaBitmap;

    if(rBitmapEx.IsTransparent())
    {
        if(rBitmapEx.IsAlpha())
        {
            aAlphaBitmap = rBitmapEx.GetAlpha();
        }
        else
        {
            aAlphaBitmap = rBitmapEx.GetMask();
        }
    }
    else if (mpAlphaVDev)
    {
        aAlphaBitmap = AlphaMask(rBitmapEx.GetSizePixel());
        aAlphaBitmap.Erase(COL_BLACK); // opaque
    }

    SalBitmap* pSalAlphaBmp = aAlphaBitmap.ImplGetSalBitmap().get();

    bDone = mpGraphics->DrawTransformedBitmap(
        aNull,
        aTopX,
        aTopY,
        *pSalSrcBmp,
        pSalAlphaBmp,
        *this);

    if (mpAlphaVDev)
    {
        // Merge bitmap alpha to alpha device
        AlphaMask aBlack(rBitmapEx.GetSizePixel());
        aBlack.Erase(0); // opaque
        mpAlphaVDev->DrawTransformBitmapExDirect(aFullTransform, BitmapEx(aBlack, aAlphaBitmap));
    }

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
        tools::Rectangle aRegionRectangle(GetActiveClipRegion().GetBoundRect());

        // caution! Range from rectangle, one too much (!)
        aRegionRectangle.AdjustRight(-1);
        aRegionRectangle.AdjustBottom(-1);
        aOutPixel.intersect( vcl::unotools::b2DRectangleFromRectangle(aRegionRectangle) );
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

// MM02 add some test class to get a simple timer-based output to be able
// to check if it gets faster - and how much. Uncomment next line or set
// DO_TIME_TEST for compile time if you want to use it
// #define DO_TIME_TEST
#ifdef DO_TIME_TEST
#include <tools/time.hxx>
struct LocalTimeTest
{
    const sal_uInt64 nStartTime;
    LocalTimeTest() : nStartTime(tools::Time::GetSystemTicks()) {}
    ~LocalTimeTest()
    {
        const sal_uInt64 nEndTime(tools::Time::GetSystemTicks());
        const sal_uInt64 nDiffTime(nEndTime - nStartTime);

        if(nDiffTime > 0)
        {
            OStringBuffer aOutput("Time: ");
            OString aNumber(OString::number(nDiffTime));
            aOutput.append(aNumber);
            OSL_FAIL(aOutput.getStr());
        }
    }
};
#endif

void OutputDevice::DrawTransformedBitmapEx(
    const basegfx::B2DHomMatrix& rTransformation,
    const BitmapEx& rBitmapEx)
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if(rBitmapEx.IsEmpty())
        return;

    // MM02 compared to other public methods of OutputDevice
    // this test was missing and led to zero-ptr-accesses
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    const bool bMetafile(nullptr != mpMetaFile);
    /*
       tdf#135325 typically in these OutputDevice methods, for the in
       record-to-metafile case the  MetaFile is already written to before the
       test against mbOutputClipped to determine that output to the current
       device would result in no visual output. In this case the metafile is
       written after the test, so we must continue past mbOutputClipped if
       recording to a metafile. It's typical to record with a device of nominal
       size and play back later against something of a totally different size.
     */
    if (mbOutputClipped && !bMetafile)
        return;

#ifdef DO_TIME_TEST
    // MM02 start time test when some data (not for trivial stuff). Will
    // trigger and show data when leaving this method by destructing helper
    static const char* pEnableBitmapDrawTimerTimer(getenv("SAL_ENABLE_TIMER_BITMAPDRAW"));
    static bool bUseTimer(nullptr != pEnableBitmapDrawTimerTimer);
    std::unique_ptr<LocalTimeTest> aTimeTest(
        bUseTimer && rBitmapEx.GetSizeBytes() > 10000
        ? new LocalTimeTest()
        : nullptr);
#endif

    // decompose matrix to check rotation and shear
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
    const bool bRotated(!basegfx::fTools::equalZero(fRotate));
    const bool bSheared(!basegfx::fTools::equalZero(fShearX));
    const bool bMirroredX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirroredY(basegfx::fTools::less(aScale.getY(), 0.0));

    if(!bRotated && !bSheared && !bMirroredX && !bMirroredY)
    {
        // with no rotation, shear or mirroring it can be mapped to DrawBitmapEx
        // do *not* execute the mirroring here, it's done in the fallback
        // #i124580# the correct DestSize needs to be calculated based on MaxXY values
        Point aDestPt(basegfx::fround(aTranslate.getX()), basegfx::fround(aTranslate.getY()));
        const Size aDestSize(
            basegfx::fround(aScale.getX() + aTranslate.getX()) - aDestPt.X(),
            basegfx::fround(aScale.getY() + aTranslate.getY()) - aDestPt.Y());
        const Point aOrigin = GetMapMode().GetOrigin();
        if (!bMetafile && comphelper::LibreOfficeKit::isActive() && GetMapMode().GetMapUnit() != MapUnit::MapPixel)
        {
            aDestPt.Move(aOrigin.getX(), aOrigin.getY());
            EnableMapMode(false);
        }

        DrawBitmapEx(aDestPt, aDestSize, rBitmapEx);
        if (!bMetafile && comphelper::LibreOfficeKit::isActive() && GetMapMode().GetMapUnit() != MapUnit::MapPixel)
        {
            EnableMapMode();
            aDestPt.Move(-aOrigin.getX(), -aOrigin.getY());
        }
        return;
    }

    const bool bInvert(RasterOp::Invert == meRasterOp);
    const bool bBitmapChangedColor(mnDrawMode & (DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap | DrawModeFlags::GrayBitmap ));
    const bool bTryDirectPaint(!bInvert && !bBitmapChangedColor && !bMetafile);
    if(bTryDirectPaint)
    {
        // tdf#130768 CAUTION(!) using GetViewTransformation() is *not* enough here, it may
        // be that mnOutOffX/mnOutOffY is used - see AOO bug 75163, mentioned at
        // ImplGetDeviceTransformation declaration
        const basegfx::B2DHomMatrix aFullTransform(ImplGetDeviceTransformation() * rTransformation);

        if(DrawTransformBitmapExDirect(aFullTransform, rBitmapEx))
        {
            // we are done
            return;
        }
    }

    // take the fallback when no rotate and shear, but mirror (else we would have done this above)
    if(!bRotated && !bSheared)
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

    // at this point we are either sheared or rotated or both
    assert(bSheared || bRotated);

    // fallback; create transformed bitmap the hard way (back-transform
    // the pixels) and paint
    basegfx::B2DRange aVisibleRange(0.0, 0.0, 1.0, 1.0);

    // limit maximum area to something looking good for non-pixel-based targets (metafile, printer)
    // by using a fixed minimum (allow at least, but no need to utilize) for good smoothing and an area
    // dependent of original size for good quality when e.g. rotated/sheared. Still, limit to a maximum
    // to avoid crashes/resource problems (ca. 1500x3000 here)
    const Size& rOriginalSizePixel(rBitmapEx.GetSizePixel());
    const double fOrigArea(rOriginalSizePixel.Width() * rOriginalSizePixel.Height() * 0.5);
    const double fOrigAreaScaled(fOrigArea * 1.44);
    double fMaximumArea(std::clamp(fOrigAreaScaled, 1000000.0, 4500000.0));
    // tdf#130768 CAUTION(!) using GetViewTransformation() is *not* enough here, it may
    // be that mnOutOffX/mnOutOffY is used - see AOO bug 75163, mentioned at
    // ImplGetDeviceTransformation declaration
    basegfx::B2DHomMatrix aFullTransform(ImplGetDeviceTransformation() * rTransformation);

    if(!bMetafile)
    {
        if ( !TransformAndReduceBitmapExToTargetRange( aFullTransform, aVisibleRange, fMaximumArea ) )
            return;
    }

    if(aVisibleRange.isEmpty())
        return;

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

    // Remove scaling from aFulltransform: we transform due to shearing or rotation, scaling
    // will happen according to aDestSize.
    basegfx::B2DVector aFullScale, aFullTranslate;
    double fFullRotate, fFullShearX;
    aFullTransform.decompose(aFullScale, aFullTranslate, fFullRotate, fFullShearX);
    // Require positive scaling, negative scaling would loose horizontal or vertical flip.
    if (aFullScale.getX() > 0 && aFullScale.getY() > 0)
    {
        basegfx::B2DHomMatrix aTransform = basegfx::utils::createScaleB2DHomMatrix(
            rOriginalSizePixel.getWidth() / aFullScale.getX(),
            rOriginalSizePixel.getHeight() / aFullScale.getY());
        aFullTransform *= aTransform;
    }

    double fSourceRatio = 1.0;
    if (rOriginalSizePixel.getHeight() != 0)
    {
        fSourceRatio = rOriginalSizePixel.getWidth() / rOriginalSizePixel.getHeight();
    }
    double fTargetRatio = 1.0;
    if (aFullScale.getY() != 0)
    {
        fTargetRatio = aFullScale.getX() / aFullScale.getY();
    }
    bool bAspectRatioKept = rtl::math::approxEqual(fSourceRatio, fTargetRatio);
    if (bSheared || !bAspectRatioKept)
    {
        // Not only rotation, or scaling does not keep aspect ratio.
        aTransformed = aTransformed.getTransformed(
            aFullTransform,
            aVisibleRange,
            fMaximumArea);
    }
    else
    {
        // Just rotation, can do that directly.
        fFullRotate = fmod(fFullRotate * -1, F_2PI);
        if (fFullRotate < 0)
        {
            fFullRotate += F_2PI;
        }
        Degree10 nAngle10(basegfx::fround(basegfx::rad2deg(fFullRotate) * 10));
        aTransformed.Rotate(nAngle10, COL_TRANSPARENT);
    }
    basegfx::B2DRange aTargetRange(0.0, 0.0, 1.0, 1.0);

    // get logic object target range
    aTargetRange.transform(rTransformation);

    // get from unified/relative VisibleRange to logoc one
    aVisibleRange.transform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
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

void OutputDevice::DrawImage( const Point& rPos, const Image& rImage, DrawImageFlags nStyle )
{
    assert(!is_double_buffered_window());

    DrawImage( rPos, Size(), rImage, nStyle );
}

void OutputDevice::DrawImage( const Point& rPos, const Size& rSize,
                              const Image& rImage, DrawImageFlags nStyle )
{
    assert(!is_double_buffered_window());

    bool bIsSizeValid = !rSize.IsEmpty();

    if (!ImplIsRecordLayout())
    {
        Image& rNonConstImage = const_cast<Image&>(rImage);
        if (bIsSizeValid)
            rNonConstImage.Draw(this, rPos, nStyle, &rSize);
        else
            rNonConstImage.Draw(this, rPos, nStyle);
    }
}

namespace
{
    // Co = Cs + Cd*(1-As) premultiplied alpha -or-
    // Co = (AsCs + AdCd*(1-As)) / Ao
    sal_uInt8 CalcColor( const sal_uInt8 nSourceColor, const sal_uInt8 nSourceAlpha,
                                const sal_uInt8 nDstAlpha, const sal_uInt8 nResAlpha, const sal_uInt8 nDestColor )
    {
        int c = nResAlpha ? ( static_cast<int>(nSourceAlpha)*nSourceColor + static_cast<int>(nDstAlpha)*nDestColor -
                              static_cast<int>(nDstAlpha)*nDestColor*nSourceAlpha/255 ) / static_cast<int>(nResAlpha) : 0;
        return sal_uInt8( c );
    }

    BitmapColor AlphaBlend( int nX,               int nY,
                                   const tools::Long            nMapX,
                                   const tools::Long            nMapY,
                                   BitmapReadAccess const *  pP,
                                   BitmapReadAccess const *  pA,
                                   BitmapReadAccess const *  pB,
                                   BitmapWriteAccess const * pAlphaW,
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
        nResAlpha = static_cast<int>(nSrcAlpha) + static_cast<int>(nDstAlpha) - static_cast<int>(nDstAlpha)*nSrcAlpha/255;

        aDstCol.SetRed( CalcColor( aSrcCol.GetRed(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetRed() ) );
        aDstCol.SetBlue( CalcColor( aSrcCol.GetBlue(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetBlue() ) );
        aDstCol.SetGreen( CalcColor( aSrcCol.GetGreen(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetGreen() ) );

        return aDstCol;
    }
}

void OutputDevice::BlendBitmap(
            const SalTwoRect&   rPosAry,
            const Bitmap&       rBmp )
{
    mpGraphics->BlendBitmap( rPosAry, *rBmp.ImplGetSalBitmap(), *this );
}

Bitmap OutputDevice::BlendBitmapWithAlpha(
            Bitmap&             aBmp,
            BitmapReadAccess const *   pP,
            BitmapReadAccess const *   pA,
            const tools::Rectangle&    aDstRect,
            const sal_Int32     nOffY,
            const sal_Int32     nDstHeight,
            const sal_Int32     nOffX,
            const sal_Int32     nDstWidth,
            const tools::Long*         pMapX,
            const tools::Long*         pMapY )

{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;
    sal_uInt8   nResAlpha;

    SAL_WARN_IF( !mpAlphaVDev, "vcl.gdi", "BlendBitmapWithAlpha(): call me only with valid alpha VirtualDevice!" );

    bool bOldMapMode( mpAlphaVDev->IsMapModeEnabled() );
    mpAlphaVDev->EnableMapMode(false);

    Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );
    BitmapScopedWriteAccess pAlphaW(aAlphaBitmap);

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        Bitmap::ScopedReadAccess pB(aBmp);
        BitmapScopedWriteAccess pW(aDither);

        if (pB && pP && pA && pW && pAlphaW)
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const tools::Long nMapY = pMapY[ nY ];
                const tools::Long nModY = ( nOutY & 0x0FL ) << 4;
                int nOutX;

                Scanline pScanline = pW->GetScanline(nY);
                Scanline pScanlineAlpha = pAlphaW->GetScanline(nY);
                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const tools::Long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB.get(), pAlphaW.get(), nResAlpha );

                    aIndex.SetIndex( static_cast<sal_uInt8>( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16 ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16 ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16 ] ) );
                    pW->SetPixelOnData( pScanline, nX, aIndex );

                    aIndex.SetIndex( static_cast<sal_uInt8>( nVCLRLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16 ] +
                                                   nVCLGLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16 ] +
                                                   nVCLBLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16 ] ) );
                    pAlphaW->SetPixelOnData( pScanlineAlpha, nX, aIndex );
                }
            }
        }
        pB.reset();
        pW.reset();
        res = aDither;
    }
    else
    {
        BitmapScopedWriteAccess pB(aBmp);
        if (pB && pP && pA && pAlphaW)
        {
            for( nY = 0; nY < nDstHeight; nY++ )
            {
                const tools::Long  nMapY = pMapY[ nY ];
                Scanline pScanlineB = pB->GetScanline(nY);
                Scanline pScanlineAlpha = pAlphaW->GetScanline(nY);

                for( nX = 0; nX < nDstWidth; nX++ )
                {
                    const tools::Long nMapX = pMapX[ nX ];
                    aDstCol = AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB.get(), pAlphaW.get(), nResAlpha );

                    pB->SetPixelOnData(pScanlineB, nX, pB->GetBestMatchingColor(aDstCol));
                    pAlphaW->SetPixelOnData(pScanlineAlpha, nX, pB->GetBestMatchingColor(Color(255L-nResAlpha, 255L-nResAlpha, 255L-nResAlpha)));
                }
            }
        }
        pB.reset();
        res = aBmp;
    }

    pAlphaW.reset();
    mpAlphaVDev->DrawBitmap( aDstRect.TopLeft(), aAlphaBitmap );
    mpAlphaVDev->EnableMapMode( bOldMapMode );

    return res;
}

Bitmap OutputDevice::BlendBitmap(
            Bitmap&             aBmp,
            BitmapReadAccess const * pP,
            BitmapReadAccess const * pA,
            const sal_Int32     nOffY,
            const sal_Int32     nDstHeight,
            const sal_Int32     nOffX,
            const sal_Int32     nDstWidth,
            const tools::Rectangle&    aBmpRect,
            const Size&         aOutSz,
            const bool          bHMirr,
            const bool          bVMirr,
            const tools::Long*         pMapX,
            const tools::Long*         pMapY )
{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        Bitmap::ScopedReadAccess pB(aBmp);
        BitmapScopedWriteAccess pW(aDither);

        if( pB && pP && pA && pW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                tools::Long nMapY = pMapY[ nY ];
                if (bVMirr)
                {
                    nMapY = aBmpRect.Bottom() - nMapY;
                }
                const tools::Long nModY = ( nOutY & 0x0FL ) << 4;
                int nOutX;

                Scanline pScanline = pW->GetScanline(nY);
                Scanline pScanlineAlpha = pA->GetScanline(nMapY);
                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    tools::Long  nMapX = pMapX[ nX ];
                    if (bHMirr)
                    {
                        nMapX = aBmpRect.Right() - nMapX;
                    }
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = pB->GetColor( nY, nX );
                    aDstCol.Merge( pP->GetColor( nMapY, nMapX ), pA->GetIndexFromData( pScanlineAlpha, nMapX ) );
                    aIndex.SetIndex( static_cast<sal_uInt8>( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16 ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16 ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16 ] ) );
                    pW->SetPixelOnData( pScanline, nX, aIndex );
                }
            }
        }

        pB.reset();
        pW.reset();
        res = aDither;
    }
    else
    {
        BitmapScopedWriteAccess pB(aBmp);

        bool bFastBlend = false;
        if( pP && pA && pB && !bHMirr && !bVMirr )
        {
            SalTwoRect aTR(aBmpRect.Left(), aBmpRect.Top(), aBmpRect.GetWidth(), aBmpRect.GetHeight(),
                            nOffX, nOffY, aOutSz.Width(), aOutSz.Height());

            bFastBlend = ImplFastBitmapBlending( *pB,*pP,*pA, aTR );
        }

        if( pP && pA && pB && !bFastBlend )
        {
            switch( pP->GetScanlineFormat() )
            {
                case ScanlineFormat::N8BitPal:
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            tools::Long  nMapY = pMapY[ nY ];
                            if ( bVMirr )
                            {
                                nMapY = aBmpRect.Bottom() - nMapY;
                            }
                            Scanline pPScan = pP->GetScanline( nMapY );
                            Scanline pAScan = pA->GetScanline( nMapY );
                            Scanline pBScan = pB->GetScanline( nY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                tools::Long nMapX = pMapX[ nX ];

                                if ( bHMirr )
                                {
                                    nMapX = aBmpRect.Right() - nMapX;
                                }
                                aDstCol = pB->GetPixelFromData( pBScan, nX );
                                aDstCol.Merge( pP->GetPaletteColor( pPScan[ nMapX ] ), pAScan[ nMapX ] );
                                pB->SetPixelOnData( pBScan, nX, aDstCol );
                            }
                        }
                    }
                    break;

                default:
                {

                    for( nY = 0; nY < nDstHeight; nY++ )
                    {
                        tools::Long  nMapY = pMapY[ nY ];

                        if ( bVMirr )
                        {
                            nMapY = aBmpRect.Bottom() - nMapY;
                        }
                        Scanline pAScan = pA->GetScanline( nMapY );
                        Scanline pBScan = pB->GetScanline(nY);
                        for( nX = 0; nX < nDstWidth; nX++ )
                        {
                            tools::Long nMapX = pMapX[ nX ];

                            if ( bHMirr )
                            {
                                nMapX = aBmpRect.Right() - nMapX;
                            }
                            aDstCol = pB->GetPixelFromData( pBScan, nX );
                            aDstCol.Merge( pP->GetColor( nMapY, nMapX ), pAScan[ nMapX ] );
                            pB->SetPixelOnData( pBScan, nX, aDstCol );
                        }
                    }
                }
                break;
            }
        }

        pB.reset();
        res = aBmp;
    }

    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
