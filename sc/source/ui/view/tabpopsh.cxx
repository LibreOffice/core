/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabpopsh.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 15:09:08 $
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
#include "precompiled_sc.hxx"


#include "tabpopsh.hxx"
#include "sc.hrc"

#undef ShellClass
#define ShellClass ScTabPopShell
SFX_SLOTMAP(ScTabPopShell)
{
    SFX_SLOT( 0,0, DummyExec, DummyState, 0, SfxVoidItem )
};


TYPEINIT1(ScTabPopShell,SfxShell);

//SFX_IMPL_IDL_INTERFACE(ScTabPopShell, SfxShell, 0)
SFX_IMPL_INTERFACE(ScTabPopShell, SfxShell, ResId( 0, NULL))
{
    SFX_POPUPMENU_REGISTRATION( RID_POPUP_TAB );
}




ScTabPopShell::ScTabPopShell(SfxItemPool& rItemPool)
{
    SetPool( &rItemPool );
}


ScTabPopShell::~ScTabPopShell()
{
}


void ScTabPopShell::DummyExec( SfxRequest& rReq )
{
}


void ScTabPopShell::DummyState( SfxItemSet& rSet )
{
}
