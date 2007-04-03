/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewShellBase.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:28:12 $
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
 *    modify it under the terms of the GNU Lesser Genberal Public
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

#include "ViewShellBase.hxx"
#include <algorithm>
#include "EventMultiplexer.hxx"
#include "cache/SlsPageCacheManager.hxx"
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
#ifndef SD_DRAW_CONTROLLER_HXX
#include "DrawController.hxx"
#endif
#ifndef SD_PRINT_MANAGER_HXX
#include "PrintManager.hxx"
#endif
#include "UpdateLockManager.hxx"
#include "FrameView.hxx"
#include "ViewTabBar.hxx"
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
#include "ToolBarManager.hxx"
#include "Window.hxx"

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
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_RESOURCEID_HPP_
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#endif
#include "framework/FrameworkHelper.hxx"

#include <rtl/ref.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif
#include <comphelper/processfactory.hxx>

#ifndef SD_FU_BULLET_HXX
#include "fubullet.hxx"
#endif

using namespace sd;
#define ViewShellBase
#include "sdslots.hxx"

using ::sd::framework::FrameworkHelper;
using ::rtl::OUString;

namespace {

class CurrentPageSetter
{
public:
    CurrentPageSetter (ViewShellBase& rBase);
    void operator () (bool);
private:
    ViewShellBase& mrBase;
};

} // end of anonymous namespace


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

namespace sd {

class ViewShellBase::Implementation
{
public:
    ::std::auto_ptr<ToolBarManager> mpToolBarManager;

    /** Main controller of the view shell.  During the switching from one
        stacked shell to another this pointer may be NULL.
    */
    ::rtl::Reference<DrawController> mpController;

    /** The view tab bar is the control for switching between different
        views in one pane.
    */
    ::rtl::Reference<ViewTabBar> mpViewTabBar;

    // contains the complete area of the current view relative to the frame window
    Rectangle maClientArea;

    // This is set to true when PrepareClosing() is called.
    bool mbIsClosing;

    /** The view window is the parent of all UI elements that belong to the
        view or ViewShell.  This comprises the rulers, the scroll bars, and
        the content window.
        It does not include the ViewTabBar.
    */
    ::boost::scoped_ptr< ::Window> mpViewWindow;

    Implementation (ViewShellBase& rBase);
    ~Implementation (void);

    void LateInit (void);

    /** Show or hide the ViewTabBar.
        @param bShow
            When <TRUE/> then the ViewTabBar is shown, otherwise it is hidden.
    */
    void ShowViewTabBar (bool bShow);

    /** Common code of ViewShellBase::OuterResizePixel() and
        ViewShellBase::InnerResizePixel().
    */
    void ResizePixel (
        const Point& rOrigin,
        const Size& rSize,
        bool bOuterResize);

    /** Show or hide the specified pane.  The visibility state is taken
        fromthe given request.
        @param rRequest
            The request determines the new visibility state.  The state can
            either be toggled or be set to a given value.
        @param rsPaneURL
            This URL specifies the pane whose visibility state to set.
        @param rsViewURL
            When the pane becomes visible then this view URL specifies which
            type of view to show in it.
    */
    void SetPaneVisibility (
        const SfxRequest& rRequest,
        const ::rtl::OUString& rsPaneURL,
        const ::rtl::OUString& rsViewURL);

    void GetSlotState (SfxItemSet& rSet);

    void ProcessRestoreEditingViewSlot (void);
    void ProcessTaskPaneSlot (SfxRequest& rRequest);

private:
    ViewShellBase& mrBase;

    /** Hold a reference to the page cache manager of the slide sorter in
        order to ensure that it stays alive while the ViewShellBase is
        alive.
    */
    ::boost::shared_ptr<slidesorter::cache::PageCacheManager> mpPageCacheManager;
};


namespace {
/** The only task of this window is to forward key presses to the content
    window of the main view shell.  With the key press it forwards the focus
    so that it is not called very often.
*/
class FocusForwardingWindow : public ::Window
{
public:
    FocusForwardingWindow (::Window& rParentWindow, ViewShellBase& rBase);
    virtual ~FocusForwardingWindow (void);
    virtual void KeyInput (const KeyEvent& rKEvt);

private:
    ViewShellBase& mrBase;
};
} // end of anonymous namespace


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
    pBase->LateInit(OUString());
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
}




