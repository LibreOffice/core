/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <bf_svx/protitem.hxx>

#include <pagefrm.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <pamtyp.hxx>
#include <txtfrm.hxx>
#include <section.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <crsskip.hxx>
namespace binfilter {


// fuer den dummen ?MSC-? Compiler
/*N*/inline xub_StrLen GetSttOrEnd( BOOL bCondition, const SwCntntNode& rNd )
/*N*/{
/*N*/	return bCondition ? 0 : rNd.Len();
/*N*/}

/*************************************************************************
|*
|*	SwPosition
|*
|*	Beschreibung		PAM.DOC
|*	Ersterstellung		VB	4.3.91
|*	Letzte Aenderung	VB	4.3.91
|*
*************************************************************************/


/*N*/ SwPosition::SwPosition(const SwPosition &rPos)
/*N*/ 	: nNode(rPos.nNode),nContent(rPos.nContent)
/*N*/ {
/*N*/ }


/*N*/ SwPosition::SwPosition( const SwNodeIndex &rNode, const SwIndex &rCntnt )
/*N*/ 	: nNode( rNode ),nContent( rCntnt )
/*N*/ {
/*N*/ }

/*N*/ SwPosition::SwPosition( const SwNodeIndex &rNode )
/*N*/ 	: nNode( rNode ), nContent( 0 )
/*N*/ {
/*N*/ }

/*N*/ SwPosition::SwPosition( const SwNode& rNode )
/*N*/ 	: nNode( rNode ), nContent( 0 )
/*N*/ {
/*N*/ }


/*N*/ SwPosition &SwPosition::operator=(const SwPosition &rPos)
/*N*/ {
/*N*/ 	nNode = rPos.nNode;
/*N*/ 	nContent = rPos.nContent;
/*N*/ 	return *this;
/*N*/ }


/*N*/ FASTBOOL SwPosition::operator<(const SwPosition &rPos) const
/*N*/ {
/*N*/ 	if( nNode < rPos.nNode )
/*N*/ 		return TRUE;
/*N*/ 	if( nNode == rPos.nNode )
/*N*/ 		return ( nContent < rPos.nContent );
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL SwPosition::operator>(const SwPosition &rPos) const
/*N*/ {
/*N*/ 	if(nNode > rPos.nNode )
/*N*/ 		return TRUE;
/*N*/ 	if( nNode == rPos.nNode )
/*N*/ 		return ( nContent > rPos.nContent );
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL SwPosition::operator<=(const SwPosition &rPos) const
/*N*/ {
/*N*/ 	if(nNode < rPos.nNode )
/*N*/ 		return TRUE;
/*N*/ 	if( nNode == rPos.nNode )
/*N*/ 		return ( nContent <= rPos.nContent );
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL SwPosition::operator>=(const SwPosition &rPos) const
/*N*/ {
/*N*/ 	if(nNode > rPos.nNode )
/*N*/ 		return TRUE;
/*N*/ 	if( nNode == rPos.nNode )
/*N*/ 		return ( nContent >= rPos.nContent );
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL SwPosition::operator==(const SwPosition &rPos) const
/*N*/ {
/*N*/ 	return
/*N*/ 		( ( nNode == rPos.nNode ) && ( nContent == rPos.nContent ) ?
/*N*/ 			TRUE: FALSE);
/*N*/ }


/*N*/ FASTBOOL SwPosition::operator!=(const SwPosition &rPos) const
/*N*/ {
/*N*/ 	if( nNode != rPos.nNode )
/*N*/ 		return TRUE;
/*N*/ 	return ( nContent != rPos.nContent );
/*N*/ }

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
        else if( rEnd1 == rStt2 )
            nRet = POS_COLLIDE_END;
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
    else if( rEnd2 == rStt1 )
        nRet = POS_COLLIDE_START;
    else
        nRet = POS_BEHIND;
    return nRet;
}


/*  */

enum CHKSECTION { Chk_Both, Chk_One, Chk_None };


/*N*/ CHKSECTION lcl_TstIdx( ULONG nSttIdx, ULONG nEndIdx, const SwNode& rEndNd )
/*N*/ {
/*N*/ 	ULONG nStt = rEndNd.StartOfSectionIndex(), nEnd = rEndNd.GetIndex();
/*N*/ 	CHKSECTION eSec = nStt < nSttIdx && nEnd >= nSttIdx ? Chk_One : Chk_None;
/*N*/ 	if( nStt < nEndIdx && nEnd >= nEndIdx )
/*N*/ 		return( eSec == Chk_One ? Chk_Both : Chk_One );
/*N*/ 	return eSec;
/*N*/ }


/*N*/ FASTBOOL lcl_ChkOneRange( CHKSECTION eSec, BOOL bChkSections,
/*N*/ 					const SwNode& rBaseEnd, ULONG nStt, ULONG nEnd )
/*N*/ {
/*N*/ 	if( eSec != Chk_Both )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if( !bChkSections )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	// suche die umspannende Section
/*N*/ 	const SwNodes& rNds = rBaseEnd.GetNodes();
/*N*/ 	const SwNode *pTmp, *pNd = rNds[ nStt ];
/*N*/ 	if( !pNd->IsStartNode() )
/*N*/ 		pNd = pNd->StartOfSectionNode();
/*N*/ 
/*N*/ 	if( pNd == rNds[ nEnd ]->StartOfSectionNode() )
/*N*/ 		return TRUE;		// der gleiche StartNode, die selbe Section
/*N*/ 
/*N*/ 	// steht schon auf einem GrundSection Node ? Fehler !!!
/*N*/ 	if( !pNd->StartOfSectionIndex() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	while( ( pTmp = pNd->StartOfSectionNode())->EndOfSectionNode() !=
/*N*/ 			&rBaseEnd )
/*N*/ 		pNd = pTmp;
/*N*/ 
/*N*/ 	ULONG nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
/*N*/ 	return nSttIdx <= nStt && nStt <= nEndIdx &&
/*N*/ 		   nSttIdx <= nEnd && nEnd <= nEndIdx ? TRUE : FALSE;
/*N*/ }


/*N*/ FASTBOOL CheckNodesRange( const SwNodeIndex& rStt,
/*N*/ 						const SwNodeIndex& rEnd, FASTBOOL bChkSection )
/*N*/ {
/*N*/ 	const SwNodes& rNds = rStt.GetNodes();
/*N*/ 	ULONG nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
/*N*/ 	CHKSECTION eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfContent() );
/*N*/ 	if( Chk_None != eSec ) return eSec == Chk_Both ? TRUE : FALSE;
/*N*/ 
/*N*/ 	eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfAutotext() );
/*N*/ 	if( Chk_None != eSec )
/*N*/ 		return lcl_ChkOneRange( eSec, bChkSection,
/*N*/ 							rNds.GetEndOfAutotext(), nStt, nEnd );
/*N*/ 
/*N*/ 	eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfPostIts() );
/*N*/ 	if( Chk_None != eSec )
/*N*/ 		return lcl_ChkOneRange( eSec, bChkSection,
/*N*/ 							rNds.GetEndOfPostIts(), nStt, nEnd );
/*N*/ 
/*?*/ 	eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfInserts() );
/*?*/ 	if( Chk_None != eSec )
/*?*/ 		return lcl_ChkOneRange( eSec, bChkSection,
/*?*/ 							rNds.GetEndOfInserts(), nStt, nEnd );
/*?*/ 
/*?*/ 	eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfRedlines() );
/*?*/ 	if( Chk_None != eSec )
/*?*/ 		return lcl_ChkOneRange( eSec, bChkSection,
/*?*/ 							rNds.GetEndOfRedlines(), nStt, nEnd );
/*?*/ 
/*?*/ 	return FALSE;		// liegt irgendwo dazwischen, FEHLER
/*N*/ }


