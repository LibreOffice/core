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
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/Deck.hxx>
#include <sidebar/DeckDescriptor.hxx>
#include <sidebar/DeckTitleBar.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sidebar/PanelDescriptor.hxx>
#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sidebar/Tools.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <sfx2/sidebar/Context.hxx>
#include <sfx2/viewsh.hxx>


#include <framework/ContextChangeEventMultiplexerTunnel.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <vcl/svapp.hxx>
#include <splitwin.hxx>
#include <tools/diagnose_ex.h>
#include <tools/link.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/lok.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/UI/Sidebar.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>

#include <bitmaps.hlst>

using namespace css;
using namespace css::uno;

namespace
{
    constexpr OUStringLiteral gsReadOnlyCommandName = u".uno:EditDoc";
    const sal_Int32 gnWidthCloseThreshold (70);
    const sal_Int32 gnWidthOpenThreshold (40);

    std::string UnoNameFromDeckId(std::u16string_view rsDeckId, bool isImpress = false)
    {
        if (rsDeckId == u"SdCustomAnimationDeck")
            return ".uno:CustomAnimation";

        if (rsDeckId == u"PropertyDeck")
            return isImpress ? ".uno:ModifyPage" : ".uno:Sidebar";

        if (rsDeckId == u"SdLayoutsDeck")
            return ".uno:ModifyPage";

        if (rsDeckId == u"SdSlideTransitionDeck")
            return ".uno:SlideChangeWindow";

        if (rsDeckId == u"SdAllMasterPagesDeck")
            return ".uno:MasterSlidesPanel";

        if (rsDeckId == u"SdMasterPagesDeck")
            return ".uno:MasterSlidesPanel";

        if (rsDeckId == u"GalleryDeck")
            return ".uno:Gallery";

        return "";
    }
}

namespace sfx2::sidebar {

namespace {

    /** When in doubt, show this deck.
    */
    constexpr OUStringLiteral gsDefaultDeckId(u"PropertyDeck");
}

SidebarController::SidebarController (
    SidebarDockingWindow* pParentWindow,
    const SfxViewFrame* pViewFrame)
    : SidebarControllerInterfaceBase(m_aMutex),
      mpCurrentDeck(),
      mpParentWindow(pParentWindow),
      mpViewFrame(pViewFrame),
      mxFrame(pViewFrame->GetFrame().GetFrameInterface()),
      mpTabBar(VclPtr<TabBar>::Create(
              mpParentWindow,
              mxFrame,
              [this](const OUString& rsDeckId) { return this->OpenThenToggleDeck(rsDeckId); },
              [this](weld::Menu& rMainMenu, weld::Menu& rSubMenu,
                     const ::std::vector<TabBar::DeckMenuData>& rMenuData) { return this->ShowPopupMenu(rMainMenu, rSubMenu, rMenuData); },
              this)),
      maCurrentContext(OUString(), OUString()),
      maRequestedContext(),
      mnRequestedForceFlags(SwitchFlag_NoForce),
      mnMaximumSidebarWidth(officecfg::Office::UI::Sidebar::General::MaximumWidth::get()),
      msCurrentDeckId(gsDefaultDeckId),
      maPropertyChangeForwarder([this](){ return this->BroadcastPropertyChange(); }),
      maContextChangeUpdate([this](){ return this->UpdateConfigurations(); }),
      mbIsDeckRequestedOpen(),
      mbIsDeckOpen(),
      mbFloatingDeckClosed(!pParentWindow->IsFloatingMode()),
      mnSavedSidebarWidth(pParentWindow->GetSizePixel().Width()),
      maFocusManager([this](const Panel& rPanel){ return this->ShowPanel(rPanel); }),
      mxReadOnlyModeDispatch(),
      mbIsDocumentReadOnly(false),
      mpSplitWindow(nullptr),
      mnWidthOnSplitterButtonDown(0),
      mpResourceManager()
{
    // Decks and panel collections for this sidebar
    mpResourceManager = std::make_unique<ResourceManager>();
}

rtl::Reference<SidebarController> SidebarController::create(SidebarDockingWindow* pParentWindow,
                                                            const SfxViewFrame* pViewFrame)
{
    rtl::Reference<SidebarController> instance(new SidebarController(pParentWindow, pViewFrame));

    const css::uno::Reference<css::frame::XFrame>& rxFrame = pViewFrame->GetFrame().GetFrameInterface();
    registerSidebarForFrame(instance.get(), rxFrame->getController());
    rxFrame->addFrameActionListener(instance);
    // Listen for window events.
    instance->mpParentWindow->AddEventListener(LINK(instance.get(), SidebarController, WindowEventHandler));

    // Listen for theme property changes.
    Theme::GetPropertySet()->addPropertyChangeListener(
        "",
        static_cast<css::beans::XPropertyChangeListener*>(instance.get()));

    // Get the dispatch object as preparation to listen for changes of
    // the read-only state.
    const util::URL aURL (Tools::GetURL(gsReadOnlyCommandName));
    instance->mxReadOnlyModeDispatch = Tools::GetDispatch(rxFrame, aURL);
    if (instance->mxReadOnlyModeDispatch.is())
        instance->mxReadOnlyModeDispatch->addStatusListener(instance, aURL);

    //first UpdateConfigurations call will SwitchToDeck

    return instance;
}

SidebarController::~SidebarController()
{
}

SidebarController* SidebarController::GetSidebarControllerForFrame (
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    uno::Reference<frame::XController> const xController(rxFrame->getController());
    if (!xController.is()) // this may happen during dispose of Draw controller but perhaps it's a bug
    {
        SAL_WARN("sfx.sidebar", "GetSidebarControllerForFrame: frame has no XController");
        return nullptr;
    }
    uno::Reference<ui::XContextChangeEventListener> const xListener(
        framework::GetFirstListenerWith(
            ::comphelper::getProcessComponentContext(),
            xController,
            [] (uno::Reference<uno::XInterface> const& xRef)
            { return nullptr != dynamic_cast<SidebarController*>(xRef.get()); }
        ));

    return dynamic_cast<SidebarController*>(xListener.get());
}

void SidebarController::registerSidebarForFrame(SidebarController* pController, const css::uno::Reference<css::frame::XController>& xController)
{
    // Listen for context change events.
    css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    xMultiplexer->addContextChangeEventListener(
        static_cast<css::ui::XContextChangeEventListener*>(pController),
        xController);
}

void SidebarController::unregisterSidebarForFrame(SidebarController* pController, const css::uno::Reference<css::frame::XController>& xController)
{
    pController->saveDeckState();
    pController->disposeDecks();

    css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    xMultiplexer->removeContextChangeEventListener(
        static_cast<css::ui::XContextChangeEventListener*>(pController),
        xController);
}

void SidebarController::disposeDecks()
{
    SolarMutexGuard aSolarMutexGuard;

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (const SfxViewShell* pViewShell = mpViewFrame->GetViewShell())
        {
            const std::string hide = UnoNameFromDeckId(msCurrentDeckId, vcl::EnumContext::Application::Impress == vcl::EnumContext::GetApplicationEnum(GetCurrentContext().msApplication));
            if (!hide.empty())
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                                       (hide + "=false").c_str());
        }

