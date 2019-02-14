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

#include <controller/SlideSorterController.hxx>

#include <SlideSorter.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsSelectionFunction.hxx>
#include <controller/SlsProperties.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include "SlsListener.hxx"
#include <controller/SlsFocusManager.hxx>
#include <controller/SlsAnimator.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsInsertionIndicatorHandler.hxx>
#include <controller/SlsScrollBarManager.hxx>
#include <controller/SlsSelectionManager.hxx>
#include <controller/SlsSlotManager.hxx>
#include <controller/SlsTransferableData.hxx>
#include <controller/SlsVisibleAreaManager.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <view/SlsFontProvider.hxx>
#include <view/SlsPageObjectLayouter.hxx>
#include <view/SlsPageObjectPainter.hxx>
#include <view/SlsTheme.hxx>
#include <view/SlsToolTip.hxx>
#include <cache/SlsPageCache.hxx>
#include <cache/SlsPageCacheManager.hxx>

#include <drawdoc.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellBase.hxx>
#include <Window.hxx>
#include <FrameView.hxx>
#include <DrawDocShell.hxx>
#include <sdpage.hxx>

#include <app.hrc>
#include <sdmod.hxx>
#include <sdxfer.hxx>
#include <ViewShellHint.hxx>
#include <AccessibleSlideSorterView.hxx>
#include <AccessibleSlideSorterObject.hxx>

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
#include <vcl/settings.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <memory>

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
      mbContextMenuOpen(false),
      mbPostModelChangePending(false),
      maSelectionBeforeSwitch(),
      mnCurrentPageBeforeSwitch(0),
      mpEditModeChangeMasterPage(nullptr),
      maTotalWindowArea(),
      mnPaintEntranceCount(0)
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    OSL_ASSERT(pWindow);
    if (!pWindow)
        return;

    // The whole background is painted by the view and controls.
    vcl::Window* pParentWindow = pWindow->GetParent();
    OSL_ASSERT(pParentWindow!=nullptr);
    pParentWindow->SetBackground (Wallpaper());

    // Connect the view with the window that has been created by our base
    // class.
    pWindow->SetBackground(Wallpaper());
    pWindow->SetCenterAllowed(false);
    pWindow->SetMapMode(MapMode(MapUnit::MapPixel));
    pWindow->SetViewSize(mrView.GetModelArea().GetSize());
}

void SlideSorterController::Init()
{
    mpCurrentSlideManager.reset(new CurrentSlideManager(mrSlideSorter));
    mpPageSelector.reset(new PageSelector(mrSlideSorter));
    mpFocusManager.reset(new FocusManager(mrSlideSorter));
    mpSlotManager.reset(new SlotManager(mrSlideSorter));
    mpScrollBarManager.reset(new ScrollBarManager(mrSlideSorter));
    mpSelectionManager.reset(new SelectionManager(mrSlideSorter));
    mpClipboard.reset(new Clipboard(mrSlideSorter));

    // Create the selection function.
    SfxRequest aRequest (
        SID_OBJECT_SELECT,
        SfxCallMode::SLOT,
        mrModel.GetDocument()->GetItemPool());
    mrSlideSorter.SetCurrentFunction(CreateSelectionFunction(aRequest));

    mpListener = new Listener(mrSlideSorter);

    mpPageSelector->GetCoreSelection();
    GetSelectionManager()->SelectionHasChanged();
}

SlideSorterController::~SlideSorterController()
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

void SlideSorterController::Dispose()
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
            && ! pDescriptorAtPoint->HasState(PageDescriptor::ST_Selected))
        {
            // Make sure that the mouse is over the preview area.
            if ( ! mrView.GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
                pDescriptorAtPoint,
                view::PageObjectLayouter::Part::Preview,
                view::PageObjectLayouter::WindowCoordinateSystem).IsInside(aWindowPosition))
            {
                pDescriptorAtPoint.reset();
            }
        }
    }

    return pDescriptorAtPoint;
}

PageSelector& SlideSorterController::GetPageSelector()
{
    OSL_ASSERT(mpPageSelector != nullptr);
    return *mpPageSelector;
}

FocusManager& SlideSorterController::GetFocusManager()
{
    OSL_ASSERT(mpFocusManager != nullptr);
    return *mpFocusManager;
}