/*N*/ FASTBOOL GoNext(SwNode* pNd, SwIndex * pIdx, USHORT nMode )
/*N*/ {
/*N*/ 	if( pNd->IsCntntNode() )
/*N*/ 		return ((SwCntntNode*)pNd)->GoNext( pIdx, nMode );
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL GoPrevious( SwNode* pNd, SwIndex * pIdx, USHORT nMode )
/*N*/ {
/*N*/ 	if( pNd->IsCntntNode() )
/*N*/ 		return ((SwCntntNode*)pNd)->GoPrevious( pIdx, nMode );
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ SwCntntNode* GoNextNds( SwNodeIndex* pIdx, FASTBOOL bChk )
/*N*/ {
/*N*/ 	SwNodeIndex aIdx( *pIdx );
/*N*/ 	SwCntntNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
/*N*/ 	if( pNd )
/*N*/ 	{
/*N*/ 		if( bChk && 1 != aIdx.GetIndex() - pIdx->GetIndex() &&
/*N*/ 			!CheckNodesRange( *pIdx, aIdx, TRUE ) )
/*N*/ 				pNd = 0;
/*N*/ 		else
/*N*/ 			*pIdx = aIdx;
/*N*/ 	}
/*N*/ 	return pNd;
/*N*/ }


/*N*/ SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, FASTBOOL bChk )
/*N*/ {
/*N*/ 	SwNodeIndex aIdx( *pIdx );
/*N*/ 	SwCntntNode* pNd = aIdx.GetNodes().GoPrevious( &aIdx );
/*N*/ 	if( pNd )
/*N*/ 	{
/*N*/ 		if( bChk && 1 != pIdx->GetIndex() - aIdx.GetIndex() &&
/*N*/ 			!CheckNodesRange( *pIdx, aIdx, TRUE ) )
/*N*/ 				pNd = 0;
/*N*/ 		else
/*N*/ 			*pIdx = aIdx;
/*N*/ 	}
/*N*/ 	return pNd;
/*N*/ }

// ----------------------------------------------------------------------

/*************************************************************************
|*
|*	SwPointAndMark
|*
|*	Beschreibung		PAM.DOC
|*	Ersterstellung		VB	4.3.91
|*	Letzte Aenderung	JP	6.5.91
|*
*************************************************************************/

/*N*/ SwPaM::SwPaM( const SwPosition& rPos, SwPaM* pRing )
/*N*/ 	: aBound1( rPos ), aBound2( rPos ), Ring( pRing )
/*N*/ {
/*N*/ 	pPoint = pMark = &aBound1;
/*N*/ }

/*N*/ SwPaM::SwPaM( const SwPosition& rMk, const SwPosition& rPt, SwPaM* pRing )
/*N*/ 	: aBound1( rMk ), aBound2( rPt ), Ring( pRing )
/*N*/ {
/*N*/ 	pMark = &aBound1;
/*N*/ 	pPoint = &aBound2;
/*N*/ }


/*N*/ SwPaM::SwPaM( const SwNode& rMk, const SwNode& rPt,
/*N*/ 				long nMkOffset, long nPtOffset, SwPaM* pRing )
/*N*/ 	: aBound1( rMk ), aBound2( rPt ), Ring( pRing )
/*N*/ {
/*N*/ 	if( nMkOffset )
/*N*/ 		aBound1.nNode += nMkOffset;
/*N*/ 	if( nPtOffset )
/*N*/ 		aBound2.nNode += nPtOffset;
/*N*/ 
/*N*/ 	aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), 0 );
/*N*/ 	aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), 0 );
/*N*/ 	pMark = &aBound1;
/*N*/ 	pPoint = &aBound2;
/*N*/ }


