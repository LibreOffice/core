/*************************************************************************
 *
 *  $RCSfile: pam.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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

#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif

#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif


// fuer den dummen ?MSC-? Compiler
inline xub_StrLen GetSttOrEnd( BOOL bCondition, const SwCntntNode& rNd )
{
    return bCondition ? 0 : rNd.Len();
}

/*************************************************************************
|*
|*  SwPosition
|*
|*  Beschreibung        PAM.DOC
|*  Ersterstellung      VB  4.3.91
|*  Letzte Aenderung    VB  4.3.91
|*
*************************************************************************/


SwPosition::SwPosition(const SwPosition &rPos)
    : nNode(rPos.nNode),nContent(rPos.nContent)
{
}


SwPosition::SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt )
    : nNode( rNode ),nContent( rCntnt )
{
}

SwPosition::SwPosition( const SwNodeIndex &rNode )
    : nNode( rNode ), nContent( 0 )
{
}

SwPosition::SwPosition( const SwNode& rNode )
    : nNode( rNode ), nContent( 0 )
{
}


SwPosition &SwPosition::operator=(const SwPosition &rPos)
{
    nNode = rPos.nNode;
    nContent = rPos.nContent;
    return *this;
}


FASTBOOL SwPosition::operator<(const SwPosition &rPos) const
{
    if( nNode < rPos.nNode )
        return TRUE;
    if( nNode == rPos.nNode )
        return ( nContent < rPos.nContent );
    return FALSE;
}


FASTBOOL SwPosition::operator>(const SwPosition &rPos) const
{
    if(nNode > rPos.nNode )
        return TRUE;
    if( nNode == rPos.nNode )
        return ( nContent > rPos.nContent );
    return FALSE;
}


FASTBOOL SwPosition::operator<=(const SwPosition &rPos) const
{
    if(nNode < rPos.nNode )
        return TRUE;
    if( nNode == rPos.nNode )
        return ( nContent <= rPos.nContent );
    return FALSE;
}


FASTBOOL SwPosition::operator>=(const SwPosition &rPos) const
{
    if(nNode > rPos.nNode )
        return TRUE;
    if( nNode == rPos.nNode )
        return ( nContent >= rPos.nContent );
    return FALSE;
}


FASTBOOL SwPosition::operator==(const SwPosition &rPos) const
{
    return
        ( ( nNode == rPos.nNode ) && ( nContent == rPos.nContent ) ?
            TRUE: FALSE);
}


FASTBOOL SwPosition::operator!=(const SwPosition &rPos) const
{
    if( nNode != rPos.nNode )
        return TRUE;
    return ( nContent != rPos.nContent );
}

SwComparePosition ComparePosition(
            const SwPosition& rStt1, const SwPosition& rEnd1,
            const SwPosition& rStt2, const SwPosition& rEnd2 )
{
    SwComparePosition nRet;
    if( rStt1 < rStt2 )
    {
        if( rEnd1 > rStt2 )
        {
            if( rEnd1 >= rEnd2 )
                nRet = POS_OUTSIDE;
            else
                nRet = POS_OVERLAP_BEFORE;

        }
        else
            nRet = POS_BEFORE;
    }
    else if( rEnd2 > rStt1 )
    {
        if( rEnd2 >= rEnd1 )
        {
            if( rEnd2 == rEnd1 && rStt2 == rStt1 )
                nRet = POS_EQUAL;
            else
                nRet = POS_INSIDE;
        }
        else
            nRet = POS_OVERLAP_BEHIND;
    }
    else
        nRet = POS_BEHIND;
    return nRet;
}

/*  */

enum CHKSECTION { Chk_Both, Chk_One, Chk_None };


CHKSECTION lcl_TstIdx( ULONG nSttIdx, ULONG nEndIdx, const SwNode& rEndNd )
{
    ULONG nStt = rEndNd.StartOfSectionIndex(), nEnd = rEndNd.GetIndex();
    CHKSECTION eSec = nStt < nSttIdx && nEnd >= nSttIdx ? Chk_One : Chk_None;
    if( nStt < nEndIdx && nEnd >= nEndIdx )
        return( eSec == Chk_One ? Chk_Both : Chk_One );
    return eSec;
}


