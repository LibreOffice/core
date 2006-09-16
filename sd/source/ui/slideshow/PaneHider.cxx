/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneHider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:02:12 $
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
#include "PaneHider.hxx"

#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "slideshow.hxx"
#include "PaneManager.hxx"

namespace sd {

PaneHider::PaneHider (const ViewShell& rViewShell)
    : mrViewShell(rViewShell),
      mbWindowVisibilitySaved(false),
      mbOriginalLeftPaneWindowVisibility(false),
      mbOriginalRightPaneWindowVisibility(false)
{
    // Hide the left and right pane windows when a slideshow exists and is
    // not full screen.
    Slideshow* pSlideShow = mrViewShell.GetSlideShow();
    if (pSlideShow!=NULL && !pSlideShow->isFullScreen())
    {
        PaneManager& rPaneManager (mrViewShell.GetViewShellBase().GetPaneManager());
        mbOriginalLeftPaneWindowVisibility = rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_LEFT,
            false,
            PaneManager::CM_SYNCHRONOUS);
        mbOriginalRightPaneWindowVisibility = rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_RIGHT,
            false,
            PaneManager::CM_SYNCHRONOUS);

        mbWindowVisibilitySaved = true;
    }
}




PaneHider::~PaneHider (void)
{
    if (mbWindowVisibilitySaved)
    {
        PaneManager& rPaneManager (mrViewShell.GetViewShellBase().GetPaneManager());
        rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_LEFT,
            mbOriginalLeftPaneWindowVisibility,
            PaneManager::CM_ASYNCHRONOUS);
        rPaneManager.RequestWindowVisibilityChange(
            PaneManager::PT_RIGHT,
            mbOriginalRightPaneWindowVisibility,
            PaneManager::CM_ASYNCHRONOUS);
    }
}

} // end of namespace sd
