/*************************************************************************
 *
 *  $RCSfile: auditsh.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:33:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/srchitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>

#include "auditsh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "sc.hrc"

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
    SetUndoManager( pViewData->GetSfxDocShell()->GetUndoManager() );
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
    USHORT nSlot = rReq.GetSlot();
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
            pViewData->GetViewShell()->SetAuditShell( FALSE );
            break;

        case SID_FILL_SELECT:
            {
                const SfxItemSet* pReqArgs = rReq.GetArgs();
                if ( pReqArgs )
                {
                    const SfxPoolItem* pXItem;
                    const SfxPoolItem* pYItem;
                    if ( pReqArgs->GetItemState( SID_RANGE_COL, TRUE, &pXItem ) == SFX_ITEM_SET
                      && pReqArgs->GetItemState( SID_RANGE_ROW, TRUE, &pYItem ) == SFX_ITEM_SET )
                    {
                        DBG_ASSERT( pXItem->ISA(SfxInt16Item) && pYItem->ISA(SfxInt16Item),
                                        "falsche Items" );
                        short nCol = ((const SfxInt16Item*) pXItem)->GetValue();
                        short nRow = ((const SfxInt16Item*) pYItem)->GetValue();
                        ScViewFunc* pView = pViewData->GetView();
                        pView->MoveCursorAbs( nCol, nRow, SC_FOLLOW_LINE, FALSE, FALSE );
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
    rSet.Put( SfxBoolItem( nFunction, TRUE ) );         // aktive Funktion markieren
}


