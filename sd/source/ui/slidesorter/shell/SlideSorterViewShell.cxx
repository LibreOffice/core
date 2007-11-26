/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterViewShell.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:36:54 $
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

#include "SlideSorterViewShell.hxx"

#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsSharedPageDescriptor.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsFocusManager.hxx"
#include "framework/FrameworkHelper.hxx"
#include "AccessibleSlideSorterView.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellImplementation.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "sdattr.hrc"
#include "sdresid.hxx"
#include "Window.hxx"
#include "FrameView.hxx"
#include "sdpage.hxx"
#include "SdUnoSlideView.hxx"
#include "DrawDocShell.hxx"
#include "ViewShellManager.hxx"

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

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_PANECONTROLLER_HPP_
#include <com/sun/star/drawing/framework/PaneController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_RESOURCEID_HPP_
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#endif
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::sd::slidesorter;
#define SlideSorterViewShell
#include "sdslots.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace sd { namespace slidesorter {


SFX_IMPL_INTERFACE(SlideSorterViewShell, SfxShell, SdResId(STR_SLIDESORTERVIEWSHELL))
{
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
    // Change the winbits so that the active window accepts the focus.
    pWindow->SetStyle ((pWindow->GetStyle() & ~WB_DIALOGCONTROL) | WB_TABSTOP);
    pWindow->Hide();


    SetPool( &GetDoc()->GetPool() );
    SetUndoManager( GetDoc()->GetDocSh()->GetUndoManager() );

    mpView = mpSlideSorterView.get();
    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());
    GetFrameView()->Connect();

    pWindow->SetViewShell (this);

    SetName (String (RTL_CONSTASCII_USTRINGPARAM("SlideSorterViewShell")));
}




SlideSorterViewShell::~SlideSorterViewShell (void)
{
    DisposeFunctions();
    ReleaseListeners();

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
    catch( ::com::sun::star::uno::Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::SlideSorterViewShell::~SlideSorterViewShell(), exception caught!" );
    }

    // Reset the auto pointers explicitly to control the order of destruction.
    mpSlideSorterController.reset();
    mpSlideSorterView.reset();
    mpSlideSorterModel.reset();
}





void SlideSorterViewShell::Init (bool bIsMainViewShell)
{
    ViewShell::Init(bIsMainViewShell);

    CreateModelViewController ();
    mpView = mpSlideSorterView.get();

    // Set view pointer of base class.
    SetupControls (GetParentWindow());

    SetupListeners ();

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  At that time the correct accessibility object can
    // not be constructed.)
    if (mpContentWindow.get() !=NULL)
    {
        mpContentWindow->Hide();
        mpContentWindow->Show();
    }
}




SlideSorterViewShell* SlideSorterViewShell::GetSlideSorter (ViewShellBase& rBase)
{
    SlideSorterViewShell* pViewShell = NULL;

    // Test the center, left, and then the right pane for showing a slide sorter.
    ::rtl::OUString aPaneURLs[] = {
        FrameworkHelper::msCenterPaneURL,
        FrameworkHelper::msLeftImpressPaneURL,
        FrameworkHelper::msRightPaneURL,
        ::rtl::OUString()};

    try
    {
        Reference<XControllerManager> xControllerManager (rBase.GetController(), UNO_QUERY_THROW);
        Reference<XViewController> xViewController (xControllerManager->getViewController());
        if (xViewController.is())
            for (int i=0; pViewShell==NULL && aPaneURLs[i].getLength()>0; ++i)
            {
                Reference<XResourceId> xPaneId(
                    ResourceId::create(
                        comphelper_getProcessComponentContext(),
                        aPaneURLs[i]));
                Reference<XView> xView (xViewController->getFirstViewForAnchor(xPaneId));
                if (xView.is()
                    && xView->getResourceId()->getResourceURL().equals(
                        FrameworkHelper::msSlideSorterURL))
                {
                    pViewShell = dynamic_cast<SlideSorterViewShell*>(
                        FrameworkHelper::Instance(rBase)->GetViewShell(aPaneURLs[i]).get());
                }
            }
    }
    catch (RuntimeException&)
    {}

    return pViewShell;
}




::std::auto_ptr<DrawSubController> SlideSorterViewShell::CreateSubController (void)
{
    ::std::auto_ptr<DrawSubController> pController;

    if (IsMainViewShell())
    {
        // Create uno controller for the main view shell.
        ViewShellBase& rBase (GetViewShellBase());
        pController.reset(new SdUnoSlideView (
            rBase.GetDrawController(),
            *this,
            *GetView()));
    }
    return pController;
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
    return new ::accessibility::AccessibleSlideSorterView (
        *mpSlideSorterController.get(),
        pWindow->GetAccessibleParentWindow()->GetAccessible(),
        pWindow);
}




