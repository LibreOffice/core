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

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
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
    pTextFormatColl( 0 ), pLastNdColl(0), pFrameFormats( 0 ), pRedlData( 0 ),
    bSttWasTextNd( true ), nNdDiff( 0 ), nSetPos( 0 )
{
    pHistory = new SwHistory;
    SwDoc* pDoc = rPam.GetDoc();

    SwTextNode* pTextNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
    if( pTextNd )
    {
        pTextFormatColl = pTextNd->GetTextColl();
        pHistory->CopyAttr( pTextNd->GetpSwpHints(), nSttNode,
                            0, pTextNd->GetText().getLength(), false );
        if( pTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), nSttNode );

        if( !nSttContent )    // than take the Flys along
        {
            const size_t nArrLen = pDoc->GetSpzFrameFormats()->size();
            for( size_t n = 0; n < nArrLen; ++n )
            {
                SwFrameFormat* pFormat = (*pDoc->GetSpzFrameFormats())[n];
                SwFormatAnchor const*const  pAnchor = &pFormat->GetAnchor();
                const SwPosition* pAPos = pAnchor->GetContentAnchor();
                if (pAPos &&
                    (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                     nSttNode == pAPos->nNode.GetIndex() )
                {
                    if( !pFrameFormats )
                        pFrameFormats = new std::vector<SwFrameFormat*>;
                    pFrameFormats->push_back( pFormat );
                }
            }
        }
    }
    // consider Redline
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }
}

