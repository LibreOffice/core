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

class HelpEvent;
class Menu;
class Rectangle;
class Window;

/** Common ancestor for MenuFloatingWindow and MenuBarWindow.

The menu can be a floating window, or a menu bar.  Even though this has
'Window' in the name, it is not derived from the VCL's Window class, as the
MenuFloatingWindow's or MenuBarWindow's already are VCL Windows.
*/
class MenuWindow
{
public:

    /// Sets up some visual properties of the underlying window.
    void ImplInitMenuWindow(Window* pWin, bool bFont, bool bMenuBar);

    /// Show the appropriate help tooltip.
    bool ImplHandleHelpEvent(Window* pMenuWindow, Menu* pMenu, sal_uInt16 nHighlightedItem,
            const HelpEvent& rHEvt, const Rectangle &rHighlightRect);
};

#endif // INCLUDED_VCL_SOURCE_WINDOW_MENUWINDOW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
