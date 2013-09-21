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
#include "SidebarPanel.hxx"
#include "SidebarResource.hxx"
#include "TabBar.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sidebar/SidebarChildWindow.hxx"
#include "sfx2/sidebar/Tools.hxx"
#include "SidebarDockingWindow.hxx"
#include "Context.hxx"

#include <sfx2/sfxresid.hxx>
#include "sfx2/sfxsids.hrc"
#include "sfx2/titledockwin.hxx"
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
#include <com/sun/star/ui/UIElementFactoryManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>


using namespace css;
using namespace cssu;
using ::rtl::OUString;


#undef VERBOSE

namespace
{
    const static char gsReadOnlyCommandName[] = ".uno:EditDoc";
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
    const cssu::Reference<css::frame::XFrame>& rxFrame)
    : SidebarControllerInterfaceBase(m_aMutex),
      mpCurrentDeck(),
      mpParentWindow(pParentWindow),
      mpTabBar(new TabBar(
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
      mnWidthOnSplitterButtonDown(0),
      mpCloseIndicator()
{
    if (pParentWindow == NULL)
    {
        OSL_ASSERT(pParentWindow!=NULL);
            return;
    }

    // Listen for context change events.
    cssu::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
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
        A2S(""),
        static_cast<css::beans::XPropertyChangeListener*>(this));

    // Get the dispatch object as preparation to listen for changes of
    // the read-only state.
    const util::URL aURL (Tools::GetURL(gsReadOnlyCommandName));
    mxReadOnlyModeDispatch = Tools::GetDispatch(mxFrame, aURL);
    if (mxReadOnlyModeDispatch.is())
        mxReadOnlyModeDispatch->addStatusListener(this, aURL);

    SwitchToDeck(A2S("default"));

    WeakReference<SidebarController> xWeakController (this);
    maSidebarControllerContainer.insert(
        SidebarControllerContainer::value_type(
            rxFrame,
            xWeakController));
}




SidebarController::~SidebarController (void)
{
}




SidebarController* SidebarController::GetSidebarControllerForFrame (
    const cssu::Reference<css::frame::XFrame>& rxFrame)
{
    SidebarControllerContainer::iterator iEntry (maSidebarControllerContainer.find(rxFrame));
    if (iEntry == maSidebarControllerContainer.end())
        return NULL;

    cssu::Reference<XInterface> xController (iEntry->second.get());
    if ( ! xController.is())
        return NULL;

    return dynamic_cast<SidebarController*>(xController.get());
}




void SAL_CALL SidebarController::disposing (void)
{
    SidebarControllerContainer::iterator iEntry (maSidebarControllerContainer.find(mxFrame));
    if (iEntry != maSidebarControllerContainer.end())
        maSidebarControllerContainer.erase(iEntry);

    maFocusManager.Clear();

    cssu::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    if (xMultiplexer.is())
        xMultiplexer->removeAllContextChangeEventListeners(
            static_cast<css::ui::XContextChangeEventListener*>(this));

    if (mxReadOnlyModeDispatch.is())
        mxReadOnlyModeDispatch->removeStatusListener(this, Tools::GetURL(gsReadOnlyCommandName));
    if (mpSplitWindow != NULL)
    {
        mpSplitWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpSplitWindow = NULL;
    }

    if (mpParentWindow != NULL)
    {
        mpParentWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpParentWindow = NULL;
    }

    if (mpCurrentDeck)
    {
        mpCurrentDeck->Dispose();
        mpCurrentDeck->PrintWindowTree();
        mpCurrentDeck.reset();
    }

    mpTabBar.reset();

    Theme::GetPropertySet()->removePropertyChangeListener(
        A2S(""),
        static_cast<css::beans::XPropertyChangeListener*>(this));

    maContextChangeUpdate.CancelRequest();
    maAsynchronousDeckSwitch.CancelRequest();
}




void SAL_CALL SidebarController::notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
    throw(cssu::RuntimeException)
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
    throw(cssu::RuntimeException)
{
    (void)rEventObject;

    dispose();
}




void SAL_CALL SidebarController::propertyChange (const css::beans::PropertyChangeEvent& rEvent)
    throw(cssu::RuntimeException)
{
    (void)rEvent;

    maPropertyChangeForwarder.RequestCall();
}




void SAL_CALL SidebarController::statusChanged (const css::frame::FeatureStateEvent& rEvent)
    throw(cssu::RuntimeException)
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




void SAL_CALL SidebarController::requestLayout (void)
    throw(cssu::RuntimeException)
{
    sal_Int32 nMinimalWidth = 0;
    if (mpCurrentDeck)
    {
        mpCurrentDeck->RequestLayout();
        nMinimalWidth = mpCurrentDeck->GetMinimalWidth();
    }
    RestrictWidth(nMinimalWidth);
}




void SidebarController::BroadcastPropertyChange (void)
{
    DataChangedEvent aEvent (DATACHANGED_USER);
    mpParentWindow->NotifyAllChildren(aEvent);
    mpParentWindow->Invalidate(INVALIDATE_CHILDREN);
}




void SidebarController::NotifyResize (void)
{
    if (mpTabBar == 0)
    {
        OSL_ASSERT(mpTabBar!=0);
        return;
    }

    Window* pParentWindow = mpTabBar->GetParent();

    const sal_Int32 nWidth (pParentWindow->GetSizePixel().Width());
    const sal_Int32 nHeight (pParentWindow->GetSizePixel().Height());

    mbIsDeckOpen = (nWidth > TabBar::GetDefaultWidth());

    if (mnSavedSidebarWidth <= 0)
        mnSavedSidebarWidth = nWidth;

    bool bIsDeckVisible;
    if (mbCanDeckBeOpened)
    {
        const bool bIsOpening (nWidth > mnWidthOnSplitterButtonDown);
        if (bIsOpening)
            bIsDeckVisible = nWidth >= TabBar::GetDefaultWidth() + gnWidthOpenThreshold;
        else
            bIsDeckVisible = nWidth >= TabBar::GetDefaultWidth() + gnWidthCloseThreshold;
        mbIsDeckRequestedOpen = bIsDeckVisible;
        UpdateCloseIndicator(!bIsDeckVisible);
    }
    else
        bIsDeckVisible = false;

    // Place the deck.
    if (mpCurrentDeck)
    {
        if (bIsDeckVisible)
        {
            mpCurrentDeck->setPosSizePixel(0,0, nWidth-TabBar::GetDefaultWidth(), nHeight);
            mpCurrentDeck->Show();
            mpCurrentDeck->RequestLayout();
        }
        else
            mpCurrentDeck->Hide();
    }

    // Place the tab bar.
    mpTabBar->setPosSizePixel(nWidth-TabBar::GetDefaultWidth(),0,TabBar::GetDefaultWidth(),nHeight);
    mpTabBar->Show();

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

        if (mnWidthOnSplitterButtonDown > TabBar::GetDefaultWidth())
            mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
    }
}




