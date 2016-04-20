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

#include <comphelper/processfactory.hxx>

#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>

#include "ViewShellBase.hxx"
#include <algorithm>
#include "EventMultiplexer.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "sdresid.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "slideshow.hxx"
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
#include "SidebarPanelId.hxx"
#include "Window.hxx"
#include "framework/ConfigurationController.hxx"
#include "DocumentRenderer.hxx"
#include "sdattr.hxx"
#include "optsitem.hxx"

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
#include <vcl/settings.hxx>

#include <tools/diagnose_ex.h>

#include "fubullet.hxx"

using namespace sd;
#define ViewShellBase
#include "sdslots.hxx"

using ::sd::framework::FrameworkHelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace {

class CurrentPageSetter
{
public:
    explicit CurrentPageSetter (ViewShellBase& rBase);
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
    VclPtr<vcl::Window> mpViewWindow;
    std::shared_ptr<ToolBarManager> mpToolBarManager;
    std::shared_ptr<ViewShellManager> mpViewShellManager;
    std::shared_ptr<tools::EventMultiplexer> mpEventMultiplexer;
    std::shared_ptr<FormShellManager> mpFormShellManager;

    explicit Implementation (ViewShellBase& rBase);
    ~Implementation();

    void LateInit();

    /** Show or hide the ViewTabBar.
        @param bShow
            When <TRUE/> then the ViewTabBar is shown, otherwise it is hidden.
    */
    void ShowViewTabBar (bool bShow);

    void SetUserWantsTabBar(bool inValue);
    inline bool GetUserWantsTabBar() { return mbUserWantsTabBar; }

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
        const OUString& rsPaneURL,
        const OUString& rsViewURL);

    void GetSlotState (SfxItemSet& rSet);

    void ProcessRestoreEditingViewSlot();

private:
    ViewShellBase& mrBase;
    bool mbUserWantsTabBar;
    bool mbTabBarShouldBeVisible;
    /** Hold a reference to the page cache manager of the slide sorter in
        order to ensure that it stays alive while the ViewShellBase is
        alive.
    */
    std::shared_ptr<slidesorter::cache::PageCacheManager> mpPageCacheManager;
};

namespace {
/** The only task of this window is to forward key presses to the content
    window of the main view shell.  With the key press it forwards the focus
    so that it is not called very often.
*/
class FocusForwardingWindow : public vcl::Window
{
public:
    FocusForwardingWindow (vcl::Window& rParentWindow, ViewShellBase& rBase);
    virtual ~FocusForwardingWindow();
    virtual void dispose() override;
    virtual void KeyInput (const KeyEvent& rEvent) override;
    virtual void Command (const CommandEvent& rEvent) override;

private:
    ViewShellBase& mrBase;
};
} // end of anonymous namespace

//===== ViewShellBase =========================================================


// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new ViewShellBase object has been constructed.

SFX_IMPL_SUPERCLASS_INTERFACE(ViewShellBase, SfxViewShell)

void ViewShellBase::InitInterface_Impl()
{
}

ViewShellBase::ViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell*)
    : SfxViewShell (_pFrame,
          SfxViewShellFlags::CAN_PRINT
        | SfxViewShellFlags::HAS_PRINTOPTIONS),
      mpImpl(),
      mpDocShell (nullptr),
      mpDocument (nullptr)
{
    mpImpl.reset(new Implementation(*this));
    mpImpl->mpViewWindow = VclPtr<FocusForwardingWindow>::Create(_pFrame->GetWindow(),*this);
    mpImpl->mpViewWindow->SetBackground(Wallpaper());

    _pFrame->GetWindow().SetBackground(Application::GetSettings().GetStyleSettings().GetLightColor());

    // Set up the members in the correct order.
    if (nullptr != dynamic_cast< DrawDocShell *>( GetViewFrame()->GetObjectShell() ))
        mpDocShell = static_cast<DrawDocShell*>(
            GetViewFrame()->GetObjectShell());
    if (mpDocShell != nullptr)
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
ViewShellBase::~ViewShellBase()
{
    rtl::Reference<SlideShow> xSlideShow(SlideShow::GetSlideShow(*this));
    if (xSlideShow.is() && xSlideShow->dependsOn(this))
        SlideShow::Stop(*this);
    xSlideShow.clear();

    // Tell the controller that the ViewShellBase is not available anymore.
    if (mpImpl->mpController.get() != nullptr)
        mpImpl->mpController->ReleaseViewShellBase();

    // We have to hide the main window to prevent SFX complaining after a
    // reload about it being already visible.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell!=nullptr
        && pShell->GetActiveWindow()!=nullptr
        && pShell->GetActiveWindow()->GetParent()!=nullptr)
    {
        pShell->GetActiveWindow()->GetParent()->Hide();
    }

    mpImpl->mpToolBarManager->Shutdown();
    mpImpl->mpViewShellManager->Shutdown();

    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());

    SetWindow(nullptr);

    mpImpl->mpFormShellManager.reset();
}