// set destination after reading input
void SwUndoInserts::SetInsertRange( const SwPaM& rPam, bool bScanFlys,
                                    bool bSttIsTextNd )
{
    const SwPosition* pTmpPos = rPam.End();
    nEndNode = pTmpPos->nNode.GetIndex();
    nEndContent = pTmpPos->nContent.GetIndex();
    if( rPam.HasMark() )
    {
        if( pTmpPos == rPam.GetPoint() )
            pTmpPos = rPam.GetMark();
        else
            pTmpPos = rPam.GetPoint();

        nSttNode = pTmpPos->nNode.GetIndex();
        nSttContent = pTmpPos->nContent.GetIndex();

        if( !bSttIsTextNd )      // if a table selection is added ...
        {
            ++nSttNode;         // ... than the CopyPam is not fully correct
            bSttWasTextNd = false;
        }
    }

    if( bScanFlys && !nSttContent )
    {
        // than collect all new Flys
        SwDoc* pDoc = rPam.GetDoc();
        const size_t nArrLen = pDoc->GetSpzFrameFormats()->size();
        for( size_t n = 0; n < nArrLen; ++n )
        {
            SwFrameFormat* pFormat = (*pDoc->GetSpzFrameFormats())[n];
            SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetContentAnchor();
            if (pAPos &&
                (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                nSttNode == pAPos->nNode.GetIndex() )
            {
                std::vector<SwFrameFormat*>::iterator it;
                if( !pFrameFormats ||
                    pFrameFormats->end() == ( it = std::find( pFrameFormats->begin(), pFrameFormats->end(), pFormat ) ) )
                {
                    ::boost::shared_ptr<SwUndoInsLayFormat> const pFlyUndo(
                        new SwUndoInsLayFormat(pFormat, 0, 0));
                    m_FlyUndos.push_back(pFlyUndo);
                }
                else
                    pFrameFormats->erase( it );
            }
        }
        delete pFrameFormats, pFrameFormats = 0;
    }
}

SwUndoInserts::~SwUndoInserts()
{
    if (m_pUndoNodeIndex) // delete also the section from UndoNodes array
    {
        // Insert saves content in IconSection
        SwNodes& rUNds = m_pUndoNodeIndex->GetNodes();
        rUNds.Delete(*m_pUndoNodeIndex,
            rUNds.GetEndOfExtras().GetIndex() - m_pUndoNodeIndex->GetIndex());
        m_pUndoNodeIndex.reset();
    }
    delete pFrameFormats;
    delete pRedlData;
}

void SwUndoInserts::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam = & AddUndoRedoPaM(rContext);

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        pDoc->getIDocumentRedlineAccess().DeleteRedline( *pPam, true, USHRT_MAX );

    // if Point and Mark are different text nodes so a JoinNext has to be done
    bool bJoinNext = nSttNode != nEndNode &&
                pPam->GetMark()->nNode.GetNode().GetTextNode() &&
                pPam->GetPoint()->nNode.GetNode().GetTextNode();

    // Is there any content? (loading from template does not have content)
    if( nSttNode != nEndNode || nSttContent != nEndContent )
    {
        if( nSttNode != nEndNode )
        {
            SwTextNode* pTextNd = pDoc->GetNodes()[ nEndNode ]->GetTextNode();
            if (pTextNd && pTextNd->GetText().getLength() == nEndContent)
                pLastNdColl = pTextNd->GetTextColl();
        }

        RemoveIdxFromRange( *pPam, false );
        SetPaM(*pPam);

        // are there Footnotes or ContentFlyFrames in text?
        nSetPos = pHistory->Count();
        nNdDiff = pPam->GetMark()->nNode.GetIndex();
        DelContentIndex( *pPam->GetMark(), *pPam->GetPoint() );
        nNdDiff -= pPam->GetMark()->nNode.GetIndex();

        if( *pPam->GetPoint() != *pPam->GetMark() )
        {
            m_pUndoNodeIndex.reset(
                    new SwNodeIndex(pDoc->GetNodes().GetEndOfContent()));
            MoveToUndoNds(*pPam, m_pUndoNodeIndex.get());

            if( !bSttWasTextNd )
                pPam->Move( fnMoveBackward, fnGoContent );
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
    SwTextNode* pTextNode = rIdx.GetNode().GetTextNode();
    if( pTextNode )
    {
        if( !pTextFormatColl ) // if 0 than it's no TextNode -> delete
        {
            SwNodeIndex aDelIdx( rIdx );
            ++rIdx;
            SwContentNode* pCNd = rIdx.GetNode().GetContentNode();
            pPam->GetPoint()->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
            pPam->SetMark();
            pPam->DeleteMark();

            RemoveIdxRel( aDelIdx.GetIndex(), *pPam->GetPoint() );

            pDoc->GetNodes().Delete( aDelIdx, 1 );
        }
        else
        {
            if( bJoinNext && pTextNode->CanJoinNext())
            {
                {
                    RemoveIdxRel( rIdx.GetIndex()+1, SwPosition( rIdx,
                        SwIndex( pTextNode, pTextNode->GetText().getLength() )));
                }
                pTextNode->JoinNext();
            }
            // reset all text attributes in the paragraph!
            pTextNode->RstTextAttr( SwIndex(pTextNode, 0), pTextNode->Len(), 0, 0, true );

            pTextNode->ResetAllAttr();

            if( pDoc->GetTextFormatColls()->Contains( pTextFormatColl ))
                pTextFormatColl = static_cast<SwTextFormatColl*>(pTextNode->ChgFormatColl( pTextFormatColl )) ;

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
    SwContentNode* pCNd = pPam->GetContentNode();
    pPam->GetPoint()->nContent.Assign( pCNd, nSttContent );

    SwTextFormatColl* pSavTextFormatColl = pTextFormatColl;
    if( pTextFormatColl && pCNd && pCNd->IsTextNode() )
        pSavTextFormatColl = static_cast<SwTextNode*>(pCNd)->GetTextColl();

    pHistory->SetTmpEnd( nSetPos );

    // retrieve start position for rollback
    if( ( nSttNode != nEndNode || nSttContent != nEndContent ) && m_pUndoNodeIndex)
    {
        const bool bMvBkwrd = MovePtBackward( *pPam );

        // re-insert content again (first detach m_pUndoNodeIndex!)
        sal_uLong const nMvNd = m_pUndoNodeIndex->GetIndex();
        m_pUndoNodeIndex.reset();
        MoveFromUndoNds(*pDoc, nMvNd, *pPam->GetMark());
        if( bSttWasTextNd )
            MovePtForward( *pPam, bMvBkwrd );
        pPam->Exchange();
    }

    if( pDoc->GetTextFormatColls()->Contains( pTextFormatColl ))
    {
        SwTextNode* pTextNd = pPam->GetMark()->nNode.GetNode().GetTextNode();
        if( pTextNd )
            pTextNd->ChgFormatColl( pTextFormatColl );
    }
    pTextFormatColl = pSavTextFormatColl;

    if( pLastNdColl && pDoc->GetTextFormatColls()->Contains( pLastNdColl ) &&
        pPam->GetPoint()->nNode != pPam->GetMark()->nNode )
    {
        SwTextNode* pTextNd = pPam->GetPoint()->nNode.GetNode().GetTextNode();
        if( pTextNd )
            pTextNd->ChgFormatColl( pLastNdColl );
    }

    for (size_t n = m_FlyUndos.size(); 0 < n; --n)
    {
        m_FlyUndos[ n-1 ]->RedoImpl(rContext);
    }

    pHistory->Rollback( pDoc, nSetPos );

    if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
    {
        RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)( eOld & ~nsRedlineMode_t::REDLINE_IGNORE ));
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData, *pPam ), true);
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    }
    else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
            !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
        pDoc->getIDocumentRedlineAccess().SplitRedline( *pPam );
}

void SwUndoInserts::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM aPam( rContext.GetDoc().GetNodes().GetEndOfContent() );
    SetPaM( aPam );
    SwPaM & rRepeatPaM( rContext.GetRepeatPaM() );
    aPam.GetDoc()->getIDocumentContentOperations().CopyRange( aPam, *rRepeatPaM.GetPoint(), /*bCopyAll=*/false, /*bCheckPos=*/true );
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
