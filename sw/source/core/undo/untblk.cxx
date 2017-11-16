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
    : SwUndo( nUndoId, rPam.GetDoc() ), SwUndRng( rPam ),
    pTextFormatColl( nullptr ), pLastNdColl(nullptr), pFrameFormats( nullptr ), pRedlData( nullptr ),
    bSttWasTextNd( true ), nNdDiff( 0 ), nSetPos( 0 )
{
    pHistory.reset( new SwHistory );
    SwDoc* pDoc = rPam.GetDoc();

    SwTextNode* pTextNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
    if( pTextNd )
    {
        pTextFormatColl = pTextNd->GetTextColl();
        pHistory->CopyAttr( pTextNd->GetpSwpHints(), nSttNode,
                            0, pTextNd->GetText().getLength(), false );
        if( pTextNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pTextNd->GetpSwAttrSet(), nSttNode );

        // We may have some flys anchored to paragraph where we inserting.
        // These flys will be saved in pFrameFormats array (only flys which exist BEFORE insertion!)
        // Then in SwUndoInserts::SetInsertRange the flys saved in pFrameFormats will NOT create Undos.
        // m_FlyUndos will only be filled with newly inserted flys.

        const size_t nArrLen = pDoc->GetSpzFrameFormats()->size();
        for( size_t n = 0; n < nArrLen; ++n )
        {
            SwFrameFormat* pFormat = (*pDoc->GetSpzFrameFormats())[n];
            SwFormatAnchor const*const  pAnchor = &pFormat->GetAnchor();
            const SwPosition* pAPos = pAnchor->GetContentAnchor();
            if (pAPos &&
                (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_PARA) &&
                 nSttNode == pAPos->nNode.GetIndex() )
            {
                if( !pFrameFormats )
                    pFrameFormats = new std::vector<SwFrameFormat*>;
                pFrameFormats->push_back( pFormat );
            }
        }
    }
    // consider Redline
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineFlags( pDoc->getIDocumentRedlineAccess().GetRedlineFlags() );
    }
}

// This method does two things:
// 1. Adjusts SwUndoRng members, required for Undo.
//  Members are:
//  SwUndoRng::nSttNode - all nodes starting from this node will be deleted during Undo (in SwUndoInserts::UndoImpl)
//  SwUndoRng::nSttContent - corresponding content index in SwUndoRng::nSttNode
//  SwUndoRng::nEndNode - end node for deletion
//  SwUndoRng::nEndContent - end content index
// All these members are filled in during construction of SwUndoInserts instance, and can be adjusted using this method
//
// 2. Fills in m_FlyUndos array with flys anchored ONLY to first and last paragraphs (first == rPam.Start(), last == rPam.End())
//  Flys, anchored to any paragraph, but not first and last, are handled by DelContentIndex (see SwUndoInserts::UndoImpl) and are not stored in m_FlyUndos.

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

    // Fill m_FlyUndos with flys anchored to first and last paragraphs

    if( bScanFlys)
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
                (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_PARA) &&
                (nSttNode == pAPos->nNode.GetIndex() || nEndNode == pAPos->nNode.GetIndex()))
            {
                std::vector<SwFrameFormat*>::iterator it;
                if( !pFrameFormats ||
                    pFrameFormats->end() == ( it = std::find( pFrameFormats->begin(), pFrameFormats->end(), pFormat ) ) )
                {
                    std::shared_ptr<SwUndoInsLayFormat> const pFlyUndo(
                        new SwUndoInsLayFormat(pFormat, 0, 0));
                    m_FlyUndos.push_back(pFlyUndo);
                }
                else
                    pFrameFormats->erase( it );
            }
        }
        delete pFrameFormats;
        pFrameFormats = nullptr;
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

// Undo Insert operation
//  It's important to note that Undo stores absolute node indexes. I.e. if during insertion, you insert nodes 31 to 33,
//  during Undo nodes with indices from 31 to 33 will be deleted. Undo doesn't check that nodes 31 to 33 are the same nodes which were inserted.
//  It just deletes them.
//  This may seem as bad programming practice, but Undo actions are strongly ordered. If you change your document in some way, a new Undo action is added.
//  During Undo most recent actions will be executed first. So during execution of particular Undo action indices will be correct.
//  But storing absolute indices leads to crashes if some action in Undo fails to roll back some modifications.

