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
#include "SidebarPanel.hxx"
#include "SidebarResource.hxx"
#include "TabBar.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "SidebarDockingWindow.hxx"
#include "Context.hxx"
#include "Tools.hxx"

#include "sfxresid.hxx"
#include "sfx2/sfxsids.hrc"
#include "sfx2/titledockwin.hxx"
#include "sfxlocal.hrc"
#include <vcl/floatwin.hxx>
#include "splitwin.hxx"
#include <svl/smplhint.hxx>
#include <tools/link.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>


using namespace css;
using namespace cssu;
using ::rtl::OUString;



namespace sfx2 { namespace sidebar {

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
              ::boost::bind(&SidebarController::SwitchToDeck, this, _1),
              ::boost::bind(&SidebarController::ShowPopupMenu, this, _1,_2,_3))),
      mxFrame(rxFrame),
      maCurrentContext(OUString(), OUString()),
      msCurrentDeckId(A2S("PropertyDeck")),
      maPropertyChangeForwarder(::boost::bind(&SidebarController::BroadcastPropertyChange, this)),
      mbIsDeckClosed(false),
      mnSavedSidebarWidth(pParentWindow->GetSizePixel().Width())
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

    SwitchToDeck(A2S("default"));
}




SidebarController::~SidebarController (void)
{
}




void SAL_CALL SidebarController::disposing (void)
{
    maFocusManager.Clear();

    cssu::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    if (xMultiplexer.is())
        xMultiplexer->removeAllContextChangeEventListeners(
            static_cast<css::ui::XContextChangeEventListener*>(this));

    if (mpParentWindow != NULL)
    {
        mpParentWindow->RemoveEventListener(LINK(this, SidebarController, WindowEventHandler));
        mpParentWindow = NULL;
    }

    if (mpCurrentDeck)
    {
        mpCurrentDeck->Dispose();
        OSL_TRACE("deleting deck window subtree");
        mpCurrentDeck->PrintWindowTree();
        mpCurrentDeck.reset();
    }

    mpTabBar.reset();

    Theme::GetPropertySet()->removePropertyChangeListener(
        A2S(""),
        static_cast<css::beans::XPropertyChangeListener*>(this));
}




