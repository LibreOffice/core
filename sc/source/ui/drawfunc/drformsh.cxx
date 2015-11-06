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

#include <editeng/eeitem.hxx>
#include <svx/fontwork.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "drformsh.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "drawview.hxx"
#include "scresid.hxx"
#include <svx/svdobj.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

#define ScDrawFormShell
#include "scslots.hxx"

SFX_IMPL_INTERFACE(ScDrawFormShell, ScDrawShell)

void ScDrawFormShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                            RID_OBJECTBAR_FORMAT);

    GetStaticInterface()->RegisterPopupMenu(ScResId(RID_POPUP_DRAWFORM));
}

TYPEINIT1( ScDrawFormShell, ScDrawShell );

ScDrawFormShell::ScDrawFormShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_DRAWFORMSH);
    SetName("DrawForm");
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Form));
}

ScDrawFormShell::~ScDrawFormShell()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
