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
    , nDestSttContent(0)
    , nDestEndContent(0)
    , nInsPosContent(0)
    , nMvDestContent(rMvPos.nContent.GetIndex())
    , bJoinNext(false)
    , bJoinPrev(false)
    , bMoveRange(false)
    , bMoveRedlines(false)
{
    // get StartNode from footnotes before delete!
    SwDoc* pDoc = rRange.GetDoc();
    SwTextNode* pTextNd = pDoc->GetNodes()[ nSttNode ]->GetTextNode();
    SwTextNode* pEndTextNd = pDoc->GetNodes()[ nEndNode ]->GetTextNode();

    pHistory = new SwHistory;

    if( pTextNd )
    {
        pHistory->Add( pTextNd->GetTextColl(), nSttNode, ND_TEXTNODE );
        if ( pTextNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pTextNd->GetpSwpHints(), nSttNode,
                                0, pTextNd->GetText().getLength(), false );
        }
        if( pTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), nSttNode );
    }
    if( pEndTextNd && pEndTextNd != pTextNd )
    {
        pHistory->Add( pEndTextNd->GetTextColl(), nEndNode, ND_TEXTNODE );
        if ( pEndTextNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pEndTextNd->GetpSwpHints(), nEndNode,
                                0, pEndTextNd->GetText().getLength(), false );
        }
        if( pEndTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pEndTextNd->GetpSwAttrSet(), nEndNode );
    }

    pTextNd = rMvPos.nNode.GetNode().GetTextNode();
    if (nullptr != pTextNd)
    {
        pHistory->Add( pTextNd->GetTextColl(), nMvDestNode, ND_TEXTNODE );
        if ( pTextNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pTextNd->GetpSwpHints(), nMvDestNode,
                                0, pTextNd->GetText().getLength(), false );
        }
        if( pTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), nMvDestNode );
    }

    nFootnoteStt = pHistory->Count();
    DelFootnote( rRange );

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
    , nDestSttContent(0)
    , nDestEndContent(0)
    , nInsPosContent(0)
    , bMoveRedlines(false)
{
    bMoveRange = true;
    bJoinNext = bJoinPrev = false;

    nSttContent = nEndContent = nMvDestContent = COMPLETE_STRING;

    nSttNode = rRg.aStart.GetIndex();
    nEndNode = rRg.aEnd.GetIndex();

//  DelFootnote( rRange );
// FIXME: duplication of the method body of DelFootnote below

    // is the current move from ContentArea into the special section?
    sal_uLong nContentStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( nMvDestNode < nContentStt && rRg.aStart.GetIndex() > nContentStt )
    {
        // delete all footnotes since they are undesired there
        SwPosition aPtPos( rRg.aEnd );
        SwContentNode* pCNd = rRg.aEnd.GetNode().GetContentNode();
        if( pCNd )
            aPtPos.nContent.Assign( pCNd, pCNd->Len() );
        SwPosition aMkPos( rRg.aStart );
        if( nullptr != ( pCNd = aMkPos.nNode.GetNode().GetContentNode() ))
            aMkPos.nContent.Assign( pCNd, 0 );

        DelContentIndex( aMkPos, aPtPos, nsDelContentType::DELCNT_FTN );

        if( pHistory && !pHistory->Count() )
            DELETEZ( pHistory );
    }

    nFootnoteStt = 0;
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
    nDestSttContent   = pStt->nContent.GetIndex();
    nDestEndNode    = pEnd->nNode.GetIndex();
    nDestEndContent   = pEnd->nContent.GetIndex();

    nInsPosNode     = rInsPos.nNode.GetIndex();
    nInsPosContent    = rInsPos.nContent.GetIndex();

    if( bCorrPam )
    {
        nDestSttNode--;
        nDestEndNode--;
    }

    bJoinNext = nDestSttNode != nDestEndNode &&
                pStt->nNode.GetNode().GetTextNode() &&
                pEnd->nNode.GetNode().GetTextNode();
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

    nDestSttContent = nDestEndContent = nInsPosContent = COMPLETE_STRING;
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
            SwPaM aPam( aIdx.GetNode(), nDestSttContent,
                        *pDoc->GetNodes()[ nDestEndNode ], nDestEndContent );

            // #i17764# if redlines are to be moved, we may not remove them
            // before pDoc->Move gets a chance to handle them
            if( ! bMoveRedlines )
                RemoveIdxFromRange( aPam, false );

            SwPosition aPos( *pDoc->GetNodes()[ nInsPosNode] );
            SwContentNode* pCNd = aPos.nNode.GetNode().GetContentNode();
            aPos.nContent.Assign( pCNd, nInsPosContent );

            if( pCNd->HasSwAttrSet() )
                pCNd->ResetAllAttr();

            if( pCNd->IsTextNode() && static_cast<SwTextNode*>(pCNd)->GetpSwpHints() )
                static_cast<SwTextNode*>(pCNd)->ClearSwpHintsArr( false );

            // first delete all attributes at InsertPos
            const bool bSuccess = pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos, (bMoveRedlines)
                        ? SwMoveFlags::REDLINES
                        : SwMoveFlags::DEFAULT );
            if (!bSuccess)
                break;

            aPam.Exchange();
            aPam.DeleteMark();
            if( aPam.GetNode().IsContentNode() )
                aPam.GetNode().GetContentNode()->ResetAllAttr();
            // the Pam will be dropped now
        }

        SwTextNode* pTextNd = aIdx.GetNode().GetTextNode();
        if( bJoinNext )
        {
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex(pTextNd, pTextNd->GetText().getLength())));
            }
            // Are there any Pams in the next TextNode?
            pTextNd->JoinNext();
        }

        if( bJoinPrev && pTextNd->CanJoinPrev( &aIdx ) )
        {
            // Are there any Pams in the next TextNode?
            pTextNd = aIdx.GetNode().GetTextNode();
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex( pTextNd, pTextNd->GetText().getLength())));
            }
            pTextNd->JoinNext();
        }

    } while( false );

    if( pHistory )
    {
        if( nFootnoteStt != pHistory->Count() )
            pHistory->Rollback( pDoc, nFootnoteStt );
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
    SwPaM& rPam = AddUndoRedoPaM(rContext);
    SwDoc& rDoc = rContext.GetDoc();

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
        SwPaM aPam(*rPam.GetPoint());
        SetPaM( aPam );
        SwPosition aMvPos( aIdx, SwIndex( aIdx.GetNode().GetContentNode(),
                                        nMvDestContent ));

        DelFootnote( aPam );
        RemoveIdxFromRange( aPam, false );

        aIdx = aPam.Start()->nNode;
        bool bJoinText = aIdx.GetNode().IsTextNode();

        --aIdx;
        rDoc.getIDocumentContentOperations().MoveRange( aPam, aMvPos,
            SwMoveFlags::DEFAULT );

        if( nSttNode != nEndNode && bJoinText )
        {
            ++aIdx;
            SwTextNode * pTextNd = aIdx.GetNode().GetTextNode();
            if( pTextNd && pTextNd->CanJoinNext() )
            {
                {
                    RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                            SwIndex(pTextNd, pTextNd->GetText().getLength())));
                }
                pTextNd->JoinNext();
            }
        }
        *rPam.GetPoint() = *aPam.GetPoint();
        rPam.SetMark();
        *rPam.GetMark() = *aPam.GetMark();
    }
}

void SwUndoMove::DelFootnote( const SwPaM& rRange )
{
    // is the current move from ContentArea into the special section?
    SwDoc* pDoc = rRange.GetDoc();
    sal_uLong nContentStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( nMvDestNode < nContentStt &&
        rRange.GetPoint()->nNode.GetIndex() >= nContentStt )
    {
        // delete all footnotes since they are undesired there
        DelContentIndex( *rRange.GetMark(), *rRange.GetPoint(),
                            nsDelContentType::DELCNT_FTN );

        if( pHistory && !pHistory->Count() )
        {
            delete pHistory;
            pHistory = nullptr;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