void SAL_CALL SidebarController::notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
    throw(cssu::RuntimeException)
{
    UpdateConfigurations(
        Context(
            rEvent.ApplicationName,
            rEvent.ContextName));
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




void SAL_CALL SidebarController::requestLayout (void)
    throw(cssu::RuntimeException)
{
    if (mpCurrentDeck)
        mpCurrentDeck->RequestLayout();
    RestrictWidth();
}




void SidebarController::BroadcastPropertyChange (void)
{
    DataChangedEvent aEvent (DATACHANGED_USER);
    mpParentWindow->NotifyAllChildren(aEvent);
    mpParentWindow->Invalidate(INVALIDATE_CHILDREN);
}




void SidebarController::NotifyResize (void)
{
    if (mpTabBar == NULL)
    {
        OSL_ASSERT(mpTabBar!=NULL);
        return;
    }

    Window* pParentWindow = mpTabBar->GetParent();

    const sal_Int32 nWidth (pParentWindow->GetSizePixel().Width());
    const sal_Int32 nHeight (pParentWindow->GetSizePixel().Height());

    // Place the deck.
    if (mpCurrentDeck)
    {
        mpCurrentDeck->setPosSizePixel(0,0, nWidth-TabBar::GetDefaultWidth(), nHeight);
        mpCurrentDeck->Show();
        mpCurrentDeck->RequestLayout();
    }

    // Place the tab bar.
    mpTabBar->setPosSizePixel(nWidth-TabBar::GetDefaultWidth(),0,TabBar::GetDefaultWidth(),nHeight);
    mpTabBar->Show();

    // Determine if the closer of the deck can be shown.
    if (mpCurrentDeck)
    {
        DeckTitleBar* pTitleBar = mpCurrentDeck->GetTitleBar();
        if (pTitleBar != NULL && pTitleBar->IsVisible())
            pTitleBar->SetCloserVisible(CanModifyChildWindowWidth());
    }

    if (nWidth > TabBar::GetDefaultWidth())
        mnSavedSidebarWidth = nWidth;

    RestrictWidth();
#ifdef DEBUG
    if (mpCurrentDeck)
    {
        mpCurrentDeck->PrintWindowTree();
        sal_Int32 nPanelIndex (0);
        for (SharedPanelContainer::const_iterator
                 iPanel(mpCurrentDeck->GetPanels().begin()),
                 iEnd(mpCurrentDeck->GetPanels().end());
             iPanel!=iEnd;
             ++iPanel,++nPanelIndex)
        {
            OSL_TRACE("panel %d:", nPanelIndex);
            (*iPanel)->PrintWindowTree();
        }
    }
#endif
}




void SidebarController::UpdateConfigurations (const Context& rContext)
{
    if (maCurrentContext != rContext)
    {
        maCurrentContext = rContext;

        // Notify the tab bar about the updated set of decks.
        ResourceManager::IdContainer aDeckIds;
        ResourceManager::Instance().GetMatchingDecks (
            aDeckIds,
            rContext,
            mxFrame);
        mpTabBar->SetDecks(aDeckIds);

        // Check if the current deck is among the matching decks.
        bool bCurrentDeckMatches (false);
        for (ResourceManager::IdContainer::const_iterator
                 iDeck(aDeckIds.begin()),
                 iEnd(aDeckIds.end());
             iDeck!=iEnd;
             ++iDeck)
        {
            if (iDeck->equals(msCurrentDeckId))
            {
                bCurrentDeckMatches = true;
                break;
            }
        }

        DeckDescriptor const* pDeckDescriptor = NULL;
        if ( ! bCurrentDeckMatches)
            pDeckDescriptor = ResourceManager::Instance().GetBestMatchingDeck(rContext, mxFrame);
        else
            pDeckDescriptor = ResourceManager::Instance().GetDeckDescriptor(msCurrentDeckId);
        if (pDeckDescriptor != NULL)
        {
            msCurrentDeckId = pDeckDescriptor->msId;
            SwitchToDeck(*pDeckDescriptor, rContext);
        }

#ifdef DEBUG
        // Show the context name in the deck title bar.
        if (mpCurrentDeck)
        {
            DeckTitleBar* pTitleBar = mpCurrentDeck->GetTitleBar();
            if (pTitleBar != NULL)
                pTitleBar->SetTitle(msCurrentDeckTitle+A2S(" (")+rContext.msContext+A2S(")"));
        }
#endif
    }
}




void SidebarController::SwitchToDeck (
    const ::rtl::OUString& rsDeckId)
{
    if ( ! msCurrentDeckId.equals(rsDeckId) || mbIsDeckClosed)
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

    if ( ! msCurrentDeckId.equals(rDeckDescriptor.msId))
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

    // Reopen the deck when necessary.
    OpenDeck();

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

    if (mpCurrentDeck
        && ArePanelSetsEqual(mpCurrentDeck->GetPanels(), aPanelContextDescriptors))
    {
        // Requested set of panels is identical to the current set of
        // panels => Nothing to do.
        return;
    }

    // Provide a configuration and Deck object.
    if ( ! mpCurrentDeck)
    {
        mpCurrentDeck.reset(
            new Deck(
                rDeckDescriptor,
                mpParentWindow,
                ::boost::bind(&SidebarController::CloseDeck, this)));
        msCurrentDeckTitle = rDeckDescriptor.msTitle;
    }
    if ( ! mpCurrentDeck)
        return;

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

        // Find the corresponding panel among the currently active
        // panels.
        SharedPanelContainer::const_iterator iPanel (::std::find_if(
                rCurrentPanels.begin(),
                rCurrentPanels.end(),
                ::boost::bind(&Panel::HasIdPredicate, _1, ::boost::cref(rPanelContexDescriptor.msId))));
        if (iPanel != rCurrentPanels.end())
        {
            // Panel already exists in current deck.  Reuse it.
            aNewPanels[nWriteIndex] = *iPanel;
            OSL_TRACE("    reusing panel %s", S2A(rPanelContexDescriptor.msId));
        }
        else
        {
            // Panel does not yet exist.  Create it.
            aNewPanels[nWriteIndex] = CreatePanel(
                rPanelContexDescriptor.msId,
                mpCurrentDeck->GetPanelParentWindow(),
                rPanelContexDescriptor.msMenuCommand);
            OSL_TRACE("    creating panel %s", S2A(rPanelContexDescriptor.msId));
            bHasPanelSetChanged = true;
        }
        if (aNewPanels[nWriteIndex] != NULL)
        {
            // Depending on the context we have to collapse the panel.
            aNewPanels[nWriteIndex]->SetExpanded(rPanelContexDescriptor.mbIsInitiallyVisible);

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

    // Tell the tab bar to highlight the button associated with the
    // deck.
    mpTabBar->HighlightDeck(rDeckDescriptor.msId);

    mpParentWindow->SetText(rDeckDescriptor.msTitle);

    if (bHasPanelSetChanged)
        NotifyResize();

    // Tell the focus manager about the new panels and tab bar
    // buttons.
    maFocusManager.SetPanels(aNewPanels);
    mpTabBar->UpdateFocusManager(maFocusManager);
}




bool SidebarController::ArePanelSetsEqual (
    const SharedPanelContainer& rCurrentPanels,
    const ResourceManager::PanelContextDescriptorContainer& rRequestedPanels)
{
#ifdef DEBUG
    OSL_TRACE("current panel list:");
    for (SharedPanelContainer::const_iterator
             iPanel(rCurrentPanels.begin()),
             iEnd(rCurrentPanels.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        OSL_TRACE("    panel %s", S2A((*iPanel)->GetId()));
    }

    OSL_TRACE("requested panels: ");
    for (ResourceManager::PanelContextDescriptorContainer::const_iterator
             iId(rRequestedPanels.begin()),
             iEnd(rRequestedPanels.end());
         iId!=iEnd;
         ++iId)
    {
        OSL_TRACE("    panel %s", S2A(iId->msId));
    }
#endif

    if (rCurrentPanels.size() != rRequestedPanels.size())
        return false;
    for (sal_Int32 nIndex=0,nCount=rCurrentPanels.size(); nIndex<nCount; ++nIndex)
    {
        if (rCurrentPanels[nIndex] == NULL)
            return false;
        if ( ! rCurrentPanels[nIndex]->GetId().equals(rRequestedPanels[nIndex].msId))
            return false;
    }
    return true;
}




SharedPanel SidebarController::CreatePanel (
    const OUString& rsPanelId,
    ::Window* pParentWindow,
    const OUString& rsMenuCommand)
{
    const PanelDescriptor* pPanelDescriptor = ResourceManager::Instance().GetPanelDescriptor(rsPanelId);
    if (pPanelDescriptor == NULL)
        return SharedPanel();

#ifdef DEBUG
    // Prevent the panel not being created in the same memory of an old panel.
    ::boost::scoped_array<char> pUnused (new char[sizeof(Panel)]);
    OSL_TRACE("allocated memory at %x", pUnused.get());
#endif

    // Create the panel which is the parent window of the UIElement.
    SharedPanel pPanel (new Panel(
        *pPanelDescriptor,
        pParentWindow,
        ::boost::bind(&Deck::RequestLayout, mpCurrentDeck.get()),
        rsMenuCommand.getLength()>0
            ? ::boost::bind(&SidebarController::ShowDetailMenu,this,rsMenuCommand)
            : ::boost::function<void(void)>()));

    // Create the XUIElement.
    Reference<ui::XUIElement> xUIElement (CreateUIElement(
            pPanel->GetComponentInterface(),
            pPanelDescriptor->msImplementationURL,
            pPanelDescriptor->mbWantsCanvas));
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
    const bool bWantsCanvas)
{
    try
    {
        const ::comphelper::ComponentContext aComponentContext (::comphelper::getProcessServiceFactory());
        const Reference<ui::XUIElementFactory> xUIElementFactory (
            aComponentContext.createComponent("com.sun.star.ui.UIElementFactoryManager"),
            UNO_QUERY_THROW);

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

        Reference<ui::XUIElement> xUIElement(
            xUIElementFactory->createUIElement(
                rsImplementationURL,
                Sequence<beans::PropertyValue>(aCreationArguments.getPropertyValues())),
            UNO_QUERY_THROW);

        return xUIElement;
    }
    catch(Exception& rException)
    {
        OSL_TRACE("caught exception: %s",
            OUStringToOString(rException.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        // For some reason we can not create the actual panel.
        // Probably because its factory was not properly registered.
        // TODO: provide feedback to developer to better pinpoint the
        // source of the error.

        return NULL;
    }
}




IMPL_LINK(SidebarController, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->GetId())
        {
            case VCLEVENT_WINDOW_GETFOCUS:
            case VCLEVENT_WINDOW_LOSEFOCUS:
                break;

            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_RESIZE:
                NotifyResize();
                break;

            case VCLEVENT_WINDOW_DATACHANGED:
                // Force an update of deck and tab bar to reflect
                // changes in theme (high contrast mode).
                Theme::HandleDataChange();
                mpParentWindow->Invalidate();
                break;

            case SFX_HINT_DYING:
                dispose();
                break;

            default:
                break;
        }
    }

    return sal_True;
}




void SidebarController::ShowPopupMenu (
    const Rectangle& rButtonBox,
    const ::std::vector<TabBar::DeckMenuData>& rDeckSelectionData,
    const ::std::vector<TabBar::DeckMenuData>& rDeckShowData) const
{
    ::boost::shared_ptr<PopupMenu> pMenu = CreatePopupMenu(rDeckSelectionData, rDeckShowData);
    pMenu->SetSelectHdl(LINK(this, SidebarController, OnMenuItemSelected));

    // pass toolbox button rect so the menu can stay open on button up
    Rectangle aBox (rButtonBox);
    aBox.Move(mpTabBar->GetPosPixel().X(), 0);
    pMenu->Execute(mpParentWindow, aBox, POPUPMENU_EXECUTE_DOWN);
}




void SidebarController::ShowDetailMenu (const ::rtl::OUString& rsMenuCommand) const
{
    try
    {
        util::URL aURL;
        aURL.Complete = rsMenuCommand;

        const ::comphelper::ComponentContext aComponentContext (::comphelper::getProcessServiceFactory());
        const Reference<util::XURLTransformer> xParser (
            aComponentContext.createComponent("com.sun.star.util.URLTransformer"),
            UNO_QUERY_THROW);
        xParser->parseStrict(aURL);
        Reference<frame::XDispatchProvider> xProvider (mxFrame, UNO_QUERY_THROW);
        Reference<frame::XDispatch> xDispatch (xProvider->queryDispatch(aURL, OUString(), 0));
        if (xDispatch.is())
            xDispatch->dispatch(aURL, Sequence<beans::PropertyValue>());
    }
    catch(Exception& rException)
    {
        OSL_TRACE("caught exception: %s",
            OUStringToOString(rException.Message, RTL_TEXTENCODING_ASCII_US).getStr());
    }
}




::boost::shared_ptr<PopupMenu> SidebarController::CreatePopupMenu (
    const ::std::vector<TabBar::DeckMenuData>& rDeckSelectionData,
    const ::std::vector<TabBar::DeckMenuData>& rDeckShowData) const
{
    ::boost::shared_ptr<PopupMenu> pMenu (new PopupMenu());
    FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
    if (pMenuWindow != NULL)
    {
        pMenuWindow->SetPopupModeFlags(pMenuWindow->GetPopupModeFlags() | FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE);
    }

    SidebarResource aLocalResource;

    // Add one entry for every tool panel element to individually make
    // them visible or hide them.
    {
        sal_Int32 nIndex (MID_FIRST_PANEL);
        for(::std::vector<TabBar::DeckMenuData>::const_iterator
                iItem(rDeckSelectionData.begin()),
                iEnd(rDeckSelectionData.end());
            iItem!=iEnd;
            ++iItem)
        {
            pMenu->InsertItem(nIndex, iItem->get<0>(), MIB_RADIOCHECK);
            pMenu->CheckItem(nIndex, iItem->get<2>());
            ++nIndex;
        }
    }

    pMenu->InsertSeparator();

    // Add entry for docking or un-docking the tool panel.
    if (mpParentWindow->IsFloatingMode())
        pMenu->InsertItem(MID_LOCK_TASK_PANEL, String(SfxResId(STR_SFX_DOCK)));
    else
        pMenu->InsertItem(MID_UNLOCK_TASK_PANEL, String(SfxResId(STR_SFX_UNDOCK)));

    // Add sub menu for customization (hiding of deck tabs.)
    PopupMenu* pCustomizationMenu = new PopupMenu();
    {
        sal_Int32 nIndex (MID_FIRST_HIDE);
        for(::std::vector<TabBar::DeckMenuData>::const_iterator
                iItem(rDeckShowData.begin()),
                iEnd(rDeckShowData.end());
            iItem!=iEnd;
            ++iItem)
        {
            pCustomizationMenu->InsertItem(nIndex, iItem->get<0>(), MIB_CHECKABLE);
            pCustomizationMenu->CheckItem(nIndex, iItem->get<2>());
            ++nIndex;
        }
    }

    pCustomizationMenu->InsertSeparator();
    pCustomizationMenu->InsertItem(MID_RESTORE_DEFAULT, String(SfxResId(STRING_RESTORE)));

    pMenu->InsertItem(MID_CUSTOMIZATION, String(SfxResId(STRING_CUSTOMIZATION)));
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




void SidebarController::CloseDeck (void)
{
    if ( ! mbIsDeckClosed)
    {
        mbIsDeckClosed = true;
        if ( ! mpParentWindow->IsFloatingMode())
            mnSavedSidebarWidth = SetChildWindowWidth(TabBar::GetDefaultWidth());
        mpParentWindow->SetStyle(mpParentWindow->GetStyle() & ~WB_SIZEABLE);

        if (mpCurrentDeck)
            mpCurrentDeck->Hide();

        NotifyResize();
    }
}




void SidebarController::OpenDeck (void)
{
    if (mbIsDeckClosed)
    {
        mbIsDeckClosed = false;
        SetChildWindowWidth(mnSavedSidebarWidth);

        if (mpCurrentDeck)
            mpCurrentDeck->Show();

        NotifyResize();
    }
}




FocusManager& SidebarController::GetFocusManager (void)
{
    return maFocusManager;
}




bool SidebarController::CanModifyChildWindowWidth (void) const
{
    SfxSplitWindow* pSplitWindow = dynamic_cast<SfxSplitWindow*>(mpParentWindow->GetParent());
    if (pSplitWindow == NULL)
    {
        OSL_ASSERT(pSplitWindow!=NULL);
        return 0;
    }

    sal_uInt16 nRow (0xffff);
    sal_uInt16 nColumn (0xffff);
    pSplitWindow->GetWindowPos(mpParentWindow, nColumn, nRow);

    sal_uInt16 nRowCount (pSplitWindow->GetWindowCount(nColumn));

    return nRowCount == 1;
}




sal_Int32 SidebarController::SetChildWindowWidth (const sal_Int32 nNewWidth)
{
    SfxSplitWindow* pSplitWindow = dynamic_cast<SfxSplitWindow*>(mpParentWindow->GetParent());
    if (pSplitWindow == NULL)
        return 0;

    sal_uInt16 nRow (0xffff);
    sal_uInt16 nColumn (0xffff);
    pSplitWindow->GetWindowPos(mpParentWindow, nColumn, nRow);
    const long nColumnWidth (pSplitWindow->GetLineSize(nColumn));

    Window* pWindow = mpParentWindow;
    const Point aWindowPosition (pWindow->GetPosPixel());
    const Size aWindowSize (pWindow->GetSizePixel());

    pSplitWindow->MoveWindow(
        mpParentWindow,
        Size(nNewWidth, aWindowSize.Height()),
        nColumn,
        nRow);

    return static_cast<sal_Int32>(nColumnWidth);
}




void SidebarController::RestrictWidth (void)
{
    SfxSplitWindow* pSplitWindow = dynamic_cast<SfxSplitWindow*>(mpParentWindow->GetParent());
    if (pSplitWindow != NULL)
    {
        const sal_uInt16 nId (pSplitWindow->GetItemId(mpParentWindow));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        // Minimum width is always that of the tabbar.
        const sal_Int32 nMinimumWidth (TabBar::GetDefaultWidth());
        // Maximum width depends on whether the deck is open or closed.
        const sal_Int32 nMaximumWidth (
            mbIsDeckClosed
                ? TabBar::GetDefaultWidth()
                : 400);
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(nMinimumWidth, nMaximumWidth));
        if (nMinimumWidth == nMaximumWidth)
            pSplitWindow->SetItemSize(nSetId, nMinimumWidth);
    }
}


} } // end of namespace sfx2::sidebar
