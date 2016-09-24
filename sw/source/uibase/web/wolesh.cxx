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
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <svl/srchitem.hxx>

#include "wrtsh.hxx"
#include "view.hxx"
#include "helpid.h"
#include "globals.hrc"
#include "web.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "wolesh.hxx"
#include "cmdid.h"

#define SwWebOleShell
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebOleShell, SwOleShell)

void SwWebOleShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("oleobject");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, RID_WEBOLE_TOOLBOX);
}

SwWebOleShell::SwWebOleShell(SwView &_rView) :
    SwOleShell(_rView)

{
    SetName("Object");
    SetHelpId(SW_OLESHELL);
}

SwWebOleShell::~SwWebOleShell()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
