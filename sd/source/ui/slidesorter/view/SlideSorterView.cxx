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

#include "view/SlideSorterView.hxx"

#include "ViewShellBase.hxx"
#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShell.hxx"
#include "SlsViewCacheContext.hxx"
#include "SlsLayeredDevice.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsPageObjectPainter.hxx"
#include "view/SlsILayerPainter.hxx"
#include "view/SlsToolTip.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsClipboard.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "cache/SlsCacheContext.hxx"
#include "DrawDocShell.hxx"
#include "PaneDockingWindow.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include "glob.hrc"

#include <svl/itempool.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopage.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>

#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <algorithm>
#include <svx/sdrpagewindow.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <canvas/elapsedtime.hxx>

//#define DEBUG_TIMING
#ifdef DEBUG_TIMING
#include <vector>
#endif

using namespace std;
using namespace ::sd::slidesorter::model;
using namespace ::drawinglayer::primitive2d;

namespace sd { namespace slidesorter { namespace view {

namespace {
    /** Wrapper around the SlideSorterView that supports the IPainter
        interface and that allows the LayeredDevice to hold the
        SlideSorterView (held as unique_ptr by the SlideSorter) as
        shared_ptr.
    */
    class Painter : public ILayerPainter
    {
    public:
        explicit Painter (SlideSorterView& rView) : mrView(rView) {}
        virtual ~Painter() {}

        virtual void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea) override
        {
            mrView.Paint(rDevice,rRepaintArea);
        }

        virtual void SetLayerInvalidator (const SharedILayerInvalidator&) override {}

    private:
        SlideSorterView& mrView;
    };
}

class BackgroundPainter
    : public ILayerPainter
{
public:
    explicit BackgroundPainter (const Color& rBackgroundColor) : maBackgroundColor(rBackgroundColor) {}
    virtual ~BackgroundPainter() {}
    BackgroundPainter(const BackgroundPainter&) = delete;
    BackgroundPainter& operator=(const BackgroundPainter&) = delete;

    virtual void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea) override
    {
        rDevice.SetFillColor(maBackgroundColor);
        rDevice.SetLineColor();
        rDevice.DrawRect(rRepaintArea);
    }

    virtual void SetLayerInvalidator (const SharedILayerInvalidator&) override {}

    void SetColor (const Color& rColor) { maBackgroundColor = rColor; }

private:
    Color maBackgroundColor;
};


SlideSorterView::SlideSorterView (SlideSorter& rSlideSorter)
    : ::sd::View (
          *rSlideSorter.GetModel().GetDocument(),
          rSlideSorter.GetContentWindow(),
          rSlideSorter.GetViewShell()),
      mrSlideSorter(rSlideSorter),
      mrModel(rSlideSorter.GetModel()),
      mbIsDisposed(false),
      mpLayouter (new Layouter(rSlideSorter.GetContentWindow(), rSlideSorter.GetTheme())),
      mbPageObjectVisibilitiesValid (false),
      mpPreviewCache(),
      mpLayeredDevice(new LayeredDevice(rSlideSorter.GetContentWindow())),
      maVisiblePageRange(-1,-1),
      maPreviewSize(0,0),
      mbPreciousFlagUpdatePending(true),
      meOrientation(Layouter::GRID),
      mpPageUnderMouse(),
      mpPageObjectPainter(),
      mpBackgroundPainter(
          new BackgroundPainter(mrSlideSorter.GetTheme()->GetColor(Theme::Color_Background))),
      mpToolTip(new ToolTip(mrSlideSorter)),
      mbIsRearrangePending(true),
      maVisibilityChangeListeners()
{
    // Hide the page that contains the page objects.
    SetPageVisible (false);

    // Register the background painter on level 1 to avoid the creation of a
    // background buffer.
    mpLayeredDevice->RegisterPainter(mpBackgroundPainter, 1);

    // Wrap a shared_ptr-held-wrapper around this view and register it as
    // painter at the layered device.  There is no explicit destruction: in
    // the SlideSorterView destructor the layered device is destroyed and
    // with it the only reference to the wrapper which therefore is also
    // destroyed.
    SharedILayerPainter pPainter (new Painter(*this));

    // The painter is placed on level 1 to avoid buffering.  This should be
    // a little faster during animations because the previews are painted
    // directly into the window, not via the buffer.
    mpLayeredDevice->RegisterPainter(pPainter, 1);
}

SlideSorterView::~SlideSorterView()
{
    if ( ! mbIsDisposed)
    {
        OSL_ASSERT(mbIsDisposed);
        Dispose();
    }
}

void SlideSorterView::Init()
{
    HandleModelChange();
}

void SlideSorterView::Dispose()
{
    mpLayeredDevice->Dispose();
    mpPreviewCache.reset();

    SetPageUnderMouse(SharedPageDescriptor());

    // Hide the page to avoid problems in the view when deleting
    // visualized objects
    HideSdrPage();

    // Deletion of the objects and the page will be done in SdrModel
    // destructor (as long as objects and pages are added)

    OSL_ASSERT(mpLayeredDevice.unique());
    mpLayeredDevice.reset();

    mbIsDisposed = true;
}

sal_Int32 SlideSorterView::GetPageIndexAtPoint (const Point& rWindowPosition) const
{
    sal_Int32 nIndex (-1);

    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        nIndex = mpLayouter->GetIndexAtPoint(pWindow->PixelToLogic(rWindowPosition), false, false);

        // Clip the page index against the page count.
        if (nIndex >= mrModel.GetPageCount())
            nIndex = -1;
    }

