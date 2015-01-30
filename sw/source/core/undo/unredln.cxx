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

#include <UndoRedline.hxx>
#include <hintids.hxx>
#include <unotools/charclass.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <UndoDelete.hxx>
#include <rolbck.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <sortopt.hxx>
#include <docedt.hxx>

SwUndoRedline::SwUndoRedline( SwUndoId nUsrId, const SwPaM& rRange )
    : SwUndo( UNDO_REDLINE ), SwUndRng( rRange ),
    mpRedlData( 0 ), mpRedlSaveData( 0 ), mnUserId( nUsrId ),
    mbHiddenRedlines( false )
{
    // consider Redline
    SwDoc& rDoc = *rRange.GetDoc();
    if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        switch( mnUserId )
        {
        case UNDO_DELETE:
        case UNDO_REPLACE:
            mpRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_DELETE, rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() );
            break;
        default:
            ;
        }
        SetRedlineMode( rDoc.getIDocumentRedlineAccess().GetRedlineMode() );
    }

    sal_uLong nEndExtra = rDoc.GetNodes().GetEndOfExtras().GetIndex();

    mpRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRange, *mpRedlSaveData, false, UNDO_REJECT_REDLINE != mnUserId ))
        delete mpRedlSaveData, mpRedlSaveData = 0;
    else
    {
        mbHiddenRedlines = HasHiddenRedlines( *mpRedlSaveData );
        if( mbHiddenRedlines )           // then the NodeIndices of SwUndRng need to be corrected
        {
            nEndExtra -= rDoc.GetNodes().GetEndOfExtras().GetIndex();
            nSttNode -= nEndExtra;
            nEndNode -= nEndExtra;
        }
    }
}

SwUndoRedline::~SwUndoRedline()
{
    delete mpRedlData;
    delete mpRedlSaveData;
}

sal_uInt16 SwUndoRedline::GetRedlSaveCount() const
{
    return mpRedlSaveData ? mpRedlSaveData->size() : 0;
}

void SwUndoRedline::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam( AddUndoRedoPaM(rContext) );

    UndoRedlineImpl(*pDoc, rPam);

    if( mpRedlSaveData )
    {
        sal_uLong nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex();
        SetSaveData( *pDoc, *mpRedlSaveData );
        if( mbHiddenRedlines )
        {
            mpRedlSaveData->clear();

            nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex() - nEndExtra;
            nSttNode += nEndExtra;
            nEndNode += nEndExtra;
        }
        SetPaM(rPam, true);
    }
}

void SwUndoRedline::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));

    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    if( mpRedlSaveData && mbHiddenRedlines )
    {
        sal_uLong nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex();
        FillSaveData(rPam, *mpRedlSaveData, false, UNDO_REJECT_REDLINE != mnUserId );

        nEndExtra -= pDoc->GetNodes().GetEndOfExtras().GetIndex();
        nSttNode -= nEndExtra;
        nEndNode -= nEndExtra;
    }

    RedoRedlineImpl(*pDoc, rPam);

    SetPaM(rPam, true);
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

void SwUndoRedline::UndoRedlineImpl(SwDoc &, SwPaM &)
{
}

// default: remove redlines
void SwUndoRedline::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.getIDocumentRedlineAccess().DeleteRedline(rPam, true, USHRT_MAX);
}

SwUndoRedlineDelete::SwUndoRedlineDelete( const SwPaM& rRange, SwUndoId nUsrId )
    : SwUndoRedline( nUsrId ? nUsrId : UNDO_DELETE, rRange ),
    bCanGroup( false ), bIsDelim( false ), bIsBackspace( false )
{
    const SwTxtNode* pTNd;
    if( UNDO_DELETE == mnUserId &&
        nSttNode == nEndNode && nSttCntnt + 1 == nEndCntnt &&
        0 != (pTNd = rRange.GetNode().GetTxtNode()) )
    {
        sal_Unicode const cCh = pTNd->GetTxt()[nSttCntnt];
        if( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
        {
            bCanGroup = true;
            bIsDelim = !GetAppCharClass().isLetterNumeric( pTNd->GetTxt(),
                                                            nSttCntnt );
            bIsBackspace = nSttCntnt == rRange.GetPoint()->nContent.GetIndex();
        }
    }

    bCacheComment = false;
}

void SwUndoRedlineDelete::UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.getIDocumentRedlineAccess().DeleteRedline(rPam, true, USHRT_MAX);
}

