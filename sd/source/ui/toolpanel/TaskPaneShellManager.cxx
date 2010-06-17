/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "TaskPaneShellManager.hxx"

#include "ViewShellManager.hxx"
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>

#include <algorithm>

namespace sd { namespace toolpanel {

TaskPaneShellManager::TaskPaneShellManager (
    const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager,
    const ViewShell& rViewShell)
    : mpViewShellManager(rpViewShellManager),
      mrViewShell(rViewShell),
      maSubShells()
{
}




TaskPaneShellManager::~TaskPaneShellManager (void)
{
    while ( ! maSubShells.empty())
        RemoveSubShell(maSubShells.begin()->second.mpShell);
}




SfxShell* TaskPaneShellManager::CreateShell( ShellId nId, ::Window* , FrameView* )
{
    SubShells::const_iterator iShell (maSubShells.find(nId));
    if (iShell != maSubShells.end())
        return iShell->second.mpShell;
    else
        return NULL;
}




void TaskPaneShellManager::ReleaseShell (SfxShell* )
{
    // Nothing to do.
}

void TaskPaneShellManager::AddSubShell (
    ShellId nId,
    SfxShell* pShell,
    ::Window* pWindow)
{
    if (pShell != NULL)
    {
        maSubShells[nId] = ShellDescriptor(pShell,pWindow);
        if (pWindow != NULL)
        {
            pWindow->AddEventListener(LINK(this,TaskPaneShellManager,WindowCallback));
            if (pWindow->IsReallyVisible())
                mpViewShellManager->ActivateSubShell(mrViewShell, nId);
        }
        else
            mpViewShellManager->ActivateSubShell(mrViewShell, nId);
    }
}




void TaskPaneShellManager::RemoveSubShell (const ShellId i_nShellId)
{
    SubShells::iterator pos = maSubShells.find( i_nShellId );
    ENSURE_OR_RETURN_VOID( pos != maSubShells.end(), "no shell for this ID" );
    if ( pos->second.mpWindow != NULL )
    {
        pos->second.mpWindow->RemoveEventListener( LINK( this, TaskPaneShellManager, WindowCallback ) );
    }
    mpViewShellManager->DeactivateSubShell( mrViewShell, pos->first );
    maSubShells.erase( pos );
}




void TaskPaneShellManager::RemoveSubShell (const SfxShell* pShell)
{
    if (pShell != NULL)
    {
        SubShells::iterator iShell;
        for (iShell=maSubShells.begin(); iShell!=maSubShells.end(); ++iShell)
            if (iShell->second.mpShell == pShell)
            {
                if (iShell->second.mpWindow != NULL)
                    iShell->second.mpWindow->RemoveEventListener(
                        LINK(this,TaskPaneShellManager,WindowCallback));
                mpViewShellManager->DeactivateSubShell(mrViewShell,iShell->first);
                maSubShells.erase(iShell);
                break;
            }
    }
}




void TaskPaneShellManager::MoveToTop (SfxShell* pShell)
{
    SubShells::const_iterator iShell;
    for (iShell=maSubShells.begin(); iShell!=maSubShells.end(); ++iShell)
        if (iShell->second.mpShell == pShell)
        {
            ViewShellManager::UpdateLock aLocker (mpViewShellManager);
            mpViewShellManager->MoveSubShellToTop(mrViewShell,iShell->first);
            mpViewShellManager->MoveToTop(mrViewShell);
            break;
        }
}




IMPL_LINK(TaskPaneShellManager, WindowCallback, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        SubShells::const_iterator iShell;
        ::Window* pWindow = pEvent->GetWindow();
        for (iShell=maSubShells.begin(); iShell!=maSubShells.end(); ++iShell)
            if (iShell->second.mpWindow == pWindow)
                break;
        if (iShell != maSubShells.end())
            switch (pEvent->GetId())
            {
                case VCLEVENT_WINDOW_SHOW:
                    mpViewShellManager->ActivateSubShell(mrViewShell,iShell->first);
                    break;

                case VCLEVENT_WINDOW_HIDE:
                    // Do not activate the sub shell.  This leads to
                    // problems with shapes currently being in text edit
                    // mode: Deactivating the shell leads to leaving the
                    // text editing mode.
                    // mpViewShellManager->DeactivateSubShell(mrViewShell,iShell->first);
                    break;
            }
    }

    return 0;
}


} } // end of namespace ::sd::toolpanel
