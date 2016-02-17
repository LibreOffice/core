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
    mpRedlData( nullptr ), mpRedlSaveData( nullptr ), mnUserId( nUsrId ),
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
    {
        delete mpRedlSaveData;
        mpRedlSaveData = nullptr;
    }
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
    SwDoc& rDoc = rContext.GetDoc();
    SwPaM& rPam(AddUndoRedoPaM(rContext));

    UndoRedlineImpl(rDoc, rPam);

    if( mpRedlSaveData )
    {
        sal_uLong nEndExtra = rDoc.GetNodes().GetEndOfExtras().GetIndex();
        SetSaveData(rDoc, *mpRedlSaveData);
        if( mbHiddenRedlines )
        {
            mpRedlSaveData->clear();

            nEndExtra = rDoc.GetNodes().GetEndOfExtras().GetIndex() - nEndExtra;
            nSttNode += nEndExtra;
            nEndNode += nEndExtra;
        }
        SetPaM(rPam, true);
    }
}

void SwUndoRedline::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();
    RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));

    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    if( mpRedlSaveData && mbHiddenRedlines )
    {
        sal_uLong nEndExtra = rDoc.GetNodes().GetEndOfExtras().GetIndex();
        FillSaveData(rPam, *mpRedlSaveData, false, UNDO_REJECT_REDLINE != mnUserId );

        nEndExtra -= rDoc.GetNodes().GetEndOfExtras().GetIndex();
        nSttNode -= nEndExtra;
        nEndNode -= nEndExtra;
    }

    RedoRedlineImpl(rDoc, rPam);

    SetPaM(rPam, true);
    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
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
    const SwTextNode* pTNd;
    if( UNDO_DELETE == mnUserId &&
        nSttNode == nEndNode && nSttContent + 1 == nEndContent &&
        nullptr != (pTNd = rRange.GetNode().GetTextNode()) )
    {
        sal_Unicode const cCh = pTNd->GetText()[nSttContent];
        if( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
        {
            bCanGroup = true;
            bIsDelim = !GetAppCharClass().isLetterNumeric( pTNd->GetText(),
                                                            nSttContent );
            bIsBackspace = nSttContent == rRange.GetPoint()->nContent.GetIndex();
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
        if( rNext.nSttContent == nEndContent )
            bIsEnd = 1;
        else if( rNext.nEndContent == nSttContent )
            bIsEnd = -1;

        if( bIsEnd &&
            (( !mpRedlSaveData && !rNext.mpRedlSaveData ) ||
             ( mpRedlSaveData && rNext.mpRedlSaveData &&
                SwUndo::CanRedlineGroup( *mpRedlSaveData,
                            *rNext.mpRedlSaveData, 1 != bIsEnd )
             )))
        {
            if( 1 == bIsEnd )
                nEndContent = rNext.nEndContent;
            else
                nSttContent = rNext.nSttContent;
            bRet = true;
        }
    }
    return bRet;
}

SwUndoRedlineSort::SwUndoRedlineSort( const SwPaM& rRange,
                                    const SwSortOptions& rOpt )
    : SwUndoRedline( UNDO_SORT_TXT, rRange ),
    pOpt( new SwSortOptions( rOpt ) ),
    nSaveEndNode( nEndNode ), nOffset( 0 ), nSaveEndContent( nEndContent )
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
        OSL_ENSURE( USHRT_MAX != nFnd && nFnd+1 < (sal_uInt16)rDoc.getIDocumentRedlineAccess().GetRedlineTable().size(),
                    "could not find an Insert object" );
        ++nFnd;
        rDoc.getIDocumentRedlineAccess().GetRedlineTable()[nFnd]->Show(1, nFnd);
    }

    {
        SwPaM aTmp( *rPam.GetMark() );
        aTmp.GetMark()->nContent = 0;
        aTmp.SetMark();
        aTmp.GetPoint()->nNode = nSaveEndNode;
        aTmp.GetPoint()->nContent.Assign( aTmp.GetContentNode(), nSaveEndContent );
        rDoc.getIDocumentRedlineAccess().DeleteRedline( aTmp, true, USHRT_MAX );
    }

    rDoc.getIDocumentContentOperations().DelFullPara(rPam);

    SwPaM *const pPam = & rPam;
    pPam->DeleteMark();
    pPam->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
    SwContentNode* pCNd = pPam->GetContentNode();
    pPam->GetPoint()->nContent.Assign(pCNd, 0 );
    pPam->SetMark();

    pPam->GetPoint()->nNode += nOffsetTemp;
    pCNd = pPam->GetContentNode();
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
    SwContentNode* pCNd = pPam->GetContentNode();
    sal_Int32 nLen = pCNd->Len();
    if( nLen > nCntStt )
        nLen = nCntStt;
    pPam->GetPoint()->nContent.Assign(pCNd, nLen );
    pPam->SetMark();

    pPam->GetPoint()->nNode += nOffsetTemp;
    pCNd = pPam->GetContentNode();
    pPam->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

    SetValues( rPam );

    SetPaM( rPam );
    rPam.GetPoint()->nNode = nSaveEndNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), nSaveEndContent );
}

