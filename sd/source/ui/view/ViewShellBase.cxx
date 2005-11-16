/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewShellBase.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 09:21:03 $
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

#include "ViewShellBase.hxx"

#include "EventMultiplexer.hxx"
#include "../toolpanel/controls/MasterPageContainer.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "ShellFactory.hxx"
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#ifndef _SD_UNOKYWDS_HXX_
#include "unokywds.hxx"
#endif
#include <svx/svxids.hrc>
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef SD_PANE_CHILD_WINDOWS_HXX
#include "PaneChildWindows.hxx"
#endif
#ifndef SD_NOTES_CHILD_WINDOW_HXX
#include "NotesChildWindow.hxx"
#endif
#ifndef SD_VIEW_SHELL_MANAGER_HXX
#include "ViewShellManager.hxx"
#endif
#include "PaneManager.hxx"
#include "ViewTabBar.hxx"
#ifndef SD_DRAW_CONTROLLER_HXX
#include "DrawController.hxx"
#endif
#ifndef SD_PRINT_MANAGER_HXX
#include "PrintManager.hxx"
#endif
#include "UpdateLockManager.hxx"
#include "FrameView.hxx"
#ifndef _SFXEVENT_HXX
#include <sfx2/event.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#ifndef SD_SLIDE_SORTER_VIEW_SHELL_HXX
#include "SlideSorterViewShell.hxx"
#endif
#ifndef SD_PRESENTATION_SORTER_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif
#ifndef SD_TOOLPANEL_TASK_PANE_VIEW_SHELL_HXX
#include "TaskPaneViewShell.hxx"
#endif
#include "FormShellManager.hxx"
#include "Window.hxx"
#include <sfx2/msg.hxx>
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XVIEWDATASUPPLIER_HPP_
#include <com/sun/star/document/XViewDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#endif

#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>

#ifndef SD_FU_BULLET_HXX
#include "fubullet.hxx"
#endif

using namespace sd;
#define ViewShellBase
#include "sdslots.hxx"

namespace {
class ViewShellFactory
    : public ::sd::ShellFactory< ::sd::ViewShell>
{
public:
    ViewShellFactory (::sd::ViewShellBase& rBase, SfxViewFrame* pViewFrame);
    virtual ::sd::ViewShell* CreateShell (::sd::ShellId nId,
        ::Window* pParentWindow,
        ::sd::FrameView* pFrameView);
    virtual void ReleaseShell (::sd::ViewShell* pShell);
private:
    ::sd::ViewShellBase& mrBase;
    SfxViewFrame* mpViewFrame;
};


} // end of anonymous namespace


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sd {

class ViewShellBase::Implementation
{
public:
    Implementation (ViewShellBase& rBase);

    void ProcessRestoreEditingViewSlot ();

private:
    ViewShellBase& mrBase;

    /** Hold a reference to the page cache manager of the slide sorter in
        order to ensure that it stays alive while the ViewShellBase is
        alive.
    */
    ::boost::shared_ptr<slidesorter::cache::PageCacheManager> mpPageCacheManager;
};




//===== ViewShellBase =========================================================

TYPEINIT1(ViewShellBase, SfxViewShell);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new ViewShellBase object has been constructed.

/*
SFX_IMPL_VIEWFACTORY(ViewShellBase, SdResId(STR_DEFAULTVIEW))
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}
*/
SfxViewFactory* ViewShellBase::pFactory;
SfxViewShell* __EXPORT ViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    ViewShellBase* pBase = new ViewShellBase(pFrame, pOldView, ViewShell::ST_NONE);
    pBase->LateInit();
    return pBase;
}
void ViewShellBase::RegisterFactory( USHORT nPrio )
{
    pFactory = new SfxViewFactory(
        &CreateInstance,&InitFactory,nPrio,SdResId(STR_DEFAULTVIEW));
    InitFactory();
}
void ViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}



SFX_IMPL_INTERFACE(ViewShellBase, SfxViewShell, SdResId(STR_VIEWSHELLBASE))
{
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::LeftPaneChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::RightPaneChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::notes::NotesChildWindow::GetChildWindowId());
}




