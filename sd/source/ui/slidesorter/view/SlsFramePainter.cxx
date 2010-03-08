/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive2d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "SlsFramePainter.hxx"
#include <vcl/outdev.hxx>


namespace sd { namespace slidesorter { namespace view {

FramePainter::FramePainter (const BitmapEx& rShadowBitmap)
    : maShadowTopLeft(rShadowBitmap,-1,-1),
      maShadowTop(rShadowBitmap,0,-1),
      maShadowTopRight(rShadowBitmap,+1,-1),
      maShadowLeft(rShadowBitmap,-1,0),
      maShadowRight(rShadowBitmap,+1,0),
      maShadowBottomLeft(rShadowBitmap,-1,+1),
      maShadowBottom(rShadowBitmap,0,+1),
      maShadowBottomRight(rShadowBitmap,+1,+1),
      maShadowCenter(rShadowBitmap,0,0),
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
    maShadowTopLeft.PaintCorner(rDevice, aBox.TopLeft());
    maShadowTopRight.PaintCorner(rDevice, aBox.TopRight());
    maShadowBottomLeft.PaintCorner(rDevice, aBox.BottomLeft());
    maShadowBottomRight.PaintCorner(rDevice, aBox.BottomRight());
    maShadowLeft.PaintSide(rDevice,
        aBox.TopLeft(), aBox.BottomLeft(),
        maShadowTopLeft, maShadowBottomLeft);
    maShadowRight.PaintSide(rDevice,
        aBox.TopRight(), aBox.BottomRight(),
        maShadowTopRight, maShadowBottomRight);
    maShadowTop.PaintSide(rDevice,
        aBox.TopLeft(), aBox.TopRight(),
        maShadowTopLeft, maShadowTopRight);
    maShadowBottom.PaintSide(rDevice,
        aBox.BottomLeft(), aBox.BottomRight(),
        maShadowBottomLeft, maShadowBottomRight);
    maShadowCenter.PaintCenter(rDevice,aBox);
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
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(std::min(aBitmapSize.Width(), nRight-nX+1),aBitmapSize.Height()),
                maBitmap);
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
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(aBitmapSize.Width(), std::min(aBitmapSize.Height(), nBottom-nY+1)),
                maBitmap);
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
