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

int SfxLokHelper::createView(SfxViewShell* pViewShell)
{
    SfxViewFrame* pViewFrame = pViewShell->GetViewFrame();
    SfxRequest aRequest(pViewFrame, SID_NEWWINDOW);
    pViewFrame->ExecView_Impl(aRequest);

    // The SfxViewShell ctor always puts the view shell to the end of the vector.
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    return rViewArr.size() - 1;
}

int SfxLokHelper::getViews()
{
    SfxViewShellArr_Impl& rViewArr = SfxGetpApp()->GetViewShells_Impl();
    return rViewArr.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
