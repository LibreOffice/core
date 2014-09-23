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

#include <map>

#include "AsynchronousCall.hxx"
#include "Context.hxx"
#include "FocusManager.hxx"
#include "Panel.hxx"
#include "ResourceManager.hxx"
#include "TabBar.hxx"

#include <vcl/menu.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/weakref.hxx>



namespace
{
    typedef ::cppu::WeakComponentImplHelper4 <
        css::ui::XContextChangeEventListener,
        css::beans::XPropertyChangeListener,
        css::ui::XSidebar,
        css::frame::XStatusListener
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

class SidebarController
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SidebarControllerInterfaceBase
{
public:
    SidebarController(
        SidebarDockingWindow* pParentWindow,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~SidebarController (void);

    /** Return the SidebarController object that is associated with
        the given XFrame.
        @return
            When there is no SidebarController object for the given
            XFrame then <NULL/> is returned.
    */
    static SidebarController* GetSidebarControllerForFrame (
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    // ui::XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEventObject)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange (const css::beans::PropertyChangeEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // frame::XStatusListener
    virtual void SAL_CALL statusChanged (const css::frame::FeatureStateEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ui::XSidebar
    virtual void SAL_CALL requestLayout (void)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void NotifyResize (void);

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

    void OpenThenSwitchToDeck (
        const ::rtl::OUString& rsDeckId);

    /** Show only the tab bar, not the deck.
    */
    void RequestCloseDeck (void);

    /** Open the deck area and restore the parent window to its old width.
    */
    void RequestOpenDeck (void);

    FocusManager& GetFocusManager (void) { return maFocusManager;}

private:
    typedef ::std::map<
        const css::uno::Reference<css::frame::XFrame>,
        css::uno::WeakReference<SidebarController>
    > SidebarControllerContainer;
    static SidebarControllerContainer maSidebarControllerContainer;

    ::boost::scoped_ptr<Deck> mpCurrentDeck;
    SidebarDockingWindow* mpParentWindow;
    ::boost::scoped_ptr<TabBar> mpTabBar;
    css::uno::Reference<css::frame::XFrame> mxFrame;
    Context maCurrentContext;
    Context maRequestedContext;
    /// Use a combination of SwitchFlag_* as value.
    sal_Int32 mnRequestedForceFlags;
    ::rtl::OUString msCurrentDeckId;
    ::rtl::OUString msCurrentDeckTitle;
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
    bool mbCanDeckBeOpened;

    /** Before the deck is closed the sidebar width is saved into this variable,
        so that it can be restored when the deck is reopended.
    */
    sal_Int32 mnSavedSidebarWidth;
    FocusManager maFocusManager;
    css::uno::Reference<css::frame::XDispatch> mxReadOnlyModeDispatch;
    bool mbIsDocumentReadOnly;
    SfxSplitWindow* mpSplitWindow;
    /** When the user moves the splitter then we remember the
        width at that time.
    */
    sal_Int32 mnWidthOnSplitterButtonDown;
    /** Control that is temporarily used as replacement for the deck
        to indicate that when the current mouse drag operation ends, the
        sidebar will only show the tab bar.
    */
    ::boost::scoped_ptr<vcl::Window> mpCloseIndicator;

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
    /** Make maRequestedContext the current context.
    */
    void UpdateConfigurations (void);

    css::uno::Reference<css::ui::XUIElement> CreateUIElement (
        const css::uno::Reference<css::awt::XWindowPeer>& rxWindow,
        const ::rtl::OUString& rsImplementationURL,
        const bool bWantsCanvas,
        const Context& rContext);
    SharedPanel CreatePanel (
        const ::rtl::OUString& rsPanelId,
        vcl::Window* pParentWindow,
        const bool bIsInitiallyExpanded,
        const Context& rContext);
    void SwitchToDeck (
        const ::rtl::OUString& rsDeckId);
    void SwitchToDeck (
        const DeckDescriptor& rDeckDescriptor,
        const Context& rContext);
    void ShowPopupMenu (
        const Rectangle& rButtonBox,
        const ::std::vector<TabBar::DeckMenuData>& rMenuData) const;
    ::boost::shared_ptr<PopupMenu> CreatePopupMenu (
        const ::std::vector<TabBar::DeckMenuData>& rMenuData) const;
    DECL_LINK(OnMenuItemSelected, Menu*);
    void BroadcastPropertyChange (void);

    /** The close of the deck changes the width of the child window.
        That is only possible if there is no other docking window docked above or below the sidebar.
        Return whether the width of the child window can be modified.
    */
    bool CanModifyChildWindowWidth (void);

    /** Set the child window container to a new width.
        Return the old width.
    */
    sal_Int32 SetChildWindowWidth (const sal_Int32 nNewWidth);

    /** Update the icons displayed in the title bars of the deck and
        the panels.  This is called once when a deck is created and
        every time when a data change event is processed.
    */
    void UpdateTitleBarIcons (void);

    void UpdateDeckOpenState (void);
    void RestrictWidth (sal_Int32 nWidth);
    SfxSplitWindow* GetSplitWindow (void);
    void ProcessNewWidth (const sal_Int32 nNewWidth);
    void UpdateCloseIndicator (const bool bIsIndicatorVisible);

    /** Typically called when a panel is focused via keyboard.
        Tries to scroll the deck up or down to make the given panel
        completely visible.
    */
    void ShowPanel (const Panel& rPanel);

    Context GetCurrentContext (void) const { return maCurrentContext;}

    virtual void SAL_CALL disposing (void) SAL_OVERRIDE;
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