ViewShellBase::ViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell*)
    : SfxViewShell (_pFrame,
        SFX_VIEW_MAXIMIZE_FIRST
        | SFX_VIEW_OPTIMIZE_EACH
        | SFX_VIEW_DISABLE_ACCELS
        | SFX_VIEW_OBJECTSIZE_EMBEDDED
        | SFX_VIEW_CAN_PRINT
        | SFX_VIEW_HAS_PRINTOPTIONS),
      maMutex(),
      mpImpl(),
      mpViewShellManager (NULL),
      mpDocShell (NULL),
      mpDocument (NULL),
      mpPrintManager (new PrintManager(*this)),
      mpFormShellManager(NULL),
      mpEventMultiplexer(NULL),
      mpUpdateLockManager(new UpdateLockManager(*this))
{
    mpImpl.reset(new Implementation(*this));
    mpImpl->mpViewWindow.reset(new FocusForwardingWindow(_pFrame->GetWindow(),*this));
    mpImpl->mpViewWindow->SetBackground(Wallpaper());

    _pFrame->GetWindow().SetBackground(Wallpaper());

    // Set up the members in the correct order.
    if (GetViewFrame()->GetObjectShell()->ISA(DrawDocShell))
        mpDocShell = static_cast<DrawDocShell*>(
            GetViewFrame()->GetObjectShell());
    if (mpDocShell != NULL)
        mpDocument = mpDocShell->GetDoc();
    mpViewShellManager.reset (new ViewShellManager (*this));

    SetWindow(mpImpl->mpViewWindow.get());

    // Hide the window to avoid complaints from Sfx...SwitchViewShell...
    _pFrame->GetWindow().Hide();
}




/** In this destructor the order in which some of the members are destroyed
    (and/or being prepared to being destroyed) is important.  Change it only
    when you know what you are doing.
*/
ViewShellBase::~ViewShellBase (void)
{
    // Tell the controller that the ViewShellBase is not available anymore.
    if (mpImpl->mpController.get() != NULL)
        mpImpl->mpController->ReleaseViewShellBase();

    // We have to hide the main window to prevent SFX complaining after a
    // reload about it being already visible.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell!=NULL
        && pShell->GetActiveWindow()!=NULL
        && pShell->GetActiveWindow()->GetParent()!=NULL)
    {
        pShell->GetActiveWindow()->GetParent()->Hide();
    }

    mpUpdateLockManager->Disable();
    mpImpl->mpToolBarManager->Shutdown();
    mpViewShellManager->Shutdown();

    mpFormShellManager.reset();
    mpEventMultiplexer.reset();

    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());

    mpViewShellManager.reset();
    mpPrintManager.reset();

    SetWindow(NULL);
}




void ViewShellBase::LateInit (const ::rtl::OUString& rsDefaultView)
{
    StartListening(*GetViewFrame(),TRUE);
    StartListening(*GetDocShell(),TRUE);
    mpImpl->LateInit();
    InitializeFramework();
    mpEventMultiplexer.reset (new tools::EventMultiplexer (*this));

    mpFormShellManager = ::std::auto_ptr<FormShellManager>(new FormShellManager(*this));

    mpImpl->mpToolBarManager = ToolBarManager::Create(
        *this,
        *mpEventMultiplexer,
        *mpViewShellManager);

    try
    {
        Reference<XControllerManager> xControllerManager (GetDrawController(), UNO_QUERY_THROW);
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        if (xConfigurationController.is())
        {
            OUString sView (rsDefaultView);
            if (sView.getLength() == 0)
                sView = GetInitialViewShellType();
            ::boost::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(*this));
            pHelper->RequestView(
                sView,
                FrameworkHelper::msCenterPaneURL);
            pHelper->WaitForEvent(
                FrameworkHelper::msConfigurationUpdateEndEvent);
        }
    }
    catch (RuntimeException&)
    {}

    // AutoLayouts have to be ready.
    GetDocument()->StopWorkStartupDelay();

    UpdateBorder();

    // Remember the type of the current main view shell in the frame view.
    ViewShell* pViewShell = GetMainViewShell().get();
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