//  Has following main steps:
//  1. DelContentIndex to delete footnotes, flys, bookmarks (see comment for this function)
//     Deleted flys are stored in pHistory array.
//     First and last paragraphs flys are handled later in this function! They are not deleted by DelContentIndex!
//     For flys anchored to last paragraph, DelContentIndex re-anchors them to the last paragraph that will remain after Undo.
//     This is not fully correct, as everything between nSttNode and nEndNode should be deleted (these nodes marks range of inserted nodes).
//     But due to bug in paste (probably there), during paste all flys are anchored to last paragraph (see https://bugs.documentfoundation.org/show_bug.cgi?id=94225#c38).
//     So they should be re-anchored.
//  2. MoveToUndoNds moves nodes to Undo nodes array and removes them from document.
//  3. m_FlyUndos removes flys anchored to first and last paragraph in Undo range. This array may be empty.
//  4. Lastly (starting from if(pTextNode)), text from last paragraph is joined to last remaining paragraph and FormatColl for last paragraph is restored.
//     Format coll for last paragraph is removed during execution of UndoImpl

void SwUndoInserts::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();
    SwPaM& rPam = AddUndoRedoPaM(rContext);

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
        rDoc.getIDocumentRedlineAccess().DeleteRedline(rPam, true, USHRT_MAX);

    // if Point and Mark are different text nodes so a JoinNext has to be done
    bool bJoinNext = nSttNode != nEndNode &&
                rPam.GetMark()->nNode.GetNode().GetTextNode() &&
                rPam.GetPoint()->nNode.GetNode().GetTextNode();

    // Is there any content? (loading from template does not have content)
    if( nSttNode != nEndNode || nSttContent != nEndContent )
    {
        if( nSttNode != nEndNode )
        {
            SwTextNode* pTextNd = rDoc.GetNodes()[ nEndNode ]->GetTextNode();
            if (pTextNd && pTextNd->GetText().getLength() == nEndContent)
                pLastNdColl = pTextNd->GetTextColl();
        }

        RemoveIdxFromRange(rPam, false);
        SetPaM(rPam);

        // are there Footnotes or ContentFlyFrames in text?
        nSetPos = pHistory->Count();
        nNdDiff = rPam.GetMark()->nNode.GetIndex();
        DelContentIndex(*rPam.GetMark(), *rPam.GetPoint());
        nNdDiff -= rPam.GetMark()->nNode.GetIndex();

        if( *rPam.GetPoint() != *rPam.GetMark() )
        {
            m_pUndoNodeIndex.reset(
                    new SwNodeIndex(rDoc.GetNodes().GetEndOfContent()));
            MoveToUndoNds(rPam, m_pUndoNodeIndex.get());

            if( !bSttWasTextNd )
                rPam.Move( fnMoveBackward, GoInContent );
        }
    }

    if (m_FlyUndos.size())
    {
        sal_uLong nTmp = rPam.GetPoint()->nNode.GetIndex();
        for (size_t n = m_FlyUndos.size(); 0 < n; --n)
        {
            m_FlyUndos[ n-1 ]->UndoImpl(rContext);
        }
        nNdDiff += nTmp - rPam.GetPoint()->nNode.GetIndex();
    }

    SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
    SwTextNode* pTextNode = rIdx.GetNode().GetTextNode();
    if( pTextNode )
    {
        if( !pTextFormatColl ) // if 0 than it's no TextNode -> delete
        {
            SwNodeIndex aDelIdx( rIdx );
            ++rIdx;
            SwContentNode* pCNd = rIdx.GetNode().GetContentNode();
            rPam.GetPoint()->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
            rPam.SetMark();
            rPam.DeleteMark();

            RemoveIdxRel(aDelIdx.GetIndex(), *rPam.GetPoint());

            rDoc.GetNodes().Delete( aDelIdx );
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
            pTextNode->RstTextAttr( SwIndex(pTextNode, 0), pTextNode->Len(), 0, nullptr, true );

            pTextNode->ResetAllAttr();

            if (rDoc.GetTextFormatColls()->IsAlive(pTextFormatColl))
                pTextFormatColl = static_cast<SwTextFormatColl*>(pTextNode->ChgFormatColl( pTextFormatColl )) ;

            pHistory->SetTmpEnd( nSetPos );
            pHistory->TmpRollback(&rDoc, 0, false);
        }
    }
}

