/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneShells.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:41:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#ifndef SD_PANE_SHELLS_HXX
#include "PaneShells.hxx"
#endif

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




//===== RightPaneShell ========================================================

#undef ShellClass
#define ShellClass RightPaneShell

SFX_SLOTMAP(RightPaneShell)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(RightPaneShell, SfxShell, SdResId(STR_RIGHT_PANE_SHELL))
{
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::RightPaneChildWindow::GetChildWindowId());
}

TYPEINIT1(RightPaneShell, SfxShell);



RightPaneShell::RightPaneShell (void)
    : SfxShell()
{
    SetName(rtl::OUString::createFromAscii("RightPane"));
}




RightPaneShell::~RightPaneShell (void)
{
}

} // end of namespace ::sd
