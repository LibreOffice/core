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
#include <txtftn.hxx>
#include <acorrect.hxx>
#include <UndoManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <doctxm.hxx>
#include <ftnidx.hxx>
#include <mdiexp.hxx>
#include <mvsave.hxx>
#include <redline.hxx>
#include <rootfrm.hxx>
#include <splargs.hxx>
#include <txtfrm.hxx>
#include <breakit.hxx>
#include <vcl/layout.hxx>
#include "comcore.hrc"
#include "editsh.hxx"
#include <fmtfld.hxx>
#include <docufld.hxx>
#include <unoflatpara.hxx>
#include <SwGrammarMarkUp.hxx>
#include <docedt.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;


void _RestFlyInRange( _SaveFlyArr & rArr, const SwNodeIndex& rSttIdx,
                      const SwNodeIndex* pInsertPos )
{
    SwPosition aPos( rSttIdx );
    for( size_t n = 0; n < rArr.size(); ++n )
    {
        // create new anchor
        _SaveFly& rSave = rArr[n];
        SwFrmFmt* pFmt = rSave.pFrmFmt;

        if( rSave.bInsertPosition )
        {
            if( pInsertPos != NULL )
                aPos.nNode = *pInsertPos;
            else
                aPos.nNode = rSttIdx.GetIndex();
        }
        else
            aPos.nNode = rSttIdx.GetIndex() + rSave.nNdDiff;

        aPos.nContent.Assign( 0, 0 );
        SwFmtAnchor aAnchor( pFmt->GetAnchor() );
        aAnchor.SetAnchor( &aPos );
        pFmt->GetDoc()->GetSpzFrmFmts()->push_back( pFmt );
        pFmt->SetFmtAttr( aAnchor );
        SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
        if( pCNd && pCNd->getLayoutFrm( pFmt->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), 0, 0, false ) )
            pFmt->MakeFrms();
    }
}

void _SaveFlyInRange( const SwNodeRange& rRg, _SaveFlyArr& rArr )
{
    SwFrmFmts& rFmts = *rRg.aStart.GetNode().GetDoc()->GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < rFmts.size(); ++n )
    {
        SwFrmFmt *const pFmt = rFmts[n];
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            rRg.aStart <= pAPos->nNode && pAPos->nNode < rRg.aEnd )
        {
            _SaveFly aSave( pAPos->nNode.GetIndex() - rRg.aStart.GetIndex(),
                            pFmt, false );
            rArr.push_back( aSave );
            pFmt->DelFrms();
            rFmts.erase( rFmts.begin() + n-- );
        }
    }
}

void _SaveFlyInRange( const SwPaM& rPam, const SwNodeIndex& rInsPos,
                       _SaveFlyArr& rArr, bool bMoveAllFlys )
{
    SwFrmFmts& rFmts = *rPam.GetPoint()->nNode.GetNode().GetDoc()->GetSpzFrmFmts();
    SwFrmFmt* pFmt;
    const SwFmtAnchor* pAnchor;

    const SwPosition* pPos = rPam.Start();
    const SwNodeIndex& rSttNdIdx = pPos->nNode;
    short nSttOff = (!bMoveAllFlys && rSttNdIdx.GetNode().IsCntntNode() &&
                    pPos->nContent.GetIndex()) ? 1 : 0;

    pPos = rPam.GetPoint() == pPos ? rPam.GetMark() : rPam.GetPoint();
    const SwNodeIndex& rEndNdIdx = pPos->nNode;
    short nOff = ( bMoveAllFlys || ( rEndNdIdx.GetNode().IsCntntNode() &&
                pPos->nContent == rEndNdIdx.GetNode().GetCntntNode()->Len() ))
                    ? 0 : 1;

    const SwNodeIndex* pCntntIdx;

    for( sal_uInt16 n = 0; n < rFmts.size(); ++n )
    {
        pFmt = (SwFrmFmt*)rFmts[n];
        pAnchor = &pFmt->GetAnchor();
        const SwPosition* pAPos = pAnchor->GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            // do not move if the InsPos is in the CntntArea of the Fly
            ( 0 == ( pCntntIdx = pFmt->GetCntnt().GetCntntIdx() ) ||
              !( *pCntntIdx < rInsPos &&
                rInsPos < pCntntIdx->GetNode().EndOfSectionIndex() )) )
        {
            bool bInsPos = false;

            if( !bMoveAllFlys && rEndNdIdx == pAPos->nNode )
            {
                // Do not touch Anchor, if only a part of the EndNode
                // or the whole EndNode is identical with the SttNode
                if( rSttNdIdx != pAPos->nNode )
                {
                    // Only attach an anchor to the beginning or end
                    SwPosition aPos( rSttNdIdx );
                    SwFmtAnchor aAnchor( *pAnchor );
                    aAnchor.SetAnchor( &aPos );
                    pFmt->SetFmtAttr( aAnchor );
                }
            }
            else if( ( rSttNdIdx.GetIndex() + nSttOff <= pAPos->nNode.GetIndex()
                    && pAPos->nNode.GetIndex() <= rEndNdIdx.GetIndex() - nOff ) ||
                        ( bInsPos = (rInsPos == pAPos->nNode) ))

            {
                _SaveFly aSave( pAPos->nNode.GetIndex() - rSttNdIdx.GetIndex(),
                                pFmt, bInsPos );
                rArr.push_back( aSave );
                pFmt->DelFrms();
                rFmts.erase( rFmts.begin() + n-- );
            }
        }
    }
}

