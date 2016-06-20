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

std::uintptr_t SfxLokHelper::createView()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    SfxRequest aRequest(pViewFrame, SID_NEWWINDOW);
    pViewFrame->ExecView_Impl(aRequest);

    return reinterpret_cast<std::uintptr_t>(SfxViewShell::Current());
}

void SfxLokHelper::destroyView(std::uintptr_t nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        SfxViewShell* pViewShell = rViewArr[i];
        if (reinterpret_cast<std::uintptr_t>(pViewShell) == nId)
        {
            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
            SfxRequest aRequest(pViewFrame, SID_CLOSEWIN);
            pViewFrame->Exec_Impl(aRequest);
            break;
        }
    }
}

void SfxLokHelper::setView(std::uintptr_t nId)
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();

    for (std::size_t i = 0; i < rViewArr.size(); ++i)
    {
        SfxViewShell* pViewShell = rViewArr[i];
        if (reinterpret_cast<std::uintptr_t>(pViewShell) == nId)
        {
            if (pViewShell == SfxViewShell::Current())
                return;

            SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
            pViewFrame->MakeActive_Impl(false);
            return;
        }
    }

}

std::uintptr_t SfxLokHelper::getView(SfxViewShell* pViewShell)
{
    if (!pViewShell)
        pViewShell = SfxViewShell::Current();
    return reinterpret_cast<std::uintptr_t>(pViewShell);
}

std::size_t SfxLokHelper::getViews()
{
    std::size_t nRet = 0;

    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return nRet;

    SfxObjectShell* pObjectShell = pViewFrame->GetObjectShell();
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    for (SfxViewShell* i : rViewArr)
    {
        if (i->GetObjectShell() == pObjectShell)
            ++nRet;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