void ViewShellBase::LateInit (const OUString& rsDefaultView)
{
    StartListening(*GetViewFrame(),true);
    StartListening(*GetDocShell(),true);
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

            std::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(*this));

            // Create the resource ids for the center pane and view.
            const Reference<drawing::framework::XResourceId> xCenterPaneId (
                FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL));
            const Reference<drawing::framework::XResourceId> xCenterViewId (
                FrameworkHelper::CreateResourceId(sView, xCenterPaneId));

            // Request center pane and view.
            xConfigurationController->requestResourceActivation(xCenterPaneId, ResourceActivationMode_ADD);
            xConfigurationController->requestResourceActivation(xCenterViewId, ResourceActivationMode_REPLACE);

            // Process configuration events synchronously until the center view
            // has been created.
            sd::framework::ConfigurationController* pConfigurationController
                = dynamic_cast<sd::framework::ConfigurationController*>(xConfigurationController.get());
            if (pConfigurationController != nullptr)
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
    if (pViewShell != nullptr)
    {
        FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != nullptr)
            pFrameView->SetViewShellTypeOnLoad(pViewShell->GetShellType());
    }
    // Show/Hide the TabBar
    SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDocument()->GetDocumentType());
    bool bIsTabBarVisible = pOptions->IsTabBarVisible();
    mpImpl->SetUserWantsTabBar( bIsTabBarVisible );
}

std::shared_ptr<ViewShellManager> ViewShellBase::GetViewShellManager() const
{
    return mpImpl->mpViewShellManager;
}

std::shared_ptr<ViewShell> ViewShellBase::GetMainViewShell() const
{
    std::shared_ptr<ViewShell> pMainViewShell (
        framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
            ->GetViewShell(framework::FrameworkHelper::msCenterPaneURL));
    if (pMainViewShell.get() == nullptr)
        pMainViewShell = framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
            ->GetViewShell(framework::FrameworkHelper::msFullScreenPaneURL);
    return pMainViewShell;
}

ViewShellBase* ViewShellBase::GetViewShellBase (SfxViewFrame* pViewFrame)
{
    ViewShellBase* pBase = nullptr;

    if (pViewFrame != nullptr)
    {
        // Get the view shell for the frame and cast it to
        // sd::ViewShellBase.
        SfxViewShell* pSfxViewShell = pViewFrame->GetViewShell();
        if (pSfxViewShell!=nullptr && dynamic_cast< ::sd::ViewShellBase *>( pSfxViewShell ) !=  nullptr)
            pBase = static_cast<ViewShellBase*>(pSfxViewShell);
    }

    return pBase;
}

void ViewShellBase::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SfxViewShell::Notify(rBC, rHint);

    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if (pEventHint)
    {
        switch (pEventHint->GetEventId())
        {
            case SFX_EVENT_OPENDOC:
                if( GetDocument() && GetDocument()->IsStartWithPresentation() )
                {
                    if( GetViewFrame() )
                    {
                        GetViewFrame()->GetDispatcher()->Execute(
                            SID_PRESENTATION, SfxCallMode::ASYNCHRON );
                    }
                }
                break;

            default:
                break;
        }
    }
}

void ViewShellBase::InitializeFramework()
{
}

OUString ViewShellBase::GetSelectionText(bool bCompleteWords)
{
    std::shared_ptr<ViewShell> const pMainShell(GetMainViewShell());
    DrawViewShell *const pDrawViewShell(
            dynamic_cast<DrawViewShell*>(pMainShell.get()));
    return (pDrawViewShell)
        ?   pDrawViewShell->GetSelectionText(bCompleteWords)
        :   SfxViewShell::GetSelectionText(bCompleteWords);
}

