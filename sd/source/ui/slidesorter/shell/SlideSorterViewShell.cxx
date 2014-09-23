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

#include "SlideSorterViewShell.hxx"
#include "ViewShellImplementation.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSlotManager.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "controller/SlsProperties.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "framework/FrameworkHelper.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "sdattr.hrc"
#include "sdresid.hxx"
#include "AccessibleSlideSorterView.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "FrameView.hxx"
#include "SdUnoSlideView.hxx"
#include "ViewShellManager.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <svx/svxids.hrc>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <svx/sidebar/SelectionAnalyzer.hxx>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::sd::slidesorter;
#define SlideSorterViewShell
#include "sdslots.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;
using ::sfx2::sidebar::EnumContext;

namespace sd { namespace slidesorter {

SFX_IMPL_INTERFACE(SlideSorterViewShell, SfxShell, SdResId(STR_SLIDESORTERVIEWSHELL))

void SlideSorterViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}

TYPEINIT1(SlideSorterViewShell, ViewShell);

::boost::shared_ptr<SlideSorterViewShell> SlideSorterViewShell::Create (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::vcl::Window* pParentWindow,
    FrameView* pFrameViewArgument,
    const bool bIsCenterPane)
{
    (void)bIsCenterPane;

    ::boost::shared_ptr<SlideSorterViewShell> pViewShell;
    try
    {
        pViewShell.reset(
            new SlideSorterViewShell(pFrame,rViewShellBase,pParentWindow,pFrameViewArgument));
        pViewShell->Initialize();
        if (pViewShell->mpSlideSorter.get() == NULL)
            pViewShell.reset();
    }
    catch(Exception&)
    {
        pViewShell.reset();
    }
    return pViewShell;
}

SlideSorterViewShell::SlideSorterViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::vcl::Window* pParentWindow,
    FrameView* pFrameViewArgument)
    : ViewShell (pFrame, pParentWindow, rViewShellBase),
      mpSlideSorter(),
      mbIsArrangeGUIElementsPending(true)
{
    meShellType = ST_SLIDE_SORTER;

    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());
    GetFrameView()->Connect();

    SetName (OUString("SlideSorterViewShell"));

    pParentWindow->SetStyle(pParentWindow->GetStyle() | WB_DIALOGCONTROL);
}

