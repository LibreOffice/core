/*************************************************************************
 *
 *  $RCSfile: unovwr.cxx,v $
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
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif


//------------------------------------------------------------------

// zwei Zugriffs-Funktionen
inline SwPosition* IterPt( SwUndoIter& rUIter )
{   return rUIter.pAktPam->GetPoint();  }
inline SwPosition* IterMk( SwUndoIter& rUIter )
{   return rUIter.pAktPam->GetMark();   }

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

//------------------------------------------------------------


// OVERWRITE


SwUndoOverwrite::SwUndoOverwrite( SwDoc* pDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
    : SwUndo(UNDO_OVERWRITE), bGroup( FALSE ), pRedlSaveData( 0 )
{
#ifdef COMPACT
    pDoc->DelUndoGroups();
#endif

    if( !pDoc->IsIgnoreRedline() && pDoc->GetRedlineTbl().Count() )
    {
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );
        pRedlSaveData = new SwRedlineSaveDatas;
        if( !FillSaveData( aPam, *pRedlSaveData ))
            delete pRedlSaveData, pRedlSaveData = 0;
    }

    nSttNode = rPos.nNode.GetIndex();
    nSttCntnt = rPos.nContent.GetIndex();

    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    ASSERT( pTxtNd, "Overwrite nicht im TextNode?" );

    bInsChar = TRUE;
    if( nSttCntnt < pTxtNd->Len() )     // kein reines Einfuegen ?
    {
        aDelStr.Insert( pTxtNd->GetTxt().GetChar( nSttCntnt ) );
        if( !pHistory )
            pHistory = new SwHistory;
        SwRegHistory aRHst( *pTxtNd, pHistory );
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode, 0,
                            pTxtNd->Len(), FALSE );
        rPos.nContent++;
        bInsChar = FALSE;
    }
    pTxtNd->Insert( cIns, rPos.nContent );
    aInsStr.Insert( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pTxtNd->Erase( aTmpIndex, 1 );
    }

}

SwUndoOverwrite::~SwUndoOverwrite()
{
    delete pRedlSaveData;
}

BOOL SwUndoOverwrite::CanGrouping( SwDoc* pDoc, SwPosition& rPos,
                                    sal_Unicode cIns )
{
///  ?? was ist mit nur eingefuegten Charaktern ???

    // es kann nur das Loeschen von einzelnen char's zusammengefasst werden
    if( rPos.nNode != nSttNode || !aInsStr.Len()  ||
        ( !bGroup && aInsStr.Len() != 1 ))
        return FALSE;

    // ist der Node ueberhaupt ein TextNode?
    SwTxtNode * pDelTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( !pDelTxtNd ||
        ( pDelTxtNd->GetTxt().Len() != rPos.nContent.GetIndex() &&
            rPos.nContent.GetIndex() != ( nSttCntnt + aInsStr.Len() )))
        return FALSE;

    // befrage das einzufuegende Charakter
    if( ( CH_TXTATR_BREAKWORD == cIns && CH_TXTATR_INWORD == cIns ) ||
        WordSelection::IsNormalChar( cIns ) !=
        WordSelection::IsNormalChar( aInsStr.GetChar( aInsStr.Len()-1) ) )
        return FALSE;

    {
        SwRedlineSaveDatas* pTmpSav = new SwRedlineSaveDatas;
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex(),
                    rPos.nNode, rPos.nContent.GetIndex()+1 );

        if( !FillSaveData( aPam, *pTmpSav, FALSE ))
            delete pTmpSav, pTmpSav = 0;

        BOOL bOk = ( !pRedlSaveData && !pTmpSav ) ||
                   ( pRedlSaveData && pTmpSav &&
                        SwUndo::CanRedlineGroup( *pRedlSaveData, *pTmpSav,
                            nSttCntnt > rPos.nContent.GetIndex() ));
        delete pTmpSav;
        if( !bOk )
            return FALSE;

        pDoc->DeleteRedline( aPam, FALSE );
    }

    // Ok, die beiden 'Overwrites' koennen zusammen gefasst werden, also
    // 'verschiebe' das enstprechende Zeichen
    if( !bInsChar )
    {
        if( rPos.nContent.GetIndex() < pDelTxtNd->GetTxt().Len() )
        {
            aDelStr.Insert( pDelTxtNd->GetTxt().GetChar(rPos.nContent.GetIndex()) );
            rPos.nContent++;
        }
        else
            bInsChar = TRUE;
    }
    pDelTxtNd->Insert( cIns, rPos.nContent );
    aInsStr.Insert( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pDelTxtNd->Erase( aTmpIndex, 1 );
    }

    bGroup = TRUE;
    return TRUE;
}





void SwUndoOverwrite::Undo( SwUndoIter& rUndoIter )
{
    SwPaM* pAktPam = rUndoIter.pAktPam;
    SwDoc* pDoc = pAktPam->GetDoc();
    pAktPam->DeleteMark();
    pAktPam->GetPoint()->nNode = nSttNode;
    SwTxtNode* pTxtNd = pAktPam->GetNode()->GetTxtNode();
    ASSERT( pTxtNd, "Overwrite nicht im TextNode?" );
    SwIndex& rIdx = pAktPam->GetPoint()->nContent;
    rIdx.Assign( pTxtNd, nSttCntnt );

    SwAutoCorrExceptWord* pACEWord = pDoc->GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if( 1 == aInsStr.Len() && 1 == aDelStr.Len() )
            pACEWord->CheckChar( *pAktPam->GetPoint(), aDelStr.GetChar( 0 ) );
        pDoc->SetAutoCorrExceptWord( 0 );
    }

    // wurde nicht nur ueberschieben sondern auch geinsertet, so loesche
    // den Ueberhang
    if( aInsStr.Len() > aDelStr.Len() )
    {
        rIdx += aDelStr.Len();
        pTxtNd->Erase( rIdx, aInsStr.Len() - aDelStr.Len() );
        rIdx = nSttCntnt;
    }

    if( aDelStr.Len() )
    {
        String aTmpStr( '1' );
        sal_Unicode* pTmpStr = aTmpStr.GetBufferAccess();

        rIdx++;
        for( xub_StrLen n = 0; n < aDelStr.Len(); n++  )
        {
            // einzeln, damit die Attribute stehen bleiben !!!
            *pTmpStr = aDelStr.GetChar( n );
            pTxtNd->Insert( aTmpStr, rIdx /*???, SETATTR_NOTXTATRCHR*/ );
            rIdx -= 2;
            pTxtNd->Erase( rIdx, 1 );
            rIdx += 2;
        }
        rIdx--;
    }
    if( pHistory )
        pHistory->TmpRollback( pDoc, 0, FALSE );

    if( pAktPam->GetMark()->nContent.GetIndex() != nSttCntnt )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttCntnt;
    }

    if( pRedlSaveData )
        SetSaveData( *pDoc, *pRedlSaveData );
}