        if (mpParentWindow)
            mpParentWindow->ReleaseLOKNotifier();
    }

    mpCurrentDeck.clear();
    maFocusManager.Clear();
    mpResourceManager->disposeDecks();
}

namespace
{
    class CloseIndicator final : public InterimItemWindow
    {
    public:
        CloseIndicator(vcl::Window* pParent)
            : InterimItemWindow(pParent, "svt/ui/fixedimagecontrol.ui", "FixedImageControl")
            , m_xWidget(m_xBuilder->weld_image("image"))
        {
            InitControlBase(m_xWidget.get());

            m_xWidget->set_from_icon_name(SIDEBAR_CLOSE_INDICATOR);

            SetSizePixel(get_preferred_size());

            SetBackground(Theme::GetColor(Theme::Color_DeckBackground));
        }

        virtual ~CloseIndicator() override
        {
            disposeOnce();
        }

        virtual void dispose() override
        {
            m_xWidget.reset();
            InterimItemWindow::dispose();
        }

    private:
        std::unique_ptr<weld::Image> m_xWidget;
    };
}

void SAL_CALL SidebarController::disposing()
{
    SolarMutexGuard aSolarMutexGuard;

    mpCloseIndicator.disposeAndClear();

    maFocusManager.Clear();
    mpTabBar.disposeAndClear();

    saveDeckState();

    // clear decks
    ResourceManager::DeckContextDescriptorContainer aDecks;

    mpResourceManager->GetMatchingDecks (
            aDecks,
            GetCurrentContext(),
            IsDocumentReadOnly(),
            mxFrame->getController());

    for (const auto& rDeck : aDecks)
    {
        std::shared_ptr<DeckDescriptor> deckDesc = mpResourceManager->GetDeckDescriptor(rDeck.msId);

        VclPtr<Deck> aDeck = deckDesc->mpDeck;
        if (aDeck)
            aDeck.disposeAndClear();
    }

    uno::Reference<css::frame::XController> xController = mxFrame->getController();
    if (!xController.is())
        xController = mxCurrentController;

    mxFrame->removeFrameActionListener(this);
    unregisterSidebarForFrame(this, xController);

    if (mxReadOnlyModeDispatch.is())
        mxReadOnlyModeDispatch->removeStatusListener(this, Tools::GetURL(gsReadOnlyCommandName));
    if (mpSplitWindow != nullptr)
    {
        mpSplitWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpSplitWindow = nullptr;
    }

    if (mpParentWindow != nullptr)
    {
        mpParentWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpParentWindow = nullptr;
    }

    Theme::GetPropertySet()->removePropertyChangeListener(
        "",
        static_cast<css::beans::XPropertyChangeListener*>(this));

    maContextChangeUpdate.CancelRequest();
}

void SAL_CALL SidebarController::notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
{
    SolarMutexGuard aSolarMutexGuard;

    // Update to the requested new context asynchronously to avoid
    // subtle errors caused by SFX2 which in rare cases can not
    // properly handle a synchronous update.

    maRequestedContext = Context(
        rEvent.ApplicationName,
        rEvent.ContextName);

    if (maRequestedContext != maCurrentContext)
    {
        mxCurrentController.set(rEvent.Source, css::uno::UNO_QUERY);
        maContextChangeUpdate.RequestCall(); // async call, not a prob
                                             // calling with held
                                             // solarmutex
        // TODO: this call is redundant but mandatory for unit test to update context on document loading
        UpdateConfigurations();
    }
}

void SAL_CALL SidebarController::disposing (const css::lang::EventObject& )
{
    SolarMutexGuard aSolarMutexGuard;

    dispose();
}

void SAL_CALL SidebarController::propertyChange (const css::beans::PropertyChangeEvent& )
{
    SolarMutexGuard aSolarMutexGuard;

    maPropertyChangeForwarder.RequestCall(); // async call, not a prob
                                             // to call with held
                                             // solarmutex
}

void SAL_CALL SidebarController::statusChanged (const css::frame::FeatureStateEvent& rEvent)
{
    SolarMutexGuard aSolarMutexGuard;

    bool bIsReadWrite (true);
    if (rEvent.IsEnabled)
        rEvent.State >>= bIsReadWrite;

    if (mbIsDocumentReadOnly != !bIsReadWrite)
    {
        mbIsDocumentReadOnly = !bIsReadWrite;

        // Force the current deck to update its panel list.
        if ( ! mbIsDocumentReadOnly)
            SwitchToDefaultDeck();

        mnRequestedForceFlags |= SwitchFlag_ForceSwitch;
        maContextChangeUpdate.RequestCall(); // async call, ok to call
                                             // with held solarmutex
    }
}

void SAL_CALL SidebarController::requestLayout()
{
    SolarMutexGuard aSolarMutexGuard;

    sal_Int32 nMinimalWidth = 0;
    if (mpCurrentDeck && !mpCurrentDeck->isDisposed())
    {
        mpCurrentDeck->RequestLayout();
        nMinimalWidth = mpCurrentDeck->GetMinimalWidth();
    }
    RestrictWidth(nMinimalWidth);
}

void SidebarController::BroadcastPropertyChange()
{
    mpParentWindow->Invalidate(InvalidateFlags::Children);
}