Clipboard& SlideSorterController::GetClipboard()
{
    OSL_ASSERT(mpClipboard != nullptr);
    return *mpClipboard;
}

ScrollBarManager& SlideSorterController::GetScrollBarManager()
{
    OSL_ASSERT(mpScrollBarManager != nullptr);
    return *mpScrollBarManager;
}

std::shared_ptr<CurrentSlideManager> const & SlideSorterController::GetCurrentSlideManager() const
{
    OSL_ASSERT(mpCurrentSlideManager != nullptr);
    return mpCurrentSlideManager;
}

std::shared_ptr<SlotManager> const & SlideSorterController::GetSlotManager() const
{
    OSL_ASSERT(mpSlotManager != nullptr);
    return mpSlotManager;
}

std::shared_ptr<SelectionManager> const & SlideSorterController::GetSelectionManager() const
{
    OSL_ASSERT(mpSelectionManager != nullptr);
    return mpSelectionManager;
}

std::shared_ptr<InsertionIndicatorHandler> const &
    SlideSorterController::GetInsertionIndicatorHandler() const
{
    OSL_ASSERT(mpInsertionIndicatorHandler != nullptr);
    return mpInsertionIndicatorHandler;
}

void SlideSorterController::Paint (
    const ::tools::Rectangle& rBBox,
    vcl::Window* pWindow)
{
    if (mnPaintEntranceCount != 0)
        return;

    ++mnPaintEntranceCount;

    try
    {
        mrView.CompleteRedraw(pWindow, vcl::Region(rBBox));
    }
    catch (const Exception&)
    {
        // Ignore all exceptions.
    }

    --mnPaintEntranceCount;
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

    if (pWindow == nullptr)
        return false;

    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell == nullptr)
        return false;

    switch (rEvent.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            SdPage* pPage = nullptr;
            OUString aPopupId;

            model::PageEnumeration aSelectedPages (
                PageEnumerationProvider::CreateSelectedPagesEnumeration(mrModel));
            if (aSelectedPages.HasMoreElements())
                pPage = aSelectedPages.GetNextElement()->GetPage();

            if (mrModel.GetEditMode() == EditMode::Page)
            {
                if (pPage != nullptr)
                    aPopupId = "pagepane";
                else
                    aPopupId = "pagepanenosel";
            }
            else if (pPage != nullptr)
                aPopupId = "pagepanemaster";
            else
                aPopupId = "pagepanenoselmaster";

            std::unique_ptr<InsertionIndicatorHandler::ForceShowContext> pContext;
            if (pPage == nullptr)
            {
                // When there is no selection, then we show the insertion
                // indicator so that the user knows where a page insertion
                // would take place.
                mpInsertionIndicatorHandler->Start(false);
                mpInsertionIndicatorHandler->UpdateIndicatorIcon(SD_MOD()->pTransferClip);
                mpInsertionIndicatorHandler->UpdatePosition(
                    pWindow->PixelToLogic(rEvent.GetMousePosPixel()),
                    InsertionIndicatorHandler::MoveMode);
                pContext.reset(new InsertionIndicatorHandler::ForceShowContext(
                    mpInsertionIndicatorHandler));
            }

            pWindow->ReleaseMouse();

            Point aMenuLocation (0,0);
            if (!rEvent.IsMouseEvent())
            {
                // The event is not a mouse event.  Use the center of the
                // focused page as top left position of the context menu.
                model::SharedPageDescriptor pDescriptor (
                    GetFocusManager().GetFocusedPageDescriptor());
                if (pDescriptor.get() != nullptr)
                {
                    ::tools::Rectangle aBBox (
                        mrView.GetLayouter().GetPageObjectLayouter()->GetBoundingBox (
                            pDescriptor,
                            PageObjectLayouter::Part::PageObject,
                            PageObjectLayouter::ModelCoordinateSystem));
                    aMenuLocation = aBBox.Center();
                }
            }

            if (SfxDispatcher* pDispatcher = pViewShell->GetDispatcher())
            {
                mbContextMenuOpen = true;
                if (!rEvent.IsMouseEvent())
                    pDispatcher->ExecutePopup(aPopupId, pWindow, &aMenuLocation);
                else
                    pDispatcher->ExecutePopup(aPopupId);
                mbContextMenuOpen = false;
                mrSlideSorter.GetView().UpdatePageUnderMouse();
                ::rtl::Reference<SelectionFunction> pFunction(GetCurrentSelectionFunction());
                if (pFunction.is())
                    pFunction->ResetMouseAnchor();
            }
            if (pPage == nullptr)
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

        case CommandEventId::Wheel:
        {
            const CommandWheelData* pData = rEvent.GetWheelData();
            if (pData == nullptr)
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
                    -pData->GetNotchDelta());
            }
            else
            {
                GetScrollBarManager().Scroll(
                    ScrollBarManager::Orientation_Horizontal,
                    -pData->GetNotchDelta());
            }
            mrSlideSorter.GetView().UpdatePageUnderMouse(rEvent.GetMousePosPixel());

            bEventHasBeenHandled = true;
        }
        break;

        default: break;
    }

    return bEventHasBeenHandled;
}

