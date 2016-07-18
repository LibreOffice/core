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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLIDESORTERVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLIDESORTERVIEW_HXX

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

class LayeredDevice;
class Layouter;
class PageObjectPainter;
class ToolTip;

class SlideSorterView
    : public sd::View
{
public:

    /** Create a new view for the slide sorter.
        @param rViewShell
            This reference is simply passed to the base class and not used
            by this class.

    */
    explicit SlideSorterView (SlideSorter& rSlideSorter);
    void Init();

    virtual ~SlideSorterView();
    void Dispose();

    SlideSorterView(const SlideSorterView&) = delete;
    SlideSorterView& operator=(const SlideSorterView&) = delete;

    /** Set the general way of layouting the page objects.  Note that this
        method does not trigger any repaints or layouts.
    */
    bool SetOrientation (const Layouter::Orientation eOrientation);
    Layouter::Orientation GetOrientation() const { return meOrientation;}

    void RequestRepaint();
    void RequestRepaint (const model::SharedPageDescriptor& rDescriptor);
    void RequestRepaint (const Rectangle& rRepaintBox);
    void RequestRepaint (const vcl::Region& rRepaintRegion);

    Rectangle GetModelArea();

    /** Return the index of the page that is rendered at the given position.
        @param rPosition
            The position is expected to be in pixel coordinates.
        @return
            The returned index is -1 when there is no page object at the
            given position.
    */
    sal_Int32 GetPageIndexAtPoint (const Point& rPosition) const;

    view::Layouter& GetLayouter();

    virtual void ModelHasChanged() override;

    /** This method is typically called before a model change takes place.
        All references to model data are released.  PostModelChange() has to
        be called to complete the handling of the model change.  When the
        calls to Pre- and PostModelChange() are very close to each other you
        may call HandleModelChange() instead.
    */
    void PreModelChange();

    /** This method is typically called after a model change took place.
        References to model data are re-allocated.  Call this method only
        after PreModelChange() has been called.
    */
    void PostModelChange();

    /** This method is a convenience function that simply calls
        PreModelChange() and then PostModelChange().
    */
    void HandleModelChange();

    void HandleDrawModeChange();

    void Resize();
    virtual void CompleteRedraw (
        OutputDevice* pDevice,
        const vcl::Region& rPaintArea,
        sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr) override;
    void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea);

    virtual void ConfigurationChanged (
        utl::ConfigurationBroadcaster* pBroadcaster,
        sal_uInt32 nHint) override;

    void HandleDataChangeEvent();

    void Layout();
    /** This tells the view that it has to re-determine the visibility of
        the page objects before painting them the next time.
    */
    void InvalidatePageObjectVisibilities();

    std::shared_ptr<cache::PageCache> const & GetPreviewCache();

    /** Return the range of currently visible page objects including the
        first and last one in that range.
        @return
            The returned pair of page object indices is empty when the
            second index is lower than the first.
    */
    Pair const & GetVisiblePageRange();

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
    void AddVisibilityChangeListener (const Link<LinkParamNone*,void>& rListener);

    /** Remove a listener that is called when the set of visible slides changes.
        @param rListener
            It is save to pass a listener that was not added or has been
            removed previously.  Such calls are ignored.
    */
    void RemoveVisibilityChangeListener (const Link<LinkParamNone*,void>& rListener);

    /** The page under the mouse is not highlighted in some contexts.  Call
        this method on context changes.
    */
    void UpdatePageUnderMouse ();
    void UpdatePageUnderMouse (const Point& rMousePosition);
    void SetPageUnderMouse (const model::SharedPageDescriptor& rpDescriptor);

    bool SetState (
        const model::SharedPageDescriptor& rpDescriptor,
        const model::PageDescriptor::State eState,
        const bool bStateValue);

    void UpdateOrientation();

    std::shared_ptr<PageObjectPainter> const & GetPageObjectPainter();
    const std::shared_ptr<LayeredDevice>& GetLayeredDevice() const { return mpLayeredDevice;}

    class DrawLock
    {
    public:
        DrawLock (SlideSorter& rSlideSorter);
        ~DrawLock();
        /** When the DrawLock is disposed then it will not request a repaint
            on destruction.
        */
        void Dispose();
    private:
        view::SlideSorterView& mrView;
        VclPtr<sd::Window> mpWindow;
    };

    ToolTip& GetToolTip() const;

    virtual void DragFinished (sal_Int8 nDropAction) override;

protected:
    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;

private:
    SlideSorter& mrSlideSorter;
    model::SlideSorterModel& mrModel;
    bool mbIsDisposed;
    ::std::unique_ptr<Layouter> mpLayouter;
    bool mbPageObjectVisibilitiesValid;
    std::shared_ptr<cache::PageCache> mpPreviewCache;
    std::shared_ptr<LayeredDevice> mpLayeredDevice;
    Range maVisiblePageRange;
    Size maPreviewSize;
    bool mbPreciousFlagUpdatePending;
    Layouter::Orientation meOrientation;
    model::SharedPageDescriptor mpPageUnderMouse;
    std::shared_ptr<PageObjectPainter> mpPageObjectPainter;
    vcl::Region maRedrawRegion;
    SharedILayerPainter mpBackgroundPainter;
    std::unique_ptr<ToolTip> mpToolTip;
    bool mbIsRearrangePending;
    ::std::vector<Link<LinkParamNone*,void>> maVisibilityChangeListeners;

    /** Determine the visibility of all page objects.
    */
    void DeterminePageObjectVisibilities();

    void UpdatePreciousFlags();
    void RequestRearrange();
    void Rearrange();
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
