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
#ifndef SFX_SIDEBAR_FOCUS_MANAGER_HXX
#define SFX_SIDEBAR_FOCUS_MANAGER_HXX

#include "Panel.hxx"
#include <tools/link.hxx>

class Button;
class KeyCode;
class VclSimpleEvent;


namespace sfx2 { namespace sidebar {

class DeckTitleBar;

/** Concentrate all focus handling in this class.

    There is one ring of windows that accept the input focus which are
    cycled through with the arrow keys:
    - the closer in the deck title (present only when docked)
    - the panel title bars
    - the tab bar items

    When the focus is in a panel title then focus travels over
    - the panel title
    - the panel closer
    - the panel content

    Once the focus is in the panel content then focus cycles through
    all controls inside the panel but not back to the title bar of
    the panel.  Escape places the focus back in the panel title.
*/
class FocusManager
{
public:
    FocusManager (const ::boost::function<void(const Panel&)>& rShowPanelFunctor);
    ~FocusManager (void);

    /** Forget all panels and buttons.  Remove all window listeners.
    */
    void Clear (void);

    /** Transfer the focus into the sidebar tree of windows.  This is
        typically called from the SidebarChildWindow as result of
        pressing the F6 key.
    */
    void GrabFocus (void);

    void SetDeckTitle (DeckTitleBar* pDeckTitleBar);
    void SetPanels (const SharedPanelContainer& rPanels);
    void SetButtons (const ::std::vector<Button*>& rButtons);

private:
    DeckTitleBar* mpDeckTitleBar;
    ::std::vector<Panel*> maPanels;
    ::std::vector<Button*> maButtons;
    const ::boost::function<void(const Panel&)> maShowPanelFunctor;
    bool mbObservingContentControlFocus;
    Window* mpFirstFocusedContentControl;
    Window* mpLastFocusedWindow;

    enum PanelComponent
    {
        PC_DeckTitle,
        PC_DeckToolBox,
        PC_PanelTitle,
        PC_PanelToolBox,
        PC_PanelContent,
        PC_TabBar,
        PC_None
    };
    class FocusLocation
    {
    public:
        PanelComponent meComponent;
        sal_Int32 mnIndex;
        FocusLocation (const PanelComponent eComponent, const sal_Int32 nIndex);
    };

    /** Listen for key events for panels and buttons.
    */
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
    DECL_LINK(ChildEventListener, VclSimpleEvent*);

    void ClearPanels (void);
    void ClearButtons (void);

    /** Let the focus manager listen for window events for the given
        window.
    */
    void RegisterWindow (Window& rWindow);
    void UnregisterWindow (Window& rWindow);

    /** Remove the window from the panel or the button container.
    */
    void RemoveWindow (Window& rWindow);

    void FocusDeckTitle (void);
    bool IsDeckTitleVisible (void) const;
    bool IsPanelTitleVisible (const sal_Int32 nPanelIndex) const;

    /** Set the focus to the title bar of the panel or, if the the
        title bar is not visible, directly to the panel.
        @param nPanelIndex
            Index of the panel to focus.
        @param bFallbackToDeckTitle
            When the panel title bar is not visible then The fallback
            bias defines whether to focus the deck (true) or the panel
            content (false) will be focused instead.
    */
    void FocusPanel (
        const sal_Int32 nPanelIndex,
        const bool bFallbackToDeckTitle);

    void FocusPanelContent (const sal_Int32 nPanelIndex);
    void FocusButton (const sal_Int32 nButtonIndex);
    void ClickButton (const sal_Int32 nButtonIndex);
    bool MoveFocusInsidePanel (
        const FocusLocation aLocation,
        const sal_Int32 nDirection);
    bool MoveFocusInsideDeckTitle (
        const FocusLocation aLocation,
        const sal_Int32 nDirection);

    void HandleKeyEvent (
        const KeyCode& rKeyCode,
        const Window& rWindow);

    FocusLocation GetFocusLocation (const Window& rWindow) const;

};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