void SlideSorterController::LockModelChange()
{
    mnModelChangeLockCount += 1;
}

void SlideSorterController::UnlockModelChange()
{
    mnModelChangeLockCount -= 1;
    if (mnModelChangeLockCount==0 && mbPostModelChangePending)
    {
        PostModelChange();
    }
}

void SlideSorterController::PreModelChange()
{
    // Prevent PreModelChange to execute more than once per model lock.
    if (mbPostModelChangePending)
        return;

    if (mrSlideSorter.GetViewShell() != nullptr)
        mrSlideSorter.GetViewShell()->Broadcast(
            ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));

    GetCurrentSlideManager()->PrepareModelChange();

    if (mrSlideSorter.GetContentWindow())
        mrView.PreModelChange();

    mbPostModelChangePending = true;
}

void SlideSorterController::PostModelChange()
{
    mbPostModelChangePending = false;
    mrModel.Resync();

    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
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

    if (mrSlideSorter.GetViewShell() != nullptr)
        mrSlideSorter.GetViewShell()->Broadcast(
            ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
}

void SlideSorterController::HandleModelChange()
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

IMPL_LINK(SlideSorterController, ApplicationEventHandler, VclSimpleEvent&, rEvent, void)
{
    auto windowEvent = dynamic_cast<VclWindowEvent *>(&rEvent);
    if (windowEvent != nullptr) {
        WindowEventHandler(*windowEvent);
    }
}
IMPL_LINK(SlideSorterController, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
        vcl::Window* pWindow = rEvent.GetWindow();
        sd::Window *pActiveWindow (mrSlideSorter.GetContentWindow().get());
        switch (rEvent.GetId())
        {
            case VclEventId::WindowActivate:
            case VclEventId::WindowShow:
                if (pActiveWindow && pWindow == pActiveWindow->GetParent())
                    mrView.RequestRepaint();
                break;

            case VclEventId::WindowHide:
                if (pActiveWindow && pWindow == pActiveWindow->GetParent())
                    mrView.SetPageUnderMouse(SharedPageDescriptor());
                break;

            case VclEventId::WindowGetFocus:
                if (pActiveWindow)
                    if (pWindow == pActiveWindow)
                        GetFocusManager().ShowFocus(false);
                break;

            case VclEventId::WindowLoseFocus:
                if (pActiveWindow && pWindow == pActiveWindow)
                {
                    GetFocusManager().HideFocus();
                    mrView.GetToolTip().Hide();

                    //don't scroll back to the selected slide when we lose
                    //focus due to a temporary active context menu
                    if (!mbContextMenuOpen)
                    {
                        // Select the current slide so that it is properly
                        // visualized when the focus is moved to the edit view.
                        GetPageSelector().SelectPage(GetCurrentSlideManager()->GetCurrentSlide());
                    }
                }
                break;

            case VclEventId::ApplicationDataChanged:
            {
                // Invalidate the preview cache.
                cache::PageCacheManager::Instance()->InvalidateAllCaches();

                // Update the draw mode.
                DrawModeFlags nDrawMode (Application::GetSettings().GetStyleSettings().GetHighContrastMode()
                    ? sd::OUTPUT_DRAWMODE_CONTRAST
                    : sd::OUTPUT_DRAWMODE_COLOR);
                if (mrSlideSorter.GetViewShell() != nullptr)
                    mrSlideSorter.GetViewShell()->GetFrameView()->SetDrawMode(nDrawMode);
                if (pActiveWindow != nullptr)
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

void SlideSorterController::GetCtrlState (SfxItemSet& rSet)
{
    if (rSet.GetItemState(SID_RELOAD) != SfxItemState::UNKNOWN)
    {
        // let SFx en-/disable "last version"
        SfxViewFrame* pSlideViewFrame = SfxViewFrame::Current();
        DBG_ASSERT(pSlideViewFrame!=nullptr,
            "SlideSorterController::GetCtrlState: ViewFrame not found");
        if (pSlideViewFrame)
        {
            pSlideViewFrame->GetSlotState (SID_RELOAD, nullptr, &rSet);
        }
        else        // MI says: no MDIFrame --> disable
        {
            rSet.DisableItem(SID_RELOAD);
        }
    }

    // Output quality.
    if (rSet.GetItemState(SID_OUTPUT_QUALITY_COLOR)==SfxItemState::DEFAULT
        ||rSet.GetItemState(SID_OUTPUT_QUALITY_GRAYSCALE)==SfxItemState::DEFAULT
        ||rSet.GetItemState(SID_OUTPUT_QUALITY_BLACKWHITE)==SfxItemState::DEFAULT
        ||rSet.GetItemState(SID_OUTPUT_QUALITY_CONTRAST)==SfxItemState::DEFAULT)
    {
        if (mrSlideSorter.GetContentWindow())
        {
            DrawModeFlags nMode = mrSlideSorter.GetContentWindow()->GetDrawMode();
            sal_uInt16 nQuality = 0;

            if (nMode == sd::OUTPUT_DRAWMODE_COLOR) {
                nQuality = 0;
            } else if (nMode == sd::OUTPUT_DRAWMODE_GRAYSCALE) {
                nQuality = 1;
            } else if (nMode == sd::OUTPUT_DRAWMODE_BLACKWHITE) {
                nQuality = 2;
            } else if (nMode == sd::OUTPUT_DRAWMODE_CONTRAST) {
                nQuality = 3;
            }

            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_COLOR, nQuality==0));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_GRAYSCALE, nQuality==1));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_BLACKWHITE, nQuality==2));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_CONTRAST, nQuality==3));
        }
    }

    if (rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) == SfxItemState::DEFAULT)
    {
        rSet.Put (SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, true));
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

