/*************************************************************************
 *
 *  $RCSfile: unins.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:39:02 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
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
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>
#endif
#ifndef _DCONTACT_HXX //autogen
#include <dcontact.hxx>
#endif

class _UnReplaceData : private SwUndoSaveCntnt
{
    String sOld, sIns;
    ULONG nSttNd, nEndNd, nOffset;
    xub_StrLen nSttCnt, nEndCnt, nSetPos, nSelEnd;
    BOOL bSplitNext : 1;
    BOOL bRegExp : 1;

public:
    _UnReplaceData( const SwPaM& rPam, const String& rIns, BOOL bRegExp );
    ~_UnReplaceData();

    void Undo( SwUndoIter& rIter );
    void Redo( SwUndoIter& rIter );
    void SetEnd( const SwPaM& rPam );
};


SV_IMPL_PTRARR( _UnReplaceDatas, _UnReplaceData* )

//------------------------------------------------------------------

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

// zwei Zugriffs-Funktionen
inline SwPosition* IterPt( SwUndoIter& rUIter )
{   return rUIter.pAktPam->GetPoint();  }
inline SwPosition* IterMk( SwUndoIter& rUIter )
{   return rUIter.pAktPam->GetMark();   }

//------------------------------------------------------------

// INSERT

SwUndoInsert::SwUndoInsert( const SwNodeIndex& rNd, xub_StrLen nCnt,
                            xub_StrLen nL, BOOL bWDelim )
    : SwUndo(UNDO_INSERT), nNode( rNd.GetIndex() ), nCntnt(nCnt), nLen(nL),
        bIsWordDelim( bWDelim ), bIsAppend( FALSE ), pPos( 0 ), pTxt( 0 ),
        pRedlData( 0 )
{
    // Redline beachten
    SwDoc& rDoc = *rNd.GetNode().GetDoc();
    if( rDoc.IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( REDLINE_INSERT,
                                        rDoc.GetRedlineAuthor() );
        SetRedlineMode( rDoc.GetRedlineMode() );
    }
}

SwUndoInsert::SwUndoInsert( const SwNodeIndex& rNd )
    : SwUndo(UNDO_INSERT), nNode( rNd.GetIndex() ), nCntnt(0), nLen(1),
        bIsWordDelim( FALSE ), bIsAppend( TRUE ), pPos( 0 ), pTxt( 0 ),
        pRedlData( 0 )
{
    // Redline beachten
    SwDoc& rDoc = *rNd.GetNode().GetDoc();
    if( rDoc.IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( REDLINE_INSERT,
                                        rDoc.GetRedlineAuthor() );
        SetRedlineMode( rDoc.GetRedlineMode() );
    }
}

// stelle fest, ob das naechste Insert mit dem aktuellen zusammengefasst
// werden kann. Wenn ja, dann aender die Laenge und die InsPos.
// Dann wird von SwDoc::Insert kein neues Object in die Undoliste gestellt.

BOOL SwUndoInsert::CanGrouping( const SwPosition& rInsPos, sal_Unicode cIns )
{
    BOOL bRet = FALSE;
    if( !bIsAppend )
    {
        ++nCntnt;
        bRet = CanGrouping( rInsPos );
        --nCntnt;
        if( bRet )
            bRet = CanGrouping( cIns );
    }
    return bRet;
}


BOOL SwUndoInsert::CanGrouping( sal_Unicode cIns )
{
    if( !bIsAppend && bIsWordDelim ==
        !GetAppCharClass().isLetterNumeric( String( cIns )) )
    {
        nLen++;
        nCntnt++;
        return TRUE;
    }
    return FALSE;
}

BOOL SwUndoInsert::CanGrouping( const SwPosition& rPos )
{
    BOOL bRet = FALSE;
    if( nNode == rPos.nNode.GetIndex() &&
        nCntnt == rPos.nContent.GetIndex() )
    {
        // Redline beachten
        SwDoc& rDoc = *rPos.nNode.GetNode().GetDoc();
        if( ( ~REDLINE_SHOW_MASK & rDoc.GetRedlineMode() ) ==
            ( ~REDLINE_SHOW_MASK & GetRedlineMode() ) )
        {
            bRet = TRUE;

            // dann war oder ist noch Redline an:
            // pruefe, ob an der InsPosition ein anderer Redline
            // rumsteht. Wenn der gleiche nur einmalig vorhanden ist,
            // kann zusammen gefasst werden.
            const SwRedlineTbl& rTbl = rDoc.GetRedlineTbl();
            if( rTbl.Count() )
            {
                SwRedlineData aRData( REDLINE_INSERT, rDoc.GetRedlineAuthor() );
                const SwIndexReg* pIReg = rPos.nContent.GetIdxReg();
                SwIndex* pIdx;
                for( USHORT i = 0; i < rTbl.Count(); ++i )
                {
                    SwRedline* pRedl = rTbl[ i ];
                    if( pIReg == (pIdx = &pRedl->End()->nContent)->GetIdxReg() &&
                        nCntnt == pIdx->GetIndex() )
                    {
                        if( !pRedl->HasMark() || !pRedlData ||
                            *pRedl != *pRedlData || *pRedl != aRData )
                        {
                            bRet = FALSE;
                            break;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}

SwUndoInsert::~SwUndoInsert()
{
    if( pPos )      // loesche noch den Bereich aus dem UndoNodes Array
    {
        // Insert speichert den Inhalt in der IconSection
        SwNodes& rUNds = pPos->nNode.GetNode().GetNodes();
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
    else if( pTxt )     // der eingefuegte Text
        delete pTxt;
    delete pRedlData;
}



void SwUndoInsert::Undo( SwUndoIter& rUndoIter )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();

    if( bIsAppend )
    {
        SwPaM* pPam = rUndoIter.pAktPam;
        pPam->GetPoint()->nNode = nNode;

        if( IsRedlineOn( GetRedlineMode() ))
        {
            pPam->GetPoint()->nContent.Assign( pPam->GetCntntNode(), 0 );
            pPam->SetMark();
            pPam->Move( fnMoveBackward );
            pPam->Exchange();
            pDoc->DeleteRedline( *pPam );
        }
        pPam->DeleteMark();
        pDoc->DelFullPara( *pPam );
        pPam->GetPoint()->nContent.Assign( pPam->GetCntntNode(), 0 );
    }
    else
    {
        ULONG nNd = nNode;
        xub_StrLen nCnt = nCntnt;
        if( nLen )
        {
            SwNodeIndex aNd( pDoc->GetNodes(), nNode);
            SwCntntNode* pCNd = aNd.GetNode().GetCntntNode();
            SwPaM aPaM( *pCNd, nCntnt );

            aPaM.SetMark();

            if( pCNd->IsTxtNode() )     // Text !!
            {
                aPaM.GetPoint()->nContent -= nLen;
                if( IsRedlineOn( GetRedlineMode() ))
                    pDoc->DeleteRedline( aPaM );
                RemoveIdxFromRange( aPaM, FALSE );
                pTxt = new String( ((SwTxtNode*)pCNd)->GetTxt().Copy(
                                            nCntnt-nLen, nLen ) );
                ((SwTxtNode*)pCNd)->Erase( aPaM.GetPoint()->nContent, nLen );
            }
            else                // ansonsten Grafik/OLE/Text/...
            {
                aPaM.Move(fnMoveBackward);
                if( IsRedlineOn( GetRedlineMode() ))
                    pDoc->DeleteRedline( aPaM );
                RemoveIdxFromRange( aPaM, FALSE );
            }

            nNd = aPaM.GetPoint()->nNode.GetIndex();
            nCnt = aPaM.GetPoint()->nContent.GetIndex();

            if( !pTxt )
            {
                pPos = new SwPosition( *aPaM.GetPoint() );
                MoveToUndoNds( aPaM, &pPos->nNode, &pPos->nContent );
            }
            nNode = aPaM.GetPoint()->nNode.GetIndex();
            nCntnt = aPaM.GetPoint()->nContent.GetIndex();
        }

        // setze noch den Cursor auf den Undo-Bereich
        rUndoIter.pAktPam->DeleteMark();

        IterPt(rUndoIter)->nNode = nNd;
        IterPt(rUndoIter)->nContent.Assign( pDoc->GetNodes()[
                IterPt(rUndoIter)->nNode ]->GetCntntNode(), nCnt );
        // SPoint und GetMark auf der gleichen Position
    }
}


void SwUndoInsert::Redo( SwUndoIter& rUndoIter )
{
    // setze noch den Cursor auf den Redo-Bereich
    SwPaM* pPam = rUndoIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();
    pPam->DeleteMark();

    if( bIsAppend )
    {
        pPam->GetPoint()->nNode = nNode - 1;
        pDoc->AppendTxtNode( *pPam->GetPoint() );

        pPam->SetMark();
        pPam->Move( fnMoveBackward );
        pPam->Exchange();

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

        pPam->DeleteMark();
    }
    else
    {
        pPam->GetPoint()->nNode = nNode;
        SwCntntNode* pCNd = pDoc->GetNodes()[ pPam->GetPoint()->nNode ]->GetCntntNode();
        pPam->GetPoint()->nContent.Assign( pCNd, nCntnt );

        if( nLen )
        {
            BOOL bMvBkwrd = MovePtBackward( *pPam );

            if( pTxt )
            {
                ASSERT( pCNd->IsTxtNode(), "wo ist mein Textnode ??" );
                ((SwTxtNode*)pCNd)->Insert( *pTxt, pPam->GetMark()->nContent,
                                            INS_EMPTYEXPAND );
                DELETEZ( pTxt );
            }
            else
            {
                // Inhalt wieder einfuegen. (erst pPos abmelden !!)
                ULONG nMvNd = pPos->nNode.GetIndex();
                xub_StrLen nMvCnt = pPos->nContent.GetIndex();
                DELETEZ( pPos );
                MoveFromUndoNds( *pDoc, nMvNd, nMvCnt, *pPam->GetMark() );
            }
            nNode = pPam->GetMark()->nNode.GetIndex();
            nCntnt = pPam->GetMark()->nContent.GetIndex();

            MovePtForward( *pPam, bMvBkwrd );
            rUndoIter.pAktPam->Exchange();
            if( pRedlData && IsRedlineOn( GetRedlineMode() ))
            {
                SwRedlineMode eOld = pDoc->GetRedlineMode();
                pDoc->SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );
                pDoc->AppendRedline( new SwRedline( *pRedlData,
                                            *rUndoIter.pAktPam ));
                pDoc->SetRedlineMode_intern( eOld );
            }
            else if( !( REDLINE_IGNORE & GetRedlineMode() ) &&
                    pDoc->GetRedlineTbl().Count() )
                pDoc->SplitRedline( *rUndoIter.pAktPam );
        }
    }
}


void SwUndoInsert::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.pLastUndoObj = this;
    if( !nLen )
        return;

    SwDoc& rDoc = rUndoIter.GetDoc();
    SwNodeIndex aNd( rDoc.GetNodes(), nNode );
    SwCntntNode* pCNd = aNd.GetNode().GetCntntNode();;

    if( !bIsAppend && 1 == nLen )       // >1 dann immer nur Text, ansonsten Grafik/OLE/Text/...
    {
        SwPaM aPaM( *pCNd, nCntnt );
        aPaM.SetMark();
        aPaM.Move(fnMoveBackward);
        pCNd = aPaM.GetCntntNode();
    }

// Was passiert mit dem evt. selektierten Bereich ???

    switch( pCNd->GetNodeType() )
    {
    case ND_TEXTNODE:
        if( bIsAppend )
            rDoc.AppendTxtNode( *rUndoIter.pAktPam->GetPoint() );
        else
        {
            String aTxt( ((SwTxtNode*)pCNd)->GetTxt() );
            BOOL bGroupUndo = rDoc.DoesGroupUndo();
            rDoc.DoGroupUndo( FALSE );
            rDoc.Insert( *rUndoIter.pAktPam, aTxt.Copy( nCntnt - nLen, nLen ));
            rDoc.DoGroupUndo( bGroupUndo );
            break;
        }
    case ND_GRFNODE:
        {
            SwGrfNode* pGrfNd = (SwGrfNode*)pCNd;
            String sFile, sFilter;
            if( pGrfNd->IsGrfLink() )
                pGrfNd->GetFileFilterNms( &sFile, &sFilter );

            rDoc.Insert( *rUndoIter.pAktPam, sFile, sFilter,
                                &pGrfNd->GetGrf(),
                                0/* Grafik-Collection*/ );
        }
        break;

    case ND_OLENODE:
        {
            // StarView bietet noch nicht die Moeglichkeit ein StarOBJ zu kopieren
            SvStorageRef aRef = new SvStorage( aEmptyStr );
            SwOLEObj& rSwOLE = (SwOLEObj&)((SwOLENode*)pCNd)->GetOLEObj();
            SvInPlaceObjectRef aNew((SvPersist*) rSwOLE.GetOleRef()->CopyObject( aRef ) );
            rDoc.Insert( *rUndoIter.pAktPam, &aNew );
            break;
        }
    }
}


