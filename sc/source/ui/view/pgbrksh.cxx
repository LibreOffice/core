/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pgbrksh.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:20:32 $
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
    ScViewData* pViewData = pViewSh->GetViewData();
    SfxUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetHelpId( HID_SCSHELL_PAGEBREAK );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("PageBreak")));
}

//------------------------------------------------------------------------
ScPageBreakShell::~ScPageBreakShell()
{
}


