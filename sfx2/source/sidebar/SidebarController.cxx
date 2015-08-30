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
#include "SidebarController.hxx"
#include "Deck.hxx"
#include "DeckTitleBar.hxx"
#include "Panel.hxx"
#include "PanelTitleBar.hxx"
#include "SidebarResource.hxx"
#include "TabBar.hxx"
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include "SidebarDockingWindow.hxx"
#include "Context.hxx"

#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/titledockwin.hxx>
#include "sfxlocal.hrc"
#include <vcl/floatwin.hxx>
#include <vcl/fixed.hxx>
#include "splitwin.hxx"
#include <svl/smplhint.hxx>
#include <tools/link.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>

using namespace css;
using namespace css::uno;
using ::rtl::OUString;

namespace
{
    const static char gsReadOnlyCommandName[] = ".uno:EditDoc";
    const static char gsHideSidebarCommandName[] = ".uno:Sidebar";
    const static sal_Int32 gnMaximumSidebarWidth (400);
    const static sal_Int32 gnWidthCloseThreshold (70);
    const static sal_Int32 gnWidthOpenThreshold (40);
}

namespace sfx2 { namespace sidebar {

SidebarController::SidebarControllerContainer SidebarController::maSidebarControllerContainer;

namespace {
    enum MenuId
    {
        MID_UNLOCK_TASK_PANEL = 1,
        MID_LOCK_TASK_PANEL,
        MID_HIDE_SIDEBAR,
        MID_CUSTOMIZATION,
        MID_RESTORE_DEFAULT,
        MID_FIRST_PANEL,
        MID_FIRST_HIDE = 1000
    };