/*  */

SwUndoReplace::SwUndoReplace()
    : SwUndo( UNDO_REPLACE ), nAktPos( USHRT_MAX )
{
}

SwUndoReplace::~SwUndoReplace()
{
}

void SwUndoReplace::Undo( SwUndoIter& rUndoIter )
{
    // war dieses nicht die letze Undo-Aktion, dann setze den
    // Count neu
    if( rUndoIter.pLastUndoObj != this )
    {
        nAktPos = aArr.Count();
        rUndoIter.pLastUndoObj = this;
        bOldIterFlag = rUndoIter.bWeiter;
        rUndoIter.bWeiter = TRUE;
    }

    aArr[ --nAktPos ]->Undo( rUndoIter );

    if( !nAktPos )      // alten Status wieder zurueck
        rUndoIter.bWeiter = bOldIterFlag;
}


void SwUndoReplace::Redo( SwUndoIter& rUndoIter )
{
    // war dieses nicht die letze Undo-Aktion, dann setze den
    // Count neu
    if( rUndoIter.pLastUndoObj != this )
    {
        ASSERT( !nAktPos, "Redo ohne vorheriges Undo??" );
        rUndoIter.pLastUndoObj = this;
        bOldIterFlag = rUndoIter.bWeiter;
        rUndoIter.bWeiter = TRUE;
    }

    aArr[ nAktPos ]->Redo( rUndoIter );

    if( ++nAktPos >= aArr.Count() ) // alten Status wieder zurueck
    {
        nAktPos = USHRT_MAX;
        rUndoIter.bWeiter = bOldIterFlag;
    }
}


