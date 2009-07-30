/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wformsh.cxx,v $
 * $Revision: 1.8 $
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



#include <sfx2/msg.hxx>

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/srchitem.hxx>
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

#define SwWebDrawFormShell
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebDrawFormShell, SwDrawFormShell, SW_RES(0))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAWFORM_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TEXT_TOOLBOX));
}

TYPEINIT1(SwWebDrawFormShell, SwDrawFormShell)

SwWebDrawFormShell::SwWebDrawFormShell(SwView& rVw) :
    SwDrawFormShell(rVw)
{
}

__EXPORT SwWebDrawFormShell::~SwWebDrawFormShell()
{
}


