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

#include <config_features.h>

#include <rtl/math.hxx>
#include <comphelper/lok.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

void OutputDevice::DrawBitmapEx( const Point& rDestPt,
                                 const BitmapEx& rBitmapEx )
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if( !rBitmapEx.IsAlpha() )
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

    if ( !rBitmapEx.IsAlpha() )
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
                                 const BitmapEx& rBitmapEx)
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if ( !rBitmapEx.IsAlpha() )
    {
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx.GetBitmap() );
    }
    else
    {
        DrawBitmapEx( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmapEx, MetaActionType::BMPEXSCALEPART );
    }
}

void OutputDevice::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const BitmapEx& rBitmapEx, const MetaActionType nAction )
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if( !rBitmapEx.IsAlpha() )
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

        BitmapEx aBmpEx(vcl::drawmode::GetBitmapEx(rBitmapEx, GetDrawMode()));

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

        DrawDeviceBitmapEx( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmpEx );
    }
}

BitmapEx OutputDevice::GetBitmapEx( const Point& rSrcPt, const Size& rSize ) const
{

    // #110958# Extract alpha value from VDev, if any
    if( mpAlphaVDev )
    {
        Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( rSrcPt, rSize ) );

        // ensure 8 bit alpha
        if (aAlphaBitmap.getPixelFormat() > vcl::PixelFormat::N8_BPP)
            aAlphaBitmap.Convert( BmpConversion::N8BitNoConversion );

        return BitmapEx(GetBitmap( rSrcPt, rSize ), AlphaMask( aAlphaBitmap ) );
    }
    else
        return BitmapEx(GetBitmap( rSrcPt, rSize ));
}

