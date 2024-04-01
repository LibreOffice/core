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

#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <acorrect.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <mdiexp.hxx>
#include <mvsave.hxx>
#include <redline.hxx>
#include <rolbck.hxx>
#include <rootfrm.hxx>
#include <splargs.hxx>
#include <swcrsr.hxx>
#include <txtfrm.hxx>
#include <unoflatpara.hxx>
#include <SwGrammarMarkUp.hxx>
#include <docedt.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <undobj.hxx>
#include <frameformats.hxx>

#include <vector>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;


void RestFlyInRange( SaveFlyArr & rArr, const SwPosition& rStartPos,
                      const SwNode* pInsertPos, bool const isForceToStartPos)
{
    SwPosition aPos(rStartPos);
    for(const SaveFly & rSave : rArr)
    {
        // create new anchor
        SwFrameFormat* pFormat = rSave.pFrameFormat;
        SwFormatAnchor aAnchor( pFormat->GetAnchor() );

        if (rSave.isAtInsertNode || isForceToStartPos)
        {
            if( pInsertPos != nullptr )
            {
                if (aAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA)
                {
                    assert(pInsertPos->GetContentNode());
                    aPos.Assign( *pInsertPos->GetContentNode(),
                        rSave.nContentIndex);
                }
                else
                {
                    assert(aAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR);
                    aPos = rStartPos;
                }
            }
            else
            {
                aPos.Assign(rStartPos.GetNode());
                assert(aPos.GetNode().GetContentNode());
            }
        }
        else
        {
            aPos.Assign(rStartPos.GetNodeIndex() + rSave.nNdDiff);
            assert(aPos.GetNode().GetContentNode());
            aPos.SetContent(
                rSave.nNdDiff == SwNodeOffset(0)
                    ? rStartPos.GetContentIndex() + rSave.nContentIndex
                    : rSave.nContentIndex);
        }

        aAnchor.SetAnchor( &aPos );
        pFormat->GetDoc()->GetSpzFrameFormats()->push_back(static_cast<sw::SpzFrameFormat*>(pFormat));
        // SetFormatAttr should call Modify() and add it to the node
        pFormat->SetFormatAttr( aAnchor );
        SwContentNode* pCNd = aPos.GetNode().GetContentNode();
        if (pCNd && pCNd->getLayoutFrame(pFormat->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr))
            pFormat->MakeFrames();
    }
    sw::CheckAnchoredFlyConsistency(rStartPos.GetNode().GetDoc());
}

void SaveFlyInRange( const SwNodeRange& rRg, SaveFlyArr& rArr )
{
    sw::SpzFrameFormats& rSpzs = *rRg.aStart.GetNode().GetDoc().GetSpzFrameFormats();
    for(sw::FrameFormats<sw::SpzFrameFormat*>::size_type n = 0; n < rSpzs.size(); ++n )
    {
        auto pSpz = rSpzs[n];
        SwFormatAnchor const*const pAnchor = &pSpz->GetAnchor();
        SwNode const*const pAnchorNode = pAnchor->GetAnchorNode();
        if (pAnchorNode &&
            ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            rRg.aStart <= *pAnchorNode && *pAnchorNode < rRg.aEnd.GetNode() )
        {
            SaveFly aSave( pAnchorNode->GetIndex() - rRg.aStart.GetIndex(),
                            (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())
                                ? pAnchor->GetAnchorContentOffset()
                                : 0,
                            pSpz, false );
            rArr.push_back( aSave );
            pSpz->DelFrames();
            // set a dummy anchor position to maintain anchoring invariants
            SwFormatAnchor aAnchor( pSpz->GetAnchor() );
            aAnchor.SetAnchor(nullptr);
            pSpz->SetFormatAttr(aAnchor);
            rSpzs.erase( rSpzs.begin() + n-- );
        }
    }
    sw::CheckAnchoredFlyConsistency(rRg.aStart.GetNode().GetDoc());
}