void SwUndoRedlineDelete::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    if (rPam.GetPoint() != rPam.GetMark())
    {
        rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline(*mpRedlData, rPam), false );
    }
}

bool SwUndoRedlineDelete::CanGrouping( const SwUndoRedlineDelete& rNext )
{
    bool bRet = false;
    if( UNDO_DELETE == mnUserId && mnUserId == rNext.mnUserId &&
        bCanGroup == rNext.bCanGroup &&
        bIsDelim == rNext.bIsDelim &&
        bIsBackspace == rNext.bIsBackspace &&
        nSttNode == nEndNode &&
        rNext.nSttNode == nSttNode &&
        rNext.nEndNode == nEndNode )
    {
        int bIsEnd = 0;
        if( rNext.nSttCntnt == nEndCntnt )
            bIsEnd = 1;
        else if( rNext.nEndCntnt == nSttCntnt )
            bIsEnd = -1;

        if( bIsEnd &&
            (( !mpRedlSaveData && !rNext.mpRedlSaveData ) ||
             ( mpRedlSaveData && rNext.mpRedlSaveData &&
                SwUndo::CanRedlineGroup( *mpRedlSaveData,
                            *rNext.mpRedlSaveData, 1 != bIsEnd )
             )))
        {
            if( 1 == bIsEnd )
                nEndCntnt = rNext.nEndCntnt;
            else
                nSttCntnt = rNext.nSttCntnt;
            bRet = true;
        }
    }
    return bRet;
}

SwUndoRedlineSort::SwUndoRedlineSort( const SwPaM& rRange,
                                    const SwSortOptions& rOpt )
    : SwUndoRedline( UNDO_SORT_TXT, rRange ),
    pOpt( new SwSortOptions( rOpt ) ),
    nSaveEndNode( nEndNode ), nOffset( 0 ), nSaveEndCntnt( nEndCntnt )
{
}

SwUndoRedlineSort::~SwUndoRedlineSort()
{
    delete pOpt;
}

void SwUndoRedlineSort::UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    // rPam contains the sorted range
    // aSaveRange contains copied (i.e. original) range

    SwPosition *const pStart = rPam.Start();
    SwPosition *const pEnd   = rPam.End();

    SwNodeIndex aPrevIdx( pStart->nNode, -1 );
    sal_uLong nOffsetTemp = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();

    if( 0 == ( nsRedlineMode_t::REDLINE_SHOW_DELETE & rDoc.getIDocumentRedlineAccess().GetRedlineMode()) )
    {
        // Search both Redline objects and make them visible to make the nodes
        // consistent again. The 'delete' one is hidden, thus search for the
        // 'insert' Redline object. The former is located directly after the latter.
        sal_uInt16 nFnd = rDoc.getIDocumentRedlineAccess().GetRedlinePos(
                            *rDoc.GetNodes()[ nSttNode + 1 ],
                            nsRedlineType_t::REDLINE_INSERT );
        OSL_ENSURE( USHRT_MAX != nFnd && nFnd+1 < (sal_uInt16)rDoc.getIDocumentRedlineAccess().GetRedlineTbl().size(),
                    "could not find an Insert object" );
        ++nFnd;
        rDoc.getIDocumentRedlineAccess().GetRedlineTbl()[nFnd]->Show(1, nFnd);
    }

    {
        SwPaM aTmp( *rPam.GetMark() );
        aTmp.GetMark()->nContent = 0;
        aTmp.SetMark();
        aTmp.GetPoint()->nNode = nSaveEndNode;
        aTmp.GetPoint()->nContent.Assign( aTmp.GetCntntNode(), nSaveEndCntnt );
        rDoc.getIDocumentRedlineAccess().DeleteRedline( aTmp, true, USHRT_MAX );
    }

    rDoc.getIDocumentContentOperations().DelFullPara(rPam);

    SwPaM *const pPam = & rPam;
    pPam->DeleteMark();
    pPam->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
    SwCntntNode* pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign(pCNd, 0 );
    pPam->SetMark();

    pPam->GetPoint()->nNode += nOffsetTemp;
    pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    SetValues( *pPam );

    SetPaM(rPam);
}

