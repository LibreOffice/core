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

#include <comphelper/processfactory.hxx>

#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>

#include "ViewShellBase.hxx"
#include <algorithm>
#include "EventMultiplexer.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "sdresid.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "unokywds.hxx"
#include <svx/svxids.hrc>
#include "DrawDocShell.hxx"
#include <sfx2/app.hxx>
#include "PaneChildWindows.hxx"
#include "ViewShellManager.hxx"
#include "DrawController.hxx"
#include "FrameView.hxx"
#include "ViewTabBar.hxx"
#include <sfx2/event.hxx>
#include "drawdoc.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include "DrawViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "PresentationViewShell.hxx"
#include "FormShellManager.hxx"
#include "ToolBarManager.hxx"
#include "taskpane/PanelId.hxx"
#include "Window.hxx"
#include "framework/ConfigurationController.hxx"
#include "DocumentRenderer.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include "framework/FrameworkHelper.hxx"

#include <rtl/ref.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <tools/diagnose_ex.h>

#include "fubullet.hxx"

using namespace sd;
#define ViewShellBase
#include "sdslots.hxx"

using ::sd::framework::FrameworkHelper;
using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

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

    // This is set to true when PrepareClose() is called.
    bool mbIsClosing;

    /** The view window is the parent of all UI elements that belong to the
        view or ViewShell.  This comprises the rulers, the scroll bars, and
        the content window.
        It does not include the ViewTabBar.
    */
    ::boost::scoped_ptr< ::Window> mpViewWindow;
    ::boost::shared_ptr<ToolBarManager> mpToolBarManager;
    ::boost::shared_ptr<ViewShellManager> mpViewShellManager;
    ::boost::shared_ptr<tools::EventMultiplexer> mpEventMultiplexer;
    ::boost::shared_ptr<FormShellManager> mpFormShellManager;

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
    virtual void KeyInput (const KeyEvent& rEvent);
    virtual void Command (const CommandEvent& rEvent);

private:
    ViewShellBase& mrBase;
};
} // end of anonymous namespace


//===== ViewShellBase =========================================================

TYPEINIT1(ViewShellBase, SfxViewShell);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new ViewShellBase object has been constructed.

SfxViewFactory* ViewShellBase::pFactory;
SfxViewShell* ViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    ViewShellBase* pBase = new ViewShellBase(pFrame, pOldView);
    pBase->LateInit("");
    return pBase;
}
void ViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory( &CreateInstance,nPrio,"Default" );
    InitFactory();
}
void ViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}



SFX_IMPL_INTERFACE(ViewShellBase, SfxViewShell, SdResId(0))
{
}




ViewShellBase::ViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell*)
    : SfxViewShell (_pFrame,
          SFX_VIEW_CAN_PRINT
        | SFX_VIEW_HAS_PRINTOPTIONS),
      maMutex(),
      mpImpl(),
      mpDocShell (NULL),
      mpDocument (NULL)
{
    mpImpl.reset(new Implementation(*this));
    mpImpl->mpViewWindow.reset(new FocusForwardingWindow(_pFrame->GetWindow(),*this));
    mpImpl->mpViewWindow->SetBackground(Wallpaper());

    _pFrame->GetWindow().SetBackground(Application::GetSettings().GetStyleSettings().GetLightColor());

    // Set up the members in the correct order.
    if (GetViewFrame()->GetObjectShell()->ISA(DrawDocShell))
        mpDocShell = static_cast<DrawDocShell*>(
            GetViewFrame()->GetObjectShell());
    if (mpDocShell != NULL)
        mpDocument = mpDocShell->GetDoc();
    mpImpl->mpViewShellManager.reset(new ViewShellManager(*this));

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

    mpImpl->mpToolBarManager->Shutdown();
    mpImpl->mpViewShellManager->Shutdown();

    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());

    SetWindow(NULL);
}