    return nIndex;
}

Layouter& SlideSorterView::GetLayouter()
{
    return *mpLayouter.get();
}

void SlideSorterView::ModelHasChanged()
{
    // Ignore this call.  Rely on hints sent by the model to get informed of
    // model changes.
}

void SlideSorterView::PreModelChange()
{
    // Reset the slide under the mouse.  It will be re-set in PostModelChange().
    SetPageUnderMouse(SharedPageDescriptor());
}

void SlideSorterView::PostModelChange()
{
    // In PreModelChange() the page objects have been released.  Here we
    // create new ones.
    ::osl::MutexGuard aGuard (mrModel.GetMutex());

    model::PageEnumeration aPageEnumeration (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));

    // The new page objects have to be scaled and positioned.
    RequestRearrange();
    RequestRepaint();
}

/** At the moment for every model change all page objects are destroyed and
    re-created again.  This can be optimized by accepting hints that
    describe the type of change so that existing page objects can be
    reused.
*/
void SlideSorterView::HandleModelChange()
{
    PreModelChange ();
    PostModelChange();
}

void SlideSorterView::HandleDrawModeChange()
{
    // Replace the preview cache with a new and empty one.  The
    // PreviewRenderer that is used by the cache is replaced by this as
    // well.
    mpPreviewCache.reset();
    GetPreviewCache()->InvalidateCache();

    RequestRepaint();
}

void SlideSorterView::HandleDataChangeEvent()
{
    GetPageObjectPainter()->SetTheme(mrSlideSorter.GetTheme());

    // Update the color used by the background painter.
    std::shared_ptr<BackgroundPainter> pPainter (
        std::dynamic_pointer_cast<BackgroundPainter>(mpBackgroundPainter));
    if (pPainter)
        pPainter->SetColor(mrSlideSorter.GetTheme()->GetColor(Theme::Color_Background));

    RequestRepaint();
}

void SlideSorterView::Resize()
{
    UpdateOrientation();

    mpLayeredDevice->Resize();
    RequestRearrange();
}

void SlideSorterView::RequestRearrange()
{
    mbIsRearrangePending = true;
    Rearrange();
}

void SlideSorterView::Rearrange()
{
    if ( ! mbIsRearrangePending)
        return;
    if (mrModel.GetPageCount() <= 0)
        return;

    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if ( ! pWindow)
        return;
    const Size aWindowSize (pWindow->GetSizePixel());
    if (aWindowSize.Width()<=0 || aWindowSize.Height()<=0)
        return;

    const bool bRearrangeSuccess (
        mpLayouter->Rearrange (
            meOrientation,
            aWindowSize,
            mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
            mrModel.GetPageCount()));
    if (bRearrangeSuccess)
    {
        mbIsRearrangePending = false;
        Layout();
        UpdatePageUnderMouse();
        //        RequestRepaint();
    }
}

