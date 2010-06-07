/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"
#include "PaneShells.hxx"

#include "PaneChildWindows.hxx"

#include "glob.hrc"
#include "sdresid.hxx"

#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>

namespace sd {

//===== LeftImpressPaneShell ==================================================

#define ShellClass LeftImpressPaneShell

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
    SetName(rtl::OUString::createFromAscii("LeftImpressPane"));
}




LeftImpressPaneShell::~LeftImpressPaneShell (void)
{
}




//===== LeftDrawPaneShell =====================================================

#undef ShellClass
#define ShellClass LeftDrawPaneShell

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
    SetName(rtl::OUString::createFromAscii("LeftDrawPane"));
}




LeftDrawPaneShell::~LeftDrawPaneShell (void)
{
}




//===== ToolPanelPaneShell ========================================================

#undef ShellClass
#define ShellClass ToolPanelPaneShell

SFX_SLOTMAP( ToolPanelPaneShell )
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE( ToolPanelPaneShell, SfxShell, SdResId( STR_TOOL_PANEL_SHELL ) )
{
    SFX_CHILDWINDOW_REGISTRATION( ::sd::ToolPanelChildWindow::GetChildWindowId() );
}

TYPEINIT1( ToolPanelPaneShell, SfxShell );

ToolPanelPaneShell::ToolPanelPaneShell()
    :SfxShell()
{
    SetName( ::rtl::OUString::createFromAscii( "ToolPanel" ) );
}

ToolPanelPaneShell::~ToolPanelPaneShell(void)
{
}

} // end of namespace ::sd