void ViewShellBase::LateInit (const ::rtl::OUString& rsDefaultView)
{
    StartListening(*GetViewFrame(),sal_True);
    StartListening(*GetDocShell(),sal_True);
    mpImpl->LateInit();
    InitializeFramework();

    mpImpl->mpEventMultiplexer.reset(new tools::EventMultiplexer (*this));

    mpImpl->mpFormShellManager.reset(new FormShellManager(*this));

    mpImpl->mpToolBarManager = ToolBarManager::Create(
        *this,
        mpImpl->mpEventMultiplexer,
        mpImpl->mpViewShellManager);

    try
    {
        Reference<XControllerManager> xControllerManager (GetDrawController(), UNO_QUERY_THROW);
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        if (xConfigurationController.is())
        {
            OUString sView (rsDefaultView);
            if (sView.isEmpty())
                sView = GetInitialViewShellType();

            ::boost::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(*this));

            // Create the resource ids for the center pane and view.
            const Reference<drawing::framework::XResourceId> xCenterPaneId (
                pHelper->CreateResourceId(FrameworkHelper::msCenterPaneURL));
            const Reference<drawing::framework::XResourceId> xCenterViewId (
                pHelper->CreateResourceId(sView, xCenterPaneId));

            // Request center pane and view.
            xConfigurationController->requestResourceActivation(xCenterPaneId, ResourceActivationMode_ADD);
            xConfigurationController->requestResourceActivation(xCenterViewId, ResourceActivationMode_REPLACE);

            // Process configuration events synchronously until the center view
            // has been created.
            sd::framework::ConfigurationController* pConfigurationController
                = dynamic_cast<sd::framework::ConfigurationController*>(xConfigurationController.get());
            if (pConfigurationController != NULL)
            {
                while (
                    ! pConfigurationController->getResource(xCenterViewId).is()
                        && pConfigurationController->hasPendingRequests())
                {
                    pConfigurationController->ProcessEvent();
                }
            }
        }
    }
    catch (const RuntimeException&)
    {
    }

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




::boost::shared_ptr<ViewShellManager> ViewShellBase::GetViewShellManager (void) const
{
    return mpImpl->mpViewShellManager;
}




::boost::shared_ptr<ViewShell> ViewShellBase::GetMainViewShell (void) const
{
    ::boost::shared_ptr<ViewShell> pMainViewShell (
        framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
            ->GetViewShell(framework::FrameworkHelper::msCenterPaneURL));
    if (pMainViewShell.get() == NULL)
        pMainViewShell = framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
            ->GetViewShell(framework::FrameworkHelper::msFullScreenPaneURL);
    return pMainViewShell;
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




DrawDocShell* ViewShellBase::GetDocShell (void) const
{
    return mpDocShell;
}



SdDrawDocument* ViewShellBase::GetDocument (void) const
{
    return mpDocument;
}




void ViewShellBase::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SfxViewShell::Notify(rBC, rHint);

    if (rHint.IsA(TYPE(SfxEventHint)))
    {
        switch (static_cast<const SfxEventHint&>(rHint).GetEventId())
        {
            case SFX_EVENT_OPENDOC:
                if( GetDocument() && GetDocument()->IsStartWithPresentation() )
                {
                    if( GetViewFrame() )
                    {
                        GetViewFrame()->GetDispatcher()->Execute(
                            SID_PRESENTATION, SFX_CALLMODE_ASYNCHRON );
                    }
                }
                break;

            default:
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

    GetViewFrame()->Resize(sal_True);
}




ErrCode ViewShellBase::DoVerb (long nVerb)
{
    ErrCode aResult = ERRCODE_NONE;

    ::sd::ViewShell* pShell = GetMainViewShell().get();
    if (pShell != NULL)
        aResult = pShell->DoVerb (nVerb);

    return aResult;
}




Reference<view::XRenderable> ViewShellBase::GetRenderable (void)
{
    // Create a new DocumentRenderer on every call.  It observes the life
    // time of this ViewShellBase object.
    return Reference<view::XRenderable>(new DocumentRenderer(*this));
}




SfxPrinter* ViewShellBase::GetPrinter (sal_Bool bCreate)
{
    OSL_ASSERT(mpImpl.get()!=NULL);

    return GetDocShell()->GetPrinter (bCreate);
}




sal_uInt16 ViewShellBase::SetPrinter (
    SfxPrinter* pNewPrinter,
    sal_uInt16 nDiffFlags,
    bool bIsAPI)
{
    OSL_ASSERT(mpImpl.get()!=NULL);

    GetDocShell()->SetPrinter(pNewPrinter);

    if ( (nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ||
          nDiffFlags & SFX_PRINTER_CHG_SIZE) && pNewPrinter  )
    {
        MapMode aMap = pNewPrinter->GetMapMode();
        aMap.SetMapUnit(MAP_100TH_MM);
        MapMode aOldMap = pNewPrinter->GetMapMode();
        pNewPrinter->SetMapMode(aMap);
        Size aNewSize = pNewPrinter->GetOutputSize();

        sal_Bool bScaleAll = sal_False;
        if ( bIsAPI )
        {
            WarningBox aWarnBox (
                GetWindow(),
                (WinBits)(WB_YES_NO | WB_DEF_YES),
                String(SdResId(STR_SCALE_OBJS_TO_PAGE)));
            bScaleAll = (aWarnBox.Execute() == RET_YES);
        }

        ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
            ::boost::dynamic_pointer_cast<DrawViewShell>(GetMainViewShell()));
        if (pDrawViewShell)
        {
            SdPage* pPage = GetDocument()->GetSdPage(
                0, PK_STANDARD );
            pDrawViewShell->SetPageSizeAndBorder (
                pDrawViewShell->GetPageKind(),
                aNewSize,
                -1,-1,-1,-1,
                bScaleAll,
                pNewPrinter->GetOrientation(),
                pPage->GetPaperBin(),
                pPage->IsBackgroundFullSize());
        }

        pNewPrinter->SetMapMode(aOldMap);
    }

    return 0;
}




SfxTabPage*  ViewShellBase::CreatePrintOptionsPage(
    ::Window *pParent,
    const SfxItemSet &rOptions)
{
    (void)pParent;
    (void)rOptions;
    return NULL;
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
    if (mpImpl->mpViewTabBar.is() && mpImpl->mpViewTabBar->GetTabControl()->IsVisible())
        nTop = mpImpl->mpViewTabBar->GetHeight();
    return SvBorder(0,nTop,0,0);
}




void ViewShellBase::Execute (SfxRequest& rRequest)
{
    sal_uInt16 nSlotId = rRequest.GetSlot();

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

        case SID_TASKPANE:
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

        case SID_SHOW_TOOL_PANEL:
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
                if (!sViewURL.isEmpty())
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




void ViewShellBase::Activate (sal_Bool bIsMDIActivate)
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
    GetToolBarManager()->RequestUpdate();
}




void ViewShellBase::Deactivate (sal_Bool bIsMDIActivate)
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




sal_uInt16 ViewShellBase::PrepareClose (sal_Bool bUI, sal_Bool bForBrowsing)
{
    sal_uInt16 nResult = SfxViewShell::PrepareClose (bUI, bForBrowsing);

    if (nResult == sal_True)
    {
        mpImpl->mbIsClosing = true;

        // Forward call to main sub shell.
        ViewShell* pShell = GetMainViewShell().get();
        if (pShell != NULL)
            nResult = pShell->PrepareClose (bUI, bForBrowsing);
    }

    return nResult;
}




void ViewShellBase::WriteUserData (String& rString, sal_Bool bBrowse)
{
    SfxViewShell::WriteUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != NULL)
        pShell->WriteUserData (rString);
}