void SwUndoRedlineSort::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    SwPaM* pPam = &rPam;
    SwPosition* pStart = pPam->Start();
    SwPosition* pEnd   = pPam->End();

    SwNodeIndex aPrevIdx( pStart->nNode, -1 );
    sal_uLong nOffsetTemp = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
    const sal_Int32 nCntStt  = pStart->nContent.GetIndex();

    rDoc.SortText(rPam, *pOpt);

    pPam->DeleteMark();
    pPam->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
    SwCntntNode* pCNd = pPam->GetCntntNode();
    sal_Int32 nLen = pCNd->Len();
    if( nLen > nCntStt )
        nLen = nCntStt;
    pPam->GetPoint()->nContent.Assign(pCNd, nLen );
    pPam->SetMark();

    pPam->GetPoint()->nNode += nOffsetTemp;
    pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    SetValues( rPam );

    SetPaM( rPam );
    rPam.GetPoint()->nNode = nSaveEndNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nSaveEndCntnt );
}

void SwUndoRedlineSort::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().SortText( rContext.GetRepeatPaM(), *pOpt );
}

void SwUndoRedlineSort::SetSaveRange( const SwPaM& rRange )
{
    const SwPosition& rPos = *rRange.End();
    nSaveEndNode = rPos.nNode.GetIndex();
    nSaveEndCntnt = rPos.nContent.GetIndex();
}

void SwUndoRedlineSort::SetOffset( const SwNodeIndex& rIdx )
{
    nOffset = rIdx.GetIndex() - nSttNode;
}

SwUndoAcceptRedline::SwUndoAcceptRedline( const SwPaM& rRange )
    : SwUndoRedline( UNDO_ACCEPT_REDLINE, rRange )
{
}

void SwUndoAcceptRedline::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.getIDocumentRedlineAccess().AcceptRedline(rPam, false);
}

void SwUndoAcceptRedline::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().getIDocumentRedlineAccess().AcceptRedline(rContext.GetRepeatPaM(), true);
}

SwUndoRejectRedline::SwUndoRejectRedline( const SwPaM& rRange )
    : SwUndoRedline( UNDO_REJECT_REDLINE, rRange )
{
}

void SwUndoRejectRedline::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.getIDocumentRedlineAccess().RejectRedline(rPam, false);
}

void SwUndoRejectRedline::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().getIDocumentRedlineAccess().RejectRedline(rContext.GetRepeatPaM(), true);
}

SwUndoCompDoc::SwUndoCompDoc( const SwPaM& rRg, bool bIns )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRg ), pRedlData( 0 ),
    pUnDel( 0 ), pUnDel2( 0 ), pRedlSaveData( 0 ), bInsert( bIns )
{
    SwDoc* pDoc = (SwDoc*)rRg.GetDoc();
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        RedlineType_t eTyp = bInsert ? nsRedlineType_t::REDLINE_INSERT : nsRedlineType_t::REDLINE_DELETE;
        pRedlData = new SwRedlineData( eTyp, pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }
}