void SaveFlyInRange( const SwPaM& rPam, const SwPosition& rInsPos,
        SaveFlyArr& rArr, bool bMoveAllFlys, SwHistory *const pHistory)
{
    sw::SpzFrameFormats& rFormats = *rPam.GetPoint()->GetNode().GetDoc().GetSpzFrameFormats();
    sw::SpzFrameFormat* pFormat;
    const SwFormatAnchor* pAnchor;

    const SwPosition* pPos = rPam.Start();
    const SwNode& rSttNd = pPos->GetNode();

    SwPosition atParaEnd(*rPam.End());
    if (bMoveAllFlys)
    {
        assert(!rPam.End()->GetNode().IsTextNode() // can be table end-node
            || rPam.End()->GetContentIndex() == rPam.End()->GetNode().GetTextNode()->Len());
        atParaEnd.Adjust(SwNodeOffset(1));
    }

    for(sw::FrameFormats<sw::SpzFrameFormat*>::size_type n = 0; n < rFormats.size(); ++n )
    {
        pFormat = rFormats[n];
        pAnchor = &pFormat->GetAnchor();
        const SwPosition* pAPos = pAnchor->GetContentAnchor();
        const SwNodeIndex* pContentIdx;
        if (pAPos &&
            ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            // do not move if the InsPos is in the ContentArea of the Fly
            ( nullptr == ( pContentIdx = pFormat->GetContent().GetContentIdx() ) ||
              (*pContentIdx >= rInsPos.GetNode() ||
                rInsPos.GetNode() >= *pContentIdx->GetNode().EndOfSectionNode())))
        {
            bool bInsPos = false;

            if (       (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId()
                        && IsDestroyFrameAnchoredAtChar(*pAPos, *rPam.Start(), *rPam.End()))
                    || (RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()
                        && IsSelectFrameAnchoredAtPara(*pAPos, *rPam.Start(), atParaEnd,
                            bMoveAllFlys
                                ? DelContentType::CheckNoCntnt|DelContentType::AllMask
                                : DelContentType::AllMask))
                    || (RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()
                            && (bInsPos = (rInsPos.GetNode() == pAPos->GetNode())))
                    || (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId()
                            && (bInsPos = (rInsPos == *pAPos))))
            {
                if (pHistory)
                {
                    pHistory->AddChangeFlyAnchor(*pFormat);
                }
                SaveFly aSave( pAPos->GetNodeIndex() - rSttNd.GetIndex(),
                    (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())
                        ? (pAPos->GetNode() == rSttNd)
                            ? pAPos->GetContentIndex() - rPam.Start()->GetContentIndex()
                            : pAPos->GetContentIndex()
                        : 0,
                                pFormat, bInsPos );
                rArr.push_back( aSave );
                pFormat->DelFrames();
                // set a dummy anchor position to maintain anchoring invariants
                SwFormatAnchor aAnchor( pFormat->GetAnchor() );
                aAnchor.SetAnchor(nullptr);
                pFormat->SetFormatAttr(aAnchor);
                rFormats.erase( rFormats.begin() + n-- );
            }
        }
    }
    sw::CheckAnchoredFlyConsistency(rPam.GetPoint()->GetNode().GetDoc());
}

/// Delete and move all Flys at the paragraph, that are within the selection.
/// If there is a Fly at the SPoint, it is moved onto the Mark.
void DelFlyInRange( SwNode& rMkNd,
                    SwNode& rPtNd,
                    std::optional<sal_Int32> oMkContentIdx, std::optional<sal_Int32> oPtContentIdx)
{
    assert(oMkContentIdx.has_value() == oPtContentIdx.has_value());
    SwPosition const point(oPtContentIdx
                            ? SwPosition(rPtNd, rPtNd.GetContentNode(), *oPtContentIdx)
                            : SwPosition(rPtNd));
    SwPosition const mark(oPtContentIdx
                            ? SwPosition(rMkNd, rMkNd.GetContentNode(), *oMkContentIdx)
                            : SwPosition(rMkNd));
    SwPosition const& rStart = mark <= point ? mark : point;
    SwPosition const& rEnd   = mark <= point ? point : mark;

    SwDoc& rDoc = rMkNd.GetDoc();
    sw::SpzFrameFormats& rTable = *rDoc.GetSpzFrameFormats();
    for ( auto i = rTable.size(); i; )
    {
        sw::SpzFrameFormat* pFormat = rTable[--i];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        SwPosition const*const pAPos = rAnch.GetContentAnchor();
        if (pAPos &&
            (((rAnch.GetAnchorId() == RndStdIds::FLY_AT_PARA)
                && IsSelectFrameAnchoredAtPara(*pAPos, rStart, rEnd, oPtContentIdx
                    ? DelContentType::AllMask|DelContentType::WriterfilterHack
                    : DelContentType::AllMask|DelContentType::WriterfilterHack|DelContentType::CheckNoCntnt))
            || ((rAnch.GetAnchorId() == RndStdIds::FLY_AT_CHAR)
                && IsDestroyFrameAnchoredAtChar(*pAPos, rStart, rEnd, oPtContentIdx
                    ? DelContentType::AllMask|DelContentType::WriterfilterHack
                    : DelContentType::AllMask|DelContentType::WriterfilterHack|DelContentType::CheckNoCntnt))))
        {
            // If the Fly is deleted, all Flys in its content have to be deleted too.
            const SwFormatContent &rContent = pFormat->GetContent();
            // But only fly formats own their content, not draw formats.
            if (rContent.GetContentIdx() && pFormat->Which() == RES_FLYFRMFMT)
            {
                DelFlyInRange( rContent.GetContentIdx()->GetNode(),
                               *rContent.GetContentIdx()->
                                        GetNode().EndOfSectionNode() );
                // Position could have been moved!
                if (i > rTable.size())
                    i = rTable.size();
                else if (i == rTable.size() || pFormat != rTable[i])
                    i = std::distance(rTable.begin(), rTable.find(pFormat));
            }

            rDoc.getIDocumentLayoutAccess().DelLayoutFormat( pFormat );

            // DelLayoutFormat can also trigger the deletion of objects.
            if (i > rTable.size())
                i = rTable.size();
        }
    }
}