void ViewShellBase::ReadUserData (const String& rString, sal_Bool bBrowse)
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
    // We have to check the existence of the window, too.
    // The SfxViewFrame accesses the window without checking it.
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
        mpImpl->mpViewTabBar->GetTabControl()->Show(bVisible);

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





::boost::shared_ptr<tools::EventMultiplexer> ViewShellBase::GetEventMultiplexer (void)
{
    OSL_ASSERT(mpImpl.get()!=NULL);
    OSL_ASSERT(mpImpl->mpEventMultiplexer.get()!=NULL);

    return mpImpl->mpEventMultiplexer;
}




const Rectangle& ViewShellBase::getClientRectangle (void) const
{
    return mpImpl->maClientArea;
}


::boost::shared_ptr<ToolBarManager> ViewShellBase::GetToolBarManager (void) const
{
    OSL_ASSERT(mpImpl.get()!=NULL);
    OSL_ASSERT(mpImpl->mpToolBarManager.get()!=NULL);

    return mpImpl->mpToolBarManager;
}




::boost::shared_ptr<FormShellManager> ViewShellBase::GetFormShellManager (void) const
{
    OSL_ASSERT(mpImpl.get()!=NULL);
    OSL_ASSERT(mpImpl->mpFormShellManager.get()!=NULL);

    return mpImpl->mpFormShellManager;
}




DrawController& ViewShellBase::GetDrawController (void) const
{
    OSL_ASSERT(mpImpl.get()!=NULL);

    return *mpImpl->mpController;
}




void ViewShellBase::SetViewTabBar (const ::rtl::Reference<ViewTabBar>& rViewTabBar)
{
    OSL_ASSERT(mpImpl.get()!=NULL);

    mpImpl->mpViewTabBar = rViewTabBar;
}




