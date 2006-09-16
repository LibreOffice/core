/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolPanelChildWindow.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:16:54 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "ToolPanelChildWindow.hxx"
#include "ToolPanelDockingWindow.hxx"

#include "sdresid.hxx"
#include "app.hrc"
#ifndef _SFXAPP_HXX
#include "sfx2/app.hxx"
#endif


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