// #i59534: Redo of insertion of multiple text nodes runs into trouble
// because of unnecessary expanded redlines
// From now on this class saves the redline positions of all redlines which ends exact at the
// insert position (node _and_ content index)
SaveRedlEndPosForRestore::SaveRedlEndPosForRestore( const SwNode& rInsIdx, sal_Int32 nCnt )
    : mnSaveContent( nCnt )
{
    const SwDoc& rDest = rInsIdx.GetDoc();
    if( rDest.getIDocumentRedlineAccess().GetRedlineTable().empty() )
        return;

    SwRedlineTable::size_type nFndPos;
    const SwPosition* pEnd;
    SwPosition aSrcPos( rInsIdx, rInsIdx.GetContentNode(), nCnt );
    rDest.getIDocumentRedlineAccess().GetRedline( aSrcPos, &nFndPos );
    const SwRangeRedline* pRedl;
    while( nFndPos--
          && *( pEnd = ( pRedl = rDest.getIDocumentRedlineAccess().GetRedlineTable()[ nFndPos ] )->End() ) == aSrcPos
          && *pRedl->Start() < aSrcPos )
    {
        if( !moSaveIndex )
        {
            moSaveIndex.emplace( rInsIdx, -1 );
        }
        mvSavArr.push_back( const_cast<SwPosition*>(pEnd) );
    }
}

SaveRedlEndPosForRestore::~SaveRedlEndPosForRestore()
{
    moSaveIndex.reset();
}

void SaveRedlEndPosForRestore::Restore()
{
    if (mvSavArr.empty())
        return;
    ++(*moSaveIndex);
    SwContentNode* pNode = moSaveIndex->GetNode().GetContentNode();
    // If there's no content node at the remembered position, we will not restore the old position
    // This may happen if a table (or section?) will be inserted.
    if( pNode )
    {
        SwPosition aPos( *moSaveIndex, pNode, mnSaveContent );
        for( auto n = mvSavArr.size(); n; )
            *mvSavArr[ --n ] = aPos;
    }
}

/// Convert list of ranges of whichIds to a corresponding list of whichIds
static std::vector<sal_uInt16> lcl_RangesToVector(const WhichRangesContainer& pRanges)
{
    std::vector<sal_uInt16> aResult;

    for(const WhichPair& rPair : pRanges)
    {
        for (sal_uInt16 j = rPair.first; j <= rPair.second; j++)
            aResult.push_back(j);
    }

    return aResult;
}

void sw_GetJoinFlags( SwPaM& rPam, bool& rJoinText, bool& rJoinPrev )
{
    rJoinText = false;
    rJoinPrev = false;
    if( rPam.GetPoint()->GetNode() == rPam.GetMark()->GetNode() )
        return;

    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*
    SwTextNode *pSttNd = pStt->GetNode().GetTextNode();
    if( !pSttNd )
        return;

    SwTextNode *pEndNd = pEnd->GetNode().GetTextNode();
    rJoinText = nullptr != pEndNd;
    if( !rJoinText )
        return;

    bool bExchange = pStt == rPam.GetPoint();
    if( !pStt->GetContentIndex() &&
        pEndNd->GetText().getLength() != pEnd->GetContentIndex())
        bExchange = !bExchange;
    if( bExchange )
        rPam.Exchange();
    rJoinPrev = rPam.GetPoint() == pStt;
    OSL_ENSURE( !pStt->GetContentIndex() &&
        pEndNd->GetText().getLength() != pEnd->GetContentIndex()
        ? (rPam.GetPoint()->GetNode() < rPam.GetMark()->GetNode())
        : (rPam.GetPoint()->GetNode() > rPam.GetMark()->GetNode()),
        "sw_GetJoinFlags");
}

