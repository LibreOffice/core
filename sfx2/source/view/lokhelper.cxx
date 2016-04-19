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

void SfxLokHelper::destroyView(std::size_t nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    if (nId > rViewArr.size() - 1)
        return;

    SfxViewShell* pViewShell = rViewArr[nId];
    SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
    SfxRequest aRequest(pViewFrame, SID_CLOSEWIN);
    pViewFrame->Exec_Impl(aRequest);
}

void SfxLokHelper::setView(std::size_t nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    if (nId > rViewArr.size() - 1)
        return;

    SfxViewShell* pViewShell = rViewArr[nId];
    if (pViewShell->GetViewFrame() == SfxViewFrame::Current())
        return;

    if (SfxViewFrame* pViewFrame = pViewShell->GetViewFrame())
        pViewFrame->MakeActive_Impl(false);
}

std::size_t SfxLokHelper::getView()
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        if (rViewArr[i]->GetViewFrame() == pViewFrame)
            return i;
    }
    assert(false);
    return 0;
}

std::size_t SfxLokHelper::getViews()
{
    std::size_t nRet = 0;

    SfxObjectShell* pObjectShell = SfxViewFrame::Current()->GetObjectShell();
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    for (SfxViewShell* i : rViewArr)
    {
        if (i->GetObjectShell() == pObjectShell)
            ++nRet;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
