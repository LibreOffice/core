/*************************************************************************
 *
 *  $RCSfile: SlideSorterView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:27:04 $
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

#include "view/SlideSorterView.hxx"

#include "SlideSorterViewShell.hxx"
#include "ViewShell.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "TextLogger.hxx"

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

using namespace std;
using namespace ::sd::slidesorter::model;

namespace {
// The cache can grow up to 4 Megabytes.
const sal_Int32 nMaxCacheSize (4*1024*1024);
}

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
    mpPreviewCache (auto_ptr<cache::PageCache>(
        new cache::PageCache(*this, mrModel, nMaxCacheSize))),
    mpViewOverlay (new ViewOverlay(rViewShell)),
    mnFirstVisiblePageIndex(0),
    mnLastVisiblePageIndex(0)
{
    // Hide the page that contains the page objects.
    SetPageVisible (FALSE);

    ModelHasChanged();
}




SlideSorterView::~SlideSorterView (void)
{
    // We have to delete the contact objects before the cache is destroyed.
    // Otherwise the contact objects call back to the cache that does not
    // exist anymore.
    GetObjectContact().PrepareDelete();
    mpPreviewCache.reset();
}




controller::SlideSorterController& SlideSorterView::GetController (void)
{
    return static_cast<SlideSorterViewShell*>(pViewSh)
        ->GetSlideSorterController();
}




sal_Int32 SlideSorterView::GetPageIndexAtPoint (const Point& rPosition) const
{
    sal_Int32 nIndex = mpLayouter->GetIndexAtPoint (
        GetWindow()->PixelToLogic (rPosition));

    // Clip the page index against the page count.
    if (nIndex >= mrModel.GetPageCount())
        nIndex = -1;

    return nIndex;
}




sal_Int32 SlideSorterView::GetFadePageIndexAtPoint (
    const Point& rPosition) const
{
    Point aModelPosition (GetWindow()->PixelToLogic (rPosition));
    sal_Int32 nIndex = mpLayouter->GetIndexAtPoint (
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
            if ( ! pContact->GetFadeEffectIndicatorArea(GetWindow()).IsInside (
                aModelPosition))
            {
                nIndex = -1;
            }
        }
        else
            nIndex = -1;
    }

    return nIndex;
}




Layouter& SlideSorterView::GetLayouter (void)
{
    return *mpLayouter.get();
}




void SlideSorterView::ModelHasChanged (void)
{
    // First call our base class.
    View::ModelHasChanged ();

    // Then re-set the page as current page that contains the page objects.
    ShowPage (mpPage, Point(0,0));

    // Initialize everything that depends on a page view, now that we have
    // one.
    GetPageViewPvNum(0)->SetApplicationBackgroundColor(
        Application::GetSettings().GetStyleSettings().GetWindowColor());

    UpdatePageBorders();
}




void SlideSorterView::PreModelChange (void)
{
    // Remove all page objects from the page.
    mpPage->Clear();

    // Tell the page descriptors of the model that the page objects do not
    // exist anymore.
    model::SlideSorterModel::Enumeration aPageEnumeration (
        mrModel.GetAllPagesEnumeration());
    while (aPageEnumeration.HasMoreElements())
        aPageEnumeration.GetNextElement().ReleasePageObject();
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
        SdrPageObj* pPageObject
            = aPageEnumeration.GetNextElement().GetPageObject();
        mpPage->InsertObject (pPageObject);
        pPageObject->SetModel (&maPageModel);
    }

    // The new page objects have to be scaled and positioned.
    Resize ();
    RequestRepaint();
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

            // Tell the cache to re-fill its request queues.
            GetPreviewCache().InvalidateCache();
        }
    }
}




void SlideSorterView::Layout ()
{
    ::sd::Window* pWindow = GetWindow();
    OSL_ASSERT (pWindow!=NULL);
    // Set the model area, i.e. the smallest rectangle that includes all
    // page objects.
    Rectangle aViewBox (mpLayouter->GetPageBox(mrModel.GetPageCount()));
    pWindow->SetViewOrigin (aViewBox.TopLeft());
    pWindow->SetViewSize (aViewBox.GetSize());

    // Iterate over all page objects and place them relative to the
    // containing page.
    model::SlideSorterModel::Enumeration aPageEnumeration (
        mrModel.GetAllPagesEnumeration());
    int nIndex = 0;
    while (aPageEnumeration.HasMoreElements())
    {
        model::PageDescriptor& rDescriptor (
            aPageEnumeration.GetNextElement());
        SdrPageObj* pPageObject = rDescriptor.GetPageObject();
        Rectangle aPageObjectBox (mpLayouter->GetPageObjectBox (nIndex));
        pPageObject->SetRelativePos (aPageObjectBox.TopLeft());

        nIndex += 1;
    }
    // Set the page so that it encloses all page objects.
    mpPage->SetSize (aViewBox.GetSize());

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
        model::PageDescriptor* pDescriptor;
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
                pDescriptor = mrModel.GetPageDescriptor (nIndex);
                if (pDescriptor != NULL)
                    pContact = pDescriptor->GetViewObjectContact();

                if (pDescriptor != NULL)
                    pDescriptor->SetVisible (bIsVisible);
            }

            if (bWasVisible && ! bIsVisible)
            {
                // The page object dropped of the visible area.

                // 1. Decrease the priority with with the preview rendering
                // is scheduled.
                if (pContact != NULL)
                    GetPreviewCache().DecreaseRequestPriority (*pContact);

                // 2. Reset the precious flag.
                if (pContact != NULL)
                    GetPreviewCache().SetPreciousFlag(*pContact, false);
            }
            else if (bIsVisible && ! bWasVisible)
            {
                // The page object became visible.

                // Set the precious flag.
                if (pContact != NULL)
                    GetPreviewCache().SetPreciousFlag(*pContact, true);
            }
        }
        mnFirstVisiblePageIndex = nFirstIndex;
        mnLastVisiblePageIndex = nLastIndex;
    }
}




void SlideSorterView::RequestRepaint (void)
{
    GetWindow()->Invalidate();
}




void SlideSorterView::RequestRepaint (model::PageDescriptor& rDescriptor)
{
    GetWindow()->Invalidate(
        GetPageBoundingBox (
            rDescriptor,
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
    model::PageDescriptor& rDescriptor,
    CoordinateSystem eCoordinateSystem,
    BoundingBoxType eBoundingBoxType) const
{
    Rectangle aBBox;
    SdrObject* pPageObject = rDescriptor.GetPageObject();
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




void SlideSorterView::InitRedraw (
    OutputDevice* pDevice,
    const Region& rPaintArea,
    USHORT nPaintMode,
    const Link* pPaintProc)
{
    if (nLockRedrawSmph == 0)
    {
        // Update the page visibilities when they have been invalidated.
        if ( ! mbPageObjectVisibilitiesValid)
            DeterminePageObjectVisibilities ();
    }

    // Call the base class InitRedraw even when re-drawing is locked to let
    // it remember the request for a redraw.  The overlay is hidden during
    // this call and restored afterwards so that its XOR painting works
    // properly.
    GetOverlay().HideAndSave();
    View::InitRedraw (pDevice, rPaintArea, nPaintMode, pPaintProc);
    GetOverlay().Restore();
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
    return static_cast< ::sd::Window*>(GetWin(0));
}




void SlideSorterView::AdaptBoundingBox (
        Rectangle& rModelPageObjectBoundingBox,
        CoordinateSystem eCoordinateSystem,
        BoundingBoxType eBoundingBoxType) const
{
    CoordinateSystem aCurrentCoordinateSystem = CS_MODEL;
    ::sd::Window* pWindow = GetWindow();
    if (eBoundingBoxType == BBT_INFO)
    {
        // Make the box larger so that it encloses all relevant
        // displayed information.
        if (aCurrentCoordinateSystem == CS_MODEL)
        {
            // The relevant offsets are given in pixel values.  Therefore
            // transform the box first into screen coordinates.
            rModelPageObjectBoundingBox
                = GetWindow()->LogicToPixel (rModelPageObjectBoundingBox);
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
                = GetWindow()->PixelToLogic (rModelPageObjectBoundingBox);
        else
            rModelPageObjectBoundingBox
                = GetWindow()->LogicToPixel (rModelPageObjectBoundingBox);
    }
}




cache::PageCache& SlideSorterView::GetPreviewCache (void)
{
    return *mpPreviewCache.get();
}




ViewOverlay& SlideSorterView::GetOverlay (void)
{
    return *mpViewOverlay.get();
}




::sdr::contact::ObjectContact& SlideSorterView::GetObjectContact (void) const
{
    return GetPageViewPvNum(0)->GetWindow(0)->GetObjectContact();
}




void SlideSorterView::UpdatePageBorders (void)
{
    maPagePixelBorder = SvBorder();
    if (mrModel.GetPageCount()>0 && GetWindow()!=NULL)
    {
        // Calculate the border in model coordinates.
        Size aPageNumberAreaModelSize (
            PageObjectViewObjectContact::CalculatePageNumberAreaModelSize (
                GetWindow(),
                mrModel.GetPageCount()));
        SvBorder aModelBorder (
            PageObjectViewObjectContact::CalculatePageModelBorder (
                GetWindow(),
                mrModel.GetPageCount()));

        // Set the border at all page descriptors so that the contact
        // objects have access to them.
        model::SlideSorterModel::Enumeration aPageEnumeration (
            mrModel.GetAllPagesEnumeration());
        while (aPageEnumeration.HasMoreElements())
        {
            model::PageDescriptor& rDescriptor (
                aPageEnumeration.GetNextElement());
            rDescriptor.SetModelBorder (aModelBorder);
            rDescriptor.SetPageNumberAreaModelSize (aPageNumberAreaModelSize);
        }

        // Convert the borders to pixel coordinates and store them for later
        // use.
        Size aTopLeftBorders(GetWindow()->LogicToPixel(
            Size (aModelBorder.Left(), aModelBorder.Top())));
        Size aBottomRightBorders(GetWindow()->LogicToPixel(
            Size (aModelBorder.Right(), aModelBorder.Bottom())));
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



} } } // end of namespace ::sd::slidesorter::view
