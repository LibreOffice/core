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

#ifndef SD_SLIDESORTER_PAGE_OBJECT_PAINTER_HEADER
#define SD_SLIDESORTER_PAGE_OBJECT_PAINTER_HEADER

#include "SlideSorter.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include <boost/scoped_ptr.hpp>

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
    ~PageObjectPainter (void);

    void PaintPageObject (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor);

    void NotifyResize (const bool bForce = false);

    /** Called when the theme changes, either because it is replaced with
        another or because the system colors have changed.  So, even when
        the given theme is the same object as the one already in use by this
        painter everything that depends on the theme is updated.
    */
    void SetTheme (const ::boost::shared_ptr<view::Theme>& rpTheme);

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
    ::boost::shared_ptr<PageObjectLayouter> mpPageObjectLayouter;
    ::boost::shared_ptr<cache::PageCache> mpCache;
    ::boost::shared_ptr<controller::Properties> mpProperties;
    ::boost::shared_ptr<view::Theme> mpTheme;
    ::boost::shared_ptr<Font> mpPageNumberFont;
    ::boost::scoped_ptr<FramePainter> mpShadowPainter;
    ::boost::scoped_ptr<FramePainter> mpFocusBorderPainter;
    Bitmap maNormalBackground;
    Bitmap maSelectionBackground;
    Bitmap maFocusedSelectionBackground;
    Bitmap maFocusedBackground;
    Bitmap maMouseOverBackground;
    Bitmap maMouseOverFocusedBackground;
    Bitmap maMouseOverSelectedAndFocusedBackground;
    Size maSize;

    void PaintBackground (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor);
    void PaintPreview (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintPageNumber (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintTransitionEffect (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpDescriptor) const;
    void PaintBorder (
        OutputDevice& rDevice,
        const Theme::GradientColorType eColorType,
        const Rectangle& rBox) const;
    Bitmap& GetBackgroundForState (
        const model::SharedPageDescriptor& rpDescriptor,
        const OutputDevice& rTemplateDevice);
    Bitmap& GetBackground(
        Bitmap& rBackground,
        Theme::GradientColorType eType,
        const OutputDevice& rTemplateDevice,
        const bool bHasFocusBorder);
    Bitmap CreateBackgroundBitmap(
        const OutputDevice& rReferenceDevice,
        const Theme::GradientColorType eType,
        const bool bHasFocusBorder) const;
    Bitmap CreateMarkedPreview(
        const Size& rSize,
        const Bitmap& rPreview,
        const BitmapEx& rOverlay,
        const OutputDevice* pReferenceDevice) const;
    void InvalidateBitmaps (void);
};

} } } // end of namespace sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
