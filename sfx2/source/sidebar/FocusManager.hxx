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

/** Concentrate all focus handling in this class.
    There are two rings of windows that accept the input focus: panels
    and tab bar buttons.
    Arrow keys move the focus between them.  Tab moves focus between rings.
*/
class FocusManager
{
public:
    FocusManager (void);
    ~FocusManager (void);

    /** Forget all panels and buttons.  Remove all window listeners.
    */
    void Clear (void);

    /** Transfer the focus into the sidebar tree of windows.  This is
        typically called from the SidebarChildWindow as result of
        pressing the F6 key.
    */
    void GrabFocus (void);

    /** Handle the key event that was sent to the docking window.
    */
    long NotifyDockingWindowEvent (const KeyEvent& rKeyEvent);

    void SetPanels (const SharedPanelContainer& rPanels);

    void SetButtons (const ::std::vector<Button*>& rButtons);

private:
    ::std::vector<Panel*> maPanels;
    ::std::vector<Button*> maButtons;
    Window* mpTopLevelWindow;

    /** Listen for key events for panels and buttons.
    */
    DECL_LINK(WindowEventListener, VclSimpleEvent*);

    void ClearPanels (void);
    void ClearButtons (void);

    /** Let the focus manager listen for window events for the given
        window.
    */
    void RegisterWindow (Window& rWindow);
    void UnregisterWindow (Window& rWindow);
    void RegisterTopLevelListener (void);

    /** Remove the window from the panel or the button container.
    */
    void RemoveWindow (Window& rWindow);

    sal_Int32 GetPanelIndex (const Window& rWindow) const;
    sal_Int32 GetButtonIndex (const Window& rWindow) const;
    bool IsAnyPanelFocused (void) const;
    bool IsAnyButtonFocused (void) const;

    /** Set the focus to the title bar of the panel or, if the the
        title bar is not visible, directly to the panel.
    */
    void FocusPanel (const sal_Int32 nPanelIndex);
    void FocusPanelContent (const sal_Int32 nPanelIndex);
    void FocusButton (const sal_Int32 nButtonIndex);
    void ClickButton (const sal_Int32 nButtonIndex);
    bool MoveFocusInsidePanel (
        const sal_Int32 nPanelIndex,
        const sal_Int32 nDirection);

    void HandleKeyEvent (
        const KeyCode& rKeyCode,
        const Window& rWindow);

    void SetTopLevelWindow (Window* pWindow);
    void HandleTopLevelEvent (VclWindowEvent& rEvent);
};

} } // end of namespace sfx2::sidebar

#endif
