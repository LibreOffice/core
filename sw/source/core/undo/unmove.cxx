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
    , m_nMoveDestNode(rMvPos.GetNodeIndex())
    , m_nDestStartContent(0)
    , m_nDestEndContent(0)
    , m_nInsPosContent(0)
    , m_nMoveDestContent(rMvPos.GetContentIndex())
    , m_bJoinNext(false)
    , m_bMoveRange(false)
    , m_bMoveRedlines(false)
{
    // get StartNode from footnotes before delete!
    SwDoc& rDoc = rRange.GetDoc();
    SwTextNode* pTextNd = rDoc.GetNodes()[ m_nSttNode ]->GetTextNode();
    SwTextNode* pEndTextNd = rDoc.GetNodes()[ m_nEndNode ]->GetTextNode();

    m_pHistory.reset( new SwHistory );

    if( pTextNd )
    {
        m_pHistory->AddColl(pTextNd->GetTextColl(), m_nSttNode, SwNodeType::Text);
        if ( pTextNd->GetpSwpHints() )
        {
            m_pHistory->CopyAttr( pTextNd->GetpSwpHints(), m_nSttNode,
                                0, pTextNd->GetText().getLength(), false );
        }
        if( pTextNd->HasSwAttrSet() )
            m_pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), m_nSttNode );
    }
    if( pEndTextNd && pEndTextNd != pTextNd )
    {
        m_pHistory->AddColl(pEndTextNd->GetTextColl(), m_nEndNode, SwNodeType::Text);
        if ( pEndTextNd->GetpSwpHints() )
        {
            m_pHistory->CopyAttr( pEndTextNd->GetpSwpHints(), m_nEndNode,
                                0, pEndTextNd->GetText().getLength(), false );
        }
        if( pEndTextNd->HasSwAttrSet() )
            m_pHistory->CopyFormatAttr( *pEndTextNd->GetpSwAttrSet(), m_nEndNode );
    }

    pTextNd = rMvPos.GetNode().GetTextNode();
    if (nullptr != pTextNd)
    {
        m_pHistory->AddColl(pTextNd->GetTextColl(), m_nMoveDestNode, SwNodeType::Text);
        if ( pTextNd->GetpSwpHints() )
        {
            m_pHistory->CopyAttr( pTextNd->GetpSwpHints(), m_nMoveDestNode,
                                0, pTextNd->GetText().getLength(), false );
        }
        if( pTextNd->HasSwAttrSet() )
            m_pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), m_nMoveDestNode );
    }

    m_nFootnoteStart = m_pHistory->Count();
    DelFootnote( rRange );

    if( m_pHistory && !m_pHistory->Count() )
        m_pHistory.reset();
}

SwUndoMove::SwUndoMove( SwDoc& rDoc, const SwNodeRange& rRg,
                        const SwNode& rMvPos )
    : SwUndo(SwUndoId::MOVE, rDoc)
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
    m_bJoinNext = false;

    m_nSttContent = m_nEndContent = m_nMoveDestContent = COMPLETE_STRING;

    m_nSttNode = rRg.aStart.GetIndex();
    m_nEndNode = rRg.aEnd.GetIndex();

//  DelFootnote( rRange );
// FIXME: duplication of the method body of DelFootnote below

    // is the current move from ContentArea into the special section?
    SwNodeOffset nContentStt = rDoc.GetNodes().GetEndOfAutotext().GetIndex();
    if( m_nMoveDestNode < nContentStt && rRg.aStart.GetIndex() > nContentStt )
    {
        // delete all footnotes since they are undesired there
        SwPosition aPtPos( rRg.aEnd );
        SwContentNode* pCNd = rRg.aEnd.GetNode().GetContentNode();
        if( pCNd )
            aPtPos.SetContent( pCNd->Len() );
        SwPosition aMkPos( rRg.aStart );

        DelContentIndex( aMkPos, aPtPos, DelContentType::Ftn );

        if( m_pHistory && !m_pHistory->Count() )
            m_pHistory.reset();
    }

    m_nFootnoteStart = 0;
}

