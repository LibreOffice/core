/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "hintids.hxx"
#include <sfx2/app.hxx>
#include <tools/globname.hxx>
#include <sfx2/objface.hxx>
#include <svl/srchitem.hxx>


#include "cmdid.h"
#include "globals.hrc"
#include "uitool.hxx"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "table.hrc"
#include "wrtsh.hxx"
#include "wtabsh.hxx"

#define SwWebTableShell
#include <sfx2/msg.hxx>
#include "svx/svxids.hrc"
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebTableShell, SwTableShell, SW_RES(STR_SHELLNAME_TABLE))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_TAB_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TABLE_TOOLBOX));
}

TYPEINIT1(SwWebTableShell,SwTableShell)

SwWebTableShell::SwWebTableShell(SwView &_rView) :
    SwTableShell(_rView)
{
    GetShell().UpdateTable();
    SetName(String::CreateFromAscii("Table"));
    SetHelpId(SW_TABSHELL);
}

__EXPORT SwWebTableShell::~SwWebTableShell()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
