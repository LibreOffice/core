/*************************************************************************
 *
 *  $RCSfile: SlideSorter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:25:28 $
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
