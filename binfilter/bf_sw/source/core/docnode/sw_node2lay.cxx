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


#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#include "cntfrm.hxx"
#include "tabfrm.hxx"
#include "frmtool.hxx"
#include "section.hxx"
#include "node2lay.hxx"
namespace binfilter {


/* -----------------25.02.99 10:31-------------------
 * Die SwNode2LayImpl-Klasse erledigt die eigentliche Arbeit,
 * die SwNode2Layout-Klasse ist nur die der Oefffentlichkeit bekannte Schnittstelle
 * --------------------------------------------------*/
/*N*/ class SwNode2LayImpl
/*N*/ {
/*N*/ 	SwClientIter *pIter; // Der eigentliche Iterator
/*N*/ 	SvPtrarr *pUpperFrms;// Zum Einsammeln der Upper
/*N*/ 	ULONG nIndex;        // Der Index des einzufuegenden Nodes
/*N*/ 	BOOL bMaster	: 1; // TRUE => nur Master , FALSE => nur Frames ohne Follow
/*N*/ 	BOOL bInit		: 1; // Ist am SwClient bereits ein First()-Aufruf erfolgt?
/*N*/ public:
/*N*/ 	SwNode2LayImpl( const SwNode& rNode, ULONG nIdx, BOOL bSearch );
/*N*/ 	~SwNode2LayImpl() { delete pIter; delete pUpperFrms; }
/*N*/ 	SwFrm* NextFrm(); // liefert den naechsten "sinnvollen" Frame
/*N*/ 	SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode &rNode );
/*N*/ 	void SaveUpperFrms(); // Speichert (und lockt ggf.) die pUpper
/*N*/ 	// Fuegt unter jeden pUpper des Arrays einen Frame ein.
/*N*/ 	void RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd );
/*N*/ 
/*N*/ };

/* -----------------25.02.99 10:38-------------------
 * Hauptaufgabe des Ctor: Das richtige SwModify zu ermitteln,
 * ueber das iteriert wird.
 * Uebergibt man bSearch == TRUE, so wird der naechste Cntnt- oder TableNode
 * gesucht, der Frames besitzt ( zum Einsammeln der pUpper ), ansonsten wird
 * erwartet, das rNode bereits auf einem solchen Cntnt- oder TableNode sitzt,
 * vor oder hinter den eingefuegt werden soll.
 * --------------------------------------------------*/

