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
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <SwRewriter.hxx>
#include <osl/diagnose.h>
#include <wrtsh.hxx>
#include <officecfg/Office/Writer.hxx>

#include <strings.hrc>
#include <vector>

void SwEditShell::DeleteSel(SwPaM& rPam, bool const isArtificialSelection, bool goLeft,
                            bool* const pUndo)
{
    auto const oSelectAll(StartsWith_() != SwCursorShell::StartsWith::None
        ? ExtendedSelectedAll()
        : ::std::optional<::std::pair<SwNode const*, ::std::vector<SwTableNode *>>>{});
    // only for selections
    if (!rPam.HasMark()
        || (*rPam.GetPoint() == *rPam.GetMark()
            && !IsFlySelectedByCursor(*GetDoc(), *rPam.Start(), *rPam.End())))
    {
        return;
    }

    // Is the selection in a table? Then delete only the content of the selected boxes.
    // Here, there are two cases:
    // 1. Point and Mark are in one box, delete selection as usual
    // 2. Point and Mark are in different boxes, search all selected boxes and delete content
    // 3. Point and Mark are at the document start and end, Point is in a table: delete selection as usual
    if( rPam.GetPointNode().FindTableNode() &&
        rPam.GetPointNode().StartOfSectionNode() !=
        rPam.GetMarkNode().StartOfSectionNode() && !oSelectAll)
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
            SwNode& rNd = aDelPam.GetPointNode();
            const SwNode& rEndNd = *rNd.EndOfSectionNode();
            if( pEndSelPos->GetNodeIndex() <= rEndNd.GetIndex() )
            {
                *aDelPam.GetPoint() = *pEndSelPos;
                pEndSelPos = nullptr;     // misuse a pointer as a flag
            }
            else
            {
                // then go to the end of the selection
                aDelPam.GetPoint()->Assign(rEndNd);
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
        std::optional<SwPaM> pNewPam;
        SwPaM * pPam = &rPam;
        if (oSelectAll)
        {
            if (!oSelectAll->second.empty())
            {
                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, SwResId(STR_MULTISEL));
                GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::DELETE, &aRewriter);
            }
            // tdf#155685 tables at the end must be deleted separately
            for (SwTableNode *const pTable : oSelectAll->second)
            {
                GetDoc()->DelTable(pTable);
            }
            assert(dynamic_cast<SwShellCursor*>(&rPam)); // must be corrected pam
            pNewPam.emplace(*rPam.GetMark(), *rPam.GetPoint());
            // Selection starts at the first para of the first cell, but we
            // want to delete the table node before the first cell as well.
            pNewPam->Start()->Assign(*oSelectAll->first);
            pPam = &*pNewPam;
        }
        // delete everything
        GetDoc()->getIDocumentContentOperations().DeleteAndJoin(*pPam,
            isArtificialSelection ? SwDeleteFlags::ArtificialSelection : SwDeleteFlags::Default);
        SaveTableBoxContent( pPam->GetPoint() );
        if (oSelectAll && !oSelectAll->second.empty())
        {
            GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
        }
    }

    rPam.Normalize(goLeft); // change tracking case: will make sure to end up in the correct point
    // Selection is not needed anymore
    rPam.DeleteMark();
}

bool SwEditShell::Delete(bool const isArtificialSelection, bool goLeft)
{
    CurrShell aCurr( this );
    bool bRet = false;
    if ( !HasReadonlySel() || CursorInsideInputField() )
    {
        if (HasHiddenSections() &&
            officecfg::Office::Writer::Content::Display::ShowWarningHiddenSection::get())
        {
            if (!WarnHiddenSectionDialog())
            {
                bRet = RemoveParagraphMetadataFieldAtCursor();
                return bRet;
            }
        }

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
            DeleteSel(rPaM, isArtificialSelection, goLeft, &bUndo);
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
        if (!bRet)
        {
            InfoReadOnlyDialog(false);
        }
    }

    return bRet;
}

