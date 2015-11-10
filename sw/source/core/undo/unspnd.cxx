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
#include "doc.hxx"
#include <IDocumentRedlineAccess.hxx>
#include "pam.hxx"
#include "swtable.hxx"
#include "ndtxt.hxx"
#include "swundo.hxx"
#include <editeng/formatbreakitem.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <UndoCore.hxx>
#include "rolbck.hxx"
#include "redline.hxx"
#include "docary.hxx"
#include <IShellCursorSupplier.hxx>

// SPLITNODE

SwUndoSplitNode::SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos,
                                    bool bChkTable )
    : SwUndo( UNDO_SPLITNODE ), pHistory( nullptr ), pRedlData( nullptr ), nNode( rPos.nNode.GetIndex() ),
        nContent( rPos.nContent.GetIndex() ),
        bTableFlag( false ), bChkTableStt( bChkTable )
{
    SwTextNode *const pTextNd = rPos.nNode.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "only for TextNode" );
    if( pTextNd->GetpSwpHints() )
    {
        pHistory = new SwHistory;
        pHistory->CopyAttr( pTextNd->GetpSwpHints(), nNode, 0,
                            pTextNd->GetText().getLength(), false );
        if( !pHistory->Count() )
            DELETEZ( pHistory );
    }
    // consider Redline
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }

    nParRsid = pTextNd->GetParRsid();
}

SwUndoSplitNode::~SwUndoSplitNode()
{
    delete pHistory;
    delete pRedlData;
}

void SwUndoSplitNode::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.DeleteMark();
    if( bTableFlag )
    {
        // than a TextNode was added directly before the current table
        SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        rIdx = nNode;
        SwTextNode* pTNd;
        SwNode* pCurrNd = pDoc->GetNodes()[ nNode + 1 ];
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
        SwTextNode * pTNd = pDoc->GetNodes()[ nNode ]->GetTextNode();
        if( pTNd )
        {
            rPam.GetPoint()->nNode = *pTNd;
            rPam.GetPoint()->nContent.Assign(pTNd, pTNd->GetText().getLength());

            if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
            {
                rPam.SetMark();
                ++rPam.GetMark()->nNode;
                rPam.GetMark()->nContent.Assign( rPam.GetMark()->
                                    nNode.GetNode().GetContentNode(), 0 );
                pDoc->getIDocumentRedlineAccess().DeleteRedline( rPam, true, USHRT_MAX );
                rPam.DeleteMark();
            }

            RemoveIdxRel( nNode+1, *rPam.GetPoint() );

            pTNd->JoinNext();
            if( pHistory )
            {
                rPam.GetPoint()->nContent = 0;
                rPam.SetMark();
                rPam.GetPoint()->nContent = pTNd->GetText().getLength();

                pDoc->RstTextAttrs( rPam, true );
                pHistory->TmpRollback( pDoc, 0, false );
            }

            pDoc->UpdateParRsid( pTNd, nParRsid );
        }
    }

    // also set the cursor onto undo section
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), nContent );
}

void SwUndoSplitNode::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.GetPoint()->nNode = nNode;
    SwTextNode * pTNd = rPam.GetNode().GetTextNode();
    OSL_ENSURE(pTNd, "SwUndoSplitNode::RedoImpl(): SwTextNode expected");
    if (pTNd)
    {
        rPam.GetPoint()->nContent.Assign( pTNd, nContent );

        SwDoc* pDoc = rPam.GetDoc();
        pDoc->getIDocumentContentOperations().SplitNode( *rPam.GetPoint(), bChkTableStt );

        if( pHistory )
            pHistory->SetTmpEnd( pHistory->Count() );

        if( ( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() )) ||
            ( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() ))
        {
            rPam.SetMark();
            if( rPam.Move( fnMoveBackward ))
            {
                if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
                {
                    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
                    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld & ~nsRedlineMode_t::REDLINE_IGNORE));
                    pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData, rPam ), true);
                    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
                }
                else
                    pDoc->getIDocumentRedlineAccess().SplitRedline( rPam );
                rPam.Exchange();
            }
            rPam.DeleteMark();
        }
    }
}

void SwUndoSplitNode::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().getIDocumentContentOperations().SplitNode(
        *rContext.GetRepeatPaM().GetPoint(), bChkTableStt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
