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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSINSERTIONINDICATOROVERLAY_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSINSERTIONINDICATOROVERLAY_HXX

#include <view/SlsILayerPainter.hxx>
#include <controller/SlsTransferableData.hxx>

#include <tools/gen.hxx>
#include <vcl/bitmapex.hxx>
#include <memory>
#include <vector>

class OutputDevice;
class SdTransferable;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace view {

class FramePainter;

/** The insertion indicator is painted as a vertical or horizontal bar
    in the space between slides.
*/
class InsertionIndicatorOverlay
    : public ILayerPainter,
      public std::enable_shared_from_this<InsertionIndicatorOverlay>
{
public:
    InsertionIndicatorOverlay (SlideSorter& rSlideSorter);
    virtual ~InsertionIndicatorOverlay() COVERITY_NOEXCEPT_FALSE override;

    virtual void SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator) override;

    void Create (const SdTransferable* pTransferable);

    /** Given a position in model coordinates this method calculates the
        insertion marker both as an index in the document and as a location
        used for drawing the insertion indicator.
    */
    void SetLocation (const Point& rPosition);

    Size GetSize() const;

    virtual void Paint (
        OutputDevice& rDevice,
        const ::tools::Rectangle& rRepaintArea) override;

    bool IsVisible() const { return mbIsVisible;}
    void Hide();
    void Show();

    ::tools::Rectangle GetBoundingBox() const;

private:
    SlideSorter& mrSlideSorter;
    bool mbIsVisible;
    SharedILayerInvalidator mpLayerInvalidator;
    // Center of the insertion indicator.
    Point maLocation;
    BitmapEx maIcon;
    std::unique_ptr<FramePainter> mpShadowPainter;

    Point PaintRepresentatives (
        OutputDevice& rContent,
        const Size& rPreviewSize,
        const sal_Int32 nOffset,
        const ::std::vector<controller::TransferableData::Representative>& rPages) const;
    void PaintPageCount (
        OutputDevice& rDevice,
        const sal_Int32 nSelectionCount,
        const Size& rPreviewSize,
        const Point& rFirstPageOffset) const;
    /** Setup the insertion indicator by creating the icon.  It consists of
        scaled down previews of some of the selected pages.
    */
    void Create (
        const ::std::vector<controller::TransferableData::Representative>& rPages,
        const sal_Int32 nSelectionCount);
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