void SwUndoOverwrite::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.pLastUndoObj = this;
    SwPaM* pAktPam = rUndoIter.pAktPam;
    if( !aInsStr.Len() || pAktPam->HasMark() )
        return;

    SwDoc& rDoc = *pAktPam->GetDoc();

    BOOL bGroupUndo = rDoc.DoesGroupUndo();
    rDoc.DoGroupUndo( FALSE );
    rDoc.Overwrite( *pAktPam, aInsStr.GetChar( 0 ));
    rDoc.DoGroupUndo( bGroupUndo );
    for( xub_StrLen n = 1; n < aInsStr.Len(); ++n )
        rDoc.Overwrite( *pAktPam, aInsStr.GetChar( n ) );
}



void SwUndoOverwrite::Redo( SwUndoIter& rUndoIter )
{
    SwPaM* pAktPam = rUndoIter.pAktPam;
    SwDoc* pDoc = pAktPam->GetDoc();
    pAktPam->DeleteMark();
    pAktPam->GetPoint()->nNode = nSttNode;
    SwTxtNode* pTxtNd = pAktPam->GetNode()->GetTxtNode();
    ASSERT( pTxtNd, "Overwrite nicht im TextNode?" );
    SwIndex& rIdx = pAktPam->GetPoint()->nContent;

    if( pRedlSaveData )
    {
        rIdx.Assign( pTxtNd, nSttCntnt );
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent += aInsStr.Len();
        pDoc->DeleteRedline( *pAktPam, FALSE );
        pAktPam->DeleteMark();
    }
    rIdx.Assign( pTxtNd, aDelStr.Len() ? nSttCntnt+1 : nSttCntnt );

    for( xub_StrLen n = 0; n < aInsStr.Len(); n++  )
    {
        // einzeln, damit die Attribute stehen bleiben !!!
        pTxtNd->Insert( aInsStr.GetChar( n ), rIdx );
        if( n < aDelStr.Len() )
        {
            rIdx -= 2;
            pTxtNd->Erase( rIdx, 1 );
            rIdx += n+1 < aDelStr.Len() ? 2 : 1;
        }
    }

    // alte Anfangs-Position vom UndoNodes-Array zurueckholen
    if( pHistory )
        pHistory->SetTmpEnd( pHistory->Count() );
    if( pAktPam->GetMark()->nContent.GetIndex() != nSttCntnt )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttCntnt;
    }
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unovwr.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.35  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.34  2000/07/20 13:15:35  jp
      change old txtatr-character to the two new characters

      Revision 1.33  2000/05/19 12:53:47  jp
      use WordSelection class for check chars

      Revision 1.32  2000/05/09 10:04:28  jp
      Changes for Unicode

      Revision 1.31  1998/01/22 19:53:12  JP
      CTOR des SwPaM umgestellt


      Rev 1.30   22 Jan 1998 20:53:12   JP
   CTOR des SwPaM umgestellt

      Rev 1.29   16 Jan 1998 11:06:52   JP
   Overwrite: Redlining beachten

      Rev 1.28   19 Dec 1997 12:15:34   JP
   MSG/NOTE entfernt

      Rev 1.27   03 Nov 1997 13:06:16   MA
   precomp entfernt

      Rev 1.26   09 Oct 1997 15:45:40   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.25   11 Jun 1997 10:44:08   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.24   29 May 1997 22:56:56   JP
   CopyAttr/CopyFmtAttr von SwUndo zur SwHistory verschoben

      Rev 1.23   29 Oct 1996 15:57:26   JP
   Undo: autom. Aufnahme von Ausnahmen fuer die Autokorrektur anstossen

*************************************************************************/


