/*************************************************************************
 *
 *  $RCSfile: SlideSorterViewShell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:26:34 $
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

#include "SlideSorterViewShell.hxx"

#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsFocusManager.hxx"

#include "ViewShellBase.hxx"
#include "ViewShellImplementation.hxx"
#include "SlideChangeChildWindow.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "sdresid.hxx"
#include "Window.hxx"
#include "FrameView.hxx"
#include "sdpage.hxx"
#include "SdUnoSlideView.hxx"

#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>
#include <svx/ruler.hxx>
#include <svtools/tabbar.hxx>
#include <vcl/scrbar.hxx>

using namespace ::sd::slidesorter;
#define SlideSorterViewShell
#include "sdslots.hxx"

using namespace ::com::sun::star;

namespace sd { namespace slidesorter {

namespace {
const ULONG gnSlideSorterToolbarFeature = 0x11;
}

SFX_IMPL_INTERFACE(
    SlideSorterViewShell,
    SfxShell,
    SdResId(STR_SLIDESORTERVIEWSHELL))
{
    SFX_FEATURED_OBJECTBAR_REGISTRATION(
        SFX_OBJECTBAR_APPLICATION
        | SFX_VISIBILITY_DESKTOP
        | SFX_VISIBILITY_STANDARD
        | SFX_VISIBILITY_CLIENT
        | SFX_VISIBILITY_VIEWER
        | SFX_VISIBILITY_READONLYDOC,
        SdResId(RID_DRAW_VIEWER_TOOLBOX),
        gnSlideSorterToolbarFeature);
    SFX_FEATURED_OBJECTBAR_REGISTRATION(
        SFX_OBJECTBAR_TOOLS
        | SFX_VISIBILITY_STANDARD
        | SFX_VISIBILITY_FULLSCREEN
        | SFX_VISIBILITY_SERVER,
        SdResId(RID_SLIDE_TOOLBOX),
        gnSlideSorterToolbarFeature);
    SFX_FEATURED_OBJECTBAR_REGISTRATION(
        SFX_OBJECTBAR_OBJECT,
        SdResId(RID_SLIDE_OBJ_TOOLBOX),
        gnSlideSorterToolbarFeature);
    SFX_CHILDWINDOW_REGISTRATION(SlideChangeChildWindow::GetChildWindowId() );
}



TYPEINIT1(SlideSorterViewShell, ViewShell);



SlideSorterViewShell::SlideSorterViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    FrameView* pFrameViewArgument)
    : ViewShell (pFrame, pParentWindow, rViewShellBase),
      mbLayoutPending (true)
{
    meShellType = ST_SLIDE_SORTER;

    // Initialize the window.
    ::sd::Window* pWindow = GetActiveWindow();
    pParentWindow->SetBackground(Wallpaper());
    pWindow->SetBackground(Wallpaper());
    pWindow->SetViewOrigin (Point(0,0));
    // We do our own scrolling while dragging a page selection.
    pWindow->SetUseDropScroll (false);
    pWindow->SetStyle (pWindow->GetStyle() | WB_TABSTOP);
    pWindow->Show();


    SetPool( &GetDoc()->GetPool() );

    mpView = mpSlideSorterView.get();
    if (pFrameViewArgument != NULL)
        pFrameView = pFrameViewArgument;
    else
        pFrameView = new FrameView(GetDoc());
    GetFrameView()->Connect();

    pWindow->SetViewShell (this);

    SetName (String (RTL_CONSTASCII_USTRINGPARAM("SlideSorterViewShell")));
}




SlideSorterViewShell::~SlideSorterViewShell (void)
{
    ReleaseListeners();

    // Reset the auto pointers explicitly to control the order of destruction.
    mpSlideSorterController.reset();
    mpSlideSorterModel.reset();
    mpSlideSorterView.reset();
}





void SlideSorterViewShell::Init (void)
{
    CreateModelViewController ();
    mpView = mpSlideSorterView.get();

    // Set view pointer of base class.
    SetupControls (GetParentWindow());

    ViewShell::Init ();
    SetupListeners ();
}




DrawController* SlideSorterViewShell::GetController (void)
{
    if ( !mpController.is() && IsMainViewShell())
    {
        // Create uno controller for the main view shell.  For the ones
        // displayed in the non-center panes we may later introduce
        // sub-controllers.
        DrawController* pController = new SdUnoSlideView (
            GetViewShellBase(),
            *this,
            *GetView());
        mpController = ::comphelper::ImplementationReference<
        DrawController,
            ::com::sun::star::uno::XInterface,
            ::com::sun::star::uno::XWeak> (pController);
    }
    return mpController.get();
}




void SlideSorterViewShell::CreateModelViewController (void)
{
    mpSlideSorterModel = ::std::auto_ptr<model::SlideSorterModel>(
        CreateModel());
    DBG_ASSERT (mpSlideSorterModel.get()!=NULL,
        "Can not create model for slide browser");

    mpSlideSorterView = ::std::auto_ptr<view::SlideSorterView>(
        CreateView());
    DBG_ASSERT (mpSlideSorterView.get()!=NULL,
        "Can not create view for slide browser");

    mpSlideSorterController
        = ::std::auto_ptr<controller::SlideSorterController>(
            CreateController());
    DBG_ASSERT (mpSlideSorterController.get()!=NULL,
        "Can not create controller for slide browser");
}




model::SlideSorterModel* SlideSorterViewShell::CreateModel (void)
{
    // Get pointers to the document.
    SdDrawDocument* pDocument = GetViewShellBase().GetDocument();
    OSL_ASSERT (pDocument!=NULL);

    return new model::SlideSorterModel (*pDocument);
}




view::SlideSorterView* SlideSorterViewShell::CreateView (void)
{
    return new view::SlideSorterView (*this, *mpSlideSorterModel);
}




controller::SlideSorterController* SlideSorterViewShell::CreateController (void)
{
    controller::SlideSorterController* pController
        = new controller::SlideSorterController (
            GetViewFrame()->GetBindings().GetDispatcher()->GetFrame(),
            GetParentWindow(),
            *this,
            *mpSlideSorterModel,
            *mpSlideSorterView);
    pController->Init();
    return pController;
}




SfxShell* SlideSorterViewShell::CreateInstance (
    sal_Int32 nId,
    SfxShell* pParent,
    void* pUserData,
    ViewShellBase& rBase)
{
    return new SlideSorterViewShell (
        rBase.GetViewFrame(),
        rBase,
        static_cast< ::Window*>(pUserData),
        NULL);
}




void SlideSorterViewShell::SetupControls (::Window* pParentWindow)
{
    GetVerticalScrollBar()->Show ();
}



void SlideSorterViewShell::SetupListeners (void)
{
    if (mpTabBar.get() != NULL)
        mpTabBar->SetActivatePageHdl (
            LINK(
                mpSlideSorterController.get(),
                controller::SlideSorterController,
                TabBarHandler));

    GetParentWindow()->AddEventListener(
        LINK(
            mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
}




void SlideSorterViewShell::ReleaseListeners (void)
{
    if (mpTabBar.get() != NULL)
    {
        mpTabBar->SetActivatePageHdl (Link());
        mpTabBar.reset();
    }

    if (GetVerticalScrollBar() != NULL)
    {
        GetVerticalScrollBar()->SetScrollHdl (Link());
    }
    GetParentWindow()->RemoveEventListener(
        LINK(mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
}




void SlideSorterViewShell::GetFocus (void)
{
    GetSlideSorterController().GetFocusManager().ShowFocus();
}




void SlideSorterViewShell::LoseFocus (void)
{
    GetSlideSorterController().GetFocusManager().HideFocus();
}




SdPage* SlideSorterViewShell::GetActualPage (void)
{
    return mpSlideSorterController->GetActualPage();
}




void SlideSorterViewShell::GetMenuState ( SfxItemSet& rSet)
{
    ViewShell::GetMenuState(rSet);
    mpSlideSorterController->GetMenuState (rSet);
}




void SlideSorterViewShell::ExecCtrl (SfxRequest& rRequest)
{
    mpSlideSorterController->ExecCtrl(rRequest);
}




void SlideSorterViewShell::GetCtrlState (SfxItemSet& rSet)
{
    mpSlideSorterController->GetCtrlState(rSet);
}




void SlideSorterViewShell::FuSupport (SfxRequest& rRequest)
{
    mpSlideSorterController->FuSupport(rRequest);
}




/** We have to handle those slot calls here that need to have access to
    private or protected members and methods of this class.
*/
void SlideSorterViewShell::FuTemporary (SfxRequest& rRequest)
{
    switch (rRequest.GetSlot())
    {
        case SID_MODIFYPAGE:
            mpImpl->ProcessModifyPageSlot (
                rRequest,
                GetActualPage(),
                mpSlideSorterModel->GetPageType());
            Cancel();
            rRequest.Done ();
            break;

        default:
            mpSlideSorterController->FuTemporary(rRequest);
            break;
    }
}




