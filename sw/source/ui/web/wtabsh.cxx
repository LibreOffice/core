/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wtabsh.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:51:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "hintids.hxx"

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _GLOBNAME_HXX //autogen
#include <tools/globname.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif


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
#include "itemdef.hxx"
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



