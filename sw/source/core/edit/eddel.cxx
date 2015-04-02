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

#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <editsh.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <edimp.hxx>
#include <IMark.hxx>
#include <docary.hxx>
#include <SwRewriter.hxx>
#include <globals.hrc>

#include <comcore.hrc>
#include <list>

void SwEditShell::DeleteSel( SwPaM& rPam, bool* pUndo )
{
    bool bSelectAll = StartsWithTable() && ExtendedSelectedAll(/*bFootnotes =*/ false);
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
            GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
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
                pEndSelPos = 0;     // misuse a pointer as a flag
            }
            else
            {
                // then go to the end of the selection
                aDelPam.GetPoint()->nNode = rEndNd;
                aDelPam.Move( fnMoveBackward, fnGoCntnt );
            }
            // skip protected boxes
            if( !rNd.IsCntntNode() ||
                !rNd.IsInProtectSect() )
            {
                // delete everything
                GetDoc()->getIDocumentContentOperations().DeleteAndJoin( aDelPam );
                SaveTblBoxCntnt( aDelPam.GetPoint() );
            }

            if( !pEndSelPos ) // at the end of a selection
                break;
            aDelPam.DeleteMark();
            aDelPam.Move( fnMoveForward, fnGoCntnt ); // next box
        } while( pEndSelPos );
    }
    else
    {
        std::unique_ptr<SwPaM> pNewPam;
        SwPaM * pPam = &rPam;
        if (bSelectAll)
        {
            assert(dynamic_cast<SwShellCrsr*>(&rPam)); // must be corrected pam
            pNewPam.reset(new SwPaM(rPam));
            // Selection starts at the first para of the first cell, but we
            // want to delete the table node before the first cell as well.
            pNewPam->Start()->nNode = pNewPam->Start()->nNode.GetNode().FindTableNode()->GetIndex();
            pNewPam->Start()->nContent.Assign(nullptr, 0);
            pPam = pNewPam.get();
        }
        // delete everything
        GetDoc()->getIDocumentContentOperations().DeleteAndJoin(*pPam);
        SaveTblBoxCntnt( pPam->GetPoint() );
    }

    // Selection is not needed anymore
    rPam.DeleteMark();
}

long SwEditShell::Delete()
{
    SET_CURR_SHELL( this );
    long nRet = 0;
    if ( !HasReadonlySel() || CrsrInsideInputFld() )
    {
        StartAllAction();

        bool bUndo = GetCrsr()->GetNext() != GetCrsr();
        if( bUndo ) // more than one selection?
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_MULTISEL));

            GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_DELETE, &aRewriter);
        }

        FOREACHPAM_START(GetCrsr())
            DeleteSel( *PCURCRSR, &bUndo );
        FOREACHPAM_END()

        // If undo container then close here
        if( bUndo )
        {
            GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
        }
        EndAllAction();
        nRet = 1;
    }
    return nRet;
}

