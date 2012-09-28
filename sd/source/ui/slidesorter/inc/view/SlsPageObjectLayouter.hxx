/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_LAYOUTER_HXX
#define SD_SLIDESORTER_PAGE_OBJECT_LAYOUTER_HXX

#include "SlideSorter.hxx"
#include "model/SlsSharedPageDescriptor.hxx"
#include "tools/gen.hxx"
#include <vcl/image.hxx>

namespace sd { namespace slidesorter { namespace view {


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
        const SharedSdWindow& rpWindow,
        const sal_Int32 nPageCount);
    ~PageObjectLayouter(void);

    enum Part {
        // The focus indicator is painted outside the actual page object.
        FocusIndicator,
        // This is the outer bounding box that includes the preview, page
        // number, title.
        PageObject,
        // Bounding box of the actual preview.
        Preview,
        // Bounding box of the mouse indicator indicator frame.
        MouseOverIndicator,
        // Bounding box of the page number.
        PageNumber,
        // Bounding box of the pane name.
        Name,
        // Indicator whether or not there is a slide transition associated
        // with this slide.
        TransitionEffectIndicator
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
        @param eCoodinateSystem
            The bounding box can be returned in model and in pixel
            (window) coordinates.
    */
    Rectangle GetBoundingBox (
        const model::SharedPageDescriptor& rpPageDescriptor,
        const Part ePart,
        const CoordinateSystem eCoordinateSystem);
    Rectangle GetBoundingBox (
        const Point& rPageObjectLocation,
        const Part ePart,
        const CoordinateSystem eCoordinateSystem);
    Size GetSize (
        const Part ePart,
        const CoordinateSystem eCoordinateSystem);

    Image GetTransitionEffectIcon (void) const;

private:
    SharedSdWindow mpWindow;
    Size maPageObjectSize;
    Rectangle maFocusIndicatorBoundingBox;
    Rectangle maPageObjectBoundingBox;
    Rectangle maPageNumberAreaBoundingBox;
    Rectangle maPreviewBoundingBox;
    Rectangle maTransitionEffectBoundingBox;
    const Image maTransitionEffectIcon;
    const ::boost::shared_ptr<Font> mpPageNumberFont;

    Size GetPageNumberAreaSize (const int nPageCount);
    Rectangle CalculatePreviewBoundingBox (
        Size& rPageObjectSize,
        const Size& rPreviewModelSize,
        const sal_Int32 nPageNumberAreaWidth,
        const sal_Int32 nFocusIndicatorWidth);
};


} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