void SidebarController::NotifyResize()
{
    if (!mpTabBar)
    {
        OSL_ASSERT(mpTabBar!=nullptr);
        return;
    }

    const sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth();

    const sal_Int32 nWidth(mpParentWindow->GetSizePixel().Width());
    const sal_Int32 nHeight(mpParentWindow->GetSizePixel().Height());

    mbIsDeckOpen = (nWidth > nTabBarDefaultWidth);

    if (mnSavedSidebarWidth <= 0)
        mnSavedSidebarWidth = nWidth;

    bool bIsDeckVisible;
    const bool bIsOpening (nWidth > mnWidthOnSplitterButtonDown);
    if (bIsOpening)
        bIsDeckVisible = nWidth >= nTabBarDefaultWidth + gnWidthOpenThreshold;
    else
        bIsDeckVisible = nWidth >= nTabBarDefaultWidth + gnWidthCloseThreshold;
    mbIsDeckRequestedOpen = bIsDeckVisible;
    UpdateCloseIndicator(!bIsDeckVisible);

    if (mpCurrentDeck && !mpCurrentDeck->isDisposed())
    {
        SfxSplitWindow* pSplitWindow = GetSplitWindow();
        WindowAlign eAlign = pSplitWindow ? pSplitWindow->GetAlign() : WindowAlign::Right;
        tools::Long nDeckX, nTabX;
        if (eAlign == WindowAlign::Left)     // attach the Sidebar towards the left-side of screen
        {
            nDeckX = nTabBarDefaultWidth;
            nTabX = 0;
        }
        else   // attach the Sidebar towards the right-side of screen
        {
            nDeckX = 0;
            nTabX = nWidth - nTabBarDefaultWidth;
        }

        // Place the deck first.
        if (bIsDeckVisible)
        {
            if (comphelper::LibreOfficeKit::isActive())
            {
                // We want to let the layouter use up as much of the
                // height as necessary to make sure no scrollbar is
                // visible. This only works when there are no greedy
                // panes that fill up all available area. So we only
                // use this for the PropertyDeck, which has no such
                // panes, while most other do. This is fine, since
                // it's the PropertyDeck that really has many panes
                // that can collapse or expand. For others, limit
                // the height to something sensible.
                // tdf#130348: Add special case for ChartDeck, too.
                const sal_Int32 nExtHeight = (msCurrentDeckId == "PropertyDeck" ? 2000 :
                                              (msCurrentDeckId == "ChartDeck" ? 1200 : 600));
                // No TabBar in LOK (use nWidth in full).
                mpCurrentDeck->setPosSizePixel(nDeckX, 0, nWidth, nExtHeight);
            }
            else
                mpCurrentDeck->setPosSizePixel(nDeckX, 0, nWidth - nTabBarDefaultWidth, nHeight);
            mpCurrentDeck->Show();
            mpCurrentDeck->RequestLayout();
        }
        else
            mpCurrentDeck->Hide();

        // Now place the tab bar.
        mpTabBar->setPosSizePixel(nTabX, 0, nTabBarDefaultWidth, nHeight);
        if (!comphelper::LibreOfficeKit::isActive())
            mpTabBar->Show(); // Don't show TabBar in LOK.
    }

    // Determine if the closer of the deck can be shown.
    sal_Int32 nMinimalWidth = 0;
    if (mpCurrentDeck && !mpCurrentDeck->isDisposed())
    {
        DeckTitleBar* pTitleBar = mpCurrentDeck->GetTitleBar();
        if (pTitleBar && pTitleBar->GetVisible())
            pTitleBar->SetCloserVisible(CanModifyChildWindowWidth());
        nMinimalWidth = mpCurrentDeck->GetMinimalWidth();
    }

    RestrictWidth(nMinimalWidth);

    mpParentWindow->NotifyResize();
}

void SidebarController::ProcessNewWidth (const sal_Int32 nNewWidth)
{
    if ( ! mbIsDeckRequestedOpen)
        return;

    if (*mbIsDeckRequestedOpen)
    {
        // Deck became large enough to be shown.  Show it.
        mnSavedSidebarWidth = nNewWidth;
        if (!*mbIsDeckOpen)
            RequestOpenDeck();
    }
    else
    {
        // Deck became too small.  Close it completely.
        // If window is wider than the tab bar then mark the deck as being visible, even when it is not.
        // This is to trigger an adjustment of the width to the width of the tab bar.
        mbIsDeckOpen = true;
        RequestCloseDeck();

        if (mnWidthOnSplitterButtonDown > TabBar::GetDefaultWidth())
            mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
    }
}

void SidebarController::SyncUpdate()
{
    maPropertyChangeForwarder.Sync();
    maContextChangeUpdate.Sync();
}

void SidebarController::UpdateConfigurations()
{
    if (maCurrentContext == maRequestedContext
        && mnRequestedForceFlags == SwitchFlag_NoForce)
        return;

    if ((maCurrentContext.msApplication != "none") &&
            !maCurrentContext.msApplication.isEmpty())
    {
        mpResourceManager->SaveDecksSettings(maCurrentContext);
        mpResourceManager->SetLastActiveDeck(maCurrentContext, msCurrentDeckId);
    }

    // get last active deck for this application on first update
    if (!maRequestedContext.msApplication.isEmpty() &&
            (maCurrentContext.msApplication != maRequestedContext.msApplication))
    {
        OUString sLastActiveDeck = mpResourceManager->GetLastActiveDeck( maRequestedContext );
        if (!sLastActiveDeck.isEmpty())
            msCurrentDeckId = sLastActiveDeck;
    }

    maCurrentContext = maRequestedContext;

    mpResourceManager->InitDeckContext(GetCurrentContext());

    // Find the set of decks that could be displayed for the new context.
    ResourceManager::DeckContextDescriptorContainer aDecks;

    css::uno::Reference<css::frame::XController> xController = mxCurrentController.is() ? mxCurrentController : mxFrame->getController();

    mpResourceManager->GetMatchingDecks (
        aDecks,
        maCurrentContext,
        mbIsDocumentReadOnly,
        xController);

    maFocusManager.Clear();

    // Notify the tab bar about the updated set of decks.
    mpTabBar->SetDecks(aDecks);

    // Find the new deck.  By default that is the same as the old
    // one.  If that is not set or not enabled, then choose the
    // first enabled deck (which is PropertyDeck).
    OUString sNewDeckId;
    for (const auto& rDeck : aDecks)
    {
        if (rDeck.mbIsEnabled)
        {
            if (rDeck.msId == msCurrentDeckId)
            {
                sNewDeckId = msCurrentDeckId;
                break;
            }
            else if (sNewDeckId.getLength() == 0)
                sNewDeckId = rDeck.msId;
        }
    }

    if (sNewDeckId.getLength() == 0)
    {
        // We did not find a valid deck.
        RequestCloseDeck();
        return;
    }

    // Tell the tab bar to highlight the button associated
    // with the deck.
    mpTabBar->HighlightDeck(sNewDeckId);

    std::shared_ptr<DeckDescriptor> xDescriptor = mpResourceManager->GetDeckDescriptor(sNewDeckId);

    if (xDescriptor)
    {
        SwitchToDeck(*xDescriptor, maCurrentContext);
    }
}

namespace {

void collectUIInformation(const OUString& rDeckId)
{
    EventDescription aDescription;
    aDescription.aAction = "SIDEBAR";
    aDescription.aParent = "MainWindow";
    aDescription.aParameters = {{"PANEL", rDeckId}};
    aDescription.aKeyWord = "CurrentApp";

    UITestLogger::getInstance().logEvent(aDescription);
}

}

void SidebarController::OpenThenToggleDeck (
    const OUString& rsDeckId)
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if ( pSplitWindow && !pSplitWindow->IsFadeIn() )
        // tdf#83546 Collapsed sidebar should expand first
        pSplitWindow->FadeIn();
    else if ( IsDeckVisible( rsDeckId ) )
    {
        if( !WasFloatingDeckClosed() )
        {
            // tdf#88241 Summoning an undocked sidebar a second time should close sidebar
            mpParentWindow->Close();
            return;
        }
        else
        {
            // tdf#67627 Clicking a second time on a Deck icon will close the Deck
            RequestCloseDeck();
            return;
        }
    }
    RequestOpenDeck();
    // before SwitchToDeck which may cause the rsDeckId string to be released
    collectUIInformation(rsDeckId);
    SwitchToDeck(rsDeckId);

    // Make sure the sidebar is wide enough to fit the requested content
    if (mpCurrentDeck && mpTabBar)
    {
        sal_Int32 nRequestedWidth = mpCurrentDeck->GetMinimalWidth() + TabBar::GetDefaultWidth();
        if (mnSavedSidebarWidth < nRequestedWidth)
            SetChildWindowWidth(nRequestedWidth);
    }
}