SlideSorterViewShell::~SlideSorterViewShell (void)
{
    DisposeFunctions();

    try
    {
        ::sd::Window* pWindow = GetActiveWindow();
        if (pWindow!=NULL)
        {
            ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XComponent> xComponent (
                    pWindow->GetAccessible(false),
                    ::com::sun::star::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL("sd::SlideSorterViewShell::~SlideSorterViewShell(), exception caught!" );
    }
    GetFrameView()->Disconnect();
}

void SlideSorterViewShell::Initialize (void)
{
    mpSlideSorter = SlideSorter::CreateSlideSorter(
        *this,
        mpContentWindow,
        mpHorizontalScrollBar,
        mpVerticalScrollBar,
        mpScrollBarBox);
    mpView = &mpSlideSorter->GetView();

    doShow();

    SetPool( &GetDoc()->GetPool() );
    SetUndoManager( GetDoc()->GetDocSh()->GetUndoManager() );

    // For accessibility we have to shortly hide the content window.
    // This triggers the construction of a new accessibility object for
    // the new view shell.  (One is created earlier while the construtor
    // of the base class is executed.  At that time the correct
    // accessibility object can not be constructed.)
    SharedSdWindow pWindow (mpSlideSorter->GetContentWindow());
    if (pWindow)
    {
        pWindow->Hide();
        pWindow->Show();
    }
}

void SlideSorterViewShell::Init (bool bIsMainViewShell)
{
    ViewShell::Init(bIsMainViewShell);

    // since the updatePageList will show focus, the window.show() must be called ahead. This show is deferred from Init()
    ::sd::Window* pActiveWindow = GetActiveWindow();
    if (pActiveWindow)
        pActiveWindow->Show();
    mpSlideSorter->GetModel().UpdatePageList();

    if (mpContentWindow.get() != NULL)
        mpContentWindow->SetViewShell(this);
}

SlideSorterViewShell* SlideSorterViewShell::GetSlideSorter (ViewShellBase& rBase)
{
    SlideSorterViewShell* pViewShell = NULL;

    // Test the center and left pane for showing a slide sorter.
    OUString aPaneURLs[] = {
        FrameworkHelper::msCenterPaneURL,
        FrameworkHelper::msFullScreenPaneURL,
        FrameworkHelper::msLeftImpressPaneURL,
        OUString()};

    try
    {
        ::boost::shared_ptr<FrameworkHelper> pFrameworkHelper (FrameworkHelper::Instance(rBase));
        if (pFrameworkHelper->IsValid())
            for (int i=0; pViewShell==NULL && !aPaneURLs[i].isEmpty(); ++i)
            {
                pViewShell = dynamic_cast<SlideSorterViewShell*>(
                    pFrameworkHelper->GetViewShell(aPaneURLs[i]).get());
            }
    }
    catch (RuntimeException&)
    {}

    return pViewShell;
}

Reference<drawing::XDrawSubController> SlideSorterViewShell::CreateSubController (void)
{
    Reference<drawing::XDrawSubController> xSubController;

    if (IsMainViewShell())
    {
        // Create uno controller for the main view shell.
        xSubController = Reference<drawing::XDrawSubController>(
            new SdUnoSlideView (
                *mpSlideSorter));
    }

    return xSubController;
}

/** If there is a valid controller then create a new instance of
    <type>AccessibleSlideSorterView</type>.  Otherwise delegate this call
    to the base class to return a default object (probably an empty
    reference).
*/
::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    SlideSorterViewShell::CreateAccessibleDocumentView (::sd::Window* pWindow)
{
    // When the view is not set then the initialization is not yet complete
    // and we can not yet provide an accessibility object.
    if (mpView == NULL || mpSlideSorter.get() == NULL)
        return NULL;

    OSL_ASSERT(mpSlideSorter.get()!=NULL);

    ::accessibility::AccessibleSlideSorterView *pAccessibleView =
    new ::accessibility::AccessibleSlideSorterView(
        *mpSlideSorter.get(),
        pWindow->GetAccessibleParentWindow()->GetAccessible(),
        pWindow);

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> xRet(pAccessibleView);

    pAccessibleView->Init();

    return xRet;
}

void SlideSorterViewShell::SwitchViewFireFocus(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc )
{
    if (xAcc.get())
    {
        ::accessibility::AccessibleSlideSorterView* pBase = static_cast< ::accessibility::AccessibleSlideSorterView* >(xAcc.get());
        if (pBase)
        {
            pBase->SwitchViewActivated();
        }
    }
}

SlideSorter& SlideSorterViewShell::GetSlideSorter (void) const
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    return *mpSlideSorter;
}

bool SlideSorterViewShell::RelocateToParentWindow (::vcl::Window* pParentWindow)
{
    OSL_ASSERT(mpSlideSorter);
    if ( ! mpSlideSorter)
        return false;

    if (pParentWindow == NULL)
        WriteFrameViewData();
    const bool bSuccess (mpSlideSorter->RelocateToWindow(pParentWindow));
    if (pParentWindow != NULL)
        ReadFrameViewData(mpFrameView);

    return bSuccess;
}

::svl::IUndoManager* SlideSorterViewShell::ImpGetUndoManager (void) const
{
    SfxShell* pObjectBar = GetViewShellBase().GetViewShellManager()->GetTopShell();
    if (pObjectBar != NULL)
    {
        // When it exists then return the undo manager of the currently
        // active object bar.  The object bar is missing when the
        // SlideSorterViewShell is not the main view shell.
        return pObjectBar->GetUndoManager();
    }
    else
    {
        // Return the undo manager of this  shell when there is no object or
        // tool bar.
        return const_cast<SlideSorterViewShell*>(this)->GetUndoManager();
    }
}

void SlideSorterViewShell::GetFocus (void)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetFocusManager().ShowFocus();
}