ViewShellBase::ViewShellBase (
    SfxViewFrame* pFrame,
    SfxViewShell* pOldShell,
    ViewShell::ShellType eDefaultSubShell)
    : SfxViewShell (pFrame,
        SFX_VIEW_MAXIMIZE_FIRST
        | SFX_VIEW_OPTIMIZE_EACH
        | SFX_VIEW_DISABLE_ACCELS
        | SFX_VIEW_OBJECTSIZE_EMBEDDED
        | SFX_VIEW_CAN_PRINT
        | SFX_VIEW_HAS_PRINTOPTIONS),
      maMutex(),
      mpViewTabBar (NULL),
      mpImpl(new Implementation(*this)),
      mpViewShellManager (NULL),
      mpPaneManager (NULL),
      mpDocShell (NULL),
      mpDocument (NULL),
      mpPrintManager (new PrintManager(*this)),
      mpFormShellManager(NULL),
      mpEventMultiplexer(NULL),
      mpUpdateLockManager(new UpdateLockManager(*this))
{
    // Set up the members in the correct order.
    if (GetViewFrame()->GetObjectShell()->ISA(DrawDocShell))
        mpDocShell = static_cast<DrawDocShell*>(
            GetViewFrame()->GetObjectShell());
    if (mpDocShell != NULL)
        mpDocument = mpDocShell->GetDoc();
    mpViewShellManager.reset (new ViewShellManager (*this));
    if (eDefaultSubShell == ViewShell::ST_NONE)
        eDefaultSubShell = GetInitialViewShellType();
    mpPaneManager.reset (new PaneManager(*this, eDefaultSubShell));

    SetWindow (&pFrame->GetWindow());

    // Hide the window to avoid complaints from Sfx...SwitchViewShell...
    pFrame->GetWindow().Hide();

    // Now that this new object has (almost) finished its construction
    // we can pass it as argument to the SubShellManager constructor.
    GetViewShellManager().RegisterDefaultFactory (
        ::std::auto_ptr<ViewShellManager::ViewShellFactory>(
            new ViewShellFactory(*this, pFrame)));
}




/** Call reset() on some of the auto pointers to destroy the pointed-to
    objects in the correct order.
*/
ViewShellBase::~ViewShellBase (void)
{
    // We have to hide the main window to prevent SFX complaining after a
    // reload about it being already visible.
    ViewShell* pShell = GetMainViewShell();
    if (pShell!=NULL
        && pShell->GetActiveWindow()!=NULL
        && pShell->GetActiveWindow()->GetParent()!=NULL)
    {
        pShell->GetActiveWindow()->GetParent()->Hide();
    }

    if (mpViewTabBar.get() != NULL)
    {
        mpViewTabBar->RemoveEventListener(LINK(this,ViewShellBase,WindowEventHandler));
        mpViewTabBar.reset();
    }

    mpPaneManager->Shutdown();
    mpViewShellManager->Shutdown();

    mpEventMultiplexer.reset();
    mpFormShellManager.reset();

    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());

    mpViewShellManager.reset();
    mpPaneManager.reset();
    mpPrintManager.reset();

    SetWindow(NULL);
}




void ViewShellBase::LateInit (void)
{
    StartListening(*GetViewFrame());
    StartListening(*GetDocShell());

    if ( ! GetDocShell()->IsPreview())
    {
        mpViewTabBar.reset (new ViewTabBar(*this, &GetFrame()->GetWindow()));
        mpViewTabBar->AddEventListener(LINK(this,ViewShellBase,WindowEventHandler));
        mpViewTabBar->Show();
    }
    // else when the view shell is used to display a preview of the document
    // then do not show the tab bar.

    mpFormShellManager = ::std::auto_ptr<FormShellManager>(
        new FormShellManager (*this));

    mpEventMultiplexer.reset (new tools::EventMultiplexer (*this));

    // Initialize the pane manager.  This switches synchronously to the
    // default main view shell.
    mpPaneManager->LateInit();

    // Make sure that an instance of the controller exists.  We don't have
    // to call UpdateController() here because the registration at the frame
    // is called automatically.
    if (GetMainViewShell() != NULL)
        GetMainViewShell()->GetController();

    // Remember the type of the current main view shell in the frame view.
    ViewShell* pViewShell = GetMainViewShell();
    if (pViewShell != NULL)
    {
        FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != NULL)
            pFrameView->SetViewShellTypeOnLoad(pViewShell->GetShellType());
    }
}




ViewShellManager& ViewShellBase::GetViewShellManager (void) const
{
    return *mpViewShellManager.get();
}




ViewShell* ViewShellBase::GetMainViewShell (void) const
{
    return mpPaneManager->GetViewShell(PaneManager::PT_CENTER);
}




PaneManager& ViewShellBase::GetPaneManager (void)
{
    return *mpPaneManager.get();
}




