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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <UndoSort.hxx>

#include <doc.hxx>
#include <swundo.hxx>           // fuer die UndoIds
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
    Beschreibung:  Undo fuers Sorting
 --------------------------------------------------------------------*/


SV_IMPL_PTRARR(SwSortList, SwSortUndoElement*)
SV_IMPL_PTRARR(SwUndoSortList, SwNodeIndex*)


SwSortUndoElement::~SwSortUndoElement()
{
    // sind String Pointer gespeichert ??
    if( 0xffffffff != SORT_TXT_TBL.TXT.nKenn )
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
        // Undo Tabelle
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        if( pUndoTblAttr )
        {
            pUndoTblAttr->UndoImpl(rContext);
        }

        SwTableNode* pTblNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();

        // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2Layout aNode2Layout( *pTblNd );
        // <--

        pTblNd->DelFrms();
        const SwTable& rTbl = pTblNd->GetTable();

        SwMovedBoxes aMovedList;
        for( sal_uInt16 i=0; i < aSortList.Count(); i++)
        {
            const SwTableBox* pSource = rTbl.GetTblBox(
                    *aSortList[i]->SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTbl.GetTblBox(
                    *aSortList[i]->SORT_TXT_TBL.TBL.pTarget );

            // zurueckverschieben
            MoveCell(&rDoc, pTarget, pSource,
                     USHRT_MAX != aMovedList.GetPos(pSource) );

            // schon Verschobenen in der Liste merken
            aMovedList.Insert(pTarget, aMovedList.Count() );
        }

        // Restore table frames:
        // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const sal_uLong nIdx = pTblNd->GetIndex();
        aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(), nIdx, nIdx + 1 );
        // <--
    }
    else
    {
        // Undo Text
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        RemoveIdxFromRange(rPam, true);

        // fuer die sorted Positions einen Index anlegen.
        // JP 25.11.97: Die IndexList muss aber nach SourcePosition
        //              aufsteigend sortiert aufgebaut werden
        SwUndoSortList aIdxList( (sal_uInt8)aSortList.Count() );
        sal_uInt16 i;

        for( i = 0; i < aSortList.Count(); ++i)
            for( sal_uInt16 ii=0; ii < aSortList.Count(); ++ii )
                if( aSortList[ii]->SORT_TXT_TBL.TXT.nSource == nSttNode + i )
                {
                    SwNodeIndex* pIdx = new SwNodeIndex( rDoc.GetNodes(),
                        aSortList[ii]->SORT_TXT_TBL.TXT.nTarget );
                    aIdxList.C40_INSERT(SwNodeIndex, pIdx, i );
                    break;
                }

        for(i=0; i < aSortList.Count(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i );
            SwNodeRange aRg( *aIdxList[i], 0, *aIdxList[i], 1 );
            rDoc.MoveNodeRange(aRg, aIdx,
                IDocumentContentOperations::DOC_MOVEDEFAULT);
        }
        // Indixes loeschen
        aIdxList.DeleteAndDestroy(0, aIdxList.Count());
        SetPaM(rPam, true);
    }
}

void SwUndoSort::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if(pSortOpt->bTable)
    {
        // Redo bei Tabelle
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        SwTableNode* pTblNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();

        // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2Layout aNode2Layout( *pTblNd );
        // <--

        pTblNd->DelFrms();
        const SwTable& rTbl = pTblNd->GetTable();

        SwMovedBoxes aMovedList;
        for(sal_uInt16 i=0; i < aSortList.Count(); ++i)
        {
            const SwTableBox* pSource = rTbl.GetTblBox(
                    (const String&) *aSortList[i]->SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTbl.GetTblBox(
                    (const String&) *aSortList[i]->SORT_TXT_TBL.TBL.pTarget );

            // zurueckverschieben
            MoveCell(&rDoc, pSource, pTarget,
                     USHRT_MAX != aMovedList.GetPos( pTarget ) );
            // schon Verschobenen in der Liste merken
            aMovedList.Insert( pSource, aMovedList.Count() );
        }

        if( pUndoTblAttr )
        {
            pUndoTblAttr->RedoImpl(rContext);
        }

        // Restore table frames:
        // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const sal_uLong nIdx = pTblNd->GetIndex();
        aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(), nIdx, nIdx + 1 );
        // <--
    }
    else
    {
        // Redo for Text
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        SetPaM(rPam);
        RemoveIdxFromRange(rPam, true);

        SwUndoSortList aIdxList( (sal_uInt8)aSortList.Count() );
        sal_uInt16 i;

        for( i = 0; i < aSortList.Count(); ++i)
        {   // aktuelle Pos ist die Ausgangslage
            SwNodeIndex* pIdx = new SwNodeIndex( rDoc.GetNodes(),
                    aSortList[i]->SORT_TXT_TBL.TXT.nSource);
            aIdxList.C40_INSERT( SwNodeIndex, pIdx, i );
        }

        for(i=0; i < aSortList.Count(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i);
            SwNodeRange aRg( *aIdxList[i], 0, *aIdxList[i], 1 );
            rDoc.MoveNodeRange(aRg, aIdx,
                IDocumentContentOperations::DOC_MOVEDEFAULT);
        }
        // Indixes loeschen
        aIdxList.DeleteAndDestroy(0, aIdxList.Count());
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
    aSortList.C40_INSERT( SwSortUndoElement, pEle, aSortList.Count() );
}

void SwUndoSort::Insert( sal_uLong nOrgPos, sal_uLong nNewPos)
{
    SwSortUndoElement* pEle = new SwSortUndoElement(nOrgPos, nNewPos);
    aSortList.C40_INSERT( SwSortUndoElement, pEle, aSortList.Count() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */