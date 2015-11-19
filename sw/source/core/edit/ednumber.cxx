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
#include <editsh.hxx>
#include <edimp.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <swundo.hxx>
#include <numrule.hxx>

SwPamRanges::SwPamRanges( const SwPaM& rRing )
{
    for(SwPaM& rTmp : const_cast<SwPaM*>(&rRing)->GetRingContainer())
        Insert( rTmp.GetMark()->nNode, rTmp.GetPoint()->nNode );
}

void SwPamRanges::Insert( const SwNodeIndex& rIdx1, const SwNodeIndex& rIdx2 )
{
    SwPamRange aRg( rIdx1.GetIndex(), rIdx2.GetIndex() );
    if( aRg.nEnd < aRg.nStart )
    {   aRg.nStart = aRg.nEnd; aRg.nEnd = rIdx1.GetIndex(); }

    o3tl::sorted_vector<SwPamRange>::const_iterator it = maVector.lower_bound(aRg); //search Insert Position
    size_t nPos = it - maVector.begin();
    if (!maVector.empty() && (it != maVector.end()) && (*it) == aRg)
    {
        // is the one in the Array smaller?
        SwPamRange const& rTmp = maVector[nPos];
        if( rTmp.nEnd < aRg.nEnd )
        {
            aRg.nEnd = rTmp.nEnd;
            maVector.erase(maVector.begin() + nPos); // combine
        }
        else
            return; // done, because by precondition everything is combined
    }

    bool bEnd;
    do {
        bEnd = true;

        // combine with predecessor?
        if( nPos > 0 )
        {
            SwPamRange const& rTmp = maVector[nPos-1];
            if( rTmp.nEnd == aRg.nStart
                || rTmp.nEnd+1 == aRg.nStart )
            {
                aRg.nStart = rTmp.nStart;
                bEnd = false;
                maVector.erase( maVector.begin() + --nPos ); // combine
            }
            // range contained in rTmp?
            else if( rTmp.nStart <= aRg.nStart && aRg.nEnd <= rTmp.nEnd )
                return;
        }
        // combine with successor?
        if( nPos < maVector.size() )
        {
            SwPamRange const& rTmp = maVector[nPos];
            if( rTmp.nStart == aRg.nEnd ||
                rTmp.nStart == aRg.nEnd+1 )
            {
                aRg.nEnd = rTmp.nEnd;
                bEnd = false;
                maVector.erase( maVector.begin() + nPos ); // combine
            }

            // range contained in rTmp?
            else if( rTmp.nStart <= aRg.nStart && aRg.nEnd <= rTmp.nEnd )
                return;
        }
    } while( !bEnd );

    maVector.insert( aRg );
}

SwPaM& SwPamRanges::SetPam( size_t nArrPos, SwPaM& rPam )
{
    OSL_ASSERT( nArrPos < Count() );
    const SwPamRange& rTmp = maVector[ nArrPos ];
    rPam.GetPoint()->nNode = rTmp.nStart;
    rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), 0 );
    rPam.SetMark();
    rPam.GetPoint()->nNode = rTmp.nEnd;
    rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), 0 );
    return rPam;
}

// Rule book for outline numbering

void SwEditShell::SetOutlineNumRule(const SwNumRule& rRule)
{
    StartAllAction();       // bracketing for updating!
    GetDoc()->SetOutlineNumRule(rRule);
    EndAllAction();
}

const SwNumRule* SwEditShell::GetOutlineNumRule() const
{
    return GetDoc()->GetOutlineNumRule();
}

// Set if there is no numbering yet, else update.
// Works with old and new rules. Update only differences.

// paragraphs without numbering, with indentations
bool SwEditShell::NoNum()
{
    bool bRet = true;
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor )         // Multiple selection?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NoNum( aRangeArr.SetPam( n, aPam ));
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    else
        bRet = GetDoc()->NoNum( *pCursor );

    EndAllAction();
    return bRet;
}

