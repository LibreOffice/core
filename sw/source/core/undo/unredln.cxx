/*************************************************************************
 *
 *  $RCSfile: unredln.cxx,v $
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
#ifndef _WORDSEL_HXX
#include <svtools/wordsel.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
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
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SORTOPT_HXX
#include <sortopt.hxx>
#endif

extern void lcl_JoinText( SwPaM& rPam, BOOL bJoinPrev );
extern void lcl_GetJoinFlags( SwPaM& rPam, BOOL& rJoinTxt, BOOL& rJoinPrev );

//------------------------------------------------------------------

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }


SwUndoRedline::SwUndoRedline( USHORT nUsrId, const SwPaM& rRange )
    : SwUndo( UNDO_REDLINE ), SwUndRng( rRange ),
    pRedlData( 0 ), pRedlSaveData( 0 ), nUserId( nUsrId ),
    bHiddenRedlines( FALSE )
{
    // Redline beachten
    SwDoc& rDoc = *rRange.GetDoc();
    if( rDoc.IsRedlineOn() )
    {
        switch( nUserId )
        {
        case UNDO_DELETE:
        case UNDO_REPLACE:
            pRedlData = new SwRedlineData( REDLINE_DELETE, rDoc.GetRedlineAuthor() );
            break;
        }
        SetRedlineMode( rDoc.GetRedlineMode() );
    }

    ULONG nEndExtra = rDoc.GetNodes().GetEndOfExtras().GetIndex();

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRange, *pRedlSaveData, FALSE,
                        UNDO_REJECT_REDLINE != nUserId ))
        delete pRedlSaveData, pRedlSaveData = 0;
    else
    {
        bHiddenRedlines = HasHiddenRedlines( *pRedlSaveData );
        if( bHiddenRedlines )           // dann muessen die NodeIndizies
        {                               // vom SwUndRng korrigiert werden
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

void SwUndoRedline::Undo( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    SetPaM( *rIter.pAktPam );

// RedlineMode setzen?
    _Undo( rIter );

    if( pRedlSaveData )
    {
        ULONG nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex();
        SetSaveData( *pDoc, *pRedlSaveData );
        if( bHiddenRedlines )
        {
            pRedlSaveData->DeleteAndDestroy( 0, pRedlSaveData->Count() );

            nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex() - nEndExtra;
            nSttNode += nEndExtra;
            nEndNode += nEndExtra;
        }
        SetPaM( *rIter.pAktPam, TRUE );
    }
}


void SwUndoRedline::Redo( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( ( eOld & ~REDLINE_IGNORE) | REDLINE_ON );

    SetPaM( *rIter.pAktPam );
    if( pRedlSaveData && bHiddenRedlines )
    {
        ULONG nEndExtra = pDoc->GetNodes().GetEndOfExtras().GetIndex();
        FillSaveData( *rIter.pAktPam, *pRedlSaveData, FALSE,
                        UNDO_REJECT_REDLINE != nUserId );

        nEndExtra -= pDoc->GetNodes().GetEndOfExtras().GetIndex();
        nSttNode -= nEndExtra;
        nEndNode -= nEndExtra;
    }
    _Redo( rIter );

    SetPaM( *rIter.pAktPam, TRUE );
    pDoc->SetRedlineMode_intern( eOld );
}

// default ist leer
void SwUndoRedline::_Undo( SwUndoIter& )
{
}

// default ist Redlines entfernen
void SwUndoRedline::_Redo( SwUndoIter& rIter )
{
    rIter.GetDoc().DeleteRedline( *rIter.pAktPam );
}


/*  */

SwUndoRedlineDelete::SwUndoRedlineDelete( const SwPaM& rRange, USHORT nUsrId )
    : SwUndoRedline( nUsrId = (nUsrId ? nUsrId : UNDO_DELETE), rRange ),
    bIsDelim( FALSE ), bIsBackspace( FALSE ), bCanGroup( FALSE )
{
    const SwTxtNode* pTNd;
    if( UNDO_DELETE == nUserId &&
        nSttNode == nEndNode && nSttCntnt + 1 == nEndCntnt &&
        0 != (pTNd = rRange.GetNode()->GetTxtNode()) )
    {
        sal_Unicode cCh = pTNd->GetTxt().GetChar( nSttCntnt );
        if( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
        {
            bCanGroup = TRUE;
            bIsDelim = !WordSelection::IsNormalChar( cCh );
            bIsBackspace = nSttCntnt == rRange.GetPoint()->nContent.GetIndex();
        }
    }
}

void SwUndoRedlineDelete::_Undo( SwUndoIter& rIter )
{
    rIter.GetDoc().DeleteRedline( *rIter.pAktPam );
}

void SwUndoRedlineDelete::_Redo( SwUndoIter& rIter )
{
    rIter.GetDoc().AppendRedline( new SwRedline( *pRedlData,
                                        *rIter.pAktPam ), FALSE );
}

BOOL SwUndoRedlineDelete::CanGrouping( const SwUndoRedlineDelete& rNext )
{
    BOOL bRet = FALSE;
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
            bRet = TRUE;
        }
    }
    return bRet;
}