ViewShellBase* ViewShellBase::GetViewShellBase (SfxViewFrame* pViewFrame)
{
    ViewShellBase* pBase = NULL;

    if (pViewFrame != NULL)
    {
        // Get the view shell for the frame and cast it to
        // sd::ViewShellBase.
        SfxViewShell* pSfxViewShell = pViewFrame->GetViewShell();
        if (pSfxViewShell!=NULL && pSfxViewShell->ISA(::sd::ViewShellBase))
            pBase = static_cast<ViewShellBase*>(pSfxViewShell);
    }

    return pBase;
}




void ViewShellBase::GetMenuState (SfxItemSet& rSet)
{
}




void ViewShellBase::UpdateController (SfxBaseController* pController)
{
    ::osl::MutexGuard aGuard (maMutex);

    do
    {
        if (pController == NULL)
            break;

        Reference <frame::XController> xController (pController);
        if ( ! xController.is())
            break;

        Reference <awt::XWindow> xWindow(
            GetFrame()->GetFrame()->GetWindow().GetComponentInterface(),
            UNO_QUERY );
        if ( ! xWindow.is())
            break;

        Reference <frame::XFrame> xFrame (
            GetFrame()->GetFrame()->GetFrameInterface());
        if ( ! xFrame.is())
            break;

        SfxObjectShell* pObjectShellold = GetObjectShell();
        if (pObjectShellold == NULL)
            break;

        SetController (pController);
        xFrame->setComponent (xWindow, xController);
        xController->attachFrame (xFrame);
        SfxObjectShell* pObjectShell = GetObjectShell();
        Reference <frame::XModel> xModel (pObjectShell->GetModel());
        if (xModel.is())
        {
            xController->attachModel (xModel);
            xModel->connectController (xController);
            xModel->setCurrentController (xController);
        }
    }
    while (false);
}




DrawDocShell* ViewShellBase::GetDocShell (void) const
{
    return mpDocShell;
}



SdDrawDocument* ViewShellBase::GetDocument (void) const
{
    return mpDocument;
}




void ViewShellBase::SFX_NOTIFY(SfxBroadcaster& rBC,
    const TypeId& rBCType,
    const SfxHint& rHint,
    const TypeId& rHintType)
{
    SfxViewShell::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);

    if (rHint.IsA(TYPE(SfxEventHint)))
    {
        switch (static_cast<const SfxEventHint&>(rHint).GetEventId())
        {
            case SFX_EVENT_OPENDOC:
                if( GetDocument() && GetDocument()->IsStartWithPresentation() )
                {
                    if( GetViewFrame() )
                    {
                        GetDocument()->SetStartWithPresentation( false );
                        GetViewFrame()->GetDispatcher()->Execute(
                            SID_PRESENTATION, SFX_CALLMODE_ASYNCHRON );
                    }
                }
                else
                {
                    mpPaneManager->InitPanes();
                }
                break;

            case SFX_EVENT_CREATEDOC:
                mpPaneManager->InitPanes();
                break;

            case SFX_EVENT_ACTIVATEDOC:
                break;

            case SFX_EVENT_STARTAPP:
            case SFX_EVENT_CLOSEAPP:
            case SFX_EVENT_CLOSEDOC:
            case SFX_EVENT_SAVEDOC:
            case SFX_EVENT_SAVEASDOC:
            case SFX_EVENT_DEACTIVATEDOC:
            case SFX_EVENT_PRINTDOC:
            case SFX_EVENT_ONERROR:
            case SFX_EVENT_LOADFINISHED:
            case SFX_EVENT_SAVEFINISHED:
            case SFX_EVENT_MODIFYCHANGED:
            case SFX_EVENT_PREPARECLOSEDOC:
            case SFX_EVENT_NEWMESSAGE:
            case SFX_EVENT_TOGGLEFULLSCREENMODE:
            case SFX_EVENT_SAVEDOCDONE:
            case SFX_EVENT_SAVEASDOCDONE:
            case SFX_EVENT_MOUSEOVER_OBJECT:
            case SFX_EVENT_MOUSECLICK_OBJECT:
            case SFX_EVENT_MOUSEOUT_OBJECT:
                break;
        }
    }
}