void SidebarController::OpenThenSwitchToDeck (
    std::u16string_view rsDeckId)
{
    RequestOpenDeck();
    SwitchToDeck(rsDeckId);

}

void SidebarController::SwitchToDefaultDeck()
{
    SwitchToDeck(gsDefaultDeckId);
}

void SidebarController::SwitchToDeck (
    std::u16string_view rsDeckId)
{
    if (  msCurrentDeckId != rsDeckId
        || ! mbIsDeckOpen
        || mnRequestedForceFlags!=SwitchFlag_NoForce)
    {
        std::shared_ptr<DeckDescriptor> xDeckDescriptor = mpResourceManager->GetDeckDescriptor(rsDeckId);

        if (xDeckDescriptor)
            SwitchToDeck(*xDeckDescriptor, maCurrentContext);
    }
}

void SidebarController::CreateDeck(std::u16string_view rDeckId) {
    CreateDeck(rDeckId, maCurrentContext);
}

void SidebarController::CreateDeck(std::u16string_view rDeckId, const Context& rContext, bool bForceCreate)
{
    std::shared_ptr<DeckDescriptor> xDeckDescriptor = mpResourceManager->GetDeckDescriptor(rDeckId);

    if (!xDeckDescriptor)
        return;

    VclPtr<Deck> aDeck = xDeckDescriptor->mpDeck;
    if (!aDeck || bForceCreate)
    {
        if (aDeck)
            aDeck.disposeAndClear();

        aDeck = VclPtr<Deck>::Create(
                        *xDeckDescriptor,
                        mpParentWindow,
                        [this]() { return this->RequestCloseDeck(); });
    }
    xDeckDescriptor->mpDeck = aDeck;
    CreatePanels(rDeckId, rContext);
}

void SidebarController::CreatePanels(std::u16string_view rDeckId, const Context& rContext)
{
    std::shared_ptr<DeckDescriptor> xDeckDescriptor = mpResourceManager->GetDeckDescriptor(rDeckId);

    // init panels bounded to that deck, do not wait them being displayed as may be accessed through API

    VclPtr<Deck> pDeck = xDeckDescriptor->mpDeck;

    ResourceManager::PanelContextDescriptorContainer aPanelContextDescriptors;

    css::uno::Reference<css::frame::XController> xController = mxCurrentController.is() ? mxCurrentController : mxFrame->getController();

    mpResourceManager->GetMatchingPanels(
                                        aPanelContextDescriptors,
                                        rContext,
                                        rDeckId,
                                        xController);

    // Update the panel list.
    const sal_Int32 nNewPanelCount (aPanelContextDescriptors.size());
    SharedPanelContainer aNewPanels;
    sal_Int32 nWriteIndex (0);

    aNewPanels.resize(nNewPanelCount);

    for (sal_Int32 nReadIndex=0; nReadIndex<nNewPanelCount; ++nReadIndex)
    {
        const ResourceManager::PanelContextDescriptor& rPanelContexDescriptor (
            aPanelContextDescriptors[nReadIndex]);

        // Determine if the panel can be displayed.
        const bool bIsPanelVisible (!mbIsDocumentReadOnly || rPanelContexDescriptor.mbShowForReadOnlyDocuments);
        if ( ! bIsPanelVisible)
            continue;

        auto xOldPanel(pDeck->GetPanel(rPanelContexDescriptor.msId));
        if (xOldPanel)
        {
            xOldPanel->SetLurkMode(false);
            aNewPanels[nWriteIndex] = xOldPanel;
            xOldPanel->SetExpanded(rPanelContexDescriptor.mbIsInitiallyVisible);
            ++nWriteIndex;
        }
        else
        {
            auto aPanel = CreatePanel(rPanelContexDescriptor.msId,
                                      pDeck->GetPanelParentWindow(),
                                      rPanelContexDescriptor.mbIsInitiallyVisible,
                                      rContext,
                                      pDeck);
            if (aPanel)
            {
                aNewPanels[nWriteIndex] = std::move(aPanel);

                // Depending on the context we have to change the command
                // for the "more options" dialog.
                PanelTitleBar* pTitleBar = aNewPanels[nWriteIndex]->GetTitleBar();
                if (pTitleBar)
                {
                    pTitleBar->SetMoreOptionsCommand(
                        rPanelContexDescriptor.msMenuCommand,
                        mxFrame, xController);
                }
                ++nWriteIndex;
            }
        }
    }

    // mpCurrentPanels - may miss stuff (?)
    aNewPanels.resize(nWriteIndex);
    pDeck->ResetPanels(aNewPanels);
}

void SidebarController::SwitchToDeck (
    const DeckDescriptor& rDeckDescriptor,
    const Context& rContext)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (const SfxViewShell* pViewShell = mpViewFrame->GetViewShell())
        {
            if (msCurrentDeckId != rDeckDescriptor.msId)
            {
                const std::string hide = UnoNameFromDeckId(msCurrentDeckId, vcl::EnumContext::Application::Impress == vcl::EnumContext::GetApplicationEnum(GetCurrentContext().msApplication));
                if (!hide.empty())
                    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                                           (hide + "=false").c_str());
            }

            const std::string show = UnoNameFromDeckId(rDeckDescriptor.msId, vcl::EnumContext::Application::Impress == vcl::EnumContext::GetApplicationEnum(GetCurrentContext().msApplication));
            if (!show.empty())
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                                       (show + "=true").c_str());
        }
    }

    maFocusManager.Clear();

    const bool bForceNewDeck ((mnRequestedForceFlags&SwitchFlag_ForceNewDeck)!=0);
    const bool bForceNewPanels ((mnRequestedForceFlags&SwitchFlag_ForceNewPanels)!=0);
    mnRequestedForceFlags = SwitchFlag_NoForce;

    if (   msCurrentDeckId != rDeckDescriptor.msId
        || bForceNewDeck)
    {
        if (mpCurrentDeck)
            mpCurrentDeck->Hide();

        msCurrentDeckId = rDeckDescriptor.msId;
    }

    mpTabBar->HighlightDeck(msCurrentDeckId);

    // Determine the panels to display in the deck.
    ResourceManager::PanelContextDescriptorContainer aPanelContextDescriptors;

    css::uno::Reference<css::frame::XController> xController = mxCurrentController.is() ? mxCurrentController : mxFrame->getController();

    mpResourceManager->GetMatchingPanels(
        aPanelContextDescriptors,
        rContext,
        rDeckDescriptor.msId,
        xController);

    if (aPanelContextDescriptors.empty())
    {
        // There are no panels to be displayed in the current context.
        if (vcl::EnumContext::GetContextEnum(rContext.msContext) != vcl::EnumContext::Context::Empty)
        {
            // Switch to the "empty" context and try again.
            SwitchToDeck(
                rDeckDescriptor,
                Context(
                    rContext.msApplication,
                    vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Empty)));
            return;
        }
        else
        {
            // This is already the "empty" context. Looks like we have
            // to live with an empty deck.
        }
    }

    // Provide a configuration and Deck object.

    CreateDeck(rDeckDescriptor.msId, rContext, bForceNewDeck);

    if (bForceNewPanels && !bForceNewDeck) // already forced if bForceNewDeck
        CreatePanels(rDeckDescriptor.msId, rContext);

    if (mpCurrentDeck && mpCurrentDeck != rDeckDescriptor.mpDeck)
        mpCurrentDeck->Hide();
    mpCurrentDeck.reset(rDeckDescriptor.mpDeck);

    if ( ! mpCurrentDeck)
        return;

