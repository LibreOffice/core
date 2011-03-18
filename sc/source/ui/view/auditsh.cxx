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
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>

#include "auditsh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "document.hxx"

//------------------------------------------------------------------------

#define ScAuditingShell
#include "scslots.hxx"

//------------------------------------------------------------------------

TYPEINIT1( ScAuditingShell, SfxShell );

SFX_IMPL_INTERFACE(ScAuditingShell, SfxShell, ScResId(SCSTR_AUDITSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_AUDIT) );
}


//------------------------------------------------------------------------

ScAuditingShell::ScAuditingShell(ScViewData* pData) :
    SfxShell(pData->GetViewShell()),
    pViewData( pData ),
    nFunction( SID_FILL_ADD_PRED )
{
    SetPool( &pViewData->GetViewShell()->GetPool() );
    ::svl::IUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetHelpId( HID_SCSHELL_AUDIT );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Auditing")));
}

//------------------------------------------------------------------------

ScAuditingShell::~ScAuditingShell()
{
}

//------------------------------------------------------------------------

void ScAuditingShell::Execute( SfxRequest& rReq )
{
    SfxBindings& rBindings = pViewData->GetBindings();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_FILL_ADD_PRED:
        case SID_FILL_DEL_PRED:
        case SID_FILL_ADD_SUCC:
        case SID_FILL_DEL_SUCC:
            nFunction = nSlot;
            rBindings.Invalidate( SID_FILL_ADD_PRED );
            rBindings.Invalidate( SID_FILL_DEL_PRED );
            rBindings.Invalidate( SID_FILL_ADD_SUCC );
            rBindings.Invalidate( SID_FILL_DEL_SUCC );
            break;
        case SID_CANCEL:        // Escape
        case SID_FILL_NONE:
            pViewData->GetViewShell()->SetAuditShell( false );
            break;

        case SID_FILL_SELECT:
            {
                const SfxItemSet* pReqArgs = rReq.GetArgs();
                if ( pReqArgs )
                {
                    const SfxPoolItem* pXItem;
                    const SfxPoolItem* pYItem;
                    if ( pReqArgs->GetItemState( SID_RANGE_COL, sal_True, &pXItem ) == SFX_ITEM_SET
                      && pReqArgs->GetItemState( SID_RANGE_ROW, sal_True, &pYItem ) == SFX_ITEM_SET )
                    {
                        DBG_ASSERT( pXItem->ISA(SfxInt16Item) && pYItem->ISA(SfxInt32Item),
                                        "falsche Items" );
                        SCsCOL nCol = static_cast<SCsCOL>(((const SfxInt16Item*) pXItem)->GetValue());
                        SCsROW nRow = static_cast<SCsROW>(((const SfxInt32Item*) pYItem)->GetValue());
                        ScViewFunc* pView = pViewData->GetView();
                        pView->MoveCursorAbs( nCol, nRow, SC_FOLLOW_LINE, false, false );
                        switch ( nFunction )
                        {
                            case SID_FILL_ADD_PRED:
                                pView->DetectiveAddPred();
                                break;
                            case SID_FILL_DEL_PRED:
                                pView->DetectiveDelPred();
                                break;
                            case SID_FILL_ADD_SUCC:
                                pView->DetectiveAddSucc();
                                break;
                            case SID_FILL_DEL_SUCC:
                                pView->DetectiveDelSucc();
                                break;
                        }
                    }
                }
            }
            break;
    }
}

//------------------------------------------------------------------------

void ScAuditingShell::GetState( SfxItemSet& rSet )
{
    rSet.Put( SfxBoolItem( nFunction, sal_True ) );         // aktive Funktion markieren
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