FASTBOOL lcl_ChkOneRange( CHKSECTION eSec, BOOL bChkSections,
                    const SwNode& rBaseEnd, ULONG nStt, ULONG nEnd )
{
    if( eSec != Chk_Both )
        return FALSE;

    if( !bChkSections )
        return TRUE;

    // suche die umspannende Section
    const SwNodes& rNds = rBaseEnd.GetNodes();
    const SwNode *pTmp, *pNd = rNds[ nStt ];
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();

    if( pNd == rNds[ nEnd ]->StartOfSectionNode() )
        return TRUE;        // der gleiche StartNode, die selbe Section

    // steht schon auf einem GrundSection Node ? Fehler !!!
    if( !pNd->StartOfSectionIndex() )
        return FALSE;

    while( ( pTmp = pNd->StartOfSectionNode())->EndOfSectionNode() !=
            &rBaseEnd )
        pNd = pTmp;

    ULONG nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
    return nSttIdx <= nStt && nStt <= nEndIdx &&
           nSttIdx <= nEnd && nEnd <= nEndIdx ? TRUE : FALSE;
}


FASTBOOL CheckNodesRange( const SwNodeIndex& rStt,
                        const SwNodeIndex& rEnd, FASTBOOL bChkSection )
{
    const SwNodes& rNds = rStt.GetNodes();
    ULONG nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
    CHKSECTION eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfContent() );
    if( Chk_None != eSec ) return eSec == Chk_Both ? TRUE : FALSE;

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfAutotext() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfAutotext(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfPostIts() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfPostIts(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfInserts() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfInserts(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfRedlines() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfRedlines(), nStt, nEnd );

    return FALSE;       // liegt irgendwo dazwischen, FEHLER
}


FASTBOOL GoNext(SwNode* pNd, SwIndex * pIdx )
{
    if( pNd->IsCntntNode() )
        return ((SwCntntNode*)pNd)->GoNext( pIdx );
    return FALSE;
}


FASTBOOL GoPrevious(SwNode* pNd, SwIndex * pIdx)
{
    if( pNd->IsCntntNode() )
        return ((SwCntntNode*)pNd)->GoPrevious( pIdx );
    return FALSE;
}


SwCntntNode* GoNextNds( SwNodeIndex* pIdx, FASTBOOL bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwCntntNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != aIdx.GetIndex() - pIdx->GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, TRUE ) )
                pNd = 0;
        else
            *pIdx = aIdx;
    }
    return pNd;
}


SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, FASTBOOL bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwCntntNode* pNd = aIdx.GetNodes().GoPrevious( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != pIdx->GetIndex() - aIdx.GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, TRUE ) )
                pNd = 0;
        else
            *pIdx = aIdx;
    }
    return pNd;
}

// ----------------------------------------------------------------------

/*************************************************************************
|*
|*  SwPointAndMark
|*
|*  Beschreibung        PAM.DOC
|*  Ersterstellung      VB  4.3.91
|*  Letzte Aenderung    JP  6.5.91
|*
*************************************************************************/

SwPaM::SwPaM( const SwPosition& rPos, SwPaM* pRing )
    : aBound1( rPos ), aBound2( rPos ), Ring( pRing )
{
    pPoint = pMark = &aBound1;
}

SwPaM::SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing )
    : aBound1( rMk ), aBound2( rPt ), Ring( pRing )
{
    pMark = &aBound1;
    pPoint = &aBound2;
}

SwPaM::SwPaM( const SwNodeIndex& rMk, const SwNodeIndex& rPt,
                long nMkOffset, long nPtOffset, SwPaM* pRing )
    : aBound1( rMk ), aBound2( rPt ), Ring( pRing )
{
    if( nMkOffset )
        aBound1.nNode += nMkOffset;
    if( nPtOffset )
        aBound2.nNode += nPtOffset;

    aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), 0 );
    aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), 0 );
    pMark = &aBound1;
    pPoint = &aBound2;
}