bool sw_JoinText( SwPaM& rPam, bool bJoinPrev )
{
    SwNodeIndex aIdx( rPam.GetPoint()->GetNode() );
    SwTextNode *pTextNd = aIdx.GetNode().GetTextNode();
    SwNodeIndex aOldIdx( aIdx );
    SwTextNode *pOldTextNd = pTextNd;

    if( pTextNd && pTextNd->CanJoinNext( &aIdx ) )
    {
        SwDoc& rDoc = rPam.GetDoc();
        if( bJoinPrev )
        {
            // We do not need to handle xmlids in this case, because
            // it is only invoked if one paragraph is/becomes completely empty
            // (see sw_GetJoinFlags)
            {
                // If PageBreaks are deleted/set, it must not be added to the Undo history!
                // Also, deleting the Node is not added to the Undo history!
                ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

                /* PageBreaks, PageDesc, ColumnBreaks */
                // If we need to change something about the logic to copy the PageBreaks,
                // PageDesc, etc. we also have to change SwUndoDelete.
                // There, we copy the AUTO PageBreak from the GetMarkNode!

                /* The MarkNode */
                pTextNd = aIdx.GetNode().GetTextNode();
                if (pTextNd->HasSwAttrSet())
                {
                    if( SfxItemState::SET == pTextNd->GetpSwAttrSet()->GetItemState( RES_BREAK, false) )
                        pTextNd->ResetAttr( RES_BREAK );
                    if( pTextNd->HasSwAttrSet() &&
                        SfxItemState::SET == pTextNd->GetpSwAttrSet()->GetItemState( RES_PAGEDESC, false ) )
                        pTextNd->ResetAttr( RES_PAGEDESC );
                }

                /* The PointNode */
                if( pOldTextNd->HasSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet( rDoc.GetAttrPool(), aBreakSetRange );
                    const SfxItemSet* pSet = pOldTextNd->GetpSwAttrSet();
                    if( SfxItemState::SET == pSet->GetItemState( RES_BREAK,
                        false, &pItem ) )
                        aSet.Put( *pItem );
                    if( SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC,
                        false, &pItem ) )
                        aSet.Put( *pItem );
                    if( aSet.Count() )
                        pTextNd->SetAttr( aSet );
                }
                pOldTextNd->FormatToTextAttr( pTextNd );

                const std::shared_ptr< sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
                pContentStore->Save(rDoc, aOldIdx.GetIndex(), SAL_MAX_INT32);

                SwContentIndex aAlphaIdx(pTextNd);
                pOldTextNd->CutText( pTextNd, aAlphaIdx, SwContentIndex(pOldTextNd),
                                    pOldTextNd->Len() );
                SwPosition aAlphaPos( aIdx, aAlphaIdx );
                rDoc.CorrRel( rPam.GetPoint()->GetNode(), aAlphaPos, 0, true );

                // move all Bookmarks/TOXMarks
                if( !pContentStore->Empty() )
                    pContentStore->Restore( rDoc, aIdx.GetIndex() );

                // If the passed PaM is not in the Cursor ring,
                // treat it separately (e.g. when it's being called from AutoFormat)
                if( pOldTextNd == rPam.GetBound().GetContentNode() )
                    rPam.GetBound() = aAlphaPos;
                if( pOldTextNd == rPam.GetBound( false ).GetContentNode() )
                    rPam.GetBound( false ) = aAlphaPos;
            }
            // delete the Node, at last!
            SwNode::Merge const eOldMergeFlag(pOldTextNd->GetRedlineMergeFlag());
            if (eOldMergeFlag == SwNode::Merge::First
                && !pTextNd->IsCreateFrameWhenHidingRedlines())
            {
                sw::MoveDeletedPrevFrames(*pOldTextNd, *pTextNd);
            }
            rDoc.GetNodes().Delete( aOldIdx );
            sw::CheckResetRedlineMergeFlag(*pTextNd,
                    eOldMergeFlag == SwNode::Merge::NonFirst
                        ? sw::Recreate::Predecessor
                        : sw::Recreate::No);
        }
        else
        {
            SwTextNode* pDelNd = aIdx.GetNode().GetTextNode();
            if( pTextNd->Len() )
                pDelNd->FormatToTextAttr( pTextNd );
            else
            {
                /* This case was missed:

                   <something></something>   <-- pTextNd
                   <other>ccc</other>        <-- pDelNd

                   <something> and <other> are paragraph
                   attributes. The attribute <something> stayed if not
                   overwritten by an attribute in "ccc". Fixed by
                   first resetting all character attributes in first
                   paragraph (pTextNd).
                */
                std::vector<sal_uInt16> aShorts =
                    lcl_RangesToVector(aCharFormatSetRange);
                pTextNd->ResetAttr(aShorts);

                if( pDelNd->HasSwAttrSet() )
                {
                    // only copy the character attributes
                    SfxItemSet aTmpSet( rDoc.GetAttrPool(), aCharFormatSetRange );
                    aTmpSet.Put( *pDelNd->GetpSwAttrSet() );
                    pTextNd->SetAttr( aTmpSet );
                }
            }

            rDoc.CorrRel( aIdx.GetNode(), *rPam.GetPoint(), 0, true );
            // #i100466# adjust given <rPam>, if it does not belong to the cursors
            if ( pDelNd == rPam.GetBound().GetContentNode() )
            {
                rPam.GetBound().Assign( *pTextNd );
            }
            if( pDelNd == rPam.GetBound( false ).GetContentNode() )
            {
                rPam.GetBound( false ).Assign( *pTextNd );
            }
            pTextNd->JoinNext();
        }
        return true;
    }
    else return false;
}

