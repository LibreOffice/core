/*************************************************************************
 *
 *  $RCSfile: SlideSorterController.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:12:56 $
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

#include "controller/SlideSorterController.hxx"

#include "SlideSorterViewShell.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "SlsListener.hxx"
#include "controller/SlsFocusManager.hxx"
#include "SlsSlotManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsPageObjectFactory.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsViewOverlay.hxx"
#include "cache/SlsPageCache.hxx"
// other
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "TextLogger.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "PreviewChildWindow.hxx"
#include "SlideChangeChildWindow.hxx"
#include "PreviewWindow.hxx"
#include "FrameView.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "strings.hrc"
#include "app.hrc"
#include "glob.hrc"
#include "fuslshow.hxx"
#include "sdmod.hxx"
#include "sdxfer.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "TextLogger.hxx"

#include <vcl/window.hxx>
#include <svx/svdopage.hxx>
#include <svx/svxids.hrc>
#include <svx/ruler.hxx>
#include <svx/zoomitem.hxx>
#include <svtools/tabbar.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/topfrm.hxx>
#include <tools/link.hxx>

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;
using namespace ::sd::slidesorter::controller;

namespace sd { namespace slidesorter { namespace controller {


SlideSorterController::SlideSorterController (
    SfxViewFrame *pFrame,
    ::Window* pParentWindow,
    SlideSorterViewShell& rViewShell,
    model::SlideSorterModel& rModel,
    view::SlideSorterView& rView)
    : mrViewShell(rViewShell),
      mrModel (rModel),
      mrView (rView),
      mpPageSelector (new PageSelector (mrModel, *this)),
      mpFocusManager (new FocusManager (*this)),
      mpSlotManager (new SlotManager (*this)),
      mpClipboard (new Clipboard (*this)),
      mpScrollBarManager (new ScrollBarManager (
          *this,
          pParentWindow,
          GetViewShell().GetActiveWindow(),
          rViewShell.GetHorizontalScrollBar(),
          rViewShell.GetVerticalScrollBar(),
          rViewShell.GetScrollBarFiller())),
      mpListener(),
      mnModelChangeLockCount (0),
      mbPostModelChangePending (false)
{
    OSL_ASSERT(pFrame!=NULL);
    OSL_ASSERT(pParentWindow!=NULL);

    // The whole background is painted by the view and controls.
    pParentWindow->SetBackground (Wallpaper());

    mrModel.SetPageObjectFactory (::std::auto_ptr<PageObjectFactory> (
        new PageObjectFactory (&mrView.GetPreviewCache())));

    // Connect the view with the window that has been created by our base
    // class.
    ::sd::Window* pWindow = GetViewShell().GetActiveWindow();
    pWindow->SetBackground (Wallpaper());
    mrView.AddWin (pWindow);
    mrView.SetActualWin (pWindow);
    pWindow->SetCenterAllowed (false);
    pWindow->SetViewSize (mrView.GetModelArea().GetSize());
    mrView.HandleModelChange();

    mpScrollBarManager->LateInitialization();
}




void SlideSorterController::Init (void)
{
    // Create the selection function.
    SfxRequest aRequest (
        SID_OBJECT_SELECT,
        0,
        GetModel().GetDocument()->GetItemPool());
    FuPoor* pFunction = CreateSelectionFunction (aRequest);
    GetViewShell().SetCurrentFunction (pFunction);
    GetViewShell().SetOldFunction (pFunction);

    Listener* pListener = new Listener (*this);
    mpListener = ::comphelper::ImplementationReference
        <Listener, uno::XInterface, uno::XWeak> (pListener);

    mpPageSelector->UpdateAllPages();
    MakeSelectionVisible();
}




SlideSorterController::~SlideSorterController (void)
{
    uno::Reference<lang::XComponent> xComponent (
        mpListener.getRef(), uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    // dispose should have been called by now so that nothing is to be done
    // to shut down cleanly.
}




SlideSorterViewShell& SlideSorterController::GetViewShell (void) const
{
    return mrViewShell;
}




model::SlideSorterModel& SlideSorterController::GetModel (void) const
{
    return mrModel;
}




view::SlideSorterView& SlideSorterController::GetView (void) const
{
    return mrView;
}




model::PageDescriptor* SlideSorterController::GetPageAt (
    const Point& aPixelPosition)
{
    sal_Int32 nHitPageIndex =
        mrView.GetPageIndexAtPoint (aPixelPosition);
    model::PageDescriptor* pDescriptorAtPoint = NULL;
    if (nHitPageIndex >= 0)
        pDescriptorAtPoint = mrModel.GetPageDescriptor (nHitPageIndex);

    return pDescriptorAtPoint;
}




model::PageDescriptor* SlideSorterController::GetFadePageAt (
    const Point& aPixelPosition)
{
    sal_Int32 nHitPageIndex =
        mrView.GetFadePageIndexAtPoint (aPixelPosition);
    model::PageDescriptor* pDescriptorAtPoint = NULL;
    if (nHitPageIndex >= 0)
        pDescriptorAtPoint = mrModel.GetPageDescriptor (nHitPageIndex);

    return pDescriptorAtPoint;
}




PageSelector& SlideSorterController::GetPageSelector (void)
{
    return *mpPageSelector.get();
}




FocusManager& SlideSorterController::GetFocusManager (void)
{
    return *mpFocusManager.get();
}




Clipboard& SlideSorterController::GetClipboard (void)
{
    return *mpClipboard.get();
}




ScrollBarManager& SlideSorterController::GetScrollBarManager (void)
{
    return *mpScrollBarManager.get();
}




SdPage* SlideSorterController::GetActualPage (void)
{
    // First try the currently focused page.
    model::PageDescriptor* pDescriptor = NULL;
    if (GetFocusManager().IsFocusShowing())
        pDescriptor = GetFocusManager().GetFocusedPageDescriptor();
    if (pDescriptor == NULL)
    {
        // No focus so try the selection.
        model::SlideSorterModel::Enumeration aEnumeration (
            GetModel().GetSelectedPagesEnumeration());
        if (aEnumeration.HasMoreElements())
            pDescriptor = &aEnumeration.GetNextElement();
    }

    if (pDescriptor != NULL)
        return pDescriptor->GetPage();
    else
        return NULL;
}




void SlideSorterController::FuTemporary (SfxRequest& rRequest)
{
    mpSlotManager->FuTemporary (rRequest);
}




void SlideSorterController::FuPermanent (SfxRequest &rRequest)
{
    mpSlotManager->FuPermanent (rRequest);
}




void SlideSorterController::FuSupport (SfxRequest &rRequest)
{
    mpSlotManager->FuSupport (rRequest);
}




bool SlideSorterController::Command (
    const CommandEvent& rEvent,
    ::sd::Window* pWindow)
{
    bool bEventHasBeenHandled = false;

    switch (rEvent.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            SdPage* pPage = NULL;
            USHORT nPopupId;

            model::SlideSorterModel::Enumeration aSelectedPages (
                GetModel().GetSelectedPagesEnumeration());
            if (aSelectedPages.HasMoreElements())
                pPage = aSelectedPages.GetNextElement().GetPage();

            // When there is no selection, then we show the insertion
            // indicator so that the user knows where a page insertion would
            // take place.
            if (pPage != NULL)
                nPopupId = RID_SLIDETABLE_POPUP;
            else
            {
                nPopupId = RID_SLIDE_NOSEL_POPUP;
                GetView().GetOverlay().GetInsertionIndicatorOverlay()
                    .SetPosition(
                        pWindow->PixelToLogic(rEvent.GetMousePosPixel()));
                GetView().GetOverlay().GetInsertionIndicatorOverlay().Show();
            }

            pWindow->ReleaseMouse();
            GetViewShell().GetViewFrame()->GetDispatcher()->ExecutePopup (
                SdResId(nPopupId));
            if (pPage == NULL)
                GetView().GetOverlay().GetInsertionIndicatorOverlay().Hide();
            bEventHasBeenHandled = true;
        }
        break;
    }

    return bEventHasBeenHandled;
}




void SlideSorterController::LockModelChange (void)
{
    mnModelChangeLockCount += 1;
}




void SlideSorterController::UnlockModelChange (void)
{
    mnModelChangeLockCount -= 1;
    if (mnModelChangeLockCount==0 && mbPostModelChangePending)
        PostModelChange();
}




void SlideSorterController::PreModelChange (void)
{
    mpPageSelector->PrepareModelChange ();
    ::sd::Window* pWindow = GetViewShell().GetActiveWindow();
    if (pWindow != NULL)
        mrView.PreModelChange();

    mbPostModelChangePending = true;
}




void SlideSorterController::PostModelChange (void)
{
    mrModel.Resync();

    ::sd::Window* pWindow = GetViewShell().GetActiveWindow();
    if (pWindow != NULL)
    {
        mrView.PostModelChange ();

        pWindow->SetViewOrigin (Point (0,0));
        pWindow->SetViewSize (mrView.GetModelArea().GetSize());
        GetScrollBarManager().UpdateScrollBars (true);
    }

    mpPageSelector->HandleModelChange ();

    mbPostModelChangePending = false;
}




void SlideSorterController::HandleModelChange (void)
{
    if (mnModelChangeLockCount == 0)
    {
        PreModelChange();
        PostModelChange();
    }
    else
        // Call PreModelChange when not already done.
        if ( ! mbPostModelChangePending)
        {
            PreModelChange();
            // The PostModelChange() call will be made when the model change
            // is unlocked again.
        }
}




IMPL_LINK(SlideSorterController, TabBarHandler, TabBar*, pTabBar)
{
    EditMode eEditMode = EM_PAGE;
    switch (pTabBar->GetCurPageId())
    {
        case SlideSorterViewShell::TBE_SLIDES :
            eEditMode = EM_PAGE;
            break;
        case SlideSorterViewShell::TBE_MASTER_PAGES :
            eEditMode = EM_MASTERPAGE;
            break;
    }
    if (mrModel.GetEditMode() != eEditMode)
    {
        PreModelChange();
        mrModel.SetEditMode (eEditMode);
        PostModelChange();
    }

    return TRUE;
}




IMPL_LINK(SlideSorterController, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        SlideSorterViewShell& rShell = GetViewShell();
        DBG_ASSERT(static_cast<VclWindowEvent*>(pEvent)->GetWindow()
            == rShell.GetParentWindow(),
            "SlideSorter: received event from unknown window");
        if (static_cast<VclWindowEvent*>(pEvent)->GetWindow()
            == rShell.GetParentWindow())
        {
            switch (pEvent->GetId())
            {
                case VCLEVENT_WINDOW_ACTIVATE:
                    //                    rShell.ArrangeGUIElements();
                    mrView.RequestRepaint();
                    break;

                case VCLEVENT_WINDOW_SHOW:
                    //                    rShell.ArrangeGUIElements();
                    mrView.RequestRepaint();
                    break;
            }
        }
    }
    return TRUE;
}




void SlideSorterController::DeleteSelectedPages (void)
{
    LockModelChange();

    // Hide focus.
    bool bIsFocusShowing = GetFocusManager().IsFocusShowing();
    if (bIsFocusShowing)
        GetFocusManager().ToggleFocus();

    GetView().BegUndo (SdResId(STR_UNDO_DELETEPAGES));

    // Store pointers to all selected page descriptors.  This is necessary
    // because the pages get deselected when the first one is deleted.
    SdDrawDocument* pDocument = GetModel().GetDocument();
    model::SlideSorterModel::Enumeration aPageEnumeration (
        GetModel().GetSelectedPagesEnumeration());
    ::std::vector<SdPage*> aSelectedPages;
    while (aPageEnumeration.HasMoreElements())
        aSelectedPages.push_back (aPageEnumeration.GetNextElement().GetPage());

    // Iterate over all pages that where seleted when this method was called
    // and 1) delete the draw page, and 2) delete the notes page.
    ::std::vector<SdPage*>::iterator aI;
    for (aI=aSelectedPages.begin(); aI!=aSelectedPages.end(); aI++)
    {
        SdPage* pPage = *aI;
        USHORT nPage = (pPage->GetPageNum()-1) / 2;

        // 1) Delete draw page.  To be robust in the case that pPage points
        // to a notes page, re-fetch the page via its index.
        pPage = pDocument->GetSdPage (nPage, PK_STANDARD);
        GetView().AddUndo (new SdrUndoDelPage (*pPage));
        pDocument->RemovePage (pPage->GetPageNum());

        // 2) Delete notes page.
        pPage = pDocument->GetSdPage (nPage, PK_NOTES);
        GetView().AddUndo (new SdrUndoDelPage (*pPage));
        pDocument->RemovePage (pPage->GetPageNum());
    }

    GetView().EndUndo ();

    HandleModelChange();

    UnlockModelChange();

    // Show focus and move it to next valid location.
    if (bIsFocusShowing)
        GetFocusManager().ToggleFocus ();
    GetFocusManager().MoveFocus (FocusManager::FMD_NONE);
}




void SlideSorterController::MoveSelectedPages (USHORT nTargetPageIndex)
{
    mrView.LockRedraw (TRUE);
    mrModel.SynchronizeDocumentSelection ();
    LockModelChange();
    BOOL bMoved = mrModel.GetDocument()->MovePages (nTargetPageIndex);
    UnlockModelChange();
    mrView.LockRedraw (FALSE);
}




void SlideSorterController::GetCtrlState (SfxItemSet& rSet)
{
    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // "Letzte Version" vom SFx en/disablen lassen
        SfxViewFrame* pSlideViewFrame = SFX_APP()->GetViewFrame();
        DBG_ASSERT(pSlideViewFrame!=NULL,
            "SlideSorterController::GetCtrlState: ViewFrame not found");
        if (pSlideViewFrame->ISA(SfxTopViewFrame))
        {
            pSlideViewFrame->GetSlotState (SID_RELOAD, NULL, &rSet);
        }
        else        // MI sagt: kein MDIFrame --> disablen
        {
            rSet.DisableItem(SID_RELOAD);
        }
    }

    // Output quality.
    if (rSet.GetItemState(SID_OUTPUT_QUALITY_COLOR)==SFX_ITEM_AVAILABLE
        ||rSet.GetItemState(SID_OUTPUT_QUALITY_GRAYSCALE)==SFX_ITEM_AVAILABLE
        ||rSet.GetItemState(SID_OUTPUT_QUALITY_BLACKWHITE)==SFX_ITEM_AVAILABLE
        ||rSet.GetItemState(SID_OUTPUT_QUALITY_CONTRAST)==SFX_ITEM_AVAILABLE)
    {
        ULONG nMode = GetViewShell().GetActiveWindow()->GetDrawMode();
        UINT16 nQuality = 0;

        switch (nMode)
        {
            case ViewShell::OUTPUT_DRAWMODE_COLOR:
                nQuality = 0;
                break;
            case ViewShell::OUTPUT_DRAWMODE_GRAYSCALE:
                nQuality = 1;
                break;
            case ViewShell::OUTPUT_DRAWMODE_BLACKWHITE:
                nQuality = 2;
                break;
            case ViewShell::OUTPUT_DRAWMODE_CONTRAST:
                nQuality = 3;
                break;
        }

        rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_COLOR,
                (BOOL)(nQuality==0)));
        rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_GRAYSCALE,
                (BOOL)(nQuality==1)));
        rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_BLACKWHITE,
                (BOOL)(nQuality==2)));
        rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_CONTRAST,
                (BOOL)(nQuality==3)));
    }

    // #49150#: Qualitaet des Previewfensters aendern, falls vorhanden
    if (rSet.GetItemState(SID_PREVIEW_QUALITY_COLOR)==SFX_ITEM_AVAILABLE
        ||rSet.GetItemState(SID_PREVIEW_QUALITY_GRAYSCALE)==SFX_ITEM_AVAILABLE
        ||rSet.GetItemState(SID_PREVIEW_QUALITY_BLACKWHITE)==SFX_ITEM_AVAILABLE
        ||rSet.GetItemState(SID_PREVIEW_QUALITY_CONTRAST)==SFX_ITEM_AVAILABLE)
    {
        USHORT nId = PreviewChildWindow::GetChildWindowId();
        if (GetViewShell().GetViewFrame()->GetChildWindow(nId))
        {
            ULONG nMode = GetViewShell().GetFrameView()->GetPreviewDrawMode();
            rSet.Put (SfxBoolItem(SID_PREVIEW_QUALITY_COLOR,
                    (BOOL)(nMode == PREVIEW_DRAWMODE_COLOR)));
            rSet.Put (SfxBoolItem(SID_PREVIEW_QUALITY_GRAYSCALE,
                    (BOOL)(nMode == PREVIEW_DRAWMODE_GRAYSCALE)));
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_BLACKWHITE,
                    (BOOL)(nMode == PREVIEW_DRAWMODE_BLACKWHITE)));
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_CONTRAST,
                    (BOOL)(nMode == PREVIEW_DRAWMODE_CONTRAST)));
        }
        else
        {
            rSet.DisableItem (SID_PREVIEW_QUALITY_COLOR);
            rSet.DisableItem (SID_PREVIEW_QUALITY_GRAYSCALE);
            rSet.DisableItem (SID_PREVIEW_QUALITY_BLACKWHITE);
            rSet.DisableItem (SID_PREVIEW_QUALITY_CONTRAST);
        }
    }

    if (rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) == SFX_ITEM_AVAILABLE)
    {
        rSet.Put (SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, TRUE));
    }
}




void SlideSorterController::GetMenuState ( SfxItemSet& rSet)
{
    mpSlotManager->GetMenuState (rSet);
}




void SlideSorterController::GetStatusBarState (SfxItemSet& rSet)
{
    mpSlotManager->GetStatusBarState (rSet);
}




void SlideSorterController::ExecCtrl (SfxRequest& rRequest)
{
    mpSlotManager->ExecCtrl (rRequest);
}




void SlideSorterController::GetAttrState (SfxItemSet& rSet)
{
    mpSlotManager->GetAttrState (rSet);
}




void SlideSorterController::ExecStatusBar (SfxRequest& rRequest)
{
}




void SlideSorterController::UpdateAllPages (void)
{
    // Do a redraw.
    mrView.InvalidateAllWin();
}




void SlideSorterController::SelectionHasChanged (
    bool bMakeSelectionVisible)
{
    if (bMakeSelectionVisible)
        MakeSelectionVisible();

    SlideSorterViewShell& rViewShell (GetViewShell());
    rViewShell.Invalidate (SID_DIA_EFFECT);
    rViewShell.Invalidate (SID_DIA_SPEED);
    rViewShell.Invalidate (SID_DIA_AUTO);
    rViewShell.Invalidate (SID_DIA_TIME);
    rViewShell.Invalidate (SID_EXPAND_PAGE);
    rViewShell.Invalidate (SID_SUMMARY_PAGE);

    // StatusBar
    rViewShell.Invalidate (SID_STATUS_PAGE);
    rViewShell.Invalidate (SID_STATUS_LAYOUT);

    rViewShell.UpdateSlideChangeWindow ();
    rViewShell.UpdatePreview (GetActualPage());
    /*
    // fire accessible event
    uno::Reference<XAccessible> xAccessible(GetWindow()->GetAccessible(FALSE));

    if (xAccessible.is())
    {
        AccessibleSlideView* pAccessibleView
            = AccessibleSlideView::getImplementation (xAccessible);

        if (pAccessibleView != NULL)
        {
            const uno::Any aOldAny, aNewAny;
            pAccessibleView->FireAccessibleEvent (
                AccessibleEventId::SELECTION_CHANGED,
                aOldAny,
                aNewAny);
        }
    }
        */
}





