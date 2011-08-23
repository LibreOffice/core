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

#include <errhdl.hxx>

#include "pam.hxx"
#include "swtable.hxx"
#include "pagefrm.hxx"

#include <horiornt.hxx>

#include "viewsh.hxx"
#include "doc.hxx"
#include "viewimp.hxx"
#include "dflyobj.hxx"
#include "frmtool.hxx"
#include "dcontact.hxx"

#include <bf_svx/brkitem.hxx>
#include <bf_svx/keepitem.hxx>

#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>
#include <bf_svx/ulspitem.hxx>
#include <tgrditem.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <bf_svx/pgrditem.hxx>
#include <paratr.hxx>

#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "tabfrm.hxx"
#include "pagedesc.hxx"
#include "layact.hxx"
#include "frmsh.hxx"
#include "fmtornt.hxx"
#include "flyfrms.hxx"
#include "sectfrm.hxx"
#include "section.hxx"
#include "dbg_lay.hxx"
#include "lineinfo.hxx"
namespace binfilter {

/*N*/ BOOL SwFlowFrm::bMoveBwdJump = FALSE;


/*************************************************************************
|*
|*	SwFlowFrm::SwFlowFrm()
|*
|*	Ersterstellung		MA 26. Apr. 95
|*	Letzte Aenderung	MA 26. Apr. 95
|*
|*************************************************************************/


/*M*/ SwFlowFrm::SwFlowFrm( SwFrm &rFrm ) :
/*M*/ 	rThis( rFrm ),
/*M*/ 	pFollow( 0 )
/*M*/ {
/*M*/     bLockJoin = bIsFollow = bCntntLock = bOwnFtnNum =
/*M*/         bFtnLock = bFlyLock = FALSE;
/*M*/ }


/*************************************************************************
|*
|*  SwFlowFrm::IsFollowLocked()
|*     return TRUE if any follow has the JoinLocked flag
|*
|*************************************************************************/

/*M*/ sal_Bool SwFlowFrm::HasLockedFollow() const
/*M*/ {
/*M*/     const SwFlowFrm* pFrm = GetFollow();
/*M*/     while( pFrm )
/*M*/     {
/*M*/         if( pFrm->IsJoinLocked() )
/*M*/             return sal_True;
/*M*/         pFrm = pFrm->GetFollow();
/*M*/     }
/*M*/     return sal_False;
/*M*/ }

/*************************************************************************
|*
|*	SwFlowFrm::IsKeepFwdMoveAllowed()
|*
|*	Ersterstellung		MA 20. Jul. 94
|*	Letzte Aenderung	MA 16. May. 95
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::IsKeepFwdMoveAllowed()
/*N*/ {
/*N*/ 	//Wenn der Vorgaenger das KeepAttribut traegt und auch dessen
/*N*/ 	//Vorgaenger usw. bis zum ersten der Kette und fuer diesen das
/*N*/ 	//IsFwdMoveAllowed ein FALSE liefert, so ist das Moven eben nicht erlaubt.
/*N*/ 	SwFrm *pFrm = &rThis;
/*N*/ 	if ( !pFrm->IsInFtn() )
/*N*/ 		do
/*N*/ 		{	if ( pFrm->GetAttrSet()->GetKeep().GetValue() )
/*N*/ 				pFrm = pFrm->GetIndPrev();
/*N*/ 			else
/*N*/ 				return TRUE;
/*N*/ 		} while ( pFrm );
/*N*/ 
/*N*/ 				  //Siehe IsFwdMoveAllowed()
/*N*/ 	return pFrm ? pFrm->GetIndPrev() != 0 : FALSE;
/*N*/ }

/*************************************************************************
|*
|*	  SwFlowFrm::CheckKeep()
|*
|*	  Beschreibung
|*	  Ersterstellung	MA 20. Jun. 95
|*	  Letzte Aenderung	MA 09. Apr. 97
|*
|*************************************************************************/


/*N*/ void SwFlowFrm::CheckKeep()
/*N*/ {
/*N*/ 	//Den 'letzten' Vorgaenger mit KeepAttribut anstossen, denn
/*N*/ 	//die ganze Truppe koennte zuruckrutschen.
/*N*/ 	SwFrm *pPre = rThis.GetIndPrev();
/*N*/ 	if( pPre->IsSctFrm() )
/*N*/ 	{
/*N*/ 		SwFrm *pLast = ((SwSectionFrm*)pPre)->FindLastCntnt();
/*N*/ 		if( pLast && pLast->FindSctFrm() == pPre )
/*N*/ 			pPre = pLast;
/*N*/ 		else
/*N*/ 			return;
/*N*/ 	}
/*N*/ 	SwFrm* pTmp;
/*N*/ 	BOOL bKeep;
/*N*/ 	while ( TRUE == (bKeep = pPre->GetAttrSet()->GetKeep().GetValue()) &&
/*N*/ 			0 != ( pTmp = pPre->GetIndPrev() ) )
/*N*/ 	{
/*N*/ 		if( pTmp->IsSctFrm() )
/*N*/ 		{
/*N*/ 			SwFrm *pLast = ((SwSectionFrm*)pTmp)->FindLastCntnt();
/*N*/ 			if( pLast && pLast->FindSctFrm() == pTmp )
/*N*/ 				pTmp = pLast;
/*N*/ 			else
/*?*/ 				break;
/*N*/ 		}
/*N*/ 		pPre = pTmp;
/*N*/ 	}
/*N*/ 	if ( bKeep )
/*N*/ 		pPre->InvalidatePos();
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::IsKeep()
|*
|*	Ersterstellung		MA 09. Apr. 97
|*	Letzte Aenderung	MA 09. Apr. 97
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::IsKeep( const SwBorderAttrs &rAttrs ) const
/*N*/ {
/*N*/ 	BOOL bKeep = !rThis.IsInFtn() && rAttrs.GetAttrSet().GetKeep().GetValue();
/*N*/ 	//Keep Zaehlt nicht wenn die Umbrueche dagegen sprechen.
/*N*/ 	if ( bKeep )
/*N*/ 	{
/*N*/ 		switch ( rAttrs.GetAttrSet().GetBreak().GetBreak() )
/*N*/ 		{
/*?*/ 			case SVX_BREAK_COLUMN_AFTER:
/*?*/ 			case SVX_BREAK_COLUMN_BOTH:
/*?*/ 			case SVX_BREAK_PAGE_AFTER:
/*?*/ 			case SVX_BREAK_PAGE_BOTH:
/*?*/ 				bKeep = FALSE;
/*N*/ 		}
/*N*/ 		if ( bKeep )
/*N*/ 		{
/*N*/ 			SwFrm *pNxt;
/*N*/ 			if( 0 != (pNxt = rThis.FindNextCnt()) &&
/*N*/ 				(!pFollow || pNxt != pFollow->GetFrm()))
/*N*/ 			{
/*N*/                 const SwAttrSet* pSet = NULL;
/*N*/ 
/*N*/                 if ( pNxt->IsInTab() )
/*N*/                 {
/*N*/                     SwTabFrm* pTab = pNxt->FindTabFrm();
/*N*/                     if ( ! rThis.IsInTab() || rThis.FindTabFrm() != pTab )
/*N*/                         pSet = &pTab->GetFmt()->GetAttrSet();
/*N*/                 }
/*N*/ 
/*N*/                 if ( ! pSet )
/*N*/                     pSet = pNxt->GetAttrSet();
/*N*/ 
/*N*/                 ASSERT( pSet, "No AttrSet to check keep attribute" )
/*N*/ 
/*N*/                 if ( pSet->GetPageDesc().GetPageDesc() )
/*N*/ 					bKeep = FALSE;
/*N*/                 else switch ( pSet->GetBreak().GetBreak() )
/*N*/ 				{
/*N*/ 					case SVX_BREAK_COLUMN_BEFORE:
/*N*/ 					case SVX_BREAK_COLUMN_BOTH:
/*N*/ 					case SVX_BREAK_PAGE_BEFORE:
/*N*/ 					case SVX_BREAK_PAGE_BOTH:
/*N*/ 						bKeep = FALSE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bKeep;
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::BwdMoveNecessary()
|*
|*	Ersterstellung		MA 20. Jul. 94
|*	Letzte Aenderung	MA 02. May. 96
|*
|*************************************************************************/


/*N*/ BYTE SwFlowFrm::BwdMoveNecessary( const SwPageFrm *pPage, const SwRect &rRect )
/*N*/ {
/*N*/ 	// Der return-Wert entscheidet mit,
/*N*/ 	// ob auf Zurueckgeflossen werden muss, (3)
/*N*/ 	// ob das gute alte WouldFit gerufen werden kann (0, 1)
/*N*/ 	// oder ob ein Umhaengen und eine Probeformatierung sinnvoll ist (2)
/*N*/ 	// dabei bedeutet Bit 1, dass Objekte an mir selbst verankert sind
/*N*/ 	// und Bit 2, dass ich anderen Objekten ausweichen muss.
/*N*/ 
/*N*/ 	//Wenn ein SurroundObj, dass einen Umfluss wuenscht mit dem Rect ueberlappt
/*N*/ 	//ist der Fluss notwendig (weil die Verhaeltnisse nicht geschaetzt werden
/*N*/ 	//koennen), es kann allerdings ggf. eine TestFormatierung stattfinden.
/*N*/ 	//Wenn das SurroundObj ein Fly ist und ich selbst ein Lower bin oder der Fly
/*N*/ 	//Lower von mir ist, so spielt er keine Rolle.
/*N*/ 	//Wenn das SurroundObj in einem zeichengebunden Fly verankert ist, und ich
/*N*/ 	//selbst nicht Lower dieses Zeichengebundenen Flys bin, so spielt der Fly
/*N*/ 	//keine Rolle.
/*N*/ 	//#32639# Wenn das Objekt bei mir verankert ist kann ich es
/*N*/ 	//vernachlaessigen, weil es hoechstwahrscheinlich (!?) mitfliesst,
/*N*/ 	//eine TestFormatierung ist dann allerdings nicht erlaubt!
/*N*/ 	BYTE nRet = 0;
/*N*/ 	SwFlowFrm *pTmp = this;
/*N*/ 	do
/*N*/ 	{   // Wenn an uns oder einem Follow Objekte haengen, so
/*N*/ 		// kann keine ProbeFormatierung stattfinden, da absatzgebundene
/*N*/ 		// nicht richtig beruecksichtigt wuerden und zeichengebundene sollten
/*N*/ 		// gar nicht zur Probe formatiert werden.
/*N*/ 		if( pTmp->GetFrm()->GetDrawObjs() )
/*N*/ 			nRet = 1;
/*N*/ 		pTmp = pTmp->GetFollow();
/*N*/ 	} while ( !nRet && pTmp );
/*N*/ 	if ( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/ 		const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/ 		ULONG nIndex = ULONG_MAX;
/*N*/ 		for ( USHORT i = 0; nRet < 3 && i < rObjs.Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pObj = rObjs[i];
/*N*/ 			SdrObjUserCall *pUserCall;
/*N*/ 			const SwFrmFmt *pFmt = pObj->IsWriterFlyFrame() ?
/*N*/ 				((SwVirtFlyDrawObj*)pObj)->GetFmt() :
/*N*/ 				((SwContact*)(pUserCall = GetUserCall(pObj)))->GetFmt();
/*N*/ 			const SwRect aRect( pObj->GetBoundRect() );
/*N*/ 			if ( aRect.IsOver( rRect ) &&
/*N*/ 				 pFmt->GetSurround().GetSurround() != SURROUND_THROUGHT )
/*N*/ 			{
/*N*/ 				if( rThis.IsLayoutFrm() && //Fly Lower von This?
/*N*/ 					Is_Lower_Of( &rThis, pObj ) )
/*N*/ 					continue;
/*N*/ 				const SwFrm* pAnchor;
/*N*/ 				if( pObj->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 					if ( pFly->IsAnLower( &rThis ) )//This Lower vom Fly?
/*N*/ 						continue;
/*N*/ 					pAnchor = pFly->GetAnchor();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					pAnchor = ((SwDrawContact*)pUserCall)->GetAnchor();
/*N*/ 
/*N*/ 				if ( pAnchor == &rThis )
/*N*/ 				{
/*N*/ 					nRet |= 1;
/*N*/ 					continue;
/*N*/ 				}
/*N*/ 
/*N*/ 				//Nicht wenn das Objekt im Textfluss hinter mir verankert ist,
/*N*/ 				//denn dann weiche ich ihm nicht aus.
/*N*/ 				if ( ::binfilter::IsFrmInSameKontext( pAnchor, &rThis ) )
/*N*/ 				{
/*N*/ 					if ( pFmt->GetAnchor().GetAnchorId() == FLY_AT_CNTNT )
/*N*/ 					{
/*N*/ 						// Den Index des anderen erhalten wir immer ueber das Ankerattr.
/*N*/ 						ULONG nTmpIndex = pFmt->GetAnchor().GetCntntAnchor()->nNode.GetIndex();
/*N*/ 						// Jetzt wird noch ueberprueft, ob der aktuelle Absatz vor dem
/*N*/ 						// Anker des verdraengenden Objekts im Text steht, dann wird
/*N*/ 						// nicht ausgewichen.
/*N*/ 						// Der Index wird moeglichst ueber einen SwFmtAnchor ermittelt,
/*N*/ 						// da sonst recht teuer.
/*N*/ 						if( ULONG_MAX == nIndex )
/*N*/ 						{
/*N*/ 							const SwNode *pNode;
/*N*/ 							if ( rThis.IsCntntFrm() )
/*N*/ 								pNode = ((SwCntntFrm&)rThis).GetNode();
/*N*/ 							else if( rThis.IsSctFrm() )
/*?*/ 								pNode = ((SwSectionFmt*)((SwSectionFrm&)rThis).
/*?*/ 										GetFmt())->GetSectionNode();
/*N*/ 							else
/*N*/ 							{
/*?*/ 								ASSERT( rThis.IsTabFrm(), "new FowFrm?" );
/*?*/ 								pNode = ((SwTabFrm&)rThis).GetTable()->
/*?*/ 									GetTabSortBoxes()[0]->GetSttNd()->FindTableNode();
/*N*/ 							}
/*N*/ 							nIndex = pNode->GetIndex();
/*N*/ 						}
/*N*/ 						if( nIndex < nTmpIndex )
/*?*/ 							continue;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*?*/ 					continue;
/*N*/ 
/*N*/ 				nRet |= 2;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::CutTree(), PasteTree(), MoveSubTree()
|*
|*	Beschreibung		Eine Spezialisierte Form des Cut() und Paste(), die
|*		eine ganze Kette umhaengt (naehmlich this und folgende). Dabei werden
|* 		nur minimale Operationen und Benachrichtigungen ausgefuehrt.
|*	Ersterstellung		MA 18. Mar. 93
|*	Letzte Aenderung	MA 18. May. 95
|*
|*************************************************************************/


/*N*/ SwLayoutFrm *SwFlowFrm::CutTree( SwFrm *pStart )
/*N*/ {
/*N*/ 	//Der Start und alle Nachbarn werden ausgeschnitten, sie werden aneinander-
/*N*/ 	//gereiht und ein Henkel auf den ersten wird zurueckgeliefert.
/*N*/ 	//Zurueckbleibende werden geeignet invalidiert.
/*N*/ 
/*N*/ 	SwLayoutFrm *pLay = pStart->GetUpper();
/*N*/ 	if ( pLay->IsInFtn() )
/*?*/ 		pLay = pLay->FindFtnFrm();
/*N*/ 	if( pLay )
/*N*/ 	{
/*N*/ 		SwFrm* pTmp = pStart->GetIndPrev();
/*N*/ 		if( pTmp )
/*N*/ 			pTmp->Prepare( PREP_QUOVADIS );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Nur fix auschneiden und zwar so, dass klare Verhaeltnisse bei den
/*N*/ 	//Verlassenen herrschen. Die Pointer der ausgeschnittenen Kette zeigen
/*N*/ 	//noch wer weiss wo hin.
/*N*/ 	if ( pStart == pStart->GetUpper()->Lower() )
/*N*/ 		pStart->GetUpper()->pLower = 0;
/*N*/ 	if ( pStart->GetPrev() )
/*N*/ 	{
/*N*/ 		pStart->GetPrev()->pNext = 0;
/*N*/ 		pStart->pPrev = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pLay->IsFtnFrm() )
/*N*/     {   if ( !pLay->Lower() && !pLay->IsColLocked() &&
/*?*/              !((SwFtnFrm*)pLay)->IsBackMoveLocked() )
/*?*/ 		{	pLay->Cut();
/*?*/ 			delete pLay;
/*?*/ 		}
/*?*/ 		else
/*?*/         {   BOOL bUnlock = !((SwFtnFrm*)pLay)->IsBackMoveLocked();
/*?*/             ((SwFtnFrm*)pLay)->LockBackMove();
/*?*/ 			pLay->InvalidateSize();
/*?*/ 			pLay->Calc();
/*?*/ 			SwCntntFrm *pCnt = pLay->ContainsCntnt();
/*?*/ 			while ( pCnt && pLay->IsAnLower( pCnt ) )
/*?*/ 			{
/*?*/ 				//Kann sein, dass der CntFrm gelockt ist, wir wollen hier nicht
/*?*/ 				//in eine endlose Seitenwanderung hineinlaufen und rufen das
/*?*/ 				//Calc garnicht erst!
/*?*/ 				ASSERT( pCnt->IsTxtFrm(), "Die Graphic ist gelandet." );
/*?*/ 				if ( ((SwTxtFrm*)pCnt)->IsLocked() ||
/*?*/ 					 ((SwTxtFrm*)pCnt)->GetFollow() == pStart )
/*?*/ 					break;
/*?*/ 				pCnt->Calc();
/*?*/ 				pCnt = pCnt->GetNextCntntFrm();
/*?*/ 			}
/*?*/             if( bUnlock )
/*?*/                 ((SwFtnFrm*)pLay)->UnlockBackMove();
/*?*/ 		}
/*?*/ 		pLay = 0;
/*N*/ 	}
/*N*/ 	return pLay;
/*N*/ }



/*N*/ BOOL SwFlowFrm::PasteTree( SwFrm *pStart, SwLayoutFrm *pParent, SwFrm *pSibling,
/*N*/ 						   SwFrm *pOldParent )
/*N*/ {
/*N*/ 	//returnt TRUE wenn in der Kette ein LayoutFrm steht.
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 
/*N*/ 	//Die mit pStart beginnende Kette wird vor den Sibling unter den Parent
/*N*/ 	//gehaengt. Fuer geeignete Invalidierung wird ebenfalls gesorgt.
/*N*/ 
/*N*/ 	//Ich bekomme eine fertige Kette. Der Anfang der Kette muss verpointert
/*N*/ 	//werden, dann alle Upper fuer die Kette und schliesslich dass Ende.
/*N*/ 	//Unterwegs werden alle geeignet invalidiert.
/*N*/ 	if ( pSibling )
/*N*/ 	{
/*N*/ 		if ( 0 != (pStart->pPrev = pSibling->GetPrev()) )
/*N*/ 			pStart->GetPrev()->pNext = pStart;
/*N*/ 		else
/*N*/ 			pParent->pLower = pStart;
/*N*/ 		pSibling->_InvalidatePos();
/*N*/ 		pSibling->_InvalidatePrt();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( 0 == (pStart->pPrev = pParent->Lower()) )
/*N*/ 			pParent->pLower = pStart;
/*N*/ 		else
/*N*/ 			pParent->Lower()->pNext = pStart;
/*N*/ 	}
/*N*/ 	SwFrm *pFloat = pStart;
/*N*/ 	SwFrm *pLst;
/*N*/     SWRECTFN( pParent )
/*N*/ 	SwTwips nGrowVal = 0;
/*N*/ 	do
/*N*/ 	{	pFloat->pUpper = pParent;
/*N*/ 		pFloat->_InvalidateAll();
/*N*/         pFloat->CheckDirChange();
/*N*/ 
/*N*/ 		//Ich bin Freund des TxtFrm und darf deshalb so einiges. Das mit
/*N*/ 		//dem CacheIdx scheint etwas riskant!
/*N*/ 		if ( pFloat->IsTxtFrm() )
/*N*/ 		{
/*N*/ 			if ( ((SwTxtFrm*)pFloat)->GetCacheIdx() != USHRT_MAX )
/*N*/ 				((SwTxtFrm*)pFloat)->Init();	//Ich bin sein Freund.
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bRet = TRUE;
/*N*/ 
/*N*/         nGrowVal += (pFloat->Frm().*fnRect->fnGetHeight)();
/*N*/ 		if ( pFloat->GetNext() )
/*N*/ 			pFloat = pFloat->GetNext();
/*N*/ 		else
/*N*/ 		{	pLst = pFloat;
/*N*/ 			pFloat = 0;
/*N*/ 		}
/*N*/ 	} while ( pFloat );
/*N*/ 
/*N*/ 	if ( pSibling )
/*N*/ 	{
/*N*/ 		pLst->pNext = pSibling;
/*N*/ 		pSibling->pPrev = pLst;
/*N*/ 		if( pSibling->IsInFtn() )
/*N*/ 		{
/*N*/ 			if( pSibling->IsSctFrm() )
/*N*/ 				pSibling = ((SwSectionFrm*)pSibling)->ContainsAny();
/*N*/ 			if( pSibling )
/*N*/ 				pSibling->Prepare( PREP_ERGOSUM );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( nGrowVal )
/*N*/ 	{
/*N*/ 		if ( pOldParent && pOldParent->IsBodyFrm() ) //Fuer variable Seitenhoehe beim Browsen
/*N*/             pOldParent->Shrink( nGrowVal PHEIGHT );
/*N*/         pParent->Grow( nGrowVal PHEIGHT );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pParent->IsFtnFrm() )
/*N*/ 		((SwFtnFrm*)pParent)->InvalidateNxtFtnCnts( pParent->FindPageFrm() );
/*N*/ 	return bRet;
/*N*/ }



/*N*/ void SwFlowFrm::MoveSubTree( SwLayoutFrm* pParent, SwFrm* pSibling )
/*N*/ {
/*N*/ 	ASSERT( pParent, "Kein Parent uebergeben." );
/*N*/ 	ASSERT( rThis.GetUpper(), "Wo kommen wir denn her?" );
/*N*/ 
/*N*/ 	//Sparsamer benachrichtigen wenn eine Action laeuft.
/*N*/ 	ViewShell *pSh = rThis.GetShell();
/*N*/ 	const SwViewImp *pImp = pSh ? pSh->Imp() : 0;
/*N*/ 	const BOOL bComplete = pImp && pImp->IsAction() && pImp->GetLayAction().IsComplete();
/*N*/ 
/*N*/ 	if ( !bComplete )
/*N*/ 	{
/*N*/ 		SwFrm *pPre = rThis.GetIndPrev();
/*N*/ 		if ( pPre )
/*N*/ 		{
/*N*/ 			pPre->SetRetouche();
/*N*/ 			pPre->InvalidatePage();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	rThis.GetUpper()->SetCompletePaint();
/*N*/ 			rThis.GetUpper()->InvalidatePage();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwPageFrm *pOldPage = rThis.FindPageFrm();
/*N*/ 
/*N*/ 	SwLayoutFrm *pOldParent = CutTree( &rThis );
/*N*/ 	const BOOL bInvaLay = PasteTree( &rThis, pParent, pSibling, pOldParent );
/*N*/ 
/*N*/ 	// Wenn durch das Cut&Paste ein leerer SectionFrm entstanden ist, sollte
/*N*/ 	// dieser automatisch verschwinden.
/*N*/ 	SwSectionFrm *pSct;
/*N*/ 	if ( pOldParent && !pOldParent->Lower() &&
/*N*/ 		 (pOldParent->IsInSct() &&
/*N*/ 		  !(pSct = pOldParent->FindSctFrm())->ContainsCntnt() ) )
/*N*/ 			pSct->DelEmpty( FALSE );
/*N*/ 	// In einem spaltigen Bereich rufen wir lieber kein Calc "von unten"
/*N*/ 	if( !rThis.IsInSct() )
/*N*/ 		rThis.GetUpper()->Calc();
/*N*/ 	else if( rThis.GetUpper()->IsSctFrm() )
/*N*/ 	{
/*N*/ 		SwSectionFrm* pSct = (SwSectionFrm*)rThis.GetUpper();
/*N*/ 		BOOL bOld = pSct->IsCntntLocked();
/*N*/ 		pSct->SetCntntLock( TRUE );
/*N*/ 		pSct->Calc();
/*N*/ 		if( !bOld )
/*N*/ 			pSct->SetCntntLock( FALSE );
/*N*/ 	}
/*N*/ 	SwPageFrm *pPage = rThis.FindPageFrm();
/*N*/ 
/*N*/ 	if ( pOldPage != pPage )
/*N*/ 	{
/*N*/ 		rThis.InvalidatePage( pPage );
/*N*/ 		if ( rThis.IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			SwCntntFrm *pCnt = ((SwLayoutFrm*)&rThis)->ContainsCntnt();
/*N*/ 			if ( pCnt )
/*N*/ 				pCnt->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		else if ( pSh && pSh->GetDoc()->GetLineNumberInfo().IsRestartEachPage()
/*N*/ 				  && pPage->FindFirstBodyCntnt() == &rThis )
/*N*/ 		{
/*?*/ 			rThis._InvalidateLineNum();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( bInvaLay || (pSibling && pSibling->IsLayoutFrm()) )
/*N*/ 		rThis.GetUpper()->InvalidatePage( pPage );
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::IsAnFollow()
|*
|*	Ersterstellung		MA 26. Apr. 95
|*	Letzte Aenderung	MA 26. Apr. 95
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::IsAnFollow( const SwFlowFrm *pAssumed ) const
/*N*/ {
/*N*/ 	const SwFlowFrm *pFoll = this;
/*N*/ 	do
/*N*/ 	{	if ( pAssumed == pFoll )
/*N*/ 			return TRUE;
/*N*/ 		pFoll = pFoll->GetFollow();
/*N*/ 	} while ( pFoll );
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::FindMaster()
|*
|*	Ersterstellung		MA 26. Apr. 95
|*	Letzte Aenderung	MA 26. Apr. 95
|*
|*************************************************************************/


/*M*/ SwFlowFrm *SwFlowFrm::FindMaster()
/*M*/ {
/*M*/ 	ASSERT( IsFollow(), "FindMaster und kein Follow." );
/*M*/ 
/*M*/     SwCntntFrm *pCnt;
/*M*/ 	BOOL bCntnt;
/*M*/ 	if ( rThis.IsCntntFrm() )
/*M*/ 	{
/*M*/ 		pCnt = (SwCntntFrm*)&rThis;
/*M*/         pCnt = pCnt->GetPrevCntntFrm();
/*M*/ 
/*M*/ 		bCntnt = TRUE;
/*M*/ 	}
/*M*/ 	else if( rThis.IsTabFrm() )
/*M*/     {
/*M*/         pCnt = rThis.GetPrevCntntFrm();
/*M*/ 
/*M*/ #ifdef DBG_UTIL
/*M*/         SwCntntFrm* pTmpCnt = ((SwLayoutFrm&)rThis).ContainsCntnt();
/*M*/         ASSERT( ! pTmpCnt || pTmpCnt->GetPrevCntntFrm() == pCnt,
/*M*/                 "Two different results for the master of a table?" )
/*M*/ #endif
/*M*/ 
/*M*/ 		bCntnt = FALSE;
/*M*/ 	}
/*M*/ 	else
/*M*/ 	{
/*M*/ 		ASSERT( rThis.IsSctFrm(), "FindMaster: Funny FrameTyp" );
/*M*/ 		return ((SwSectionFrm&)rThis).FindSectionMaster();
/*M*/ 	}
/*M*/ 
/*M*/ 	while ( pCnt )
/*M*/ 	{
/*M*/ 		if ( bCntnt )
/*M*/ 		{
/*M*/ 			if ( pCnt->HasFollow() && pCnt->GetFollow() == this )
/*M*/ 				return pCnt;
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{	SwTabFrm  *pTab = pCnt->FindTabFrm();
/*M*/ 			if ( pTab && pTab->GetFollow() == this )
/*M*/ 				return pTab;
/*M*/ 		}
/*M*/ 		pCnt = pCnt->GetPrevCntntFrm();
/*M*/ 	}
/*M*/ 	ASSERT( FALSE, "Follow ist lost in Space." );
/*M*/ 	return 0;
/*M*/ }

/*************************************************************************
|*
|*	SwFrm::GetLeaf()
|*
|*	Beschreibung		Liefert das naechste/vorhergehende LayoutBlatt,
|* 		das _nicht_ unterhalb von this liegt (oder gar this selbst ist).
|* 		Ausserdem muss dieses LayoutBlatt im gleichen Textfluss wie
|* 		pAnch Ausgangsfrm liegen (Body, Ftn)
|*	Ersterstellung		MA 25. Nov. 92
|*	Letzte Aenderung	MA 25. Apr. 95
|*
|*************************************************************************/


/*N*/ const SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, BOOL bFwd,
/*N*/ 								   const SwFrm *pAnch ) const
/*N*/ {
/*N*/ 	//Ohne Fluss kein genuss...
/*N*/ 	if ( IsInTab() || !(IsInDocBody() || IsInFtn() || IsInFly()) )
/*?*/ 		return 0;
/*N*/ 
/*N*/ 	const SwFrm *pLeaf = this;
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	do
/*N*/ 	{	pLeaf = ((SwFrm*)pLeaf)->GetLeaf( eMakePage, bFwd );
/*N*/ 
/*N*/ 		if ( pLeaf &&
/*N*/ 			(!IsLayoutFrm() || !((SwLayoutFrm*)this)->IsAnLower( pLeaf )))
/*N*/ 		{
/*N*/ 			if ( pAnch->IsInDocBody() == pLeaf->IsInDocBody() &&
/*N*/ 				 pAnch->IsInFtn()	  == pLeaf->IsInFtn() )
/*N*/ 			{
/*N*/ 				bFound = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	} while ( !bFound && pLeaf );
/*N*/ 
/*N*/ 	return (const SwLayoutFrm*)pLeaf;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::GetLeaf()
|*
|*	Beschreibung		Ruft Get[Next|Prev]Leaf
|*
|*	Ersterstellung		MA 20. Mar. 93
|*	Letzte Aenderung	MA 25. Apr. 95
|*
|*************************************************************************/


/*N*/ SwLayoutFrm *SwFrm::GetLeaf( MakePageType eMakePage, BOOL bFwd )
/*N*/ {
/*N*/ 	if ( IsInFtn() )
/*?*/ 	{	DBG_BF_ASSERT(0, "STRIP");} //STRIP001 return bFwd ? GetNextFtnLeaf( eMakePage ) : GetPrevFtnLeaf( eMakePage );
/*N*/ 	if ( IsInSct() )
/*N*/ 		return bFwd ? GetNextSctLeaf( eMakePage ) : GetPrevSctLeaf( eMakePage );
/*N*/ 	return bFwd ? GetNextLeaf( eMakePage ) : GetPrevLeaf( eMakePage );
/*N*/ }



/*N*/ BOOL SwFrm::WrongPageDesc( SwPageFrm* pNew )
/*N*/ {
/*N*/ 	//Jetzt wirds leider etwas kompliziert:
/*N*/ 	//Ich bringe ich evtl. selbst
/*N*/ 	//einen Pagedesc mit; der der Folgeseite muss dann damit
/*N*/ 	//uebereinstimmen.
/*N*/ 	//Anderfalls muss ich mir etwas genauer ansehen wo der
/*N*/ 	//Folgepagedesc herkam.
/*N*/ 	//Wenn die Folgeseite selbst schon sagt, dass ihr
/*N*/ 	//Pagedesc nicht stimmt so kann ich das Teil bedenkenlos
/*N*/ 	//auswechseln.
/*N*/ 	//Wenn die Seite meint, dass ihr Pagedesc stimmt, so heisst
/*N*/ 	//das leider noch immer nicht, dass ich damit etwas anfangen
/*N*/ 	//kann: Wenn der erste BodyCntnt einen PageDesc oder einen
/*N*/ 	//PageBreak wuenscht, so muss ich ebenfalls eine neue
/*N*/ 	//Seite einfuegen; es sein denn die gewuenschte Seite ist
/*N*/ 	//die richtige.
/*N*/ 	//Wenn ich einen neue Seite eingefuegt habe, so fangen die
/*N*/ 	//Probleme leider erst an, denn wahrscheinlich wird die dann
/*N*/ 	//folgende Seite verkehrt gewesen und ausgewechselt worden
/*N*/ 	//sein. Das hat zur Folge, dass ich zwar eine neue (und
/*N*/ 	//jetzt richtige) Seite habe, die Bedingungen zum auswechseln
/*N*/ 	//aber leider noch immer stimmen.
/*N*/ 	//Ausweg: Vorlaeufiger Versuch, nur einmal eine neue Seite
/*N*/ 	//einsetzen (Leerseiten werden noetigenfalls bereits von
/*N*/ 	//InsertPage() eingefuegt.
/*N*/ 	const SwFmtPageDesc &rFmtDesc = GetAttrSet()->GetPageDesc();
/*N*/ 
/*N*/ 	//Mein Pagedesc zaehlt nicht, wenn ich ein Follow bin!
/*N*/ 	SwPageDesc *pDesc = 0;
/*N*/ 	USHORT nTmp = 0;
/*N*/ 	SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( this );
/*N*/ 	if ( !pFlow || !pFlow->IsFollow() )
/*N*/ 	{
/*N*/ 		pDesc = (SwPageDesc*)rFmtDesc.GetPageDesc();
/*N*/ 		if( pDesc )
/*N*/ 		{
/*N*/ 			if( !pDesc->GetRightFmt() )
/*?*/ 				nTmp = 2;
/*N*/ 			else if( !pDesc->GetLeftFmt() )
/*N*/ 				nTmp = 1;
/*N*/ 			else if( rFmtDesc.GetNumOffset() )
/*N*/ 				nTmp = rFmtDesc.GetNumOffset();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Bringt der Cntnt einen Pagedesc mit oder muss zaehlt die
/*N*/ 	//virtuelle Seitennummer des neuen Layoutleafs?
/*N*/ 	// Bei Follows zaehlt der PageDesc nicht
/*N*/ 	const BOOL bOdd = nTmp ? ( nTmp % 2 ? TRUE : FALSE )
/*N*/ 						   : pNew->OnRightPage();
/*N*/ 	if ( !pDesc )
/*N*/ 		pDesc = pNew->FindPageDesc();
/*N*/ 	const SwFlowFrm *pNewFlow = pNew->FindFirstBodyCntnt();
/*N*/ 	// Haben wir uns selbst gefunden?
/*N*/ 	if( pNewFlow == pFlow )
/*?*/ 		pNewFlow = NULL;
/*N*/ 	if ( pNewFlow && pNewFlow->GetFrm()->IsInTab() )
/*N*/ 		pNewFlow = pNewFlow->GetFrm()->FindTabFrm();
/*N*/ 	const SwPageDesc *pNewDesc= ( pNewFlow && !pNewFlow->IsFollow() )
/*N*/ 			? pNewFlow->GetFrm()->GetAttrSet()->GetPageDesc().GetPageDesc():0;
/*N*/ 
/*N*/ 	return ( pNew->GetPageDesc() != pDesc ||   //  own desc ?
/*N*/ 		pNew->GetFmt() != (bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) ||
/*N*/ 		( pNewDesc && pNewDesc == pDesc ) );
/*N*/ }


/*************************************************************************
|*
|*	SwFrm::GetNextLeaf()
|*
|*	Beschreibung		Liefert das naechste LayoutBlatt in den das
|* 		Frame gemoved werden kann.
|*
|*	Ersterstellung		MA 16. Nov. 92
|*	Letzte Aenderung	MA 05. Dec. 96
|*
|*************************************************************************/

/*N*/ SwLayoutFrm *SwFrm::GetNextLeaf( MakePageType eMakePage )
/*N*/ {
/*N*/ 	ASSERT( !IsInFtn(), "GetNextLeaf(), don't call me for Ftn." );
/*N*/ 	ASSERT( !IsInSct(), "GetNextLeaf(), don't call me for Sections." );
/*N*/ 
/*N*/ 	const BOOL bBody = IsInDocBody();		//Wenn ich aus dem DocBody komme
/*N*/ 											//Will ich auch im Body landen.
/*N*/ 
/*N*/ 	// Bei Flys macht es keinen Sinn, Seiten einzufuegen, wir wollen lediglich
/*N*/ 	 // die Verkettung absuchen.
/*N*/ 	if( IsInFly() )
/*N*/ 		eMakePage = MAKEPAGE_NONE;
/*N*/ 	//Bei Tabellen gleich den grossen Sprung wagen, ein einfaches GetNext...
/*N*/ 	//wuerde die erste Zellen und in der Folge alle weiteren Zellen nacheinander
/*N*/ 	//abklappern....
/*N*/ 	SwLayoutFrm *pLayLeaf;
/*N*/ 	if ( IsTabFrm() )
/*N*/ 		pLayLeaf = ((SwTabFrm*)this)->FindLastCntnt()->GetUpper();
/*N*/ 	else
/*N*/ 		pLayLeaf = GetNextLayoutLeaf();
/*N*/ 
/*N*/ 	SwLayoutFrm *pOldLayLeaf = 0;			//Damit bei neu erzeugten Seiten
/*N*/ 											//nicht wieder vom Anfang gesucht
/*N*/ 											//wird.
/*N*/ 	BOOL bNewPg = FALSE;	//nur einmal eine neue Seite einfuegen.
/*N*/ 
/*N*/ 	while ( TRUE )
/*N*/ 	{
/*N*/ 		if ( pLayLeaf )
/*N*/ 		{
/*N*/ 			//Es gibt noch einen weiteren LayoutFrm, mal sehen,
/*N*/ 			//ob er bereit ist mich aufzunehmen.
/*N*/ 			//Dazu braucht er nur von der gleichen Art wie mein Ausgangspunkt
/*N*/ 			//sein (DocBody bzw. Footnote.)
/*N*/ 			if ( pLayLeaf->FindPageFrm()->IsFtnPage() )
/*N*/ 			{	//Wenn ich bei den Endnotenseiten angelangt bin hat sichs.
/*?*/ 				pLayLeaf = 0;
/*?*/ 				continue;
/*N*/ 			}
/*N*/ 			if ( (bBody && !pLayLeaf->IsInDocBody()) || pLayLeaf->IsInTab()
/*N*/ 				 || pLayLeaf->IsInSct() )
/*N*/ 			{
/*N*/ 				//Er will mich nicht; neuer Versuch, neues Glueck
/*N*/ 				pOldLayLeaf = pLayLeaf;
/*N*/ 				pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			//Er will mich, also ist er der gesuchte und ich bin fertig.
/*N*/ 			//Bei einem Seitenwechsel kann es allerdings noch sein, dass
/*N*/ 			//Der Seitentyp nicht der gewuenschte ist, in diesem Fall muessen
/*N*/ 			//wir eine Seite des richtigen Typs einfuegen.
/*N*/ 
/*N*/ 			if( !IsFlowFrm() && ( eMakePage == MAKEPAGE_NONE ||
/*N*/ 				eMakePage==MAKEPAGE_APPEND || eMakePage==MAKEPAGE_NOSECTION ) )
/*N*/ 				return pLayLeaf;
/*N*/ 
/*N*/ 			SwPageFrm *pNew = pLayLeaf->FindPageFrm();
/*N*/ 			if ( pNew != FindPageFrm() && !bNewPg )
/*N*/ 			{
/*N*/ 				if( WrongPageDesc( pNew ) )
/*N*/ 				{
/*N*/ 					SwFtnContFrm *pCont = pNew->FindFtnCont();
/*N*/ 					if( pCont )
/*N*/ 					{
/*?*/ 						// Falls die Referenz der ersten Fussnote dieser Seite
/*?*/ 						// vor der Seite liegt, fuegen wir lieber keine neue Seite
/*?*/ 						// ein (Bug #55620#)
/*?*/ 						SwFtnFrm *pFtn = (SwFtnFrm*)pCont->Lower();
/*?*/ 						if( pFtn && pFtn->GetRef() )
/*?*/ 						{
/*?*/ 							const USHORT nRefNum = pNew->GetPhyPageNum();
/*?*/ 							if( pFtn->GetRef()->GetPhyPageNum() < nRefNum )
/*?*/ 								break;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					//Erwischt, die folgende Seite ist verkehrt, also
/*N*/ 					//muss eine neue eingefuegt werden.
/*N*/ 					if ( eMakePage == MAKEPAGE_INSERT )
/*N*/ 					{
/*N*/ 						bNewPg = TRUE;
/*N*/ 
/*N*/ 						SwPageFrm *pPg = pOldLayLeaf ?
/*N*/ 									pOldLayLeaf->FindPageFrm() : 0;
/*N*/ 						if ( pPg && pPg->IsEmptyPage() )
/*N*/ 							//Nicht hinter, sondern vor der EmptyPage einfuegen.
/*?*/ 							pPg = (SwPageFrm*)pPg->GetPrev();
/*N*/ 
/*N*/ 						if ( !pPg || pPg == pNew )
/*N*/ 							pPg = FindPageFrm();
/*N*/ 
/*N*/ 						InsertPage( pPg, FALSE );
/*N*/ 						pLayLeaf = GetNextLayoutLeaf();
/*N*/ 						pOldLayLeaf = 0;
/*N*/ 						continue;
/*N*/ 					}
/*N*/ 					else
/*?*/ 						pLayLeaf = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//Es gibt keinen passenden weiteren LayoutFrm, also muss eine
/*N*/ 			//neue Seite her.
/*N*/ 			if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
/*N*/ 			{
/*N*/ 				InsertPage(
/*N*/ 					pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
/*N*/ 					FALSE );
/*N*/ 
/*N*/ 				//und nochmal das ganze
/*N*/ 				pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
/*N*/ 			}
/*N*/ 			else
/*?*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pLayLeaf;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::GetPrevLeaf()
|*
|*	Beschreibung		Liefert das vorhergehende LayoutBlatt in das der
|* 		Frame gemoved werden kann.
|*	Ersterstellung		MA 16. Nov. 92
|*	Letzte Aenderung	MA 25. Apr. 95
|*
|*************************************************************************/


/*N*/ SwLayoutFrm *SwFrm::GetPrevLeaf( MakePageType eMakeFtn )
/*N*/ {
/*N*/ 	ASSERT( !IsInFtn(), "GetPrevLeaf(), don't call me for Ftn." );
/*N*/ 
/*N*/ 	const BOOL bBody = IsInDocBody();		//Wenn ich aus dem DocBody komme
/*N*/ 											//will ich auch im Body landen.
/*N*/ 	const BOOL bFly  = IsInFly();
/*N*/ 
/*N*/ 	SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
/*N*/ 	SwLayoutFrm *pPrevLeaf = 0;
/*N*/ 
/*N*/ 	while ( pLayLeaf )
/*N*/ 	{
/*N*/ 		if ( pLayLeaf->IsInTab() || 	//In Tabellen geht's niemals hinein.
/*N*/ 			 pLayLeaf->IsInSct() )		//In Bereiche natuerlich auch nicht!
/*N*/ 			pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
/*N*/ 		else if ( bBody && pLayLeaf->IsInDocBody() )
/*N*/ 		{
/*N*/ 			if ( pLayLeaf->Lower() )
/*N*/ 				break;
/*N*/ 			pPrevLeaf = pLayLeaf;
/*N*/ 			pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
/*N*/ 			if ( pLayLeaf )
/*N*/ 				SwFlowFrm::SetMoveBwdJump( TRUE );
/*N*/ 		}
/*N*/ 		else if ( bFly )
/*N*/ 			break;	//Cntnts in Flys sollte jedes Layout-Blatt recht sein.
/*N*/ 		else
/*N*/ 			pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
/*N*/ 	}
/*N*/ 	return pLayLeaf ? pLayLeaf : pPrevLeaf;
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::IsPrevObjMove()
|*
|*	Ersterstellung		MA 20. Feb. 96
|*	Letzte Aenderung	MA 22. Feb. 96
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::IsPrevObjMove() const
/*N*/ {
/*N*/ 	//TRUE der FlowFrm soll auf einen Rahmen des Vorgaengers Ruecksicht nehmen
/*N*/ 	//     und fuer diesen ggf. Umbrechen.
/*N*/ 
/*N*/ 	//!!!!!!!!!!!Hack!!!!!!!!!!!
/*N*/ 	if ( rThis.GetUpper()->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SwFrm *pPre = rThis.FindPrev();
/*N*/ 
/*N*/ 	if ( pPre && pPre->GetDrawObjs() )
/*N*/ 	{
/*N*/ 		ASSERT( SwFlowFrm::CastFlowFrm( pPre ),	"new flowfrm?" );
/*N*/ 		if(	SwFlowFrm::CastFlowFrm( pPre )->IsAnFollow( this ) )
/*?*/ 			return FALSE;
/*N*/ 		SwFrm* pPreUp = pPre->GetUpper();
/*N*/ 		// Wenn der Upper ein SectionFrm oder die Spalte eines SectionFrms ist,
/*N*/ 		// duerfen wir aus diesem durchaus heraushaengen,
/*N*/ 		// es muss stattdessen der Upper des SectionFrms beruecksichtigt werden.
/*N*/ 		if( pPreUp->IsInSct() )
/*N*/ 		{
/*N*/ 			if( pPreUp->IsSctFrm() )
/*N*/ 				pPreUp = pPreUp->GetUpper();
/*?*/ 			else if( pPreUp->IsColBodyFrm() &&
/*?*/ 					 pPreUp->GetUpper()->GetUpper()->IsSctFrm() )
/*?*/                 pPreUp = pPreUp->GetUpper()->GetUpper()->GetUpper();
/*N*/ 		}
/*N*/ 		const long nBottom = pPreUp->Frm().Bottom();
/*N*/ 		const long nRight  = pPreUp->Frm().Right();
/*N*/ 		const FASTBOOL bCol = pPreUp->IsColBodyFrm();//ColFrms jetzt mit BodyFrm
/*N*/ 		for ( USHORT i = 0; i < pPre->GetDrawObjs()->Count(); ++i )
/*N*/ 		{
/*N*/ 			const SdrObject *pObj = (*pPre->GetDrawObjs())[i];
/*N*/ 			if ( pObj->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 
/*N*/ 				if ( WEIT_WECH != pFly->Frm().Top() && !pFly->IsFlyInCntFrm() )
/*N*/ 				{
/*N*/ 					if( pObj->GetSnapRect().Top()  > nBottom )
/*N*/ 						return TRUE;
/*N*/ 					if( bCol && pObj->GetSnapRect().Left() > nRight )
/*N*/ 					{
/*N*/ 						SwFmtHoriOrient aHori( pFly->GetFmt()->GetHoriOrient() );
/*N*/ 						if( FRAME == aHori.GetRelationOrient() ||
/*N*/ 							PRTAREA == aHori.GetRelationOrient() ||
/*N*/ 							REL_CHAR == aHori.GetRelationOrient() ||
/*N*/ 							REL_FRM_LEFT == aHori.GetRelationOrient() ||
/*N*/ 							REL_FRM_RIGHT == aHori.GetRelationOrient() )
/*N*/ 						{
/*N*/ 							if( HORI_NONE == aHori.GetHoriOrient() )
/*N*/ 							{
/*N*/ 								SwTwips nAdd = 0;
/*N*/ 								switch ( aHori.GetRelationOrient() )
/*N*/ 								{
/*N*/ 									case PRTAREA:
/*?*/ 										nAdd = pFly->Prt().Left(); break;
/*?*/ 									case REL_FRM_RIGHT:
/*?*/ 										nAdd = pFly->Frm().Width(); break;
/*?*/ 									case REL_CHAR:
/*?*/ 										if( pFly->IsFlyAtCntFrm() )
/*?*/ 											nAdd = ((SwFlyAtCntFrm*)pFly)->GetLastCharX();
/*?*/ 										break;
/*N*/ 								}
/*N*/ 								nAdd += aHori.GetPos();
/*N*/ 								if( nAdd < pPreUp->Frm().Width() &&
/*N*/ 									nAdd + pFly->Frm().Width() > 0 )
/*N*/ 									return TRUE;
/*N*/ 							}
/*N*/ 							else
/*?*/ 								return TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	BOOL SwFlowFrm::IsPageBreak()
|*
|*	Beschreibung		Wenn vor dem Frm ein harter Seitenumbruch steht UND
|*		es einen Vorgaenger auf der gleichen Seite gibt, wird TRUE
|*		zurueckgeliefert (es muss ein PageBreak erzeugt werden) FALSE sonst.
|*		Wenn in bAct TRUE uebergeben wird, gibt die Funktion dann TRUE
|*		zurueck, wenn ein PageBreak besteht.
|* 		Fuer Follows wird der harte Seitenumbruch natuerlich nicht
|* 		ausgewertet.
|*		Der Seitenumbruch steht im eigenen FrmFmt (BEFORE) oder im FrmFmt
|*		des Vorgaengers (AFTER). Wenn es keinen Vorgaenger auf der Seite
|*		gibt ist jede weitere Ueberlegung ueberfluessig.
|* 		Ein Seitenumbruch (oder der Bedarf) liegt auch dann vor, wenn
|* 		im FrmFmt ein PageDesc angegeben wird.
|*		Die Implementierung arbeitet zuaechst nur auf CntntFrms!
|*		-->Fuer LayoutFrms ist die Definition des Vorgaengers unklar.
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 21. Mar. 95
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::IsPageBreak( BOOL bAct ) const
/*N*/ {
/*N*/ 	const SwAttrSet *pSet;
/*N*/ 	if ( !IsFollow() && rThis.IsInDocBody() &&
/*N*/ 		 !(pSet = rThis.GetAttrSet())->GetDoc()->IsBrowseMode() )
/*N*/ 	{
/*N*/ 		//Vorgaenger ermitteln
/*N*/ 		const SwFrm *pPrev = rThis.FindPrev();
/*N*/ 		while ( pPrev && ( !pPrev->IsInDocBody() ||
/*N*/ 				( pPrev->IsTxtFrm() && ((SwTxtFrm*)pPrev)->IsHiddenNow() ) ) )
/*?*/ 			pPrev = pPrev->FindPrev();
/*N*/ 
/*N*/ 		if ( pPrev )
/*N*/ 		{
/*N*/ 			ASSERT( pPrev->IsInDocBody(), "IsPageBreak: Not in DocBody?" );
/*N*/ 			if ( bAct )
/*N*/ 			{	if ( rThis.FindPageFrm() == pPrev->FindPageFrm() )
/*N*/ 					return FALSE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{	if ( rThis.FindPageFrm() != pPrev->FindPageFrm() )
/*N*/ 					return FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			const SvxBreak eBreak = pSet->GetBreak().GetBreak();
/*N*/ 			if ( eBreak == SVX_BREAK_PAGE_BEFORE || eBreak == SVX_BREAK_PAGE_BOTH )
/*N*/ 				return TRUE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
/*N*/ 				if ( ePrB == SVX_BREAK_PAGE_AFTER ||
/*N*/ 					 ePrB == SVX_BREAK_PAGE_BOTH  ||
/*N*/ 					 pSet->GetPageDesc().GetPageDesc() )
/*N*/ 					return TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	BOOL SwFlowFrm::IsColBreak()
|*
|*	Beschreibung		Wenn vor dem Frm ein harter Spaltenumbruch steht UND
|*		es einen Vorgaenger in der gleichen Spalte gibt, wird TRUE
|*		zurueckgeliefert (es muss ein PageBreak erzeugt werden) FALSE sonst.
|*		Wenn in bAct TRUE uebergeben wird, gibt die Funktion dann TRUE
|*		zurueck, wenn ein ColBreak besteht.
|* 		Fuer Follows wird der harte Spaltenumbruch natuerlich nicht
|* 		ausgewertet.
|*		Der Spaltenumbruch steht im eigenen FrmFmt (BEFORE) oder im FrmFmt
|*		des Vorgaengers (AFTER). Wenn es keinen Vorgaenger in der Spalte
|*		gibt ist jede weitere Ueberlegung ueberfluessig.
|*		Die Implementierung arbeitet zuaechst nur auf CntntFrms!
|*		-->Fuer LayoutFrms ist die Definition des Vorgaengers unklar.
|*	Ersterstellung		MA 11. Jun. 93
|*	Letzte Aenderung	MA 21. Mar. 95
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::IsColBreak( BOOL bAct ) const
/*N*/ {
/*N*/ 	if ( !IsFollow() && (rThis.IsMoveable() || bAct) )
/*N*/ 	{
/*N*/ 		const SwFrm *pCol = rThis.FindColFrm();
/*N*/ 		if ( pCol )
/*N*/ 		{
/*N*/ 			//Vorgaenger ermitteln
/*N*/ 			const SwFrm *pPrev = rThis.FindPrev();
/*N*/ 			while( pPrev && ( ( !pPrev->IsInDocBody() && !rThis.IsInFly() ) ||
/*N*/ 				   ( pPrev->IsTxtFrm() && ((SwTxtFrm*)pPrev)->IsHiddenNow() ) ) )
/*?*/ 					pPrev = pPrev->FindPrev();
/*N*/ 
/*N*/ 			if ( pPrev )
/*N*/ 			{
/*N*/ 				if ( bAct )
/*N*/ 				{	if ( pCol == pPrev->FindColFrm() )
/*?*/ 						return FALSE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{	if ( pCol != pPrev->FindColFrm() )
/*N*/ 						return FALSE;
/*N*/ 				}
/*N*/ 
/*N*/ 				const SvxBreak eBreak = rThis.GetAttrSet()->GetBreak().GetBreak();
/*N*/ 				if ( eBreak == SVX_BREAK_COLUMN_BEFORE ||
/*N*/ 					 eBreak == SVX_BREAK_COLUMN_BOTH )
/*N*/ 					return TRUE;
/*N*/ 				else
/*N*/ 				{
/*N*/ 					const SvxBreak &ePrB = pPrev->GetAttrSet()->GetBreak().GetBreak();
/*N*/ 					if ( ePrB == SVX_BREAK_COLUMN_AFTER ||
/*N*/ 						 ePrB == SVX_BREAK_COLUMN_BOTH )
/*?*/ 						return TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ SwTwips SwFlowFrm::CalcUpperSpace( const SwBorderAttrs *pAttrs,
/*N*/ 	const SwFrm* pPr ) const
/*N*/ {
/*N*/ 	const SwFrm *pPre = pPr ? pPr : rThis.GetPrev();
/*N*/ 	BOOL bInFtn = rThis.IsInFtn();
/*N*/ 	do {
/*N*/ 		while( pPre && ( (pPre->IsTxtFrm() && ((SwTxtFrm*)pPre)->IsHiddenNow())
/*N*/ 			   || ( pPre->IsSctFrm() && !((SwSectionFrm*)pPre)->GetSection() ) ) )
/*N*/ 			pPre = pPre->GetPrev();
/*N*/ 		if( !pPre && bInFtn )
/*N*/ 		{
/*N*/ 			bInFtn = FALSE;
/*N*/ 			if( !rThis.IsInSct() || rThis.IsSctFrm() ||
/*N*/ 				!rThis.FindSctFrm()->IsInFtn() )
/*N*/ 				pPre = rThis.FindFtnFrm()->GetPrev();
/*N*/ 			if( pPre )
/*N*/ 			{
/*?*/ 				pPre = ((SwFtnFrm*)pPre)->Lower();
/*?*/ 				if( pPre )
/*?*/ 					while( pPre->GetNext() )
/*?*/ 						pPre = pPre->GetNext();
/*?*/ 				continue;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( pPre && pPre->IsSctFrm() )
/*N*/ 		{
/*N*/ 			SwSectionFrm* pSect = (SwSectionFrm*)pPre;
/*N*/ 			pPre = pSect->FindLastCntnt();
/*N*/ 			// If the last content is in a table _inside_ the section,
/*N*/ 			// take the table herself.
/*N*/ 			if( pPre && pPre->IsInTab() && !pSect->IsInTab() )
/*?*/ 				pPre = pPre->FindTabFrm();
/*N*/ 		}
/*N*/ 		break;
/*?*/ 	} while( pPre );
/*N*/ 	SwBorderAttrAccess *pAccess;
/*N*/ 	SwFrm* pOwn;
/*N*/ 	if( !pAttrs )
/*N*/ 	{
/*N*/ 		if( rThis.IsSctFrm() )
/*N*/ 		{
/*N*/ 			SwSectionFrm* pFoll = &((SwSectionFrm&)rThis);
/*N*/ 			do
/*N*/ 				pOwn = pFoll->ContainsAny();
/*N*/ 			while( !pOwn && 0 != ( pFoll = pFoll->GetFollow() ) );
/*N*/ 			if( !pOwn )
/*?*/ 				return 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pOwn = &rThis;
/*N*/ 		pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), pOwn );
/*N*/ 		pAttrs = pAccess->Get();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pAccess = NULL;
/*N*/ 		pOwn = &rThis;
/*N*/ 	}
/*N*/ 	SwTwips nUpper = 0;
/*N*/ 	if( pPre )
/*N*/ 	{
/*N*/ 		const SvxULSpaceItem &rPrevUL = pPre->GetAttrSet()->GetULSpace();
/*N*/ 		if( rThis.GetAttrSet()->GetDoc()->IsParaSpaceMax() )
/*N*/ 		{
/*?*/ 			nUpper = rPrevUL.GetLower() + pAttrs->GetULSpace().GetUpper();
/*?*/ 			SwTwips nAdd = 0;
/*?*/ 			if ( pOwn->IsTxtFrm() )
/*?*/ 				nAdd = Max( nAdd, long(((SwTxtFrm&)rThis).GetLineSpace()) );
/*?*/ 			if ( pPre->IsTxtFrm() )
/*?*/ 				nAdd = Max( nAdd, long(((SwTxtFrm*)pPre)->GetLineSpace()) );
/*?*/ 			nUpper += nAdd;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nUpper = Max( rPrevUL.GetLower(), pAttrs->GetULSpace().GetUpper() );
/*N*/ 			if ( pOwn->IsTxtFrm() )
/*N*/ 				nUpper = Max( nUpper, long(((SwTxtFrm*)pOwn)->GetLineSpace()) );
/*N*/ 			if ( pPre->IsTxtFrm() )
/*N*/ 				nUpper = Max( nUpper, long(((SwTxtFrm*)pPre)->GetLineSpace()) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( rThis.GetAttrSet()->GetDoc()->IsParaSpaceMaxAtPages() &&
/*N*/ 			 1) //STRIP001 CastFlowFrm( pOwn )->HasParaSpaceAtPages( rThis.IsSctFrm() ) )
/*?*/ 	{	DBG_BF_ASSERT(0, "STRIP");} //STRIP001 nUpper = pAttrs->GetULSpace().GetUpper();
/*N*/ 
/*N*/ 	nUpper += pAttrs->GetTopLine( &rThis );
/*N*/ 
/*N*/     if( rThis.IsInDocBody() && rThis.GetAttrSet()->GetParaGrid().GetValue() )
/*N*/     {
/*N*/         const SwPageFrm* pPg = rThis.FindPageFrm();
/*N*/         GETGRID( pPg )
/*N*/         if( pGrid )
/*N*/         {
/*?*/             const SwFrm* pBody = pPg->FindBodyCont();
/*?*/             if( pBody )
/*?*/             {
/*?*/                 long nSum = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
/*?*/                 SWRECTFN( (&rThis) )
/*?*/                 SwTwips nOrig = (pBody->*fnRect->fnGetPrtTop)();
/*?*/                 SwTwips nTop = (rThis.Frm().*fnRect->fnGetTop)();
/*?*/                 if( bVert )
/*?*/                 {
/*?*/                     nTop -= nUpper;
/*?*/                     SwTwips nY = nOrig - nSum *( ( nOrig - nTop ) / nSum );
/*?*/                     if( nY > nTop )
/*?*/                         nY -= nSum;
/*?*/                     nUpper = nTop + nUpper - nY;
/*?*/                 }
/*?*/                 else
/*?*/                 {
/*?*/                     nTop += nUpper;
/*?*/                     SwTwips nY = nOrig + nSum *( ( nTop - nOrig ) / nSum );
/*?*/                     if( nY < nTop )
/*?*/                         nY += nSum;
/*?*/                     nUpper = nY - rThis.Frm().Top();
/*?*/                 }
/*?*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	delete pAccess;
/*N*/ 	return nUpper;
/*N*/ }

/*************************************************************************
|*
|*	BOOL SwFlowFrm::CheckMoveFwd()
|*
|*	Beschreibung		Moved den Frm vorwaerts wenn es durch die aktuellen
|* 		Bedingungen und Attribute notwendig erscheint.
|*	Ersterstellung		MA 05. Dec. 96
|*	Letzte Aenderung	MA 09. Mar. 98
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::CheckMoveFwd( BOOL &rbMakePage, BOOL bKeep, BOOL bMovedBwd )
/*N*/ {
/*N*/ 	const SwFrm* pNxt = rThis.GetIndNext();
/*N*/ 
/*N*/ 	if ( bKeep && //!bMovedBwd &&
/*N*/ 		 ( !pNxt || ( pNxt->IsTxtFrm() && ((SwTxtFrm*)pNxt)->IsEmptyMaster() ) ) &&
/*N*/ 		 ( 0 != (pNxt = rThis.FindNext()) ) && IsKeepFwdMoveAllowed() )
/*N*/ 	{
/*N*/ 		if( pNxt->IsSctFrm() )
/*N*/ 		{   // Nicht auf leere SectionFrms hereinfallen
/*?*/ 			const SwFrm* pTmp = NULL;
/*?*/ 			while( pNxt && pNxt->IsSctFrm() &&
/*?*/ 				   ( !((SwSectionFrm*)pNxt)->GetSection() ||
/*?*/ 					 !( pTmp = ((SwSectionFrm*)pNxt)->ContainsAny() ) ) )
/*?*/ 			{
/*?*/ 				pNxt = pNxt->FindNext();
/*?*/ 				pTmp = NULL;
/*?*/ 			}
/*?*/ 			if( pTmp )
/*?*/ 				pNxt = pTmp; // the content of the next notempty sectionfrm
/*N*/ 		}
/*N*/ 		if( pNxt && pNxt->GetValidPosFlag() )
/*N*/ 		{
/*N*/ 			BOOL bMove = FALSE;
/*N*/ 			const SwSectionFrm *pSct = rThis.FindSctFrm();
/*N*/ 			if( pSct && !pSct->GetValidSizeFlag() )
/*N*/ 			{
/*?*/ 				const SwSectionFrm* pNxtSct = pNxt->FindSctFrm();
/*?*/ 				if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
/*?*/ 					bMove = TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bMove = TRUE;
/*N*/ 			if( bMove )
/*N*/ 			{
/*N*/ 				//Keep together with the following frame
/*N*/ 				MoveFwd( rbMakePage, FALSE );
/*N*/ 				return TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bMovedFwd = FALSE;
/*N*/ 
/*N*/ 	if ( rThis.GetIndPrev() )
/*N*/ 	{
/*N*/ 		if ( IsPrevObjMove() ) //Auf Objekte des Prev Ruecksicht nehmen?
/*N*/ 		{
/*N*/ 			bMovedFwd = TRUE;
/*N*/ 			if ( !MoveFwd( rbMakePage, FALSE ) )
/*N*/ 				rbMakePage = FALSE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( IsPageBreak( FALSE ) )
/*N*/ 			{
/*N*/ 				while ( MoveFwd( rbMakePage, TRUE ) )
/*N*/ 						/* do nothing */;
/*N*/ 				rbMakePage = FALSE;
/*N*/ 				bMovedFwd = TRUE;
/*N*/ 			}
/*N*/ 			else if ( IsColBreak ( FALSE ) )
/*N*/ 			{
/*N*/ 				const SwPageFrm *pPage = rThis.FindPageFrm();
/*N*/ 				SwFrm *pCol = rThis.FindColFrm();
/*N*/ 				do
/*N*/ 				{	MoveFwd( rbMakePage, FALSE );
/*N*/ 					SwFrm *pTmp = rThis.FindColFrm();
/*N*/ 					if( pTmp != pCol )
/*N*/ 					{
/*N*/ 						bMovedFwd = TRUE;
/*N*/ 						pCol = pTmp;
/*N*/ 					}
/*N*/ 					else
/*?*/ 						break;
/*N*/ 				} while ( IsColBreak( FALSE ) );
/*N*/ 				if ( pPage != rThis.FindPageFrm() )
/*N*/ 					rbMakePage = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bMovedFwd;
/*N*/ }

/*************************************************************************
|*
|*	BOOL SwFlowFrm::MoveFwd()
|*
|*	Beschreibung		Returnwert sagt, ob der Frm die Seite gewechselt hat.
|*	Ersterstellung		MA 05. Dec. 96
|*	Letzte Aenderung	MA 05. Dec. 96
|*
|*************************************************************************/


/*N*/ BOOL SwFlowFrm::MoveFwd( BOOL bMakePage, BOOL bPageBreak, BOOL bMoveAlways )
/*N*/ {
/*N*/ //!!!!MoveFtnCntFwd muss ggf. mitgepflegt werden.
/*N*/ 	SwFtnBossFrm *pOldBoss = rThis.FindFtnBossFrm();
/*N*/ 	if ( rThis.IsInFtn() )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	return ((SwCntntFrm&)rThis).MoveFtnCntFwd( bMakePage, pOldBoss );
/*N*/ 
/*N*/ 	if( !IsFwdMoveAllowed() && !bMoveAlways )
/*N*/ 	{
/*N*/ 		BOOL bNoFwd = TRUE;
/*N*/ 		if( rThis.IsInSct() )
/*N*/ 		{
/*?*/ 			SwFtnBossFrm* pBoss = rThis.FindFtnBossFrm();
/*?*/ 			bNoFwd = !pBoss->IsInSct() || ( !pBoss->Lower()->GetNext() &&
/*?*/ 					 !pBoss->GetPrev() );
/*N*/ 		}
/*N*/ 		if( bNoFwd )
/*N*/ 		{
/*N*/ 			//Fuer PageBreak ist das Moven erlaubt, wenn der Frm nicht
/*N*/ 			//bereits der erste der Seite ist.
/*N*/ 			if ( !bPageBreak )
/*?*/ 				return FALSE;
/*N*/ 
/*N*/ 			const SwFrm *pCol = rThis.FindColFrm();
/*N*/ 			if ( !pCol || !pCol->GetPrev() )
/*?*/ 				return FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bSamePage = TRUE;
/*N*/ 	SwLayoutFrm *pNewUpper =
/*N*/ 			rThis.GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, TRUE );
/*N*/ 
/*N*/ 	if ( pNewUpper )
/*N*/ 	{
/*N*/ 		PROTOCOL_ENTER( &rThis, PROT_MOVE_FWD, 0, 0 );
/*N*/ 		SwPageFrm *pOldPage = pOldBoss->FindPageFrm();
/*N*/ 		//Wir moven uns und alle direkten Nachfolger vor den ersten
/*N*/ 		//CntntFrm unterhalb des neuen Uppers.
/*N*/ 
/*N*/ 		// Wenn unser NewUpper in einem SectionFrm liegt, muessen wir
/*N*/ 		// verhindern, dass sich dieser im Calc selbst zerstoert
/*N*/ 		SwSectionFrm* pSect = pNewUpper->FindSctFrm();
/*N*/ 		BOOL bUnlock = FALSE;
/*N*/ 		if( pSect )
/*N*/ 		{
/*N*/ 			// Wenn wir nur innerhalb unseres SectionFrms die Spalte wechseln,
/*N*/ 			// rufen wir lieber kein Calc, sonst wird noch der SectionFrm
/*N*/ 			// formatiert, der wiederum uns ruft etc.
/*N*/ 			if( pSect != rThis.FindSctFrm() )
/*N*/ 			{
/*N*/ 				bUnlock = !pSect->IsColLocked();
/*N*/ 				pSect->ColLock();
/*N*/ 				pNewUpper->Calc();
/*N*/ 				if( bUnlock )
/*N*/ 					pSect->ColUnlock();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pNewUpper->Calc();
/*N*/ 
/*N*/ 		SwFtnBossFrm *pNewBoss = pNewUpper->FindFtnBossFrm();
/*N*/ 		BOOL bBossChg = pNewBoss != pOldBoss;
/*N*/ 		pNewBoss = pNewBoss->FindFtnBossFrm( TRUE );
/*N*/ 		pOldBoss = pOldBoss->FindFtnBossFrm( TRUE );
/*N*/ 		SwPageFrm* pNewPage = pOldPage;
/*N*/ 
/*N*/ 		//Erst die Fussnoten verschieben!
/*N*/ 		BOOL bFtnMoved = FALSE;
/*N*/ 		if ( pNewBoss != pOldBoss )
/*N*/ 		{
/*N*/ 			pNewPage = pNewBoss->FindPageFrm();
/*N*/ 			bSamePage = pNewPage == pOldPage;
/*N*/ 			//Damit die Fussnoten nicht auf dumme Gedanken kommen
/*N*/ 			//setzen wir hier die Deadline.
/*N*/             SWRECTFN( pOldBoss )
/*N*/ 			SwSaveFtnHeight aHeight( pOldBoss,
/*N*/                 (pOldBoss->Frm().*fnRect->fnGetBottom)() );
/*N*/ 			SwCntntFrm* pStart = rThis.IsCntntFrm() ?
/*N*/ 				(SwCntntFrm*)&rThis : ((SwLayoutFrm&)rThis).ContainsCntnt();
/*N*/ 			ASSERT( pStart, "MoveFwd: Missing Content" );
/*N*/ 			SwLayoutFrm* pBody = pStart ? ( pStart->IsTxtFrm() ?
/*N*/ 				(SwLayoutFrm*)((SwTxtFrm*)pStart)->FindBodyFrm() : 0 ) : 0;
/*N*/ 			if( pBody )
/*N*/ 				bFtnMoved = pBody->MoveLowerFtns( pStart, pOldBoss, pNewBoss,
/*N*/ 												  FALSE);
/*N*/ 		}
/*N*/ 		// Bei SectionFrms ist es moeglich, dass wir selbst durch pNewUpper->Calc()
/*N*/ 		// bewegt wurden, z. B. in den pNewUpper.
/*N*/ 		// MoveSubTree bzw. PasteTree ist auf so etwas nicht vorbereitet.
/*N*/ 		if( pNewUpper != rThis.GetUpper() )
/*N*/ 		{
/*N*/ 			MoveSubTree( pNewUpper, pNewUpper->Lower() );
/*N*/ 
/*N*/ 			if ( bFtnMoved && !bSamePage )
/*N*/ 			{
/*?*/ 				pOldPage->UpdateFtnNum();
/*?*/ 				pNewPage->UpdateFtnNum();
/*N*/ 			}
/*N*/ 
/*N*/ 			if( bBossChg )
/*N*/ 			{
/*N*/ 				rThis.Prepare( PREP_BOSS_CHGD, 0, FALSE );
/*N*/ 				if( !bSamePage )
/*N*/ 				{
/*N*/ 					ViewShell *pSh = rThis.GetShell();
/*N*/ 					if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
/*N*/ 						pSh->GetDoc()->SetNewFldLst();	//Wird von CalcLayout() hinterher erledigt!
/*N*/ 					pNewPage->InvalidateSpelling();
/*N*/ 					pNewPage->InvalidateAutoCompleteWords();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/         // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
/*N*/         if ( !pNewPage->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/         {
/*N*/             //Bei Sections kann es passieren, das wir gleich  in den Follow geflutscht
/*N*/             //sind. Dadurch wird nicht vom GetLeaf fuer die richtige Seite gesorgt.
/*N*/             //Das muessen wir fuer diesen Fall pruefen.
/*N*/             if ( !bSamePage && pNewUpper->IsInSct() &&
/*N*/                  ( rThis.GetAttrSet()->GetPageDesc().GetPageDesc() ||
/*N*/                    pOldPage->GetPageDesc()->GetFollow() != pNewPage->GetPageDesc() ) )
/*N*/                 SwFrm::CheckPageDescs( pNewPage, FALSE );
/*N*/         }
/*N*/ 	}
/*N*/ 	return bSamePage;
/*N*/ }


/*************************************************************************
|*
|*	BOOL SwFlowFrm::MoveBwd()
|*
|*	Beschreibung		Returnwert sagt, ob der Frm die Seite wechseln soll.
|*						Sollte von abgeleiteten Klassen gerufen werden.
|* 						Das moven selbst muessen die abgeleiteten uebernehmen.
|*	Ersterstellung		MA 05. Dec. 96
|*	Letzte Aenderung	MA 05. Dec. 96
|*
|*************************************************************************/

/*N*/ BOOL SwFlowFrm::MoveBwd( BOOL &rbReformat )
/*N*/ {
/*N*/ 	SwFlowFrm::SetMoveBwdJump( FALSE );
/*N*/ 
/*N*/ 	SwFtnFrm* pFtn = rThis.FindFtnFrm();
/*N*/ 	if ( pFtn && pFtn->IsBackMoveLocked() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	SwFtnBossFrm * pOldBoss = rThis.FindFtnBossFrm();
/*N*/ 	SwPageFrm * const pOldPage = pOldBoss->FindPageFrm();
/*N*/ 	SwLayoutFrm *pNewUpper = 0;
/*N*/ 	FASTBOOL bCheckPageDescs = FALSE;
/*N*/ 
/*N*/ 	if ( pFtn )
/*N*/ 	{
/*N*/ 		//Wenn die Fussnote bereits auf der gleichen Seite/Spalte wie die Referenz
/*N*/ 		//steht, ist nix mit zurueckfliessen. Die breaks brauche fuer die
/*N*/ 		//Fussnoten nicht geprueft zu werden.
/*N*/ 		BOOL bEndnote = pFtn->GetAttr()->GetFtn().IsEndNote();
/*N*/ 		SwFrm* pRef = bEndnote && pFtn->IsInSct() ?
/*N*/ 			pFtn->FindSctFrm()->FindLastCntnt( FINDMODE_LASTCNT ) : pFtn->GetRef();
/*N*/ 		ASSERT( pRef, "MoveBwd: Endnote for an empty section?" );
/*N*/ 		if( !bEndnote )
/*N*/ 			pOldBoss = pOldBoss->FindFtnBossFrm( TRUE );
/*N*/ 		SwFtnBossFrm *pRefBoss = pRef->FindFtnBossFrm( !bEndnote );
/*N*/ 		        if ( pOldBoss != pRefBoss &&
/*N*/              // OD 08.11.2002 #104840# - use <SwLayoutFrm::IsBefore(..)>
/*N*/              ( !bEndnote ||
/*N*/                pRefBoss->IsBefore( pOldBoss ) )
/*N*/            )
/*?*/ 			pNewUpper = rThis.GetLeaf( MAKEPAGE_FTN, FALSE );
/*N*/ 	}
/*N*/ 	else if ( IsPageBreak( TRUE ) )	//PageBreak zu beachten?
/*N*/ 	{
/*N*/ 		//Wenn auf der vorhergehenden Seite kein Frm im Body steht,
/*N*/ 		//so ist das Zurueckfliessen trotz Pagebreak sinnvoll
/*N*/ 		//(sonst: leere Seite).
/*N*/ 		//Natuerlich muessen Leereseiten geflissentlich uebersehen werden!
/*N*/ 		const SwFrm *pFlow = &rThis;
/*N*/ 		do
/*N*/ 		{	pFlow = pFlow->FindPrev();
/*N*/ 		} while ( pFlow && ( pFlow->FindPageFrm() == pOldPage ||
/*N*/ 				  !pFlow->IsInDocBody() ) );
/*N*/ 		if ( pFlow )
/*N*/ 		{
/*N*/ 			long nDiff = pOldPage->GetPhyPageNum() - pFlow->GetPhyPageNum();
/*N*/ 			if ( nDiff > 1 )
/*N*/ 			{
/*N*/ 				if ( ((SwPageFrm*)pOldPage->GetPrev())->IsEmptyPage() )
/*N*/ 					nDiff -= 1;
/*N*/ 				if ( nDiff > 1 )
/*N*/ 				{
/*N*/ 					pNewUpper = rThis.GetLeaf( MAKEPAGE_NONE, FALSE );
/*N*/ 
/*N*/                     //
/*N*/                     // START OF HACK for #i14206#
/*N*/                     //
/*N*/ 
/*N*/                     // Get the bodyframe of the next page.
/*N*/                     // There was a loop in this situation:
/*N*/                     // Page 5: Section frame
/*N*/                     // Page 6: Empty body frame
/*N*/                     // Page 7: Tab frame with page break before.
/*N*/                     // Here, the tab frame moves to page 5. Therefore the
/*N*/                     // section frame on page 5 is invalidated. During further
/*N*/                     // formatting of the tab frame, it is moved to page 6
/*N*/                     // because of the page break. During formatting of
/*N*/                     // the section frame, the tab frame moves to page 7 again and so on.
/*N*/ 
/*N*/                     if ( pFlow->IsInSct() && SwFlowFrm::IsMoveBwdJump() && 2 == nDiff &&
/*N*/                          !((SwPageFrm*)pOldPage->GetPrev())->IsEmptyPage() &&
/*N*/                          pNewUpper && pNewUpper->IsPageBodyFrm() )
/*N*/                     {
/*N*/     					SwPageFrm* pNextPage = (SwPageFrm*)pNewUpper->GetUpper()->GetNext();
/*N*/                         if ( pNextPage )
/*N*/                         {
/*N*/                             SwFrm* pLayout = pNextPage->Lower();
/*N*/                             if ( pLayout && pLayout->IsHeaderFrm() )
/*N*/                                 pLayout = pLayout->GetNext();
/*N*/ 
/*N*/                             if ( pLayout && pLayout->IsBodyFrm() && !((SwLayoutFrm*)pLayout)->Lower() )
/*N*/                             {
/*N*/                                 pNewUpper = (SwLayoutFrm*)pLayout;
/*N*/                             	SwFlowFrm::SetMoveBwdJump( FALSE );
/*N*/                             }
/*N*/                         }
/*N*/                     }
/*N*/ 
/*N*/                     //
/*N*/                     // END OF HACK for #i14206#
/*N*/                     //
/*N*/ 
/*N*/ 					bCheckPageDescs = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( IsColBreak( TRUE ) )
/*N*/ 	{
/*N*/ 		//Wenn in der vorhergehenden Spalte kein CntntFrm steht, so ist
/*N*/ 		//das Zurueckfliessen trotz ColumnBreak sinnvoll
/*N*/ 		//(sonst: leere Spalte).
/*N*/ 		if( rThis.IsInSct() )
/*N*/ 		{
/*?*/ 			pNewUpper = rThis.GetLeaf( MAKEPAGE_NONE, FALSE );
/*?*/ 			if( pNewUpper && !SwFlowFrm::IsMoveBwdJump() &&
/*?*/ 				( pNewUpper->ContainsCntnt() ||
/*?*/ 				  ( ( !pNewUpper->IsColBodyFrm() ||
/*?*/ 					  !pNewUpper->GetUpper()->GetPrev() ) &&
/*?*/ 					!pNewUpper->FindSctFrm()->GetPrev() ) ) )
/*?*/ 				pNewUpper = 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwFrm *pCol = rThis.FindColFrm();
/*N*/ 			BOOL bGoOn = TRUE;
/*N*/ 			BOOL bJump = FALSE;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				if ( pCol->GetPrev() )
/*N*/ 					pCol = pCol->GetPrev();
/*N*/ 				else
/*N*/ 				{
/*N*/ 					bGoOn = FALSE;
/*N*/ 					pCol = rThis.GetLeaf( MAKEPAGE_NONE, FALSE );
/*N*/ 				}
/*N*/ 				if ( pCol )
/*N*/ 				{
/*N*/ 					// ColumnFrms jetzt mit BodyFrm
/*N*/ 					SwLayoutFrm* pColBody = pCol->IsColumnFrm() ?
/*N*/ 						(SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower() :
/*N*/ 						(SwLayoutFrm*)pCol;
/*N*/ 					if ( pColBody->ContainsCntnt() )
/*N*/ 					{
/*N*/ 						bGoOn = FALSE; // Hier gibt's Inhalt, wir akzeptieren diese
/*N*/ 						// nur, wenn GetLeaf() das MoveBwdJump-Flag gesetzt hat.
/*N*/ 						if( SwFlowFrm::IsMoveBwdJump() )
/*N*/ 							pNewUpper = pColBody;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if( pNewUpper ) // Wir hatten schon eine leere Spalte, haben
/*?*/ 							bJump = TRUE;   // also eine uebersprungen
/*N*/ 						pNewUpper = pColBody;  // Diese leere Spalte kommt in Frage,
/*N*/ 										   	// trotzdem weitersuchen
/*N*/ 					}
/*N*/ 				}
/*N*/ 			} while( bGoOn );
/*N*/ 			if( bJump )
/*?*/ 				SwFlowFrm::SetMoveBwdJump( TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else //Keine Breaks also kann ich zurueckfliessen
/*N*/ 		pNewUpper = rThis.GetLeaf( MAKEPAGE_NONE, FALSE );
/*N*/ 
/*N*/ 	//Fuer Follows ist das zurueckfliessen nur dann erlaubt wenn in der
/*N*/ 	//neuen Umgebung kein Nachbar existiert (denn dieses waere der Master).
/*N*/ 	//(6677)Wenn allerdings leere Blaetter uebersprungen wurden wird doch gemoved.
/*N*/ 	if ( pNewUpper && IsFollow() && pNewUpper->Lower() )
/*N*/ 	{
/*N*/ 		if ( SwFlowFrm::IsMoveBwdJump() )
/*N*/ 		{
/*N*/ 			//Nicht hinter den Master sondern in das naechstfolgende leere
/*N*/ 			//Blatt moven.
/*N*/ 			SwFrm *pFrm = pNewUpper->Lower();
/*N*/ 			while ( pFrm->GetNext() )
/*N*/ 				pFrm = pFrm->GetNext();
/*N*/ 			pNewUpper = pFrm->GetLeaf( MAKEPAGE_INSERT, TRUE );
/*N*/ 			if( pNewUpper == rThis.GetUpper() )	//Landen wir wieder an der gleichen Stelle?
/*?*/ 				pNewUpper = NULL;			//dann eruebrigt sich das Moven
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pNewUpper = 0;
/*N*/ 	}
/*N*/ 	if ( pNewUpper && !ShouldBwdMoved( pNewUpper, TRUE, rbReformat ) )
/*N*/ 	{
/*N*/ 		if( !pNewUpper->Lower() )
/*N*/ 		{
/*N*/ 			if( pNewUpper->IsFtnContFrm() )
/*N*/ 			{
/*?*/ 				pNewUpper->Cut();
/*?*/ 				delete pNewUpper;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwSectionFrm* pSectFrm = pNewUpper->FindSctFrm();
/*N*/ 				if( pSectFrm && !pSectFrm->IsColLocked() && !pSectFrm->ContainsCntnt() )
/*N*/ 				{
/*N*/ 					pSectFrm->DelEmpty( TRUE );
/*N*/ 					delete pSectFrm;
/*N*/ 					rThis.bValidPos = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pNewUpper = 0;
/*N*/ 	}
/*N*/ 	if ( pNewUpper )
/*N*/ 	{
/*N*/ 		PROTOCOL_ENTER( &rThis, PROT_MOVE_BWD, 0, 0 );
/*N*/ 		if ( pNewUpper->IsFtnContFrm() )
/*N*/ 		{
/*?*/ 			//Kann sein, dass ich einen Container bekam.
/*?*/ 			SwFtnFrm *pOld = rThis.FindFtnFrm();
/*?*/ 			SwFtnFrm *pNew = new SwFtnFrm( pOld->GetFmt(),
/*?*/ 										   pOld->GetRef(), pOld->GetAttr() );
/*?*/ 			if ( pOld->GetMaster() )
/*?*/ 			{
/*?*/ 				pNew->SetMaster( pOld->GetMaster() );
/*?*/ 				pOld->GetMaster()->SetFollow( pNew );
/*?*/ 			}
/*?*/ 			pNew->SetFollow( pOld );
/*?*/ 			pOld->SetMaster( pNew );
/*?*/ 			pNew->Paste( pNewUpper );
/*?*/ 			pNewUpper = pNew;
/*N*/ 		}
/*N*/ 		if( pNewUpper->IsFtnFrm() && rThis.IsInSct() )
/*N*/ 		{
/*?*/ 			SwSectionFrm* pSct = rThis.FindSctFrm();
/*?*/ 			//Wenn wir in einem Bereich in einer Fussnote stecken, muss im
/*?*/ 			//neuen Upper ggf. ein SwSectionFrm angelegt werden
/*?*/ 			if( pSct->IsInFtn() )
/*?*/ 			{
/*?*/ 				SwFrm* pTmp = pNewUpper->Lower();
/*?*/ 				if( pTmp )
/*?*/ 				{
/*?*/ 					while( pTmp->GetNext() )
/*?*/ 						pTmp = pTmp->GetNext();
/*?*/ 					if( !pTmp->IsSctFrm() ||
/*?*/ 						((SwSectionFrm*)pTmp)->GetFollow() != pSct )
/*?*/ 						pTmp = NULL;
/*?*/ 				}
/*?*/ 				if( pTmp )
/*?*/ 					pNewUpper = (SwSectionFrm*)pTmp;
/*?*/ 				else
/*?*/ 				{
/*?*/                     DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pSct = new SwSectionFrm( *pSct, TRUE );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		BOOL bUnlock = FALSE;
/*N*/ 		BOOL bFollow;
/*N*/ 		//Section locken, sonst kann sie bei Fluss des einzigen Cntnt etwa
/*N*/ 		//von zweiter in die erste Spalte zerstoert werden.
/*N*/ 		SwSectionFrm* pSect = pNewUpper->FindSctFrm();
/*N*/ 		if( pSect )
/*N*/ 		{
/*N*/ 			bUnlock = !pSect->IsColLocked();
/*N*/ 			pSect->ColLock();
/*N*/ 			bFollow = pSect->HasFollow();
/*N*/ 		}
/*N*/ 		pNewUpper->Calc();
/*N*/ 		rThis.Cut();
/*N*/ 		if( bUnlock )
/*N*/ 		{
/*N*/ 			if( pSect->HasFollow() != bFollow )
/*N*/ 				pSect->_InvalidateSize();
/*N*/ 			pSect->ColUnlock();
/*N*/ 		}
/*N*/ 
/*N*/ 		rThis.Paste( pNewUpper );
/*N*/ 
/*N*/         SwPageFrm *pNewPage = rThis.FindPageFrm();
/*N*/ 		if( pNewPage != pOldPage )
/*N*/ 		{
/*N*/ 			rThis.Prepare( PREP_BOSS_CHGD, (const void*)pOldPage, FALSE );
/*N*/ 			ViewShell *pSh = rThis.GetShell();
/*N*/ 			if ( pSh && !pSh->Imp()->IsUpdateExpFlds() )
/*N*/ 				pSh->GetDoc()->SetNewFldLst();	//Wird von CalcLayout() hinterher eledigt!
/*N*/ 			pNewPage->InvalidateSpelling();
/*N*/ 			pNewPage->InvalidateAutoCompleteWords();
/*N*/             // OD 30.10.2002 #97265# - no <CheckPageDesc(..)> in online layout
/*N*/             if ( !pNewPage->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/             {
/*N*/                 if ( bCheckPageDescs && pNewPage->GetNext() )
/*N*/                     SwFrm::CheckPageDescs( (SwPageFrm*)pNewPage->GetNext(), FALSE);
/*N*/                 else if ( rThis.GetAttrSet()->GetPageDesc().GetPageDesc() )
/*N*/                 {
/*N*/                     //Erste Seite wird etwa durch Ausblenden eines Bereiches leer
/*N*/                     SwFrm::CheckPageDescs( (SwPageFrm*)pNewPage, FALSE);
/*N*/                 }
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pNewUpper != 0;
/*N*/ }

/*************************************************************************
|*
|*	SwFlowFrm::CastFlowFrm
|*
|*	Ersterstellung		MA 03. May. 95
|*	Letzte Aenderung	AMA 02. Dec. 97
|*
|*************************************************************************/

/*N*/ SwFlowFrm *SwFlowFrm::CastFlowFrm( SwFrm *pFrm )
/*N*/ {
/*N*/ 	if ( pFrm->IsCntntFrm() )
/*N*/ 		return (SwCntntFrm*)pFrm;
/*N*/ 	if ( pFrm->IsTabFrm() )
/*N*/ 		return (SwTabFrm*)pFrm;
/*N*/ 	if ( pFrm->IsSctFrm() )
/*N*/ 		return (SwSectionFrm*)pFrm;
/*?*/ 	return 0;
/*N*/ }

/*N*/ const SwFlowFrm *SwFlowFrm::CastFlowFrm( const SwFrm *pFrm )
/*N*/ {
/*N*/ 	if ( pFrm->IsCntntFrm() )
/*N*/ 		return (SwCntntFrm*)pFrm;
/*N*/ 	if ( pFrm->IsTabFrm() )
/*N*/ 		return (SwTabFrm*)pFrm;
/*N*/ 	if ( pFrm->IsSctFrm() )
/*N*/ 		return (SwSectionFrm*)pFrm;
/*?*/ 	return 0;
/*N*/ }





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