static void lcl_syncGrammarError( SwTextNode &rTextNode, linguistic2::ProofreadingResult& rResult,
    const ModelToViewHelper &rConversionMap )
{
    if( rTextNode.IsGrammarCheckDirty() )
        return;
    SwGrammarMarkUp* pWrong = rTextNode.GetGrammarCheck();
    linguistic2::SingleProofreadingError* pArray = rResult.aErrors.getArray();
    sal_uInt16 j = 0;
    if( pWrong )
    {
        for( sal_Int32 i = 0; i < rResult.aErrors.getLength(); ++i )
        {
            const linguistic2::SingleProofreadingError &rError = rResult.aErrors[i];
            const sal_Int32 nStart = rConversionMap.ConvertToModelPosition( rError.nErrorStart ).mnPos;
            const sal_Int32 nEnd = rConversionMap.ConvertToModelPosition( rError.nErrorStart + rError.nErrorLength ).mnPos;
            if( i != j )
                pArray[j] = pArray[i];
            if( pWrong->LookForEntry( nStart, nEnd ) )
                ++j;
        }
    }
    if( rResult.aErrors.getLength() > j )
        rResult.aErrors.realloc( j );
}

uno::Any SwDoc::Spell( SwPaM& rPaM,
                    uno::Reference< XSpellChecker1 > const &xSpeller,
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
                    bool bGrammarCheck,
                    SwRootFrame const*const pLayout,
                    SwConversionArgs *pConvArgs  ) const
{
    SwPosition* const pSttPos = rPaM.Start();
    SwPosition* const pEndPos = rPaM.End();

    std::unique_ptr<SwSpellArgs> pSpellArgs;
    if (pConvArgs)
    {
        pConvArgs->SetStart(*pSttPos);
        pConvArgs->SetEnd(*pEndPos);
    }
    else
        pSpellArgs.reset(new SwSpellArgs( xSpeller, *pSttPos, *pEndPos, bGrammarCheck ));

    SwNodeOffset nCurrNd = pSttPos->GetNodeIndex();
    SwNodeOffset nEndNd = pEndPos->GetNodeIndex();

    uno::Any aRet;
    if( nCurrNd <= nEndNd )
    {
        SwContentFrame* pContentFrame;
        bool bGoOn = true;
        while( bGoOn )
        {
            SwNode* pNd = GetNodes()[ nCurrNd ];
            switch( pNd->GetNodeType() )
            {
            case SwNodeType::Text:
                if( nullptr != ( pContentFrame = pNd->GetTextNode()->getLayoutFrame( getIDocumentLayoutAccess().GetCurrentLayout() )) )
                {
                    // skip protected and hidden Cells and Flys
                    if( pContentFrame->IsProtected() )
                    {
                        nCurrNd = pNd->EndOfSectionIndex();
                    }
                    else if( !pContentFrame->IsHiddenNow() )
                    {
                        if( pPageCnt && *pPageCnt && pPageSt )
                        {
                            sal_uInt16 nPageNr = pContentFrame->GetPhyPageNum();
                            if( !*pPageSt )
                            {
                                *pPageSt = nPageNr;
                                if( *pPageCnt < *pPageSt )
                                    *pPageCnt = *pPageSt;
                            }
                            tools::Long nStat;
                            if( nPageNr >= *pPageSt )
                                nStat = nPageNr - *pPageSt + 1;
                            else
                                nStat = nPageNr + *pPageCnt - *pPageSt + 1;
                            ::SetProgressState( nStat, GetDocShell() );
                        }
                        //Spell() changes the pSpellArgs in case an error is found
                        sal_Int32 nBeginGrammarCheck = 0;
                        sal_Int32 nEndGrammarCheck = 0;
                        if( pSpellArgs && pSpellArgs->bIsGrammarCheck)
                        {
                            nBeginGrammarCheck = &pSpellArgs->pStartPos->GetNode() == pNd ?  pSpellArgs->pStartPos->GetContentIndex() : 0;
                            // if grammar checking starts inside of a sentence the start position has to be adjusted
                            if( nBeginGrammarCheck )
                            {
                                SwContentIndex aStartIndex( pNd->GetTextNode(), nBeginGrammarCheck );
                                SwPosition aStart( *pNd, aStartIndex );
                                SwCursor aCursor(aStart, nullptr);
                                SwPosition aOrigPos = *aCursor.GetPoint();
                                aCursor.GoSentence( SwCursor::START_SENT );
                                if( aOrigPos != *aCursor.GetPoint() )
                                {
                                    nBeginGrammarCheck = aCursor.GetPoint()->GetContentIndex();
                                }
                            }
                            nEndGrammarCheck = (&pSpellArgs->pEndPos->GetNode() == pNd)
                                ? pSpellArgs->pEndPos->GetContentIndex()
                                : pNd->GetTextNode()
                                    ->GetText().getLength();
                        }

                        sal_Int32 nSpellErrorPosition = pNd->GetTextNode()->GetText().getLength();
                        if( (!pConvArgs && pNd->GetTextNode()->Spell( pSpellArgs.get() )) ||
                            ( pConvArgs && pNd->GetTextNode()->Convert( *pConvArgs )))
                        {
                            // Cancel and remember position
                            if( pSpellArgs )
                                nSpellErrorPosition = pSpellArgs->pStartPos->GetContentIndex() > pSpellArgs->pEndPos->GetContentIndex() ?
                                            pSpellArgs->pEndPos->GetContentIndex() :
                                            pSpellArgs->pStartPos->GetContentIndex();
                            if( nCurrNd != nEndNd )
                            {
                                pSttPos->Assign(nCurrNd, pSttPos->GetContentIndex());
                                pEndPos->Assign(nCurrNd, pEndPos->GetContentIndex());
                                nCurrNd = nEndNd;
                            }
                        }

                        if( pSpellArgs && pSpellArgs->bIsGrammarCheck )
                        {
                            uno::Reference< linguistic2::XProofreadingIterator >  xGCIterator( GetGCIterator() );
                            if (xGCIterator.is())
                            {
                                uno::Reference< lang::XComponent > xDoc = GetDocShell()->GetBaseModel();
                                // Expand the string:
                                const ModelToViewHelper aConversionMap(*pNd->GetTextNode(), pLayout);
                                const OUString& aExpandText = aConversionMap.getViewText();

                                // get XFlatParagraph to use...
                                uno::Reference< text::XFlatParagraph > xFlatPara = new SwXFlatParagraph( *pNd->GetTextNode(), aExpandText, aConversionMap );

                                // get error position of cursor in XFlatParagraph
                                linguistic2::ProofreadingResult aResult;
                                bool bGrammarErrors;
                                do
                                {
                                    aConversionMap.ConvertToViewPosition( nBeginGrammarCheck );
                                    aResult = xGCIterator->checkSentenceAtPosition(
                                            xDoc, xFlatPara, aExpandText, lang::Locale(), nBeginGrammarCheck, -1, -1 );

                                    lcl_syncGrammarError( *pNd->GetTextNode(), aResult, aConversionMap );

                                    // get suggestions to use for the specific error position
                                    bGrammarErrors = aResult.aErrors.hasElements();
                                    // if grammar checking doesn't have any progress then quit
                                    if( aResult.nStartOfNextSentencePosition <= nBeginGrammarCheck )
                                        break;
                                    // prepare next iteration
                                    nBeginGrammarCheck = aResult.nStartOfNextSentencePosition;
                                }
                                while( nSpellErrorPosition > aResult.nBehindEndOfSentencePosition && !bGrammarErrors && aResult.nBehindEndOfSentencePosition < nEndGrammarCheck );

                                if( bGrammarErrors && nSpellErrorPosition >= aResult.nBehindEndOfSentencePosition )
                                {
                                    aRet <<= aResult;
                                    //put the cursor to the current error
                                    const linguistic2::SingleProofreadingError &rError = aResult.aErrors[0];
                                    pSttPos->Assign(nCurrNd, pSttPos->GetContentIndex());
                                    pEndPos->Assign(nCurrNd, pEndPos->GetContentIndex());
                                    pSpellArgs->pStartPos->Assign(*pNd->GetTextNode(), aConversionMap.ConvertToModelPosition( rError.nErrorStart ).mnPos );
                                    pSpellArgs->pEndPos->Assign(*pNd->GetTextNode(), aConversionMap.ConvertToModelPosition( rError.nErrorStart + rError.nErrorLength ).mnPos );
                                    nCurrNd = nEndNd;
                                }
                            }
                        }
                    }
                }
                break;
            case SwNodeType::Section:
                if( static_cast<SwSectionNode*>(pNd)->GetSection().IsProtect() ||
                    static_cast<SwSectionNode*>(pNd)->GetSection().IsHidden() )
                    nCurrNd = pNd->EndOfSectionIndex();
                break;
            case SwNodeType::End:
                {
                    break;
                }
            default: break;
            }

            bGoOn = nCurrNd < nEndNd;
            ++nCurrNd;
        }
    }

    if( !aRet.hasValue() )
    {
        if (pConvArgs)
            aRet <<= pConvArgs->aConvText;
        else
            aRet <<= pSpellArgs->xSpellAlt;
    }

    return aRet;
}