bool SwEditShell::SelectionHasNumber() const
{
    bool bResult = HasNumber();
    const SwTextNode * pTextNd =
        GetCursor()->GetPoint()->nNode.GetNode().GetTextNode();
    if (!bResult && pTextNd && pTextNd->Len()==0 && !pTextNd->GetNumRule()) {
        SwPamRanges aRangeArr( *GetCursor() );
        SwPaM aPam( *GetCursor()->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            {
                sal_uInt32 nStt = aPam.GetPoint()->nNode.GetIndex(),
                      nEnd = aPam.GetMark()->nNode.GetIndex();
                if( nStt > nEnd )
                {
                    sal_uInt32 nTmp = nStt; nStt = nEnd; nEnd = nTmp;
                }
                for (sal_uInt32 nPos = nStt; nPos<=nEnd; nPos++)
                {
                    pTextNd = mpDoc->GetNodes()[nPos]->GetTextNode();
                    if (pTextNd && pTextNd->Len()!=0)
                    {
                        bResult = pTextNd->HasNumber();

                        // #b6340308# special case: outline numbered, not counted paragraph
                        if ( bResult &&
                            pTextNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
                            !pTextNd->IsCountedInList() )
                        {
                            bResult = false;
                        }
                        if (!bResult) {
                            break;
                        }
                    }
                }
            }
        }

    }

    return bResult;
}

// add a new function to determine number on/off status
bool SwEditShell::SelectionHasBullet() const
{
    bool bResult = HasBullet();
    const SwTextNode * pTextNd =
        GetCursor()->GetPoint()->nNode.GetNode().GetTextNode();
    if (!bResult && pTextNd && pTextNd->Len()==0 && !pTextNd->GetNumRule()) {
        SwPamRanges aRangeArr( *GetCursor() );
        SwPaM aPam( *GetCursor()->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            {
                sal_uInt32 nStt = aPam.GetPoint()->nNode.GetIndex(),
                      nEnd = aPam.GetMark()->nNode.GetIndex();
                if( nStt > nEnd )
                {
                    sal_uInt32 nTmp = nStt; nStt = nEnd; nEnd = nTmp;
                }
                for (sal_uInt32 nPos = nStt; nPos<=nEnd; nPos++)
                {
                    pTextNd = mpDoc->GetNodes()[nPos]->GetTextNode();
                    if (pTextNd && pTextNd->Len()!=0)
                    {
                        bResult = pTextNd->HasBullet();

                        if (!bResult) {
                            break;
                        }
                    }
                }
            }
        }
    }

    return bResult;
}

// -> #i29560#
bool SwEditShell::HasNumber() const
{
    bool bResult = false;

    const SwTextNode * pTextNd =
        GetCursor()->GetPoint()->nNode.GetNode().GetTextNode();

    if (pTextNd)
    {
        bResult = pTextNd->HasNumber();

        // special case: outline numbered, not counted paragraph
        if ( bResult &&
             pTextNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
             !pTextNd->IsCountedInList() )
        {
            bResult = false;
        }
    }

    return bResult;
}

bool SwEditShell::HasBullet() const
{
    bool bResult = false;

    const SwTextNode * pTextNd =
        GetCursor()->GetPoint()->nNode.GetNode().GetTextNode();

    if (pTextNd)
    {
        bResult = pTextNd->HasBullet();
    }

    return bResult;
}
// <- #i29560#

