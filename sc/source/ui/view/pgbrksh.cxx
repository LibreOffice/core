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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>

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
    ::svl::IUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