void SlideSorterView::UpdateOrientation()
{
    // The layout of slides depends on whether the slide sorter is
    // displayed in the center or the side pane.
    if (mrSlideSorter.GetViewShell()->IsMainViewShell())
        SetOrientation(Layouter::GRID);
    else
    {
        // Get access to the docking window.
        vcl::Window* pWindow = mrSlideSorter.GetContentWindow();
        PaneDockingWindow* pDockingWindow = nullptr;
        while (pWindow!=nullptr && pDockingWindow==nullptr)
        {
            pDockingWindow = dynamic_cast<PaneDockingWindow*>(pWindow);
            pWindow = pWindow->GetParent();
        }

        if (pDockingWindow != nullptr)
        {
            const long nScrollBarSize (
                Application::GetSettings().GetStyleSettings().GetScrollBarSize());
            switch (pDockingWindow->GetOrientation())
            {
                case PaneDockingWindow::HorizontalOrientation:
                    if (SetOrientation(Layouter::HORIZONTAL))
                    {
                        const Range aRange (mpLayouter->GetValidVerticalSizeRange());
                        pDockingWindow->SetValidSizeRange(Range(
                            aRange.Min() + nScrollBarSize,
                            aRange.Max() + nScrollBarSize));
                    }
                    break;

                case PaneDockingWindow::VerticalOrientation:
                    if (SetOrientation(Layouter::VERTICAL))
                    {
                        const Range aRange (mpLayouter->GetValidHorizontalSizeRange());
                        pDockingWindow->SetValidSizeRange(Range(
                            aRange.Min() + nScrollBarSize,
                            aRange.Max() + nScrollBarSize));
                    }
                    break;

                case PaneDockingWindow::UnknownOrientation:
                    if (SetOrientation(Layouter::GRID))
                    {
                        const sal_Int32 nAdditionalSize (10);
                        pDockingWindow->SetMinOutputSizePixel(Size(
                            mpLayouter->GetValidHorizontalSizeRange().Min()
                                + nScrollBarSize
                                + nAdditionalSize,
                            mpLayouter->GetValidVerticalSizeRange().Min()
                                + nScrollBarSize
                                + nAdditionalSize));
                    }
                    return;
            }
        }
        else
        {
            // We are not placed in a docking window.  One possible reason
            // is that the slide sorter is temporarily into a cache and was
            // reparented to a non-docking window.
            SetOrientation(Layouter::GRID);
        }
    }
}

void SlideSorterView::Layout ()
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        // Set the model area, i.e. the smallest rectangle that includes all
        // page objects.
        const Rectangle aViewBox (mpLayouter->GetTotalBoundingBox());
        pWindow->SetViewOrigin (aViewBox.TopLeft());
        pWindow->SetViewSize (aViewBox.GetSize());

        std::shared_ptr<PageObjectLayouter> pPageObjectLayouter(
            mpLayouter->GetPageObjectLayouter());
        if (pPageObjectLayouter)
        {
            const Size aNewPreviewSize (mpLayouter->GetPageObjectLayouter()->GetPreviewSize());
            if (maPreviewSize != aNewPreviewSize && GetPreviewCache())
            {
                mpPreviewCache->ChangeSize(aNewPreviewSize, Bitmap::HasFastScale());
                maPreviewSize = aNewPreviewSize;
            }
        }

        // Iterate over all page objects and place them relative to the
        // containing page.
        model::PageEnumeration aPageEnumeration (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
        while (aPageEnumeration.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
            pDescriptor->SetBoundingBox(mpLayouter->GetPageObjectBox(pDescriptor->GetPageIndex()));
        }
    }

    InvalidatePageObjectVisibilities ();
}

void SlideSorterView::InvalidatePageObjectVisibilities()
{
    mbPageObjectVisibilitiesValid = false;
}

void SlideSorterView::DeterminePageObjectVisibilities()
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        // Set this flag to true here so that an invalidate during the
        // visibility calculation can correctly invalidate it again.
        mbPageObjectVisibilitiesValid = true;

        Rectangle aViewArea (pWindow->PixelToLogic(Rectangle(Point(0,0),pWindow->GetSizePixel())));
        const Range aRange (mpLayouter->GetRangeOfVisiblePageObjects(aViewArea));
        const Range aUnion(
            ::std::min(maVisiblePageRange.Min(), aRange.Min()),
            ::std::max(maVisiblePageRange.Max(), aRange.Max()));

        // For page objects that just dropped off the visible area we
        // decrease the priority of pending requests for preview bitmaps.
        if (maVisiblePageRange != aRange)
            mbPreciousFlagUpdatePending |= true;

        model::SharedPageDescriptor pDescriptor;
        for (long nIndex=aUnion.Min(); nIndex<=aUnion.Max(); nIndex++)
        {
            pDescriptor = mrModel.GetPageDescriptor(nIndex);
            if (pDescriptor.get() != nullptr)
                SetState(
                    pDescriptor,
                    PageDescriptor::ST_Visible,
                    aRange.IsInside(nIndex));
        }

        // Broadcast a change of the set of visible page objects.
        if (maVisiblePageRange != aRange)
        {
            maVisiblePageRange = aRange;

            // Tell the listeners that the visibility of some objects has
            // changed.
            ::std::vector<Link<LinkParamNone*,void>>& aChangeListeners (maVisibilityChangeListeners);
            for (::std::vector<Link<LinkParamNone*,void>>::const_iterator
                     iLink(aChangeListeners.begin()),
                     iEnd(aChangeListeners.end());
                 iLink!=iEnd;
                 ++iLink)
            {
                iLink->Call(nullptr);
            }
        }

        // Restore the mouse over state.
        UpdatePageUnderMouse();
    }
}