// delete, split list
void SwEditShell::DelNumRules()
{
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    if( pCursor->IsMultiSelection() )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
        {
            GetDoc()->DelNumRules( aRangeArr.SetPam( n, aPam ) );
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    else
        GetDoc()->DelNumRules( *pCursor );

    // Call AttrChangeNotify on the UI-side. Should actually be redundant but there was a bug once.
    CallChgLnk();

    // Cursor cannot be in front of a label anymore, because numbering/bullet is deleted.
    SetInFrontOfLabel( false );

    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
}

// up- & downgrading
bool SwEditShell::NumUpDown( bool bDown )
{
    StartAllAction();

    bool bRet = true;
    SwPaM* pCursor = GetCursor();
    if( !pCursor->IsMultiSelection() )
        bRet = GetDoc()->NumUpDown( *pCursor, bDown );
    else
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->NumUpDown( aRangeArr.SetPam( n, aPam ), bDown );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    GetDoc()->getIDocumentState().SetModified();

    // #i54693# Update marked numbering levels
    if ( IsInFrontOfLabel() )
        UpdateMarkedListLevel();

    CallChgLnk();

    EndAllAction();
    return bRet;
}

bool SwEditShell::IsFirstOfNumRuleAtCursorPos() const
{
    return SwDoc::IsFirstOfNumRuleAtPos( *GetCursor()->GetPoint() );
}

// -> #i23725#, #i90078#
void SwEditShell::ChangeIndentOfAllListLevels( const short nDiff )
{
    StartAllAction();

    const SwNumRule *pCurNumRule = GetNumRuleAtCurrCursorPos();
    if ( pCurNumRule != nullptr )
    {
        SwNumRule aRule(*pCurNumRule);
        const SwNumFormat& aRootNumFormat(aRule.Get(0));
        if( nDiff > 0 || aRootNumFormat.GetIndentAt() + nDiff > 0) // fdo#42708
        {
            // #i90078#
            aRule.ChangeIndent( nDiff );
        }
        // no start of new list
        SetCurNumRule( aRule, false );
    }

    EndAllAction();
}

// #i90078#
void SwEditShell::SetIndent(short nIndent, const SwPosition & rPos)
{
    StartAllAction();

    SwNumRule *pCurNumRule = SwDoc::GetNumRuleAtPos(rPos);

    if (pCurNumRule)
    {
        SwNumRule aRule(*pCurNumRule);
        if ( !IsMultiSelection() && IsFirstOfNumRuleAtCursorPos() )
        {
            aRule.SetIndentOfFirstListLevelAndChangeOthers( nIndent );
        }
        else
        {
            const SwTextNode* pTextNode = rPos.nNode.GetNode().GetTextNode();
            if ( pTextNode != nullptr
                 && pTextNode->GetActualListLevel() >= 0 )
            {
                aRule.SetIndent( nIndent, static_cast< sal_uInt16 >( pTextNode->GetActualListLevel() ) );
            }
        }

        // change numbering rule - changed numbering rule is not applied at <aPaM>
        SwPaM aPaM(rPos);
        GetDoc()->SetNumRule( aPaM, aRule, false, OUString(), false );
    }

    EndAllAction();
}

bool SwEditShell::MoveParagraph( long nOffset )
{
    StartAllAction();

    SwPaM *pCursor = GetCursor();
    if( !pCursor->HasMark() )
    {
        // Ensures that Bound1 and Bound2 are in the same Node
        pCursor->SetMark();
        pCursor->DeleteMark();
    }

    bool bRet = GetDoc()->MoveParagraph( *pCursor, nOffset );

    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
    return bRet;
}

int SwEditShell::GetCurrentParaOutlineLevel( ) const
{
    int nLevel = 0;

    SwPaM* pCursor = GetCursor();
    const SwTextNode* pTextNd = pCursor->GetNode().GetTextNode();
    if( pTextNd )
        nLevel = pTextNd->GetAttrOutlineLevel();
    return nLevel;
}

void SwEditShell::GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower )
{
    SwPaM* pCursor = GetCursor();
    SwPaM aCursor( *pCursor->Start() );
    aCursor.SetMark();
    if( pCursor->HasMark() )
        *aCursor.GetPoint() = *pCursor->End();
    SwDoc::GotoNextNum( *aCursor.GetPoint(), false, &rUpper, &rLower );
}

