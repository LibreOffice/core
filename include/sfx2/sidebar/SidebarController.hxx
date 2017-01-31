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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_SIDEBARCONTROLLER_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_SIDEBARCONTROLLER_HXX

#include <sal/config.h>

#include <sfx2/sidebar/AsynchronousCall.hxx>
#include <sfx2/sidebar/Context.hxx>
#include <sfx2/sidebar/FocusManager.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/TabBar.hxx>

#include <vcl/menu.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <boost/optional.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>


namespace
{
    typedef cppu::WeakComponentImplHelper <
        css::ui::XContextChangeEventListener,
        css::beans::XPropertyChangeListener,
        css::ui::XSidebar,
        css::frame::XStatusListener,
        css::frame::XFrameActionListener
        > SidebarControllerInterfaceBase;
}

class SfxSplitWindow;
class FixedBitmap;

namespace sfx2 { namespace sidebar {

class ContentPanelDescriptor;
class Deck;
class DeckDescriptor;
class SidebarDockingWindow;
class TabBar;
class TabBarConfiguration;

class SFX2_DLLPUBLIC SidebarController
    : private ::cppu::BaseMutex,
      public SidebarControllerInterfaceBase
{
public:
    SidebarController(
        SidebarDockingWindow* pParentWindow,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~SidebarController() override;
    SidebarController(const SidebarController&) = delete;
    SidebarController& operator=( const SidebarController& ) = delete;

    /** Return the SidebarController object that is associated with
        the given XFrame.
        @return
            When there is no SidebarController object for the given
            XFrame then <NULL/> is returned.
    */
    static SidebarController* GetSidebarControllerForFrame (
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static void registerSidebarForFrame(SidebarController* pController, const css::uno::Reference<css::frame::XController>& xFrame);

    static void unregisterSidebarForFrame(SidebarController* pController, const css::uno::Reference<css::frame::XController>& xFrame);

    // ui::XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEventObject)
        throw(css::uno::RuntimeException, std::exception) override;

    // beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange (const css::beans::PropertyChangeEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    // frame::XStatusListener
    virtual void SAL_CALL statusChanged (const css::frame::FeatureStateEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    // frame::XFrameActionListener
    virtual void SAL_CALL frameAction (const css::frame::FrameActionEvent& rEvent)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // ui::XSidebar
    virtual void SAL_CALL requestLayout()
        throw(css::uno::RuntimeException, std::exception) override;

    void NotifyResize();

    /** In some situations it is necessary to force an update of the
        current deck and its panels.  One reason is a change of the
        view scale.  Some panels can handle this only when
        constructed.  In this case we have to a context change and
        also force that all panels are destroyed and created new.
    */
    const static sal_Int32 SwitchFlag_NoForce = 0x00;
    const static sal_Int32 SwitchFlag_ForceSwitch = 0x01;
    const static sal_Int32 SwitchFlag_ForceNewDeck = 0x02;
    const static sal_Int32 SwitchFlag_ForceNewPanels = 0x02;

    const static sal_Int32 gnMaximumSidebarWidth = 400;

    void OpenThenSwitchToDeck(const OUString& rsDeckId);
    void OpenThenToggleDeck(const OUString& rsDeckId);

    /** Show only the tab bar, not the deck.
    */
    void RequestCloseDeck();

    /** Open the deck area and restore the parent window to its old width.
    */
    void RequestOpenDeck();

    /** Returns true when the given deck is the currently visible deck
     */
    bool IsDeckVisible(const OUString& rsDeckId);

    FocusManager& GetFocusManager() { return maFocusManager;}

    ResourceManager* GetResourceManager() { return mpResourceManager.get();}

   // std::unique_ptr<ResourceManager> GetResourceManager() { return mpResourceManager;}

    const Context& GetCurrentContext() const { return maCurrentContext;}
    bool IsDocumentReadOnly (void) const { return mbIsDocumentReadOnly;}

    void SwitchToDeck(const OUString& rsDeckId);
    void SwitchToDefaultDeck();

    void CreateDeck(const OUString& rDeckId);
    void CreateDeck(const OUString& rDeckId, const Context& rContext, bool bForceCreate = false);

    ResourceManager::DeckContextDescriptorContainer GetMatchingDecks();
    ResourceManager::PanelContextDescriptorContainer GetMatchingPanels(const OUString& rDeckId);

    void notifyDeckTitle(const OUString& targetDeckId);

    void updateModel(const css::uno::Reference<css::frame::XModel>& xModel);

    void disposeDecks();

    void FadeIn();
    void FadeOut();

private:

    VclPtr<Deck> mpCurrentDeck;
    VclPtr<SidebarDockingWindow> mpParentWindow;
    VclPtr<TabBar> mpTabBar;
    css::uno::Reference<css::frame::XFrame> mxFrame;
    Context maCurrentContext;
    Context maRequestedContext;
    css::uno::Reference<css::frame::XController> mxCurrentController;
    /// Use a combination of SwitchFlag_* as value.
    sal_Int32 mnRequestedForceFlags;
    OUString msCurrentDeckId;
    AsynchronousCall maPropertyChangeForwarder;
    AsynchronousCall maContextChangeUpdate;
    AsynchronousCall maAsynchronousDeckSwitch;

    /** Two flags control whether the deck is displayed or if only the
        tab bar remains visible.
        The mbIsDeckOpen flag stores the current state while
        mbIsDeckRequestedOpen stores how this state should be.  User
        actions like clicking on the deck closer affect the
        mbIsDeckRequestedOpen.  Normally both flags have the same
        value.  A document being read-only can prevent the deck from opening.
    */
    ::boost::optional<bool> mbIsDeckRequestedOpen;
    ::boost::optional<bool> mbIsDeckOpen;

    /** Before the deck is closed the sidebar width is saved into this variable,
        so that it can be restored when the deck is reopened.
    */
    sal_Int32 mnSavedSidebarWidth;
    FocusManager maFocusManager;
    css::uno::Reference<css::frame::XDispatch> mxReadOnlyModeDispatch;
    bool mbIsDocumentReadOnly;
    VclPtr<SfxSplitWindow> mpSplitWindow;
    /** When the user moves the splitter then we remember the
        width at that time.
    */
    sal_Int32 mnWidthOnSplitterButtonDown;
    /** Control that is temporarily used as replacement for the deck
        to indicate that when the current mouse drag operation ends, the
        sidebar will only show the tab bar.
    */
    VclPtr<vcl::Window> mpCloseIndicator;

    DECL_LINK(WindowEventHandler, VclWindowEvent&, void);
    /** Make maRequestedContext the current context.
    */
    void UpdateConfigurations();

    css::uno::Reference<css::ui::XUIElement> CreateUIElement (
        const css::uno::Reference<css::awt::XWindowPeer>& rxWindow,
        const OUString& rsImplementationURL,
        const bool bWantsCanvas,
        const Context& rContext);

    void CreatePanels(
        const OUString& rDeckId,
        const Context& rContext);
    VclPtr<Panel> CreatePanel (
        const OUString& rsPanelId,
        vcl::Window* pParentWindow,
        const bool bIsInitiallyExpanded,
        const Context& rContext,
        const VclPtr<Deck>& pDeck);

    void SwitchToDeck (
        const DeckDescriptor& rDeckDescriptor,
        const Context& rContext);

    void ShowPopupMenu (
        const Rectangle& rButtonBox,
        const ::std::vector<TabBar::DeckMenuData>& rMenuData) const;
    VclPtr<PopupMenu> CreatePopupMenu (
        const ::std::vector<TabBar::DeckMenuData>& rMenuData) const;
    DECL_LINK(OnMenuItemSelected, Menu*, bool);
    void BroadcastPropertyChange();

    /** The close of the deck changes the width of the child window.
        That is only possible if there is no other docking window docked above or below the sidebar.
        Return whether the width of the child window can be modified.
    */
    bool CanModifyChildWindowWidth();

    /** Set the child window container to a new width.
        Return the old width.
    */
    sal_Int32 SetChildWindowWidth (const sal_Int32 nNewWidth);

    /** Update the icons displayed in the title bars of the deck and
        the panels.  This is called once when a deck is created and
        every time when a data change event is processed.
    */
    void UpdateTitleBarIcons();

    void UpdateDeckOpenState();
    void RestrictWidth (sal_Int32 nWidth);
    SfxSplitWindow* GetSplitWindow();
    void ProcessNewWidth (const sal_Int32 nNewWidth);
    void UpdateCloseIndicator (const bool bIsIndicatorVisible);

    /** Typically called when a panel is focused via keyboard.
        Tries to scroll the deck up or down to make the given panel
        completely visible.
    */
    void ShowPanel (const Panel& rPanel);

    virtual void SAL_CALL disposing() override;

    std::unique_ptr<ResourceManager> mpResourceManager;

};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
