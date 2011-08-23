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

#include <docary.hxx>
#include <bf_svtools/itemiter.hxx>

#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtanchr.hxx>
#include <fmtpdsc.hxx>

#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <ftninfo.hxx>
#include <tgrditem.hxx>

#include "viewimp.hxx"
#include "pagefrm.hxx"
#include "doc.hxx"
#include "fesh.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "hints.hxx"

#include "ftnidx.hxx"
#include "bodyfrm.hxx"
#include "ftnfrm.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "layact.hxx"
#include "flyfrms.hxx"
#include "frmsh.hxx"
#include "pagedesc.hxx"
#include <bf_svx/frmdiritem.hxx>
namespace binfilter {

/*************************************************************************
|*
|*	SwBodyFrm::SwBodyFrm()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 01. Aug. 93
|*
|*************************************************************************/
/*N*/ SwBodyFrm::SwBodyFrm( SwFrmFmt *pFmt ):
/*N*/ 	SwLayoutFrm( pFmt )
/*N*/ {
/*N*/     nType = FRMC_BODY;
/*N*/ }

/*************************************************************************
|*
|*	SwBodyFrm::Format()
|*
|*	Ersterstellung		MA 30. May. 94
|*	Letzte Aenderung	MA 20. Jan. 99
|*
|*************************************************************************/
/*N*/ void SwBodyFrm::Format( const SwBorderAttrs *pAttrs )
/*N*/ {
/*N*/ 	//Formatieren des Body ist zu einfach, deshalb bekommt er ein eigenes
/*N*/ 	//Format; Umrandungen und dergl. sind hier nicht zu beruecksichtigen.
/*N*/ 	//Breite ist die der PrtArea des Uppers, Hoehe ist die Hoehe der PrtArea
/*N*/ 	//des Uppers abzueglich der Nachbarn (Wird eigentlich eingestellt aber
/*N*/ 	//Vorsicht ist die Mutter der Robustheit).
/*N*/ 	//Die PrtArea ist stets so gross wie der Frm itself.
/*N*/ 
/*N*/ 	if ( !bValidSize )
/*N*/ 	{
/*N*/ 		SwTwips nHeight = GetUpper()->Prt().Height();
/*N*/         SwTwips nWidth = GetUpper()->Prt().Width();
/*N*/ 		const SwFrm *pFrm = GetUpper()->Lower();
/*N*/ 		do
/*N*/         {
/*N*/             if ( pFrm != this )
/*N*/             {
/*N*/                 if( pFrm->IsVertical() )
/*?*/                     nWidth -= pFrm->Frm().Width();
/*N*/                 else
/*N*/                     nHeight -= pFrm->Frm().Height();
/*N*/             }
/*N*/ 			pFrm = pFrm->GetNext();
/*N*/ 		} while ( pFrm );
/*N*/ 		if ( nHeight < 0 )
/*?*/ 			nHeight = 0;
/*N*/ 		Frm().Height( nHeight );
/*N*/         if( IsVertical() && !IsReverse() && nWidth != Frm().Width() )
/*?*/             Frm().Pos().X() += Frm().Width() - nWidth;
/*N*/         Frm().Width( nWidth );
/*N*/ 	}
/*N*/ 
/*N*/     BOOL bNoGrid = TRUE;
/*N*/     if( GetUpper()->IsPageFrm() && ((SwPageFrm*)GetUpper())->HasGrid() )
/*N*/     {
/*?*/         GETGRID( ((SwPageFrm*)GetUpper()) )
/*?*/         if( pGrid )
/*?*/         {
/*?*/             bNoGrid = FALSE;
/*?*/             long nSum = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
/*?*/             SWRECTFN( this )
/*?*/             long nSize = (Frm().*fnRect->fnGetWidth)();
/*?*/             long nBorder = 0;
/*?*/             if( GRID_LINES_CHARS == pGrid->GetGridType() )
/*?*/             {
/*?*/                 nBorder = nSize % pGrid->GetBaseHeight();
/*?*/                 nSize -= nBorder;
/*?*/                 nBorder /= 2;
/*?*/             }
/*?*/             (Prt().*fnRect->fnSetPosX)( nBorder );
/*?*/             (Prt().*fnRect->fnSetWidth)( nSize );
/*?*/             nBorder = (Frm().*fnRect->fnGetHeight)();
/*?*/             nSize = nBorder / nSum;
/*?*/             if( nSize > pGrid->GetLines() )
/*?*/                 nSize = pGrid->GetLines();
/*?*/             nSize *= nSum;
/*?*/             nBorder -= nSize;
/*?*/             nBorder /= 2;
/*?*/             (Prt().*fnRect->fnSetPosY)( nBorder );
/*?*/             (Prt().*fnRect->fnSetHeight)( nSize );
/*?*/         }
/*N*/     }
/*N*/     if( bNoGrid )
/*N*/     {
/*N*/         Prt().Pos().X() = Prt().Pos().Y() = 0;
/*N*/         Prt().Height( Frm().Height() );
/*N*/         Prt().Width( Frm().Width() );
/*N*/     }
/*N*/ 	bValidSize = bValidPrtArea = TRUE;
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::SwPageFrm(), ~SwPageFrm()
|*
|*	Ersterstellung		MA 20. Oct. 92
|*	Letzte Aenderung	MA 08. Dec. 97
|*
|*************************************************************************/
/*N*/ SwPageFrm::SwPageFrm( SwFrmFmt *pFmt, SwPageDesc *pPgDsc ) :
/*N*/ 	SwFtnBossFrm( pFmt ),
/*N*/ 	pSortedObjs( 0 ),
/*N*/ 	pDesc( pPgDsc ),
/*N*/ 	nPhyPageNum( 0 )
/*N*/ {
/*N*/     SetDerivedVert( FALSE );
/*N*/     SetDerivedR2L( FALSE );
/*N*/     if( pDesc )
/*N*/     {
/*N*/         bHasGrid = TRUE;
/*N*/         GETGRID( this )
/*N*/         if( !pGrid )
/*N*/             bHasGrid = FALSE;
/*N*/     }
/*N*/     else
/*?*/         bHasGrid = FALSE;
/*N*/ 	SetMaxFtnHeight( pPgDsc->GetFtnInfo().GetHeight() ?
/*N*/ 					 pPgDsc->GetFtnInfo().GetHeight() : LONG_MAX ),
/*N*/     nType = FRMC_PAGE;
/*N*/ 	bInvalidLayout = bInvalidCntnt = bInvalidSpelling = TRUE;
/*N*/ 	bInvalidFlyLayout = bInvalidFlyCntnt = bInvalidFlyInCnt =
/*N*/ 	bFtnPage = bEndNotePage = FALSE;
/*N*/ 
/*N*/ 	SwDoc *pDoc = pFmt->GetDoc();
/*N*/ 	if ( pDoc->IsBrowseMode() )
/*N*/ 	{
/*N*/ 		Frm().Height( 0 );
/*N*/ 		ViewShell *pSh = GetShell();
/*N*/ 		long nWidth = pSh ? pSh->VisArea().Width():0;
/*N*/ 		if ( !nWidth )
/*N*/ 			nWidth = 5000L;		//aendert sich sowieso
/*N*/ 		Frm().Width ( nWidth );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		Frm().SSize( pFmt->GetFrmSize().GetSize() );
/*N*/ 
/*N*/ 	//Body-Bereich erzeugen und einsetzen, aber nur wenn ich nicht gerade
/*N*/ 	//eine Leerseite bin.
/*N*/ 	if ( FALSE == (bEmptyPage = pFmt == pDoc->GetEmptyPageFmt()) )
/*N*/ 	{
/*N*/ 		bEmptyPage = FALSE;
/*N*/ 		Calc();								//Damit die PrtArea stimmt.
/*N*/ 		SwBodyFrm *pBodyFrm = new SwBodyFrm( pDoc->GetDfltFrmFmt() );
/*N*/ 		pBodyFrm->ChgSize( Prt().SSize() );
/*N*/ 		pBodyFrm->Paste( this );
/*N*/ 		pBodyFrm->Calc();					//Damit die Spalten korrekt
/*N*/ 											//eingesetzt werden koennen.
/*N*/ 		pBodyFrm->InvalidatePos();
/*N*/ 
/*N*/ 		if ( pDoc->IsBrowseMode() )
/*N*/ 			_InvalidateSize();		//Alles nur gelogen
/*N*/ 
/*N*/ 		//Header/Footer einsetzen, nur rufen wenn aktiv.
/*N*/ 		if ( pFmt->GetHeader().IsActive() )
/*N*/ 			PrepareHeader();
/*N*/ 		if ( pFmt->GetFooter().IsActive() )
/*N*/ 			PrepareFooter();
/*N*/ 
/*N*/ 		const SwFmtCol &rCol = pFmt->GetCol();
/*N*/ 		if ( rCol.GetNumCols() > 1 )
/*N*/ 		{
/*N*/ 			const SwFmtCol aOld; //ChgColumns() verlaesst sich darauf, dass ein
/*N*/ 								 //Old-Wert hereingereicht wird.
/*N*/ 			pBodyFrm->ChgColumns( aOld, rCol );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SwPageFrm::~SwPageFrm()
/*N*/ {
/*N*/ 	//FlyContainer entleeren, delete der Flys uebernimmt der Anchor
/*N*/ 	//(Basisklasse SwFrm)
/*N*/ 	if ( pSortedObjs )
/*N*/ 	{
/*N*/ 		//Objekte koennen (warum auch immer) auch an Seiten verankert sein,
/*N*/ 		//die vor Ihren Ankern stehen. Dann wuerde auf bereits freigegebenen
/*N*/ 		//Speicher zugegriffen.
/*N*/ 		for ( USHORT i = 0; i < pSortedObjs->Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pObj = (*pSortedObjs)[i];
/*N*/ 			if ( pObj->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 				if ( pFly->IsFlyFreeFrm() )
/*N*/ 					((SwFlyFreeFrm*)pFly)->SetPage ( 0 );
/*N*/ 			}
/*N*/ 			else if ( pObj->GetUserCall() )
/*N*/             {
/*N*/                 // OD 24.06.2003 #108784# - consider 'virtual' drawing objects
/*N*/                 if ( pObj->ISA(SwDrawVirtObj) )
/*N*/                 {
/*N*/                     SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
/*N*/                     pDrawVirtObj->SetPageFrm( 0 );
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     ((SwDrawContact*)pObj->GetUserCall())->ChgPage( 0 );
/*N*/                 }
/*N*/             }
/*N*/ 		}
/*N*/ 		delete pSortedObjs;
/*N*/ 		pSortedObjs = 0;		//Auf 0 setzen, sonst rauchts beim Abmdelden von Flys!
/*N*/ 	}
/*N*/ 
/*N*/ 	//Damit der Zugriff auf zerstoerte Seiten verhindert werden kann.
/*N*/ 	if ( !IsEmptyPage() ) //#59184# sollte fuer Leerseiten unnoetig sein.
/*N*/ 	{
/*N*/ 		SwDoc *pDoc = GetFmt()->GetDoc();
/*N*/ 		if( pDoc && !pDoc->IsInDtor() )
/*N*/ 		{
/*N*/ 			ViewShell *pSh = GetShell();
/*N*/ 			if ( pSh )
/*N*/ 			{
/*N*/ 				SwViewImp *pImp = pSh->Imp();
/*N*/ 				pImp->SetFirstVisPageInvalid();
/*N*/ 				if ( pImp->IsAction() )
/*N*/ 					pImp->GetLayAction().SetAgain();
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SwPageFrm::CheckGrid( BOOL bInvalidate )
/*N*/ {
/*N*/     BOOL bOld = bHasGrid;
/*N*/     bHasGrid = TRUE;
/*N*/     GETGRID( this )
/*N*/     bHasGrid = 0 != pGrid;
/*N*/     if( bInvalidate || bOld != bHasGrid )
/*N*/     {
/*?*/         SwLayoutFrm* pBody = FindBodyCont();
/*?*/         if( pBody )
/*?*/         {
/*?*/             pBody->InvalidatePrt();
/*?*/             SwCntntFrm* pFrm = pBody->ContainsCntnt();
/*?*/             while( pBody->IsAnLower( pFrm ) )
/*?*/             {
/*?*/                 ((SwTxtFrm*)pFrm)->Prepare( PREP_CLEAR );
/*?*/                 pFrm = pFrm->GetNextCntntFrm();
/*?*/             }
/*?*/         }
/*?*/         SetCompletePaint();
/*N*/     }
/*N*/ }


/*N*/ void SwPageFrm::CheckDirection( BOOL bVert )
/*N*/ {
/*N*/     UINT16 nDir =
/*N*/             ((SvxFrameDirectionItem&)GetFmt()->GetAttr( RES_FRAMEDIR )).GetValue();
/*N*/     if( bVert )
/*N*/     {
/*N*/         if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir ||
/*N*/             GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/             bVertical = 0;
/*N*/         else
/*N*/             bVertical = 1;
/*
        if( pDesc && pDesc->GetName().GetChar(0)=='x')
            bReverse = 1;
        else
 */
/*N*/             bReverse = 0;
/*N*/         bInvalidVert = 0;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         if( FRMDIR_HORI_RIGHT_TOP == nDir )
/*N*/             bRightToLeft = 1;
/*N*/         else
/*N*/             bRightToLeft = 0;
/*N*/         bInvalidR2L = 0;
/*N*/     }
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::PreparePage()
|*
|*	Beschreibung		Erzeugt die Spezifischen Flys zur Seite und formatiert
|* 		generischen Cntnt
|*	Ersterstellung		MA 20. Oct. 92
|*	Letzte Aenderung	MA 09. Nov. 95
|*
|*************************************************************************/
/*N*/ void MA_FASTCALL lcl_FormatLay( SwLayoutFrm *pLay )
/*N*/ {
/*N*/ 	//Alle LayoutFrms - nicht aber Tables, Flys o.ae. - formatieren.
/*N*/ 
/*N*/ 	SwFrm *pTmp = pLay->Lower();
/*N*/ 	//Erst die untergeordneten
/*N*/ 	while ( pTmp )
/*N*/ 	{
/*N*/ 		if ( pTmp->GetType() & 0x00FF )
/*N*/ 			::binfilter::lcl_FormatLay( (SwLayoutFrm*)pTmp );
/*N*/ 		pTmp = pTmp->GetNext();
/*N*/ 	}
/*N*/ 	pLay->Calc();
/*N*/ }

/*N*/ void MA_FASTCALL lcl_MakeObjs( const SwSpzFrmFmts &rTbl, SwPageFrm *pPage )
/*N*/ {
/*N*/ 	//Anlegen bzw. registrieren von Flys und Drawobjekten.
/*N*/ 	//Die Formate stehen in der SpzTbl (vom Dokument).
/*N*/ 	//Flys werden angelegt, DrawObjekte werden bei der Seite angemeldet.
/*N*/ 
/*N*/ 	for ( USHORT i = 0; i < rTbl.Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pSdrObj;
/*N*/ 		SwFrmFmt *pFmt = rTbl[i];
/*N*/ 		const SwFmtAnchor &rAnch = pFmt->GetAnchor();
/*N*/ 		if ( rAnch.GetPageNum() == pPage->GetPhyPageNum() )
/*N*/ 		{
/*N*/ 			if( rAnch.GetCntntAnchor() )
/*N*/ 			{
/*N*/ 				if( FLY_PAGE == rAnch.GetAnchorId() )
/*N*/ 				{
/*N*/ 					SwFmtAnchor aAnch( rAnch );
/*N*/ 					aAnch.SetAnchor( 0 );
/*N*/ 					pFmt->SetAttr( aAnch );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					continue;
/*N*/ 			}
/*N*/ 
/*N*/ 			//Wird ein Rahmen oder ein SdrObject beschrieben?
/*N*/ 			BOOL bSdrObj = RES_DRAWFRMFMT == pFmt->Which();
/*N*/ 			pSdrObj = 0;
/*N*/ 			if ( bSdrObj  && 0 == (pSdrObj = pFmt->FindSdrObject()) )
/*N*/ 			{
/*N*/ 				ASSERT( FALSE, "DrawObject not found." );
/*N*/ 				pFmt->GetDoc()->DelFrmFmt( pFmt );
/*N*/ 				--i;
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			//Das Objekt kann noch an einer anderen Seite verankert sein.
/*N*/ 			//Z.B. beim Einfuegen einer neuen Seite aufgrund eines
/*N*/ 			//Pagedescriptor-Wechsels. Das Objekt muss dann umgehaengt
/*N*/ 			//werden.
/*N*/ 			//Fuer bestimmte Faelle ist das Objekt bereits an der richtigen
/*N*/ 			//Seite verankert. Das wird hier automatisch erledigt und braucht
/*N*/ 			//- wenngleich performater machbar - nicht extra codiert werden.
/*N*/ 			SwPageFrm *pPg = pPage->IsEmptyPage() ? (SwPageFrm*)pPage->GetNext() : pPage;
/*N*/ 			if ( bSdrObj )
/*N*/ 			{
/*N*/                 // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
/*N*/                 if ( pSdrObj->ISA(SwDrawVirtObj) )
/*N*/                 {
/*N*/                     SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pSdrObj);
/*N*/                     SwDrawContact* pContact =
/*N*/                             static_cast<SwDrawContact*>(GetUserCall(&(pDrawVirtObj->GetReferencedObj())));
/*N*/                     if ( pContact )
/*N*/                     {
/*N*/                         pDrawVirtObj->RemoveFromWriterLayout();
/*N*/                         pDrawVirtObj->RemoveFromDrawingPage();
/*N*/                         pPg->SwFrm::AppendVirtDrawObj( pContact, pDrawVirtObj );
/*N*/                     }
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pSdrObj);
/*N*/                     if ( pContact->GetAnchor() )
/*N*/                         pContact->DisconnectFromLayout( false );
/*N*/                     pPg->SwFrm::AppendDrawObj( pContact );
/*N*/                 }
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwClientIter aIter( *pFmt );
/*N*/ 				SwClient *pTmp = aIter.First( TYPE(SwFrm) );
/*N*/ 				SwFlyFrm *pFly;
/*N*/ 				if ( pTmp )
/*N*/ 				{
/*N*/ 					pFly = (SwFlyFrm*)pTmp;
/*N*/ 					if( pFly->GetAnchor() )
/*N*/ 						pFly->GetAnchor()->RemoveFly( pFly );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, pPg );
/*N*/ 				pPg->SwFrm::AppendFly( pFly );
/*N*/ 				::binfilter::RegistFlys( pPg, pFly );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwPageFrm::PreparePage( BOOL bFtn )
/*N*/ {
/*N*/ 	SetFtnPage( bFtn );
/*N*/ 
/*N*/ 	//Klare Verhaeltnisse schaffen, sprich LayoutFrms der Seite formatieren.
/*N*/ 	if ( Lower() )
/*N*/ 		::binfilter::lcl_FormatLay( this );
/*N*/ 
/*N*/ 	//Vorhandene Flys bei der Seite anmelden.
/*N*/ 	::binfilter::RegistFlys( this, this );
/*N*/ 
/*N*/ 	//Flys und DrawObjekte die noch am Dokument bereitstehen.
/*N*/ 	//Fussnotenseiten tragen keine Seitengebundenen Flys!
/*N*/ 	//Es kann Flys und Objekte geben, die auf Leerseiten (Seitennummernmaessig)
/*N*/ 	//stehen wollen, diese werden jedoch von den Leerseiten ignoriert;
/*N*/ 	//sie werden von den Folgeseiten aufgenommen.
/*N*/ 	if ( !bFtn && !IsEmptyPage() )
/*N*/ 	{
/*N*/ 		SwDoc *pDoc = GetFmt()->GetDoc();
/*N*/ 
/*N*/ 		if ( GetPrev() && ((SwPageFrm*)GetPrev())->IsEmptyPage() )
/*N*/ 			lcl_MakeObjs( *pDoc->GetSpzFrmFmts(), (SwPageFrm*)GetPrev() );
/*N*/ 		lcl_MakeObjs( *pDoc->GetSpzFrmFmts(), this );
/*N*/ 
/*N*/ 		//Kopf-/Fusszeilen) formatieren.
/*N*/ 		SwLayoutFrm *pLow = (SwLayoutFrm*)Lower();
/*N*/ 		while ( pLow )
/*N*/ 		{
/*N*/ 			if ( pLow->GetType() & (FRMTYPE_HEADER|FRMTYPE_FOOTER) )
/*N*/ 			{
/*?*/ 				SwCntntFrm *pCntnt = pLow->ContainsCntnt();
/*?*/ 				while ( pCntnt && pLow->IsAnLower( pCntnt ) )
/*?*/ 				{
/*?*/ 					pCntnt->OptCalc();	//Nicht die Vorgaenger
/*?*/ 					pCntnt = pCntnt->GetNextCntntFrm();
/*?*/ 				}
/*N*/ 			}
/*N*/ 			pLow = (SwLayoutFrm*)pLow->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::Modify()
|*
|*	Ersterstellung		MA 20. Oct. 92
|*	Letzte Aenderung	MA 03. Mar. 96
|*
|*************************************************************************/
/*N*/ void SwPageFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
/*N*/ {
/*N*/ 	ViewShell *pSh = GetShell();
/*N*/ 	if ( pSh )
/*N*/ 		pSh->SetFirstVisPageInvalid();
/*N*/ 	BYTE nInvFlags = 0;
/*N*/ 
/*N*/ 	if( pNew && RES_ATTRSET_CHG == pNew->Which() )
/*N*/ 	{
/*?*/ 		SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
/*?*/ 		SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
/*?*/ 		SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
/*?*/ 		SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
/*?*/ 		while( TRUE )
/*?*/ 		{
/*?*/ 			_UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
/*?*/ 						 (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
/*?*/ 						 &aOldSet, &aNewSet );
/*?*/ 			if( aNIter.IsAtEnd() )
/*?*/ 				break;
/*?*/ 			aNIter.NextItem();
/*?*/ 			aOIter.NextItem();
/*?*/ 		}
/*?*/ 		if ( aOldSet.Count() || aNewSet.Count() )
/*?*/ 			SwLayoutFrm::Modify( &aOldSet, &aNewSet );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		_UpdateAttr( pOld, pNew, nInvFlags );
/*N*/ 
/*N*/ 	if ( nInvFlags != 0 )
/*N*/ 	{
/*N*/ 		InvalidatePage( this );
/*N*/ 		if ( nInvFlags & 0x01 )
/*N*/ 			_InvalidatePrt();
/*N*/ 		if ( nInvFlags & 0x02 )
/*N*/ 			SetCompletePaint();
/*N*/ 		if ( nInvFlags & 0x04 && GetNext() )
/*N*/ 			GetNext()->InvalidatePos();
/*N*/ 		if ( nInvFlags & 0x08 )
/*N*/ 			PrepareHeader();
/*N*/ 		if ( nInvFlags & 0x10 )
/*N*/ 			PrepareFooter();
/*N*/         if ( nInvFlags & 0x20 )
/*N*/             CheckGrid( nInvFlags & 0x40 );
/*N*/ 	}
/*N*/ }

/*N*/ void SwPageFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
/*N*/ 							 BYTE &rInvFlags,
/*N*/ 							 SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
/*N*/ {
/*N*/ 	BOOL bClear = TRUE;
/*N*/ 	const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 		case RES_FMT_CHG:
/*N*/ 		{
/*N*/ 			//Wenn sich das FrmFmt aendert kann hier einiges passieren.
/*N*/ 			//Abgesehen von den Grossenverhaeltnissen sind noch andere
/*N*/ 			//Dinge betroffen.
/*N*/ 			//1. Spaltigkeit.
/*N*/ 			ASSERT( pOld && pNew, "FMT_CHG Missing Format." );
/*N*/ 			const SwFmt* pOldFmt = ((SwFmtChg*)pOld)->pChangedFmt;
/*N*/ 			const SwFmt* pNewFmt = ((SwFmtChg*)pNew)->pChangedFmt;
/*N*/ 			ASSERT( pOldFmt && pNewFmt, "FMT_CHG Missing Format." );
/*N*/ 
/*N*/ 			const SwFmtCol &rOldCol = pOldFmt->GetCol();
/*N*/ 			const SwFmtCol &rNewCol = pNewFmt->GetCol();
/*N*/ 			if( rOldCol != rNewCol )
/*N*/ 			{
/*N*/ 				SwLayoutFrm *pB = FindBodyCont();
/*N*/ 				ASSERT( pB, "Seite ohne Body." );
/*N*/ 				pB->ChgColumns( rOldCol, rNewCol );
/*N*/                 rInvFlags |= 0x20;
/*N*/ 			}
/*N*/ 
/*N*/ 			//2. Kopf- und Fusszeilen.
/*N*/ 			const SwFmtHeader &rOldH = pOldFmt->GetHeader();
/*N*/ 			const SwFmtHeader &rNewH = pNewFmt->GetHeader();
/*N*/ 			if( rOldH != rNewH )
/*N*/ 				rInvFlags |= 0x08;
/*N*/ 
/*N*/ 			const SwFmtFooter &rOldF = pOldFmt->GetFooter();
/*N*/ 			const SwFmtFooter &rNewF = pNewFmt->GetFooter();
/*N*/ 			if( rOldF != rNewF )
/*N*/ 				rInvFlags |= 0x10;
/*N*/             CheckDirChange();
/*N*/ 		}
/*N*/ 			/* kein break hier */
/*N*/ 		case RES_FRM_SIZE:
/*N*/ 		{
/*N*/             const SwRect aOldPageFrmRect( Frm() );
/*N*/ 			if ( GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 			{
/*N*/                 bValidSize = FALSE;
/*N*/                 // OD 28.10.2002 #97265# - Don't call <SwPageFrm::MakeAll()>
/*N*/                 // Calculation of the page is not necessary, because its size is
/*N*/                 // is invalidated here and further invalidation is done in the
/*N*/                 // calling method <SwPageFrm::Modify(..)> and probably by calling
/*N*/                 // <SwLayoutFrm::Modify(..)> at the end.
/*N*/                 // It can also causes inconsistences, because the lowers are
/*N*/                 // adjusted, but not calculated, and a <SwPageFrm::MakeAll()> of
/*N*/                 // a next page is called. This is performed on the switch to the
/*N*/                 // online layout.
/*N*/                 //MakeAll();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				const SwFmtFrmSize &rSz = nWhich == RES_FMT_CHG ?
/*N*/ 						((SwFmtChg*)pNew)->pChangedFmt->GetFrmSize() :
/*N*/                         (const SwFmtFrmSize&)*pNew;
/*N*/ 
/*N*/ 				Frm().Height( Max( rSz.GetHeight(), long(MINLAY) ) );
/*N*/ 				Frm().Width ( Max( rSz.GetWidth(),	long(MINLAY) ) );
/*N*/                 AdjustRootSize( CHG_CHGPAGE, &aOldPageFrmRect );
/*N*/ 			}
/*N*/ 			//Window aufraeumen.
/*N*/ 			rInvFlags |= 0x03;
/*N*/             if ( aOldPageFrmRect.Height() != Frm().Height() )
/*N*/ 				rInvFlags |= 0x04;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 		case RES_COL:
/*N*/ 		{
/*N*/ 			SwLayoutFrm *pB = FindBodyCont();
/*N*/ 			ASSERT( pB, "Seite ohne Body." );
/*N*/ 			pB->ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
/*N*/             rInvFlags |= 0x22;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 		case RES_HEADER:
/*N*/ 			rInvFlags |= 0x08;
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_FOOTER:
/*N*/ 			rInvFlags |= 0x10;
/*N*/ 			break;
/*N*/         case RES_TEXTGRID:
/*N*/             rInvFlags |= 0x60;
/*N*/             break;
/*N*/ 
/*N*/ 		case RES_PAGEDESC_FTNINFO:
/*N*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //Die derzeit einzig sichere Methode:
/*N*/ 			break;
/*N*/         case RES_FRAMEDIR :
/*?*/             CheckDirChange();
/*N*/             break;
/*N*/ 
/*N*/ 		default:
/*N*/ 			bClear = FALSE;
/*N*/ 	}
/*N*/ 	if ( bClear )
/*N*/ 	{
/*N*/ 		if ( pOldSet || pNewSet )
/*N*/ 		{
/*?*/ 			if ( pOldSet )
/*?*/ 				pOldSet->ClearItem( nWhich );
/*?*/ 			if ( pNewSet )
/*?*/ 				pNewSet->ClearItem( nWhich );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			SwLayoutFrm::Modify( pOld, pNew );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*				  SwPageFrm::GetInfo()
|*
|*	  Beschreibung		erfragt Informationen
|*	  Ersterstellung	JP 31.03.94
|*	  Letzte Aenderung	JP 31.03.94
|*
*************************************************************************/
    // erfrage vom Modify Informationen

/*************************************************************************
|*
|*	SwPageFrm::SetPageDesc()
|*
|*	Ersterstellung		MA 02. Nov. 94
|*	Letzte Aenderung	MA 02. Nov. 94
|*
|*************************************************************************/
/*N*/ void  SwPageFrm::SetPageDesc( SwPageDesc *pNew, SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	pDesc = pNew;
/*N*/ 	if ( pFmt )
/*N*/ 		SetFrmFmt( pFmt );
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::FindPageDesc()
|*
|*	Beschreibung		Der richtige PageDesc wird bestimmt:
|*		0.	Vom Dokument bei Fussnotenseiten und Endnotenseiten
|* 		1.	vom ersten BodyCntnt unterhalb der Seite.
|* 		2.	vom PageDesc der vorstehenden Seite.
|* 		3.	bei Leerseiten vom PageDesc der vorigen Seite.
|* 		3.1 vom PageDesc der folgenden Seite wenn es keinen Vorgaenger gibt.
|* 		4.	es ist der Default-PageDesc sonst.
|*		5.  Im BrowseMode ist der Pagedesc immer der vom ersten Absatz im
|* 			Dokument oder Standard (der 0-te) wenn der erste Absatz keinen
|* 			wuenscht.
|*	   (6.  Im HTML-Mode ist der Pagedesc immer die HTML-Seitenvorlage.)
|*	Ersterstellung		MA 15. Feb. 93
|*	Letzte Aenderung	MA 17. Jun. 99
|*
|*************************************************************************/
/*N*/ SwPageDesc *SwPageFrm::FindPageDesc()
/*N*/ {
/*N*/ 	//0.
/*N*/ 	if ( IsFtnPage() )
/*N*/ 	{
/*?*/ 		SwDoc *pDoc = GetFmt()->GetDoc();
/*?*/ 		if ( IsEndNotePage() )
/*?*/ 			return pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
/*?*/ 		else
/*?*/ 			return pDoc->GetFtnInfo().GetPageDesc( *pDoc );
/*N*/ 	}
/*N*/ 
/*N*/ 	//6.
/*N*/ 	//if ( GetFmt()->GetDoc()->IsHTMLMode() )
/*N*/ 	//	return GetFmt()->GetDoc()->GetPageDescFromPool( RES_POOLPAGE_HTML );
/*N*/ 
/*N*/ 	SwPageDesc *pRet = 0;
/*N*/ 
/*N*/ 	//5.
/*N*/ 	if ( GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/ 	{
/*N*/ 		SwCntntFrm *pFrm = GetUpper()->ContainsCntnt();
/*N*/ 		while ( !pFrm->IsInDocBody() )
/*?*/ 			pFrm = pFrm->GetNextCntntFrm();
/*N*/ 		SwFrm *pFlow = pFrm;
/*N*/ 		if ( pFlow->IsInTab() )
/*?*/ 			pFlow = pFlow->FindTabFrm();
/*N*/ 		pRet = (SwPageDesc*)pFlow->GetAttrSet()->GetPageDesc().GetPageDesc();
/*N*/ 		if ( !pRet )
/*?*/ 			pRet = &GetFmt()->GetDoc()->_GetPageDesc( 0 );
/*N*/ 		return pRet;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwFrm *pFlow = FindFirstBodyCntnt();
/*N*/ 	if ( pFlow && pFlow->IsInTab() )
/*N*/ 		pFlow = pFlow->FindTabFrm();
/*N*/ 
/*N*/ 	//1.
/*N*/ 	if ( pFlow )
/*N*/ 	{
/*N*/ 		SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pFlow );
/*N*/ 		if ( !pTmp->IsFollow() )
/*N*/ 			pRet = (SwPageDesc*)pFlow->GetAttrSet()->GetPageDesc().GetPageDesc();
/*N*/ 	}
/*N*/ 
/*N*/ 	//3. und 3.1
/*N*/ 	if ( !pRet && IsEmptyPage() )
/*N*/ 		pRet = GetPrev() ? ((SwPageFrm*)GetPrev())->GetPageDesc()->GetFollow() :
/*N*/ 			   GetNext() ? ((SwPageFrm*)GetNext())->GetPageDesc() : 0;
/*N*/ 
/*N*/ 	//2.
/*N*/ 	if ( !pRet )
/*N*/ 		pRet = GetPrev() ?
/*N*/ 					((SwPageFrm*)GetPrev())->GetPageDesc()->GetFollow() : 0;
/*N*/ 
/*N*/ 	//4.
/*N*/ 	if ( !pRet )
/*N*/ 		pRet = (SwPageDesc*)&GetFmt()->GetDoc()->GetPageDesc( 0 );
/*N*/ 
/*N*/ 
/*N*/ 	ASSERT( pRet, "Kein Descriptor gefunden." );
/*N*/ 	return pRet;
/*N*/ }
/*************************************************************************
|*
|*	SwPageFrm::AdjustRootSize()
|*
|*	Ersterstellung		MA 13. Aug. 93
|*	Letzte Aenderung	MA 25. Jun. 95
|*
|*************************************************************************/
//Wenn der RootFrm seine Groesse aendert muss benachrichtigt werden.
/*N*/ void AdjustSizeChgNotify( SwRootFrm *pRoot )
/*N*/ {
/*N*/ 	const BOOL bOld = pRoot->IsSuperfluous();
/*N*/ 	pRoot->bCheckSuperfluous = FALSE;
/*N*/ 	ViewShell *pSh = pRoot->GetCurrShell();
/*N*/ 	if ( pSh )
/*N*/ 	{
/*N*/ 		pSh->Imp()->NotifySizeChg( pRoot->Frm().SSize() );//Einmal fuer das Drawing.
/*N*/ 		do
/*N*/ 		{	pSh->SizeChgNotify( pRoot->Frm().SSize() );	  //Einmal fuer jede Sicht.
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 		} while ( pSh != pRoot->GetCurrShell() );
/*N*/ 	}
/*N*/ 	pRoot->bCheckSuperfluous = bOld;
/*N*/ }

/*N*/ void MA_FASTCALL lcl_AdjustRoot( SwFrm *pPage, long nOld )
/*N*/ {
/*N*/ 	//Groesse der groessten Seite ermitteln.
/*N*/ 	//nOld enthaelt den alten Wert wenn die Seite geschrumpft ist und
/*N*/ 	//den aktuellen Wert wenn sie etwa ausgeschnitten wurde. Dadurch
/*N*/ 	//kann abgebrochen werden, wenn eine Seite gefunden wird, deren Wert
/*N*/ 	//dem alten entspricht.
/*N*/     long nMax = pPage->Frm().Width();
/*N*/ 	if ( nMax == nOld )
/*N*/ 		nMax = 0;
/*N*/ 	const SwFrm *pFrm = pPage->GetUpper()->Lower();
/*N*/ 	while ( pFrm )
/*N*/ 	{
/*N*/ 		if ( pFrm != pPage )
/*N*/ 		{
/*N*/             const SwTwips nTmp = pFrm->Frm().Width();
/*N*/ 			if ( nTmp == nOld )
/*N*/ 			{
/*N*/ 				nMax = 0;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			else if ( nTmp > nMax )
/*N*/ 				nMax = nTmp;
/*N*/ 		}
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/ 	}
/*N*/ 	if ( nMax )
/*N*/         pPage->GetUpper()->ChgSize( Size( nMax,
/*N*/                                           pPage->GetUpper()->Frm().Height() ) );
/*N*/ }

/*N*/ void SwPageFrm::AdjustRootSize( const SwPageChg eChgType, const SwRect *pOld )
/*N*/ {
/*N*/ 	if ( !GetUpper() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	const SwRect aOld( GetUpper()->Frm() );
/*N*/ 
/*N*/     const SwTwips nVar = Frm().Height();
/*N*/     SwTwips nFix = Frm().Width();
/*N*/ 	SwTwips nDiff = 0;
/*N*/ 
/*N*/ 	switch ( eChgType )
/*N*/ 	{
/*N*/ 		case CHG_NEWPAGE:
/*N*/ 			{
/*N*/                 if( nFix > GetUpper()->Prt().Width() )
/*N*/                     GetUpper()->ChgSize( Size(nFix,GetUpper()->Frm().Height()));
/*N*/ 				nDiff = nVar;
/*N*/ 				if ( GetPrev() && !((SwPageFrm*)GetPrev())->IsEmptyPage() )
/*N*/ 					nDiff += DOCUMENTBORDER/2;
/*N*/ 				else if ( !IsEmptyPage() && GetNext() )
/*N*/ 					nDiff += DOCUMENTBORDER/2;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case CHG_CUTPAGE:
/*N*/ 			{
/*N*/                 if ( nFix == GetUpper()->Prt().Width() )
/*N*/                     ::binfilter::lcl_AdjustRoot( this, nFix );
/*N*/ 				nDiff = -nVar;
/*N*/ 				if ( GetPrev() && !((SwPageFrm*)GetPrev())->IsEmptyPage() )
/*N*/ 					nDiff -= DOCUMENTBORDER/2;
/*N*/ 				else if ( !IsEmptyPage() && GetNext() )
/*N*/ 					nDiff -= DOCUMENTBORDER/2;
/*N*/ 				if ( IsEmptyPage() && GetNext() && GetPrev() )
/*N*/ 					nDiff = -nVar;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case CHG_CHGPAGE:
/*N*/ 			{
/*N*/ 				ASSERT( pOld, "ChgPage ohne OldValue nicht moeglich." );
/*N*/                 if ( pOld->Width() < nFix )
/*N*/ 				{
/*N*/                     if ( nFix > GetUpper()->Prt().Width() )
/*N*/                         GetUpper()->ChgSize( Size( nFix,
/*N*/                                                 GetUpper()->Frm().Height() ) );
/*N*/ 				}
/*N*/                 else if ( pOld->Width() > nFix )
/*N*/                     ::binfilter::lcl_AdjustRoot( this, pOld->Width() );
/*N*/                 nDiff = nVar - pOld->Height();
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		default:
/*?*/ 			ASSERT( FALSE, "Neuer Typ fuer PageChg." );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nDiff > 0 )
/*N*/         GetUpper()->Grow( nDiff );
/*N*/ 	else if ( nDiff < 0 )
/*N*/         GetUpper()->Shrink( -nDiff );
/*N*/ 
/*N*/ 	//Fix(8522): Calc auf die Root damit sich dir PrtArea sofort einstellt.
/*N*/ 	//Anderfalls gibt es Probleme wenn mehrere Aenderungen innerhalb einer
/*N*/ 	//Action laufen.
/*N*/ 	GetUpper()->Calc();
/*N*/ 
/*N*/ 	if ( aOld != GetUpper()->Frm() )
/*N*/ 	{
/*N*/ 		SwLayoutFrm *pUp = GetUpper();
/*N*/ 		if ( eChgType == CHG_CUTPAGE )
/*N*/ 		{
/*N*/ 			//Seiten vorher kurz aushaengen, weil sonst falsch formatiert wuerde.
/*N*/ 			SwFrm *pSibling = GetNext();
/*N*/ 			if ( ((SwRootFrm*)pUp)->GetLastPage() == this )
/*N*/ 				::binfilter::SetLastPage( (SwPageFrm*)GetPrev() );
/*N*/ 			Remove();
/*N*/ 			::binfilter::AdjustSizeChgNotify( (SwRootFrm*)pUp );
/*N*/ 			InsertBefore( pUp, pSibling );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			::binfilter::AdjustSizeChgNotify( (SwRootFrm*)pUp );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::Cut()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 22. Jun. 95
|*
|*************************************************************************/
/*N*/ inline void SetLastPage( SwPageFrm *pPage )
/*N*/ {
/*N*/ 			((SwRootFrm*)pPage->GetUpper())->pLastPage = pPage;
/*N*/ }

/*N*/ void SwPageFrm::Cut()
/*N*/ {
/*N*/     AdjustRootSize( CHG_CUTPAGE, 0 );
/*N*/ 
/*N*/ 	ViewShell *pSh = GetShell();
/*N*/ 	if ( !IsEmptyPage() )
/*N*/ 	{
/*N*/ 		if ( GetNext() )
/*N*/ 			GetNext()->InvalidatePos();
/*N*/ 
/*N*/ 		//Flys deren Anker auf anderen Seiten stehen umhaengen.
/*N*/ 		//DrawObjecte spielen hier keine Rolle.
/*N*/ 		if ( GetSortedObjs() )
/*N*/ 		{
/*N*/ 			for ( int i = 0; GetSortedObjs() &&
/*N*/ 							 (USHORT)i < GetSortedObjs()->Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pO = (*GetSortedObjs())[i];
/*N*/ 				SwFlyFrm *pFly;
/*N*/ 				if ( pO->IsWriterFlyFrame() &&
/*N*/ 					 (pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm())->IsFlyAtCntFrm() )
/*N*/ 				{
/*N*/ 					SwPageFrm *pAnchPage = pFly->GetAnchor() ?
/*N*/ 								pFly->GetAnchor()->FindPageFrm() : 0;
/*N*/ 					if ( pAnchPage && (pAnchPage != this) )
/*N*/ 					{
/*N*/ 						MoveFly( pFly, pAnchPage );
/*N*/ 						--i;
/*N*/ 						pFly->InvalidateSize();
/*N*/ 						pFly->_InvalidatePos();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		//Window aufraeumen
/*N*/ 		if ( pSh && pSh->GetWin() )
/*N*/ 			pSh->InvalidateWindows( Frm() );
/*N*/ 	}
/*N*/ 
/*N*/ 	// die Seitennummer der Root runterzaehlen.
/*N*/ 	((SwRootFrm*)GetUpper())->DecrPhyPageNums();
/*N*/ 	SwPageFrm *pPg = (SwPageFrm*)GetNext();
/*N*/ 	if ( pPg )
/*N*/ 	{
/*N*/ 		while ( pPg )
/*N*/ 		{
/*N*/ 			pPg->DecrPhyPageNum();	//inline --nPhyPageNum
/*N*/ 			pPg = (SwPageFrm*)pPg->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		::binfilter::SetLastPage( (SwPageFrm*)GetPrev() );
/*N*/ 
/*N*/ 	// Alle Verbindungen kappen.
/*N*/ 	Remove();
/*N*/ 	if ( pSh )
/*N*/ 		pSh->SetFirstVisPageInvalid();
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::Paste()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 07. Dec. 94
|*
|*************************************************************************/
/*N*/ void SwPageFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
/*N*/ {
/*N*/ 	ASSERT( pParent->IsRootFrm(), "Parent ist keine Root." );
/*N*/ 	ASSERT( pParent, "Kein Parent fuer Paste." );
/*N*/ 	ASSERT( pParent != this, "Bin selbst der Parent." );
/*N*/ 	ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
/*N*/ 	ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
/*N*/ 			"Bin noch irgendwo angemeldet." );
/*N*/ 
/*N*/ 	//In den Baum einhaengen.
/*N*/ 	InsertBefore( (SwLayoutFrm*)pParent, pSibling );
/*N*/ 
/*N*/ 	// die Seitennummer am Root hochzaehlen.
/*N*/ 	((SwRootFrm*)GetUpper())->IncrPhyPageNums();
/*N*/ 	if( GetPrev() )
/*N*/ 		SetPhyPageNum( ((SwPageFrm*)GetPrev())->GetPhyPageNum() + 1 );
/*N*/ 	else
/*N*/ 		SetPhyPageNum( 1 );
/*N*/ 	SwPageFrm *pPg = (SwPageFrm*)GetNext();
/*N*/ 	if ( pPg )
/*N*/ 	{
/*N*/ 		while ( pPg )
/*N*/ 		{
/*N*/ 			pPg->IncrPhyPageNum();	//inline ++nPhyPageNum
/*N*/ 			pPg->_InvalidatePos();
/*N*/ 			pPg->InvalidateLayout();
/*N*/ 			pPg = (SwPageFrm*)pPg->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		::binfilter::SetLastPage( this );
/*N*/ 
/*N*/     if( Frm().Width() != pParent->Prt().Width() )
/*N*/ 		_InvalidateSize();
/*N*/ 	InvalidatePos();
/*N*/ 
/*N*/     AdjustRootSize( CHG_NEWPAGE, 0 );
/*N*/ 
/*N*/ 	ViewShell *pSh = GetShell();
/*N*/ 	if ( pSh )
/*N*/ 		pSh->SetFirstVisPageInvalid();
/*N*/ }

/*************************************************************************
|*
|*	SwPageFrm::PrepareRegisterChg()
|*
|*	Ersterstellung		AMA 22. Jul. 96
|*	Letzte Aenderung	AMA 22. Jul. 96
|*
|*************************************************************************/
/*N*/ void lcl_PrepFlyInCntRegister( SwCntntFrm *pFrm )
/*N*/ {
/*N*/ 	pFrm->Prepare( PREP_REGISTER );
/*N*/ 	if( pFrm->GetDrawObjs() )
/*N*/ 	{
/*N*/ 		for( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
/*N*/ 		{
/*N*/ 			SwFlyFrm *pFly;
/*N*/ 			SdrObject *pO = (*pFrm->GetDrawObjs())[i];
/*N*/ 			if( pO->IsWriterFlyFrame() &&
/*N*/ 				0 != (pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()) &&
/*N*/ 				pFly->IsFlyInCntFrm() )
/*N*/ 			{
/*N*/ 				SwCntntFrm *pCnt = pFly->ContainsCntnt();
/*N*/ 				while ( pCnt )
/*N*/ 				{
/*N*/ 					lcl_PrepFlyInCntRegister( pCnt );
/*N*/ 					pCnt = pCnt->GetNextCntntFrm();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwPageFrm::PrepareRegisterChg()
/*N*/ {
/*N*/ 	SwCntntFrm *pFrm = FindFirstBodyCntnt();
/*N*/ 	while( pFrm )
/*N*/ 	{
/*N*/ 		lcl_PrepFlyInCntRegister( pFrm );
/*N*/ 		pFrm = pFrm->GetNextCntntFrm();
/*N*/ 		if( !IsAnLower( pFrm ) )
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	if( GetSortedObjs() )
/*N*/ 	{
/*N*/ 		for( USHORT i = 0; i < GetSortedObjs()->Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pO = (*GetSortedObjs())[i];
/*N*/ 			if ( pO->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 				pFrm = pFly->ContainsCntnt();
/*N*/ 				while ( pFrm )
/*N*/ 				{
/*N*/ 					::binfilter::lcl_PrepFlyInCntRegister( pFrm );
/*N*/ 					pFrm = pFrm->GetNextCntntFrm();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::CheckPageDescs()
|*
|*	Beschreibung		Prueft alle Seiten ab der uebergebenen, daraufhin,
|* 		ob sie das richtige FrmFmt verwenden. Wenn 'falsche' Seiten
|*		aufgespuehrt werden, so wird versucht die Situation moeglichst
|* 		einfache zu bereinigen.
|*
|*	Ersterstellung		MA 10. Feb. 93
|*	Letzte Aenderung	MA 18. Apr. 96
|*
|*************************************************************************/
/*N*/ void SwFrm::CheckPageDescs( SwPageFrm *pStart, BOOL bNotifyFields )
/*N*/ {
/*N*/ 	ASSERT( pStart, "Keine Startpage." );
/*N*/ 
/*N*/ 	ViewShell *pSh	 = pStart->GetShell();
/*N*/ 	SwViewImp *pImp  = pSh ? pSh->Imp() : 0;
/*N*/ 
/*N*/ 	if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
/*N*/ 	{
/*?*/ 		pImp->GetLayAction().SetCheckPageNum( pStart->GetPhyPageNum() );
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	//Fuer das Aktualisieren der Seitennummern-Felder gibt nDocPos
/*N*/ 	//die Seitenposition an, _ab_ der invalidiert werden soll.
/*N*/ 	SwTwips nDocPos  = LONG_MAX;
/*N*/ 
/*N*/ 	SwRootFrm *pRoot = (SwRootFrm*)pStart->GetUpper();
/*N*/ 	SwDoc* pDoc		 = pStart->GetFmt()->GetDoc();
/*N*/ 	const BOOL bFtns = 0 != pDoc->GetFtnIdxs().Count();
/*N*/ 
/*N*/ 	SwPageFrm *pPage = pStart;
/*N*/ 	if( pPage->GetPrev() && ((SwPageFrm*)pPage->GetPrev())->IsEmptyPage() )
/*?*/ 		pPage = (SwPageFrm*)pPage->GetPrev();
/*N*/ 	while ( pPage )
/*N*/ 	{
/*N*/ 		//gewuenschten PageDesc und FrmFmt festellen.
/*N*/ 		SwPageDesc *pDesc = pPage->FindPageDesc();
/*N*/ 		BOOL bCheckEmpty = pPage->IsEmptyPage();
/*N*/ 		BOOL bActOdd = pPage->OnRightPage();
/*N*/ 		BOOL bOdd = pPage->WannaRightPage();
/*N*/ 		SwFrmFmt *pFmtWish = bOdd ? pDesc->GetRightFmt()
/*N*/ 								  : pDesc->GetLeftFmt();
/*N*/ 
/*N*/ 		if ( bActOdd != bOdd ||
/*N*/ 			 pDesc != pPage->GetPageDesc() ||		//falscher Desc
/*N*/              ( pFmtWish != pPage->GetFmt()  &&      //falsches Format und
/*N*/                ( !pPage->IsEmptyPage() || pFmtWish ) //nicht Leerseite
/*N*/              )
/*N*/            )
/*N*/ 		{
/*N*/ 			//Wenn wir schon ein Seite veraendern muessen kann das eine
/*N*/ 			//Weile dauern, deshalb hier den WaitCrsr pruefen.
/*N*/ 			if( pImp )
/*N*/ 				pImp->CheckWaitCrsr();
/*N*/ 
/*N*/ 			//Ab hier muessen die Felder invalidiert werden!
/*N*/ 			if ( nDocPos == LONG_MAX )
/*N*/ 				nDocPos = pPage->GetPrev() ?
/*N*/ 							pPage->GetPrev()->Frm().Top() :	pPage->Frm().Top();
/*N*/ 
/*N*/ 			//Faelle:
/*N*/ 			//1. Wir haben eine EmptyPage und wollen eine "Normalseite".
/*N*/ 			//		->EmptyPage wegwerfen und weiter mit der naechsten.
/*N*/ 			//2. Wir haben eine EmptyPage und wollen eine EmptyPage mit
/*N*/ 			//	 anderem Descriptor.
/*N*/ 			//		->Descriptor austauschen.
/*N*/ 			//3. Wir haben eine "Normalseite" und wollen eine EmptyPage.
/*N*/ 			//		->Emptypage einfuegen, nicht aber wenn die Vorseite
/*N*/ 			//							   bereits eine EmptyPage ist -> 6.
/*N*/ 			//4. Wir haben eine "Normalseite" und wollen eine "Normalseite"
/*N*/ 			//	 mit anderem Descriptor
/*N*/ 			//		->Descriptor und Format austauschen
/*N*/ 			//5. Wir haben eine "Normalseite" und wollen eine "Normalseite"
/*N*/ 			//	 mit anderem Format
/*N*/ 			//		->Format austauschen.
/*N*/ 			//6. Wir haben kein Wunschformat erhalten, also nehmen wir das
/*N*/ 			//	 'andere' Format (rechts/links) des PageDesc.
/*N*/ 
/*N*/ 			if ( pPage->IsEmptyPage() && ( pFmtWish ||			//1.
/*N*/ 				 ( !bOdd && !pPage->GetPrev() ) ) )
/*N*/ 			{
/*N*/ 				SwPageFrm *pTmp = (SwPageFrm*)pPage->GetNext();
/*N*/ 				pPage->Cut();
/*N*/ 				delete pPage;
/*N*/ 				if ( pStart == pPage )
/*?*/ 					pStart = pTmp;
/*N*/ 				pPage = pTmp;
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			else if ( pPage->IsEmptyPage() && !pFmtWish &&	//2.
/*N*/ 					  pDesc != pPage->GetPageDesc() )
/*N*/ 			{
/*N*/ 				pPage->SetPageDesc( pDesc, 0 );
/*N*/ 			}
/*N*/             else if ( !pPage->IsEmptyPage() &&      //3.
/*N*/                       bActOdd != bOdd &&
/*N*/                       ( ( !pPage->GetPrev() && !bOdd ) ||
/*N*/ 						( pPage->GetPrev() &&
/*N*/                           !((SwPageFrm*)pPage->GetPrev())->IsEmptyPage() )
/*N*/                       )
/*N*/                     )
/*N*/ 			{
/*N*/ 				if ( pPage->GetPrev() )
/*N*/ 					pDesc = ((SwPageFrm*)pPage->GetPrev())->GetPageDesc();
/*N*/ 				SwPageFrm *pTmp = new SwPageFrm( pDoc->GetEmptyPageFmt(),pDesc);
/*N*/ 				pTmp->Paste( pRoot, pPage );
/*N*/ 				pTmp->PreparePage( FALSE );
/*N*/ 				pPage = pTmp;
/*N*/ 			}
/*N*/ 			else if ( pPage->GetPageDesc() != pDesc )			//4.
/*N*/ 			{
/*N*/ 				SwPageDesc *pOld = pPage->GetPageDesc();
/*N*/ 				pPage->SetPageDesc( pDesc, pFmtWish );
/*N*/ 				if ( bFtns )
/*N*/ 				{
/*?*/ 					//Wenn sich bestimmte Werte der FtnInfo veraendert haben
/*?*/ 					//muss etwas passieren. Wir versuchen den Schaden zu
/*?*/ 					//begrenzen.
/*?*/ 					//Wenn die Seiten keinen FtnCont hat, ist zwar theoretisches
/*?*/ 					//ein Problem denkbar, aber das ignorieren wir mit aller Kraft.
/*?*/ 					//Bei Aenderungen hoffen wir mal, dass eine Invalidierung
/*?*/ 					//ausreicht, denn alles andere wuerde viel Kraft kosten.
/*?*/ 					SwFtnContFrm *pCont = pPage->FindFtnCont();
/*?*/ 					if ( pCont && !(pOld->GetFtnInfo() == pDesc->GetFtnInfo()) )
/*?*/ 						pCont->_InvalidateAll();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( pFmtWish && pPage->GetFmt() != pFmtWish )			//5.
/*N*/ 			{
/*N*/ 				pPage->SetFrmFmt( pFmtWish );
/*N*/ 			}
/*N*/ 			else if ( !pFmtWish )										//6.
/*N*/ 			{
/*N*/ 				//Format mit verdrehter Logic besorgen.
/*N*/ 				pFmtWish = bOdd ? pDesc->GetLeftFmt() : pDesc->GetRightFmt();
/*N*/ 				if ( pPage->GetFmt() != pFmtWish )
/*N*/ 					pPage->SetFrmFmt( pFmtWish );
/*N*/ 			}
/*N*/ #ifdef DBG_UTIL
/*N*/ 			else
/*N*/ 			{
/*?*/ 				ASSERT( FALSE, "CheckPageDescs, missing solution" );
/*N*/ 			}
/*N*/ #endif
/*N*/ 		}
/*N*/ 		if ( bCheckEmpty )
/*N*/ 		{
/*N*/ 			//Es kann noch sein, dass die Leerseite schlicht  ueberflussig ist.
/*N*/ 			//Obiger Algorithmus kann dies leider nicht feststellen.
/*N*/ 			//Eigentlich muesste die Leerseite einfach praeventiv entfernt
/*N*/ 			//werden; sie wuerde ja ggf. wieder eingefuegt.
/*N*/ 			//Die EmptyPage ist genau dann ueberfluessig, wenn die Folgeseite
/*N*/ 			//auch ohne sie auskommt. Dazu muessen wir uns die Verhaeltnisse
/*N*/ 			//genauer ansehen. Wir bestimmen den PageDesc und die virtuelle
/*N*/ 			//Seitennummer manuell.
/*N*/ 			SwPageFrm *pPg = (SwPageFrm*)pPage->GetNext();
/*N*/ 			if( !pPg || pPage->OnRightPage() == pPg->WannaRightPage() )
/*N*/ 			{
/*?*/ 				//Die Folgeseite hat kein Problem ein FrmFmt zu finden oder keinen
/*?*/ 				//Nachfolger, also ist die Leerseite ueberfluessig.
/*?*/ 				SwPageFrm *pTmp = (SwPageFrm*)pPage->GetNext();
/*?*/ 				pPage->Cut();
/*?*/ 				delete pPage;
/*?*/ 				if ( pStart == pPage )
/*?*/ 					pStart = pTmp;
/*?*/ 				pPage = pTmp;
/*?*/ 				continue;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	pRoot->SetAssertFlyPages();
/*N*/ 	pRoot->AssertPageFlys( pStart );
/*N*/ 
/*N*/ 	if ( bNotifyFields && (!pImp || !pImp->IsUpdateExpFlds()) )
/*N*/ 	{
/*N*/ 		SwDocPosUpdate aMsgHnt( nDocPos );
/*N*/ 		pDoc->UpdatePageFlds( &aMsgHnt );
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	//Ein paar Pruefungen muessen schon erlaubt sein.
/*N*/ 
/*N*/ 	//1. Keine zwei EmptyPages hintereinander.
/*N*/ 	//2. Alle PageDescs richtig?
/*N*/ 	BOOL bEmpty = FALSE;
/*N*/ 	SwPageFrm *pPg = pStart;
/*N*/ 	while ( pPg )
/*N*/ 	{
/*N*/ 		if ( pPg->IsEmptyPage() )
/*N*/ 		{
/*N*/ 			if ( bEmpty )
/*N*/ 			{
/*?*/ 				ASSERT( FALSE, "Doppelte Leerseiten." );
/*?*/ 				break;	//Einmal reicht.
/*N*/ 			}
/*N*/ 			bEmpty = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bEmpty = FALSE;
/*N*/ 
/*N*/ //MA 21. Jun. 95: Kann zu testzwecken 'rein, ist aber bei zyklen durchaus
/*N*/ //moeglich: Ein paar Seiten, auf der ersten 'erste Seite' anwenden,
/*N*/ //rechte als folge der ersten, linke als folge der rechten, rechte als
/*N*/ //folge der linken.
/*N*/ //		ASSERT( pPg->GetPageDesc() == pPg->FindPageDesc(),
/*N*/ //				"Seite mit falschem Descriptor." );
/*N*/ 
/*N*/ 		pPg = (SwPageFrm*)pPg->GetNext();
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::InsertPage()
|*
|*	Beschreibung
|*	Ersterstellung		MA 10. Feb. 93
|*	Letzte Aenderung	MA 27. Jul. 93
|*
|*************************************************************************/
/*N*/ SwPageFrm *SwFrm::InsertPage( SwPageFrm *pPrevPage, BOOL bFtn )
/*N*/ {
/*N*/ 	SwRootFrm *pRoot = (SwRootFrm*)pPrevPage->GetUpper();
/*N*/ 	SwPageFrm *pSibling = (SwPageFrm*)pRoot->GetLower();
/*N*/ 	SwPageDesc *pDesc = pSibling->GetPageDesc();
/*N*/ 
/*N*/ 	pSibling = (SwPageFrm*)pPrevPage->GetNext();
/*N*/ 		//Rechte (ungerade) oder linke (gerade) Seite einfuegen?
/*N*/ 	BOOL bNextOdd = !pPrevPage->OnRightPage();
/*N*/ 	BOOL bWishedOdd = bNextOdd;
/*N*/ 
/*N*/ 	//Welcher PageDesc gilt?
/*N*/ 	//Bei CntntFrm der aus dem Format wenn einer angegeben ist,
/*N*/ 	//der Follow vom bereits in der PrevPage gueltigen sonst.
/*N*/ 	pDesc = 0;
/*N*/ 	if ( IsFlowFrm() && !SwFlowFrm::CastFlowFrm( this )->IsFollow() )
/*N*/ 	{	SwFmtPageDesc &rDesc = (SwFmtPageDesc&)GetAttrSet()->GetPageDesc();
/*N*/ 		pDesc = rDesc.GetPageDesc();
/*N*/ 		if ( rDesc.GetNumOffset() )
/*N*/ 		{
/*N*/ 			bWishedOdd = rDesc.GetNumOffset() % 2 ? TRUE : FALSE;
/*N*/ 			//Die Gelegenheit nutzen wir um das Flag an der Root zu pflegen.
/*N*/ 			pRoot->SetVirtPageNum( TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !pDesc )
/*N*/ 		pDesc = pPrevPage->GetPageDesc()->GetFollow();
/*N*/ 
/*N*/ 	ASSERT( pDesc, "Missing PageDesc" );
/*N*/ 	if( !(bWishedOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
/*N*/ 		bWishedOdd = !bWishedOdd;
/*N*/ 
/*N*/ 	SwDoc *pDoc = pPrevPage->GetFmt()->GetDoc();
/*N*/ 	SwFrmFmt *pFmt;
/*N*/ 	BOOL bCheckPages = FALSE;
/*N*/ 	//Wenn ich kein FrmFmt fuer die Seite gefunden habe, muss ich eben eine
/*N*/ 	//Leerseite einfuegen.
/*N*/ 	if( bWishedOdd != bNextOdd )
/*N*/ 	{	pFmt = pDoc->GetEmptyPageFmt();
/*N*/ 		SwPageDesc *pTmpDesc = pPrevPage->GetPageDesc();
/*N*/ 		SwPageFrm *pPage = new SwPageFrm( pFmt, pTmpDesc );
/*N*/ 		pPage->Paste( pRoot, pSibling );
/*N*/ 		pPage->PreparePage( bFtn );
/*N*/ 		//Wenn der Sibling keinen Bodytext enthaelt kann ich ihn vernichten
/*N*/ 		//Es sei denn, es ist eine Fussnotenseite
/*N*/ 		if ( pSibling && !pSibling->IsFtnPage() &&
/*N*/ 			 !pSibling->FindFirstBodyCntnt() )
/*N*/ 		{
/*N*/ 			SwPageFrm *pDel = pSibling;
/*N*/ 			pSibling = (SwPageFrm*)pSibling->GetNext();
/*N*/ 			if ( pDoc->GetFtnIdxs().Count() )
/*?*/ 				pRoot->RemoveFtns( pDel, TRUE );
/*N*/ 			pDel->Cut();
/*N*/ 			delete pDel;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bCheckPages = TRUE;
/*N*/ 	}
/*N*/ 	pFmt = bWishedOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt();
/*N*/ 	ASSERT( pFmt, "Descriptor without format." );
/*N*/ 	SwPageFrm *pPage = new SwPageFrm( pFmt, pDesc );
/*N*/ 	pPage->Paste( pRoot, pSibling );
/*N*/ 	pPage->PreparePage( bFtn );
/*N*/ 	//Wenn der Sibling keinen Bodytext enthaelt kann ich ihn vernichten
/*N*/ 	//Es sei denn es ist eine Fussnotenseite.
/*N*/ 	if ( pSibling && !pSibling->IsFtnPage() &&
/*N*/ 		 !pSibling->FindFirstBodyCntnt() )
/*N*/ 	{
/*N*/ 		SwPageFrm *pDel = pSibling;
/*N*/ 		pSibling = (SwPageFrm*)pSibling->GetNext();
/*N*/ 		if ( pDoc->GetFtnIdxs().Count() )
/*?*/ 			pRoot->RemoveFtns( pDel, TRUE );
/*N*/ 		pDel->Cut();
/*N*/ 		delete pDel;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bCheckPages = TRUE;
/*N*/ 
/*N*/ 	if ( pSibling )
/*N*/ 	{
/*N*/ 		if ( bCheckPages )
/*N*/ 		{
/*N*/ 			CheckPageDescs( pSibling, FALSE );
/*N*/ 			ViewShell *pSh = GetShell();
/*N*/ 			SwViewImp *pImp = pSh ? pSh->Imp() : 0;
/*N*/ 			if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
/*N*/ 			{
/*?*/ 				const USHORT nNum = pImp->GetLayAction().GetCheckPageNum();
/*?*/ 				if ( nNum == pPrevPage->GetPhyPageNum() + 1 )
/*?*/ 					pImp->GetLayAction().SetCheckPageNumDirect(
/*?*/ 													pSibling->GetPhyPageNum() );
/*?*/ 				return pPage;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pRoot->AssertPageFlys( pSibling );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Fuer das Aktualisieren der Seitennummern-Felder gibt nDocPos
/*N*/ 	//die Seitenposition an, _ab_ der invalidiert werden soll.
/*N*/ 	ViewShell *pSh = GetShell();
/*N*/ 	if ( !pSh || !pSh->Imp()->IsUpdateExpFlds() )
/*N*/ 	{
/*N*/ 		SwDocPosUpdate aMsgHnt( pPrevPage->Frm().Top() );
/*N*/ 		pDoc->UpdatePageFlds( &aMsgHnt );
/*N*/ 	}
/*N*/ 	return pPage;
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::GrowFrm()
|*
|*	Ersterstellung		MA 30. Jul. 92
|*	Letzte Aenderung	MA 05. May. 94
|*
|*************************************************************************/

/*N*/ SwTwips SwRootFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/ 	if ( !bTst )
/*N*/         Frm().SSize().Height() += nDist;
/*N*/ 	return nDist;
/*N*/ }
/*************************************************************************
|*
|*	SwRootFrm::ShrinkFrm()
|*
|*	Ersterstellung		MA 30. Jul. 92
|*	Letzte Aenderung	MA 05. May. 94
|*
|*************************************************************************/
/*N*/ SwTwips SwRootFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/ 	ASSERT( nDist >= 0, "nDist < 0." );
/*N*/     ASSERT( nDist <= Frm().Height(), "nDist > als aktuelle Groesse." );
/*N*/ 
/*N*/ 	if ( !bTst )
/*N*/         Frm().SSize().Height() -= nDist;
/*N*/ 	return nDist;
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::RemoveSuperfluous()
|*
|*	Beschreibung:		Entfernung von ueberfluessigen Seiten.
|*			Arbeitet nur wenn das Flag bCheckSuperfluous gesetzt ist.
|*			Definition: Eine Seite ist genau dann leer, wenn der
|*			Body-Textbereich keinen CntntFrm enthaelt, aber nicht, wenn noch
|* 			mindestens ein Fly an der Seite klebt.
|* 			Die Seite ist auch dann nicht leer, wenn sie noch eine
|* 			Fussnote enthaelt.
|*			Es muss zweimal angesetzt werden um leeren Seiten aufzuspueren:
|* 				- einmal fuer die Endnotenseiten.
|* 				- und einmal fuer die Seiten des Bodytextes.
|*
|*	Ersterstellung		MA 20. May. 92
|*	Letzte Aenderung	MA 10. Jan. 95
|*
|*************************************************************************/
void SwRootFrm::RemoveSuperfluous()
{
    if ( !IsSuperfluous() )
        return;
    bCheckSuperfluous = FALSE;

    SwPageFrm *pPage = GetLastPage();
    long nDocPos = LONG_MAX;

    //Jetzt wird fuer die jeweils letzte Seite geprueft ob sie leer ist
    //bei der ersten nicht leeren Seite wird die Schleife beendet.
    do
    {
        bool bExistEssentialObjs = ( 0 != pPage->GetSortedObjs() );
        if ( bExistEssentialObjs )
        {
            //Nur weil die Seite Flys hat sind wir noch lange nicht fertig,
            //denn wenn alle Flys an generischem Inhalt haengen, so ist sie
            //trotzdem ueberfluessig (Ueberpruefung auf DocBody sollte reichen).
            // OD 19.06.2003 #108784# - consider that drawing objects in
            // header/footer are supported now.
            bool bOnlySuperfluosObjs = true;
            SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
            for ( USHORT i = 0; bOnlySuperfluosObjs && i < rObjs.Count(); ++i )
            {
                SdrObject *pO = rObjs[i];
                if ( pO->IsWriterFlyFrame() )
                {
                    SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
                    // OD 19.06.2003 #108784# - correction
                    if ( !pFly->GetAnchor()->FindFooterOrHeader() )
                    {
                        bOnlySuperfluosObjs = false;
                    }
                }
                else
                {
                    // OD 19.06.2003 #108784# - determine, if drawing object
                    // isn't anchored in header/footer frame. If so, drawing
                    // object isn't superfluos.
                    SwFrm* pAnchorFrm = 0L;
                    if ( pO->ISA(SwDrawVirtObj) )
                    {
                        pAnchorFrm = static_cast<SwDrawVirtObj*>(pO)->GetAnchorFrm();
                    }
                    else
                    {
                        SwDrawContact* pDrawContact =
                                static_cast<SwDrawContact*>(pO->GetUserCall());
                        pAnchorFrm = pDrawContact ? pDrawContact->GetAnchor() : 0L;
                    }
                    if ( pAnchorFrm )
                    {
                        if ( !pAnchorFrm->FindFooterOrHeader() )
                        {
                            bOnlySuperfluosObjs = false;
                        }
                    }
                }
            }
            bExistEssentialObjs = !bOnlySuperfluosObjs;
        }

        // OD 19.06.2003 #108784# - optimization: check first, if essential objects
        // exists.
        if ( bExistEssentialObjs || pPage->FindFirstBodyCntnt() || pPage->FindFtnCont() )
        {
            if ( pPage->IsFtnPage() )
            {
                while ( pPage->IsFtnPage() )
                {
                    pPage = (SwPageFrm*)pPage->GetPrev();
                    ASSERT( pPage, "Nur noch Endnotenseiten uebrig." );
                }
                continue;
            }
            else
                pPage = 0;
        }

        if ( pPage )
        {
            SwPageFrm *pEmpty = pPage;
            pPage = (SwPageFrm*)pPage->GetPrev();
            if ( GetFmt()->GetDoc()->GetFtnIdxs().Count() )
                RemoveFtns( pEmpty, TRUE );
            pEmpty->Cut();
            delete pEmpty;
            nDocPos = pPage ? pPage->Frm().Top() : 0;
        }
    } while ( pPage );

    ViewShell *pSh = GetShell();
    if ( nDocPos != LONG_MAX &&
         (!pSh || !pSh->Imp()->IsUpdateExpFlds()) )
    {
        SwDocPosUpdate aMsgHnt( nDocPos );
        GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
    }
}

/*************************************************************************
|*
|*	SwRootFrm::AssertFlyPages()
|*
|*	Beschreibung		Stellt sicher, dass genuegend Seiten vorhanden
|* 		sind, damit alle Seitengebundenen Rahmen und DrawObject
|*		untergebracht sind.
|*
|*	Ersterstellung		MA 27. Jul. 93
|*	Letzte Aenderung	MA 24. Apr. 97
|*
|*************************************************************************/
/*N*/ void SwRootFrm::AssertFlyPages()
/*N*/ {
/*N*/ 	if ( !IsAssertFlyPages() )
/*N*/ 		return;
/*N*/ 	bAssertFlyPages = FALSE;
/*N*/ 
/*N*/ 	SwDoc *pDoc = GetFmt()->GetDoc();
/*N*/ 	const SwSpzFrmFmts *pTbl = pDoc->GetSpzFrmFmts();
/*N*/ 
/*N*/ 	//Auf welche Seite will der 'letzte' Fly?
/*N*/ 	USHORT nMaxPg = 0;
        USHORT i=0;
/*N*/ 	for ( i = 0; i < pTbl->Count(); ++i )
/*N*/ 	{
/*N*/ 		const SwFmtAnchor &rAnch = (*pTbl)[i]->GetAnchor();
/*N*/ 		if ( !rAnch.GetCntntAnchor() && nMaxPg < rAnch.GetPageNum() )
/*N*/ 			nMaxPg = rAnch.GetPageNum();
/*N*/ 	}
/*N*/ 	//Wieviele Seiten haben wir derzeit?
/*N*/ 	SwPageFrm *pPage = (SwPageFrm*)Lower();
/*N*/ 	while ( pPage && pPage->GetNext() &&
/*N*/ 			!((SwPageFrm*)pPage->GetNext())->IsFtnPage() )
/*N*/ 	{
/*N*/ 		pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nMaxPg > pPage->GetPhyPageNum() )
/*N*/ 	{
/*N*/ 		//Die Seiten werden ausgehend von der letzten Seite konsequent
/*N*/ 		//nach den Regeln der PageDescs weitergefuehrt.
/*N*/ 		BOOL bOdd = pPage->GetPhyPageNum() % 2 ? TRUE : FALSE;
/*N*/ 		SwPageDesc *pDesc = pPage->GetPageDesc();
/*N*/ 		SwFrm *pSibling = pPage->GetNext();
/*N*/ 		for ( i = pPage->GetPhyPageNum(); i < nMaxPg; ++i  )
/*N*/ 		{
/*N*/ 			if ( !(bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
/*N*/ 			{
/*N*/ 				//Leerseite einfuegen, die Flys werden aber erst von
/*N*/ 				//der naechsten Seite aufgenommen!
/*N*/ 				pPage = new SwPageFrm( pDoc->GetEmptyPageFmt(), pDesc );
/*N*/ 				pPage->Paste( this, pSibling );
/*N*/ 				pPage->PreparePage( FALSE );
/*N*/ 				bOdd = bOdd ? FALSE : TRUE;
/*N*/ 				++i;
/*N*/ 			}
/*N*/ 			pPage = new
/*N*/ 					SwPageFrm( (bOdd ? pDesc->GetRightFmt() :
/*N*/ 									   pDesc->GetLeftFmt()), pDesc );
/*N*/ 			pPage->Paste( this, pSibling );
/*N*/ 			pPage->PreparePage( FALSE );
/*N*/ 			bOdd = bOdd ? FALSE : TRUE;
/*N*/ 			pDesc = pDesc->GetFollow();
/*N*/ 		}
/*N*/ 		//Jetzt koennen die Endnotenseiten natuerlich wieder krumm sein;
/*N*/ 		//in diesem Fall werden sie vernichtet.
/*N*/ 		if ( pDoc->GetFtnIdxs().Count() )
/*N*/ 		{
/*?*/ 			pPage = (SwPageFrm*)Lower();
/*?*/ 			while ( pPage && !pPage->IsFtnPage() )
/*?*/ 				pPage = (SwPageFrm*)pPage->GetNext();
/*?*/ 
/*?*/ 			if ( pPage )
/*?*/ 			{
/*?*/ 				SwPageDesc *pDesc = pPage->FindPageDesc();
/*?*/ 				bOdd = pPage->OnRightPage();
/*?*/ 				if ( pPage->GetFmt() !=
/*?*/ 					 (bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
/*?*/ 					RemoveFtns( pPage, FALSE, TRUE );
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::AssertPageFlys()
|*
|*	Beschreibung		Stellt sicher, dass ab der uebergebenen Seite
|* 		auf allen Seiten die Seitengebunden Objecte auf der richtigen
|* 		Seite (Seitennummer stehen).
|*
|*	Ersterstellung		MA 02. Nov. 94
|*	Letzte Aenderung	MA 10. Aug. 95
|*
|*************************************************************************/
/*N*/ void SwRootFrm::AssertPageFlys( SwPageFrm *pPage )
/*N*/ {
/*N*/ 	while ( pPage )
/*N*/ 	{
/*N*/ 		if ( pPage->GetSortedObjs() )
/*N*/ 		{
/*N*/ 			pPage->GetSortedObjs();
/*N*/ 			for ( int i = 0;
/*N*/ 				  pPage->GetSortedObjs() && USHORT(i) < pPage->GetSortedObjs()->Count();
/*N*/ 				  ++i)
/*N*/ 			{
/*N*/ 				SwFrmFmt *pFmt = ::binfilter::FindFrmFmt( (*pPage->GetSortedObjs())[i] );
/*N*/ 				const SwFmtAnchor &rAnch = pFmt->GetAnchor();
/*N*/ 				const USHORT nPg = rAnch.GetPageNum();
/*N*/ 				if ( rAnch.GetAnchorId() == FLY_PAGE &&
/*N*/ 					 nPg != pPage->GetPhyPageNum() )
/*N*/ 				{
/*N*/ 					//Das er auf der falschen Seite steht muss noch nichts
/*N*/ 					//heissen, wenn er eigentlich auf der Vorseite
/*N*/ 					//stehen will und diese eine EmptyPage ist.
/*N*/                     if( nPg && !(pPage->GetPhyPageNum()-1 == nPg &&
/*N*/                         ((SwPageFrm*)pPage->GetPrev())->IsEmptyPage()) )
/*N*/ 					{
/*?*/ 						//Umhaengen kann er sich selbst, indem wir ihm
/*?*/ 						//einfach ein Modify mit seinem AnkerAttr schicken.
/*?*/ #ifndef DBG_UTIL
/*?*/ 						pFmt->SwModify::Modify( 0, (SwFmtAnchor*)&rAnch );
/*?*/ #else
/*?*/ 						const USHORT nCnt = pPage->GetSortedObjs()->Count();
/*?*/ 						pFmt->SwModify::Modify( 0, (SwFmtAnchor*)&rAnch );
/*?*/ 						ASSERT( !pPage->GetSortedObjs() ||
/*?*/ 								nCnt != pPage->GetSortedObjs()->Count(),
/*?*/ 								"Kann das Obj nicht umhaengen." );
/*?*/ #endif
/*?*/ 						--i;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::ChgSize()
|*
|*	Ersterstellung		MA 24. Jul. 92
|*	Letzte Aenderung	MA 13. Aug. 93
|*
|*************************************************************************/
/*N*/ void SwRootFrm::ChgSize( const Size& aNewSize )
/*N*/ {
/*N*/ 	Frm().SSize() = aNewSize;
/*N*/ 	_InvalidatePrt();
/*N*/     bFixSize = FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::MakeAll()
|*
|*	Ersterstellung		MA 17. Nov. 92
|*	Letzte Aenderung	MA 19. Apr. 93
|*
|*************************************************************************/
/*N*/ void SwRootFrm::MakeAll()
/*N*/ {
/*N*/ 	if ( !bValidPos )
/*N*/ 	{	bValidPos = TRUE;
/*N*/ 		aFrm.Pos().X() = aFrm.Pos().Y() = DOCUMENTBORDER;
/*N*/ 	}
/*N*/ 	if ( !bValidPrtArea )
/*N*/ 	{	bValidPrtArea = TRUE;
/*N*/ 		aPrt.Pos().X() = aPrt.Pos().Y() = 0;
/*N*/ 		aPrt.SSize( aFrm.SSize() );
/*N*/ 	}
/*N*/ 	if ( !bValidSize )
/*N*/ 		//SSize wird von den Seiten (Cut/Paste) eingestellt.
/*N*/ 		bValidSize = TRUE;
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::ImplInvalidateBrowseWidth()
|*
|*	Ersterstellung		MA 08. Jun. 96
|*	Letzte Aenderung	MA 08. Jun. 96
|*
|*************************************************************************/
/*N*/ void SwRootFrm::ImplInvalidateBrowseWidth()
/*N*/ {
/*N*/ 	bBrowseWidthValid = FALSE;
/*N*/ 	SwFrm *pPg = Lower();
/*N*/ 	while ( pPg )
/*N*/ 	{
/*N*/ 		pPg->InvalidateSize();
/*N*/ 		pPg = pPg->GetNext();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::ImplCalcBrowseWidth()
|*
|*	Ersterstellung		MA 07. Jun. 96
|*	Letzte Aenderung	MA 13. Jun. 96
|*
|*************************************************************************/
/*N*/ void SwRootFrm::ImplCalcBrowseWidth()
/*N*/ {
/*N*/ 	ASSERT( GetFmt()->GetDoc()->IsBrowseMode(),
/*N*/ 			"CalcBrowseWidth and not in BrowseView" );
/*N*/ 
/*N*/ 	//Die (minimale) Breite wird von Rahmen, Tabellen und Zeichenobjekten
/*N*/ 	//bestimmt. Die Breite wird nicht anhand ihrer aktuellen Groessen bestimmt,
/*N*/ 	//sondern anhand der Attribute. Es interessiert also nicht wie breit sie
/*N*/ 	//sind, sondern wie breit sie sein wollen.
/*N*/ 	//Rahmen und Zeichenobjekte innerhalb ander Objekte (Rahmen, Tabellen)
/*N*/ 	//Zaehlen nicht.
/*N*/ 	//Seitenraender und Spalten werden hier nicht beruecksichtigt.
/*N*/ 
/*N*/ 	SwFrm *pFrm = ContainsCntnt();
/*N*/ 	while ( pFrm && !pFrm->IsInDocBody() )
/*?*/ 		pFrm = ((SwCntntFrm*)pFrm)->GetNextCntntFrm();
/*N*/ 	if ( !pFrm )
/*N*/ 		return;
/*N*/ 
/*N*/ 	bBrowseWidthValid = TRUE;
/*N*/ 	ViewShell *pSh = GetShell();
/*N*/ 	nBrowseWidth = pSh
/*N*/ 					? MINLAY + 2 * pSh->GetOut()->
/*N*/ 								PixelToLogic( pSh->GetBrowseBorder() ).Width()
/*N*/ 					: 5000;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if ( pFrm->IsInTab() )
/*N*/ 			pFrm = pFrm->FindTabFrm();
/*N*/ 
/*N*/ 		if ( pFrm->IsTabFrm() &&
/*N*/ 			 !((SwLayoutFrm*)pFrm)->GetFmt()->GetFrmSize().GetWidthPercent() )
/*N*/ 		{
/*N*/ 			SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
/*N*/ 			const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 			const SwFmtHoriOrient &rHori = rAttrs.GetAttrSet().GetHoriOrient();
/*N*/ 			long nWidth = rAttrs.GetSize().Width();
/*N*/ 			if ( nWidth < USHRT_MAX-2000 && //-2000, weil bei Randeinstellung per
/*N*/ 											//Zuppeln das USHRT_MAX verlorengeht!
/*N*/ 				 HORI_FULL != rHori.GetHoriOrient() )
/*N*/ 			{
/*N*/ 				const SwHTMLTableLayout *pLayoutInfo =
/*N*/ 					((const SwTabFrm *)pFrm)->GetTable()
/*N*/ 											->GetHTMLTableLayout();
/*N*/ 				if ( pLayoutInfo )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	nWidth = Min( nWidth, pLayoutInfo->GetBrowseWidthMin() );
/*N*/ 
/*N*/ 				switch ( rHori.GetHoriOrient() )
/*N*/ 				{
/*?*/ 					case HORI_NONE:
/*?*/ 						                        // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
/*?*/                         nWidth += rAttrs.CalcLeft( pFrm ) + rAttrs.CalcRight( pFrm );
/*?*/ 						break;
/*?*/ 					case HORI_LEFT_AND_WIDTH:
/*?*/ 						nWidth += rAttrs.CalcLeft( pFrm );
/*N*/ 				}
/*N*/ 				nBrowseWidth = Max( nBrowseWidth, nWidth );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( pFrm->GetDrawObjs() )
/*N*/ 		{
/*N*/ 			for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = (*pFrm->GetDrawObjs())[i];
/*N*/ 				SwFrmFmt *pFmt = ::binfilter::FindFrmFmt( pObj );
/*N*/ 				const FASTBOOL bFly = pObj->IsWriterFlyFrame();
/*N*/ 				if ( bFly &&
/*N*/ 					 WEIT_WECH == ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Frm().Width()||
/*N*/ 					 pFmt->GetFrmSize().GetWidthPercent() )
/*?*/ 					continue;
/*N*/ 
/*N*/ 				long nWidth = 0;
/*N*/ 				switch ( pFmt->GetAnchor().GetAnchorId() )
/*N*/ 				{
/*N*/ 					case FLY_IN_CNTNT:
/*N*/ 						nWidth = bFly ? pFmt->GetFrmSize().GetWidth() :
/*N*/ 										pObj->GetBoundRect().GetWidth();
/*N*/ 						break;
/*N*/ 					case FLY_AT_CNTNT:
/*N*/ 						{
/*N*/ 						if ( bFly )
/*N*/ 						{
/*N*/ 							nWidth = pFmt->GetFrmSize().GetWidth();
/*N*/ 							const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
/*N*/ 							switch ( rHori.GetHoriOrient() )
/*N*/ 							{
/*N*/ 								case HORI_NONE:
/*N*/ 									nWidth += rHori.GetPos();
/*N*/ 									break;
/*N*/ 								case HORI_INSIDE:
/*N*/ 								case HORI_LEFT:
/*?*/ 									if ( PRTAREA == rHori.GetRelationOrient() )
/*?*/ 										nWidth += pFrm->Prt().Left();
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 							//Fuer Zeichenobjekte ist die Auswahl sehr klein,
/*N*/ 							//weil sie keine Attribute haben, also durch ihre
/*N*/ 							//aktuelle Groesse bestimmt werden.
/*N*/ 							nWidth = pObj->GetBoundRect().Right() -
/*N*/ 									 pObj->GetAnchorPos().X();
/*N*/ 
/*N*/ //MA 31. Jan. 97: Zaehlt doch garnicht mehr, seit die Flys den Rand nicht
/*N*/ //mehr beruecksichtigen.
/*N*/ //						const SwContact *pCon = (SwContact*)pObj->GetUserCall();
/*N*/ //						const SvxLRSpaceItem &rLR = pCon->GetFmt()->GetLRSpace();
/*N*/ //						nWidth += rLR.GetLeft() + rLR.GetRight();
/*N*/ 						}
/*N*/ 						break;
/*N*/ 					default:	/* do nothing */;
/*N*/ 				}
/*N*/ 				nBrowseWidth = Max( nBrowseWidth, nWidth );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pFrm = pFrm->FindNextCnt();
/*N*/ 	} while ( pFrm );
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::StartAllAction()
|*
|*	Ersterstellung		MA 08. Mar. 98
|*	Letzte Aenderung	MA 08. Mar. 98
|*
|*************************************************************************/

/*N*/ void SwRootFrm::StartAllAction()
/*N*/ {
/*N*/ 	ViewShell *pSh = GetCurrShell();
/*N*/ 	if ( pSh )
/*N*/ 		do
/*N*/ 		{	if ( pSh->ISA( SwCrsrShell ) )
/*N*/ 				((SwCrsrShell*)pSh)->StartAction();
/*N*/ 			else
/*?*/ 				pSh->StartAction();
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 
/*N*/ 		} while ( pSh != GetCurrShell() );
/*N*/ }

/*N*/ void SwRootFrm::EndAllAction( BOOL bVirDev )
/*N*/ {
/*N*/ 	ViewShell *pSh = GetCurrShell();
/*N*/ 	if ( pSh )
/*N*/ 		do
/*N*/ 		{
/*N*/ 			const BOOL bOldEndActionByVirDev = pSh->IsEndActionByVirDev();
/*N*/ 			pSh->SetEndActionByVirDev( bVirDev );
/*N*/ 			if ( pSh->ISA( SwCrsrShell ) )
/*N*/ 			{
/*N*/ 				((SwCrsrShell*)pSh)->EndAction();
/*N*/ 				((SwCrsrShell*)pSh)->CallChgLnk();
/*N*/ 				if ( pSh->ISA( SwFEShell ) )
/*N*/ 					((SwFEShell*)pSh)->SetChainMarker();
/*N*/ 			}
/*N*/ 			else
/*?*/ 				pSh->EndAction();
/*N*/ 			pSh->SetEndActionByVirDev( bOldEndActionByVirDev );
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 
/*N*/ 		} while ( pSh != GetCurrShell() );
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