/// Delete and move all Flys at the paragraph, that are within the selection.
/// If there is a Fly at the SPoint, it is moved onto the Mark.
void DelFlyInRange( const SwNodeIndex& rMkNdIdx,
                    const SwNodeIndex& rPtNdIdx )
{
    const bool bDelFwrd = rMkNdIdx.GetIndex() <= rPtNdIdx.GetIndex();

    SwDoc* pDoc = rMkNdIdx.GetNode().GetDoc();
    SwFrmFmts& rTbl = *pDoc->GetSpzFrmFmts();
    for ( sal_uInt16 i = rTbl.size(); i; )
    {
        SwFrmFmt *pFmt = rTbl[--i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        SwPosition const*const pAPos = rAnch.GetCntntAnchor();
        if (pAPos &&
            ((rAnch.GetAnchorId() == FLY_AT_PARA) ||
             (rAnch.GetAnchorId() == FLY_AT_CHAR)) &&
            ( bDelFwrd
                ? rMkNdIdx < pAPos->nNode && pAPos->nNode <= rPtNdIdx
                : rPtNdIdx <= pAPos->nNode && pAPos->nNode < rMkNdIdx ))
        {
            // Only move the Anchor??
            if( rPtNdIdx == pAPos->nNode )
            {
                SwFmtAnchor aAnch( pFmt->GetAnchor() );
                SwPosition aPos( rMkNdIdx );
                aAnch.SetAnchor( &aPos );
                pFmt->SetFmtAttr( aAnch );
            }
            else
            {
                // If the Fly is deleted, all Flys in it's content have to be deleted too.
                const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
                if( rCntnt.GetCntntIdx() )
                {
                    DelFlyInRange( *rCntnt.GetCntntIdx(),
                                    SwNodeIndex( *rCntnt.GetCntntIdx()->
                                            GetNode().EndOfSectionNode() ));
                    // Position could have been moved!
                    if( i > rTbl.size() )
                        i = rTbl.size();
                    else if( pFmt != rTbl[i] )
                        i = rTbl.GetPos( pFmt );
                }

                pDoc->getIDocumentLayoutAccess().DelLayoutFmt( pFmt );

                // DelLayoutFmt can also trigger the deletion of objects.
                if( i > rTbl.size() )
                    i = rTbl.size();
            }
        }
    }
}

// #i59534: Redo of insertion of multiple text nodes runs into trouble
// because of unnecessary expanded redlines
// From now on this class saves the redline positions of all redlines which ends exact at the
// insert position (node _and_ content index)
_SaveRedlEndPosForRestore::_SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx, sal_Int32 nCnt )
    : pSavArr( 0 ), pSavIdx( 0 ), nSavCntnt( nCnt )
{
    SwNode& rNd = rInsIdx.GetNode();
    SwDoc* pDest = rNd.GetDoc();
    if( !pDest->getIDocumentRedlineAccess().GetRedlineTbl().empty() )
    {
        sal_uInt16 nFndPos;
        const SwPosition* pEnd;
        SwPosition aSrcPos( rInsIdx, SwIndex( rNd.GetCntntNode(), nCnt ));
        pDest->getIDocumentRedlineAccess().GetRedline( aSrcPos, &nFndPos );
        const SwRangeRedline* pRedl;
        while( nFndPos--
              && *( pEnd = ( pRedl = pDest->getIDocumentRedlineAccess().GetRedlineTbl()[ nFndPos ] )->End() ) == aSrcPos
              && *pRedl->Start() < aSrcPos )
        {
            if( !pSavArr )
            {
                pSavArr = new std::vector<SwPosition*>;
                pSavIdx = new SwNodeIndex( rInsIdx, -1 );
            }
            pSavArr->push_back( (SwPosition*)pEnd );
        }
    }
}

