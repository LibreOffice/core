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

#include <cmdid.h>
#include <sfx2/request.hxx>
#include <svx/svdview.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <mediash.hxx>

#include <sfx2/objface.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/MediaShellHelpers.hxx>

#define ShellClass_SwMediaShell
#include <swslots.hxx>

using namespace svx;

SFX_IMPL_INTERFACE(SwMediaShell, SwBaseShell)

void SwMediaShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(u"media"_ustr);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible,
                                            ToolbarId::Media_Toolbox);
}

void SwMediaShell::ExecMedia(SfxRequest const& rReq)
{
    SwWrtShell* pSh = &GetShell();
    SdrView* pSdrView = pSh->GetDrawView();

    if (!pSdrView)
        return;

    const bool bChanged = pSdrView->GetModel().IsChanged();
    pSdrView->GetModel().SetChanged(false);

    switch (rReq.GetSlot())
    {
        case SID_DELETE:
            if (pSh->IsObjSelected())
            {
                pSh->SetModified();
                pSh->DelSelectedObj();

                if (pSh->IsSelFrameMode())
                    pSh->LeaveSelFrameMode();

                GetView().AttrChangedNotify(nullptr);
            }
            break;

        case SID_AVMEDIA_TOOLBOX:
            if (pSh->IsObjSelected())
                MediaShellHelpers::Execute(pSdrView, rReq);
            break;

        default:
            break;
    }

    if (pSdrView->GetModel().IsChanged())
        GetShell().SetModified();
    else if (bChanged)
        pSdrView->GetModel().SetChanged();
}

void SwMediaShell::GetMediaState(SfxItemSet& rSet)
{
    MediaShellHelpers::GetState(GetShell().GetDrawView(), rSet);
}

SwMediaShell::SwMediaShell(SwView& _rView)
    : SwBaseShell(_rView)
{
    SetName(u"Media Playback"_ustr);
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Media));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
