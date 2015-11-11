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

#include <sfx2/msg.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <sfx2/objface.hxx>
#include <tools/globname.hxx>
#include <sfx2/app.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "wformsh.hxx"
#include "globals.hrc"
#include "web.hrc"
#include "popup.hrc"
#include "shells.hrc"

#include <sfx2/request.hxx>
    // needed for -fsanitize=function visibility of typeinfo for functions of
    // type void(SfxShell*,SfxRequest&) defined in swslots.hxx
#define SwWebDrawFormShell
#include "swslots.hxx"

SFX_IMPL_SUPERCLASS_INTERFACE(SwWebDrawFormShell, SwDrawFormShell)

void SwWebDrawFormShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(SW_RES(MN_DRAWFORM_POPUPMENU));

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, RID_TEXT_TOOLBOX);
}


SwWebDrawFormShell::SwWebDrawFormShell(SwView& rVw) :
    SwDrawFormShell(rVw)
{
}

SwWebDrawFormShell::~SwWebDrawFormShell()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
