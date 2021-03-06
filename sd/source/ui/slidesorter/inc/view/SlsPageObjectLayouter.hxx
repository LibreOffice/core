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

#pragma once

#include <model/SlsSharedPageDescriptor.hxx>
#include <tools/gen.hxx>
#include <vcl/image.hxx>

namespace vcl { class Font; }
namespace sd { class Window; }

namespace sd::slidesorter::view {

/** In contrast to the Layouter that places page objects in the view, the
    PageObjectLayouter places the parts of individual page objects like page
    number area, borders, preview.
*/
class PageObjectLayouter
{
public:
    /** Create a new PageObjectLayouter object.
        @param rPageObjectSize
            In general either the width or the height will be 0 in order to
            signal that this size component has to be calculated from the other.
            This calculation will make the preview as large as possible.
        @param nPageCount
            The page count is used to determine how wide the page number
            area has to be, how many digits to except for the largest page number.
    */
    PageObjectLayouter(
        const Size& rPageObjectWindowSize,
        const Size& rPreviewModelSize,
        sd::Window *pWindow,
        const sal_Int32 nPageCount);
    ~PageObjectLayouter();

    enum class Part {
        // The focus indicator is painted outside the actual page object.
        FocusIndicator,
        // This is the outer bounding box that includes the preview, page
        // number, title.
        PageObject,
        // Bounding box of the actual preview.
        Preview,
        // Bounding box of the page number.
        PageNumber,
        // Indicator whether or not there is a slide transition associated
        // with this slide.
        TransitionEffectIndicator,
        // Indicator whether or not there is a custom animation associated
        // with this slide.
        CustomAnimationEffectIndicator
    };
    /** Two coordinate systems are supported.  They differ only in
        translation not in scale.  Both relate to pixel values in the window.
        A position in the model coordinate system does not change when the window content is
        scrolled up or down.  In the window coordinate system (relative
        to the top left point of the window)scrolling leads to different values.
    */
    enum CoordinateSystem {
        WindowCoordinateSystem,
        ModelCoordinateSystem
    };

    /** Return the bounding box of the page object or one of its graphical
        parts.
        @param rWindow
            This device is used to translate between model and window
            coordinates.
        @param rpPageDescriptor
            The page for which to calculate the bounding box.  This may be
            NULL.  When it is NULL then a generic bounding box is calculated
            for the location (0,0).
        @param ePart
            The part of the page object for which to return the bounding
            box.
        @param eCoordinateSystem
            The bounding box can be returned in model and in pixel
            (window) coordinates.
        @param bIgnoreLocation
            Return a position ignoring the slides' location, ie. as if
            we were the first slide.
    */
    ::tools::Rectangle GetBoundingBox (
        const model::SharedPageDescriptor& rpPageDescriptor,
        const Part ePart,
        const CoordinateSystem eCoordinateSystem,
        bool bIgnoreLocation = false);

    /// the size of the embedded preview: position independent, in window coordinate system
    Size GetPreviewSize();

    /// the maximum size of each tile, also position independent, in window coordinate system
    Size GetGridMaxSize();

    const Image& GetTransitionEffectIcon() const { return maTransitionEffectIcon;}
    const Image& GetCustomAnimationEffectIcon() const { return maCustomAnimationEffectIcon;}

private:
    ::tools::Rectangle GetBoundingBox (
        const Point& rPageObjectLocation,
        const Part ePart,
        const CoordinateSystem eCoordinateSystem);

private:
    VclPtr<sd::Window> mpWindow;
    ::tools::Rectangle maFocusIndicatorBoundingBox;
    ::tools::Rectangle maPageObjectBoundingBox;
    ::tools::Rectangle maPageNumberAreaBoundingBox;
    ::tools::Rectangle maPreviewBoundingBox;
    ::tools::Rectangle maTransitionEffectBoundingBox;
    ::tools::Rectangle maCustomAnimationEffectBoundingBox;
    const Image maTransitionEffectIcon;
    const Image maCustomAnimationEffectIcon;
    const std::shared_ptr<vcl::Font> mpPageNumberFont;

    Size GetPageNumberAreaSize (const int nPageCount);
    ::tools::Rectangle CalculatePreviewBoundingBox (
        Size& rPageObjectSize,
        const Size& rPreviewModelSize,
        const sal_Int32 nPageNumberAreaWidth,
        const sal_Int32 nFocusIndicatorWidth);
};

} // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
