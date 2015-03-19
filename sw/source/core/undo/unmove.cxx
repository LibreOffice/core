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

#include <UndoSplitMove.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>

// MOVE
SwUndoMove::SwUndoMove( const SwPaM& rRange, const SwPosition& rMvPos )
    : SwUndo( UNDO_MOVE )
    , SwUndRng( rRange )
    , nDestSttNode(0)
    , nDestEndNode(0)
    , nInsPosNode(0)
    , nMvDestNode(rMvPos.nNode.GetIndex())
    , nDestSttCntnt(0)
    , nDestEndCntnt(0)
    , nInsPosCntnt(0)
    , nMvDestCntnt(rMvPos.nContent.GetIndex())
    , bJoinNext(false)
    , bJoinPrev(false)
    , bMoveRange(false)
    , bMoveRedlines(false)
{
    // get StartNode from footnotes before delete!
    SwDoc* pDoc = rRange.GetDoc();
    SwTxtNode* pTxtNd = pDoc->GetNodes()[ nSttNode ]->GetTxtNode();
    SwTxtNode* pEndTxtNd = pDoc->GetNodes()[ nEndNode ]->GetTxtNode();

    pHistory = new SwHistory;

    if( pTxtNd )
    {
        pHistory->Add( pTxtNd->GetTxtColl(), nSttNode, ND_TEXTNODE );
        if ( pTxtNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode,
                                0, pTxtNd->GetTxt().getLength(), false );
        }
        if( pTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nSttNode );
    }
    if( pEndTxtNd && pEndTxtNd != pTxtNd )
    {
        pHistory->Add( pEndTxtNd->GetTxtColl(), nEndNode, ND_TEXTNODE );
        if ( pEndTxtNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pEndTxtNd->GetpSwpHints(), nEndNode,
                                0, pEndTxtNd->GetTxt().getLength(), false );
        }
        if( pEndTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pEndTxtNd->GetpSwAttrSet(), nEndNode );
    }

    pTxtNd = rMvPos.nNode.GetNode().GetTxtNode();
    if (0 != pTxtNd)
    {
        pHistory->Add( pTxtNd->GetTxtColl(), nMvDestNode, ND_TEXTNODE );
        if ( pTxtNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nMvDestNode,
                                0, pTxtNd->GetTxt().getLength(), false );
        }
        if( pTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nMvDestNode );
    }

    nFtnStt = pHistory->Count();
    DelFtn( rRange );

    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}

SwUndoMove::SwUndoMove( SwDoc* pDoc, const SwNodeRange& rRg,
                        const SwNodeIndex& rMvPos )
    : SwUndo(UNDO_MOVE)
    , nDestSttNode(0)
    , nDestEndNode(0)
    , nInsPosNode(0)
    , nMvDestNode(rMvPos.GetIndex())
    , nDestSttCntnt(0)
    , nDestEndCntnt(0)
    , nInsPosCntnt(0)
    , bMoveRedlines(false)
{
    bMoveRange = true;
    bJoinNext = bJoinPrev = false;

    nSttCntnt = nEndCntnt = nMvDestCntnt = COMPLETE_STRING;

    nSttNode = rRg.aStart.GetIndex();
    nEndNode = rRg.aEnd.GetIndex();

//  DelFtn( rRange );
// FIXME: duplication of the method body of DelFtn below

    // is the current move from CntntArea into the special section?
    sal_uLong nCntntStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( nMvDestNode < nCntntStt && rRg.aStart.GetIndex() > nCntntStt )
    {
        // delete all footnotes since they are undesired there
        SwPosition aPtPos( rRg.aEnd );
        SwCntntNode* pCNd = rRg.aEnd.GetNode().GetCntntNode();
        if( pCNd )
            aPtPos.nContent.Assign( pCNd, pCNd->Len() );
        SwPosition aMkPos( rRg.aStart );
        if( 0 != ( pCNd = aMkPos.nNode.GetNode().GetCntntNode() ))
            aMkPos.nContent.Assign( pCNd, 0 );

        DelCntntIndex( aMkPos, aPtPos, nsDelCntntType::DELCNT_FTN );

        if( pHistory && !pHistory->Count() )
            DELETEZ( pHistory );
    }

    nFtnStt = 0;
}

void SwUndoMove::SetDestRange( const SwPaM& rRange,
                                const SwPosition& rInsPos,
                                bool bJoin, bool bCorrPam )
{
    const SwPosition *pStt = rRange.Start(),
                    *pEnd = rRange.GetPoint() == pStt
                        ? rRange.GetMark()
                        : rRange.GetPoint();

    nDestSttNode    = pStt->nNode.GetIndex();
    nDestSttCntnt   = pStt->nContent.GetIndex();
    nDestEndNode    = pEnd->nNode.GetIndex();
    nDestEndCntnt   = pEnd->nContent.GetIndex();

    nInsPosNode     = rInsPos.nNode.GetIndex();
    nInsPosCntnt    = rInsPos.nContent.GetIndex();

    if( bCorrPam )
    {
        nDestSttNode--;
        nDestEndNode--;
    }

    bJoinNext = nDestSttNode != nDestEndNode &&
                pStt->nNode.GetNode().GetTxtNode() &&
                pEnd->nNode.GetNode().GetTxtNode();
    bJoinPrev = bJoin;
}