/*  */

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

void SwUndoRedlineSort::_Undo( SwUndoIter& rIter )
{
    // im rIter.pAktPam ist der sortiete Bereich,
    // im aSaveRange steht der kopierte, sprich der originale.
    SwDoc& rDoc = rIter.GetDoc();

    if( 0 == ( REDLINE_SHOW_DELETE & rDoc.GetRedlineMode()) )
    {
        // die beiden Redline Objecte suchen und diese dann anzeigen lassen,
        // damit die Nodes wieder uebereinstimmen!
        // das Geloeschte ist versteckt, also suche das INSERT
        // Redline Object. Dahinter steht das Geloeschte
        USHORT nFnd = rDoc.GetRedlinePos(
                            *rDoc.GetNodes()[ nSttNode + nOffset + 1 ],
                            REDLINE_INSERT );
        ASSERT( USHRT_MAX != nFnd && nFnd+1 < rDoc.GetRedlineTbl().Count(),
                    "kein Insert Object gefunden" );
        ++nFnd;
        rDoc.GetRedlineTbl()[nFnd]->Show();
        SetPaM( *rIter.pAktPam );
    }

    {
        SwPaM aTmp( *rIter.pAktPam->GetMark() );
        aTmp.SetMark();
        aTmp.GetPoint()->nNode = nSaveEndNode;
        aTmp.GetPoint()->nContent.Assign( aTmp.GetCntntNode(), nSaveEndCntnt );
        rDoc.DeleteRedline( aTmp );
    }

    rDoc.DelFullPara( *rIter.pAktPam );
    SetPaM( *rIter.pAktPam );
}

void SwUndoRedlineSort::_Redo( SwUndoIter& rIter )
{
    SwPaM& rPam = *rIter.pAktPam;
    rIter.GetDoc().SortText( rPam, *pOpt );
    SetPaM( rPam );
    rPam.GetPoint()->nNode = nSaveEndNode;
    rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nSaveEndCntnt );
}

void SwUndoRedlineSort::Repeat( SwUndoIter& rIter )
{
    rIter.GetDoc().SortText( *rIter.pAktPam, *pOpt );
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

/*  */

SwUndoAcceptRedline::SwUndoAcceptRedline( const SwPaM& rRange )
    : SwUndoRedline( UNDO_ACCEPT_REDLINE, rRange )
{
}

void SwUndoAcceptRedline::_Redo( SwUndoIter& rIter )
{
    rIter.GetDoc().AcceptRedline( *rIter.pAktPam, FALSE );
}

void SwUndoAcceptRedline::Repeat( SwUndoIter& rIter )
{
    rIter.GetDoc().AcceptRedline( *rIter.pAktPam );
}

SwUndoRejectRedline::SwUndoRejectRedline( const SwPaM& rRange )
    : SwUndoRedline( UNDO_REJECT_REDLINE, rRange )
{
}

void SwUndoRejectRedline::_Redo( SwUndoIter& rIter )
{
    rIter.GetDoc().RejectRedline( *rIter.pAktPam, FALSE );
}

void SwUndoRejectRedline::Repeat( SwUndoIter& rIter )
{
    rIter.GetDoc().RejectRedline( *rIter.pAktPam );
}

/*  */

SwUndoCompDoc::SwUndoCompDoc( const SwPaM& rRg, BOOL bIns )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRg ), pRedlData( 0 ),
    pUnDel( 0 ), pRedlSaveData( 0 ), bInsert( bIns )
{
    SwDoc* pDoc = (SwDoc*)rRg.GetDoc();
    if( pDoc->IsRedlineOn() )
    {
        SwRedlineType eTyp = bInsert ? REDLINE_INSERT : REDLINE_DELETE;
        pRedlData = new SwRedlineData( eTyp, pDoc->GetRedlineAuthor() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }
}

SwUndoCompDoc::SwUndoCompDoc( const SwRedline& rRedl )
    : SwUndo( UNDO_COMPAREDOC ), SwUndRng( rRedl ), pRedlData( 0 ),
    pUnDel( 0 ), pRedlSaveData( 0 ),
    // fuers MergeDoc wird aber der jeweils umgekehrte Zweig benoetigt!
    bInsert( REDLINE_DELETE == rRedl.GetType() )
{
    SwDoc* pDoc = (SwDoc*)rRedl.GetDoc();
    if( pDoc->IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( rRedl.GetRedlineData() );
        SetRedlineMode( pDoc->GetRedlineMode() );
    }

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveData( rRedl, *pRedlSaveData, FALSE, TRUE ))
        delete pRedlSaveData, pRedlSaveData = 0;
}

SwUndoCompDoc::~SwUndoCompDoc()
{
    delete pRedlData;
    delete pUnDel;
    delete pRedlSaveData;
}