void SwUndoReplace::AddEntry( const SwPaM& rPam, const String& rInsert,
                                BOOL bRegExp )
{
    _UnReplaceData* pNew = new _UnReplaceData( rPam, rInsert, bRegExp );
    aArr.C40_INSERT(_UnReplaceData, pNew, aArr.Count() );
}

void SwUndoReplace::SetEntryEnd( const SwPaM& rPam )
{
    _UnReplaceData* pEntry = aArr[ aArr.Count()-1 ];
    pEntry->SetEnd( rPam );
}

_UnReplaceData::_UnReplaceData( const SwPaM& rPam, const String& rIns,
                                BOOL bRgExp )
    : nOffset( 0 ), sIns( rIns )
{
    bRegExp = bRgExp;

    const SwNodes& rNds = rPam.GetDoc()->GetNodes();
    const SwPosition *pStt = rPam.Start(),
                    *pEnd = rPam.GetPoint() == pStt
                        ? rPam.GetMark()
                        : rPam.GetPoint();

    nSttNd = nEndNd = pStt->nNode.GetIndex();
    nSttCnt = pStt->nContent.GetIndex();
    nSelEnd = nEndCnt = pEnd->nContent.GetIndex();

    bSplitNext = nSttNd != pEnd->nNode.GetIndex();

    SwTxtNode* pNd = pStt->nNode.GetNode().GetTxtNode();
    ASSERT( pNd, "wo ist der TextNode" );

    pHistory = new SwHistory;
    DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );

    nSetPos = pHistory->Count();

    ULONG nNewPos = pStt->nNode.GetIndex();
    nOffset = nSttNd - nNewPos;

    if( pNd->GetpSwpHints() )
        pHistory->CopyAttr( pNd->GetpSwpHints(), nNewPos, 0,
                            pNd->GetTxt().Len(), TRUE );

    if( bSplitNext )
    {
        if( pNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pNd->GetpSwAttrSet(), nNewPos );
        pHistory->Add( pNd->GetTxtColl(), nNewPos, ND_TEXTNODE );

        SwTxtNode* pNext = pEnd->nNode.GetNode().GetTxtNode();
        ULONG nTmp = pNext->GetIndex();
        pHistory->CopyAttr( pNext->GetpSwpHints(), nTmp, 0,
                            pNext->GetTxt().Len(), TRUE );
        if( pNext->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pNext->GetpSwAttrSet(), nTmp );
        pHistory->Add( pNext->GetTxtColl(),nTmp, ND_TEXTNODE );
    }

    if( !pHistory->Count() )
        delete pHistory, pHistory = 0;

    xub_StrLen nECnt = bSplitNext ? pNd->GetTxt().Len() : pEnd->nContent.GetIndex();
    sOld = pNd->GetTxt().Copy( nSttCnt, nECnt - nSttCnt );
}