void SwUndoMove::SetDestRange( const SwNodeIndex& rStt,
                                const SwNodeIndex& rEnd,
                                const SwNodeIndex& rInsPos )
{
    nDestSttNode = rStt.GetIndex();
    nDestEndNode = rEnd.GetIndex();
    if( nDestSttNode > nDestEndNode )
    {
        nDestSttNode = nDestEndNode;
        nDestEndNode = rStt.GetIndex();
    }
    nInsPosNode  = rInsPos.GetIndex();

    nDestSttCntnt = nDestEndCntnt = nInsPosCntnt = COMPLETE_STRING;
}

void SwUndoMove::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    // Block so that we can jump out of it
    do {
        // create index position and section based on the existing values
        SwNodeIndex aIdx( pDoc->GetNodes(), nDestSttNode );

        if( bMoveRange )
        {
            // only a move with SwRange
            SwNodeRange aRg( aIdx, aIdx );
            aRg.aEnd = nDestEndNode;
            aIdx = nInsPosNode;
            bool bSuccess = pDoc->getIDocumentContentOperations().MoveNodeRange( aRg, aIdx,
                    SwMoveFlags::DEFAULT );
            if (!bSuccess)
                break;
        }
        else
        {
            SwPaM aPam( aIdx.GetNode(), nDestSttCntnt,
                        *pDoc->GetNodes()[ nDestEndNode ], nDestEndCntnt );

            // #i17764# if redlines are to be moved, we may not remove them
            // before pDoc->Move gets a chance to handle them
            if( ! bMoveRedlines )
                RemoveIdxFromRange( aPam, false );

            SwPosition aPos( *pDoc->GetNodes()[ nInsPosNode] );
            SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
            aPos.nContent.Assign( pCNd, nInsPosCntnt );

            if( pCNd->HasSwAttrSet() )
                pCNd->ResetAllAttr();

            if( pCNd->IsTxtNode() && static_cast<SwTxtNode*>(pCNd)->GetpSwpHints() )
                static_cast<SwTxtNode*>(pCNd)->ClearSwpHintsArr( false );

            // first delete all attributes at InsertPos
            const bool bSuccess = pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos, (bMoveRedlines)
                        ? SwMoveFlags::REDLINES
                        : SwMoveFlags::DEFAULT );
            if (!bSuccess)
                break;

            aPam.Exchange();
            aPam.DeleteMark();
            if( aPam.GetNode().IsCntntNode() )
                aPam.GetNode().GetCntntNode()->ResetAllAttr();
            // the Pam will be dropped now
        }

        SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
        if( bJoinNext )
        {
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex(pTxtNd, pTxtNd->GetTxt().getLength())));
            }
            // Are there any Pams in the next TextNode?
            pTxtNd->JoinNext();
        }

        if( bJoinPrev && pTxtNd->CanJoinPrev( &aIdx ) )
        {
            // Are there any Pams in the next TextNode?
            pTxtNd = aIdx.GetNode().GetTxtNode();
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex( pTxtNd, pTxtNd->GetTxt().getLength())));
            }
            pTxtNd->JoinNext();
        }

    } while( false );

    if( pHistory )
    {
        if( nFtnStt != pHistory->Count() )
            pHistory->Rollback( pDoc, nFtnStt );
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    // set the cursor onto Undo area
    if( !bMoveRange )
    {
        AddUndoRedoPaM(rContext);
    }
}

void SwUndoMove::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM *const pPam = & AddUndoRedoPaM(rContext);
    SwDoc & rDoc = rContext.GetDoc();

    SwNodes& rNds = rDoc.GetNodes();
    SwNodeIndex aIdx( rNds, nMvDestNode );

    if( bMoveRange )
    {
        // only a move with SwRange
        SwNodeRange aRg( rNds, nSttNode, rNds, nEndNode );
        rDoc.getIDocumentContentOperations().MoveNodeRange( aRg, aIdx, (bMoveRedlines)
                ? SwMoveFlags::REDLINES
                : SwMoveFlags::DEFAULT );
    }
    else
    {
        SwPaM aPam( *pPam->GetPoint() );
        SetPaM( aPam );
        SwPosition aMvPos( aIdx, SwIndex( aIdx.GetNode().GetCntntNode(),
                                        nMvDestCntnt ));

        DelFtn( aPam );
        RemoveIdxFromRange( aPam, false );

        aIdx = aPam.Start()->nNode;
        bool bJoinTxt = aIdx.GetNode().IsTxtNode();

        --aIdx;
        rDoc.getIDocumentContentOperations().MoveRange( aPam, aMvPos,
            SwMoveFlags::DEFAULT );

        if( nSttNode != nEndNode && bJoinTxt )
        {
            ++aIdx;
            SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();
            if( pTxtNd && pTxtNd->CanJoinNext() )
            {
                {
                    RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                            SwIndex(pTxtNd, pTxtNd->GetTxt().getLength())));
                }
                pTxtNd->JoinNext();
            }
        }
        *pPam->GetPoint() = *aPam.GetPoint();
        pPam->SetMark();
        *pPam->GetMark() = *aPam.GetMark();
    }
}

void SwUndoMove::DelFtn( const SwPaM& rRange )
{
    // is the current move from CntntArea into the special section?
    SwDoc* pDoc = rRange.GetDoc();
    sal_uLong nCntntStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( nMvDestNode < nCntntStt &&
        rRange.GetPoint()->nNode.GetIndex() >= nCntntStt )
    {
        // delete all footnotes since they are undesired there
        DelCntntIndex( *rRange.GetMark(), *rRange.GetPoint(),
                            nsDelCntntType::DELCNT_FTN );

        if( pHistory && !pHistory->Count() )
            delete pHistory, pHistory = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