#ifdef DEBUG
    // Show the context name in the deck title bar.
    DeckTitleBar* pDebugTitleBar = mpCurrentDeck->GetTitleBar();
    if (pDebugTitleBar)
        pDebugTitleBar->SetTitle(rDeckDescriptor.msTitle + " (" + maCurrentContext.msContext + ")");
#endif

    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth();
    WindowAlign eAlign = pSplitWindow ? pSplitWindow->GetAlign() : WindowAlign::Right;
    tools::Long nDeckX;
    if (eAlign == WindowAlign::Left)     // attach the Sidebar towards the left-side of screen
    {
        nDeckX = nTabBarDefaultWidth;
    }
    else   // attach the Sidebar towards the right-side of screen
    {
        nDeckX = 0;
    }

    // Activate the deck and the new set of panels.
    mpCurrentDeck->setPosSizePixel(
        nDeckX,
        0,
        mpParentWindow->GetSizePixel().Width() - nTabBarDefaultWidth,
        mpParentWindow->GetSizePixel().Height());

    mpCurrentDeck->Show();

    mpParentWindow->SetText(rDeckDescriptor.msTitle);

    NotifyResize();

    // Tell the focus manager about the new panels and tab bar
    // buttons.
    maFocusManager.SetDeck(mpCurrentDeck);
    maFocusManager.SetPanels(mpCurrentDeck->GetPanels());

    mpTabBar->UpdateFocusManager(maFocusManager);
    UpdateTitleBarIcons();
}

void SidebarController::notifyDeckTitle(std::u16string_view targetDeckId)
{
    if (msCurrentDeckId == targetDeckId)
    {
        maFocusManager.SetDeck(mpCurrentDeck);
        mpTabBar->UpdateFocusManager(maFocusManager);
        UpdateTitleBarIcons();
    }
}

std::shared_ptr<Panel> SidebarController::CreatePanel (
    std::u16string_view rsPanelId,
    weld::Widget* pParentWindow,
    const bool bIsInitiallyExpanded,
    const Context& rContext,
    const VclPtr<Deck>& pDeck)
{
    std::shared_ptr<PanelDescriptor> xPanelDescriptor = mpResourceManager->GetPanelDescriptor(rsPanelId);

    if (!xPanelDescriptor)
        return nullptr;

    // Create the panel which is the parent window of the UIElement.
    auto xPanel = std::make_shared<Panel>(
        *xPanelDescriptor,
        pParentWindow,
        bIsInitiallyExpanded,
        pDeck,
        [this]() { return this->GetCurrentContext(); },
        mxFrame);

    // Create the XUIElement.
    Reference<ui::XUIElement> xUIElement (CreateUIElement(
            xPanel->GetElementParentWindow(),
            xPanelDescriptor->msImplementationURL,
            xPanelDescriptor->mbWantsCanvas,
            rContext));
    if (xUIElement.is())
    {
        // Initialize the panel and add it to the active deck.
        xPanel->SetUIElement(xUIElement);
    }
    else
    {
        xPanel.reset();
    }

    return xPanel;
}

Reference<ui::XUIElement> SidebarController::CreateUIElement (
    const Reference<awt::XWindow>& rxWindow,
    const OUString& rsImplementationURL,
    const bool bWantsCanvas,
    const Context& rContext)
{
    try
    {
        const Reference<XComponentContext> xComponentContext (::comphelper::getProcessComponentContext() );
        const Reference<ui::XUIElementFactory> xUIElementFactory =
               ui::theUIElementFactoryManager::get( xComponentContext );

       // Create the XUIElement.
        ::comphelper::NamedValueCollection aCreationArguments;
        aCreationArguments.put("Frame", makeAny(mxFrame));
        aCreationArguments.put("ParentWindow", makeAny(rxWindow));
        SidebarDockingWindow* pSfxDockingWindow = mpParentWindow.get();
        if (pSfxDockingWindow != nullptr)
            aCreationArguments.put("SfxBindings", makeAny(reinterpret_cast<sal_uInt64>(&pSfxDockingWindow->GetBindings())));
        aCreationArguments.put("Theme", Theme::GetPropertySet());
        aCreationArguments.put("Sidebar", makeAny(Reference<ui::XSidebar>(static_cast<ui::XSidebar*>(this))));
        if (bWantsCanvas)
        {
            Reference<rendering::XSpriteCanvas> xCanvas (VCLUnoHelper::GetWindow(rxWindow)->GetSpriteCanvas());
            aCreationArguments.put("Canvas", makeAny(xCanvas));
        }

        if (mxCurrentController.is())
        {
            OUString aModule = Tools::GetModuleName(mxCurrentController);
            if (!aModule.isEmpty())
            {
                aCreationArguments.put("Module", makeAny(aModule));
            }
            aCreationArguments.put("Controller", makeAny(mxCurrentController));
        }

        aCreationArguments.put("ApplicationName", makeAny(rContext.msApplication));
        aCreationArguments.put("ContextName", makeAny(rContext.msContext));

        Reference<ui::XUIElement> xUIElement(
            xUIElementFactory->createUIElement(
                rsImplementationURL,
                aCreationArguments.getPropertyValues()),
            UNO_SET_THROW);

        return xUIElement;
    }
    catch(const Exception&)
    {
        TOOLS_WARN_EXCEPTION("sfx.sidebar", "Cannot create panel " << rsImplementationURL);
        return nullptr;
    }
}

