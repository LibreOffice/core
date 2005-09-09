/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TaskPaneShellManager.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:33:35 $
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
    ViewShellManager::UpdateLock aLocker (mrViewShellManager);

    maShellStack.push_back(pShell);
}




void TaskPaneShellManager::RemoveSubShell (SfxShell* pShell)
{
    ShellStack::iterator aFoundShell (::std::find (
        maShellStack.begin(),
        maShellStack.end(),
        pShell));
    if (aFoundShell != maShellStack.end())
    {
        ViewShellManager::UpdateLock aLocker (mrViewShellManager);

        maShellStack.erase(aFoundShell);
    }
}




void TaskPaneShellManager::MoveToTop (SfxShell* pShell)
{
    ViewShellManager::UpdateLock aLocker (mrViewShellManager);

    if (maShellStack.empty() || maShellStack.back()!=pShell)
    {
        // Tell the ViewShellManager to rebuild the stack at least from the
        // current position of the shell.
        mrViewShellManager.InvalidateShellStack(pShell);

        // Move the given shell to the top of the local stack.
        RemoveSubShell (pShell);
        AddSubShell (pShell);
    }

    // Move the view shell to the top of the view shell stack.  This has to
    // be done even when the sub shell has not been moved.
    mrViewShellManager.MoveToTop (&mrViewShell);
}




void TaskPaneShellManager::GetLowerShellList (::std::vector<SfxShell*>& rShellList) const
{
    // No sub shells below the shell yet.
}




void TaskPaneShellManager::GetUpperShellList (::std::vector<SfxShell*>& rShellList) const
{
    ShellStack::const_iterator iShell;
    for (iShell=maShellStack.begin(); iShell!=maShellStack.end(); ++iShell)
        rShellList.push_back (*iShell);
}



} } // end of namespace ::sd::toolpanel