void ViewShellBase::InnerResizePixel (const Point& rOrigin, const Size &rSize)
{
    Size aObjSize = GetObjectShell()->GetVisArea().GetSize();
    if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
    {
        SvBorder aBorder( GetBorderPixel() );
        Size aSize( rSize );
        aSize.Width() -= (aBorder.Left() + aBorder.Right());
        aSize.Height() -= (aBorder.Top() + aBorder.Bottom());
        Size aObjSizePixel = GetWindow()->LogicToPixel( aObjSize, MAP_100TH_MM );
        SfxViewShell::SetZoomFactor( Fraction( aSize.Width(), aObjSizePixel.Width() ),
            Fraction( aSize.Height(), aObjSizePixel.Height() ) );
    }

    ResizePixel (rOrigin, rSize, false);
}



void ViewShellBase::OuterResizePixel (const Point& rOrigin, const Size &rSize)
{
    ResizePixel (rOrigin, rSize, true);
}




void ViewShellBase::ResizePixel (
    const Point& rOrigin,
    const Size &rSize,
    bool bOuterResize)
{
    // Forward the call to both the base class and the main stacked sub
    // shell only when main sub shell exists.
    ViewShell* pMainViewShell
        = mpPaneManager->GetViewShell(PaneManager::PT_CENTER);
    if (pMainViewShell != NULL)
    {
        //Rectangle aModelRectangle (GetWindow()->PixelToLogic(
        //    Rectangle(rOrigin, rSize)));
        SetWindow (pMainViewShell->GetActiveWindow());
        if (mpViewTabBar.get()!=NULL && mpViewTabBar->IsVisible())
            mpViewTabBar->SetPosSizePixel (rOrigin, rSize);
        SvBorder aBorder (pMainViewShell->GetBorder(bOuterResize));
        aBorder += GetBorder(bOuterResize);
        SetBorderPixel (aBorder);


        SvBorder aBaseBorder (ArrangeGUIElements(rOrigin, rSize));
        maClientArea = Rectangle(
            Point (rOrigin.X()+aBaseBorder.Left(),
                rOrigin.Y()+aBaseBorder.Top()),
            Size (rSize.Width() - aBaseBorder.Left() - aBaseBorder.Right(),
                rSize.Height() - aBaseBorder.Top() - aBaseBorder.Bottom()));

        pMainViewShell->Resize( maClientArea.TopLeft(), maClientArea.GetSize() );
    }
    else
    {
        // We have to set a border at all times so when the main view shell
        // is not yet ready we simply set an empty border.
        SetBorderPixel (SvBorder());
        maClientArea = Rectangle( rOrigin, rSize );;
    }
}




void ViewShellBase::Rearrange (void)
{
    OSL_ASSERT(GetViewFrame()!=NULL);
    GetViewFrame()->Resize(TRUE);
}




ErrCode ViewShellBase::DoVerb (long nVerb)
{
    ErrCode aResult = ERRCODE_NONE;

    ::sd::ViewShell* pShell
          = mpPaneManager->GetViewShell(PaneManager::PT_CENTER);
    if (pShell != NULL)
        aResult = pShell->DoVerb (nVerb);

    return aResult;
}




SfxPrinter* ViewShellBase::GetPrinter (BOOL bCreate)
{
    return mpPrintManager->GetPrinter (bCreate);
}




USHORT ViewShellBase::SetPrinter (
    SfxPrinter* pNewPrinter,

    USHORT nDiffFlags)
{
    return mpPrintManager->SetPrinter (pNewPrinter, nDiffFlags);
}




PrintDialog* ViewShellBase::CreatePrintDialog (::Window *pParent)
{
    return mpPrintManager->CreatePrintDialog (pParent);
}




SfxTabPage*  ViewShellBase::CreatePrintOptionsPage(
    ::Window *pParent,
    const SfxItemSet &rOptions)
{
    return mpPrintManager->CreatePrintOptionsPage (pParent, rOptions);
}




USHORT  ViewShellBase::Print(SfxProgress& rProgress, PrintDialog* pDlg)
{
    return mpPrintManager->Print (rProgress, pDlg);
}




ErrCode ViewShellBase::DoPrint (
    SfxPrinter* pPrinter,
    PrintDialog* pPrintDialog,
    BOOL bSilent)
{
    return mpPrintManager->DoPrint (pPrinter, pPrintDialog, bSilent);
}




USHORT ViewShellBase::SetPrinterOptDlg (
    SfxPrinter* pNewPrinter,
    USHORT nDiffFlags,
    BOOL bShowDialog)
{
   return mpPrintManager->SetPrinterOptDlg (
       pNewPrinter,
       nDiffFlags,
       bShowDialog);
}




void ViewShellBase::PreparePrint (PrintDialog* pPrintDialog)
{
    SfxViewShell::PreparePrint (pPrintDialog);
    return mpPrintManager->PreparePrint (pPrintDialog);
}