::Window* ViewShellBase::GetViewWindow (void)
{
    OSL_ASSERT(mpImpl.get()!=NULL);

    return mpImpl->mpViewWindow.get();
}


::rtl::OUString ImplRetrieveLabelFromCommand( const Reference< XFrame >& xFrame, const ::rtl::OUString& aCmdURL )
{
    ::rtl::OUString aLabel;

    if ( !aCmdURL.isEmpty() ) try
    {
        Reference< XMultiServiceFactory > xServiceManager( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext(), UNO_QUERY_THROW );

        Reference< XModuleManager2 > xModuleManager( ModuleManager::create(xContext) );
        Reference< XInterface > xIfac( xFrame, UNO_QUERY_THROW );

        ::rtl::OUString aModuleIdentifier( xModuleManager->identify( xIfac ) );

        if( !aModuleIdentifier.isEmpty() )
        {
            Reference< XNameAccess > const xNameAccess(
                    frame::UICommandDescription::create(xContext) );
            Reference< ::com::sun::star::container::XNameAccess > m_xUICommandLabels( xNameAccess->getByName( aModuleIdentifier ), UNO_QUERY_THROW );
            Sequence< PropertyValue > aPropSeq;
            if( m_xUICommandLabels->getByName( aCmdURL ) >>= aPropSeq )
            {
                for( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                {
                    if ( aPropSeq[i].Name == "Name" )
                    {
                        aPropSeq[i].Value >>= aLabel;
                        break;
                    }
                }
            }
        }
    }
    catch (const Exception&)
    {
    }

    return aLabel;
}

::rtl::OUString ViewShellBase::RetrieveLabelFromCommand( const ::rtl::OUString& aCmdURL ) const
{
    Reference< XFrame > xFrame( GetMainViewShell()->GetViewFrame()->GetFrame().GetFrameInterface(), UNO_QUERY );
    return ImplRetrieveLabelFromCommand( xFrame, aCmdURL );
}



//===== ViewShellBase::Implementation =========================================

ViewShellBase::Implementation::Implementation (ViewShellBase& rBase)
    : mpController(),
      mpViewTabBar(),
      maClientArea(),
      mbIsClosing(false),
      mpViewWindow(),
      mpToolBarManager(),
      mpViewShellManager(),
      mpEventMultiplexer(),
      mpFormShellManager(),
      mrBase(rBase),
      mpPageCacheManager(slidesorter::cache::PageCacheManager::Instance())
{
}




ViewShellBase::Implementation::~Implementation (void)
{
    mpController = NULL;
    mpViewTabBar = NULL;
    mpViewWindow.reset();
    mpToolBarManager.reset();
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
            pHelper->RunOnConfigurationEvent("ConfigurationUpdateEnd", CurrentPageSetter(mrBase));
        }
    }
}