    /** When in doubt, show this deck.
    */
    static const char gsDefaultDeckId[] = "PropertyDeck";
}

SidebarController::SidebarController (
    SidebarDockingWindow* pParentWindow,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : SidebarControllerInterfaceBase(m_aMutex),
      mpCurrentDeck(),
      mpParentWindow(pParentWindow),
      mpTabBar(VclPtr<TabBar>::Create(
              mpParentWindow,
              rxFrame,
              ::boost::bind(&SidebarController::OpenThenSwitchToDeck, this, _1),
              ::boost::bind(&SidebarController::ShowPopupMenu, this, _1,_2))),
      mxFrame(rxFrame),
      maCurrentContext(OUString(), OUString()),
      maRequestedContext(),
      mnRequestedForceFlags(SwitchFlag_NoForce),
      msCurrentDeckId(gsDefaultDeckId),
      msCurrentDeckTitle(),
      maPropertyChangeForwarder(::boost::bind(&SidebarController::BroadcastPropertyChange, this)),
      maContextChangeUpdate(::boost::bind(&SidebarController::UpdateConfigurations, this)),
      maAsynchronousDeckSwitch(),
      mbIsDeckRequestedOpen(),
      mbIsDeckOpen(),
      mbCanDeckBeOpened(true),
      mnSavedSidebarWidth(pParentWindow->GetSizePixel().Width()),
      maFocusManager(::boost::bind(&SidebarController::ShowPanel, this, _1)),
      mxReadOnlyModeDispatch(),
      mbIsDocumentReadOnly(false),
      mpSplitWindow(NULL),
      mnWidthOnSplitterButtonDown(0)
{
    // Listen for context change events.
    css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    if (xMultiplexer.is())
        xMultiplexer->addContextChangeEventListener(
            static_cast<css::ui::XContextChangeEventListener*>(this),
            mxFrame->getController());

    // Listen for window events.
    mpParentWindow->AddEventListener(LINK(this, SidebarController, WindowEventHandler));

    // Listen for theme property changes.
    Theme::GetPropertySet()->addPropertyChangeListener(
        OUString(""),
        static_cast<css::beans::XPropertyChangeListener*>(this));

    // Get the dispatch object as preparation to listen for changes of
    // the read-only state.
    const util::URL aURL (Tools::GetURL(gsReadOnlyCommandName));
    mxReadOnlyModeDispatch = Tools::GetDispatch(mxFrame, aURL);
    if (mxReadOnlyModeDispatch.is())
        mxReadOnlyModeDispatch->addStatusListener(this, aURL);

    SwitchToDeck(gsDefaultDeckId);

    WeakReference<SidebarController> xWeakController (this);
    maSidebarControllerContainer.insert(
        SidebarControllerContainer::value_type(
            rxFrame,
            xWeakController));
}

SidebarController::~SidebarController()
{
}

SidebarController* SidebarController::GetSidebarControllerForFrame (
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    SidebarControllerContainer::iterator iEntry (maSidebarControllerContainer.find(rxFrame));
    if (iEntry == maSidebarControllerContainer.end())
        return NULL;

    css::uno::Reference<XInterface> xController (iEntry->second.get());
    if ( ! xController.is())
        return NULL;

    return dynamic_cast<SidebarController*>(xController.get());
}

void SAL_CALL SidebarController::disposing()
{
    mpCloseIndicator.disposeAndClear();

    SidebarControllerContainer::iterator iEntry (maSidebarControllerContainer.find(mxFrame));
    if (iEntry != maSidebarControllerContainer.end())
        maSidebarControllerContainer.erase(iEntry);

    maFocusManager.Clear();

    css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    if (xMultiplexer.is())
        xMultiplexer->removeAllContextChangeEventListeners(
            static_cast<css::ui::XContextChangeEventListener*>(this));

    if (mxReadOnlyModeDispatch.is())
        mxReadOnlyModeDispatch->removeStatusListener(this, Tools::GetURL(gsReadOnlyCommandName));
    if (mpSplitWindow != nullptr)
    {
        mpSplitWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpSplitWindow = NULL;
    }

    if (mpParentWindow != nullptr)
    {
        mpParentWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpParentWindow = NULL;
    }

    if (mpCurrentDeck)
    {
        mpCurrentDeck.disposeAndClear();
    }

    mpTabBar.disposeAndClear();

    Theme::GetPropertySet()->removePropertyChangeListener(
        OUString(""),
        static_cast<css::beans::XPropertyChangeListener*>(this));

    maContextChangeUpdate.CancelRequest();
    maAsynchronousDeckSwitch.CancelRequest();
}

void SAL_CALL SidebarController::notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
    throw(css::uno::RuntimeException, std::exception)
{
    // Update to the requested new context asynchronously to avoid
    // subtle errors caused by SFX2 which in rare cases can not
    // properly handle a synchronous update.
    maRequestedContext = Context(
        rEvent.ApplicationName,
        rEvent.ContextName);
    if (maRequestedContext != maCurrentContext)
    {
        maAsynchronousDeckSwitch.CancelRequest();
        maContextChangeUpdate.RequestCall();
    }
}

void SAL_CALL SidebarController::disposing (const css::lang::EventObject& rEventObject)
    throw(css::uno::RuntimeException, std::exception)
{
    (void)rEventObject;

    dispose();
}

void SAL_CALL SidebarController::propertyChange (const css::beans::PropertyChangeEvent& rEvent)
    throw(css::uno::RuntimeException, std::exception)
{
    (void)rEvent;

    maPropertyChangeForwarder.RequestCall();
}

void SAL_CALL SidebarController::statusChanged (const css::frame::FeatureStateEvent& rEvent)
    throw(css::uno::RuntimeException, std::exception)
{
    bool bIsReadWrite (true);
    if (rEvent.IsEnabled)
        rEvent.State >>= bIsReadWrite;

    if (mbIsDocumentReadOnly != !bIsReadWrite)
    {
        mbIsDocumentReadOnly = !bIsReadWrite;

        // Force the current deck to update its panel list.
        if ( ! mbIsDocumentReadOnly)
            msCurrentDeckId = gsDefaultDeckId;
        mnRequestedForceFlags |= SwitchFlag_ForceSwitch;
        maAsynchronousDeckSwitch.CancelRequest();
        maContextChangeUpdate.RequestCall();
    }
}

void SAL_CALL SidebarController::requestLayout()
    throw(css::uno::RuntimeException, std::exception)
{
    sal_Int32 nMinimalWidth = 0;
    if (mpCurrentDeck)
    {
        mpCurrentDeck->RequestLayout();
        nMinimalWidth = mpCurrentDeck->GetMinimalWidth();
    }
    RestrictWidth(nMinimalWidth);
}

void SidebarController::BroadcastPropertyChange()
{
    DataChangedEvent aEvent (DataChangedEventType::USER);
    mpParentWindow->NotifyAllChildren(aEvent);
    mpParentWindow->Invalidate(INVALIDATE_CHILDREN);
}

void SidebarController::NotifyResize()
{
    if (!mpTabBar)
    {
        OSL_ASSERT(mpTabBar!=nullptr);
        return;
    }

    vcl::Window* pParentWindow = mpTabBar->GetParent();
    sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor();

    const sal_Int32 nWidth (pParentWindow->GetSizePixel().Width());
    const sal_Int32 nHeight (pParentWindow->GetSizePixel().Height());

    mbIsDeckOpen = (nWidth > nTabBarDefaultWidth);

    if (mnSavedSidebarWidth <= 0)
        mnSavedSidebarWidth = nWidth;

    bool bIsDeckVisible;
    if (mbCanDeckBeOpened)
    {
        const bool bIsOpening (nWidth > mnWidthOnSplitterButtonDown);
        if (bIsOpening)
            bIsDeckVisible = nWidth >= nTabBarDefaultWidth + gnWidthOpenThreshold;
        else
            bIsDeckVisible = nWidth >= nTabBarDefaultWidth + gnWidthCloseThreshold;
        mbIsDeckRequestedOpen = bIsDeckVisible;
        UpdateCloseIndicator(!bIsDeckVisible);
    }
    else
        bIsDeckVisible = false;

    if (mpCurrentDeck)
    {
        SfxSplitWindow* pSplitWindow = GetSplitWindow();
        WindowAlign eAlign = pSplitWindow ? pSplitWindow->GetAlign() : WINDOWALIGN_RIGHT;
        long nDeckX, nTabX;
        if (eAlign == WINDOWALIGN_LEFT)     // attach the Sidebar towards the left-side of screen
        {
            nDeckX = nTabBarDefaultWidth;
            nTabX = 0;
        }
        else   // attach the Sidebar towards the right-side of screen
        {
            nDeckX = 0;
            nTabX = nWidth-nTabBarDefaultWidth;
        }

        // Place the deck first.
        if (bIsDeckVisible)
        {
            mpCurrentDeck->setPosSizePixel(nDeckX, 0, nWidth - nTabBarDefaultWidth, nHeight);
            mpCurrentDeck->Show();
            mpCurrentDeck->RequestLayout();
        }
        else
            mpCurrentDeck->Hide();

        // Now place the tab bar.
        mpTabBar->setPosSizePixel(nTabX, 0, nTabBarDefaultWidth, nHeight);
        mpTabBar->Show();

    }

    // Determine if the closer of the deck can be shown.
    sal_Int32 nMinimalWidth = 0;
    if (mpCurrentDeck)
    {
        DeckTitleBar* pTitleBar = mpCurrentDeck->GetTitleBar();
        if (pTitleBar != NULL && pTitleBar->IsVisible())
            pTitleBar->SetCloserVisible(CanModifyChildWindowWidth());
        nMinimalWidth = mpCurrentDeck->GetMinimalWidth();
    }

    RestrictWidth(nMinimalWidth);
}

void SidebarController::ProcessNewWidth (const sal_Int32 nNewWidth)
{
    if ( ! mbIsDeckRequestedOpen)
        return;

    if (mbIsDeckRequestedOpen.get())
     {
        // Deck became large enough to be shown.  Show it.
        mnSavedSidebarWidth = nNewWidth;
        RequestOpenDeck();
    }
    else
    {
        // Deck became too small.  Close it completely.
        // If window is wider than the tab bar then mark the deck as being visible, even when it its not.
        // This is to trigger an adjustment of the width to the width of the tab bar.
        mbIsDeckOpen = true;
        RequestCloseDeck();

        if (mnWidthOnSplitterButtonDown > TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor())
            mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
    }
}

void SidebarController::UpdateConfigurations()
{
    if (maCurrentContext != maRequestedContext
        || mnRequestedForceFlags!=SwitchFlag_NoForce)
    {
        maCurrentContext = maRequestedContext;

        // Find the set of decks that could be displayed for the new context.
        ResourceManager::DeckContextDescriptorContainer aDecks;
        ResourceManager::Instance().GetMatchingDecks (
            aDecks,
            maCurrentContext,
            mbIsDocumentReadOnly,
            mxFrame);

        // Notify the tab bar about the updated set of decks.
        mpTabBar->SetDecks(aDecks);

        // Find the new deck.  By default that is the same as the old
        // one.  If that is not set or not enabled, then choose the
        // first enabled deck (which is PropertyDeck).
        OUString sNewDeckId;
        for (ResourceManager::DeckContextDescriptorContainer::const_iterator
                 iDeck(aDecks.begin()),
                 iEnd(aDecks.end());
             iDeck!=iEnd;
             ++iDeck)
        {
            if (iDeck->mbIsEnabled)
            {
                if (iDeck->msId.equals(msCurrentDeckId))
                {
                    sNewDeckId = msCurrentDeckId;
                    break;
                }
                else if (sNewDeckId.getLength() == 0)
                    sNewDeckId = iDeck->msId;
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

        const DeckDescriptor* pDescriptor =
            ResourceManager::Instance().GetDeckDescriptor(sNewDeckId);

        if (pDescriptor)
        {
            SwitchToDeck(
                *pDescriptor,
                maCurrentContext);
        }
    }
}

void SidebarController::OpenThenSwitchToDeck (
    const ::rtl::OUString& rsDeckId)
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if ( pSplitWindow && !pSplitWindow->IsFadeIn() )
        // tdf#83546 Collapsed sidebar should expand first
        pSplitWindow->FadeIn();
    else if ( IsDeckVisible( rsDeckId ) )
    {
        if ( pSplitWindow )
            // tdf#67627 Clicking a second time on a Deck icon will close the Deck
            RequestCloseDeck();
        else
            // tdf#88241 Summoning an undocked sidebar a second time should close sidebar
            mpParentWindow->Close();
        return;
    }
    RequestOpenDeck();
    SwitchToDeck(rsDeckId);
    mpTabBar->Invalidate();
    mpTabBar->HighlightDeck(rsDeckId);
}

void SidebarController::SwitchToDeck (
    const ::rtl::OUString& rsDeckId)
{
    if ( ! msCurrentDeckId.equals(rsDeckId)
        || ! mbIsDeckOpen
        || mnRequestedForceFlags!=SwitchFlag_NoForce)
    {
        const DeckDescriptor* pDeckDescriptor = ResourceManager::Instance().GetDeckDescriptor(rsDeckId);
        if (pDeckDescriptor != NULL)
            SwitchToDeck(*pDeckDescriptor, maCurrentContext);
    }
}

void SidebarController::SwitchToDeck (
    const DeckDescriptor& rDeckDescriptor,
    const Context& rContext)
{
    maFocusManager.Clear();

    const bool bForceNewDeck ((mnRequestedForceFlags&SwitchFlag_ForceNewDeck)!=0);
    const bool bForceNewPanels ((mnRequestedForceFlags&SwitchFlag_ForceNewPanels)!=0);

    if ( ! msCurrentDeckId.equals(rDeckDescriptor.msId)
        || bForceNewDeck)
    {
        // When the deck changes then destroy the deck and all panels
        // and create everything new.
        mpCurrentDeck.disposeAndClear();

        msCurrentDeckId = rDeckDescriptor.msId;
    }
    mpTabBar->HighlightDeck(msCurrentDeckId);

    // Determine the panels to display in the deck.
    ResourceManager::PanelContextDescriptorContainer aPanelContextDescriptors;
    ResourceManager::Instance().GetMatchingPanels(
        aPanelContextDescriptors,
        rContext,
        rDeckDescriptor.msId,
        mxFrame);

    if (aPanelContextDescriptors.empty())
    {
        // There are no panels to be displayed in the current context.
        if (EnumContext::GetContextEnum(rContext.msContext) != EnumContext::Context_Empty)
        {
            // Switch to the "empty" context and try again.
            SwitchToDeck(
                rDeckDescriptor,
                Context(
                    rContext.msApplication,
                    EnumContext::GetContextName(EnumContext::Context_Empty)));
            return;
        }
        else
        {
            // This is already the "empty" context. Looks like we have
            // to live with an empty deck.
        }
    }

    // Provide a configuration and Deck object.
    if ( ! mpCurrentDeck)
    {
        mpCurrentDeck.reset(
            VclPtr<Deck>::Create(
                rDeckDescriptor,
                mpParentWindow,
                ::boost::bind(&SidebarController::RequestCloseDeck, this)));
        msCurrentDeckTitle = rDeckDescriptor.msTitle;

    }
    if ( ! mpCurrentDeck)
        return;

#ifdef DEBUG
    // Show the context name in the deck title bar.
    DeckTitleBar* pDebugTitleBar = mpCurrentDeck->GetTitleBar();
    if (pDebugTitleBar != NULL)
        pDebugTitleBar->SetTitle(rDeckDescriptor.msTitle + " (" + maCurrentContext.msContext + ")");
#endif

    // Update the panel list.
    const sal_Int32 nNewPanelCount (aPanelContextDescriptors.size());
    SharedPanelContainer aNewPanels;
    const SharedPanelContainer& rCurrentPanels (mpCurrentDeck->GetPanels());

    aNewPanels.resize(nNewPanelCount);
    sal_Int32 nWriteIndex (0);
    bool bHasPanelSetChanged (false);
    for (sal_Int32 nReadIndex=0; nReadIndex<nNewPanelCount; ++nReadIndex)
    {
        const ResourceManager::PanelContextDescriptor& rPanelContexDescriptor (
            aPanelContextDescriptors[nReadIndex]);

        // Determine if the panel can be displayed.
        const bool bIsPanelVisible (!mbIsDocumentReadOnly || rPanelContexDescriptor.mbShowForReadOnlyDocuments);
        if ( ! bIsPanelVisible)
            continue;

        // Find the corresponding panel among the currently active
        // panels.
        SharedPanelContainer::const_iterator iPanel = rCurrentPanels.end();

        if (!bForceNewPanels)
        {
            iPanel = rCurrentPanels.end();
            for (auto a = rCurrentPanels.begin(); a != rCurrentPanels.end(); ++a)
            {
                if ((*a)->HasIdPredicate(rPanelContexDescriptor.msId))
                {
                    iPanel = a;
                    break;
                }
            }
        }
        if (iPanel != rCurrentPanels.end())
        {
            // Panel already exists in current deck.  Reuse it.
            aNewPanels[nWriteIndex] = *iPanel;
            aNewPanels[nWriteIndex]->SetExpanded(rPanelContexDescriptor.mbIsInitiallyVisible);
        }
        else
        {
            // Panel does not yet exist or creation of new panels is forced.
            // Create it.
            aNewPanels[nWriteIndex] = CreatePanel(
                rPanelContexDescriptor.msId,
                mpCurrentDeck->GetPanelParentWindow(),
                rPanelContexDescriptor.mbIsInitiallyVisible,
                rContext);
            bHasPanelSetChanged = true;
        }
        if (aNewPanels[nWriteIndex] != nullptr)
        {
            // Depending on the context we have to change the command
            // for the "more options" dialog.
            PanelTitleBar* pTitleBar = aNewPanels[nWriteIndex]->GetTitleBar();
            if (pTitleBar != NULL)
            {
                pTitleBar->SetMoreOptionsCommand(
                    rPanelContexDescriptor.msMenuCommand,
                    mxFrame);
            }

            ++nWriteIndex;
        }

    }
    // mpCurrentPanels - may miss stuff (?)
    aNewPanels.resize(nWriteIndex);

    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor();
    WindowAlign eAlign = pSplitWindow ? pSplitWindow->GetAlign() : WINDOWALIGN_RIGHT;
    long nDeckX;
    if (eAlign == WINDOWALIGN_LEFT)     // attach the Sidebar towards the left-side of screen
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

    mpCurrentDeck->ResetPanels(aNewPanels);
    mpCurrentDeck->Show();

    mpParentWindow->SetText(rDeckDescriptor.msTitle);

    if (bHasPanelSetChanged)
        NotifyResize();

    // Tell the focus manager about the new panels and tab bar
    // buttons.
    maFocusManager.SetDeckTitle(mpCurrentDeck->GetTitleBar());
    maFocusManager.SetPanels(aNewPanels);
    mpTabBar->UpdateFocusManager(maFocusManager);
    UpdateTitleBarIcons();
}

VclPtr<Panel> SidebarController::CreatePanel (
    const OUString& rsPanelId,
    vcl::Window* pParentWindow,
    const bool bIsInitiallyExpanded,
    const Context& rContext)
{
    const PanelDescriptor* pPanelDescriptor = ResourceManager::Instance().GetPanelDescriptor(rsPanelId);
    if (pPanelDescriptor == NULL)
        return NULL;

    // Create the panel which is the parent window of the UIElement.
    VclPtr<Panel> pPanel = VclPtr<Panel>::Create(
        *pPanelDescriptor,
        pParentWindow,
        bIsInitiallyExpanded,
        ::boost::bind(&Deck::RequestLayout, mpCurrentDeck.get()),
        ::boost::bind(&SidebarController::GetCurrentContext, this));

    // Create the XUIElement.
    Reference<ui::XUIElement> xUIElement (CreateUIElement(
            pPanel->GetComponentInterface(),
            pPanelDescriptor->msImplementationURL,
            pPanelDescriptor->mbWantsCanvas,
            rContext));
    if (xUIElement.is())
    {
        // Initialize the panel and add it to the active deck.
        pPanel->SetUIElement(xUIElement);
    }
    else
    {
        pPanel.disposeAndClear();
    }

    return pPanel;
}

Reference<ui::XUIElement> SidebarController::CreateUIElement (
    const Reference<awt::XWindowPeer>& rxWindow,
    const ::rtl::OUString& rsImplementationURL,
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
        SfxDockingWindow* pSfxDockingWindow = dynamic_cast<SfxDockingWindow*>(mpParentWindow.get());
        if (pSfxDockingWindow != NULL)
            aCreationArguments.put("SfxBindings", makeAny(sal_uInt64(&pSfxDockingWindow->GetBindings())));
        aCreationArguments.put("Theme", Theme::GetPropertySet());
        aCreationArguments.put("Sidebar", makeAny(Reference<ui::XSidebar>(static_cast<ui::XSidebar*>(this))));
        if (bWantsCanvas)
        {
            Reference<rendering::XSpriteCanvas> xCanvas (VCLUnoHelper::GetWindow(rxWindow)->GetSpriteCanvas());
            aCreationArguments.put("Canvas", makeAny(xCanvas));
        }
        aCreationArguments.put("ApplicationName", makeAny(rContext.msApplication));
        aCreationArguments.put("ContextName", makeAny(rContext.msContext));

        Reference<ui::XUIElement> xUIElement(
            xUIElementFactory->createUIElement(
                rsImplementationURL,
                Sequence<beans::PropertyValue>(aCreationArguments.getPropertyValues())),
            UNO_QUERY_THROW);

        return xUIElement;
    }
    catch(const Exception& rException)
    {
        SAL_WARN("sfx.sidebar", "Cannot create panel: " << rException.Message);
        return NULL;
    }
}

IMPL_LINK(SidebarController, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent==NULL)
        return sal_IntPtr(false);

    if (pEvent->GetWindow() == mpParentWindow)
    {
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_RESIZE:
                NotifyResize();
                break;

            case VCLEVENT_WINDOW_DATACHANGED:
                // Force an update of deck and tab bar to reflect
                // changes in theme (high contrast mode).
                Theme::HandleDataChange();
                UpdateTitleBarIcons();
                mpParentWindow->Invalidate();
                mnRequestedForceFlags |= SwitchFlag_ForceNewDeck | SwitchFlag_ForceNewPanels;
                maAsynchronousDeckSwitch.CancelRequest();
                maContextChangeUpdate.RequestCall();
                break;

            case SFX_HINT_DYING:
                dispose();
                break;

            case VCLEVENT_WINDOW_PAINT:
                OSL_TRACE("Paint");
                break;

            default:
                break;
        }
    }
    else if (pEvent->GetWindow()==mpSplitWindow && mpSplitWindow!=nullptr)
    {
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
                mnWidthOnSplitterButtonDown = mpParentWindow->GetSizePixel().Width();
                break;

            case VCLEVENT_WINDOW_MOUSEBUTTONUP:
            {
                ProcessNewWidth(mpParentWindow->GetSizePixel().Width());
                mnWidthOnSplitterButtonDown = 0;
                break;
            }

            case SFX_HINT_DYING:
                dispose();
                break;
         }
    }

