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

#include <boost/property_tree/json_parser.hpp>

#include <ViewShellBase.hxx>
#include <algorithm>
#include <EventMultiplexer.hxx>
#include <cache/SlsPageCacheManager.hxx>
#include <app.hrc>
#include <slideshow.hxx>
#include <unokywds.hxx>
#include <svx/svxids.hrc>
#include <DrawDocShell.hxx>
#include <ViewShellManager.hxx>
#include <DrawController.hxx>
#include <FrameView.hxx>
#include <ViewTabBar.hxx>
#include <ResourceId.hxx>
#include <sfx2/event.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <FormShellManager.hxx>
#include <ToolBarManager.hxx>
#include <Window.hxx>
#include <framework/ConfigurationController.hxx>
#include <framework/ConfigurationChangeEvent.hxx>
#include <DocumentRenderer.hxx>
#include <optsitem.hxx>
#include <sdmod.hxx>

#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <framework/FrameworkHelper.hxx>

#include <sal/log.hxx>
#include <rtl/ref.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svl/whiter.hxx>
#include <svx/theme/ThemeColorChangerCommon.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/notebookbar/SfxNotebookBar.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <editeng/editview.hxx>
#include <tools/svborder.hxx>
#include <o3tl/unreachable.hxx>

#include <fubullet.hxx>
#include <drawview.hxx>

using namespace sd;
#define ShellClass_ViewShellBase
#include <sdslots.hxx>

using ::sd::framework::FrameworkHelper;

using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::drawing::framework;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

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

namespace sd {

class ViewShellBase::Implementation
{
public:
    SdViewOptions maViewOptions;

    /** Main controller of the view shell.  During the switching from one
        stacked shell to another this pointer may be NULL.
    */
    ::rtl::Reference<DrawController> mpController;

    /** The view tab bar is the control for switching between different
        views in one pane.
    */
    ::rtl::Reference<ViewTabBar> mpViewTabBar;

    // contains the complete area of the current view relative to the frame window
    ::tools::Rectangle maClientArea;

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
    std::unique_ptr<FormShellManager> mpFormShellManager;

    explicit Implementation (ViewShellBase& rBase);
    ~Implementation();

    void LateInit();

    /** Show or hide the ViewTabBar.
        @param bShow
            When <TRUE/> then the ViewTabBar is shown, otherwise it is hidden.
    */
    void ShowViewTabBar (bool bShow);

    void SetUserWantsTabBar(bool inValue);
    bool GetUserWantsTabBar() const { return mbUserWantsTabBar; }

    /** Common code of ViewShellBase::OuterResizePixel() and
        ViewShellBase::InnerResizePixel().
    */
    void ResizePixel (
        const Point& rOrigin,
        const Size& rSize,
        bool bOuterResize);

    /** Show or hide the specified pane.  The visibility state is taken
        from the given request.
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
    virtual ~FocusForwardingWindow() override;
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
    SfxViewFrame& _rFrame,
    SfxViewShell*)
    : SfxViewShell(_rFrame, SfxViewShellFlags::HAS_PRINTOPTIONS),
      mpDocShell (nullptr),
      mpDocument (nullptr)
{
    mpImpl.reset(new Implementation(*this));
    mpImpl->mpViewWindow = VclPtr<FocusForwardingWindow>::Create(_rFrame.GetWindow(),*this);
    mpImpl->mpViewWindow->SetBackground(Wallpaper());

    _rFrame.GetWindow().SetBackground(Application::GetSettings().GetStyleSettings().GetLightColor());

    // Set up the members in the correct order.
    if (auto pDrawDocShell = dynamic_cast< DrawDocShell *>( GetViewFrame().GetObjectShell() ))
        mpDocShell = pDrawDocShell;
    if (mpDocShell != nullptr)
        mpDocument = mpDocShell->GetDoc();
    mpImpl->mpViewShellManager = std::make_shared<ViewShellManager>(*this);

    SetWindow(mpImpl->mpViewWindow.get());

    // Hide the window to avoid complaints from Sfx...SwitchViewShell...
    _rFrame.GetWindow().Hide();
}

/** In this destructor the order in which some of the members are destroyed
    (and/or being prepared to being destroyed) is important.  Change it only
    when you know what you are doing.
*/
ViewShellBase::~ViewShellBase()
{
    // Notify other LOK views that we are going away.
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_VIEW_CURSOR_VISIBLE, "visible", "false"_ostr);
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", ""_ostr);
    SfxLokHelper::notifyOtherViews(this, LOK_CALLBACK_GRAPHIC_VIEW_SELECTION, "selection", "EMPTY"_ostr);

