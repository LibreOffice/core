/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterView.cxx,v $
 * $Revision: 1.27 $
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

#include "precompiled_sd.hxx"

#include "view/SlideSorterView.hxx"

#include "ViewShellBase.hxx"
#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShell.hxx"
#include "SlsViewCacheContext.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "view/SlsHighlightObject.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageObjectFactory.hxx"
#include "controller/SlsProperties.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "cache/SlsCacheContext.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"
#include "DrawDocShell.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include "glob.hrc"

#include <svx/svdpagv.hxx>
#include <svx/svdopage.hxx>
#include <svx/xoutx.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <algorithm>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svtools/itempool.hxx>

using namespace std;
using namespace ::sd::slidesorter::model;

namespace sd { namespace slidesorter { namespace view {

TYPEINIT1(SlideSorterView, ::sd::View);


SlideSorterView::SlideSorterView (SlideSorter& rSlideSorter)
    : ::sd::View (
        rSlideSorter.GetModel().GetDocument(),
        NULL,
        rSlideSorter.GetViewShell()),
    mrSlideSorter(rSlideSorter),
    mrModel(rSlideSorter.GetModel()),
    maPageModel(),
    mpPage(new SdrPage(maPageModel)),
    mpLayouter (new Layouter ()),
    mbPageObjectVisibilitiesValid (false),
    mpPreviewCache(),
    mpViewOverlay (new ViewOverlay(rSlideSorter)),
    mnFirstVisiblePageIndex(0),
    mnLastVisiblePageIndex(-1),
    mbModelChangedWhileModifyEnabled(true),
    maPreviewSize(0,0),
    mbPreciousFlagUpdatePending(true),
    maPageNumberAreaModelSize(0,0),
    maModelBorder(),
    meOrientation(VERTICAL)
{
    maPageModel.GetItemPool().FreezeIdRanges();

    // Hide the page that contains the page objects.
    SetPageVisible (FALSE);

    LocalModelHasChanged();
}




SlideSorterView::~SlideSorterView (void)
{
    // Inform the contact objects to disconnect from the preview cache.
    // Otherwise each dying contact object invalidates its preview.  When
    // the previews are kept for a later re-use than this invalidation is
    // not wanted.
    ::boost::shared_ptr<cache::PageCache> pEmptyCache;
    model::PageEnumeration aPageEnumeration (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aPageEnumeration.HasMoreElements())
    {
        view::PageObjectViewObjectContact* pContact
            = aPageEnumeration.GetNextElement()->GetViewObjectContact();
        if (pContact != NULL)
            pContact->SetCache(pEmptyCache);
    }
    mpPreviewCache.reset();

    // Remove all page objects from the page.
    mpPage->Clear();
}




sal_Int32 SlideSorterView::GetPageIndexAtPoint (const Point& rPosition) const
{
    sal_Int32 nIndex (-1);

    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        nIndex = mpLayouter->GetIndexAtPoint (pWindow->PixelToLogic (rPosition));

        // Clip the page index against the page count.
        if (nIndex >= mrModel.GetPageCount())
            nIndex = -1;
    }

    return nIndex;
}




sal_Int32 SlideSorterView::GetFadePageIndexAtPoint (
    const Point& rPosition) const
{
    sal_Int32 nIndex (-1);

    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        Point aModelPosition (pWindow->PixelToLogic (rPosition));
        nIndex = mpLayouter->GetIndexAtPoint(
            aModelPosition,
            true // Include page borders into hit test
            );

        // Clip the page index against the page count.
        if (nIndex >= mrModel.GetPageCount())
            nIndex = -1;

        if (nIndex >= 0)
        {
            // Now test whether the given position is inside the area of the
            // fade effect indicator.
            view::PageObjectViewObjectContact* pContact
                = mrModel.GetPageDescriptor(nIndex)->GetViewObjectContact();
            if (pContact != NULL)
            {
                if ( ! pContact->GetBoundingBox(
                    *pWindow,
                    PageObjectViewObjectContact::FadeEffectIndicatorBoundingBox,
                    PageObjectViewObjectContact::ModelCoordinateSystem).IsInside (
                    aModelPosition))
                {
                    nIndex = -1;
                }
            }
            else
                nIndex = -1;
        }
    }

