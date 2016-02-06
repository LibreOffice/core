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

#include "SlsFramePainter.hxx"
#include <vcl/outdev.hxx>
#include <vcl/bitmapaccess.hxx>

namespace sd { namespace slidesorter { namespace view {

FramePainter::FramePainter (const BitmapEx& rShadowBitmap)
    : maTopLeft(rShadowBitmap,-1,-1),
      maTop(rShadowBitmap,0,-1),
      maTopRight(rShadowBitmap,+1,-1),
      maLeft(rShadowBitmap,-1,0),
      maRight(rShadowBitmap,+1,0),
      maBottomLeft(rShadowBitmap,-1,+1),
      maBottom(rShadowBitmap,0,+1),
      maBottomRight(rShadowBitmap,+1,+1),
      maCenter(rShadowBitmap,0,0),
      mbIsValid(false)
{
    if (rShadowBitmap.GetSizePixel().Width() == rShadowBitmap.GetSizePixel().Height()
        && (rShadowBitmap.GetSizePixel().Width()-1)%2 == 0
        && ((rShadowBitmap.GetSizePixel().Width()-1)/2)%2 == 1)
    {
        mbIsValid = true;
    }
    else
    {
        OSL_ASSERT(rShadowBitmap.GetSizePixel().Width() == rShadowBitmap.GetSizePixel().Height());
        OSL_ASSERT((rShadowBitmap.GetSizePixel().Width()-1)%2 == 0);
        OSL_ASSERT(((rShadowBitmap.GetSizePixel().Width()-1)/2)%2 == 1);
    }
}

FramePainter::~FramePainter()
{
}

void FramePainter::PaintFrame (
    OutputDevice& rDevice,
    const Rectangle& rBox) const
{
    if ( ! mbIsValid)
        return;

    // Paint the shadow.
    maTopLeft.PaintCorner(rDevice, rBox.TopLeft());
    maTopRight.PaintCorner(rDevice, rBox.TopRight());
    maBottomLeft.PaintCorner(rDevice, rBox.BottomLeft());
    maBottomRight.PaintCorner(rDevice, rBox.BottomRight());
    maLeft.PaintSide(rDevice, rBox.TopLeft(), rBox.BottomLeft(), maTopLeft, maBottomLeft);
    maRight.PaintSide(rDevice, rBox.TopRight(), rBox.BottomRight(), maTopRight, maBottomRight);
    maTop.PaintSide(rDevice, rBox.TopLeft(), rBox.TopRight(), maTopLeft, maTopRight);
    maBottom.PaintSide(rDevice, rBox.BottomLeft(), rBox.BottomRight(), maBottomLeft, maBottomRight);
    maCenter.PaintCenter(rDevice,rBox);
}

void FramePainter::AdaptColor (
    const Color aNewColor,
    const bool bEraseCenter)
{
    // Get the source color.
    if (maCenter.maBitmap.IsEmpty())
        return;
    BitmapReadAccess* pReadAccess = maCenter.maBitmap.GetBitmap().AcquireReadAccess();
    if (pReadAccess == nullptr)
        return;
    const Color aSourceColor = pReadAccess->GetColor(0,0);
    Bitmap::ReleaseAccess(pReadAccess);

    // Erase the center bitmap.
    if (bEraseCenter)
        maCenter.maBitmap.SetEmpty();

    // Replace the color in all bitmaps.
    maTopLeft.maBitmap.Replace(aSourceColor, aNewColor);
    maTop.maBitmap.Replace(aSourceColor, aNewColor);
    maTopRight.maBitmap.Replace(aSourceColor, aNewColor);
    maLeft.maBitmap.Replace(aSourceColor, aNewColor);
    maCenter.maBitmap.Replace(aSourceColor, aNewColor);
    maRight.maBitmap.Replace(aSourceColor, aNewColor);
    maBottomLeft.maBitmap.Replace(aSourceColor, aNewColor);
    maBottom.maBitmap.Replace(aSourceColor, aNewColor);
    maBottomRight.maBitmap.Replace(aSourceColor, aNewColor);
}

//===== FramePainter::OffsetBitmap ============================================

FramePainter::OffsetBitmap::OffsetBitmap (
    const BitmapEx& rBitmap,
    const sal_Int32 nHorizontalPosition,
    const sal_Int32 nVerticalPosition)
    : maBitmap(),
      maOffset()
{
    OSL_ASSERT(nHorizontalPosition>=-1 && nHorizontalPosition<=+1);
    OSL_ASSERT(nVerticalPosition>=-1 && nVerticalPosition<=+1);

    const sal_Int32 nS (1);
    const sal_Int32 nC (::std::max<sal_Int32>(0,(rBitmap.GetSizePixel().Width()-nS)/2));
    const sal_Int32 nO (nC/2);

    const Point aOrigin(
        nHorizontalPosition<0 ? 0 : (nHorizontalPosition == 0 ? nC : nC+nS),
        nVerticalPosition<0 ? 0 : (nVerticalPosition == 0 ? nC : nC+nS));
    const Size aSize(
        nHorizontalPosition==0 ? nS : nC,
        nVerticalPosition==0 ? nS : nC);
    maBitmap = BitmapEx(rBitmap, aOrigin, aSize);
    if (maBitmap.IsEmpty())
        return;
    maOffset = Point(
        nHorizontalPosition<0 ? -nO : nHorizontalPosition>0 ? -nO : 0,
        nVerticalPosition<0 ? -nO : nVerticalPosition>0 ? -nO : 0);

    // Enlarge the side bitmaps so that painting the frame requires less
    // paint calls.
    const sal_Int32 nSideBitmapSize (64);
    if (nHorizontalPosition == 0 && nVerticalPosition == 0)
    {
        maBitmap.Scale(Size(nSideBitmapSize,nSideBitmapSize));
    }
    else if (nHorizontalPosition == 0)
    {
        maBitmap.Scale(Size(nSideBitmapSize,aSize.Height()));
    }
    else if (nVerticalPosition == 0)
    {
        maBitmap.Scale(Size(maBitmap.GetSizePixel().Width(), nSideBitmapSize));
    }
}

void FramePainter::OffsetBitmap::PaintCorner (
    OutputDevice& rDevice,
    const Point& rAnchor) const
{
    if ( ! maBitmap.IsEmpty())
        rDevice.DrawBitmapEx(rAnchor+maOffset, maBitmap);
}

void FramePainter::OffsetBitmap::PaintSide (
    OutputDevice& rDevice,
    const Point& rAnchor1,
    const Point& rAnchor2,
    const OffsetBitmap& rCornerBitmap1,
    const OffsetBitmap& rCornerBitmap2) const
{
    if (maBitmap.IsEmpty())
        return;

    const Size aBitmapSize (maBitmap.GetSizePixel());
    if (rAnchor1.Y() == rAnchor2.Y())
    {
        // Side is horizontal.
        const sal_Int32 nY (rAnchor1.Y() + maOffset.Y());
        const sal_Int32 nLeft (
            rAnchor1.X()
            + rCornerBitmap1.maBitmap.GetSizePixel().Width()
            + rCornerBitmap1.maOffset.X());
        const sal_Int32 nRight (
            rAnchor2.X()
            + rCornerBitmap2.maOffset.X()\
            - 1);
        for (sal_Int32 nX=nLeft; nX<=nRight; nX+=aBitmapSize.Width())
        {
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(std::min(aBitmapSize.Width(),static_cast<long>(nRight-nX+1)),aBitmapSize.Height()),
                maBitmap);
        }
    }
    else if (rAnchor1.X() == rAnchor2.X())
    {
        // Side is vertical.
        const sal_Int32 nX (rAnchor1.X() + maOffset.X());
        const sal_Int32 nTop (
            rAnchor1.Y()
            + rCornerBitmap1.maBitmap.GetSizePixel().Height()
            + rCornerBitmap1.maOffset.Y());
        const sal_Int32 nBottom (
            rAnchor2.Y()
            + rCornerBitmap2.maOffset.Y()
            - 1);
        for (sal_Int32 nY=nTop; nY<=nBottom; nY+=aBitmapSize.Height())
        {
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(aBitmapSize.Width(), std::min(aBitmapSize.Height(), static_cast<long>(nBottom-nY+1))),
                maBitmap);
        }
    }
    else
    {
        // Diagonal sides indicates an error.
        OSL_ASSERT(false);
    }
}

void FramePainter::OffsetBitmap::PaintCenter (
    OutputDevice& rDevice,
    const Rectangle& rBox) const
{
    const Size aBitmapSize (maBitmap.GetSizePixel());
    for (long nY=rBox.Top(); nY<=rBox.Bottom(); nY+=aBitmapSize.Height())
        for (long nX=rBox.Left(); nX<=rBox.Right(); nX+=aBitmapSize.Width())
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(
                    ::std::min(aBitmapSize.Width(), rBox.Right()-nX+1),
                    std::min(aBitmapSize.Height(), rBox.Bottom()-nY+1)),
                maBitmap);
}

} } } // end of namespace sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