void ViewShellBase::Implementation::ShowViewTabBar (bool bShow)
{
    if (mpViewTabBar.is()
        && (mpViewTabBar->GetTabControl()->IsVisible()==sal_True) != bShow)
    {
        mpViewTabBar->GetTabControl()->Show(bShow ? sal_True : sal_False);
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
    if (mpViewTabBar.is() && mpViewTabBar->GetTabControl()->IsVisible())
        mpViewTabBar->GetTabControl()->SetPosSizePixel (rOrigin, rSize);

    // Calculate and set the border before the controls are placed.
    SvBorder aBorder;
    if (pMainViewShell != NULL)
        aBorder = pMainViewShell->GetBorder(bOuterResize);
    aBorder += mrBase.GetBorder(bOuterResize);
    if (mrBase.GetBorderPixel() != aBorder)
        mrBase.SetBorderPixel(aBorder);

    // Place the ViewTabBar at the top.  It is part of the border.
    SvBorder aBaseBorder;
    if (mpViewTabBar.is() && mpViewTabBar->GetTabControl()->IsVisible())
    {
        aBaseBorder.Top() = mpViewTabBar->GetHeight();
        mpViewTabBar->GetTabControl()->SetPosSizePixel(
            rOrigin, Size(rSize.Width(),aBaseBorder.Top()));
    }

    // The view window gets the remaining space.
    Point aViewWindowPosition (
        rOrigin.X()+aBaseBorder.Left(),
        rOrigin.Y()+aBaseBorder.Top());

    // -1 (below) is there to let one line of _pFrame->GetWindow() visible,
    // so that it plays better with the overall look
    Size aViewWindowSize (
        rSize.Width() - aBaseBorder.Left() - aBaseBorder.Right(),
        rSize.Height() - aBaseBorder.Top() - aBaseBorder.Bottom() - 1);
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

        const Reference< XComponentContext > xContext(
            ::comphelper::getProcessComponentContext() );
        Reference<XResourceId> xPaneId (ResourceId::create(
            xContext, rsPaneURL));
        Reference<XResourceId> xViewId (ResourceId::createWithAnchorURL(
            xContext, rsViewURL, rsPaneURL));

        // Determine the new visibility state.
        const SfxItemSet* pArguments = rRequest.GetArgs();
        sal_Bool bShowChildWindow;
        sal_uInt16 nSlotId = rRequest.GetSlot();
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
                xConfigurationController->getRequestedConfiguration());
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
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
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
            xConfigurationController->getRequestedConfiguration());
        if ( ! xConfiguration.is())
            throw RuntimeException();

        const Reference< XComponentContext > xContext(
            ::comphelper::getProcessComponentContext() );
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
                            xContext, FrameworkHelper::msLeftImpressPaneURL);
                        break;

                    case SID_LEFT_PANE_DRAW:
                        xResourceId = ResourceId::create(
                            xContext, FrameworkHelper::msLeftDrawPaneURL);
                        break;

                    case SID_TASKPANE:
                        xResourceId = ResourceId::create(
                            xContext, FrameworkHelper::msRightPaneURL);
                        break;

                    case SID_NORMAL_MULTI_PANE_GUI:
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msImpressViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_SLIDE_SORTER_MULTI_PANE_GUI:
                    case SID_DIAMODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msSlideSorterURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_OUTLINEMODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msOutlineViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_HANDOUTMODE:
                        // There is only the master page mode for the handout
                        // view so ignore the master page flag.
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msHandoutViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    case SID_NOTESMODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msNotesViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        break;

                    default:
                        // Ignore all other items.  They are not meant to be
                        // handled by us.
                        break;
                }
            }
            catch (const DeploymentException&)
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
    catch (const RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

}




void ViewShellBase::Implementation::ProcessTaskPaneSlot (SfxRequest& rRequest)
{
    // Set the visibility state of the toolpanel and one of its top
    // level panels.
    toolpanel::PanelId nPanelId (
        toolpanel::PID_UNKNOWN);
    bool bPanelIdGiven = false;

    // Extract the given arguments.
    const SfxItemSet* pArgs = rRequest.GetArgs();
    if (pArgs)
    {
        if (pArgs->Count() == 2)
        {
            SFX_REQUEST_ARG (rRequest, pPanelId, SfxUInt32Item,
                ID_VAL_PANEL_INDEX, sal_False);
            if (pPanelId != NULL)
            {
                nPanelId = static_cast<
                    toolpanel::PanelId>(
                        pPanelId->GetValue());
                bPanelIdGiven = true;
            }
        }
    }

    // Ignore the request for some combinations of panels and view
    // shell types.
    if (bPanelIdGiven
        && ! (nPanelId==toolpanel::PID_LAYOUT
            && mrBase.GetMainViewShell()!=NULL
            && mrBase.GetMainViewShell()->GetShellType()==ViewShell::ST_OUTLINE))
    {
        framework::FrameworkHelper::Instance(mrBase)->RequestTaskPanel(
            framework::FrameworkHelper::msLayoutTaskPanelURL);
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
            xSet->setPropertyValue (rtl::OUString("CurrentPage"), aPage);
        }
        catch (const RuntimeException&)
        {
            // We have not been able to set the current page at the main view.
            // This is sad but still leaves us in a valid state.  Therefore,
            // this exception is silently ignored.
        }
        catch (const beans::UnknownPropertyException&)
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
    OSL_TRACE("created FocusForwardingWindow at %x", this);
}




FocusForwardingWindow::~FocusForwardingWindow (void)
{
    OSL_TRACE("destroyed FocusForwardingWindow at %x", this);
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




void FocusForwardingWindow::Command (const CommandEvent& rEvent)
{
    ::boost::shared_ptr<ViewShell> pViewShell = mrBase.GetMainViewShell();
    if (pViewShell.get() != NULL)
    {
        ::Window* pWindow = pViewShell->GetActiveWindow();
        if (pWindow != NULL)
        {
            pWindow->Command(rEvent);
        }
    }
}


} // end of anonymouse namespace

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