void SlideSorterViewShell::GetStatusBarState (SfxItemSet& rSet)
{
    mpSlideSorterController->GetStatusBarState(rSet);
}




void SlideSorterViewShell::FuPermanent (SfxRequest& rRequest)
{
    mpSlideSorterController->FuPermanent(rRequest);
}




void SlideSorterViewShell::GetAttrState (SfxItemSet& rSet)
{
    mpSlideSorterController->GetAttrState(rSet);
}




void SlideSorterViewShell::ExecStatusBar (SfxRequest& rRequest)
{
    mpSlideSorterController->ExecStatusBar(rRequest);
}




void SlideSorterViewShell::Paint (
    const Rectangle& rBBox,
    ::sd::Window* pWindow)
{
    SetActiveWindow (pWindow);
    mpSlideSorterView->InitRedraw (pWindow, Region(rBBox));
}




void SlideSorterViewShell::ArrangeGUIElements (void)
{
    Point aOrigin (aViewPos);
    Size aSize (aViewSize);

    if (aSize.Width()!=0 && aSize.Height()!=0)
    {
        // Prevent untimely redraws while the view is not yet correctly
        // resized.
        mpSlideSorterView->LockRedraw (TRUE);
        if (GetActiveWindow() != NULL)
            GetActiveWindow()->EnablePaint (FALSE);

        if (mpTabBar.get() != NULL)
        {
            Size aTabSize (mpTabBar->GetSizePixel());

            Point aTabPosition (mpTabBar->GetPosPixel());
            mpTabBar->SetPosSizePixel (
                aOrigin,
                Size(aSize.Width(), aTabSize.Height()));
            mpTabBar->Resize();
            aOrigin.Y() += aTabSize.Height();
        }

        maAllWindowRectangle
            = mpSlideSorterController->Resize (Rectangle(aOrigin, aSize));

        if (GetActiveWindow() != NULL)
            GetActiveWindow()->EnablePaint (TRUE);

        mbLayoutPending = false;
        mpSlideSorterView->LockRedraw (FALSE);
    }
    else
    {
        maAllWindowRectangle = Rectangle();
    }
}