/*N*/ SwPaM::SwPaM( const SwNode& rMk, xub_StrLen nMkCntnt,
/*N*/ 			  const SwNode& rPt, xub_StrLen nPtCntnt, SwPaM* pRing )
/*N*/ 	: aBound1( rMk ), aBound2( rPt ), Ring( pRing )
/*N*/ {
/*N*/ 	aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), nMkCntnt );
/*N*/ 	aBound2.nContent.Assign( aBound2.nNode.GetNode().GetCntntNode(), nPtCntnt );
/*N*/ 	pMark = &aBound1;
/*N*/ 	pPoint = &aBound2;
/*N*/ }

/*N*/ SwPaM::SwPaM( SwPaM &rPam )
/*N*/ 	: aBound1( *(rPam.pPoint) ), aBound2( *(rPam.pMark) ), Ring( &rPam )
/*N*/ {
/*N*/ 	pPoint = &aBound1;
/*N*/ 	pMark  = rPam.HasMark() ? &aBound2 : pPoint;
/*N*/ }

/*N*/ SwPaM::SwPaM( const SwNode& rNd, xub_StrLen nCntnt, SwPaM* pRing )
/*N*/ 	: aBound1( rNd ), aBound2( rNd ), Ring( pRing )
/*N*/ {
/*N*/ 	aBound1.nContent.Assign( aBound1.nNode.GetNode().GetCntntNode(), nCntnt );
/*N*/ 	aBound2.nContent = aBound1.nContent;
/*N*/ 	pPoint = pMark = &aBound1;
/*N*/ }

/*N*/ SwPaM::SwPaM( const SwNodeIndex& rNd, xub_StrLen nCntnt, SwPaM* pRing )
/*N*/ 	: aBound1( rNd ), aBound2( rNd ), Ring( pRing )
/*N*/ {
/*N*/ 	aBound1.nContent.Assign( rNd.GetNode().GetCntntNode(), nCntnt );
/*N*/ 	aBound2.nContent = aBound1.nContent;
/*N*/ 	pPoint = pMark = &aBound1;
/*N*/ }