void SlideSorterView::UpdatePreciousFlags()
{
    if (mbPreciousFlagUpdatePending)
    {
        mbPreciousFlagUpdatePending = false;

        model::SharedPageDescriptor pDescriptor;
        std::shared_ptr<cache::PageCache> pCache = GetPreviewCache();
        sal_Int32 nPageCount (mrModel.GetPageCount());

        for (int nIndex=0; nIndex<=nPageCount; ++nIndex)
        {
            pDescriptor = mrModel.GetPageDescriptor(nIndex);
            if (pDescriptor.get() != nullptr)
            {
                pCache->SetPreciousFlag(
                    pDescriptor->GetPage(),
                    maVisiblePageRange.IsInside(nIndex));
            }
            else
            {
                // At least one cache entry can not be updated.  Remember to
                // repeat the whole updating later and leave the loop now.
                mbPreciousFlagUpdatePending = true;
                break;
            }
        }
    }
}

bool SlideSorterView::SetOrientation (const Layouter::Orientation eOrientation)
{
    if (meOrientation != eOrientation)
    {
        meOrientation = eOrientation;
        return true;
    }
    else
        return false;
}

void SlideSorterView::RequestRepaint()
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        mpLayeredDevice->InvalidateAllLayers(
            Rectangle(
                pWindow->PixelToLogic(Point(0,0)),
                pWindow->PixelToLogic(pWindow->GetSizePixel())));
        pWindow->Invalidate();
    }
}

void SlideSorterView::RequestRepaint (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor)
        RequestRepaint(rpDescriptor->GetBoundingBox());
}

void SlideSorterView::RequestRepaint (const Rectangle& rRepaintBox)
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        mpLayeredDevice->InvalidateAllLayers(rRepaintBox);
        pWindow->Invalidate(rRepaintBox);
    }
}

void SlideSorterView::RequestRepaint (const vcl::Region& rRepaintRegion)
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        mpLayeredDevice->InvalidateAllLayers(rRepaintRegion);
        pWindow->Invalidate(rRepaintRegion);
    }
}

Rectangle SlideSorterView::GetModelArea()
{
    return mpLayouter->GetTotalBoundingBox();
}

#ifdef DEBUG_TIMING
static ::canvas::tools::ElapsedTime gaTimer;
static const size_t gFrameTimeCount (10);
static size_t gFrameTimeIndex (0);
static ::std::vector<double> gFrameTimes (gFrameTimeCount, 0);
static double gFrameTimeSum (0);
static const Rectangle gFrameTimeBox (10,10,150,20);
static double gnLastFrameStart = 0;
#endif

