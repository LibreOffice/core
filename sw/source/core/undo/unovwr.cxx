/*************************************************************************
 *
 *  $RCSfile: unovwr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-27 16:53:28 $
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

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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


using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

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
        if( !FillSaveData( aPam, *pRedlSaveData, FALSE ))
            delete pRedlSaveData, pRedlSaveData = 0;
    }

    nSttNode = rPos.nNode.GetIndex();
    nSttCntnt = rPos.nContent.GetIndex();

    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    ASSERT( pTxtNd, "Overwrite nicht im TextNode?" );

    bInsChar = TRUE;
    xub_StrLen nTxtNdLen = pTxtNd->GetTxt().Len();
    if( nSttCntnt < nTxtNdLen )     // kein reines Einfuegen ?
    {
        aDelStr.Insert( pTxtNd->GetTxt().GetChar( nSttCntnt ) );
        if( !pHistory )
            pHistory = new SwHistory;
        SwRegHistory aRHst( *pTxtNd, pHistory );
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode, 0,
                            nTxtNdLen, FALSE );
        rPos.nContent++;
        bInsChar = FALSE;
    }

    BOOL bOldExpFlg = pTxtNd->IsIgnoreDontExpand();
    pTxtNd->SetIgnoreDontExpand( TRUE );

    pTxtNd->Insert( cIns, rPos.nContent );
    aInsStr.Insert( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pTxtNd->Erase( aTmpIndex, 1 );
    }
    pTxtNd->SetIgnoreDontExpand( bOldExpFlg );
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

    CharClass& rCC = GetAppCharClass();

    // befrage das einzufuegende Charakter
    if( ( CH_TXTATR_BREAKWORD == cIns && CH_TXTATR_INWORD == cIns ) ||
        rCC.isLetterNumeric( String( cIns ), 0 ) !=
        rCC.isLetterNumeric( aInsStr, aInsStr.Len()-1 ) )
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

    BOOL bOldExpFlg = pDelTxtNd->IsIgnoreDontExpand();
    pDelTxtNd->SetIgnoreDontExpand( TRUE );

    pDelTxtNd->Insert( cIns, rPos.nContent );
    aInsStr.Insert( cIns );

    if( !bInsChar )
    {
        const SwIndex aTmpIndex( rPos.nContent, -2 );
        pDelTxtNd->Erase( aTmpIndex, 1 );
    }
    pDelTxtNd->SetIgnoreDontExpand( bOldExpFlg );

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

        BOOL bOldExpFlg = pTxtNd->IsIgnoreDontExpand();
        pTxtNd->SetIgnoreDontExpand( TRUE );

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
        pTxtNd->SetIgnoreDontExpand( bOldExpFlg );
        rIdx--;
    }
    if( pHistory )
    {
        if( pTxtNd->GetpSwpHints() )
            pTxtNd->ClearSwpHintsArr( FALSE );
        pHistory->TmpRollback( pDoc, 0, FALSE );
    }

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

    BOOL bOldExpFlg = pTxtNd->IsIgnoreDontExpand();
    pTxtNd->SetIgnoreDontExpand( TRUE );

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
    pTxtNd->SetIgnoreDontExpand( bOldExpFlg );

    // alte Anfangs-Position vom UndoNodes-Array zurueckholen
    if( pHistory )
        pHistory->SetTmpEnd( pHistory->Count() );
    if( pAktPam->GetMark()->nContent.GetIndex() != nSttCntnt )
    {
        pAktPam->SetMark();
        pAktPam->GetMark()->nContent = nSttCntnt;
    }
}

//------------------------------------------------------------

struct _UndoTransliterate_Data
{
    String sText;
    _UndoTransliterate_Data* pNext;
    SwHistory* pHistory;
    Sequence <long>* pOffsets;
    ULONG nNdIdx;
    xub_StrLen nStart;

    _UndoTransliterate_Data( ULONG nNd, xub_StrLen nStt, const String& rTxt )
        : pNext( 0 ), pOffsets( 0 ), pHistory( 0 ), sText( rTxt ),
        nStart( nStt ), nNdIdx( nNd )
    {}
    ~_UndoTransliterate_Data() { delete pOffsets; delete pHistory; }

    void SetChangeAtNode( SwDoc& rDoc );
};

SwUndoTransliterate::SwUndoTransliterate( const SwPaM& rPam,
                            const utl::TransliterationWrapper& rTrans )
    : SwUndo( UNDO_TRANSLITERATE ), SwUndRng( rPam ),
    nType( rTrans.getType() ), pData( 0 ), pLastData( 0 )
{
}

SwUndoTransliterate::~SwUndoTransliterate()
{
    _UndoTransliterate_Data* pD = pData;
    while( pD )
    {
        pData = pD;
        pD = pD->pNext;
        delete pData;
    }
}

void SwUndoTransliterate::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    for( _UndoTransliterate_Data* pD = pData; pD; pD = pD->pNext )
        pD->SetChangeAtNode( rDoc );

    rDoc.DoUndo( bUndo );
    SetPaM( rUndoIter, TRUE );
}

void SwUndoTransliterate::Redo( SwUndoIter& rUndoIter )
{
/* ??? */   rUndoIter.SetUpdateAttr( TRUE );

    SetPaM( *rUndoIter.pAktPam );
    Repeat( rUndoIter );
}

