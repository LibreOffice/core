/*************************************************************************
 *
 *  $RCSfile: SlideSorterView.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-14 16:36:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_SLIDE_SORTER_VIEW_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_VIEW_HXX

#include "View.hxx"

#include <so3/iface.hxx>

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <memory>

class Point;

namespace sdr { namespace contact {
class ObjectContact;
} }

namespace sd { namespace slidesorter {
class SlideSorterViewShell;
} }

namespace sd { namespace slidesorter { namespace controller {
class SlideSorterController;
} } }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace view {

class Layouter;
class ViewOverlay;


class SlideSorterView
    : public View
{
public:
    /** Create a new view for the slide sorter.
        @param rViewShell
            This reference is simply passed to the base class and not used
            by this class.

    */
    SlideSorterView (
        SlideSorterViewShell& rViewShell,
        model::SlideSorterModel& rModel);

    virtual ~SlideSorterView (void);

    void RequestRepaint (void);
    void RequestRepaint (model::PageDescriptor& rDescriptor);

    Rectangle GetModelArea (void);

    enum CoordinateSystem { CS_SCREEN, CS_MODEL };
    enum BoundingBoxType { BBT_SHAPE, BBT_INFO };

    /** Return the rectangle that bounds the page object represented by the
        given page descriptor.
        @param rDescriptor
            The descriptor of the page for which to return the bounding box.
        @param eCoordinateSystem
            Specifies whether to return the screen or model coordinates.
        @param eBoundingBoxType
            Specifies whether to return the bounding box of only the page
            object or the one that additionally includes other displayed
            information like page name and fader symbol.
    */
    Rectangle GetPageBoundingBox (
        model::PageDescriptor& rDescriptor,
        CoordinateSystem eCoordinateSystem,
        BoundingBoxType eBoundingBoxType) const;

    /** Return the rectangle that bounds the page object represented by the
        given page index .
        @param nIndex
            The index of the page for which to return the bounding box.
        @param eCoordinateSystem
            Specifies whether to return the screen or model coordinates.
        @param eBoundingBoxType
            Specifies whether to return the bounding box of only the page
            object or the one that additionally includes other displayed
            information like page name and fader symbol.
    */
    Rectangle GetPageBoundingBox (
        sal_Int32 nIndex,
        CoordinateSystem eCoordinateSystem,
        BoundingBoxType eBoundingBoxType) const;

    /** Return the index of the page that is rendered at the given position.
        @param rPosition
            The position is expected to be in pixel coordinates.
        @return
            The returned index is -1 when there is no page object at the
            given position.
    */
    sal_Int32 GetPageIndexAtPoint (const Point& rPosition) const;
    sal_Int32 GetFadePageIndexAtPoint (const Point& rPosition) const;

    view::Layouter& GetLayouter (void);

    virtual void ModelHasChanged (void);

    /** This method is typically called before a model change takes place.
        All references to model data are released.  PostModelChange() has to
        be called to complete the handling of the model change.  When the
        calls to Pre- and PostModelChange() are very close to each other you
        may call HandleModelChange() instead.
    */
    void PreModelChange (void);

    /** This method is typically called after a model change took place.
        References to model data are re-allocated.  Call this method only
        after PreModelChange() has been called.
    */
    void PostModelChange (void);

    /** This method is a convenience function that simply calls
        PreModelChange() and then PostModelChange().
    */
    void HandleModelChange (void);

    virtual void Resize (void);
    virtual void CompleteRedraw (
        OutputDevice* pDevice,
        const Region& rPaintArea,
        ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);
    virtual void InvalidateOneWin (
        ::Window& rWindow);
    virtual void InvalidateOneWin (
        ::Window& rWindow,
        const Rectangle& rPaintArea );

    void Layout (void);
    /** This tells the view that it has to re-determine the visibility of
        the page objects before painting them the next time.
    */
    void InvalidatePageObjectVisibilities (void);

    /** Return the window to which this view renders its output.
    */
    ::sd::Window* GetWindow (void) const;


    cache::PageCache& GetPreviewCache (void);

    view::ViewOverlay& GetOverlay (void);

    /** Set the bounding box of the insertion marker in model coordinates.

        It will be painted as a dark rectangle that fills the given box.
    */
    void SetInsertionMarker (const Rectangle& rBBox);

    /** Specify whether the insertion marker will be painted or not.
    */
    void SetInsertionMarkerVisibility (bool bVisible);

    /** Set the size and position of the selection rectangle.

        It will be painted as a dashed rectangle.
    */
    void SetSelectionRectangle (const Rectangle& rBox);

    /** Specify whether the selection rectangle will be painted or not.
    */
    void SetSelectionRectangleVisibility (bool bVisible);

    ::sdr::contact::ObjectContact& GetObjectContact (void) const;

private:
    model::SlideSorterModel& mrModel;
    /// This model is used for the maPage object.
    SdrModel maPageModel;
    /** This page acts as container for the page objects that represent the
        pages of the document that is represented by the SlideSorterModel.
    */
    SdrPage* mpPage;
    std::auto_ptr<Layouter> mpLayouter;
    bool mbPageObjectVisibilitiesValid;
    std::auto_ptr<cache::PageCache> mpPreviewCache;
    std::auto_ptr<ViewOverlay> mpViewOverlay;

    int mnFirstVisiblePageIndex;
    int mnLastVisiblePageIndex;

    SvBorder maPagePixelBorder;

    /** Adapt the coordinates of the given bounding box according to the
        other parameters.
        @param rModelPageObjectBoundingBox
            Bounding box given in model coordinates that bounds only the
            page object.
        @param eCoordinateSystem
            When CS_SCREEN is given then the bounding box is converted into
            screen coordinates.
        @param eBoundingBoxType
            When BBT_INFO is given then the bounding box is made larger so
            that it encloses all relevant displayed information.
        */
    void AdaptBoundingBox (
        Rectangle& rModelPageObjectBoundingBox,
        CoordinateSystem eCoordinateSystem,
        BoundingBoxType eBoundingBoxType) const;

    /** Determine the visibility of all page objects.
    */
    void DeterminePageObjectVisibilities (void);

    controller::SlideSorterController& GetController (void);

    /** Update the page borders used by the layouter by using those returned
        by the first page.  Call this function when the model changes,
        especially when the number of pages changes, or when the window is
        resized as the borders may be device dependent.
    */
    void UpdatePageBorders (void);
};

} } } // end of namespace ::sd::slidesorter::view

#endif