void SlideSorterController::UpdateAllPages()
{
    // Do a redraw.
    mrSlideSorter.GetContentWindow()->Invalidate();
}

void SlideSorterController::Resize (const ::tools::Rectangle& rAvailableSpace)
{
    if (maTotalWindowArea != rAvailableSpace)
    {
        maTotalWindowArea = rAvailableSpace;
        Rearrange(true);
    }
}

void  SlideSorterController::Rearrange (bool bForce)
{
    if (maTotalWindowArea.IsEmpty())
        return;

    if (mnModelChangeLockCount>0)
    {
        mbIsForcedRearrangePending |= bForce;
        return;
    }
    else
        mbIsForcedRearrangePending = false;

    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    if (!pWindow)
        return;

    if (bForce)
        mrView.UpdateOrientation();

    // Place the scroll bars.
    ::tools::Rectangle aNewContentArea = GetScrollBarManager().PlaceScrollBars(
        maTotalWindowArea,
        mrView.GetOrientation() != view::Layouter::VERTICAL,
        mrView.GetOrientation() != view::Layouter::HORIZONTAL);

    bool bSizeHasChanged (false);
    // Only when bForce is not true we have to test for a size change in
    // order to determine whether the window and the view have to be resized.
    if ( ! bForce)
    {
        ::tools::Rectangle aCurrentContentArea (pWindow->GetPosPixel(), pWindow->GetOutputSizePixel());
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
    GetScrollBarManager().UpdateScrollBars(!bForce);

    // Keep the current slide in the visible area.
    GetVisibleAreaManager().RequestCurrentSlideVisible();

    mrView.RequestRepaint();
}

rtl::Reference<FuPoor> SlideSorterController::CreateSelectionFunction (SfxRequest& rRequest)
{
    rtl::Reference<FuPoor> xFunc( SelectionFunction::Create(mrSlideSorter, rRequest) );
    return xFunc;
}

::rtl::Reference<SelectionFunction> SlideSorterController::GetCurrentSelectionFunction()
{
    rtl::Reference<FuPoor> pFunction (mrSlideSorter.GetViewShell()->GetCurrentFunction());
    return ::rtl::Reference<SelectionFunction>(dynamic_cast<SelectionFunction*>(pFunction.get()));
}

void SlideSorterController::PrepareEditModeChange()
{
    //  Before we throw away the page descriptors we prepare for selecting
    //  descriptors in the other mode and for restoring the current
    //  selection when switching back to the current mode.
    if (mrModel.GetEditMode() != EditMode::Page)
        return;

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
        if (pPage!=nullptr && mpEditModeChangeMasterPage==nullptr)
            mpEditModeChangeMasterPage = &static_cast<SdPage&>(
                pPage->TRG_GetMasterPage());

        maSelectionBeforeSwitch.push_back(pPage);
    }

    // Remember the current page.
    if (mrSlideSorter.GetViewShell() != nullptr)
        mnCurrentPageBeforeSwitch = (mrSlideSorter.GetViewShell()->GetViewShellBase()
        .GetMainViewShell()->GetActualPage()->GetPageNum()-1)/2;
}

