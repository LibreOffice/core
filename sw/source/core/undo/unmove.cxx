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
    : SwUndo( SwUndoId::MOVE, rRange.GetDoc() )
    , SwUndRng( rRange )
    , m_nDestStartNode(0)
    , m_nDestEndNode(0)
    , m_nInsPosNode(0)
    , m_nMoveDestNode(rMvPos.nNode.GetIndex())
    , m_nDestStartContent(0)
    , m_nDestEndContent(0)
    , m_nInsPosContent(0)
    , m_nMoveDestContent(rMvPos.nContent.GetIndex())
    , m_bJoinNext(false)
    , m_bJoinPrev(false)
    , m_bMoveRange(false)
    , m_bMoveRedlines(false)
{
    // get StartNode from footnotes before delete!
    SwDoc* pDoc = rRange.GetDoc();
    SwTextNode* pTextNd = pDoc->GetNodes()[ nSttNode ]->GetTextNode();
    SwTextNode* pEndTextNd = pDoc->GetNodes()[ nEndNode ]->GetTextNode();

    pHistory.reset( new SwHistory );

    if( pTextNd )
    {
        pHistory->Add( pTextNd->GetTextColl(), nSttNode, SwNodeType::Text );
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
        pHistory->Add( pEndTextNd->GetTextColl(), nEndNode, SwNodeType::Text );
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
        pHistory->Add( pTextNd->GetTextColl(), m_nMoveDestNode, SwNodeType::Text );
        if ( pTextNd->GetpSwpHints() )
        {
            pHistory->CopyAttr( pTextNd->GetpSwpHints(), m_nMoveDestNode,
                                0, pTextNd->GetText().getLength(), false );
        }
        if( pTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), m_nMoveDestNode );
    }

    m_nFootnoteStart = pHistory->Count();
    DelFootnote( rRange );

    if( pHistory && !pHistory->Count() )
        pHistory.reset();
}

SwUndoMove::SwUndoMove( SwDoc* pDoc, const SwNodeRange& rRg,
                        const SwNodeIndex& rMvPos )
    : SwUndo(SwUndoId::MOVE, pDoc)
    , m_nDestStartNode(0)
    , m_nDestEndNode(0)
    , m_nInsPosNode(0)
    , m_nMoveDestNode(rMvPos.GetIndex())
    , m_nDestStartContent(0)
    , m_nDestEndContent(0)
    , m_nInsPosContent(0)
    , m_bMoveRedlines(false)
{
    m_bMoveRange = true;
    m_bJoinNext = m_bJoinPrev = false;

    nSttContent = nEndContent = m_nMoveDestContent = COMPLETE_STRING;

    nSttNode = rRg.aStart.GetIndex();
    nEndNode = rRg.aEnd.GetIndex();

//  DelFootnote( rRange );
// FIXME: duplication of the method body of DelFootnote below

    // is the current move from ContentArea into the special section?
    sal_uLong nContentStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( m_nMoveDestNode < nContentStt && rRg.aStart.GetIndex() > nContentStt )
    {
        // delete all footnotes since they are undesired there
        SwPosition aPtPos( rRg.aEnd );
        SwContentNode* pCNd = rRg.aEnd.GetNode().GetContentNode();
        if( pCNd )
            aPtPos.nContent.Assign( pCNd, pCNd->Len() );
        SwPosition aMkPos( rRg.aStart );
        if( nullptr != ( pCNd = aMkPos.nNode.GetNode().GetContentNode() ))
            aMkPos.nContent.Assign( pCNd, 0 );

        DelContentIndex( aMkPos, aPtPos, DelContentType::Ftn );

        if( pHistory && !pHistory->Count() )
            pHistory.reset();
    }

    m_nFootnoteStart = 0;
}

void SwUndoMove::SetDestRange( const SwPaM& rRange,
                                const SwPosition& rInsPos,
                                bool bJoin, bool bCorrPam )
{
    const SwPosition *pStt = rRange.Start(),
                    *pEnd = rRange.GetPoint() == pStt
                        ? rRange.GetMark()
                        : rRange.GetPoint();

    m_nDestStartNode    = pStt->nNode.GetIndex();
    m_nDestStartContent   = pStt->nContent.GetIndex();
    m_nDestEndNode    = pEnd->nNode.GetIndex();
    m_nDestEndContent   = pEnd->nContent.GetIndex();

    m_nInsPosNode     = rInsPos.nNode.GetIndex();
    m_nInsPosContent    = rInsPos.nContent.GetIndex();

    if( bCorrPam )
    {
        m_nDestStartNode--;
        m_nDestEndNode--;
    }

    m_bJoinNext = m_nDestStartNode != m_nDestEndNode &&
                pStt->nNode.GetNode().GetTextNode() &&
                pEnd->nNode.GetNode().GetTextNode();
    m_bJoinPrev = bJoin;
}

