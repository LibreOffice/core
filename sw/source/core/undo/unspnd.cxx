/*************************************************************************
 *
 *  $RCSfile: unspnd.cxx,v $
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

#include "doc.hxx"
#include "pam.hxx"
#include "swtable.hxx"
#include "ndtxt.hxx"
#include "swundo.hxx"           // fuer die UndoIds

#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#include "undobj.hxx"
#include "rolbck.hxx"
#include "redline.hxx"
#include "docary.hxx"


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }


//------------------------------------------------------------------

// SPLITNODE


SwUndoSplitNode::SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos,
                                    BOOL bChkTable )
    : SwUndo( UNDO_SPLITNODE ), nNode( rPos.nNode.GetIndex() ),
        nCntnt( rPos.nContent.GetIndex() ), pHistory( 0 ),
        bTblFlag( FALSE ), bChkTblStt( bChkTable ), pRedlData( 0 )
{
    SwTxtNode* pTxtNd = pDoc->GetNodes()[ rPos.nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "nur beim TextNode rufen!" );
    if( pTxtNd->GetpSwpHints() )
    {
        pHistory = new SwHistory;
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nNode, 0,
                            pTxtNd->GetTxt().Len(), FALSE );
        if( !pHistory->Count() )
            DELETEZ( pHistory );
    }
    // Redline beachten
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( REDLINE_INSERT, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}




SwUndoSplitNode::~SwUndoSplitNode()
{
    delete pHistory;
    delete pRedlData;
}



void SwUndoSplitNode::Undo( SwUndoIter& rUndoIter )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();
    SwPaM& rPam = *rUndoIter.pAktPam;
    rPam.DeleteMark();
    if( bTblFlag )
    {
        // dann wurde direkt vor der akt. Tabelle ein TextNode eingefuegt.
        SwNodeIndex& rIdx = rPam.GetPoint()->nNode;
        rIdx = nNode;
        SwTxtNode* pTNd;
        SwNode* pCurrNd = pDoc->GetNodes()[ nNode + 1 ];
        SwTableNode* pTblNd = pCurrNd->FindTableNode();
        if( pCurrNd->IsCntntNode() && pTblNd &&
            0 != ( pTNd = pDoc->GetNodes()[ pTblNd->GetIndex()-1 ]->GetTxtNode() ))
        {
            // verschiebe die BreakAttribute noch
            SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
            const SfxItemSet* pNdSet = pTNd->GetpSwAttrSet();
            if( pNdSet )
            {
                const SfxPoolItem *pItem;
                if( SFX_ITEM_SET == pNdSet->GetItemState( RES_PAGEDESC, FALSE,
                    &pItem ) )
                    pTableFmt->SetAttr( *pItem );

                if( SFX_ITEM_SET == pNdSet->GetItemState( RES_BREAK, FALSE,
                     &pItem ) )
                    pTableFmt->SetAttr( *pItem );
            }

            // dann loesche den wieder
            SwNodeIndex aDelNd( *pTblNd, -1 );
            rPam.GetPoint()->nContent.Assign( (SwCntntNode*)pCurrNd, 0 );
            RemoveIdxRel( aDelNd.GetIndex(), *rPam.GetPoint() );
            pDoc->GetNodes().Delete( aDelNd );
        }
    }
    else
    {
        SwTxtNode * pTNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
        if( pTNd )
        {
            rPam.GetPoint()->nNode = *pTNd;
            rPam.GetPoint()->nContent.Assign( pTNd, pTNd->GetTxt().Len() );

            if( IsRedlineOn( GetRedlineMode() ))
            {
                rPam.SetMark();
                rPam.GetMark()->nNode++;
                rPam.GetMark()->nContent.Assign( rPam.GetMark()->
                                    nNode.GetNode().GetCntntNode(), 0 );
                pDoc->DeleteRedline( rPam );
                rPam.DeleteMark();
            }

            RemoveIdxRel( nNode+1, *rPam.GetPoint() );

            pTNd->JoinNext();
            if( pHistory )
            {
                rPam.GetPoint()->nContent = 0;
                rPam.SetMark();
                rPam.GetPoint()->nContent = pTNd->GetTxt().Len();

                pDoc->RstTxtAttr( rPam, TRUE );
                pHistory->TmpRollback( pDoc, 0, FALSE );
            }
        }
    }

    // setze noch den Cursor auf den Undo-Bereich
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nCntnt );
}


void SwUndoSplitNode::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_SPLITNODE == rUndoIter.GetLastUndoId() )
        return;
    rUndoIter.GetDoc().SplitNode( *rUndoIter.pAktPam->GetPoint(), bChkTblStt );
    rUndoIter.pLastUndoObj = this;
}


void SwUndoSplitNode::Redo( SwUndoIter& rUndoIter )
{
    SwPaM& rPam = *rUndoIter.pAktPam;
    ULONG nOldNode = rPam.GetPoint()->nNode.GetIndex();
    rPam.GetPoint()->nNode = nNode;
    SwTxtNode * pTNd = rPam.GetNode()->GetTxtNode();
    if( pTNd )              // sollte eigentlich immer ein TextNode sein !!
    {
        rPam.GetPoint()->nContent.Assign( pTNd, nCntnt );

        SwDoc* pDoc = rPam.GetDoc();
        pDoc->SplitNode( *rPam.GetPoint(), bChkTblStt );

        if( pHistory )
            pHistory->SetTmpEnd( pHistory->Count() );

        if( ( pRedlData && IsRedlineOn( GetRedlineMode() )) ||
            ( !( REDLINE_IGNORE & GetRedlineMode() ) &&
                pDoc->GetRedlineTbl().Count() ))
        {
            rPam.SetMark();
            if( rPam.Move( fnMoveBackward ))
            {
                if( pRedlData && IsRedlineOn( GetRedlineMode() ))
                {
                    SwRedlineMode eOld = pDoc->GetRedlineMode();
                    pDoc->SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );
                    pDoc->AppendRedline( new SwRedline( *pRedlData, rPam ));
                    pDoc->SetRedlineMode_intern( eOld );
                }
                else
                    pDoc->SplitRedline( rPam );
                rPam.Exchange();
            }
            rPam.DeleteMark();
        }
    }
    else
        rPam.GetPoint()->nNode = nOldNode;
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unspnd.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.41  2000/09/18 16:04:30  willem.vandorp
      OpenOffice header added.

      Revision 1.40  2000/07/03 19:25:15  jp
      Bug #67696#: RstTxtAttr must delete TOX- and RefMarks

      Revision 1.39  1998/04/02 13:14:10  JP
      Redo: Undo-Flag wird schon von der EditShell abgeschaltet


      Rev 1.38   02 Apr 1998 15:14:10   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.37   08 Jan 1998 20:56:02   JP
   SwDoc::GetRedlineTbl returnt jetzt eine Referenz

      Rev 1.36   19 Dec 1997 12:14:22   JP
   Undo: Redlining beachten

      Rev 1.35   03 Nov 1997 13:06:08   MA
   precomp entfernt

      Rev 1.34   30 Oct 1997 14:35:46   AMA
   Chg: Kein AutoFlag mehr an Break bzw. PageDesc-Attributen

      Rev 1.33   09 Oct 1997 15:45:40   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.32   18 Aug 1997 10:34:54   OS
   includes

      Rev 1.31   15 Aug 1997 12:38:04   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.30   12 Aug 1997 12:36:46   OS
   Header-Umstellung

      Rev 1.29   07 Aug 1997 15:04:56   OM
   Headerfile-Umstellung

      Rev 1.28   07 Jul 1997 11:21:10   JP
   Bug #41313#: SplitNode - Flag fuer Sonderbehandlung von 1.Box in 1.Tabelle

      Rev 1.27   11 Jun 1997 10:44:08   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.26   29 May 1997 22:56:56   JP
   CopyAttr/CopyFmtAttr von SwUndo zur SwHistory verschoben

      Rev 1.25   06 May 1997 12:06:12   MA
   swtablehxx aufgeteilt

      Rev 1.24   06 Feb 1997 13:26:56   JP
   BreakAttribute beachten

      Rev 1.23   29 Jan 1997 14:57:02   JP
   neu: Flag fuer Sonderbehandlung in Tabellen

      Rev 1.22   29 Oct 1996 14:55:40   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.21   23 Sep 1996 20:06:28   JP
   SetTmpEnd: DocPtr entfernt

      Rev 1.20   24 Nov 1995 17:14:02   OM
   PCH->PRECOMPILED

      Rev 1.19   23 Jun 1995 07:58:56   OS
   +inline GetDoc

      Rev 1.18   22 Jun 1995 19:33:36   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.17   04 Mar 1995 13:31:44   MA
   unnoetiges SEXPORT entfernt.

      Rev 1.16   23 Feb 1995 23:04:22   ER
   sexport

      Rev 1.15   08 Feb 1995 23:52:32   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.14   24 Jan 1995 19:07:46   JP
   JoinNext/-Prev: zus. Parameter - seine Position im NodesArray

      Rev 1.13   15 Dec 1994 20:48:38   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.12   25 Oct 1994 14:50:38   MA
   PreHdr.

      Rev 1.11   02 Aug 1994 16:48:08   JP
   SwCntntNode::JoinNext/JoinPrev(): Nodes-Array nicht mehr als Parameter

      Rev 1.10   15 Mar 1994 11:33:32   JP
   TmpRollback: wenn vom gesamten Node die Attribute gesichert werden, dann
       immer von vorne wieder setzen.

      Rev 1.9   02 Mar 1994 19:50:34   MI
   Underscore im Namen der #pragmas

      Rev 1.8   17 Feb 1994 08:31:16   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.7   16 Feb 1994 13:22:20   MI
   Pragmas zurechtgerueckt

*************************************************************************/