void SlideSorterController::ChangeEditMode (EditMode eEditMode)
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
}

void SlideSorterController::FinishEditModeChange()
{
    if (mrModel.GetEditMode() == EditMode::MasterPage)
    {
        mpPageSelector->DeselectAllPages();

        // Search for the master page that was determined in
        // PrepareEditModeChange() and make it the current page.
        PageEnumeration aAllPages (PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
        while (aAllPages.HasMoreElements())
        {
            SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
            if (pDescriptor->GetPage() == mpEditModeChangeMasterPage)
            {
                GetCurrentSlideManager()->SwitchCurrentSlide(pDescriptor);
                mpPageSelector->SelectPage(pDescriptor);
                break;
            }
        }
    }
    else
    {
        PageSelector::BroadcastLock aBroadcastLock (*mpPageSelector);

        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnCurrentPageBeforeSwitch));
        GetCurrentSlideManager()->SwitchCurrentSlide(pDescriptor);

        // Restore the selection.
        mpPageSelector->DeselectAllPages();
        for (const auto& rpPage : maSelectionBeforeSwitch)
        {
            mpPageSelector->SelectPage(rpPage);
        }
        maSelectionBeforeSwitch.clear( );
    }
    mpEditModeChangeMasterPage = nullptr;
}

void SlideSorterController::PageNameHasChanged (int nPageIndex, const OUString& rsOldName)
{
    // Request a repaint for the page object whose name has changed.
    model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != nullptr)
        mrView.RequestRepaint(pDescriptor);

    // Get a pointer to the corresponding accessible object and notify
    // that of the name change.
    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    if ( ! pWindow)
        return;

    css::uno::Reference< css::accessibility::XAccessible >
        xAccessible (pWindow->GetAccessible(false));
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
    if (pAccessibleView == nullptr)
        return;

    ::accessibility::AccessibleSlideSorterObject* pChild
            = pAccessibleView->GetAccessibleChildImplementation(nPageIndex);
    if (pChild == nullptr || pChild->GetPage() == nullptr)
        return;

    OUString sNewName (pChild->GetPage()->GetName());
    pChild->FireAccessibleEvent(
        css::accessibility::AccessibleEventId::NAME_CHANGED,
        makeAny(rsOldName),
        makeAny(sNewName));
}

void SlideSorterController::SetDocumentSlides (const Reference<container::XIndexAccess>& rxSlides)
{
    if (mrModel.GetDocumentSlides() != rxSlides)
    {
        ModelChangeLock aLock (*this);
        PreModelChange();

        mrModel.SetDocumentSlides(rxSlides);
    }
}

VisibleAreaManager& SlideSorterController::GetVisibleAreaManager() const
{
    OSL_ASSERT(mpVisibleAreaManager);
    return *mpVisibleAreaManager;
}

void SlideSorterController::CheckForMasterPageAssignment()
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
                pDescriptor->GetPage());
        }
    }
}

void SlideSorterController::CheckForSlideTransitionAssignment()
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
                pDescriptor->GetPage());
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

SlideSorterController::ModelChangeLock::~ModelChangeLock() COVERITY_NOEXCEPT_FALSE
{
    Release();
}

void SlideSorterController::ModelChangeLock::Release()
{
    if (mpController != nullptr)
    {
        mpController->UnlockModelChange();
        mpController = nullptr;
    }
}

} } } // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
