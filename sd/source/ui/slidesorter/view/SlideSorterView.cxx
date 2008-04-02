/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterView.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:48:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "view/SlideSorterView.hxx"

#include "ViewShellBase.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShell.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageObjectFactory.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
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
#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <algorithm>
#include <svx/sdr/contact/objectcontact.hxx>

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif
#include <svtools/itempool.hxx>

using namespace std;
using namespace ::sd::slidesorter::model;

namespace sd { namespace slidesorter { namespace view {



SlideSorterView::SlideSorterView (
    SlideSorterViewShell& rViewShell,
    model::SlideSorterModel& rModel)
    : ::sd::View (
        rModel.GetDocument(),
        NULL,
        &rViewShell),
    mrModel (rModel),
    maPageModel(),
    mpPage(new SdrPage(maPageModel)),
    mpLayouter (new Layouter ()),
    mbPageObjectVisibilitiesValid (false),
    mpPreviewCache(),
    mpViewOverlay (new ViewOverlay(rViewShell)),
    mnFirstVisiblePageIndex(0),
    mnLastVisiblePageIndex(-1),
    mbModelChangedWhileModifyEnabled(true),
    maPreviewSize(0,0),
    mbPreciousFlagUpdatePending(true),
    maPageNumberAreaModelSize(0,0),
    maModelBorder()
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
    model::SlideSorterModel::Enumeration aPageEnumeration (
        mrModel.GetAllPagesEnumeration());
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




controller::SlideSorterController& SlideSorterView::GetController (void)
{
    return static_cast<SlideSorterViewShell*>(mpViewSh)->GetSlideSorterController();
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
        controller::SlideSorterController::ModelChangeLock alock( GetController() );
        GetController().HandleModelChange();
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
    GetSdrPageView()->SetApplicationBackgroundColor(
        Application::GetSettings().GetStyleSettings().GetWindowColor());

    UpdatePageBorders();
}




void SlideSorterView::PreModelChange (void)
{
    // Reset the slide under the mouse.  It will be set to the correct slide
    // on the next mouse motion.
    GetOverlay().GetMouseOverIndicatorOverlay().SetSlideUnderMouse(SharedPageDescriptor());

    // Tell the page descriptors of the model that the page objects do not
    // exist anymore.
    model::SlideSorterModel::Enumeration aPageEnumeration (
        mrModel.GetAllPagesEnumeration());
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

    model::SlideSorterModel::Enumeration aPageEnumeration (
        mrModel.GetAllPagesEnumeration());
    while (aPageEnumeration.HasMoreElements())
    {
        SdrPageObj* pPageObject = aPageEnumeration.GetNextElement()->GetPageObject();
        mpPage->InsertObject (pPageObject);
        pPageObject->SetModel (&maPageModel);
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
            new controller::PageObjectFactory(GetPreviewCache())));

    RequestRepaint();
}




void SlideSorterView::Resize (void)
{
    ::sd::Window* pWindow = GetWindow();
    if (mrModel.GetPageCount()>0 && pWindow != NULL)
    {
        UpdatePageBorders();
        if (mpLayouter->Rearrange (
            pWindow->GetSizePixel(),
            mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
            pWindow))
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
        model::SlideSorterModel::Enumeration aPageEnumeration (
            mrModel.GetAllPagesEnumeration());
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

        view::PageObjectViewObjectContact* pContact = NULL;
        model::SharedPageDescriptor pDescriptor;
        ::boost::shared_ptr<cache::PageCache> pCache = GetPreviewCache();
        sal_Int32 nPageCount (mrModel.GetPageCount());

        for (int nIndex=0; nIndex<=nPageCount; ++nIndex)
        {
            pContact = NULL;
            pDescriptor = mrModel.GetPageDescriptor (nIndex);
            if (pDescriptor.get() != NULL)
                pContact = pDescriptor->GetViewObjectContact();

            if (pContact != NULL)
            {
                pCache->SetPreciousFlag(
                    *pContact,
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
        Resize();
        maPreviewSize = pWindow->LogicToPixel(mpLayouter->GetPageObjectSize());
        mpPreviewCache.reset(new cache::PageCache(*this, mrModel, maPreviewSize));
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

        // Set the border at all page descriptors so that the contact
        // objects have access to them.
        model::SlideSorterModel::Enumeration aPageEnumeration (
            mrModel.GetAllPagesEnumeration());
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
    while (false);

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

} } } // end of namespace ::sd::slidesorter::view