/** We have to distinguish two cases: a) the selection fits completely into
    the visible area, b) it does not.
    a) When the selection fits completely into the visible area we have to
    decide how to align it.  It is done by scrolling it there and thus when
    we scoll up the (towards the end of the document) the bottom of the
    selection is aligned with the bottom of the window.  When we scroll
    down (towards the beginning of the document) the top of the selection is
    aligned with the top of the window.
    b) We have to decide what part of the selection to make visible.  Here
    we use the eSelectionHint and concentrate on either the first, the last,
    or the most recently selected page.  We then again apply the algorithm
    of a).

*/
void SlideSorterController::MakeSelectionVisible (
    SelectionHint eSelectionHint)
{
    // Determine the descriptors of the first, last, and most recently
    // selected page and the bounding box that encloses all those page objects.
    model::PageDescriptor* pFirst = NULL;
    model::PageDescriptor* pLast = NULL;
    Rectangle aSelectionBox;
    model::SlideSorterModel::Enumeration aSelectedPages (
        mrModel.GetSelectedPagesEnumeration());
    while (aSelectedPages.HasMoreElements())
    {
        model::PageDescriptor* pDescriptor (&aSelectedPages.GetNextElement());

        if (pFirst == NULL)
            pFirst = pDescriptor;
        pLast = pDescriptor;

        aSelectionBox.Union (mrView.GetPageBoundingBox (
            *pDescriptor,
            view::SlideSorterView::CS_MODEL,
            view::SlideSorterView::BBT_INFO));
    }
    model::PageDescriptor* pRecent
        = GetPageSelector().GetMostRecentlySelectedPage();

    // Determine scroll direction and the position in model coordinates that
    // will be aligned with the top or bottom window border.
    Rectangle aVisibleArea (GetViewShell().GetActiveWindow()->PixelToLogic(
        Rectangle(
            Point(0,0),
            GetViewShell().GetActiveWindow()->GetOutputSizePixel())));
    if (aSelectionBox.GetHeight() > aVisibleArea.GetHeight())
    {
        // We can show only a part of the selection.

        // Get the bounding box of the page object on which to concentrate.
        model::PageDescriptor* pRepresentative;
        switch (eSelectionHint)
        {
            case SH_FIRST:
                pRepresentative = pFirst;
                break;

            case SH_LAST:
                pRepresentative = pLast;
                break;

            case SH_RECENT:
                if (pRecent == NULL)
                    pRepresentative = pFirst;
                else
                    pRepresentative = pRecent;
                break;
        }
        if (pRepresentative != NULL)
            aSelectionBox = mrView.GetPageBoundingBox (
                *pRepresentative,
                view::SlideSorterView::CS_MODEL,
                view::SlideSorterView::BBT_INFO);
    }

    MakeRectangleVisible (aSelectionBox);
}




