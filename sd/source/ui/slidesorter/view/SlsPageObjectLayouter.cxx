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

#include "view/SlsPageObjectLayouter.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "view/SlsFontProvider.hxx"
#include "view/SlsTheme.hxx"
#include "tools/IconCache.hxx"
#include "Window.hxx"

#include "bitmaps.hlst"

namespace sd { namespace slidesorter { namespace view {

namespace {
const static sal_Int32 gnLeftPageNumberOffset = 2;
const static sal_Int32 gnRightPageNumberOffset = 5;
const static sal_Int32 gnOuterBorderWidth = 5;
const static sal_Int32 gnInfoAreaMinWidth = 26;
}

PageObjectLayouter::PageObjectLayouter (
    const Size& rPageObjectWindowSize,
    const Size& rPageSize,
    sd::Window *pWindow,
    const sal_Int32 nPageCount)
    : mpWindow(pWindow),
      maPageObjectSize(rPageObjectWindowSize.Width(), rPageObjectWindowSize.Height()),
      maPageObjectBoundingBox(),
      maPageNumberAreaBoundingBox(),
      maPreviewBoundingBox(),
      maTransitionEffectBoundingBox(),
      maTransitionEffectIcon(IconCache::Instance().GetIcon(BMP_FADE_EFFECT_INDICATOR)),
      maCustomAnimationEffectIcon(IconCache::Instance().GetIcon(BMP_CUSTOM_ANIMATION_INDICATOR)),
      mpPageNumberFont(Theme::GetFont(Theme::Font_PageNumber, *pWindow))
{
    const Size aPageNumberAreaSize (GetPageNumberAreaSize(nPageCount));

    const int nMaximumBorderWidth (gnOuterBorderWidth);
    const int nFocusIndicatorWidth (Theme_FocusIndicatorWidth);

    maPreviewBoundingBox = CalculatePreviewBoundingBox(
        maPageObjectSize,
        Size(rPageSize.Width(), rPageSize.Height()),
        aPageNumberAreaSize.Width(),
        nFocusIndicatorWidth);
    maFocusIndicatorBoundingBox = ::tools::Rectangle(Point(0,0), maPageObjectSize);
    maPageObjectBoundingBox = ::tools::Rectangle(
        Point(
            nFocusIndicatorWidth,
            nFocusIndicatorWidth),
        Size(
            maPageObjectSize.Width()-2*nFocusIndicatorWidth,
            maPageObjectSize.Height()-2*nFocusIndicatorWidth));

    maPageNumberAreaBoundingBox = ::tools::Rectangle(
        Point(
            std::max(gnLeftPageNumberOffset,
                sal_Int32(maPreviewBoundingBox.Left()
                - gnRightPageNumberOffset
                - aPageNumberAreaSize.Width())),
            nMaximumBorderWidth),
        aPageNumberAreaSize);

    const Size aIconSize (maTransitionEffectIcon.GetSizePixel());
    maTransitionEffectBoundingBox = ::tools::Rectangle(
        Point(
            (maPreviewBoundingBox.Left() - 2*aIconSize.Width()) / 2,
            maPreviewBoundingBox.Bottom() - aIconSize.Height()),
        aIconSize);
    maCustomAnimationEffectBoundingBox = ::tools::Rectangle(
        Point(
            (maPreviewBoundingBox.Left() - 2*aIconSize.Width()) / 2,
            maPreviewBoundingBox.Bottom() - 2*aIconSize.Height()),
        aIconSize);
}

PageObjectLayouter::~PageObjectLayouter()
{
}

::tools::Rectangle PageObjectLayouter::CalculatePreviewBoundingBox (
    Size& rPageObjectSize,
    const Size& rPageSize,
    const sal_Int32 nPageNumberAreaWidth,
    const sal_Int32 nFocusIndicatorWidth)
{
    const sal_Int32 nIconWidth (maTransitionEffectIcon.GetSizePixel().Width());
    const sal_Int32 nLeftAreaWidth (
        ::std::max(
            gnInfoAreaMinWidth,
            gnRightPageNumberOffset
                + ::std::max(
                    nPageNumberAreaWidth,
                    nIconWidth)));
    sal_Int32 nPreviewWidth;
    sal_Int32 nPreviewHeight;
    const double nPageAspectRatio (double(rPageSize.Width()) / double(rPageSize.Height()));
    if (rPageObjectSize.Height() == 0)
    {
        // Calculate height so that the preview fills the available
        // horizontal space completely while observing the aspect ratio of
        // the preview.
        nPreviewWidth = rPageObjectSize.Width()
            - nLeftAreaWidth - gnOuterBorderWidth - 2*nFocusIndicatorWidth - 1;
        nPreviewHeight = ::basegfx::fround(nPreviewWidth / nPageAspectRatio);
        rPageObjectSize.setHeight(nPreviewHeight + 2*gnOuterBorderWidth + 2*nFocusIndicatorWidth + 1);
    }
    else if (rPageObjectSize.Width() == 0)
    {
        // Calculate the width of the page object so that the preview fills
        // the available vertical space completely while observing the
        // aspect ratio of the preview.
        nPreviewHeight = rPageObjectSize.Height() - 2*gnOuterBorderWidth - 2*nFocusIndicatorWidth - 1;
        nPreviewWidth = ::basegfx::fround(nPreviewHeight * nPageAspectRatio);
        rPageObjectSize.setWidth(nPreviewWidth
            + nLeftAreaWidth + gnOuterBorderWidth + 2*nFocusIndicatorWidth + 1);

    }
    else
    {
        // The size of the page object is given.  Calculate the size of the
        // preview.
        nPreviewWidth = rPageObjectSize.Width()
            - nLeftAreaWidth - gnOuterBorderWidth - 2*nFocusIndicatorWidth - 1;
        nPreviewHeight = rPageObjectSize.Height()
            - gnOuterBorderWidth - 2*nFocusIndicatorWidth - 1;
        if (double(nPreviewWidth)/double(nPreviewHeight) > nPageAspectRatio)
            nPreviewWidth = ::basegfx::fround(nPreviewHeight * nPageAspectRatio);
        else
            nPreviewHeight = ::basegfx::fround(nPreviewWidth / nPageAspectRatio);
    }
    // When the preview does not fill the available space completely then
    // place it flush right and vertically centered.
    const int nLeft (rPageObjectSize.Width()
        - gnOuterBorderWidth - nPreviewWidth - nFocusIndicatorWidth - 1);
    const int nTop ((rPageObjectSize.Height() - nPreviewHeight)/2);
    return ::tools::Rectangle(
        nLeft,
        nTop,
        nLeft + nPreviewWidth,
        nTop + nPreviewHeight);
}

::tools::Rectangle PageObjectLayouter::GetBoundingBox (
    const model::SharedPageDescriptor& rpPageDescriptor,
    const Part ePart,
    const CoordinateSystem eCoordinateSystem,
    bool bIgnoreLocation)
{
    OSL_ASSERT(rpPageDescriptor);
    Point aLocation(0,0);
    if (rpPageDescriptor)
        aLocation = rpPageDescriptor->GetLocation( bIgnoreLocation );
    return GetBoundingBox(aLocation, ePart, eCoordinateSystem);
}

::tools::Rectangle PageObjectLayouter::GetBoundingBox (
    const Point& rPageObjectLocation,
    const Part ePart,
    const CoordinateSystem eCoordinateSystem)
{
    ::tools::Rectangle aBoundingBox;
    switch (ePart)
    {
        case Part::FocusIndicator:
            aBoundingBox = maFocusIndicatorBoundingBox;
            break;

        case Part::PageObject:
            aBoundingBox = maPageObjectBoundingBox;
            break;

        case Part::Preview:
            aBoundingBox = maPreviewBoundingBox;
            break;

        case Part::PageNumber:
            aBoundingBox = maPageNumberAreaBoundingBox;
            break;

        case Part::TransitionEffectIndicator:
            aBoundingBox = maTransitionEffectBoundingBox;
            break;
        case Part::CustomAnimationEffectIndicator:
            aBoundingBox = maCustomAnimationEffectBoundingBox;
            break;
    }

    // Adapt coordinates to the requested coordinate system.
    Point aLocation (rPageObjectLocation);
    if (eCoordinateSystem == WindowCoordinateSystem)
        aLocation += mpWindow->GetMapMode().GetOrigin();

    return ::tools::Rectangle(
        aBoundingBox.TopLeft() + aLocation,
        aBoundingBox.BottomRight() + aLocation);
}

Size PageObjectLayouter::GetPreviewSize ()
{
    return GetBoundingBox(Point(0,0), PageObjectLayouter::Part::Preview,
                          WindowCoordinateSystem).GetSize();
}

Size PageObjectLayouter::GetGridMaxSize()
{
    return GetBoundingBox(Point(0,0), PageObjectLayouter::Part::FocusIndicator,
                          WindowCoordinateSystem).GetSize();
}

Size PageObjectLayouter::GetPageNumberAreaSize (const int nPageCount)
{
    OSL_ASSERT(mpWindow);

    // Set the correct font.
    vcl::Font aOriginalFont (mpWindow->GetFont());
    if (mpPageNumberFont)
        mpWindow->SetFont(*mpPageNumberFont);

    OUString sPageNumberTemplate;
    if (nPageCount < 10)
        sPageNumberTemplate = "9";
    else if (nPageCount < 100)
        sPageNumberTemplate = "99";
    else if (nPageCount < 200)
        // Just for the case that 1 is narrower than 9.
        sPageNumberTemplate = "199";
    else if (nPageCount < 1000)
        sPageNumberTemplate = "999";
    else
        sPageNumberTemplate = "9999";
    // More then 9999 pages are not handled.

    const Size aSize (
        mpWindow->GetTextWidth(sPageNumberTemplate),
        mpWindow->GetTextHeight());

    mpWindow->SetFont(aOriginalFont);

    return aSize;
}

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