/*N*/ SwPaM::~SwPaM() {}

/*N*/ void SwPaM::SetMark()
/*N*/ {
/*N*/ 	if(pPoint == &aBound1)
/*N*/ 		pMark = &aBound2;
/*N*/ 	else
/*N*/ 		pMark = &aBound1;
/*N*/ 	(*pMark) = (*pPoint);
/*N*/ }

/*N*/ #ifdef DBG_UTIL

/*N*/ void SwPaM::Exchange()
/*N*/ {
/*N*/ 	if(pPoint != pMark)
/*N*/ 	{
/*N*/ 		SwPosition *pTmp = pPoint;
/*N*/ 		pPoint = pMark;
/*N*/ 		pMark = pTmp;
/*N*/ 	}
/*N*/ }
/*N*/ #endif


/*?*/SwPaM &SwPaM::operator=( SwPaM &rPam )
/*?*/{
/*?*/	*pPoint = *( rPam.pPoint );
/*?*/	if( rPam.HasMark() )
/*?*/	{
/*?*/		SetMark();
/*?*/		*pMark = *( rPam.pMark );
/*?*/	}
/*?*/ 	else
/*?*/ 		DeleteMark();
/*?*/	return *this;
/*?*/}

// Bewegen des Cursors


/*N*/ FASTBOOL SwPaM::Move( SwMoveFn fnMove, SwGoInDoc fnGo )
/*N*/ {
/*N*/ 	return (*fnGo)( *this, fnMove );
/*N*/ }


/*************************************************************************
|*
|*	  void SwPaM::MakeRegion( SwMoveFn, SwPaM*, const SwPaM* )
|*
|*	  Beschreibung		Setzt den 1. SwPaM auf den uebergebenen SwPaM
|*						oder setzt auf den Anfang oder Ende vom Document.
|*						SPoint bleibt auf der Position stehen, GetMark aendert
|*						sich entsprechend !
|*
|*	  Parameter 		SwDirection 	gibt an, ob an Anfang / Ende
|*						SwPaM * 		der zu setzende Bereich
|*						const SwPaM&	der enventuell vorgegeben Bereich
|*	  Return-Werte		SwPaM*			der entsprehend neu gesetzte Bereich
|*
|*	  Ersterstellung	JP 26.04.91
|*	  Letzte Aenderung	JP 26.04.91
|*
*************************************************************************/




/*N*/ USHORT SwPaM::GetPageNum( BOOL bAtPoint, const Point* pLayPos )
/*N*/ {
/*N*/ 	// return die Seitennummer am Cursor
/*N*/ 	// (fuer Reader + Seitengebundene Rahmen)
/*N*/ 	const SwCntntFrm* pCFrm;
/*N*/ 	const SwPageFrm *pPg;
/*N*/ 	const SwCntntNode *pNd ;
/*N*/ 	const SwPosition* pPos = bAtPoint ? pPoint : pMark;
/*N*/ 
/*N*/ 	if( 0 != ( pNd = pPos->nNode.GetNode().GetCntntNode() ) &&
/*N*/ 		0 != ( pCFrm = pNd->GetFrm( pLayPos, pPos, FALSE )) &&
/*N*/ 		0 != ( pPg = pCFrm->FindPageFrm() ))
/*N*/ 		return pPg->GetPhyPageNum();
/*N*/ 	return 0;
/*N*/ }

//--------------------	Suche nach Formaten( FormatNamen ) -----------------

// die Funktion gibt in Suchrichtung den folgenden Node zurueck.
// Ist in der Richtung keiner mehr vorhanden oder ist dieser ausserhalb
// des Bereiches, wird ein 0 Pointer returnt.
// Das rbFirst gibt an, ob es man zu erstenmal einen Node holt. Ist das der
// Fall, darf die Position vom Pam nicht veraendert werden!



// ----------------------------------------------------------------------

// hier folgen die Move-Methoden ( Foward, Backward; Content, Node, Doc )


/*N*/ void GoStartDoc( SwPosition * pPos )
/*N*/ {
/*N*/ 	SwNodes& rNodes = pPos->nNode.GetNodes();
/*N*/ 	pPos->nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
/*N*/ 	// es muss immer ein ContentNode gefunden werden !!
/*N*/ 	SwCntntNode* pCNd = rNodes.GoNext( &pPos->nNode );
/*N*/ 	if( pCNd )
/*N*/ 		pCNd->MakeStartIndex( &pPos->nContent );
/*N*/ }


