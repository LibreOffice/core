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

#include "PaneShells.hxx"

#include "PaneChildWindows.hxx"

#include "glob.hrc"
#include "sdresid.hxx"

#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>

namespace sd {

//===== LeftImpressPaneShell ==================================================

SFX_SLOTMAP(LeftImpressPaneShell)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(LeftImpressPaneShell, SfxShell, SdResId(STR_LEFT_IMPRESS_PANE_SHELL))
{
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::LeftPaneImpressChildWindow::GetChildWindowId());
}

TYPEINIT1(LeftImpressPaneShell, SfxShell);



LeftImpressPaneShell::LeftImpressPaneShell (void)
    : SfxShell()
{
    SetName(OUString("LeftImpressPane"));
}




LeftImpressPaneShell::~LeftImpressPaneShell (void)
{
}




//===== LeftDrawPaneShell =====================================================

SFX_SLOTMAP(LeftDrawPaneShell)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(LeftDrawPaneShell, SfxShell, SdResId(STR_LEFT_DRAW_PANE_SHELL))
{
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::LeftPaneDrawChildWindow::GetChildWindowId());
}

TYPEINIT1(LeftDrawPaneShell, SfxShell);



LeftDrawPaneShell::LeftDrawPaneShell (void)
    : SfxShell()
{
    SetName(OUString("LeftDrawPane"));
}




LeftDrawPaneShell::~LeftDrawPaneShell (void)
{
}


} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