_UnReplaceData::~_UnReplaceData()
{
}

void _UnReplaceData::Undo( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    SwPaM& rPam = *rIter.pAktPam;
    rPam.DeleteMark();

    SwTxtNode* pNd = pDoc->GetNodes()[ nSttNd - nOffset ]->GetTxtNode();
    ASSERT( pNd, "Wo ist der TextNode geblieben?" )

    SwAutoCorrExceptWord* pACEWord = pDoc->GetAutoCorrExceptWord();
    if( pACEWord )
    {
        if( 1 == sIns.Len() && 1 == sOld.Len() )
        {
            SwPosition aPos( *pNd ); aPos.nContent.Assign( pNd, nSttCnt );
            pACEWord->CheckChar( aPos, sOld.GetChar( 0 ) );
        }
        pDoc->SetAutoCorrExceptWord( 0 );
    }

    SwIndex aIdx( pNd, nSttCnt );
    if( nSttNd == nEndNd )
    {
        pNd->Erase( aIdx, sIns.Len() );
/*      if( bSplitNext )
        {
            SwPosition aPos( *pNd, aIdx );
            pDoc->SplitNode( aPos, FALSE );
            pNd = pDoc->GetNodes()[ nSttNd - nOffset ]->GetTxtNode();
            aIdx.Assign( pNd, nSttCnt );
        }
*/  }
    else
    {
        rPam.GetPoint()->nNode = *pNd;
        rPam.GetPoint()->nContent.Assign( pNd, nSttCnt );
        rPam.SetMark();
        rPam.GetPoint()->nNode = nEndNd - nOffset;
        rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nEndCnt );

        pDoc->DeleteAndJoin( rPam );
        rPam.DeleteMark();
        pNd = rPam.GetNode()->GetTxtNode();
        ASSERT( pNd, "Wo ist der TextNode geblieben?" );
        aIdx.Assign( pNd, nSttCnt );
    }

    if( bSplitNext )
    {
        SwPosition aPos( *pNd, aIdx );
        pDoc->SplitNode( aPos, FALSE );
        pNd = pDoc->GetNodes()[ nSttNd - nOffset ]->GetTxtNode();
        aIdx.Assign( pNd, nSttCnt );
    }

    if( sOld.Len() )
        pNd->Insert( sOld, aIdx );

    if( pHistory )
    {
        if( pNd->GetpSwpHints() )
            pNd->ClearSwpHintsArr( FALSE );

        pHistory->TmpRollback( pDoc, nSetPos, FALSE );
        if( nSetPos )       // es gab Fussnoten/FlyFrames
        {
            // gibts ausser diesen noch andere ?
            if( nSetPos < pHistory->Count() )
            {
                // dann sicher die Attribute anderen Attribute
                SwHistory aHstr;
                aHstr.Move( 0, pHistory, nSetPos );
                pHistory->Rollback( pDoc );
                pHistory->Move( 0, &aHstr );
            }
            else
            {
                pHistory->Rollback( pDoc );
                DELETEZ( pHistory );
            }
        }
    }

    rPam.GetPoint()->nNode = nSttNd;
    rPam.GetPoint()->nContent = aIdx;
}

