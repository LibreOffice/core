/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/devtools/DevelopmentToolChildWindow.hxx>
#include <sfx2/devtools/DevelopmentToolDockingWindow.hxx>
#include <sfx2/sfxsids.hrc>

SFX_IMPL_DOCKINGWINDOW_WITHID(DevelopmentToolChildWindow, SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW);

DevelopmentToolChildWindow::DevelopmentToolChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId,
                                                       SfxBindings* pBindings,
                                                       SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParentWindow, nId)
{
    VclPtr<DevelopmentToolDockingWindow> pWin
        = VclPtr<DevelopmentToolDockingWindow>::Create(pBindings, this, pParentWindow);
    SetWindow(pWin);
    SetAlignment(SfxChildAlignment::BOTTOM);
    pWin->Initialize(pInfo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