/*N*/ SwNode2LayImpl::SwNode2LayImpl( const SwNode& rNode, ULONG nIdx, BOOL bSearch )
/*N*/ 	: pUpperFrms( NULL ), nIndex( nIdx ), bInit( FALSE )
/*N*/ {
/*N*/ 	const SwNode* pNd = NULL;
/*N*/ 	if( bSearch || rNode.IsSectionNode() )
/*N*/ 	{
/*N*/ 		// Suche den naechsten Cntnt/TblNode, der einen Frame besitzt,
/*N*/ 		// damit wir uns vor/hinter ihn haengen koennen
/*N*/ 		if( !bSearch && rNode.GetIndex() < nIndex )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwNodeIndex aTmp( *rNode.EndOfSectionNode(), +1 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwNodeIndex aTmp( rNode, -1 );
/*N*/ 			pNd = rNode.GetNodes().GoNextWithFrm( &aTmp );
/*N*/ 			bMaster = TRUE;
/*N*/ 			if( !bSearch && pNd && rNode.EndOfSectionIndex() < pNd->GetIndex() )
/*N*/ 				pNd = NULL; // Nicht ueber den Bereich hinausschiessen
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pNd = &rNode;
/*N*/ 		bMaster = nIndex < rNode.GetIndex();
/*N*/ 	}
/*N*/ 	if( pNd )
/*N*/ 	{
/*N*/ 		SwModify *pMod;
/*N*/ 		if( pNd->IsCntntNode() )
/*?*/ 			pMod = (SwModify*)pNd->GetCntntNode();
/*N*/ 		else
/*N*/ 		{
/*?*/ 			ASSERT( pNd->IsTableNode(), "For Tablenodes only" );
/*?*/ 			pMod = pNd->GetTableNode()->GetTable().GetFrmFmt();
/*N*/ 		}
/*N*/ 		pIter = new SwClientIter( *pMod );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pIter = NULL;
/*N*/ }

/* -----------------25.02.99 10:41-------------------
 * SwNode2LayImpl::NextFrm() liefert den naechsten "sinnvollen" Frame,
 * beim ersten Aufruf wird am eigentlichen Iterator ein First gerufen,
 * danach die Next-Methode. Das Ergebnis wird auf Brauchbarkeit untersucht,
 * so werden keine Follows akzeptiert, ein Master wird beim Einsammeln der
 * pUpper und beim Einfuegen vor ihm akzeptiert. Beim Einfuegen dahinter
 * wird vom Master ausgehend der letzte Follow gesucht und zurueckgegeben.
 * Wenn der Frame innerhalb eines SectionFrms liegt, wird noch festgestellt,
 * ob statt des Frames der SectionFrm der geeignete Rueckgabewert ist, dies
 * ist der Fall, wenn der neu einzufuegende Node ausserhalb des Bereichs liegt.
 * --------------------------------------------------*/
/*N*/ SwFrm* SwNode2LayImpl::NextFrm()
/*N*/ {
/*N*/ 	SwFrm* pRet;
/*N*/ 	if( !pIter )
/*N*/ 		return FALSE;
/*N*/ 	if( !bInit )
/*N*/ 	{
/*N*/ 		 pRet = (SwFrm*)pIter->First(TYPE(SwFrm));
/*N*/ 		 bInit = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pRet = (SwFrm*)pIter->Next();
/*N*/ 	while( pRet )
/*N*/ 	{
/*N*/ 		SwFlowFrm* pFlow = SwFlowFrm::CastFlowFrm( pRet );
/*N*/ 		ASSERT( pFlow, "Cntnt or Table expected?!" );
/*N*/ 		// Follows sind fluechtige Gestalten, deshalb werden sie ignoriert.
/*N*/ 		// Auch wenn wir hinter dem Frame eingefuegt werden sollen, nehmen wir
/*N*/ 		// zunaechst den Master, hangeln uns dann aber zum letzten Follow durch.
/*N*/ 		if( !pFlow->IsFollow() )
/*N*/ 		{
/*N*/ 			if( !bMaster )
/*N*/ 			{
/*N*/ 				while( pFlow->HasFollow() )
/*N*/ 					pFlow = pFlow->GetFollow();
/*N*/ 				pRet = pFlow->GetFrm();
/*N*/ 			}
/*N*/ 			if( pRet->IsInSct() )
/*N*/ 			{
/*N*/ 				SwSectionFrm* pSct = pRet->FindSctFrm();
/*N*/ 				// Vorsicht: Wenn wir in einer Fussnote sind, so kann diese
/*N*/ 				// Layoutmaessig in einem spaltigen Bereich liegen, obwohl
/*N*/ 				// sie nodemaessig ausserhalb liegt. Deshalb muss bei Fussnoten
/*N*/ 				// ueberprueft werden, ob auch der SectionFrm in der Fussnote
/*N*/ 				// und nicht ausserhalb liegt.
/*N*/ 				if( !pRet->IsInFtn() || pSct->IsInFtn() )
/*N*/ 				{
/*N*/ 					ASSERT( pSct && pSct->GetSection(), "Where's my section?" );
/*N*/ 					SwSectionNode* pNd = pSct->GetSection()->GetFmt()->GetSectionNode();
/*N*/ 					ASSERT( pNd, "Lost SectionNode" );
/*N*/ 					// Wenn der erhaltene Frame in einem Bereichsframe steht,
/*N*/ 					// dessen Bereich den Ausgangsnode nicht umfasst, so kehren
/*N*/ 					// wir mit dem SectionFrm zurueck, sonst mit dem Cntnt/TabFrm
/*N*/ 					if( bMaster )
/*N*/ 					{
/*N*/ 						if( pNd->GetIndex() >= nIndex )
/*N*/ 							pRet = pSct;
/*N*/ 					}
/*N*/ 					else if( pNd->EndOfSectionIndex() < nIndex )
/*N*/ 						pRet = pSct;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			return pRet;
/*N*/ 		}
/*N*/ 		pRet = (SwFrm*)pIter->Next();
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void SwNode2LayImpl::SaveUpperFrms()
/*N*/ {
/*N*/ 	pUpperFrms = new SvPtrarr( 0, 20 );
/*N*/ 	SwFrm* pFrm;
/*N*/ 	while( 0 != (pFrm = NextFrm()) )
/*N*/ 	{
/*N*/ 		SwFrm* pPrv = pFrm->GetPrev();
/*N*/ 		pFrm = pFrm->GetUpper();
/*N*/ 		if( pFrm )
/*N*/ 		{
/*N*/ 			if( pFrm->IsFtnFrm() )
/*?*/ 				((SwFtnFrm*)pFrm)->ColLock();
/*N*/ 			else if( pFrm->IsInSct() )
/*?*/ 				pFrm->FindSctFrm()->ColLock();
/*N*/ 			if( pPrv && pPrv->IsSctFrm() )
/*N*/ 				((SwSectionFrm*)pPrv)->LockJoin();
/*N*/ 			pUpperFrms->Insert( (void*)pPrv, pUpperFrms->Count() );
/*N*/ 			pUpperFrms->Insert( (void*)pFrm, pUpperFrms->Count() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete pIter;
/*N*/ 	pIter = NULL;
/*N*/ }

/*N*/ SwLayoutFrm* SwNode2LayImpl::UpperFrm( SwFrm* &rpFrm, const SwNode &rNode )
/*N*/ {
/*N*/ 	rpFrm = NextFrm();
/*N*/ 	if( !rpFrm )
/*N*/ 		return NULL;
/*N*/ 	SwLayoutFrm* pUpper = rpFrm->GetUpper();
/*N*/ 	if( rpFrm->IsSctFrm() )
/*N*/ 	{
/*?*/ 		const SwNode* pNode = rNode.StartOfSectionNode();
/*?*/ 		if( pNode->IsSectionNode() )
/*?*/ 		{
/*?*/ 			SwFrm* pFrm = bMaster ? rpFrm->FindPrev() : rpFrm->FindNext();
/*?*/ 			if( pFrm && pFrm->IsSctFrm() )
/*?*/ 			{
/*?*/ 				if( ((SwSectionNode*)pNode)->GetSection() ==
/*?*/ 					*((SwSectionFrm*)pFrm)->GetSection() )
/*?*/ 				{
/*?*/ 					rpFrm = bMaster ? NULL : ((SwLayoutFrm*)pFrm)->Lower();
/*?*/ 					return ((SwLayoutFrm*)pFrm);
/*?*/ 				}
/*?*/                 pUpper = new SwSectionFrm(((SwSectionNode*)pNode)->GetSection());
/*?*/ 				pUpper->Paste( rpFrm->GetUpper(),
/*?*/ 							   bMaster ? rpFrm : rpFrm->GetNext() );
/*?*/                 static_cast<SwSectionFrm*>(pUpper)->Init();
/*?*/ 				rpFrm = NULL;
/*?*/ 				return pUpper;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	};
/*N*/ 	if( !bMaster )
/*N*/ 		rpFrm = rpFrm->GetNext();
/*N*/ 	return pUpper;
/*N*/ }

/*N*/ void SwNode2LayImpl::RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd )
/*N*/ {
/*N*/ 	ASSERT( pUpperFrms, "RestoreUpper without SaveUpper?" )
/*N*/ 	SwNode* pNd;
/*N*/ 	SwDoc *pDoc = rNds.GetDoc();
/*N*/ 	BOOL bFirst = TRUE;
/*N*/ 	for( ; nStt < nEnd; ++nStt )
/*N*/ 	{
/*N*/ 		SwFrm* pNew = 0;
/*N*/ 		SwFrm* pNxt;
/*N*/ 		SwLayoutFrm* pUp;
/*N*/ 		if( (pNd = rNds[nStt])->IsCntntNode() )
/*?*/ 			for( USHORT n = 0; n < pUpperFrms->Count(); )
/*?*/ 			{
/*?*/ 				pNxt = (SwFrm*)(*pUpperFrms)[n++];
/*?*/ 				if( bFirst && pNxt && pNxt->IsSctFrm() )
/*?*/ 					((SwSectionFrm*)pNxt)->UnlockJoin();
/*?*/ 				pUp = (SwLayoutFrm*)(*pUpperFrms)[n++];
/*?*/ 				if( pNxt )
/*?*/ 					pNxt = pNxt->GetNext();
/*?*/ 				else
/*?*/ 					pNxt = pUp->Lower();
/*?*/ 				pNew = ((SwCntntNode*)pNd)->MakeFrm();
/*?*/ 				pNew->Paste( pUp, pNxt );
/*?*/ 				(*pUpperFrms)[n-2] = pNew;
/*?*/ 			}
/*N*/ 		else if( pNd->IsTableNode() )
/*?*/ 			for( USHORT x = 0; x < pUpperFrms->Count(); )
/*?*/ 			{
/*?*/ 				pNxt = (SwFrm*)(*pUpperFrms)[x++];
/*?*/ 				if( bFirst && pNxt && pNxt->IsSctFrm() )
/*?*/ 					((SwSectionFrm*)pNxt)->UnlockJoin();
/*?*/ 				pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
/*?*/ 				if( pNxt )
/*?*/ 					pNxt = pNxt->GetNext();
/*?*/ 				else
/*?*/ 					pNxt = pUp->Lower();
/*?*/ 				pNew = ((SwTableNode*)pNd)->MakeFrm();
/*?*/ 				ASSERT( pNew->IsTabFrm(), "Table exspected" );
/*?*/ 				pNew->Paste( pUp, pNxt );
/*?*/ 				((SwTabFrm*)pNew)->RegistFlys();
/*?*/ 				(*pUpperFrms)[x-2] = pNew;
/*?*/ 			}
/*N*/ 		else if( pNd->IsSectionNode() )
/*N*/ 		{
/*N*/ 			nStt = pNd->EndOfSectionIndex();
/*N*/ 			for( USHORT x = 0; x < pUpperFrms->Count(); )
/*N*/ 			{
/*N*/ 				pNxt = (SwFrm*)(*pUpperFrms)[x++];
/*N*/ 				if( bFirst && pNxt && pNxt->IsSctFrm() )
/*N*/ 					((SwSectionFrm*)pNxt)->UnlockJoin();
/*N*/ 				pUp = (SwLayoutFrm*)(*pUpperFrms)[x++];
/*N*/ 				ASSERT( pUp->GetUpper() || pUp->IsFlyFrm(), "Lost Upper" );
/*N*/ 				::binfilter::_InsertCnt( pUp, pDoc, pNd->GetIndex(), FALSE, nStt+1, pNxt );
/*N*/ 				pNxt = pUp->Lower();
/*N*/ 				if( pNxt )
/*N*/ 					while( pNxt->GetNext() )
/*N*/ 						pNxt = pNxt->GetNext();
/*N*/ 				(*pUpperFrms)[x-2] = pNxt;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		bFirst = FALSE;
/*N*/ 	}
/*N*/ 	for( USHORT x = 0; x < pUpperFrms->Count(); ++x )
/*N*/ 	{
/*N*/ 		SwFrm* pTmp = (SwFrm*)(*pUpperFrms)[++x];
/*N*/ 		if( pTmp->IsFtnFrm() )
/*?*/ 			((SwFtnFrm*)pTmp)->ColUnlock();
/*N*/ 		else if( pTmp->IsInSct() )
/*?*/ 			pTmp->FindSctFrm()->ColUnlock();
/*N*/ 	}
/*N*/ }


/*N*/ SwNode2Layout::SwNode2Layout( const SwNode& rNd, ULONG nIdx )
/*N*/ {
/*N*/ 	pImpl = new SwNode2LayImpl( rNd, nIdx, FALSE );
/*N*/ }

/*N*/ SwNode2Layout::SwNode2Layout( const SwNode& rNd )
/*N*/ {
/*N*/ 	pImpl = new SwNode2LayImpl( rNd, rNd.GetIndex(), TRUE );
/*N*/ 	pImpl->SaveUpperFrms();
/*N*/ }

/*N*/ void SwNode2Layout::RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd )
/*N*/ {
/*N*/ 	ASSERT( pImpl, "RestoreUpperFrms without SaveUpperFrms" );
/*N*/ 	pImpl->RestoreUpperFrms( rNds, nStt, nEnd );
/*N*/ }

/*N*/ SwFrm* SwNode2Layout::NextFrm()
/*N*/ {
/*N*/ 	return pImpl->NextFrm();
/*N*/ }

/*N*/ SwLayoutFrm* SwNode2Layout::UpperFrm( SwFrm* &rpFrm, const SwNode &rNode )
/*N*/ {
/*N*/ 	return pImpl->UpperFrm( rpFrm, rNode );
/*N*/ }

/*N*/ SwNode2Layout::~SwNode2Layout()
/*N*/ {
/*N*/ 	delete pImpl;
/*N*/ }



}
