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


#include "controller/SlideSorterController.hxx"

#include "SlideSorter.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "SlsListener.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsAnimator.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsSlotManager.hxx"
#include "controller/SlsTransferable.hxx"
#include "controller/SlsVisibleAreaManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsFontProvider.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsPageObjectPainter.hxx"
#include "view/SlsTheme.hxx"
#include "view/SlsToolTip.hxx"
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
#include "ViewShellHint.hxx"
#include "AccessibleSlideSorterView.hxx"
#include "AccessibleSlideSorterObject.hxx"

#include <vcl/window.hxx>
#include <svx/svdopage.hxx>
#include <svx/svxids.hrc>
#include <svx/ruler.hxx>
#include <sfx2/zoomitem.hxx>
#include <svtools/tabbar.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
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
using namespace ::basegfx;

namespace sd { namespace slidesorter { namespace controller {


SlideSorterController::SlideSorterController (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mrModel(mrSlideSorter.GetModel()),
      mrView(mrSlideSorter.GetView()),
      mpPageSelector(),
      mpFocusManager(),
      mpSlotManager(),
      mpScrollBarManager(),
      mpCurrentSlideManager(),
      mpSelectionManager(),
      mpClipboard(),
      mpInsertionIndicatorHandler(new InsertionIndicatorHandler(rSlideSorter)),
      mpAnimator(new Animator(rSlideSorter)),
      mpVisibleAreaManager(new VisibleAreaManager(rSlideSorter)),
      mpListener(),
      mnModelChangeLockCount(0),
      mbIsForcedRearrangePending(false),
      mbPreModelChangeDone(false),
      mbPostModelChangePending(false),
      maSelectionBeforeSwitch(),
      mnCurrentPageBeforeSwitch(0),
      mpEditModeChangeMasterPage(NULL),
      maTotalWindowArea(),
      mnPaintEntranceCount(0),
      mbIsContextMenuOpen(false)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    OSL_ASSERT(pWindow);
    if (pWindow)
    {
        // The whole background is painted by the view and controls.
        ::Window* pParentWindow = pWindow->GetParent();
        OSL_ASSERT(pParentWindow!=NULL);
        pParentWindow->SetBackground (Wallpaper());

        // Connect the view with the window that has been created by our base
        // class.
        pWindow->SetBackground(Wallpaper());
        pWindow->SetCenterAllowed(false);
        pWindow->SetMapMode(MapMode(MAP_PIXEL));
        pWindow->SetViewSize(mrView.GetModelArea().GetSize());
    }
}




void SlideSorterController::Init (void)
{
    mpCurrentSlideManager.reset(new CurrentSlideManager(mrSlideSorter));
    mpPageSelector.reset(new PageSelector(mrSlideSorter));
    mpFocusManager.reset(new FocusManager(mrSlideSorter));
    mpSlotManager.reset(new SlotManager(mrSlideSorter));
    mpScrollBarManager.reset(new ScrollBarManager(mrSlideSorter));
    mpSelectionManager.reset(new SelectionManager(mrSlideSorter));
    mpClipboard.reset(new Clipboard(mrSlideSorter));

    mpScrollBarManager->LateInitialization();

    // Create the selection function.
    SfxRequest aRequest (
        SID_OBJECT_SELECT,
        0,
        mrModel.GetDocument()->GetItemPool());
    mrSlideSorter.SetCurrentFunction(CreateSelectionFunction(aRequest));

    mpListener = new Listener(mrSlideSorter);

    mpPageSelector->GetCoreSelection();
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
    catch( uno::Exception& )
    {
        OSL_FAIL( "sd::SlideSorterController::~SlideSorterController(), exception caught!" );
    }

    // dispose should have been called by now so that nothing is to be done
    // to shut down cleanly.
}




void SlideSorterController::Dispose (void)
{
    mpInsertionIndicatorHandler->End(Animator::AM_Immediate);
    mpClipboard.reset();
    mpSelectionManager.reset();
    mpAnimator->Dispose();
}




model::SharedPageDescriptor SlideSorterController::GetPageAt (
    const Point& aWindowPosition)
{
    sal_Int32 nHitPageIndex (mrView.GetPageIndexAtPoint(aWindowPosition));
    model::SharedPageDescriptor pDescriptorAtPoint;
    if (nHitPageIndex >= 0)
    {
        pDescriptorAtPoint = mrModel.GetPageDescriptor(nHitPageIndex);

        // Depending on a property we may have to check that the mouse is no
        // just over the page object but over the preview area.
        if (pDescriptorAtPoint
            && mrSlideSorter.GetProperties()->IsOnlyPreviewTriggersMouseOver()
            && ! pDescriptorAtPoint->HasState(PageDescriptor::ST_Selected))
        {
            // Make sure that the mouse is over the preview area.
            if ( ! mrView.GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
                pDescriptorAtPoint,
                view::PageObjectLayouter::Preview,
                view::PageObjectLayouter::WindowCoordinateSystem).IsInside(aWindowPosition))
            {
                pDescriptorAtPoint.reset();
            }
        }
    }

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




::boost::shared_ptr<InsertionIndicatorHandler>
    SlideSorterController::GetInsertionIndicatorHandler (void) const
{
    OSL_ASSERT(mpInsertionIndicatorHandler.get()!=NULL);
    return mpInsertionIndicatorHandler;
}




void SlideSorterController::Paint (
    const Rectangle& rBBox,
    ::Window* pWindow)
{
    if (mnPaintEntranceCount == 0)
    {
        ++mnPaintEntranceCount;

        try
        {
            mrView.CompleteRedraw(pWindow, Region(rBBox), 0);
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

    if (pWindow == NULL)
        return false;

    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell == NULL)
        return false;

    switch (rEvent.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            SdPage* pPage = NULL;
            sal_uInt16 nPopupId;

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
            ::boost::scoped_ptr<InsertionIndicatorHandler::ForceShowContext> pContext;
            if (pPage == NULL)
            {
                // When there is no selection, then we show the insertion
                // indicator so that the user knows where a page insertion
                // would take place.
                mpInsertionIndicatorHandler->Start(false);
                mpInsertionIndicatorHandler->UpdateIndicatorIcon(
                    dynamic_cast<Transferable*>(SD_MOD()->pTransferClip));
                mpInsertionIndicatorHandler->UpdatePosition(
                    pWindow->PixelToLogic(rEvent.GetMousePosPixel()),
                    InsertionIndicatorHandler::MoveMode);
                pContext.reset(new InsertionIndicatorHandler::ForceShowContext(
                    mpInsertionIndicatorHandler));
            }

            pWindow->ReleaseMouse();

            Point aMenuLocation (0,0);
            if (rEvent.IsMouseEvent())
            {
                // We have to explicitly specify the location of the menu
                // when the slide sorter is placed in an undocked child
                // menu.  But when it is docked it does not hurt, so we
                // specify the location always.
                aMenuLocation = rEvent.GetMousePosPixel();
            }
            else
            {
                // The event is not a mouse event.  Use the center of the
                // focused page as top left position of the context menu.
                model::SharedPageDescriptor pDescriptor (
                    GetFocusManager().GetFocusedPageDescriptor());
                if (pDescriptor.get() != NULL)
                {
                    Rectangle aBBox (
                        mrView.GetLayouter().GetPageObjectLayouter()->GetBoundingBox (
                            pDescriptor,
                            PageObjectLayouter::PageObject,
                            PageObjectLayouter::ModelCoordinateSystem));
                    aMenuLocation = aBBox.Center();
                }
            }

            mbIsContextMenuOpen = true;
            if (pViewShell != NULL)
            {
                SfxDispatcher* pDispatcher = pViewShell->GetDispatcher();
                if (pDispatcher != NULL)
                {
                    pDispatcher->ExecutePopup(
                        SdResId(nPopupId),
                        pWindow,
                        &aMenuLocation);
                    mrSlideSorter.GetView().UpdatePageUnderMouse();
                    ::rtl::Reference<SelectionFunction> pFunction(GetCurrentSelectionFunction());
                    if (pFunction.is())
                        pFunction->ResetMouseAnchor();
                }
            }
            mbIsContextMenuOpen = false;
            if (pPage == NULL)
            {
                // Remember the position of the insertion indicator before
                // it is hidden, so that a pending slide insertion slot call
                // finds the right place to insert a new slide.
                GetSelectionManager()->SetInsertionPosition(
                    GetInsertionIndicatorHandler()->GetInsertionPageIndex());
            }
            pContext.reset();
            bEventHasBeenHandled = true;
        }
        break;

        case COMMAND_WHEEL:
        {
            const CommandWheelData* pData = rEvent.GetWheelData();
            if (pData == NULL)
                return false;
            if (pData->IsMod1())
            {
                // We do not support zooming with control+mouse wheel.
                return false;
            }
            // Determine whether to scroll horizontally or vertically.  This
            // depends on the orientation of the scroll bar and the
            // IsHoriz() flag of the event.
            if ((mrSlideSorter.GetView().GetOrientation()==view::Layouter::HORIZONTAL)
                == pData->IsHorz())
            {
                GetScrollBarManager().Scroll(
                    ScrollBarManager::Orientation_Vertical,
                    ScrollBarManager::Unit_Slide,
                    -pData->GetNotchDelta());
            }
            else
            {
                GetScrollBarManager().Scroll(
                    ScrollBarManager::Orientation_Horizontal,
                    ScrollBarManager::Unit_Slide,
                    -pData->GetNotchDelta());
            }
            mrSlideSorter.GetView().UpdatePageUnderMouse(rEvent.GetMousePosPixel());

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
    {
        PostModelChange();
    }
}




void SlideSorterController::PreModelChange (void)
{
    // Prevent PreModelChange to execute more than once per model lock.
    if (mbPostModelChangePending)
        return;
    mbPreModelChangeDone = true;

    if (mrSlideSorter.GetViewShell() != NULL)
        mrSlideSorter.GetViewShell()->Broadcast(
            ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));

    GetCurrentSlideManager()->PrepareModelChange();

    if (mrSlideSorter.GetContentWindow())
        mrView.PreModelChange();

    mbPostModelChangePending = true;
}




void SlideSorterController::PostModelChange (void)
{
    mbPostModelChangePending = false;
    mrModel.Resync();

    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        GetCurrentSlideManager()->HandleModelChange();

        mrView.PostModelChange ();

        pWindow->SetViewOrigin (Point (0,0));
        pWindow->SetViewSize (mrView.GetModelArea().GetSize());

        // The visibility of the scroll bars may have to be changed.  Then
        // the size of the view has to change, too.  Let Rearrange() handle
        // that.
        Rearrange(mbIsForcedRearrangePending);
    }

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
        ModelChangeLock aLock (*this);
        PreModelChange();
    }
}




IMPL_LINK(SlideSorterController, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        ::Window* pWindow = pEvent->GetWindow();
        SharedSdWindow pActiveWindow (mrSlideSorter.GetContentWindow());
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_ACTIVATE:
            case VCLEVENT_WINDOW_SHOW:
                if (pActiveWindow && pWindow == pActiveWindow->GetParent())
                    mrView.RequestRepaint();
                break;

            case VCLEVENT_WINDOW_HIDE:
                if (pActiveWindow && pWindow == pActiveWindow->GetParent())
                    mrView.SetPageUnderMouse(SharedPageDescriptor());
                break;

            case VCLEVENT_WINDOW_GETFOCUS:
                if (pActiveWindow)
                    if (pWindow == pActiveWindow.get())
                        GetFocusManager().ShowFocus(false);
                break;

            case VCLEVENT_WINDOW_LOSEFOCUS:
                if (pActiveWindow && pWindow == pActiveWindow.get())
                {
                    GetFocusManager().HideFocus();
                    mrView.GetToolTip().Hide();

                    // Select the current slide so that it is properly
                    // visualized when the focus is moved to the edit view.
                    GetPageSelector().SelectPage(GetCurrentSlideManager()->GetCurrentSlide());
                }
                break;

            case VCLEVENT_APPLICATION_DATACHANGED:
            {
                // Invalidate the preview cache.
                cache::PageCacheManager::Instance()->InvalidateAllCaches();

                // Update the draw mode.
                sal_uLong nDrawMode (Application::GetSettings().GetStyleSettings().GetHighContrastMode()
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

                // Update theme colors.
                mrSlideSorter.GetProperties()->HandleDataChangeEvent();
                mrSlideSorter.GetTheme()->Update(mrSlideSorter.GetProperties());
                mrView.HandleDataChangeEvent();
            }
            break;

            default:
                break;
        }
    }

