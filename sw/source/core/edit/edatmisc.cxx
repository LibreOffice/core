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
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <edimp.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>

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
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_RESETATTR, nullptr);
    }

    for(SwPaM& rCurCrsr : pCrsr->GetRingContainer())
        GetDoc()->ResetAttrs(rCurCrsr, true, attrs);

    if( bUndoGroup )
    {
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_RESETATTR, nullptr);
    }
    CallChgLnk();
    EndAllAction();
}

void SwEditShell::GCAttr()
{
    for(SwPaM& rPaM : GetCrsr()->GetRingContainer())
    {
        if ( !rPaM.HasMark() )
        {
            SwTextNode *const pTextNode =
                rPaM.GetPoint()->nNode.GetNode().GetTextNode();
            if (pTextNode)
            {
                pTextNode->GCAttr();
            }
        }
        else
        {
            const SwNodeIndex& rEnd = rPaM.End()->nNode;
            SwNodeIndex aIdx( rPaM.Start()->nNode );
            SwNode* pNd = &aIdx.GetNode();
            do {
                if( pNd->IsTextNode() )
                    static_cast<SwTextNode*>(pNd)->GCAttr();
            }
            while( nullptr != ( pNd = GetDoc()->GetNodes().GoNext( &aIdx )) &&
                    aIdx <= rEnd );
        }
    }
}

/// Set the attribute as new default attribute in the document.
void SwEditShell::SetDefault( const SfxPoolItem& rFormatHint )
{
    // 7502: Action-Parenthesis
    StartAllAction();
    GetDoc()->SetDefault( rFormatHint );
    EndAllAction();
}

/// request the default attribute in this document.
const SfxPoolItem& SwEditShell::GetDefault( sal_uInt16 nFormatHint ) const
{
    return GetDoc()->GetDefault( nFormatHint );
}

void SwEditShell::SetAttrItem( const SfxPoolItem& rHint, SetAttrMode nFlags )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )     // Ring of Cursors
    {
        bool bIsTableMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, nullptr);

        for(SwPaM& rPaM : GetCrsr()->GetRingContainer())
        {
            if( rPaM.HasMark() && ( bIsTableMode ||
                *rPaM.GetPoint() != *rPaM.GetMark() ))
            {
                GetDoc()->getIDocumentContentOperations().InsertPoolItem(rPaM, rHint, nFlags );
            }
        }

        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_INSATTR, nullptr);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->getIDocumentContentOperations().InsertPoolItem( *pCrsr, rHint, nFlags );
    }
    EndAllAction();
}

void SwEditShell::SetAttrSet( const SfxItemSet& rSet, SetAttrMode nFlags, SwPaM* pPaM )
{
    SET_CURR_SHELL( this );

    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    StartAllAction();
    if( pCrsr->GetNext() != pCrsr )     // Ring of Cursors
    {
        bool bIsTableMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_INSATTR, nullptr);

        for(SwPaM& rTmpCrsr : pCrsr->GetRingContainer())
        {
            if( rTmpCrsr.HasMark() && ( bIsTableMode ||
                *rTmpCrsr.GetPoint() != *rTmpCrsr.GetMark() ))
            {
                GetDoc()->getIDocumentContentOperations().InsertItemSet(rTmpCrsr, rSet, nFlags );
            }
        }

        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_INSATTR, nullptr);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();
        GetDoc()->getIDocumentContentOperations().InsertItemSet( *pCrsr, rSet, nFlags );
    }
    EndAllAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