SwPaM::SwPaM( const SwNode& rMk, const SwNode& rPt,
                long nMkOffset, long nPtOffset, SwPaM* pRing )
    : aBound1( rMk ), aBound2( rPt ), Ring( pRing )
{
    if( nMkOffset )
        aBound1.nNode += nMkOffset;
    if( nPtOffset )
        aBound2.nNode += nPtOffset;

    aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), 0 );
    aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), 0 );
    pMark = &aBound1;
    pPoint = &aBound2;
}

SwPaM::SwPaM( const SwNodeIndex& rMk, xub_StrLen nMkCntnt,
              const SwNodeIndex& rPt, xub_StrLen nPtCntnt, SwPaM* pRing )
    : aBound1( rMk ), aBound2( rPt ), Ring( pRing )
{
    aBound1.nContent.Assign( rMk.GetNode().GetCntntNode(), nMkCntnt );
    aBound2.nContent.Assign( rPt.GetNode().GetCntntNode(), nPtCntnt );
    pMark = &aBound1;
    pPoint = &aBound2;
}

SwPaM::SwPaM( const SwNode& rMk, xub_StrLen nMkCntnt,
              const SwNode& rPt, xub_StrLen nPtCntnt, SwPaM* pRing )
    : aBound1( rMk ), aBound2( rPt ), Ring( pRing )
{
    aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), nMkCntnt );
    aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), nPtCntnt );
    pMark = &aBound1;
    pPoint = &aBound2;
}

SwPaM::SwPaM( SwPaM &rPam )
    : aBound1( *(rPam.pPoint) ), aBound2( *(rPam.pMark) ), Ring( &rPam )
{
    pPoint = &aBound1;
    pMark  = rPam.HasMark() ? &aBound2 : pPoint;
}

SwPaM::SwPaM( const SwNode& rNd, xub_StrLen nCntnt, SwPaM* pRing )
    : aBound1( rNd ), aBound2( rNd ), Ring( pRing )
{
    aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), nCntnt );
    aBound2.nContent = aBound1.nContent;
    pPoint = pMark = &aBound1;
}

SwPaM::SwPaM( const SwNodeIndex& rNd, xub_StrLen nCntnt, SwPaM* pRing )
    : aBound1( rNd ), aBound2( rNd ), Ring( pRing )
{
    aBound1.nContent.Assign( rNd.GetNode().GetCntntNode(), nCntnt );
    aBound2.nContent = aBound1.nContent;
    pPoint = pMark = &aBound1;
}

SwPaM::~SwPaM() {}

void SwPaM::SetMark()
{
    if(pPoint == &aBound1)
        pMark = &aBound2;
    else
        pMark = &aBound1;
    (*pMark) = (*pPoint);
}

#ifndef PRODUCT

void SwPaM::Exchange()
{
    if(pPoint != pMark)
    {
        SwPosition *pTmp = pPoint;
        pPoint = pMark;
        pMark = pTmp;
    }
}
#endif


SwPaM &SwPaM::operator=( SwPaM &rPam )
{
    MoveTo( &rPam );
    *pPoint = *( rPam.pPoint );
    if( rPam.HasMark() )
    {
        SetMark();
        *pMark = *( rPam.pMark );
    }
    return *this;
}

// Bewegen des Cursors


FASTBOOL SwPaM::Move( SwMoveFn fnMove, SwGoInDoc fnGo )
{
    return (*fnGo)( *this, fnMove );
}


/*************************************************************************
|*
|*    void SwPaM::MakeRegion( SwMoveFn, SwPaM*, const SwPaM* )
|*
|*    Beschreibung      Setzt den 1. SwPaM auf den uebergebenen SwPaM
|*                      oder setzt auf den Anfang oder Ende vom Document.
|*                      SPoint bleibt auf der Position stehen, GetMark aendert
|*                      sich entsprechend !
|*
|*    Parameter         SwDirection     gibt an, ob an Anfang / Ende
|*                      SwPaM *         der zu setzende Bereich
|*                      const SwPaM&    der enventuell vorgegeben Bereich
|*    Return-Werte      SwPaM*          der entsprehend neu gesetzte Bereich
|*
|*    Ersterstellung    JP 26.04.91
|*    Letzte Aenderung  JP 26.04.91
|*
*************************************************************************/