void SwUndoMove::SetDestRange( const SwNode& rStt,
                                const SwNode& rEnd,
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
    SwDoc& rDoc = rContext.GetDoc();

    // Block so that we can jump out of it
    do {
        // create index position and section based on the existing values
        SwNodeIndex aIdx( rDoc.GetNodes(), m_nDestStartNode );

        if( m_bMoveRange )
        {
            // only a move with SwRange
            SwNodeRange aRg( aIdx, aIdx );
            aRg.aEnd = m_nDestEndNode;
            aIdx = m_nInsPosNode;
            bool bSuccess = rDoc.getIDocumentContentOperations().MoveNodeRange( aRg, aIdx.GetNode(),
                    SwMoveFlags::DEFAULT );
            if (!bSuccess)
                break;
        }
        else
        {
            SwPaM aPam( aIdx.GetNode(), m_nDestStartContent,
                        *rDoc.GetNodes()[ m_nDestEndNode ], m_nDestEndContent );

            // #i17764# if redlines are to be moved, we may not remove them
            // before rDoc.Move gets a chance to handle them
            if( ! m_bMoveRedlines )
                RemoveIdxFromRange( aPam, false );

            SwPosition aPos( *rDoc.GetNodes()[ m_nInsPosNode] );
            SwContentNode* pCNd = aPos.GetNode().GetContentNode();
            aPos.SetContent( m_nInsPosContent );

            if( pCNd->HasSwAttrSet() )
                pCNd->ResetAllAttr();

            if( pCNd->IsTextNode() && static_cast<SwTextNode*>(pCNd)->GetpSwpHints() )
                static_cast<SwTextNode*>(pCNd)->ClearSwpHintsArr( false );

            // first delete all attributes at InsertPos
            const bool bSuccess = rDoc.getIDocumentContentOperations().MoveRange( aPam, aPos, m_bMoveRedlines
                        ? SwMoveFlags::REDLINES
                        : SwMoveFlags::DEFAULT );
            if (!bSuccess)
                break;

            aPam.Exchange();
            aPam.DeleteMark();
            if( aPam.GetPointNode().IsContentNode() )
                aPam.GetPointNode().GetContentNode()->ResetAllAttr();
            // the Pam will be dropped now
        }

        SwTextNode* pTextNd = aIdx.GetNode().GetTextNode();
        if( m_bJoinNext )
        {
            {
                RemoveIdxRel( aIdx.GetIndex() + 1,
                        SwPosition( aIdx, pTextNd, pTextNd->GetText().getLength()) );
            }
            // Are there any Pams in the next TextNode?
            pTextNd->JoinNext();
        }

    } while( false );

    if( m_pHistory )
    {
        if( m_nFootnoteStart != m_pHistory->Count() )
            m_pHistory->Rollback( rDoc, m_nFootnoteStart );
        m_pHistory->TmpRollback( rDoc, 0 );
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
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
        SwNodeRange aRg( rNds, m_nSttNode, m_nEndNode );
        rDoc.getIDocumentContentOperations().MoveNodeRange( aRg, aIdx.GetNode(), m_bMoveRedlines
                ? SwMoveFlags::REDLINES
                : SwMoveFlags::DEFAULT );
    }
    else
    {
        SwPaM aPam(*rPam.GetPoint());
        SetPaM( aPam );
        SwPosition aMvPos( aIdx, aIdx.GetNode().GetContentNode(), m_nMoveDestContent );

        DelFootnote( aPam );
        RemoveIdxFromRange( aPam, false );

        aIdx = aPam.Start()->GetNode();
        bool bJoinText = aIdx.GetNode().IsTextNode();

        --aIdx;
        rDoc.getIDocumentContentOperations().MoveRange( aPam, aMvPos,
            SwMoveFlags::DEFAULT );

        if( m_nSttNode != m_nEndNode && bJoinText )
        {
            ++aIdx;
            SwTextNode * pTextNd = aIdx.GetNode().GetTextNode();
            if( pTextNd && pTextNd->CanJoinNext() )
            {
                {
                    RemoveIdxRel( aIdx.GetIndex() + 1,
                        SwPosition( *pTextNd, pTextNd->GetText().getLength()) );
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
    SwDoc& rDoc = rRange.GetDoc();
    SwNodeOffset nContentStt = rDoc.GetNodes().GetEndOfAutotext().GetIndex();
    if( m_nMoveDestNode < nContentStt &&
        rRange.GetPoint()->GetNodeIndex() >= nContentStt )
    {
        // delete all footnotes since they are undesired there
        DelContentIndex( *rRange.GetMark(), *rRange.GetPoint(),
                            DelContentType::Ftn );

        if( m_pHistory && !m_pHistory->Count() )
        {
            m_pHistory.reset();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
