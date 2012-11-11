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

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IShellCursorSupplier.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <redline.hxx>

SwUndoInserts::SwUndoInserts( SwUndoId nUndoId, const SwPaM& rPam )
    : SwUndo( nUndoId ), SwUndRng( rPam ),
    pTxtFmtColl( 0 ), pLastNdColl(0), pFrmFmts( 0 ), pRedlData( 0 ),
    bSttWasTxtNd( sal_True ), nNdDiff( 0 ), pPos( 0 ), nSetPos( 0 )
{
    pHistory = new SwHistory;
    SwDoc* pDoc = (SwDoc*)rPam.GetDoc();

    SwTxtNode* pTxtNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        pTxtFmtColl = pTxtNd->GetTxtColl();
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode,
                            0, pTxtNd->GetTxt().Len(), false );
        if( pTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nSttNode );

        if( !nSttCntnt )    // than take the Flys along
        {
            sal_uInt16 nArrLen = pDoc->GetSpzFrmFmts()->size();
            for( sal_uInt16 n = 0; n < nArrLen; ++n )
            {
                SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
                SwFmtAnchor const*const  pAnchor = &pFmt->GetAnchor();
                const SwPosition* pAPos = pAnchor->GetCntntAnchor();
                if (pAPos &&
                    (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                     nSttNode == pAPos->nNode.GetIndex() )
                {
                    if( !pFrmFmts )
                        pFrmFmts = new std::vector<SwFrmFmt*>;
                    pFrmFmts->push_back( pFmt );
                }
            }
        }
    }
    // consider Redline
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}

// set destination after reading input
void SwUndoInserts::SetInsertRange( const SwPaM& rPam, sal_Bool bScanFlys,
                                    sal_Bool bSttIsTxtNd )
{
    const SwPosition* pTmpPos = rPam.End();
    nEndNode = pTmpPos->nNode.GetIndex();
    nEndCntnt = pTmpPos->nContent.GetIndex();
    if( rPam.HasMark() )
    {
        if( pTmpPos == rPam.GetPoint() )
            pTmpPos = rPam.GetMark();
        else
            pTmpPos = rPam.GetPoint();

        nSttNode = pTmpPos->nNode.GetIndex();
        nSttCntnt = pTmpPos->nContent.GetIndex();

        if( !bSttIsTxtNd )      // if a table selection is added ...
        {
            ++nSttNode;         // ... than the CopyPam is not fully correct
            bSttWasTxtNd = sal_False;
        }
    }

    if( bScanFlys && !nSttCntnt )
    {
        // than collect all new Flys
        SwDoc* pDoc = (SwDoc*)rPam.GetDoc();
        sal_uInt16 nArrLen = pDoc->GetSpzFrmFmts()->size();
        for( sal_uInt16 n = 0; n < nArrLen; ++n )
        {
            SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
            SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
            if (pAPos &&
                (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                nSttNode == pAPos->nNode.GetIndex() )
            {
                std::vector<SwFrmFmt*>::iterator it;
                if( !pFrmFmts ||
                    pFrmFmts->end() == ( it = std::find( pFrmFmts->begin(), pFrmFmts->end(), pFmt ) ) )
                {
                    ::boost::shared_ptr<SwUndoInsLayFmt> const pFlyUndo(
                        new SwUndoInsLayFmt(pFmt, 0, 0));
                    m_FlyUndos.push_back(pFlyUndo);
                }
                else
                    pFrmFmts->erase( it );
            }
        }
        delete pFrmFmts, pFrmFmts = 0;
    }
}

SwUndoInserts::~SwUndoInserts()
{
    if( pPos ) // delete also the section from UndoNodes array
    {
        // Insert saves content in IconSection
        SwNodes& rUNds = pPos->nNode.GetNodes();
        if( pPos->nContent.GetIndex() ) // do not delete complete Node
        {
            SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
            OSL_ENSURE( pTxtNd, "no TextNode to delete from" );
            if( pTxtNd ) // robust
            {
                pTxtNd->EraseText( pPos->nContent );
            }
            pPos->nNode++;
        }
        pPos->nContent.Assign( 0, 0 );
        rUNds.Delete( pPos->nNode, rUNds.GetEndOfExtras().GetIndex() -
                                    pPos->nNode.GetIndex() );
        delete pPos;
    }
    delete pFrmFmts;
    delete pRedlData;
}

void SwUndoInserts::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam = & AddUndoRedoPaM(rContext);

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        pDoc->DeleteRedline( *pPam, true, USHRT_MAX );

    // if Point and Mark are different text nodes so a JoinNext has to be done
    sal_Bool bJoinNext = nSttNode != nEndNode &&
                pPam->GetMark()->nNode.GetNode().GetTxtNode() &&
                pPam->GetPoint()->nNode.GetNode().GetTxtNode();


    // Is there any content? (loading from template does not have content)
    if( nSttNode != nEndNode || nSttCntnt != nEndCntnt )
    {
        if( nSttNode != nEndNode )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ nEndNode ]->GetTxtNode();
            if( pTxtNd && pTxtNd->GetTxt().Len() == nEndCntnt )
                pLastNdColl = pTxtNd->GetTxtColl();
        }

        RemoveIdxFromRange( *pPam, sal_False );
        SetPaM(*pPam);

        // are there Footnotes or CntntFlyFrames in text?
        nSetPos = pHistory->Count();
        nNdDiff = pPam->GetMark()->nNode.GetIndex();
        DelCntntIndex( *pPam->GetMark(), *pPam->GetPoint() );
        nNdDiff -= pPam->GetMark()->nNode.GetIndex();

        if( *pPam->GetPoint() != *pPam->GetMark() )
        {
            pPos = new SwPosition( *pPam->GetPoint() );
            MoveToUndoNds( *pPam, &pPos->nNode, &pPos->nContent );

            if( !bSttWasTxtNd )
                pPam->Move( fnMoveBackward, fnGoCntnt );
        }
    }

    if (m_FlyUndos.size())
    {
        sal_uLong nTmp = pPam->GetPoint()->nNode.GetIndex();
        for (size_t n = m_FlyUndos.size(); 0 < n; --n)
        {
            m_FlyUndos[ n-1 ]->UndoImpl(rContext);
        }
        nNdDiff += nTmp - pPam->GetPoint()->nNode.GetIndex();
    }

    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    SwTxtNode* pTxtNode = rIdx.GetNode().GetTxtNode();
    if( pTxtNode )
    {
        if( !pTxtFmtColl ) // if 0 than it's no TextNode -> delete
        {
            SwNodeIndex aDelIdx( rIdx );
            ++rIdx;
            SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
            xub_StrLen nCnt = 0;
            if( pCNd )
                nCnt = pCNd->Len();
            pPam->GetPoint()->nContent.Assign( pCNd, nCnt );
            pPam->SetMark();
            pPam->DeleteMark();

            RemoveIdxRel( aDelIdx.GetIndex(), *pPam->GetPoint() );

            pDoc->GetNodes().Delete( aDelIdx, 1 );
        }
        else
        {
            if( bJoinNext && pTxtNode->CanJoinNext())
            {
                {
                    RemoveIdxRel( rIdx.GetIndex()+1, SwPosition( rIdx,
                            SwIndex( pTxtNode, pTxtNode->GetTxt().Len() )));
                }
                pTxtNode->JoinNext();
            }
            // reset all text attributes in the paragraph!
            pTxtNode->RstAttr( SwIndex(pTxtNode, 0), pTxtNode->Len(),
                                0, 0, true );

            pTxtNode->ResetAllAttr();

            if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
                pTxtFmtColl = (SwTxtFmtColl*)pTxtNode->ChgFmtColl( pTxtFmtColl );

            pHistory->SetTmpEnd( nSetPos );
            pHistory->TmpRollback( pDoc, 0, false );
        }
    }
}