void SwUndoRedlineSort::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().SortText( rContext.GetRepeatPaM(), *pOpt );
}

void SwUndoRedlineSort::SetSaveRange( const SwPaM& rRange )
{
    const SwPosition& rPos = *rRange.End();
    nSaveEndNode = rPos.nNode.GetIndex();
    nSaveEndContent = rPos.nContent.GetIndex();
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
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRg ), pRedlData( nullptr ),
    pUnDel( nullptr ), pUnDel2( nullptr ), pRedlSaveData( nullptr ), bInsert( bIns )
{
    SwDoc* pDoc = rRg.GetDoc();
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        RedlineType_t eTyp = bInsert ? nsRedlineType_t::REDLINE_INSERT : nsRedlineType_t::REDLINE_DELETE;
        pRedlData = new SwRedlineData( eTyp, pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }
}

SwUndoCompDoc::SwUndoCompDoc( const SwRangeRedline& rRedl )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRedl ), pRedlData( nullptr ),
    pUnDel( nullptr ), pUnDel2( nullptr ), pRedlSaveData( nullptr ),
    // for MergeDoc the corresponding inverse is needed
    bInsert( nsRedlineType_t::REDLINE_DELETE == rRedl.GetType() )
{
    SwDoc* pDoc = rRedl.GetDoc();
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( rRedl.GetRedlineData() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRedl, *pRedlSaveData, false ))
    {
        delete pRedlSaveData;
        pRedlSaveData = nullptr;
    }
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
    SwDoc& rDoc = rContext.GetDoc();
    SwPaM& rPam(AddUndoRedoPaM(rContext));

    if( !bInsert )
    {
        // delete Redlines
        RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
        rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON));

        rDoc.getIDocumentRedlineAccess().DeleteRedline(rPam, true, USHRT_MAX);

        rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );

        // per definition Point is end (in SwUndRng!)
        SwContentNode* pCSttNd = rPam.GetContentNode(false);
        SwContentNode* pCEndNd = rPam.GetContentNode();

        // if start- and end-content is zero, then the doc-compare moves
        // complete nodes into the current doc. And then the selection
        // must be from end to start, so the delete join into the right
        // direction.
        if( !nSttContent && !nEndContent )
            rPam.Exchange();

        bool bJoinText, bJoinPrev;
        sw_GetJoinFlags(rPam, bJoinText, bJoinPrev);

        pUnDel = new SwUndoDelete(rPam, false);

        if( bJoinText )
            sw_JoinText(rPam, bJoinPrev);

        if( pCSttNd && !pCEndNd)
        {
            // #112139# Do not step behind the end of content.
            SwNode & rTmp = rPam.GetNode();
            SwNode * pEnd = rDoc.GetNodes().DocumentSectionEndNode(&rTmp);

            if (&rTmp != pEnd)
            {
                rPam.SetMark();
                ++rPam.GetPoint()->nNode;
                rPam.GetBound().nContent.Assign( nullptr, 0 );
                rPam.GetBound( false ).nContent.Assign( nullptr, 0 );
                pUnDel2 = new SwUndoDelete(rPam, true);
            }
        }
        rPam.DeleteMark();
    }
    else
    {
        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            rDoc.getIDocumentRedlineAccess().DeleteRedline(rPam, true, USHRT_MAX);

            if( pRedlSaveData )
                SetSaveData(rDoc, *pRedlSaveData);
        }
        SetPaM(rPam, true);
    }
}

void SwUndoCompDoc::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();
    SwPaM& rPam(AddUndoRedoPaM(rContext));

    if( bInsert )
    {
        if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            SwRangeRedline* pTmp = new SwRangeRedline(*pRedlData, rPam);
            static_cast<SwRedlineTable&>(rDoc.getIDocumentRedlineAccess().GetRedlineTable()).Insert( pTmp );
            pTmp->InvalidateRange();
        }
        else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
            rDoc.getIDocumentRedlineAccess().SplitRedline(rPam);
    }
    else
    {
        if( pUnDel2 )
        {
            pUnDel2->UndoImpl(rContext);
            delete pUnDel2;
            pUnDel2 = nullptr;
        }
        pUnDel->UndoImpl(rContext);
        delete pUnDel;
        pUnDel = nullptr;

        SetPaM(rPam);

        SwRangeRedline* pTmp = new SwRangeRedline(*pRedlData, rPam);
        static_cast<SwRedlineTable&>(rDoc.getIDocumentRedlineAccess().GetRedlineTable()).Insert( pTmp );
        pTmp->InvalidateRange();
    }

    SetPaM(rPam, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