    return nIndex;
}




Layouter& SlideSorterView::GetLayouter (void)
{
    return *mpLayouter.get();
}




void SlideSorterView::ModelHasChanged (void)
{
    if (mbModelChangedWhileModifyEnabled)
    {
        controller::SlideSorterController::ModelChangeLock alock( mrSlideSorter.GetController() );
        mrSlideSorter.GetController().HandleModelChange();
        LocalModelHasChanged();
    }
}




void SlideSorterView::LocalModelHasChanged(void)
{
    mbModelChangedWhileModifyEnabled = false;

    // First call our base class.
    View::ModelHasChanged ();

    // Then re-set the page as current page that contains the page objects.
    ShowSdrPage(mpPage);

    // Initialize everything that depends on a page view, now that we have
    // one.
    //      SetApplicationDocumentColor(
    //          Application::GetSettings().GetStyleSettings().GetWindowColor());

    UpdatePageBorders();
}




void SlideSorterView::PreModelChange (void)
{
    // Reset the slide under the mouse.  It will be set to the correct slide
    // on the next mouse motion.
    GetOverlay().GetMouseOverIndicatorOverlay().SetSlideUnderMouse(SharedPageDescriptor());

    // Tell the page descriptors of the model that the page objects do not
    // exist anymore.
    model::PageEnumeration aPageEnumeration (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aPageEnumeration.HasMoreElements())
        aPageEnumeration.GetNextElement()->ReleasePageObject();

    // Remove all page objects from the page.
    mpPage->Clear();
}




void SlideSorterView::PostModelChange (void)
{
    // In PreModelChange() the page objects have been released.  Here we
    // create new ones.
    ::osl::MutexGuard aGuard (mrModel.GetMutex());

    model::PageEnumeration aPageEnumeration (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aPageEnumeration.HasMoreElements())
    {
        SdrPageObj* pPageObject = aPageEnumeration.GetNextElement()->GetPageObject();
        if (pPageObject != NULL)
            AddSdrObject(*pPageObject);
    }

    // The new page objects have to be scaled and positioned.
    Layout ();
}




/** At the moment for every model change all page objects are destroyed and
    re-created again.  This can be optimized by accepting hints that
    describe the type of change so that existing page objects can be
    reused.
*/
void SlideSorterView::HandleModelChange (void)
{
    PreModelChange ();
    PostModelChange();
}




void SlideSorterView::HandleDrawModeChange (void)
{
    UpdatePageBorders();

    // Replace the preview cache with a new and empty one.  The
    // PreviewRenderer that is used by the cache is replaced by this as
    // well.
    mpPreviewCache.reset();
    GetPreviewCache()->InvalidateCache(true);
    mrModel.SetPageObjectFactory(
        ::std::auto_ptr<controller::PageObjectFactory>(
            new controller::PageObjectFactory(
                GetPreviewCache(),
                mrSlideSorter.GetController().GetProperties())));

    RequestRepaint();
}




void SlideSorterView::Resize (void)
{
    ::sd::Window* pWindow = GetWindow();
    if (mrModel.GetPageCount()>0 && pWindow != NULL)
    {
        UpdatePageBorders();
        bool bRearrangeSuccess (false);
        if (meOrientation == HORIZONTAL)
        {
            bRearrangeSuccess = mpLayouter->RearrangeHorizontal (
                pWindow->GetSizePixel(),
                mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
                pWindow,
                mrModel.GetPageCount());
        }
        else
        {
            bRearrangeSuccess = mpLayouter->RearrangeVertical (
                pWindow->GetSizePixel(),
                mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
                pWindow);
        }

        if (bRearrangeSuccess)
        {
            Layout();
            pWindow->Invalidate();
        }
    }
}




void SlideSorterView::Layout ()
{
    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        // Set the model area, i.e. the smallest rectangle that includes all
        // page objects.
        Rectangle aViewBox (mpLayouter->GetPageBox(mrModel.GetPageCount()));
        pWindow->SetViewOrigin (aViewBox.TopLeft());
        pWindow->SetViewSize (aViewBox.GetSize());

        Size aPageObjectPixelSize (pWindow->LogicToPixel(mpLayouter->GetPageObjectSize()));
        if (maPreviewSize != aPageObjectPixelSize && mpPreviewCache.get()!=NULL)
        {
            mpPreviewCache->ChangeSize(aPageObjectPixelSize);
            maPreviewSize = aPageObjectPixelSize;
        }

        // Iterate over all page objects and place them relative to the
        // containing page.
        model::PageEnumeration aPageEnumeration (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
        int nIndex = 0;
        while (aPageEnumeration.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
            SdrPageObj* pPageObject = pDescriptor->GetPageObject();
            Rectangle aPageObjectBox (mpLayouter->GetPageObjectBox (nIndex));
            pPageObject->SetRelativePos (aPageObjectBox.TopLeft());

            nIndex += 1;
        }
        // Set the page so that it encloses all page objects.
        mpPage->SetSize (aViewBox.GetSize());

        view::HighlightObject* pHighlightObject
            = mrSlideSorter.GetController().GetHighlightObject();
        if  (pHighlightObject != NULL)
            pHighlightObject->UpdatePosition();
    }

    InvalidatePageObjectVisibilities ();
}




void SlideSorterView::InvalidatePageObjectVisibilities (void)
{
    mbPageObjectVisibilitiesValid = false;
}




void SlideSorterView::DeterminePageObjectVisibilities (void)
{
    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        // Set this flag to true here so that an invalidate during the
        // visibility calculation can correctly invalidate it again.
        mbPageObjectVisibilitiesValid = true;

        Rectangle aViewArea (
            Point(0,0),
            pWindow->GetSizePixel());
        aViewArea = pWindow->PixelToLogic (aViewArea);
        int nFirstIndex =
            mpLayouter->GetIndexOfFirstVisiblePageObject (aViewArea);
        int nLastIndex =
            mpLayouter->GetIndexOfLastVisiblePageObject (aViewArea);

        // For page objects that just dropped off the visible area we
        // decrease the priority of pending requests for preview bitmaps.

        int nMinIndex = ::std::min (mnFirstVisiblePageIndex, nFirstIndex);
        int nMaxIndex = ::std::max (mnLastVisiblePageIndex, nLastIndex);
        if (mnFirstVisiblePageIndex!=nFirstIndex || mnLastVisiblePageIndex!=nLastIndex)
            mbPreciousFlagUpdatePending |= true;
        model::SharedPageDescriptor pDescriptor;
        view::PageObjectViewObjectContact* pContact;
        for (int nIndex=nMinIndex; nIndex<=nMaxIndex; nIndex++)
        {
            // Determine the visibility before and after the change so that
            // we can handle the page objects for which the visibility has
            // changed.
            bool bWasVisible = nIndex>=mnFirstVisiblePageIndex
                && nIndex<=mnLastVisiblePageIndex;
            bool bIsVisible = nIndex>=nFirstIndex && nIndex<=nLastIndex;

            // Get the view-object-contact.
            if (bWasVisible != bIsVisible)
            {
                pContact = NULL;
                pDescriptor = mrModel.GetPageDescriptor(nIndex);
                if (pDescriptor.get() != NULL)
                    pContact = pDescriptor->GetViewObjectContact();

                if (pDescriptor.get() != NULL)
                    pDescriptor->SetVisible (bIsVisible);
            }

        }
        mnFirstVisiblePageIndex = nFirstIndex;
        mnLastVisiblePageIndex = nLastIndex;
    }
}




