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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_VIEW_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_VIEW_HXX

#include "SlideSorter.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsSharedPageDescriptor.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsILayerPainter.hxx"

#include "View.hxx"
#include <sfx2/viewfrm.hxx>
#include "pres.hxx"
#include <tools/gen.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/region.hxx>
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

class Point;

namespace sd { namespace slidesorter { namespace controller {
class Properties;
} } }

namespace sd { namespace slidesorter { namespace cache {
class PageCache;
} } }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {

class ButtonBar;
class LayeredDevice;
class Layouter;
class PageObjectPainter;
class SelectionPainter;
class ToolTip;


class SlideSorterView
    : public sd::View,
      public ::boost::noncopyable
{
public:
    TYPEINFO ();

    /** Create a new view for the slide sorter.
        @param rViewShell
            This reference is simply passed to the base class and not used
            by this class.

    */
    SlideSorterView (SlideSorter& rSlideSorter);
    void Init (void);

    virtual ~SlideSorterView (void);
    void Dispose (void);

    /** Set the general way of layouting the page objects.  Note that this
        method does not trigger any repaints or layouts.
    */
    bool SetOrientation (const Layouter::Orientation eOrientation);
    Layouter::Orientation GetOrientation (void) const;

    void RequestRepaint (void);
    void RequestRepaint (const model::SharedPageDescriptor& rDescriptor);
    void RequestRepaint (const Rectangle& rRepaintBox);
    void RequestRepaint (const Region& rRepaintRegion);

    Rectangle GetModelArea (void);

    /** Return the index of the page that is rendered at the given position.
        @param rPosition
            The position is expected to be in pixel coordinates.
        @return
            The returned index is -1 when there is no page object at the
            given position.
    */
    sal_Int32 GetPageIndexAtPoint (const Point& rPosition) const;

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

    void HandleDrawModeChange (void);

    virtual void Resize (void);
    virtual void CompleteRedraw (
        OutputDevice* pDevice,
        const Region& rPaintArea,
        sdr::contact::ViewObjectContactRedirector* pRedirector = NULL);
    void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea);

    virtual void ConfigurationChanged (
        utl::ConfigurationBroadcaster* pBroadcaster,
        sal_uInt32 nHint);

    void HandleDataChangeEvent (void);

    void Layout (void);
    /** This tells the view that it has to re-determine the visibility of
        the page objects before painting them the next time.
    */
    void InvalidatePageObjectVisibilities (void);

    /** Return the window to which this view renders its output.
    */
    //    ::boost::shared_ptr<sd::Window> GetWindow (void) const;

    ::boost::shared_ptr<cache::PageCache> GetPreviewCache (void);

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

    /** Return the range of currently visible page objects including the
        first and last one in that range.
        @return
            The returned pair of page object indices is empty when the
            second index is lower than the first.
    */
    Pair GetVisiblePageRange (void);

    /** Add a shape to the page.  Typically used from inside
        PostModelChange().
    */
    //    void AddSdrObject (SdrObject& rObject);

    /** Add a listener that is called when the set of visible slides.
        @param rListener
            When this method is called multiple times for the same listener
            the second and all following calls are ignored.  Each listener
            is added only once.
    */
    void AddVisibilityChangeListener (const Link& rListener);

    /** Remove a listener that is called when the set of visible slides changes.
        @param rListener
            It is save to pass a listener that was not added or has been
            removed previously.  Such calls are ignored.
    */
    void RemoveVisibilityChangeListener (const Link& rListener);

    /** The page under the mouse is not highlighted in some contexts.  Call
        this method on context changes.
    */
    void UpdatePageUnderMouse (bool bAnimate);
    void UpdatePageUnderMouse (
        const Point& rMousePosition,
        const bool bIsMouseButtonDown,
        const bool bAnimate = true);
    void UpdatePageUnderMouse (
        const model::SharedPageDescriptor& rpDescriptor,
        const Point& rMousePosition,
        const bool bIsMouseButtonDown,
        const bool bAnimate = true);
    void SetPageUnderMouse (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bAnimate = true);

    bool SetState (
        const model::SharedPageDescriptor& rpDescriptor,
        const model::PageDescriptor::State eState,
        const bool bStateValue,
        const bool bAnimate = true);

    void UpdateOrientation (void);

    ::boost::shared_ptr<PageObjectPainter> GetPageObjectPainter (void);
    ::boost::shared_ptr<LayeredDevice> GetLayeredDevice (void) const;

    class DrawLock
    {
    public:
        DrawLock (SlideSorter& rSlideSorter);
        ~DrawLock (void);
        /** When the DrawLock is disposed then it will not request a repaint
            on destruction.
        */
        void Dispose (void);
    private:
        view::SlideSorterView& mrView;
        SharedSdWindow mpWindow;
    };

    ButtonBar& GetButtonBar (void) const;
    ToolTip& GetToolTip (void) const;

protected:
    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint);

private:
    SlideSorter& mrSlideSorter;
    model::SlideSorterModel& mrModel;
    bool mbIsDisposed;
    ::std::auto_ptr<Layouter> mpLayouter;
    bool mbPageObjectVisibilitiesValid;
    ::boost::shared_ptr<cache::PageCache> mpPreviewCache;
    ::boost::shared_ptr<LayeredDevice> mpLayeredDevice;
    Range maVisiblePageRange;
    bool mbModelChangedWhileModifyEnabled;
    Size maPreviewSize;
    bool mbPreciousFlagUpdatePending;
    Layouter::Orientation meOrientation;
    ::boost::shared_ptr<controller::Properties> mpProperties;
    model::SharedPageDescriptor mpPageUnderMouse;
    sal_Int32 mnButtonUnderMouse;
    ::boost::shared_ptr<PageObjectPainter> mpPageObjectPainter;
    ::boost::shared_ptr<SelectionPainter> mpSelectionPainter;
    Region maRedrawRegion;
    SharedILayerPainter mpBackgroundPainter;
    ::boost::scoped_ptr<ButtonBar> mpButtonBar;
    ::boost::scoped_ptr<ToolTip> mpToolTip;
    bool mbIsRearrangePending;
    ::std::vector<Link> maVisibilityChangeListeners;

    /** Determine the visibility of all page objects.
    */
    void DeterminePageObjectVisibilities (void);

    void UpdatePreciousFlags (void);
    void RequestRearrange (void);
    void Rearrange (void);
};


} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