IMPL_LINK(SidebarController, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetWindow() == mpParentWindow)
    {
        switch (rEvent.GetId())
        {
            case VclEventId::WindowShow:
            case VclEventId::WindowResize:
                NotifyResize();
                break;

            case VclEventId::WindowDataChanged:
                // Force an update of deck and tab bar to reflect
                // changes in theme (high contrast mode).
                Theme::HandleDataChange();
                UpdateTitleBarIcons();
                mpParentWindow->Invalidate();
                mnRequestedForceFlags |= SwitchFlag_ForceNewDeck | SwitchFlag_ForceNewPanels;
                maContextChangeUpdate.RequestCall();
                break;

            case VclEventId::ObjectDying:
                dispose();
                break;

            case VclEventId::WindowPaint:
                SAL_INFO("sfx.sidebar", "Paint");
                break;

            default:
                break;
        }
    }
    else if (rEvent.GetWindow()==mpSplitWindow && mpSplitWindow!=nullptr)
    {
        switch (rEvent.GetId())
        {
            case VclEventId::WindowMouseButtonDown:
                mnWidthOnSplitterButtonDown = mpParentWindow->GetSizePixel().Width();
                break;

            case VclEventId::WindowMouseButtonUp:
            {
                ProcessNewWidth(mpParentWindow->GetSizePixel().Width());
                mnWidthOnSplitterButtonDown = 0;
                break;
            }

            case VclEventId::ObjectDying:
                dispose();
                break;

            default: break;
         }
    }
}

void SidebarController::ShowPopupMenu(
    weld::Menu& rMainMenu, weld::Menu& rSubMenu,
    const ::std::vector<TabBar::DeckMenuData>& rMenuData) const
{
    PopulatePopupMenus(rMainMenu, rSubMenu, rMenuData);
    rMainMenu.connect_activate(LINK(const_cast<SidebarController*>(this), SidebarController, OnMenuItemSelected));
    rSubMenu.connect_activate(LINK(const_cast<SidebarController*>(this), SidebarController, OnSubMenuItemSelected));
}

void SidebarController::PopulatePopupMenus(weld::Menu& rMenu, weld::Menu& rCustomizationMenu,
                                           const std::vector<TabBar::DeckMenuData>& rMenuData) const
{
    // Add one entry for every tool panel element to individually make
    // them visible or hide them.
    sal_Int32 nIndex (0);
    for (const auto& rItem : rMenuData)
    {
        OString sIdent("select" + OString::number(nIndex));
        rMenu.insert(nIndex, OUString::fromUtf8(sIdent), rItem.msDisplayName,
                     nullptr, nullptr, nullptr, TRISTATE_FALSE);
        rMenu.set_active(sIdent, rItem.mbIsCurrentDeck);
        rMenu.set_sensitive(sIdent, rItem.mbIsEnabled && rItem.mbIsActive);

        if (!comphelper::LibreOfficeKit::isActive())
        {
            if (rItem.mbIsCurrentDeck)
            {
                // Don't allow the currently visible deck to be disabled.
                OString sSubIdent("nocustomize" + OString::number(nIndex));
                rCustomizationMenu.insert(nIndex, OUString::fromUtf8(sSubIdent), rItem.msDisplayName,
                                          nullptr, nullptr, nullptr, TRISTATE_FALSE);
                rCustomizationMenu.set_active(sSubIdent, true);
            }
            else
            {
                OString sSubIdent("customize" + OString::number(nIndex));
                rCustomizationMenu.insert(nIndex, OUString::fromUtf8(sSubIdent), rItem.msDisplayName,
                                          nullptr, nullptr, nullptr, TRISTATE_TRUE);
                rCustomizationMenu.set_active(sSubIdent, rItem.mbIsEnabled && rItem.mbIsActive);
            }
        }

        ++nIndex;
    }

    bool bHideLock = true;
    bool bHideUnLock = true;
    // LOK doesn't support docked/undocked; Sidebar is floating but rendered docked in browser.
    if (!comphelper::LibreOfficeKit::isActive())
    {
        // Add entry for docking or un-docking the tool panel.
        if (mpParentWindow->IsFloatingMode())
            bHideLock = false;
        else
            bHideUnLock = false;
    }
    rMenu.set_visible("locktaskpanel", !bHideLock);
    rMenu.set_visible("unlocktaskpanel", !bHideUnLock);

    // No Restore or Customize options for LoKit.
    rMenu.set_visible("customization", !comphelper::LibreOfficeKit::isActive());
}

IMPL_LINK(SidebarController, OnMenuItemSelected, const OString&, rCurItemId, void)
{
    if (rCurItemId == "unlocktaskpanel")
    {
        mpParentWindow->SetFloatingMode(true);
        if (mpParentWindow->IsFloatingMode())
            mpParentWindow->ToTop(ToTopFlags::GrabFocusOnly);
    }
    else if (rCurItemId == "locktaskpanel")
    {
        mpParentWindow->SetFloatingMode(false);
    }
    else if (rCurItemId == "hidesidebar")
    {
        if (!comphelper::LibreOfficeKit::isActive())
        {
            const util::URL aURL(Tools::GetURL(".uno:Sidebar"));
            Reference<frame::XDispatch> xDispatch(Tools::GetDispatch(mxFrame, aURL));
            if (xDispatch.is())
                xDispatch->dispatch(aURL, Sequence<beans::PropertyValue>());
        }
        else
        {
            // In LOK we don't really destroy the sidebar when "closing";
            // we simply hide it. This is because recreating it is problematic
            // See notes in SidebarDockingWindow::NotifyResize().
            RequestCloseDeck();
        }
    }
    else
    {
        try
        {
            OString sNumber;
            if (rCurItemId.startsWith("select", &sNumber))
            {
                RequestOpenDeck();
                SwitchToDeck(mpTabBar->GetDeckIdForIndex(sNumber.toInt32()));
            }
            mpParentWindow->GrabFocusToDocument();
        }
        catch (RuntimeException&)
        {
        }
    }
}

IMPL_LINK(SidebarController, OnSubMenuItemSelected, const OString&, rCurItemId, void)
{
    if (rCurItemId == "restoredefault")
        mpTabBar->RestoreHideFlags();
    else
    {
        try
        {
            OString sNumber;
            if (rCurItemId.startsWith("customize", &sNumber))
            {
                mpTabBar->ToggleHideFlag(sNumber.toInt32());

                // Find the set of decks that could be displayed for the new context.
                ResourceManager::DeckContextDescriptorContainer aDecks;
                mpResourceManager->GetMatchingDecks (
                                            aDecks,
                                            GetCurrentContext(),
                                            IsDocumentReadOnly(),
                                            mxFrame->getController());
                // Notify the tab bar about the updated set of decks.
                maFocusManager.Clear();
                mpTabBar->SetDecks(aDecks);
                mpTabBar->HighlightDeck(mpCurrentDeck->GetId());
                mpTabBar->UpdateFocusManager(maFocusManager);
            }
            mpParentWindow->GrabFocusToDocument();
        }
        catch (RuntimeException&)
        {
        }
    }
}


