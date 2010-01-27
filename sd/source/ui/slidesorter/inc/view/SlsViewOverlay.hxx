/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewOverlay.hxx,v $
 * $Revision: 1.10 $
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
#include "view/SlsILayerPainter.hxx"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <osl/mutex.hxx>
#include <tools/gen.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


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

namespace sd { namespace slidesorter { namespace view {


class LayeredDevice;
class InsertionIndicatorOverlay;
class SelectionRectangleOverlay;
class SubstitutionOverlay;
class ViewOverlay;

/** This base class of slide sorter overlays uses the drawing layer overlay
    support for the display.
*/
class OverlayBase :
    public ILayerPainter,
    public ::boost::enable_shared_from_this<OverlayBase>
{
public:
    OverlayBase (ViewOverlay& rViewOverlay, const sal_Int32 nLayer);
    virtual ~OverlayBase (void);

    bool IsVisible (void) const;
    void SetIsVisible (const bool bIsVisible);

    virtual void SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator);

    sal_Int32 GetLayerIndex (void) const;

protected:
    ::osl::Mutex maMutex;

    ViewOverlay& mrViewOverlay;
    SharedILayerInvalidator mpLayerInvalidator;

    class Invalidator;
    friend class Invalidator;

    virtual Rectangle GetBoundingBox (void) const = 0;

private:
    bool mbIsVisible;
    const sal_Int32 mnLayerIndex;

    void Invalidate (const Rectangle& rInvalidationBox);
};




/** During internal drag and drop the outlines of the selected slides are
    painted at the mouse position in dashed lines.
*/
class SubstitutionOverlay
    : public OverlayBase
{
public:
    SubstitutionOverlay (ViewOverlay& rViewOverlay, const sal_Int32 nLayer);
    virtual ~SubstitutionOverlay (void);

    /** Setup the substitution display of the given set of selected pages.
        The given mouse position is remembered so that it later can be
        returned by GetPosition(). This is a convenience feature.
    */
    void Create (
        model::PageEnumeration& rSelection,
        const Point& rPosition,
        const model::SharedPageDescriptor& rpHitDescriptor);

    /** Clear the substitution display.  Until the next call of Create() no
        substution is painted.
    */
    void Clear (void);

    /** Move the substitution display by the given amount of pixels.
    */
    void Move (const Point& rOffset);
    void SetPosition (const Point& rPosition);
    Point GetPosition (void) const;

    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea);

protected:
    virtual Rectangle GetBoundingBox (void) const;

private:
    Point maPosition;
    Point maTranslation;
    /** The substitution paints only the page object under the mouse and the
        8-neighborhood around it.  It uses different levels of transparency
        for the center and the four elements at its sides and the four
        elements at its corners.  All values between 0 (opaque) and 100
        (fully transparent.)
    */
    static const sal_Int32 mnCenterTransparency;
    static const sal_Int32 mnSideTransparency;
    static const sal_Int32 mnCornerTransparency;

    class ItemDescriptor
    {
    public:
        BitmapEx maImage;
        Point maLocation;
        double mnTransparency;
        basegfx::B2DPolygon maShape;
    };
    ::std::vector<ItemDescriptor> maItems;
    Rectangle maBoundingBox;
};




/** Slides can be selected by drawing a selection rectangle in the slide
    sorter.  When the left mouse button is released all slides that are at
    least partially in the rectangle are selected.
*/
class SelectionRectangleOverlay
    : public OverlayBase
{
public:
    SelectionRectangleOverlay (ViewOverlay& rViewOverlay, const sal_Int32 nLayer);

    void Start (const Point& rAnchor);
    void Update (const Point& rSecondCorner);

    Rectangle GetSelectionRectangle (void);

    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea);

protected:
    virtual Rectangle GetBoundingBox (void) const;

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
    InsertionIndicatorOverlay (ViewOverlay& rViewOverlay, const sal_Int32 nLayer);

    /** Given a position in model coordinates this method calculates the
        insertion marker both as an index in the document and as a location
        used for drawing the insertion indicator.
    */
    void SetLocation (const Point& rPosition);

    virtual void Paint (
        OutputDevice& rDevice,
        const Rectangle& rRepaintArea);

protected:
    virtual Rectangle GetBoundingBox (void) const;

private:
    // Center of the insertion indicator.
    Point maLocation;
    /** Remember whether the insertion indicator is displayed before (left
        of or above) or after (right of or below) the page at the insertion
        index.
    */
    bool mbIsBeforePage;
    ImageList maIcons;
    Image maIconWithBorder;
    Image maIcon;
    Image maMask;
    void SetPositionAndSize (const Rectangle& rBoundingBox);
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
    ViewOverlay (
        SlideSorter& rSlideSorter,
        const ::boost::shared_ptr<LayeredDevice>& rpLayeredDevice);
    virtual ~ViewOverlay (void);

    ::boost::shared_ptr<SelectionRectangleOverlay> GetSelectionRectangleOverlay (void);
    ::boost::shared_ptr<InsertionIndicatorOverlay> GetInsertionIndicatorOverlay (void);
    ::boost::shared_ptr<SubstitutionOverlay> GetSubstitutionOverlay (void);

    SlideSorter& GetSlideSorter (void) const;
    ::boost::shared_ptr<LayeredDevice> GetLayeredDevice (void) const;

private:
    SlideSorter& mrSlideSorter;
    const ::boost::shared_ptr<LayeredDevice> mpLayeredDevice;
    ::boost::shared_ptr<SelectionRectangleOverlay> mpSelectionRectangleOverlay;
    ::boost::shared_ptr<InsertionIndicatorOverlay> mpInsertionIndicatorOverlay;
    ::boost::shared_ptr<SubstitutionOverlay> mpSubstitutionOverlay;
};



} } } // end of namespace ::sd::slidesorter::view

#endif
