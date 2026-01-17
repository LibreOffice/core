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

#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>

#include <vcl/image.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metaactiontypes.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/virdev.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <bitmap/bmpfast.hxx>
#include <drawmode.hxx>
#include <salbmp.hxx>
#include <salgdi.hxx>

void OutputDevice::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    assert(!is_double_buffered_window());

    const Size aSizePix(rBitmap.GetSizePixel());

    if (!rBitmap.HasAlpha())
    {
        DrawBitmap(rDestPt, PixelToLogic(aSizePix), Point(), aSizePix, rBitmap, MetaActionType::BMP);
        return;
    }

    DrawAlphaBitmap(rDestPt, PixelToLogic(aSizePix), Point(), aSizePix, rBitmap, MetaActionType::BMPEX);
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    assert(!is_double_buffered_window());

    if (!rBitmap.HasAlpha())
    {
        DrawBitmap(rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, MetaActionType::BMPSCALE);
        return;
    }

    DrawAlphaBitmap(rDestPt, rDestSize, Point(), rBitmap.GetSizePixel(), rBitmap, MetaActionType::BMPEXSCALE);
}

void OutputDevice::DrawBitmap( const Point& rDestPt, const Size& rDestSize,
                                   const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                   const Bitmap& rBitmap)
{
    assert(!is_double_buffered_window());

    if (!rBitmap.HasAlpha())
    {
        DrawBitmap(rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, MetaActionType::BMPSCALEPART);
        return;
    }

    DrawAlphaBitmap(rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, rBitmap, MetaActionType::BMPEXSCALEPART);
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

    if (mnDrawMode & (DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap))
    {
        sal_uInt8 cCmpVal;

        if (mnDrawMode & DrawModeFlags::BlackBitmap)
            cCmpVal = 0;
        else
            cCmpVal = 255;

        Color aCol(cCmpVal, cCmpVal, cCmpVal);
        auto popIt = ScopedPush(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);
        SetLineColor(aCol);
        SetFillColor(aCol);
        DrawRect(tools::Rectangle(rDestPt, rDestSize));
        return;
    }

    Bitmap aBmp(rBitmap);

    if (mnDrawMode & DrawModeFlags::GrayBitmap && !aBmp.IsEmpty())
        aBmp.Convert(BmpConversion::N8BitGreys);

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

    if (!mpGraphics && !AcquireGraphics())
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if (aBmp.IsEmpty())
        return;

    SalTwoRect aPosAry(rSrcPtPixel.X(), rSrcPtPixel.Y(), rSrcSizePixel.Width(), rSrcSizePixel.Height(),
                       ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                       ImplLogicWidthToDevicePixel(rDestSize.Width()),
                       ImplLogicHeightToDevicePixel(rDestSize.Height()));

    if (!aPosAry.mnSrcWidth || !aPosAry.mnSrcHeight || !aPosAry.mnDestWidth || !aPosAry.mnDestHeight)
        return;

    const BmpMirrorFlags nMirrFlags = AdjustTwoRect( aPosAry, aBmp.GetSizePixel() );

    if ( nMirrFlags != BmpMirrorFlags::NONE )
        aBmp.Mirror( nMirrFlags );

    if (!aPosAry.mnSrcWidth || !aPosAry.mnSrcHeight || !aPosAry.mnDestWidth || !aPosAry.mnDestHeight)
        return;

    if (nAction == MetaActionType::BMPSCALE && CanSubsampleBitmap())
    {
        double nScaleX = aPosAry.mnDestWidth  / static_cast<double>(aPosAry.mnSrcWidth);
        double nScaleY = aPosAry.mnDestHeight / static_cast<double>(aPosAry.mnSrcHeight);

        // If subsampling, use Bitmap::Scale() for subsampling of better quality.

        // but hidpi surfaces like the cairo one have their own scale, so don't downscale
        // past the surface scaling which can retain the extra detail
        double fScale(1.0);
        if (mpGraphics->ShouldDownscaleIconsAtSurface(fScale))
        {
            nScaleX *= fScale;
            nScaleY *= fScale;
        }

        if ( nScaleX < 1.0 || nScaleY < 1.0 )
        {
            aBmp.Scale(nScaleX, nScaleY);
            aPosAry.mnSrcWidth = aPosAry.mnDestWidth * fScale;
            aPosAry.mnSrcHeight = aPosAry.mnDestHeight * fScale;
        }
    }

    mpGraphics->DrawBitmap( aPosAry, *aBmp.ImplGetSalBitmap(), *this );
}