void SidebarController::RequestCloseDeck()
{
    if (comphelper::LibreOfficeKit::isActive() && mpCurrentDeck)
    {
        const vcl::ILibreOfficeKitNotifier* pNotifier = mpCurrentDeck->GetLOKNotifier();
        auto pMobileNotifier = SfxViewShell::Current();
        const SfxViewShell* pViewShell = SfxViewShell::Current();
        if (pMobileNotifier && pViewShell && pViewShell->isLOKMobilePhone())
        {
            // Mobile phone.
            std::stringstream aStream;
            boost::property_tree::ptree aTree;
            aTree.put("id", mpParentWindow->get_id()); // TODO could be missing - sort out
            aTree.put("type", "dockingwindow");
            aTree.put("text", mpParentWindow->GetText());
            aTree.put("enabled", false);
            boost::property_tree::write_json(aStream, aTree);
            const std::string message = aStream.str();
            pMobileNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
        }
        else if (pNotifier)
            pNotifier->notifyWindow(mpCurrentDeck->GetLOKWindowId(), "close");
    }

    mbIsDeckRequestedOpen = false;
    UpdateDeckOpenState();

    if (!mpCurrentDeck)
        mpTabBar->RemoveDeckHighlight();
}

void SidebarController::RequestOpenDeck()
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if ( pSplitWindow && !pSplitWindow->IsFadeIn() )
        // tdf#83546 Collapsed sidebar should expand first
        pSplitWindow->FadeIn();

    mbIsDeckRequestedOpen = true;
    UpdateDeckOpenState();
}

bool SidebarController::IsDeckOpen(const sal_Int32 nIndex)
{
    if (nIndex >= 0)
    {
        OUString asDeckId(mpTabBar->GetDeckIdForIndex(nIndex));
        return IsDeckVisible(asDeckId);
    }
    return mbIsDeckOpen && *mbIsDeckOpen;
}

bool SidebarController::IsDeckVisible(std::u16string_view rsDeckId)
{
    return mbIsDeckOpen && *mbIsDeckOpen && msCurrentDeckId == rsDeckId;
}

void SidebarController::UpdateDeckOpenState()
{
    if ( ! mbIsDeckRequestedOpen)
        // No state requested.
        return;

    const sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth();

    // Update (change) the open state when it either has not yet been initialized
    // or when its value differs from the requested state.
    if ( mbIsDeckOpen && *mbIsDeckOpen == *mbIsDeckRequestedOpen )
        return;

    if (*mbIsDeckRequestedOpen)
    {
        if (!mpParentWindow->IsFloatingMode())
        {
            if (mnSavedSidebarWidth <= nTabBarDefaultWidth)
                SetChildWindowWidth(SidebarChildWindow::GetDefaultWidth(mpParentWindow));
            else
                SetChildWindowWidth(mnSavedSidebarWidth);
        }
        else
        {
            // Show the Deck by resizing back to the original size (before hiding).
            Size aNewSize(mpParentWindow->GetFloatingWindow()->GetSizePixel());
            Point aNewPos(mpParentWindow->GetFloatingWindow()->GetPosPixel());

            aNewPos.setX(aNewPos.X() - mnSavedSidebarWidth + nTabBarDefaultWidth);
            aNewSize.setWidth(mnSavedSidebarWidth);

            mpParentWindow->GetFloatingWindow()->SetPosSizePixel(aNewPos, aNewSize);

            if (comphelper::LibreOfficeKit::isActive())
            {
                // Sidebar wide enough to render the menu; enable it.
                mpTabBar->EnableMenuButton(true);

                if (const SfxViewShell* pViewShell = mpViewFrame->GetViewShell())
                {
                    const std::string uno = UnoNameFromDeckId(msCurrentDeckId, vcl::EnumContext::Application::Impress == vcl::EnumContext::GetApplicationEnum(GetCurrentContext().msApplication));
                    if (!uno.empty())
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                                                (uno + "=true").c_str());
                }
            }
        }
    }
    else
    {
        if ( ! mpParentWindow->IsFloatingMode())
            mnSavedSidebarWidth = SetChildWindowWidth(nTabBarDefaultWidth);
        else
        {
            // Hide the Deck by resizing to the width of the TabBar.
            Size aNewSize(mpParentWindow->GetFloatingWindow()->GetSizePixel());
            Point aNewPos(mpParentWindow->GetFloatingWindow()->GetPosPixel());
            mnSavedSidebarWidth = aNewSize.Width(); // Save the current width to restore.

            aNewPos.setX(aNewPos.X() + mnSavedSidebarWidth - nTabBarDefaultWidth);
            if (comphelper::LibreOfficeKit::isActive())
            {
                // Hide by collapsing, otherwise with 0x0 the client might expect
                // to get valid dimensions on rendering and not collapse the sidebar.
                aNewSize.setWidth(1);
            }
            else
                aNewSize.setWidth(nTabBarDefaultWidth);

            mpParentWindow->GetFloatingWindow()->SetPosSizePixel(aNewPos, aNewSize);

            if (comphelper::LibreOfficeKit::isActive())
            {
                // Sidebar too narrow to render the menu; disable it.
                mpTabBar->EnableMenuButton(false);

                if (const SfxViewShell* pViewShell = mpViewFrame->GetViewShell())
                {
                    const std::string uno = UnoNameFromDeckId(msCurrentDeckId, vcl::EnumContext::Application::Impress == vcl::EnumContext::GetApplicationEnum(GetCurrentContext().msApplication));
                    if (!uno.empty())
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                                                (uno + "=false").c_str());
                }
            }
        }

        if (mnWidthOnSplitterButtonDown > nTabBarDefaultWidth)
            mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
        mpParentWindow->SetStyle(mpParentWindow->GetStyle() & ~WB_SIZEABLE);
    }

    mbIsDeckOpen = *mbIsDeckRequestedOpen;
    if (*mbIsDeckOpen && mpCurrentDeck)
        mpCurrentDeck->Show();
    NotifyResize();
}

bool SidebarController::CanModifyChildWindowWidth()
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if (pSplitWindow == nullptr)
        return false;

    sal_uInt16 nRow (0xffff);
    sal_uInt16 nColumn (0xffff);
    if (pSplitWindow->GetWindowPos(mpParentWindow, nColumn, nRow))
    {
        sal_uInt16 nRowCount (pSplitWindow->GetWindowCount(nColumn));
        return nRowCount==1;
    }
    else
        return false;
}

sal_Int32 SidebarController::SetChildWindowWidth (const sal_Int32 nNewWidth)
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if (pSplitWindow == nullptr)
        return 0;

    sal_uInt16 nRow (0xffff);
    sal_uInt16 nColumn (0xffff);
    pSplitWindow->GetWindowPos(mpParentWindow, nColumn, nRow);
    const tools::Long nColumnWidth (pSplitWindow->GetLineSize(nColumn));

    vcl::Window* pWindow = mpParentWindow;
    const Size aWindowSize (pWindow->GetSizePixel());

    pSplitWindow->MoveWindow(
        mpParentWindow,
        Size(nNewWidth, aWindowSize.Height()),
        nColumn,
        nRow,
        false);
    static_cast<SplitWindow*>(pSplitWindow)->Split();

    return static_cast<sal_Int32>(nColumnWidth);
}