// See SwUndoInserts::UndoImpl comments
// All actions here should be done in reverse order to what is done in SwUndoInserts::UndoImpl!

void SwUndoInserts::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // position cursor onto REDO section
    SwPaM& rPam(rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc* pDoc = rPam.GetDoc();
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nSttNode - nNdDiff;
    SwContentNode* pCNd = rPam.GetContentNode();
    rPam.GetPoint()->nContent.Assign( pCNd, nSttContent );

    SwTextFormatColl* pSavTextFormatColl = pTextFormatColl;
    if( pTextFormatColl && pCNd && pCNd->IsTextNode() )
        pSavTextFormatColl = static_cast<SwTextNode*>(pCNd)->GetTextColl();

    pHistory->SetTmpEnd( nSetPos );

    // retrieve start position for rollback
    if( ( nSttNode != nEndNode || nSttContent != nEndContent ) && m_pUndoNodeIndex)
    {
        const bool bMvBkwrd = MovePtBackward(rPam);

        // re-insert content again (first detach m_pUndoNodeIndex!)
        sal_uLong const nMvNd = m_pUndoNodeIndex->GetIndex();
        m_pUndoNodeIndex.reset();
        MoveFromUndoNds(*pDoc, nMvNd, *rPam.GetMark());
        if( bSttWasTextNd )
            MovePtForward(rPam, bMvBkwrd);
        rPam.Exchange();
    }

    if (pDoc->GetTextFormatColls()->IsAlive(pTextFormatColl))
    {
        SwTextNode* pTextNd = rPam.GetMark()->nNode.GetNode().GetTextNode();
        if( pTextNd )
            pTextNd->ChgFormatColl( pTextFormatColl );
    }
    pTextFormatColl = pSavTextFormatColl;

    if (pLastNdColl && pDoc->GetTextFormatColls()->IsAlive(pLastNdColl)
        && rPam.GetPoint()->nNode != rPam.GetMark()->nNode)
    {
        SwTextNode* pTextNd = rPam.GetPoint()->nNode.GetNode().GetTextNode();
        if( pTextNd )
            pTextNd->ChgFormatColl( pLastNdColl );
    }

    // tdf#108124 (10/25/2017)
    // During UNDO we call SwUndoInsLayFormat::UndoImpl in reverse order,
    //  firstly for m_FlyUndos[ m_FlyUndos.size()-1 ], etc.
    // As absolute node index of fly stored in SwUndoFlyBase::nNdPgPos we
    //  should recover from Undo in direct order (last should be recovered first)
    // During REDO we should recover Flys (Images) in direct order,
    //  firstly m_FlyUndos[0], then with m_FlyUndos[1] index, etc.

    for (size_t n = 0; m_FlyUndos.size() > n; ++n)
    {
        m_FlyUndos[n]->RedoImpl(rContext);
    }

    pHistory->Rollback( pDoc, nSetPos );

    if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
    {
        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld & ~RedlineFlags::Ignore );
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData, rPam ), true);
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }
    else if( !( RedlineFlags::Ignore & GetRedlineFlags() ) &&
            !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty() )
        pDoc->getIDocumentRedlineAccess().SplitRedline(rPam);
}

void SwUndoInserts::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM aPam( rContext.GetDoc().GetNodes().GetEndOfContent() );
    SetPaM( aPam );
    SwPaM & rRepeatPaM( rContext.GetRepeatPaM() );
    aPam.GetDoc()->getIDocumentContentOperations().CopyRange( aPam, *rRepeatPaM.GetPoint(), /*bCopyAll=*/false, /*bCheckPos=*/true );
}

SwUndoInsDoc::SwUndoInsDoc( const SwPaM& rPam )
    : SwUndoInserts( SwUndoId::INSDOKUMENT, rPam )
{
}

SwUndoCpyDoc::SwUndoCpyDoc( const SwPaM& rPam )
    : SwUndoInserts( SwUndoId::COPY, rPam )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
