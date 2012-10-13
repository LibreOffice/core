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

#include <UndoRedline.hxx>
#include <hintids.hxx>
#include <unotools/charclass.hxx>
#include <doc.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <UndoDelete.hxx>
#include <rolbck.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <sortopt.hxx>

extern void sw_JoinText( SwPaM& rPam, sal_Bool bJoinPrev );
extern void sw_GetJoinFlags( SwPaM& rPam, sal_Bool& rJoinTxt, sal_Bool& rJoinPrev );

SwUndoRedline::SwUndoRedline( SwUndoId nUsrId, const SwPaM& rRange )
    : SwUndo( UNDO_REDLINE ), SwUndRng( rRange ),
    pRedlData( 0 ), pRedlSaveData( 0 ), nUserId( nUsrId ),
    bHiddenRedlines( sal_False )
{
    // consider Redline
    SwDoc& rDoc = *rRange.GetDoc();
    if( rDoc.IsRedlineOn() )
    {
        switch( nUserId )
        {
        case UNDO_DELETE:
        case UNDO_REPLACE:
            pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_DELETE, rDoc.GetRedlineAuthor() );
            break;
        default:
            ;
        }
        SetRedlineMode( rDoc.GetRedlineMode() );
    }

    sal_uLong nEndExtra = rDoc.GetNodes().GetEndOfExtras().GetIndex();

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRange, *pRedlSaveData, sal_False,
                        UNDO_REJECT_REDLINE != nUserId ))
        delete pRedlSaveData, pRedlSaveData = 0;
    else
    {
        bHiddenRedlines = HasHiddenRedlines( *pRedlSaveData );
        if( bHiddenRedlines )           // then the NodeIndices of SwUndRng need to be corrected
        {
            nEndExtra -= rDoc.GetNodes().GetEndOfExtras().GetIndex();
            nSttNode -= nEndExtra;
            nEndNode -= nEndExtra;
        }
    }
}

SwUndoRedline::~SwUndoRedline()
{
    delete pRedlData;
    delete pRedlSaveData;
}

sal_uInt16 SwUndoRedline::GetRedlSaveCount() const
{
    return pRedlSaveData ? pRedlSaveData->size() : 0;
}

void SwUndoRedline::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM & rPam( AddUndoRedoPaM(rContext) );

    UndoRedlineImpl(*pDoc, rPam);

    if( pRedlSaveData )
    {
        sal_uLong nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex();
        SetSaveData( *pDoc, *pRedlSaveData );
        if( bHiddenRedlines )
        {
            pRedlSaveData->DeleteAndDestroyAll();

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
    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern((RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));

    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    if( pRedlSaveData && bHiddenRedlines )
    {
        sal_uLong nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex();
        FillSaveData(rPam, *pRedlSaveData, sal_False,
                        UNDO_REJECT_REDLINE != nUserId );

        nEndExtra -= pDoc->GetNodes().GetEndOfExtras().GetIndex();
        nSttNode -= nEndExtra;
        nEndNode -= nEndExtra;
    }

    RedoRedlineImpl(*pDoc, rPam);

    SetPaM(rPam, true);
    pDoc->SetRedlineMode_intern( eOld );
}

void SwUndoRedline::UndoRedlineImpl(SwDoc &, SwPaM &)
{
}

// default: remove redlines
void SwUndoRedline::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.DeleteRedline(rPam, true, USHRT_MAX);
}

SwUndoRedlineDelete::SwUndoRedlineDelete( const SwPaM& rRange, SwUndoId nUsrId )
    : SwUndoRedline( nUsrId ? nUsrId : UNDO_DELETE, rRange ),
    bCanGroup( sal_False ), bIsDelim( sal_False ), bIsBackspace( sal_False )
{
    const SwTxtNode* pTNd;
    if( UNDO_DELETE == nUserId &&
        nSttNode == nEndNode && nSttCntnt + 1 == nEndCntnt &&
        0 != (pTNd = rRange.GetNode()->GetTxtNode()) )
    {
        sal_Unicode cCh = pTNd->GetTxt().GetChar( nSttCntnt );
        if( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
        {
            bCanGroup = sal_True;
            bIsDelim = !GetAppCharClass().isLetterNumeric( pTNd->GetTxt(),
                                                            nSttCntnt );
            bIsBackspace = nSttCntnt == rRange.GetPoint()->nContent.GetIndex();
        }
    }

    bCacheComment = false;
}

void SwUndoRedlineDelete::UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.DeleteRedline(rPam, true, USHRT_MAX);
}

void SwUndoRedlineDelete::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    if (rPam.GetPoint() != rPam.GetMark())
    {
        rDoc.AppendRedline( new SwRedline(*pRedlData, rPam), sal_False );
    }
}

