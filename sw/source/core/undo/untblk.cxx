/*************************************************************************
 *
 *  $RCSfile: untblk.cxx,v $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
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
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif



SwUndoInserts::SwUndoInserts( USHORT nUndoId, const SwPaM& rPam )
    : SwUndo( nUndoId ), SwUndRng( rPam ), nSetPos( 0 ),
    pTxtFmtColl( 0 ), pLastNdColl(0), pPos( 0 ), nNdDiff( 0 ),
    pFrmFmts( 0 ), pFlyUndos(0), pRedlData( 0 ), bSttWasTxtNd( TRUE )
{
    pHistory = new SwHistory;
    SwDoc* pDoc = (SwDoc*)rPam.GetDoc();

    SwTxtNode* pTxtNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        pTxtFmtColl = pTxtNd->GetTxtColl();
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode,
                            0, pTxtNd->GetTxt().Len(), FALSE );
        if( pTxtNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nSttNode );

        if( !nSttCntnt )    // dann werden Flys mitgenommen !!
        {
            USHORT nArrLen = pDoc->GetSpzFrmFmts()->Count();
            for( USHORT n = 0; n < nArrLen; ++n )
            {
                SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
                const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
                const SwPosition* pAPos;
                if ( pAnchor->GetAnchorId() == FLY_AT_CNTNT &&
                     0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
                     nSttNode == pAPos->nNode.GetIndex() )
                {
                    if( !pFrmFmts )
                        pFrmFmts = new SvPtrarr;
                    pFrmFmts->Insert( pFmt, pFrmFmts->Count() );
                }
            }
        }
    }
    // Redline beachten
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( REDLINE_INSERT, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}

// setze den Destination-Bereich nach dem Einlesen.

void SwUndoInserts::SetInsertRange( const SwPaM& rPam, BOOL bScanFlys,
                                    BOOL bSttIsTxtNd )
{
    nEndNode = rPam.End()->nNode.GetIndex();
    nEndCntnt = rPam.End()->nContent.GetIndex();
    if( rPam.HasMark() )
    {
        nSttNode = rPam.Start()->nNode.GetIndex();
        nSttCntnt = rPam.Start()->nContent.GetIndex();

        if( !bSttIsTxtNd )      // wird eine Tabellenselektion eingefuegt,
        {
            ++nSttNode;         // dann stimmt der CopyPam nicht ganz
            bSttWasTxtNd = FALSE;
        }
    }

    if( bScanFlys && !nSttCntnt )
    {
        // dann alle neuen Flys zusammen sammeln !!
        SwDoc* pDoc = (SwDoc*)rPam.GetDoc();
        pFlyUndos = new SwUndos();
        USHORT nFndPos, nArrLen = pDoc->GetSpzFrmFmts()->Count();
        for( USHORT n = 0; n < nArrLen; ++n )
        {
            SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
            const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
            const SwPosition* pAPos;
            if( pAnchor->GetAnchorId() == FLY_AT_CNTNT &&
                0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
                nSttNode == pAPos->nNode.GetIndex() )
            {
                if( !pFrmFmts ||
                    USHRT_MAX == ( nFndPos = pFrmFmts->GetPos( pFmt ) ) )
                {
                    SwUndoInsLayFmt* pFlyUndo = new SwUndoInsLayFmt( pFmt );
                    pFlyUndos->Insert( pFlyUndo, pFlyUndos->Count() );
                }
                else
                    pFrmFmts->Remove( nFndPos );
            }
        }
        delete pFrmFmts, pFrmFmts = 0;
        if( !pFlyUndos->Count() )
            delete pFlyUndos, pFlyUndos = 0;
    }
}


SwUndoInserts::~SwUndoInserts()
{
    if( pPos )      // loesche noch den Bereich aus dem UndoNodes Array
    {
        // Insert speichert den Inhalt in der IconSection
        SwNodes& rUNds = pPos->nNode.GetNodes();
        if( pPos->nContent.GetIndex() )         // nicht den gesamten Node loeschen
        {
            SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
            ASSERT( pTxtNd, "kein TextNode, aus dem geloescht werden soll" );
            pTxtNd->Erase( pPos->nContent );
            pPos->nNode++;
        }
        pPos->nContent.Assign( 0, 0 );
        rUNds.Delete( pPos->nNode, rUNds.GetEndOfExtras().GetIndex() -
                                    pPos->nNode.GetIndex() );
        delete pPos;
    }
    delete pFrmFmts;
    delete pFlyUndos;
    delete pRedlData;
}


void SwUndoInserts::Undo( SwUndoIter& rUndoIter )
{
    SwPaM * pPam = rUndoIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();
    SetPaM( rUndoIter );
    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    if( IsRedlineOn( GetRedlineMode() ))
        pDoc->DeleteRedline( *pPam );

    // sind an Point/Mark 2 unterschiedliche TextNodes, dann muss ein
    // JoinNext ausgefuehrt werden.
    BOOL bJoinNext = nSttNode != nEndNode &&
                pPam->GetMark()->nNode.GetNode().GetTxtNode() &&
                pPam->GetPoint()->nNode.GetNode().GetTxtNode();


    // gibts ueberhaupt Inhalt ? (laden von Zeichenvorlagen hat kein Inhalt!)
    if( nSttNode != nEndNode || nSttCntnt != nEndCntnt )
    {
        if( nSttNode != nEndNode )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ nEndNode ]->GetTxtNode();
            if( pTxtNd && pTxtNd->GetTxt().Len() == nEndCntnt )
                pLastNdColl = pTxtNd->GetTxtColl();
        }

        RemoveIdxFromRange( *pPam, FALSE );
        SetPaM( rUndoIter );

        // sind Fussnoten oder CntntFlyFrames im Text ??
        nSetPos = pHistory->Count();
        nNdDiff = pPam->GetMark()->nNode.GetIndex();
        DelCntntIndex( *pPam->GetMark(), *pPam->GetPoint() );
        nNdDiff -= pPam->GetMark()->nNode.GetIndex();

        if( *pPam->GetPoint() != *pPam->GetMark() )
        {
            pPos = new SwPosition( *pPam->GetPoint() );
            MoveToUndoNds( *pPam, &pPos->nNode, &pPos->nContent );

            if( !bSttWasTxtNd )
                pPam->Move( fnMoveBackward, fnGoCntnt );
        }
    }

    if( pFlyUndos )
    {
        ULONG nTmp = pPam->GetPoint()->nNode.GetIndex();
        for( USHORT n = pFlyUndos->Count(); n; )
            (*pFlyUndos)[ --n ]->Undo( rUndoIter );
        nNdDiff += nTmp - pPam->GetPoint()->nNode.GetIndex();
    }

    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    SwTxtNode* pTxtNode = rIdx.GetNode().GetTxtNode();
    if( pTxtNode )
    {
        if( !pTxtFmtColl )      // falls 0, dann war hier auch kein TextNode,
        {                       // dann muss dieser geloescht werden,
            SwNodeIndex aDelIdx( rIdx );
            rIdx++;
            SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
            xub_StrLen nCnt = 0; if( pCNd ) nCnt = pCNd->Len();
            pPam->GetPoint()->nContent.Assign( pCNd, nCnt );
            pPam->SetMark();
            pPam->DeleteMark();

            RemoveIdxRel( aDelIdx.GetIndex(), *pPam->GetPoint() );

            pDoc->GetNodes().Delete( aDelIdx, 1 );
        }
        else
        {
            pDoc->RstTxtAttr( *pPam, TRUE );
            if( bJoinNext && pTxtNode->CanJoinNext())
            {
                {
                    RemoveIdxRel( rIdx.GetIndex()+1, SwPosition( rIdx,
                            SwIndex( pTxtNode, pTxtNode->GetTxt().Len() )));
                }
                pTxtNode->JoinNext();
            }

            // setze alle Attribute im Node zurueck
// ALT          pDoc->ResetAttr( *pPam, FALSE );
            pTxtNode->SwCntntNode::ResetAllAttr();

            if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
                pTxtFmtColl = (SwTxtFmtColl*)pTxtNode->ChgFmtColl( pTxtFmtColl );

            pHistory->SetTmpEnd( nSetPos );
            pHistory->TmpRollback( pDoc, 0, FALSE );
        }
    }

    pDoc->DoUndo( bUndo );
    if( pPam != rUndoIter.pAktPam )
        delete pPam;
}

void SwUndoInserts::Redo( SwUndoIter& rUndoIter )
{
    // setze noch den Cursor auf den Redo-Bereich
    SwPaM* pPam = rUndoIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nSttNode - nNdDiff;
    SwCntntNode* pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign( pCNd, nSttCntnt );

    SwTxtFmtColl* pSavTxtFmtColl = pTxtFmtColl;
    if( pTxtFmtColl && pCNd && pCNd->IsTxtNode() )
        pSavTxtFmtColl = ((SwTxtNode*)pCNd)->GetTxtColl();

    pHistory->SetTmpEnd( nSetPos );
    pHistory->TmpRollback( pDoc, 0, FALSE );

    // alte Anfangs-Position fuers Rollback zurueckholen
    if( ( nSttNode != nEndNode || nSttCntnt != nEndCntnt ) && pPos )
    {
        BOOL bMvBkwrd = MovePtBackward( *pPam );

        // Inhalt wieder einfuegen. (erst pPos abmelden !!)
        ULONG nMvNd = pPos->nNode.GetIndex();
        xub_StrLen nMvCnt = pPos->nContent.GetIndex();
        DELETEZ( pPos );
        MoveFromUndoNds( *pDoc, nMvNd, nMvCnt, *pPam->GetMark() );
        if( bSttWasTxtNd )
            MovePtForward( *pPam, bMvBkwrd );
        pPam->Exchange();
    }

    if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
    {
        SwTxtNode* pTxtNd = pPam->GetMark()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pTxtFmtColl );
    }
    pTxtFmtColl = pSavTxtFmtColl;

    if( pLastNdColl && USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pLastNdColl ) &&
        pPam->GetPoint()->nNode != pPam->GetMark()->nNode )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pLastNdColl );
    }

    if( pFlyUndos )
        for( USHORT n = pFlyUndos->Count(); n; )
            (*pFlyUndos)[ --n ]->Redo( rUndoIter );

    pHistory->Rollback( pDoc, nSetPos );

    if( pRedlData && IsRedlineOn( GetRedlineMode() ))
    {
        SwRedlineMode eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );
        pDoc->AppendRedline( new SwRedline( *pRedlData, *pPam ));
        pDoc->SetRedlineMode_intern( eOld );
    }
    else if( !( REDLINE_IGNORE & GetRedlineMode() ) &&
            pDoc->GetRedlineTbl().Count() )
        pDoc->SplitRedline( *pPam );
}

void SwUndoInserts::Repeat( SwUndoIter& rUndoIter )
{
    if( GetId() == rUndoIter.GetLastUndoId() )
        return;

    SwPaM aPam( *rUndoIter.pAktPam->GetPoint() );
    SetPaM( aPam );
    aPam.GetDoc()->Copy( aPam, *rUndoIter.pAktPam->GetPoint() );

    rUndoIter.pLastUndoObj = this;
}


/*  */