void SlideSorterView::UpdatePreciousFlags (void)
{
    if (mbPreciousFlagUpdatePending)
    {
        mbPreciousFlagUpdatePending = false;

        model::SharedPageDescriptor pDescriptor;
        ::boost::shared_ptr<cache::PageCache> pCache = GetPreviewCache();
        sal_Int32 nPageCount (mrModel.GetPageCount());

        for (int nIndex=0; nIndex<=nPageCount; ++nIndex)
        {
            pDescriptor = mrModel.GetPageDescriptor(nIndex);
            if (pDescriptor.get() != NULL)
            {
                pCache->SetPreciousFlag(
                    pDescriptor->GetPage(),
                    (nIndex>=mnFirstVisiblePageIndex && nIndex<=mnLastVisiblePageIndex));
                SSCD_SET_VISIBILITY(mrModel.GetDocument(), nIndex,
                    (nIndex>=mnFirstVisiblePageIndex && nIndex<=mnLastVisiblePageIndex));
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




void SlideSorterView::SetOrientation (const Orientation eOrientation)
{
    meOrientation = eOrientation;
    RequestRepaint();
}




SlideSorterView::Orientation SlideSorterView::GetOrientation (void) const
{
    return meOrientation;
}




void SlideSorterView::RequestRepaint (void)
{
    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
        pWindow->Invalidate();
}




void SlideSorterView::RequestRepaint (const model::SharedPageDescriptor& rpDescriptor)
{
    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
        pWindow->Invalidate(
            GetPageBoundingBox (
                rpDescriptor,
                CS_MODEL,
                BBT_INFO));
}




Rectangle SlideSorterView::GetModelArea (void)
{
    return Rectangle (
        Point (0,0),
        Size (mpPage->GetSize().Width(),mpPage->GetSize().Height()));
}




Rectangle SlideSorterView::GetPageBoundingBox (
    const model::SharedPageDescriptor& rpDescriptor,
    CoordinateSystem eCoordinateSystem,
    BoundingBoxType eBoundingBoxType) const
{
    Rectangle aBBox;
    SdrObject* pPageObject = rpDescriptor->GetPageObject();
    if (pPageObject != NULL)
    {
        aBBox = pPageObject->GetCurrentBoundRect();
        AdaptBoundingBox (aBBox, eCoordinateSystem, eBoundingBoxType);
    }

    return aBBox;
}




Rectangle SlideSorterView::GetPageBoundingBox (
    sal_Int32 nIndex,
    CoordinateSystem eCoordinateSystem,
    BoundingBoxType eBoundingBoxType) const
{
    Rectangle aBBox;
    if (nIndex >= 0 && nIndex<mrModel.GetPageCount())
    {
        aBBox = mpLayouter->GetPageObjectBox(nIndex);
        AdaptBoundingBox (aBBox, eCoordinateSystem, eBoundingBoxType);
    }

    return aBBox;
}




void SlideSorterView::CompleteRedraw (
    OutputDevice* pDevice,
    const Region& rPaintArea,
    USHORT nPaintMode,
    ::sdr::contact::ViewObjectContactRedirector* pRedirector)
{
    if (mnLockRedrawSmph == 0)
    {
        // Update the page visibilities when they have been invalidated.
        if ( ! mbPageObjectVisibilitiesValid)
            DeterminePageObjectVisibilities();

        if (mbPreciousFlagUpdatePending)
            UpdatePreciousFlags();

        // Call the base class InitRedraw even when re-drawing is locked to
        // let it remember the request for a redraw.
        View::CompleteRedraw (pDevice, rPaintArea, nPaintMode, pRedirector);
    }
    else
    {
        // In sd::View::CompleteRedraw() this call is recorded and given
        // region is painted when the view is unlocked.
        View::CompleteRedraw (pDevice, rPaintArea, nPaintMode, pRedirector);
    }
}




void SlideSorterView::InvalidateOneWin (::Window& rWindow)
{
    //  if ( IsInvalidateAllowed() )
    View::InvalidateOneWin (rWindow);
}




void SlideSorterView::InvalidateOneWin (
    ::Window& rWindow,
    const Rectangle& rPaintArea)
{
    //  if( IsInvalidateAllowed() )
    View::InvalidateOneWin (rWindow, rPaintArea);
}




::sd::Window* SlideSorterView::GetWindow (void) const
{
    return static_cast< ::sd::Window*>(GetFirstOutputDevice());
}




void SlideSorterView::AdaptBoundingBox (
        Rectangle& rModelPageObjectBoundingBox,
        CoordinateSystem eCoordinateSystem,
        BoundingBoxType eBoundingBoxType) const
{
    CoordinateSystem aCurrentCoordinateSystem = CS_MODEL;
    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        if (eBoundingBoxType == BBT_INFO)
        {
            // Make the box larger so that it encloses all relevant
            // displayed information.
            if (aCurrentCoordinateSystem == CS_MODEL)
            {
                // The relevant offsets are given in pixel values.  Therefore
                // transform the box first into screen coordinates.
                rModelPageObjectBoundingBox
                    = pWindow->LogicToPixel (rModelPageObjectBoundingBox);
                aCurrentCoordinateSystem = CS_SCREEN;
            }
            rModelPageObjectBoundingBox.Left() -= maPagePixelBorder.Left();
            rModelPageObjectBoundingBox.Right() += maPagePixelBorder.Right();
            rModelPageObjectBoundingBox.Top() -= maPagePixelBorder.Top();
            rModelPageObjectBoundingBox.Bottom() += maPagePixelBorder.Bottom();
        }

        // Make sure that the bounding box is given in the correct coordinate
        // system.
        if (eCoordinateSystem != aCurrentCoordinateSystem)
        {
            if (eCoordinateSystem == CS_MODEL)
                rModelPageObjectBoundingBox
                    = pWindow->PixelToLogic (rModelPageObjectBoundingBox);
            else
                rModelPageObjectBoundingBox
                    = pWindow->LogicToPixel (rModelPageObjectBoundingBox);
        }
    }
}




::boost::shared_ptr<cache::PageCache> SlideSorterView::GetPreviewCache (void)
{
    ::sd::Window* pWindow = GetWindow();
    if (pWindow != NULL && mpPreviewCache.get() == NULL)
    {
        maPreviewSize = pWindow->LogicToPixel(mpLayouter->GetPageObjectSize());
        mpPreviewCache.reset(
            new cache::PageCache(
                maPreviewSize,
                cache::SharedCacheContext(new ViewCacheContext(mrSlideSorter.GetModel(), *this))));
    }

    return mpPreviewCache;
}




ViewOverlay& SlideSorterView::GetOverlay (void)
{
    return *mpViewOverlay.get();
}




::sdr::contact::ObjectContact& SlideSorterView::GetObjectContact (void) const
{
    return GetSdrPageView()->GetPageWindow(0)->GetObjectContact();
}




SlideSorterView::PageRange SlideSorterView::GetVisiblePageRange (void)
{
    const int nMaxPageIndex (mrModel.GetPageCount() - 1);
    if ( ! mbPageObjectVisibilitiesValid)
        DeterminePageObjectVisibilities();
    return PageRange(
        ::std::min(mnFirstVisiblePageIndex,nMaxPageIndex),
        ::std::min(mnLastVisiblePageIndex, nMaxPageIndex));
}




void SlideSorterView::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    ::sd::DrawDocShell* pDocShell = mrModel.GetDocument()->GetDocSh();
    if (pDocShell!=NULL && pDocShell->IsEnableSetModified())
        mbModelChangedWhileModifyEnabled = true;

    ::sd::View::SFX_NOTIFY(rBroadcaster, rBroadcastType, rHint, rHintType);
}




void SlideSorterView::UpdatePageBorders (void)
{
    maPagePixelBorder = SvBorder();
    ::sd::Window* pWindow = GetWindow();
    if (mrModel.GetPageCount()>0 && pWindow!=NULL)
    {
        // Calculate the border in model coordinates.
        maPageNumberAreaModelSize = PageObjectViewObjectContact::CalculatePageNumberAreaModelSize (
            pWindow,
            mrModel.GetPageCount());
        maModelBorder = PageObjectViewObjectContact::CalculatePageModelBorder (
            pWindow,
            mrModel.GetPageCount());

        // Depending on values in the global properties the border has to be
        // extended a little bit.
        ::boost::shared_ptr<controller::Properties> pProperties(
            mrSlideSorter.GetController().GetProperties());
        if (pProperties.get()!=NULL && pProperties->IsHighlightCurrentSlide())
        {
            Size aBorderSize (pWindow->PixelToLogic (Size(3,3)));
            maModelBorder.Left() += aBorderSize.Width();
            maModelBorder.Right() += aBorderSize.Width();
            maModelBorder.Top() += aBorderSize.Height();
            maModelBorder.Bottom() += aBorderSize.Height();
        }

        // Set the border at all page descriptors so that the contact
        // objects have access to them.
        model::PageEnumeration aPageEnumeration (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
        while (aPageEnumeration.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
            pDescriptor->SetModelBorder(maModelBorder);
            pDescriptor->SetPageNumberAreaModelSize(maPageNumberAreaModelSize);
        }

        // Convert the borders to pixel coordinates and store them for later
        // use.
        Size aTopLeftBorders(pWindow->LogicToPixel(
            Size (maModelBorder.Left(), maModelBorder.Top())));
        Size aBottomRightBorders(pWindow->LogicToPixel(
            Size (maModelBorder.Right(), maModelBorder.Bottom())));
        maPagePixelBorder = SvBorder (
            aTopLeftBorders.Width(),
            aTopLeftBorders.Height(),
            aBottomRightBorders.Width(),
            aBottomRightBorders.Height());
    }

    // Finally tell the layouter about the borders.
    mpLayouter->SetBorders (2,5,4,5);
    mpLayouter->SetPageBorders (
        maPagePixelBorder.Left(),
        maPagePixelBorder.Right(),
        maPagePixelBorder.Top(),
        maPagePixelBorder.Bottom());
}




Size SlideSorterView::GetPageNumberAreaModelSize (void) const
{
    return maPageNumberAreaModelSize;
}




SvBorder SlideSorterView::GetModelBorder (void) const
{
    return maModelBorder;
}




void SlideSorterView::AddSdrObject (SdrObject& rObject)
{
    mpPage->InsertObject(&rObject);
    rObject.SetModel(&maPageModel);
}

} } } // end of namespace ::sd::slidesorter::view