void OutputDevice::DrawAlphaBitmap( const Point& rDestPt, const Size& rDestSize,
                                 const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                 const Bitmap& rBitmap, const MetaActionType nAction )
{
    assert(!is_double_buffered_window());
    assert(rBitmap.HasAlpha());

    if( ImplIsRecordLayout() )
        return;

    if (RasterOp::Invert == meRasterOp)
    {
        DrawRect(tools::Rectangle(rDestPt, rDestSize));
        return;
    }

    Bitmap aBmp(vcl::drawmode::GetBitmap(rBitmap, GetDrawMode()));

    if (mpMetaFile)
    {
        switch(nAction)
        {
            case MetaActionType::BMPEX:
                mpMetaFile->AddAction(new MetaBmpExAction(rDestPt, aBmp));
                break;

            case MetaActionType::BMPEXSCALE:
                mpMetaFile->AddAction(new MetaBmpExScaleAction(rDestPt, rDestSize, aBmp));
                break;

            case MetaActionType::BMPEXSCALEPART:
                mpMetaFile->AddAction(new MetaBmpExScalePartAction(rDestPt, rDestSize,
                                                                   rSrcPtPixel, rSrcSizePixel, aBmp));
                break;

            default:
                break;
        }
    }

    if (!IsDeviceOutputNecessary())
        return;

    if (!mpGraphics && !AcquireGraphics())
        return;

    if (mbInitClipRegion)
        InitClipRegion();

    if (mbOutputClipped)
        return;

    DrawDeviceBitmap(rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp);
}

void OutputDevice::DrawDeviceBitmap( const Point& rDestPt, const Size& rDestSize,
                                     const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                     Bitmap& rBitmap )
{
    assert(!is_double_buffered_window());

    if (rBitmap.HasAlpha())
    {
        DrawDeviceAlphaBitmap(rBitmap,
                              rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel);
        return;
    }

    if (rBitmap.IsEmpty())
        return;

    SalTwoRect aPosAry(rSrcPtPixel.X(), rSrcPtPixel.Y(), rSrcSizePixel.Width(),
                       rSrcSizePixel.Height(), ImplLogicXToDevicePixel(rDestPt.X()),
                       ImplLogicYToDevicePixel(rDestPt.Y()),
                       ImplLogicWidthToDevicePixel(rDestSize.Width()),
                       ImplLogicHeightToDevicePixel(rDestSize.Height()));

    const BmpMirrorFlags nMirrFlags = AdjustTwoRect(aPosAry, rBitmap.GetSizePixel());

    if (!(aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight))
        return;

    if (nMirrFlags != BmpMirrorFlags::NONE)
        rBitmap.Mirror(nMirrFlags);

    const SalBitmap* pSalSrcBmp = rBitmap.ImplGetSalBitmap().get();

    assert(!rBitmap.HasAlpha()
            && "I removed some code here that will need to be restored");

    mpGraphics->DrawBitmap(aPosAry, *pSalSrcBmp, *this);
}