bool SwEditShell::Copy( SwEditShell& rDestShell )
{
    CurrShell aCurr( &rDestShell );

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
                if( &rDestShell == this )
                {
                    // First cursor represents the target position!!
                    rPaM.DeleteMark();
                    pPos = rPaM.GetPoint();
                    continue;
                }
                else
                    pPos = rDestShell.GetCursor()->GetPoint();
            }
            if( IsBlockMode() )
            {   // In block mode different insert positions will be calculated
                // by simulated cursor movements from the given first insert position
                if( nMove )
                {
                    SwCursor aCursor( *pPos, nullptr);
                    if (aCursor.UpDown(false, nMove, nullptr, 0, *GetLayout()))
                    {
                        pInsertPos = std::make_shared<SwPosition>( *aCursor.GetPoint() );
                        aInsertList.push_back( pInsertPos );
                    }
                }
                else
                    pInsertPos = std::make_shared<SwPosition>( *pPos );
                ++nMove;
            }
            SwPosition *pTmp = IsBlockMode() ? pInsertPos.get() : pPos;
            // Check if a selection would be copied into itself
            if( rDestShell.GetDoc() == GetDoc() &&
                *rPaM.Start() <= *pTmp && *pTmp < *rPaM.End() )
                return false;
        }
    }

    rDestShell.StartAllAction();
    SwPosition *pPos = nullptr;
    bool bRet = false;
    bool bFirstMove = true;
    SwNodeIndex aSttNdIdx( rDestShell.GetDoc()->GetNodes() );
    sal_Int32 nSttCntIdx = 0;
    // For block selection this list is filled with the insert positions
    auto pNextInsert = aInsertList.begin();

    rDestShell.GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        if( !pPos )
        {
            if( &rDestShell == this )
            {
                // First cursor represents the target position!!
                rPaM.DeleteMark();
                pPos = rPaM.GetPoint();
                continue;
            }
            else
                pPos = rDestShell.GetCursor()->GetPoint();
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
            aSttNdIdx = pPos->GetNodeIndex()-1;
            nSttCntIdx = pPos->GetContentIndex();
            bFirstMove = false;
        }

        const bool bSuccess( GetDoc()->getIDocumentContentOperations().CopyRange(rPaM, *pPos, SwCopyFlags::CheckPosInFly) );
        if (!bSuccess)
            continue;

        SwPaM aInsertPaM(*pPos, SwPosition(aSttNdIdx));
        rDestShell.GetDoc()->MakeUniqueNumRules(aInsertPaM);

        bRet = true;
    }

    // Maybe nothing has been moved?
    if( !bFirstMove )
    {
        SwPaM* pCursor = rDestShell.GetCursor();
        pCursor->SetMark();
        pCursor->GetPoint()->Assign( aSttNdIdx.GetIndex()+1, nSttCntIdx );
        pCursor->Exchange();
    }
    else
    {
        // If the cursor moved during move process, move also its GetMark
        rDestShell.GetCursor()->SetMark();
        rDestShell.GetCursor()->DeleteMark();
    }
#if OSL_DEBUG_LEVEL > 0
    // check if the indices are registered in the correct nodes
    {
        for(SwPaM& rCmp : rDestShell.GetCursor()->GetRingContainer())
        {
            OSL_ENSURE( rCmp.GetPoint()->GetContentNode()
                        == rCmp.GetPointContentNode(), "Point in wrong Node" );
            OSL_ENSURE( rCmp.GetMark()->GetContentNode()
                        == rCmp.GetMarkContentNode(), "Mark in wrong Node" );
        }
    }
#endif

    // close Undo container here
    rDestShell.GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    rDestShell.EndAllAction();

    rDestShell.SaveTableBoxContent( rDestShell.GetCursor()->GetPoint() );

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
    CurrShell aCurr( this );

    bool bRet = false;
    if (!HasReadonlySel(true))
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

/** Replace a selected area in a text node with a given string.
 *
 * Method to replace a text selection with a new string while
 * keeping possible comments (they will be moved to the end
 * of the selection).
 *
 * @param rNewStr     the new string which the selected area is to be replaced with
 * @return            true, if something has been replaced, false otherwise.
 */
bool SwEditShell::ReplaceKeepComments( const OUString& rNewStr)
{
    bool bRet       = false;
    SwPaM *pCursor  = GetCursor();

    if(pCursor != nullptr)
    {
        // go sure that the text selection pointed to by pCursor is valid
        if(pCursor->HasMark())
        {
            // preserve comments inside of the number by deleting number portions starting from the back
            OUString aSelectedText = pCursor->GetText();
            sal_Int32 nCommentPos(aSelectedText.lastIndexOf(CH_TXTATR_INWORD));
            // go sure that we have a valid selection and a comment has been found
            while (nCommentPos > -1)
            {
                // select the part of the text after the last found comment
                // selection start:
                pCursor->GetPoint()->AdjustContent(nCommentPos + 1);
                // selection end is left where it is -> will be adjusted later on
                // delete the part of the word after the last found comment
                Replace(OUString(), false);
                // put the selection start back to the beginning of the word
                pCursor->GetPoint()->AdjustContent(-(nCommentPos + 1));
                // adjust the selection end, so that the last comment is no longer selected:
                pCursor->GetMark()->AdjustContent(-1);
                // search for the next possible comment
                aSelectedText = pCursor->GetText();
                nCommentPos = aSelectedText.lastIndexOf(CH_TXTATR_INWORD);
            }
            bRet = Replace(rNewStr, false);
        }
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
            CurrShell aCurr( this );
            StartAllAction();
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( *pCursor );
            EndAllAction();
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
