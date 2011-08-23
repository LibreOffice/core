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

#include "viewsh.hxx"

#include <horiornt.hxx>

#include "doc.hxx"
#include "pagefrm.hxx"

#include <txtftn.hxx>
#include <flyfrm.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <dflyobj.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/charrotateitem.hxx>
#include <com/sun/star/i18n/ScriptType.hdl>

#include "txtcfg.hxx"
#include "porftn.hxx"
#include "porfly.hxx"
#include "itrform2.hxx"
#include "frmsh.hxx"
#include "ftnfrm.hxx"	// FindErgoSumFrm(), FindQuoVadisFrm(),
#include "pagedesc.hxx"
#include "sectfrm.hxx"	// SwSectionFrm
namespace binfilter {

using namespace ::com::sun::star;

extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

/*************************************************************************
 *								_IsFtnNumFrm()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrm::_IsFtnNumFrm() const
/*N*/ {
/*N*/ 	const SwFtnFrm* pFtn = FindFtnFrm()->GetMaster();
/*N*/ 	while( pFtn && !pFtn->ContainsCntnt() )
/*?*/ 		pFtn = pFtn->GetMaster();
/*N*/ 	return !pFtn;
/*N*/ }

/*************************************************************************
 *								FindFtn()
 *************************************************************************/

// Sucht innerhalb einer Master-Follow-Kette den richtigen TxtFrm zum SwTxtFtn

/*N*/ SwTxtFrm *SwTxtFrm::FindFtnRef( const SwTxtFtn *pFtn )
/*N*/ {
/*N*/ 	SwTxtFrm *pFrm = this;
/*N*/ 	const sal_Bool bFwd = *pFtn->GetStart() >= GetOfst();
/*N*/ 	while( pFrm )
/*N*/ 	{
/*N*/ 		if( SwFtnBossFrm::FindFtn( pFrm, pFtn ) )
/*N*/ 			return pFrm;
/*N*/ 		pFrm = bFwd ? pFrm->GetFollow() :
/*N*/ 					  pFrm->IsFollow() ? pFrm->FindMaster() : 0;
/*N*/ 	}
/*N*/ 	return pFrm;
/*N*/ }

/*************************************************************************
 *								CalcFtnFlag()
 *************************************************************************/

/*N*/ #ifndef DBG_UTIL
/*N*/ void SwTxtFrm::CalcFtnFlag()
/*N*/ #else
/*N*/ void SwTxtFrm::CalcFtnFlag( xub_StrLen nStop )//Fuer den Test von SplitFrm
/*N*/ #endif
/*N*/ {
/*N*/ 	bFtn = sal_False;
/*N*/ 
/*N*/ 	const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
/*N*/ 	if( !pHints )
/*N*/ 		return;
/*N*/ 
/*N*/ 	const MSHORT nSize = pHints->Count();
/*N*/ 
/*N*/ #ifndef DBG_UTIL
/*N*/ 	const xub_StrLen nEnd = GetFollow() ? GetFollow()->GetOfst() : STRING_LEN;
/*N*/ #else
/*N*/ 	const xub_StrLen nEnd = nStop != STRING_LEN ? nStop
/*N*/ 						: GetFollow() ? GetFollow()->GetOfst() : STRING_LEN;
/*N*/ #endif
/*N*/ 
/*N*/ 	for( MSHORT i = 0; i < nSize; ++i )
/*N*/ 	{
/*N*/ 		const SwTxtAttr *pHt = (*pHints)[i];
/*N*/ 		if ( pHt->Which() == RES_TXTATR_FTN )
/*N*/ 		{
/*N*/ 			const xub_StrLen nIdx = *pHt->GetStart();
/*N*/ 			if ( nEnd < nIdx )
/*N*/ 				break;
/*N*/ 			if( GetOfst() <= nIdx )
/*N*/ 			{
/*N*/ 				bFtn = sal_True;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *								CalcPrepFtnAdjust()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrm::CalcPrepFtnAdjust()
/*N*/ {
/*N*/ 	ASSERT( HasFtn(), "Wer ruft mich da?" );
/*N*/ 	SwFtnBossFrm *pBoss = FindFtnBossFrm( sal_True );
/*N*/ 	const SwFtnFrm *pFtn = pBoss->FindFirstFtn( this );
/*N*/ 	if( pFtn && FTNPOS_CHAPTER != GetNode()->GetDoc()->GetFtnInfo().ePos &&
/*N*/ 		( !pBoss->GetUpper()->IsSctFrm() ||
/*N*/ 		!((SwSectionFrm*)pBoss->GetUpper())->IsFtnAtEnd() ) )
/*N*/ 	{
/*N*/ 		const SwFtnContFrm *pCont = pBoss->FindFtnCont();
/*N*/ 		sal_Bool bReArrange = sal_True;
/*N*/ 
/*N*/         SWRECTFN( this )
/*N*/         if ( pCont && (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(),
/*N*/                                           (Frm().*fnRect->fnGetBottom)() ) > 0 )
/*N*/ 		{
/*N*/             pBoss->RearrangeFtns( (Frm().*fnRect->fnGetBottom)(), sal_False,
/*N*/ 								  pFtn->GetAttr() );
/*N*/ 			ValidateBodyFrm();
/*N*/ 			ValidateFrm();
/*N*/ 			pFtn = pBoss->FindFirstFtn( this );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bReArrange = sal_False;
/*N*/ 		if( !pCont || !pFtn || bReArrange != (pFtn->FindFtnBossFrm() == pBoss) )
/*N*/ 		{
/*?*/ 			SwTxtFormatInfo aInf( this );
/*?*/ 			SwTxtFormatter aLine( this, &aInf );
/*?*/ 			aLine.TruncLines();
/*?*/ 			SetPara( 0 );		//Wird ggf. geloescht!
/*?*/ 			ResetPreps();
/*?*/ 			return sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::GetFtnLine()
 *************************************************************************/

/*N*/ SwTwips SwTxtFrm::GetFtnLine( const SwTxtFtn *pFtn, sal_Bool bLocked ) const
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(),
/*N*/             "SwTxtFrm::GetFtnLine with swapped frame" )
/*N*/ 
/*N*/ 	SwTxtFrm *pThis = (SwTxtFrm*)this;
/*N*/ 
/*N*/ 	if( !HasPara() )
/*N*/ 	{
/*N*/ 		// Es sieht ein wenig gehackt aus, aber man riskiert einen Haufen
/*N*/ 		// Aerger, wenn man versucht, per pThis->GetFormatted() doch
/*N*/ 		// noch an den richtigen Wert heranzukommen. Durch das PREP_ADJUST
/*N*/ 		// stellen wir sicher, dass wir noch einmal drankommen, dann aber
/*N*/ 		// von der Ref aus!
/*N*/ 		// Trotzdem wollen wir nichts unversucht lassen und geben die
/*N*/ 		// Unterkante des Frames zurueck.
/*?*/ 		if( !bLocked )
/*?*/ 			pThis->Prepare( PREP_ADJUST_FRM );
/*?*/         return IsVertical() ? Frm().Left() : Frm().Bottom();
/*N*/ 	}
/*N*/ 
/*N*/     SWAP_IF_NOT_SWAPPED( this )
/*N*/ 
/*N*/ 	SwTxtInfo aInf( pThis );
/*N*/ 	SwTxtIter aLine( pThis, &aInf );
/*N*/ 	const xub_StrLen nPos = *pFtn->GetStart();
/*N*/ 	aLine.CharToLine( nPos );
/*N*/ 
/*N*/     SwTwips nRet = aLine.Y() + SwTwips(aLine.GetLineHeight());
/*N*/     if( IsVertical() )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         nRet = SwitchHorizontalToVertical( nRet );
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ 
/*N*/     return nRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::GetFtnRstHeight()
 *************************************************************************/

// Ermittelt die max. erreichbare Hoehe des TxtFrm im Ftn-Bereich.
// Sie wird eingeschraenkt durch den unteren Rand der Zeile mit
// der Ftn-Referenz.

/*N*/ SwTwips SwTxtFrm::_GetFtnFrmHeight() const
/*N*/ {
/*N*/ 	ASSERT( !IsFollow() && IsInFtn(), "SwTxtFrm::SetFtnLine: moon walk" );
/*N*/ 
/*N*/ 	const SwFtnFrm *pFtnFrm = FindFtnFrm();
/*N*/ 	const SwTxtFrm *pRef = (const SwTxtFrm *)pFtnFrm->GetRef();
/*N*/ 	const SwFtnBossFrm *pBoss = FindFtnBossFrm();
/*N*/ 	if( pBoss != pRef->FindFtnBossFrm( !pFtnFrm->GetAttr()->
/*N*/ 										GetFtn().IsEndNote() ) )
/*N*/ 		return 0;
/*N*/ 
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/ 
/*N*/ 	SwTwips nHeight = pRef->IsInFtnConnect() ?
/*N*/ 							1 : pRef->GetFtnLine( pFtnFrm->GetAttr(), sal_False );
/*N*/ 	if( nHeight )
/*N*/ 	{
/*N*/ 		// So komisch es aussehen mag: Die erste Ftn auf der Seite darf sich
/*N*/ 		// nicht mit der Ftn-Referenz beruehren, wenn wir im Ftn-Bereich Text
/*N*/ 		// eingeben.
/*N*/ 		const SwFrm *pCont = pFtnFrm->GetUpper();
/*N*/ 		//Hoehe innerhalb des Cont, die ich mir 'eh noch genehmigen darf.
/*N*/         SWRECTFN( pCont )
/*N*/         SwTwips nTmp = (*fnRect->fnYDiff)( (pCont->*fnRect->fnGetPrtBottom)(),
/*N*/                                            (Frm().*fnRect->fnGetTop)() );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if( nTmp < 0 )
/*N*/ 		{
/*?*/ 			sal_Bool bInvalidPos = sal_False;
/*?*/             const SwLayoutFrm* pTmp = GetUpper();
/*?*/ 			while( !bInvalidPos && pTmp )
/*?*/ 			{
/*?*/                 bInvalidPos = !pTmp->GetValidPosFlag() ||
/*?*/                                !pTmp->Lower()->GetValidPosFlag();
/*?*/ 				if( pTmp == pCont )
/*?*/ 					break;
/*?*/ 				pTmp = pTmp->GetUpper();
/*?*/ 			}
/*?*/ 			ASSERT( bInvalidPos, "Hanging below FtnCont" );
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/         if ( (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(), nHeight) > 0 )
/*N*/ 		{
/*N*/ 			//Wachstumspotential den Containers.
/*N*/ 			if ( !pRef->IsInFtnConnect() )
/*N*/ 			{
/*N*/ 				SwSaveFtnHeight aSave( (SwFtnBossFrm*)pBoss, nHeight  );
/*N*/                 nHeight = ((SwFtnContFrm*)pCont)->Grow( LONG_MAX PHEIGHT, sal_True );
/*N*/ 			}
/*N*/ 			else
/*N*/                 nHeight = ((SwFtnContFrm*)pCont)->Grow( LONG_MAX PHEIGHT, sal_True );
/*N*/ 
/*N*/ 			nHeight += nTmp;
/*N*/ 			if( nHeight < 0 )
/*N*/ 				nHeight = 0;
/*N*/ 		}
/*N*/ 		else
/*?*/ 		{   // The container has to shrink
/*?*/             nTmp += (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(), nHeight);
/*?*/ 			if( nTmp > 0 )
/*?*/ 				nHeight = nTmp;
/*?*/ 			else
/*?*/ 				nHeight = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ 
/*N*/ 	return nHeight;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::FindErgoSumFrm()
 *************************************************************************/


/*************************************************************************
 *						SwTxtFrm::FindQuoVadisFrm()
 *************************************************************************/

/*N*/ SwTxtFrm *SwTxtFrm::FindQuoVadisFrm()
/*N*/ {
/*N*/ 	// Erstmal feststellen, ob wir in einem FtnFrm stehen:
/*N*/ 	if( GetIndPrev() || !IsInFtn() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	// Zum Vorgaenger-FtnFrm
/*N*/ 	SwFtnFrm *pFtnFrm = FindFtnFrm()->GetMaster();
/*N*/ 	if( !pFtnFrm )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	// Nun den letzten Cntnt:
/*?*/ 	const SwCntntFrm *pCnt = pFtnFrm->ContainsCntnt();
/*?*/ 	if( !pCnt )
/*?*/ 		return NULL;
/*?*/ 	const SwCntntFrm *pLast;
/*?*/ 	do
/*?*/ 	{	pLast = pCnt;
/*?*/ 		pCnt = pCnt->GetNextCntntFrm();
/*?*/ 	} while( pCnt && pFtnFrm->IsAnLower( pCnt ) );
/*?*/ 	return (SwTxtFrm*)pLast;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::SetErgoSumNum()
 *************************************************************************/


/*************************************************************************
 *						SwTxtFrm::RemoveFtn()
 *************************************************************************/

/*N*/ void SwTxtFrm::RemoveFtn( const xub_StrLen nStart, const xub_StrLen nLen )
/*N*/ {
/*N*/ 	if ( !IsFtnAllowed() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	SwpHints *pHints = GetTxtNode()->GetpSwpHints();
/*N*/ 	if( !pHints )
/*N*/ 		return;
/*N*/ 
/*N*/ 	sal_Bool bRollBack = nLen != STRING_LEN;
/*N*/ 	MSHORT nSize = pHints->Count();
/*N*/ 	xub_StrLen nEnd;
/*N*/ 	SwTxtFrm* pSource;
/*N*/ 	if( bRollBack )
/*N*/ 	{
/*?*/ 		nEnd = nStart + nLen;
/*?*/ 		pSource = GetFollow();
/*?*/ 		if( !pSource )
/*?*/ 			return;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nEnd = STRING_LEN;
/*N*/ 		pSource = this;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nSize )
/*N*/ 	{
/*N*/ 		SwPageFrm* pUpdate = NULL;
/*N*/ 		sal_Bool bRemove = sal_False;
/*N*/ 		SwFtnBossFrm *pFtnBoss = 0;
/*N*/ 		SwFtnBossFrm *pEndBoss = 0;
/*N*/ 		sal_Bool bFtnEndDoc
/*N*/ 			= FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFtnInfo().ePos;
/*N*/ 		for( MSHORT i = nSize; i; )
/*N*/ 		{
/*N*/ 			SwTxtAttr *pHt = pHints->GetHt(--i);
/*N*/ 			if ( RES_TXTATR_FTN != pHt->Which() )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			const xub_StrLen nIdx = *pHt->GetStart();
/*N*/ 			if( nStart > nIdx )
/*N*/ 				break;
/*N*/ 
/*?*/ 			if( nEnd >= nIdx )
/*?*/ 			{
/*?*/ 				SwTxtFtn *pFtn = (SwTxtFtn*)pHt;
/*?*/ 				sal_Bool bEndn = pFtn->GetFtn().IsEndNote();
/*?*/ 
/*?*/ 				if( bEndn )
/*?*/ 				{
/*?*/ 					if( !pEndBoss )
/*?*/ 						pEndBoss = pSource->FindFtnBossFrm();
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					if( !pFtnBoss )
/*?*/ 					{
/*?*/ 						pFtnBoss = pSource->FindFtnBossFrm( sal_True );
/*?*/ 						if( pFtnBoss->GetUpper()->IsSctFrm() )
/*?*/ 						{
/*?*/ 							SwSectionFrm* pSect = (SwSectionFrm*)
/*?*/ 												  pFtnBoss->GetUpper();
/*?*/ 							if( pSect->IsFtnAtEnd() )
/*?*/ 								bFtnEndDoc = sal_False;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 				// Wir loeschen nicht, sondern wollen die Ftn verschieben.
/*?*/ 				// Drei Faelle koennen auftreten:
/*?*/ 				// 1) Es gibt weder Follow noch PrevFollow
/*?*/ 				//	  -> RemoveFtn()  (vielleicht sogar ein ASSERT wert)
/*?*/ 				// 2) nStart > GetOfst, ich habe einen Follow
/*?*/ 				//	  -> Ftn wandert in den Follow
/*?*/ 				// 3) nStart < GetOfst, ich bin ein Follow
/*?*/ 				//	  -> Ftn wandert in den PrevFollow
/*?*/ 				// beide muessen auf einer Seite/in einer Spalte stehen.
/*?*/ 
/*?*/ 				SwFtnFrm *pFtnFrm = bEndn ? pEndBoss->FindFtn( pSource, pFtn ) :
/*?*/ 											pFtnBoss->FindFtn( pSource, pFtn );
/*?*/ 
/*?*/ 				if( pFtnFrm )
/*?*/ 				{
/*?*/ 					const sal_Bool bEndDoc = bEndn ? sal_True : bFtnEndDoc;
/*?*/ 					if( bRollBack )
/*?*/ 					{
/*?*/ 						while ( pFtnFrm )
/*?*/ 						{
/*?*/ 							pFtnFrm->SetRef( this );
/*?*/ 							pFtnFrm = pFtnFrm->GetFollow();
/*?*/ 							SetFtn( sal_True );
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else if( GetFollow() )
                            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 					{
/*?*/ 					else
/*?*/ 					{
/*?*/                         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( !bEndDoc || ( bEndn && pEndBoss->IsInSct() &&
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		if( pUpdate )
/*?*/ 			pUpdate->UpdateFtnNum();
/*N*/ 		// Wir bringen die Oszillation zum stehen:
/*N*/ 		if( bRemove && !bFtnEndDoc && HasPara() )
/*N*/ 		{
/*?*/ 			ValidateBodyFrm();
/*?*/ 			ValidateFrm();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Folgendes Problem: Aus dem FindBreak heraus wird das RemoveFtn aufgerufen,
/*N*/ 	// weil die letzte Zeile an den Follow abgegeben werden soll. Der Offset
/*N*/ 	// des Follows ist aber veraltet, er wird demnaechst gesetzt. CalcFntFlag ist
/*N*/ 	// auf einen richtigen Follow-Offset angewiesen. Deshalb wird hier kurzfristig
/*N*/ 	// der Follow-Offset manipuliert.
/*N*/ 	xub_StrLen nOldOfst = STRING_LEN;
/*N*/ 	if( HasFollow() && nStart > GetOfst() )
/*N*/ 	{
/*N*/ 		nOldOfst = GetFollow()->GetOfst();
/*N*/ 		GetFollow()->ManipOfst( nStart + ( bRollBack ? nLen : 0 ) );
/*N*/ 	}
/*N*/ 	pSource->CalcFtnFlag();
/*N*/ 	if( nOldOfst < STRING_LEN )
/*N*/ 		GetFollow()->ManipOfst( nOldOfst );
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::ConnectFtn()
 *************************************************************************/
// sal_False, wenn irgendetwas schief gegangen ist.
// Es gibt eigentlich nur zwei Moeglichkeiten:
// a) Die Ftn ist bereits vorhanden
// => dann wird sie gemoved, wenn ein anderer pSrcFrm gefunden wurde
// b) Die Ftn ist nicht vorhanden
// => dann wird sie fuer uns angelegt.
// Ob die Ftn schliesslich auf unserer Spalte/Seite landet oder nicht,
// spielt in diesem Zusammenhang keine Rolle.
// Optimierungen bei Endnoten.
// Noch ein Problem: wenn die Deadline im Ftn-Bereich liegt, muss die
// Ftn verschoben werden.

/*N*/ void SwTxtFrm::ConnectFtn( SwTxtFtn *pFtn, const SwTwips nDeadLine )
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(),
/*N*/             "SwTxtFrm::ConnectFtn with swapped frame" );
/*N*/ 
/*N*/ 	bFtn = sal_True;
/*N*/ 	bInFtnConnect = sal_True;	//Bloss zuruecksetzen!
/*N*/ 	sal_Bool bEnd = pFtn->GetFtn().IsEndNote();
/*N*/ 
/*N*/ 	// Wir brauchen immer einen Boss (Spalte/Seite)
/*N*/ 	SwSectionFrm *pSect;
/*N*/ 	SwCntntFrm *pCntnt = this;
/*N*/ 	if( bEnd && IsInSct() )
/*N*/ 	{
/*?*/ 		pSect = FindSctFrm();
/*?*/ 		if( pSect->IsEndnAtEnd() )
/*?*/ 			pCntnt = pSect->FindLastCntnt( FINDMODE_ENDNOTE );
/*?*/ 		if( !pCntnt )
/*?*/ 			pCntnt = this;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwFtnBossFrm *pBoss = pCntnt->FindFtnBossFrm( !bEnd );
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	SwTwips nRstHeight = GetRstHeight();
/*N*/ #endif
/*N*/ 
/*N*/ 	pSect = pBoss->FindSctFrm();
/*N*/ 	sal_Bool bDocEnd = bEnd ? !( pSect && pSect->IsEndnAtEnd() ) :
/*N*/ 				   ( !( pSect && pSect->IsFtnAtEnd() ) &&
/*N*/ 		  			 FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFtnInfo().ePos );
/*N*/ 	//Ftn kann beim Follow angemeldet sein.
/*N*/ 	SwCntntFrm *pSrcFrm = FindFtnRef( pFtn );
/*N*/ 
/*N*/ 	if( bDocEnd )
/*N*/ 	{
/*?*/ 		if( pSect && pSrcFrm )
/*?*/ 		{
/*?*/ 			SwFtnFrm *pFtnFrm = pBoss->FindFtn( pSrcFrm, pFtn );
/*?*/ 			if( pFtnFrm && pFtnFrm->IsInSct() )
/*?*/ 			{
/*?*/ 				pBoss->RemoveFtn( pSrcFrm, pFtn );
/*?*/ 				pSrcFrm = 0;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else if( bEnd && pSect )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwFtnFrm *pFtnFrm = pSrcFrm ? pBoss->FindFtn( pSrcFrm, pFtn ) : NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bDocEnd || bEnd )
/*N*/ 	{
/*?*/ 		if( !pSrcFrm )
/*?*/ 			pBoss->AppendFtn( this, pFtn );
/*?*/ 		else if( pSrcFrm != this )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
/*?*/ 		bInFtnConnect = sal_False;
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwSaveFtnHeight aHeight( pBoss, nDeadLine );
/*N*/ 
/*N*/ 	if( !pSrcFrm )		// Es wurde ueberhaupt keine Ftn gefunden.
/*N*/ 		pBoss->AppendFtn( this, pFtn );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwFtnFrm *pFtnFrm = pBoss->FindFtn( pSrcFrm, pFtn );
/*N*/ 		SwFtnBossFrm *pFtnBoss = pFtnFrm->FindFtnBossFrm();
/*N*/ 
/*N*/ 		sal_Bool bBrutal = sal_False;
/*N*/ 
/*N*/ 		if( pFtnBoss == pBoss )	// Ref und Ftn sind auf der selben Seite/Spalte.
/*N*/ 		{
/*N*/ 			SwFrm *pCont = pFtnFrm->GetUpper();
/*N*/ 
/*N*/             SWRECTFN ( pCont )
/*N*/             long nDiff = (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(),
/*N*/                                              nDeadLine );
/*N*/ 
/*N*/             if( nDiff >= 0 )
/*N*/ 			{
/*N*/ 				//Wenn die Fussnote bei einem Follow angemeldet ist, so ist
/*N*/ 				//es jetzt an der Zeit sie umzumelden.
/*N*/ 				if ( pSrcFrm != this )
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
/*N*/ 				//Es steht Platz zur Verfuegung, also kann die Fussnote evtl.
/*N*/ 				//wachsen.
/*N*/                 if ( pFtnFrm->GetFollow() && nDiff > 0 )
/*N*/ 				{
/*?*/                     SwTwips nHeight = (pCont->Frm().*fnRect->fnGetHeight)();
/*?*/ 					pBoss->RearrangeFtns( nDeadLine, sal_False, pFtn );
/*?*/ 					ValidateBodyFrm();
/*?*/ 					ValidateFrm();
/*?*/ 					ViewShell *pSh = GetShell();
/*?*/                     if ( pSh && nHeight == (pCont->Frm().*fnRect->fnGetHeight)() )
/*?*/ 						//Damit uns nix durch die Lappen geht.
/*?*/ 						pSh->InvalidateWindows( pCont->Frm() );
/*?*/ 				}
/*?*/ 				bInFtnConnect = sal_False;
/*?*/ 				return;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bBrutal = sal_True;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Ref und Ftn sind nicht auf einer Seite, Move-Versuch ist noetig.
/*N*/ 			SwFrm* pTmp = this;
/*N*/ 			while( pTmp->GetNext() && pSrcFrm != pTmp )
/*?*/ 				pTmp = pTmp->GetNext();
/*N*/ 			if( pSrcFrm == pTmp )
/*N*/ 				bBrutal = sal_True;
/*N*/ 			else
/*N*/ 			{   // Wenn unser Boss in einem spaltigen Bereich sitzt, es aber auf
/*N*/ 				// der Seite schon einen FtnContainer gibt, hilft nur die brutale
/*N*/ 				// Methode
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				if( pSect && pSect->FindFtnBossFrm( !bEnd )->FindFtnCont() )
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Die brutale Loesung: Fussnote entfernen und appenden.
/*N*/ 		// Es muss SetFtnDeadLine() gerufen werden, weil nach
/*N*/ 		// RemoveFtn die nMaxFtnHeight evtl. besser auf unsere Wuensche
/*N*/ 		// eingestellt werden kann.
/*N*/ 		if( bBrutal )
/*N*/ 		{
/*N*/ 			pBoss->RemoveFtn( pSrcFrm, pFtn, sal_False );
/*N*/ 			SwSaveFtnHeight *pHeight = bEnd ? NULL :
/*N*/ 				new SwSaveFtnHeight( pBoss, nDeadLine );
/*N*/ 			pBoss->AppendFtn( this, pFtn );
/*N*/ 			delete pHeight;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// In spaltigen Bereichen, die noch nicht bis zum Seitenrand gehen,
/*N*/ 	// ist kein RearrangeFtns sinnvoll, da der Fussnotencontainer noch
/*N*/ 	// nicht kalkuliert worden ist.
/*N*/ 	if( !pSect || !pSect->Growable() )
/*N*/ 	{
/*N*/ 		// Umgebung validieren, um Oszillationen zu verhindern.
/*N*/ 		SwSaveFtnHeight aNochmal( pBoss, nDeadLine );
/*N*/ 		ValidateBodyFrm();
/*N*/ 		pBoss->RearrangeFtns( nDeadLine, sal_True );
/*N*/ 		ValidateFrm();
/*N*/ 	}
/*N*/ 	else if( pSect->IsFtnAtEnd() )
/*N*/ 	{
/*?*/ 		ValidateBodyFrm();
/*?*/ 		ValidateFrm();
/*N*/ 	}
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	// pFtnFrm kann sich durch Calc veraendert haben ...
/*N*/ 	SwFtnFrm *pFtnFrm = pBoss->FindFtn( this, pFtn );
/*N*/ 	if( pFtnFrm && pBoss != pFtnFrm->FindFtnBossFrm( !bEnd ) )
/*N*/ 	{
/*N*/ 		int bla = 5;
/*N*/ 	}
/*N*/ 	nRstHeight = GetRstHeight();
/*N*/ #endif
/*N*/ 	bInFtnConnect = sal_False;
/*N*/ 	return;
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::NewFtnPortion()
 *************************************************************************/

// Die Portion fuer die Ftn-Referenz im Text
/*N*/ SwFtnPortion *SwTxtFormatter::NewFtnPortion( SwTxtFormatInfo &rInf,
/*N*/ 											 SwTxtAttr *pHint )
/*N*/ {
/*N*/     ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
/*N*/             "NewFtnPortion with unswapped frame" );
/*N*/ 
/*N*/ 	if( !pFrm->IsFtnAllowed() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	SwTxtFtn  *pFtn = (SwTxtFtn*)pHint;
/*N*/ 	SwFmtFtn& rFtn = (SwFmtFtn&)pFtn->GetFtn();
/*N*/ 	SwDoc *pDoc = pFrm->GetNode()->GetDoc();
/*N*/ 
/*N*/ 	if( rInf.IsTest() )
/*?*/ 		return new SwFtnPortion( rFtn.GetViewNumStr( *pDoc ), pFrm, pFtn );
/*N*/ 
/*N*/     SWAP_IF_SWAPPED( pFrm )
/*N*/ 
/*N*/ 	KSHORT nReal;
/*N*/ 	{
/*N*/ 		KSHORT nOldReal = pCurr->GetRealHeight();
/*N*/ 		KSHORT nOldAscent = pCurr->GetAscent();
/*N*/ 		KSHORT nOldHeight = pCurr->Height();
/*N*/ 		((SwTxtFormatter*)this)->CalcRealHeight();
/*N*/ 		nReal = pCurr->GetRealHeight();
/*N*/ 		if( nReal < nOldReal )
/*N*/ 			nReal = nOldReal;
/*N*/ 		pCurr->SetRealHeight( nOldReal );
/*N*/ 		pCurr->Height( nOldHeight );
/*N*/ 		pCurr->SetAscent( nOldAscent );
/*N*/ 	}
/*N*/ 
/*N*/ 	SwTwips nLower = Y() + nReal;
/*N*/ 
/*N*/     SWRECTFN( pFrm )
/*N*/ 
/*N*/     if( bVert )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         nLower = pFrm->SwitchHorizontalToVertical( nLower );
/*N*/ 
/*N*/     SwTwips nAdd;
/*N*/ 
/*N*/     if( pFrm->IsInTab() && (!pFrm->IsInSct() || pFrm->FindSctFrm()->IsInTab()) )
/*N*/ 	{
/*?*/ 		SwFrm *pRow = pFrm;
/*?*/ 		while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
/*?*/ 			pRow = pRow->GetUpper();
/*?*/ 
/*?*/         const SwTwips nMin = (pRow->Frm().*fnRect->fnGetBottom)();
/*?*/ 
/*?*/         if( ( ! bVert && nMin > nLower ) || ( bVert && nMin < nLower ) )
/*?*/ 			nLower = nMin;
/*?*/ 
/*?*/         nAdd = (pRow->GetUpper()->*fnRect->fnGetBottomMargin)();
/*N*/ 	}
/*N*/ 	else
/*N*/         nAdd = (pFrm->*fnRect->fnGetBottomMargin)();
/*N*/ 
/*N*/ 	if( nAdd > 0 )
/*N*/     {
/*N*/         if ( bVert )
/*N*/             nLower -= nAdd;
/*N*/         else
/*N*/             nLower += nAdd;
/*N*/     }
/*N*/ 			
/*N*/     // #i10770#: If there are fly frames anchored at previous paragraphs,
/*N*/     // the deadline should consider their lower borders.
/*N*/     SwFrm* pStartFrm = pFrm->GetUpper()->GetLower();
/*N*/     ASSERT( pStartFrm, "Upper has no lower" )
/*N*/     SwTwips nFlyLower = bVert ? LONG_MAX : 0;
/*N*/     while ( pStartFrm != pFrm )
/*N*/     {
/*N*/         ASSERT( pStartFrm, "Frame chain is broken" )
/*N*/         if ( pStartFrm->GetDrawObjs() )
/*N*/         {
/*N*/             const SwDrawObjs &rObjs = *pStartFrm->GetDrawObjs();
/*N*/             for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/             {
/*N*/                 SdrObject *pO = rObjs[i];
/*N*/                 SwRect aRect( pO->GetBoundRect() );
/*N*/ 
/*N*/                 if ( ! pO->IsWriterFlyFrame() ||
/*N*/                      ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->IsValid() )
/*N*/                 {
/*N*/                     const SwTwips nBottom = (aRect.*fnRect->fnGetBottom)();
/*N*/                     if ( (*fnRect->fnYDiff)( nBottom, nFlyLower ) > 0 )
/*N*/                         nFlyLower = nBottom;
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         pStartFrm = pStartFrm->GetNext();
/*N*/     }
/*N*/ 
/*N*/     if ( bVert )
/*N*/         nLower = Min( nLower, nFlyLower );
/*N*/     else
/*N*/         nLower = Max( nLower, nFlyLower );
/*N*/ 
/*N*/ 		//6995: Wir frischen nur auf. Das Connect tut fuer diesen Fall nix
/*N*/ 		//Brauchbares, sondern wuerde stattdessen fuer diesen Fall meist die
/*N*/ 		//Ftn wegwerfen und neu erzeugen.
/*N*/ 
/*N*/ 	if( !rInf.IsQuick() )
/*N*/ 		pFrm->ConnectFtn( pFtn, nLower );
/*N*/ 
/*N*/ 	SwTxtFrm *pScrFrm = pFrm->FindFtnRef( pFtn );
/*N*/ 	SwFtnBossFrm *pBoss = pFrm->FindFtnBossFrm( !rFtn.IsEndNote() );
/*N*/ 	SwFtnFrm *pFtnFrm = NULL;
/*N*/ 	if( pScrFrm )
/*N*/ 	{
/*N*/ 		pFtnFrm = pBoss->FindFtn( pScrFrm, pFtn );
/*N*/ 		if( pFtnFrm && pFtnFrm->Lower() )
/*N*/ 		{
/*N*/ 			SwTxtFrm *pTxtFrm = NULL;
/*N*/ 			if(	pFtnFrm->Lower()->IsTxtFrm() )
/*N*/ 				pTxtFrm = (SwTxtFrm*)pFtnFrm->Lower();
/*N*/ 			else if( pFtnFrm->Lower()->IsSctFrm() )
/*N*/ 			{
/*?*/ 				SwFrm* pCntnt = ((SwSectionFrm*)pFtnFrm->Lower())->ContainsCntnt();
/*?*/ 				if( pCntnt && pCntnt->IsTxtFrm() )
/*?*/ 					pTxtFrm = (SwTxtFrm*)pCntnt;
/*N*/ 			}
/*N*/ 			if ( pTxtFrm && pTxtFrm->HasPara() )
/*N*/ 			{
/*N*/ 				SwParaPortion *pPara = pTxtFrm->GetPara();
/*N*/ 				SwLinePortion *pTmp = pPara->GetPortion();
/*N*/ 				while( pTmp )
/*N*/ 				{
/*N*/ 					if( pTmp->IsFtnNumPortion() )
/*N*/ 					{
/*N*/ 						SeekAndChg( rInf );
/*N*/ 						if( ((SwFtnNumPortion*)pTmp)->DiffFont( rInf.GetFont() ) )
/*N*/ 							pTxtFrm->Prepare(PREP_FTN);
/*N*/ 						break;
/*N*/ 					}
/*?*/ 					pTmp = pTmp->GetPortion();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Wir erkundigen uns, ob durch unser Append irgendeine
/*N*/ 	// Fussnote noch auf der Seite/Spalte steht. Wenn nicht verschwindet
/*N*/ 	// auch unsere Zeile. Dies fuehrt zu folgendem erwuenschten
/*N*/ 	// Verhalten: Ftn1 pass noch auf die Seite/Spalte, Ftn2 nicht mehr.
/*N*/ 	// Also bleibt die Ftn2-Referenz auf der Seite/Spalte stehen. Die
/*N*/ 	// Fussnote selbst folgt aber erst auf der naechsten Seite/Spalte.
/*N*/ 	// Ausnahme: Wenn keine weitere Zeile auf diese Seite/Spalte passt,
/*N*/ 	// so sollte die Ftn2-Referenz auch auf die naechste wandern.
/*N*/ 	if( !rFtn.IsEndNote() )
/*N*/ 	{
/*N*/ 		SwSectionFrm *pSct = pBoss->FindSctFrm();
/*N*/ 		sal_Bool bAtSctEnd = pSct && pSct->IsFtnAtEnd();
/*N*/ 		if( FTNPOS_CHAPTER != pDoc->GetFtnInfo().ePos || bAtSctEnd )
/*N*/ 		{
/*N*/ 			SwFrm* pFtnCont = pBoss->FindFtnCont();
/*N*/ 			// Wenn der Boss in einem Bereich liegt, kann es sich nur um eine
/*N*/ 			// Spalte dieses Bereichs handeln. Wenn dies nicht die erste Spalte
/*N*/ 			// ist, duerfen wir ausweichen
/*N*/ 			if( !pFrm->IsInTab() && ( GetLineNr() > 1 || pFrm->GetPrev() ||
/*N*/ 				( !bAtSctEnd && pFrm->GetIndPrev() ) ||
/*N*/ 				( pSct && pBoss->GetPrev() ) ) )
/*N*/ 			{
/*N*/ 				if( !pFtnCont )
/*N*/ 				{
/*?*/ 					rInf.SetStop( sal_True );
/*?*/                     UNDO_SWAP( pFrm )
/*?*/ 					return 0;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Es darf keine Fussnotencontainer in spaltigen Bereichen und
/*N*/ 					// gleichzeitig auf der Seite/Seitenspalte geben
/*N*/ 					if( pSct && !bAtSctEnd ) // liegt unser Container in einem (spaltigen) Bereich?
/*N*/ 					{
/*?*/ 						SwFtnBossFrm* pTmp = pBoss->FindSctFrm()->FindFtnBossFrm( sal_True );
/*?*/ 						SwFtnContFrm* pFtnC = pTmp->FindFtnCont();
/*?*/ 						if( pFtnC )
/*?*/ 						{
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwFtnFrm* pTmp = (SwFtnFrm*)pFtnC->Lower();
/*?*/ 						}
/*N*/ 					}
/*N*/ 					// Ist dies die letzte passende Zeile?
/*N*/                     SwTwips nTmpBot = Y() + nReal * 2;
/*N*/ 
/*N*/                     if( bVert )
                            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                         nTmpBot = pFrm->SwitchHorizontalToVertical( nTmpBot );
/*N*/ 
/*N*/                     SWRECTFN( pFtnCont )
/*N*/ 
/*N*/                     long nDiff = (*fnRect->fnYDiff)(
/*N*/                                      (pFtnCont->Frm().*fnRect->fnGetTop)(),
/*N*/                                       nTmpBot );
/*N*/ 
/*N*/                     if( pScrFrm && nDiff < 0 )
/*N*/ 					{
/*?*/ 						if( pFtnFrm )
/*?*/ 						{
/*?*/ 							SwFtnBossFrm *pFtnBoss = pFtnFrm->FindFtnBossFrm();
/*?*/ 							if( pFtnBoss != pBoss )
/*?*/ 							{
/*?*/ 								// Wir sind in der letzte Zeile und die Fussnote
/*?*/ 								// ist auf eine andere Seite gewandert, dann wollen
/*?*/ 								// wir mit ...
/*?*/ 								rInf.SetStop( sal_True );
/*?*/                                 UNDO_SWAP( pFrm )
/*?*/ 								return 0;
/*?*/ 							}
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Endlich: FtnPortion anlegen und raus hier...
/*N*/ 	SwFtnPortion *pRet = new SwFtnPortion( rFtn.GetViewNumStr( *pDoc ),
/*N*/ 											pFrm, pFtn, nReal );
/*N*/ 	rInf.SetFtnInside( sal_True );
/*N*/ 
/*N*/     UNDO_SWAP( pFrm )
/*N*/ 
/*N*/ 	return pRet;
/*N*/  }

/*************************************************************************
 *						SwTxtFormatter::NewFtnNumPortion()
 *************************************************************************/

// Die Portion fuer die Ftn-Nummerierung im Ftn-Bereich

/*N*/ SwNumberPortion *SwTxtFormatter::NewFtnNumPortion( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/ 	ASSERT( pFrm->IsInFtn() && !pFrm->GetIndPrev() && !rInf.IsFtnDone(),
/*N*/ 			"This is the wrong place for a ftnnumber" );
/*N*/ 	if( rInf.GetTxtStart() != nStart ||
/*N*/ 		rInf.GetTxtStart() != rInf.GetIdx() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	const SwFtnFrm *pFtnFrm = pFrm->FindFtnFrm();
/*N*/ 	const SwTxtFtn *pFtn = pFtnFrm->GetAttr();
/*N*/ 
/*N*/ 	// Aha, wir sind also im Fussnotenbereich
/*N*/ 	SwFmtFtn& rFtn = (SwFmtFtn&)pFtn->GetFtn();
/*N*/ 
/*N*/ 	SwDoc *pDoc = pFrm->GetNode()->GetDoc();
/*N*/ 	XubString aFtnTxt( rFtn.GetViewNumStr( *pDoc, sal_True ));
/*N*/ 
/*N*/ 	const SwEndNoteInfo* pInfo;
/*N*/ 	if( rFtn.IsEndNote() )
/*?*/ 		pInfo = &pDoc->GetEndNoteInfo();
/*N*/ 	else
/*N*/ 		pInfo = &pDoc->GetFtnInfo();
/*N*/ 	const SwAttrSet& rSet = pInfo->GetCharFmt(*pDoc)->GetAttrSet();
/*N*/ 
/*N*/ 	const SwAttrSet* pParSet = &rInf.GetCharAttr();
/*N*/     SwFont *pFnt = new SwFont( pParSet, rInf.GetDoc() );
/*N*/     pFnt->SetDiffFnt(&rSet, rInf.GetDoc() );
/*N*/ 	SwTxtFtn* pTxtFtn = rFtn.GetTxtFtn();
/*N*/ 	if( pTxtFtn )
/*N*/ 	{
/*N*/ 		SwScriptInfo aScriptInfo;
/*N*/         SwAttrIter aIter( (SwTxtNode&)pTxtFtn->GetTxtNode(), aScriptInfo );
/*N*/ 		aIter.Seek( *pTxtFtn->GetStart() );
/*N*/ 		// Achtung: Wenn die Kriterien, nach denen der FtnReferenz-Font
/*N*/ 		// auf den FtnNumerierungsfont wirkt, geaendert werden, muss die
/*N*/ 		// untenstehende Methode SwFtnNumPortion::DiffFont() angepasst
/*N*/ 		// werden.
/*N*/ 		if( aIter.GetFnt()->IsSymbol(rInf.GetVsh()) || aIter.GetFnt()->GetCharSet() !=
/*N*/ 			pFnt->GetCharSet() )
/*N*/ 		{
/*N*/ 			const BYTE nAct = pFnt->GetActual();
/*N*/ 			pFnt->SetName( aIter.GetFnt()->GetName(), nAct );
/*N*/ 			pFnt->SetStyleName( aIter.GetFnt()->GetStyleName(), nAct );
/*N*/ 			pFnt->SetFamily( aIter.GetFnt()->GetFamily(),nAct );
/*N*/ 			pFnt->SetCharSet( aIter.GetFnt()->GetCharSet(), nAct );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*M*/       pFnt->SetVertical( pFnt->GetOrientation(), pFrm->IsVertical() );
/*N*/ 	return new SwFtnNumPortion( aFtnTxt, pFnt );
/*N*/ }

/*************************************************************************
 *					SwTxtFormatter::NewErgoSumPortion()
 *************************************************************************/

/*?*/ XubString lcl_GetPageNumber( const SwPageFrm* pPage )
/*?*/ {
/*?*/ 	ASSERT( pPage, "GetPageNumber: Homeless TxtFrm" );
/*?*/ 	MSHORT nVirtNum = pPage->GetVirtPageNum();
/*?*/ 	const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
/*?*/ 	return rNum.GetNumStr( nVirtNum );
/*?*/ }

/*N*/ SwErgoSumPortion *SwTxtFormatter::NewErgoSumPortion( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/ 	// Wir koennen nicht davon ausgehen, dass wir ein Follow sind
/*N*/ 	// 7983: GetIdx() nicht nStart
/*N*/ 	if( !pFrm->IsInFtn()  || pFrm->GetPrev() ||
/*N*/ 		rInf.IsErgoDone() || rInf.GetIdx() != pFrm->GetOfst() ||
/*N*/ 		pFrm->ImplFindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	// Aha, wir sind also im Fussnotenbereich
/*N*/ 	const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
/*N*/ 	SwTxtFrm *pQuoFrm = pFrm->FindQuoVadisFrm();
/*N*/ 	if( !pQuoFrm )
/*N*/ 		return 0;
/*?*/ 	const SwPageFrm* pPage = pFrm->FindPageFrm();
/*?*/ 	const SwPageFrm* pQuoPage = pQuoFrm->FindPageFrm();
/*?*/ 	if( pPage == pQuoFrm->FindPageFrm() )
/*?*/ 		return 0; // Wenn der QuoVadis auf der selben (spaltigen) Seite steht
            {DBG_BF_ASSERT(0, "STRIP");} return 0;//STRIP001 /*?*/ 	const XubString aPage = lcl_GetPageNumber( pPage );
/*N*/ }

/*************************************************************************
 *					SwTxtFormatter::FormatQuoVadis()
 *************************************************************************/

/*M*/ xub_StrLen SwTxtFormatter::FormatQuoVadis( const xub_StrLen nOffset )
/*M*/ {
/*M*/     ASSERT( ! pFrm->IsVertical() || ! pFrm->IsSwapped(),
/*M*/             "SwTxtFormatter::FormatQuoVadis with swapped frame" );
/*M*/ 
/*M*/ 	if( !pFrm->IsInFtn() || pFrm->ImplFindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
/*M*/ 		return nOffset;
/*M*/ 
/*M*/ 	const SwFrm* pErgoFrm = pFrm->FindFtnFrm()->GetFollow();
/*M*/ 	if( !pErgoFrm && pFrm->HasFollow() )
/*M*/ 		pErgoFrm = pFrm->GetFollow();
/*M*/ 	if( !pErgoFrm )
/*M*/ 		return nOffset;
/*M*/ 
/*M*/ 	if( pErgoFrm == pFrm->GetNext() )
/*M*/ 	{
/*M*/ 		SwFrm *pCol = pFrm->FindColFrm();
/*M*/ 		while( pCol && !pCol->GetNext() )
/*M*/ 			pCol = pCol->GetUpper()->FindColFrm();
/*M*/ 		if( pCol )
/*M*/ 			return nOffset;
/*M*/ 	}
/*M*/ 	else
/*M*/ 	{
/*M*/ 		const SwPageFrm* pPage = pFrm->FindPageFrm();
/*M*/ 		const SwPageFrm* pErgoPage = pErgoFrm->FindPageFrm();
/*M*/ 		if( pPage == pErgoFrm->FindPageFrm() )
/*M*/ 			return nOffset; // Wenn der ErgoSum auf der selben Seite steht
/*M*/ 	}
/*M*/ 
/*M*/ 	SwTxtFormatInfo &rInf = GetInfo();
/*M*/ 	const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
/*M*/ 	if( !rFtnInfo.aQuoVadis.Len() )
/*M*/ 		return nOffset;
/*M*/ 
/*M*/ 	// Ein Wort zu QuoVadis/ErgoSum:
/*M*/ 	// Fuer diese Texte wird der am Absatz eingestellte Font verwendet.
/*M*/ 	// Wir initialisieren uns also:
/*M*/ //	ResetFont();
/*M*/ 	FeedInf( rInf );
/*M*/ 	SeekStartAndChg( rInf, sal_True );
/*M*/ 	if( GetRedln() && pCurr->HasRedline() )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 GetRedln()->Seek( *pFnt, nOffset, 0 );
/*M*/ 
/*M*/ 	// Ein fieser Sonderfall: Flyfrms reichen in die Zeile und stehen
/*M*/ 	// natuerlich da, wo wir unseren Quovadis Text reinsetzen wollen.
/*M*/ 	// Erst mal sehen, ob es so schlimm ist:
/*M*/ 	SwLinePortion *pPor = pCurr->GetFirstPortion();
/*M*/ 	KSHORT nLastLeft = 0;
/*M*/ 	while( pPor )
/*M*/ 	{
/*M*/ 		if ( pPor->IsFlyPortion() )
/*M*/ 			nLastLeft = ( (SwFlyPortion*) pPor)->Fix() +
/*M*/ 						( (SwFlyPortion*) pPor)->Width();
/*M*/ 		pPor = pPor->GetPortion();
/*M*/ 	}
/*M*/ 	// Das alte Spiel: wir wollen, dass die Zeile an einer bestimmten
/*M*/ 	// Stelle umbricht, also beeinflussen wir die Width.
/*M*/ 	// nLastLeft ist jetzt quasi der rechte Rand.
/*M*/ 	const KSHORT nOldRealWidth = rInf.RealWidth();
/*M*/ 	rInf.RealWidth( nOldRealWidth - nLastLeft );
/*M*/ 
/*?*/ 	XubString aErgo = lcl_GetPageNumber( pErgoFrm->FindPageFrm() );
/*?*/ 	SwQuoVadisPortion *pQuo = new SwQuoVadisPortion(rFtnInfo.aQuoVadis, aErgo );
/*M*/ 	pQuo->SetAscent( rInf.GetAscent()  );
/*M*/ 	pQuo->Height( rInf.GetTxtHeight() );
/*M*/ 	pQuo->Format( rInf );
/*M*/     USHORT nQuoWidth = pQuo->Width();
/*M*/     SwLinePortion* pCurrPor = pQuo;
/*M*/ 
/*M*/     while ( rInf.GetRest() )
/*M*/     {
/*M*/         SwLinePortion* pFollow = rInf.GetRest();
/*M*/ 	    rInf.SetRest( 0 );
/*M*/         pCurrPor->Move( rInf );
/*M*/ 
/*M*/         ASSERT( pFollow->IsQuoVadisPortion(),
/*M*/                 "Quo Vadis, rest of QuoVadisPortion" )
/*M*/ 
/*M*/         // format the rest and append it to the other QuoVadis parts
/*M*/ 		pFollow->Format( rInf );
/*M*/         nQuoWidth += pFollow->Width();
/*M*/ 
/*M*/         pCurrPor->Append( pFollow );
/*M*/         pCurrPor = pFollow;
/*M*/     }
/*M*/ 
/*M*/     nLastLeft = nOldRealWidth - nQuoWidth;
/*M*/     Right( Right() - nQuoWidth );
/*M*/ 
/*M*/     SWAP_IF_NOT_SWAPPED( pFrm )
/*M*/ 
/*M*/ 	const xub_StrLen nRet = FormatLine( nStart );
/*M*/ 
/*M*/     UNDO_SWAP( pFrm )
/*M*/ 
/*M*/ 	Right( rInf.Left() + nOldRealWidth - 1 );
/*M*/ 
/*M*/ 	nLastLeft = nOldRealWidth - pCurr->Width();
/*M*/ 	FeedInf( rInf );
/*M*/ #ifdef USED
/*M*/ 	ASSERT( nOldRealWidth == rInf.RealWidth() && nLastLeft >= pQuo->Width(),
/*M*/ 			"SwTxtFormatter::FormatQuoVadis: crime doesn't pay" );
/*M*/ #endif
/*M*/ 
/*M*/ 	// Es kann durchaus sein, dass am Ende eine Marginportion steht,
/*M*/ 	// die beim erneuten Aufspannen nur Aerger bereiten wuerde.
/*M*/ 	pPor = pCurr->FindLastPortion();
/*M*/ 	SwGluePortion *pGlue = pPor->IsMarginPortion() ?
/*M*/ 		(SwMarginPortion*) pPor : 0;
/*M*/ 	if( pGlue )
/*M*/ 	{
/*M*/ 		pGlue->Height( 0 );
/*M*/ 		pGlue->Width( 0 );
/*M*/ 		pGlue->SetLen( 0 );
/*M*/ 		pGlue->SetAscent( 0 );
/*M*/ 		pGlue->SetPortion( NULL );
/*M*/ 		pGlue->SetFixWidth(0);
/*M*/ 	}
/*M*/ 
/*M*/ 	// Luxus: Wir sorgen durch das Aufspannen von Glues dafuer,
/*M*/ 	// dass der QuoVadis-Text rechts erscheint:
/*M*/     nLastLeft -= nQuoWidth;
/*M*/ 	if( nLastLeft )
/*M*/ 	{
/*M*/ 		if( nLastLeft > pQuo->GetAscent() ) // Mindestabstand
/*M*/ 		{
/*M*/ 			switch( GetAdjust() )
/*M*/ 			{
/*M*/ 				case SVX_ADJUST_BLOCK:
/*M*/ 				{
/*M*/ 					if( !pCurr->GetLen() ||
/*M*/ 						CH_BREAK != GetInfo().GetChar(nStart+pCurr->GetLen()-1))
/*M*/ 						nLastLeft = pQuo->GetAscent();
/*M*/ 					nQuoWidth += nLastLeft;
/*M*/ 					break;
/*M*/ 				}
/*M*/ 				case SVX_ADJUST_RIGHT:
/*M*/ 				{
/*M*/ 					nLastLeft = pQuo->GetAscent();
/*M*/ 					nQuoWidth += nLastLeft;
/*M*/ 					break;
/*M*/ 				}
/*M*/ 				case SVX_ADJUST_CENTER:
/*M*/ 				{
/*M*/ 					nQuoWidth += pQuo->GetAscent();
/*M*/ 					long nDiff = nLastLeft - nQuoWidth;
/*M*/ 					if( nDiff < 0 )
/*M*/ 					{
/*M*/ 						nLastLeft = pQuo->GetAscent();
/*M*/ 						nQuoWidth = -nDiff + nLastLeft;
/*M*/ 					}
/*M*/ 					else
/*M*/ 					{
/*M*/ 						nQuoWidth = 0;
/*M*/ 						nLastLeft = ( pQuo->GetAscent() + nDiff ) / 2;
/*M*/ 					}
/*M*/ 					break;
/*M*/ 				}
/*M*/ 				default:
/*M*/ 					nQuoWidth += nLastLeft;
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else
/*M*/ 			nQuoWidth += nLastLeft;
/*M*/ 		if( nLastLeft )
/*M*/ 		{
/*M*/ 			pGlue = new SwGluePortion(0);
/*M*/ 			pGlue->Width( nLastLeft );
/*M*/ 			pPor->Append( pGlue );
/*M*/ 			pPor = pPor->GetPortion();
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	// Jetzt aber: die QuoVadis-Portion wird angedockt:
/*M*/     pCurrPor = pQuo;
/*M*/     while ( pCurrPor )
/*M*/     {
/*M*/         // pPor->Append deletes the pPortoin pointer of pPor. Therefore
/*M*/         // we have to keep a pointer to the next portion
/*M*/         pQuo = (SwQuoVadisPortion*)pCurrPor->GetPortion();
/*M*/         pPor->Append( pCurrPor );
/*M*/         pPor = pPor->GetPortion();
/*M*/         pCurrPor = pQuo;
/*M*/     }
/*M*/ 
/*M*/ 	pCurr->Width( pCurr->Width() + KSHORT( nQuoWidth ) );
/*M*/ 
/*M*/ 	// Und noch einmal adjustieren wegen des Adjustment und nicht zu Letzt
/*M*/ 	// wegen folgendem Sonderfall: In der Zeile hat der DummUser durchgaengig
/*M*/ 	// einen kleineren Font eingestellt als der vom QuoVadis-Text ...
/*M*/ 	CalcAdjustLine( pCurr );
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ #endif
/*M*/ 
/*M*/ 	// Uff...
/*M*/ 	return nRet;
/*M*/ }


/*************************************************************************
 *					SwTxtFormatter::MakeDummyLine()
 *************************************************************************/

// MakeDummyLine() erzeugt eine Line, die bis zum unteren Seitenrand
// reicht. DummyLines bzw. DummyPortions sorgen dafuer, dass Oszillationen
// zum stehen kommen, weil Rueckflussmoeglichkeiten genommen werden.
// Sie werden bei absatzgebundenen Frames in Fussnoten und bei Ftn-
// Oszillationen verwendet.


/*************************************************************************
 *					   SwFtnSave::SwFtnSave()
 *************************************************************************/

/*M*/ SwFtnSave::SwFtnSave( const SwTxtSizeInfo &rInf, const SwTxtFtn* pTxtFtn )
/*M*/ 	: pInf( &((SwTxtSizeInfo&)rInf) )
/*M*/ {
/*M*/ 	if( pTxtFtn && rInf.GetTxtFrm() )
/*M*/ 	{
/*M*/ 		pFnt = ((SwTxtSizeInfo&)rInf).GetFont();
/*M*/ 	  	pOld = new SwFont( *pFnt );
/*M*/ 		pOld->GetTox() = pFnt->GetTox();
/*M*/ 		pFnt->GetTox() = 0;
/*M*/ 		SwFmtFtn& rFtn = (SwFmtFtn&)pTxtFtn->GetFtn();
/*M*/ 		const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
/*M*/ 
/*M*/         // examine text and set script
/*M*/         String aTmpStr( rFtn.GetViewNumStr( *pDoc ) );
/*M*/         pFnt->SetActual( WhichFont( 0, &aTmpStr, 0 ) );
/*M*/ 
/*M*/         const SwEndNoteInfo* pInfo;
/*M*/ 		if( rFtn.IsEndNote() )
/*M*/ 			pInfo = &pDoc->GetEndNoteInfo();
/*M*/ 		else
/*M*/ 			pInfo = &pDoc->GetFtnInfo();
/*M*/ 		const SwAttrSet& rSet =	pInfo->GetAnchorCharFmt((SwDoc&)*pDoc)->GetAttrSet();
/*M*/         pFnt->SetDiffFnt( &rSet, rInf.GetDoc() );
/*M*/ 
/*M*/         // we reduce footnote size, if we are inside a double line portion
/*M*/         if ( ! pOld->GetEscapement() && 50 == pOld->GetPropr() )
/*M*/         {
/*M*/             Size aSize = pFnt->GetSize( pFnt->GetActual() );
/*M*/             pFnt->SetSize( Size( (long)aSize.Width() / 2,
/*M*/                                  (long)aSize.Height() / 2 ),
/*M*/                            pFnt->GetActual() );
/*M*/         }
/*M*/ 
/*M*/         // set the correct rotation at the footnote font
/*M*/         const SfxPoolItem* pItem;
/*M*/         if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ROTATE,
/*M*/ 			sal_True, &pItem ))
/*M*/             pFnt->SetVertical( ((SvxCharRotateItem*)pItem)->GetValue(),
/*M*/                                 rInf.GetTxtFrm()->IsVertical() );
/*M*/ 
/*M*/         pFnt->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
/*M*/ 
/*M*/ 		if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
/*M*/ 			sal_True, &pItem ))
/*M*/ 			pFnt->SetBackColor( new Color( ((SvxBrushItem*)pItem)->GetColor() ) );
/*M*/ 	}
/*M*/ 	else
/*M*/ 		pFnt = NULL;
/*M*/ }

/*************************************************************************
 *					   SwFtnSave::~SwFtnSave()
 *************************************************************************/

/*N*/ SwFtnSave::~SwFtnSave()
/*N*/ {
/*N*/ 	if( pFnt )
/*N*/ 	{
/*N*/ 		// SwFont zurueckstellen
/*N*/ 		*pFnt = *pOld;
/*N*/ 		pFnt->GetTox() = pOld->GetTox();
/*N*/ 		pFnt->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
/*N*/ 		delete pOld;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwFtnPortion::SwFtnPortion()
 *************************************************************************/

/*N*/ SwFtnPortion::SwFtnPortion( const XubString &rExpand, SwTxtFrm *pFrm,
/*N*/                             SwTxtFtn *pFtn, KSHORT nReal )
/*N*/         : SwFldPortion( rExpand, 0 ), pFrm(pFrm), pFtn(pFtn), nOrigHeight( nReal )
/*N*/ {
/*N*/ 	SetLen(1);
/*N*/ 	SetWhichPor( POR_FTN );
/*N*/ }

/*************************************************************************
 *						SwFtnPortion::GetExpTxt()
 *************************************************************************/

/*N*/ sal_Bool SwFtnPortion::GetExpTxt( const SwTxtSizeInfo &, XubString &rTxt ) const
/*N*/ {
/*N*/ 	rTxt = aExpand;
/*N*/ 	return sal_True;
/*N*/ }



/*************************************************************************
 *				   virtual SwFtnPortion::Format()
 *************************************************************************/

/*N*/ sal_Bool SwFtnPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	SwFtnSave aFtnSave( rInf, pFtn );
/*N*/     // the idx is manipulated in SwExpandPortion::Format
/*N*/     // this flag indicates, that a footnote is allowed to trigger
/*N*/     // an underflow during SwTxtGuess::Guess
/*N*/     rInf.SetFakeLineStart( rInf.GetIdx() > rInf.GetLineStart() );
/*N*/     sal_Bool bFull = SwFldPortion::Format( rInf );
/*N*/     rInf.SetFakeLineStart( sal_False );
/*N*/ 	SetAscent( rInf.GetAscent() );
/*N*/     Height( rInf.GetTxtHeight() );
/*N*/ 	rInf.SetFtnDone( !bFull );
/*N*/ 	if( !bFull )
/*N*/ 		rInf.SetParaFtn();
/*N*/ 	return bFull;
/*N*/ }

/*************************************************************************
 *				 virtual SwFtnPortion::Paint()
 *************************************************************************/

/*N*/ void SwFtnPortion::Paint( const SwTxtPaintInfo &rInf ) const
/*N*/ {
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	SwFtnSave aFtnSave( rInf, pFtn );
/*N*/ }

/*************************************************************************
 *				 virtual SwFtnPortion::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwFtnPortion::GetTxtSize( const SwTxtSizeInfo &rInfo ) const
/*N*/ {
            DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	SwFtnSave aFtnSave( rInfo, pFtn );
/*N*/ }

/*************************************************************************
 *						class SwQuoVadisPortion
 *************************************************************************/



/*************************************************************************
 *				   virtual SwQuoVadisPortion::Format()
 *************************************************************************/


/*************************************************************************
 *				 virtual SwQuoVadisPortion::GetExpTxt()
 *************************************************************************/


/*************************************************************************
 *              virtual SwQuoVadisPortion::HandlePortion()
 *************************************************************************/


/*************************************************************************
 *				 virtual SwQuoVadisPortion::Paint()
 *************************************************************************/


/*************************************************************************
 *						class SwErgoSumPortion
 *************************************************************************/




/*************************************************************************
 *				   virtual SwErgoSumPortion::Format()
 *************************************************************************/


/*************************************************************************
 * sal_Bool SwFtnNumPortion::DiffFont()
 * 	liefert sal_True, wenn der Font der FtnReferenz (pFont) eine Aenderung
 *  des Fonts der FtnNumerierung (pFnt) erforderlich macht.
 *  Die Bedingungen sind ein Spiegel dessen, was in NewFtnNumPortion steht
 *************************************************************************/

/*N*/ sal_Bool SwFtnNumPortion::DiffFont( SwFont* pFont )
/*N*/ {
/*N*/ 	if( pFnt->GetName() != pFont->GetName() ||
/*N*/ 		pFnt->GetStyleName() != pFont->GetStyleName() ||
/*N*/ 		pFnt->GetFamily() != pFont->GetFamily() ||
/*N*/ 		pFont->GetCharSet() != pFnt->GetCharSet() )
/*N*/ 		return sal_True;
/*N*/ 	return sal_False;
/*N*/ }

/*************************************************************************
 *						SwParaPortion::SetErgoSumNum()
 *************************************************************************/


/*************************************************************************
 *						SwParaPortion::UpdateQuoVadis()
 *
 * Wird im SwTxtFrm::Prepare() gerufen
 *************************************************************************/

/*N*/ sal_Bool SwParaPortion::UpdateQuoVadis( const XubString &rQuo )
/*N*/ {
/*N*/ 	SwLineLayout *pLay = this;
/*N*/ 	while( pLay->GetNext() )
/*N*/ 	{
///*?*/ 		DBG_LOOP;
/*?*/ 		pLay = pLay->GetNext();
/*N*/ 	}
/*N*/ 	SwLinePortion	  *pPor = pLay;
/*N*/ 	SwQuoVadisPortion *pQuo = 0;
/*N*/ 	while( pPor && !pQuo )
/*N*/ 	{
/*N*/ 		if ( pPor->IsQuoVadisPortion() )
/*?*/ 			pQuo = (SwQuoVadisPortion*)pPor;
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !pQuo )
/*N*/ 		return sal_False;
/*N*/ 
            {DBG_BF_ASSERT(0, "STRIP");} return sal_False;//STRIP001 /*?*/ 	return pQuo->GetQuoTxt() == rQuo;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