void _UnReplaceData::Redo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    SwPaM& rPam = *rIter.pAktPam;
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = nSttNd;

    SwTxtNode* pNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    ASSERT( pNd, "Wo ist der TextNode geblieben?" )
    rPam.GetPoint()->nContent.Assign( pNd, nSttCnt );
    rPam.SetMark();
    if( bSplitNext )
    {
        rPam.GetPoint()->nNode = nSttNd + 1;
        pNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    }
    rPam.GetPoint()->nContent.Assign( pNd, nSelEnd );

    if( pHistory )
    {
        SwHistory* pSave = pHistory;
        SwHistory aHst;
        pHistory = &aHst;
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory->Count();

        pHistory = pSave;
        pHistory->Move( 0, &aHst );
    }
    else
    {
        pHistory = new SwHistory;
        DelCntntIndex( *rPam.GetMark(), *rPam.GetPoint() );
        nSetPos = pHistory->Count();
        if( !nSetPos )
            delete pHistory, pHistory = 0;
    }

    rDoc.Replace( rPam, sIns, bRegExp );
    rPam.DeleteMark();
    rDoc.DoUndo( bUndo );
}

void _UnReplaceData::SetEnd( const SwPaM& rPam )
{
    if( rPam.GetPoint()->nNode != rPam.GetMark()->nNode )
    {
        // es wurden mehrere Absaetze eingefuegt
        const SwPosition* pEnd = rPam.End();
        nEndNd = nOffset + pEnd->nNode.GetIndex();
        nEndCnt = pEnd->nContent.GetIndex();
    }
}

