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
#include <com/sun/star/frame/XModel.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;


void RestFlyInRange( SaveFlyArr & rArr, const SwPosition& rStartPos,
                      const SwNodeIndex* pInsertPos )
{
    SwPosition aPos(rStartPos);
    for(const SaveFly & rSave : rArr)
    {
        // create new anchor
        SwFrameFormat* pFormat = rSave.pFrameFormat;
        SwFormatAnchor aAnchor( pFormat->GetAnchor() );

        if (rSave.isAtInsertNode)
        {
            if( pInsertPos != nullptr )
            {
                if (aAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA)
                {
                    aPos.nNode = *pInsertPos;
                    aPos.nContent.Assign(dynamic_cast<SwIndexReg*>(&aPos.nNode.GetNode()),
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
                aPos.nNode = rStartPos.nNode;
                aPos.nContent.Assign(dynamic_cast<SwIndexReg*>(&aPos.nNode.GetNode()), 0);
            }
        }
        else
        {
            aPos.nNode = rStartPos.nNode.GetIndex() + rSave.nNdDiff;
            aPos.nContent.Assign(dynamic_cast<SwIndexReg*>(&aPos.nNode.GetNode()),
                rSave.nNdDiff == 0
                    ? rStartPos.nContent.GetIndex() + rSave.nContentIndex
                    : rSave.nContentIndex);
        }

        aAnchor.SetAnchor( &aPos );
        pFormat->GetDoc()->GetSpzFrameFormats()->push_back( pFormat );
        // SetFormatAttr should call Modify() and add it to the node
        pFormat->SetFormatAttr( aAnchor );
        SwContentNode* pCNd = aPos.nNode.GetNode().GetContentNode();
        if (pCNd && pCNd->getLayoutFrame(pFormat->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, nullptr))
            pFormat->MakeFrames();
    }
    sw::CheckAnchoredFlyConsistency(rStartPos.nNode.GetNode().GetDoc());
}

void SaveFlyInRange( const SwNodeRange& rRg, SaveFlyArr& rArr )
{
    SwFrameFormats& rFormats = *rRg.aStart.GetNode().GetDoc().GetSpzFrameFormats();
    for( SwFrameFormats::size_type n = 0; n < rFormats.size(); ++n )
    {
        SwFrameFormat *const pFormat = rFormats[n];
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();
        if (pAPos &&
            ((RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            rRg.aStart <= pAPos->nNode && pAPos->nNode < rRg.aEnd )
        {
            SaveFly aSave( pAPos->nNode.GetIndex() - rRg.aStart.GetIndex(),
                            (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())
                                ? pAPos->nContent.GetIndex()
                                : 0,
                            pFormat, false );
            rArr.push_back( aSave );
            pFormat->DelFrames();
            // set a dummy anchor position to maintain anchoring invariants
            SwFormatAnchor aAnchor( pFormat->GetAnchor() );
            aAnchor.SetAnchor(nullptr);
            pFormat->SetFormatAttr(aAnchor);
            rFormats.erase( rFormats.begin() + n-- );
        }
    }
    sw::CheckAnchoredFlyConsistency(rRg.aStart.GetNode().GetDoc());
}

void SaveFlyInRange( const SwPaM& rPam, const SwPosition& rInsPos,
                       SaveFlyArr& rArr, bool bMoveAllFlys )
{
    SwFrameFormats& rFormats = *rPam.GetPoint()->nNode.GetNode().GetDoc().GetSpzFrameFormats();
    SwFrameFormat* pFormat;
    const SwFormatAnchor* pAnchor;

    const SwPosition* pPos = rPam.Start();
    const SwNodeIndex& rSttNdIdx = pPos->nNode;

    SwPosition atParaEnd(*rPam.End());
    if (bMoveAllFlys)
    {
        assert(!rPam.End()->nNode.GetNode().IsTextNode() // can be table end-node
            || rPam.End()->nContent.GetIndex() == rPam.End()->nNode.GetNode().GetTextNode()->Len());
        ++atParaEnd.nNode;
        atParaEnd.nContent.Assign(atParaEnd.nNode.GetNode().GetContentNode(), 0);
    }

    for( SwFrameFormats::size_type n = 0; n < rFormats.size(); ++n )
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
              (*pContentIdx >= rInsPos.nNode ||
                rInsPos.nNode >= pContentIdx->GetNode().EndOfSectionIndex())))
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
                            && (bInsPos = (rInsPos.nNode == pAPos->nNode)))
                    || (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId()
                            && (bInsPos = (rInsPos == *pAPos))))
            {
                SaveFly aSave( pAPos->nNode.GetIndex() - rSttNdIdx.GetIndex(),
                    (RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId())
                        ? (pAPos->nNode == rSttNdIdx)
                            ? pAPos->nContent.GetIndex() - rPam.Start()->nContent.GetIndex()
                            : pAPos->nContent.GetIndex()
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
    sw::CheckAnchoredFlyConsistency(rPam.GetPoint()->nNode.GetNode().GetDoc());
}

/// Delete and move all Flys at the paragraph, that are within the selection.
/// If there is a Fly at the SPoint, it is moved onto the Mark.
void DelFlyInRange( const SwNodeIndex& rMkNdIdx,
                    const SwNodeIndex& rPtNdIdx,
                    SwIndex const*const pMkIdx, SwIndex const*const pPtIdx)
{
    assert((pMkIdx == nullptr) == (pPtIdx == nullptr));
    SwPosition const point(pPtIdx
                            ? SwPosition(rPtNdIdx, *pPtIdx)
                            : SwPosition(rPtNdIdx));
    SwPosition const mark(pPtIdx
                            ? SwPosition(rMkNdIdx, *pMkIdx)
                            : SwPosition(rMkNdIdx));
    SwPosition const& rStart = mark <= point ? mark : point;
    SwPosition const& rEnd   = mark <= point ? point : mark;

    SwDoc& rDoc = rMkNdIdx.GetNode().GetDoc();
    SwFrameFormats& rTable = *rDoc.GetSpzFrameFormats();
    for ( auto i = rTable.size(); i; )
    {
        SwFrameFormat *pFormat = rTable[--i];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        SwPosition const*const pAPos = rAnch.GetContentAnchor();
        if (pAPos &&
            (((rAnch.GetAnchorId() == RndStdIds::FLY_AT_PARA)
                && IsSelectFrameAnchoredAtPara(*pAPos, rStart, rEnd, pPtIdx
                    ? DelContentType::AllMask|DelContentType::WriterfilterHack
                    : DelContentType::AllMask|DelContentType::WriterfilterHack|DelContentType::CheckNoCntnt))
            || ((rAnch.GetAnchorId() == RndStdIds::FLY_AT_CHAR)
                && IsDestroyFrameAnchoredAtChar(*pAPos, rStart, rEnd, pPtIdx
                    ? DelContentType::AllMask|DelContentType::WriterfilterHack
                    : DelContentType::AllMask|DelContentType::WriterfilterHack|DelContentType::CheckNoCntnt))))
        {
            // If the Fly is deleted, all Flys in its content have to be deleted too.
            const SwFormatContent &rContent = pFormat->GetContent();
            // But only fly formats own their content, not draw formats.
            if (rContent.GetContentIdx() && pFormat->Which() == RES_FLYFRMFMT)
            {
                DelFlyInRange( *rContent.GetContentIdx(),
                                SwNodeIndex( *rContent.GetContentIdx()->
                                        GetNode().EndOfSectionNode() ));
                // Position could have been moved!
                if (i > rTable.size())
                    i = rTable.size();
                else if (pFormat != rTable[i])
                    i = std::distance(rTable.begin(), rTable.find( pFormat ));
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
SaveRedlEndPosForRestore::SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx, sal_Int32 nCnt )
    : mnSaveContent( nCnt )
{
    SwNode& rNd = rInsIdx.GetNode();
    SwDoc& rDest = rNd.GetDoc();
    if( rDest.getIDocumentRedlineAccess().GetRedlineTable().empty() )
        return;

    SwRedlineTable::size_type nFndPos;
    const SwPosition* pEnd;
    SwPosition aSrcPos( rInsIdx, SwIndex( rNd.GetContentNode(), nCnt ));
    rDest.getIDocumentRedlineAccess().GetRedline( aSrcPos, &nFndPos );
    const SwRangeRedline* pRedl;
    while( nFndPos--
          && *( pEnd = ( pRedl = rDest.getIDocumentRedlineAccess().GetRedlineTable()[ nFndPos ] )->End() ) == aSrcPos
          && *pRedl->Start() < aSrcPos )
    {
        if( !mpSaveIndex )
        {
            mpSaveIndex.reset(new SwNodeIndex( rInsIdx, -1 ));
        }
        mvSavArr.push_back( const_cast<SwPosition*>(pEnd) );
    }
}

SaveRedlEndPosForRestore::~SaveRedlEndPosForRestore()
{
    mpSaveIndex.reset();
}

void SaveRedlEndPosForRestore::Restore()
{
    if (mvSavArr.empty())
        return;
    ++(*mpSaveIndex);
    SwContentNode* pNode = mpSaveIndex->GetNode().GetContentNode();
    // If there's no content node at the remembered position, we will not restore the old position
    // This may happen if a table (or section?) will be inserted.
    if( pNode )
    {
        SwPosition aPos( *mpSaveIndex, SwIndex( pNode, mnSaveContent ));
        for( auto n = mvSavArr.size(); n; )
            *mvSavArr[ --n ] = aPos;
    }
}

/// Convert list of ranges of whichIds to a corresponding list of whichIds
static std::vector<sal_uInt16> lcl_RangesToVector(const sal_uInt16 * pRanges)
{
    std::vector<sal_uInt16> aResult;

    int i = 0;
    while (pRanges[i] != 0)
    {
        OSL_ENSURE(pRanges[i+1] != 0, "malformed ranges");

        for (sal_uInt16 j = pRanges[i]; j <= pRanges[i+1]; j++)
            aResult.push_back(j);

        i += 2;
    }

    return aResult;
}

void sw_GetJoinFlags( SwPaM& rPam, bool& rJoinText, bool& rJoinPrev )
{
    rJoinText = false;
    rJoinPrev = false;
    if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
        return;

    const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
    SwTextNode *pSttNd = pStt->nNode.GetNode().GetTextNode();
    if( !pSttNd )
        return;

    SwTextNode *pEndNd = pEnd->nNode.GetNode().GetTextNode();
    rJoinText = nullptr != pEndNd;
    if( !rJoinText )
        return;

    bool bExchange = pStt == rPam.GetPoint();
    if( !pStt->nContent.GetIndex() &&
        pEndNd->GetText().getLength() != pEnd->nContent.GetIndex())
        bExchange = !bExchange;
    if( bExchange )
        rPam.Exchange();
    rJoinPrev = rPam.GetPoint() == pStt;
    OSL_ENSURE( !pStt->nContent.GetIndex() &&
        pEndNd->GetText().getLength() != pEnd->nContent.GetIndex()
        ? (rPam.GetPoint()->nNode < rPam.GetMark()->nNode)
        : (rPam.GetPoint()->nNode > rPam.GetMark()->nNode),
        "sw_GetJoinFlags");
}

bool sw_JoinText( SwPaM& rPam, bool bJoinPrev )
{
    SwNodeIndex aIdx( rPam.GetPoint()->nNode );
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
                    const SfxPoolItem* pItem;
                    if( SfxItemState::SET == pTextNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, false, &pItem ) )
                        pTextNd->ResetAttr( RES_BREAK );
                    if( pTextNd->HasSwAttrSet() &&
                        SfxItemState::SET == pTextNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, false, &pItem ) )
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

                SwIndex aAlphaIdx(pTextNd);
                pOldTextNd->CutText( pTextNd, aAlphaIdx, SwIndex(pOldTextNd),
                                    pOldTextNd->Len() );
                SwPosition aAlphaPos( aIdx, aAlphaIdx );
                rDoc.CorrRel( rPam.GetPoint()->nNode, aAlphaPos, 0, true );

                // move all Bookmarks/TOXMarks
                if( !pContentStore->Empty() )
                    pContentStore->Restore( rDoc, aIdx.GetIndex() );

                // If the passed PaM is not in the Cursor ring,
                // treat it separately (e.g. when it's being called from AutoFormat)
                if( pOldTextNd == rPam.GetBound().nContent.GetIdxReg() )
                    rPam.GetBound() = aAlphaPos;
                if( pOldTextNd == rPam.GetBound( false ).nContent.GetIdxReg() )
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

            rDoc.CorrRel( aIdx, *rPam.GetPoint(), 0, true );
            // #i100466# adjust given <rPam>, if it does not belong to the cursors
            if ( pDelNd == rPam.GetBound().nContent.GetIdxReg() )
            {
                rPam.GetBound() = SwPosition( SwNodeIndex( *pTextNd ), SwIndex( pTextNd ) );
            }
            if( pDelNd == rPam.GetBound( false ).nContent.GetIdxReg() )
            {
                rPam.GetBound( false ) = SwPosition( SwNodeIndex( *pTextNd ), SwIndex( pTextNd ) );
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
    SwPosition* pSttPos = rPaM.Start(), *pEndPos = rPaM.End();

    std::unique_ptr<SwSpellArgs> pSpellArgs;
    if (pConvArgs)
    {
        pConvArgs->SetStart(pSttPos->nNode.GetNode().GetTextNode(), pSttPos->nContent);
        pConvArgs->SetEnd(  pEndPos->nNode.GetNode().GetTextNode(), pEndPos->nContent );
    }
    else
        pSpellArgs.reset(new SwSpellArgs( xSpeller,
                            pSttPos->nNode.GetNode().GetTextNode(), pSttPos->nContent,
                            pEndPos->nNode.GetNode().GetTextNode(), pEndPos->nContent,
                            bGrammarCheck ));

    sal_uLong nCurrNd = pSttPos->nNode.GetIndex();
    sal_uLong nEndNd = pEndPos->nNode.GetIndex();

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
                    else if( !static_cast<SwTextFrame*>(pContentFrame)->IsHiddenNow() )
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
                            long nStat;
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
                            nBeginGrammarCheck = pSpellArgs->pStartNode == pNd ?  pSpellArgs->pStartIdx->GetIndex() : 0;
                            // if grammar checking starts inside of a sentence the start position has to be adjusted
                            if( nBeginGrammarCheck )
                            {
                                SwIndex aStartIndex( dynamic_cast< SwTextNode* >( pNd ), nBeginGrammarCheck );
                                SwPosition aStart( *pNd, aStartIndex );
                                SwCursor aCursor(aStart, nullptr);
                                SwPosition aOrigPos = *aCursor.GetPoint();
                                aCursor.GoSentence( SwCursor::START_SENT );
                                if( aOrigPos != *aCursor.GetPoint() )
                                {
                                    nBeginGrammarCheck = aCursor.GetPoint()->nContent.GetIndex();
                                }
                            }
                            nEndGrammarCheck = (pSpellArgs->pEndNode == pNd)
                                ? pSpellArgs->pEndIdx->GetIndex()
                                : pNd->GetTextNode()
                                    ->GetText().getLength();
                        }

                        sal_Int32 nSpellErrorPosition = pNd->GetTextNode()->GetText().getLength();
                        if( (!pConvArgs && pNd->GetTextNode()->Spell( pSpellArgs.get() )) ||
                            ( pConvArgs && pNd->GetTextNode()->Convert( *pConvArgs )))
                        {
                            // Cancel and remember position
                            pSttPos->nNode = nCurrNd;
                            pEndPos->nNode = nCurrNd;
                            nCurrNd = nEndNd;
                            if( pSpellArgs )
                                nSpellErrorPosition = pSpellArgs->pStartIdx->GetIndex() > pSpellArgs->pEndIdx->GetIndex() ?
                                            pSpellArgs->pEndIdx->GetIndex() :
                                            pSpellArgs->pStartIdx->GetIndex();
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
                                    nCurrNd = pNd->GetIndex();
                                    pSttPos->nNode = nCurrNd;
                                    pEndPos->nNode = nCurrNd;
                                    pSpellArgs->pStartNode = pNd->GetTextNode();
                                    pSpellArgs->pEndNode = pNd->GetTextNode();
                                    pSpellArgs->pStartIdx->Assign(pNd->GetTextNode(), aConversionMap.ConvertToModelPosition( rError.nErrorStart ).mnPos );
                                    pSpellArgs->pEndIdx->Assign(pNd->GetTextNode(), aConversionMap.ConvertToModelPosition( rError.nErrorStart + rError.nErrorLength ).mnPos );
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
    const SwNode *m_pStart;
    const SwNode *m_pEnd;
          SwNode *m_pNode;
    sal_uInt16 *m_pPageCnt;
    sal_uInt16 *m_pPageSt;

    sal_uInt32 m_nNode;
    sal_Int32 m_nPamStart;
    sal_Int32 m_nPamLen;

public:
    SwHyphArgs( const SwPaM *pPam, const Point &rPoint,
                sal_uInt16* pPageCount, sal_uInt16* pPageStart );
    void SetPam( SwPaM *pPam ) const;
    void SetNode( SwNode *pNew ) { m_pNode = pNew; }
    inline void SetRange( const SwNode *pNew );
    void NextNode() { ++m_nNode; }
    sal_uInt16 *GetPageCnt() { return m_pPageCnt; }
    sal_uInt16 *GetPageSt() { return m_pPageSt; }
};

}

SwHyphArgs::SwHyphArgs( const SwPaM *pPam, const Point &rCursorPos,
                         sal_uInt16* pPageCount, sal_uInt16* pPageStart )
     : SwInterHyphInfo( rCursorPos ), m_pNode(nullptr),
     m_pPageCnt( pPageCount ), m_pPageSt( pPageStart )
{
    // The following constraints have to be met:
    // 1) there is at least one Selection
    // 2) SPoint() == Start()
    OSL_ENSURE( pPam->HasMark(), "SwDoc::Hyphenate: blowing in the wind");
    OSL_ENSURE( *pPam->GetPoint() <= *pPam->GetMark(),
            "SwDoc::Hyphenate: New York, New York");

    const SwPosition *pPoint = pPam->GetPoint();
    m_nNode = pPoint->nNode.GetIndex();

    // Set start
    m_pStart = pPoint->nNode.GetNode().GetTextNode();
    m_nPamStart = pPoint->nContent.GetIndex();

    // Set End and Length
    const SwPosition *pMark = pPam->GetMark();
    m_pEnd = pMark->nNode.GetNode().GetTextNode();
    m_nPamLen = pMark->nContent.GetIndex();
    if( pPoint->nNode == pMark->nNode )
        m_nPamLen = m_nPamLen - pPoint->nContent.GetIndex();
}

inline void SwHyphArgs::SetRange( const SwNode *pNew )
{
    m_nStart = m_pStart == pNew ? m_nPamStart : 0;
    m_nEnd   = m_pEnd   == pNew ? m_nPamStart + m_nPamLen : SAL_MAX_INT32;
}

void SwHyphArgs::SetPam( SwPaM *pPam ) const
{
    if( !m_pNode )
        *pPam->GetPoint() = *pPam->GetMark();
    else
    {
        pPam->GetPoint()->nNode = m_nNode;
        pPam->GetPoint()->nContent.Assign( m_pNode->GetContentNode(), m_nWordStart );
        pPam->GetMark()->nNode = m_nNode;
        pPam->GetMark()->nContent.Assign( m_pNode->GetContentNode(),
                                          m_nWordStart + m_nWordLen );
        OSL_ENSURE( m_nNode == m_pNode->GetIndex(),
                "SwHyphArgs::SetPam: Pam disaster" );
    }
}

// Returns true if we can proceed.
static bool lcl_HyphenateNode( const SwNodePtr& rpNd, void* pArgs )
{
    // Hyphenate returns true if there is a hyphenation point and sets pPam
    SwTextNode *pNode = rpNd->GetTextNode();
    SwHyphArgs *pHyphArgs = static_cast<SwHyphArgs*>(pArgs);
    if( pNode )
    {
        // sw_redlinehide: this will be called once per node for merged nodes;
        // the fully deleted ones won't have frames so are skipped.
        SwContentFrame* pContentFrame = pNode->getLayoutFrame( pNode->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout() );
        if( pContentFrame && !static_cast<SwTextFrame*>(pContentFrame)->IsHiddenNow() )
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
                long nStat = nPageNr >= *pPageSt ? nPageNr - *pPageSt + 1
                                         : nPageNr + *pPageCnt - *pPageSt + 1;
                ::SetProgressState( nStat, pNode->GetDoc().GetDocShell() );
            }
            pHyphArgs->SetRange( rpNd );
            if( pNode->Hyphenate( *pHyphArgs ) )
            {
                pHyphArgs->SetNode( rpNd );
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
    SwNodeIndex aTmpIdx( pPam->GetMark()->nNode, 1 );
    GetNodes().ForEach( pPam->GetPoint()->nNode, aTmpIdx,
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
    const SwPosition* pStt = rPaM.Start();
    const SwPosition* pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();

    const sal_uLong nSttNd = pStt->nNode.GetIndex();
    const sal_uLong nEndNd = pEnd->nNode.GetIndex();

    const sal_Int32 nSttCnt = pStt->nContent.GetIndex();
    const sal_Int32 nEndCnt = pEnd->nContent.GetIndex();

    const SwTextNode* pTNd = pStt->nNode.GetNode().GetTextNode();
    if( pStt == pEnd && pTNd )                  // no region ?
    {
        // do nothing
        return;
    }

    if( nSttNd != nEndNd )
    {
        SwNodeIndex aIdx( pStt->nNode );
        if( nSttCnt )
        {
            ++aIdx;
            if( pTNd )
                pTNd->CountWords( rStat, nSttCnt, pTNd->GetText().getLength() );
        }

        for( ; aIdx.GetIndex() < nEndNd; ++aIdx )
            if( nullptr != ( pTNd = aIdx.GetNode().GetTextNode() ))
                pTNd->CountWords( rStat, 0, pTNd->GetText().getLength() );

        if( nEndCnt && nullptr != ( pTNd = pEnd->nNode.GetNode().GetTextNode() ))
            pTNd->CountWords( rStat, 0, nEndCnt );
    }
    else if( pTNd && nSttCnt < nEndCnt )
        pTNd->CountWords( rStat, nSttCnt, nEndCnt );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