void ViewShellBase::UIActivating( SfxInPlaceClient* pClient )
{
    ViewShell* pViewShell = mpPaneManager->GetViewShell(PaneManager::PT_CENTER);
    if ( pViewShell )
        pViewShell->UIActivating( pClient );

    SfxViewShell::UIActivating( pClient );
}

void ViewShellBase::UIDeactivated( SfxInPlaceClient* pClient )
{
    SfxViewShell::UIDeactivated( pClient );

    ViewShell* pViewShell = mpPaneManager->GetViewShell(PaneManager::PT_CENTER);
    if ( pViewShell )
        pViewShell->UIDeactivated( pClient );
}


SvBorder ViewShellBase::GetBorder (bool bOuterResize)
{
    int nTop = 0;
    if (mpViewTabBar.get()!=NULL && mpViewTabBar->IsVisible())
        nTop = mpViewTabBar->GetHeight();
    return SvBorder(0,nTop,0,0);
}




SvBorder ViewShellBase::ArrangeGUIElements (
    const Point& rOrigin,
    const Size& rSize)
{
    int nTop =  0;
    if (mpViewTabBar.get()!=NULL && mpViewTabBar->IsVisible())
    {
        nTop =  mpViewTabBar->GetHeight();
        mpViewTabBar->SetPosSizePixel (rOrigin, Size(rSize.Width(),nTop));
    }

    return SvBorder(0,nTop,0,0);
}




void ViewShellBase::Execute (SfxRequest& rRequest)
{
    USHORT nSlotId = rRequest.GetSlot();
    const SfxItemSet* pArgs = rRequest.GetArgs();

    switch (nSlotId)
    {
        case SID_SWITCH_SHELL:
            mpPaneManager->ExecuteModeChange (rRequest);
            break;

        case SID_LEFT_PANE_DRAW:
        case SID_LEFT_PANE_IMPRESS:
        case SID_RIGHT_PANE:
        case SID_NOTES_WINDOW:
        case SID_NORMAL_MULTI_PANE_GUI:
        case SID_SLIDE_SORTER_MULTI_PANE_GUI:
        case SID_DRAWINGMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
            mpPaneManager->ExecuteSlot (rRequest);
            break;

        case SID_WIN_FULLSCREEN:
            // The full screen mode is not supported.  Ignore the request.
            break;

        case SID_TASK_PANE:
        {
            // Set the visibility state of the toolpanel and one of its top
            // level panels.
            BOOL bShowToolPanel = TRUE;
            toolpanel::TaskPaneViewShell::PanelId nPanelId (
                toolpanel::TaskPaneViewShell::PID_UNKNOWN);
            bool bPanelIdGiven = false;

            // Extract the given arguments.
            if (pArgs)
            {
                if ((pArgs->Count() == 1) || (pArgs->Count() == 2))
                {
                    SFX_REQUEST_ARG (rRequest, pIsPanelVisible,
                        SfxBoolItem, ID_VAL_ISVISIBLE, FALSE);
                    if (pIsPanelVisible != NULL)
                        bShowToolPanel = pIsPanelVisible->GetValue();
                }
                if (pArgs->Count() == 2)
                {
                    SFX_REQUEST_ARG (rRequest, pPanelId, SfxUInt32Item,
                        ID_VAL_PANEL_INDEX, FALSE);
                    if (pPanelId != NULL)
                    {
                        nPanelId = static_cast<
                            toolpanel::TaskPaneViewShell::PanelId>(
                                pPanelId->GetValue());
                        bPanelIdGiven = true;
                    }
                }
            }

            // Ignore the request for some combinations of panels and view
            // shell types.
            if (bPanelIdGiven
                && ! (nPanelId==toolpanel::TaskPaneViewShell::PID_LAYOUT
                    && GetMainViewShell()!=NULL
                    && GetMainViewShell()->GetShellType()==ViewShell::ST_OUTLINE))
            {
                // Set the visibility of the right pane.
                GetPaneManager().RequestViewShellChange(
                    PaneManager::PT_RIGHT,
                    ViewShell::ST_TASK_PANE,
                    PaneManager::CM_SYNCHRONOUS);

                // Now we can call the tool pane to make the specified panel
                // visible.
                if (bShowToolPanel && bPanelIdGiven)
                {
                    toolpanel::TaskPaneViewShell* pTaskPane
                        = static_cast<toolpanel::TaskPaneViewShell*>(
                            GetPaneManager().GetViewShell(PaneManager::PT_RIGHT));
                    if (pTaskPane != NULL)
                        pTaskPane->ShowPanel (nPanelId);
                }
            }
        }
        break;

        case SID_RESTORE_EDITING_VIEW:
            mpImpl->ProcessRestoreEditingViewSlot();
            break;

        default:
            // Ignore any other slot.
            rRequest.Ignore ();
            break;
    }
}