    sfx2::SfxNotebookBar::CloseMethod(GetFrame()->GetBindings());

    rtl::Reference<SlideShow> xSlideShow(SlideShow::GetSlideShow(*this));
    if (xSlideShow.is() && xSlideShow->dependsOn(this))
        SlideShow::Stop(*this);
    xSlideShow.clear();

    // Tell the controller that the ViewShellBase is not available anymore.
    if (mpImpl->mpController)
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

    EndListening(GetViewFrame());
    EndListening(*GetDocShell());

    SetWindow(nullptr);

    mpImpl->mpFormShellManager.reset();
}

void ViewShellBase::LateInit (const OUString& rsDefaultView)
{
    StartListening(GetViewFrame(), DuplicateHandling::Prevent);
    StartListening(*GetDocShell(), DuplicateHandling::Prevent);
    mpImpl->LateInit();
    InitializeFramework();

    mpImpl->mpEventMultiplexer = std::make_shared<tools::EventMultiplexer>(*this);

    mpImpl->mpFormShellManager = std::make_unique<FormShellManager>(*this);

    mpImpl->mpToolBarManager = ToolBarManager::Create(
        *this,
        mpImpl->mpEventMultiplexer,
        mpImpl->mpViewShellManager);

    try
    {
        rtl::Reference<::sd::DrawController> xControllerManager (GetDrawController());
        rtl::Reference<::sd::framework::ConfigurationController> xConfigurationController;
        if (xControllerManager)
            xConfigurationController = xControllerManager->getConfigurationController();
        if (xConfigurationController.is())
        {
            OUString sView (rsDefaultView);
            if (sView.isEmpty())
                sView = GetInitialViewShellType();

            FrameworkHelper::Instance(*this);

            // Create the resource ids for the center pane and view.
            const rtl::Reference<framework::ResourceId> xCenterPaneId (
                new ::sd::framework::ResourceId(FrameworkHelper::msCenterPaneURL));
            const rtl::Reference<framework::ResourceId> xCenterViewId (
                new ::sd::framework::ResourceId(sView, xCenterPaneId));

            // Request center pane and view.
            xConfigurationController->requestResourceActivation(xCenterPaneId, framework::ResourceActivationMode::ADD);
            xConfigurationController->requestResourceActivation(xCenterViewId, framework::ResourceActivationMode::REPLACE);

            // Process configuration events synchronously until the center view
            // has been created.
            while ( !xConfigurationController->getResource(xCenterViewId).is()
                    && xConfigurationController->hasPendingRequests())
            {
                xConfigurationController->ProcessEvent();
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
    SdOptions* pOptions = SdModule::get()->GetSdOptions(GetDocument()->GetDocumentType());
    bool bIsTabBarVisible = pOptions->IsTabBarVisible();
    mpImpl->SetUserWantsTabBar( bIsTabBarVisible );
}

std::shared_ptr<ViewShellManager> const & ViewShellBase::GetViewShellManager() const
{
    return mpImpl->mpViewShellManager;
}

std::shared_ptr<ViewShell> ViewShellBase::GetMainViewShell() const
{
    std::shared_ptr<ViewShell> pMainViewShell (
        framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
            ->GetViewShell(framework::FrameworkHelper::msCenterPaneURL));
    if (pMainViewShell == nullptr)
        pMainViewShell = framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))
            ->GetViewShell(framework::FrameworkHelper::msFullScreenPaneURL);
    return pMainViewShell;
}

ViewShellBase* ViewShellBase::GetViewShellBase (SfxViewFrame const * pViewFrame)
{
    ViewShellBase* pBase = nullptr;

    if (pViewFrame != nullptr)
    {
        // Get the view shell for the frame and cast it to
        // sd::ViewShellBase.
        SfxViewShell* pSfxViewShell = pViewFrame->GetViewShell();
        pBase = dynamic_cast< ::sd::ViewShellBase *>( pSfxViewShell );
    }

    return pBase;
}

void ViewShellBase::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SfxViewShell::Notify(rBC, rHint);

    const SfxHintId nHintId = rHint.GetId();

    if (nHintId == SfxHintId::ThisIsAnSfxEventHint)
    {
        switch (static_cast<const SfxEventHint&>(rHint).GetEventId())
        {
            case SfxEventHintId::OpenDoc:
            {
                const sal_uInt16 nStartingSlide
                    = GetDocument() ? GetDocument()->GetStartWithPresentation() : 0;
                if (nStartingSlide)
                {
                    SfxUInt16Item aItem(FN_PARAM_1, nStartingSlide);
                    GetViewFrame().GetDispatcher()->ExecuteList(
                        SID_PRESENTATION, SfxCallMode::ASYNCHRON, { &aItem });
                }
                break;
            }
            default:
                break;
        }
    }
    else
    {
        switch (nHintId)
        {
            case SfxHintId::LanguageChanged:
            {
                GetViewFrame().GetBindings().Invalidate(SID_LANGUAGE_STATUS);
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

OUString ViewShellBase::GetSelectionText(bool bCompleteWords, bool /*bOnlyASample*/)
{
    std::shared_ptr<ViewShell> const pMainShell(GetMainViewShell());
    DrawViewShell *const pDrawViewShell(
            dynamic_cast<DrawViewShell*>(pMainShell.get()));
    return pDrawViewShell
        ?   pDrawViewShell->GetSelectionText(bCompleteWords)
        :   SfxViewShell::GetSelectionText(bCompleteWords);
}

bool ViewShellBase::HasSelection(bool bText) const
{
    std::shared_ptr<ViewShell> const pMainShell(GetMainViewShell());
    DrawViewShell *const pDrawViewShell(
            dynamic_cast<DrawViewShell*>(pMainShell.get()));
    return pDrawViewShell
        ?   pDrawViewShell->HasSelection(bText)
        :   SfxViewShell::HasSelection(bText);
}

void ViewShellBase::InnerResizePixel (const Point& rOrigin, const Size &rSize, bool)
{
    Size aObjSize = GetObjectShell()->GetVisArea().GetSize();
    if ( !aObjSize.IsEmpty() )
    {
        SvBorder aBorder( GetBorderPixel() );
        Size aSize( rSize );
        aSize.AdjustWidth( -(aBorder.Left() + aBorder.Right()) );
        aSize.AdjustHeight( -(aBorder.Top() + aBorder.Bottom()) );
        Size aObjSizePixel = mpImpl->mpViewWindow->LogicToPixel(aObjSize, MapMode(MapUnit::Map100thMM));
        SfxViewShell::SetZoomFactor(
            Fraction( aSize.Width(), std::max( aObjSizePixel.Width(), static_cast<::tools::Long>(1) ) ),
            Fraction( aSize.Height(), std::max( aObjSizePixel.Height(), static_cast<::tools::Long>(1)) ) );
    }

    mpImpl->ResizePixel(rOrigin, rSize, false);
}

void ViewShellBase::OuterResizePixel (const Point& rOrigin, const Size &rSize)
{
    mpImpl->ResizePixel (rOrigin, rSize, true);
}

void ViewShellBase::Rearrange()
{
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
        SAL_WARN("sd.view", "Rearrange: window missing");
    }

    GetViewFrame().Resize(true);
}

ErrCode ViewShellBase::DoVerb(sal_Int32 nVerb)
{
    ErrCode aResult = ERRCODE_NONE;

    ::sd::ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        aResult = pShell->DoVerb(nVerb);

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
    OSL_ASSERT(mpImpl != nullptr);

    return GetDocShell()->GetPrinter (bCreate);
}

sal_uInt16 ViewShellBase::SetPrinter (
    SfxPrinter* pNewPrinter,
    SfxPrinterChangeFlags nDiffFlags)
{
    OSL_ASSERT(mpImpl != nullptr);

    GetDocShell()->SetPrinter(pNewPrinter);

    if ( (nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION ||
          nDiffFlags & SfxPrinterChangeFlags::CHG_SIZE) && pNewPrinter  )
    {
        MapMode aMap = pNewPrinter->GetMapMode();
        aMap.SetMapUnit(MapUnit::Map100thMM);
        MapMode aOldMap = pNewPrinter->GetMapMode();
        pNewPrinter->SetMapMode(aMap);
        Size aNewSize = pNewPrinter->GetOutputSize();

        std::shared_ptr<DrawViewShell> pDrawViewShell (
            std::dynamic_pointer_cast<DrawViewShell>(GetMainViewShell()));
        if (pDrawViewShell)
        {
            SdPage* pPage = GetDocument()->GetSdPage(
                0, PageKind::Standard );
            pDrawViewShell->SetPageSizeAndBorder (
                pDrawViewShell->GetPageKind(),
                aNewSize,
                -1,-1,-1,-1,
                false/*bScaleAll*/,
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
            DrawController* pDrawController(GetDrawController());
            if (pDrawController)
            {
                rtl::Reference<framework::ConfigurationController> xConfigurationController (
                    pDrawController->getConfigurationController());
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

        case SID_BOTTOM_PANE_IMPRESS:
            mpImpl->SetPaneVisibility(
                rRequest,
                framework::FrameworkHelper::msBottomImpressPaneURL,
                framework::FrameworkHelper::msNotesPanelViewURL);
            break;

        case SID_TOGGLE_TABBAR_VISIBILITY:
        {
            SdOptions* pOptions = SdModule::get()->GetSdOptions(GetDocument()->GetDocumentType());
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

        case SID_PROTECTPOS:
        case SID_PROTECTSIZE:
        {
            ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
            if (!pDocSh)
                break;
            ::sd::View* pView = pDocSh->GetViewShell()->GetView();

            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            assert ( rMarkList.GetMarkCount() == 1 );

            SdrObject* pGraphicObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            if (nSlotId == SID_PROTECTSIZE)
                pGraphicObj->SetResizeProtect(!pGraphicObj->IsResizeProtect());
            else
                pGraphicObj->SetMoveProtect(!pGraphicObj->IsMoveProtect());

        }
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

    FuBullet::GetSlotState( rSet, nullptr, &GetViewFrame() );
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
    if (pShell == nullptr)
        return;

    pShell->ReadUserDataSequence (rSequence);

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
                case PageKind::Standard:
                    sViewURL = framework::FrameworkHelper::msImpressViewURL;
                    break;
                case PageKind::Notes:
                    sViewURL = framework::FrameworkHelper::msNotesViewURL;
                    break;
                case PageKind::Handout:
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

void ViewShellBase::Activate (bool bIsMDIActivate)
{
    SfxViewShell::Activate(bIsMDIActivate);

    DrawController* pDrawController(GetDrawController());
    if (pDrawController)
    {
        rtl::Reference<framework::ConfigurationController> xConfigurationController (
            pDrawController->getConfigurationController());
        if (xConfigurationController.is())
            xConfigurationController->update();
    }
    GetToolBarManager()->RequestUpdate();
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
        pShell->WriteUserData();
}

void ViewShellBase::ReadUserData (const OUString& rString, bool bBrowse)
{
    SfxViewShell::ReadUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        pShell->ReadUserData();
}

SdrView* ViewShellBase::GetDrawView() const
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetMainViewShell().get();
    if (pShell != nullptr)
        return pShell->GetDrawView ();

    return nullptr;
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
    if (pMainViewShell == nullptr || GetWindow()==nullptr)
        return;

    SvBorder aCurrentBorder (GetBorderPixel());
    bool bOuterResize ( ! GetDocShell()->IsInPlaceActive());
    SvBorder aBorder (GetBorder(bOuterResize));
    aBorder += pMainViewShell->GetBorder();

    if (bForce || (aBorder != aCurrentBorder))
    {
        SetBorderPixel (aBorder);
        InvalidateBorder();
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

OUString ViewShellBase::GetInitialViewShellType() const
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

        css::uno::Any aAny = xViewData->getByIndex(0);
        Sequence<beans::PropertyValue> aProperties;
        if ( ! (aAny >>= aProperties))
            break;

        // Search the properties for the one that tells us what page kind to
        // use.
        auto pProperty = std::find_if(std::cbegin(aProperties), std::cend(aProperties),
            [](const beans::PropertyValue& rProperty) { return rProperty.Name == sUNO_View_PageKind; });
        if (pProperty != std::cend(aProperties))
        {
            sal_Int16 nPageKind = 0;
            pProperty->Value >>= nPageKind;
            switch (static_cast<PageKind>(nPageKind))
            {
                case PageKind::Standard:
                    sRequestedView = FrameworkHelper::msImpressViewURL;
                    break;

                case PageKind::Handout:
                    sRequestedView = FrameworkHelper::msHandoutViewURL;
                    break;

                case PageKind::Notes:
                    sRequestedView = FrameworkHelper::msNotesViewURL;
                    break;

                default:
                    // The page kind is invalid.  This is probably an
                    // error by the caller.  We use the standard type to
                    // keep things going.
                    SAL_WARN( "sd.view", "ViewShellBase::GetInitialViewShellType: invalid page kind");
                    sRequestedView = FrameworkHelper::msImpressViewURL;
                    break;
            }
        }
    }
    while (false);

    return sRequestedView;
}

const SdViewOptions& ViewShellBase::GetViewOptions() const
{
    return mpImpl->maViewOptions;
}

void ViewShellBase::SetViewOptions(const SdViewOptions& rOptions) const
{
    mpImpl->maViewOptions = rOptions;
}

std::shared_ptr<tools::EventMultiplexer> const & ViewShellBase::GetEventMultiplexer() const
{
    OSL_ASSERT(mpImpl != nullptr);
    OSL_ASSERT(mpImpl->mpEventMultiplexer != nullptr);

    return mpImpl->mpEventMultiplexer;
}

const ::tools::Rectangle& ViewShellBase::getClientRectangle() const
{
    return mpImpl->maClientArea;
}

std::shared_ptr<ToolBarManager> const & ViewShellBase::GetToolBarManager() const
{
    OSL_ASSERT(mpImpl != nullptr);
    OSL_ASSERT(mpImpl->mpToolBarManager != nullptr);

    return mpImpl->mpToolBarManager;
}

FormShellManager* ViewShellBase::GetFormShellManager() const
{
    OSL_ASSERT(mpImpl != nullptr);
    OSL_ASSERT(mpImpl->mpFormShellManager != nullptr);

    return mpImpl->mpFormShellManager.get();
}

DrawController* ViewShellBase::GetDrawController() const
{
    OSL_ASSERT(mpImpl != nullptr);

    return mpImpl->mpController.get();
}

void ViewShellBase::SetViewTabBar (const ::rtl::Reference<ViewTabBar>& rViewTabBar)
{
    OSL_ASSERT(mpImpl != nullptr);

    mpImpl->mpViewTabBar = rViewTabBar;
}

vcl::Window* ViewShellBase::GetViewWindow()
{
    OSL_ASSERT(mpImpl != nullptr);

    return mpImpl->mpViewWindow.get();
}

OUString ViewShellBase::RetrieveLabelFromCommand( const OUString& aCmdURL ) const
{
    OUString aModuleName;
    if (SfxViewFrame* pViewFrame = GetMainViewShell()->GetViewFrame())
        aModuleName = vcl::CommandInfoProvider::GetModuleIdentifier(pViewFrame->GetFrame().GetFrameInterface());
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(aCmdURL, aModuleName);
    return vcl::CommandInfoProvider::GetLabelForCommand(aProperties);
}

int ViewShellBase::getPart() const
{
    ViewShell* pViewShell = framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))->GetViewShell(FrameworkHelper::msCenterPaneURL).get();

    if (DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(pViewShell))
    {
        return pDrawViewShell->GetCurPagePos();
    }

    return 0;
}

int ViewShellBase::getEditMode() const
{
    ViewShell* pViewShell = framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))->GetViewShell(FrameworkHelper::msCenterPaneURL).get();

    if (DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(pViewShell))
    {
        switch ( pDrawViewShell->GetEditMode() )
        {
        case EditMode::Page:
            switch (pDrawViewShell->GetPageKind())
            {
                case PageKind::Standard:
                    return 0;
                case PageKind::Notes:
                    return 2;
                default:
                    assert(!"Unhandled page kind");
                    return 0;
            }
        case EditMode::MasterPage:
            switch (pDrawViewShell->GetPageKind())
            {
                case PageKind::Standard:
                    return 1;
                default:
                    assert(!"Unhandled page kind");
                    return 1;
            }
        }
    }

    return 0;
}

void ViewShellBase::setEditMode(int nMode)
{
    ViewShell* pViewShell = framework::FrameworkHelper::Instance(*this)->GetViewShell(FrameworkHelper::msCenterPaneURL).get();

    if (DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(pViewShell))
    {
        EditMode eOrigEditMode = pDrawViewShell->GetEditMode();
        PageKind eOrigPageKind = pDrawViewShell->GetPageKind();
        sal_uInt16 nSelectedPage = pDrawViewShell->GetCurPagePos();

        switch ( nMode )
        {
        case 0:
            pDrawViewShell->SetPageKind(PageKind::Standard);
            pDrawViewShell->ChangeEditMode(EditMode::Page, false);
            break;
        case 1:
            pDrawViewShell->SetPageKind(PageKind::Standard);
            pDrawViewShell->ChangeEditMode(EditMode::MasterPage, false);
            break;
        case 2:
            pDrawViewShell->SetPageKind(PageKind::Notes);
            pDrawViewShell->ChangeEditMode(EditMode::Page, false);
            break;
        }

        /*
           If the EditMode is unchanged, then ChangeEditMode was typically a
           no-op, and an additional explicit SwitchPage is required to reselect
           the equivalent page from the other mode, otherwise a switch from
           e.g. Notes to Standard will still render the still selected Note
           page.
        */
        if (eOrigEditMode == pDrawViewShell->GetEditMode() &&
            eOrigPageKind != pDrawViewShell->GetPageKind())
        {
            pDrawViewShell->SwitchPage(nSelectedPage);
        }
    }
}

void ViewShellBase::afterCallbackRegistered()
{
    // common tasks
    SfxViewShell::afterCallbackRegistered();

    SfxObjectShell* pDocShell = GetObjectShell();
    if (pDocShell)
    {
        std::shared_ptr<model::ColorSet> pThemeColors = pDocShell->GetThemeColors();
        std::set<Color> aDocumentColors = pDocShell->GetDocColors();
        svx::theme::notifyLOK(pThemeColors, aDocumentColors);
    }

    if (comphelper::LibreOfficeKit::isActive() && mpDocument && mpDocument->GetStartWithPresentation())
    {
        // Be consistent with SidebarController, emit JSON.
        boost::property_tree::ptree aTree;
        aTree.put("commandName", ".uno:StartWithPresentation");
        aTree.put("state", "true");
        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, OString(aStream.str()));
    }
}

void ViewShellBase::NotifyCursor(SfxViewShell* pOtherShell) const
{
    ViewShell* pThisShell = framework::FrameworkHelper::Instance(*const_cast<ViewShellBase*>(this))->GetViewShell(FrameworkHelper::msCenterPaneURL).get();

    DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(pThisShell);
    if (!pDrawViewShell)
        return;

    if (this == pOtherShell)
        return;

    DrawView* pDrawView = pDrawViewShell->GetDrawView();
    if (!pDrawView)
        return;

    if (pDrawView->GetTextEditObject())
    {
        // Blinking cursor.
        EditView& rEditView = pDrawView->GetTextEditOutlinerView()->GetEditView();
        rEditView.RegisterOtherShell(pOtherShell);
        rEditView.ShowCursor();
        rEditView.RegisterOtherShell(nullptr);
        // Text selection, if any.
        rEditView.DrawSelectionXOR(pOtherShell);

        // Shape text lock.
        if (OutlinerView* pOutlinerView = pDrawView->GetTextEditOutlinerView())
        {
            ::tools::Rectangle aRectangle = pOutlinerView->GetOutputArea();
            vcl::Window* pWin = pThisShell->GetActiveWindow();
            if (pWin && pWin->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRectangle = o3tl::toTwips(aRectangle, o3tl::Length::mm100);
            OString sRectangle = aRectangle.toString();
            SfxLokHelper::notifyOtherView(pDrawViewShell->GetViewShellBase(), pOtherShell, LOK_CALLBACK_VIEW_LOCK, "rectangle", sRectangle);
        }
    }
    else
    {
        // Graphic selection.
        pDrawView->AdjustMarkHdl(pOtherShell);
    }
}

::Color ViewShellBase::GetColorConfigColor(svtools::ColorConfigEntry nColorType) const
{
    Color aColor;

    const SdViewOptions& rViewOptions = GetViewOptions();
    switch (nColorType)
    {
        case svtools::ColorConfigEntry::DOCCOLOR:
        {
            aColor = rViewOptions.mnDocBackgroundColor;
            break;
        }
        // Should never be called for an unimplemented color type
        default:
        {
            O3TL_UNREACHABLE;
        }
    }

    return aColor;
}

//===== ViewShellBase::Implementation =========================================

ViewShellBase::Implementation::Implementation (ViewShellBase& rBase)
    : mbIsClosing(false),
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
    if (pViewShell == nullptr)
        return;

    FrameView* pFrameView = pViewShell->GetFrameView();
    if (pFrameView == nullptr)
        return;

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
    pHelper->RunOnConfigurationEvent(framework::ConfigurationChangeEventType::ConfigurationUpdateEnd, CurrentPageSetter(mrBase));
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
        aBorder = pMainViewShell->GetBorder();
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

    maClientArea = ::tools::Rectangle(Point(0,0), aViewWindowSize);
}

void ViewShellBase::Implementation::SetPaneVisibility (
    const SfxRequest& rRequest,
    const OUString& rsPaneURL,
    const OUString& rsViewURL)
{
    try
    {
        DrawController* pDrawController(mrBase.GetDrawController());
        if (!pDrawController)
            return;

        rtl::Reference<framework::ResourceId> xPaneId (new sd::framework::ResourceId(
            rsPaneURL));
        rtl::Reference<framework::ResourceId> xViewId (new sd::framework::ResourceId(
            rsViewURL, rsPaneURL));

        // Determine the new visibility state.
        const SfxItemSet* pArguments = rRequest.GetArgs();
        bool bShowChildWindow;
        sal_uInt16 nSlotId = rRequest.GetSlot();
        if (pArguments != nullptr)
            bShowChildWindow = static_cast<const SfxBoolItem&>(
                pArguments->Get(nSlotId)).GetValue();
        else
        {
            rtl::Reference<sd::framework::ConfigurationController> xConfigurationController (
                pDrawController->getConfigurationController());
            if ( ! xConfigurationController.is())
                throw RuntimeException();
            rtl::Reference<framework::Configuration> xConfiguration (
                xConfigurationController->getRequestedConfiguration());
            if ( ! xConfiguration.is())
                throw RuntimeException();

            bShowChildWindow = ! xConfiguration->hasResource(xPaneId);
        }

        // Set the desired visibility state at the current configuration
        // and update it accordingly.
        rtl::Reference<sd::framework::ConfigurationController> xConfigurationController (
            pDrawController->getConfigurationController());
        if ( ! xConfigurationController.is())
            throw RuntimeException();
        if (bShowChildWindow)
        {
            xConfigurationController->requestResourceActivation(
                xPaneId,
                framework::ResourceActivationMode::ADD);
            xConfigurationController->requestResourceActivation(
                xViewId,
                framework::ResourceActivationMode::REPLACE);
        }
        else
            xConfigurationController->requestResourceDeactivation(
                xPaneId);
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sd.view");
    }
}

void ViewShellBase::Implementation::GetSlotState (SfxItemSet& rSet)
{
    try
    {
        // Get some frequently used values.
        DrawController* pDrawController(mrBase.GetDrawController());
        if (!pDrawController)
            return;
        rtl::Reference<sd::framework::ConfigurationController> xConfigurationController (
            pDrawController->getConfigurationController());
        if ( ! xConfigurationController.is())
            throw RuntimeException();
        rtl::Reference<sd::framework::Configuration> xConfiguration (
            xConfigurationController->getRequestedConfiguration());
        if ( ! xConfiguration.is())
            throw RuntimeException();

        SfxWhichIter aSetIterator (rSet);
        sal_uInt16 nItemId (aSetIterator.FirstWhich());

        while (nItemId > 0)
        {
            bool bState (false);
            bool bEnabled;
            rtl::Reference<framework::ResourceId> xResourceId;
            try
            {
                bEnabled = true;
                // Check if the right view is active
                switch (nItemId)
                {
                    case SID_LEFT_PANE_IMPRESS:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msLeftImpressPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_LEFT_PANE_DRAW:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msLeftDrawPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_BOTTOM_PANE_IMPRESS:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msBottomImpressPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_DRAWINGMODE:
                    case SID_NORMAL_MULTI_PANE_GUI:
                    case SID_SLIDE_MASTER_MODE:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msImpressViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_SLIDE_SORTER_MULTI_PANE_GUI:
                    case SID_SLIDE_SORTER_MODE:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msSlideSorterURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_OUTLINE_MODE:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msOutlineViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_HANDOUT_MASTER_MODE:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msHandoutViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_NOTES_MODE:
                    case SID_NOTES_MASTER_MODE:
                        xResourceId = new sd::framework::ResourceId(
                            FrameworkHelper::msNotesViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        bState = xConfiguration->hasResource(xResourceId);
                        break;

                    case SID_TOGGLE_TABBAR_VISIBILITY:
                        bState = GetUserWantsTabBar();
                        break;
                    case SID_PROTECTPOS:
                    case SID_PROTECTSIZE:
                    {
                        ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell*>(SfxObjectShell::Current());
                        if (pDocSh)
                        {
                            ::sd::View* pView = pDocSh->GetViewShell()->GetView();
                            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                            if ( rMarkList.GetMarkCount() == 1 ) // graphic menu only effective on single item
                            {
                                const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                                const SdrObjKind nSdrObjKind = pObj->GetObjIdentifier();

                                if ( nSdrObjKind == SdrObjKind::Graphic )
                                {
                                    if ( nItemId == SID_PROTECTSIZE )
                                    {
                                        bState = pObj->IsResizeProtect();
                                        if ( pObj->IsMoveProtect() )
                                            bEnabled = false;
                                    }
                                    else
                                        bState = pObj->IsMoveProtect();

                                    break;
                                }
                            }
                        }
                        bEnabled = false;
                    }
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

            // Check if edit mode fits too
            if (bState && bEnabled)
            {
                ViewShell* const pCenterViewShell = FrameworkHelper::Instance(mrBase)->GetViewShell(
                    FrameworkHelper::msCenterPaneURL).get();
                DrawViewShell* const pShell = dynamic_cast<DrawViewShell*>(pCenterViewShell);
                if (pShell)
                {
                    switch (nItemId)
                    {
                        case SID_DRAWINGMODE:
                        case SID_NORMAL_MULTI_PANE_GUI:
                        case SID_NOTES_MODE:
                            bState = pShell->GetEditMode() == EditMode::Page;
                            break;
                        case SID_SLIDE_MASTER_MODE:
                        case SID_NOTES_MASTER_MODE:
                            bState = pShell->GetEditMode() == EditMode::MasterPage;
                            break;
                    }
                }
            }

            // And finally set the state.
            rSet.Put(SfxBoolItem(nItemId, bState));
            if (!bEnabled)
                rSet.DisableItem( nItemId );

            nItemId = aSetIterator.NextWhich();
        }
    }
    catch (const RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd.view");
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

    if (auto pViewShell = mrBase.GetMainViewShell().get())
    {
        pFrameView = pViewShell->GetFrameView();
    }

    if (pFrameView==nullptr)
        return;

    try
    {
        // Get the current page either from the DrawPagesSupplier or the
        // MasterPagesSupplier.
        Any aPage;
        if (pFrameView->GetViewShEditModeOnLoad() == EditMode::Page)
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
        DrawController* pDrawController = mrBase.GetDrawController();
        pDrawController->setPropertyValue (u"CurrentPage"_ustr, aPage);
    }
    catch (const RuntimeException&)
    {
        // We have not been able to set the current page at the main view.
        // This is sad but still leaves us in a valid state.  Therefore,
        // this exception is silently ignored.
    }
    catch (const beans::UnknownPropertyException&)
    {
        SAL_WARN("sd.view", "CurrentPage property unknown");
    }
}

} // end of anonymous namespace

SdViewOptions::SdViewOptions()
    : msColorSchemeName(u"Default"_ustr)
{
    const svtools::ColorConfig& rColorConfig = SdModule::get()->GetColorConfig();
    mnAppBackgroundColor = rColorConfig.GetColorValue(svtools::APPBACKGROUND).nColor;
    mnDocBackgroundColor = rColorConfig.GetColorValue(svtools::DOCCOLOR).nColor;
}

//===== FocusForwardingWindow =================================================

namespace sd {
namespace {

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
    if (pViewShell != nullptr)
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
    if (pViewShell != nullptr)
    {
        vcl::Window* pWindow = pViewShell->GetActiveWindow();
        if (pWindow != nullptr)
        {
            pWindow->Command(rEvent);
        }
    }
}

} // end of anonymous namespace

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