    return sal_IntPtr(true);
}

void SidebarController::ShowPopupMenu (
    const Rectangle& rButtonBox,
    const ::std::vector<TabBar::DeckMenuData>& rMenuData) const
{
    ::boost::shared_ptr<PopupMenu> pMenu = CreatePopupMenu(rMenuData);
    pMenu->SetSelectHdl(LINK(const_cast<SidebarController*>(this), SidebarController, OnMenuItemSelected));

    // pass toolbox button rect so the menu can stay open on button up
    Rectangle aBox (rButtonBox);
    aBox.Move(mpTabBar->GetPosPixel().X(), 0);
    pMenu->Execute(mpParentWindow, aBox, PopupMenuFlags::ExecuteDown);
}

::boost::shared_ptr<PopupMenu> SidebarController::CreatePopupMenu (
    const ::std::vector<TabBar::DeckMenuData>& rMenuData) const
{
    // Create the top level popup menu.
    ::boost::shared_ptr<PopupMenu> pMenu (new PopupMenu());
    FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
    if (pMenuWindow != NULL)
    {
        pMenuWindow->SetPopupModeFlags(pMenuWindow->GetPopupModeFlags() | FloatWinPopupFlags::NoMouseUpClose);
    }

    // Create sub menu for customization (hiding of deck tabs.)
    PopupMenu* pCustomizationMenu = new PopupMenu();

    SidebarResource aLocalResource;

    // Add one entry for every tool panel element to individually make
    // them visible or hide them.
    sal_Int32 nIndex (0);
    for(::std::vector<TabBar::DeckMenuData>::const_iterator
            iItem(rMenuData.begin()),
            iEnd(rMenuData.end());
        iItem!=iEnd;
        ++iItem,++nIndex)
    {
        const sal_Int32 nMenuIndex (nIndex+MID_FIRST_PANEL);
        pMenu->InsertItem(nMenuIndex, iItem->msDisplayName, MenuItemBits::RADIOCHECK);
        pMenu->CheckItem(nMenuIndex, iItem->mbIsCurrentDeck);
        pMenu->EnableItem(nMenuIndex, iItem->mbIsEnabled&&iItem->mbIsActive);

        const sal_Int32 nSubMenuIndex (nIndex+MID_FIRST_HIDE);
        if (iItem->mbIsCurrentDeck)
        {
            // Don't allow the currently visible deck to be disabled.
            pCustomizationMenu->InsertItem(nSubMenuIndex, iItem->msDisplayName, MenuItemBits::RADIOCHECK);
            pCustomizationMenu->CheckItem(nSubMenuIndex, true);
        }
        else
        {
            pCustomizationMenu->InsertItem(nSubMenuIndex, iItem->msDisplayName, MenuItemBits::CHECKABLE);
            pCustomizationMenu->CheckItem(nSubMenuIndex, iItem->mbIsActive);
        }
    }

    pMenu->InsertSeparator();

    // Add entry for docking or un-docking the tool panel.
    if (mpParentWindow->IsFloatingMode())
        pMenu->InsertItem(MID_LOCK_TASK_PANEL, SFX2_RESSTR(STR_SFX_DOCK));
    else
        pMenu->InsertItem(MID_UNLOCK_TASK_PANEL, SFX2_RESSTR(STR_SFX_UNDOCK));

    pMenu->InsertItem(MID_HIDE_SIDEBAR, SFX2_RESSTR(STRING_HIDE_SIDEBAR));
    pCustomizationMenu->InsertSeparator();
    pCustomizationMenu->InsertItem(MID_RESTORE_DEFAULT, SFX2_RESSTR(STRING_RESTORE));

    pMenu->InsertItem(MID_CUSTOMIZATION, SFX2_RESSTR(STRING_CUSTOMIZATION));
    pMenu->SetPopupMenu(MID_CUSTOMIZATION, pCustomizationMenu);

    pMenu->RemoveDisabledEntries(false, false);

    return pMenu;
}