void SidebarController::RestrictWidth (sal_Int32 nWidth)
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if (pSplitWindow != nullptr)
    {
        const sal_uInt16 nId (pSplitWindow->GetItemId(mpParentWindow.get()));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        const sal_Int32 nRequestedWidth = TabBar::GetDefaultWidth() + nWidth;

        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(nRequestedWidth, getMaximumWidth()));
    }
}

SfxSplitWindow* SidebarController::GetSplitWindow()
{
    if (mpParentWindow != nullptr)
    {
        SfxSplitWindow* pSplitWindow = dynamic_cast<SfxSplitWindow*>(mpParentWindow->GetParent());
        if (pSplitWindow != mpSplitWindow)
        {
            if (mpSplitWindow != nullptr)
                mpSplitWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));

            mpSplitWindow = pSplitWindow;

            if (mpSplitWindow != nullptr)
                mpSplitWindow->AddEventListener(LINK(this, SidebarController, WindowEventHandler));
        }
        return mpSplitWindow;
    }
    else
        return nullptr;
}

void SidebarController::UpdateCloseIndicator (const bool bCloseAfterDrag)
{
    if (mpParentWindow == nullptr)
        return;

    if (bCloseAfterDrag)
    {
        // Make sure that the indicator exists.
        if (!mpCloseIndicator)
            mpCloseIndicator.reset(VclPtr<CloseIndicator>::Create(mpParentWindow));

        // Place and show the indicator.
        const Size aWindowSize (mpParentWindow->GetSizePixel());
        const Size aImageSize (mpCloseIndicator->GetSizePixel());
        mpCloseIndicator->SetPosPixel(
            Point(
                aWindowSize.Width() - TabBar::GetDefaultWidth() - aImageSize.Width(),
                (aWindowSize.Height() - aImageSize.Height())/2));
        mpCloseIndicator->Show();
    }
    else
    {
        // Hide but don't delete the indicator.
        if (mpCloseIndicator)
            mpCloseIndicator->Hide();
    }
}

void SidebarController::UpdateTitleBarIcons()
{
    if ( ! mpCurrentDeck)
        return;

    const bool bIsHighContrastModeActive (Theme::IsHighContrastMode());

    const ResourceManager& rResourceManager = *mpResourceManager;

    // Update the deck icon.
    std::shared_ptr<DeckDescriptor> xDeckDescriptor = rResourceManager.GetDeckDescriptor(mpCurrentDeck->GetId());
    if (xDeckDescriptor && mpCurrentDeck->GetTitleBar())
    {
        const OUString sIconURL(
            bIsHighContrastModeActive
                ? xDeckDescriptor->msHighContrastTitleBarIconURL
                : xDeckDescriptor->msTitleBarIconURL);
        mpCurrentDeck->GetTitleBar()->SetIcon(Tools::GetImage(sIconURL, mxFrame));
    }

    // Update the panel icons.
    const SharedPanelContainer& rPanels (mpCurrentDeck->GetPanels());
    for (const auto& rxPanel : rPanels)
    {
        if ( ! rxPanel)
            continue;
        if (!rxPanel->GetTitleBar())
            continue;
        std::shared_ptr<PanelDescriptor> xPanelDescriptor = rResourceManager.GetPanelDescriptor(rxPanel->GetId());
        if (!xPanelDescriptor)
            continue;
        const OUString sIconURL (
            bIsHighContrastModeActive
               ? xPanelDescriptor->msHighContrastTitleBarIconURL
               : xPanelDescriptor->msTitleBarIconURL);
        rxPanel->GetTitleBar()->SetIcon(Tools::GetImage(sIconURL, mxFrame));
    }
}

void SidebarController::ShowPanel (const Panel& rPanel)
{
    if (mpCurrentDeck)
    {
        if (!IsDeckOpen())
            RequestOpenDeck();
        mpCurrentDeck->ShowPanel(rPanel);
    }
}

ResourceManager::DeckContextDescriptorContainer SidebarController::GetMatchingDecks()
{
    ResourceManager::DeckContextDescriptorContainer aDecks;
    mpResourceManager->GetMatchingDecks (aDecks,
                                        GetCurrentContext(),
                                        IsDocumentReadOnly(),
                                        mxFrame->getController());
    return aDecks;
}

ResourceManager::PanelContextDescriptorContainer SidebarController::GetMatchingPanels(std::u16string_view rDeckId)
{
    ResourceManager::PanelContextDescriptorContainer aPanels;

    mpResourceManager->GetMatchingPanels(aPanels,
                                        GetCurrentContext(),
                                        rDeckId,
                                        mxFrame->getController());
    return aPanels;
}

void SidebarController::updateModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    mpResourceManager->UpdateModel(xModel);
}

void SidebarController::FadeOut()
{
    if (mpSplitWindow)
        mpSplitWindow->FadeOut();
}

void SidebarController::FadeIn()
{
    if (mpSplitWindow)
        mpSplitWindow->FadeIn();
}

tools::Rectangle SidebarController::GetDeckDragArea() const
{
    tools::Rectangle aRect;
    if (mpCurrentDeck)
    {
        if (DeckTitleBar* pTitleBar = mpCurrentDeck->GetTitleBar())
        {
            aRect = pTitleBar->GetDragArea();
        }
    }
    return aRect;
}

void SidebarController::frameAction(const css::frame::FrameActionEvent& rEvent)
{
    if (rEvent.Frame == mxFrame)
    {
        if (rEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING)
            unregisterSidebarForFrame(this, mxFrame->getController());
        else if (rEvent.Action == css::frame::FrameAction_COMPONENT_REATTACHED)
            registerSidebarForFrame(this, mxFrame->getController());
    }
}

void SidebarController::saveDeckState()
{
    // Impress shutdown : context (frame) is disposed before sidebar disposing
    // calc writer : context (frame) is disposed after sidebar disposing
    // so need to test if GetCurrentContext is still valid regarding msApplication
    if (GetCurrentContext().msApplication != "none")
    {
        mpResourceManager->SaveDecksSettings(GetCurrentContext());
        mpResourceManager->SaveLastActiveDeck(GetCurrentContext(), msCurrentDeckId);
    }
}

bool SidebarController::hasChartContextCurrently() const
{
    return GetCurrentContext().msApplication == "com.sun.star.chart2.ChartDocument";
}

sfx2::sidebar::SidebarController* SidebarController::GetSidebarControllerForView(SfxViewShell* pViewShell)
{
    if (!pViewShell)
        return nullptr;

    Reference<css::frame::XController2> xController(pViewShell->GetController(), UNO_QUERY);
    if (!xController.is())
        return nullptr;

    // Make sure there is a model behind the controller, otherwise getSidebar() can crash.
    if (!xController->getModel().is())
        return nullptr;

    Reference<css::ui::XSidebarProvider> xSidebarProvider = xController->getSidebar();
    if (!xSidebarProvider.is())
        return nullptr;

    Reference<css::ui::XSidebar> xSidebar = xSidebarProvider->getSidebar();
    if (!xSidebar.is())
        return nullptr;

    return dynamic_cast<sfx2::sidebar::SidebarController*>(xSidebar.get());
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
