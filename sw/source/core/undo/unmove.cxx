/*************************************************************************
 *
 *  $RCSfile: unmove.cxx,v $
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
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

// MOVE

SwUndoMove::SwUndoMove( const SwPaM& rRange, const SwPosition& rMvPos )
    : SwUndo( UNDO_MOVE ), SwUndRng( rRange ),
    nMvDestNode( rMvPos.nNode.GetIndex() ),
    nMvDestCntnt( rMvPos.nContent.GetIndex() )
{
    bMoveRange = bJoinNext = bJoinPrev = FALSE;

    // StartNode vorm loeschen von Fussnoten besorgen!
    SwDoc* pDoc = rRange.GetDoc();
    SwTxtNode* pTxtNd = pDoc->GetNodes()[ nSttNode ]->GetTxtNode();
    SwTxtNode* pEndTxtNd = pDoc->GetNodes()[ nEndNode ]->GetTxtNode();

    pHistory = new SwHistory;

    if( pTxtNd )
    {
        pHistory->Add( pTxtNd->GetTxtColl(), nSttNode, ND_TEXTNODE );
        if( pTxtNd->GetpSwpHints() )
            pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode,
                                0, pTxtNd->GetTxt().Len(), FALSE );
        if( pTxtNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nSttNode );
    }
    if( pEndTxtNd && pEndTxtNd != pTxtNd )
    {
        pHistory->Add( pEndTxtNd->GetTxtColl(), nEndNode, ND_TEXTNODE );
        if( pEndTxtNd->GetpSwpHints() )
            pHistory->CopyAttr( pEndTxtNd->GetpSwpHints(), nEndNode,
                                0, pEndTxtNd->GetTxt().Len(), FALSE );
        if( pEndTxtNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pEndTxtNd->GetpSwAttrSet(), nEndNode );
    }

    if( 0 != (pTxtNd = rRange.GetDoc()->GetNodes()[ rMvPos.nNode ]->GetTxtNode() ))
    {
        pHistory->Add( pTxtNd->GetTxtColl(), nMvDestNode, ND_TEXTNODE );
        if( pTxtNd->GetpSwpHints() )
            pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nMvDestNode,
                                0, pTxtNd->GetTxt().Len(), FALSE );
        if( pTxtNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nMvDestNode );
    }


    nFtnStt = pHistory->Count();
    DelFtn( rRange );

    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}


SwUndoMove::SwUndoMove( SwDoc* pDoc, const SwNodeRange& rRg,
                        const SwNodeIndex& rMvPos )
    : SwUndo( UNDO_MOVE ),
    nMvDestNode( rMvPos.GetIndex() )
{
    bMoveRange = TRUE;
    bJoinNext = bJoinPrev = FALSE;

    nSttCntnt = nEndCntnt = nMvDestCntnt = STRING_MAXLEN;

    nSttNode = rRg.aStart.GetIndex();
    nEndNode = rRg.aEnd.GetIndex();

//  DelFtn( rRange );

    // wird aus dem CntntBereich in den Sonderbereich verschoben ?
    ULONG nCntntStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( nMvDestNode < nCntntStt && rRg.aStart.GetIndex() > nCntntStt )
    {
        // loesche alle Fussnoten. Diese sind dort nicht erwuenscht.
        SwPosition aPtPos( rRg.aEnd );
        SwCntntNode* pCNd = rRg.aEnd.GetNode().GetCntntNode();
        if( pCNd )
            aPtPos.nContent.Assign( pCNd, pCNd->Len() );
        SwPosition aMkPos( rRg.aStart );
        if( 0 != ( pCNd = aMkPos.nNode.GetNode().GetCntntNode() ))
            aMkPos.nContent.Assign( pCNd, 0 );

        DelCntntIndex( aMkPos, aPtPos, SwUndoSaveCntnt::DELCNT_FTN );

        if( pHistory && !pHistory->Count() )
            DELETEZ( pHistory );
    }

    nFtnStt = 0;
}



void SwUndoMove::SetDestRange( const SwPaM& rRange,
                                const SwPosition& rInsPos,
                                BOOL bJoin, BOOL bCorrPam )
{
    const SwPosition *pStt = rRange.Start(),
                    *pEnd = rRange.GetPoint() == pStt
                        ? rRange.GetMark()
                        : rRange.GetPoint();

    nDestSttNode    = pStt->nNode.GetIndex();
    nDestSttCntnt   = pStt->nContent.GetIndex();
    nDestEndNode    = pEnd->nNode.GetIndex();
    nDestEndCntnt   = pEnd->nContent.GetIndex();

    nInsPosNode     = rInsPos.nNode.GetIndex();
    nInsPosCntnt    = rInsPos.nContent.GetIndex();

    if( bCorrPam )
    {
        nDestSttNode--;
        nDestEndNode--;
    }

    bJoinNext = nDestSttNode != nDestEndNode &&
                pStt->nNode.GetNode().GetTxtNode() &&
                pEnd->nNode.GetNode().GetTxtNode();
    bJoinPrev = bJoin;
}


void SwUndoMove::SetDestRange( const SwNodeIndex& rStt,
                                const SwNodeIndex& rEnd,
                                const SwNodeIndex& rInsPos )
{
    nDestSttNode = rStt.GetIndex();
    nDestEndNode = rEnd.GetIndex();
    if( nDestSttNode > nDestEndNode )
    {
        nDestSttNode = nDestEndNode;
        nDestEndNode = rStt.GetIndex();
    }
    nInsPosNode  = rInsPos.GetIndex();

    nDestSttCntnt = nDestEndCntnt = nInsPosCntnt = STRING_MAXLEN;
}


void SwUndoMove::Undo( SwUndoIter& rUndoIter )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();
    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    // Block, damit aus diesem gesprungen werden kann
    do {
        // erzeuge aus den Werten die Insert-Position und den Bereich
        SwNodeIndex aIdx( pDoc->GetNodes(), nDestSttNode );

        if( bMoveRange )
        {
            // nur ein Move mit SwRange
            SwNodeRange aRg( aIdx, aIdx );
            aRg.aEnd = nDestEndNode;
            aIdx = nInsPosNode;
            if( !pDoc->Move( aRg, aIdx ) )
                break;
        }
        else
        {
            SwPaM aPam( aIdx.GetNode(), nDestSttCntnt,
                        *pDoc->GetNodes()[ nDestEndNode ], nDestEndCntnt );

            RemoveIdxFromRange( aPam, FALSE );

            SwPosition aPos( *pDoc->GetNodes()[ nInsPosNode] );
            SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
            aPos.nContent.Assign( pCNd, nInsPosCntnt );

            if( pCNd->GetpSwAttrSet() )
                pCNd->ResetAllAttr();

            if( pCNd->IsTxtNode() && ((SwTxtNode*)pCNd)->GetpSwpHints() )
                ((SwTxtNode*)pCNd)->ClearSwpHintsArr( FALSE, FALSE );

            // an der InsertPos erstmal alle Attribute entfernen,
            if( !pDoc->Move( aPam, aPos ) )
                break;

            aPam.Exchange();
            aPam.DeleteMark();
//          pDoc->ResetAttr( aPam, FALSE );
            if( aPam.GetNode()->IsCntntNode() )
                aPam.GetNode()->GetCntntNode()->ResetAllAttr();
            // der Pam wird jetzt aufgegeben.
        }

        SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode();
        if( bJoinNext )
        {
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex( pTxtNd, pTxtNd->GetTxt().Len() ) ) );
            }
            // sind keine Pams mehr im naechsten TextNode
            pTxtNd->JoinNext();
        }

        if( bJoinPrev && pTxtNd->CanJoinPrev( &aIdx ) )
        {
            // ?? sind keine Pams mehr im naechsten TextNode ??
            pTxtNd = aIdx.GetNode().GetTxtNode();
            {
                RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                        SwIndex( pTxtNd, pTxtNd->GetTxt().Len() ) ) );
            }
            pTxtNd->JoinNext();
        }

    } while( FALSE );

    if( pHistory )
    {
        if( nFtnStt != pHistory->Count() )
            pHistory->Rollback( pDoc, nFtnStt );
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    pDoc->DoUndo( bUndo );

    // setze noch den Cursor auf den Undo-Bereich
    if( !bMoveRange )
        SetPaM( rUndoIter );
}


void SwUndoMove::Redo( SwUndoIter& rUndoIter )
{
    SwPaM* pPam = rUndoIter.pAktPam;
    SwDoc& rDoc = *pPam->GetDoc();

    SwNodes& rNds = rDoc.GetNodes();
    SwNodeIndex aIdx( rNds, nMvDestNode );

    if( bMoveRange )
    {
        // nur ein Move mit SwRange
        SwNodeRange aRg( rNds, nSttNode, rNds, nEndNode );
        rDoc.Move( aRg, aIdx );
    }
    else
    {
        SwPaM aPam( *pPam->GetPoint() );
        SetPaM( aPam );
        SwPosition aMvPos( aIdx, SwIndex( aIdx.GetNode().GetCntntNode(),
                                        nMvDestCntnt ));

        DelFtn( aPam );
        RemoveIdxFromRange( aPam, FALSE );

        aIdx = aPam.Start()->nNode;
        BOOL bJoinTxt = aIdx.GetNode().IsTxtNode();

        aIdx--;
        rDoc.Move( aPam, aMvPos );

        if( nSttNode != nEndNode && bJoinTxt )
        {
            aIdx++;
            SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();
            if( pTxtNd && pTxtNd->CanJoinNext() )
            {
                {
                    RemoveIdxRel( aIdx.GetIndex() + 1, SwPosition( aIdx,
                            SwIndex( pTxtNd, pTxtNd->GetTxt().Len() ) ) );
                }
                pTxtNd->JoinNext();
            }
        }
        *pPam->GetPoint() = *aPam.GetPoint();
        pPam->SetMark();
        *pPam->GetMark() = *aPam.GetMark();
    }
}


void SwUndoMove::DelFtn( const SwPaM& rRange )
{
    // wird aus dem CntntBereich in den Sonderbereich verschoben ?
    SwDoc* pDoc = rRange.GetDoc();
    ULONG nCntntStt = pDoc->GetNodes().GetEndOfAutotext().GetIndex();
    if( nMvDestNode < nCntntStt &&
        rRange.GetPoint()->nNode.GetIndex() >= nCntntStt )
    {
        // loesche alle Fussnoten. Diese sind dort nicht erwuenscht.
        DelCntntIndex( *rRange.GetMark(), *rRange.GetPoint(),
                            SwUndoSaveCntnt::DELCNT_FTN );

        if( pHistory && !pHistory->Count() )
            delete pHistory, pHistory = 0;
    }
}

void SwUndoMove::AddTblMrgFlyHstry( SwHistory& rHstr )
{
    if( !pHistory )
        pHistory = new SwHistory;

    USHORT nInsPos = nFtnStt;
    nFtnStt += rHstr.Count();
    pHistory->Move( nInsPos, &rHstr );
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unmove.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.39  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.38  2000/05/09 10:04:24  jp
      Changes for Unicode

      Revision 1.37  1999/07/08 17:18:46  JP
      Bug #67507#: ClearSwpHintsArr - optional dont delete fields/hardblanks/softhyps


      Rev 1.36   08 Jul 1999 19:18:46   JP
   Bug #67507#: ClearSwpHintsArr - optional dont delete fields/hardblanks/softhyps

      Rev 1.35   02 Apr 1998 15:13:32   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.34   27 Mar 1998 15:33:24   JP
   UndoMove: GetHistory durch AddTblMrgFlyHstry ersetzt

      Rev 1.33   22 Jan 1998 20:53:10   JP
   CTOR des SwPaM umgestellt

      Rev 1.32   13 Jan 1998 21:36:06   JP
   Fussnoten in der IconSection zulassen (Redlining!)

      Rev 1.31   19 Dec 1997 12:15:34   JP
   MSG/NOTE entfernt

      Rev 1.30   18 Nov 1997 16:36:48   JP
   ResetAttr uber den Node und nicht uebers Doc rufen

      Rev 1.29   03 Nov 1997 13:06:24   MA
   precomp entfernt

      Rev 1.28   09 Oct 1997 15:45:38   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.27   17 Jun 1997 13:02:54   JP
   SetDestRange: Schnittstelle geaendert

      Rev 1.26   11 Jun 1997 10:44:48   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.25   29 May 1997 22:56:32   JP
   CopyAttr/CopyFmtAttr von SwUndo zur SwHistory verschoben

      Rev 1.24   03 Apr 1997 16:30:08   JP
   Bug #35738#: Attribute/Vorlagen richtig restaurieren

      Rev 1.23   31 Jan 1997 10:13:34   JP
   Bug #35409#: Attribute von der InsPos beim Undo wieder herstellen

      Rev 1.22   29 Oct 1996 14:54:30   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.21   04 Oct 1996 12:52:28   JP
   neu: GetHistory - legt ggf. das History-Object an

      Rev 1.20   24 Sep 1996 10:27:20   JP
   erweitert fuers Undo von Move( SwRanges ), Redo FEHLT!

      Rev 1.19   24 Nov 1995 17:13:58   OM
   PCH->PRECOMPILED

      Rev 1.18   22 Jun 1995 19:33:08   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.17   08 Feb 1995 23:52:58   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.16   24 Jan 1995 19:07:46   JP
   JoinNext/-Prev: zus. Parameter - seine Position im NodesArray

      Rev 1.15   15 Dec 1994 20:48:16   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.14   25 Oct 1994 14:50:56   MA
   PreHdr.

      Rev 1.13   02 Aug 1994 16:49:56   JP
   SwCntntNode::JoinNext/JoinPrev(): Nodes-Array nicht mehr als Parameter

      Rev 1.12   07 Jun 1994 19:48:40   JP
   UndoMove: Member nNdDiff wird nicht benoetigt

      Rev 1.11   07 Jun 1994 19:01:04   JP
   neu: wird Text in die "Sonderbereiche" verschoben und befinden sich
       in diesem Fussnoten, dann muessen sie geloescht werden.

      Rev 1.10   02 Mar 1994 19:50:28   MI
   Underscore im Namen der #pragmas

      Rev 1.9   17 Feb 1994 08:31:00   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.8   16 Feb 1994 13:17:18   MI
   Pragmas zurechtgerueckt

*************************************************************************/