void SlideSorterViewShell::LoseFocus (void)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetFocusManager().HideFocus();
}

SdPage* SlideSorterViewShell::getCurrentPage(void) const
{
    // since SlideSorterViewShell::GetActualPage() currently also
    // returns master pages, which is a wrong behaviour for GetActualPage(),
    // we can just use that for now
    return const_cast<SlideSorterViewShell*>(this)->GetActualPage();
}

SdPage* SlideSorterViewShell::GetActualPage (void)
{
    SdPage* pCurrentPage = NULL;

    // 1. Try to get the current page from the view shell in the center pane
    // (if we are that not ourself).
    if ( ! IsMainViewShell())
    {
        ::boost::shared_ptr<ViewShell> pMainViewShell = GetViewShellBase().GetMainViewShell();
        if (pMainViewShell.get() != NULL)
            pCurrentPage = pMainViewShell->GetActualPage();
    }

    if (pCurrentPage == NULL)
    {
        model::SharedPageDescriptor pDescriptor (
            mpSlideSorter->GetController().GetCurrentSlideManager()->GetCurrentSlide());
        if (pDescriptor.get() != NULL)
            pCurrentPage = pDescriptor->GetPage();
    }

    if (pCurrentPage == NULL)
    {

    }

    return pCurrentPage;
}

void SlideSorterViewShell::GetMenuState ( SfxItemSet& rSet)
{
    ViewShell::GetMenuState(rSet);
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetSlotManager()->GetMenuState(rSet);
}

void SlideSorterViewShell::GetClipboardState ( SfxItemSet& rSet)
{
    ViewShell::GetMenuState(rSet);
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetSlotManager()->GetClipboardState(rSet);
}

void SlideSorterViewShell::ExecCtrl (SfxRequest& rRequest)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().ExecCtrl(rRequest);
}

void SlideSorterViewShell::GetCtrlState (SfxItemSet& rSet)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetCtrlState(rSet);
}

void SlideSorterViewShell::FuSupport (SfxRequest& rRequest)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().FuSupport(rRequest);
}

/** We have to handle those slot calls here that need to have access to
    private or protected members and methods of this class.
*/
void SlideSorterViewShell::FuTemporary (SfxRequest& rRequest)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    switch (rRequest.GetSlot())
    {
        case SID_MODIFYPAGE:
        {
            SdPage* pCurrentPage = GetActualPage();
            if (pCurrentPage != NULL)
                mpImpl->ProcessModifyPageSlot (
                    rRequest,
                    pCurrentPage,
                    mpSlideSorter->GetModel().GetPageType());
            Cancel();
            rRequest.Done ();
        }
        break;

        default:
            mpSlideSorter->GetController().FuTemporary(rRequest);
            break;
    }
}

void SlideSorterViewShell::GetStatusBarState (SfxItemSet& rSet)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetStatusBarState(rSet);
}

void SlideSorterViewShell::FuPermanent (SfxRequest& rRequest)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().FuPermanent(rRequest);
}

void SlideSorterViewShell::GetAttrState (SfxItemSet& rSet)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetAttrState(rSet);
}

void SlideSorterViewShell::ExecStatusBar (SfxRequest& rRequest)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().ExecStatusBar(rRequest);
}

void SlideSorterViewShell::Paint (
    const Rectangle& rBBox,
    ::sd::Window* pWindow)
{
    SetActiveWindow (pWindow);
    OSL_ASSERT(mpSlideSorter);
    if (mpSlideSorter)
        mpSlideSorter->GetController().Paint(rBBox,pWindow);
}