void SlideSorterView::CompleteRedraw (
    OutputDevice* pDevice,
    const vcl::Region& rPaintArea,
    sdr::contact::ViewObjectContactRedirector* pRedirector)
{
    (void)pRedirector;
#ifdef DEBUG_TIMING
    const double nStartTime (gaTimer.getElapsedTime());
    OSL_TRACE("SlideSorterView::CompleteRedraw start at %f, %s",
        nStartTime,
        mnLockRedrawSmph ? "locked" : "");
#endif

    if (pDevice == nullptr || pDevice!=mrSlideSorter.GetContentWindow())
        return;

    // The parent implementation of CompleteRedraw is called only when
    // painting is locked.  We do all the painting ourself.  When painting
    // is locked the parent implementation keeps track of the repaint
    // requests and later, when painting is unlocked, calls CompleteRedraw
    // for all missed repaints.

    if (mnLockRedrawSmph == 0)
    {
        mrSlideSorter.GetContentWindow()->IncrementLockCount();
        if (mpLayeredDevice->HandleMapModeChange())
            DeterminePageObjectVisibilities();
        mpLayeredDevice->Repaint(rPaintArea);
        mrSlideSorter.GetContentWindow()->DecrementLockCount();
    }
    else
    {
        maRedrawRegion.Union(rPaintArea);
    }

#ifdef DEBUG_TIMING
    const double nEndTime (gaTimer.getElapsedTime());
    OSL_TRACE("SlideSorterView::CompleteRedraw end at %f after %fms", nEndTime, (nEndTime-nStartTime)*1000);
    gFrameTimeSum -= gFrameTimes[gFrameTimeIndex];
    gFrameTimes[gFrameTimeIndex] = nStartTime - gnLastFrameStart;
    gnLastFrameStart = nStartTime;
    gFrameTimeSum += gFrameTimes[gFrameTimeIndex];
    gFrameTimeIndex = (gFrameTimeIndex+1) % gFrameTimeCount;

    mrSlideSorter.GetContentWindow()->SetFillColor(COL_BLUE);
    mrSlideSorter.GetContentWindow()->DrawRect(gFrameTimeBox);
    mrSlideSorter.GetContentWindow()->SetTextColor(COL_WHITE);
    mrSlideSorter.GetContentWindow()->DrawText(
        gFrameTimeBox,
        OUString::number(1 / (gFrameTimeSum / gFrameTimeCount)),
        DrawTextFlags::Right | DrawTextFlags::VCenter);
    //    mrSlideSorter.GetContentWindow()->Invalidate(gFrameTimeBox);
#endif
}

void SlideSorterView::Paint (
    OutputDevice& rDevice,
    const Rectangle& rRepaintArea)
{
    if ( ! mpPageObjectPainter)
        if ( ! GetPageObjectPainter())
            return;

    // Update the page visibilities when they have been invalidated.
    if ( ! mbPageObjectVisibilitiesValid)
        DeterminePageObjectVisibilities();

    if (mbPreciousFlagUpdatePending)
        UpdatePreciousFlags();

    if (mbIsRearrangePending)
        Rearrange();

    // Paint all page objects that are fully or partially inside the
    // repaint region.
    const Range aRange (mpLayouter->GetRangeOfVisiblePageObjects(rRepaintArea));
    for (long nIndex=aRange.Min(); nIndex<=aRange.Max(); ++nIndex)
    {
        model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (!pDescriptor || ! pDescriptor->HasState(PageDescriptor::ST_Visible))
            continue;

        mpPageObjectPainter->PaintPageObject(rDevice, pDescriptor);
    }
}

void SlideSorterView::ConfigurationChanged (
    utl::ConfigurationBroadcaster* pBroadcaster,
    sal_uInt32 nHint)
{
    // Some changes of the configuration (some of the colors for example)
    // may affect the previews.  Throw away the old ones and create new ones.
    cache::PageCacheManager::Instance()->InvalidateAllCaches();

    ::sd::View::ConfigurationChanged(pBroadcaster, nHint);
    RequestRepaint();

}

std::shared_ptr<cache::PageCache> const & SlideSorterView::GetPreviewCache()
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow && mpPreviewCache.get() == nullptr)
    {
        mpPreviewCache.reset(
            new cache::PageCache(
                mpLayouter->GetPageObjectSize(),
                Bitmap::HasFastScale(),
                cache::SharedCacheContext(new ViewCacheContext(mrSlideSorter))));
    }

    return mpPreviewCache;
}

Pair const & SlideSorterView::GetVisiblePageRange()
{
    if ( ! mbPageObjectVisibilitiesValid)
        DeterminePageObjectVisibilities();
    return maVisiblePageRange;
}

void SlideSorterView::AddVisibilityChangeListener (const Link<LinkParamNone*,void>& rListener)
{
    if (::std::find (
        maVisibilityChangeListeners.begin(),
        maVisibilityChangeListeners.end(),
        rListener) == maVisibilityChangeListeners.end())
    {
        maVisibilityChangeListeners.push_back(rListener);
    }
}

void SlideSorterView::RemoveVisibilityChangeListener(const Link<LinkParamNone*,void>&rListener)
{
    maVisibilityChangeListeners.erase (
        ::std::find (
            maVisibilityChangeListeners.begin(),
            maVisibilityChangeListeners.end(),
            rListener));
}