_SaveRedlEndPosForRestore::~_SaveRedlEndPosForRestore()
{
    delete pSavArr;
    delete pSavIdx;
}

void _SaveRedlEndPosForRestore::_Restore()
{
    ++(*pSavIdx);
    SwCntntNode* pNode = pSavIdx->GetNode().GetCntntNode();
    // If there's no content node at the remembered position, we will not restore the old position
    // This may happen if a table (or section?) will be inserted.
    if( pNode )
    {
        SwPosition aPos( *pSavIdx, SwIndex( pNode, nSavCntnt ));
        for( sal_uInt16 n = pSavArr->size(); n; )
            *(*pSavArr)[ --n ] = aPos;
    }
}

void SwDoc::SetModified(SwPaM &rPaM)
{
    SwDataChanged aTmp( rPaM );
    getIDocumentState().SetModified();
}

/// Convert list of ranges of whichIds to a corresponding list of whichIds
static std::vector<sal_uInt16> * lcl_RangesToVector(sal_uInt16 * pRanges)
{
    std::vector<sal_uInt16> * pResult = new std::vector<sal_uInt16>();

    int i = 0;
    while (pRanges[i] != 0)
    {
        OSL_ENSURE(pRanges[i+1] != 0, "malformed ranges");

        for (sal_uInt16 j = pRanges[i]; j < pRanges[i+1]; j++)
            pResult->push_back(j);

        i += 2;
    }

    return pResult;
}

void sw_GetJoinFlags( SwPaM& rPam, bool& rJoinTxt, bool& rJoinPrev )
{
    rJoinTxt = false;
    rJoinPrev = false;
    if( rPam.GetPoint()->nNode != rPam.GetMark()->nNode )
    {
        const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
        SwTxtNode *pSttNd = pStt->nNode.GetNode().GetTxtNode();
        if( pSttNd )
        {
            SwTxtNode *pEndNd = pEnd->nNode.GetNode().GetTxtNode();
            rJoinTxt = 0 != pEndNd;
            if( rJoinTxt )
            {
                bool bExchange = pStt == rPam.GetPoint();
                if( !pStt->nContent.GetIndex() &&
                    pEndNd->GetTxt().getLength() != pEnd->nContent.GetIndex())
                    bExchange = !bExchange;
                if( bExchange )
                    rPam.Exchange();
                rJoinPrev = rPam.GetPoint() == pStt;
                OSL_ENSURE( !pStt->nContent.GetIndex() &&
                    pEndNd->GetTxt().getLength() != pEnd->nContent.GetIndex()
                    ? rPam.GetPoint()->nNode < rPam.GetMark()->nNode
                    : rPam.GetPoint()->nNode > rPam.GetMark()->nNode,
                    "sw_GetJoinFlags");
            }
        }
    }
}