IMPL_LINK(SidebarController, OnMenuItemSelected, Menu*, pMenu)
{
    if (pMenu == NULL)
    {
        OSL_ENSURE(pMenu!=NULL, "sfx2::sidebar::SidebarController::OnMenuItemSelected: illegal menu!");
        return 0;
    }

    pMenu->Deactivate();
    const sal_Int32 nIndex (pMenu->GetCurItemId());
    switch (nIndex)
    {
        case MID_UNLOCK_TASK_PANEL:
            mpParentWindow->SetFloatingMode(true);
            break;

        case MID_LOCK_TASK_PANEL:
            mpParentWindow->SetFloatingMode(false);
            break;

        case MID_RESTORE_DEFAULT:
            mpTabBar->RestoreHideFlags();
            break;

        case MID_HIDE_SIDEBAR:
        {
            const util::URL aURL (Tools::GetURL(gsHideSidebarCommandName));
            Reference<frame::XDispatch> mxDispatch (Tools::GetDispatch(mxFrame, aURL));
            if (mxDispatch.is())
                    mxDispatch->dispatch(aURL, Sequence<beans::PropertyValue>());
            break;
        }
        default:
        {
            try
            {
                if (nIndex >= MID_FIRST_PANEL && nIndex<MID_FIRST_HIDE)
                {
                    RequestOpenDeck();
                    SwitchToDeck(mpTabBar->GetDeckIdForIndex(nIndex - MID_FIRST_PANEL));
                }
                else if (nIndex >=MID_FIRST_HIDE)
                    if (pMenu->GetItemBits(nIndex) == MenuItemBits::CHECKABLE)
                        mpTabBar->ToggleHideFlag(nIndex-MID_FIRST_HIDE);
            }
            catch (RuntimeException&)
            {
            }
        }
        break;
    }

    return 1;
}

