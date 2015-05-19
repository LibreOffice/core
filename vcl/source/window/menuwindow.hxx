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

#ifndef INCLUDED_VCL_SOURCE_WINDOW_MENUWINDOW_HXX
#define INCLUDED_VCL_SOURCE_WINDOW_MENUWINDOW_HXX

#include <sal/types.h>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/event.hxx>

class HelpEvent;
class Image;
class Menu;
class MenuBar;
class Rectangle;
namespace vcl { class Window; }

/** Common ancestor for MenuFloatingWindow and MenuBarWindow.

The menu can be a floating window, or a menu bar.  Even though this has
'Window' in the name, it is not derived from the VCL's Window class, as the
MenuFloatingWindow's or MenuBarWindow's already are VCL Windows.

TODO: move here stuff that was a mentioned previously when there was no
common class for MenuFloatingWindow and MenuBarWindow:

// a basic class for both (due to pActivePopup, Timer,...) would be nice,
// but a container class should have been created then, as they
// would be derived from different windows
// In most functions we would have to create exceptions for
// menubar, popupmenu, hence we made two classes

*/
class MenuWindow
{
public:
    virtual ~MenuWindow() {}

    /// Show the appropriate help tooltip.
    static bool ImplHandleHelpEvent(vcl::Window* pMenuWindow, Menu* pMenu, sal_uInt16 nHighlightedItem,
            const HelpEvent& rHEvt, const Rectangle &rHighlightRect);
};

/// Interface for the MenuBarWindow functionality.
class IMenuBarWindow : public MenuWindow
{
public:
    virtual ~IMenuBarWindow() {}

    virtual sal_uLong GetFocusId() const = 0;
    virtual void SetFocusId(sal_uLong nId) = 0;

    virtual bool HandleKeyEvent(const KeyEvent& rKEvent, bool bFromMenu = true) = 0;
    virtual void LayoutChanged() = 0;
    virtual void PopupClosed(Menu* pMenu) = 0;
    virtual void ShowButtons(bool bClose, bool bFloat, bool bHide) = 0;

    virtual void ChangeHighlightItem(sal_uInt16 n, bool bSelectPopupEntry, bool bAllowRestoreFocus = true, bool bDefaultToDocument = true ) = 0;
    virtual sal_uInt16 GetHighlightedItem() const = 0;

    virtual void SetAutoPopup(bool bAuto) = 0;
    virtual void SetMenu(MenuBar* pMenu) = 0;
    virtual void SetHeight(long nHeight) = 0;
    virtual void KillActivePopup() = 0;

    /// Add an arbitrary button to the menubar that will appear next to the close button.
    virtual sal_uInt16 AddMenuBarButton(const Image&, const Link<>&, const OUString&, sal_uInt16 nPos) = 0;
    virtual void SetMenuBarButtonHighlightHdl(sal_uInt16 nId, const Link<>&) = 0;
    virtual Rectangle GetMenuBarButtonRectPixel(sal_uInt16 nId) = 0;
    virtual void RemoveMenuBarButton(sal_uInt16 nId) = 0;
    virtual bool HandleMenuButtonEvent(sal_uInt16 i_nButtonId) = 0;
};

#endif // INCLUDED_VCL_SOURCE_WINDOW_MENUWINDOW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