void ViewShellBase::GetState (SfxItemSet& rSet)
{
    // The full screen mode is not supported.  Disable the the slot so that
    // it appears grayed out when somebody uses configures the menu to show
    // an menu item for it.
    if (rSet.GetItemState(SID_WIN_FULLSCREEN) == SFX_ITEM_AVAILABLE)
        rSet.DisableItem(SID_WIN_FULLSCREEN);

    mpPaneManager->GetSlotState (rSet);

    FuBullet::GetSlotState( rSet, 0, GetViewFrame() );
}

void ViewShellBase::WriteUserDataSequence (
    ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence,
    sal_Bool bBrowse)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->WriteUserDataSequence (rSequence, bBrowse);
}




void ViewShellBase::ReadUserDataSequence (
    const ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence,
    sal_Bool bBrowse)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
    {
        pShell->ReadUserDataSequence (rSequence, bBrowse);

        // For certain shell types ReadUserDataSequence may have changed the
        // type to another one.  Make sure that the center pane shows the
        // right view shell.
        switch (pShell->GetShellType())
        {
            case ViewShell::ST_IMPRESS:
            case ViewShell::ST_NOTES:
            case ViewShell::ST_HANDOUT:
            {
                ViewShell::ShellType eType (ViewShell::ST_NONE);
                switch (PTR_CAST(DrawViewShell, pShell)->GetPageKind())
                {
                    case PK_STANDARD:
                        eType = ViewShell::ST_IMPRESS;
                        break;
                    case PK_NOTES:
                        eType = ViewShell::ST_NOTES;
                        break;
                    case PK_HANDOUT:
                        eType = ViewShell::ST_HANDOUT;
                        break;
                }
                if (eType != ViewShell::ST_NONE)
                    GetPaneManager().RequestMainViewShellChange(eType);
            }
        }
    }
}




void ViewShellBase::Activate (BOOL bIsMDIActivate)
{
    SfxViewShell::Activate(bIsMDIActivate);
    GetPaneManager().InitPanes ();
}




void ViewShellBase::Deactivate (BOOL bIsMDIActivate)
{
    SfxViewShell::Deactivate(bIsMDIActivate);
}




void ViewShellBase::SetZoomFactor (
    const Fraction &rZoomX,
    const Fraction &rZoomY)
{
    SfxViewShell::SetZoomFactor (rZoomX, rZoomY);
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->SetZoomFactor (rZoomX, rZoomY);
}




USHORT ViewShellBase::PrepareClose (BOOL bUI, BOOL bForBrowsing)
{
    USHORT nResult = SfxViewShell::PrepareClose (bUI, bForBrowsing);

    if (nResult == TRUE)
    {
        // Forward call to main sub shell.
        ViewShell* pShell = GetMainViewShell();
        if (pShell != NULL)
            nResult = pShell->PrepareClose (bUI, bForBrowsing);
    }

    return nResult;
}




void ViewShellBase::WriteUserData (String& rString, BOOL bBrowse)
{
    SfxViewShell::WriteUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->WriteUserData (rString);
}




void ViewShellBase::ReadUserData (const String& rString, BOOL bBrowse)
{
    SfxViewShell::ReadUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->ReadUserData (rString);
}




SdrView* ViewShellBase::GetDrawView (void) const
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        return pShell->GetDrawView ();
    else
        return SfxViewShell::GetDrawView();
}




void ViewShellBase::AdjustPosSizePixel (const Point &rOfs, const Size &rSize)
{
    SfxViewShell::AdjustPosSizePixel (rOfs, rSize);
}




void ViewShellBase::SetBusyState (bool bBusy)
{
    if (GetDocShell() != NULL)
        GetDocShell()->SetWaitCursor (bBusy);
}




void ViewShellBase::UpdateBorder ( bool bForce /* = false */ )
{
    ViewShell* pMainViewShell = GetMainViewShell();
    if (pMainViewShell != NULL)
    {
        SvBorder aCurrentBorder (GetBorderPixel());

        bool bOuterResize ( ! GetDocShell()->IsInPlaceActive());
        SvBorder aBorder (pMainViewShell->GetBorder(bOuterResize));
        aBorder += GetBorder(bOuterResize);

        if (bForce || (aBorder != aCurrentBorder))
        {
            SetBorderPixel (aBorder);
            InvalidateBorder();
        }
    }
}