void SidebarController::UpdateConfigurations (void)
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
        // first enabled deck.
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

        SwitchToDeck(
            *ResourceManager::Instance().GetDeckDescriptor(sNewDeckId),
            maCurrentContext);
    }
}




void SidebarController::OpenThenSwitchToDeck (
    const ::rtl::OUString& rsDeckId)
{
    RequestOpenDeck();
    SwitchToDeck(rsDeckId);
    mpTabBar->Invalidate();
}




void SidebarController::RequestSwitchToDeck (
    const ::rtl::OUString& rsDeckId)
{
    maContextChangeUpdate.CancelRequest();
    maAsynchronousDeckSwitch.RequestCall(
        ::boost::bind(&SidebarController::OpenThenSwitchToDeck, this, rsDeckId));
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
    mnRequestedForceFlags = SwitchFlag_NoForce;

    if ( ! msCurrentDeckId.equals(rDeckDescriptor.msId)
        || bForceNewDeck)
    {
        // When the deck changes then destroy the deck and all panels
        // and create everything new.
        if (mpCurrentDeck)
        {
            mpCurrentDeck->Dispose();
            mpCurrentDeck.reset();
        }

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
            new Deck(
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
        SharedPanelContainer::const_iterator iPanel;
        if (bForceNewPanels)
        {
            // All panels have to be created in any case.  There is no
            // point in searching already existing panels.
            iPanel = rCurrentPanels.end();
        }
        else
        {
            iPanel = ::std::find_if(
                rCurrentPanels.begin(),
                rCurrentPanels.end(),
                ::boost::bind(&Panel::HasIdPredicate, _1, ::boost::cref(rPanelContexDescriptor.msId)));
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
        if (aNewPanels[nWriteIndex] != 0)
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
    aNewPanels.resize(nWriteIndex);

    // Activate the deck and the new set of panels.
    mpCurrentDeck->setPosSizePixel(
        0,
        0,
        mpParentWindow->GetSizePixel().Width()-TabBar::GetDefaultWidth(),
        mpParentWindow->GetSizePixel().Height());
    mpCurrentDeck->SetPanels(aNewPanels);
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




SharedPanel SidebarController::CreatePanel (
    const OUString& rsPanelId,
    ::Window* pParentWindow,
    const bool bIsInitiallyExpanded,
    const Context& rContext)
{
    const PanelDescriptor* pPanelDescriptor = ResourceManager::Instance().GetPanelDescriptor(rsPanelId);
    if (pPanelDescriptor == NULL)
        return SharedPanel();

    // Create the panel which is the parent window of the UIElement.
    SharedPanel pPanel (new Panel(
        *pPanelDescriptor,
        pParentWindow,
        bIsInitiallyExpanded,
        ::boost::bind(&Deck::RequestLayout, mpCurrentDeck.get()),
        ::boost::bind(&SidebarController::GetCurrentContext, this)));

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
        pPanel.reset();
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
               ui::UIElementFactoryManager::create( xComponentContext );

       // Create the XUIElement.
        ::comphelper::NamedValueCollection aCreationArguments;
        aCreationArguments.put("Frame", makeAny(mxFrame));
        aCreationArguments.put("ParentWindow", makeAny(rxWindow));
        SfxDockingWindow* pSfxDockingWindow = dynamic_cast<SfxDockingWindow*>(mpParentWindow);
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
        return sal_False;

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
    else if (pEvent->GetWindow()==mpSplitWindow && mpSplitWindow!=NULL)
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

    return sal_True;
}




void SidebarController::ShowPopupMenu (
    const Rectangle& rButtonBox,
    const ::std::vector<TabBar::DeckMenuData>& rMenuData) const
{
    ::boost::shared_ptr<PopupMenu> pMenu = CreatePopupMenu(rMenuData);
    pMenu->SetSelectHdl(LINK(this, SidebarController, OnMenuItemSelected));

    // pass toolbox button rect so the menu can stay open on button up
    Rectangle aBox (rButtonBox);
    aBox.Move(mpTabBar->GetPosPixel().X(), 0);
    pMenu->Execute(mpParentWindow, aBox, POPUPMENU_EXECUTE_DOWN);
}




::boost::shared_ptr<PopupMenu> SidebarController::CreatePopupMenu (
    const ::std::vector<TabBar::DeckMenuData>& rMenuData) const
{
    // Create the top level popup menu.
    ::boost::shared_ptr<PopupMenu> pMenu (new PopupMenu());
    FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
    if (pMenuWindow != NULL)
    {
        pMenuWindow->SetPopupModeFlags(pMenuWindow->GetPopupModeFlags() | FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE);
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
        pMenu->InsertItem(nMenuIndex, iItem->msDisplayName, MIB_RADIOCHECK);
        pMenu->CheckItem(nMenuIndex, iItem->mbIsCurrentDeck ? sal_True : sal_False);
        pMenu->EnableItem(nMenuIndex, (iItem->mbIsEnabled&&iItem->mbIsActive) ? sal_True : sal_False);

        const sal_Int32 nSubMenuIndex (nIndex+MID_FIRST_HIDE);
        if (iItem->mbIsCurrentDeck)
        {
            // Don't allow the currently visible deck to be disabled.
            pCustomizationMenu->InsertItem(nSubMenuIndex, iItem->msDisplayName, MIB_RADIOCHECK);
            pCustomizationMenu->CheckItem(nSubMenuIndex, sal_True);
        }
        else
        {
            pCustomizationMenu->InsertItem(nSubMenuIndex, iItem->msDisplayName, MIB_CHECKABLE);
            pCustomizationMenu->CheckItem(nSubMenuIndex, iItem->mbIsActive ? sal_True : sal_False);
        }
    }

    pMenu->InsertSeparator();

    // Add entry for docking or un-docking the tool panel.
    if (mpParentWindow->IsFloatingMode())
        pMenu->InsertItem(MID_LOCK_TASK_PANEL, SFX2_RESSTR(STR_SFX_DOCK));
    else
        pMenu->InsertItem(MID_UNLOCK_TASK_PANEL, SFX2_RESSTR(STR_SFX_UNDOCK));

    pCustomizationMenu->InsertSeparator();
    pCustomizationMenu->InsertItem(MID_RESTORE_DEFAULT, SFX2_RESSTR(STRING_RESTORE));

    pMenu->InsertItem(MID_CUSTOMIZATION, SFX2_RESSTR(STRING_CUSTOMIZATION));
    pMenu->SetPopupMenu(MID_CUSTOMIZATION, pCustomizationMenu);

    pMenu->RemoveDisabledEntries(sal_False, sal_False);

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
            mpParentWindow->SetFloatingMode(sal_True);
            break;

        case MID_LOCK_TASK_PANEL:
            mpParentWindow->SetFloatingMode(sal_False);
            break;

        case MID_RESTORE_DEFAULT:
            mpTabBar->RestoreHideFlags();
            break;

        default:
        {
            try
            {
                if (nIndex >= MID_FIRST_PANEL && nIndex<MID_FIRST_HIDE)
                    SwitchToDeck(mpTabBar->GetDeckIdForIndex(nIndex - MID_FIRST_PANEL));
                else if (nIndex >=MID_FIRST_HIDE)
                    if (pMenu->GetItemBits(nIndex) == MIB_CHECKABLE)
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




void SidebarController::RequestCloseDeck (void)
{
    mbIsDeckRequestedOpen = false;
    UpdateDeckOpenState();
}




void SidebarController::RequestOpenDeck (void)
{
    mbIsDeckRequestedOpen = true;
    UpdateDeckOpenState();
}




void SidebarController::UpdateDeckOpenState (void)
{
    if ( ! mbIsDeckRequestedOpen)
        // No state requested.
        return;

    // Update (change) the open state when it either has not yet been initialized
    // or when its value differs from the requested state.
    if ( ! mbIsDeckOpen
        || mbIsDeckOpen.get() != mbIsDeckRequestedOpen.get())
    {
        if (mbIsDeckRequestedOpen.get())
        {
            if (mnSavedSidebarWidth <= TabBar::GetDefaultWidth())
                SetChildWindowWidth(SidebarChildWindow::GetDefaultWidth(mpParentWindow));
            else
                SetChildWindowWidth(mnSavedSidebarWidth);
        }
        else
        {
            if ( ! mpParentWindow->IsFloatingMode())
                mnSavedSidebarWidth = SetChildWindowWidth(TabBar::GetDefaultWidth());
            if (mnWidthOnSplitterButtonDown > TabBar::GetDefaultWidth())
                mnSavedSidebarWidth = mnWidthOnSplitterButtonDown;
            mpParentWindow->SetStyle(mpParentWindow->GetStyle() & ~WB_SIZEABLE);
        }

        mbIsDeckOpen = mbIsDeckRequestedOpen.get();
        if (mbIsDeckOpen.get() && mpCurrentDeck)
            mpCurrentDeck->Show(mbIsDeckOpen.get());
        NotifyResize();
    }
}




FocusManager& SidebarController::GetFocusManager (void)
{
    return maFocusManager;
}




bool SidebarController::CanModifyChildWindowWidth (void)
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

    Window* pWindow = mpParentWindow;
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
        const sal_uInt16 nId (pSplitWindow->GetItemId(mpParentWindow));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(TabBar::GetDefaultWidth() + nWidth, gnMaximumSidebarWidth));
    }
}




SfxSplitWindow* SidebarController::GetSplitWindow (void)
{
    if (mpParentWindow != NULL)
    {
        SfxSplitWindow* pSplitWindow = dynamic_cast<SfxSplitWindow*>(mpParentWindow->GetParent());
        if (pSplitWindow != mpSplitWindow)
        {
            if (mpSplitWindow != NULL)
                mpSplitWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));

            mpSplitWindow = pSplitWindow;

            if (mpSplitWindow != NULL)
                mpSplitWindow->AddEventListener(LINK(this, SidebarController, WindowEventHandler));
        }
        return mpSplitWindow;
    }
    else
        return NULL;
}




void SidebarController::UpdateCloseIndicator (const bool bCloseAfterDrag)
{
    if (mpParentWindow == NULL)
        return;

    if (bCloseAfterDrag)
    {
        // Make sure that the indicator exists.
        if ( ! mpCloseIndicator)
        {
            mpCloseIndicator.reset(new FixedImage(mpParentWindow));
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




void SidebarController::UpdateTitleBarIcons (void)
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




Context SidebarController::GetCurrentContext (void) const
{
    return maCurrentContext;
}


} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
