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

#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <sfx2/sfxsids.hrc>
#include <helpids.h>
#include <sfx2/dockwin.hxx>

namespace sfx2 { namespace sidebar {

SFX_IMPL_DOCKINGWINDOW_WITHID(SidebarChildWindow, SID_SIDEBAR);

SidebarChildWindow::SidebarChildWindow (vcl::Window* pParentWindow, sal_uInt16 nId,
                                        SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParentWindow, nId)
{
    SetWindow(VclPtr<SidebarDockingWindow>::Create(pBindings, *this, pParentWindow,
                                                       WB_STDDOCKWIN | WB_OWNERDRAWDECORATION |
                                                       WB_CLIPCHILDREN | WB_SIZEABLE |
                                                       WB_3DLOOK | WB_ROLLABLE));
    SetAlignment(SfxChildAlignment::RIGHT);

    GetWindow()->SetHelpId(HID_SIDEBAR_WINDOW);
    GetWindow()->SetOutputSizePixel(Size(GetDefaultWidth(GetWindow()), 450));

    SfxDockingWindow* pDockingParent = dynamic_cast<SfxDockingWindow*>(GetWindow());
    if (pDockingParent != nullptr)
    {
        if (pInfo && pInfo->aExtraString.isEmpty() && pInfo->aModule != "sdraw" && pInfo->aModule != "simpress")
        {
            // When this is the first start (never had the sidebar open yet),
            // default to non-expanded sidebars in Writer and Calc.
            //
            // HACK: unfortunately I haven't found a clean solution to do
            // this, so do it this way:
            //
            pDockingParent->SetSizePixel(Size(TabBar::GetDefaultWidth() * GetWindow()->GetDPIScaleFactor(),
                        pDockingParent->GetSizePixel().Height()));
        }
        pDockingParent->Initialize(pInfo);
    }
    SetHideNotDelete(true);

    GetWindow()->Show();
}

sal_Int32 SidebarChildWindow::GetDefaultWidth (vcl::Window const * pWindow)
{
    if (pWindow != nullptr)
    {
        // Width of the paragraph panel.
        const static sal_Int32 nMaxPropertyPageWidth(146);

        return pWindow->LogicToPixel(Point(nMaxPropertyPageWidth,1), MapMode(MapUnit::MapAppFont)).X()
            + TabBar::GetDefaultWidth() * pWindow->GetDPIScaleFactor();
    }
    else
        return 0;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