::boost::shared_ptr<ViewShell> ViewShellBase::GetMainViewShell (void) const
{
    return framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
        ->GetViewShell(framework::FrameworkHelper::msCenterPaneURL);
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




void ViewShellBase::GetMenuState (SfxItemSet& )
{
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
                    //                    mpPaneManager->InitPanes();
                }
                break;

            case SFX_EVENT_CREATEDOC:
                //                mpPaneManager->InitPanes();
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




void ViewShellBase::InitializeFramework (void)
{
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
        Size aObjSizePixel = mpImpl->mpViewWindow->LogicToPixel( aObjSize, MAP_100TH_MM );
        SfxViewShell::SetZoomFactor(
            Fraction( aSize.Width(), std::max( aObjSizePixel.Width(), (long int)1 ) ),
            Fraction( aSize.Height(), std::max( aObjSizePixel.Height(), (long int)1) ) );
    }

    mpImpl->ResizePixel(rOrigin, rSize, false);
}




void ViewShellBase::OuterResizePixel (const Point& rOrigin, const Size &rSize)
{
    mpImpl->ResizePixel (rOrigin, rSize, true);
}




void ViewShellBase::Rearrange (void)
{
    OSL_ASSERT(GetViewFrame()!=NULL);

    // There is a bug in the communication between embedded objects and the
    // framework::LayoutManager that leads to missing resize updates.  The
    // following workaround enforces such an update by cycling the border to
    // zero and back to the current value.
    if (GetWindow() != NULL)
    {
        SetBorderPixel(SvBorder());
        UpdateBorder(true);
    }
    else
    {
        OSL_TRACE("Rearrange: window missing");
    }

    GetViewFrame()->Resize(TRUE);
}