void SwUndoCompDoc::Undo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    SetPaM( *pPam );

    if( !bInsert )
    {
        // die Redlines loeschen
        SwRedlineMode eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern( ( eOld & ~REDLINE_IGNORE) | REDLINE_ON );

        pDoc->DeleteRedline( *pPam );

        pDoc->SetRedlineMode_intern( eOld );

        BOOL bJoinTxt, bJoinPrev;
        ::lcl_GetJoinFlags( *pPam, bJoinTxt, bJoinPrev );

        pUnDel = new SwUndoDelete( *pPam, FALSE );

        if( bJoinTxt )
            ::lcl_JoinText( *pPam, bJoinPrev );
    }
    else
    {
        if( IsRedlineOn( GetRedlineMode() ))
        {
            pDoc->DeleteRedline( *pPam );

            if( pRedlSaveData )
                SetSaveData( *pDoc, *pRedlSaveData );
        }
    }
}

void SwUndoCompDoc::Redo( SwUndoIter& rIter )
{
    // setze noch den Cursor auf den Redo-Bereich
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    rIter.pLastUndoObj = 0;

    if( bInsert )
    {
        SetPaM( *pPam );

        if( pRedlData && IsRedlineOn( GetRedlineMode() ))
        {
            SwRedline* pTmp = new SwRedline( *pRedlData, *pPam );
            ((SwRedlineTbl&)pDoc->GetRedlineTbl()).Insert( pTmp );
            pTmp->InvalidateRange();

/*
            SwRedlineMode eOld = pDoc->GetRedlineMode();
            pDoc->SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );
            pDoc->AppendRedline( new SwRedline( *pRedlData, *pPam ));
            pDoc->SetRedlineMode_intern( eOld );
*/
        }
        else if( !( REDLINE_IGNORE & GetRedlineMode() ) &&
                pDoc->GetRedlineTbl().Count() )
            pDoc->SplitRedline( *pPam );
    }
    else
    {
//      SwRedlineMode eOld = pDoc->GetRedlineMode();
//      pDoc->SetRedlineMode_intern( ( eOld & ~REDLINE_IGNORE) | REDLINE_ON );

        pUnDel->Undo( rIter );
        delete pUnDel, pUnDel = 0;

        SwRedline* pTmp = new SwRedline( *pRedlData, *pPam );
        ((SwRedlineTbl&)pDoc->GetRedlineTbl()).Insert( pTmp );
        pTmp->InvalidateRange();

//      pDoc->SetRedlineMode_intern( eOld );
    }
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unredln.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.19  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.18  2000/07/20 13:15:39  jp
      change old txtatr-character to the two new characters

      Revision 1.17  2000/05/19 12:53:54  jp
      use WordSelection class for check chars

      Revision 1.16  2000/05/09 10:04:31  jp
      Changes for Unicode

      Revision 1.15  1998/11/10 11:29:32  JP
      Bug #59223#: Undo CompDoc - das Join der Nodes nicht vergessen


      Rev 1.14   10 Nov 1998 12:29:32   JP
   Bug #59223#: Undo CompDoc - das Join der Nodes nicht vergessen

      Rev 1.13   22 Sep 1998 10:11:08   JP
   Bug #55909#: SwUndoCompDoc - die alten Redlines im Bereich merken

      Rev 1.12   07 Jul 1998 13:23:22   JP
   DocComp: Undo/Redo muessen das Layout invalidieren (fuer die korrekte Redlineanzeige)

      Rev 1.11   02 Apr 1998 15:14:10   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.10   05 Mar 1998 19:30:52   JP
   UndoRedlineDelete: CanCombine nur wenn sich alles in einem Node abspielt

      Rev 1.9   05 Mar 1998 12:57:56   JP
   SwUndoCompareDoc: fuers Merge doch einen eigene CTOR

      Rev 1.8   02 Mar 1998 09:50:12   JP
   UndoCompare: fuers MergeDoc erweitert und umgestellt

      Rev 1.7   12 Feb 1998 16:51:04   JP
   neu: Undo fuer Dokumentvergleich

      Rev 1.6   30 Jan 1998 19:34:50   JP
   bei vorhandenen versteckten Redlines die NodeIndizies korrigieren

      Rev 1.5   29 Jan 1998 22:50:26   JP
   RedlSaveData muss in Redo neu besorgt werden, wenn versteckte Deletes existierten

      Rev 1.4   28 Jan 1998 19:50:56   JP
   nach Redo: Pam auf jedenfall in ContentNodes setzen

      Rev 1.3   22 Jan 1998 20:53:12   JP
   CTOR des SwPaM umgestellt

      Rev 1.2   13 Jan 1998 21:38:10   JP
   neu: Undo fuer Accept-/Reject-Redline

      Rev 1.1   08 Jan 1998 21:09:28   JP
   weitere Redlining Erweiterung

      Rev 1.0   06 Jan 1998 16:27:40   JP
   Initial revision.

*************************************************************************/