namespace {

class SwHyphArgs : public SwInterHyphInfo
{
    SwNodeIndex m_aNodeIdx;
    const SwNode *m_pStart;
    const SwNode *m_pEnd;
    sal_uInt16 *m_pPageCnt;
    sal_uInt16 *m_pPageSt;

    sal_Int32 m_nPamStart;
    sal_Int32 m_nPamLen;

public:
    SwHyphArgs( const SwPaM *pPam, const Point &rPoint,
                sal_uInt16* pPageCount, sal_uInt16* pPageStart );
    void SetPam( SwPaM *pPam ) const;
    void SetNode( SwNode& rNew ) { m_aNodeIdx.Assign(rNew); }
    inline void SetRange( const SwNode *pNew );
    void NextNode() { ++m_aNodeIdx; }
    sal_uInt16 *GetPageCnt() { return m_pPageCnt; }
    sal_uInt16 *GetPageSt() { return m_pPageSt; }
};

}

SwHyphArgs::SwHyphArgs( const SwPaM *pPam, const Point &rCursorPos,
                         sal_uInt16* pPageCount, sal_uInt16* pPageStart )
     : SwInterHyphInfo( rCursorPos ), m_aNodeIdx(pPam->GetPoint()->GetNode()),
     m_pPageCnt( pPageCount ), m_pPageSt( pPageStart )
{
    // The following constraints have to be met:
    // 1) there is at least one Selection
    // 2) SPoint() == Start()
    OSL_ENSURE( pPam->HasMark(), "SwDoc::Hyphenate: blowing in the wind");
    OSL_ENSURE( *pPam->GetPoint() <= *pPam->GetMark(),
            "SwDoc::Hyphenate: New York, New York");

    const SwPosition *pPoint = pPam->GetPoint();

    // Set start
    m_pStart = pPoint->GetNode().GetTextNode();
    m_nPamStart = pPoint->GetContentIndex();

    // Set End and Length
    const SwPosition *pMark = pPam->GetMark();
    m_pEnd = pMark->GetNode().GetTextNode();
    m_nPamLen = pMark->GetContentIndex();
    if( pPoint->GetNode() == pMark->GetNode() )
        m_nPamLen = m_nPamLen - pPoint->GetContentIndex();
}