void ViewShellBase::ShowUIControls (bool bVisible)
{
    if (mpViewTabBar.get() != NULL)
        mpViewTabBar->Show (bVisible);

    ViewShell* pMainViewShell = GetMainViewShell();
    if (pMainViewShell != NULL)
        pMainViewShell->ShowUIControls (bVisible);

    UpdateBorder();
    if (bVisible)
        Rearrange();
}




ViewShell::ShellType ViewShellBase::GetInitialViewShellType (void)
{
    ViewShell::ShellType aShellType (ViewShell::ST_IMPRESS);

    do
    {
        Reference<document::XViewDataSupplier> xViewDataSupplier (
            GetDocShell()->GetModel(), UNO_QUERY);
        if ( ! xViewDataSupplier.is())
            break;

        Reference<container::XIndexAccess> xViewData (xViewDataSupplier->getViewData());
        if ( ! xViewData.is())
            break;
        if (xViewData->getCount() == 0)
            break;

        sal_Int32 nView = 0;
        ::com::sun::star::uno::Any aAny = xViewData->getByIndex(nView);
        Sequence<beans::PropertyValue> aProperties;
        if ( ! (aAny >>= aProperties))
            break;

        // Search the properties for the one that tells us what page kind to
        // use.
        for (sal_Int32 n=0; n<aProperties.getLength(); n++)
        {
            const beans::PropertyValue& rProperty (aProperties[n]);
            if (rProperty.Name.compareToAscii(sUNO_View_PageKind) == COMPARE_EQUAL)
            {
                sal_Int16 nPageKind;
                rProperty.Value >>= nPageKind;
                switch ((PageKind)nPageKind)
                {
                    case PK_STANDARD:
                        aShellType = ViewShell::ST_IMPRESS;
                        break;

                    case PK_HANDOUT:
                        aShellType = ViewShell::ST_HANDOUT;
                        break;

                    case PK_NOTES:
                        aShellType = ViewShell::ST_NOTES;
                        break;

                    default:
                        // The page kind is invalid.  This is propably an
                        // error by the caller.  We use the standard type to
                        // keep things going.
                        DBG_ASSERT(sal_False, "ViewShellBase::GetInitialViewShellType: invalid page kind");
                        aShellType = ViewShell::ST_IMPRESS;
                        break;
                }
                break;
            }
        }
    }
    while (false);

    return aShellType;
}




IMPL_LINK(ViewShellBase, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent->GetId() == VCLEVENT_WINDOW_SHOW)
        UpdateBorder();
    return 1;
}




/** this method starts the presentation by
    executing the slot SID_PRESENTATION asynchronous */