void sw_JoinText( SwPaM& rPam, bool bJoinPrev )
{
    SwNodeIndex aIdx( rPam.GetPoint()->nNode );
    SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
    SwNodeIndex aOldIdx( aIdx );
    SwTxtNode *pOldTxtNd = pTxtNd;

    if( pTxtNd && pTxtNd->CanJoinNext( &aIdx ) )
    {
        SwDoc* pDoc = rPam.GetDoc();
        if( bJoinPrev )
        {
            // We do not need to handle xmlids in this case, because
            // it is only invoked if one paragraph is completely empty
            // (see sw_GetJoinFlags)
            {
                // If PageBreaks are deleted/set, it must not be added to the Undo history!
                // Also, deleteing the Node is not added to the Undo histroy!
                ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

                /* PageBreaks, PageDesc, ColumnBreaks */
                // If we need to change something about the logic to copy the PageBreaks,
                // PageDesc, etc. we also have to change SwUndoDelete.
                // There, we copy the AUTO PageBreak from the GetMarkNode!

                /* The GetMarkNode */
                if( ( pTxtNd = aIdx.GetNode().GetTxtNode())->HasSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    if( SfxItemState::SET == pTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, false, &pItem ) )
                        pTxtNd->ResetAttr( RES_BREAK );
                    if( pTxtNd->HasSwAttrSet() &&
                        SfxItemState::SET == pTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, false, &pItem ) )
                        pTxtNd->ResetAttr( RES_PAGEDESC );
                }

                /* The PointNode */
                if( pOldTxtNd->HasSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet( pDoc->GetAttrPool(), aBreakSetRange );
                    const SfxItemSet* pSet = pOldTxtNd->GetpSwAttrSet();
                    if( SfxItemState::SET == pSet->GetItemState( RES_BREAK,
                        false, &pItem ) )
                        aSet.Put( *pItem );
                    if( SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC,
                        false, &pItem ) )
                        aSet.Put( *pItem );
                    if( aSet.Count() )
                        pTxtNd->SetAttr( aSet );
                }
                pOldTxtNd->FmtToTxtAttr( pTxtNd );

                const boost::shared_ptr< sw::mark::CntntIdxStore> pCntntStore(sw::mark::CntntIdxStore::Create());
                pCntntStore->Save( pDoc, aOldIdx.GetIndex(), pOldTxtNd->Len() );

                SwIndex aAlphaIdx(pTxtNd);
                pOldTxtNd->CutText( pTxtNd, aAlphaIdx, SwIndex(pOldTxtNd),
                                    pOldTxtNd->Len() );
                SwPosition aAlphaPos( aIdx, aAlphaIdx );
                pDoc->CorrRel( rPam.GetPoint()->nNode, aAlphaPos, 0, true );

                // move all Bookmarks/TOXMarks
                if( !pCntntStore->Empty() )
                    pCntntStore->Restore( pDoc, aIdx.GetIndex() );

                // If the passed PaM is not in the Crsr ring,
                // treat it separately (e.g. when it's being called from AutoFormat)
                if( pOldTxtNd == rPam.GetBound( true ).nContent.GetIdxReg() )
                    rPam.GetBound( true ) = aAlphaPos;
                if( pOldTxtNd == rPam.GetBound( false ).nContent.GetIdxReg() )
                    rPam.GetBound( false ) = aAlphaPos;
            }
            // delete the Node, at last!
            pDoc->GetNodes().Delete( aOldIdx, 1 );
        }
        else
        {
            SwTxtNode* pDelNd = aIdx.GetNode().GetTxtNode();
            if( pTxtNd->Len() )
                pDelNd->FmtToTxtAttr( pTxtNd );
            else
            {
                /* This case was missed:

                   <something></something>   <-- pTxtNd
                   <other>ccc</other>        <-- pDelNd

                   <something> and <other> are paragraph
                   attributes. The attribute <something> stayed if not
                   overwritten by an attribute in "ccc". Fixed by
                   first resetting all character attributes in first
                   paragraph (pTxtNd).
                */
                std::vector<sal_uInt16> * pShorts =
                    lcl_RangesToVector(aCharFmtSetRange);
                pTxtNd->ResetAttr(*pShorts);
                delete pShorts;

                if( pDelNd->HasSwAttrSet() )
                {
                    // only copy the character attributes
                    SfxItemSet aTmpSet( pDoc->GetAttrPool(), aCharFmtSetRange );
                    aTmpSet.Put( *pDelNd->GetpSwAttrSet() );
                    pTxtNd->SetAttr( aTmpSet );
                }
            }

            pDoc->CorrRel( aIdx, *rPam.GetPoint(), 0, true );
            // #i100466# adjust given <rPam>, if it does not belong to the cursors
            if ( pDelNd == rPam.GetBound( true ).nContent.GetIdxReg() )
            {
                rPam.GetBound( true ) = SwPosition( SwNodeIndex( *pTxtNd ), SwIndex( pTxtNd ) );
            }
            if( pDelNd == rPam.GetBound( false ).nContent.GetIdxReg() )
            {
                rPam.GetBound( false ) = SwPosition( SwNodeIndex( *pTxtNd ), SwIndex( pTxtNd ) );
            }
            pTxtNd->JoinNext();
        }
    }
}