SwUndoInsDoc::SwUndoInsDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_INSDOKUMENT, rPam )
{
}

SwUndoCpyDoc::SwUndoCpyDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_COPY, rPam )
{
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/untblk.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.52  2000/09/18 16:04:30  willem.vandorp
      OpenOffice header added.

      Revision 1.51  2000/07/03 19:25:19  jp
      Bug #67696#: RstTxtAttr must delete TOX- and RefMarks

      Revision 1.50  2000/05/09 10:04:56  jp
      Changes for Unicode

      Revision 1.49  1999/03/18 18:41:22  JP
      Bug #63715#: SetRange - nur absatzgebundene Fly einsammeln


      Rev 1.48   18 Mar 1999 19:41:22   JP
   Bug #63715#: SetRange - nur absatzgebundene Fly einsammeln

      Rev 1.47   28 Jul 1998 13:00:44   JP
   Bug #53951#: Redlines am Anfang vom Node Copy/Move Bereich sonderbehandeln

      Rev 1.46   06 May 1998 21:33:36   JP
   das einfuegen einer Tabelleselektion setzt den StartNode falsch

      Rev 1.45   20 Mar 1998 14:53:22   JP
   Bug #48632# Undo - nicht auf 0 Pointer zugreifen

      Rev 1.44   12 Feb 1998 16:51:58   JP
   kleinere Optimierungen

      Rev 1.43   29 Jan 1998 21:31:52   JP
   GetEndOfIcons ersetzt durch GetEndOfExtras, das auf GetEndOfRedlines mappt

      Rev 1.42   22 Jan 1998 20:53:12   JP
   CTOR des SwPaM umgestellt

      Rev 1.41   08 Jan 1998 20:56:04   JP
   SwDoc::GetRedlineTbl returnt jetzt eine Referenz

      Rev 1.40   19 Dec 1997 12:14:24   JP
   Undo: Redlining beachten

      Rev 1.39   18 Nov 1997 16:36:46   JP
   ResetAttr uber den Node und nicht uebers Doc rufen

      Rev 1.38   03 Nov 1997 13:06:26   MA
   precomp entfernt

      Rev 1.37   09 Oct 1997 15:45:48   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.36   03 Sep 1997 10:29:56   JP
   zusaetzliches include von docary

      Rev 1.35   18 Aug 1997 10:36:06   OS
   includes

      Rev 1.34   15 Aug 1997 12:38:00   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.33   11 Jun 1997 10:44:08   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

*************************************************************************/