bool SwEditShell::MoveNumParas( bool bUpperLower, bool bUpperLeft )
{
    StartAllAction();

    // On all selections?
    SwPaM* pCursor = GetCursor();
    SwPaM aCursor( *pCursor->Start() );
    aCursor.SetMark();

    if( pCursor->HasMark() )
        *aCursor.GetPoint() = *pCursor->End();

    bool bRet = false;
    sal_uInt8 nUpperLevel, nLowerLevel;
    if( SwDoc::GotoNextNum( *aCursor.GetPoint(), false,
                                &nUpperLevel, &nLowerLevel ))
    {
        if( bUpperLower )
        {
            // on top of the next numbering
            long nOffset = 0;
            const SwNode* pNd;

            if( bUpperLeft ) // move up
            {
                SwPosition aPos( *aCursor.GetMark() );
                if( SwDoc::GotoPrevNum( aPos, false ) )
                    nOffset = aPos.nNode.GetIndex() -
                            aCursor.GetMark()->nNode.GetIndex();
                else
                {
                    sal_uLong nStt = aPos.nNode.GetIndex(), nIdx = nStt - 1;
                    while( nIdx && (
                        ( pNd = GetDoc()->GetNodes()[ nIdx ])->IsSectionNode() ||
                        ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode())))
                        --nIdx;
                    if( GetDoc()->GetNodes()[ nIdx ]->IsTextNode() )
                        nOffset = nIdx - nStt;
                }
            }
            else             // move down
            {
                const SwNumRule* pOrig = aCursor.GetNode(false).GetTextNode()->GetNumRule();
                if( aCursor.GetNode().IsTextNode() &&
                    pOrig == aCursor.GetNode().GetTextNode()->GetNumRule() )
                {
                    sal_uLong nStt = aCursor.GetPoint()->nNode.GetIndex(), nIdx = nStt+1;

                    while (nIdx < GetDoc()->GetNodes().Count()-1)
                    {
                        pNd = GetDoc()->GetNodes()[ nIdx ];

                        if (pNd->IsSectionNode() ||
                            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode()) ||
                            ( pNd->IsTextNode() && pOrig == static_cast<const SwTextNode*>(pNd)->GetNumRule() &&
                              static_cast<const SwTextNode*>(pNd)->GetActualListLevel() > nUpperLevel ))
                        {
                            ++nIdx;
                        }
                        // #i57856#
                        else
                        {
                            break;
                        }
                    }

                    if( nStt == nIdx || !GetDoc()->GetNodes()[ nIdx ]->IsTextNode() )
                        nOffset = 1;
                    else
                        nOffset = nIdx - nStt;
                }
                else
                    nOffset = 1;
            }

            if( nOffset )
            {
                aCursor.Move( fnMoveBackward, fnGoNode );
                bRet = GetDoc()->MoveParagraph( aCursor, nOffset );
            }
        }
        else if( (bUpperLeft ? nUpperLevel : nLowerLevel+1) < MAXLEVEL )
        {
            aCursor.Move( fnMoveBackward, fnGoNode );
            bRet = GetDoc()->NumUpDown( aCursor, !bUpperLeft );
        }
    }

    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
    return bRet;
}

bool SwEditShell::OutlineUpDown( short nOffset )
{
    StartAllAction();

    bool bRet = true;
    SwPaM* pCursor = GetCursor();
    if( !pCursor->IsMultiSelection() )
        bRet = GetDoc()->OutlineUpDown( *pCursor, nOffset );
    else
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
            bRet = bRet && GetDoc()->OutlineUpDown(
                                    aRangeArr.SetPam( n, aPam ), nOffset );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
    return bRet;
}

bool SwEditShell::MoveOutlinePara( short nOffset )
{
    StartAllAction();
    bool bRet = GetDoc()->MoveOutlinePara( *GetCursor(), nOffset );
    EndAllAction();
    return bRet;
}

// Outlines and SubOutline are ReadOnly?
bool SwEditShell::IsProtectedOutlinePara() const
{
    bool bRet = false;
    const SwNode& rNd = GetCursor()->Start()->nNode.GetNode();
    if( rNd.IsTextNode() )
    {
        const SwOutlineNodes& rOutlNd = GetDoc()->GetNodes().GetOutLineNds();
        SwNodePtr pNd = const_cast<SwNodePtr>(&rNd);
        bool bFirst = true;
        sal_uInt16 nPos;
        int nLvl(0);
        if( !rOutlNd.Seek_Entry( pNd, &nPos ) && nPos )
            --nPos;

        for( ; nPos < rOutlNd.size(); ++nPos )
        {
            SwNodePtr pTmpNd = rOutlNd[ nPos ];

            int nTmpLvl = pTmpNd->GetTextNode()->GetAttrOutlineLevel();

            OSL_ENSURE( nTmpLvl >= 0 && nTmpLvl <= MAXLEVEL,
                    "<SwEditShell::IsProtectedOutlinePara()>" );

            if( bFirst )
            {
                nLvl = nTmpLvl;
                bFirst = false;
            }
            else if( nLvl >= nTmpLvl )
                break;

            if( pTmpNd->IsProtect() )
            {
                bRet = true;
                break;
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OSL_FAIL("Cursor not on an outline node");
    }
#endif
    return bRet;
}

/** Test whether outline may be moved (bCopy == false)
 *                           or copied (bCopy == true)
 * Verify these conditions:
 * 1) outline must be within main body (and not in redline)
 * 2) outline must not be within table
 * 3) if bCopy is set, outline must not be write protected
 */
static bool lcl_IsOutlineMoveAndCopyable( const SwDoc* pDoc, sal_uInt16 nIdx, bool bCopy )
{
    const SwNodes& rNds = pDoc->GetNodes();
    const SwNode* pNd = rNds.GetOutLineNds()[ nIdx ];
    return pNd->GetIndex() >= rNds.GetEndOfExtras().GetIndex() &&   // 1) body
            !pNd->FindTableNode() &&                                // 2) table
            ( bCopy || !pNd->IsProtect() );                         // 3) write
}

bool SwEditShell::IsOutlineMovable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, false );
}