long SwEditShell::Copy( SwEditShell* pDestShell )
{
    if( !pDestShell )
        pDestShell = this;

    SET_CURR_SHELL( pDestShell );

    // List of insert positions for smart insert of block selections
    std::list< boost::shared_ptr<SwPosition> > aInsertList;

    // Fill list of insert positions
    {
        SwPosition * pPos = 0;
        boost::shared_ptr<SwPosition> pInsertPos;
        sal_uInt16 nMove = 0;
        FOREACHPAM_START(GetCrsr())

            if( !pPos )
            {
                if( pDestShell == this )
                {
                    // First cursor represents the target position!!
                    PCURCRSR->DeleteMark();
                    pPos = (SwPosition*)PCURCRSR->GetPoint();
                    continue;
                }
                else
                    pPos = pDestShell->GetCrsr()->GetPoint();
            }
            if( IsBlockMode() )
            {   // In block mode different insert positions will be calculated
                // by simulated cursor movements from the given first insert position
                if( nMove )
                {
                    SwCursor aCrsr( *pPos, 0, false);
                    if( aCrsr.UpDown( false, nMove, 0, 0 ) )
                    {
                        pInsertPos.reset( new SwPosition( *aCrsr.GetPoint() ) );
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
                *PCURCRSR->Start() <= *pTmp && *pTmp < *PCURCRSR->End() )
                return sal_False;
        FOREACHPAM_END()
    }

    pDestShell->StartAllAction();
    SwPosition *pPos = 0;
    bool bRet = false;
    bool bFirstMove = true;
    SwNodeIndex aSttNdIdx( pDestShell->GetDoc()->GetNodes() );
    sal_Int32 nSttCntIdx = 0;
    // For block selection this list is filled with the insert positions
    std::list< boost::shared_ptr<SwPosition> >::iterator pNextInsert = aInsertList.begin();

    pDestShell->GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    FOREACHPAM_START(GetCrsr())

        if( !pPos )
        {
            if( pDestShell == this )
            {
                // First cursor represents the target position!!
                PCURCRSR->DeleteMark();
                pPos = (SwPosition*)PCURCRSR->GetPoint();
                continue;
            }
            else
                pPos = pDestShell->GetCrsr()->GetPoint();
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
        if( !PCURCRSR->HasMark() || *PCURCRSR->GetPoint() == *PCURCRSR->GetMark() )
            continue;

        if( bFirstMove )
        {
            // Store start position of the new area
            aSttNdIdx = pPos->nNode.GetIndex()-1;
            nSttCntIdx = pPos->nContent.GetIndex();
            bFirstMove = false;
        }

        const bool bSuccess( GetDoc()->getIDocumentContentOperations().CopyRange( *PCURCRSR, *pPos, false ) );
        if (!bSuccess)
            continue;

        SwPaM aInsertPaM(*pPos, SwPosition(aSttNdIdx));
        pDestShell->GetDoc()->MakeUniqueNumRules(aInsertPaM);

        bRet = true;
    FOREACHPAM_END()

    // Maybe nothing has been moved?
    if( !bFirstMove )
    {
        SwPaM* pCrsr = pDestShell->GetCrsr();
        pCrsr->SetMark();
        pCrsr->GetPoint()->nNode = aSttNdIdx.GetIndex()+1;
        pCrsr->GetPoint()->nContent.Assign( pCrsr->GetCntntNode(),nSttCntIdx);
        pCrsr->Exchange();
    }
    else
    {
        // If the cursor moved during move process, move also its GetMark
        pDestShell->GetCrsr()->SetMark();
        pDestShell->GetCrsr()->DeleteMark();
    }
#if OSL_DEBUG_LEVEL > 0
// check if the indices are registered in the correct nodes
{
    SwPaM* pCmp = (SwPaM*)pDestShell->GetCrsr();        // store pointer to cursor
    do {
        OSL_ENSURE( pCmp->GetPoint()->nContent.GetIdxReg()
                    == pCmp->GetCntntNode(), "Point in wrong Node" );
        OSL_ENSURE( pCmp->GetMark()->nContent.GetIdxReg()
                    == pCmp->GetCntntNode(false), "Mark in wrong Node" );
        bool bTst = *pCmp->GetPoint() == *pCmp->GetMark();
        (void) bTst;
    } while( pDestShell->GetCrsr() != ( pCmp = static_cast<SwPaM*>(pCmp->GetNext()) ) );
}
#endif

    // close Undo container here
    pDestShell->GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    pDestShell->EndAllAction();

    pDestShell->SaveTblBoxCntnt( pDestShell->GetCrsr()->GetPoint() );

    return (long)bRet;
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
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

        FOREACHPAM_START(GetCrsr())
            if( PCURCRSR->HasMark() && *PCURCRSR->GetMark() != *PCURCRSR->GetPoint() )
            {
                bRet = GetDoc()->getIDocumentContentOperations().ReplaceRange( *PCURCRSR, rNewStr, bRegExpRplc )
                    || bRet;
                SaveTblBoxCntnt( PCURCRSR->GetPoint() );
            }
        FOREACHPAM_END()

        // close Undo container here
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
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
        SwPaM* pCrsr = GetCrsr();
        // no multi selection
        if( pCrsr->GetNext() == pCrsr && !HasReadonlySel() )
        {
            SET_CURR_SHELL( this );
            StartAllAction();
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( *pCrsr );
            EndAllAction();
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
