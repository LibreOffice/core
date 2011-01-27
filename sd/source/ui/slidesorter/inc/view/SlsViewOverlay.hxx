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

#ifndef SD_SLIDESORTER_VIEW_OVERLAY_HXX
#define SD_SLIDESORTER_VIEW_OVERLAY_HXX

#include "model/SlsSharedPageDescriptor.hxx"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <osl/mutex.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <tools/gen.hxx>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>

class OutputDevice;
class Region;


namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {
class PageEnumeration;
} } }

namespace sd { namespace slidesorter { namespace controller {
class SlideSorterController;
} } }

namespace sdr { namespace overlay {
class OverlayManager;
} }

namespace sd { namespace slidesorter { namespace view {


class InsertionIndicatorOverlay;
class PageObjectViewObjectContact;
class SelectionRectangleOverlay;
class SubstitutionOverlay;
class ViewOverlay;

/** This base class of slide sorter overlays uses the drawing layer overlay
    support for the display.
*/
class OverlayBase
    : public sdr::overlay::OverlayObject
{
public:
    OverlayBase (ViewOverlay& rViewOverlay);
    virtual ~OverlayBase (void);

protected:
    ::osl::Mutex maMutex;

    ViewOverlay& mrViewOverlay;

    /** Make sure that the overlay object is registered at the
        OverlayManager.  This registration is done on demand.
    */
    void EnsureRegistration (void);
    void RemoveRegistration();
};




/** During internal drag and drop the outlines of the selected slides are
    painted at the mouse position in dashed lines.
*/
class SubstitutionOverlay
    : public OverlayBase
{
public:
    SubstitutionOverlay (ViewOverlay& rViewOverlay);
    virtual ~SubstitutionOverlay (void);

    /** Setup the substitution display of the given set of selected pages.
        The given mouse position is remembered so that it later can be
        returned by GetPosition(). This is a convenience feature.
    */
    void Create (
        model::PageEnumeration& rSelection,
        const Point& rPosition);

    /** Clear the substitution display.  Until the next call of Create() no
        substution is painted.
    */
    void Clear (void);

    /** Move the substitution display by the given amount of pixels.
    */
    void Move (const Point& rOffset);
    void SetPosition (const Point& rPosition);
    Point GetPosition (void) const;

    // react on stripe definition change
    virtual void stripeDefinitionHasChanged();

protected:
    // geometry creation for OverlayObject
    virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

private:
    Point maPosition;
    basegfx::B2DPolyPolygon maShapes;
};




/** Slides can be selected by drawing a selection rectangle in the slide
    sorter.  When the left mouse button is released all slides that are at
    least partially in the rectangle are selected.
*/
class SelectionRectangleOverlay
    : public OverlayBase
{
public:
    SelectionRectangleOverlay (ViewOverlay& rViewOverlay);
    virtual ~SelectionRectangleOverlay();

    void Start (const Point& rAnchor);
    void Update (const Point& rSecondCorner);

    Rectangle GetSelectionRectangle (void);

    // react on stripe definition change
    virtual void stripeDefinitionHasChanged();

protected:
    // geometry creation for OverlayObject
    virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

private:
    Point maAnchor;
    Point maSecondCorner;
};




/** The insertion indicator is painted as a vertical or horizonal bar
    in the space between slides.
*/
class InsertionIndicatorOverlay
    : public OverlayBase
{
public:
    InsertionIndicatorOverlay (ViewOverlay& rViewOverlay);
    virtual ~InsertionIndicatorOverlay();

    /** Given a position in model coordinates this method calculates the
        insertion marker both as an index in the document and as a rectangle
        used for drawing the insertion indicator.
    */
    void SetPosition (const Point& rPosition);

    sal_Int32 GetInsertionPageIndex (void) const;

protected:
    // geometry creation for OverlayObject
    virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

private:
    sal_Int32 mnInsertionIndex;
    Rectangle maBoundingBox;

    void SetPositionAndSize (const Rectangle& rBoundingBox);
};




/** Paint a frame around the slide preview under the mouse.  The actual
    painting is done by the PageObjectViewObjectContact of the slidesorter.
*/
class MouseOverIndicatorOverlay
    : public OverlayBase
{
public:
    MouseOverIndicatorOverlay (ViewOverlay& rViewOverlay);
    virtual ~MouseOverIndicatorOverlay (void);

    /** Set the page object for which to paint a mouse over indicator.
        @param pContact
            A value of <NULL/> indicates to not paint the mouse over indicator.
    */
    void SetSlideUnderMouse (const model::SharedPageDescriptor& rpDescriptor);

protected:
    // geometry creation for OverlayObject
    virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

private:
    /** The page under the mouse is stored as weak shared pointer so that
        model changes can be handled without having the SlideSorterModel
        inform this class explicitly.
    */
    ::boost::weak_ptr<model::PageDescriptor> mpPageUnderMouse;

    view::PageObjectViewObjectContact* GetViewObjectContact (void) const;
};




/** The view overlay manages and paints some indicators that are painted on
    top of the regular view content (the page objects).  It is separated
    from the view to allow the indicators to be altered in position and size
    without repainting the whole view content (inside that the bounding box
    of the indicator).  This is achieved by using the drawing layer overlay
    support.

    The view overlay itself simply gives access to the more specialized
    classes that handle individual indicators.

*/
class ViewOverlay
{
public:
    ViewOverlay (SlideSorter& rSlideSorter);
    ~ViewOverlay (void);

    SelectionRectangleOverlay& GetSelectionRectangleOverlay (void);
    MouseOverIndicatorOverlay& GetMouseOverIndicatorOverlay (void);
    InsertionIndicatorOverlay& GetInsertionIndicatorOverlay (void);
    SubstitutionOverlay& GetSubstitutionOverlay (void);

    SlideSorter& GetSlideSorter (void) const;

    sdr::overlay::OverlayManager* GetOverlayManager (void) const;

private:
    SlideSorter& mrSlideSorter;
    SelectionRectangleOverlay maSelectionRectangleOverlay;
    MouseOverIndicatorOverlay maMouseOverIndicatorOverlay;
    InsertionIndicatorOverlay maInsertionIndicatorOverlay;
    SubstitutionOverlay maSubstitutionOverlay;
};



} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