    return sal_True;
}




void SlideSorterController::GetCtrlState (SfxItemSet& rSet)
{
    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // "Letzte Version" vom SFx en/disablen lassen
        SfxViewFrame* pSlideViewFrame = SfxViewFrame::Current();
        DBG_ASSERT(pSlideViewFrame!=NULL,
            "SlideSorterController::GetCtrlState: ViewFrame not found");
        if (pSlideViewFrame)
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
        if (mrSlideSorter.GetContentWindow())
        {
            sal_uLong nMode = mrSlideSorter.GetContentWindow()->GetDrawMode();
            sal_uInt16 nQuality = 0;

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
                    (sal_Bool)(nQuality==0)));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_GRAYSCALE,
                    (sal_Bool)(nQuality==1)));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_BLACKWHITE,
                    (sal_Bool)(nQuality==2)));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_CONTRAST,
                    (sal_Bool)(nQuality==3)));
        }
    }

    if (rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) == SFX_ITEM_AVAILABLE)
    {
        rSet.Put (SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, sal_True));
    }
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
    mrSlideSorter.GetContentWindow()->Invalidate();
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

    if (aNewContentArea.IsEmpty())
        return aNewContentArea;

    if (mnModelChangeLockCount>0)
    {
        mbIsForcedRearrangePending |= bForce;
        return aNewContentArea;
    }
    else
        mbIsForcedRearrangePending = false;

    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        if (bForce)
            mrView.UpdateOrientation();

        // Place the scroll bars.
        aNewContentArea = GetScrollBarManager().PlaceScrollBars(
            maTotalWindowArea,
            mrView.GetOrientation() != view::Layouter::VERTICAL,
            mrView.GetOrientation() != view::Layouter::HORIZONTAL);

        bool bSizeHasChanged (false);
        // Only when bForce is not true we have to test for a size change in
        // order to determine whether the window and the view have to be resized.
        if ( ! bForce)
        {
            Rectangle aCurrentContentArea (pWindow->GetPosPixel(), pWindow->GetOutputSizePixel());
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

        // Keep the current slide in the visible area.
        GetVisibleAreaManager().RequestCurrentSlideVisible();

        mrView.RequestRepaint();
    }

    return aNewContentArea;
}