bool SwEditShell::IsOutlineCopyable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, true );
}

bool SwEditShell::NumOrNoNum(
    bool bNumOn,
    bool bChkStart )
{
    bool bRet = false;

    if ( !IsMultiSelection()
         && !HasSelection()
         && ( !bChkStart || IsSttPara() ) )
    {
        StartAllAction();
        bRet = GetDoc()->NumOrNoNum( GetCursor()->GetPoint()->nNode, !bNumOn );
        EndAllAction();
    }
    return bRet;
}

bool SwEditShell::IsNoNum( bool bChkStart ) const
{
    // a Backspace in the paragraph without number becomes a Delete
    bool bResult = false;

    if ( !IsMultiSelection()
         && !HasSelection()
         && ( !bChkStart || IsSttPara() ) )
    {
        const SwTextNode* pTextNd = GetCursor()->GetNode().GetTextNode();
        if ( pTextNd != nullptr )
        {
            bResult =  !pTextNd->IsCountedInList();
        }
    }

    return bResult;
}

sal_uInt8 SwEditShell::GetNumLevel() const
{
    // return current level where the point of the cursor is
    sal_uInt8 nLevel = MAXLEVEL;

    SwPaM* pCursor = GetCursor();
    const SwTextNode* pTextNd = pCursor->GetNode().GetTextNode();

    OSL_ENSURE( pTextNd, "GetNumLevel() without text node" );
    if ( pTextNd == nullptr )
        return nLevel;

    const SwNumRule* pRule = pTextNd->GetNumRule();
    if ( pRule != nullptr )
    {
        const int nListLevelOfTextNode( pTextNd->GetActualListLevel() );
        if ( nListLevelOfTextNode >= 0 )
        {
            nLevel = static_cast<sal_uInt8>( nListLevelOfTextNode );
        }
    }

    return nLevel;
}

const SwNumRule* SwEditShell::GetNumRuleAtCurrCursorPos() const
{
    return SwDoc::GetNumRuleAtPos( *GetCursor()->GetPoint() );
}

const SwNumRule* SwEditShell::GetNumRuleAtCurrentSelection() const
{
    const SwNumRule* pNumRuleAtCurrentSelection = nullptr;

    bool bDifferentNumRuleFound = false;
    for(const SwPaM& rCurrentCursor : GetCursor()->GetRingContainer())
    {
        const SwNodeIndex aEndNode = rCurrentCursor.End()->nNode;

        for ( SwNodeIndex aNode = rCurrentCursor.Start()->nNode; aNode <= aEndNode; ++aNode )
        {
            const SwNumRule* pNumRule = SwDoc::GetNumRuleAtPos( SwPosition( aNode ) );
            if ( pNumRule == nullptr )
            {
                continue;
            }
            else if ( pNumRule != pNumRuleAtCurrentSelection )
            {
                if ( pNumRuleAtCurrentSelection == nullptr )
                {
                    pNumRuleAtCurrentSelection = pNumRule;
                }
                else
                {
                    pNumRuleAtCurrentSelection = nullptr;
                    bDifferentNumRuleFound = true;
                    break;
                }
            }
        }
        if(bDifferentNumRuleFound)
            break;
    }

    return pNumRuleAtCurrentSelection;
}