SvBorder SlideSorterViewShell::GetBorder (bool bOuterResize)
{
    SvBorder aBorder;

    ScrollBar* pScrollBar = GetVerticalScrollBar();
    if (pScrollBar != NULL && pScrollBar->IsVisible())
        aBorder.Right() = pScrollBar->GetOutputSizePixel().Width();

    pScrollBar = GetHorizontalScrollBar();
    if (pScrollBar != NULL && pScrollBar->IsVisible())
        aBorder.Bottom() = pScrollBar->GetOutputSizePixel().Height();

    return aBorder;
}




void SlideSorterViewShell::Command (
    const CommandEvent& rEvent,
    ::sd::Window* pWindow)
{
    if ( ! mpSlideSorterController->Command (rEvent, pWindow))
        ViewShell::Command (rEvent, pWindow);
}




ScrollBar* SlideSorterViewShell::GetVerticalScrollBar (void) const
{
    return mpVerticalScrollBar.get();
}




ScrollBar* SlideSorterViewShell::GetHorizontalScrollBar (void) const
{
    return mpHorizontalScrollBar.get();
}




ScrollBarBox* SlideSorterViewShell::GetScrollBarFiller (void) const
{
    return mpScrollBarBox.get();
}




void SlideSorterViewShell::ReadFrameViewData (FrameView* pFrameView)
{
    if (pFrameView != NULL)
    {
        view::SlideSorterView& rView (*mpSlideSorterView);
        rView.SetLineDraft (pFrameView->IsLineDraft());
        rView.SetFillDraft (pFrameView->IsFillDraft());
        rView.SetTextDraft (pFrameView->IsTextDraft());
        rView.SetGrafDraft (pFrameView->IsGrafDraft());
    //AF    rView.ChangePagesPerRow (pFrameView->GetSlidesPerRow());

    // DrawMode for 'main' window
        if (GetActiveWindow()->GetDrawMode() != pFrameView->GetDrawMode() )
            GetActiveWindow()->SetDrawMode( pFrameView->GetDrawMode() );
    }
}