FunctionReference SlideSorterController::CreateSelectionFunction (SfxRequest& rRequest)
{
    FunctionReference xFunc( SelectionFunction::Create(mrSlideSorter, rRequest) );
    return xFunc;
}




::rtl::Reference<SelectionFunction> SlideSorterController::GetCurrentSelectionFunction (void)
{
    FunctionReference pFunction (mrSlideSorter.GetViewShell()->GetCurrentFunction());
    return ::rtl::Reference<SelectionFunction>(dynamic_cast<SelectionFunction*>(pFunction.get()));
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
    bool bResult (false);
    if (mrModel.GetEditMode() != eEditMode)
    {
        ModelChangeLock aLock (*this);
        PreModelChange();
        // Do the actual edit mode switching.
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
        SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
        if ( ! pWindow)
            return;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        xAccessible (pWindow->GetAccessible(sal_False));
    if ( ! xAccessible.is())
        return;

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
        return;

    ::accessibility::AccessibleSlideSorterObject* pChild
            = pAccessibleView->GetAccessibleChildImplementation(nPageIndex);
    if (pChild == NULL || pChild->GetPage() == NULL)
        return;

    ::rtl::OUString sOldName (rsOldName);
    ::rtl::OUString sNewName (pChild->GetPage()->GetName());
    pChild->FireAccessibleEvent(
        ::com::sun::star::accessibility::AccessibleEventId::NAME_CHANGED,
        makeAny(sOldName),
        makeAny(sNewName));
}




void SlideSorterController::SetDocumentSlides (const Reference<container::XIndexAccess>& rxSlides)
{
    if (mrModel.GetDocumentSlides() != rxSlides)
    {
        ModelChangeLock aLock (*this);
        PreModelChange();

        mrModel.SetDocumentSlides(rxSlides);
        mrView.Layout();

        // Select just the current slide.
        PageSelector::BroadcastLock aBroadcastLock (*mpPageSelector);
        mpPageSelector->DeselectAllPages();
        mpPageSelector->SelectPage(mpCurrentSlideManager->GetCurrentSlide());
    }
}




::boost::shared_ptr<Animator> SlideSorterController::GetAnimator (void) const
{
    return mpAnimator;
}




VisibleAreaManager& SlideSorterController::GetVisibleAreaManager (void) const
{
    OSL_ASSERT(mpVisibleAreaManager);
    return *mpVisibleAreaManager;
}




void SlideSorterController::CheckForMasterPageAssignment (void)
{
    if (mrModel.GetPageCount()%2==0)
        return;
    PageEnumeration aAllPages (PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aAllPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        if (pDescriptor->UpdateMasterPage())
        {
            mrView.GetPreviewCache()->InvalidatePreviewBitmap (
                pDescriptor->GetPage(),
                true);
        }
    }
}




void SlideSorterController::CheckForSlideTransitionAssignment (void)
{
    if (mrModel.GetPageCount()%2==0)
        return;
    PageEnumeration aAllPages (PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aAllPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        if (pDescriptor->UpdateTransitionFlag())
        {
            mrView.GetPreviewCache()->InvalidatePreviewBitmap (
                pDescriptor->GetPage(),
                true);
        }
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
