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

#include "pagefrm.hxx"
#include "viewsh.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"

#ifndef _SVX_KEEPITEM_HXX //autogen
#include <bf_svx/keepitem.hxx>
#endif

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif

#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "frmsh.hxx"
#include "pagedesc.hxx"
#include "sectfrm.hxx"
#include "dbg_lay.hxx"
namespace binfilter {


//------------------------------------------------------------------------
//				Move-Methoden
//------------------------------------------------------------------------

/*************************************************************************
|*
|*	SwCntntFrm::ShouldBwdMoved()
|*
|* 	Beschreibung		Returnwert sagt, ob der Frm verschoben werden sollte.
|*	Ersterstellung		MA 05. Dec. 96
|*	Letzte Aenderung	MA 05. Dec. 96
|*
|*************************************************************************/


/*N*/ BOOL SwCntntFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL, BOOL & )
/*N*/ {
/*N*/ 	if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()))
/*N*/ 	{
/*N*/ 		//Das zurueckfliessen von Frm's ist leider etwas Zeitintensiv.
/*N*/ 		//Der haufigste Fall ist der, dass dort wo der Frm hinfliessen
/*N*/ 		//moechte die FixSize die gleiche ist, die der Frm selbst hat.
/*N*/ 		//In diesem Fall kann einfach geprueft werden, ob der Frm genug
/*N*/ 		//Platz fuer seine VarSize findet, ist dies nicht der Fall kann
/*N*/ 		//gleich auf das Verschieben verzichtet werden.
/*N*/ 		//Die Pruefung, ob der Frm genug Platz findet fuehrt er selbst
/*N*/ 		//durch, dabei wird beruecksichtigt, dass er sich moeglicherweise
/*N*/ 		//aufspalten kann.
/*N*/ 		//Wenn jedoch die FixSize eine andere ist oder Flys im Spiel sind
/*N*/ 		//(an der alten oder neuen Position) hat alle Prueferei keinen Sinn
/*N*/ 		//der Frm muss dann halt Probehalber verschoben werden (Wenn ueberhaupt
/*N*/ 		//etwas Platz zur Verfuegung steht).
/*N*/ 
/*N*/ 		//Die FixSize der Umgebungen in denen Cntnts herumlungern ist immer
/*N*/ 		//Die Breite.
/*N*/ 
/*N*/ 		//Wenn mehr als ein Blatt zurueckgegangen wurde (z.B. ueberspringen
/*N*/ 		//von Leerseiten), so muss in jedemfall gemoved werden - sonst wuerde,
/*N*/ 		//falls der Frm nicht in das Blatt passt, nicht mehr auf die
/*N*/ 		//dazwischenliegenden Blaetter geachtet werden.
/*N*/ 		BYTE nMoveAnyway = 0;
/*N*/ 		SwPageFrm * const pNewPage = pNewUpper->FindPageFrm();
/*N*/ 		SwPageFrm *pOldPage = FindPageFrm();
/*N*/ 
/*N*/ 		if ( SwFlowFrm::IsMoveBwdJump() )
/*N*/ 			return TRUE;
/*N*/ 
/*N*/ 		if( IsInFtn() && IsInSct() )
/*N*/ 		{
/*?*/ 			SwFtnFrm* pFtn = FindFtnFrm();
/*?*/ 			SwSectionFrm* pMySect = pFtn->FindSctFrm();
/*?*/ 			if( pMySect && pMySect->IsFtnLock() )
/*?*/ 			{
/*?*/ 				SwSectionFrm *pSect = pNewUpper->FindSctFrm();
/*?*/ 				while( pSect && pSect->IsInFtn() )
/*?*/ 					pSect = pSect->GetUpper()->FindSctFrm();
/*?*/ 				ASSERT( pSect, "Escaping footnote" );
/*?*/ 				if( pSect != pMySect )
/*?*/ 					return FALSE;
/*?*/ 			}
/*N*/ 		}
/*N*/         SWRECTFN( this )
/*N*/         SWRECTFNX( pNewUpper )
/*N*/         if( Abs( (pNewUpper->Prt().*fnRectX->fnGetWidth)() -
/*N*/                  (GetUpper()->Prt().*fnRect->fnGetWidth)() ) > 1 )
/*N*/ 			nMoveAnyway = 2; // Damit kommt nur noch ein _WouldFit mit Umhaengen in Frage
/*N*/ 		if ( (nMoveAnyway |= BwdMoveNecessary( pOldPage, Frm() )) < 3 )
/*N*/ 		{
/*N*/ 			SwTwips nSpace = 0;
/*N*/ 			SwRect aRect( pNewUpper->Prt() );
/*N*/ 			aRect.Pos() += pNewUpper->Frm().Pos();
/*N*/ 			const SwFrm *pPrevFrm = pNewUpper->Lower();
/*N*/ 			while ( pPrevFrm )
/*N*/ 			{
/*N*/                 (aRect.*fnRectX->fnSetTop)(
/*N*/                     (pPrevFrm->Frm().*fnRectX->fnGetBottom)() );
/*N*/ 				pPrevFrm = pPrevFrm->GetNext();
/*N*/ 			}
/*N*/ 
/*N*/ 			nMoveAnyway |= BwdMoveNecessary( pNewPage, aRect);
/*N*/ 			if ( nMoveAnyway < 3 )
/*N*/ 			{
/*N*/                 //Zur Verfuegung stehenden Raum berechnen.
/*N*/                 nSpace = (aRect.*fnRectX->fnGetHeight)();
/*N*/ 				if ( IsInFtn() || GetAttrSet()->GetDoc()->IsBrowseMode() ||
/*N*/ 					 ( pNewUpper->IsInSct() && ( pNewUpper->IsSctFrm() ||
/*N*/ 					   ( pNewUpper->IsColBodyFrm() &&
/*N*/ 					     !pNewUpper->GetUpper()->GetPrev() &&
/*N*/ 						 !pNewUpper->GetUpper()->GetNext() ) ) ) )
/*N*/                     nSpace += pNewUpper->Grow( LONG_MAX PHEIGHT, TRUE );
/*N*/ 				if ( nSpace )
/*N*/ 				{
/*N*/ 					//Keine Beruecksichtigung der Fussnoten die an dem Absatz
/*N*/ 					//kleben, denn dies wuerde extrem unuebersichtlichen Code
/*N*/ 					//beduerfen (wg. Beruecksichtung der Breiten und vor allem
/*N*/ 					//der Flys, die ja wieder Einfluss auf die Fussnoten nehmen...).
/*N*/ 
/*N*/ 					// _WouldFit kann bei gleicher Breite und _nur_ selbst verankerten Flys
/*N*/ 					// befragt werden.
/*N*/ 					// _WouldFit kann auch gefragt werden, wenn _nur_ fremdverankerte Flys vorliegen,
/*N*/ 					// dabei ist sogar die Breite egal, da ein TestFormat in der neuen Umgebung
/*N*/ 					// vorgenommen wird.
/*N*/ 					return _WouldFit( nSpace, pNewUpper, nMoveAnyway == 2 );
/*N*/ 				}
/*N*/ 				//Bei einem spaltigen Bereichsfrischling kann _WouldFit kein
/*N*/ 				//brauchbares Ergebnis liefern, also muessen wir wirklich
/*N*/ 				//zurueckfliessen
/*N*/ 				else if( pNewUpper->IsInSct() && pNewUpper->IsColBodyFrm() &&
/*N*/                     !(pNewUpper->Prt().*fnRectX->fnGetWidth)() &&
/*N*/ 					( pNewUpper->GetUpper()->GetPrev() ||
/*N*/ 					  pNewUpper->GetUpper()->GetNext() ) )
/*?*/ 					return TRUE;
/*N*/ 				else
/*N*/ 					return FALSE; // Kein Platz, dann ist es sinnlos, zurueckzufliessen
/*N*/ 			}
/*N*/ 		}
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return  FALSE;
/*N*/ }

//------------------------------------------------------------------------
//				Calc-Methoden
//------------------------------------------------------------------------

/*************************************************************************
|*
|*	SwFrm::Prepare()
|*
|*	Beschreibung		Bereitet den Frm auf die 'Formatierung' (MakeAll())
|*		vor. Diese Methode dient dazu auf dem Stack Platz einzusparen,
|*		denn zur Positionsberechnung des Frm muss sichergestellt sein, dass
|*		die Position von Upper und Prev gueltig sind, mithin also ein
|*		rekursiver Aufruf (Schleife waere relativ teuer, da selten notwendig).
|*		Jeder Aufruf von MakeAll verbraucht aber ca. 500Byte Stack -
|*		das Ende ist leicht abzusehen. _Prepare benoetigt nur wenig Stack,
|*		deshalb solle der Rekursive Aufruf hier kein Problem sein.
|*		Ein weiterer Vorteil ist, das eines schoenen Tages das _Prepare und
|*		damit die Formatierung von Vorgaengern umgangen werden kann.
|*		So kann evtl. mal 'schnell' an's Dokumentende gesprungen werden.
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 13. Dec. 93
|*
|*************************************************************************/
//Zwei kleine Freundschaften werden hier zu einem Geheimbund.
/*N*/ inline void PrepareLock( SwFlowFrm *pTab )
/*N*/ {
/*N*/ 	pTab->LockJoin();
/*N*/ }
/*N*/ inline void PrepareUnlock( SwFlowFrm *pTab )
/*N*/ {
/*N*/ 	pTab->UnlockJoin();
/*N*/ 
/*N*/ }