/*  */


SwUndoReRead::SwUndoReRead( const SwPaM& rPam, const SwGrfNode& rGrfNd )
    : SwUndo( UNDO_REREAD ), nPos( rPam.GetPoint()->nNode.GetIndex() )
{
    SaveGraphicData( rGrfNd );
}


SwUndoReRead::~SwUndoReRead()
{
    delete pGrf;
    delete pNm;
    delete pFltr;
}


void SwUndoReRead::SetAndSave( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    SwGrfNode* pGrfNd = rDoc.GetNodes()[ nPos ]->GetGrfNode();

    if( !pGrfNd )
        return ;

        // die alten Werte zwischen speichern
    Graphic* pOldGrf = pGrf;
    String* pOldNm = pNm;
    String* pOldFltr = pFltr;
    USHORT nOldMirr = nMirr;

    SaveGraphicData( *pGrfNd );
    if( pOldNm )
    {
        pGrfNd->ReRead( *pOldNm, pFltr ? *pFltr : aEmptyStr, 0, 0, TRUE );
        delete pOldNm;
        delete pOldFltr;
    }
    else
    {
        pGrfNd->ReRead( aEmptyStr, aEmptyStr, pOldGrf, 0, TRUE );
        delete pOldGrf;
    }

    if( RES_DONT_MIRROR_GRF != nOldMirr )
        pGrfNd->SetAttr( SwMirrorGrf() );

    rIter.pSelFmt = pGrfNd->GetFlyFmt();
}


void SwUndoReRead::Undo( SwUndoIter& rIter )
{
    SetAndSave( rIter );
}


void SwUndoReRead::Redo( SwUndoIter& rIter )
{
    SetAndSave( rIter );
}


void SwUndoReRead::SaveGraphicData( const SwGrfNode& rGrfNd )
{
    if( rGrfNd.IsGrfLink() )
    {
        pNm = new String;
        pFltr = new String;
        rGrfNd.GetFileFilterNms( pNm, pFltr );
        pGrf = 0;
    }
    else
    {
        ((SwGrfNode&)rGrfNd).SwapIn( TRUE );
        pGrf = new Graphic( rGrfNd.GetGrf() );
        pNm = pFltr = 0;
    }
    nMirr = rGrfNd.GetSwAttrSet().GetMirrorGrf().GetValue();
}

/*  */

SwUndoInsertLabel::SwUndoInsertLabel( const SwLabelType eTyp,
                                    const String &rTxt, const BOOL bBef,
                                    const USHORT nId, const BOOL bCpyBorder )
    : SwUndo( UNDO_INSERTLABEL ), eType( eTyp ), sText( rTxt ),
    bBefore( bBef ), nFldId( nId ), aPos( 0, 0 ), nLayerId( 0 ),
    bCpyBrd( bCpyBorder )
{
    bUndoKeep = FALSE;
    OBJECT.pUndoFly = 0;
    OBJECT.pUndoAttr = 0;
}

SwUndoInsertLabel::~SwUndoInsertLabel()
{
    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        delete OBJECT.pUndoFly;
        delete OBJECT.pUndoAttr;
    }
    else
        delete NODE.pUndoInsNd;
}

