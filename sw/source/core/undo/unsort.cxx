/*************************************************************************
 *
 *  $RCSfile: unsort.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

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
#ifndef _NODE_HXX //autogen
#include <node.hxx>
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
        pTblNd->DelFrms();
        SwNodeIndex aBehindIdx( *pTblNd->EndOfSectionNode() );
        rDoc.GetNodes().GoNext( &aBehindIdx );          // Index in Cntnt, hinter der Tabelle
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
        pTblNd->MakeFrms( &aBehindIdx);
    }
    else
    {
        // Undo Text
        RemoveIdx( *rIter.pAktPam );

        // fuer die sorted Positions einen Index anlegen.
        // JP 25.11.97: Die IndexList muss aber nach SourcePosition
        //              aufsteigend sortiert aufgebaut werden
        SwUndoSortList aIdxList( (BYTE)aSortList.Count() );
        for(USHORT i=0; i < aSortList.Count(); ++i)
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
            rDoc.Move(aRg, aIdx);
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
        pTblNd->DelFrms();
        SwNodeIndex aBehindIdx( *pTblNd->EndOfSectionNode() );
        rDoc.GetNodes().GoNext( &aBehindIdx );            // Index in Cntnt, hinter der Tabelle
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

        pTblNd->MakeFrms( &aBehindIdx );
    }
    else
    {
        // Redo bei Text
        RemoveIdx( *rIter.pAktPam );

        SwUndoSortList aIdxList( (BYTE)aSortList.Count() );
        for(USHORT i=0; i < aSortList.Count(); ++i)
        {   // aktuelle Pos ist die Ausgangslage
            SwNodeIndex* pIdx = new SwNodeIndex( rDoc.GetNodes(),
                    aSortList[i]->SORT_TXT_TBL.TXT.nSource);
            aIdxList.C40_INSERT( SwNodeIndex, pIdx, i );
        }

        for(i=0; i < aSortList.Count(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i);
            SwNodeRange aRg( *aIdxList[i], 0, *aIdxList[i], 1 );
            rDoc.Move(aRg, aIdx);
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


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unsort.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.45  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.44  2000/05/09 10:04:47  jp
      Changes for Unicode

      Revision 1.43  1998/04/02 13:14:10  JP
      Redo: Undo-Flag wird schon von der EditShell abgeschaltet


      Rev 1.42   02 Apr 1998 15:14:10   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.41   13 Feb 1998 10:31:48   JP
   compiler error

      Rev 1.40   12 Feb 1998 20:14:00   JP
   ueberfluessige Code entfernt

      Rev 1.39   12 Jan 1998 16:54:46   TJ
   include

      Rev 1.38   08 Jan 1998 21:09:28   JP
   weitere Redlining Erweiterung

      Rev 1.37   19 Dec 1997 12:14:24   JP
   Undo: Redlining beachten

      Rev 1.36   25 Nov 1997 12:34:50   JP
   Bug #45167#: Undo auf Sort - die Move Reihenfolge muss beachtet werden

      Rev 1.35   03 Nov 1997 13:06:08   MA
   precomp entfernt

      Rev 1.34   29 Oct 1997 17:04:12   JP
   Bug #45167#: Redo-SortText - Source/Target vertauschen

      Rev 1.33   09 Oct 1997 15:45:40   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.32   11 Sep 1997 17:58:48   JP
   Bug #41397#: Sortieren von Tabellen - Formeln/Werte der Boxen mitnehmen

      Rev 1.31   18 Aug 1997 10:34:54   OS
   includes

      Rev 1.30   11 Jun 1997 10:44:08   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.29   29 Oct 1996 14:55:26   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.28   14 May 1996 11:35:32   JP
   jetzt der richtige fix fuer Bug #27768#

      Rev 1.27   13 May 1996 18:19:20   JP
   Bug #27768#: Redo: Code sollte der gleiche wie beim Undo sein

      Rev 1.26   24 Apr 1996 15:55:56   TRI
   WTC Anpassung

      Rev 1.25   24 Nov 1995 17:13:56   OM
   PCH->PRECOMPILED

      Rev 1.24   17 Nov 1995 10:20:40   MA
   Segmentierung

      Rev 1.23   27 Jul 1995 08:48:28   mk
   an SCC4.0.1a angepasst (MDA)

      Rev 1.22   22 Jun 1995 19:33:14   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.21   04 Mar 1995 13:31:24   MA
   unnoetiges SEXPORT entfernt.

*************************************************************************/

