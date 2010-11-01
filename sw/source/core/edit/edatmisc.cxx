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


#include <editsh.hxx>
#include <doc.hxx>      // fuer aNodes
#include <pam.hxx>      // fuer SwPaM
#include <edimp.hxx>    // fuer MACROS
#include <swundo.hxx>   // fuer die UndoIds
#include <ndtxt.hxx>    // fuer Get-/ChgFmt Set-/GetAttrXXX



/*************************************
 * harte Formatierung (Attribute)
 *************************************/


void SwEditShell::ResetAttr( const SvUShortsSort* pAttrs )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    BOOL bUndoGroup = GetCrsr()->GetNext() != GetCrsr();
    if( bUndoGroup )
        GetDoc()->StartUndo(UNDO_RESETATTR, NULL);

        FOREACHPAM_START(this)
            // if ( PCURCRSR->HasMark() )
                GetDoc()->ResetAttrs(*PCURCRSR, sal_True, pAttrs);
        FOREACHPAM_END()

    if( bUndoGroup )
        GetDoc()->EndUndo(UNDO_RESETATTR, NULL);
    CallChgLnk();
    EndAllAction();
}



void SwEditShell::GCAttr()
{
//JP 04.02.97: wozu eine Action-Klammerung - ein Formatierung sollte nicht
//              ausgeloest werden, so dass es hier ueberfluessig ist.
//              Sonst Probleme im MouseBut.DownHdl - Bug 35562
//  StartAllAction();
    FOREACHPAM_START(this)
        SwTxtNode *pTxtNode;
        if ( !PCURCRSR->HasMark() )
        {
            if( 0 != (pTxtNode = GetDoc()->GetNodes()[
                                PCURCRSR->GetPoint()->nNode]->GetTxtNode()))
                pTxtNode->GCAttr();
        }
        else
        {
            const SwNodeIndex& rEnd = PCURCRSR->End()->nNode;
            SwNodeIndex aIdx( PCURCRSR->Start()->nNode );
            SwNode* pNd = &aIdx.GetNode();
            do {
                if( pNd->IsTxtNode() )
                    ((SwTxtNode*)pNd)->GCAttr();
            }
            while( 0 != ( pNd = GetDoc()->GetNodes().GoNext( &aIdx )) &&
                    aIdx <= rEnd );
        }
    FOREACHPAM_END()
//  EndAllAction();
}

// Setze das Attribut als neues default Attribut im Dokument.


void SwEditShell::SetDefault( const SfxPoolItem& rFmtHint )
{
    // 7502: Action-Klammerung
    StartAllAction();
    GetDoc()->SetDefault( rFmtHint );
    EndAllAction();
}

/*

void SwEditShell::SetDefault( const SfxItemSet& rSet )
{
    // 7502: Action-Klammerung
    StartAllAction();
    GetDoc()->SetDefault( rSet );
    EndAllAction();
}
*/

// Erfrage das Default Attribut in diesem Dokument.

const SfxPoolItem& SwEditShell::GetDefault( USHORT nFmtHint ) const
{
    return GetDoc()->GetDefault( nFmtHint );

}


void SwEditShell::SetAttr( const SfxPoolItem& rHint, USHORT nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        BOOL bIsTblMode = IsTableMode();
        GetDoc()->StartUndo(UNDO_INSATTR, NULL);

        FOREACHPAM_START(this)
            if( PCURCRSR->HasMark() && ( bIsTblMode ||
                *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() ))
            {
                GetDoc()->InsertPoolItem(*PCURCRSR, rHint, nFlags );
            }
        FOREACHPAM_END()

        GetDoc()->EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->InsertPoolItem( *pCrsr, rHint, nFlags );
    }
    EndAllAction();
}


void SwEditShell::SetAttr( const SfxItemSet& rSet, USHORT nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        BOOL bIsTblMode = IsTableMode();
        GetDoc()->StartUndo(UNDO_INSATTR, NULL);

        FOREACHPAM_START(this)
            if( PCURCRSR->HasMark() && ( bIsTblMode ||
                *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() ))
            {
                GetDoc()->InsertItemSet(*PCURCRSR, rSet, nFlags );
            }
        FOREACHPAM_END()

        GetDoc()->EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->InsertItemSet( *pCrsr, rSet, nFlags );
    }
    EndAllAction();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