void SwUndoTransliterate::Repeat( SwUndoIter& rUndoIter )
{
    SwPaM& rPam = *rUndoIter.pAktPam;
    SwDoc& rDoc = rUndoIter.GetDoc();

    utl::TransliterationWrapper aTrans(
                        ::comphelper::getProcessServiceFactory(), nType );
    rDoc.TransliterateText( rPam, aTrans );

    rUndoIter.pLastUndoObj = this;
}

void SwUndoTransliterate::AddChanges( const SwTxtNode& rTNd,
                    xub_StrLen nStart, xub_StrLen nNewLen,
                     ::com::sun::star::uno::Sequence <long>& rOffsets )
{
    long nOffsLen = rOffsets.getLength();
    _UndoTransliterate_Data* pNew = new _UndoTransliterate_Data(
                        rTNd.GetIndex(), nStart,
                        rTNd.GetTxt().Copy( nStart, xub_StrLen(nOffsLen) ));
    if( pData )
        pLastData->pNext = pNew;
    else
        pData = pNew;
    pLastData = pNew;

    const long* pOffsets = rOffsets.getConstArray();
    // where did we need less memory ?
    // check for 1-1 mappings:
    BOOL bOneToOne = TRUE;
    const long* p = pOffsets;
    for( long n = 0; n < nOffsLen; ++n, ++p )
        if( *p != ( nStart + n ))
        {
            // create the Offset array
            pNew->pOffsets = new Sequence <long> ( nNewLen );
            long* pIdx = pNew->pOffsets->getArray();
            p = pOffsets;
            long nMyOff, nNewVal = nStart;
            for( n = 0, nMyOff = nStart; n < nOffsLen; ++p, ++n, ++nMyOff )
            {
                if( *p < nMyOff )
                {
                    // something is deleted
                    nMyOff = *p;
                    ++nNewVal;
                }
                else if( *p > nMyOff )
                {
                    for( ; *p > nMyOff; ++nMyOff )
                        *pIdx++ = nNewVal-1;
                    --nMyOff;
                    --n;
                    --p;
                }
                else
                    *pIdx++ = nNewVal++;
            }
            while( n++ < nNewLen )
                *pIdx++ = nNewVal-1;

            // and then we need to save the attributes/bookmarks
            // but this data must moved every time to the last in the chain!
            _UndoTransliterate_Data* pD = pData;
            while( pD != pNew )
            {
                if( pD->nNdIdx == pNew->nNdIdx && pD->pHistory )
                {
                    // same node and have a history?
                    pNew->pHistory = pD->pHistory;
                    pD->pHistory = 0;
                    break;          // more can't exist
                }
                pD = pD->pNext;
            }

            if( !pNew->pHistory )
            {
                pNew->pHistory = new SwHistory;
                SwRegHistory aRHst( rTNd, pNew->pHistory );
                pNew->pHistory->CopyAttr( rTNd.GetpSwpHints(),
                        pNew->nNdIdx, 0, rTNd.GetTxt().Len(), FALSE );
            }
            break;
        }
}

void _UndoTransliterate_Data::SetChangeAtNode( SwDoc& rDoc )
{
    SwTxtNode* pTNd = rDoc.GetNodes()[ nNdIdx ]->GetTxtNode();
    if( pTNd )
    {
        xub_StrLen nDataLen = sText.Len();
        Sequence <long> aOffsets( nDataLen );
        String sChgd;
        if( pOffsets )
            aOffsets = *pOffsets;
        else
        {
            long* p = aOffsets.getArray();
            for( xub_StrLen n = 0; n < nDataLen; ++n, ++p )
                *p = n + nStart;
        }
        pTNd->ReplaceTextOnly( nStart, sText, aOffsets );

        if( pHistory )
        {
            if( pTNd->GetpSwpHints() )
                pTNd->ClearSwpHintsArr( FALSE );
            pHistory->TmpRollback( &rDoc, 0, FALSE );
            pHistory->SetTmpEnd( pHistory->Count() );
        }
    }
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unovwr.cxx,v 1.7 2001-02-27 16:53:28 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.6  2001/02/23 14:17:56  jp
      change transliteration from 1-1 to 1-n mapping

      Revision 1.5  2000/12/21 09:29:24  jp
      new: transliteration

      Revision 1.4  2000/11/06 10:47:36  jp
      use new flag from the txtnode for textattribut expansion

      Revision 1.3  2000/10/26 11:24:24  jp
      for bug #78848#: don't call DeleteRedline

      Revision 1.2  2000/10/25 15:13:25  jp
      use CharClass/BreakIt instead of old WordSelection

      Revision 1.1.1.1  2000/09/19 00:08:28  hr
      initial import

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