bool ViewShellBase::HasSelection(bool bText) const
{
    std::shared_ptr<ViewShell> const pMainShell(GetMainViewShell());
    DrawViewShell *const pDrawViewShell(
            dynamic_cast<DrawViewShell*>(pMainShell.get()));
    return (pDrawViewShell)
        ?   pDrawViewShell->HasSelection(bText)
        :   SfxViewShell::HasSelection(bText);
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

void ViewShellBase::Rearrange()
{
    OSL_ASSERT(GetViewFrame()!=nullptr);

    // There is a bug in the communication between embedded objects and the
    // framework::LayoutManager that leads to missing resize updates.  The
    // following workaround enforces such an update by cycling the border to
    // zero and back to the current value.
    if (GetWindow() != nullptr)
    {
        SetBorderPixel(SvBorder());
        UpdateBorder(true);
    }
    else
    {
        OSL_TRACE("Rearrange: window missing");
    }

    GetViewFrame()->Resize(true);
}

ErrCode ViewShellBase::DoVerb (long nVerb)
{
    ErrCode aResult = ERRCODE_NONE;

    ::sd::ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        aResult = pShell->DoVerb (nVerb);

    return aResult;
}

Reference<view::XRenderable> ViewShellBase::GetRenderable()
{
    // Create a new DocumentRenderer on every call.  It observes the life
    // time of this ViewShellBase object.
    return Reference<view::XRenderable>(new DocumentRenderer(*this));
}

SfxPrinter* ViewShellBase::GetPrinter (bool bCreate)
{
    OSL_ASSERT(mpImpl.get()!=nullptr);

    return GetDocShell()->GetPrinter (bCreate);
}

sal_uInt16 ViewShellBase::SetPrinter (
    SfxPrinter* pNewPrinter,
    SfxPrinterChangeFlags nDiffFlags)
{
    OSL_ASSERT(mpImpl.get()!=nullptr);

    GetDocShell()->SetPrinter(pNewPrinter);

    if ( (nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION ||
          nDiffFlags & SfxPrinterChangeFlags::CHG_SIZE) && pNewPrinter  )
    {
        MapMode aMap = pNewPrinter->GetMapMode();
        aMap.SetMapUnit(MAP_100TH_MM);
        MapMode aOldMap = pNewPrinter->GetMapMode();
        pNewPrinter->SetMapMode(aMap);
        Size aNewSize = pNewPrinter->GetOutputSize();

        bool bScaleAll = false;

        std::shared_ptr<DrawViewShell> pDrawViewShell (
            std::dynamic_pointer_cast<DrawViewShell>(GetMainViewShell()));
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

        case SID_TOGGLE_TABBAR_VISIBILITY:
        {
            SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDocument()->GetDocumentType());
            bool bIsTabBarVisible = pOptions->IsTabBarVisible();
            pOptions->SetTabBarVisible( !bIsTabBarVisible );
            mpImpl->SetUserWantsTabBar( !bIsTabBarVisible );
            rRequest.Done();
        }
        break;

        // draw
        case SID_DRAWINGMODE:
        // impress normal
        case SID_NORMAL_MULTI_PANE_GUI:
        case SID_NOTES_MODE:
        case SID_OUTLINE_MODE:
        case SID_SLIDE_SORTER_MULTI_PANE_GUI:
        case SID_SLIDE_SORTER_MODE:
        // impress master
        case SID_SLIDE_MASTER_MODE:
        case SID_NOTES_MASTER_MODE:
        case SID_HANDOUT_MASTER_MODE:
            framework::FrameworkHelper::Instance(*this)->HandleModeChangeSlot(nSlotId, rRequest);
            break;

        case SID_WIN_FULLSCREEN:
            // The full screen mode is not supported.  Ignore the request.
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

    FuBullet::GetSlotState( rSet, nullptr, GetViewFrame() );
}

void ViewShellBase::WriteUserDataSequence (
    css::uno::Sequence< css::beans::PropertyValue >& rSequence)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        pShell->WriteUserDataSequence (rSequence);
}

