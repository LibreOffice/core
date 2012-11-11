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


#include <editsh.hxx>
#include <doc.hxx>      // fuer aNodes
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>      // fuer SwPaM
#include <edimp.hxx>    // fuer MACROS
#include <swundo.hxx>   // fuer die UndoIds
#include <ndtxt.hxx>    // fuer Get-/ChgFmt Set-/GetAttrXXX



/*************************************
 * harte Formatierung (Attribute)
 *************************************/


void SwEditShell::ResetAttr( const std::set<sal_uInt16> &attrs, SwPaM* pPaM )
{
    SET_CURR_SHELL( this );
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr( );

    StartAllAction();
    sal_Bool bUndoGroup = pCrsr->GetNext() != pCrsr;
    if( bUndoGroup )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_RESETATTR, NULL);
    }

        SwPaM* pStartCrsr = pCrsr;
        do {
                GetDoc()->ResetAttrs(*pCrsr, true, attrs);
        } while ( ( pCrsr = ( SwPaM* ) pCrsr->GetNext() ) != pStartCrsr );

    if( bUndoGroup )
    {
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_RESETATTR, NULL);
    }
    CallChgLnk();
    EndAllAction();
}



void SwEditShell::GCAttr()
{
    FOREACHPAM_START(this)
        if ( !PCURCRSR->HasMark() )
        {
            SwTxtNode *const pTxtNode =
                PCURCRSR->GetPoint()->nNode.GetNode().GetTxtNode();
            if (pTxtNode)
            {
                pTxtNode->GCAttr();
            }
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
}

// Setze das Attribut als neues default Attribut im Dokument.


void SwEditShell::SetDefault( const SfxPoolItem& rFmtHint )
{
    // 7502: Action-Klammerung
    StartAllAction();
    GetDoc()->SetDefault( rFmtHint );
    EndAllAction();
}

// Erfrage das Default Attribut in diesem Dokument.

const SfxPoolItem& SwEditShell::GetDefault( sal_uInt16 nFmtHint ) const
{
    return GetDoc()->GetDefault( nFmtHint );

}


void SwEditShell::SetAttr( const SfxPoolItem& rHint, sal_uInt16 nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        sal_Bool bIsTblMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, NULL);

        FOREACHPAM_START(this)
            if( PCURCRSR->HasMark() && ( bIsTblMode ||
                *PCURCRSR->GetPoint() != *PCURCRSR->GetMark() ))
            {
                GetDoc()->InsertPoolItem(*PCURCRSR, rHint, nFlags );
            }
        FOREACHPAM_END()

        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_INSATTR, NULL);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->InsertPoolItem( *pCrsr, rHint, nFlags );
    }
    EndAllAction();
}


void SwEditShell::SetAttr( const SfxItemSet& rSet, sal_uInt16 nFlags, SwPaM* pPaM )
{
    SET_CURR_SHELL( this );

    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    StartAllAction();
    if( pCrsr->GetNext() != pCrsr )     // Ring von Cursorn
    {
        sal_Bool bIsTblMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, NULL);

        SwPaM* pTmpCrsr = pCrsr;
        SwPaM* pStartPaM = pCrsr;
        do {
            if( pTmpCrsr->HasMark() && ( bIsTblMode ||
                *pTmpCrsr->GetPoint() != *pTmpCrsr->GetMark() ))
            {
                GetDoc()->InsertItemSet(*pTmpCrsr, rSet, nFlags );
            }
        } while ( ( pTmpCrsr = (SwPaM*)pTmpCrsr->GetNext() ) != pStartPaM );

        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_INSATTR, NULL);
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