/*N*/ void GoEndDoc( SwPosition * pPos )
/*N*/ {
/*N*/ 	SwNodes& rNodes = pPos->nNode.GetNodes();
/*N*/ 	pPos->nNode = rNodes.GetEndOfContent();
/*N*/ 	SwCntntNode* pCNd = GoPreviousNds( &pPos->nNode, TRUE );
/*N*/ 	if( pCNd )
/*N*/ 		pCNd->MakeEndIndex( &pPos->nContent );
/*N*/ }


/*N*/ void GoStartSection( SwPosition * pPos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// gehe an das Ende der akt. Grund-Section


/*N*/ void GoEndSection( SwPosition * pPos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }



/*N*/ FASTBOOL GoInDoc( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/ 	(*fnMove->fnDoc)( rPam.GetPoint() );
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ FASTBOOL GoInSection( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ FASTBOOL GoInNode( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/ 	SwCntntNode *pNd = (*fnMove->fnNds)( &rPam.GetPoint()->nNode, TRUE );
/*N*/ 	if( pNd )
/*N*/ 		rPam.GetPoint()->nContent.Assign( pNd,
/*N*/ 						::binfilter::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
/*N*/ 	return 0 != pNd;
/*N*/ }


/*N*/ FASTBOOL GoInCntnt( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/ 	if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
/*N*/ 						&rPam.GetPoint()->nContent, CRSR_SKIP_CHARS ))
/*N*/ 		return TRUE;
/*N*/ 	return GoInNode( rPam, fnMove );
/*N*/ }

/*N*/ FASTBOOL GoInCntntCells( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------


/*N*/ FASTBOOL GoPrevPara( SwPaM & rPam, SwPosPara aPosPara )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL GoCurrPara( SwPaM & rPam, SwPosPara aPosPara )
/*N*/ {
/*N*/ 	SwPosition& rPos = *rPam.GetPoint();
/*N*/ 	SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
/*N*/ 	if( pNd )
/*N*/ 	{
/*N*/ 		xub_StrLen nOld = rPos.nContent.GetIndex(),
/*N*/ 				   nNew = aPosPara == fnMoveForward ? 0 : pNd->Len();
/*N*/ 		// stand er schon auf dem Anfang/Ende dann zum naechsten/vorherigen
/*N*/ 		if( nOld != nNew )
/*N*/ 		{
/*N*/ 			rPos.nContent.Assign( pNd, nNew );
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// den Node noch etwas bewegen ( auf den naechsten/vorh. CntntNode)
/*N*/ 	if( ( aPosPara==fnParaStart && 0 != ( pNd =
/*N*/ 			GoPreviousNds( &rPos.nNode, TRUE ))) ||
/*N*/ 		( aPosPara==fnParaEnd && 0 != ( pNd =
/*N*/ 			GoNextNds( &rPos.nNode, TRUE ))) )
/*N*/ 	{
/*N*/ 		rPos.nContent.Assign( pNd,
/*N*/ 						::binfilter::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ));
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL GoNextPara( SwPaM & rPam, SwPosPara aPosPara )
/*N*/ {
/*N*/ 	if( rPam.Move( fnMoveForward, fnGoNode ) )
/*N*/ 	{
/*N*/ 		// steht immer auf einem ContentNode !
/*N*/ 		SwPosition& rPos = *rPam.GetPoint();
/*N*/ 		SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
/*N*/ 		rPos.nContent.Assign( pNd,
/*N*/ 						::binfilter::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }



/*N*/ FASTBOOL GoCurrSection( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {
/*N*/ 	SwPosition& rPos = *rPam.GetPoint();
/*N*/ 	SwPosition aSavePos( rPos );		// eine Vergleichsposition
/*N*/ 	SwNodes& rNds = aSavePos.nNode.GetNodes();
/*N*/ 	(rNds.*fnMove->fnSection)( &rPos.nNode );
/*N*/ 	SwCntntNode *pNd;
/*N*/ 	if( 0 == ( pNd = rPos.nNode.GetNode().GetCntntNode()) &&
/*N*/ 		0 == ( pNd = (*fnMove->fnNds)( &rPos.nNode, TRUE )) )
/*N*/ 	{
/*?*/ 		rPos = aSavePos;		// Cusror nicht veraendern
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	rPos.nContent.Assign( pNd,
/*N*/ 						::binfilter::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
/*N*/ 	return aSavePos != rPos;
/*N*/ }


/*N*/ FASTBOOL GoNextSection( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ FASTBOOL GoPrevSection( SwPaM & rPam, SwMoveFn fnMove )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return TRUE;
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