void ViewShellBase::ReadUserDataSequence (
    const css::uno::Sequence< css::beans::PropertyValue >& rSequence)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
    {
        pShell->ReadUserDataSequence (rSequence, true/*bBrowse*/);

        // For certain shell types ReadUserDataSequence may have changed the
        // type to another one.  Make sure that the center pane shows the
        // right view shell.
        switch (pShell->GetShellType())
        {
            case ViewShell::ST_IMPRESS:
            case ViewShell::ST_NOTES:
            case ViewShell::ST_HANDOUT:
            {
                OUString sViewURL;
                switch (dynamic_cast<DrawViewShell&>(*pShell).GetPageKind())
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

void ViewShellBase::Activate (bool bIsMDIActivate)
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

void ViewShellBase::Deactivate (bool bIsMDIActivate)
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
    if (pShell != nullptr)
        pShell->SetZoomFactor (rZoomX, rZoomY);
}

bool ViewShellBase::PrepareClose (bool bUI)
{
    bool bResult = SfxViewShell::PrepareClose (bUI);

    if (bResult)
    {
        mpImpl->mbIsClosing = true;

        // Forward call to main sub shell.
        ViewShell* pShell = GetMainViewShell().get();
        if (pShell != nullptr)
            bResult = pShell->PrepareClose (bUI);
    }

    return bResult;
}

void ViewShellBase::WriteUserData (OUString& rString, bool bBrowse)
{
    SfxViewShell::WriteUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        pShell->WriteUserData (rString);
}

void ViewShellBase::ReadUserData (const OUString& rString, bool bBrowse)
{
    SfxViewShell::ReadUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        pShell->ReadUserData (rString);
}

SdrView* ViewShellBase::GetDrawView() const
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
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
    if (GetDocShell() != nullptr)
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
    if (pMainViewShell != nullptr && GetWindow()!=nullptr)
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
    mpImpl->ShowViewTabBar(bVisible);

    ViewShell* pMainViewShell = GetMainViewShell().get();
    if (pMainViewShell != nullptr)
        pMainViewShell->ShowUIControls (bVisible);

    UpdateBorder();
    if (bVisible)
        Rearrange();
}

OUString ViewShellBase::GetInitialViewShellType()
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
        css::uno::Any aAny = xViewData->getByIndex(nView);
        Sequence<beans::PropertyValue> aProperties;
        if ( ! (aAny >>= aProperties))
            break;

        // Search the properties for the one that tells us what page kind to
        // use.
        for (sal_Int32 n=0; n<aProperties.getLength(); n++)
        {
            const beans::PropertyValue& rProperty (aProperties[n]);
            if (rProperty.Name == sUNO_View_PageKind)
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
                        // The page kind is invalid.  This is probably an
                        // error by the caller.  We use the standard type to
                        // keep things going.
                        DBG_ASSERT(false, "ViewShellBase::GetInitialViewShellType: invalid page kind");
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

std::shared_ptr<tools::EventMultiplexer> ViewShellBase::GetEventMultiplexer()
{
    OSL_ASSERT(mpImpl.get()!=nullptr);
    OSL_ASSERT(mpImpl->mpEventMultiplexer.get()!=nullptr);

    return mpImpl->mpEventMultiplexer;
}

const Rectangle& ViewShellBase::getClientRectangle() const
{
    return mpImpl->maClientArea;
}

std::shared_ptr<ToolBarManager> ViewShellBase::GetToolBarManager() const
{
    OSL_ASSERT(mpImpl.get()!=nullptr);
    OSL_ASSERT(mpImpl->mpToolBarManager.get()!=nullptr);

    return mpImpl->mpToolBarManager;
}

std::shared_ptr<FormShellManager> ViewShellBase::GetFormShellManager() const
{
    OSL_ASSERT(mpImpl.get()!=nullptr);
    OSL_ASSERT(mpImpl->mpFormShellManager.get()!=nullptr);

    return mpImpl->mpFormShellManager;
}

DrawController& ViewShellBase::GetDrawController() const
{
    OSL_ASSERT(mpImpl.get()!=nullptr);

    return *mpImpl->mpController;
}

void ViewShellBase::SetViewTabBar (const ::rtl::Reference<ViewTabBar>& rViewTabBar)
{
    OSL_ASSERT(mpImpl.get()!=nullptr);

    mpImpl->mpViewTabBar = rViewTabBar;
}

vcl::Window* ViewShellBase::GetViewWindow()
{
    OSL_ASSERT(mpImpl.get()!=nullptr);

    return mpImpl->mpViewWindow.get();
}

OUString ImplRetrieveLabelFromCommand( const Reference< XFrame >& xFrame, const OUString& aCmdURL )
{
    OUString aLabel;

    if ( !aCmdURL.isEmpty() ) try
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext(), UNO_QUERY_THROW );

        Reference< XModuleManager2 > xModuleManager( ModuleManager::create(xContext) );

        OUString aModuleIdentifier( xModuleManager->identify( Reference<XInterface>( xFrame, UNO_QUERY_THROW ) ) );

        if( !aModuleIdentifier.isEmpty() )
        {
            Reference< XNameAccess > const xNameAccess(
                    frame::theUICommandDescription::get(xContext) );
            Reference< css::container::XNameAccess > xUICommandLabels( xNameAccess->getByName( aModuleIdentifier ), UNO_QUERY_THROW );
            Sequence< PropertyValue > aPropSeq;
            if( xUICommandLabels->getByName( aCmdURL ) >>= aPropSeq )
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

OUString ViewShellBase::RetrieveLabelFromCommand( const OUString& aCmdURL ) const
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
      mbUserWantsTabBar(false),
      mbTabBarShouldBeVisible(true),
      mpPageCacheManager(slidesorter::cache::PageCacheManager::Instance())
{
}

ViewShellBase::Implementation::~Implementation()
{
    mpController = nullptr;
    mpViewTabBar = nullptr;
    mpViewWindow.disposeAndClear();
    mpToolBarManager.reset();
}

void ViewShellBase::Implementation::LateInit()
{
    mpController = new DrawController(mrBase);
}

void ViewShellBase::Implementation::ProcessRestoreEditingViewSlot()
{
    ViewShell* pViewShell = mrBase.GetMainViewShell().get();
    if (pViewShell != nullptr)
    {
        FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != nullptr)
        {
            // Set view shell, edit mode, and page kind.
            // pFrameView->SetViewShEditMode(
            //     pFrameView->GetViewShEditModeOnLoad(),
            //     pFrameView->GetPageKindOnLoad());
            pFrameView->SetViewShEditMode(
                pFrameView->GetViewShEditModeOnLoad() );
            pFrameView->SetPageKind(
                pFrameView->GetPageKindOnLoad());
            std::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(mrBase));
            pHelper->RequestView(
                FrameworkHelper::GetViewURL(pFrameView->GetViewShellTypeOnLoad()),
                FrameworkHelper::msCenterPaneURL);
            pHelper->RunOnConfigurationEvent("ConfigurationUpdateEnd", CurrentPageSetter(mrBase));
        }
    }
}

void ViewShellBase::Implementation::SetUserWantsTabBar(bool inValue)
{
    mbUserWantsTabBar = inValue;
    // Call ShowViewTabBar to refresh the TabBar visibility
    ShowViewTabBar(mbTabBarShouldBeVisible);
}

void ViewShellBase::Implementation::ShowViewTabBar (bool bShow)
{
    mbTabBarShouldBeVisible = bShow;
    bShow = bShow && mbUserWantsTabBar;

    if (mpViewTabBar.is()
        && mpViewTabBar->GetTabControl()->IsVisible() != bShow)
    {
        mpViewTabBar->GetTabControl()->Show(bShow);
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
    if (pMainViewShell != nullptr)
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

    Size aViewWindowSize (
        rSize.Width() - aBaseBorder.Left() - aBaseBorder.Right(),
        rSize.Height() - aBaseBorder.Top() - aBaseBorder.Bottom());
    mpViewWindow->SetPosSizePixel(aViewWindowPosition, aViewWindowSize);

    maClientArea = Rectangle(Point(0,0), aViewWindowSize);
}

void ViewShellBase::Implementation::SetPaneVisibility (
    const SfxRequest& rRequest,
    const OUString& rsPaneURL,
    const OUString& rsViewURL)
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
        bool bShowChildWindow;
        sal_uInt16 nSlotId = rRequest.GetSlot();
        if (pArguments != nullptr)
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

        FrameView *pFrameView;
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
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_LEFT_PANE_DRAW:
                        xResourceId = ResourceId::create(
                            xContext, FrameworkHelper::msLeftDrawPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_NORMAL_MULTI_PANE_GUI:
                        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
                        bState = pFrameView->GetViewShEditMode() == EM_PAGE
                            && pFrameView->GetPageKind() == PK_STANDARD;
                        break;

                    case SID_SLIDE_MASTER_MODE:
                        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
                        bState = pFrameView->GetViewShEditMode() == EM_MASTERPAGE
                            && pFrameView->GetPageKind() == PK_STANDARD;
                        break;

                    case SID_SLIDE_SORTER_MULTI_PANE_GUI:
                    case SID_SLIDE_SORTER_MODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msSlideSorterURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_OUTLINE_MODE:
                        xResourceId = ResourceId::createWithAnchorURL(
                            xContext,
                            FrameworkHelper::msOutlineViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_HANDOUT_MASTER_MODE:
                        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
                        bState = pFrameView->GetViewShEditMode() == EM_MASTERPAGE
                            && pFrameView->GetPageKind() == PK_HANDOUT;
                        break;

                    case SID_NOTES_MODE:
                        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
                        bState = pFrameView->GetViewShEditMode() == EM_PAGE
                            && pFrameView->GetPageKind() == PK_NOTES;
                        break;

                    case SID_NOTES_MASTER_MODE:
                        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
                        bState = pFrameView->GetViewShEditMode() == EM_MASTERPAGE
                            && pFrameView->GetPageKind() == PK_NOTES;
                        break;

                    case SID_TOGGLE_TABBAR_VISIBILITY:
                        bState = GetUserWantsTabBar();
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

            // Take the master page mode into account.
            switch (nItemId)
            {
                case SID_NORMAL_MULTI_PANE_GUI:
                case SID_NOTES_MODE:
                {
                    // Determine the master page mode.
                    ViewShell* pCenterViewShell = FrameworkHelper::Instance(mrBase)->GetViewShell(
                        FrameworkHelper::msCenterPaneURL).get();
                    bool bMasterPageMode (false);
                    if (DrawViewShell* pShell = dynamic_cast<DrawViewShell*>(pCenterViewShell))
                    {
                        if (pShell->GetEditMode() == EM_MASTERPAGE)
                        {
                            bMasterPageMode = true;
                        }
                    }

                    bState &= !bMasterPageMode;
                    break;
                }

                case SID_HANDOUT_MASTER_MODE:
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

} // end of namespace sd

//===== CurrentPageSetter ===========================================

namespace {

CurrentPageSetter::CurrentPageSetter (ViewShellBase& rBase)
    : mrBase(rBase)
{
}

void CurrentPageSetter::operator() (bool)
{
    FrameView* pFrameView = nullptr;

    if (mrBase.GetMainViewShell() != nullptr)
    {
        pFrameView = mrBase.GetMainViewShell()->GetFrameView();
    }

    if (pFrameView!=nullptr)
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
            xSet->setPropertyValue ("CurrentPage", aPage);
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
    vcl::Window& rParentWindow,
    ViewShellBase& rBase)
    : vcl::Window(&rParentWindow, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
        mrBase(rBase)
{
    SAL_INFO("sd.view", "created FocusForwardingWindow at " << this);
}

FocusForwardingWindow::~FocusForwardingWindow()
{
    disposeOnce();
}

void FocusForwardingWindow::dispose()
{
    SAL_INFO("sd.view", "destroyed FocusForwardingWindow at " << this);
    vcl::Window::dispose();
}

void FocusForwardingWindow::KeyInput (const KeyEvent& rKEvt)
{
    std::shared_ptr<ViewShell> pViewShell = mrBase.GetMainViewShell();
    if (pViewShell.get() != nullptr)
    {
        vcl::Window* pWindow = pViewShell->GetActiveWindow();
        if (pWindow != nullptr)
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
    std::shared_ptr<ViewShell> pViewShell = mrBase.GetMainViewShell();
    if (pViewShell.get() != nullptr)
    {
        vcl::Window* pWindow = pViewShell->GetActiveWindow();
        if (pWindow != nullptr)
        {
            pWindow->Command(rEvent);
        }
    }
}

} // end of anonymouse namespace

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
