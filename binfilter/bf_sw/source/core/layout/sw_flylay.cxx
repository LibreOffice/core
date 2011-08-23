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

#include <horiornt.hxx>

#include "doc.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "ftnfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "hints.hxx"
#include "pam.hxx"
#include "sectfrm.hxx"


#include <bf_svx/svdpage.hxx>

#include <bf_svx/ulspitem.hxx>

#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include "tabfrm.hxx"
#include "flyfrms.hxx"

#ifdef ACCESSIBLE_LAYOUT
#include <frmsh.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|*	SwFlyFreeFrm::SwFlyFreeFrm(), ~SwFlyFreeFrm()
|*
|*	Ersterstellung		MA 03. Dec. 92
|*	Letzte Aenderung	MA 09. Apr. 99
|*
|*************************************************************************/

/*N*/ SwFlyFreeFrm::SwFlyFreeFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
/*N*/ 	SwFlyFrm( pFmt, pAnch ),
/*N*/ 	pPage( 0 )
/*N*/ {
/*N*/ }

/*N*/ SwFlyFreeFrm::~SwFlyFreeFrm()
/*N*/ {
/*N*/ 	//und Tschuess.
/*N*/ 	if( GetPage() )
/*N*/ 	{
/*N*/ 		if( GetFmt()->GetDoc()->IsInDtor() )
/*N*/ 		{
/*N*/ 			if ( IsFlyAtCntFrm() && GetPage()->GetSortedObjs() )
/*N*/ 				GetPage()->GetSortedObjs()->Remove( GetVirtDrawObj() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwRect aTmp( AddSpacesToFrm() );
/*N*/ 			SwFlyFreeFrm::NotifyBackground( GetPage(), aTmp, PREP_FLY_LEAVE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFlyFreeFrm::NotifyBackground()
|*
|*	Beschreibung		Benachrichtigt den Hintergrund (alle CntntFrms die
|*		gerade ueberlappt werden. Ausserdem wird das Window in einigen
|* 		Faellen direkt invalidiert (vor allem dort, wo keine CntntFrms
|*		ueberlappt werden.
|* 		Es werden auch die CntntFrms innerhalb von anderen Flys
|*		beruecksichtigt.
|*	Ersterstellung		MA 03. Dec. 92
|*	Letzte Aenderung	MA 26. Aug. 93
|*
|*************************************************************************/

/*N*/ void SwFlyFreeFrm::NotifyBackground( SwPageFrm *pPage,
/*N*/ 									 const SwRect& rRect, PrepareHint eHint )
/*N*/ {
/*N*/ 	::binfilter::Notify_Background( GetVirtDrawObj(), pPage, rRect, eHint, TRUE );
/*N*/ }

/*************************************************************************
|*
|*	SwFlyFreeFrm::MakeAll()
|*
|*	Ersterstellung		MA 18. Feb. 94
|*	Letzte Aenderung	MA 03. Mar. 97
|*
|*************************************************************************/

/*N*/ void SwFlyFreeFrm::MakeAll()
/*N*/ {
/*N*/     if ( !GetAnchor() || IsLocked() || IsColLocked() )
/*N*/ 		return;
/*N*/     if( !GetPage() && GetAnchor() && GetAnchor()->IsInFly() )
/*N*/     {
/*?*/         SwFlyFrm* pFly = GetAnchor()->FindFlyFrm();
/*?*/         SwPageFrm *pPage = pFly ? pFly->FindPageFrm() : NULL;
/*?*/         if( pPage )
/*?*/             pPage->SwPageFrm::AppendFly( this );
/*N*/     }
/*N*/     if( !GetPage() )
/*?*/         return;
/*N*/ 
/*N*/ 	Lock();	//Der Vorhang faellt
/*N*/ 
/*N*/ 	//uebernimmt im DTor die Benachrichtigung
/*N*/ 	const SwFlyNotify aNotify( this );
/*N*/ 
/*N*/ 	if ( IsClipped() )
/*N*/ 		bValidPos = bValidSize = bHeightClipped = bWidthClipped = FALSE;
/*N*/ 
/*N*/ 	while ( !bValidPos || !bValidSize || !bValidPrtArea || bFormatHeightOnly )
/*N*/ 	{
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/        SWRECTFN( this )
/*N*/ #endif
/*N*/ 		const SwFmtFrmSize *pSz;
/*N*/ 		{	//Zusaetzlicher Scope, damit aAccess vor dem Check zerstoert wird!
/*N*/ 
/*N*/ 			SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
/*N*/ 			const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 			pSz = &rAttrs.GetAttrSet().GetFrmSize();
/*N*/ 
/*N*/ 			//Nur einstellen wenn das Flag gesetzt ist!!
/*N*/ 			if ( !bValidSize )
/*N*/ 			{
/*N*/ 				bValidPrtArea = FALSE;
/*N*/ 				const Size aTmp( CalcRel( *pSz ) );
/*N*/ 				const SwTwips nMin = MINFLY + rAttrs.CalcLeftLine()+rAttrs.CalcRightLine();
/*N*/                 long nDiff = bVert ? aTmp.Height() : aTmp.Width();
/*N*/                 if( nDiff < nMin )
/*?*/                     nDiff = nMin;
/*N*/                 nDiff -= (aFrm.*fnRect->fnGetWidth)();
/*N*/                 if( nDiff )
/*N*/                 {
/*N*/                     (aFrm.*fnRect->fnAddRight)( nDiff );
/*N*/                     bValidPos = FALSE;
/*N*/                 }
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( !bValidPrtArea )
/*N*/ 				MakePrtArea( rAttrs );
/*N*/ 
/*N*/ 			if ( !bValidSize || bFormatHeightOnly )
/*N*/ 			{
/*N*/ 				bValidSize = FALSE;
/*N*/ 				Format( &rAttrs );
/*N*/ 				bFormatHeightOnly = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( !bValidPos )
/*N*/ 			{
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/                 const Point aOldPos( (Frm().*fnRect->fnGetPos)() );
/*N*/ 				MakeFlyPos();
/*N*/                 if( aOldPos == (Frm().*fnRect->fnGetPos)() )
/*N*/ #else
/*?*/ 				const Point aOldPos( Frm().Pos() );
/*?*/ 				MakeFlyPos();
/*?*/ 				if( aOldPos == Frm().Pos() )
/*N*/ #endif
/*N*/ 				{
/*N*/ 					if( !bValidPos && GetAnchor()->IsInSct() &&
/*N*/ 						!GetAnchor()->FindSctFrm()->IsValid() )
/*?*/ 						bValidPos = TRUE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bValidSize = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bValidPos && bValidSize )
/*N*/ 			CheckClip( *pSz );
/*N*/ 	}
/*N*/ 	Unlock();
/*N*/ 
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/ #ifdef DBG_UTIL
/*N*/     SWRECTFN( this )
/*N*/     ASSERT( bHeightClipped || ( (Frm().*fnRect->fnGetHeight)() > 0 &&
/*N*/             (Prt().*fnRect->fnGetHeight)() > 0),
/*N*/ 			"SwFlyFreeFrm::Format(), flipping Fly." );
/*N*/ 
/*N*/ #endif
/*N*/ #else
/*?*/ 	ASSERT( bHeightClipped || (Frm().Height() > 0 && Prt().Height() > 0),
/*?*/ 			"SwFlyFreeFrm::Format(), flipping Fly." );
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	SwFlyFreeFrm::CheckClip()
|*
|*	Ersterstellung		MA 21. Feb. 94
|*	Letzte Aenderung	MA 03. Mar. 97
|*
|*************************************************************************/

/*N*/ void SwFlyFreeFrm::CheckClip( const SwFmtFrmSize &rSz )
/*N*/ {
/*N*/ 	//Jetzt ist es ggf. an der Zeit geignete Massnahmen zu ergreifen wenn
/*N*/ 	//der Fly nicht in seine Umgebung passt.
/*N*/     //Zuerst gibt der Fly seine Position auf. Danach wird er zunaechst
/*N*/     //formatiert. Erst wenn er auch durch die Aufgabe der Position nicht
/*N*/ 	//passt wird die Breite oder Hoehe aufgegeben - der Rahmen wird soweit
/*N*/ 	//wie notwendig zusammengequetscht.
/*N*/ 
/*N*/ 	const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
/*N*/ 	SwRect aClip, aTmpStretch;
/*N*/ 	::binfilter::CalcClipRect( pObj, aClip, TRUE );
/*N*/ 	::binfilter::CalcClipRect( pObj, aTmpStretch, FALSE );
/*N*/ 	aClip._Intersection( aTmpStretch );
/*N*/ 
/*N*/     const long nBot = Frm().Top() + Frm().Height();
/*N*/     const long nRig = Frm().Left() + Frm().Width();
/*N*/     const long nClipBot = aClip.Top() + aClip.Height();
/*N*/     const long nClipRig = aClip.Left() + aClip.Width();
/*N*/ 
/*N*/ 	const FASTBOOL bBot = nBot > nClipBot;
/*N*/ 	const FASTBOOL bRig = nRig > nClipRig;
/*N*/ 	if ( bBot || bRig )
/*N*/ 	{
/*N*/ 		FASTBOOL bAgain = FALSE;
/*N*/         if ( bBot && !GetDrawObjs() && !GetAnchor()->IsInTab() )
/*N*/ 		{
/*N*/ 			SwFrm* pHeader = FindFooterOrHeader();
/*N*/ 			// In a header, correction of the position is no good idea.
/*N*/ 			// If the fly moves, some paragraphs has to be formatted, this
/*N*/             // could cause a change of the height of the headerframe,
/*N*/ 			// now the flyframe can change its position and so on ...
/*N*/ 			if( !pHeader || !pHeader->IsHeaderFrm() )
/*N*/ 			{
/*N*/ 				const long nOld = Frm().Top();
/*N*/                 Frm().Pos().Y() = Max( aClip.Top(), nClipBot - Frm().Height() );
/*N*/                 if ( Frm().Top() != nOld )
/*N*/ 					bAgain = TRUE;
/*N*/ 				bHeightClipped = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bRig )
/*N*/ 		{
/*N*/ 			const long nOld = Frm().Left();
/*N*/             Frm().Pos().X() = Max( aClip.Left(), nClipRig - Frm().Width() );
/*N*/ 			if ( Frm().Left() != nOld )
/*N*/ 			{
/*N*/ 				const SwFmtHoriOrient &rH = GetFmt()->GetHoriOrient();
/*N*/ 				// Links ausgerichtete duerfen nicht nach links verschoben werden,
/*N*/ 				// wenn sie einem anderen ausweichen.
/*N*/ 				if( rH.GetHoriOrient() == HORI_LEFT )
/*?*/ 					Frm().Pos().X() = nOld;
/*N*/ 				else
/*N*/ 					bAgain = TRUE;
/*N*/ 			}
/*N*/ 			bWidthClipped = TRUE;
/*N*/ 		}
/*N*/ 		if ( bAgain )
/*N*/ 			bValidSize = FALSE;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//Wenn wir hier ankommen ragt der Frm in unerlaubte Bereiche
/*N*/ 			//hinein, und eine Positionskorrektur ist nicht erlaubt bzw.
/*N*/ 			//moeglich oder noetig.
/*N*/ 
/*N*/ 			//Fuer Flys mit OLE-Objekten als Lower sorgen wir dafuer, dass
/*N*/ 			//immer proportional Resized wird.
/*N*/ 			Size aOldSize( Frm().SSize() );
/*N*/ 
/*N*/ 			//Zuerst wird das FrmRect eingestellt, und dann auf den Frm
/*N*/ 			//uebertragen.
/*N*/ 			SwRect aFrmRect( Frm() );
/*N*/ 
/*N*/ 			if ( bBot )
/*N*/ 			{
/*N*/ 				long nDiff = nClipBot;
/*N*/                 nDiff -= aFrmRect.Top(); //nDiff ist die verfuegbare Strecke.
/*N*/ 				nDiff = aFrmRect.Height() - nDiff;
/*N*/ 				aFrmRect.Height( aFrmRect.Height() - nDiff );
/*N*/ 				bHeightClipped = TRUE;
/*N*/ 			}
/*N*/ 			if ( bRig )
/*N*/ 			{
/*N*/ 				long nDiff = nClipRig;
/*N*/                 nDiff -= aFrmRect.Left();//nDiff ist die verfuegbare Strecke.
/*N*/ 				nDiff = aFrmRect.Width() - nDiff;
/*N*/ 				aFrmRect.Width( aFrmRect.Width() - nDiff );
/*N*/ 				bWidthClipped = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/           if ( Lower() && Lower()->IsNoTxtFrm() && !FindFooterOrHeader() )
/*N*/ 			{
/*?*/ 				//Wenn Breite und Hoehe angepasst wurden, so ist die
/*?*/ 				//groessere Veraenderung massgeblich.
/*?*/ 				if ( aFrmRect.Width() != aOldSize.Width() &&
/*?*/ 					 aFrmRect.Height()!= aOldSize.Height() )
/*?*/ 				{
/*?*/ 					if ( (aOldSize.Width() - aFrmRect.Width()) >
/*?*/ 						 (aOldSize.Height()- aFrmRect.Height())	)
/*?*/ 						aFrmRect.Height( aOldSize.Height() );
/*?*/ 					else
/*?*/ 						aFrmRect.Width( aOldSize.Width() );
/*?*/ 				}
/*?*/ 
/*?*/ 				//Breite angepasst? - Hoehe dann proportional verkleinern
/*?*/ 				if( aFrmRect.Width() != aOldSize.Width() )
/*?*/                 {
/*?*/ 					aFrmRect.Height( aFrmRect.Width() * aOldSize.Height() /
/*?*/ 									 aOldSize.Width() );
/*?*/                     bHeightClipped = TRUE;
/*?*/                 }
/*?*/ 				//Hoehe angepasst? - Breite dann proportional verkleinern
/*?*/                 else if( aFrmRect.Height() != aOldSize.Height() )
/*?*/                 {
/*?*/ 					aFrmRect.Width( aFrmRect.Height() * aOldSize.Width() /
/*?*/ 									aOldSize.Height() );
/*?*/                     bWidthClipped = TRUE;
/*?*/                 }
/*?*/ 
/*?*/ //                if( bWidthClipped || bHeightClipped )
/*?*/ //                {
/*?*/ //                    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
/*?*/ //                    pFmt->LockModify();
/*?*/ //                    SwFmtFrmSize aFrmSize( rSz );
/*?*/ //                    aFrmSize.SetWidth( aFrmRect.Width() );
/*?*/ //                    aFrmSize.SetHeight( aFrmRect.Height() );
/*?*/ //                    pFmt->SetAttr( aFrmSize );
/*?*/ //                    pFmt->UnlockModify();
/*?*/ //                }
/*N*/ 			}
/*N*/ 
/*N*/ 			//Jetzt die Einstellungen am Frm vornehmen, bei Spalten werden
/*N*/ 			//die neuen Werte in die Attribute eingetragen, weil es sonst
/*N*/ 			//ziemlich fiese Oszillationen gibt.
/*N*/ 			const long nPrtHeightDiff = Frm().Height() - Prt().Height();
/*N*/ 			const long nPrtWidthDiff  = Frm().Width()  - Prt().Width();
/*N*/ 			Frm().Height( aFrmRect.Height() );
/*N*/ 			Frm().Width ( Max( long(MINLAY), aFrmRect.Width() ) );
/*N*/ 			if ( Lower() && Lower()->IsColumnFrm() )
/*N*/ 			{
/*?*/ 				ColLock();	//Grow/Shrink locken.
/*?*/ 				const Size aOldSize( Prt().SSize() );
/*?*/ 				Prt().Height( Frm().Height() - nPrtHeightDiff );
/*?*/ 				Prt().Width ( Frm().Width()  - nPrtWidthDiff );
/*?*/ 				ChgLowersProp( aOldSize );
/*?*/ 				SwFrm *pLow = Lower();
/*?*/ 				do
/*?*/ 				{	pLow->Calc();
/*?*/ 					// auch den (Column)BodyFrm mitkalkulieren
/*?*/ 					((SwLayoutFrm*)pLow)->Lower()->Calc();
/*?*/ 					pLow = pLow->GetNext();
/*?*/ 				} while ( pLow );
/*?*/ 				::binfilter::CalcCntnt( this );
/*?*/ 				ColUnlock();
/* MA 02. Sep. 96: Wenn das Attribut gesetzt wird funktionieren Flys in Flys
 * nicht  (30095 30096)
                SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
                pFmt->LockModify();
                SwFmtFrmSize aFrmSize( rSz );
                if ( bRig )
                    aFrmSize.SetWidth( Frm().Width() );
                if ( bBot )
                {
                    aFrmSize.SetSizeType( ATT_FIX_SIZE );
                    aFrmSize.SetHeight( Frm().Height() );
                    bFixHeight = TRUE;
                    bMinHeight = FALSE;
                }
                pFmt->SetAttr( aFrmSize );
                pFmt->UnlockModify();
*/
/*?*/ //Stattdessen:
/*?*/ 				if ( !bValidSize && !bWidthClipped )
/*?*/ 					bFormatHeightOnly = bValidSize = TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				Prt().Height( Frm().Height() - nPrtHeightDiff );
/*N*/ 				Prt().Width ( Frm().Width()  - nPrtWidthDiff );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFlyLayFrm::SwFlyLayFrm()
|*
|*	Ersterstellung		MA 25. Aug. 92
|*	Letzte Aenderung	MA 09. Apr. 99
|*
|*************************************************************************/

/*N*/ SwFlyLayFrm::SwFlyLayFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
/*N*/ 	SwFlyFreeFrm( pFmt, pAnch )
/*N*/ {
/*N*/ 	bLayout = TRUE;
/*N*/ }

/*************************************************************************
|*
|*	SwFlyLayFrm::Modify()
|*
|*	Ersterstellung		MA 08. Feb. 93
|*	Letzte Aenderung	MA 28. Aug. 93
|*
|*************************************************************************/

/*N*/ void SwFlyLayFrm::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pNew ? pNew->Which() : 0;
/*N*/ 
/*N*/ 	SwFmtAnchor *pAnch = 0;
/*N*/ 	if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
/*N*/ 		((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, FALSE,
/*N*/ 			(const SfxPoolItem**)&pAnch ))
/*N*/ 		;		// Beim GetItemState wird der AnkerPointer gesetzt !
/*N*/ 
/*N*/ 	else if( RES_ANCHOR == nWhich )
/*N*/ 	{
/*N*/ 		//Ankerwechsel, ich haenge mich selbst um.
/*N*/ 		//Es darf sich nicht um einen Wechsel des Ankertyps handeln,
/*N*/ 		//dies ist nur ueber die SwFEShell moeglich.
/*?*/ 		pAnch = (SwFmtAnchor*)pNew;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pAnch )
/*N*/ 	{
/*?*/ 		ASSERT( pAnch->GetAnchorId() ==
/*?*/ 				GetFmt()->GetAnchor().GetAnchorId(),
/*?*/ 				"8-) Unzulaessiger Wechsel des Ankertyps." );
/*?*/ 
/*?*/ 		//Abmelden, Seite besorgen, an den entsprechenden LayoutFrm
/*?*/ 		//haengen.
/*?*/ 		SwRect aOld( AddSpacesToFrm() );
/*?*/ 		SwPageFrm *pOldPage = GetPage();
/*?*/ 		GetAnchor()->RemoveFly( this );
/*?*/ 
/*?*/ 		if( FLY_PAGE == pAnch->GetAnchorId() )
/*?*/ 		{
/*?*/ 			USHORT nPgNum = pAnch->GetPageNum();
/*?*/ 			SwRootFrm *pRoot = FindRootFrm();
/*?*/ 			SwPageFrm *pPage = (SwPageFrm*)pRoot->Lower();
/*?*/ 			for ( USHORT i = 1; (i <= nPgNum) && pPage; ++i,
/*?*/ 								pPage = (SwPageFrm*)pPage->GetNext() )
/*?*/ 			{
/*?*/ 				if ( i == nPgNum )
/*?*/ 					pPage->PlaceFly( this, 0, pAnch );
/*?*/ 			}
/*?*/ 			if( !pPage )
/*?*/ 			{
/*?*/ 				pRoot->SetAssertFlyPages();
/*?*/ 				pRoot->AssertFlyPages();
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
/*?*/ 			SwCntntFrm *pCntnt = GetFmt()->GetDoc()->GetNodes().GoNext( &aIdx )->
/*?*/ 						 GetCntntNode()->GetFrm( 0, 0, FALSE );
/*?*/ 			if( pCntnt )
/*?*/ 			{
/*?*/ 				SwFlyFrm *pTmp = pCntnt->FindFlyFrm();
/*?*/ 				if( pTmp )
/*?*/ 					pTmp->AppendFly( this );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if ( pOldPage && pOldPage != GetPage() )
/*?*/ 			NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );
/*?*/ 		SetCompletePaint();
/*?*/ 		InvalidateAll();
/*?*/ 		SetNotifyBack();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SwFlyFrm::Modify( pOld, pNew );
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::AppendFly()
|*
|*	Ersterstellung		MA 10. Oct. 92
|*	Letzte Aenderung	MA 08. Jun. 96
|*
|*************************************************************************/

/*N*/ void SwPageFrm::AppendFly( SwFlyFrm *pNew )
/*N*/ {
/*N*/ 	if ( !pNew->GetVirtDrawObj()->IsInserted() )
/*N*/ 		FindRootFrm()->GetDrawPage()->InsertObject(
/*N*/ 				(SdrObject*)pNew->GetVirtDrawObj(),
/*N*/ 				pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );
/*N*/ 
/*N*/ 	InvalidateSpelling();
/*N*/ 	InvalidateAutoCompleteWords();
/*N*/ 
/*N*/ 	if ( GetUpper() )
/*N*/ 	{
/*N*/ 		((SwRootFrm*)GetUpper())->SetIdleFlags();
/*N*/ 		((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
/*N*/ 	}
/*N*/ 
/*N*/ 	const SdrObjectPtr pObj = pNew->GetVirtDrawObj();
/*N*/ 	ASSERT( pNew->GetAnchor(), "Fly without Anchor" );
/*N*/ 	SwFlyFrm *pFly = pNew->GetAnchor()->FindFlyFrm();
/*N*/ 	if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
/*N*/ 	{
/*?*/ 		UINT32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1;
/*?*/ 		if ( pObj->GetPage() )
/*?*/ 			pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(), nNewNum);
/*?*/ 		else
/*?*/ 			pObj->SetOrdNum( nNewNum );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Flys die im Cntnt sitzen beachten wir nicht weiter.
/*N*/ 	if ( pNew->IsFlyInCntFrm() )
/*N*/ 		InvalidateFlyInCnt();
/*N*/     else
/*N*/     {
/*N*/         InvalidateFlyCntnt();
/*N*/ 
/*N*/         if ( !pSortedObjs )
/*N*/             pSortedObjs = new SwSortDrawObjs();
/*N*/         if ( !pSortedObjs->Insert( pObj ) )
/*?*/             ASSERT( FALSE, "Fly nicht in Sorted eingetragen." );
/*N*/ 
/*N*/         ((SwFlyFreeFrm*)pNew)->SetPage( this );
/*N*/         pNew->InvalidatePage( this );
/*N*/ 
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 		// Notify accessible layout. That's required at this place for
/*N*/ 		// frames only where the anchor is moved. Creation of new frames
/*N*/ 		// is additionally handled by the SwFrmNotify class.
/*N*/ 		if( GetUpper() &&
/*N*/ 			static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
/*N*/ 		 	static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/     }
/*N*/ 
/*N*/     if( pNew->GetDrawObjs() )
/*N*/     {
/*?*/         SwDrawObjs &rObjs = *pNew->GetDrawObjs();
/*?*/         for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*?*/         {
/*?*/             SdrObject *pO = rObjs[i];
/*?*/             if( pO->IsWriterFlyFrame() )
/*?*/             {
/*?*/                 SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*?*/                 if( pFly->IsFlyFreeFrm() && !((SwFlyFreeFrm*)pFly)->GetPage() )
/*?*/                     SwPageFrm::AppendFly( pFly );
/*?*/             }
/*?*/         }
/*N*/     }
/*N*/ 
/*N*/ 	//fix(3018): Kein pNew->Calc() oder sonstiges hier.
/*N*/ 	//Code enfernt in flylay.cxx Rev 1.51
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::RemoveFly()
|*
|*	Ersterstellung		MA 10. Oct. 92
|*	Letzte Aenderung	MA 26. Aug. 96
|*
|*************************************************************************/

/*N*/ void SwPageFrm::RemoveFly( SwFlyFrm *pToRemove )
/*N*/ {
/*N*/ 	const UINT32 nOrdNum = pToRemove->GetVirtDrawObj()->GetOrdNum();
/*N*/ 	FindRootFrm()->GetDrawPage()->RemoveObject( nOrdNum );
/*N*/ 	pToRemove->GetVirtDrawObj()->ReferencedObj().SetOrdNum( nOrdNum );
/*N*/ 
/*N*/ 	if ( GetUpper() )
/*N*/ 	{
/*N*/ 		if ( !pToRemove->IsFlyInCntFrm() )
/*N*/ 			((SwRootFrm*)GetUpper())->SetSuperfluous();
/*N*/ 		((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
/*N*/ 	}
/*N*/ 
/*N*/ 	//Flys die im Cntnt sitzen beachten wir nicht weiter.
/*N*/ 	if ( pToRemove->IsFlyInCntFrm() )
/*?*/ 		return;
/*N*/ 
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	// Notify accessible layout. That's required at this place for
/*N*/ 	// frames only where the anchor is moved. Creation of new frames
/*N*/ 	// is additionally handled by the SwFrmNotify class.
/*N*/ 	if( GetUpper() &&
/*N*/ 		static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
/*N*/ 		static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	//Collections noch nicht loeschen. Das passiert am Ende
/*N*/ 	//der Action im RemoveSuperfluous der Seite - angestossen von gleich-
/*N*/ 	//namiger Methode der Root.
/*N*/ 	//Die FlyColl kann bereits weg sein, weil der DTor der Seite
/*N*/ 	//gerade 'laeuft'
/*N*/ 	if ( pSortedObjs )
/*N*/ 	{
/*N*/ 		pSortedObjs->Remove( pToRemove->GetVirtDrawObj() );
/*N*/ 		if ( !pSortedObjs->Count() )
/*N*/ 		{	DELETEZ( pSortedObjs );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	((SwFlyFreeFrm*)pToRemove)->SetPage( 0 );
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::MoveFly
|*
|*	Ersterstellung		MA 25. Jan. 97
|*	Letzte Aenderung	MA 25. Jan. 97
|*
|*************************************************************************/

/*N*/ void SwPageFrm::MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest )
/*N*/ {
/*N*/ 	//Invalidierungen
/*N*/ 	if ( GetUpper() )
/*N*/ 	{
/*N*/ 		((SwRootFrm*)GetUpper())->SetIdleFlags();
/*N*/ 		if ( !pToMove->IsFlyInCntFrm() && pDest->GetPhyPageNum() < GetPhyPageNum() )
/*N*/ 			((SwRootFrm*)GetUpper())->SetSuperfluous();
/*N*/ 	}
/*N*/ 	pDest->InvalidateSpelling();
/*N*/ 	pDest->InvalidateAutoCompleteWords();
/*N*/ 
/*N*/ 	if ( pToMove->IsFlyInCntFrm() )
/*N*/ 	{
/*?*/ 		pDest->InvalidateFlyInCnt();
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	// Notify accessible layout. That's required at this place for
/*N*/ 	// frames only where the anchor is moved. Creation of new frames
/*N*/ 	// is additionally handled by the SwFrmNotify class.
/*N*/ 	if( GetUpper() &&
/*N*/ 		static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
/*N*/ 		static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	//Die FlyColl kann bereits weg sein, weil der DTor der Seite
/*N*/ 	//gerade 'laeuft'
/*N*/ 	const SdrObjectPtr pObj = pToMove->GetVirtDrawObj();
/*N*/ 	if ( pSortedObjs )
/*N*/ 	{
/*N*/ 		pSortedObjs->Remove( pObj );
/*N*/ 		if ( !pSortedObjs->Count() )
/*N*/ 		{	DELETEZ( pSortedObjs );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Anmelden
/*N*/ 	if ( !pDest->GetSortedObjs() )
/*N*/ 		pDest->pSortedObjs = new SwSortDrawObjs();
/*N*/ 	if ( !pDest->GetSortedObjs()->Insert( pObj ) )
/*?*/ 		ASSERT( FALSE, "Fly nicht in Sorted eingetragen." );
/*N*/ 
/*N*/ 	((SwFlyFreeFrm*)pToMove)->SetPage( pDest );
/*N*/ 	pToMove->InvalidatePage( pDest );
/*N*/ 	pToMove->SetNotifyBack();
/*N*/ 	pDest->InvalidateFlyCntnt();
/*N*/ 
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	// Notify accessible layout. That's required at this place for
/*N*/ 	// frames only where the anchor is moved. Creation of new frames
/*N*/ 	// is additionally handled by the SwFrmNotify class.
/*N*/ 	if( GetUpper() &&
/*N*/ 		static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
/*N*/ 		static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::AppendDrawObject(), RemoveDrawObject()
|*
|*	Ersterstellung		MA 09. Jan. 95
|*	Letzte Aenderung	MA 31. Jul. 96
|*
|*************************************************************************/

/*N*/ void SwPageFrm::AppendDrawObj( SwDrawContact *pNew )
/*N*/ {
/*N*/ 	if ( GetUpper() )
/*N*/ 		((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
/*N*/ 
/*N*/ 	const SdrObjectPtr pObj = pNew->GetMaster();
/*N*/ 	ASSERT( pNew->GetAnchor(), "Contact without Anchor" );
/*N*/ 	SwFlyFrm *pFly = pNew->GetAnchor()->FindFlyFrm();
/*N*/ 	if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
/*N*/ 	{
/*?*/ 		UINT32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1;
/*?*/ 		if ( pObj->GetPage() )
/*?*/ 			pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(), nNewNum);
/*?*/ 		else
/*?*/ 			pObj->SetOrdNum( nNewNum );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( FLY_IN_CNTNT == pNew->GetFmt()->GetAnchor().GetAnchorId() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ( !pSortedObjs )
/*N*/ 		pSortedObjs = new SwSortDrawObjs();
/*N*/ 	if ( !pSortedObjs->Insert( pObj ) )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/ 		USHORT nIdx;
/*N*/ 		ASSERT( pSortedObjs->Seek_Entry( pObj, &nIdx ),
/*N*/ 				"Fly nicht in Sorted eingetragen." );
/*N*/ #endif
/*N*/ 	}
/*N*/ 	pNew->ChgPage( this );
/*N*/ }

// OD 20.05.2003 #108784# - adding 'virtual' drawing object to page frame
void SwPageFrm::AppendVirtDrawObj( SwDrawContact* _pDrawContact,
                                   SwDrawVirtObj* _pDrawVirtObj )
{
    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    ASSERT( _pDrawVirtObj->GetAnchorFrm(), "virtual draw contact without anchor" );
    SwFlyFrm *pFly = _pDrawVirtObj->GetAnchorFrm()->FindFlyFrm();
    if ( pFly && _pDrawVirtObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        UINT32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect() + 1;
        if ( _pDrawVirtObj->GetPage() )
            _pDrawVirtObj->GetPage()->SetObjectOrdNum( _pDrawVirtObj->GetOrdNumDirect(), nNewNum);
        else
            _pDrawVirtObj->SetOrdNum( nNewNum );
    }

    if ( FLY_IN_CNTNT == _pDrawContact->GetFmt()->GetAnchor().GetAnchorId() )
    {
        return;
    }

    if ( !pSortedObjs )
    {
        pSortedObjs = new SwSortDrawObjs();
    }
    if ( !pSortedObjs->Insert( _pDrawVirtObj ) )
    {
#ifdef DBG_UTIL
        USHORT nIdx;
        ASSERT( pSortedObjs->Seek_Entry( _pDrawVirtObj, &nIdx ),
                "Fly nicht in Sorted eingetragen." );
#endif
    }
    _pDrawVirtObj->SetPageFrm( this );
}

/*N*/ void SwPageFrm::RemoveDrawObj( SwDrawContact *pToRemove )
/*N*/ {
/*N*/ 	//Auch Zeichengebundene muessen hier leider durchlaufen, weil beim
/*N*/ 	//setzen des Attributes zuerst das Attribut gesetzt und dann das Modify
/*N*/ 	//versendet wird.
/*N*/ 
/*N*/ 	//Die FlyColl kann bereits weg sein, weil der DTor der Seite
/*N*/ 	//gerade 'laeuft'
/*N*/ 	if ( pSortedObjs )
/*N*/ 	{
/*N*/ 		const SdrObjectPtr *pDel = pSortedObjs->GetData();
/*N*/ 		pSortedObjs->Remove( pToRemove->GetMaster() );
/*N*/ 		if ( !pSortedObjs->Count() )
/*N*/ 		{
/*N*/ 			DELETEZ( pSortedObjs );
/*N*/ 		}
/*N*/ 		if ( GetUpper() )
/*N*/ 		{
/*N*/ 			if ( FLY_IN_CNTNT != pToRemove->GetFmt()->GetAnchor().GetAnchorId() )
/*N*/ 			{
/*N*/ 				((SwRootFrm*)GetUpper())->SetSuperfluous();
/*N*/ 				InvalidatePage();
/*N*/ 			}
/*N*/ 			((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pToRemove->ChgPage( 0 );
/*N*/ }

// OD 20.05.2003 #108784# - remove 'virtual' drawing object from page frame.
void SwPageFrm::RemoveVirtDrawObj( SwDrawContact* _pDrawContact,
                                   SwDrawVirtObj* _pDrawVirtObj )
{
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( _pDrawVirtObj );
        if ( !pSortedObjs->Count() )
        {
            DELETEZ( pSortedObjs );
        }
        if ( GetUpper() )
        {
            if ( FLY_IN_CNTNT != _pDrawContact->GetFmt()->GetAnchor().GetAnchorId() )
            {
                ((SwRootFrm*)GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
        }
    }
    _pDrawVirtObj->SetPageFrm( 0 );
}

/*************************************************************************
|*
|*	SwPageFrm::PlaceFly
|*
|*	Ersterstellung		MA 08. Feb. 93
|*	Letzte Aenderung	MA 27. Feb. 93
|*
|*************************************************************************/

/*N*/ SwFrm *SwPageFrm::PlaceFly( SwFlyFrm *pFly, SwFrmFmt *pFmt,
/*N*/ 							const SwFmtAnchor *pAnch )
/*N*/ {
/*N*/ 	//Der Fly will immer an der Seite direkt haengen.
/*N*/ 	ASSERT( pAnch->GetAnchorId() == FLY_PAGE, "Unerwartete AnchorId." );
/*N*/ 
/*N*/ 	//Wenn ein Fly uebergeben wurde, so benutzen wir diesen, ansonsten wird
/*N*/ 	//mit dem Format einer erzeugt.
/*N*/ 	if ( pFly )
/*?*/ 		SwFrm::AppendFly( pFly );
/*N*/ 	else
/*N*/ 	{	ASSERT( pFmt, ":-( kein Format fuer Fly uebergeben." );
/*N*/ 		pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, this );
/*N*/ 		SwFrm::AppendFly( pFly );
/*N*/ 		::binfilter::RegistFlys( this, pFly );
/*N*/ 	}
/*N*/ 	return pFly;
/*N*/ }

/*************************************************************************
|*
|*	::CalcClipRect
|*
|*	Ersterstellung		AMA 24. Sep. 96
|*	Letzte Aenderung	MA  18. Dec. 96
|*
|*************************************************************************/

BOOL CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, BOOL bMove )
{
    BOOL bRet = TRUE;
    if ( pSdrObj->IsWriterFlyFrame() )
    {
        const SwFlyFrm *pFly = ((const SwVirtFlyDrawObj*)pSdrObj)->GetFlyFrm();
        if( pFly->IsFlyLayFrm() )
        {
#ifdef AMA_OUT_OF_FLY
            const SwFrm *pClip = pFly->GetAnchor()->FindPageFrm();
#else
            const SwFrm *pClip = pFly->GetAnchor();
#endif
            pClip->Calc();

            rRect = pClip->Frm();
            SWRECTFN( pClip )

            //Vertikales clipping: Top und Bottom, ggf. an PrtArea
            const SwFmtVertOrient &rV = pFly->GetFmt()->GetVertOrient();
            if( rV.GetVertOrient() != VERT_NONE &&
                rV.GetRelationOrient() == PRTAREA )
            {
                (rRect.*fnRect->fnSetTop)( (pClip->*fnRect->fnGetPrtTop)() );
                (rRect.*fnRect->fnSetBottom)( (pClip->*fnRect->fnGetPrtBottom)() );
            }
            //Horizontales clipping: Left und Right, ggf. an PrtArea
            const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
            if( rH.GetHoriOrient() != HORI_NONE &&
                rH.GetRelationOrient() == PRTAREA )
            {
                (rRect.*fnRect->fnSetLeft)( (pClip->*fnRect->fnGetPrtLeft)() );
                (rRect.*fnRect->fnSetRight)((pClip->*fnRect->fnGetPrtRight)());
            }
        }
        else if( pFly->IsFlyAtCntFrm() )
        {
            const SwFrm *pClip = pFly->GetAnchor();
            SWRECTFN( pClip )
            const SwLayoutFrm *pUp = pClip->GetUpper();
            const SwFrm *pCell = pUp->IsCellFrm() ? pUp : 0;
            USHORT nType = bMove ? FRM_ROOT   | FRM_FLY | FRM_HEADER |
                                   FRM_FOOTER | FRM_FTN
                                 : FRM_BODY   | FRM_FLY | FRM_HEADER |
                                   FRM_FOOTER | FRM_CELL| FRM_FTN;

            while ( !(pUp->GetType() & nType) || pUp->IsColBodyFrm() )
            {
                pUp = pUp->GetUpper();
                if ( !pCell && pUp->IsCellFrm() )
                    pCell = pUp;
            }
            if ( bMove )
            {
                if ( pUp->IsRootFrm() )
                {
                    rRect  = pUp->Prt();
                    rRect += pUp->Frm().Pos();
                    pUp = 0;
                }
            }
            if ( pUp )
            {
                if ( !pUp->IsFooterFrm() && ( !pUp->IsFlyFrm() ||
                     (!pUp->Lower() || !pUp->Lower()->IsColumnFrm()) ) )
                    pUp->Calc();
                if ( pUp->GetType() & FRM_BODY )
                {
                    const SwPageFrm *pPg;
                    if ( pUp->GetUpper() != (pPg = pFly->FindPageFrm()) )
                        pUp = pPg->FindBodyCont();
                    rRect = pUp->GetUpper()->Frm();
                    (rRect.*fnRect->fnSetTop)( (pUp->*fnRect->fnGetPrtTop)() );
                    (rRect.*fnRect->fnSetBottom)((pUp->*fnRect->fnGetPrtBottom)());
                }
                else
                {
                    if( ( pUp->GetType() & (FRM_FLY | FRM_FTN ) ) &&
                        !pUp->Frm().IsInside( pFly->Frm().Pos() ) )
                    {
                        if( pUp->IsFlyFrm() )
                        {
                            SwFlyFrm *pTmpFly = (SwFlyFrm*)pUp;
                            while( pTmpFly->GetNextLink() )
                            {
                                pTmpFly = pTmpFly->GetNextLink();
                                if( pTmpFly->Frm().IsInside( pFly->Frm().Pos() ) )
                                    break;
                            }
                            pUp = pTmpFly;
                        }
                        else if( pUp->IsInFtn() )
                        {
                            const SwFtnFrm *pTmp = pUp->FindFtnFrm();
                            while( pTmp->GetFollow() )
                            {
                                pTmp = pTmp->GetFollow();
                                if( pTmp->Frm().IsInside( pFly->Frm().Pos() ) )
                                    break;
                            }
                            pUp = pTmp;
                        }
                    }
                    rRect = pUp->Prt();
                    rRect.Pos() += pUp->Frm().Pos();
                    if ( pUp->GetType() & (FRM_HEADER | FRM_FOOTER) )
                    {
                        rRect.Left ( pUp->GetUpper()->Frm().Left() );
                        rRect.Width( pUp->GetUpper()->Frm().Width());
                    }
                    else if ( pUp->IsCellFrm() )				//MA_FLY_HEIGHT
                    {
                        const SwFrm *pTab = pUp->FindTabFrm();
                        (rRect.*fnRect->fnSetBottom)(
                                    (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                    }
                }
            }
            if ( pCell )
            {
                //CellFrms koennen auch in 'unerlaubten' Bereichen stehen, dann
                //darf der Fly das auch.
                SwRect aTmp( pCell->Prt() );
                aTmp += pCell->Frm().Pos();
                rRect.Union( aTmp );
            }
        }
        else
        {
            const SwFrm *pUp = pFly->GetAnchor()->GetUpper();
            if( !pUp->IsFooterFrm() )
                pUp->Calc();
            SWRECTFN( pFly->GetAnchor() )
            while( pUp->IsColumnFrm() || pUp->IsSctFrm() || pUp->IsColBodyFrm())
                pUp = pUp->GetUpper();
            rRect = pUp->Frm();
            if( !pUp->IsBodyFrm() )
            {
                rRect += pUp->Prt().Pos();
                rRect.SSize( pUp->Prt().SSize() );
                if ( pUp->IsCellFrm() )
                {
                    const SwFrm *pTab = pUp->FindTabFrm();
                    (rRect.*fnRect->fnSetBottom)(
                                    (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                }
            }
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SwFmt *pFmt = ((SwContact*)GetUserCall(pSdrObj))->GetFmt();
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            if( bMove )
            {
                nTop = bVert ? ((SwFlyInCntFrm*)pFly)->GetRefPoint().X() :
                               ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y();
                nTop = (*fnRect->fnYInc)( nTop, -nHeight );
                long nWidth = (pFly->Frm().*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y() :
                            ((SwFlyInCntFrm*)pFly)->GetRefPoint().X(), nWidth );
                nHeight = 2*nHeight - rUL.GetLower() - rUL.GetUpper();
            }
            else
            {
                nTop = (*fnRect->fnYInc)( (pFly->Frm().*fnRect->fnGetBottom)(),
                                           rUL.GetLower() - nHeight );
                nHeight = 2*nHeight - (pFly->Frm().*fnRect->fnGetHeight)()
                          - rUL.GetLower() - rUL.GetUpper();
            }
            (rRect.*fnRect->fnSetTopAndHeight)( nTop, nHeight );
        }
    }
    else
    {
        const SwDrawContact *pC = (const SwDrawContact*)GetUserCall(pSdrObj);
        const SwFrmFmt  *pFmt = (const SwFrmFmt*)pC->GetFmt();
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( FLY_IN_CNTNT == rAnch.GetAnchorId() )
        {
            const SwFrm *pFrm = pC->GetAnchor();
            if( !pFrm )
            {
                ((SwDrawContact*)pC)->ConnectToLayout();
                pFrm = pC->GetAnchor();
            }
            const SwFrm *pUp = pFrm->GetUpper();
            if( !pUp->IsFooterFrm() )
                pUp->Calc();
            rRect = pUp->Prt();
            rRect += pUp->Frm().Pos();
            SWRECTFN( pFrm )
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            SwRect aSnapRect( pSdrObj->GetSnapRect() );
            long nTmpH = 0;
            if( bMove )
            {
                nTop = (*fnRect->fnYInc)( bVert ? pSdrObj->GetAnchorPos().X() :
                                       pSdrObj->GetAnchorPos().Y(), -nHeight );
                long nWidth = (aSnapRect.*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            pSdrObj->GetAnchorPos().Y() :
                            pSdrObj->GetAnchorPos().X(), nWidth );
            }
            else
            {
                nTop = (*fnRect->fnYInc)( (aSnapRect.*fnRect->fnGetTop)(),
                                          rUL.GetLower() + nTmpH - nHeight );
                nTmpH = bVert ? pSdrObj->GetBoundRect().GetWidth() :
                                pSdrObj->GetBoundRect().GetHeight();
            }
            nHeight = 2*nHeight - nTmpH - rUL.GetLower() - rUL.GetUpper();
            (rRect.*fnRect->fnSetTopAndHeight)( nTop, nHeight );
        }
        else
        {
            // OD 23.06.2003 #108784# - restrict clip rectangle for drawing
            // objects in header/footer to the page frame.
            const SwFrm* pAnchorFrm = 0L;
            if ( pSdrObj->ISA(SwDrawVirtObj) )
            {
                pAnchorFrm = static_cast<const SwDrawVirtObj*>(pSdrObj)->GetAnchorFrm();
            }
            else
            {
                pAnchorFrm = pC->GetAnchor();
            }
            if ( pAnchorFrm && pAnchorFrm->FindFooterOrHeader() )
            {
                // clip frame is the page frame the header/footer is on.
                const SwFrm* pClipFrm = pAnchorFrm->FindPageFrm();
                rRect = pClipFrm->Frm();
            }
            else
            {
                bRet = FALSE;
            }
        }
    }
    return bRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