void SlideSorterController::MakeRectangleVisible (const Rectangle& rBox)
{
    Rectangle aVisibleArea (GetViewShell().GetActiveWindow()->PixelToLogic(
        Rectangle(
            Point(0,0),
            GetViewShell().GetActiveWindow()->GetOutputSizePixel())));

    // Scroll the visible area to make aSelectionBox visible.
    long nNewTop = aVisibleArea.Top();
    if (rBox.Top() < aVisibleArea.Top())
        nNewTop = rBox.Top();
    else if (rBox.Bottom() > aVisibleArea.Bottom())
        nNewTop = rBox.Bottom() - aVisibleArea.GetHeight();
    // otherwise we do not modify the visible area.

    // Make some corrections of the new visible area.
    Rectangle aModelArea (mrView.GetModelArea());
    if (nNewTop + aVisibleArea.GetHeight() > aModelArea.Bottom())
        nNewTop = aModelArea.GetHeight() - aVisibleArea.GetHeight();
    if (nNewTop < aModelArea.Top())
        nNewTop = aModelArea.Top();

    // Scroll.
    if (nNewTop != aVisibleArea.Top())
    {
        GetScrollBarManager().SetTop (nNewTop);
        mrView.InvalidatePageObjectVisibilities ();
    }
}




Rectangle SlideSorterController::Resize (const Rectangle& rAvailableSpace)
{
    Rectangle aWindowArea;
    Rectangle aAvailableSpace (rAvailableSpace);

    ::sd::Window* pWindow = GetViewShell().GetActiveWindow();
    if (pWindow != NULL)
    {
        // Place the scroll bars.
        aWindowArea = GetScrollBarManager().PlaceScrollBars (rAvailableSpace);

        // The browser window gets the remaining space.
        pWindow->SetPosSizePixel (
            aWindowArea.TopLeft(),
            aWindowArea.GetSize());
        mrView.Resize();

        // Adapt the scroll bars to the new zoom factor of the browser
        // window and the arrangement of the page objects.
        GetScrollBarManager().UpdateScrollBars ();
    }

    return aWindowArea;
}