bool SlideSorterViewShell::RelocateToParentWindow (::Window* pParentWindow)
{
    ReleaseListeners();

    ViewShell::RelocateToParentWindow(pParentWindow);

    SetupControls(GetParentWindow());
    SetupListeners();

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  At that time the correct accessibility object can
    // not be constructed.)
    if (mpContentWindow.get() !=NULL)
    {
        mpContentWindow->Hide();
        mpContentWindow->Show();
    }

    Resize();

    return true;
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

    mpView = mpSlideSorterView.get();

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




SfxUndoManager* SlideSorterViewShell::ImpGetUndoManager (void) const
{
    SfxShell* pObjectBar = GetViewShellBase().GetViewShellManager().GetTopShell();
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




SfxShell* SlideSorterViewShell::CreateInstance (
    sal_Int32,
    SfxShell*,
    void* pUserData,
    ViewShellBase& rBase)
{
    return new SlideSorterViewShell (
        rBase.GetViewFrame(),
        rBase,
        static_cast< ::Window*>(pUserData),
        NULL);
}




void SlideSorterViewShell::SetupControls (::Window* )
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

    mpSlideSorterController->GetScrollBarManager().Connect();

    GetParentWindow()->AddEventListener(
        LINK(
            mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
    GetActiveWindow()->AddEventListener(
        LINK(
            mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
    Application::AddEventListener(
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

    mpSlideSorterController->GetScrollBarManager().Disconnect();

    GetActiveWindow()->RemoveEventListener(
        LINK(mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
    GetParentWindow()->RemoveEventListener(
        LINK(mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
    Application::RemoveEventListener(
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




SdPage* SlideSorterViewShell::getCurrentPage(void) const
{
    // since SlideSorterViewShell::GetActualPage() currently also
    // returns master pages, which is a wrong behaviour for GetActualPage(),
    // we can just use that for now
    return const_cast<SlideSorterViewShell*>(this)->GetActualPage();
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
        {
            SdPage* pCurrentPage = GetActualPage();
            if (pCurrentPage != NULL)
                mpImpl->ProcessModifyPageSlot (
                    rRequest,
                    pCurrentPage,
                    mpSlideSorterModel->GetPageType());
            Cancel();
            rRequest.Done ();
        }
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
    mpSlideSorterController->Paint(rBBox,pWindow);
}




void SlideSorterViewShell::ArrangeGUIElements (void)
{
    Point aOrigin (maViewPos);
    Size aSize (maViewSize);

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




SvBorder SlideSorterViewShell::GetBorder (bool )
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

        USHORT nSlidesPerRow (pFrameView->GetSlidesPerRow());
        if (nSlidesPerRow == 0 || ! IsMainViewShell())
        {
            // When a value of 0 (automatic) is given or the the slide
            // sorter is displayed in a side pane then we ignore the value
            // of the frame view and adapt the number of columns
            // automatically to the window width.
            rView.GetLayouter().SetColumnCount(1,5);
        }
        else
            rView.GetLayouter().SetColumnCount(nSlidesPerRow,nSlidesPerRow);
        mpSlideSorterController->Rearrange(true);

        // DrawMode for 'main' window
        if (GetActiveWindow()->GetDrawMode() != pFrameView->GetDrawMode() )
            GetActiveWindow()->SetDrawMode( pFrameView->GetDrawMode() );
    }
}




void SlideSorterViewShell::WriteFrameViewData()
{
    if (mpFrameView != NULL)
    {
        view::SlideSorterView& rView (*mpSlideSorterView);
        mpFrameView->SetSlidesPerRow((USHORT)rView.GetLayouter().GetColumnCount());

        // DrawMode for 'main' window
        if( mpFrameView->GetDrawMode() != GetActiveWindow()->GetDrawMode() )
            mpFrameView->SetDrawMode( GetActiveWindow()->GetDrawMode() );

        SdPage* pActualPage = GetActualPage();
        if (pActualPage != NULL)
        {
            // The slide sorter is not expected to switch the current page
            // other then by double clicks.  That is handled seperatly.
            //            mpFrameView->SetSelectedPage((pActualPage->GetPageNum()- 1) / 2);
        }
        else
        {
            // We have no current page to set but at least we can make sure
            // that the index of the frame view has a legal value.
            if (mpFrameView->GetSelectedPage() >= mpSlideSorterModel->GetPageCount())
                mpFrameView->SetSelectedPage((USHORT)mpSlideSorterModel->GetPageCount()-1);
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
                mpTabBar->SetCurPageId ((USHORT)eEntry);
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
        model::SharedPageDescriptor pDescriptor (aPages.GetNextElement());
        rPageContainer.push_back(pDescriptor->GetPage());
    }
}




void SlideSorterViewShell::AddSelectionChangeListener (
    const Link& rCallback)
{
    GetSlideSorterController().AddSelectionChangeListener(rCallback);
}




void SlideSorterViewShell::RemoveSelectionChangeListener (
    const Link& rCallback)
{
    GetSlideSorterController().RemoveSelectionChangeListener(rCallback);
}



} } // end of namespace ::sd::slidesorter