void SwEditShell::SetCurNumRule( const SwNumRule& rRule,
                                 bool bCreateNewList,
                                 const OUString& rContinuedListId,
                                 const bool bResetIndentAttrs )
{
    StartAllAction();

    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );

    SwPaM* pCursor = GetCursor();
    if( IsMultiSelection() )
    {
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        OUString sContinuedListId(rContinuedListId);
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            OUString sListId = GetDoc()->SetNumRule( aPam, rRule,
                                  bCreateNewList, sContinuedListId,
                                  true, bResetIndentAttrs );

            //tdf#87548 On creating a new list for a multi-selection only
            //create a single new list for the multi-selection, not one per selection
            if (bCreateNewList)
            {
                sContinuedListId = sListId;
                bCreateNewList = false;
            }

            GetDoc()->SetCounted( aPam, true );
        }
    }
    else
    {
        GetDoc()->SetNumRule( *pCursor, rRule,
                              bCreateNewList, rContinuedListId,
                              true, bResetIndentAttrs );
        GetDoc()->SetCounted( *pCursor, true );
    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );

    EndAllAction();
}

OUString SwEditShell::GetUniqueNumRuleName( const OUString* pChkStr, bool bAutoNum ) const
{
    return GetDoc()->GetUniqueNumRuleName( pChkStr, bAutoNum );
}

void SwEditShell::ChgNumRuleFormats( const SwNumRule& rRule )
{
    StartAllAction();
    GetDoc()->ChgNumRuleFormats( rRule );
    EndAllAction();
}

bool SwEditShell::ReplaceNumRule( const OUString& rOldRule, const OUString& rNewRule )
{
    StartAllAction();
    bool bRet = GetDoc()->ReplaceNumRule( *GetCursor()->GetPoint(), rOldRule, rNewRule );
    EndAllAction();
    return bRet;
}

void SwEditShell::SetNumRuleStart( bool bFlag, SwPaM* pPaM )
{
    StartAllAction();
    SwPaM* pCursor = pPaM ? pPaM : GetCursor();
    if( pCursor->IsMultiSelection() )         // multiple selection ?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNumRuleStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), bFlag );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    else
        GetDoc()->SetNumRuleStart( *pCursor->GetPoint(), bFlag );

    EndAllAction();
}

bool SwEditShell::IsNumRuleStart( SwPaM* pPaM ) const
{
    SwPaM* pCursor = pPaM ? pPaM : GetCursor( );
    const SwTextNode* pTextNd = pCursor->GetNode().GetTextNode();
    return pTextNd && pTextNd->IsListRestart();
}

void SwEditShell::SetNodeNumStart( sal_uInt16 nStt, SwPaM* pPaM )
{
    StartAllAction();

    SwPaM* pCursor = pPaM ? pPaM : GetCursor();
    if( pCursor->IsMultiSelection() )         // multiple selection ?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
        SwPamRanges aRangeArr( *pCursor );
        SwPaM aPam( *pCursor->GetPoint() );
        for( size_t n = 0; n < aRangeArr.Count(); ++n )
            GetDoc()->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), nStt );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
    }
    else
        GetDoc()->SetNodeNumStart( *pCursor->GetPoint(), nStt );

    EndAllAction();
}

sal_uInt16 SwEditShell::GetNodeNumStart( SwPaM* pPaM ) const
{
    SwPaM* pCursor = pPaM ? pPaM : GetCursor();
    const SwTextNode* pTextNd = pCursor->GetNode().GetTextNode();
    // correction: check, if list restart value is set at text node and
    // use new method <SwTextNode::GetAttrListRestartValue()>.
    // return USHRT_MAX, if no list restart value is found.
    if ( pTextNd && pTextNd->HasAttrListRestartValue() )
    {
        return static_cast<sal_uInt16>(pTextNd->GetAttrListRestartValue());
    }
    return USHRT_MAX;
}

const SwNumRule * SwEditShell::SearchNumRule( const bool bForward,
                                              const bool bNum,
                                              const bool bOutline,
                                              int nNonEmptyAllowed,
                                              OUString& sListId )
{
    return GetDoc()->SearchNumRule( *(bForward ? GetCursor()->End() : GetCursor()->Start()),
                                    bForward, bNum, bOutline, nNonEmptyAllowed,
                                    sListId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