sal_Bool SwUndoRedlineDelete::CanGrouping( const SwUndoRedlineDelete& rNext )
{
    sal_Bool bRet = sal_False;
    if( UNDO_DELETE == nUserId && nUserId == rNext.nUserId &&
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
            (( !pRedlSaveData && !rNext.pRedlSaveData ) ||
             ( pRedlSaveData && rNext.pRedlSaveData &&
                SwUndo::CanRedlineGroup( *pRedlSaveData,
                            *rNext.pRedlSaveData, 1 != bIsEnd )
             )))
        {
            if( 1 == bIsEnd )
                nEndCntnt = rNext.nEndCntnt;
            else
                nSttCntnt = rNext.nSttCntnt;
            bRet = sal_True;
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

    if( 0 == ( nsRedlineMode_t::REDLINE_SHOW_DELETE & rDoc.GetRedlineMode()) )
    {
        // Search both Redline objects and make them visible to make the nodes
        // consistent again. The 'delete' one is hidden, thus search for the
        // 'insert' Redline object. The former is located directly after the latter.
        sal_uInt16 nFnd = rDoc.GetRedlinePos(
                            *rDoc.GetNodes()[ nSttNode + 1 ],
                            nsRedlineType_t::REDLINE_INSERT );
        OSL_ENSURE( USHRT_MAX != nFnd && nFnd+1 < (sal_uInt16)rDoc.GetRedlineTbl().size(),
                    "could not find an Insert object" );
        ++nFnd;
        rDoc.GetRedlineTbl()[nFnd]->Show( 1 );
    }

    {
        SwPaM aTmp( *rPam.GetMark() );
        aTmp.GetMark()->nContent = 0;
        aTmp.SetMark();
        aTmp.GetPoint()->nNode = nSaveEndNode;
        aTmp.GetPoint()->nContent.Assign( aTmp.GetCntntNode(), nSaveEndCntnt );
        rDoc.DeleteRedline( aTmp, true, USHRT_MAX );
    }

    rDoc.DelFullPara(rPam);

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
    xub_StrLen nCntStt  = pStart->nContent.GetIndex();

    rDoc.SortText(rPam, *pOpt);

    pPam->DeleteMark();
    pPam->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
    SwCntntNode* pCNd = pPam->GetCntntNode();
    xub_StrLen nLen = pCNd->Len();
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
    rDoc.AcceptRedline(rPam, false);
}

void SwUndoAcceptRedline::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().AcceptRedline(rContext.GetRepeatPaM(), true);
}

SwUndoRejectRedline::SwUndoRejectRedline( const SwPaM& rRange )
    : SwUndoRedline( UNDO_REJECT_REDLINE, rRange )
{
}

void SwUndoRejectRedline::RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam)
{
    rDoc.RejectRedline(rPam, false);
}

void SwUndoRejectRedline::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().RejectRedline(rContext.GetRepeatPaM(), true);
}

SwUndoCompDoc::SwUndoCompDoc( const SwPaM& rRg, sal_Bool bIns )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRg ), pRedlData( 0 ),
    pUnDel( 0 ), pUnDel2( 0 ), pRedlSaveData( 0 ), bInsert( bIns )
{
    SwDoc* pDoc = (SwDoc*)rRg.GetDoc();
    if( pDoc->IsRedlineOn() )
    {
        RedlineType_t eTyp = bInsert ? nsRedlineType_t::REDLINE_INSERT : nsRedlineType_t::REDLINE_DELETE;
        pRedlData = new SwRedlineData( eTyp, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}

SwUndoCompDoc::SwUndoCompDoc( const SwRedline& rRedl )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRedl ), pRedlData( 0 ),
    pUnDel( 0 ), pUnDel2( 0 ), pRedlSaveData( 0 ),
    // for MergeDoc the corresponding inverse is needed
    bInsert( nsRedlineType_t::REDLINE_DELETE == rRedl.GetType() )
{
    SwDoc* pDoc = (SwDoc*)rRedl.GetDoc();
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( rRedl.GetRedlineData() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRedl, *pRedlSaveData, sal_False, sal_True ))
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
        RedlineMode_t eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern((RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON));

        pDoc->DeleteRedline( *pPam, true, USHRT_MAX );

        pDoc->SetRedlineMode_intern( eOld );

        // per definition Point is end (in SwUndRng!)
        SwCntntNode* pCSttNd = pPam->GetCntntNode( sal_False );
        SwCntntNode* pCEndNd = pPam->GetCntntNode( sal_True );

        // if start- and end-content is zero, then the doc-compare moves
        // complete nodes into the current doc. And then the selection
        // must be from end to start, so the delete join into the right
        // direction.
        if( !nSttCntnt && !nEndCntnt )
            pPam->Exchange();

        sal_Bool bJoinTxt, bJoinPrev;
        sw_GetJoinFlags( *pPam, bJoinTxt, bJoinPrev );

        pUnDel = new SwUndoDelete( *pPam, sal_False );

        if( bJoinTxt )
            sw_JoinText( *pPam, bJoinPrev );

        if( pCSttNd && !pCEndNd)
        {
            // #112139# Do not step behind the end of content.
            SwNode * pTmp = pPam->GetNode(sal_True);
            if (pTmp)
            {
                SwNode * pEnd = pDoc->GetNodes().DocumentSectionEndNode(pTmp);

                if (pTmp != pEnd)
                {
                    pPam->SetMark();
                    pPam->GetPoint()->nNode++;
                    pPam->GetBound( sal_True ).nContent.Assign( 0, 0 );
                    pPam->GetBound( sal_False ).nContent.Assign( 0, 0 );
                    pUnDel2 = new SwUndoDelete( *pPam, sal_True );
                }
            }
        }
        pPam->DeleteMark();
    }
    else
    {
        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        {
            pDoc->DeleteRedline( *pPam, true, USHRT_MAX );

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
            SwRedline* pTmp = new SwRedline( *pRedlData, *pPam );
            ((SwRedlineTbl&)pDoc->GetRedlineTbl()).Insert( pTmp );
            pTmp->InvalidateRange();
        }
        else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
                !pDoc->GetRedlineTbl().empty() )
            pDoc->SplitRedline( *pPam );
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

        SwRedline* pTmp = new SwRedline( *pRedlData, *pPam );
        ((SwRedlineTbl&)pDoc->GetRedlineTbl()).Insert( pTmp );
        if (pTmp) // #i19649#
            pTmp->InvalidateRange();
    }

    SetPaM(*pPam, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