ToolTip& SlideSorterView::GetToolTip() const
{
    OSL_ASSERT(mpToolTip);
    return *mpToolTip;
}

void SlideSorterView::DragFinished (sal_Int8 nDropAction)
{
    mrSlideSorter.GetController().GetClipboard().DragFinished(nDropAction);

    View::DragFinished(nDropAction);
}

void SlideSorterView::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    ::sd::View::Notify(rBroadcaster, rHint);
}

void SlideSorterView::UpdatePageUnderMouse ()
{
    VclPtr<ScrollBar> pVScrollBar (mrSlideSorter.GetVerticalScrollBar());
    VclPtr<ScrollBar> pHScrollBar (mrSlideSorter.GetHorizontalScrollBar());
    if ((pVScrollBar && pVScrollBar->IsVisible() && pVScrollBar->IsTracking())
        || (pHScrollBar && pHScrollBar->IsVisible() && pHScrollBar->IsTracking()))
    {
        // One of the scroll bars is tracking mouse movement.  Do not
        // highlight the slide under the mouse in this case.
        SetPageUnderMouse(SharedPageDescriptor());
        return;
    }

    sd::Window *pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow && pWindow->IsVisible() && ! pWindow->IsMouseCaptured())
    {
        const Window::PointerState aPointerState (pWindow->GetPointerState());
        const Rectangle aWindowBox (pWindow->GetPosPixel(), pWindow->GetSizePixel());
        if (aWindowBox.IsInside(aPointerState.maPos))
        {
            UpdatePageUnderMouse(aPointerState.maPos);
            return;
        }
    }

    SetPageUnderMouse(SharedPageDescriptor());
}

void SlideSorterView::UpdatePageUnderMouse (
    const Point& rMousePosition)
{
    SetPageUnderMouse(mrSlideSorter.GetController().GetPageAt(rMousePosition));
}

void SlideSorterView::SetPageUnderMouse (
    const model::SharedPageDescriptor& rpDescriptor)
{
    if (mpPageUnderMouse != rpDescriptor)
    {
        if (mpPageUnderMouse)
            SetState(mpPageUnderMouse, PageDescriptor::ST_MouseOver, false);

        mpPageUnderMouse = rpDescriptor;

        if (mpPageUnderMouse)
            SetState(mpPageUnderMouse, PageDescriptor::ST_MouseOver, true);

        // Change the quick help text to display the name of the page under
        // the mouse.
        mpToolTip->SetPage(rpDescriptor);
    }
}

bool SlideSorterView::SetState (
    const model::SharedPageDescriptor& rpDescriptor,
    const PageDescriptor::State eState,
    const bool bStateValue)
{
    if ( ! rpDescriptor)
        return false;

    const bool bModified (rpDescriptor->SetState(eState, bStateValue));
    if ( ! bModified)
        return false;

    // When the page object is not visible (i.e. not on the screen then
    // nothing has to be painted.
    if (rpDescriptor->HasState(PageDescriptor::ST_Visible))
    {
        // For most states a change of that state leads to visible
        // difference and we have to request a repaint.
        if (eState != PageDescriptor::ST_WasSelected)
            RequestRepaint(rpDescriptor);
    }

    return bModified;
}

std::shared_ptr<PageObjectPainter> const & SlideSorterView::GetPageObjectPainter()
{
    if ( ! mpPageObjectPainter)
        mpPageObjectPainter.reset(new PageObjectPainter(mrSlideSorter));
    return mpPageObjectPainter;
}

//===== SlideSorterView::DrawLock =============================================

SlideSorterView::DrawLock::DrawLock (SlideSorter& rSlideSorter)
    : mrView(rSlideSorter.GetView()),
      mpWindow(rSlideSorter.GetContentWindow())
{
    if (mrView.mnLockRedrawSmph == 0)
        mrView.maRedrawRegion.SetEmpty();
    ++mrView.mnLockRedrawSmph;
}

SlideSorterView::DrawLock::~DrawLock()
{
    OSL_ASSERT(mrView.mnLockRedrawSmph>0);
    --mrView.mnLockRedrawSmph;
    if (mrView.mnLockRedrawSmph == 0)
        if (mpWindow)
        {
            mpWindow->Invalidate(mrView.maRedrawRegion);
            mpWindow->Update();
        }
}

void SlideSorterView::DrawLock::Dispose()
{
    mpWindow.reset();
}

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
