/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

