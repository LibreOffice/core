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
#include <IDocumentRedlineAccess.hxx>
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
    SwPaM* pCursor = pPaM ? pPaM : GetCursor( );

    StartAllAction();
    bool bUndoGroup = pCursor->GetNext() != pCursor;
    if( bUndoGroup )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::RESETATTR, nullptr);
    }

    for(SwPaM& rCurrentCursor : pCursor->GetRingContainer())
        GetDoc()->ResetAttrs(rCurrentCursor, true, attrs, true, GetLayout());

    if( bUndoGroup )
    {
        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::RESETATTR, nullptr);
    }
    CallChgLnk();
    EndAllAction();
}

void SwEditShell::GCAttr()
{
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
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

// tdf#122893 turn off ShowChanges mode to apply paragraph formatting permanently with redlining
// ie. in all directly preceding deleted paragraphs at the actual cursor positions
static void lcl_disableShowChangesIfNeeded( SwDoc *const pDoc, const SwNode& rNode, RedlineFlags &eRedlMode )
{
    if ( IDocumentRedlineAccess::IsShowChanges(eRedlMode) &&
        // is there redlining at beginning of the position (possible redline block before the modified node)
        pDoc->getIDocumentRedlineAccess().GetRedlinePos( rNode, USHRT_MAX ) <
                   pDoc->getIDocumentRedlineAccess().GetRedlineTable().size() )
    {
        eRedlMode = RedlineFlags::ShowInsert | RedlineFlags::Ignore;
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags( eRedlMode );
    }
}

void SwEditShell::SetAttrItem( const SfxPoolItem& rHint, SetAttrMode nFlags, const bool bParagraphSetting )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    RedlineFlags eRedlMode = GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags(), eOldMode = eRedlMode;
    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor )     // Ring of Cursors
    {
        bool bIsTableMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSATTR, nullptr);

        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            if( rPaM.HasMark() && ( bIsTableMode ||
                *rPaM.GetPoint() != *rPaM.GetMark() ))
            {
                if (bParagraphSetting)
                    lcl_disableShowChangesIfNeeded( GetDoc(), (*rPaM.Start()).nNode.GetNode(), eRedlMode);

                GetDoc()->getIDocumentContentOperations().InsertPoolItem(rPaM, rHint, nFlags, GetLayout());
            }
        }

        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSATTR, nullptr);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();

        if (bParagraphSetting)
            lcl_disableShowChangesIfNeeded( GetDoc(), (*pCursor->Start()).nNode.GetNode(), eRedlMode);

        GetDoc()->getIDocumentContentOperations().InsertPoolItem(*pCursor, rHint, nFlags, GetLayout());
    }
    EndAllAction();
    GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eOldMode );
}

void SwEditShell::SetAttrSet( const SfxItemSet& rSet, SetAttrMode nFlags, SwPaM* pPaM, const bool bParagraphSetting )
{
    SET_CURR_SHELL( this );
    SwPaM* pCursor = pPaM ? pPaM : GetCursor();
    StartAllAction();
    RedlineFlags eRedlMode = GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags(), eOldMode = eRedlMode;
    if( pCursor->GetNext() != pCursor )     // Ring of Cursors
    {
        bool bIsTableMode = IsTableMode();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSATTR, nullptr);

        for(SwPaM& rTmpCursor : pCursor->GetRingContainer())
        {
            if( rTmpCursor.HasMark() && ( bIsTableMode ||
                *rTmpCursor.GetPoint() != *rTmpCursor.GetMark() ))
            {
                if (bParagraphSetting)
                    lcl_disableShowChangesIfNeeded( GetDoc(), (*rTmpCursor.Start()).nNode.GetNode(), eRedlMode);

                GetDoc()->getIDocumentContentOperations().InsertItemSet(rTmpCursor, rSet, nFlags, GetLayout());
            }
        }

        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSATTR, nullptr);
    }
    else
    {
        if( !HasSelection() )
            UpdateAttr();

        if (bParagraphSetting)
            lcl_disableShowChangesIfNeeded( GetDoc(), (*pCursor->Start()).nNode.GetNode(), eRedlMode);

        GetDoc()->getIDocumentContentOperations().InsertItemSet(*pCursor, rSet, nFlags, GetLayout());
    }
    EndAllAction();
    GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eOldMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