ErrCode ViewShellBase::DoVerb (long nVerb)
{
    ErrCode aResult = ERRCODE_NONE;

    ::sd::ViewShell* pShell = GetMainViewShell().get();
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




USHORT  ViewShellBase::Print(SfxProgress& rProgress, BOOL bIsAPI, PrintDialog* pDlg)
{
    return mpPrintManager->Print (rProgress, bIsAPI, pDlg);
}




ErrCode ViewShellBase::DoPrint (
    SfxPrinter* pPrinter,
    PrintDialog* pPrintDialog,
    BOOL bSilent, BOOL bIsAPI )
{
    return mpPrintManager->DoPrint (pPrinter, pPrintDialog, bSilent, bIsAPI );
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
    mpImpl->ShowViewTabBar(false);

    ViewShell* pViewShell = GetMainViewShell().get();
    if ( pViewShell )
        pViewShell->UIActivating( pClient );

    SfxViewShell::UIActivating( pClient );
}




void ViewShellBase::UIDeactivated( SfxInPlaceClient* pClient )
{
    SfxViewShell::UIDeactivated( pClient );

    mpImpl->ShowViewTabBar(true);

    ViewShell* pViewShell = GetMainViewShell().get();
    if ( pViewShell )
        pViewShell->UIDeactivated( pClient );
}




SvBorder ViewShellBase::GetBorder (bool )
{
    int nTop = 0;
    if (mpImpl->mpViewTabBar.is() && mpImpl->mpViewTabBar->IsVisible())
        nTop = mpImpl->mpViewTabBar->GetHeight();
    return SvBorder(0,nTop,0,0);
}




void ViewShellBase::Execute (SfxRequest& rRequest)
{
    USHORT nSlotId = rRequest.GetSlot();

    switch (nSlotId)
    {
        case SID_SWITCH_SHELL:
        {
            Reference<XControllerManager> xControllerManager (GetController(), UNO_QUERY);
            if (xControllerManager.is())
            {
                Reference<XConfigurationController> xConfigurationController (
                    xControllerManager->getConfigurationController());
                if (xConfigurationController.is())
                    xConfigurationController->update();
            }
        }
        break;

        case SID_LEFT_PANE_DRAW:
            mpImpl->SetPaneVisibility(
                rRequest,
                framework::FrameworkHelper::msLeftDrawPaneURL,
                framework::FrameworkHelper::msSlideSorterURL);
            break;

        case SID_LEFT_PANE_IMPRESS:
            mpImpl->SetPaneVisibility(
                rRequest,
                framework::FrameworkHelper::msLeftImpressPaneURL,
                framework::FrameworkHelper::msSlideSorterURL);
            break;

        case SID_RIGHT_PANE:
            mpImpl->SetPaneVisibility(
                rRequest,
                framework::FrameworkHelper::msRightPaneURL,
                framework::FrameworkHelper::msTaskPaneURL);
            break;

        case SID_NORMAL_MULTI_PANE_GUI:
        case SID_SLIDE_SORTER_MULTI_PANE_GUI:
        case SID_DRAWINGMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
            framework::FrameworkHelper::Instance(*this)->HandleModeChangeSlot(nSlotId, rRequest);
            break;

        case SID_WIN_FULLSCREEN:
            // The full screen mode is not supported.  Ignore the request.
            break;

        case SID_TASK_PANE:
            mpImpl->ProcessTaskPaneSlot(rRequest);
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

    mpImpl->GetSlotState(rSet);

    FuBullet::GetSlotState( rSet, 0, GetViewFrame() );
}




void ViewShellBase::WriteUserDataSequence (
    ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence,
    sal_Bool bBrowse)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != NULL)
        pShell->WriteUserDataSequence (rSequence, bBrowse);
}




void ViewShellBase::ReadUserDataSequence (
    const ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence,
    sal_Bool bBrowse)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
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
                ::rtl::OUString sViewURL;
                switch (PTR_CAST(DrawViewShell, pShell)->GetPageKind())
                {
                    default:
                    case PK_STANDARD:
                        sViewURL = framework::FrameworkHelper::msImpressViewURL;
                        break;
                    case PK_NOTES:
                        sViewURL = framework::FrameworkHelper::msNotesViewURL;
                        break;
                    case PK_HANDOUT:
                        sViewURL = framework::FrameworkHelper::msHandoutViewURL;
                        break;
                }
                if (sViewURL.getLength() > 0)
                    framework::FrameworkHelper::Instance(*this)->RequestView(
                        sViewURL,
                        framework::FrameworkHelper::msCenterPaneURL);
            }
            break;

            default:
                break;
        }
    }
}




void ViewShellBase::Activate (BOOL bIsMDIActivate)
{
    SfxViewShell::Activate(bIsMDIActivate);

    Reference<XControllerManager> xControllerManager (GetController(), UNO_QUERY);
    if (xControllerManager.is())
    {
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        if (xConfigurationController.is())
            xConfigurationController->update();
    }
    GetToolBarManager().RequestUpdate();
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
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != NULL)
        pShell->SetZoomFactor (rZoomX, rZoomY);
}




USHORT ViewShellBase::PrepareClose (BOOL bUI, BOOL bForBrowsing)
{
    USHORT nResult = SfxViewShell::PrepareClose (bUI, bForBrowsing);

    if (nResult == TRUE)
    {
        mpImpl->mbIsClosing = true;

        // Forward call to main sub shell.
        ViewShell* pShell = GetMainViewShell().get();
        if (pShell != NULL)
            nResult = pShell->PrepareClose (bUI, bForBrowsing);
    }

    return nResult;
}




void ViewShellBase::WriteUserData (String& rString, BOOL bBrowse)
{
    SfxViewShell::WriteUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != NULL)
        pShell->WriteUserData (rString);
}