void SlideSorterViewShell::WriteFrameViewData()
{
    if (pFrameView != NULL)
    {
        view::SlideSorterView& rView (*mpSlideSorterView);
        pFrameView->SetLineDraft( rView.IsLineDraft() );
        pFrameView->SetFillDraft( rView.IsFillDraft() );
        pFrameView->SetTextDraft( rView.IsTextDraft() );
        pFrameView->SetGrafDraft( rView.IsGrafDraft() );
        //AF    pFrameView->SetSlidesPerRow(rView.GetPagesPerRow());

        // DrawMode for 'main' window
        if( pFrameView->GetDrawMode() != GetActiveWindow()->GetDrawMode() )
            pFrameView->SetDrawMode( GetActiveWindow()->GetDrawMode() );

        SdPage* pActualPage = GetActualPage();

        if (pActualPage != NULL)
            pFrameView->SetSelectedPage (
                ( pActualPage->GetPageNum() - 1 ) / 2 );
    }
}




BOOL SlideSorterViewShell::HasUIFeature (ULONG nFeature)
{
    switch (nFeature)
    {
        case gnSlideSorterToolbarFeature:
            // Return true only when this is a main view.
            if (IsMainViewShell())
                return TRUE;
            else
                return FALSE;

        default:
            // Unknown features are not supported.
            return FALSE;
    }
}




void SlideSorterViewShell::SetZoom (long int nZoom)
{
    mpSlideSorterController->SetZoom (nZoom);
}




void SlideSorterViewShell::SetZoomRect (const Rectangle& rZoomRect)
{
    Size aPageSize (mpSlideSorterView->GetPageBoundingBox(
        0,
        view::SlideSorterView::CS_MODEL,
        view::SlideSorterView::BBT_SHAPE).GetSize());

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

    // #106268#
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
}




SlideSorterViewShell::TabBarEntry
    SlideSorterViewShell::SwitchTabBar (TabBarEntry eEntry)
{
    SlideSorterViewShell::TabBarEntry eCurrentEntry;

    if (mpTabBar.get() != NULL)
    {
        switch (eEntry)
        {
            case TBE_SWITCH:
                if (mpTabBar->GetCurPageId() == TBE_SLIDES)
                    eEntry = TBE_MASTER_PAGES;
                else
                    eEntry = TBE_SLIDES;
                // Fall through.

            case TBE_SLIDES:
            case TBE_MASTER_PAGES:
                mpTabBar->SetCurPageId (eEntry);
                mpSlideSorterController->HandleModelChange();
                break;

            default:
                // Unknown values are ignored.
                break;
        }

        eCurrentEntry = static_cast<TabBarEntry>(mpTabBar->GetCurPageId());
    }
    else
        eCurrentEntry = TBE_SLIDES;

    return eCurrentEntry;
}




void SlideSorterViewShell::UpdateScrollBars (void)
{
    // Do not call the overwritten method of the base class: We do all the
    // scroll bar setup by ourselves.
    mpSlideSorterController->GetScrollBarManager().UpdateScrollBars (false);
}




controller::SlideSorterController&
    SlideSorterViewShell::GetSlideSorterController (void)
{
    return *mpSlideSorterController;
}




void SlideSorterViewShell::StartDrag (
    const Point& rDragPt,
    ::Window* pWindow )
{
    GetSlideSorterController().GetClipboard().StartDrag (
        rDragPt,
        pWindow);
}




void SlideSorterViewShell::DragFinished (
    sal_Int8 nDropAction)
{
    GetSlideSorterController().GetClipboard().DragFinished (nDropAction);
}




sal_Int8 SlideSorterViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    return GetSlideSorterController().GetClipboard().AcceptDrop (
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
    USHORT nPage,
    USHORT nLayer)
{
    return GetSlideSorterController().GetClipboard().ExecuteDrop (
        rEvt,
        rTargetHelper,
        pTargetWindow,
        nPage,
        nLayer);
}




void SlideSorterViewShell::GetSelectedPages (
    ::std::vector<SdPage*>& rPageContainer)
{
    model::PageEnumeration aPages (
        mpSlideSorterModel->GetSelectedPagesEnumeration());
    while (aPages.HasMoreElements())
    {
        model::PageDescriptor& rDescriptor (aPages.GetNextElement());
        rPageContainer.push_back (rDescriptor.GetPage());
    }
}


} } // end of namespace ::sd::slidesorter