/*N*/ void SwFrm::PrepareMake()
/*N*/ {
/*N*/ 	StackHack aHack;
/*N*/ 	if ( GetUpper() )
/*N*/ 	{
/*M*/ 		if( !GetUpper()->IsSctFrm() && !GetUpper()->IsFooterFrm() )
/*N*/ 			GetUpper()->Calc();
/*N*/ 		ASSERT( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
/*N*/ 		if ( !GetUpper() )
/*?*/ 			return;
/*N*/ 
/*N*/ 		const BOOL bCnt = IsCntntFrm();
/*N*/ 		const BOOL bTab = IsTabFrm();
/*N*/ 		BOOL bNoSect = IsInSct();
/*N*/ 		BOOL bOldTabLock = FALSE, bFoll = FALSE;
/*N*/ 		SwFlowFrm* pThis = bCnt ? (SwCntntFrm*)this : NULL;
/*N*/ 
/*N*/ 		if ( bTab )
/*N*/ 		{
/*N*/ 			pThis = (SwTabFrm*)this;
/*N*/ 			bOldTabLock = ((SwTabFrm*)this)->IsJoinLocked();
/*N*/ 			::binfilter::PrepareLock( (SwTabFrm*)this );
/*N*/ 			bFoll = pThis->IsFollow();
/*N*/ 		}
/*N*/ 		else if( IsSctFrm() )
/*N*/ 		{
/*N*/ 			pThis = (SwSectionFrm*)this;
/*N*/ 			bFoll = pThis->IsFollow();
/*N*/ 			bNoSect = FALSE;
/*N*/ 		}
/*N*/ 		else if ( bCnt && TRUE == (bFoll = pThis->IsFollow()) &&
/*N*/ 			 GetPrev() )
/*N*/ 		{
/*N*/ 			//Wenn der Master gereade ein CalcFollow ruft braucht die Kette
/*N*/ 			//nicht durchlaufen werden. Das spart Zeit und vermeidet Probleme.
/*N*/ 			if ( ((SwTxtFrm*)((SwTxtFrm*)this)->FindMaster())->IsLocked() )
/*N*/ 			{
/*N*/ 				MakeAll();
/*N*/ 				return;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		SwFrm *pFrm = GetUpper()->Lower();
/*N*/ 		while ( pFrm != this )
/*N*/ 		{
/*N*/ 			ASSERT( pFrm, ":-( Layoutgeruest wackelig (this not found)." );
/*N*/ 			if ( !pFrm )
/*?*/ 				return;	//Oioioioi ...
/*N*/ 
/*N*/ 			if ( !pFrm->IsValid() )
/*N*/ 			{
/*N*/ 				//Ein kleiner Eingriff der hoffentlich etwas zur Verbesserung
/*N*/ 				//der Stabilitaet beitraegt:
/*N*/ 				//Wenn ich Follow _und_ Nachbar eines Frms vor mir bin,
/*N*/ 				//so wuerde dieser mich beim Formatieren deleten; wie jeder
/*N*/ 				//leicht sehen kann waere dies eine etwas unuebersichtliche
/*N*/ 				//Situation die es zu vermeiden gilt.
/*N*/ 				if ( bFoll && pFrm->IsFlowFrm() &&
/*N*/ 					 (SwFlowFrm::CastFlowFrm(pFrm))->IsAnFollow( pThis ) )
/*?*/ 					break;
/*N*/ 
/*N*/ //MA: 24. Mar. 94, Calc wuerde doch nur wieder in ein _Prepare laufen und so
/*N*/ //die ganze Kette nocheinmal abhuenern.
/*N*/ //				pFrm->Calc();
/*N*/ 				pFrm->MakeAll();
/*N*/ 				if( IsSctFrm() && !((SwSectionFrm*)this)->GetSection() )
/*?*/ 					break;
/*N*/ 			}
/*N*/ 			//Die Kette kann bei CntntFrms waehrend des durchlaufens
/*N*/ 			//aufgebrochen werden, deshalb muss der Nachfolger etwas
/*N*/ 			//umstaendlich ermittelt werden. However, irgendwann _muss_
/*N*/ 			//ich wieder bei mir selbst ankommen.
/*N*/ 			pFrm = pFrm->FindNext();
/*N*/ 
/*N*/ 			//Wenn wir in einem SectionFrm gestartet sind, koennen wir durch die
/*N*/ 			//MakeAll-Aufrufe in einen Section-Follow gewandert sein.
/*N*/ 			//FindNext liefert allerdings den SectionFrm, nicht seinen Inhalt.
/*N*/ 			// => wir finden uns selbst nicht mehr!
/*N*/ 			if( bNoSect && pFrm && pFrm->IsSctFrm() )
/*N*/ 			{
/*N*/ 				SwFrm* pCnt = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 				if( pCnt )
/*N*/ 					pFrm = pCnt;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech II)." );
/*N*/ 		if ( !GetUpper() )
/*?*/ 			return;
/*N*/ 
/*M*/ 		if( !GetUpper()->IsSctFrm() && !GetUpper()->IsFooterFrm() )
/*N*/ 			GetUpper()->Calc();
/*N*/ 
/*N*/ 		ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech III)." );
/*N*/ 
/*N*/ 		if ( bTab && !bOldTabLock )
/*N*/ 			::binfilter::PrepareUnlock( (SwTabFrm*)this );
/*N*/ 	}
/*N*/ 	MakeAll();
/*N*/ }

/*N*/ void SwFrm::OptPrepareMake()
/*N*/ {
/*M*/	if ( GetUpper() && !GetUpper()->IsFooterFrm() )
/*N*/ 	{
/*N*/ 		GetUpper()->Calc();
/*N*/ 		ASSERT( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
/*N*/ 		if ( !GetUpper() )
/*?*/ 			return;
/*N*/ 	}
/*N*/ 	if ( GetPrev() && !GetPrev()->IsValid() )
/*N*/ 		PrepareMake();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		StackHack aHack;
/*N*/ 		MakeAll();
/*N*/ 	}
/*N*/ }




/*************************************************************************
|*
|*	SwFrm::MakePos()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 24. May. 93
|*
|*************************************************************************/

// Hier wird GetPrev() zurueckgegeben, allerdings werden
// dabei leere SectionFrms ueberlesen
/*N*/ SwFrm* lcl_Prev( SwFrm* pFrm, BOOL bSectPrv = TRUE )
/*N*/ {
/*N*/ 	SwFrm* pRet = pFrm->GetPrev();
/*N*/ 	if( !pRet && pFrm->GetUpper() && pFrm->GetUpper()->IsSctFrm() &&
/*N*/ 		bSectPrv && !pFrm->IsColumnFrm() )
/*N*/ 		pRet = pFrm->GetUpper()->GetPrev();
/*N*/ 	while( pRet && pRet->IsSctFrm() &&
/*N*/ 		   !((SwSectionFrm*)pRet)->GetSection() )
/*N*/ 		pRet = pRet->GetPrev();
/*N*/ 	return pRet;
/*N*/ }

/*N*/ SwFrm* lcl_NotHiddenPrev( SwFrm* pFrm )
/*N*/ {
/*N*/ 	SwFrm *pRet = pFrm;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		pRet = lcl_Prev( pRet );
/*N*/ 	} while ( pRet && pRet->IsTxtFrm() && ((SwTxtFrm*)pRet)->IsHiddenNow() );
/*N*/ 	return pRet;
/*N*/ }

/*N*/ void SwFrm::MakePos()
/*N*/ {
/*N*/ 	if ( !bValidPos )
/*N*/ 	{
/*N*/ 		bValidPos = TRUE;
/*N*/ 		FASTBOOL bUseUpper = FALSE;
/*N*/ 		SwFrm* pPrv = lcl_Prev( this );
/*N*/         if ( pPrv &&
/*N*/              ( !pPrv->IsCntntFrm() ||
/*N*/                ( ((SwCntntFrm*)pPrv)->GetFollow() != this ) )
/*N*/            )
/*N*/ 		{
/*N*/             if ( !StackHack::IsLocked() &&
/*N*/                  ( !IsInSct() || IsSctFrm() ) &&
/*N*/                  !pPrv->IsSctFrm() &&
/*N*/                  !pPrv->GetAttrSet()->GetKeep().GetValue()
/*N*/                )
/*N*/             {
/*N*/ 				pPrv->Calc();	//hierbei kann der Prev verschwinden!
/*N*/             }
/*N*/ 			else if ( pPrv->Frm().Top() == 0 )
/*N*/             {
/*N*/ 				bUseUpper = TRUE;
/*N*/             }
/*N*/ 		}
/*N*/ 
/*N*/ 		pPrv = lcl_Prev( this, FALSE );
/*N*/         USHORT nMyType = GetType();
/*N*/         SWRECTFN( this )
/*N*/ 		if ( !bUseUpper && pPrv )
/*N*/         {
/*N*/             aFrm.Pos( pPrv->Frm().Pos() );
/*N*/             if( FRM_NEIGHBOUR & nMyType )
/*N*/             {
/*N*/                 BOOL bR2L = IsRightToLeft();
/*N*/                 if( bR2L )
/*N*/                     (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() -
/*N*/                                                (aFrm.*fnRect->fnGetWidth)() );
/*N*/                 else
/*N*/                     (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() +
/*N*/                                           (pPrv->Frm().*fnRect->fnGetWidth)() );
/*N*/             }
/*N*/             else if( bVert && FRM_NOTE_VERT & nMyType )
/*N*/             {
/*N*/                 if( bReverse )
/*N*/                     aFrm.Pos().X() += pPrv->Frm().Width();
/*N*/                 else
/*N*/                     aFrm.Pos().X() -= aFrm.Width();
/*N*/             }
/*N*/             else
/*N*/                 aFrm.Pos().Y() += pPrv->Frm().Height();
/*N*/ 		}
/*N*/ 		else if ( GetUpper() )
/*N*/ 		{
/*N*/             /// OD 15.10.2002 #103517# - add safeguard for <SwFooterFrm::Calc()>
/*N*/             /// If parent frame is a footer frame and its <ColLocked()>, then
/*N*/             /// do *not* calculate it.
/*N*/             /// NOTE: Footer frame is <ColLocked()> during its
/*N*/             ///     <FormatSize(..)>, which is called from <Format(..)>, which
/*N*/             ///     is called from <MakeAll()>, which is called from <Calc()>.
/*N*/             if ( !GetUpper()->IsSctFrm() &&
/*N*/                  !( GetUpper()->IsFooterFrm() &&
/*N*/                     GetUpper()->IsColLocked() )
/*N*/                )
/*N*/             {
/*N*/                 SwFlyFrm* pTmpFly = FindFlyFrm();
/*N*/                 if( !pTmpFly || !pTmpFly->IsFlyInCntFrm() )
/*N*/                     GetUpper()->Calc();
/*N*/             }
/*N*/ 			pPrv = lcl_Prev( this, FALSE );
/*N*/ 			if ( !bUseUpper && pPrv )
/*N*/             {
/*N*/                 aFrm.Pos( pPrv->Frm().Pos() );
/*N*/                 if( FRM_NEIGHBOUR & nMyType )
/*N*/                 {
/*N*/                     BOOL bR2L = IsRightToLeft();
/*N*/                     if( bR2L )
/*N*/                         (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() -
/*N*/                                                  (aFrm.*fnRect->fnGetWidth)() );
/*N*/                     else
/*N*/                         (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() +
/*N*/                                           (pPrv->Frm().*fnRect->fnGetWidth)() );
/*N*/                 }
/*N*/                 else if( bVert && FRM_NOTE_VERT & nMyType )
/*N*/                 {
/*N*/                     if( bReverse )
/*N*/                         aFrm.Pos().X() += pPrv->Frm().Width();
/*N*/                     else
/*N*/                         aFrm.Pos().X() -= aFrm.Width();
/*N*/                 }
/*N*/                 else
/*N*/                     aFrm.Pos().Y() += pPrv->Frm().Height();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				aFrm.Pos( GetUpper()->Frm().Pos() );
/*N*/ 				aFrm.Pos() += GetUpper()->Prt().Pos();
/*N*/                 if( FRM_NEIGHBOUR & nMyType && IsRightToLeft() )
/*N*/                 {
/*N*/                     if( bVert )
/*N*/                         aFrm.Pos().Y() += GetUpper()->Prt().Height()
/*N*/                                           - aFrm.Height();
/*N*/                     else
/*N*/                         aFrm.Pos().X() += GetUpper()->Prt().Width()
/*N*/                                           - aFrm.Width();
/*N*/                 }
/*N*/                 else if( bVert && FRM_NOTE_VERT & nMyType && !bReverse )
/*N*/                     aFrm.Pos().X() -= aFrm.Width() - GetUpper()->Prt().Width();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aFrm.Pos().X() = aFrm.Pos().Y() = 0;
/*N*/         if( IsBodyFrm() && bVert && !bReverse && GetUpper() )
/*N*/             aFrm.Pos().X() += GetUpper()->Prt().Width() - aFrm.Width();
/*N*/ 		bValidPos = TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::MakeAll()
|*
|*	Ersterstellung		MA 23. Feb. 93
|*	Letzte Aenderung	MA 20. Jul. 98
|*
|*************************************************************************/

/*N*/ void lcl_CheckObjects( SwSortDrawObjs* pSortedObjs, SwFrm* pFrm, long& rBot )
/*N*/ {
/*N*/ 	//Und dann kann es natuerlich noch Absatzgebundene
/*N*/ 	//Rahmen geben, die unterhalb ihres Absatzes stehen.
/*N*/ 	long nMax = 0;
/*N*/ 	for ( USHORT i = 0; i < pSortedObjs->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pObj = (*pSortedObjs)[i];
/*N*/ 		long nTmp = 0;
/*N*/ 		if ( pObj->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 			if( pFly->Frm().Top() != WEIT_WECH &&
/*N*/ 				( pFrm->IsPageFrm() ? pFly->IsFlyLayFrm() :
/*N*/ 				  ( pFly->IsFlyAtCntFrm() &&
/*N*/ 					( pFrm->IsBodyFrm() ? pFly->GetAnchor()->IsInDocBody() :
/*N*/ 										  pFly->GetAnchor()->IsInFtn() ) ) ) )
/*N*/ 			{
/*N*/ 				nTmp = pFly->Frm().Bottom();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nTmp = pObj->GetBoundRect().Bottom();
/*N*/ 		nMax = Max( nTmp, nMax );
/*N*/ 	}
/*N*/ 	++nMax; //Unterkante vs. Hoehe!
/*N*/ 	rBot = Max( rBot, nMax );
/*N*/ }


/*N*/ void SwPageFrm::MakeAll()
/*N*/ {
/*N*/ 	PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )
/*N*/ 
/*N*/ 	const SwRect aOldRect( Frm() );		//Anpassung der Root-Groesse
/*N*/ 	const SwLayNotify aNotify( this );	//uebernimmt im DTor die Benachrichtigung
/*N*/ 	SwBorderAttrAccess *pAccess = 0;
/*N*/ 	const SwBorderAttrs*pAttrs = 0;
/*N*/ 
/*N*/ 	while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	{
/*N*/ 		if ( !bValidPos )
/*N*/ 		{
/*N*/ 			MakePos();
/*N*/ 			if ( GetPrev() &&  !((SwPageFrm*)GetPrev())->IsEmptyPage() )
/*N*/ 				aFrm.Pos().Y() += DOCUMENTBORDER/2;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( !bValidSize || !bValidPrtArea )
/*N*/ 		{
/*N*/ 			if ( IsEmptyPage() )
/*N*/ 			{
/*N*/ 				Frm().Width( 0 );  Prt().Width( 0 );
/*N*/ 				Frm().Height( 0 ); Prt().Height( 0 );
/*N*/ 				Prt().Left( 0 );   Prt().Top( 0 );
/*N*/ 				bValidSize = bValidPrtArea = TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if ( !pAccess )
/*N*/ 				{
/*N*/ 					pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
/*N*/ 					pAttrs = pAccess->Get();
/*N*/ 				}
/*N*/ 				//Bei der BrowseView gelten feste Einstellungen.
/*N*/ 				ViewShell *pSh = GetShell();
/*N*/ 				if ( pSh && GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 				{
/*N*/ 					const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
/*N*/ 					const long nTop    = pAttrs->CalcTopLine()   + aBorder.Height();
/*N*/ 					const long nBottom = pAttrs->CalcBottomLine()+ aBorder.Height();
/*N*/ 
/*N*/ 					long nWidth = GetUpper() ? ((SwRootFrm*)GetUpper())->
/*N*/ 						GetBrowseWidth() + 2 * aBorder.Width() : 0;
/*N*/ //					if ( !pSh->VisArea().Width() )
/*N*/ //						nWidth = Max( nWidth, 5000L );
/*N*/ 					if ( nWidth < pSh->VisArea().Width() )
/*N*/ 						nWidth = pSh->VisArea().Width();
/*N*/ 					nWidth = Max( nWidth, 2L * aBorder.Width() + 4L*MM50 );
/*N*/ 					Frm().Width( nWidth );
/*N*/ 
/*N*/ 					SwLayoutFrm *pBody = FindBodyCont();
/*N*/ 					if ( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
/*N*/ 					{
/*N*/ 						//Fuer Spalten gilt eine feste Hoehe
/*N*/ 						Frm().Height( pAttrs->GetSize().Height() );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						//Fuer Seiten ohne Spalten bestimmt der Inhalt die
/*N*/ 						//Groesse.
/*N*/ 						long nBot = Frm().Top() + nTop;
/*N*/ 						SwFrm *pFrm = Lower();
/*N*/ 						while ( pFrm )
/*N*/ 						{
/*N*/ 							long nTmp = 0;
/*N*/ 							SwFrm *pCnt = ((SwLayoutFrm*)pFrm)->ContainsAny();
/*N*/ 							while ( pCnt && (pCnt->GetUpper() == pFrm ||
/*N*/ 											 ((SwLayoutFrm*)pFrm)->IsAnLower( pCnt )))
/*N*/ 							{
/*N*/ 								nTmp += pCnt->Frm().Height();
/*N*/ 								if( pCnt->IsTxtFrm() &&
/*N*/ 									((SwTxtFrm*)pCnt)->IsUndersized() )
/*N*/ 									nTmp += ((SwTxtFrm*)pCnt)->GetParHeight()
/*N*/ 											- pCnt->Prt().Height();
/*N*/ 								else if( pCnt->IsSctFrm() &&
/*N*/ 										 ((SwSectionFrm*)pCnt)->IsUndersized() )
/*N*/ 									nTmp += ((SwSectionFrm*)pCnt)->Undersize();
/*N*/ 								pCnt = pCnt->FindNext();
/*N*/ 							}
/*N*/                             // OD 29.10.2002 #97265# - consider invalid body frame properties
/*N*/                             if ( pFrm->IsBodyFrm() &&
/*N*/                                  ( !pFrm->GetValidSizeFlag() ||
/*N*/                                    !pFrm->GetValidPrtAreaFlag() ) &&
/*N*/                                  ( pFrm->Frm().Height() < pFrm->Prt().Height() )
/*N*/                                )
/*N*/                             {
/*N*/                                 nTmp = Min( nTmp, pFrm->Frm().Height() );
/*N*/                             }
/*N*/                             else
/*N*/                             {
/*N*/                                 // OD 30.10.2002 #97265# - assert invalid lower property
/*N*/                                 ASSERT( !(pFrm->Frm().Height() < pFrm->Prt().Height()),
/*N*/                                         "SwPageFrm::MakeAll(): Lower with frame height < printing height" );
/*N*/                                 nTmp += pFrm->Frm().Height() - pFrm->Prt().Height();
/*N*/                             }
/*N*/ 							if ( !pFrm->IsBodyFrm() )
/*N*/ 								nTmp = Min( nTmp, pFrm->Frm().Height() );
/*N*/ 							nBot += nTmp;
/*N*/ 							// Hier werden die absatzgebundenen Objekte ueberprueft,
/*N*/ 							// ob sie ueber den Body/FtnCont hinausragen.
/*N*/ 							if( pSortedObjs && !pFrm->IsHeaderFrm() &&
/*N*/ 								!pFrm->IsFooterFrm() )
/*N*/ 								lcl_CheckObjects( pSortedObjs, pFrm, nBot );
/*N*/ 							pFrm = pFrm->GetNext();
/*N*/ 						}
/*N*/ 						nBot += nBottom;
/*N*/ 						//Und die Seitengebundenen
/*N*/ 						if ( pSortedObjs )
/*N*/ 							lcl_CheckObjects( pSortedObjs, this, nBot );
/*N*/ 						nBot -= Frm().Top();
/*N*/ 						if ( !GetPrev() )
/*N*/ 							nBot = Max( nBot, pSh->VisArea().Height() );
/*N*/ 						Frm().Height( nBot );
/*N*/ 					}
/*N*/ 					Prt().Left ( pAttrs->CalcLeftLine() + aBorder.Width() );
/*N*/ 					Prt().Top  ( nTop );
/*N*/ 					Prt().Width( Frm().Width() - ( Prt().Left()
/*N*/ 						+ pAttrs->CalcRightLine() + aBorder.Width() ) );
/*N*/ 					Prt().Height( Frm().Height() - (nTop + nBottom) );
/*N*/ 					bValidSize = bValidPrtArea = TRUE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{	//FixSize einstellen, bei Seiten nicht vom Upper sondern vom
/*N*/ 					//Attribut vorgegeben.
/*N*/ 					Frm().SSize( pAttrs->GetSize() );
/*N*/ 					Format( pAttrs );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	} //while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	delete pAccess;
/*N*/ 	if ( Frm() != aOldRect )
/*N*/ 		AdjustRootSize( CHG_CHGPAGE, &aOldRect );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	//Der Upper (Root) muss mindestens so breit
/*N*/ 	//sein, dass er die breiteste Seite aufnehmen kann.
/*N*/ 	if ( GetUpper() )
/*N*/     {
/*N*/         ASSERT( GetUpper()->Prt().Width() >= aFrm.Width(), "Rootsize" );
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::MakeAll()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 28. Nov. 95
|*
|*************************************************************************/


/*N*/ void SwLayoutFrm::MakeAll()
/*N*/ {
/*N*/ 	PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )
/*N*/ 
/*N*/ 		//uebernimmt im DTor die Benachrichtigung
/*N*/ 	const SwLayNotify aNotify( this );
/*N*/     BOOL bVert = IsVertical();
/*N*/     SwRectFn fnRect = ( IsNeighbourFrm() == bVert )? fnRectHori : fnRectVert;
/*N*/ 
/*N*/ 	SwBorderAttrAccess *pAccess = 0;
/*N*/ 	const SwBorderAttrs*pAttrs = 0;
/*N*/ 
/*N*/ 	while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	{
/*N*/ 		if ( !bValidPos )
/*N*/ 			MakePos();
/*N*/ 
/*N*/ 		if ( GetUpper() )
/*N*/ 		{
/*N*/ 			if ( !bValidSize )
/*N*/ 			{
/*N*/ 				//FixSize einstellen, die VarSize wird von Format() nach
/*N*/ 				//Berechnung der PrtArea eingestellt.
/*N*/ 				bValidPrtArea = FALSE;
/*N*/                 SwTwips nPrtWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
/*N*/                 if( bVert && ( IsBodyFrm() || IsFtnContFrm() ) )
/*N*/                 {
/*N*/                     SwFrm* pNxt = GetPrev();
/*N*/                     while( pNxt && !pNxt->IsHeaderFrm() )
/*N*/                         pNxt = pNxt->GetPrev();
/*N*/                     if( pNxt )
/*N*/                         nPrtWidth -= pNxt->Frm().Height();
/*N*/                     pNxt = GetNext();
/*N*/                     while( pNxt && !pNxt->IsFooterFrm() )
/*N*/                         pNxt = pNxt->GetNext();
/*N*/                     if( pNxt )
/*N*/                         nPrtWidth -= pNxt->Frm().Height();
/*N*/                 }
/*N*/                 const long nDiff = nPrtWidth - (Frm().*fnRect->fnGetWidth)();
/*N*/                 if( IsNeighbourFrm() && IsRightToLeft() )
/*N*/                     (Frm().*fnRect->fnSubLeft)( nDiff );
/*N*/                 else
/*N*/                     (Frm().*fnRect->fnAddRight)( nDiff );
/*N*/ 			}
/*N*/ 			else
/*N*/             {   // Don't leave your upper
/*N*/                 const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/                 if( (Frm().*fnRect->fnOverStep)( nDeadLine ) )
/*N*/                     bValidSize = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bValidSize || !bValidPrtArea )
/*N*/ 		{
/*N*/ 			if ( !pAccess )
/*N*/ 			{
/*N*/ 				pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
/*N*/ 				pAttrs  = pAccess->Get();
/*N*/ 			}
/*N*/ 			Format( pAttrs );
/*N*/ 		}
/*N*/ 	} //while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	if ( pAccess )
/*N*/ 		delete pAccess;
/*N*/ }

/*************************************************************************
|*
|*	SwCntntFrm::MakePrtArea()
|*
|*	Ersterstellung		MA 17. Nov. 92
|*	Letzte Aenderung	MA 03. Mar. 96
|*
|*************************************************************************/

/*N*/ BOOL SwCntntFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
/*N*/ {
/*N*/ 	BOOL bSizeChgd = FALSE;
/*N*/ 
/*N*/ 	if ( !bValidPrtArea )
/*N*/ 	{
/*N*/ 		bValidPrtArea = TRUE;
/*N*/ 
/*N*/         SWRECTFN( this )
/*N*/ 		const FASTBOOL bTxtFrm = IsTxtFrm();
/*N*/ 		SwTwips nUpper = 0;
/*N*/ 		if ( bTxtFrm && ((SwTxtFrm*)this)->IsHiddenNow() )
/*N*/ 		{
/*N*/ 			if ( ((SwTxtFrm*)this)->HasFollow() )
/*N*/ 				((SwTxtFrm*)this)->JoinFrm();
/*N*/ 
/*N*/             if( (Prt().*fnRect->fnGetHeight)() )
/*N*/ 				((SwTxtFrm*)this)->HideHidden();
/*N*/             Prt().Pos().X() = Prt().Pos().Y() = 0;
/*N*/             (Prt().*fnRect->fnSetWidth)( (Frm().*fnRect->fnGetWidth)() );
/*N*/             (Prt().*fnRect->fnSetHeight)( 0 );
/*N*/             nUpper = -( (Frm().*fnRect->fnGetHeight)() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//Vereinfachung: CntntFrms sind immer in der Hoehe Variabel!
/*N*/ 
/*N*/ 			//An der FixSize gibt der umgebende Frame die Groesse vor, die
/*N*/ 			//Raender werden einfach abgezogen.
/*N*/             const long nLeft = rAttrs.CalcLeft( this );
/*N*/ #ifdef BIDI
/*N*/             const long nRight = ((SwBorderAttrs&)rAttrs).CalcRight( this );
/*N*/             (this->*fnRect->fnSetXMargins)( nLeft, nRight );
/*N*/ #else
/*N*/             (this->*fnRect->fnSetXMargins)( nLeft, rAttrs.CalcRight() );
/*N*/ #endif
/*N*/ 
/*N*/ 			ViewShell *pSh = GetShell();
/*N*/             SwTwips nWidthArea;
/*N*/             if( pSh && 0!=(nWidthArea=(pSh->VisArea().*fnRect->fnGetWidth)()) &&
/*N*/                 GetUpper()->IsPageBodyFrm() &&  // nicht dagegen bei BodyFrms in Columns
/*N*/                 pSh->GetDoc()->IsBrowseMode() )
/*N*/ 			{
/*N*/ 				//Nicht ueber die Kante des sichbaren Bereiches hinausragen.
/*N*/ 				//Die Seite kann breiter sein, weil es Objekte mit "ueberbreite"
/*N*/ 				//geben kann (RootFrm::ImplCalcBrowseWidth())
/*N*/ 				long nMinWidth = 0;
/*N*/ 
/*N*/ 				for (USHORT i = 0; GetDrawObjs() && i < GetDrawObjs()->Count();++i)
/*N*/ 				{
/*N*/ 					SdrObject *pObj = (*GetDrawObjs())[i];
/*N*/ 					SwFrmFmt *pFmt = ::binfilter::FindFrmFmt( pObj );
/*N*/ 					const FASTBOOL bFly = pObj->IsWriterFlyFrame();
/*N*/ 					if ( bFly &&
/*N*/ 						 WEIT_WECH == ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Frm().Width()||
/*N*/ 						 pFmt->GetFrmSize().GetWidthPercent() )
/*N*/ 						continue;
/*N*/ 
/*N*/ 					if ( FLY_IN_CNTNT == pFmt->GetAnchor().GetAnchorId() )
/*N*/ 						nMinWidth = Max( nMinWidth,
/*N*/ 									  bFly ? pFmt->GetFrmSize().GetWidth()
/*N*/ 										   : pObj->GetBoundRect().GetWidth() );
/*N*/ 				}
/*N*/ 
/*N*/ 				const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
/*N*/                 long nWidth = nWidthArea - 2 * ( IsVertical() ?
/*N*/                                            aBorder.Width() : aBorder.Height() );
/*N*/                 nWidth -= (Prt().*fnRect->fnGetLeft)();
/*N*/ 				nWidth -= rAttrs.CalcRightLine();
/*N*/ 				nWidth = Max( nMinWidth, nWidth );
/*N*/                 (Prt().*fnRect->fnSetWidth)( Min( nWidth,
/*N*/                                             (Prt().*fnRect->fnGetWidth)() ) );
/*N*/ 			}
/*N*/ 
/*N*/             if ( (Prt().*fnRect->fnGetWidth)() <= MINLAY )
/*N*/ 			{
/*N*/ 				//Die PrtArea sollte schon wenigstens MINLAY breit sein, passend
/*N*/ 				//zu den Minimalwerten des UI
/*N*/                 (Prt().*fnRect->fnSetWidth)( Min( long(MINLAY),
/*N*/                                              (Frm().*fnRect->fnGetWidth)() ) );
/*N*/                 SwTwips nTmp = (Frm().*fnRect->fnGetWidth)() -
/*N*/                                (Prt().*fnRect->fnGetWidth)();
/*N*/                 if( (Prt().*fnRect->fnGetLeft)() > nTmp )
/*N*/                     (Prt().*fnRect->fnSetLeft)( nTmp );
/*N*/ 			}
/*N*/ 
/*N*/ 			//Fuer die VarSize gelten folgende Regeln:
/*N*/ 			//1. Der erste einer Kette hat keinen Rand nach oben
/*N*/ 			//2. Nach unten gibt es nie einen Rand
/*N*/ 			//3. Der Rand nach oben ist das Maximum aus dem Abstand des
/*N*/ 			//	 Prev nach unten und dem eigenen Abstand nach oben.
/*N*/ 			//Die drei Regeln werden auf die Berechnung der Freiraeume, die von
/*N*/ 			//UL- bzw. LRSpace vorgegeben werden, angewand. Es gibt in alle
/*N*/ 			//Richtungen jedoch ggf. trotzdem einen Abstand; dieser wird durch
/*N*/ 			//Umrandung und/oder Schatten vorgegeben.
/*N*/ 			//4. Der Abstand fuer TextFrms entspricht mindestens dem Durchschuss
/*N*/ 
/*N*/ 			nUpper = CalcUpperSpace( &rAttrs, NULL );
/*N*/             // in balanced columned section frames we do not want the
/*N*/             // common border
/*N*/             sal_Bool bCommonBorder = sal_True;
/*N*/             if ( IsInSct() && GetUpper()->IsColBodyFrm() )
/*N*/             {
/*N*/                 const SwSectionFrm* pSct = FindSctFrm();
/*N*/                 bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
/*N*/             }
/*N*/             SwTwips nLower = bCommonBorder ?
/*N*/                              rAttrs.GetBottomLine( this ) :
/*N*/                              rAttrs.CalcBottomLine();
/*N*/ 
/*N*/             (Prt().*fnRect->fnSetPosY)( (!bVert || bReverse) ? nUpper : nLower);
/*N*/             nUpper += nLower;
/*N*/             nUpper -= (Frm().*fnRect->fnGetHeight)() -
/*N*/                       (Prt().*fnRect->fnGetHeight)();
/*N*/ 		}
/*N*/ 		//Wenn Unterschiede zwischen Alter und neuer Groesse,
/*N*/ 		//Grow() oder Shrink() rufen
/*N*/ 		if ( nUpper )
/*N*/ 		{
/*N*/ 			if ( nUpper > 0 )
/*N*/                 GrowFrm( nUpper );
/*N*/ 			else
/*N*/                 ShrinkFrm( -nUpper );
/*N*/ 			bSizeChgd = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bSizeChgd;
/*N*/ }

/*************************************************************************
|*
|*	SwCntntFrm::MakeAll()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 16. Dec. 96
|*
|*************************************************************************/

#define STOP_FLY_FORMAT 10

/*N*/ inline void ValidateSz( SwFrm *pFrm )
/*N*/ {
/*N*/ 	if ( pFrm )
/*N*/ 	{
/*N*/ 		pFrm->bValidSize = TRUE;
/*N*/ 		pFrm->bValidPrtArea = TRUE;
/*N*/ 	}
/*N*/ }


/*N*/ void SwCntntFrm::MakeAll()
/*N*/ {
/*N*/ 	ASSERT( GetUpper(), "keinen Upper?" );
/*N*/ 	ASSERT( IsTxtFrm(), "MakeAll(), NoTxt" );
/*N*/ 
/*N*/ 	if ( !IsFollow() && StackHack::IsLocked() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ( IsJoinLocked() )
/*N*/ 		return;
/*N*/ 
/*N*/     ASSERT( !((SwTxtFrm*)this)->IsSwapped(), "Calculation of a swapped frame" );
/*N*/ 
/*N*/     StackHack aHack;
/*N*/ 
/*N*/ 	if ( ((SwTxtFrm*)this)->IsLocked() )
/*N*/ 	{
/*N*/ 		ASSERT( FALSE, "Format fuer gelockten TxtFrm." );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	LockJoin();
/*N*/     long nFormatCount = 0;
/*N*/ 	PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	SwDoc *pDoc = GetAttrSet()->GetDoc();
/*N*/ 	if( pDoc )
/*N*/ 	{
/*N*/ 		static sal_Bool bWarn = sal_False;
/*N*/ 		if( pDoc->InXMLExport() )
/*N*/ 		{
/*N*/ 			ASSERT( bWarn, "Formatting during XML-export!" );
/*N*/ 			bWarn = sal_True;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bWarn = sal_False;
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	//uebernimmt im DTor die Benachrichtigung
/*N*/ 	SwCntntNotify *pNotify = new SwCntntNotify( this );
/*N*/ 
/*N*/ 	BOOL	bMakePage	= TRUE;		//solange TRUE kann eine neue Seite
/*N*/ 									//angelegt werden (genau einmal)
/*N*/ 	BOOL 	bMovedBwd	= FALSE;	//Wird TRUE wenn der Frame zurueckfliesst
/*N*/ 	BOOL	bMovedFwd	= FALSE;	//solange FALSE kann der Frm zurueck-
/*N*/ 									//fliessen (solange, bis er einmal
/*N*/ 									//vorwaerts ge'moved wurde).
/*N*/ 	BOOL	bFormatted	= FALSE;	//Fuer die Witwen und Waisen Regelung
/*N*/ 									//wird der letzte CntntFrm einer Kette
/*N*/ 									//u.U. zum Formatieren angeregt, dies
/*N*/ 									//braucht nur einmal zu passieren.
/*N*/ 									//Immer wenn der Frm gemoved wird muss
/*N*/ 									//das Flag zurueckgesetzt werden.
/*N*/ 	BOOL	bMustFit	= FALSE;	//Wenn einmal die Notbremse gezogen wurde,
/*N*/ 									//werden keine anderen Prepares mehr
/*N*/ 									//abgesetzt.
/*N*/ 	BOOL	bFitPromise = FALSE;	//Wenn ein Absatz nicht passte, mit WouldFit
/*N*/ 									//aber verspricht, dass er sich passend
/*N*/ 									//einstellt wird dieses Flag gesetzt.
/*N*/ 									//Wenn er dann sein Versprechen nicht haelt,
/*N*/ 									//kann kontrolliert verfahren werden.
/*N*/ 	BOOL bMoveable;
/*N*/ 	const BOOL bFly	= IsInFly();
/*N*/ 	const BOOL bTab = IsInTab();
/*N*/ 	const BOOL bFtn = IsInFtn();
/*N*/ 	const BOOL bSct = IsInSct();
/*N*/ 	Point aOldFrmPos;				//Damit bei Turnarounds jew. mit der
/*N*/ 	Point aOldPrtPos;				//letzten Pos verglichen und geprueft
/*N*/ 									//werden kann, ob ein Prepare sinnvoll ist.
/*N*/ 
/*N*/ 	SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
/*N*/ 	const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 
/*N*/     const BOOL bKeep = IsKeep( rAttrs );
/*N*/ 
/*N*/ 	SwSaveFtnHeight *pSaveFtn = 0;
/*N*/ 	if ( bFtn )
/*N*/ 	{
/*N*/ 		SwFtnFrm *pFtn = FindFtnFrm();
/*N*/ 		SwSectionFrm* pSct = pFtn->FindSctFrm();
/*N*/ 		if ( !((SwTxtFrm*)pFtn->GetRef())->IsLocked() )
/*N*/ 		{
/*N*/ 			SwFtnBossFrm* pBoss = pFtn->GetRef()->FindFtnBossFrm(
/*N*/ 									pFtn->GetAttr()->GetFtn().IsEndNote() );
/*N*/ 			if( !pSct || pSct->IsColLocked() || !pSct->Growable() )
/*N*/ 				pSaveFtn = new SwSaveFtnHeight( pBoss,
/*N*/ 					((SwTxtFrm*)pFtn->GetRef())->GetFtnLine( pFtn->GetAttr(),
/*N*/ 												   pFtn->IsBackMoveLocked() ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Wenn ein Follow neben seinem Master steht und nicht passt, kann er
/*N*/ 	//gleich verschoben werden.
/*N*/ 	if( lcl_Prev( this ) && ((SwTxtFrm*)this)->IsFollow() && IsMoveable() )
/*N*/ 	{
/*N*/ 		bMovedFwd = TRUE;
/*N*/ 		MoveFwd( bMakePage, FALSE );
/*N*/ 	}
/*N*/ 
/*N*/     // OD 08.11.2002 #104840# - check footnote content for forward move.
/*N*/     // If a content of a footnote is on a prior page/column as its invalid
/*N*/     // reference, it can be moved forward.
/*N*/     if ( bFtn && !bValidPos )
/*N*/     {
/*N*/         SwFtnFrm* pFtn = FindFtnFrm();
/*N*/         SwCntntFrm* pRefCnt = pFtn ? pFtn->GetRef() : 0;
/*N*/         if ( pRefCnt && !pRefCnt->IsValid() )
/*N*/         {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     SWRECTFN( this )
/*N*/ 
/*N*/ 	while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	{
/*N*/ 		if ( TRUE == (bMoveable = IsMoveable()) )
/*N*/ 		{
/*N*/ 			SwFrm *pPre = GetIndPrev();
/*N*/ 			if ( CheckMoveFwd( bMakePage, bKeep, bMovedBwd ) )
/*N*/ 			{
/*N*/                 SWREFRESHFN( this )
/*N*/ 				bMovedFwd = TRUE;
/*N*/ 				if ( bMovedBwd )
/*N*/ 				{
/*N*/ 					//Beim zurueckfliessen wurde der Upper angeregt sich
/*N*/ 					//vollstaendig zu Painten, dass koennen wir uns jetzt
/*N*/ 					//nach dem hin und her fliessen sparen.
/*N*/ 					GetUpper()->ResetCompletePaint();
/*N*/ 					//Der Vorgaenger wurde Invalidiert, das ist jetzt auch obsolete.
/*N*/ 					ASSERT( pPre, "missing old Prev" );
/*N*/ 					if( !pPre->IsSctFrm() )
/*N*/ 						::binfilter::ValidateSz( pPre );
/*N*/ 				}
/*N*/ 				bMoveable = IsMoveable();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/         aOldFrmPos = (Frm().*fnRect->fnGetPos)();
/*N*/         aOldPrtPos = (Prt().*fnRect->fnGetPos)();
/*N*/ 
/*N*/ 		if ( !bValidPos )
/*N*/ 			MakePos();
/*N*/ 
/*N*/ 		//FixSize einstellen, die VarSize wird von Format() justiert.
/*N*/ 		if ( !bValidSize )
/*N*/             (Frm().*fnRect->fnSetWidth)( (GetUpper()->
/*N*/                                          Prt().*fnRect->fnGetWidth)() );
/*N*/ 		if ( !bValidPrtArea )
/*N*/ 		{
/*N*/             const long nOldW = (Prt().*fnRect->fnGetWidth)();
/*N*/ 			MakePrtArea( rAttrs );
/*N*/             if ( nOldW != (Prt().*fnRect->fnGetWidth)() )
/*N*/ 				Prepare( PREP_FIXSIZE_CHG );
/*N*/ 		}
/*N*/ 
/*N*/         if ( aOldFrmPos != (Frm().*fnRect->fnGetPos)() )
/*N*/             CalcFlys( TRUE );
/*N*/ 		//Damit die Witwen- und Waisen-Regelung eine Change bekommt muss der
/*N*/ 		//CntntFrm benachrichtigt werden.
/*N*/ 		//Kriterium:
/*N*/ 		//- Er muss Moveable sein (sonst mach das Spalten keinen Sinn.)
/*N*/ 		//- Er muss mit der Unterkante der PrtArea des Upper ueberlappen.
/*N*/ 		if ( !bMustFit )
/*N*/ 		{
/*N*/ 			BOOL bWidow = TRUE;
/*N*/             const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/             if ( bMoveable && !bFormatted && ( GetFollow() ||
/*N*/                  ( (Frm().*fnRect->fnOverStep)( nDeadLine ) ) ) )
/*N*/             {
/*N*/ 				Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
/*N*/ 				bValidSize = bWidow = FALSE;
/*N*/ 			}
/*N*/             if( (Frm().*fnRect->fnGetPos)() != aOldFrmPos ||
/*N*/                 (Prt().*fnRect->fnGetPos)() != aOldPrtPos )
/*N*/ 			{
/*N*/ 				// In diesem Prepare erfolgt ggf. ein _InvalidateSize().
/*N*/ 				// bValidSize wird FALSE und das Format() wird gerufen.
/*N*/ 				Prepare( PREP_POS_CHGD, (const void*)&bFormatted, FALSE );
/*N*/ 				if ( bWidow && GetFollow() )
/*N*/ 				{	Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
/*N*/ 					bValidSize = FALSE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bValidSize )
/*N*/         {
/*N*/             bValidSize = bFormatted = TRUE;
/*N*/             ++nFormatCount;
/*N*/             if( nFormatCount > STOP_FLY_FORMAT )
/*N*/                 SetFlyLock( TRUE );
/*N*/ 			Format();
/*N*/ 		}
/*N*/ 
        // FME 16.07.2003 #i16930# - removed this code because it did not work

        // OD 04.04.2003 #108446# - react on the situation detected in the text
        // formatting - see <SwTxtFrm::FormatAdjust(..)>:
        // text frame has to move forward, because its text formatting stopped,
        // created a follow and detected, that it contains no content.
/*        if ( IsTxtFrm() && bValidPos && bValidSize && bValidPrtArea &&
             (Frm().*fnRect->fnGetHeight)() == 0 &&
             HasFollow()
           )
        {
            SwFrm* pOldUpper = GetUpper();
            MoveFwd( TRUE, FALSE );
            if ( GetUpper() != pOldUpper )
            {
                bMovedFwd = TRUE;
                SWREFRESHFN( this )
                continue;
            }
        } */
/*N*/ 
/*N*/         //Wenn ich der erste einer Kette bin koennte ich mal sehen ob
/*N*/ 		//ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
/*N*/ 		//Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
/*N*/ 		//geflossen sein.
/*N*/ 		BOOL bDummy;
/*N*/ 		if ( !lcl_Prev( this ) && !bMovedFwd && (bMoveable || (bFly && !bTab)) &&
/*N*/ 			 (!bFtn || !GetUpper()->FindFtnFrm()->GetPrev()) && MoveBwd( bDummy ))
/*N*/ 		{
/*N*/             SWREFRESHFN( this )
/*N*/ 			bMovedBwd = TRUE;
/*N*/ 			bFormatted = FALSE;
/*N*/             if ( bKeep && bMoveable )
/*N*/ 			{
/*N*/ 				if( CheckMoveFwd( bMakePage, FALSE, bMovedBwd ) )
/*N*/ 				{
/*N*/ 					bMovedFwd = TRUE;
/*N*/ 					bMoveable = IsMoveable();
/*N*/                     SWREFRESHFN( this )
/*N*/ 				}
/*N*/                 Point aOldPos = (Frm().*fnRect->fnGetPos)();
/*N*/                 MakePos();
/*N*/                 if( aOldPos != (Frm().*fnRect->fnGetPos)() )
/*N*/ 				{
/*N*/ 					CalcFlys( TRUE );
/*N*/ 					Prepare( PREP_POS_CHGD, (const void*)&bFormatted, FALSE );
/*N*/ 					if ( !bValidSize )
/*N*/ 					{
/*N*/                         (Frm().*fnRect->fnSetWidth)( (GetUpper()->
/*N*/                                                 Prt().*fnRect->fnGetWidth)() );
/*N*/ 						if ( !bValidPrtArea )
/*N*/ 						{
/*N*/                             const long nOldW = (Prt().*fnRect->fnGetWidth)();
/*N*/                             MakePrtArea( rAttrs );
/*N*/                             if( nOldW != (Prt().*fnRect->fnGetWidth)() )
/*N*/ 								Prepare( PREP_FIXSIZE_CHG, 0, FALSE );
/*N*/ 						}
/*N*/ 						if( GetFollow() )
/*N*/ 							Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
/*N*/ 						bValidSize = bFormatted = TRUE;
/*N*/ 						Format();
/*N*/ 					}
/*N*/ 				}
/*N*/                 SwFrm *pNxt = HasFollow() ? NULL : FindNext();
/*N*/ 				while( pNxt && pNxt->IsSctFrm() )
/*N*/ 				{   // Leere Bereiche auslassen, in die anderen hinein
/*N*/ 					if( ((SwSectionFrm*)pNxt)->GetSection() )
/*N*/ 					{
/*N*/ 						SwFrm* pTmp = ((SwSectionFrm*)pNxt)->ContainsAny();
/*N*/ 						if( pTmp )
/*N*/ 						{
/*N*/ 							pNxt = pTmp;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					pNxt = pNxt->FindNext();
/*N*/ 				}
/*N*/                 if ( pNxt )
/*N*/ 				{
/*N*/ 					pNxt->Calc();
/*N*/ 					if( bValidPos && !GetIndNext() )
/*N*/ 					{
/*N*/ 						SwSectionFrm *pSct = FindSctFrm();
/*N*/ 						if( pSct && !pSct->GetValidSizeFlag() )
/*N*/ 						{
/*N*/ 							SwSectionFrm* pNxtSct = pNxt->FindSctFrm();
/*N*/ 							if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
/*N*/ 								bValidPos = FALSE;
/*N*/ 						}
/*N*/ 						else
/*N*/ 							bValidPos = FALSE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Der TxtFrm Validiert sich bei Fussnoten ggf. selbst, dass kann leicht
/*N*/ 		//dazu fuehren, dass seine Position obwohl unrichtig valide ist.
/*N*/ 		if ( bValidPos )
/*N*/ 		{
/*N*/ 			if ( bFtn )
/*N*/             {
/*N*/                 bValidPos = FALSE;
/*N*/ 				MakePos();
/*N*/                 aOldFrmPos = (Frm().*fnRect->fnGetPos)();
/*N*/                 aOldPrtPos = (Prt().*fnRect->fnGetPos)();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Wieder ein Wert ungueltig? - dann nochmal das ganze...
/*N*/ 		if ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		//Fertig?
/*N*/ 		// Achtung, wg. Hoehe==0, ist es besser statt Bottom() Top()+Height() zu nehmen
/*N*/         // (kommt bei Undersized TxtFrms an der Unterkante eines spaltigen Bereichs vor)
/*N*/         if( (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)() )
/*N*/             >= 0 )
/*N*/ 		{
/*N*/             if ( bKeep && bMoveable )
/*N*/ 			{
/*N*/ 				//Wir sorgen dafuer, dass der Nachfolger gleich mit formatiert
/*N*/ 				//wird. Dadurch halten wir das Heft in der Hand, bis wirklich
/*N*/ 				//(fast) alles stabil ist. So vermeiden wir Endlosschleifen,
/*N*/ 				//die durch staendig wiederholte Versuche entstehen.
/*N*/ 				//Das bMoveFwdInvalid ist fuer #38407# notwendig. War urspruenglich
/*N*/ 				//in flowfrm.cxx rev 1.38 behoben, das unterbrach aber obiges
/*N*/ 				//Schema und spielte lieber Tuerme von Hanoi (#43669#).
/*N*/                 SwFrm *pNxt = HasFollow() ? NULL : FindNext();
/*N*/ 				// Bei Bereichen nehmen wir lieber den Inhalt, denn nur
/*N*/ 				// dieser kann ggf. die Seite wechseln
/*N*/ 				while( pNxt && pNxt->IsSctFrm() )
/*N*/ 				{
/*N*/ 					if( ((SwSectionFrm*)pNxt)->GetSection() )
/*N*/ 					{
/*N*/ 						pNxt = ((SwSectionFrm*)pNxt)->ContainsAny();
/*N*/ 						break;
/*N*/ 					}
/*N*/ 					pNxt = pNxt->FindNext();
/*N*/ 				}
/*N*/ 				if ( pNxt )
/*N*/ 				{
/*N*/ 					const FASTBOOL bMoveFwdInvalid = 0 != GetIndNext();
/*N*/                     const FASTBOOL bNxtNew =
/*N*/                         ( 0 == (pNxt->Prt().*fnRect->fnGetHeight)() ) &&
/*N*/                         (!pNxt->IsTxtFrm() ||!((SwTxtFrm*)pNxt)->IsHiddenNow());
/*N*/ 					pNxt->Calc();
/*N*/ 					if ( !bMovedBwd &&
/*N*/ 						 ((bMoveFwdInvalid && !GetIndNext()) ||
/*N*/ 						  bNxtNew) )
/*N*/                     {
/*N*/                         if( bMovedFwd )
/*N*/                             pNotify->SetInvaKeep();
/*N*/ 						bMovedFwd = FALSE;
/*N*/                     }
/*N*/ 				}
/*N*/ 			}
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 
/*N*/ 		//Ich passe nicht mehr in meinen Uebergeordneten, also ist es jetzt
/*N*/ 		//an der Zeit moeglichst konstruktive Veranderungen vorzunehmen
/*N*/ 
/*N*/ 		//Wenn ich den uebergeordneten Frm nicht verlassen darf, habe
/*N*/ 		//ich ein Problem; Frei nach Artur Dent tun wir das einzige das man
/*N*/ 		//mit einen nicht loesbaren Problem tun kann: wir ignorieren es - und
/*N*/ 		//zwar mit aller Kraft.
/*N*/ 		if ( !bMoveable || IsUndersized() )
/*N*/ 		{
/*N*/ 			if( !bMoveable && IsInTab() )
/*N*/ 			{
/*N*/                 long nDiff = -(Frm().*fnRect->fnBottomDist)(
/*N*/                                         (GetUpper()->*fnRect->fnGetPrtBottom)() );
/*N*/                 long nReal = GetUpper()->Grow( nDiff PHEIGHT );
/*N*/ 				if( nReal )
/*N*/ 					continue;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		//Wenn ich nun ueberhaupt ganz und garnicht in meinen Upper passe
/*N*/ 		//so kann die Situation vielleicht doch noch durch Aufbrechen
/*N*/ 		//aufgeklart werden. Diese Situation tritt bei einem frisch
/*N*/ 		//erzeugten Follow auf, der zwar auf die Folgeseite geschoben wurde
/*N*/ 		//aber selbst noch zu gross fuer diese ist; also wiederum
/*N*/ 		//aufgespalten werden muss.
/*N*/ 		//Wenn ich nicht passe und nicht Spaltbar (WouldFit()) bin, so schicke
/*N*/ 		//ich meinem TxtFrmanteil die Nachricht, dass eben falls moeglich
/*N*/ 		//trotz des Attributes 'nicht aufspalten' aufgespalten werden muss.
/*N*/ 		BOOL bMoveOrFit = FALSE;
/*N*/ 		BOOL bDontMoveMe = !GetIndPrev();
/*N*/ 		if( bDontMoveMe && IsInSct() )
/*N*/ 		{
/*N*/ 			SwFtnBossFrm* pBoss = FindFtnBossFrm();
/*N*/ 			bDontMoveMe = !pBoss->IsInSct() ||
/*N*/ 						  ( !pBoss->Lower()->GetNext() && !pBoss->GetPrev() );
/*N*/ 		}
/*N*/ 
/*N*/         if ( bDontMoveMe && (Frm().*fnRect->fnGetHeight)() >
/*N*/                             (GetUpper()->Prt().*fnRect->fnGetHeight)() )
/*N*/ 		{
/*N*/             if ( !bFitPromise )
/*N*/ 			{
/*N*/                 SwTwips nTmp = (GetUpper()->Prt().*fnRect->fnGetHeight)() -
/*N*/                                (Prt().*fnRect->fnGetTop)();
/*N*/ 				BOOL bSplit = !GetIndPrev();
/*N*/ 				if ( nTmp > 0 && WouldFit( nTmp, bSplit ) )
/*N*/ 				{
/*N*/ 					Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
/*N*/ 					bValidSize = FALSE;
/*N*/ 					bFitPromise = TRUE;
/*N*/ 					continue;
/*N*/ 				}
                /* -----------------19.02.99 12:58-------------------
                 * Frueher wurde in Rahmen und Bereichen niemals versucht,
                 * durch bMoveOrFit den TxtFrm unter Verzicht auf seine
                 * Attribute (Widows,Keep) doch noch passend zu bekommen.
                 * Dies haette zumindest bei spaltigen Rahmen versucht
                 * werden muessen, spaetestens bei verketteten Rahmen und
                 * in Bereichen muss es versucht werden.
                 * Ausnahme: Wenn wir im FormatWidthCols stehen, duerfen die
                 * Attribute nicht ausser Acht gelassen werden.
                 * --------------------------------------------------*/
/*N*/ 				else if ( !bFtn && bMoveable &&
/*N*/ 					  ( !bFly || !FindFlyFrm()->IsColLocked() ) &&
/*N*/ 					  ( !bSct || !FindSctFrm()->IsColLocked() ) )
/*N*/ 					bMoveOrFit = TRUE;
/*N*/             }
/*N*/ #ifdef DBG_UTIL
/*N*/ 			else
/*N*/ 			{
/*N*/ 				ASSERT( FALSE, "+TxtFrm hat WouldFit-Versprechen nicht eingehalten." );
/*N*/ 			}
/*N*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		//Mal sehen ob ich irgenwo Platz finde...
/*N*/ 		//Benachbarte Fussnoten werden in _MoveFtnCntFwd 'vorgeschoben'
/*N*/ 		SwFrm *pPre = GetIndPrev();
/*N*/ 		SwFrm *pOldUp = GetUpper();

/* MA 13. Oct. 98: Was soll das denn sein!?
 * AMA 14. Dec 98: Wenn ein spaltiger Bereich keinen Platz mehr fuer seinen ersten ContentFrm
 * 		bietet, so soll dieser nicht nur in die naechste Spalte, sondern ggf. bis zur naechsten
 * 		Seite wandern und dort einen Section-Follow erzeugen.
 */
/*N*/ 		if( IsInSct() && bMovedFwd && bMakePage && pOldUp->IsColBodyFrm() &&
/*N*/ 			pOldUp->GetUpper()->GetUpper()->IsSctFrm() &&
/*N*/ 			( pPre || pOldUp->GetUpper()->GetPrev() ) &&
/*N*/ 			((SwSectionFrm*)pOldUp->GetUpper()->GetUpper())->MoveAllowed(this) )
/*N*/ 			bMovedFwd = FALSE;
/*N*/ 
/*N*/         const sal_Bool bCheckForGrownBody = pOldUp->IsBodyFrm();
/*N*/         const long nOldBodyHeight = (pOldUp->Frm().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/ 		if ( !bMovedFwd && !MoveFwd( bMakePage, FALSE ) )
/*N*/ 			bMakePage = FALSE;
/*N*/         SWREFRESHFN( this )
/*N*/ 
/*N*/         // If MoveFwd moves the paragraph to the next page, a following
/*N*/         // paragraph, which contains footnotes can can cause the old upper
/*N*/         // frame to grow. In this case we explicitely allow a new check
/*N*/         // for MoveBwd. Robust: We also check the bMovedBwd flag again.
/*N*/         // If pOldUp was a footnote frame, it has been deleted inside MoveFwd.
/*N*/         // Therefore we only check for growing body frames.
/*N*/         if ( bCheckForGrownBody && ! bMovedBwd && pOldUp != GetUpper() &&
/*N*/              (pOldUp->Frm().*fnRect->fnGetHeight)() > nOldBodyHeight )
/*N*/             bMovedFwd = FALSE;
/*N*/         else
/*N*/             bMovedFwd = TRUE;
/*N*/ 
/*N*/ 		bFormatted = FALSE;
/*N*/ 		if ( bMoveOrFit && GetUpper() == pOldUp )
/*N*/ 		{
/*N*/ 			Prepare( PREP_MUST_FIT, 0, FALSE );
/*N*/ 			bValidSize = FALSE;
/*N*/ 			bMustFit = TRUE;
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 		if ( bMovedBwd && GetUpper() )
/*N*/ 		{	//Unuetz gewordene Invalidierungen zuruecknehmen.
/*N*/ 			GetUpper()->ResetCompletePaint();
/*N*/ 			if( pPre && !pPre->IsSctFrm() )
/*N*/ 				::binfilter::ValidateSz( pPre );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( bValidPos && bValidSize && bValidPrtArea && GetDrawObjs() &&
/*N*/ 			 Prt().SSize() != pNotify->Prt().SSize() )
/*N*/ 		{
/*N*/ 			//Wenn sich meine PrtArea in der Groesse verandert hat, so ist die
/*N*/ 			//automatische Ausrichtung der Flys zum Teufel. Diese muss
/*N*/ 			//Waehrend der Fahrt korrigiert werden, weil sie mich ggf. wiederum
/*N*/ 			//invalidiert.
/*N*/ 			SwDrawObjs &rObjs = *GetDrawObjs();
/*N*/ 			for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pO = rObjs[i];
/*N*/ 				if ( pO->IsWriterFlyFrame() )
/*N*/ 					((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->InvalidatePos();
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 	} //while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 
/*N*/ 	if ( pSaveFtn )
/*N*/ 		delete pSaveFtn;
/*N*/ 
/*N*/ 	UnlockJoin();
/*N*/ 	if ( bMovedFwd || bMovedBwd )
/*N*/ 		pNotify->SetInvaKeep();
/*N*/ 	delete pNotify;
/*N*/     SetFlyLock( FALSE );
/*N*/ }

/*************************************************************************
|*
|*	SwCntntFrm::_WouldFit()
|*
|*	Ersterstellung		MA 28. Feb. 95
|*	Letzte Aenderung	AMA 15. Feb. 99
|*
|*************************************************************************/




/*N*/ void MakeNxt( SwFrm *pFrm, SwFrm *pNxt )
/*N*/ {
/*N*/ 	//fix(25455): Validieren, sonst kommt es zu einer Rekursion.
/*N*/ 	//Der erste Versuch, der Abbruch mit pFrm = 0 wenn !Valid,
/*N*/ 	//fuehrt leider zu dem Problem, dass das Keep dann u.U. nicht mehr
/*N*/ 	//korrekt beachtet wird (27417)
/*N*/ 	const BOOL bOldPos = pFrm->GetValidPosFlag();
/*N*/ 	const BOOL bOldSz  = pFrm->GetValidSizeFlag();
/*N*/ 	const BOOL bOldPrt = pFrm->GetValidPrtAreaFlag();
/*N*/ 	pFrm->bValidPos = pFrm->bValidPrtArea = pFrm->bValidSize = TRUE;
/*N*/ 
/*N*/ 	//fix(29272): Nicht MakeAll rufen, dort wird evtl. pFrm wieder invalidert
/*N*/ 	//und kommt rekursiv wieder herein.
/*N*/ 	if ( pNxt->IsCntntFrm() )
/*N*/ 	{
/*N*/ 		SwCntntNotify aNotify( (SwCntntFrm*)pNxt );
/*N*/ 		SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
/*N*/ 		const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 		if ( !pNxt->GetValidSizeFlag() )
/*N*/         {
/*N*/             if( pNxt->IsVertical() )
/*N*/                 pNxt->Frm().Height( pNxt->GetUpper()->Prt().Height() );
/*N*/             else
/*N*/                 pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
/*N*/         }
/*N*/ 		((SwCntntFrm*)pNxt)->MakePrtArea( rAttrs );
/*N*/ 		pNxt->Format( &rAttrs );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwLayNotify aNotify( (SwLayoutFrm*)pNxt );
/*N*/ 		SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
/*N*/ 		const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 		if ( !pNxt->GetValidSizeFlag() )
/*N*/         {
/*N*/             if( pNxt->IsVertical() )
/*N*/                 pNxt->Frm().Height( pNxt->GetUpper()->Prt().Height() );
/*N*/             else
/*N*/                 pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
/*N*/         }
/*N*/ 		pNxt->Format( &rAttrs );
/*N*/ 	}
/*N*/ 
/*N*/ 	pFrm->bValidPos  	 = bOldPos;
/*N*/ 	pFrm->bValidSize 	 = bOldSz;
/*N*/ 	pFrm->bValidPrtArea  = bOldPrt;
/*N*/ }

// Diese Routine ueberprueft, ob zwischen dem FtnBoss von pFrm und dem
// von pNxt keine anderen FtnBosse liegen


/*N*/ BOOL SwCntntFrm::_WouldFit( SwTwips nSpace, SwLayoutFrm *pNewUpper, BOOL bTstMove )
/*N*/ {
/*N*/ 	//Damit die Fussnote sich ihren Platz sorgsam waehlt, muss
/*N*/ 	//sie in jedem Fall gemoved werden, wenn zwischen dem
/*N*/ 	//neuen Upper und ihrer aktuellen Seite/Spalte mindestens eine
/*N*/ 	//Seite/Spalte liegt.
/*N*/ 	SwFtnFrm* pFtnFrm = 0;
/*N*/ 	if ( IsInFtn() )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bRet;
/*N*/ 	BOOL bSplit = !pNewUpper->Lower();
/*N*/ 	SwCntntFrm *pFrm = this;
/*N*/ 	const SwFrm *pPrev = pNewUpper->Lower();
/*N*/ 	if( pPrev && pPrev->IsFtnFrm() )
/*N*/ 		pPrev = ((SwFtnFrm*)pPrev)->Lower();
/*N*/ 	while ( pPrev && pPrev->GetNext() )
/*N*/ 		pPrev = pPrev->GetNext();
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if ( bTstMove || IsInFly() || ( IsInSct() &&
/*N*/ 			 ( pFrm->GetUpper()->IsColBodyFrm() || ( pFtnFrm &&
/*N*/ 			   pFtnFrm->GetUpper()->GetUpper()->IsColumnFrm() ) ) ) )
/*N*/ 		{
/*N*/ 			//Jetzt wirds ein bischen hinterlistig; empfindliche Gemueter sollten
/*N*/ 			//lieber wegsehen. Wenn ein Flys Spalten enthaelt so sind die Cntnts
/*N*/ 			//moveable, mit Ausnahme der in der letzten Spalte (siehe
/*N*/ 			//SwFrm::IsMoveable()). Zurueckfliessen duerfen sie aber natuerlich.
/*N*/ 			//Das WouldFit() liefert leider nur dann einen vernueftigen Wert, wenn
/*N*/ 			//der Frm moveable ist. Um dem WouldFit() einen Moveable Frm
/*N*/ 			//vorzugaukeln haenge ich ihn einfach solange um.
/*N*/ 			// Auch bei spaltigen Bereichen muss umgehaengt werden, damit
/*N*/ 			// SwSectionFrm::Growable() den richtigen Wert liefert.
/*N*/ 			// Innerhalb von Fussnoten muss ggf. sogar der SwFtnFrm umgehaengt werden,
/*N*/ 			// falls es dort keinen SwFtnFrm gibt.
/*N*/ 			SwFrm* pTmpFrm = pFrm->IsInFtn() && !pNewUpper->FindFtnFrm() ?
/*N*/ 							 (SwFrm*)pFrm->FindFtnFrm() : pFrm;
/*N*/ 			SwLayoutFrm *pUp = pTmpFrm->GetUpper();
/*N*/ 			SwFrm *pOldNext = pTmpFrm->GetNext();
/*N*/ 			pTmpFrm->Remove();
/*N*/ 			pTmpFrm->InsertBefore( pNewUpper, 0 );
/*N*/             if ( pFrm->IsTxtFrm() &&
/*N*/                  ( bTstMove ||
/*N*/                    ((SwTxtFrm*)pFrm)->HasFollow() ||
/*N*/                    ( !((SwTxtFrm*)pFrm)->HasPara() &&
/*N*/                      !((SwTxtFrm*)pFrm)->IsEmpty()
/*N*/                    )
/*N*/                  )
/*N*/                )
/*N*/ 			{
/*N*/ 				bTstMove = TRUE;
/*N*/ 				bRet = ((SwTxtFrm*)pFrm)->TestFormat( pPrev, nSpace, bSplit );
/*N*/ 			}
/*N*/ 			else
/*?*/ 				bRet = pFrm->WouldFit( nSpace, bSplit );
/*N*/ 			pTmpFrm->Remove();
/*N*/ 			pTmpFrm->InsertBefore( pUp, pOldNext );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bRet = pFrm->WouldFit( nSpace, bSplit );
/*N*/ 
/*N*/         SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
/*N*/         const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 
/*N*/ 		//Bitter aber wahr: Der Abstand muss auch noch mit einkalkuliert werden.
/*N*/ 		//Bei TestFormatierung ist dies bereits geschehen.
/*N*/ 		if ( bRet && !bTstMove )
/*N*/ 		{
/*N*/ 			SwTwips nUpper;
/*N*/ 			if ( pPrev )
/*N*/ 			{
/*N*/ 				nUpper = CalcUpperSpace( NULL, pPrev );
/*N*/ 
/*N*/                 // in balanced columned section frames we do not want the
/*N*/                 // common border
/*N*/                 sal_Bool bCommonBorder = sal_True;
/*N*/                 if ( pFrm->IsInSct() && pFrm->GetUpper()->IsColBodyFrm() )
/*N*/                 {
/*?*/                     const SwSectionFrm* pSct = pFrm->FindSctFrm();
/*?*/                     bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
/*N*/                 }
/*N*/                 nUpper += bCommonBorder ?
/*N*/                           rAttrs.GetBottomLine( pFrm ) :
/*N*/                           rAttrs.CalcBottomLine();
/*N*/ 			}
/*N*/ 			else
/*N*/             {
/*N*/                 if( pFrm->IsVertical() )
/*?*/                     nUpper = pFrm->Frm().Width() - pFrm->Prt().Width();
/*N*/                 else
/*N*/                     nUpper = pFrm->Frm().Height() - pFrm->Prt().Height();
/*N*/             }
/*N*/ 			nSpace -= nUpper;
/*N*/ 			if ( nSpace < 0 )
/*N*/ 				bRet = FALSE;
/*N*/ 		}
/*N*/ 
/*N*/       if ( bRet && !bSplit && pFrm->IsKeep( rAttrs ) )
/*N*/ 		{
/*N*/ 			if( bTstMove )
/*N*/             {
/*?*/ 				while( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->HasFollow() )
/*?*/                 {
/*?*/ 					pFrm = ((SwTxtFrm*)pFrm)->GetFollow();
/*?*/                 }
/*?*/                 // OD 11.04.2003 #108824# - If last follow frame of <this> text
/*?*/                 // frame isn't valid, a formatting of the next content frame
/*?*/                 // doesn't makes sense. Thus, return TRUE.
/*?*/                 if ( IsAnFollow( pFrm ) && !pFrm->IsValid() )
/*?*/                 {
/*?*/                     ASSERT( false, "Only a warning for task 108824:/n<SwCntntFrm::_WouldFit(..) - follow not valid!" );
/*?*/                     return TRUE;
/*?*/                 }
/*N*/             }
/*N*/ 			SwFrm *pNxt;
/*N*/ 			if( 0 != (pNxt = pFrm->FindNext()) && pNxt->IsCntntFrm() &&
/*N*/ 				( !pFtnFrm || ( pNxt->IsInFtn() &&
/*N*/ 				  pNxt->FindFtnFrm()->GetAttr() == pFtnFrm->GetAttr() ) ) )
/*N*/ 			{
/*N*/ 				// ProbeFormatierung vertraegt keine absatz- oder gar zeichengebundene Objekte
/*N*/ 				if( bTstMove && pNxt->GetDrawObjs() )
/*N*/ 					return TRUE;
/*N*/ 
/*N*/ 				if ( !pNxt->IsValid() )
/*N*/ 					MakeNxt( pFrm, pNxt );
/*N*/ 
/*N*/ 				//Kleiner Trick: Wenn der naechste einen Vorgaenger hat, so hat
/*N*/ 				//er den Absatzabstand bereits berechnet. Er braucht dann nicht
/*N*/ 				//teuer kalkuliert werden.
/*N*/ 				if( lcl_NotHiddenPrev( pNxt ) )
/*N*/ 					pPrev = 0;
/*N*/ 				else
/*N*/ 				{
/*?*/ 					if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow() )
/*?*/ 						pPrev = lcl_NotHiddenPrev( pFrm );
/*?*/ 					else
/*?*/ 						pPrev = pFrm;
/*N*/ 				}
/*N*/ 				pFrm = (SwCntntFrm*)pNxt;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pFrm = 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFrm = 0;
/*N*/ 
/*N*/ 	} while ( bRet && pFrm );
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }





}
