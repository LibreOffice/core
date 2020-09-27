/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <vcl/EnumContext.hxx>

#include <auditsh.hxx>
#include <tabvwsh.hxx>
#include <sc.hrc>
#include <document.hxx>

#define ShellClass_ScAuditingShell
#include <scslots.hxx>


SFX_IMPL_INTERFACE(ScAuditingShell, SfxShell)

void ScAuditingShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("audit");
}

ScAuditingShell::ScAuditingShell(ScViewData& rData) :
    SfxShell(rData.GetViewShell()),
    rViewData( rData ),
    nFunction( SID_FILL_ADD_PRED )
{
    SetPool( &rViewData.GetViewShell()->GetPool() );
    SfxUndoManager* pMgr = rViewData.GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !rViewData.GetDocument().IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetName("Auditing");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Auditing));
}

ScAuditingShell::~ScAuditingShell()
{
}

void ScAuditingShell::Execute( const SfxRequest& rReq )
{
    SfxBindings& rBindings = rViewData.GetBindings();
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
            rViewData.GetViewShell()->SetAuditShell( false );
            break;

        case SID_FILL_SELECT:
            {
                const SfxItemSet* pReqArgs = rReq.GetArgs();
                if ( pReqArgs )
                {
                    const SfxPoolItem* pXItem;
                    const SfxPoolItem* pYItem;
                    if ( pReqArgs->GetItemState( SID_RANGE_COL, true, &pXItem ) == SfxItemState::SET
                      && pReqArgs->GetItemState( SID_RANGE_ROW, true, &pYItem ) == SfxItemState::SET )
                    {
                        OSL_ENSURE( dynamic_cast<const SfxInt16Item*>( pXItem) != nullptr && dynamic_cast<const SfxInt32Item*>( pYItem) !=  nullptr,
                                        "wrong items" );
                        SCCOL nCol = static_cast<SCCOL>(static_cast<const SfxInt16Item*>(pXItem)->GetValue());
                        SCROW nRow = static_cast<SCROW>(static_cast<const SfxInt32Item*>(pYItem)->GetValue());
                        ScViewFunc* pView = rViewData.GetView();
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

void ScAuditingShell::GetState( SfxItemSet& rSet )
{
    rSet.Put( SfxBoolItem( nFunction, true ) );         // mark active functions
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
