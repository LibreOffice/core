/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterController.cxx,v $
 * $Revision: 1.43 $
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

#include "controller/SlideSorterController.hxx"

#include "SlideSorter.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "SlsListener.hxx"
#include "controller/SlsFocusManager.hxx"
#include "SlsSlotManager.hxx"
#include "SlsSelectionCommand.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsPageObjectFactory.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsAnimator.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsFontProvider.hxx"
#include "view/SlsHighlightObject.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"

#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "TextLogger.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "FrameView.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "strings.hrc"
#include "app.hrc"
#include "glob.hrc"
#include "sdmod.hxx"
#include "sdxfer.hxx"
#include "FrameView.hxx"
#include "ViewShellHint.hxx"
#include "AccessibleSlideSorterView.hxx"
#include "AccessibleSlideSorterObject.hxx"

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
#include <vcl/svapp.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;
using namespace ::sd::slidesorter::controller;

namespace sd { namespace slidesorter { namespace controller {


SlideSorterController::SlideSorterController (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mrModel(mrSlideSorter.GetModel()),
      mrView(mrSlideSorter.GetView()),
      mpPageSelector(),
      mpFocusManager(),
      mpSlotManager(),
      mpClipboard(),
      mpScrollBarManager(),
      mpCurrentSlideManager(),
      mpSelectionManager(),
      mpAnimator(new Animator(rSlideSorter)),
      mpListener(),
      mnModelChangeLockCount (0),
      mbPostModelChangePending (false),
      maSelectionBeforeSwitch(),
      mnCurrentPageBeforeSwitch(0),
      mpEditModeChangeMasterPage(NULL),
      maTotalWindowArea(),
      mnPaintEntranceCount(0),
      mbIsContextMenuOpen(false),
      mpProperties(new Properties()),
      mpHighlightObject(NULL)
{
    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
    OSL_ASSERT(pWindow!=NULL);
    if (pWindow != NULL)
    {
        // The whole background is painted by the view and controls.
        ::Window* pParentWindow = pWindow->GetParent();
        OSL_ASSERT(pParentWindow!=NULL);
        pParentWindow->SetBackground (Wallpaper());

        // Connect the view with the window that has been created by our base
        // class.
        pWindow->SetBackground (Wallpaper());
        mrView.AddWindowToPaintView(pWindow);
        mrView.SetActualWin(pWindow);
        pWindow->SetCenterAllowed (false);
        pWindow->SetViewSize (mrView.GetModelArea().GetSize());
        pWindow->EnableRTL(FALSE);

        // Reinitialize colors in Properties with window specific values.
        mpProperties->SetBackgroundColor(
            pWindow->GetSettings().GetStyleSettings().GetWindowColor());
        mpProperties->SetTextColor(
            pWindow->GetSettings().GetStyleSettings().GetWindowTextColor());
        mpProperties->SetSelectionColor(
            pWindow->GetSettings().GetStyleSettings().GetMenuHighlightColor());
        mpProperties->SetHighlightColor(
            pWindow->GetSettings().GetStyleSettings().GetMenuHighlightColor());
    }
}




void SlideSorterController::Init (void)
{
    mrView.HandleModelChange();

    mpCurrentSlideManager.reset(new CurrentSlideManager(mrSlideSorter));
    mpPageSelector.reset(new PageSelector(mrSlideSorter));
    mpFocusManager.reset(new FocusManager(mrSlideSorter));
    mpSlotManager.reset(new SlotManager(mrSlideSorter));
    mpClipboard.reset(new Clipboard(mrSlideSorter));
    mpScrollBarManager.reset(new ScrollBarManager(mrSlideSorter));
    mpSelectionManager.reset(new SelectionManager(mrSlideSorter));

    mpScrollBarManager->LateInitialization();

    // Create the selection function.
    SfxRequest aRequest (
        SID_OBJECT_SELECT,
        0,
        mrModel.GetDocument()->GetItemPool());
    mrSlideSorter.SetCurrentFunction(CreateSelectionFunction(aRequest));

    mpListener = new Listener(mrSlideSorter);

    mpPageSelector->UpdateAllPages();
    GetSelectionManager()->SelectionHasChanged();
}




SlideSorterController::~SlideSorterController (void)
{
    try
    {
        uno::Reference<lang::XComponent> xComponent (
            static_cast<XWeak*>(mpListener.get()), uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
    catch( uno::Exception& e )
    {
        (void)e;
        DBG_ERROR( "sd::SlideSorterController::~SlideSorterController(), exception caught!" );
    }

    // dispose should have been called by now so that nothing is to be done
    // to shut down cleanly.
}




SlideSorter& SlideSorterController::GetSlideSorter (void) const
{
    return mrSlideSorter;
}




model::SharedPageDescriptor SlideSorterController::GetPageAt (
    const Point& aPixelPosition)
{
    sal_Int32 nHitPageIndex (mrView.GetPageIndexAtPoint(aPixelPosition));
    model::SharedPageDescriptor pDescriptorAtPoint;
    if (nHitPageIndex >= 0)
        pDescriptorAtPoint = mrModel.GetPageDescriptor(nHitPageIndex);

    return pDescriptorAtPoint;
}




model::SharedPageDescriptor SlideSorterController::GetFadePageAt (
    const Point& aPixelPosition)
{
    sal_Int32 nHitPageIndex (mrView.GetFadePageIndexAtPoint(aPixelPosition));
    model::SharedPageDescriptor pDescriptorAtPoint;
    if (nHitPageIndex >= 0)
        pDescriptorAtPoint = mrModel.GetPageDescriptor(nHitPageIndex);

    return pDescriptorAtPoint;
}




PageSelector& SlideSorterController::GetPageSelector (void)
{
    OSL_ASSERT(mpPageSelector.get()!=NULL);
    return *mpPageSelector.get();
}




FocusManager& SlideSorterController::GetFocusManager (void)
{
    OSL_ASSERT(mpFocusManager.get()!=NULL);
    return *mpFocusManager.get();
}




Clipboard& SlideSorterController::GetClipboard (void)
{
    OSL_ASSERT(mpClipboard.get()!=NULL);
    return *mpClipboard.get();
}




ScrollBarManager& SlideSorterController::GetScrollBarManager (void)
{
    OSL_ASSERT(mpScrollBarManager.get()!=NULL);
    return *mpScrollBarManager.get();
}




::boost::shared_ptr<CurrentSlideManager> SlideSorterController::GetCurrentSlideManager (void) const
{
    OSL_ASSERT(mpCurrentSlideManager.get()!=NULL);
    return mpCurrentSlideManager;
}




::boost::shared_ptr<SlotManager> SlideSorterController::GetSlotManager (void) const
{
    OSL_ASSERT(mpSlotManager.get()!=NULL);
    return mpSlotManager;
}




::boost::shared_ptr<SelectionManager> SlideSorterController::GetSelectionManager (void) const
{
    OSL_ASSERT(mpSelectionManager.get()!=NULL);
    return mpSelectionManager;
}




void SlideSorterController::Paint (
    const Rectangle& rBBox,
    ::Window* pWindow)
{
    //    if (mnPaintEntranceCount == 0)
    {
        ++mnPaintEntranceCount;

        try
        {
            if (GetSelectionManager()->IsMakeSelectionVisiblePending())
                GetSelectionManager()->MakeSelectionVisible();

            mrView.SetApplicationDocumentColor(GetProperties()->GetBackgroundColor());
            mrView.CompleteRedraw(pWindow, Region(rBBox), 0, 0);
        }
        catch (const Exception&)
        {
            // Ignore all exceptions.
        }

        --mnPaintEntranceCount;
    }
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

    ViewShell* pViewShell = mrSlideSorter.GetViewShell();

    switch (rEvent.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            SdPage* pPage = NULL;
            USHORT nPopupId;

            model::PageEnumeration aSelectedPages (
                PageEnumerationProvider::CreateSelectedPagesEnumeration(mrModel));
            if (aSelectedPages.HasMoreElements())
                pPage = aSelectedPages.GetNextElement()->GetPage();

            // Choose the popup menu depending on a) the type of the main
            // view shell, b) the edit mode, and c) on whether the selection
            // is empty or not.
            ViewShell::ShellType eMainViewShellType (ViewShell::ST_NONE);
            ::boost::shared_ptr<ViewShell> pMainViewShell (
                pViewShell->GetViewShellBase().GetMainViewShell());
            if (pMainViewShell.get() != NULL)
                eMainViewShellType = pMainViewShell->GetShellType();
            switch (eMainViewShellType)
            {
                case ViewShell::ST_DRAW:
                    if (pPage != NULL)
                        nPopupId = RID_SLIDE_SORTER_DRAW_SEL_POPUP;
                    else
                        nPopupId = RID_SLIDE_SORTER_DRAW_NOSEL_POPUP;
                    break;

                default:
                    if (mrModel.GetEditMode() == EM_PAGE)
                        if (pPage != NULL)
                            nPopupId = RID_SLIDE_SORTER_IMPRESS_SEL_POPUP;
                        else
                            nPopupId = RID_SLIDE_SORTER_IMPRESS_NOSEL_POPUP;
                    else
                        if (pPage != NULL)
                            nPopupId = RID_SLIDE_SORTER_MASTER_SEL_POPUP;
                        else
                            nPopupId = RID_SLIDE_SORTER_MASTER_NOSEL_POPUP;
            }

            if (pPage == NULL)
            {
                // When there is no selection, then we show the insertion
                // indicator so that the user knows where a page insertion
                // would take place.
                mrView.GetOverlay().GetInsertionIndicatorOverlay()
                    .SetPosition(
                        pWindow->PixelToLogic(rEvent.GetMousePosPixel()));
                mrView.GetOverlay().GetInsertionIndicatorOverlay().Show();
            }

            pWindow->ReleaseMouse();
            if (rEvent.IsMouseEvent())
            {
                mbIsContextMenuOpen = true;
                if (pViewShell != NULL)
                    pViewShell->GetDispatcher()->ExecutePopup(SdResId(nPopupId));
            }
            else
            {
                // The event is not a mouse event.  Use the center of the
                // focused page as top left position of the context menu.
                if (pPage != NULL)
                {
                    model::SharedPageDescriptor pDescriptor (
                        GetFocusManager().GetFocusedPageDescriptor());
                    if (pDescriptor.get() != NULL)
                    {
                        Rectangle aBBox (mrView.GetPageBoundingBox (
                            pDescriptor,
                            view::SlideSorterView::CS_SCREEN,
                            view::SlideSorterView::BBT_SHAPE));
                        Point aPosition (aBBox.Center());
                        mbIsContextMenuOpen = true;
                        if (pViewShell != NULL)
                            pViewShell->GetViewFrame()->GetDispatcher()->ExecutePopup(
                                SdResId(nPopupId),
                                pWindow,
                                &aPosition);
                    }
                }
            }
            mbIsContextMenuOpen = false;
            if (pPage == NULL)
                mrView.GetOverlay().GetInsertionIndicatorOverlay().Hide();
            bEventHasBeenHandled = true;
        }
        break;

        case COMMAND_WHEEL:
        {
            // We ignore zooming with control+mouse wheel.
            const CommandWheelData* pData = rEvent.GetWheelData();
            if (pData!=NULL && pData->IsMod1())
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
    if (mrSlideSorter.GetViewShell() != NULL)
        mrSlideSorter.GetViewShell()->Broadcast(
            ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));

    mpPageSelector->PrepareModelChange();
    GetCurrentSlideManager()->PrepareModelChange();

    // The highlight object will be destroyed in
    // SlideSorterView::PreModelChange() along with the page objects.
    mpHighlightObject = NULL;

    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
    if (pWindow != NULL)
        mrView.PreModelChange();

    if (mpHighlightObject != NULL)
    {
        SdrPage* pPage = mpHighlightObject->GetPage();
        if (pPage != NULL)
        {
            pPage->RemoveObject(mpHighlightObject->GetOrdNum());
            delete mpHighlightObject;
        }
        mpHighlightObject = NULL;
    }
    mbPostModelChangePending = true;
}




void SlideSorterController::PostModelChange (const bool bSkipModelResync)
{
    if ( ! bSkipModelResync)
        mrModel.Resync();

    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
    if (pWindow != NULL)
    {
        GetCurrentSlideManager()->HandleModelChange();

        mrView.PostModelChange ();

        mpHighlightObject = new HighlightObject(mrSlideSorter);
        if (mpHighlightObject != NULL)
        {
            mrView.AddSdrObject(*mpHighlightObject);
            // Move to below all other objects in paint order.
            SdrPage* pPage = mpHighlightObject->GetPage();
            if (pPage != NULL)
            {
                pPage->SetObjectOrdNum(mpHighlightObject->GetOrdNum(), 0);
            }
        }

        pWindow->SetViewOrigin (Point (0,0));
        pWindow->SetViewSize (mrView.GetModelArea().GetSize());

        // The visibility of the scroll bars may have to be changed.  Then
        // the size of the view has to change, too.  Let Rearrange() handle
        // that.
        Rearrange();
    }

    mpPageSelector->HandleModelChange ();

    mbPostModelChangePending = false;
    if (mrSlideSorter.GetViewShell() != NULL)
        mrSlideSorter.GetViewShell()->Broadcast(
            ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
}




void SlideSorterController::HandleModelChange (void)
{
    // Ignore this call when the document is not in a valid state, i.e. has
    // not the same number of regular and notes pages.
    bool bIsDocumentValid = (mrModel.GetDocument()->GetPageCount() % 2 == 1);


    if (bIsDocumentValid)
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
}




IMPL_LINK(SlideSorterController, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        ::Window* pWindow = pEvent->GetWindow();
        ::sd::Window* pActiveWindow = mrSlideSorter.GetActiveWindow();
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_ACTIVATE:
            case VCLEVENT_WINDOW_SHOW:
                if (pActiveWindow != NULL && pWindow == pActiveWindow->GetParent())
                    mrView.RequestRepaint();
                break;

            case VCLEVENT_WINDOW_GETFOCUS:
                // Show focus but only when the focus was not set to the
                // window as a result of a mouse click.
                if (pActiveWindow != NULL && pWindow == pActiveWindow)
                    if (pWindow->GetPointerState().mnState==0)
                        GetFocusManager().ShowFocus();
                break;

            case VCLEVENT_WINDOW_LOSEFOCUS:
                if (pActiveWindow != NULL && pWindow == pActiveWindow)
                    GetFocusManager().HideFocus();
                break;

            case VCLEVENT_APPLICATION_DATACHANGED:
            {
                // Invalidate the preview cache.
                cache::PageCacheManager::Instance()->InvalidateAllCaches();

                // Update the draw mode.
                ULONG nDrawMode (Application::GetSettings().GetStyleSettings().GetHighContrastMode()
                    ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
                    : ViewShell::OUTPUT_DRAWMODE_COLOR);
                if (mrSlideSorter.GetViewShell() != NULL)
                    mrSlideSorter.GetViewShell()->GetFrameView()->SetDrawMode(nDrawMode);
                if (pActiveWindow != NULL)
                    pActiveWindow->SetDrawMode(nDrawMode);
                mrView.HandleDrawModeChange();

                // When the system font has changed a layout has to be done.
                mrView.Resize();
                FontProvider::Instance().Invalidate();
            }
            break;

            default:
                break;
        }
    }

    return TRUE;
}




void SlideSorterController::GetCtrlState (SfxItemSet& rSet)
{
    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // "Letzte Version" vom SFx en/disablen lassen
        SfxViewFrame* pSlideViewFrame = SfxViewFrame::Current();
        DBG_ASSERT(pSlideViewFrame!=NULL,
            "SlideSorterController::GetCtrlState: ViewFrame not found");
        if (pSlideViewFrame && pSlideViewFrame->ISA(SfxTopViewFrame))
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
        ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
        if (pWindow != NULL)
        {
            ULONG nMode = pWindow->GetDrawMode();
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




void SlideSorterController::ExecStatusBar (SfxRequest& )
{
}




void SlideSorterController::UpdateAllPages (void)
{
    // Do a redraw.
    mrView.InvalidateAllWin();
}




Rectangle SlideSorterController::Resize (const Rectangle& rAvailableSpace)
{
    Rectangle aContentArea (rAvailableSpace);

    if (maTotalWindowArea != rAvailableSpace)
    {
        maTotalWindowArea = rAvailableSpace;
        aContentArea = Rearrange(true);
    }

    return aContentArea;
}




Rectangle  SlideSorterController::Rearrange (bool bForce)
{
    Rectangle aNewContentArea (maTotalWindowArea);

    ::boost::shared_ptr<sd::Window> pWindow = mrSlideSorter.GetContentWindow();
    if (pWindow.get() != NULL)
    {
        // Place the scroll bars.
        aNewContentArea = GetScrollBarManager().PlaceScrollBars(maTotalWindowArea);

        bool bSizeHasChanged (false);
        // Only when bForce is not true we have to test for a size change in
        // order to determine whether the window and the view have to be resized.
        if ( ! bForce)
        {
            Rectangle aCurrentContentArea (
                pWindow->GetPosPixel(),
                pWindow->GetOutputSizePixel());
            bSizeHasChanged = (aNewContentArea != aCurrentContentArea);
        }
        if (bForce || bSizeHasChanged)
        {
            // The browser window gets the remaining space.
            pWindow->SetPosSizePixel (aNewContentArea.TopLeft(), aNewContentArea.GetSize());
            mrView.Resize();
        }

        // Adapt the scroll bars to the new zoom factor of the browser
        // window and the arrangement of the page objects.
        GetScrollBarManager().UpdateScrollBars(false, !bForce);
    }

    return aNewContentArea;
}




void SlideSorterController::SetZoom (long int nZoom)
{
    ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
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
        // Apply rounding, so that a current zoom factor of 1 is still
        // increased.
        nMaxFactor = (nCurrentZoom * 18 + 5) / 10;
        nZoom = Min(nMaxFactor, nZoom);
    }
    if (nZoom < 1)
        nZoom = 1;

    mrView.LockRedraw (TRUE);
    mrView.GetLayouter().SetZoom(nZoom/100.0, pWindow);
    mrView.Layout();
    GetScrollBarManager().UpdateScrollBars (false);
    mrView.GetPreviewCache()->InvalidateCache();
    mrView.RequestRepaint();
    mrView.LockRedraw (FALSE);

    /*
        ViewShell::SetZoom (nZoom);
        GetViewFrame()->GetBindings().Invalidate (SID_ATTR_ZOOM);
    */
}




FunctionReference SlideSorterController::CreateSelectionFunction (SfxRequest& rRequest)
{
    FunctionReference xFunc( SelectionFunction::Create(mrSlideSorter, rRequest) );
    return xFunc;
}




void SlideSorterController::PrepareEditModeChange (void)
{
    //  Before we throw away the page descriptors we prepare for selecting
    //  descriptors in the other mode and for restoring the current
    //  selection when switching back to the current mode.
    if (mrModel.GetEditMode() == EM_PAGE)
    {
        maSelectionBeforeSwitch.clear();

        // Search for the first selected page and determine the master page
        // used by its page object.  It will be selected after the switch.
        // In the same loop the current selection is stored.
        PageEnumeration aSelectedPages (
            PageEnumerationProvider::CreateSelectedPagesEnumeration(mrModel));
        while (aSelectedPages.HasMoreElements())
        {
            SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
            SdPage* pPage = pDescriptor->GetPage();
            // Remember the master page of the first selected descriptor.
            if (pPage!=NULL && mpEditModeChangeMasterPage==NULL)
                mpEditModeChangeMasterPage = &static_cast<SdPage&>(
                    pPage->TRG_GetMasterPage());

            maSelectionBeforeSwitch.push_back(pPage);
        }

        // Remember the current page.
        if (mrSlideSorter.GetViewShell() != NULL)
            mnCurrentPageBeforeSwitch = (mrSlideSorter.GetViewShell()->GetViewShellBase()
            .GetMainViewShell()->GetActualPage()->GetPageNum()-1)/2;
    }
}




bool SlideSorterController::ChangeEditMode (EditMode eEditMode)
{
    ModelChangeLock aLock (*this);

    bool bResult (false);
    if (mrModel.GetEditMode() != eEditMode)
    {
        // Do the actual edit mode switching.
        PreModelChange();
        bResult = mrModel.SetEditMode(eEditMode);
        if (bResult)
            HandleModelChange();
    }
    return bResult;
}




void SlideSorterController::FinishEditModeChange (void)
{
    if (mrModel.GetEditMode() == EM_MASTERPAGE)
    {
        // Search for the master page that was determined in
        // PrepareEditModeChange() and make it the current page.
        PageEnumeration aAllPages (PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
        while (aAllPages.HasMoreElements())
        {
            SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
            if (pDescriptor->GetPage() == mpEditModeChangeMasterPage)
            {
                GetCurrentSlideManager()->SwitchCurrentSlide(pDescriptor);
                break;
            }
        }
    }
    else
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnCurrentPageBeforeSwitch));
        GetCurrentSlideManager()->SwitchCurrentSlide(pDescriptor);

        // Restore the selection.
        ::std::vector<SdPage*>::iterator iPage;
        for (iPage=maSelectionBeforeSwitch.begin();
             iPage!=maSelectionBeforeSwitch.end();
             ++iPage)
        {
            mpPageSelector->SelectPage(*iPage);
        }
        maSelectionBeforeSwitch.clear( );
    }
    mpEditModeChangeMasterPage = NULL;
}




void SlideSorterController::PageNameHasChanged (int nPageIndex, const String& rsOldName)
{
    // Request a repaint for the page object whose name has changed.
    model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        mrView.RequestRepaint(pDescriptor);

    // Get a pointer to the corresponding accessible object and notify
    // that of the name change.
    do
    {
        ::sd::Window* pWindow = mrSlideSorter.GetActiveWindow();
        if (pWindow == NULL)
            break;

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
            xAccessible (pWindow->GetAccessible(FALSE));
        if ( ! xAccessible.is())
            break;

        // Now comes a small hack.  We assume that the accessible object is
        // an instantiation of AccessibleSlideSorterView and cast it to that
        // class.  The cleaner alternative to this cast would be a new member
        // in which we would store the last AccessibleSlideSorterView object
        // created by SlideSorterViewShell::CreateAccessibleDocumentView().
        // But then there is no guaranty that the accessible object obtained
        // from the window really is that instance last created by
        // CreateAccessibleDocumentView().
        // However, the dynamic cast together with the check of the result
        // being NULL should be safe enough.
        ::accessibility::AccessibleSlideSorterView* pAccessibleView
              = dynamic_cast< ::accessibility::AccessibleSlideSorterView*>(xAccessible.get());
        if (pAccessibleView == NULL)
            break;

        ::accessibility::AccessibleSlideSorterObject* pChild
              = pAccessibleView->GetAccessibleChildImplementation(nPageIndex);
        if (pChild == NULL || pChild->GetPage() == NULL)
            break;

        ::rtl::OUString sOldName (rsOldName);
        ::rtl::OUString sNewName (pChild->GetPage()->GetName());
        pChild->FireAccessibleEvent(
            ::com::sun::star::accessibility::AccessibleEventId::NAME_CHANGED,
            makeAny(sOldName),
            makeAny(sNewName));
    }
    while (false);
}




bool SlideSorterController::IsContextMenuOpen (void) const
{
    return mbIsContextMenuOpen;
}




::boost::shared_ptr<Properties> SlideSorterController::GetProperties (void) const
{
    return mpProperties;
}




void SlideSorterController::SetDocumentSlides (const Reference<container::XIndexAccess>& rxSlides)
{
    if (mrModel.GetDocumentSlides() != rxSlides)
    {
        PreModelChange();
        mrModel.SetDocumentSlides(rxSlides);
        mrView.Layout();
        PostModelChange(false);
    }
}




::boost::shared_ptr<Animator> SlideSorterController::GetAnimator (void) const
{
    return mpAnimator;
}




view::HighlightObject* SlideSorterController::GetHighlightObject (void) const
{
    if (GetProperties()->IsHighlightCurrentSlide())
        return mpHighlightObject;
    else
        return NULL;
}


//===== SlideSorterController::ModelChangeLock ================================

SlideSorterController::ModelChangeLock::ModelChangeLock (
    SlideSorterController& rController)
    : mpController(&rController)
{
    mpController->LockModelChange();
}




SlideSorterController::ModelChangeLock::~ModelChangeLock (void)
{
    Release();
}




void SlideSorterController::ModelChangeLock::Release (void)
{
    if (mpController != NULL)
    {
        mpController->UnlockModelChange();
        mpController = NULL;
    }
}

} } } // end of namespace ::sd::slidesorter
