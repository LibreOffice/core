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

/** Concentrate all focus handling in this class.

    There is one ring of windows that accept the input focus which are
    cycled through with the tab key:
    - the panel title bars
    - the tab bar

    When the focus is on a panel expander:
    - tab key moves the focus to the next or previous panel or the tab bar
    - arrow keys move the focus to the panel toolbox
    - enter key moves the focus to the panel content
    - escape key moves the focus to the document

    When the focus is on a panel toolbox:
    - tab key moves the focus to the next or previous panel or the tab bar
    - arrow keys move the focus to the panel expander
    - escape key moves the focus to the document

    When the focus is in the panel content:
    - normal keyboard navigation for dialogs applies to controls inside the panel
    - escape key moves the focus to the panel expander or the tab bar menu button for single panel
      decks

    When the focus is on the tab bar:
    - tab key moves the focus to the first or last panel expander or panel content for single panel
      decks
    - arrow keys move the focus among the tab bar buttons
    - escape key moves the focus to the document

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

    void SetPanels(const SharedPanelContainer& rPanels);
    void SetButtons(const std::vector<weld::Widget*>& rButtons);

private:
    std::vector<VclPtr<Panel> > maPanels;
    std::vector<weld::Widget*> maButtons;
    const std::function<void(const Panel&)> maShowPanelFunctor;

    enum PanelComponent
    {
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
    DECL_LINK(ChildEventListener, VclWindowEvent&, void);

    void ClearPanels();
    void ClearButtons();

    /** Let the focus manager listen for window events for the given
        window.
    */
    void RegisterWindow(weld::Widget& rWidget);
    static void UnregisterWindow(weld::Widget& rWidget);

    bool IsPanelTitleVisible(const sal_Int32 nPanelIndex) const;

    /** Set the focus to the title bar of the panel or, if the
        title bar is not visible, directly to the panel.
        @param nPanelIndex
            Index of the panel to focus.
        @param bFallbackToMenuButton
            When the panel title bar is not visible then the fallback
            bias defines whether to focus the menu button (true) or the panel
            content (false) will be focused instead.
    */
    void FocusPanel(const sal_Int32 nPanelIndex,
                    const bool bFallbackToMenuButton);

    void FocusPanelContent(const sal_Int32 nPanelIndex);
    void FocusButton(const sal_Int32 nButtonIndex);

    bool HandleKeyEvent(const vcl::KeyCode& rKeyCode,
                        const FocusLocation& rLocation);

    FocusLocation GetFocusLocation(const vcl::Window& rWindow) const;
    FocusLocation GetFocusLocation() const;

};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