void SlideSorterViewShell::ArrangeGUIElements (void)
{
    if (IsActive())
    {
        OSL_ASSERT(mpSlideSorter.get()!=NULL);
        mpSlideSorter->ArrangeGUIElements(maViewPos, maViewSize);
        mbIsArrangeGUIElementsPending = false;
    }
    else
        mbIsArrangeGUIElementsPending = true;
}

void SlideSorterViewShell::Activate (bool bIsMDIActivate)
{
    ViewShell::Activate(bIsMDIActivate);
    if (mbIsArrangeGUIElementsPending)
        ArrangeGUIElements();

    // Determine and broadcast the context that belongs to the main view shell.
    EnumContext::Context eContext = EnumContext::Context_Unknown;
    ::boost::shared_ptr<ViewShell> pMainViewShell (GetViewShellBase().GetMainViewShell());
    ViewShell::ShellType eMainViewShellType (
        pMainViewShell
            ? pMainViewShell->GetShellType()
            : ViewShell::ST_NONE);
    switch (eMainViewShellType)
    {
        case ViewShell::ST_IMPRESS:
        case ViewShell::ST_SLIDE_SORTER:
        case ViewShell::ST_NOTES:
        case ViewShell::ST_DRAW:
            eContext = EnumContext::Context_DrawPage;
            if (pMainViewShell->ISA(DrawViewShell))
            {
                DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(pMainViewShell.get());
                if (pDrawViewShell != NULL)
                    eContext = EnumContext::GetContextEnum(pDrawViewShell->GetSidebarContextName());
            }
            break;

        default:
            break;
    }
    ContextChangeEventMultiplexer::NotifyContextChange(
        &GetViewShellBase(),
        eContext);
}

void SlideSorterViewShell::Deactivate (bool /*bIsMDIActivate*/)
{
    // Save Settings - Specifically SlidesPerRow to retrieve it later
    WriteFrameViewData();
}

SvBorder SlideSorterViewShell::GetBorder (bool )
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    return mpSlideSorter->GetBorder();
}

void SlideSorterViewShell::Command (
    const CommandEvent& rEvent,
    ::sd::Window* pWindow)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    if ( ! mpSlideSorter->GetController().Command (rEvent, pWindow))
        ViewShell::Command (rEvent, pWindow);
}

void SlideSorterViewShell::ReadFrameViewData (FrameView* pFrameView)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    if (pFrameView != NULL)
    {
        view::SlideSorterView& rView (mpSlideSorter->GetView());

        sal_uInt16 nSlidesPerRow (pFrameView->GetSlidesPerRow());
        if (nSlidesPerRow > 0
            && rView.GetOrientation() == view::Layouter::GRID
            && IsMainViewShell())
        {
            rView.GetLayouter().SetColumnCount(nSlidesPerRow,nSlidesPerRow);
        }
        if (IsMainViewShell())
            mpSlideSorter->GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
                mpFrameView->GetSelectedPage());
        mpSlideSorter->GetController().Rearrange(true);

        // DrawMode for 'main' window
        if (GetActiveWindow()->GetDrawMode() != pFrameView->GetDrawMode() )
            GetActiveWindow()->SetDrawMode( pFrameView->GetDrawMode() );
    }

    // When this slide sorter is not displayed in the main window then we do
    // not share the same frame view and have to find other ways to acquire
    // certain values.
    if ( ! IsMainViewShell())
    {
        ::boost::shared_ptr<ViewShell> pMainViewShell = GetViewShellBase().GetMainViewShell();
        if (pMainViewShell.get() != NULL)
            mpSlideSorter->GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
                pMainViewShell->getCurrentPage());
    }
}

