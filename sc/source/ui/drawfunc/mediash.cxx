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

#include <sfx2/objface.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/MediaShellHelpers.hxx>

#include <mediash.hxx>
#include <strings.hrc>
#include <viewdata.hxx>
#include <drawview.hxx>
#include <scresid.hxx>

#define ShellClass_ScMediaShell
#include <scslots.hxx>

using namespace svx;

SFX_IMPL_INTERFACE(ScMediaShell, ScDrawShell)

void ScMediaShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible,
                                            ToolbarId::Media_Objectbar);

    GetStaticInterface()->RegisterPopupMenu(u"media"_ustr);
}

ScMediaShell::ScMediaShell(ScViewData& rData)
    : ScDrawShell(rData)
{
    SetName(ScResId(SCSTR_MEDIASHELL));
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Media));
}

ScMediaShell::~ScMediaShell() {}

void ScMediaShell::GetMediaState(SfxItemSet& rSet)
{
    MediaShellHelpers::GetState(GetViewData().GetScDrawView(), rSet);
}

void ScMediaShell::ExecuteMedia(const SfxRequest& rReq)
{
    MediaShellHelpers::Execute(GetViewData().GetScDrawView(), rReq);
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