void ViewShellBase::ReadUserData (const String& rString, BOOL bBrowse)
{
    SfxViewShell::ReadUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != NULL)
        pShell->ReadUserData (rString);
}




SdrView* ViewShellBase::GetDrawView (void) const
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
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
    // The following calls to SetBorderPixel() and InvalidateBorder() are
    // made only for the main view shell.  This not only avoids unnecessary
    // calls for the views in side panes but prevents calling an already
    // dying SfxViewShell base class.
    // For issue #140703# we have to check the existence of the window,
    // too.  The SfxTopViewFrame accesses the window without checking it.
    ViewShell* pMainViewShell = GetMainViewShell().get();
    if (pMainViewShell != NULL && GetWindow()!=NULL)
    {
        SvBorder aCurrentBorder (GetBorderPixel());
        bool bOuterResize ( ! GetDocShell()->IsInPlaceActive());
        SvBorder aBorder (GetBorder(bOuterResize));
        aBorder += pMainViewShell->GetBorder(bOuterResize);

        if (bForce || (aBorder != aCurrentBorder))
        {
            SetBorderPixel (aBorder);
            InvalidateBorder();
        }
    }
}




void ViewShellBase::ShowUIControls (bool bVisible)
{
    if (mpImpl->mpViewTabBar.is())
        mpImpl->mpViewTabBar->Show(bVisible);

    ViewShell* pMainViewShell = GetMainViewShell().get();
    if (pMainViewShell != NULL)
        pMainViewShell->ShowUIControls (bVisible);

    UpdateBorder();
    if (bVisible)
        Rearrange();
}




OUString ViewShellBase::GetInitialViewShellType (void)
{
    OUString sRequestedView (FrameworkHelper::msImpressViewURL);

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
                sal_Int16 nPageKind = 0;
                rProperty.Value >>= nPageKind;
                switch ((PageKind)nPageKind)
                {
                    case PK_STANDARD:
                        sRequestedView = FrameworkHelper::msImpressViewURL;
                        break;

                    case PK_HANDOUT:
                        sRequestedView = FrameworkHelper::msHandoutViewURL;
                        break;

                    case PK_NOTES:
                        sRequestedView = FrameworkHelper::msNotesViewURL;
                        break;

                    default:
                        // The page kind is invalid.  This is propably an
                        // error by the caller.  We use the standard type to
                        // keep things going.
                        DBG_ASSERT(sal_False, "ViewShellBase::GetInitialViewShellType: invalid page kind");
                        sRequestedView = FrameworkHelper::msImpressViewURL;
                        break;
                }
                break;
            }
        }
    }
    while (false);

    return sRequestedView;
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




const Rectangle& ViewShellBase::getClientRectangle (void) const
{
    return mpImpl->maClientArea;
}




::boost::shared_ptr<UpdateLockManager> ViewShellBase::GetUpdateLockManager (void) const
{
    return mpUpdateLockManager;
}




ToolBarManager& ViewShellBase::GetToolBarManager (void) const
{
    return *mpImpl->mpToolBarManager;
}




FormShellManager& ViewShellBase::GetFormShellManager (void) const
{
    return *mpFormShellManager;
}




DrawController& ViewShellBase::GetDrawController (void) const
{
    return *mpImpl->mpController;
}




void ViewShellBase::SetViewTabBar (const ::rtl::Reference<ViewTabBar>& rViewTabBar)
{
    mpImpl->mpViewTabBar = rViewTabBar;
}




::Window* ViewShellBase::GetViewWindow (void)
{
    return mpImpl->mpViewWindow.get();
}




//===== ViewShellBase::Implementation =========================================

ViewShellBase::Implementation::Implementation (ViewShellBase& rBase)
    : mpToolBarManager(),
      mpController(),
      mpViewTabBar(),
      maClientArea(),
      mbIsClosing(false),
      mpViewWindow(),
      mrBase(rBase),
      mpPageCacheManager(slidesorter::cache::PageCacheManager::Instance())
{
}