void SidebarController::RequestCloseDeck()
{
    mbIsDeckRequestedOpen = false;
    UpdateDeckOpenState();

    // remove highlight from TabBar, because Deck will be closed
    mpTabBar->RemoveDeckHighlight();
}

void SidebarController::RequestOpenDeck()
{
    mbIsDeckRequestedOpen = true;
    UpdateDeckOpenState();
}

bool SidebarController::IsDeckVisible(const OUString& rsDeckId)
{
    return mbIsDeckOpen && mbIsDeckOpen.get() && msCurrentDeckId == rsDeckId;
}

void SidebarController::UpdateDeckOpenState()
{
    if ( ! mbIsDeckRequestedOpen)
        // No state requested.
        return;

    sal_Int32 nTabBarDefaultWidth = TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor();

    // Update (change) the open state when it either has not yet been initialized
    // or when its value differs from the requested state.
    if ( ! mbIsDeckOpen
        || mbIsDeckOpen.get() != mbIsDeckRequestedOpen.get())
    {
        if (mbIsDeckRequestedOpen.get())
        {
            if (mnSavedSidebarWidth <= nTabBarDefaultWidth)
                SetChildWindowWidth(SidebarChildWindow::GetDefaultWidth(mpParentWindow));
            else
                SetChildWindowWidth(mnSavedSidebarWidth);
        }
        else
        {
            if ( ! mpParentWindow->IsFloatingMode())
                mnSavedSidebarWidth = SetChildWindowWidth(nTabBarDefaultWidth);
            if (mnWidthOnSplitterButtonDown > nTabBarDefaultWidth)
                mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
            mpParentWindow->SetStyle(mpParentWindow->GetStyle() & ~WB_SIZEABLE);
        }

        mbIsDeckOpen = mbIsDeckRequestedOpen.get();
        if (mbIsDeckOpen.get() && mpCurrentDeck)
            mpCurrentDeck->Show(mbIsDeckOpen.get());
        NotifyResize();
    }
}

