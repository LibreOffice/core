/*************************************************************************
 *
 *  $RCSfile: PaneChildWindows.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:49:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "PaneChildWindows.hxx"
#include "PaneDockingWindow.hrc"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include <sfx2/app.hxx>
#include <sfx2/dockwin.hxx>

namespace sd
{
    SFX_IMPL_DOCKINGWINDOW(LeftPaneChildWindow, SID_LEFT_PANE)
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
        PaneManager::PT_LEFT,
        SdResId(STR_LEFT_PANE_TITLE));
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
        PaneManager::PT_RIGHT,
        SdResId(STR_RIGHT_PANE_TITLE));
    eChildAlignment = SFX_ALIGN_RIGHT;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize (pInfo);
    SetHideNotDelete (TRUE);
};




RightPaneChildWindow::~RightPaneChildWindow (void)
{}




} // end of namespace ::sd
