/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneChildWindows.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-24 16:15:40 $
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
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sd
{
    // We use SID_LEFT_PANE_IMPRESS and SID_LEFT_PANE_IMPRESS_DRAW to have
    // separate strings.  Internally we use SID_LEFT_PANE_IMPESS for
    // controlling the visibility of the left pane.
    SFX_IMPL_DOCKINGWINDOW(LeftPaneChildWindow, SID_LEFT_PANE_IMPRESS)
    SFX_IMPL_DOCKINGWINDOW(RightPaneChildWindow, SID_RIGHT_PANE)
}


#include "PaneDockingWindow.hxx"
#include "ViewShellBase.hxx"

namespace sd {

//===== LeftPaneChildWindow ===================================================

LeftPaneChildWindow::LeftPaneChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParentWindow, nId)
{
    ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pBindings->GetDispatcher()->GetFrame());
    if (pBase != NULL)
    {
        PaneManager& rPaneManager (pBase->GetPaneManager());
        pWindow = new PaneDockingWindow (
            pBindings,
            this,
            pParentWindow,
            rPaneManager.GetDockingWindowTitle(PaneManager::PT_LEFT),
            PaneManager::PT_LEFT,
            rPaneManager.GetWindowTitle(PaneManager::PT_LEFT));
        eChildAlignment = SFX_ALIGN_LEFT;
        static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
        SetHideNotDelete (TRUE);
        rPaneManager.SetWindow(PaneManager::PT_LEFT, pWindow);
    }
}




LeftPaneChildWindow::~LeftPaneChildWindow (void)
{
    ViewShellBase* pBase = NULL;
    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(pWindow);
    if (pDockingWindow != NULL)
        pBase = ViewShellBase::GetViewShellBase(
            pDockingWindow->GetBindings().GetDispatcher()->GetFrame());
    if (pBase != NULL)
    {
        // Tell the ViewShellBase that the window of this slide sorter is
        // not available anymore.
        pBase->GetPaneManager().SetWindow(PaneManager::PT_LEFT, NULL);
    }
}






//===== RightPaneChildWindow ==================================================

RightPaneChildWindow::RightPaneChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParentWindow, nId)
{
    ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pBindings->GetDispatcher()->GetFrame());
    if (pBase != NULL)
    {
        PaneManager& rPaneManager (pBase->GetPaneManager());
        pWindow = new PaneDockingWindow (
            pBindings,
            this,
            pParentWindow,
            rPaneManager.GetDockingWindowTitle(PaneManager::PT_RIGHT),
            PaneManager::PT_RIGHT,
            rPaneManager.GetWindowTitle(PaneManager::PT_RIGHT));
        eChildAlignment = SFX_ALIGN_RIGHT;
        static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
        SetHideNotDelete (TRUE);
        rPaneManager.SetWindow(PaneManager::PT_RIGHT, pWindow);
    }
};




RightPaneChildWindow::~RightPaneChildWindow (void)
{
    ViewShellBase* pBase = NULL;
    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(pWindow);
    if (pDockingWindow != NULL)
        pBase = ViewShellBase::GetViewShellBase(
            pDockingWindow->GetBindings().GetDispatcher()->GetFrame());
    if (pBase != NULL)
    {
        // Tell the ViewShellBase that the window of this slide sorter is
        // not available anymore.
        pBase->GetPaneManager().SetWindow(PaneManager::PT_RIGHT, NULL);
    }
}




} // end of namespace ::sd