ViewShellBase::Implementation::~Implementation (void)
{
    mpToolBarManager.reset();
    mpController = NULL;
    mpViewTabBar = NULL;
    mpViewWindow.reset();
    mpPageCacheManager.reset();
}




void ViewShellBase::Implementation::LateInit (void)
{
    mpController = new DrawController(mrBase);
}




void ViewShellBase::Implementation::ProcessRestoreEditingViewSlot (void)
{
    ViewShell* pViewShell = mrBase.GetMainViewShell().get();
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
            ::boost::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(mrBase));
            pHelper->RequestView(
                pHelper->GetViewURL(pFrameView->GetViewShellTypeOnLoad()),
                FrameworkHelper::msCenterPaneURL);
            pHelper->RunOnConfigurationEvent(
                ::rtl::OUString::createFromAscii("ConfigurationUpdateEnd"),
                CurrentPageSetter(mrBase));
        }
    }
}




void ViewShellBase::Implementation::ShowViewTabBar (bool bShow)
{
    if (mpViewTabBar.is()
        && (mpViewTabBar->IsVisible()==TRUE) != bShow)
    {
        mpViewTabBar->Show(bShow ? TRUE : FALSE);
        mrBase.Rearrange();
    }
}




void ViewShellBase::Implementation::ResizePixel (
    const Point& rOrigin,
    const Size &rSize,
    bool bOuterResize)
{
    if (mbIsClosing)
        return;

    // Forward the call to both the base class and the main stacked sub
    // shell only when main sub shell exists.
    ViewShell* pMainViewShell = mrBase.GetMainViewShell().get();

    // Set the ViewTabBar temporarily to full size so that, when asked
    // later, it can return its true height.
    mrBase.SetWindow (mpViewWindow.get());
    if (mpViewTabBar.is() && mpViewTabBar->IsVisible())
        mpViewTabBar->SetPosSizePixel (rOrigin, rSize);

    // Calculate and set the border before the controls are placed.
    SvBorder aBorder;
    if (pMainViewShell != NULL)
        aBorder = pMainViewShell->GetBorder(bOuterResize);
    aBorder += mrBase.GetBorder(bOuterResize);
    if (mrBase.GetBorderPixel() != aBorder)
        mrBase.SetBorderPixel(aBorder);

    // Place the ViewTabBar at the top.  It is part of the border.
    SvBorder aBaseBorder;
    if (mpViewTabBar.is() && mpViewTabBar->IsVisible())
    {
        aBaseBorder.Top() = mpViewTabBar->GetHeight();
        mpViewTabBar->SetPosSizePixel (rOrigin, Size(rSize.Width(),aBaseBorder.Top()));
    }

    // The view window gets the remaining space.
    Point aViewWindowPosition (
        rOrigin.X()+aBaseBorder.Left(),
        rOrigin.Y()+aBaseBorder.Top());
    Size aViewWindowSize (
        rSize.Width() - aBaseBorder.Left() - aBaseBorder.Right(),
        rSize.Height() - aBaseBorder.Top() - aBaseBorder.Bottom());
    mpViewWindow->SetPosSizePixel(aViewWindowPosition, aViewWindowSize);

    maClientArea = Rectangle(Point(0,0), aViewWindowSize);
}




