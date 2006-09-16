/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:09:17 $
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

#include "SlideSorter.hxx"

#include "SlideSorterChildWindow.hrc"
#include "SlideSorterViewShell.hxx"

#include "DrawViewShell.hxx"
#include "PaneManager.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"

#include "glob.hrc"

#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#include "sdresid.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


namespace sd { namespace slidesorter {


SlideSorter::SlideSorter (
    SfxBindings *pBindings,
    SfxChildWindow *pChildWindow,
    ::Window* pParentWindow)
    : SfxDockingWindow (
        pBindings,
        pChildWindow,
        pParentWindow,
        SdResId(FLT_WIN_SLIDE_BROWSER))
{
    FreeResource();
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        pBindings->GetDispatcher()->GetFrame()));
    rBase.GetPaneManager().SetWindow (PaneManager::PT_LEFT, this);
}




SlideSorter::~SlideSorter (void)
{
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        GetBindings().GetDispatcher()->GetFrame()));
    // Tell the view shell base that the window of this slide sorter is not
    // available anymore.
    rBase.GetPaneManager().SetWindow (PaneManager::PT_LEFT, NULL);
}




void SlideSorter::Resize (void)
{
    SfxDockingWindow::Resize();

    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        GetBindings().GetDispatcher()->GetFrame()));
    ViewShell* pViewShell = rBase.GetPaneManager().GetViewShell (
        PaneManager::PT_LEFT);
    if (pViewShell != NULL)
        pViewShell->OuterResizePixel(GetPosPixel(),GetSizePixel());
}


} } // end of namespace ::sd::slidesorter