void OutputDevice::DrawDeviceBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                     const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                     BitmapEx& rBitmapEx )
{
    assert(!is_double_buffered_window());

    if (rBitmapEx.IsAlpha())
    {
        DrawDeviceAlphaBitmap(rBitmapEx.GetBitmap(), rBitmapEx.GetAlpha(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel);
    }
    else if (!rBitmapEx.IsEmpty())
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
            std::shared_ptr<SalBitmap> xMaskBmp = rBitmapEx.maAlphaMask.ImplGetSalBitmap();

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
                                              BitmapEx(rBitmapEx.GetAlpha(),
                                                       rBitmapEx.GetAlpha()));
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

bool OutputDevice::DrawTransformBitmapExDirect(
        const basegfx::B2DHomMatrix& aFullTransform,
        const BitmapEx& rBitmapEx,
        double fAlpha)
{
    assert(!is_double_buffered_window());

    bool bDone = false;

    // try to paint directly
    const basegfx::B2DPoint aNull(aFullTransform * basegfx::B2DPoint(0.0, 0.0));
    const basegfx::B2DPoint aTopX(aFullTransform * basegfx::B2DPoint(1.0, 0.0));
    const basegfx::B2DPoint aTopY(aFullTransform * basegfx::B2DPoint(0.0, 1.0));
    SalBitmap* pSalSrcBmp = rBitmapEx.GetBitmap().ImplGetSalBitmap().get();
    Bitmap aAlphaBitmap;

    if(rBitmapEx.IsAlpha())
    {
        aAlphaBitmap = rBitmapEx.GetAlpha();
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
        fAlpha,
        *this);

    if (mpAlphaVDev)
    {
        // Merge bitmap alpha to alpha device
        AlphaMask aAlpha(rBitmapEx.GetSizePixel());
        aAlpha.Erase( ( 1 - fAlpha ) * 255 );
        mpAlphaVDev->DrawTransformBitmapExDirect(aFullTransform, BitmapEx(aAlpha, aAlphaBitmap));
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
    const BitmapEx& rBitmapEx,
    double fAlpha)
{
    assert(!is_double_buffered_window());

    if( ImplIsRecordLayout() )
        return;

    if(rBitmapEx.IsEmpty())
        return;

    if(rtl::math::approxEqual( fAlpha, 0.0 ))
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

    BitmapEx bitmapEx = rBitmapEx;

    const bool bInvert(RasterOp::Invert == meRasterOp);
    const bool bBitmapChangedColor(mnDrawMode & (DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap | DrawModeFlags::GrayBitmap ));
    const bool bTryDirectPaint(!bInvert && !bBitmapChangedColor && !bMetafile);
    // tdf#130768 CAUTION(!) using GetViewTransformation() is *not* enough here, it may
    // be that mnOutOffX/mnOutOffY is used - see AOO bug 75163, mentioned at
    // ImplGetDeviceTransformation declaration
    basegfx::B2DHomMatrix aFullTransform(ImplGetDeviceTransformation() * rTransformation);

    // First try to handle additional alpha blending, either directly, or modify the bitmap.
    if(!rtl::math::approxEqual( fAlpha, 1.0 ))
    {
        if(bTryDirectPaint)
        {
            if(DrawTransformBitmapExDirect(aFullTransform, bitmapEx, fAlpha))
            {
                // we are done
                return;
            }
        }
        // Apply the alpha manually.
        sal_uInt8 nColor( static_cast<sal_uInt8>( ::basegfx::fround( 255.0*(1.0 - fAlpha) + .5) ) );
        AlphaMask aAlpha( bitmapEx.GetSizePixel(), &nColor );
        if( bitmapEx.IsAlpha())
            aAlpha.BlendWith( bitmapEx.GetAlpha());
        bitmapEx = BitmapEx( bitmapEx.GetBitmap(), aAlpha );
    }
    if(rtl::math::approxEqual( fAlpha, 1.0 ))
        fAlpha = 1.0; // avoid the need for approxEqual in backends

    if(bTryDirectPaint && DrawTransformBitmapExDirect(aFullTransform, bitmapEx))
        return;

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

        DrawBitmapEx(aDestPt, aDestSize, bitmapEx);
        if (!bMetafile && comphelper::LibreOfficeKit::isActive() && GetMapMode().GetMapUnit() != MapUnit::MapPixel)
        {
            EnableMapMode();
            aDestPt.Move(-aOrigin.getX(), -aOrigin.getY());
        }
        return;
    }

    if(bTryDirectPaint && DrawTransformBitmapExDirect(aFullTransform, bitmapEx))
    {
        // we are done
        return;
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

        DrawBitmapEx(aDestPt, aDestSize, bitmapEx);
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
    const Size& rOriginalSizePixel(bitmapEx.GetSizePixel());
    const double fOrigArea(rOriginalSizePixel.Width() * rOriginalSizePixel.Height() * 0.5);
    const double fOrigAreaScaled(fOrigArea * 1.44);
    double fMaximumArea(std::clamp(fOrigAreaScaled, 1000000.0, 4500000.0));

    if(!bMetafile)
    {
        if ( !TransformAndReduceBitmapExToTargetRange( aFullTransform, aVisibleRange, fMaximumArea ) )
            return;
    }

    if(aVisibleRange.isEmpty())
        return;

    BitmapEx aTransformed(bitmapEx);

    // #122923# when the result needs an alpha channel due to being rotated or sheared
    // and thus uncovering areas, add these channels so that the own transformer (used
    // in getTransformed) also creates a transformed alpha channel
    if(!aTransformed.IsAlpha() && (bSheared || bRotated))
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
        fFullRotate = fmod(fFullRotate * -1, 2 * M_PI);
        if (fFullRotate < 0)
        {
            fFullRotate += 2 * M_PI;
        }
        Degree10 nAngle10(basegfx::fround(basegfx::rad2deg<10>(fFullRotate)));
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

bool OutputDevice::ImplementsFastDrawTransformedBitmap() const
{
    return mpGraphics->ImplementsFastDrawTransformedBitmap(!GetOutputSizePixel().IsEmpty());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
