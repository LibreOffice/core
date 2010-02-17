/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewCacheContext.hxx,v $
 *
 * $Revision: 1.3 $
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

#include "view/SlsPageObjectLayouter.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "view/SlsFontProvider.hxx"
#include "tools/IconCache.hxx"
#include "Window.hxx"
#include "res_bmp.hrc"

namespace sd { namespace slidesorter { namespace view {

namespace {
const static sal_Int32 gnPageNumberOffset = 5;
const static sal_Int32 gnPageNumberFrameHorizontalOffset = 2;
const static sal_Int32 gnPageNumberFrameVerticalOffset = 1;
const static sal_Int32 gnOuterBorderWidth = 6;
const static sal_Int32 gnInfoAreaMinWidth = 26;
const static Size gaButtonSize (32,32);
const static sal_Int32 gnButtonGap (5);
}

PageObjectLayouter::PageObjectLayouter (
    const Size& rPageObjectWindowSize,
    const Size& rPageSize,
    const SharedSdWindow& rpWindow,
    const sal_Int32 nPageCount)
    : mpWindow(rpWindow),
      maPageObjectSize(rPageObjectWindowSize.Width(), rPageObjectWindowSize.Height()),
      mnModelToWindowScale(1),
      maPageObjectBoundingBox(),
      maPageNumberAreaBoundingBox(),
      maPreviewBoundingBox(),
      maTransitionEffectBoundingBox(),
      maButtonAreaBoundingBox(),
      maTransitionEffectIcon(IconCache::Instance().GetIcon(BMP_FADE_EFFECT_INDICATOR))
{
    const Size aPageNumberAreaSize (GetPageNumberAreaSize(nPageCount));

    const int nMaximumBorderWidth (gnOuterBorderWidth);

    maPreviewBoundingBox = CalculatePreviewBoundingBox(
        maPageObjectSize,
        Size(rPageSize.Width(), rPageSize.Height()),
        aPageNumberAreaSize.Width());
    maPageObjectBoundingBox = Rectangle(Point(0,0), maPageObjectSize);

    maPageNumberAreaBoundingBox = Rectangle(
        Point(
            maPreviewBoundingBox.Left()
                - gnPageNumberOffset
                - gnPageNumberFrameHorizontalOffset
                - aPageNumberAreaSize.Width(),
            nMaximumBorderWidth),
        aPageNumberAreaSize);

    maPageNumberFrameBoundingBox = Rectangle(
        maPageNumberAreaBoundingBox.Left() - gnPageNumberFrameHorizontalOffset,
        maPageNumberAreaBoundingBox.Top() - gnPageNumberFrameVerticalOffset,
        maPageNumberAreaBoundingBox.Right() + gnPageNumberFrameHorizontalOffset,
        maPageNumberAreaBoundingBox.Bottom() + gnPageNumberFrameVerticalOffset);

    const Size aIconSize (maTransitionEffectIcon.GetSizePixel());
    maTransitionEffectBoundingBox = Rectangle(
        Point(
            (maPreviewBoundingBox.Left() - aIconSize.Width()) / 2,
            maPreviewBoundingBox.Bottom() - aIconSize.Height()),
        aIconSize);

    maButtonAreaBoundingBox = Rectangle(
        0,
        maPageObjectBoundingBox.Bottom() - gaButtonSize.Height() - gnButtonGap,
        maPageObjectBoundingBox.Right() - gnButtonGap,
        maPageObjectBoundingBox.Bottom() - gnButtonGap);
}




PageObjectLayouter::~PageObjectLayouter(void)
{
}




Rectangle PageObjectLayouter::CalculatePreviewBoundingBox (
    Size& rPageObjectSize,
    const Size& rPageSize,
    const sal_Int32 nPageNumberAreaWidth)
{
    const sal_Int32 nIconWidth (maTransitionEffectIcon.GetSizePixel().Width());
    const sal_Int32 nLeftAreaWidth (
        ::std::max(
            gnInfoAreaMinWidth,
            2*gnPageNumberOffset
                + ::std::max(
                    nPageNumberAreaWidth + 2 * gnPageNumberFrameHorizontalOffset,
                    nIconWidth)));
    sal_Int32 nPreviewWidth;
    sal_Int32 nPreviewHeight;
    const double nPageAspectRatio (double(rPageSize.Width()) / double(rPageSize.Height()));
    if (rPageObjectSize.Height() == 0)
    {
        // Calculate height so that the preview fills the available
        // horizontal space completely while observing the aspect ratio of
        // the preview.
        nPreviewWidth = rPageObjectSize.Width() - nLeftAreaWidth - gnOuterBorderWidth - 1;
        nPreviewHeight = ::basegfx::fround(nPreviewWidth / nPageAspectRatio);
        rPageObjectSize.setHeight(nPreviewHeight + 2*gnOuterBorderWidth + 1);
    }
    else if (rPageObjectSize.Width() == 0)
    {
        // Calculate the width of the page object so that the preview fills
        // the available vertical space completely while observing the
        // aspect ratio of the preview.
        nPreviewHeight = rPageObjectSize.Height() - 2*gnOuterBorderWidth - 1;
        nPreviewWidth = ::basegfx::fround(nPreviewHeight * nPageAspectRatio);
        rPageObjectSize.setWidth(nPreviewWidth + nLeftAreaWidth + gnOuterBorderWidth + 1);

    }
    else
    {
        // The size of the page object is given.  Calculate the size of the
        // preview.
        nPreviewWidth = rPageObjectSize.Width() - nLeftAreaWidth - gnOuterBorderWidth - 1;
        nPreviewHeight = rPageObjectSize.Height() - gnOuterBorderWidth - 1;
        if (double(nPreviewWidth)/double(nPreviewHeight) > nPageAspectRatio)
            nPreviewWidth = ::basegfx::fround(nPreviewHeight * nPageAspectRatio);
        else
            nPreviewHeight = ::basegfx::fround(nPreviewWidth / nPageAspectRatio);
    }
    // When the preview does not fill the available space completely then
    // place it flush right and vertically centered.
    const int nLeft (rPageObjectSize.Width() - gnOuterBorderWidth - nPreviewWidth - 1);
    const int nTop ((rPageObjectSize.Height() - nPreviewHeight)/2);
    return Rectangle(
        nLeft,
        nTop,
        nLeft + nPreviewWidth,
        nTop + nPreviewHeight);
}




Rectangle PageObjectLayouter::GetBoundingBox (
    const model::SharedPageDescriptor& rpPageDescriptor,
    const Part ePart,
    const CoordinateSystem eCoordinateSystem,
    const sal_Int32 nIndex)
{
    Rectangle aBoundingBox;
    switch (ePart)
    {
        case PageObject:
        case MouseOverIndicator:
            aBoundingBox = maPageObjectBoundingBox;
            break;

        case Preview:
            aBoundingBox = maPreviewBoundingBox;
            break;

        case PageNumber:
            aBoundingBox = maPageNumberAreaBoundingBox;
            break;

        case PageNumberFrame:
            aBoundingBox = maPageNumberFrameBoundingBox;
            break;

        case Name:
            aBoundingBox = maPageNumberAreaBoundingBox;
            break;

        case TransitionEffectIndicator:
            aBoundingBox = maTransitionEffectBoundingBox;
            break;

        case ButtonArea:
            aBoundingBox = maButtonAreaBoundingBox;
            break;

        case Button:
            aBoundingBox = Rectangle(
                maPageObjectBoundingBox.BottomRight()
                    - Point(
                        (nIndex+1)*(gaButtonSize.Width() + gnButtonGap),
                        gaButtonSize.Height() + gnButtonGap),
                gaButtonSize);
            break;
    }

    Point aLocation (rpPageDescriptor ? rpPageDescriptor->GetLocation() : Point(0,0));
    if (eCoordinateSystem == ScreenCoordinateSystem)
        aLocation += mpWindow->GetMapMode().GetOrigin();

    return  Rectangle(
        aBoundingBox.TopLeft() + aLocation,
        aBoundingBox.BottomRight() + aLocation);
}




Size PageObjectLayouter::GetPageObjectSize (void) const
{
    return maPageObjectSize;
}




Size PageObjectLayouter::GetPreviewSize (void) const
{
    return maPreviewBoundingBox.GetSize();
}




Size PageObjectLayouter::GetPageNumberAreaSize (const int nPageCount)
{
    OSL_ASSERT(mpWindow);

    // Set the correct font.
    Font aOriginalFont (mpWindow->GetFont());
    mpWindow->SetFont(*FontProvider::Instance().GetFont(*mpWindow));

    String sPageNumberTemplate;
    if (nPageCount < 10)
        sPageNumberTemplate = String::CreateFromAscii("9");
    else if (nPageCount < 100)
        sPageNumberTemplate = String::CreateFromAscii("99");
    else if (nPageCount < 200)
        // Just for the case that 1 is narrower than 9.
        sPageNumberTemplate = String::CreateFromAscii("199");
    else if (nPageCount < 1000)
        sPageNumberTemplate = String::CreateFromAscii("999");
    else
        sPageNumberTemplate = String::CreateFromAscii("9999");
    // More then 9999 pages are not handled.

    const Size aSize (
        mpWindow->GetTextWidth(sPageNumberTemplate),
        mpWindow->GetTextHeight());

    mpWindow->SetFont(aOriginalFont);

    return aSize;
}




Image PageObjectLayouter::GetTransitionEffectIcon (void) const
{
    return maTransitionEffectIcon;
}




sal_Int32 PageObjectLayouter::GetButtonIndexAt (
    const model::SharedPageDescriptor& rpPageDescriptor,
    const Point& rWindowLocation)
{
    if ( ! GetBoundingBox(rpPageDescriptor, ButtonArea, WindowCoordinateSystem)
        .IsInside(rWindowLocation))
    {
        return -1;
    }
    for (sal_Int32 nIndex=0; nIndex<3; ++nIndex)
    {
        if (GetBoundingBox(rpPageDescriptor, Button, WindowCoordinateSystem, nIndex)
            .IsInside(rWindowLocation))
        {
            return nIndex;
        }
    }
    return -1;
}



} } } // end of namespace ::sd::slidesorter::view