SwPaM* SwPaM::MakeRegion( SwMoveFn fnMove, const SwPaM * pOrigRg )
{
    SwPaM* pPam;
    if( pOrigRg == 0 )
    {
        pPam = new SwPaM( *pPoint );
        pPam->SetMark();                    // setze Anfang fest
        pPam->Move( fnMove, fnGoSection);       // an Anfang / Ende vom Node

        // stelle SPoint wieder auf alte Position, GetMark auf das "Ende"
        pPam->Exchange();
    }
    else
    {
        pPam = new SwPaM( *(SwPaM*)pOrigRg );   // die Suchregion ist vorgegeben
        // sorge dafuer, dass SPoint auf dem "echten" StartPunkt steht
        // FORWARD  --> SPoint immer kleiner  als GetMark
        // BACKWARD --> SPoint immer groesser als GetMark
        if( (pPam->GetMark()->*fnMove->fnCmpOp)( *pPam->GetPoint() ) )
            pPam->Exchange();
    }
    return pPam;
}

USHORT SwPaM::GetPageNum( BOOL bAtPoint, const Point* pLayPos )
{
    // return die Seitennummer am Cursor
    // (fuer Reader + Seitengebundene Rahmen)
    const SwCntntFrm* pCFrm;
    const SwPageFrm *pPg;
    const SwCntntNode *pNd ;
    const SwPosition* pPos = bAtPoint ? pPoint : pMark;

    if( 0 != ( pNd = pPos->nNode.GetNode().GetCntntNode() ) &&
        0 != ( pCFrm = pNd->GetFrm( pLayPos, pPos, FALSE )) &&
        0 != ( pPg = pCFrm->FindPageFrm() ))
        return pPg->GetPhyPageNum();
    return 0;
}

// steht in etwas geschuetztem oder in die Selektion umspannt
// etwas geschuetztes.
FASTBOOL SwPaM::HasReadonlySel() const
{
    FASTBOOL bRet = FALSE;
    Point aTmpPt;
    const SwCntntNode *pNd;
    const SwCntntFrm *pFrm;

    if( 0 != ( pNd = GetPoint()->nNode.GetNode().GetCntntNode() ))
        pFrm = pNd->GetFrm( &aTmpPt, GetPoint(), FALSE );
    else
        pFrm = 0;

    if( pFrm && pFrm->IsProtected() )
        bRet = TRUE;
    else if( pNd )
    {
        const SwSectionNode* pSNd = pNd->GetSectionNode();
        if( pSNd && pSNd->GetSection().IsProtectFlag() )
            bRet = TRUE;
    }

    if( !bRet && HasMark() && GetPoint()->nNode != GetMark()->nNode )
    {
        if( 0 != ( pNd = GetMark()->nNode.GetNode().GetCntntNode() ))
            pFrm = pNd->GetFrm( &aTmpPt, GetMark(), FALSE );
        else
            pFrm = 0;

        if( pFrm && pFrm->IsProtected() )
            bRet = TRUE;
        else if( pNd )
        {
            const SwSectionNode* pSNd = pNd->GetSectionNode();
            if( pSNd && pSNd->GetSection().IsProtectFlag() )
                bRet = TRUE;
        }

        // oder sollte eine geschuetzte Section innerhalb der
        // Selektion liegen?
        if( !bRet )
        {
            ULONG nSttIdx = GetMark()->nNode.GetIndex(),
                    nEndIdx = GetPoint()->nNode.GetIndex();
            if( nEndIdx <= nSttIdx )
            {
                ULONG nTmp = nSttIdx;
                nSttIdx = nEndIdx;
                nEndIdx = nTmp;
            }

            // wenn ein geschuetzter Bereich zwischen den Nodes stehen soll,
            // muss die Selektion selbst schon x Nodes umfassen.
            // (TxtNd, SectNd, TxtNd, EndNd, TxtNd )
            if( nSttIdx + 3 < nEndIdx )
            {
                const SwSectionFmts& rFmts = GetDoc()->GetSections();
                for( USHORT n = rFmts.Count(); n;  )
                {
                    const SwSectionFmt* pFmt = rFmts[ --n ];
                    if( pFmt->GetProtect().IsCntntProtected() )
                    {
                        const SwFmtCntnt& rCntnt = pFmt->GetCntnt(FALSE);
                        ASSERT( rCntnt.GetCntntIdx(), "wo ist der SectionNode?" );
                        ULONG nIdx = rCntnt.GetCntntIdx()->GetIndex();
                        if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                        {
/*                          // ist es keine gelinkte Section, dann kann sie auch
                            // nicht mitselektiert werden
                            const SwSection& rSect = *pFmt->GetSection();
                            if( CONTENT_SECTION == rSect.GetType() )
                            {
                                RestoreSavePos();
                                return TRUE;
                            }
*/
                            bRet = TRUE;
                            break;
                        }
                    }
                }

#ifdef CHECK_CELL_READONLY
//JP 22.01.99: bisher wurden Tabelle, die in der Text-Selektion standen
//              nicht beachtet. Wollte man das haben, dann muss dieser
//              Code freigeschaltet werden

                if( !bRet )
                {
                    // dann noch ueber alle Tabellen
                    const SwFrmFmts& rFmts = *GetDoc()->GetTblFrmFmts();
                    for( n = rFmts.Count(); n ;  )
                    {
                        SwFrmFmt* pFmt = (SwFrmFmt*)rFmts[ --n ];
                        const SwTable* pTbl = SwTable::FindTable( pFmt );
                        ULONG nIdx = pTbl ? pTbl->GetTabSortBoxes()[0]->GetSttIdx()
                                          : 0;
                        if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                        {
                            // dann teste mal alle Boxen
                            const SwTableSortBoxes& rBoxes = pTbl->GetTabSortBoxes();

                            for( USHORT i =  rBoxes.Count(); i; )
                                if( rBoxes[ --i ]->GetFrmFmt()->GetProtect().
                                    IsCntntProtected() )
                                {
                                    bRet = TRUE;
                                    break;
                                }

                            if( bRet )
                                break;
                        }
                    }
                }
#endif
            }
        }
    }
    return bRet;
}