inline void SwHyphArgs::SetRange( const SwNode *pNew )
{
    m_nStart = m_pStart == pNew ? m_nPamStart : 0;
    m_nEnd   = m_pEnd   == pNew ? m_nPamStart + m_nPamLen : SAL_MAX_INT32;
}

void SwHyphArgs::SetPam( SwPaM *pPam ) const
{
    pPam->GetPoint()->Assign( m_aNodeIdx, m_nWordStart );
    pPam->GetMark()->Assign( m_aNodeIdx, m_nWordStart + m_nWordLen );
}

// Returns true if we can proceed.
static bool lcl_HyphenateNode( SwNode* pNd, void* pArgs )
{
    // Hyphenate returns true if there is a hyphenation point and sets pPam
    SwTextNode *pNode = pNd->GetTextNode();
    SwHyphArgs *pHyphArgs = static_cast<SwHyphArgs*>(pArgs);
    if( pNode )
    {
        // sw_redlinehide: this will be called once per node for merged nodes;
        // the fully deleted ones won't have frames so are skipped.
        SwContentFrame* pContentFrame = pNode->getLayoutFrame( pNode->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout() );
        if( pContentFrame && !pContentFrame->IsHiddenNow() )
        {
            sal_uInt16 *pPageSt = pHyphArgs->GetPageSt();
            sal_uInt16 *pPageCnt = pHyphArgs->GetPageCnt();
            if( pPageCnt && *pPageCnt && pPageSt )
            {
                sal_uInt16 nPageNr = pContentFrame->GetPhyPageNum();
                if( !*pPageSt )
                {
                    *pPageSt = nPageNr;
                    if( *pPageCnt < *pPageSt )
                        *pPageCnt = *pPageSt;
                }
                tools::Long nStat = nPageNr >= *pPageSt ? nPageNr - *pPageSt + 1
                                         : nPageNr + *pPageCnt - *pPageSt + 1;
                ::SetProgressState( nStat, pNode->GetDoc().GetDocShell() );
            }
            pHyphArgs->SetRange( pNd );
            if( pNode->Hyphenate( *pHyphArgs ) )
            {
                pHyphArgs->SetNode( *pNd );
                return false;
            }
        }
    }
    pHyphArgs->NextNode();
    return true;
}

