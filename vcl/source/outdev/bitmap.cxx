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

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLHelper.hxx>
#endif
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/window.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bmpfast.hxx>
#include <salgdi.hxx>
#include <salbmp.hxx>
#include <image.h>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <memory>
#include <comphelper/lok.hxx>
#include <bitmapwriteaccess.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/helpers.hxx>
#include <tools/debug.hxx>

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
                if ( nAction == MetaActionType::BMPSCALE )
                    ScaleBitmap (aBmp, aPosAry);

                mpGraphics->DrawBitmap( aPosAry, *aBmp.ImplGetSalBitmap(), this );
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
                                           const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY )
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
    long    nX = ImplLogicXToDevicePixel( rSrcPt.X() );
    long    nY = ImplLogicYToDevicePixel( rSrcPt.Y() );
    long    nWidth = ImplLogicWidthToDevicePixel( rSize.Width() );
    long    nHeight = ImplLogicHeightToDevicePixel( rSize.Height() );

    if ( mpGraphics || AcquireGraphics() )
    {
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
                    if ( aVDev.get()->mpGraphics || aVDev.get()->AcquireGraphics() )
                    {
                        if ( (nWidth > 0) && (nHeight > 0) )
                        {
                            SalTwoRect aPosAry(nX, nY, nWidth, nHeight,
                                              (aRect.Left() < mnOutOffX) ? (mnOutOffX - aRect.Left()) : 0L,
                                              (aRect.Top() < mnOutOffY) ? (mnOutOffY - aRect.Top()) : 0L,
                                              nWidth, nHeight);
                            aVDev.get()->mpGraphics->CopyBits( aPosAry, mpGraphics, this, this );
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
                std::shared_ptr<SalBitmap> pSalBmp = mpGraphics->GetBitmap( nX, nY, nWidth, nHeight, this );

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
            aAlphaBitmap.Convert( BmpConversion::N8BitGreys );

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

                if (bTryDirectPaint && mpGraphics->DrawAlphaBitmap(aPosAry, *pSalSrcBmp, *xMaskBmp, this))
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

                    mpGraphics->DrawBitmap(aPosAry, *pSalSrcBmp, *xMaskBmp, this);
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

    if (bHMirr)
    {
        aOutSz.setWidth( -aOutSz.Width() );
        aOutPt.AdjustX( -(aOutSz.Width() - 1) );
    }

    if (bVMirr)
    {
        aOutSz.setHeight( -aOutSz.Height() );
        aOutPt.AdjustY( -(aOutSz.Height() - 1) );
    }

    if (!aDstRect.Intersection(tools::Rectangle(aOutPt, aOutSz)).IsEmpty())
    {
        static const char* pDisableNative = getenv( "SAL_DISABLE_NATIVE_ALPHA");
        // #i83087# Naturally, system alpha blending cannot work with
        // separate alpha VDev

        // Not clear how the above comment relates to the following declaration and initialisation
        // of bTryDirectPaint. Does bTryDirectPaint being true mean that we can use "system alpha
        // blending"? Or that we can't? Or are the two not related at all, and should the above
        // comment actually be better located below, before the "if (mpAlphaVDev)" test?

        bool bTryDirectPaint(!pDisableNative && !bHMirr && !bVMirr);

        if (bTryDirectPaint)
        {
            Point aRelPt = aOutPt + Point(mnOutOffX, mnOutOffY);
            SalTwoRect aTR(
                rSrcPtPixel.X(), rSrcPtPixel.Y(),
                rSrcSizePixel.Width(), rSrcSizePixel.Height(),
                aRelPt.X(), aRelPt.Y(),
                aOutSz.Width(), aOutSz.Height());

            SalBitmap* pSalSrcBmp = rBmp.ImplGetSalBitmap().get();
            SalBitmap* pSalAlphaBmp = rAlpha.ImplGetSalBitmap().get();

            // try to blend the alpha bitmap with the alpha virtual device
            if (mpAlphaVDev)
            {
                Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( aRelPt, aOutSz ) );
                if (aAlphaBitmap.ImplGetSalBitmap())
                {
                    SalBitmap* pSalAlphaBmp2 = aAlphaBitmap.ImplGetSalBitmap().get();
                    if (mpGraphics->BlendAlphaBitmap(aTR, *pSalSrcBmp, *pSalAlphaBmp, *pSalAlphaBmp2, this))
                    {
                        mpAlphaVDev->BlendBitmap(aTR, rAlpha);
                        return;
                    }
                }
            }
            else
            {
                if (mpGraphics->DrawAlphaBitmap(aTR, *pSalSrcBmp, *pSalAlphaBmp, this))
                    return;
            }
        }

        // we need to make sure OpenGL never reaches this slow code path

#if HAVE_FEATURE_OPENGL
        assert(!OpenGLHelper::isVCLOpenGLEnabled());
#endif
        tools::Rectangle aBmpRect(Point(), rBmp.GetSizePixel());
        if (!aBmpRect.Intersection(tools::Rectangle(rSrcPtPixel, rSrcSizePixel)).IsEmpty())
        {
            Point     auxOutPt(LogicToPixel(rDestPt));
            Size      auxOutSz(LogicToPixel(rDestSize));

            DrawDeviceAlphaBitmapSlowPath(rBmp, rAlpha, aDstRect, aBmpRect, auxOutSz, auxOutPt);
        }
    }
}

namespace
{

struct LinearScaleContext
{
    std::unique_ptr<long[]> mpMapX;
    std::unique_ptr<long[]> mpMapY;

    std::unique_ptr<long[]> mpMapXOffset;
    std::unique_ptr<long[]> mpMapYOffset;

    LinearScaleContext(tools::Rectangle const & aDstRect, tools::Rectangle const & aBitmapRect,
                 Size const & aOutSize, long nOffX, long nOffY)

        : mpMapX(new long[aDstRect.GetWidth()])
        , mpMapY(new long[aDstRect.GetHeight()])
        , mpMapXOffset(new long[aDstRect.GetWidth()])
        , mpMapYOffset(new long[aDstRect.GetHeight()])
    {
        const long nSrcWidth = aBitmapRect.GetWidth();
        const long nSrcHeight = aBitmapRect.GetHeight();

        generateSimpleMap(
            nSrcWidth,  aDstRect.GetWidth(), aBitmapRect.Left(),
            aOutSize.Width(),  nOffX, mpMapX.get(), mpMapXOffset.get());

        generateSimpleMap(
            nSrcHeight, aDstRect.GetHeight(), aBitmapRect.Top(),
            aOutSize.Height(), nOffY, mpMapY.get(), mpMapYOffset.get());
    }

private:

    static void generateSimpleMap(long nSrcDimension, long nDstDimension, long nDstLocation,
                                  long nOutDimention, long nOffset, long* pMap, long* pMapOffset)
    {

        const double fReverseScale = (std::abs(nOutDimention) > 1) ? (nSrcDimension - 1) / double(std::abs(nOutDimention) - 1) : 0.0;

        long nSampleRange = std::max(0L, nSrcDimension - 2);

        for (long i = 0; i < nDstDimension; i++)
        {
            double fTemp = std::abs((nOffset + i) * fReverseScale);

            pMap[i] = MinMax(nDstLocation + long(fTemp), 0, nSampleRange);
            pMapOffset[i] = static_cast<long>((fTemp - pMap[i]) * 128.0);
        }
    }

public:
    bool blendBitmap(
            const BitmapWriteAccess* pDestination,
            const BitmapReadAccess*  pSource,
            const BitmapReadAccess*  pSourceAlpha,
            const long nDstWidth,
            const long nDstHeight)
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
            const long nDstWidth,
            const long nDstHeight)
    {
        Scanline pLine0, pLine1;
        Scanline pLineAlpha0, pLineAlpha1;
        Scanline pColorSample1, pColorSample2;
        Scanline pDestScanline;

        long nColor1Line1, nColor2Line1, nColor3Line1;
        long nColor1Line2, nColor2Line2, nColor3Line2;
        long nAlphaLine1, nAlphaLine2;

        sal_uInt8 nColor1, nColor2, nColor3, nAlpha;

        for (long nY = 0; nY < nDstHeight; nY++)
        {
            const long nMapY  = mpMapY[nY];
            const long nMapFY = mpMapYOffset[nY];

            pLine0 = pSource->GetScanline(nMapY);
            // tdf#95481 guard nMapY + 1 to be within bounds
            pLine1 = (nMapY + 1 < pSource->Height()) ? pSource->GetScanline(nMapY + 1) : pLine0;

            pLineAlpha0 = pSourceAlpha->GetScanline(nMapY);
            // tdf#95481 guard nMapY + 1 to be within bounds
            pLineAlpha1 = (nMapY + 1 < pSourceAlpha->Height()) ? pSourceAlpha->GetScanline(nMapY + 1) : pLineAlpha0;

            pDestScanline = pDestination->GetScanline(nY);

            for (long nX = 0; nX < nDstWidth; nX++)
            {
                const long nMapX = mpMapX[nX];
                const long nMapFX = mpMapXOffset[nX];

                pColorSample1 = pLine0 + 3 * nMapX;
                pColorSample2 = (nMapX + 1 < pSource->Width()) ? pColorSample1 + 3 : pColorSample1;
                nColor1Line1 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor2Line1 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor3Line1 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1 = pLine1 + 3 * nMapX;
                pColorSample2 = (nMapX + 1 < pSource->Width()) ? pColorSample1 + 3 : pColorSample1;
                nColor1Line2 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor2Line2 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1++;
                pColorSample2++;
                nColor3Line2 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1 = pLineAlpha0 + nMapX;
                pColorSample2 = (nMapX + 1 < pSourceAlpha->Width()) ? pColorSample1 + 1 : pColorSample1;
                nAlphaLine1 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                pColorSample1 = pLineAlpha1 + nMapX;
                pColorSample2 = (nMapX + 1 < pSourceAlpha->Width()) ? pColorSample1 + 1 : pColorSample1;
                nAlphaLine2 = (static_cast<long>(*pColorSample1) << 7) + nMapFX * (static_cast<long>(*pColorSample2) - *pColorSample1);

                nColor1 = (nColor1Line1 + nMapFY * ((nColor1Line2 >> 7) - (nColor1Line1 >> 7))) >> 7;
                nColor2 = (nColor2Line1 + nMapFY * ((nColor2Line2 >> 7) - (nColor2Line1 >> 7))) >> 7;
                nColor3 = (nColor3Line1 + nMapFY * ((nColor3Line2 >> 7) - (nColor3Line1 >> 7))) >> 7;

                nAlpha  = (nAlphaLine1  + nMapFY * ((nAlphaLine2  >> 7) - (nAlphaLine1 >> 7))) >> 7;

                *pDestScanline = ColorChannelMerge(*pDestScanline, nColor1, nAlpha);
                pDestScanline++;
                *pDestScanline = ColorChannelMerge(*pDestScanline, nColor2, nAlpha);
                pDestScanline++;
                *pDestScanline = ColorChannelMerge(*pDestScanline, nColor3, nAlpha);
                pDestScanline++;
                pDestScanline++;
            }
        }
    }
};

struct TradScaleContext
{
    std::unique_ptr<long[]> mpMapX;
    std::unique_ptr<long[]> mpMapY;

    TradScaleContext(tools::Rectangle const & aDstRect, tools::Rectangle const & aBitmapRect,
                 Size const & aOutSize, long nOffX, long nOffY)

        : mpMapX(new long[aDstRect.GetWidth()])
        , mpMapY(new long[aDstRect.GetHeight()])
    {
        const long nSrcWidth = aBitmapRect.GetWidth();
        const long nSrcHeight = aBitmapRect.GetHeight();

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

    static void generateSimpleMap(long nSrcDimension, long nDstDimension, long nDstLocation,
                                  long nOutDimention, long nOffset, bool bMirror, long* pMap)
    {
        long nMirrorOffset = 0;

        if (bMirror)
            nMirrorOffset = (nDstLocation << 1) + nSrcDimension - 1;

        for (long i = 0; i < nDstDimension; ++i, ++nOffset)
        {
            pMap[i] = nDstLocation + nOffset * nSrcDimension / nOutDimention;
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

    const long nDstWidth = aDstRect.GetWidth();
    const long nDstHeight = aDstRect.GetHeight();

    // calculate offset in original bitmap
    // in RTL case this is a little more complicated since the contents of the
    // bitmap is not mirrored (it never is), however the paint region and bmp region
    // are in mirrored coordinates, so the intersection of (aOutPt,aOutSz) with these
    // is content wise somewhere else and needs to take mirroring into account
    const long nOffX = IsRTLEnabled()
                            ? aOutSize.Width() - aDstRect.GetWidth() - (aDstRect.Left() - aOutPoint.X())
                            : aDstRect.Left() - aOutPoint.X();

    const long nOffY = aDstRect.Top() - aOutPoint.Y();

    TradScaleContext aTradContext(aDstRect, aBmpRect, aOutSize, nOffX, nOffY);

    Bitmap::ScopedReadAccess pBitmapReadAccess(const_cast<Bitmap&>(rBitmap));
    AlphaMask::ScopedReadAccess pAlphaReadAccess(const_cast<AlphaMask&>(rAlpha));

    DBG_ASSERT( pAlphaReadAccess->GetScanlineFormat() == ScanlineFormat::N8BitPal ||
                pAlphaReadAccess->GetScanlineFormat() == ScanlineFormat::N8BitTcMask,
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
    assert(!is_double_buffered_window());

    bool bDone = false;

    // try to paint directly
    const basegfx::B2DPoint aNull(aFullTransform * basegfx::B2DPoint(0.0, 0.0));
    const basegfx::B2DPoint aTopX(aFullTransform * basegfx::B2DPoint(1.0, 0.0));
    const basegfx::B2DPoint aTopY(aFullTransform * basegfx::B2DPoint(0.0, 1.0));
    SalBitmap* pSalSrcBmp = rBitmapEx.GetBitmap().ImplGetSalBitmap().get();
    SalBitmap* pSalAlphaBmp = nullptr;

    if(rBitmapEx.IsTransparent())
    {
        if(rBitmapEx.IsAlpha())
        {
            pSalAlphaBmp = rBitmapEx.GetAlpha().ImplGetSalBitmap().get();
        }
        else
        {
            pSalAlphaBmp = rBitmapEx.GetMask().ImplGetSalBitmap().get();
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
        const tools::Rectangle aRegionRectangle(GetActiveClipRegion().GetBoundRect());

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
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if(rBitmapEx.IsEmpty())
        return;

    // decompose matrix to check rotation and shear
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;
    rTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
    const bool bRotated(!basegfx::fTools::equalZero(fRotate));
    const bool bSheared(!basegfx::fTools::equalZero(fShearX));
    const bool bMirroredX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirroredY(basegfx::fTools::less(aScale.getY(), 0.0));

    const bool bMetafile = mpMetaFile != nullptr;

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

    // we have rotation,shear or mirror, check if some crazy mode needs the
    // created transformed bitmap
    const bool bInvert(RasterOp::Invert == meRasterOp);
    const bool bBitmapChangedColor(mnDrawMode & (DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap | DrawModeFlags::GrayBitmap ));
    bool bDone(false);
    const basegfx::B2DHomMatrix aFullTransform(GetViewTransformation() * rTransformation);
    const bool bTryDirectPaint(!bInvert && !bBitmapChangedColor && !bMetafile );

    if(bTryDirectPaint)
    {
        bDone = DrawTransformBitmapExDirect(aFullTransform, rBitmapEx);
    }

    if(!bDone)
    {
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

        // fallback; create transformed bitmap the hard way (back-transform
        // the pixels) and paint
        basegfx::B2DRange aVisibleRange(0.0, 0.0, 1.0, 1.0);

        // limit maximum area to something looking good for non-pixel-based targets (metafile, printer)
        // by using a fixed minimum (allow at least, but no need to utilize) for good smoothing and an area
        // dependent of original size for good quality when e.g. rotated/sheared. Still, limit to a maximum
        // to avoid crashes/resource problems (ca. 1500x3000 here)
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
                fMaximumArea);
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
    }
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

    bool bIsSizeValid = rSize.getWidth() != 0 && rSize.getHeight() != 0;

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
                                   const long            nMapX,
                                   const long            nMapY,
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
    mpGraphics->BlendBitmap( rPosAry, *rBmp.ImplGetSalBitmap(), this );
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
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4;
                int nOutX;

                Scanline pScanline = pW->GetScanline(nY);
                Scanline pScanlineAlpha = pAlphaW->GetScanline(nY);
                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
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
                const long  nMapY = pMapY[ nY ];
                Scanline pScanlineB = pB->GetScanline(nY);
                Scanline pScanlineAlpha = pAlphaW->GetScanline(nY);

                for( nX = 0; nX < nDstWidth; nX++ )
                {
                    const long nMapX = pMapX[ nX ];
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
        Bitmap::ScopedReadAccess pB(aBmp);
        BitmapScopedWriteAccess pW(aDither);

        if( pB && pP && pA && pW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                long nMapY = pMapY[ nY ];
                if (bVMirr)
                {
                    nMapY = aBmpRect.Bottom() - nMapY;
                }
                const long nModY = ( nOutY & 0x0FL ) << 4;
                int nOutX;

                Scanline pScanline = pW->GetScanline(nY);
                Scanline pScanlineAlpha = pA->GetScanline(nMapY);
                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    long  nMapX = pMapX[ nX ];
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
        if( pP && pA && pB )
        {
            if( !bHMirr && !bVMirr )
            {
                SalTwoRect aTR(aBmpRect.Left(), aBmpRect.Top(), aBmpRect.GetWidth(), aBmpRect.GetHeight(),
                               nOffX, nOffY, aOutSz.Width(), aOutSz.Height());

                bFastBlend = ImplFastBitmapBlending( *pB,*pP,*pA, aTR );
            }
        }

        if( pP && pA && pB && !bFastBlend )
        {
            switch( pP->GetScanlineFormat() )
            {
                case ScanlineFormat::N8BitPal:
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            long  nMapY = pMapY[ nY ];
                            if ( bVMirr )
                            {
                                nMapY = aBmpRect.Bottom() - nMapY;
                            }
                            Scanline pPScan = pP->GetScanline( nMapY );
                            Scanline pAScan = pA->GetScanline( nMapY );
                            Scanline pBScan = pB->GetScanline( nY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                long nMapX = pMapX[ nX ];

                                if ( bHMirr )
                                {
                                    nMapX = aBmpRect.Right() - nMapX;
                                }
                                aDstCol = pB->GetPixelFromData( pBScan, nX );
                                pB->SetPixelOnData( pBScan, nX, aDstCol.Merge( pP->GetPaletteColor( pPScan[ nMapX ] ),
                                                                                pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                default:
                {

                    for( nY = 0; nY < nDstHeight; nY++ )
                    {
                        long  nMapY = pMapY[ nY ];

                        if ( bVMirr )
                        {
                            nMapY = aBmpRect.Bottom() - nMapY;
                        }
                        Scanline pAScan = pA->GetScanline( nMapY );
                        Scanline pBScan = pB->GetScanline(nY);
                        for( nX = 0; nX < nDstWidth; nX++ )
                        {
                            long nMapX = pMapX[ nX ];

                            if ( bHMirr )
                            {
                                nMapX = aBmpRect.Right() - nMapX;
                            }
                            aDstCol = pB->GetPixelFromData( pBScan, nX );
                            pB->SetPixelOnData( pBScan, nX, aDstCol.Merge( pP->GetColor( nMapY, nMapX ),
                                                                 pAScan[ nMapX ] ) );
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
