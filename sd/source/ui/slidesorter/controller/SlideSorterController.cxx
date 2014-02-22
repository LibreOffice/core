/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
#include "controller/SlsTransferableData.hxx"
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
#include <vcl/settings.hxx>

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
        
        ::Window* pParentWindow = pWindow->GetParent();
        OSL_ASSERT(pParentWindow!=NULL);
        pParentWindow->SetBackground (Wallpaper());

        
        
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

        
        
        if (pDescriptorAtPoint
            && mrSlideSorter.GetProperties()->IsOnlyPreviewTriggersMouseOver()
            && ! pDescriptorAtPoint->HasState(PageDescriptor::ST_Selected))
        {
            
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
            if (rEvent.IsMouseEvent())
            {
                
                
                
                
                aMenuLocation = rEvent.GetMousePosPixel();
            }
            else
            {
                
                
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
                
                return false;
            }
            
            
            
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

        
        
        
        Rearrange(mbIsForcedRearrangePending);
    }

    if (mrSlideSorter.GetViewShell() != NULL)
        mrSlideSorter.GetViewShell()->Broadcast(
            ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
}




void SlideSorterController::HandleModelChange (void)
{
    
    
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

                    
                    
                    GetPageSelector().SelectPage(GetCurrentSlideManager()->GetCurrentSlide());
                }
                break;

            case VCLEVENT_APPLICATION_DATACHANGED:
            {
                
                cache::PageCacheManager::Instance()->InvalidateAllCaches();

                
                sal_uLong nDrawMode (Application::GetSettings().GetStyleSettings().GetHighContrastMode()
                    ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
                    : ViewShell::OUTPUT_DRAWMODE_COLOR);
                if (mrSlideSorter.GetViewShell() != NULL)
                    mrSlideSorter.GetViewShell()->GetFrameView()->SetDrawMode(nDrawMode);
                if (pActiveWindow != 0)
                    pActiveWindow->SetDrawMode(nDrawMode);
                mrView.HandleDrawModeChange();

                
                mrView.Resize();
                FontProvider::Instance().Invalidate();

                
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
        
        SfxViewFrame* pSlideViewFrame = SfxViewFrame::Current();
        DBG_ASSERT(pSlideViewFrame!=NULL,
            "SlideSorterController::GetCtrlState: ViewFrame not found");
        if (pSlideViewFrame)
        {
            pSlideViewFrame->GetSlotState (SID_RELOAD, NULL, &rSet);
        }
        else        
        {
            rSet.DisableItem(SID_RELOAD);
        }
    }

    
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

            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_COLOR, nQuality==0));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_GRAYSCALE, nQuality==1));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_BLACKWHITE, nQuality==2));
            rSet.Put (SfxBoolItem (SID_OUTPUT_QUALITY_CONTRAST, nQuality==3));
        }
    }

    if (rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) == SFX_ITEM_AVAILABLE)
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




void SlideSorterController::ExecStatusBar (SfxRequest& )
{
}




void SlideSorterController::UpdateAllPages (void)
{
    
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

        
        aNewContentArea = GetScrollBarManager().PlaceScrollBars(
            maTotalWindowArea,
            mrView.GetOrientation() != view::Layouter::VERTICAL,
            mrView.GetOrientation() != view::Layouter::HORIZONTAL);

        bool bSizeHasChanged (false);
        
        
        if ( ! bForce)
        {
            Rectangle aCurrentContentArea (pWindow->GetPosPixel(), pWindow->GetOutputSizePixel());
            bSizeHasChanged = (aNewContentArea != aCurrentContentArea);
        }
        if (bForce || bSizeHasChanged)
        {
            
            pWindow->SetPosSizePixel (aNewContentArea.TopLeft(), aNewContentArea.GetSize());
            mrView.Resize();
        }

        
        
        GetScrollBarManager().UpdateScrollBars(false, !bForce);

        
        GetVisibleAreaManager().RequestCurrentSlideVisible();

        mrView.RequestRepaint();
    }

    return aNewContentArea;
}




rtl::Reference<FuPoor> SlideSorterController::CreateSelectionFunction (SfxRequest& rRequest)
{
    rtl::Reference<FuPoor> xFunc( SelectionFunction::Create(mrSlideSorter, rRequest) );
    return xFunc;
}




::rtl::Reference<SelectionFunction> SlideSorterController::GetCurrentSelectionFunction (void)
{
    rtl::Reference<FuPoor> pFunction (mrSlideSorter.GetViewShell()->GetCurrentFunction());
    return ::rtl::Reference<SelectionFunction>(dynamic_cast<SelectionFunction*>(pFunction.get()));
}




void SlideSorterController::PrepareEditModeChange (void)
{
    
    
    
    if (mrModel.GetEditMode() == EM_PAGE)
    {
        maSelectionBeforeSwitch.clear();

        
        
        
        PageEnumeration aSelectedPages (
            PageEnumerationProvider::CreateSelectedPagesEnumeration(mrModel));
        while (aSelectedPages.HasMoreElements())
        {
            SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
            SdPage* pPage = pDescriptor->GetPage();
            
            if (pPage!=NULL && mpEditModeChangeMasterPage==NULL)
                mpEditModeChangeMasterPage = &static_cast<SdPage&>(
                    pPage->TRG_GetMasterPage());

            maSelectionBeforeSwitch.push_back(pPage);
        }

        
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
        mpPageSelector->DeselectAllPages();

        
        
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

        
        mpPageSelector->DeselectAllPages();
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




void SlideSorterController::PageNameHasChanged (int nPageIndex, const OUString& rsOldName)
{
    
    model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        mrView.RequestRepaint(pDescriptor);

    
    
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if ( ! pWindow)
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        xAccessible (pWindow->GetAccessible(false));
    if ( ! xAccessible.is())
        return;

    
    
    
    
    
    
    
    
    
    
    ::accessibility::AccessibleSlideSorterView* pAccessibleView
            = dynamic_cast< ::accessibility::AccessibleSlideSorterView*>(xAccessible.get());
    if (pAccessibleView == NULL)
        return;

    ::accessibility::AccessibleSlideSorterObject* pChild
            = pAccessibleView->GetAccessibleChildImplementation(nPageIndex);
    if (pChild == NULL || pChild->GetPage() == NULL)
        return;

    OUString sOldName (rsOldName);
    OUString sNewName (pChild->GetPage()->GetName());
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

} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