//--------------------  Suche nach Formaten( FormatNamen ) -----------------

// die Funktion gibt in Suchrichtung den folgenden Node zurueck.
// Ist in der Richtung keiner mehr vorhanden oder ist dieser ausserhalb
// des Bereiches, wird ein 0 Pointer returnt.
// Das rbFirst gibt an, ob es man zu erstenmal einen Node holt. Ist das der
// Fall, darf die Position vom Pam nicht veraendert werden!


SwCntntNode* GetNode( SwPaM & rPam, FASTBOOL& rbFirst, SwMoveFn fnMove,
                        FASTBOOL bInReadOnly )
{
    SwCntntNode * pNd = 0;
    SwCntntFrm* pFrm;
    if( ((*rPam.GetPoint()).*fnMove->fnCmpOp)( *rPam.GetMark() ) ||
        ( *rPam.GetPoint() == *rPam.GetMark() && rbFirst ) )
    {
        if( rbFirst )
        {
            rbFirst = FALSE;
            pNd = rPam.GetCntntNode();
            if( pNd &&
                ( 0 == ( pFrm = pNd->GetFrm()) ||
                  ( !bInReadOnly && pFrm->IsProtected() ) ||
                  (pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow()) ) ||
                ( !bInReadOnly && pNd->FindSectionNode() &&
                  pNd->FindSectionNode()->GetSection().IsProtect() ))
                pNd = 0;
        }

        if( !pNd )          // steht Cursor auf keinem ContentNode ?
        {
            SwPosition aPos( *rPam.GetPoint() );
            FASTBOOL bSrchForward = fnMove == fnMoveForward;
            SwNodes& rNodes = aPos.nNode.GetNodes();

            // zum naechsten / vorherigen ContentNode
// Funktioniert noch alles, wenn die Uerbpruefung vom ueberspringen der
// Sektions herausgenommen wird ??
//          if( (*fnMove->fnNds)( rNodes, &aPos.nNode ) )
            while( TRUE )
            {
                pNd = bSrchForward
                        ? rNodes.GoNextSection( &aPos.nNode, TRUE, !bInReadOnly )
                        : rNodes.GoPrevSection( &aPos.nNode, TRUE, !bInReadOnly );
                if( pNd )
                {
                    aPos.nContent.Assign( pNd, ::GetSttOrEnd( bSrchForward,*pNd ));
                    // liegt Position immer noch im Bereich ?
                    if( (aPos.*fnMove->fnCmpOp)( *rPam.GetMark() ) )
                    {
                        // nur in der AutoTextSection koennen Node stehen, die
                        // nicht angezeigt werden !!
                        if( 0 == ( pFrm = pNd->GetFrm()) ||
                            ( !bInReadOnly && pFrm->IsProtected() ) ||
                            ( pFrm->IsTxtFrm() &&
                                ((SwTxtFrm*)pFrm)->IsHiddenNow() ) )

//                          rNodes[ rNodes.EndOfAutotext ]->StartOfSection().GetIndex()
//                          < aPos.nNode.GetIndex() && aPos.nNode.GetIndex()
//                          < rNodes.EndOfAutotext.GetIndex() &&
//                          0 == ( pFrm = pNd->GetFrm()) &&
//                          pFrm->IsProtected() )
                        {
                            pNd = 0;
                            continue;       // suche weiter
                        }
                        *(SwPosition*)rPam.GetPoint() = aPos;
                    }
                    else
                        pNd = 0;            // kein gueltiger Node
                    break;
                }
                break;
            }
        }
    }
    return pNd;
}

