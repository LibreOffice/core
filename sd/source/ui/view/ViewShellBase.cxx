/*************************************************************************
 *
 *  $RCSfile: ViewShellBase.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 09:00:41 $
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

#include "ViewShellBase.hxx"

#include "../toolpanel/controls/MasterPageContainer.hxx"
#include "ShellFactory.hxx"
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
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
#ifndef SD_PREVIEW_SORTER_VIEW_SHELL_HXX
#include "PreviewViewShell.hxx"
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

#define ViewShellBase
using namespace sd;
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
    ViewShellBase* pBase = new ViewShellBase(pFrame, pOldView);
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
      mpViewShellManager (NULL),
      mpPaneManager (NULL),
      mpDocShell (NULL),
      mpDocument (NULL),
      maPrintManager (*this),
      mpFormShellManager(NULL)
{
    // Set up the members in the correct order.
    if (GetViewFrame()->GetObjectShell()->ISA(DrawDocShell))
        mpDocShell = static_cast<DrawDocShell*>(
            GetViewFrame()->GetObjectShell());
    if (mpDocShell != NULL)
        mpDocument = mpDocShell->GetDoc();
    mpViewShellManager.reset (new ViewShellManager (*this));
    mpPaneManager.reset (new PaneManager(*this));

    SetWindow (&pFrame->GetWindow());

    // Now that this new object has (almost) finished its construction
    // we can pass it as argument to the SubShellManager constructor.
    GetViewShellManager().RegisterDefaultFactory (
        ::std::auto_ptr<ViewShellManager::ViewShellFactory>(
            new ViewShellFactory(*this, pFrame)));

    // Tell the pane manager what shell to put into the center pane.
    // Setting the window later will create that shell.
    mpPaneManager->RequestMainViewShellChange (
        eDefaultSubShell);
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

    mpPaneManager->Shutdown();

    mpViewTabBar.reset();
    mpFormShellManager.reset();

    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());

    mpViewShellManager.reset();
    mpPaneManager.reset();

    SetWindow(NULL);
}




void ViewShellBase::LateInit (void)
{
    StartListening(*GetViewFrame());
    StartListening(*GetDocShell());

    mpViewTabBar.reset (new ViewTabBar(*this, &GetFrame()->GetWindow()));
    mpViewTabBar->Show();

    // Try to init the panes for which the windows are already present.
    // This usually includes the center pane but not the panes inside
    // dockable child windows.
    mpPaneManager->LateInit();

    mpFormShellManager = ::std::auto_ptr<FormShellManager>(
        new FormShellManager (*this));

    // Make sure that an instance of the controller exists.  We don't have
    // to call UpdateController() here because the registration at the frame
    // is called automatically.
    //    GetMainViewShell()->GetController();
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




void ViewShellBase::UpdateController (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    ViewShell* pViewShell = GetMainViewShell();
    SfxBaseController* pController = pViewShell->GetController();

    if (pController == NULL)
        pController = new SfxBaseController (this);
    if (pController != NULL)
    {
        SetController (pController);
        Reference <frame::XController> xController (pController);
        Reference <awt::XWindow> xWindow(
            GetFrame()->GetFrame()->GetWindow().GetComponentInterface(),
            UNO_QUERY );
        Reference <frame::XFrame> xFrame (
            GetFrame()->GetFrame()->GetFrameInterface());
        SfxObjectShell* pObjectShellold = GetObjectShell();
        if (xFrame.is() && xController.is())
        {
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
    }
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
        OSL_TRACE ("ViewShellBase::SFX_NOTIFY: %p %d",
            this,
            static_cast<const SfxEventHint&>(rHint).GetEventId());
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
        Rectangle aModelRectangle (GetWindow()->PixelToLogic(
            Rectangle(rOrigin, rSize)));
        SetWindow (pMainViewShell->GetActiveWindow());
        if (mpViewTabBar.get()!=NULL && mpViewTabBar->IsVisible())
            mpViewTabBar->SetPosSizePixel (rOrigin, rSize);
        SvBorder aBorder (pMainViewShell->GetBorder(bOuterResize));
        aBorder += GetBorder(bOuterResize);
        SetBorderPixel (aBorder);

        SvBorder aBaseBorder (ArrangeGUIElements(rOrigin, rSize));
        pMainViewShell->Resize (
            Point (rOrigin.X()+aBaseBorder.Left(),
                rOrigin.Y()+aBaseBorder.Top()),
            Size (rSize.Width() - aBaseBorder.Left() - aBaseBorder.Right(),
                rSize.Height() - aBaseBorder.Top() - aBaseBorder.Bottom()));
    }
    else
        // We have to set a border at all times so when the main view shell
        // is not yet ready we simply set an empty border.
        SetBorderPixel (SvBorder());
}




void ViewShellBase::Rearrange (void)
{
    ::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        ResizePixel (
            pWindow->GetPosPixel(),
            pWindow->GetOutputSizePixel(),
            GetDocShell()->IsInPlaceActive());
    }
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
    return maPrintManager.GetPrinter (bCreate);
}




USHORT ViewShellBase::SetPrinter (
    SfxPrinter* pNewPrinter,

    USHORT nDiffFlags)
{
    return maPrintManager.SetPrinter (pNewPrinter, nDiffFlags);
}




PrintDialog* ViewShellBase::CreatePrintDialog (::Window *pParent)
{
    return maPrintManager.CreatePrintDialog (pParent);
}




SfxTabPage*  ViewShellBase::CreatePrintOptionsPage(
    ::Window *pParent,
    const SfxItemSet &rOptions)
{
    return maPrintManager.CreatePrintOptionsPage (pParent, rOptions);
}




USHORT  ViewShellBase::Print(SfxProgress& rProgress, PrintDialog* pDlg)
{
    return maPrintManager.Print (rProgress, pDlg);
}




ErrCode ViewShellBase::DoPrint (
    SfxPrinter* pPrinter,
    PrintDialog* pPrintDialog,
    BOOL bSilent)
{
    return maPrintManager.DoPrint (pPrinter, pPrintDialog, bSilent);
}




USHORT ViewShellBase::SetPrinterOptDlg (
    SfxPrinter* pNewPrinter,
    USHORT nDiffFlags,
    BOOL bShowDialog)
{
   return maPrintManager.SetPrinterOptDlg (
       pNewPrinter, nDiffFlags, bShowDialog);
}




void ViewShellBase::PreparePrint (PrintDialog* pPrintDialog)
{
    SfxViewShell::PreparePrint (pPrintDialog);
    return maPrintManager.PreparePrint (pPrintDialog);
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
    switch (nSlotId)
    {
        case SID_SWITCH_SHELL:
            mpPaneManager->ExecuteModeChange (rRequest);
            break;

        case SID_LEFT_PANE:
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

        default:
            // Ignore any other slot.
            rRequest.Ignore ();
            break;
    }
}




void ViewShellBase::GetState (SfxItemSet& rSet)
{
    mpPaneManager->GetSlotState (rSet);
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
        pShell->ReadUserDataSequence (rSequence, bBrowse);
}




void ViewShellBase::Activate (BOOL IsMDIActivate)
{
    GetPaneManager().InitPanes ();
}




void ViewShellBase::Deactivate (BOOL IsMDIActivate)
{
}




void ViewShellBase::UIActivate (SvInPlaceObject *pIPObj)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->UIActivate (pIPObj);
}




void ViewShellBase::UIDeactivate (SvInPlaceObject *pIPObj)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->UIDeactivate (pIPObj);
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

    // AdjustPostSizePixel of the view shells has been replaced by Resize.
    /*
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell();
    if (pShell != NULL)
        pShell->AdjustPosSizePixel (rOfs, rSize);
    */
}




void ViewShellBase::SetBusyState (bool bBusy)
{
    if (GetDocShell() != NULL)
        GetDocShell()->SetWaitCursor (bBusy);
}




void ViewShellBase::UpdateBorder (void)
{
    ViewShell* pMainViewShell = GetMainViewShell();
    if (pMainViewShell != NULL)
    {
        bool bOuterResize ( ! GetDocShell()->IsInPlaceActive());
        SvBorder aBorder (pMainViewShell->GetBorder(bOuterResize));
        aBorder += GetBorder(bOuterResize);
        SetBorderPixel (aBorder);
        InvalidateBorder();
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

        case ViewShell::ST_PREVIEW:
            pNewShell = new PreviewViewShell (
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
