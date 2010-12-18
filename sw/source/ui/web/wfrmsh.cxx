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
#include <sfx2/objface.hxx>
#include <svl/srchitem.hxx> //***

#include "swtypes.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wfrmsh.hxx"
#include "globals.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "web.hrc"


// STATIC DATA -----------------------------------------------------------
#define SwWebFrameShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SFX_IMPL_INTERFACE(SwWebFrameShell, SwFrameShell, SW_RES(STR_SHELLNAME_WEBFRAME))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_FRM_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_WEBFRAME_TOOLBOX));
}



TYPEINIT1(SwWebFrameShell, SwFrameShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwWebFrameShell::SwWebFrameShell(SwView &_rView) :
    SwFrameShell(_rView)
{
    SetHelpId(SW_WEBFRAMESHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwWebFrameShell::~SwWebFrameShell()
{
}







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
