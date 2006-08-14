/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unsort.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:51:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SORTOPT_HXX
#include <sortopt.hxx>
#endif
#ifndef _DOCSORT_HXX
#include <docsort.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

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


SwUndoSort::SwUndoSort( ULONG nStt, ULONG nEnd, const SwTableNode& rTblNd,
                        const SwSortOptions& rOpt, BOOL bSaveTable )
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



void SwUndoSort::Undo( SwUndoIter& rIter)
{
    SwDoc&  rDoc = rIter.GetDoc();
    if(pSortOpt->bTable)
    {
        // Undo Tabelle
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        if( pUndoTblAttr )
            pUndoTblAttr->Undo( rIter );

        SwTableNode* pTblNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();

        // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2Layout aNode2Layout( *pTblNd );
        // <--

        pTblNd->DelFrms();
        const SwTable& rTbl = pTblNd->GetTable();

        SwMovedBoxes aMovedList;
        for( USHORT i=0; i < aSortList.Count(); i++)
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
        const ULONG nIdx = pTblNd->GetIndex();
        aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(), nIdx, nIdx + 1 );
        // <--
    }
    else
    {
        // Undo Text
        RemoveIdx( *rIter.pAktPam );

        // fuer die sorted Positions einen Index anlegen.
        // JP 25.11.97: Die IndexList muss aber nach SourcePosition
        //              aufsteigend sortiert aufgebaut werden
        SwUndoSortList aIdxList( (BYTE)aSortList.Count() );
        USHORT i;

        for( i = 0; i < aSortList.Count(); ++i)
            for( USHORT ii=0; ii < aSortList.Count(); ++ii )
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
            rDoc.Move(aRg, aIdx, IDocumentContentOperations::DOC_MOVEDEFAULT);
        }
        // Indixes loeschen
        aIdxList.DeleteAndDestroy(0, aIdxList.Count());
        SetPaM( rIter, TRUE );
    }
}


void SwUndoSort::Redo( SwUndoIter& rIter)
{
    SwDoc& rDoc = rIter.GetDoc();

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
        for(USHORT i=0; i < aSortList.Count(); ++i)
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
            pUndoTblAttr->Redo( rIter );

        // Restore table frames:
        // --> FME 2004-11-26 #i37739# A simple 'MakeFrms' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const ULONG nIdx = pTblNd->GetIndex();
        aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(), nIdx, nIdx + 1 );
        // <--
    }
    else
    {
        // Redo bei Text
        RemoveIdx( *rIter.pAktPam );

        SwUndoSortList aIdxList( (BYTE)aSortList.Count() );
        USHORT i;

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
            rDoc.Move(aRg, aIdx, IDocumentContentOperations::DOC_MOVEDEFAULT);
        }
        // Indixes loeschen
        aIdxList.DeleteAndDestroy(0, aIdxList.Count());
        SetPaM( rIter, TRUE );
        const SwTxtNode* pTNd = rIter.pAktPam->GetNode()->GetTxtNode();
        if( pTNd )
            rIter.pAktPam->GetPoint()->nContent = pTNd->GetTxt().Len();
    }
}


void SwUndoSort::Repeat(SwUndoIter& rIter)
{
    if(!pSortOpt->bTable)
    {
        SwPaM* pPam = rIter.pAktPam;
        SwDoc& rDoc = *pPam->GetDoc();

        if( !rDoc.IsIdxInTbl( pPam->Start()->nNode ) )
            rDoc.SortText(*pPam, *pSortOpt);
    }
    // Tabelle ist nicht Repeat-Faehig
    rIter.pLastUndoObj = this;
}


void SwUndoSort::RemoveIdx( SwPaM& rPam )
{
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nSttNode;

    SwCntntNode* pCNd = rPam.GetCntntNode();
    xub_StrLen nLen = pCNd->Len();
    if( nLen >= nSttCntnt )
        nLen = nSttCntnt;
    rPam.GetPoint()->nContent.Assign(pCNd, nLen );
    rPam.SetMark();

    rPam.GetPoint()->nNode = nEndNode;
    pCNd = rPam.GetCntntNode();
    nLen = pCNd->Len();
    if( nLen >= nEndCntnt )
        nLen = nEndCntnt;
    rPam.GetPoint()->nContent.Assign(pCNd, nLen );
    RemoveIdxFromRange( rPam, TRUE );
}


void SwUndoSort::Insert( const String& rOrgPos, const String& rNewPos)
{
    SwSortUndoElement* pEle = new SwSortUndoElement(rOrgPos, rNewPos);
    aSortList.C40_INSERT( SwSortUndoElement, pEle, aSortList.Count() );
}


void SwUndoSort::Insert( ULONG nOrgPos, ULONG nNewPos)
{
    SwSortUndoElement* pEle = new SwSortUndoElement(nOrgPos, nNewPos);
    aSortList.C40_INSERT( SwSortUndoElement, pEle, aSortList.Count() );
}


