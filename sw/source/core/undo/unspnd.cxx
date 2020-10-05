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
#include <IDocumentRedlineAccess.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <frmfmt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <IShellCursorSupplier.hxx>
#include <osl/diagnose.h>

// SPLITNODE

SwUndoSplitNode::SwUndoSplitNode( SwDoc& rDoc, const SwPosition& rPos,
                                    bool bChkTable )
    : SwUndo( SwUndoId::SPLITNODE, &rDoc ), m_nNode( rPos.nNode.GetIndex() ),
        m_nContent( rPos.nContent.GetIndex() ),
        m_bTableFlag( false ), m_bCheckTableStart( bChkTable )
{
    SwTextNode *const pTextNd = rPos.nNode.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "only for TextNode" );
    if( pTextNd->GetpSwpHints() )
    {
        m_pHistory.reset(new SwHistory);
        m_pHistory->CopyAttr(pTextNd->GetpSwpHints(), m_nNode, 0,
                            pTextNd->GetText().getLength(), false );
        if (!m_pHistory->Count())
        {
            m_pHistory.reset();
        }
    }
    // consider Redline
    if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        m_pRedlineData.reset( new SwRedlineData( RedlineType::Insert, rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() ) );
        SetRedlineFlags( rDoc.getIDocumentRedlineAccess().GetRedlineFlags() );
    }

    m_nParRsid = pTextNd->GetParRsid();
}

SwUndoSplitNode::~SwUndoSplitNode()
{
    m_pHistory.reset();
    m_pRedlineData.reset();
}

void SwUndoSplitNode::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.DeleteMark();
    if( m_bTableFlag )
    {
        // than a TextNode was added directly before the current table
        SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        rIdx = m_nNode;
        SwTextNode* pTNd;
        SwNode* pCurrNd = pDoc->GetNodes()[ m_nNode + 1 ];
        SwTableNode* pTableNd = pCurrNd->FindTableNode();
        if( pCurrNd->IsContentNode() && pTableNd &&
            nullptr != ( pTNd = pDoc->GetNodes()[ pTableNd->GetIndex()-1 ]->GetTextNode() ))
        {
            // move break attributes
            SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();
            const SfxItemSet* pNdSet = pTNd->GetpSwAttrSet();
            if( pNdSet )
            {
                const SfxPoolItem *pItem;
                if( SfxItemState::SET == pNdSet->GetItemState( RES_PAGEDESC, false,
                    &pItem ) )
                    pTableFormat->SetFormatAttr( *pItem );

                if( SfxItemState::SET == pNdSet->GetItemState( RES_BREAK, false,
                     &pItem ) )
                    pTableFormat->SetFormatAttr( *pItem );
            }

            // than delete it again
            SwNodeIndex aDelNd( *pTableNd, -1 );
            rPam.GetPoint()->nContent.Assign( static_cast<SwContentNode*>(pCurrNd), 0 );
            RemoveIdxRel( aDelNd.GetIndex(), *rPam.GetPoint() );
            pDoc->GetNodes().Delete( aDelNd );
        }
    }
    else
    {
        SwTextNode * pTNd = pDoc->GetNodes()[ m_nNode ]->GetTextNode();
        if( pTNd )
        {
            rPam.GetPoint()->nNode = *pTNd;
            rPam.GetPoint()->nContent.Assign(pTNd, pTNd->GetText().getLength());

            if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
            {
                rPam.SetMark();
                ++rPam.GetMark()->nNode;
                rPam.GetMark()->nContent.Assign( rPam.GetMark()->
                                    nNode.GetNode().GetContentNode(), 0 );
                pDoc->getIDocumentRedlineAccess().DeleteRedline( rPam, true, RedlineType::Any );
                rPam.DeleteMark();
            }

            RemoveIdxRel( m_nNode+1, *rPam.GetPoint() );

            pTNd->JoinNext();
            if (m_pHistory)
            {
                rPam.GetPoint()->nContent = 0;
                rPam.SetMark();
                rPam.GetPoint()->nContent = pTNd->GetText().getLength();

                pDoc->RstTextAttrs( rPam, true );
                m_pHistory->TmpRollback( pDoc, 0, false );
            }

            pDoc->UpdateParRsid( pTNd, m_nParRsid );
        }
    }

    // also set the cursor onto undo section
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = m_nNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), m_nContent );
}

void SwUndoSplitNode::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.GetPoint()->nNode = m_nNode;
    SwTextNode * pTNd = rPam.GetNode().GetTextNode();
    OSL_ENSURE(pTNd, "SwUndoSplitNode::RedoImpl(): SwTextNode expected");
    if (!pTNd)
        return;

    rPam.GetPoint()->nContent.Assign( pTNd, m_nContent );

    SwDoc& rDoc = rPam.GetDoc();
    rDoc.getIDocumentContentOperations().SplitNode( *rPam.GetPoint(), m_bCheckTableStart );

    if (m_pHistory)
    {
        m_pHistory->SetTmpEnd(m_pHistory->Count());
    }

    if( !(( m_pRedlineData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() )) ||
        ( !( RedlineFlags::Ignore & GetRedlineFlags() ) &&
            !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )))
        return;

    rPam.SetMark();
    if( rPam.Move( fnMoveBackward ))
    {
        if( m_pRedlineData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
        {
            RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
            rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags::Ignore);
            rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *m_pRedlineData, rPam ), true);
            rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
        }
        else
            rDoc.getIDocumentRedlineAccess().SplitRedline( rPam );
        rPam.Exchange();
    }
    rPam.DeleteMark();
}

void SwUndoSplitNode::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().getIDocumentContentOperations().SplitNode(
        *rContext.GetRepeatPaM().GetPoint(), m_bCheckTableStart );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