Bitmap OutputDevice::GetBitmap( const Point& rSrcPt, const Size& rSize ) const
{
    if ( !mpGraphics && !AcquireGraphics() )
        return Bitmap();

    assert(mpGraphics);

    tools::Long    nX = ImplLogicXToDevicePixel( rSrcPt.X() );
    tools::Long    nY = ImplLogicYToDevicePixel( rSrcPt.Y() );
    tools::Long    nWidth = ImplLogicWidthToDevicePixel( rSize.Width() );
    tools::Long    nHeight = ImplLogicHeightToDevicePixel( rSize.Height() );
    if ( nWidth <= 0 || nHeight <= 0 || nX > (mnOutWidth + mnOutOffX) || nY > (mnOutHeight + mnOutOffY))
        return Bitmap();

    tools::Rectangle   aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
    bool bClipped = false;

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

    if (bClipped)
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

                return aVDev->GetBitmap( Point(), aVDev->GetOutputSizePixel() );
            }
        }
    }

    std::shared_ptr<SalBitmap> pSalBmp;
    // if we are a virtual device, we might need to remove the unused alpha channel
    bool bWithoutAlpha = false;
    if (OUTDEV_VIRDEV == GetOutDevType())
        bWithoutAlpha = static_cast<const VirtualDevice*>(this)->IsWithoutAlpha();

    pSalBmp = mpGraphics->GetBitmap( nX, nY, nWidth, nHeight, *this, bWithoutAlpha );

    Bitmap aBmp;

    if( pSalBmp )
        aBmp.ImplSetSalBitmap(pSalBmp);

    return aBmp;
}

void OutputDevice::DrawDeviceAlphaBitmap( const Bitmap& rBmp,
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

    Point aRelPt = aOutPt + Point(mnOutOffX, mnOutOffY);
    SalTwoRect aTR(
        rSrcPtPixel.X(), rSrcPtPixel.Y(),
        rSrcSizePixel.Width(), rSrcSizePixel.Height(),
        aRelPt.X(), aRelPt.Y(),
        aOutSz.Width(), aOutSz.Height());

    Bitmap bitmap(rBmp);
    if(bHMirr || bVMirr)
    {
        bitmap.Mirror(mirrorFlags);
    }
    SalBitmap* pSalSrcBmp = bitmap.ImplGetSalBitmap().get();
    mpGraphics->DrawAlphaBitmap(aTR, *pSalSrcBmp, *this);
}

bool OutputDevice::HasFastDrawTransformedBitmap() const
{
    if( ImplIsRecordLayout() )
        return false;

    if (!mpGraphics && !AcquireGraphics())
        return false;
    assert(mpGraphics);

    return mpGraphics->HasFastDrawTransformedBitmap();
}

bool OutputDevice::DrawTransformedBitmap(
        const basegfx::B2DHomMatrix& aFullTransform,
        const Bitmap& rBitmap,
        double fAlpha)
{
    assert(!is_double_buffered_window());

    // try to paint directly
    const basegfx::B2DPoint aNull(aFullTransform * basegfx::B2DPoint(0.0, 0.0));
    const basegfx::B2DPoint aTopX(aFullTransform * basegfx::B2DPoint(1.0, 0.0));
    const basegfx::B2DPoint aTopY(aFullTransform * basegfx::B2DPoint(0.0, 1.0));
    SalBitmap* pSalSrcBmp = rBitmap.ImplGetSalBitmap().get();

    return mpGraphics->DrawTransformedBitmap(
        aNull,
        aTopX,
        aTopY,
        *pSalSrcBmp,
        fAlpha,
        *this);
};

void OutputDevice::DrawImage( const Point& rPos, const Image& rImage, DrawImageFlags nStyle )
{
    assert(!is_double_buffered_window());

    DrawImage( rPos, Size(), rImage, nStyle );
}

void OutputDevice::DrawImage( const Point& rPos, const Size& rSize,
                              const Image& rImage, DrawImageFlags nStyle )
{
    assert(!is_double_buffered_window());

    if (!ImplIsRecordLayout())
    {
        if (!rSize.IsEmpty())
            rImage.Draw(this, rPos, nStyle, &rSize);
        else
            rImage.Draw(this, rPos, nStyle);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
