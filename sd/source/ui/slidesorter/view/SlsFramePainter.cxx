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

#include "precompiled_sd.hxx"

#include "SlsFramePainter.hxx"
#include <vcl/outdev.hxx>
#include <vcl/bmpacc.hxx>


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




FramePainter::~FramePainter (void)
{
}




void FramePainter::PaintFrame (
    OutputDevice& rDevice,
    const Rectangle aBox) const
{
    if ( ! mbIsValid)
        return;

    // Paint the shadow.
    maTopLeft.PaintCorner(rDevice, aBox.TopLeft());
    maTopRight.PaintCorner(rDevice, aBox.TopRight());
    maBottomLeft.PaintCorner(rDevice, aBox.BottomLeft());
    maBottomRight.PaintCorner(rDevice, aBox.BottomRight());
    maLeft.PaintSide(rDevice, aBox.TopLeft(), aBox.BottomLeft(), maTopLeft, maBottomLeft);
    maRight.PaintSide(rDevice, aBox.TopRight(), aBox.BottomRight(), maTopRight, maBottomRight);
    maTop.PaintSide(rDevice, aBox.TopLeft(), aBox.TopRight(), maTopLeft, maTopRight);
    maBottom.PaintSide(rDevice, aBox.BottomLeft(), aBox.BottomRight(), maBottomLeft, maBottomRight);
    maCenter.PaintCenter(rDevice,aBox);
}




void FramePainter::AdaptColor (
    const Color aNewColor,
    const bool bEraseCenter)
{
    // Get the source color.
    if (maCenter.maBitmap.IsEmpty())
        return;
    BitmapReadAccess* pReadAccess = maCenter.maBitmap.GetBitmap().AcquireReadAccess();
    if (pReadAccess == NULL)
        return;
    const Color aSourceColor = pReadAccess->GetColor(0,0);
    maCenter.maBitmap.GetBitmap().ReleaseAccess(pReadAccess);

    // Erase the center bitmap.
    if (bEraseCenter)
        maCenter.maBitmap.SetEmpty();

    // Replace the color in all bitmaps.
    maTopLeft.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maTop.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maTopRight.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maLeft.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maCenter.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maRight.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maBottomLeft.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maBottom.maBitmap.Replace(aSourceColor, aNewColor, 0);
    maBottomRight.maBitmap.Replace(aSourceColor, aNewColor, 0);
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
        maBitmap.Scale(Size(nSideBitmapSize,nSideBitmapSize), BMP_SCALE_FAST);
    }
    else if (nHorizontalPosition == 0)
    {
        maBitmap.Scale(Size(nSideBitmapSize,aSize.Height()), BMP_SCALE_FAST);
    }
    else if (nVerticalPosition == 0)
    {
        maBitmap.Scale(Size(maBitmap.GetSizePixel().Width(), nSideBitmapSize), BMP_SCALE_FAST);
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
        // Diagonal sides indicatee an error.
        OSL_ASSERT(false);
    }
}




void FramePainter::OffsetBitmap::PaintCenter (
    OutputDevice& rDevice,
    const Rectangle& rBox) const
{
    const Size aBitmapSize (maBitmap.GetSizePixel());
    for (sal_Int32 nY=rBox.Top(); nY<=rBox.Bottom(); nY+=aBitmapSize.Height())
        for (sal_Int32 nX=rBox.Left(); nX<=rBox.Right(); nX+=aBitmapSize.Width())
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(
                    ::std::min(aBitmapSize.Width(), rBox.Right()-nX+1),
                    std::min(aBitmapSize.Height(), rBox.Bottom()-nY+1)),
                maBitmap);
}



} } } // end of namespace sd::slidesorter::view
