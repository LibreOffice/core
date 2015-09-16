/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/lokhelper.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>

#include <shellimpl.hxx>

int SfxLokHelper::createView()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    SfxRequest aRequest(pViewFrame, SID_NEWWINDOW);
    pViewFrame->ExecView_Impl(aRequest);

    // The SfxViewShell ctor always puts the view shell to the end of the vector.
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    return rViewArr.size() - 1;
}

void SfxLokHelper::destroyView(size_t nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    if (nId > rViewArr.size() - 1)
        return;

    SfxViewShell* pViewShell = rViewArr[nId];
    SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
    SfxRequest aRequest(pViewFrame, SID_CLOSEWIN);
    pViewFrame->Exec_Impl(aRequest);
}

void SfxLokHelper::setView(size_t nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    if (nId > rViewArr.size() - 1)
        return;

    SfxViewShell* pViewShell = rViewArr[nId];
    if (SfxViewFrame* pViewFrame = pViewShell->GetViewFrame())
        pViewFrame->MakeActive_Impl(false);
}

size_t SfxLokHelper::getView()
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    for (size_t i = 0; i < rViewArr.size(); ++i)
    {
        if (rViewArr[i]->GetViewFrame() == pViewFrame)
            return i;
    }
    assert(false);
    return 0;
}

int SfxLokHelper::getViews()
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    return rViewArr.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