bool SidebarController::CanModifyChildWindowWidth()
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if (pSplitWindow == NULL)
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
    if (pSplitWindow == NULL)
        return 0;

    sal_uInt16 nRow (0xffff);
    sal_uInt16 nColumn (0xffff);
    pSplitWindow->GetWindowPos(mpParentWindow, nColumn, nRow);
    const long nColumnWidth (pSplitWindow->GetLineSize(nColumn));

    vcl::Window* pWindow = mpParentWindow;
    const Size aWindowSize (pWindow->GetSizePixel());

    pSplitWindow->MoveWindow(
        mpParentWindow,
        Size(nNewWidth, aWindowSize.Height()),
        nColumn,
        nRow);
    static_cast<SplitWindow*>(pSplitWindow)->Split();

    return static_cast<sal_Int32>(nColumnWidth);
}

void SidebarController::RestrictWidth (sal_Int32 nWidth)
{
    SfxSplitWindow* pSplitWindow = GetSplitWindow();
    if (pSplitWindow != NULL)
    {
        const sal_uInt16 nId (pSplitWindow->GetItemId(mpParentWindow.get()));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor() + nWidth,
                  gnMaximumSidebarWidth * mpTabBar->GetDPIScaleFactor()));
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
        return NULL;
}

void SidebarController::UpdateCloseIndicator (const bool bCloseAfterDrag)
{
    if (mpParentWindow == nullptr)
        return;

    if (bCloseAfterDrag)
    {
        // Make sure that the indicator exists.
        if ( ! mpCloseIndicator)
        {
            mpCloseIndicator.reset(VclPtr<FixedImage>::Create(mpParentWindow));
            FixedImage* pFixedImage = static_cast<FixedImage*>(mpCloseIndicator.get());
            const Image aImage (Theme::GetImage(Theme::Image_CloseIndicator));
            pFixedImage->SetImage(aImage);
            pFixedImage->SetSizePixel(aImage.GetSizePixel());
            pFixedImage->SetBackground(Theme::GetWallpaper(Theme::Paint_DeckBackground));
        }

        // Place and show the indicator.
        const Size aWindowSize (mpParentWindow->GetSizePixel());
        const Size aImageSize (mpCloseIndicator->GetSizePixel());
        mpCloseIndicator->SetPosPixel(
            Point(
                aWindowSize.Width() - TabBar::GetDefaultWidth() * mpTabBar->GetDPIScaleFactor() - aImageSize.Width(),
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
    const ResourceManager& rResourceManager (ResourceManager::Instance());

    // Update the deck icon.
    const DeckDescriptor* pDeckDescriptor = rResourceManager.GetDeckDescriptor(mpCurrentDeck->GetId());
    if (pDeckDescriptor != NULL && mpCurrentDeck->GetTitleBar())
    {
        const OUString sIconURL(
            bIsHighContrastModeActive
                ? pDeckDescriptor->msHighContrastTitleBarIconURL
                : pDeckDescriptor->msTitleBarIconURL);
        mpCurrentDeck->GetTitleBar()->SetIcon(Tools::GetImage(sIconURL, mxFrame));
    }

    // Update the panel icons.
    const SharedPanelContainer& rPanels (mpCurrentDeck->GetPanels());
    for (SharedPanelContainer::const_iterator
             iPanel(rPanels.begin()), iEnd(rPanels.end());
             iPanel!=iEnd;
             ++iPanel)
    {
        if ( ! *iPanel)
            continue;
        if ((*iPanel)->GetTitleBar() == NULL)
            continue;
        const PanelDescriptor* pPanelDescriptor = rResourceManager.GetPanelDescriptor((*iPanel)->GetId());
        if (pPanelDescriptor == NULL)
            continue;
        const OUString sIconURL (
            bIsHighContrastModeActive
               ? pPanelDescriptor->msHighContrastTitleBarIconURL
               : pPanelDescriptor->msTitleBarIconURL);
        (*iPanel)->GetTitleBar()->SetIcon(Tools::GetImage(sIconURL, mxFrame));
    }
}

void SidebarController::ShowPanel (const Panel& rPanel)
{
    if (mpCurrentDeck)
        mpCurrentDeck->ShowPanel(rPanel);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