void SwUndoInsertLabel::Undo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();

    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        ASSERT( OBJECT.pUndoAttr && OBJECT.pUndoFly, "Pointer nicht initialisiert" )
        SwFrmFmt* pFmt;
        SdrObject *pSdrObj;
        if( OBJECT.pUndoAttr &&
            0 != (pFmt = (SwFrmFmt*)OBJECT.pUndoAttr->GetFmt( rDoc )) &&
            ( LTYPE_DRAW != eType ||
              0 != (pSdrObj = pFmt->FindSdrObject()) ) )
        {
            OBJECT.pUndoAttr->Undo( rIter );
            OBJECT.pUndoFly->Undo( rIter );
            if( LTYPE_DRAW == eType )
            {
                pSdrObj->SetRelativePos( aPos );
                pSdrObj->SetLayer( nLayerId );
            }
        }
    }
    else if( NODE.nNode )
    {
        if ( eType == LTYPE_TABLE && bUndoKeep )
        {
            SwTableNode *pNd = rDoc.GetNodes()[
                        rDoc.GetNodes()[NODE.nNode-1]->StartOfSectionIndex()]->GetTableNode();
            if ( pNd )
                pNd->GetTable().GetFrmFmt()->ResetAttr( RES_KEEP );
        }
        SwPaM aPam( *rIter.pAktPam->GetPoint() );
        aPam.GetPoint()->nNode = NODE.nNode;
        aPam.SetMark();
        aPam.GetPoint()->nNode = NODE.nNode + 1;
        NODE.pUndoInsNd = new SwUndoDelete( aPam, TRUE );
    }
}


void SwUndoInsertLabel::Redo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();

    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        ASSERT( OBJECT.pUndoAttr && OBJECT.pUndoFly, "Pointer nicht initialisiert" )
        SwFrmFmt* pFmt;
        SdrObject *pSdrObj;
        if( OBJECT.pUndoAttr &&
            0 != (pFmt = (SwFrmFmt*)OBJECT.pUndoAttr->GetFmt( rDoc )) &&
            ( LTYPE_DRAW != eType ||
              0 != (pSdrObj = pFmt->FindSdrObject()) ) )
        {
            OBJECT.pUndoFly->Redo( rIter );
            OBJECT.pUndoAttr->Redo( rIter );
            if( LTYPE_DRAW == eType )
            {
                pSdrObj->SetRelativePos( Point(0,0) );
                pSdrObj->SetLayer( nLayerId );
                if( pSdrObj->GetLayer() == rDoc.GetHellId() )
                    pSdrObj->SetLayer( rDoc.GetHeavenId() );
            }
        }
    }
    else if( NODE.pUndoInsNd )
    {
        if ( eType == LTYPE_TABLE && bUndoKeep )
        {
            SwTableNode *pNd = rDoc.GetNodes()[
                        rDoc.GetNodes()[NODE.nNode-1]->StartOfSectionIndex()]->GetTableNode();
            if ( pNd )
                pNd->GetTable().GetFrmFmt()->SetAttr( SvxFmtKeepItem(TRUE) );
        }
        NODE.pUndoInsNd->Undo( rIter );
        delete NODE.pUndoInsNd, NODE.pUndoInsNd = 0;
    }
}

void SwUndoInsertLabel::Repeat( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    const SwPosition& rPos = *rIter.pAktPam->GetPoint();

    ULONG nIdx = 0;

    SwCntntNode* pCNd = rPos.nNode.GetNode().GetCntntNode();
    if( pCNd )
        switch( eType )
        {
        case LTYPE_TABLE:
            {
                const SwTableNode* pTNd = pCNd->FindTableNode();
                if( pTNd )
                    nIdx = pTNd->GetIndex();
            }
            break;

        case LTYPE_FLY:
        case LTYPE_OBJECT:
            {
                SwFlyFrm* pFly;
                SwCntntFrm *pCnt = pCNd->GetFrm();
                if( pCnt && 0 != ( pFly = pCnt->FindFlyFrm() ) )
                    nIdx = pFly->GetFmt()->GetCntnt().GetCntntIdx()->GetIndex();
            }
            break;
        }

    if( nIdx )
    {
        rDoc.InsertLabel( eType, sText, bBefore, nFldId, nIdx, bCpyBrd );
    }
}

void SwUndoInsertLabel::SetFlys( SwFrmFmt& rOldFly, SfxItemSet& rChgSet,
                                SwFrmFmt& rNewFly )
{
    if( LTYPE_OBJECT == eType || LTYPE_DRAW == eType )
    {
        _UndoFmtAttr aTmp( rOldFly, FALSE );
        rOldFly.SetAttr( rChgSet );
        if( aTmp.pUndo )
            OBJECT.pUndoAttr = aTmp.pUndo;
        OBJECT.pUndoFly = new SwUndoInsLayFmt( &rNewFly );
    }
}

void SwUndoInsertLabel::SetDrawObj( const Point& rPos, BYTE nLId )
{
    if( LTYPE_DRAW == eType )
    {
        aPos = rPos;
        nLayerId = nLId;
    }
}

