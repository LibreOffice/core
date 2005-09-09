/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneChildWindows.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:49:33 $
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

#include "PaneChildWindows.hxx"
#include "PaneDockingWindow.hrc"
#include "app.hrc"
#include "sdresid.hxx"
#include <sfx2/app.hxx>
#include <sfx2/dockwin.hxx>

namespace sd
{
    // We use SID_LEFT_PANE_IMPRESS and SID_LEFT_PANE_IMPRESS_DRAW to have
    // separate strings.  Internally we use SID_LEFT_PANE_IMPESS for
    // controlling the visibility of the left pane.
    SFX_IMPL_DOCKINGWINDOW(LeftPaneChildWindow, SID_LEFT_PANE_IMPRESS)
    SFX_IMPL_DOCKINGWINDOW(RightPaneChildWindow, SID_RIGHT_PANE)
}


#include "PaneDockingWindow.hxx"

namespace sd {

//===== LeftPaneChildWindow ===================================================

LeftPaneChildWindow::LeftPaneChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParentWindow, nId)
{
    pWindow = new PaneDockingWindow (
        pBindings,
        this,
        pParentWindow,
        SdResId(FLT_LEFT_PANE_DOCKING_WINDOW),
        PaneManager::PT_LEFT);
    eChildAlignment = SFX_ALIGN_LEFT;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
    SetHideNotDelete (TRUE);
};




LeftPaneChildWindow::~LeftPaneChildWindow (void)
{}






//===== RightPaneChildWindow ==================================================

RightPaneChildWindow::RightPaneChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParentWindow, nId)
{
    pWindow = new PaneDockingWindow (
        pBindings,
        this,
        pParentWindow,
        SdResId(FLT_RIGHT_PANE_DOCKING_WINDOW),
        PaneManager::PT_RIGHT);
    eChildAlignment = SFX_ALIGN_RIGHT;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
    SetHideNotDelete (TRUE);
};




RightPaneChildWindow::~RightPaneChildWindow (void)
{}




} // end of namespace ::sd