uno::Reference< XHyphenatedWord >  SwDoc::Hyphenate(
                            SwPaM *pPam, const Point &rCursorPos,
                             sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    OSL_ENSURE(this == &pPam->GetDoc(), "SwDoc::Hyphenate: strangers in the night");

    if( *pPam->GetPoint() > *pPam->GetMark() )
        pPam->Exchange();

    SwHyphArgs aHyphArg( pPam, rCursorPos, pPageCnt, pPageSt );
    SwNodeIndex aTmpIdx( pPam->GetMark()->GetNode(), 1 );
    GetNodes().ForEach( pPam->GetPoint()->GetNode(), aTmpIdx.GetNode(),
                    lcl_HyphenateNode, &aHyphArg );
    aHyphArg.SetPam( pPam );
    return aHyphArg.GetHyphWord();  // will be set by lcl_HyphenateNode
}

// Save the current values to add them as automatic entries to AutoCorrect.
void SwDoc::SetAutoCorrExceptWord( std::unique_ptr<SwAutoCorrExceptWord> pNew )
{
    mpACEWord = std::move(pNew);
}

void SwDoc::DeleteAutoCorrExceptWord()
{
    mpACEWord.reset();
}

void SwDoc::CountWords( const SwPaM& rPaM, SwDocStat& rStat )
{
    // This is a modified version of SwDoc::TransliterateText
    auto [pStt, pEnd] = rPaM.StartEnd(); // SwPosition*

    const SwNodeOffset nSttNd = pStt->GetNodeIndex();
    const SwNodeOffset nEndNd = pEnd->GetNodeIndex();

    const sal_Int32 nSttCnt = pStt->GetContentIndex();
    const sal_Int32 nEndCnt = pEnd->GetContentIndex();

    const SwTextNode* pTNd = pStt->GetNode().GetTextNode();
    if( pStt == pEnd && pTNd )                  // no region ?
    {
        // do nothing
        return;
    }

    if( nSttNd != nEndNd )
    {
        SwNodeIndex aIdx( pStt->GetNode() );
        if( nSttCnt )
        {
            ++aIdx;
            if( pTNd )
                pTNd->CountWords( rStat, nSttCnt, pTNd->GetText().getLength() );
        }

        for( ; aIdx.GetIndex() < nEndNd; ++aIdx )
            if( nullptr != ( pTNd = aIdx.GetNode().GetTextNode() ))
                pTNd->CountWords( rStat, 0, pTNd->GetText().getLength() );

        if( nEndCnt && nullptr != ( pTNd = pEnd->GetNode().GetTextNode() ))
            pTNd->CountWords( rStat, 0, nEndCnt );
    }
    else if( pTNd && nSttCnt < nEndCnt )
        pTNd->CountWords( rStat, nSttCnt, nEndCnt );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
