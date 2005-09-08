/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pgbrksh.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:02:46 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>

#include "pgbrksh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "sc.hrc"

//------------------------------------------------------------------------

#define ScPageBreakShell
#include "scslots.hxx"

//------------------------------------------------------------------------

TYPEINIT1( ScPageBreakShell, SfxShell );

SFX_IMPL_INTERFACE(ScPageBreakShell, SfxShell, ScResId(SCSTR_PAGEBREAKSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_PAGEBREAK) );
}


//------------------------------------------------------------------------
ScPageBreakShell::ScPageBreakShell( ScTabViewShell* pViewSh ) :
    SfxShell(pViewSh)
{
    SetPool( &pViewSh->GetPool() );
    SetUndoManager( pViewSh->GetViewData()->GetSfxDocShell()->GetUndoManager() );
    SetHelpId( HID_SCSHELL_PAGEBREAK );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("PageBreak")));
}

//------------------------------------------------------------------------
ScPageBreakShell::~ScPageBreakShell()
{
}


