/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    : SwUndo( UNDO_MOVE ), SwUndRng( rRange ),
    nMvDestNode( rMvPos.nNode.GetIndex() ),
    nMvDestCntnt( rMvPos.nContent.GetIndex() ),
    bMoveRedlines( false )
{
    bMoveRange = bJoinNext = bJoinPrev = sal_False;

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
                                0, pTxtNd->GetTxt().Len(), false );
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
                                0, pEndTxtNd->GetTxt().Len(), false );
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
                                0, pTxtNd->GetTxt().Len(), false );
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
    : SwUndo( UNDO_MOVE ),
    nMvDestNode( rMvPos.GetIndex() ),
    bMoveRedlines( false )
{
    bMoveRange = sal_True;
    bJoinNext = bJoinPrev = sal_False;

    nSttCntnt = nEndCntnt = nMvDestCntnt = STRING_MAXLEN;

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
                                sal_Bool bJoin, sal_Bool bCorrPam )
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

    nDestSttCntnt = nDestEndCntnt = nInsPosCntnt = STRING_MAXLEN;
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
            bool bSuccess = pDoc->MoveNodeRange( aRg, aIdx,
                    IDocumentContentOperations::DOC_MOVEDEFAULT );
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
                RemoveIdxFromRange( aPam, sal_False );

            SwPosition aPos( *pDoc->GetNodes()[ nInsPosNode] );
            SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
            aPos.nContent.Assign( pCNd, nInsPosCntnt );

            if( pCNd->HasSwAttrSet() )
                pCNd->ResetAllAttr();

            if( pCNd->IsTxtNode() && ((SwTxtNode*)pCNd)->GetpSwpHints() )
                ((SwTxtNode*)pCNd)->ClearSwpHintsArr( false );

            // first delete all attributes at InsertPos
            const bool bSuccess = pDoc->MoveRange( aPam, aPos, (bMoveRedlines)
                        ? IDocumentContentOperations::DOC_MOVEREDLINES
                        : IDocumentContentOperations::DOC_MOVEDEFAULT );
            if (!bSuccess)
                break;

            aPam.Exchange();
            aPam.DeleteMark();
            if( aPam.GetNode()->IsCntntNode() )
                aPam.GetNode()->GetCntntNode()->ResetAllAttr();
            // the Pam will be dropped now
        }

        SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
        if( bJoinNext )
        {
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex( pTxtNd, pTxtNd->GetTxt().Len() ) ) );
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
                        SwIndex( pTxtNd, pTxtNd->GetTxt().Len() ) ) );
            }
            pTxtNd->JoinNext();
        }

    } while( sal_False );

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
        rDoc.MoveNodeRange( aRg, aIdx, (bMoveRedlines)
                ? IDocumentContentOperations::DOC_MOVEREDLINES
                : IDocumentContentOperations::DOC_MOVEDEFAULT );
    }
    else
    {
        SwPaM aPam( *pPam->GetPoint() );
        SetPaM( aPam );
        SwPosition aMvPos( aIdx, SwIndex( aIdx.GetNode().GetCntntNode(),
                                        nMvDestCntnt ));

        DelFtn( aPam );
        RemoveIdxFromRange( aPam, sal_False );

        aIdx = aPam.Start()->nNode;
        sal_Bool bJoinTxt = aIdx.GetNode().IsTxtNode();

        aIdx--;
        rDoc.MoveRange( aPam, aMvPos,
            IDocumentContentOperations::DOC_MOVEDEFAULT );

        if( nSttNode != nEndNode && bJoinTxt )
        {
            ++aIdx;
            SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();
            if( pTxtNd && pTxtNd->CanJoinNext() )
            {
                {
                    RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                            SwIndex( pTxtNd, pTxtNd->GetTxt().Len() ) ) );
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
