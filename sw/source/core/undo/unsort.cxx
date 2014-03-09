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
    : SwUndo(UNDO_SORT_TXT)
    , SwUndRng(rRg)
    , pUndoTblAttr(0)
    , pRedlData(0)
    , nTblNd(0)
{
    pSortOpt = new SwSortOptions(rOpt);
}

SwUndoSort::SwUndoSort( sal_uLong nStt, sal_uLong nEnd, const SwTableNode& rTblNd,
                        const SwSortOptions& rOpt, bool bSaveTable )
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
                    *aSortList[i].SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTbl.GetTblBox(
                    *aSortList[i].SORT_TXT_TBL.TBL.pTarget );

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
            rPam.GetPoint()->nContent = pTNd->GetTxt().getLength();
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

void SwUndoSort::Insert( const OUString& rOrgPos, const OUString& rNewPos)
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
