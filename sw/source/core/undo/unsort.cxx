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

#include <UndoSort.hxx>
#include <doc.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <UndoTable.hxx>
#include <sortopt.hxx>
#include <docsort.hxx>
#include <redline.hxx>
#include <node2lay.hxx>

/*--------------------------------------------------------------------
    Description:   Undo for Sorting
 --------------------------------------------------------------------*/

SwSortUndoElement::~SwSortUndoElement()
{
    // are there string pointers saved?
    if( 0xffffffff != SORT_TXT_TBL.TXT.nKenn ) // Kenn(ung) = identifier
    {
        delete SORT_TXT_TBL.TBL.pSource;
        delete SORT_TXT_TBL.TBL.pTarget;
    }
}

SwUndoSort::SwUndoSort(const SwPaM& rRg, const SwSortOptions& rOpt)
    : SwUndo(UNDO_SORT_TXT), SwUndRng(rRg), pUndoTblAttr( 0 ),
    pRedlData( 0 )
{
    pSortOpt = new SwSortOptions(rOpt);
}

SwUndoSort::SwUndoSort( sal_uLong nStt, sal_uLong nEnd, const SwTableNode& rTblNd,
                        const SwSortOptions& rOpt, sal_Bool bSaveTable )
    : SwUndo(UNDO_SORT_TBL), pUndoTblAttr( 0 ), pRedlData( 0 )
{
    nSttNode = nStt;
    nEndNode = nEnd;
    nTblNd   = rTblNd.GetIndex();

    pSortOpt = new SwSortOptions(rOpt);
    if( bSaveTable )
        pUndoTblAttr = new SwUndoAttrTbl( rTblNd );
}

SwUndoSort::~SwUndoSort()
{
    delete pSortOpt;
    delete pUndoTblAttr;
    delete pRedlData;
}

void SwUndoSort::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if(pSortOpt->bTable)
    {
        // Undo for Table
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        if( pUndoTblAttr )
        {
            pUndoTblAttr->UndoImpl(rContext);
        }

        SwTableNode* pTblNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();

        // #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2Layout aNode2Layout( *pTblNd );

        pTblNd->DelFrms();
        const SwTable& rTbl = pTblNd->GetTable();

        SwMovedBoxes aMovedList;
        for( sal_uInt16 i=0; i < aSortList.size(); i++)
        {
            const SwTableBox* pSource = rTbl.GetTblBox(
                    *aSortList[i].SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTbl.GetTblBox(
                    *aSortList[i].SORT_TXT_TBL.TBL.pTarget );

            // move back
            MoveCell(&rDoc, pTarget, pSource,
                     USHRT_MAX != aMovedList.GetPos(pSource) );

            // store moved entry in list
            aMovedList.push_back(pTarget);
        }

        // Restore table frames:
        // #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const sal_uLong nIdx = pTblNd->GetIndex();
        aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(), nIdx, nIdx + 1 );
    }
    else
    {
        // Undo for Text
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        RemoveIdxFromRange(rPam, true);

        // create index for (sorted) positions
        // The IndexList must be created based on (asc.) sorted SourcePosition.
        SwUndoSortList aIdxList;
        sal_uInt16 i;

        for( i = 0; i < aSortList.size(); ++i)
            for( sal_uInt16 ii=0; ii < aSortList.size(); ++ii )
                if( aSortList[ii].SORT_TXT_TBL.TXT.nSource == nSttNode + i )
                {
                    SwNodeIndex* pIdx = new SwNodeIndex( rDoc.GetNodes(),
                        aSortList[ii].SORT_TXT_TBL.TXT.nTarget );
                    aIdxList.insert( aIdxList.begin() + i, pIdx );
                    break;
                }

        for(i=0; i < aSortList.size(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i );
            SwNodeRange aRg( *aIdxList[i], 0, *aIdxList[i], 1 );
            rDoc.MoveNodeRange(aRg, aIdx,
                IDocumentContentOperations::DOC_MOVEDEFAULT);
        }
        // delete indices
        for(SwUndoSortList::const_iterator it = aIdxList.begin(); it != aIdxList.end(); ++it)
            delete *it;
        aIdxList.clear();
        SetPaM(rPam, true);
    }
}

void SwUndoSort::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if(pSortOpt->bTable)
    {
        // Redo for Table
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        SwTableNode* pTblNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();

        // #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2Layout aNode2Layout( *pTblNd );

        pTblNd->DelFrms();
        const SwTable& rTbl = pTblNd->GetTable();

        SwMovedBoxes aMovedList;
        for(sal_uInt16 i=0; i < aSortList.size(); ++i)
        {
            const SwTableBox* pSource = rTbl.GetTblBox(
                    (const String&) *aSortList[i].SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTbl.GetTblBox(
                    (const String&) *aSortList[i].SORT_TXT_TBL.TBL.pTarget );

            // move back
            MoveCell(&rDoc, pSource, pTarget,
                     USHRT_MAX != aMovedList.GetPos( pTarget ) );
            // store moved entry in list
            aMovedList.push_back( pSource );
        }

        if( pUndoTblAttr )
        {
            pUndoTblAttr->RedoImpl(rContext);
        }

        // Restore table frames:
        // #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const sal_uLong nIdx = pTblNd->GetIndex();
        aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(), nIdx, nIdx + 1 );
    }
    else
    {
        // Redo for Text
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        SetPaM(rPam);
        RemoveIdxFromRange(rPam, true);

        SwUndoSortList aIdxList;
        sal_uInt16 i;

        for( i = 0; i < aSortList.size(); ++i)
        {   // current position is starting point
            SwNodeIndex* pIdx = new SwNodeIndex( rDoc.GetNodes(),
                    aSortList[i].SORT_TXT_TBL.TXT.nSource);
            aIdxList.insert( aIdxList.begin() + i, pIdx );
        }

        for(i=0; i < aSortList.size(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i);
            SwNodeRange aRg( *aIdxList[i], 0, *aIdxList[i], 1 );
            rDoc.MoveNodeRange(aRg, aIdx,
                IDocumentContentOperations::DOC_MOVEDEFAULT);
        }
        // delete indices
        for(SwUndoSortList::const_iterator it = aIdxList.begin(); it != aIdxList.end(); ++it)
            delete *it;
        aIdxList.clear();
        SetPaM(rPam, true);
        SwTxtNode const*const pTNd = rPam.GetNode()->GetTxtNode();
        if( pTNd )
        {
            rPam.GetPoint()->nContent = pTNd->GetTxt().Len();
        }
    }
}

void SwUndoSort::RepeatImpl(::sw::RepeatContext & rContext)
{
    // table not repeat capable
    if(!pSortOpt->bTable)
    {
        SwPaM *const pPam = & rContext.GetRepeatPaM();
        SwDoc& rDoc = *pPam->GetDoc();

        if( !rDoc.IsIdxInTbl( pPam->Start()->nNode ) )
            rDoc.SortText(*pPam, *pSortOpt);
    }
}

void SwUndoSort::Insert( const String& rOrgPos, const String& rNewPos)
{
    SwSortUndoElement* pEle = new SwSortUndoElement(rOrgPos, rNewPos);
    aSortList.push_back( pEle );
}

void SwUndoSort::Insert( sal_uLong nOrgPos, sal_uLong nNewPos)
{
    SwSortUndoElement* pEle = new SwSortUndoElement(nOrgPos, nNewPos);
    aSortList.push_back( pEle );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
