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

#include <editsh.hxx>
#include <doc.hxx>      // for aNodes
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>      // for SwPaM
#include <edimp.hxx>    // for MACROS
#include <swundo.hxx>   // for the UndoIds
#include <ndtxt.hxx>    // fot Get-/ChgFmt Set-/GetAttrXXX

/*
 * hard formatting (Attribute)
 */

void SwEditShell::ResetAttr( const std::set<sal_uInt16> &attrs, SwPaM* pPaM )
{
    SET_CURR_SHELL( this );
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr( );

    StartAllAction();
    bool bUndoGroup = pCrsr->GetNext() != pCrsr;
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
    FOREACHPAM_START(GetCrsr())
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

/// Set the attribute as new default attribute in the document.
void SwEditShell::SetDefault( const SfxPoolItem& rFmtHint )
{
    // 7502: Action-Parenthesis
    StartAllAction();
    GetDoc()->SetDefault( rFmtHint );
    EndAllAction();
}

/// request the default attribute in this document.
const SfxPoolItem& SwEditShell::GetDefault( sal_uInt16 nFmtHint ) const
{
    return GetDoc()->GetDefault( nFmtHint );
}

void SwEditShell::SetAttr( const SfxPoolItem& rHint, sal_uInt16 nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring of Cursors
    {
        sal_Bool bIsTblMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, NULL);

        FOREACHPAM_START(GetCrsr())
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
    if( pCrsr->GetNext() != pCrsr )     // Ring of Cursors
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
