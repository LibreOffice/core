/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "TaskPaneShellManager.hxx"

#include "SidebarShellManager.hxx"
#include "ViewShellManager.hxx"
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>

#include <algorithm>

namespace sd { namespace sidebar {

SidebarShellManager::SidebarShellManager (
    const ::boost::shared_ptr<ViewShellManager>& rpViewShellManager,
    const ViewShell& rViewShell)
    : mpViewShellManager(rpViewShellManager),
      mrViewShell(rViewShell),
      maSubShells()
{
}




SidebarShellManager::~SidebarShellManager (void)
{
    while ( ! maSubShells.empty())
        RemoveSubShell(maSubShells.begin()->second.mpShell);
}




SfxShell* SidebarShellManager::CreateShell( ShellId nId, ::Window* , FrameView* )
{
    SubShells::const_iterator iShell (maSubShells.find(nId));
    if (iShell != maSubShells.end())
        return iShell->second.mpShell;
    else
        return NULL;
}




void SidebarShellManager::ReleaseShell (SfxShell* )
{
    // Nothing to do.
}

void SidebarShellManager::AddSubShell (
    ShellId nId,
    SfxShell* pShell,
    ::Window* pWindow)
{
    if (pShell != NULL)
    {
        maSubShells[nId] = ShellDescriptor(pShell,pWindow);
        if (pWindow != NULL)
        {
            pWindow->AddEventListener(LINK(this,SidebarShellManager,WindowCallback));
            if (pWindow->IsReallyVisible())
                mpViewShellManager->ActivateSubShell(mrViewShell, nId);
        }
        else
            mpViewShellManager->ActivateSubShell(mrViewShell, nId);
    }
}




void SidebarShellManager::RemoveSubShell (const ShellId i_nShellId)
{
    SubShells::iterator pos = maSubShells.find( i_nShellId );
    ENSURE_OR_RETURN_VOID( pos != maSubShells.end(), "no shell for this ID" );
    if ( pos->second.mpWindow != NULL )
    {
        pos->second.mpWindow->RemoveEventListener( LINK( this, SidebarShellManager, WindowCallback ) );
    }
    mpViewShellManager->DeactivateSubShell( mrViewShell, pos->first );
    maSubShells.erase( pos );
}




void SidebarShellManager::RemoveSubShell (const SfxShell* pShell)
{
    if (pShell != NULL)
    {
        SubShells::iterator iShell;
        for (iShell=maSubShells.begin(); iShell!=maSubShells.end(); ++iShell)
            if (iShell->second.mpShell == pShell)
            {
                if (iShell->second.mpWindow != NULL)
                    iShell->second.mpWindow->RemoveEventListener(
                        LINK(this,SidebarShellManager,WindowCallback));
                mpViewShellManager->DeactivateSubShell(mrViewShell,iShell->first);
                maSubShells.erase(iShell);
                break;
            }
    }
}




void SidebarShellManager::MoveToTop (SfxShell* pShell)
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




IMPL_LINK(SidebarShellManager, WindowCallback, VclWindowEvent*, pEvent)
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


} } // end of namespace ::sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