void SlideSorterController::SetZoom (long int nZoom)
{
    ::sd::Window* pWindow = GetViewShell().GetActiveWindow();
    long int nCurrentZoom ((long int)(
        pWindow->GetMapMode().GetScaleX().operator double() * 100));

    if (nZoom > nCurrentZoom)
    {
        Size aPageSize (mrView.GetPageBoundingBox(
            0,
            view::SlideSorterView::CS_MODEL,
            view::SlideSorterView::BBT_SHAPE).GetSize());
        Size aWindowSize (pWindow->PixelToLogic(
            pWindow->GetOutputSizePixel()));

        // The zoom factor must not grow by more then the ratio of the
        // widths of the output window and the page objects.
        long nMaxFactor
            = nCurrentZoom * aWindowSize.Width() / aPageSize.Width();
        nMaxFactor = nCurrentZoom * 18 / 10;
        nZoom = Min(nMaxFactor, nZoom);
    }
    if (nZoom < 1)
        nZoom = 1;

    mrView.LockRedraw (TRUE);
    mrView.GetLayouter().SetZoom(nZoom/100.0, pWindow);
    mrView.Layout();
    GetScrollBarManager().UpdateScrollBars (false);
    mrView.GetPreviewCache().InvalidateCache();
    mrView.RequestRepaint();
    mrView.LockRedraw (FALSE);

    /*
        ViewShell::SetZoom (nZoom);
        GetViewFrame()->GetBindings().Invalidate (SID_ATTR_ZOOM);
    */
}




FuPoor* SlideSorterController::CreateSelectionFunction (SfxRequest& rRequest)
{
    return new SelectionFunction (*this, rRequest);
}


} } } // end of namespace ::sd::slidesorter