void SwUndoMove::SetDestRange( const SwNodeIndex& rStt,
                                const SwNodeIndex& rEnd,
                                const SwNodeIndex& rInsPos )
{
    m_nDestStartNode = rStt.GetIndex();
    m_nDestEndNode = rEnd.GetIndex();
    if( m_nDestStartNode > m_nDestEndNode )
    {
        m_nDestStartNode = m_nDestEndNode;
        m_nDestEndNode = rStt.GetIndex();
    }
    m_nInsPosNode  = rInsPos.GetIndex();

    m_nDestStartContent = m_nDestEndContent = m_nInsPosContent = COMPLETE_STRING;
}

void SwUndoMove::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    // Block so that we can jump out of it
    do {
        // create index position and section based on the existing values
        SwNodeIndex aIdx( pDoc->GetNodes(), m_nDestStartNode );

        if( m_bMoveRange )
        {
            // only a move with SwRange
            SwNodeRange aRg( aIdx, aIdx );
            aRg.aEnd = m_nDestEndNode;
            aIdx = m_nInsPosNode;
            bool bSuccess = pDoc->getIDocumentContentOperations().MoveNodeRange( aRg, aIdx,
                    SwMoveFlags::DEFAULT );
            if (!bSuccess)
                break;
        }
        else
        {
            SwPaM aPam( aIdx.GetNode(), m_nDestStartContent,
                        *pDoc->GetNodes()[ m_nDestEndNode ], m_nDestEndContent );

            // #i17764# if redlines are to be moved, we may not remove them
            // before pDoc->Move gets a chance to handle them
            if( ! m_bMoveRedlines )
                RemoveIdxFromRange( aPam, false );

            SwPosition aPos( *pDoc->GetNodes()[ m_nInsPosNode] );
            SwContentNode* pCNd = aPos.nNode.GetNode().GetContentNode();
            aPos.nContent.Assign( pCNd, m_nInsPosContent );

            if( pCNd->HasSwAttrSet() )
                pCNd->ResetAllAttr();

            if( pCNd->IsTextNode() && static_cast<SwTextNode*>(pCNd)->GetpSwpHints() )
                static_cast<SwTextNode*>(pCNd)->ClearSwpHintsArr( false );

            // first delete all attributes at InsertPos
            const bool bSuccess = pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos, m_bMoveRedlines
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
        if( m_bJoinNext )
        {
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex(pTextNd, pTextNd->GetText().getLength())));
            }
            // Are there any Pams in the next TextNode?
            pTextNd->JoinNext();
        }

        if( m_bJoinPrev && pTextNd->CanJoinPrev( &aIdx ) )
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
        if( m_nFootnoteStart != pHistory->Count() )
            pHistory->Rollback( pDoc, m_nFootnoteStart );
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    // set the cursor onto Undo area
    if( !m_bMoveRange )
    {
        AddUndoRedoPaM(rContext);
    }
}

void SwUndoMove::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM& rPam = AddUndoRedoPaM(rContext);
    SwDoc& rDoc = rContext.GetDoc();

    SwNodes& rNds = rDoc.GetNodes();
    SwNodeIndex aIdx( rNds, m_nMoveDestNode );

    if( m_bMoveRange )
    {
        // only a move with SwRange
        SwNodeRange aRg( rNds, nSttNode, rNds, nEndNode );
        rDoc.getIDocumentContentOperations().MoveNodeRange( aRg, aIdx, m_bMoveRedlines
                ? SwMoveFlags::REDLINES
                : SwMoveFlags::DEFAULT );
    }
    else
    {
        SwPaM aPam(*rPam.GetPoint());
        SetPaM( aPam );
        SwPosition aMvPos( aIdx, SwIndex( aIdx.GetNode().GetContentNode(),
                                        m_nMoveDestContent ));

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
    if( m_nMoveDestNode < nContentStt &&
        rRange.GetPoint()->nNode.GetIndex() >= nContentStt )
    {
        // delete all footnotes since they are undesired there
        DelContentIndex( *rRange.GetMark(), *rRange.GetPoint(),
                            DelContentType::Ftn );

        if( pHistory && !pHistory->Count() )
        {
            pHistory.reset();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