void ViewShellBase::StartPresentation()
{
    if( GetViewFrame() && GetViewFrame()->GetDispatcher() )
        GetViewFrame()->GetDispatcher()->Execute(SID_PRESENTATION, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}




/** this methods ends the presentation by
    executing the slot SID_PRESENTATION_END asynchronous */
void ViewShellBase::StopPresentation()
{
    if( GetViewFrame() && GetViewFrame()->GetDispatcher() )
        GetViewFrame()->GetDispatcher()->Execute(SID_PRESENTATION_END, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}




tools::EventMultiplexer& ViewShellBase::GetEventMultiplexer (void)
{
    // Maybe we should throw an exception in the unlikely case that
    // mpEventMultiplexer is NULL.
    return *mpEventMultiplexer.get();
}




UpdateLockManager& ViewShellBase::GetUpdateLockManager (void) const
{
    return *mpUpdateLockManager;
}




//===== ViewShellBase::Implementation =========================================

ViewShellBase::Implementation::Implementation (ViewShellBase& rBase)
    : mrBase(rBase),
      mpPageCacheManager(slidesorter::cache::PageCacheManager::Instance())
{
}




void ViewShellBase::Implementation::ProcessRestoreEditingViewSlot (void)
{
    FrameView* pFrameView = NULL;
    SfxViewFrame* pViewFrame = NULL;

    if (mrBase.GetMainViewShell() != NULL)
    {
        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
        pViewFrame = mrBase.GetMainViewShell()->GetViewFrame();
    }

    if (pFrameView!=NULL && pViewFrame!=NULL)
    {
        PageKind ePageKind (pFrameView->GetPageKindOnLoad());

        ViewShell* pViewShell = mrBase.GetMainViewShell();
        if (pViewShell != NULL)
        {
            FrameView* pFrameView = pViewShell->GetFrameView();
            if (pFrameView != NULL)
            {
                // Set view shell, edit mode, and page kind.
                pFrameView->SetViewShEditMode(
                    pFrameView->GetViewShEditModeOnLoad(),
                    pFrameView->GetPageKindOnLoad());
                pFrameView->SetPageKind(
                    pFrameView->GetPageKindOnLoad());
                mrBase.GetPaneManager().RequestMainViewShellChange(
                    pFrameView->GetViewShellTypeOnLoad(),
                    PaneManager::CM_SYNCHRONOUS);

                try
                {
                    // Get the current page either from the
                    // DrawPagesSupplier or the MasterPagesSupplier.
                    Any aPage;
                    if (pFrameView->GetViewShEditModeOnLoad() == EM_PAGE)
                    {
                        Reference<drawing::XDrawPagesSupplier> xPagesSupplier (
                            mrBase.GetController()->getModel(), UNO_QUERY_THROW);
                        Reference<container::XIndexAccess> xPages (
                            xPagesSupplier->getDrawPages(), UNO_QUERY_THROW);
                        aPage = xPages->getByIndex(pFrameView->GetSelectedPageOnLoad());
                    }
                    else
                    {
                        Reference<drawing::XMasterPagesSupplier> xPagesSupplier (
                            mrBase.GetController()->getModel(), UNO_QUERY_THROW);
                        Reference<container::XIndexAccess> xPages (
                            xPagesSupplier->getMasterPages(), UNO_QUERY_THROW);
                        aPage = xPages->getByIndex(pFrameView->GetSelectedPageOnLoad());
                    }
                    // Switch to the page last edited by setting the
                    // CurrentPage property.
                    Reference<beans::XPropertySet> xSet (mrBase.GetController(), UNO_QUERY_THROW);
                    xSet->setPropertyValue (String::CreateFromAscii("CurrentPage"), aPage);
                }
                catch (RuntimeException aException)
                {
                    // We have not been able to set the current page at the main view.
                    // This is sad but still leaves us in a valid state.  Therefore,
                    // this exception is silently ignored.
                }
            }
        }
    }
}


} // end of namespace sd




//===== ViewShellFactory ======================================================

namespace {
using namespace sd;

ViewShellFactory::ViewShellFactory (
    ::sd::ViewShellBase& rBase,
    SfxViewFrame* pViewFrame)
    : mrBase (rBase),
      mpViewFrame(pViewFrame)
{}




::sd::ViewShell* ViewShellFactory::CreateShell (
    ::sd::ShellId nId,
    ::Window* pParentWindow,
    ::sd::FrameView* pFrameView)
{
    ViewShell::ShellType eShellType = static_cast<ViewShell::ShellType>(nId);

    ViewShell* pNewShell = NULL;

    switch (eShellType)
    {
        case ViewShell::ST_IMPRESS:
            pNewShell = new DrawViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                PK_STANDARD,
                pFrameView);
            break;

        case ViewShell::ST_NOTES:
            pNewShell = new DrawViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                PK_NOTES,
                pFrameView);
            break;

        case ViewShell::ST_HANDOUT:
            pNewShell = new DrawViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                PK_HANDOUT,
                pFrameView);
            break;

        case ViewShell::ST_DRAW:
            pNewShell = new GraphicViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                pFrameView);
            break;

        case ViewShell::ST_OUTLINE:
            pNewShell = new OutlineViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                pFrameView);
            break;

        case ViewShell::ST_SLIDE:
            pNewShell = new SlideViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                pFrameView);
            break;

        case ViewShell::ST_SLIDE_SORTER:
            pNewShell = new ::sd::slidesorter::SlideSorterViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                pFrameView);
            break;

        case ViewShell::ST_PRESENTATION:
            pNewShell = new PresentationViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                pFrameView);
            break;

        case ViewShell::ST_TASK_PANE:
            pNewShell = new ::sd::toolpanel::TaskPaneViewShell (
                mpViewFrame,
                mrBase,
                pParentWindow,
                pFrameView);
            break;

        default:
            // mpMainSubShell will be reset by using pNewShell with is
            // default NULL value.
            break;
    }
    return pNewShell;
}




void ViewShellFactory::ReleaseShell (::sd::ViewShell* pShell)
{
    delete pShell;
}


} // end of anonymouse namespace
