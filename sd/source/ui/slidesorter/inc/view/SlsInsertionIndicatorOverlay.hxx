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

#ifndef SD_SLIDESORTER_INSERTION_INDICATOR_OVERLAY_HXX
#define SD_SLIDESORTER_INSERTION_INDICATOR_OVERLAY_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include "view/SlsILayerPainter.hxx"
#include "controller/SlsTransferable.hxx"

#include <tools/gen.hxx>
#include <vcl/bitmapex.hxx>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>

class OutputDevice;
class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {
class PageEnumeration;
} } }

namespace sd { namespace slidesorter { namespace controller {
class Transferable;
} } }

namespace sd { namespace slidesorter { namespace view {

class FramePainter;
class LayeredDevice;

/** The insertion indicator is painted as a vertical or horizonal bar
    in the space between slides.
*/
class InsertionIndicatorOverlay
    : public ILayerPainter,
      public ::boost::enable_shared_from_this<InsertionIndicatorOverlay>
{
public:
    InsertionIndicatorOverlay (SlideSorter& rSlideSorter);
    virtual ~InsertionIndicatorOverlay (void);

    virtual void SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator);

    void Create (const controller::Transferable* pTransferable);

    /** Given a position in model coordinates this method calculates the
        insertion marker both as an index in the document and as a location
        used for drawing the insertion indicator.
    */
    void SetLocation (const Point& rPosition);

    Size GetSize (void) const;

    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea);

    bool IsVisible (void) const;
    void Hide (void);
    void Show (void);

    Rectangle GetBoundingBox (void) const;

private:
    SlideSorter& mrSlideSorter;
    bool mbIsVisible;
    const sal_Int32 mnLayerIndex;
    SharedILayerInvalidator mpLayerInvalidator;
    // Center of the insertion indicator.
    Point maLocation;
    BitmapEx maIcon;
    Point maIconOffset;
    ::boost::scoped_ptr<FramePainter> mpShadowPainter;

    void SetPositionAndSize (const Rectangle& rBoundingBox);
    void SelectRepresentatives (
        model::PageEnumeration& rSelection,
        ::std::vector<model::SharedPageDescriptor>& rDescriptors) const;
    Point PaintRepresentatives (
        OutputDevice& rContent,
        const Size aPreviewSize,
        const sal_Int32 nOffset,
        const ::std::vector<controller::Transferable::Representative>& rPages) const;
    void PaintPageCount (
        OutputDevice& rDevice,
        const sal_Int32 nSelectionCount,
        const Size aPreviewSize,
        const Point aFirstPageOffset) const;
    /** Setup the insertion indicator by creating the icon.  It consists of
        scaled down previews of some of the selected pages.
    */
    void Create (
        const ::std::vector<controller::Transferable::Representative>& rPages,
        const sal_Int32 nSelectionCount);
};



} } } // end of namespace ::sd::slidesorter::view

#endif
