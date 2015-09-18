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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSPAGEOBJECTPAINTER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSPAGEOBJECTPAINTER_HXX

#include "SlideSorter.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include <memory>

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }

namespace sd { namespace slidesorter { namespace view {

class Layouter;
class PageObjectLayouter;
class FramePainter;

class PageObjectPainter
{
public:
    PageObjectPainter (const SlideSorter& rSlideSorter);
    ~PageObjectPainter();

    void PaintPageObject (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor);

    /** Called when the theme changes, either because it is replaced with
        another or because the system colors have changed.  So, even when
        the given theme is the same object as the one already in use by this
        painter everything that depends on the theme is updated.
    */
    void SetTheme (const std::shared_ptr<view::Theme>& rpTheme);

    /** Return a preview bitmap for the given page descriptor.  When the
        page is excluded from the show then the preview is marked
        accordingly.
        @rpDescriptor
            Defines the page for which to return the preview.
        @pReferenceDevice
            When not <NULL/> then this reference device is used to created a
            compatible bitmap.
        @return
            The returned bitmap may have a different size then the preview area.
    */
    Bitmap GetPreviewBitmap (
        const model::SharedPageDescriptor& rpDescriptor,
        const OutputDevice* pReferenceDevice) const;

private:
    const Layouter& mrLayouter;
    std::shared_ptr<cache::PageCache> mpCache;
    std::shared_ptr<controller::Properties> mpProperties;
    std::shared_ptr<view::Theme> mpTheme;
    std::shared_ptr<vcl::Font> mpPageNumberFont;
    std::unique_ptr<FramePainter> mpShadowPainter;
    std::unique_ptr<FramePainter> mpFocusBorderPainter;
    Bitmap maNormalBackground;
    Bitmap maSelectionBackground;
    Bitmap maFocusedSelectionBackground;
    Bitmap maFocusedBackground;
    Bitmap maMouseOverBackground;
    Bitmap maMouseOverFocusedBackground;
    Bitmap maMouseOverSelectedBackground;
    Bitmap maMouseOverSelectedAndFocusedBackground;

    void PaintBackground (
        PageObjectLayouter *pPageObjectLayouter,
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintPreview (
        PageObjectLayouter *pPageObjectLayouter,
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintPageNumber (
        PageObjectLayouter *pPageObjectLayouter,
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    static void PaintTransitionEffect (
        PageObjectLayouter *pPageObjectLayouter,
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor);
    static void PaintCustomAnimationEffect (
        PageObjectLayouter *pPageObjectLayouter,
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor);
    void PaintBorder (
        OutputDevice& rDevice,
        const Theme::GradientColorType eColorType,
        const Rectangle& rBox) const;
    void PaintBackgroundDetail(
        PageObjectLayouter *pPageObjectLayouter,
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;

    static Bitmap CreateMarkedPreview(
        const Size& rSize,
        const Bitmap& rPreview,
        const BitmapEx& rOverlay,
        const OutputDevice* pReferenceDevice);

    /** Update the local pointer to the page object layouter to the
        one owned by the general layouter.
        Return <TRUE/> when after the call we have a valid page object layouter.
    */
    bool UpdatePageObjectLayouter();
};

} } } // end of namespace sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
