/*************************************************************************
 *
 *  $RCSfile: TaskPaneShellManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:36:56 $
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

#include "TaskPaneShellManager.hxx"

#include "ViewShellManager.hxx"
#include <osl/diagnose.h>
#include <algorithm>

namespace sd { namespace toolpanel {

TaskPaneShellManager::TaskPaneShellManager (
    ViewShellManager& rViewShellManager,
    const ViewShell& rViewShell)
    : maShellStack(),
      mrViewShellManager(rViewShellManager),
      mrViewShell(rViewShell)
{
}




void TaskPaneShellManager::AddSubShell (SfxShell* pShell)
{
    ViewShellManager::UpdateLocker aLocker (mrViewShellManager);
    maShellStack.push_front (pShell);
}




void TaskPaneShellManager::RemoveSubShell (SfxShell* pShell)
{
    ShellStack::iterator aFoundShell (::std::find (
        maShellStack.begin(),
        maShellStack.end(),
        pShell));
    if (aFoundShell != maShellStack.end())
    {
        ViewShellManager::UpdateLocker aLocker (mrViewShellManager);
        maShellStack.erase (aFoundShell);
    }
}




void TaskPaneShellManager::MoveToTop (SfxShell* pShell)
{
    if (maShellStack.empty() || maShellStack.front()!=pShell)
    {
        // Move the given shell to the top of the local stack.
        ViewShellManager::UpdateLocker aLocker (mrViewShellManager);

        RemoveSubShell (pShell);
        AddSubShell (pShell);
    }
    // Move the view shell to the top of the view shell stack.
    mrViewShellManager.MoveToTop (&mrViewShell);
}




void TaskPaneShellManager::GetLowerShellList (
    ::std::vector<SfxShell*>& rShellList) const
{
    // No sub shells below the shell yet.
}




void TaskPaneShellManager::GetUpperShellList (
    ::std::vector<SfxShell*>& rShellList) const
{
    OSL_TRACE ("TaskPaneShellManager::GetUpperShellList");
    ShellStack::const_iterator iShell;
    for (iShell=maShellStack.begin(); iShell!=maShellStack.end(); ++iShell)
    {
        rShellList.push_back (*iShell);
        OSL_TRACE ("    %p", *iShell);
    }
}



} } // end of namespace ::sd::toolpanel