// ----------------------------------------------------------------------

// hier folgen die Move-Methoden ( Foward, Backward; Content, Node, Doc )


void GoStartDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
    // es muss immer ein ContentNode gefunden werden !!
    SwCntntNode* pCNd = rNodes.GoNext( &pPos->nNode );
    if( pCNd )
        pCNd->MakeStartIndex( &pPos->nContent );
}


void GoEndDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = rNodes.GetEndOfContent();
    SwCntntNode* pCNd = GoPreviousNds( &pPos->nNode, TRUE );
    if( pCNd )
        pCNd->MakeEndIndex( &pPos->nContent );
}


void GoStartSection( SwPosition * pPos )
{
    // springe zum Anfang der Section
    SwNodes& rNodes = pPos->nNode.GetNodes();
    USHORT nLevel = rNodes.GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { rNodes.GoStartOfSection( &pPos->nNode ); } while( nLevel-- );

    // steht jetzt schon auf einem CntntNode
    pPos->nNode.GetNode().GetCntntNode()->MakeStartIndex( &pPos->nContent );
}

// gehe an das Ende der akt. Grund-Section


void GoEndSection( SwPosition * pPos )
{
    // springe zum Anfang/Ende der Section
    SwNodes& rNodes = pPos->nNode.GetNodes();
    USHORT nLevel = rNodes.GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { rNodes.GoEndOfSection( &pPos->nNode ); } while( nLevel-- );

    // steht jetzt auf einem EndNode, also zum vorherigen CntntNode
    if( GoPreviousNds( &pPos->nNode, TRUE ) )
        pPos->nNode.GetNode().GetCntntNode()->MakeEndIndex( &pPos->nContent );
}



FASTBOOL GoInDoc( SwPaM & rPam, SwMoveFn fnMove )
{
    (*fnMove->fnDoc)( rPam.pPoint );
    return TRUE;
}


FASTBOOL GoInSection( SwPaM & rPam, SwMoveFn fnMove )
{
    (*fnMove->fnSections)( (SwPosition*)rPam.GetPoint() );
    return TRUE;
}


FASTBOOL GoInNode( SwPaM & rPam, SwMoveFn fnMove )
{
    SwCntntNode *pNd = (*fnMove->fnNds)( &rPam.pPoint->nNode, TRUE );
    if( pNd )
        rPam.pPoint->nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return 0 != pNd;
}


FASTBOOL GoInCntnt( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.pPoint->nNode.GetNode(), &rPam.pPoint->nContent ))
        return TRUE;
    return GoInNode( rPam, fnMove );
}

// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------


FASTBOOL GoPrevPara( SwPaM & rPam, SwPosPara aPosPara )
{
    if( rPam.Move( fnMoveBackward, fnGoNode ) )
    {
        // steht immer auf einem ContentNode !
        SwCntntNode * pNd = rPam.pPoint->nNode.GetNode().GetCntntNode();
        rPam.pPoint->nContent.Assign( pNd,
                            ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
        return TRUE;
    }
    return FALSE;
}


FASTBOOL GoCurrPara( SwPaM & rPam, SwPosPara aPosPara )
{
    SwCntntNode * pNd = rPam.pPoint->nNode.GetNode().GetCntntNode();
    if( pNd )
    {
        xub_StrLen nOld = rPam.pPoint->nContent.GetIndex(),
                   nNew = aPosPara == fnMoveForward ? 0 : pNd->Len();
        // stand er schon auf dem Anfang/Ende dann zum naechsten/vorherigen
        if( nOld != nNew )
        {
            rPam.pPoint->nContent.Assign( pNd, nNew );
            return TRUE;
        }
    }
    // den Node noch etwas bewegen ( auf den naechsten/vorh. CntntNode)
    if( ( aPosPara==fnParaStart && 0 != ( pNd =
            GoPreviousNds( &rPam.pPoint->nNode, TRUE ))) ||
        ( aPosPara==fnParaEnd && 0 != ( pNd =
            GoNextNds( &rPam.pPoint->nNode, TRUE ))) )
    {
        rPam.pPoint->nContent.Assign( pNd,
                        ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ));
        return TRUE;
    }
    return FALSE;
}


FASTBOOL GoNextPara( SwPaM & rPam, SwPosPara aPosPara )
{
    if( rPam.Move( fnMoveForward, fnGoNode ) )
    {
        // steht immer auf einem ContentNode !
        SwCntntNode * pNd = rPam.pPoint->nNode.GetNode().GetCntntNode();
        rPam.pPoint->nContent.Assign( pNd,
                        ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
        return TRUE;
    }
    return FALSE;
}



FASTBOOL GoCurrSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition aSavePos( *rPam.pPoint );        // eine Vergleichsposition
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    (rNds.*fnMove->fnSection)( &rPam.pPoint->nNode );
    SwCntntNode *pNd;
    if( 0 == ( pNd = rPam.pPoint->nNode.GetNode().GetCntntNode()) &&
        0 == ( pNd = (*fnMove->fnNds)( &rPam.pPoint->nNode, TRUE )) )
    {
        *rPam.pPoint = aSavePos;        // Cusror nicht veraendern
        return FALSE;
    }

    rPam.pPoint->nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return aSavePos != *rPam.pPoint;
}


FASTBOOL GoNextSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition aSavePos( *rPam.pPoint );        // eine Vergleichsposition
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    rNds.GoEndOfSection( &rPam.pPoint->nNode );

    // kein weiterer ContentNode vorhanden ?
    if( !GoInCntnt( rPam, fnMoveForward ) )
    {
        *rPam.pPoint = aSavePos;        // Cusror nicht veraendern
        return FALSE;
    }
    (rNds.*fnMove->fnSection)( &rPam.pPoint->nNode );
    SwCntntNode *pNd = rPam.pPoint->nNode.GetNode().GetCntntNode();
    rPam.pPoint->nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return TRUE;
}


FASTBOOL GoPrevSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition aSavePos( *rPam.pPoint );        // eine Vergleichsposition
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    rNds.GoStartOfSection( &rPam.pPoint->nNode );

    // kein weiterer ContentNode vorhanden ?
    if( !GoInCntnt( rPam, fnMoveBackward ))
    {
        *rPam.pPoint = aSavePos;        // Cusror nicht veraendern
        return FALSE;
    }
    (rNds.*fnMove->fnSection)( &rPam.pPoint->nNode );
    SwCntntNode *pNd = rPam.pPoint->nNode.GetNode().GetCntntNode();
    rPam.pPoint->nContent.Assign( pNd,
                            ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ));
    return TRUE;
}




