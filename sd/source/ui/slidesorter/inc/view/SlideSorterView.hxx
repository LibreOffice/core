/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterView.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:38:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_SLIDE_SORTER_VIEW_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_VIEW_HXX

#include "View.hxx"

#include "model/SlsSharedPageDescriptor.hxx"

#include <sfx2/viewfrm.hxx>

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <memory>
#include <boost/shared_ptr.hpp>

class Point;

namespace sdr { namespace contact {
class ObjectContact;
} }

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {
class SlideSorterController;
} } }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {

class Layouter;
class ViewOverlay;


class SlideSorterView
    : public View
{
public:
    TYPEINFO();

    /** Create a new view for the slide sorter.
        @param rViewShell
            This reference is simply passed to the base class and not used
            by this class.

    */
    SlideSorterView (SlideSorter& rSlideSorter);

    virtual ~SlideSorterView (void);

    enum Orientation { HORIZONTAL, VERTICAL };
    void SetOrientation (const Orientation eOrientation);
    Orientation GetOrientation (void) const;

    void RequestRepaint (void);
    void RequestRepaint (const model::SharedPageDescriptor& rDescriptor);

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
        const model::SharedPageDescriptor& rpDescriptor,
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

    void LocalModelHasChanged(void);

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

    void HandleDrawModeChange (void);

    virtual void Resize (void);
    virtual void CompleteRedraw (
        OutputDevice* pDevice,
        const Region& rPaintArea,
        USHORT nPaintMode,
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


    ::boost::shared_ptr<cache::PageCache> GetPreviewCache (void);

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

    typedef ::std::pair<sal_Int32,sal_Int32> PageRange;
    /** Return the range of currently visible page objects including the
        first and last one in that range.
        @return
            The returned pair of page object indices is empty when the
            second index is lower than the first.
    */
    PageRange GetVisiblePageRange (void);

    /** Return the size of the area where the page numbers are displayed.
        @return
            The returned size is given in model coordinates.
    */
    Size GetPageNumberAreaModelSize (void) const;

    /** Return the size of the border around the original SdrPageObj.
    */
    SvBorder GetModelBorder (void) const;

    /** Add a shape to the page.  Typically used from inside
        PostModelChange().
    */
    void AddSdrObject (SdrObject& rObject);

protected:
    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint);

private:
    SlideSorter& mrSlideSorter;
    model::SlideSorterModel& mrModel;
    /// This model is used for the maPage object.
    SdrModel maPageModel;
    /** This page acts as container for the page objects that represent the
        pages of the document that is represented by the SlideSorterModel.
    */
    SdrPage* mpPage;
    ::std::auto_ptr<Layouter> mpLayouter;
    bool mbPageObjectVisibilitiesValid;
    ::boost::shared_ptr<cache::PageCache> mpPreviewCache;
    ::std::auto_ptr<ViewOverlay> mpViewOverlay;

    int mnFirstVisiblePageIndex;
    int mnLastVisiblePageIndex;

    SvBorder maPagePixelBorder;

    bool mbModelChangedWhileModifyEnabled;

    Size maPreviewSize;

    bool mbPreciousFlagUpdatePending;

    Size maPageNumberAreaModelSize;
    SvBorder maModelBorder;

    Orientation meOrientation;

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

    /** Update the page borders used by the layouter by using those returned
        by the first page.  Call this function when the model changes,
        especially when the number of pages changes, or when the window is
        resized as the borders may be device dependent.
    */
    void UpdatePageBorders (void);

    void UpdatePreciousFlags (void);
};


} } } // end of namespace ::sd::slidesorter::view

#endif