void ViewShellBase::Implementation::SetPaneVisibility (
    const SfxRequest& rRequest,
    const ::rtl::OUString& rsPaneURL,
    const ::rtl::OUString& rsViewURL)
{
    try
    {
        Reference<XControllerManager> xControllerManager (mrBase.GetController(), UNO_QUERY_THROW);

        Reference<XResourceId> xPaneId (ResourceId::create(
            comphelper_getProcessComponentContext(), rsPaneURL));
        Reference<XResourceId> xViewId (ResourceId::createWithAnchorURL(
            comphelper_getProcessComponentContext(), rsViewURL, rsPaneURL));

        // Determine the new visibility state.
        const SfxItemSet* pArguments = rRequest.GetArgs();
        BOOL bShowChildWindow;
        USHORT nSlotId = rRequest.GetSlot();
        if (pArguments != NULL)
            bShowChildWindow = static_cast<const SfxBoolItem&>(
                pArguments->Get(nSlotId)).GetValue();
        else
        {
            Reference<XConfigurationController> xConfigurationController (
                xControllerManager->getConfigurationController());
            if ( ! xConfigurationController.is())
                throw RuntimeException();
            Reference<XConfiguration> xConfiguration (
                xConfigurationController->getConfiguration());
            if ( ! xConfiguration.is())
                throw RuntimeException();

            bShowChildWindow = ! xConfiguration->hasResource(xPaneId);
        }

        // Set the desired visibility state at the current configuration
        // and update it accordingly.
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        if ( ! xConfigurationController.is())
            throw RuntimeException();
        if (bShowChildWindow)
        {
            xConfigurationController->requestResourceActivation(
                xPaneId,
                ResourceActivationMode_ADD);
            xConfigurationController->requestResourceActivation(
                xViewId,
                ResourceActivationMode_REPLACE);
        }
        else
            xConfigurationController->requestResourceDeactivation(
                xPaneId);
    }
    catch (RuntimeException&)
    {
        DBG_ASSERT(false, "ViewShellBase::Implementation::SetPaneVisibility(): caught exception");
    }
}





void ViewShellBase::Implementation::GetSlotState (SfxItemSet& rSet)
{
    try
    {
        // Get some frequently used values.
        Reference<XControllerManager> xControllerManager (mrBase.GetController(), UNO_QUERY_THROW);
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        if ( ! xConfigurationController.is())
            throw RuntimeException();
        Reference<XConfiguration> xConfiguration (
            xConfigurationController->getConfiguration());
        if ( ! xConfiguration.is())
            throw RuntimeException();

        SfxWhichIter aSetIterator (rSet);
        sal_uInt16 nItemId (aSetIterator.FirstWhich());
        while (nItemId > 0)
        {
            bool bState (false);
            Reference<XResourceId> xResourceId;
            try
            {
                switch (nItemId)
                {
                    case SID_LEFT_PANE_IMPRESS:
                        xResourceId = ResourceId::create(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msLeftImpressPaneURL);
                        break;

                    case SID_LEFT_PANE_DRAW:
                        xResourceId = ResourceId::create(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msLeftDrawPaneURL);
                        break;

                    case SID_RIGHT_PANE:
                        xResourceId = ResourceId::create(
                            comphelper_getProcessComponentContext(),
                                FrameworkHelper::msRightPaneURL);
                        break;

                    case SID_NORMAL_MULTI_PANE_GUI:
                        xResourceId = ResourceId::createWithAnchorURL(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msImpressViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_SLIDE_SORTER_MULTI_PANE_GUI:
                    case SID_DIAMODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msSlideSorterURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_OUTLINEMODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msOutlineViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_HANDOUTMODE:
                        // There is only the master page mode for the handout
                        // view so ignore the master page flag.
                        xResourceId = ResourceId::createWithAnchorURL(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msHandoutViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_NOTESMODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            comphelper_getProcessComponentContext(),
                            FrameworkHelper::msNotesViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    default:
                        // Ignore all other items.  They are not meant to be
                        // handled by us.
                        break;
                }
            }
            catch (DeploymentException)
            {
            }

            // Determine the state for the resource.
            bState = xConfiguration->hasResource(xResourceId);

            // Take the master page mode into account.
            switch (nItemId)
            {
                case SID_NORMAL_MULTI_PANE_GUI:
                case SID_NOTESMODE:
                {
                    // Determine the master page mode.
                    ViewShell* pCenterViewShell = FrameworkHelper::Instance(mrBase)->GetViewShell(
                        FrameworkHelper::msCenterPaneURL).get();
                    bool bMasterPageMode (false);
                    if (pCenterViewShell!=NULL && pCenterViewShell->ISA(DrawViewShell))
                        if (PTR_CAST(DrawViewShell,pCenterViewShell)->GetEditMode()
                            == EM_MASTERPAGE)
                        {
                            bMasterPageMode = true;
                        }

                    bState &= !bMasterPageMode;
                    break;
                }

                case SID_HANDOUTMODE:
                    // There is only the master page mode for the handout
                    // view so ignore the master page flag.
                    break;
            }

            // And finally set the state.
            rSet.Put(SfxBoolItem(nItemId, bState));

            nItemId = aSetIterator.NextWhich();
        }
    }
    catch (RuntimeException&)
    {
        DBG_ASSERT(false, "ViewShellBase::Implementation::GetSlotState(): caught exception");
    }

}




void ViewShellBase::Implementation::ProcessTaskPaneSlot (SfxRequest& rRequest)
{
    // Set the visibility state of the toolpanel and one of its top
    // level panels.
    BOOL bShowToolPanel = TRUE;
    toolpanel::TaskPaneViewShell::PanelId nPanelId (
        toolpanel::TaskPaneViewShell::PID_UNKNOWN);
    bool bPanelIdGiven = false;

    // Extract the given arguments.
    const SfxItemSet* pArgs = rRequest.GetArgs();
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
            && mrBase.GetMainViewShell()!=NULL
            && mrBase.GetMainViewShell()->GetShellType()==ViewShell::ST_OUTLINE))
    {
        // Set the visibility of the right pane.
        framework::FrameworkHelper::Instance(mrBase)->RequestView(
            framework::FrameworkHelper::msTaskPaneURL,
            framework::FrameworkHelper::msRightPaneURL);

        if (bShowToolPanel && bPanelIdGiven)
            framework::FrameworkHelper::Instance(mrBase)->RunOnConfigurationEvent(
                framework::FrameworkHelper::msConfigurationUpdateEndEvent,
                toolpanel::PanelActivation(mrBase,nPanelId));
    }
}


} // end of namespace sd




