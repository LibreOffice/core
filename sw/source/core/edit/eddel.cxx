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

#include <memory>
#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <editsh.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <unocrsr.hxx>
#include <swundo.hxx>
#include <edimp.hxx>
#include <IMark.hxx>
#include <docary.hxx>
#include <SwRewriter.hxx>
#include <globals.hrc>

#include <strings.hrc>
#include <vector>

void SwEditShell::DeleteSel( SwPaM& rPam, bool* pUndo )
{
    bool bSelectAll = StartsWithTable() && ExtendedSelectedAll();
    // only for selections
    if( !rPam.HasMark() || *rPam.GetPoint() == *rPam.GetMark())
        return;

    // Is the selection in a table? Then delete only the content of the selected boxes.
    // Here, there are two cases:
    // 1. Point and Mark are in one box, delete selection as usual
    // 2. Point and Mark are in different boxes, search all selected boxes and delete content
    // 3. Point and Mark are at the document start and end, Point is in a table: delete selection as usual
    if( rPam.GetNode().FindTableNode() &&
        rPam.GetNode().StartOfSectionNode() !=
        rPam.GetNode(false).StartOfSectionNode() && !bSelectAll )
    {
        // group the Undo in the table
        if( pUndo && !*pUndo )
        {
            GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
            *pUndo = true;
        }
        SwPaM aDelPam( *rPam.Start() );
        const SwPosition* pEndSelPos = rPam.End();
        do {
            aDelPam.SetMark();
            SwNode& rNd = aDelPam.GetNode();
            const SwNode& rEndNd = *rNd.EndOfSectionNode();
            if( pEndSelPos->nNode.GetIndex() <= rEndNd.GetIndex() )
            {
                *aDelPam.GetPoint() = *pEndSelPos;
                pEndSelPos = nullptr;     // misuse a pointer as a flag
            }
            else
            {
                // then go to the end of the selection
                aDelPam.GetPoint()->nNode = rEndNd;
                aDelPam.Move( fnMoveBackward, GoInContent );
            }
            // skip protected boxes
            if( !rNd.IsContentNode() ||
                !rNd.IsInProtectSect() )
            {
                // delete everything
                GetDoc()->getIDocumentContentOperations().DeleteAndJoin( aDelPam );
                SaveTableBoxContent( aDelPam.GetPoint() );
            }

            if( !pEndSelPos ) // at the end of a selection
                break;
            aDelPam.DeleteMark();
            aDelPam.Move( fnMoveForward, GoInContent ); // next box
        } while( pEndSelPos );
    }
    else
    {
        std::unique_ptr<SwPaM> pNewPam;
        SwPaM * pPam = &rPam;
        if (bSelectAll)
        {
            assert(dynamic_cast<SwShellCursor*>(&rPam)); // must be corrected pam
            pNewPam.reset(new SwPaM(*rPam.GetMark(), *rPam.GetPoint()));
            // Selection starts at the first para of the first cell, but we
            // want to delete the table node before the first cell as well.
            pNewPam->Start()->nNode = pNewPam->Start()->nNode.GetNode().FindTableNode()->GetIndex();
            pNewPam->Start()->nContent.Assign(nullptr, 0);
            pPam = pNewPam.get();
        }
        // delete everything
        GetDoc()->getIDocumentContentOperations().DeleteAndJoin(*pPam);
        SaveTableBoxContent( pPam->GetPoint() );
    }

    // Selection is not needed anymore
    rPam.DeleteMark();
}

bool SwEditShell::Delete()
{
    SET_CURR_SHELL( this );
    bool bRet = false;
    if ( !HasReadonlySel() || CursorInsideInputField() )
    {
        StartAllAction();

        bool bUndo = GetCursor()->GetNext() != GetCursor();
        if( bUndo ) // more than one selection?
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SwResId(STR_MULTISEL));

            GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::DELETE, &aRewriter);
        }

        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            DeleteSel( rPaM, &bUndo );
        }

        // If undo container then close here
        if( bUndo )
        {
            GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
        }
        EndAllAction();
        bRet = true;
    }
    else
    {
        bRet = RemoveParagraphMetadataFieldAtCursor();
    }

    return bRet;
}

