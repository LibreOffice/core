/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ToolPanelChildWindow.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "ToolPanelChildWindow.hxx"
#include "ToolPanelDockingWindow.hxx"

#include "sdresid.hxx"
#include "app.hrc"
#include "sfx2/app.hxx"


namespace sd { namespace toolpanel {



ToolPanelChildWindow::ToolPanelChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParentWindow, nId)
{
    pWindow = new ToolPanelDockingWindow (pBindings, this, pParentWindow);
    eChildAlignment = SFX_ALIGN_RIGHT;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
    //  SetHideNotDelete (TRUE);
};




ToolPanelChildWindow::~ToolPanelChildWindow()
{}


SFX_IMPL_DOCKINGWINDOW(ToolPanelChildWindow, SID_TOOLPANEL)




} } // end of namespace ::sd::toolpanel
