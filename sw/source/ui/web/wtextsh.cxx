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

#include "hintids.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/app.hxx>
#include <svl/srchitem.hxx>
#include <basic/sbxvar.hxx>
#include <svx/svxids.hrc>

#include "swtypes.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wtextsh.hxx"
#include "basesh.hxx"
#include "globals.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "web.hrc"

#include <unomid.h>

// STATIC DATA
#define SwWebTextShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebTextShell, SwBaseShell, SW_RES(STR_SHELLNAME_WEBTEXT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_TEXT_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TEXT_TOOLBOX));
    SFX_CHILDWINDOW_REGISTRATION(FN_EDIT_FORMULA);
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_FIELD);
}

TYPEINIT1(SwWebTextShell, SwTextShell)

SwWebTextShell::SwWebTextShell(SwView &_rView) :
    SwTextShell(_rView)
{
    SetHelpId(SW_WEBTEXTSHELL);
}

SwWebTextShell::~SwWebTextShell()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
