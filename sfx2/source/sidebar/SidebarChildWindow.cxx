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

#include "TabBar.hxx"
#include "sfx2/sidebar/SidebarChildWindow.hxx"
#include "SidebarDockingWindow.hxx"
#include "sfx2/sfxsids.hrc"
#include "helpid.hrc"
#include "sfx2/dockwin.hxx"
#include <sfx2/sidebar/ResourceDefinitions.hrc>


namespace sfx2 { namespace sidebar {


SFX_IMPL_DOCKINGWINDOW_WITHID(SidebarChildWindow, SID_SIDEBAR);


SidebarChildWindow::SidebarChildWindow (
    Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParentWindow, nId)
{
    this->pWindow = new SidebarDockingWindow(
        pBindings,
        *this,
        pParentWindow,
        WB_STDDOCKWIN | WB_OWNERDRAWDECORATION | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
    eChildAlignment = SFX_ALIGN_RIGHT;

    this->pWindow->SetHelpId(HID_SIDEBAR_WINDOW);
    this->pWindow->SetOutputSizePixel(Size(GetDefaultWidth(this->pWindow), 450));

    SfxDockingWindow* pDockingParent = dynamic_cast<SfxDockingWindow*>(this->pWindow);
    if (pDockingParent != NULL)
        pDockingParent->Initialize(pInfo);
    SetHideNotDelete(sal_True);

    this->pWindow->Show();
}




sal_Int32 SidebarChildWindow::GetDefaultWidth (Window* pWindow)
{
    if (pWindow != NULL)
    {
        // Width of the paragraph panel.
        const static sal_Int32 nMaxPropertyPageWidth (115);

        return pWindow->LogicToPixel(Point(nMaxPropertyPageWidth,1), MAP_APPFONT).X()
            + TabBar::GetDefaultWidth();
    }
    else
        return 0;
}


} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
