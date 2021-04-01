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
#pragma once

#include <sfx2/sidebar/Panel.hxx>
#include <tools/link.hxx>
#include <vcl/keycod.hxx>

namespace weld {
class Widget;
}

namespace sfx2::sidebar {

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
    FocusManager(const std::function<void(const Panel&)>& rShowPanelFunctor);
    ~FocusManager();

    /** Forget all panels and buttons.  Remove all window listeners.
    */
    void Clear();

    /** Transfer the focus into the sidebar tree of windows.  This is
        typically called from the SidebarChildWindow as result of
        pressing the F6 key.
    */
    void GrabFocus();
    void GrabFocusPanel();

    void SetDeck(Deck* pDeck);
    void SetPanels(const SharedPanelContainer& rPanels);
    void SetButtons(const std::vector<weld::Widget*>& rButtons);

private:
    VclPtr<Deck> mxDeck;
    DeckTitleBar* mpDeckTitleBar;
    SharedPanelContainer maPanels;
    std::vector<weld::Widget*> maButtons;
    const std::function<void(const Panel&)> maShowPanelFunctor;

    enum PanelComponent
    {
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
        FocusLocation(const PanelComponent eComponent, const sal_Int32 nIndex);
    };

    /** Listen for key events for panels and buttons.
    */
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    void ClearPanels();
    void ClearButtons();

    /** Let the focus manager listen for window events for the given
        window.
    */
    void RegisterWindow(weld::Widget& rWidget);
    static void UnregisterWindow(weld::Widget& rWidget);

    void FocusDeckTitle();
    bool IsDeckTitleVisible() const;
    bool IsPanelTitleVisible(const sal_Int32 nPanelIndex) const;

    /** Set the focus to the title bar of the panel or, if the
        title bar is not visible, directly to the panel.
        @param nPanelIndex
            Index of the panel to focus.
        @param bFallbackToDeckTitle
            When the panel title bar is not visible then The fallback
            bias defines whether to focus the deck (true) or the panel
            content (false) will be focused instead.
    */
    void FocusPanel(const sal_Int32 nPanelIndex,
                    const bool bFallbackToDeckTitle);

    void FocusPanelContent(const sal_Int32 nPanelIndex);
    void FocusButton(const sal_Int32 nButtonIndex);
    void MoveFocusInsidePanel(const FocusLocation& rLocation,
                              const sal_Int32 nDirection);
    bool MoveFocusInsideDeckTitle(const FocusLocation& rLocation,
                                  const sal_Int32 nDirection);

    bool HandleKeyEvent(const vcl::KeyCode& rKeyCode,
                        const FocusLocation& rLocation);

    FocusLocation GetFocusLocation() const;

};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