//===== CurrentPageSetter ===========================================

namespace {

CurrentPageSetter::CurrentPageSetter (ViewShellBase& rBase)
    : mrBase(rBase)
{
}





void CurrentPageSetter::operator() (bool)
{
    FrameView* pFrameView = NULL;

    if (mrBase.GetMainViewShell() != NULL)
    {
        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
    }

    if (pFrameView!=NULL)
    {
        try
        {
            // Get the current page either from the DrawPagesSupplier or the
            // MasterPagesSupplier.
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
            // Switch to the page last edited by setting the CurrentPage
            // property.
            Reference<beans::XPropertySet> xSet (mrBase.GetController(), UNO_QUERY_THROW);
            xSet->setPropertyValue (String::CreateFromAscii("CurrentPage"), aPage);
        }
        catch (RuntimeException aException)
        {
            // We have not been able to set the current page at the main view.
            // This is sad but still leaves us in a valid state.  Therefore,
            // this exception is silently ignored.
        }
        catch (beans::UnknownPropertyException aException)
        {
            DBG_ASSERT(false,"CurrentPage property unknown");
        }
    }
}

} // end of anonymouse namespace




//===== FocusForwardingWindow =================================================

namespace sd { namespace {

FocusForwardingWindow::FocusForwardingWindow (
    ::Window& rParentWindow,
    ViewShellBase& rBase)
    : ::Window(&rParentWindow, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
        mrBase(rBase)
{
}




FocusForwardingWindow::~FocusForwardingWindow (void)
{
}




void FocusForwardingWindow::KeyInput (const KeyEvent& rKEvt)
{
    ::boost::shared_ptr<ViewShell> pViewShell = mrBase.GetMainViewShell();
    if (pViewShell.get() != NULL)
    {
        ::Window* pWindow = pViewShell->GetActiveWindow();
        if (pWindow != NULL)
        {
            // Forward the focus so that the window is called directly the
            // next time.
            pWindow->GrabFocus();
            // Forward the key press as well.
            pWindow->KeyInput(rKEvt);
        }
    }
}

} // end of anonymouse namespace

} // end of namespace sd