static void lcl_syncGrammarError( SwTxtNode &rTxtNode, linguistic2::ProofreadingResult& rResult,
    const ModelToViewHelper &rConversionMap )
{
    if( rTxtNode.IsGrammarCheckDirty() )
        return;
    SwGrammarMarkUp* pWrong = rTxtNode.GetGrammarCheck();
    linguistic2::SingleProofreadingError* pArray = rResult.aErrors.getArray();
    sal_uInt16 i, j = 0;
    if( pWrong )
    {
        for( i = 0; i < rResult.aErrors.getLength(); ++i )
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
                    uno::Reference< XSpellChecker1 >  &xSpeller,
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
                    bool bGrammarCheck,
                    SwConversionArgs *pConvArgs  ) const
{
    SwPosition* pSttPos = rPaM.Start(), *pEndPos = rPaM.End();

    SwSpellArgs      *pSpellArgs = 0;
    if (pConvArgs)
    {
        pConvArgs->SetStart(pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent);
        pConvArgs->SetEnd(  pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent );
    }
    else
        pSpellArgs = new SwSpellArgs( xSpeller,
                            pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent,
                            pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent,
                            bGrammarCheck );

    sal_uLong nCurrNd = pSttPos->nNode.GetIndex();
    sal_uLong nEndNd = pEndPos->nNode.GetIndex();

    uno::Any aRet;
    if( nCurrNd <= nEndNd )
    {
        SwCntntFrm* pCntFrm;
        bool bGoOn = true;
        while( bGoOn )
        {
            SwNode* pNd = GetNodes()[ nCurrNd ];
            switch( pNd->GetNodeType() )
            {
            case ND_TEXTNODE:
                if( 0 != ( pCntFrm = pNd->GetTxtNode()->getLayoutFrm( getIDocumentLayoutAccess().GetCurrentLayout() )) )
                {
                    // skip protected and hidden Cells and Flys
                    if( pCntFrm->IsProtected() )
                    {
                        nCurrNd = pNd->EndOfSectionIndex();
                    }
                    else if( !static_cast<SwTxtFrm*>(pCntFrm)->IsHiddenNow() )
                    {
                        if( pPageCnt && *pPageCnt && pPageSt )
                        {
                            sal_uInt16 nPageNr = pCntFrm->GetPhyPageNum();
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
                            ::SetProgressState( nStat, (SwDocShell*)GetDocShell() );
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
                                SwIndex aStartIndex( dynamic_cast< SwTxtNode* >( pNd ), nBeginGrammarCheck );
                                SwPosition aStart( *pNd, aStartIndex );
                                SwCursor aCrsr(aStart, 0, false);
                                SwPosition aOrigPos = *aCrsr.GetPoint();
                                aCrsr.GoSentence( SwCursor::START_SENT );
                                if( aOrigPos != *aCrsr.GetPoint() )
                                {
                                    nBeginGrammarCheck = aCrsr.GetPoint()->nContent.GetIndex();
                                }
                            }
                            nEndGrammarCheck = (pSpellArgs->pEndNode == pNd)
                                ? pSpellArgs->pEndIdx->GetIndex()
                                : pNd->GetTxtNode()
                                    ->GetTxt().getLength();
                        }

                        sal_Int32 nSpellErrorPosition = pNd->GetTxtNode()->GetTxt().getLength();
                        if( (!pConvArgs && pNd->GetTxtNode()->Spell( pSpellArgs )) ||
                            ( pConvArgs && pNd->GetTxtNode()->Convert( *pConvArgs )))
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
                                uno::Reference< lang::XComponent > xDoc( const_cast<SwDocShell*>(GetDocShell())->GetBaseModel(), uno::UNO_QUERY );
                                // Expand the string:
                                const ModelToViewHelper aConversionMap(*pNd->GetTxtNode());
                                OUString aExpandText = aConversionMap.getViewText();

                                // get XFlatParagraph to use...
                                uno::Reference< text::XFlatParagraph > xFlatPara = new SwXFlatParagraph( *pNd->GetTxtNode(), aExpandText, aConversionMap );

                                // get error position of cursor in XFlatParagraph
                                linguistic2::ProofreadingResult aResult;
                                sal_Int32 nGrammarErrors;
                                do
                                {
                                    aConversionMap.ConvertToViewPosition( nBeginGrammarCheck );
                                    aResult = xGCIterator->checkSentenceAtPosition(
                                            xDoc, xFlatPara, aExpandText, lang::Locale(), nBeginGrammarCheck, -1, -1 );

                                    lcl_syncGrammarError( *pNd->GetTxtNode(), aResult, aConversionMap );

                                    // get suggestions to use for the specific error position
                                    nGrammarErrors = aResult.aErrors.getLength();
                                    // if grammar checking doesn't have any progress then quit
                                    if( aResult.nStartOfNextSentencePosition <= nBeginGrammarCheck )
                                        break;
                                    // prepare next iteration
                                    nBeginGrammarCheck = aResult.nStartOfNextSentencePosition;
                                }
                                while( nSpellErrorPosition > aResult.nBehindEndOfSentencePosition && !nGrammarErrors && aResult.nBehindEndOfSentencePosition < nEndGrammarCheck );

                                if( nGrammarErrors > 0 && nSpellErrorPosition >= aResult.nBehindEndOfSentencePosition )
                                {
                                    aRet <<= aResult;
                                    //put the cursor to the current error
                                    const linguistic2::SingleProofreadingError &rError = aResult.aErrors[0];
                                    nCurrNd = pNd->GetIndex();
                                    pSttPos->nNode = nCurrNd;
                                    pEndPos->nNode = nCurrNd;
                                    pSpellArgs->pStartNode = pNd->GetTxtNode();
                                    pSpellArgs->pEndNode = pNd->GetTxtNode();
                                    pSpellArgs->pStartIdx->Assign(pNd->GetTxtNode(), aConversionMap.ConvertToModelPosition( rError.nErrorStart ).mnPos );
                                    pSpellArgs->pEndIdx->Assign(pNd->GetTxtNode(), aConversionMap.ConvertToModelPosition( rError.nErrorStart + rError.nErrorLength ).mnPos );
                                    nCurrNd = nEndNd;
                                }
                            }
                        }
                    }
                }
                break;
            case ND_SECTIONNODE:
                if( ( static_cast<SwSectionNode*>(pNd)->GetSection().IsProtect() ||
                    static_cast<SwSectionNode*>(pNd)->GetSection().IsHidden() ) )
                    nCurrNd = pNd->EndOfSectionIndex();
                break;
            case ND_ENDNODE:
                {
                    break;
                }
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
    delete pSpellArgs;

    return aRet;
}

class SwHyphArgs : public SwInterHyphInfo
{
    const SwNode *pStart;
    const SwNode *pEnd;
          SwNode *pNode;
    sal_uInt16 *pPageCnt;
    sal_uInt16 *pPageSt;

    sal_uInt32 nNode;
    sal_Int32 nPamStart;
    sal_Int32 nPamLen;

public:
    SwHyphArgs( const SwPaM *pPam, const Point &rPoint,
                sal_uInt16* pPageCount, sal_uInt16* pPageStart );
    void SetPam( SwPaM *pPam ) const;
    inline void SetNode( SwNode *pNew ) { pNode = pNew; }
    inline void SetRange( const SwNode *pNew );
    inline void NextNode() { ++nNode; }
    inline sal_uInt16 *GetPageCnt() { return pPageCnt; }
    inline sal_uInt16 *GetPageSt() { return pPageSt; }
};

SwHyphArgs::SwHyphArgs( const SwPaM *pPam, const Point &rCrsrPos,
                         sal_uInt16* pPageCount, sal_uInt16* pPageStart )
     : SwInterHyphInfo( rCrsrPos ), pNode(0),
     pPageCnt( pPageCount ), pPageSt( pPageStart )
{
    // The following constraints have to be met:
    // 1) there is at least one Selection
    // 2) SPoint() == Start()
    OSL_ENSURE( pPam->HasMark(), "SwDoc::Hyphenate: blowing in the wind");
    OSL_ENSURE( *pPam->GetPoint() <= *pPam->GetMark(),
            "SwDoc::Hyphenate: New York, New York");

    const SwPosition *pPoint = pPam->GetPoint();
    nNode = pPoint->nNode.GetIndex();

    // Set start
    pStart = pPoint->nNode.GetNode().GetTxtNode();
    nPamStart = pPoint->nContent.GetIndex();

    // Set End and Length
    const SwPosition *pMark = pPam->GetMark();
    pEnd = pMark->nNode.GetNode().GetTxtNode();
    nPamLen = pMark->nContent.GetIndex();
    if( pPoint->nNode == pMark->nNode )
        nPamLen = nPamLen - pPoint->nContent.GetIndex();
}

inline void SwHyphArgs::SetRange( const SwNode *pNew )
{
    nStart = pStart == pNew ? nPamStart : 0;
    nEnd   = pEnd   == pNew ? nPamStart + nPamLen : SAL_MAX_INT32;
}

void SwHyphArgs::SetPam( SwPaM *pPam ) const
{
    if( !pNode )
        *pPam->GetPoint() = *pPam->GetMark();
    else
    {
        pPam->GetPoint()->nNode = nNode;
        pPam->GetPoint()->nContent.Assign( pNode->GetCntntNode(), nWordStart );
        pPam->GetMark()->nNode = nNode;
        pPam->GetMark()->nContent.Assign( pNode->GetCntntNode(),
                                          nWordStart + nWordLen );
        OSL_ENSURE( nNode == pNode->GetIndex(),
                "SwHyphArgs::SetPam: Pam disaster" );
    }
}

// Returns true if we can proceed.
static bool lcl_HyphenateNode( const SwNodePtr& rpNd, void* pArgs )
{
    // Hyphenate returns true if there is a hyphenation point and sets pPam
    SwTxtNode *pNode = rpNd->GetTxtNode();
    SwHyphArgs *pHyphArgs = (SwHyphArgs*)pArgs;
    if( pNode )
    {
        SwCntntFrm* pCntFrm = pNode->getLayoutFrm( pNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );
        if( pCntFrm && !static_cast<SwTxtFrm*>(pCntFrm)->IsHiddenNow() )
        {
            sal_uInt16 *pPageSt = pHyphArgs->GetPageSt();
            sal_uInt16 *pPageCnt = pHyphArgs->GetPageCnt();
            if( pPageCnt && *pPageCnt && pPageSt )
            {
                sal_uInt16 nPageNr = pCntFrm->GetPhyPageNum();
                if( !*pPageSt )
                {
                    *pPageSt = nPageNr;
                    if( *pPageCnt < *pPageSt )
                        *pPageCnt = *pPageSt;
                }
                long nStat = nPageNr >= *pPageSt ? nPageNr - *pPageSt + 1
                                         : nPageNr + *pPageCnt - *pPageSt + 1;
                ::SetProgressState( nStat, (SwDocShell*)pNode->GetDoc()->GetDocShell() );
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
                            SwPaM *pPam, const Point &rCrsrPos,
                             sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    OSL_ENSURE(this == pPam->GetDoc(), "SwDoc::Hyphenate: strangers in the night");

    if( *pPam->GetPoint() > *pPam->GetMark() )
        pPam->Exchange();

    SwHyphArgs aHyphArg( pPam, rCrsrPos, pPageCnt, pPageSt );
    SwNodeIndex aTmpIdx( pPam->GetMark()->nNode, 1 );
    GetNodes().ForEach( pPam->GetPoint()->nNode, aTmpIdx,
                    lcl_HyphenateNode, &aHyphArg );
    aHyphArg.SetPam( pPam );
    return aHyphArg.GetHyphWord();  // will be set by lcl_HyphenateNode
}

// Save the current values to add them as automatic entries to AutoCorrect.
void SwDoc::SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew )
{
    if( pNew != mpACEWord )
        delete mpACEWord;
    mpACEWord = pNew;
}

void SwDoc::DeleteAutoCorrExceptWord()
{
    delete mpACEWord;
    mpACEWord = 0;
}

void SwDoc::CountWords( const SwPaM& rPaM, SwDocStat& rStat ) const
{
    // This is a modified version of SwDoc::TransliterateText
    const SwPosition* pStt = rPaM.Start();
    const SwPosition* pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();

    const sal_uLong nSttNd = pStt->nNode.GetIndex();
    const sal_uLong nEndNd = pEnd->nNode.GetIndex();

    const sal_Int32 nSttCnt = pStt->nContent.GetIndex();
    const sal_Int32 nEndCnt = pEnd->nContent.GetIndex();

    const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
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
                pTNd->CountWords( rStat, nSttCnt, pTNd->GetTxt().getLength() );
        }

        for( ; aIdx.GetIndex() < nEndNd; ++aIdx )
            if( 0 != ( pTNd = aIdx.GetNode().GetTxtNode() ))
                pTNd->CountWords( rStat, 0, pTNd->GetTxt().getLength() );

        if( nEndCnt && 0 != ( pTNd = pEnd->nNode.GetNode().GetTxtNode() ))
            pTNd->CountWords( rStat, 0, nEndCnt );
    }
    else if( pTNd && nSttCnt < nEndCnt )
        pTNd->CountWords( rStat, nSttCnt, nEndCnt );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