void SwUndoInserts::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // position cursor onto REDO section
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc* pDoc = pPam->GetDoc();
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nSttNode - nNdDiff;
    SwCntntNode* pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign( pCNd, nSttCntnt );

    SwTxtFmtColl* pSavTxtFmtColl = pTxtFmtColl;
    if( pTxtFmtColl && pCNd && pCNd->IsTxtNode() )
        pSavTxtFmtColl = ((SwTxtNode*)pCNd)->GetTxtColl();

    pHistory->SetTmpEnd( nSetPos );

    // retrieve start position for rollback
    if( ( nSttNode != nEndNode || nSttCntnt != nEndCntnt ) && pPos )
    {
        sal_Bool bMvBkwrd = MovePtBackward( *pPam );

        // re-insert content again (first detach pPos!)
        sal_uLong nMvNd = pPos->nNode.GetIndex();
        xub_StrLen nMvCnt = pPos->nContent.GetIndex();
        DELETEZ( pPos );
        MoveFromUndoNds( *pDoc, nMvNd, nMvCnt, *pPam->GetMark() );
        if( bSttWasTxtNd )
            MovePtForward( *pPam, bMvBkwrd );
        pPam->Exchange();
    }

    if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
    {
        SwTxtNode* pTxtNd = pPam->GetMark()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pTxtFmtColl );
    }
    pTxtFmtColl = pSavTxtFmtColl;

    if( pLastNdColl && USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pLastNdColl ) &&
        pPam->GetPoint()->nNode != pPam->GetMark()->nNode )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pLastNdColl );
    }

    for (size_t n = m_FlyUndos.size(); 0 < n; --n)
    {
        m_FlyUndos[ n-1 ]->RedoImpl(rContext);
    }

    pHistory->Rollback( pDoc, nSetPos );

    if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
    {
        RedlineMode_t eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern((RedlineMode_t)( eOld & ~nsRedlineMode_t::REDLINE_IGNORE ));
        pDoc->AppendRedline( new SwRedline( *pRedlData, *pPam ), true);
        pDoc->SetRedlineMode_intern( eOld );
    }
    else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
            !pDoc->GetRedlineTbl().empty() )
        pDoc->SplitRedline( *pPam );
}

void SwUndoInserts::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM aPam( rContext.GetDoc().GetNodes().GetEndOfContent() );
    SetPaM( aPam );
    SwPaM & rRepeatPaM( rContext.GetRepeatPaM() );
    aPam.GetDoc()->CopyRange( aPam, *rRepeatPaM.GetPoint(), false );
}

SwUndoInsDoc::SwUndoInsDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_INSDOKUMENT, rPam )
{
}

SwUndoCpyDoc::SwUndoCpyDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_COPY, rPam )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