bool SwEditShell::Copy( SwEditShell* pDestShell )
{
    if( !pDestShell )
        pDestShell = this;

    SET_CURR_SHELL( pDestShell );

    // List of insert positions for smart insert of block selections
    std::vector< std::shared_ptr<SwPosition> > aInsertList;

    // Fill list of insert positions
    {
        SwPosition * pPos = nullptr;
        std::shared_ptr<SwPosition> pInsertPos;
        sal_uInt16 nMove = 0;
        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            if( !pPos )
            {
                if( pDestShell == this )
                {
                    // First cursor represents the target position!!
                    rPaM.DeleteMark();
                    pPos = rPaM.GetPoint();
                    continue;
                }
                else
                    pPos = pDestShell->GetCursor()->GetPoint();
            }
            if( IsBlockMode() )
            {   // In block mode different insert positions will be calculated
                // by simulated cursor movements from the given first insert position
                if( nMove )
                {
                    SwCursor aCursor( *pPos, nullptr);
                    if (aCursor.UpDown(false, nMove, nullptr, 0, *GetLayout()))
                    {
                        pInsertPos.reset( new SwPosition( *aCursor.GetPoint() ) );
                        aInsertList.push_back( pInsertPos );
                    }
                }
                else
                    pInsertPos.reset( new SwPosition( *pPos ) );
                ++nMove;
            }
            SwPosition *pTmp = IsBlockMode() ? pInsertPos.get() : pPos;
            // Check if a selection would be copied into itself
            if( pDestShell->GetDoc() == GetDoc() &&
                *rPaM.Start() <= *pTmp && *pTmp < *rPaM.End() )
                return false;
        }
    }

    pDestShell->StartAllAction();
    SwPosition *pPos = nullptr;
    bool bRet = false;
    bool bFirstMove = true;
    SwNodeIndex aSttNdIdx( pDestShell->GetDoc()->GetNodes() );
    sal_Int32 nSttCntIdx = 0;
    // For block selection this list is filled with the insert positions
    auto pNextInsert = aInsertList.begin();

    pDestShell->GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        if( !pPos )
        {
            if( pDestShell == this )
            {
                // First cursor represents the target position!!
                rPaM.DeleteMark();
                pPos = rPaM.GetPoint();
                continue;
            }
            else
                pPos = pDestShell->GetCursor()->GetPoint();
        }
        if( !bFirstMove )
        {
            if( pNextInsert != aInsertList.end() )
            {
                pPos = pNextInsert->get();
                ++pNextInsert;
            }
            else if( IsBlockMode() )
                GetDoc()->getIDocumentContentOperations().SplitNode( *pPos, false );
        }

        // Only for a selection (non-text nodes have selection but Point/GetMark are equal)
        if( !rPaM.HasMark() || *rPaM.GetPoint() == *rPaM.GetMark() )
            continue;

        if( bFirstMove )
        {
            // Store start position of the new area
            aSttNdIdx = pPos->nNode.GetIndex()-1;
            nSttCntIdx = pPos->nContent.GetIndex();
            bFirstMove = false;
        }

        const bool bSuccess( GetDoc()->getIDocumentContentOperations().CopyRange( rPaM, *pPos, /*bCopyAll=*/false, /*bCheckPos=*/true ) );
        if (!bSuccess)
            continue;

        SwPaM aInsertPaM(*pPos, SwPosition(aSttNdIdx));
        pDestShell->GetDoc()->MakeUniqueNumRules(aInsertPaM);

        bRet = true;
    }

    // Maybe nothing has been moved?
    if( !bFirstMove )
    {
        SwPaM* pCursor = pDestShell->GetCursor();
        pCursor->SetMark();
        pCursor->GetPoint()->nNode = aSttNdIdx.GetIndex()+1;
        pCursor->GetPoint()->nContent.Assign( pCursor->GetContentNode(),nSttCntIdx);
        pCursor->Exchange();
    }
    else
    {
        // If the cursor moved during move process, move also its GetMark
        pDestShell->GetCursor()->SetMark();
        pDestShell->GetCursor()->DeleteMark();
    }
#if OSL_DEBUG_LEVEL > 0
    // check if the indices are registered in the correct nodes
    {
        for(SwPaM& rCmp : pDestShell->GetCursor()->GetRingContainer())
        {
            OSL_ENSURE( rCmp.GetPoint()->nContent.GetIdxReg()
                        == rCmp.GetContentNode(), "Point in wrong Node" );
            OSL_ENSURE( rCmp.GetMark()->nContent.GetIdxReg()
                        == rCmp.GetContentNode(false), "Mark in wrong Node" );
        }
    }
#endif

    // close Undo container here
    pDestShell->GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    pDestShell->EndAllAction();

    pDestShell->SaveTableBoxContent( pDestShell->GetCursor()->GetPoint() );

    return bRet;
}

/** Replace a selected area in a text node with a given string.
 *
 * Intended for "search & replace".
 *
 * @param bRegExpRplc if <true> replace tabs (\\t) and replace with found string (not \&).
 *                    E.g. [Fnd: "zzz", Repl: "xx\t\\t..&..\&"] --> "xx\t<Tab>..zzz..&"
 */
bool SwEditShell::Replace( const OUString& rNewStr, bool bRegExpRplc )
{
    SET_CURR_SHELL( this );

    bool bRet = false;
    if( !HasReadonlySel() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            if( rPaM.HasMark() && *rPaM.GetMark() != *rPaM.GetPoint() )
            {
                bRet = sw::ReplaceImpl(rPaM, rNewStr, bRegExpRplc, *GetDoc(), GetLayout())
                    || bRet;
                SaveTableBoxContent( rPaM.GetPoint() );
            }
        }

        // close Undo container here
        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
        EndAllAction();
    }
    return bRet;
}

/// special method for JOE's wizards
bool SwEditShell::DelFullPara()
{
    bool bRet = false;
    if( !IsTableMode() )
    {
        SwPaM* pCursor = GetCursor();
        // no multi selection
        if( !pCursor->IsMultiSelection() && !HasReadonlySel() )
        {
            SET_CURR_SHELL( this );
            StartAllAction();
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( *pCursor );
            EndAllAction();
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