void SlideSorterViewShell::WriteFrameViewData (void)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    if (mpFrameView != NULL)
    {
        view::SlideSorterView& rView (mpSlideSorter->GetView());
        mpFrameView->SetSlidesPerRow((sal_uInt16)rView.GetLayouter().GetColumnCount());

        // DrawMode for 'main' window
        if( mpFrameView->GetDrawMode() != GetActiveWindow()->GetDrawMode() )
            mpFrameView->SetDrawMode( GetActiveWindow()->GetDrawMode() );

        SdPage* pActualPage = GetActualPage();
        if (pActualPage != NULL)
        {
            if (IsMainViewShell())
                mpFrameView->SetSelectedPage((pActualPage->GetPageNum()- 1) / 2);
            // else
            // The slide sorter is not expected to switch the current page
            // other then by double clicks.  That is handled separately.
        }
        else
        {
            // We have no current page to set but at least we can make sure
            // that the index of the frame view has a legal value.
            if (mpFrameView->GetSelectedPage() >= mpSlideSorter->GetModel().GetPageCount())
                mpFrameView->SetSelectedPage((sal_uInt16)mpSlideSorter->GetModel().GetPageCount()-1);
        }
    }
}

void SlideSorterViewShell::SetZoom (long int )
{
    // Ignored.
    // The zoom scale is adapted internally to fit a number of columns in
    // the window.
}

void SlideSorterViewShell::SetZoomRect (const Rectangle& rZoomRect)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    Size aPageSize (mpSlideSorter->GetView().GetLayouter().GetPageObjectSize());

    Rectangle aRect(rZoomRect);

    if (aRect.GetWidth()  < aPageSize.Width())
    {
        long nWidthDiff  = (aPageSize.Width() - aRect.GetWidth()) / 2;

        aRect.Left() -= nWidthDiff;
        aRect.Right() += nWidthDiff;

        if (aRect.Left() < 0)
        {
            aRect.SetPos(Point(0, aRect.Top()));
        }
    }

    if (aRect.GetHeight()  < aPageSize.Height())
    {
        long nHeightDiff  = (aPageSize.Height() - aRect.GetHeight()) / 2;

        aRect.Top() -= nHeightDiff;
        aRect.Bottom() += nHeightDiff;

        if (aRect.Top() < 0)
        {
            aRect.SetPos(Point(aRect.Left(), 0));
        }
    }

    ViewShell::SetZoomRect(aRect);

    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
}

void SlideSorterViewShell::UpdateScrollBars (void)
{
    // Do not call the overwritten method of the base class: We do all the
    // scroll bar setup by ourselves.
    mpSlideSorter->GetController().GetScrollBarManager().UpdateScrollBars (false);
}

void SlideSorterViewShell::StartDrag (
    const Point& rDragPt,
    ::vcl::Window* pWindow )
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetClipboard().StartDrag (
        rDragPt,
        pWindow);
}

void SlideSorterViewShell::DragFinished (
    sal_Int8 nDropAction)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetClipboard().DragFinished (nDropAction);
}

sal_Int8 SlideSorterViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    return mpSlideSorter->GetController().GetClipboard().AcceptDrop (
        rEvt,
        rTargetHelper,
        pTargetWindow,
        nPage,
        nLayer);
}

sal_Int8 SlideSorterViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    return mpSlideSorter->GetController().GetClipboard().ExecuteDrop (
        rEvt,
        rTargetHelper,
        pTargetWindow,
        nPage,
        nLayer);
}

::boost::shared_ptr<SlideSorterViewShell::PageSelection>
    SlideSorterViewShell::GetPageSelection (void) const
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    return mpSlideSorter->GetController().GetPageSelector().GetPageSelection();
}

void SlideSorterViewShell::SetPageSelection (
    const ::boost::shared_ptr<PageSelection>& rSelection)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetPageSelector().SetPageSelection(rSelection);
}

void SlideSorterViewShell::AddSelectionChangeListener (
    const Link& rCallback)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetSelectionManager()->AddSelectionChangeListener(rCallback);
}

void SlideSorterViewShell::RemoveSelectionChangeListener (
    const Link& rCallback)
{
    OSL_ASSERT(mpSlideSorter.get()!=NULL);
    mpSlideSorter->GetController().GetSelectionManager()->RemoveSelectionChangeListener(rCallback);
}

} } // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