SwUndoCompDoc::SwUndoCompDoc( const SwRangeRedline& rRedl )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRedl ), pRedlData( 0 ),
    pUnDel( 0 ), pUnDel2( 0 ), pRedlSaveData( 0 ),
    // for MergeDoc the corresponding inverse is needed
    bInsert( nsRedlineType_t::REDLINE_DELETE == rRedl.GetType() )
{
    SwDoc* pDoc = (SwDoc*)rRedl.GetDoc();
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( rRedl.GetRedlineData() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRedl, *pRedlSaveData, false, true ))
        delete pRedlSaveData, pRedlSaveData = 0;
}

SwUndoCompDoc::~SwUndoCompDoc()
{
    delete pRedlData;
    delete pUnDel;
    delete pUnDel2;
    delete pRedlSaveData;
}

void SwUndoCompDoc::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam( & AddUndoRedoPaM(rContext) );

    if( !bInsert )
    {
        // delete Redlines
        RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON));

        pDoc->getIDocumentRedlineAccess().DeleteRedline( *pPam, true, USHRT_MAX );

        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );

        // per definition Point is end (in SwUndRng!)
        SwCntntNode* pCSttNd = pPam->GetCntntNode( false );
        SwCntntNode* pCEndNd = pPam->GetCntntNode( true );

        // if start- and end-content is zero, then the doc-compare moves
        // complete nodes into the current doc. And then the selection
        // must be from end to start, so the delete join into the right
        // direction.
        if( !nSttCntnt && !nEndCntnt )
            pPam->Exchange();

        bool bJoinTxt, bJoinPrev;
        sw_GetJoinFlags( *pPam, bJoinTxt, bJoinPrev );

        pUnDel = new SwUndoDelete( *pPam, false );

        if( bJoinTxt )
            sw_JoinText( *pPam, bJoinPrev );

        if( pCSttNd && !pCEndNd)
        {
            // #112139# Do not step behind the end of content.
            SwNode & rTmp = pPam->GetNode(true);
            SwNode * pEnd = pDoc->GetNodes().DocumentSectionEndNode(&rTmp);

            if (&rTmp != pEnd)
            {
                pPam->SetMark();
                pPam->GetPoint()->nNode++;
                pPam->GetBound( true ).nContent.Assign( 0, 0 );
                pPam->GetBound( false ).nContent.Assign( 0, 0 );
                pUnDel2 = new SwUndoDelete( *pPam, true );
            }
        }
        pPam->DeleteMark();
    }
    else
    {
        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            pDoc->getIDocumentRedlineAccess().DeleteRedline( *pPam, true, USHRT_MAX );

            if( pRedlSaveData )
                SetSaveData( *pDoc, *pRedlSaveData );
        }
        SetPaM(*pPam, true);
    }
}

void SwUndoCompDoc::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam( & AddUndoRedoPaM(rContext) );

    if( bInsert )
    {
        if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            SwRangeRedline* pTmp = new SwRangeRedline( *pRedlData, *pPam );
            ((SwRedlineTbl&)pDoc->getIDocumentRedlineAccess().GetRedlineTbl()).Insert( pTmp );
            pTmp->InvalidateRange();
        }
        else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                !pDoc->getIDocumentRedlineAccess().GetRedlineTbl().empty() )
            pDoc->getIDocumentRedlineAccess().SplitRedline( *pPam );
    }
    else
    {
        if( pUnDel2 )
        {
            pUnDel2->UndoImpl(rContext);
            delete pUnDel2, pUnDel2 = 0;
        }
        pUnDel->UndoImpl(rContext);
        delete pUnDel, pUnDel = 0;

        SetPaM( *pPam );

        SwRangeRedline* pTmp = new SwRangeRedline( *pRedlData, *pPam );
        ((SwRedlineTbl&)pDoc->getIDocumentRedlineAccess().GetRedlineTbl()).Insert( pTmp );
        pTmp->InvalidateRange();
    }

    SetPaM(*pPam, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
