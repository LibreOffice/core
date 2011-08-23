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

#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "node.hxx"

#include <horiornt.hxx>

#include "doc.hxx"
#include "frmtool.hxx"

#include <frmfmt.hxx>
#include "tabfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrms.hxx"
#include "ftnfrm.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
namespace binfilter {

/*************************************************************************
|*
|*	FindBodyCont, FindLastBodyCntnt()
|*
|*	Beschreibung		Sucht den ersten/letzten CntntFrm im BodyText unterhalb
|* 		der Seite.
|*	Ersterstellung		MA 15. Feb. 93
|*	Letzte Aenderung	MA 18. Apr. 94
|*
|*************************************************************************/
/*N*/ SwLayoutFrm *SwFtnBossFrm::FindBodyCont()
/*N*/ {
/*N*/ 	SwFrm *pLay = Lower();
/*N*/ 	while ( pLay && !pLay->IsBodyFrm() )
/*N*/ 		pLay = pLay->GetNext();
/*N*/ 	return (SwLayoutFrm*)pLay;
/*N*/ }

SwCntntFrm *SwPageFrm::FindLastBodyCntnt()
{
    SwCntntFrm *pRet = FindFirstBodyCntnt();
    SwCntntFrm *pNxt = pRet;
    while ( pNxt && pNxt->IsInDocBody() && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNextCnt();
    }
    return pRet;
}

/*************************************************************************
|*
|*	SwLayoutFrm::ContainsCntnt
|*
|*	Beschreibung			Prueft, ob der Frame irgendwo in seiner
|*			untergeordneten Struktur einen oder mehrere CntntFrm's enthaelt;
|*			Falls ja wird der erste gefundene CntntFrm zurueckgegeben.
|*
|*	Ersterstellung		MA 13. May. 92
|*	Letzte Aenderung	MA 20. Apr. 94
|*
|*************************************************************************/

/*N*/ const SwCntntFrm *SwLayoutFrm::ContainsCntnt() const
/*N*/ {
/*N*/ 	//LayoutBlatt nach unten hin suchen und wenn dieses keinen Inhalt hat
/*N*/ 	//solange die weiteren Blatter abklappern bis Inhalt gefunden oder der
/*N*/ 	//this verlassen wird.
/*N*/ 	//Sections: Cntnt neben Sections wuerde so nicht gefunden (leere Section
/*N*/ 	//direct neben CntntFrm), deshalb muss fuer diese Aufwendiger rekursiv gesucht
/*N*/ 	//werden.
/*N*/ 
/*N*/ 	const SwLayoutFrm *pLayLeaf = this;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		while ( (!pLayLeaf->IsSctFrm() || pLayLeaf == this ) &&
/*N*/ 				pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
/*N*/ 			pLayLeaf = (SwLayoutFrm*)pLayLeaf->Lower();
/*N*/ 
/*N*/ 		if( pLayLeaf->IsSctFrm() && pLayLeaf != this )
/*N*/ 		{
/*N*/ 			const SwCntntFrm *pCnt = pLayLeaf->ContainsCntnt();
/*N*/ 			if( pCnt )
/*N*/ 				return pCnt;
/*N*/ 			if( pLayLeaf->GetNext() )
/*N*/ 			{
/*N*/ 				if( pLayLeaf->GetNext()->IsLayoutFrm() )
/*N*/ 				{
/*N*/ 					pLayLeaf = (SwLayoutFrm*)pLayLeaf->GetNext();
/*N*/ 					continue;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					return (SwCntntFrm*)pLayLeaf->GetNext();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( pLayLeaf->Lower() )
/*N*/ 			return (SwCntntFrm*)pLayLeaf->Lower();
/*N*/ 
/*N*/ 		pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
/*N*/ 		if( !IsAnLower( pLayLeaf) )
/*N*/ 			return 0;
/*N*/ 	} while( pLayLeaf );
/*?*/ 	return 0;
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::FirstCell
|*
|*	Beschreibung	ruft zunaechst ContainsAny auf, um in die innerste Zelle
|* 					hineinzukommen. Dort hangelt es sich wieder hoch zum
|*					ersten SwCellFrm, seit es SectionFrms gibt, reicht kein
|*					ContainsCntnt()->GetUpper() mehr...
|*	Ersterstellung		AMA 17. Mar. 99
|*	Letzte Aenderung	AMA 17. Mar. 99
|*
|*************************************************************************/

/*N*/ const SwCellFrm *SwLayoutFrm::FirstCell() const
/*N*/ {
/*N*/ 	const SwFrm* pCnt = ContainsAny();
/*N*/ 	while( pCnt && !pCnt->IsCellFrm() )
/*N*/ 		pCnt = pCnt->GetUpper();
/*N*/ 	return (const SwCellFrm*)pCnt;
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::ContainsAny
|*
|*	Beschreibung wie ContainsCntnt, nur dass nicht nur CntntFrms, sondern auch
|*			Bereiche und Tabellen zurueckgegeben werden.
|*	Ersterstellung		AMA 10. Mar. 99
|*	Letzte Aenderung	AMA 10. Mar. 99
|*
|*************************************************************************/

/*N*/ const SwFrm *SwLayoutFrm::ContainsAny() const
/*N*/ {
/*N*/ 	//LayoutBlatt nach unten hin suchen und wenn dieses keinen Inhalt hat
/*N*/ 	//solange die weiteren Blatter abklappern bis Inhalt gefunden oder der
/*N*/ 	//this verlassen wird.
/*N*/ 	// Oder bis wir einen SectionFrm oder TabFrm gefunden haben
/*N*/ 
/*N*/ 	const SwLayoutFrm *pLayLeaf = this;
/*N*/ 	BOOL bNoFtn = IsSctFrm();
/*N*/ 	do
/*N*/ 	{
/*N*/ 		while ( ( (!pLayLeaf->IsSctFrm() && !pLayLeaf->IsTabFrm())
/*N*/ 				 || pLayLeaf == this ) &&
/*N*/ 				pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
/*N*/ 			pLayLeaf = (SwLayoutFrm*)pLayLeaf->Lower();
/*N*/ 
/*N*/ 		if( ( pLayLeaf->IsTabFrm() || pLayLeaf->IsSctFrm() )
/*N*/ 			&& pLayLeaf != this )
/*N*/ 		{
/*N*/ 			// Wir liefern jetzt auch "geloeschte" SectionFrms zurueck,
/*N*/ 			// damit diese beim SaveCntnt und RestoreCntnt mitgepflegt werden.
/*N*/ 			return pLayLeaf;
/*N*/ #ifdef USED
/*N*/ 			const SwCntntFrm *pCnt = pLayLeaf->ContainsCntnt();
/*N*/ 			if( pCnt )
/*N*/ 				return pLayLeaf;
/*N*/ 			if( pLayLeaf->GetNext() )
/*N*/ 			{
/*N*/ 				if( pLayLeaf->GetNext()->IsLayoutFrm() )
/*N*/ 				{
/*N*/ 					pLayLeaf = (SwLayoutFrm*)pLayLeaf->GetNext();
/*N*/ 					continue;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					return (SwCntntFrm*)pLayLeaf->GetNext();
/*N*/ 			}
/*N*/ #endif
/*N*/ 		}
/*N*/ 		else if ( pLayLeaf->Lower() )
/*N*/ 			return (SwCntntFrm*)pLayLeaf->Lower();
/*N*/ 
/*N*/ 		pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
/*N*/ 		if( bNoFtn && pLayLeaf && pLayLeaf->IsInFtn() )
/*N*/ 		{
/*?*/ 			do
/*?*/ 			{
/*?*/ 				pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
/*?*/ 			} while( pLayLeaf && pLayLeaf->IsInFtn() );
/*N*/ 		}
/*N*/ 		if( !IsAnLower( pLayLeaf) )
/*N*/ 			return 0;
/*?*/ 	} while( pLayLeaf );
/*?*/ 	return 0;
/*N*/ }


/*************************************************************************
|*
|*	SwFrm::GetLower()
|*
|*	Ersterstellung		MA 27. Jul. 92
|*	Letzte Aenderung	MA 09. Oct. 97
|*
|*************************************************************************/
/*N*/ const SwFrm* SwFrm::GetLower() const
/*N*/ {
/*N*/ 	return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
/*N*/ }

/*N*/ SwFrm* SwFrm::GetLower()
/*N*/ {
/*N*/ 	return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::IsAnLower()
|*
|*	Ersterstellung		MA 18. Mar. 93
|*	Letzte Aenderung	MA 18. Mar. 93
|*
|*************************************************************************/
/*N*/ BOOL SwLayoutFrm::IsAnLower( const SwFrm *pAssumed ) const
/*N*/ {
/*N*/ 	const SwFrm *pUp = pAssumed;
/*N*/ 	while ( pUp )
/*N*/ 	{
/*N*/ 		if ( pUp == this )
/*N*/ 			return TRUE;
/*N*/ 		if ( pUp->IsFlyFrm() )
/*N*/ 			pUp = ((SwFlyFrm*)pUp)->GetAnchor();
/*N*/ 		else
/*N*/ 			pUp = pUp->GetUpper();
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/** method to check relative position of layout frame to
    a given layout frame.

    OD 08.11.2002 - refactoring of pseudo-local method <lcl_Apres(..)> in
    <txtftn.cxx> for #104840#.

    @param _aCheckRefLayFrm
    constant reference of an instance of class <SwLayoutFrm> which
    is used as the reference for the relative position check.

    @author OD

    @return true, if <this> is positioned before the layout frame <p>
*/
/*N*/ bool SwLayoutFrm::IsBefore( const SwLayoutFrm* _pCheckRefLayFrm ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::GetPrevLayoutLeaf()
|*
|*	Beschreibung		Findet das vorhergehende Layout-Blatt. Ein Layout-
|* 		Blatt ist ein LayoutFrm, der keinen LayoutFrm in seinen Unterbaum hat;
|* 		genau gesagt, darf pLower kein LayoutFrm sein.
|* 		Anders ausgedrueckt: pLower darf 0 sein oder auf einen CntntFrm
|* 		zeigen.
|* 		pLower darf allerdings auf einen TabFrm zeigen, denn diese stehen
|* 		direkt neben den CntntFrms.
|*	Ersterstellung		MA 29. May. 92
|*	Letzte Aenderung	MA 30. Oct. 97
|*
|*************************************************************************/
/*N*/ const SwFrm * MA_FASTCALL lcl_LastLower( const SwFrm *pFrm )
/*N*/ {
/*N*/ 	const SwFrm *pLower = pFrm->GetLower();
/*N*/ 	if ( pLower )
/*N*/ 		while ( pLower->GetNext() )
/*N*/ 			pLower = pLower->GetNext();
/*N*/ 	return pLower;
/*N*/ }

/*N*/ const SwLayoutFrm *SwFrm::GetPrevLayoutLeaf() const
/*N*/ {
/*N*/ 	const SwFrm		  *pFrm = this;
/*N*/ 	const SwLayoutFrm *pLayoutFrm = 0;
/*N*/ 	const SwFrm		  *p;
/*N*/ 	FASTBOOL bGoingUp = TRUE;
/*N*/ 	do {
/*N*/ 		FASTBOOL bGoingBwd = FALSE, bGoingDown = FALSE;
/*N*/ 		if( !(bGoingDown = (!bGoingUp && ( 0 != (p = ::binfilter::lcl_LastLower( pFrm ))))) &&
/*N*/ 			!(bGoingBwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetPrevLink()
/*N*/ 													  : pFrm->GetPrev()))) &&
/*N*/ 			!(bGoingUp = (0 != (p = pFrm->GetUpper()))))
/*N*/ 			return 0;
/*N*/ 		bGoingUp = !( bGoingBwd || bGoingDown );
/*N*/ 		pFrm = p;
/*N*/ 		p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0;
/*N*/ 	} while( (p && !p->IsFlowFrm()) ||
/*N*/ 			 pFrm == this ||
/*N*/ 			 0 == (pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm:0) ||
/*N*/ 			 pLayoutFrm->IsAnLower( this ) );
/*N*/ 
/*N*/ 	return pLayoutFrm;
/*N*/ }
/*************************************************************************
|*
|*	SwFrm::GetNextLayoutLeaf
|*
|*	Beschreibung		Findet das naechste Layout-Blatt. Ein Layout-Blatt
|* 			ist ein LayoutFrm, der kein LayoutFrm in seinen Unterbaum hat;
|* 			genau gesagt, darf pLower kein LayoutFrm sein.
|* 			Anders ausgedrueckt: pLower darf 0 sein oder auf einen CntntFrm
|* 			zeigen.
|* 			pLower darf allerdings auf einen TabFrm zeigen, denn diese stehen
|* 			direkt neben den CntntFrms.
|*	Ersterstellung		MA 13. May. 92
|*	Letzte Aenderung	MA 30. Oct. 97
|*
|*************************************************************************/
/*N*/ const SwLayoutFrm *SwFrm::GetNextLayoutLeaf() const
/*N*/ {
/*N*/ 	const SwFrm		  *pFrm = this;
/*N*/ 	const SwLayoutFrm *pLayoutFrm = 0;
/*N*/ 	const SwFrm 	  *p;
/*N*/ 	FASTBOOL bGoingUp = FALSE;
/*N*/ 	do {
/*N*/ 		FASTBOOL bGoingFwd = FALSE, bGoingDown = FALSE;
/*N*/ 		if( !(bGoingDown = (!bGoingUp && ( 0 !=
/*N*/ 			(p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
/*N*/ 			!(bGoingFwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink()
/*N*/ 													  : pFrm->GetNext()))) &&
/*N*/ 			!(bGoingUp = (0 != (p = pFrm->GetUpper()))))
/*N*/ 			return 0;
/*N*/ 		bGoingUp = !( bGoingFwd || bGoingDown );
/*N*/ 		pFrm = p;
/*N*/ 		p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0;
/*N*/ 	} while( (p && !p->IsFlowFrm()) ||
/*N*/ 			 pFrm == this ||
/*N*/ 			 0 == (pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm:0 ) ||
/*N*/ 			 pLayoutFrm->IsAnLower( this ) );
/*N*/ 
/*N*/ 	return pLayoutFrm;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::FindRootFrm(), FindTabFrm(), FindFtnFrm(), FindFlyFrm(),
|* 		   FindPageFrm(), FindColFrm()
|*
|*	Ersterstellung		??
|*	Letzte Aenderung	MA 05. Sep. 93
|*
|*************************************************************************/
/*N*/ SwRootFrm* SwFrm::FindRootFrm()
/*N*/ {
/*N*/ 	// MIB: A layout frame is always registerd at a SwFrmFmt and a content
/*N*/ 	// frame alyways at a SwCntntNode. For any other case we won't find
/*N*/ 	// a root frame.
/*N*/ 	// Casting the GetDep() result instead of the frame itself (that has
/*N*/ 	// been done before) makes it save to use that method in constructors
/*N*/ 	// and destructors.
/*N*/ 	ASSERT( GetDep(), "frame is not registered any longer" );
/*N*/ 	ASSERT( IsLayoutFrm() || IsCntntFrm(), "invalid frame type" );
/*N*/ 	SwDoc *pDoc = IsLayoutFrm()
/*N*/ 						? static_cast < SwFrmFmt * >( GetDep() )->GetDoc()
/*N*/ 						: static_cast < SwCntntNode * >( GetDep() )->GetDoc();
/*N*/ 	return pDoc->GetRootFrm();
/*N*/ }

/*N*/ SwPageFrm* SwFrm::FindPageFrm()
/*N*/ {
/*N*/ 	SwFrm *pRet = this;
/*N*/ 	while ( pRet && !pRet->IsPageFrm() )
/*N*/ 	{
/*N*/ 		if ( pRet->GetUpper() )
/*N*/ 			pRet = pRet->GetUpper();
/*N*/ 		else if ( pRet->IsFlyFrm() )
/*N*/ 		{
/*N*/             if ( ((SwFlyFrm*)pRet)->IsFlyFreeFrm() &&
/*N*/                  ((SwFlyFreeFrm*)pRet)->GetPage() )
/*N*/ 				pRet = ((SwFlyFreeFrm*)pRet)->GetPage();
/*N*/ 			else
/*N*/ 				pRet = ((SwFlyFrm*)pRet)->GetAnchor();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			return 0;
/*N*/ 	}
/*N*/ 	return (SwPageFrm*)pRet;
/*N*/ }

/*N*/ SwFtnBossFrm* SwFrm::FindFtnBossFrm( BOOL bFootnotes )
/*N*/ {
/*N*/ 	SwFrm *pRet = this;
/*N*/ 	// Innerhalb einer Tabelle gibt es keine Fussnotenbosse, auch spaltige
/*N*/ 	// Bereiche enthalten dort keine Fussnotentexte
/*N*/ 	if( pRet->IsInTab() )
/*N*/ 		pRet = pRet->FindTabFrm();
/*N*/ 	while ( pRet && !pRet->IsFtnBossFrm() )
/*N*/ 	{
/*N*/ 		if ( pRet->GetUpper() )
/*N*/ 			pRet = pRet->GetUpper();
/*N*/ 		else if ( pRet->IsFlyFrm() )
/*N*/ 		{
/*N*/ 			if ( ((SwFlyFrm*)pRet)->IsFlyFreeFrm() )
/*N*/ 				pRet = ((SwFlyFreeFrm*)pRet)->GetPage();
/*N*/ 			else
/*N*/ 				pRet = ((SwFlyFrm*)pRet)->GetAnchor();
/*N*/ 		}
/*N*/ 		else
/*?*/ 			return 0;
/*N*/ 	}
/*N*/ 	if( bFootnotes && pRet && pRet->IsColumnFrm() &&
/*N*/ 		!pRet->GetNext() && !pRet->GetPrev() )
/*N*/ 	{
/*?*/ 		SwSectionFrm* pSct = pRet->FindSctFrm();
/*?*/ 		ASSERT( pSct, "FindFtnBossFrm: Single column outside section?" );
/*?*/ 		if( !pSct->IsFtnAtEnd() )
/*?*/ 			return pSct->FindFtnBossFrm( TRUE );
/*N*/ 	}
/*N*/ 	return (SwFtnBossFrm*)pRet;
/*N*/ }

/*N*/ SwTabFrm* SwFrm::ImplFindTabFrm()
/*N*/ {
/*N*/ 	SwFrm *pRet = this;
/*N*/ 	while ( !pRet->IsTabFrm() )
/*N*/ 	{
/*N*/ 		pRet = pRet->GetUpper();
/*N*/ 		if ( !pRet )
/*?*/ 			return 0;
/*N*/ 	}
/*N*/ 	return (SwTabFrm*)pRet;
/*N*/ }

/*N*/ SwSectionFrm* SwFrm::ImplFindSctFrm()
/*N*/ {
/*N*/ 	SwFrm *pRet = this;
/*N*/ 	while ( !pRet->IsSctFrm() )
/*N*/ 	{
/*N*/ 		pRet = pRet->GetUpper();
/*N*/ 		if ( !pRet )
/*?*/ 			return 0;
/*N*/ 	}
/*N*/ 	return (SwSectionFrm*)pRet;
/*N*/ }


/*N*/ SwFtnFrm *SwFrm::ImplFindFtnFrm()
/*N*/ {
/*N*/ 	SwFrm *pRet = this;
/*N*/ 	while ( !pRet->IsFtnFrm() )
/*N*/ 	{
/*N*/ 		pRet = pRet->GetUpper();
/*N*/ 		if ( !pRet )
/*?*/ 			return 0;
/*N*/ 	}
/*N*/ 	return (SwFtnFrm*)pRet;
/*N*/ }

/*N*/ SwFlyFrm *SwFrm::ImplFindFlyFrm()
/*N*/ {
/*N*/ 	const SwFrm *pRet = this;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if ( pRet->IsFlyFrm() )
/*N*/ 			return (SwFlyFrm*)pRet;
/*N*/ 		else
/*N*/ 			pRet = pRet->GetUpper();
/*N*/ 	} while ( pRet );
/*?*/ 	return 0;
/*N*/ }

/*N*/ SwFrm *SwFrm::FindColFrm()
/*N*/ {
/*N*/ 	SwFrm *pFrm = this;
/*N*/ 	do
/*N*/ 	{	pFrm = pFrm->GetUpper();
/*N*/ 	} while ( pFrm && !pFrm->IsColumnFrm() );
/*N*/ 	return pFrm;
/*N*/ }

/*N*/ SwFrm* SwFrm::FindFooterOrHeader()
/*N*/ {
/*N*/ 	SwFrm* pRet = this;
/*N*/ 	do
/*N*/ 	{	if ( pRet->GetType() & 0x0018 )	//Header und Footer
/*N*/ 			return pRet;
/*N*/ 		else if ( pRet->GetUpper() )
/*N*/ 			pRet = pRet->GetUpper();
/*N*/ 		else if ( pRet->IsFlyFrm() )
/*N*/ 			pRet = ((SwFlyFrm*)pRet)->GetAnchor();
/*N*/ 		else
/*N*/ 			return 0;
/*N*/ 	} while ( pRet );
/*?*/ 	return pRet;
/*N*/ }




/*************************************************************************
|*
|*	SwFrmFrm::GetAttrSet()
|*
|*	Ersterstellung		MA 02. Aug. 93
|*	Letzte Aenderung	MA 02. Aug. 93
|*
|*************************************************************************/
/*N*/ const SwAttrSet* SwFrm::GetAttrSet() const
/*N*/ {
/*N*/ 	if ( IsCntntFrm() )
/*N*/ 		return &((const SwCntntFrm*)this)->GetNode()->GetSwAttrSet();
/*N*/ 	else
/*N*/ 		return &((const SwLayoutFrm*)this)->GetFmt()->GetAttrSet();
/*N*/ }

/*N*/ SwAttrSet* SwFrm::GetAttrSet()
/*N*/ {
/*N*/ 	if ( IsCntntFrm() )
/*N*/ 		return &((SwCntntFrm*)this)->GetNode()->GetSwAttrSet();
/*N*/ 	else
/*N*/ 		return (SwAttrSet*)&((SwLayoutFrm*)this)->GetFmt()->GetAttrSet();
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::_FindNext(), _FindPrev(), InvalidateNextPos()
|* 		   _FindNextCnt() geht in Tabellen und Bereiche hineinund liefert
|*         nur SwCntntFrms.
|*
|*	Beschreibung		Invalidiert die Position des Naechsten Frames.
|*		Dies ist der direkte Nachfolger, oder bei CntntFrm's der naechste
|*		CntntFrm der im gleichen Fluss liegt wie ich:
|* 		- Body,
|* 		- Fussnoten,
|* 		- Bei Kopf-/Fussbereichen ist die Benachrichtigung nur innerhalb des
|* 		  Bereiches weiterzuleiten.
|* 		- dito fuer Flys.
|* 		- Cntnts in Tabs halten sich ausschliesslich innerhalb ihrer Zelle
|* 		  auf.
|* 		- Tabellen verhalten sich prinzipiell analog zu den Cntnts
|* 		- Bereiche ebenfalls
|*	Ersterstellung		AK 14-Feb-1991
|*	Letzte Aenderung	AMA 10. Mar. 99
|*
|*************************************************************************/

// Diese Hilfsfunktion ist ein Aequivalent zur ImplGetNextCntntFrm()-Methode,
// sie liefert allerdings neben ContentFrames auch TabFrms und SectionFrms.
/*N*/ SwFrm* lcl_NextFrm( SwFrm* pFrm )
/*N*/ {
/*N*/ 	SwFrm *pRet = 0;
/*N*/ 	FASTBOOL bGoingUp = FALSE;
/*N*/ 	do {
/*N*/ 		SwFrm *p;
/*N*/ 		FASTBOOL bGoingFwd = FALSE, bGoingDown = FALSE;
/*N*/ 		if( !(bGoingDown = (!bGoingUp && ( 0 !=
/*N*/ 			 (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
/*N*/ 			!(bGoingFwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink()
/*N*/ 													  : pFrm->GetNext()))) &&
/*N*/ 			!(bGoingUp = (0 != (p = pFrm->GetUpper()))))
/*N*/ 			return 0;
/*N*/ 		bGoingUp = !(bGoingFwd || bGoingDown);
/*N*/ 		pFrm = p;
/*N*/ 	} while ( 0 == (pRet = ( ( pFrm->IsCntntFrm() || ( !bGoingUp &&
/*N*/ 			( pFrm->IsTabFrm() || pFrm->IsSctFrm() ) ) )? pFrm : 0 ) ) );
/*N*/ 	return pRet;
/*N*/ }

/*N*/ SwFrm *SwFrm::_FindNext()
/*N*/ {
/*N*/ 	BOOL bIgnoreTab = FALSE;
/*N*/ 	SwFrm *pThis = this;
/*N*/ 
/*N*/ 	if ( IsTabFrm() )
/*N*/ 	{
/*N*/ 		//Der letzte Cntnt der Tabelle wird
/*N*/ 		//gegriffen und dessen Nachfolger geliefert. Um die Spezialbeh.
/*N*/ 		//Fuer Tabellen (s.u.) auszuschalten wird bIgnoreTab gesetzt.
/*N*/ 		if ( ((SwTabFrm*)this)->GetFollow() )
/*N*/ 			return ((SwTabFrm*)this)->GetFollow();
/*N*/ 
/*N*/ 		pThis = ((SwTabFrm*)this)->FindLastCntnt();
/*N*/ 		if ( !pThis )
/*?*/ 			pThis = this;
/*N*/ 		bIgnoreTab = TRUE;
/*N*/ 	}
/*N*/ 	else if ( IsSctFrm() )
/*N*/ 	{
/*N*/ 		//Der letzte Cntnt des Bereichs wird gegriffen und dessen Nachfolger
/*N*/ 		// geliefert.
/*N*/ 		if ( ((SwSectionFrm*)this)->GetFollow() )
/*N*/ 			return ((SwSectionFrm*)this)->GetFollow();
/*N*/ 
/*N*/ 		pThis = ((SwSectionFrm*)this)->FindLastCntnt();
/*N*/ 		if ( !pThis )
/*N*/ 			pThis = this;
/*N*/ 	}
/*N*/ 	else if ( IsCntntFrm() )
/*N*/ 	{
/*N*/ 		if( ((SwCntntFrm*)this)->GetFollow() )
/*N*/ 			return ((SwCntntFrm*)this)->GetFollow();
/*N*/ 	}
/*N*/     else if ( IsRowFrm() )
/*N*/     {
/*N*/         SwFrm* pUpper = GetUpper();
/*N*/         if ( pUpper->IsTabFrm() && ((SwTabFrm*)pUpper)->GetFollow() )
/*N*/             return ((SwTabFrm*)pUpper)->GetFollow()->GetLower();
/*N*/         else return NULL;
/*N*/     }
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ 
/*N*/ 	SwFrm* pRet = NULL;
/*N*/ 	const BOOL bFtn  = pThis->IsInFtn();
/*N*/ 	if ( !bIgnoreTab && pThis->IsInTab() )
/*N*/ 	{
/*N*/ 		SwLayoutFrm *pUp = pThis->GetUpper();
/*N*/ 		while ( !pUp->IsCellFrm() )
/*?*/ 			pUp = pUp->GetUpper();
/*N*/ 		ASSERT( pUp, "Cntnt in Tabelle aber nicht in Zelle." );
/*N*/ 		SwFrm *pNxt = lcl_NextFrm( pThis );
/*N*/ 		if ( pUp->IsAnLower( pNxt ) )
/*?*/ 			pRet = pNxt;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const BOOL bBody = pThis->IsInDocBody();
/*N*/ 		SwFrm *pNxtCnt = lcl_NextFrm( pThis );
/*N*/ 		if ( pNxtCnt )
/*N*/ 		{
/*N*/ 			if ( bBody || bFtn )
/*N*/ 			{
/*N*/ 				while ( pNxtCnt )
/*N*/ 				{
/*N*/                     // OD 02.04.2003 #108446# - check for endnote, only if found
/*N*/                     // next content isn't contained in a section, that collect its
/*N*/                     // endnotes at its end.
/*N*/                     bool bEndn = IsInSct() && !IsSctFrm() &&
/*N*/                                  ( !pNxtCnt->IsInSct() ||
/*N*/                                    !pNxtCnt->FindSctFrm()->IsEndnAtEnd()
/*N*/                                  );
/*N*/                     if ( ( bBody && pNxtCnt->IsInDocBody() ) ||
/*N*/                          ( pNxtCnt->IsInFtn() &&
/*N*/                            ( bFtn ||
/*N*/                              ( bEndn && pNxtCnt->FindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
/*N*/                            )
/*N*/                          )
/*N*/                        )
/*N*/ 					{
/*N*/ 						pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
/*N*/ 													: (SwFrm*)pNxtCnt;
/*N*/ 						break;
/*N*/ 					}
/*N*/ 					pNxtCnt = lcl_NextFrm( pNxtCnt );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( pThis->IsInFly() )
/*N*/ 			{
/*N*/ 				pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
/*N*/ 											: (SwFrm*)pNxtCnt;
/*N*/ 			}
/*N*/ 			else	//Fuss-/oder Kopfbereich
/*N*/ 			{
/*N*/ 				const SwFrm *pUp = pThis->GetUpper();
/*N*/ 				const SwFrm *pCntUp = pNxtCnt->GetUpper();
/*N*/ 				while ( pUp && pUp->GetUpper() &&
/*N*/ 						!pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
/*?*/ 					pUp = pUp->GetUpper();
/*N*/ 				while ( pCntUp && pCntUp->GetUpper() &&
/*N*/ 						!pCntUp->IsHeaderFrm() && !pCntUp->IsFooterFrm() )
/*N*/ 					pCntUp = pCntUp->GetUpper();
/*N*/ 				if ( pCntUp == pUp )
/*N*/ 				{
/*?*/ 					pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
/*?*/ 												: (SwFrm*)pNxtCnt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( pRet && pRet->IsInSct() )
/*N*/ 	{
/*N*/ 		SwSectionFrm* pSct = pRet->FindSctFrm();
/*N*/ 		//Fussnoten in spaltigen Rahmen duerfen nicht den Bereich
/*N*/ 		//liefern, der die Fussnoten umfasst
/*N*/ 		if( !pSct->IsAnLower( this ) &&
/*N*/ 			(!bFtn || pSct->IsInFtn() ) )
/*N*/ 			return pSct;
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }

/*N*/ SwCntntFrm *SwFrm::_FindNextCnt()
/*N*/ {
/*N*/ 	SwFrm *pThis = this;
/*N*/ 
/*N*/ 	if ( IsTabFrm() )
/*N*/ 	{
/*N*/ 		if ( ((SwTabFrm*)this)->GetFollow() )
/*N*/ 		{
/*N*/ 			pThis = ((SwTabFrm*)this)->GetFollow()->ContainsCntnt();
/*N*/ 			if( pThis )
/*N*/ 				return (SwCntntFrm*)pThis;
/*N*/ 		}
/*N*/ 		pThis = ((SwTabFrm*)this)->FindLastCntnt();
/*N*/ 		if ( !pThis )
/*?*/ 			return 0;
/*N*/ 	}
/*N*/ 	else if ( IsSctFrm() )
/*N*/ 	{
/*?*/ 		if ( ((SwSectionFrm*)this)->GetFollow() )
/*?*/ 		{
/*?*/ 			pThis = ((SwSectionFrm*)this)->GetFollow()->ContainsCntnt();
/*?*/ 			if( pThis )
/*?*/ 				return (SwCntntFrm*)pThis;
/*?*/ 		}
/*?*/ 		pThis = ((SwSectionFrm*)this)->FindLastCntnt();
/*?*/ 		if ( !pThis )
/*?*/ 			return 0;
/*N*/ 	}
/*N*/ 	else if ( IsCntntFrm() && ((SwCntntFrm*)this)->GetFollow() )
/*N*/ 		return ((SwCntntFrm*)this)->GetFollow();
/*N*/ 
/*N*/ 	if ( pThis->IsCntntFrm() )
/*N*/ 	{
/*N*/ 		const BOOL bBody = pThis->IsInDocBody();
/*N*/ 		const BOOL bFtn  = pThis->IsInFtn();
/*N*/ 		SwCntntFrm *pNxtCnt = ((SwCntntFrm*)pThis)->GetNextCntntFrm();
/*N*/ 		if ( pNxtCnt )
/*N*/ 		{
/*N*/ 			if ( bBody || bFtn )
/*N*/ 			{
/*N*/ 				while ( pNxtCnt )
/*N*/ 				{
/*N*/ 					if ( (bBody && pNxtCnt->IsInDocBody()) ||
/*N*/ 						 (bFtn	&& pNxtCnt->IsInFtn()) )
/*N*/ 						return pNxtCnt;
/*N*/ 					pNxtCnt = pNxtCnt->GetNextCntntFrm();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( pThis->IsInFly() )
/*N*/ 				return pNxtCnt;
/*N*/ 			else	//Fuss-/oder Kopfbereich
/*N*/ 			{
/*N*/ 				const SwFrm *pUp = pThis->GetUpper();
/*N*/ 				const SwFrm *pCntUp = pNxtCnt->GetUpper();
/*N*/ 				while ( pUp && pUp->GetUpper() &&
/*N*/ 						!pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
/*?*/ 					pUp = pUp->GetUpper();
/*N*/ 				while ( pCntUp && pCntUp->GetUpper() &&
/*N*/ 						!pCntUp->IsHeaderFrm() && !pCntUp->IsFooterFrm() )
/*N*/ 					pCntUp = pCntUp->GetUpper();
/*N*/ 				if ( pCntUp == pUp )
/*?*/ 					return pNxtCnt;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ SwFrm *SwFrm::_FindPrev()
/*N*/ {
/*N*/ 	BOOL bIgnoreTab = FALSE;
/*N*/ 	SwFrm *pThis = this;
/*N*/ 
/*N*/ 	if ( IsTabFrm() )
/*N*/ 	{
/*N*/ 		//Der erste Cntnt der Tabelle wird
/*N*/ 		//gegriffen und dessen Vorgaenger geliefert. Um die Spezialbeh.
/*N*/ 		//Fuer Tabellen (s.u.) auszuschalten wird bIgnoreTab gesetzt.
/*N*/ 		pThis = ((SwTabFrm*)this)->ContainsCntnt();
/*N*/ 		bIgnoreTab = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pThis->IsCntntFrm() )
/*N*/ 	{
/*N*/ 		SwCntntFrm *pPrvCnt = ((SwCntntFrm*)pThis)->GetPrevCntntFrm();
/*N*/ 		if( !pPrvCnt )
/*N*/ 			return 0;
/*N*/ 		if ( !bIgnoreTab && pThis->IsInTab() )
/*N*/ 		{
/*?*/ 			SwLayoutFrm *pUp = pThis->GetUpper();
/*?*/ 			while ( !pUp->IsCellFrm() )
/*?*/ 				pUp = pUp->GetUpper();
/*?*/ 			ASSERT( pUp, "Cntnt in Tabelle aber nicht in Zelle." );
/*?*/ 			if ( pUp->IsAnLower( pPrvCnt ) )
/*?*/ 				return pPrvCnt;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwFrm* pRet;
/*N*/ 			const BOOL bBody = pThis->IsInDocBody();
/*N*/ 			const BOOL bFtn  = bBody ? FALSE : pThis->IsInFtn();
/*N*/ 			if ( bBody || bFtn )
/*N*/ 			{
/*N*/ 				while ( pPrvCnt )
/*N*/ 				{
/*N*/ 					if ( (bBody && pPrvCnt->IsInDocBody()) ||
/*N*/ 							(bFtn	&& pPrvCnt->IsInFtn()) )
/*N*/ 					{
/*N*/ 						pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
/*N*/ 												  : (SwFrm*)pPrvCnt;
/*N*/ 						return pRet;
/*N*/ 					}
/*N*/ 					pPrvCnt = pPrvCnt->GetPrevCntntFrm();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( pThis->IsInFly() )
/*N*/ 			{
/*N*/ 				pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
/*N*/ 											: (SwFrm*)pPrvCnt;
/*N*/ 				return pRet;
/*N*/ 			}
/*N*/ 			else	//Fuss-/oder Kopfbereich oder Fly
/*N*/ 			{
/*?*/ 				const SwFrm *pUp = pThis->GetUpper();
/*?*/ 				const SwFrm *pCntUp = pPrvCnt->GetUpper();
/*?*/ 				while ( pUp && pUp->GetUpper() &&
/*?*/ 						!pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
/*?*/ 					pUp = pUp->GetUpper();
/*?*/ 				while ( pCntUp && pCntUp->GetUpper() )
/*?*/ 					pCntUp = pCntUp->GetUpper();
/*?*/ 				if ( pCntUp == pUp )
/*?*/ 				{
/*?*/ 					pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
/*?*/ 												: (SwFrm*)pPrvCnt;
/*?*/ 					return pRet;
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ void SwFrm::ImplInvalidateNextPos( BOOL bNoFtn )
/*N*/ {
/*N*/ 	SwFrm *pFrm;
/*N*/ 	if ( 0 != (pFrm = _FindNext()) )
/*N*/ 	{
/*N*/ 		if( pFrm->IsSctFrm() )
/*N*/ 		{
/*N*/ 			while( pFrm && pFrm->IsSctFrm() )
/*N*/ 			{
/*N*/ 				if( ((SwSectionFrm*)pFrm)->GetSection() )
/*N*/ 				{
/*N*/ 					SwFrm* pTmp = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 					if( pTmp )
/*N*/                         pTmp->InvalidatePos();
/*N*/ 					else if( !bNoFtn )
/*N*/ 						((SwSectionFrm*)pFrm)->InvalidateFtnPos();
/*N*/ 					if( !IsInSct() || FindSctFrm()->GetFollow() != pFrm )
/*N*/ 						pFrm->InvalidatePos();
/*N*/ 					return;
/*N*/ 				}
/*N*/ 				pFrm = pFrm->FindNext();
/*N*/ 			}
/*N*/ 			if( pFrm )
/*N*/ 			{
/*N*/ 				if ( pFrm->IsSctFrm())
/*N*/ 				{ // Damit der Inhalt eines Bereichs die Chance erhaelt,
/*?*/ 				  // die Seite zu wechseln, muss er ebenfalls invalidiert werden.
/*?*/ 					SwFrm* pTmp = ((SwSectionFrm*)pFrm)->ContainsAny();
/*?*/ 					if( pTmp )
/*?*/                         pTmp->InvalidatePos();
/*?*/ 					if( !IsInSct() || FindSctFrm()->GetFollow() != pFrm )
/*?*/ 						pFrm->InvalidatePos();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					pFrm->InvalidatePos();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFrm->InvalidatePos();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  lcl_IsInColSect()
|*		liefert nur TRUE, wenn der Frame _direkt_ in einem spaltigen Bereich steht,
|*		nicht etwa, wenn er in einer Tabelle steht, die in einem spaltigen Bereich ist.
|*
|*************************************************************************/

/*N*/ BOOL lcl_IsInColSct( const SwFrm *pUp )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	while( pUp )
/*N*/ 	{
/*N*/ 		if( pUp->IsColumnFrm() )
/*N*/ 			bRet = TRUE;
/*N*/ 		else if( pUp->IsSctFrm() )
/*N*/ 			return bRet;
/*N*/ 		else if( pUp->IsTabFrm() )
/*N*/ 			return FALSE;
/*N*/ 		pUp = pUp->GetUpper();
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	  SwFrm::IsMoveable();
|*
|*	  Ersterstellung	MA 09. Mar. 93
|*	  Letzte Aenderung	MA 05. May. 95
|*
|*************************************************************************/
/*N*/ BOOL SwFrm::IsMoveable() const
/*N*/ {
/*N*/ 	if ( IsFlowFrm() )
/*N*/ 	{
/*N*/ 		if( IsInSct() && lcl_IsInColSct( GetUpper() ) )
/*?*/ 			return TRUE;
/*N*/         if( IsInFly() || IsInDocBody() || IsInFtn() )
/*N*/ 		{
/*N*/ 			if ( IsInTab() && !IsTabFrm() )
/*N*/ 				return FALSE;
/*N*/ 			BOOL bRet = TRUE;
/*N*/ 			if ( IsInFly() )
/*N*/ 			{
/*N*/ 				//Wenn der Fly noch einen Follow hat ist der Inhalt auf jeden
/*N*/ 				//Fall moveable
/*N*/ 				if ( !((SwFlyFrm*)FindFlyFrm())->GetNextLink() )
/*N*/ 				{
/*N*/ 					//Fuer Inhalt innerhab von Spaltigen Rahmen ist nur der Inhalt
/*N*/ 					//der letzten Spalte nicht moveable.
/*N*/ 					const SwFrm *pCol = GetUpper();
/*N*/ 					while ( pCol && !pCol->IsColumnFrm() )
/*N*/ 						pCol = pCol->GetUpper();
/*N*/ 					if ( !pCol || (pCol && !pCol->GetNext()) )
/*N*/ 						bRet = FALSE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			return bRet;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	  SwFrm::ImplGetNextCntntFrm(), ImplGetPrevCntntFrm()
|*
|*		Rueckwaertswandern im Baum: Den untergeordneten Frm greifen,
|*		wenn es einen gibt und nicht gerade zuvor um eine Ebene
|*		aufgestiegen wurde (das wuerde zu einem endlosen Auf und Ab
|*		fuehren!). Damit wird sichergestellt, dass beim
|*		Rueckwaertswandern alle Unterbaeume durchsucht werden. Wenn
|*		abgestiegen wurde, wird zuerst an das Ende der Kette gegangen,
|*		weil im weiteren ja vom letzten Frm innerhalb eines anderen
|*		Frms rueckwaerts gegangen wird.
|* 		Vorwaetzwander funktioniert analog.
|*
|*	  Ersterstellung	??
|*	  Letzte Aenderung	MA 30. Oct. 97
|*
|*************************************************************************/


// Achtung: Fixes in ImplGetNextCntntFrm() muessen moeglicherweise auch in
// die weiter oben stehende Methode lcl_NextFrm(..) eingepflegt werden
/*M*/ SwCntntFrm* SwFrm::ImplGetNextCntntFrm() const
/*M*/ {
/*M*/ 	const SwFrm *pFrm = this;
/*M*/     // #100926#
/*M*/     SwCntntFrm *pCntntFrm = 0;
/*M*/     FASTBOOL bGoingUp = ! IsCntntFrm();
/*M*/ 	do {
/*M*/ 		const SwFrm *p;
/*M*/ 		FASTBOOL bGoingFwd = FALSE, bGoingDown = FALSE;
/*M*/ 		if( !(bGoingDown = (!bGoingUp && ( 0 !=
/*M*/ 			 (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
/*M*/ 			!(bGoingFwd = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink()
/*M*/ 													  : pFrm->GetNext()))) &&
/*M*/ 			!(bGoingUp = (0 != (p = pFrm->GetUpper()))))
/*M*/ 			return 0;
/*M*/ 		bGoingUp = !(bGoingFwd || bGoingDown);
/*M*/ 		pFrm = p;
/*M*/ 	} while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));
/*M*/ 	return pCntntFrm;
/*M*/ 
/*M*/ }

/*M*/ SwCntntFrm* SwFrm::ImplGetPrevCntntFrm() const
/*M*/ {
/*M*/ 	const SwFrm *pFrm = this;
/*M*/     SwCntntFrm *pCntntFrm = 0;
/*M*/     // #100926#
/*M*/     FASTBOOL bGoingUp = ! IsCntntFrm();
/*M*/ 	do {
/*M*/ 		const SwFrm *p;
/*M*/ 		FASTBOOL bGoingBack = FALSE, bGoingDown = FALSE;
/*M*/ 		if( !(bGoingDown = (!bGoingUp && (0 !=
/*M*/ 			 (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)))) &&
/*M*/ 			!(bGoingBack = (0 != (p = pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetPrevLink()
/*M*/ 													   : pFrm->GetPrev()))) &&
/*M*/ 			!(bGoingUp = (0 != (p = pFrm->GetUpper()))))
/*M*/ 			return 0;
/*M*/ 		bGoingUp = !(bGoingBack || bGoingDown);
/*M*/ 		if( bGoingDown && p )
/*M*/ 			while ( p->GetNext() )
/*M*/ 				p = p->GetNext();
/*M*/ 		pFrm = p;
/*M*/ 	} while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));
/*M*/ 	return pCntntFrm;
/*M*/ }

/*************************************************************************
|*
|*	  SwFrm::SetInfFlags();
|*
|*	  Ersterstellung	MA 05. Apr. 94
|*	  Letzte Aenderung	MA 05. Apr. 94
|*
|*************************************************************************/
/*N*/ void SwFrm::SetInfFlags()
/*N*/ {
/*N*/ 	if ( !IsFlyFrm() && !GetUpper() ) //noch nicht gepastet, keine Informationen
/*N*/ 		return;						  //lieferbar
/*N*/ 
/*N*/ 	bInfInvalid = bInfBody = bInfTab = bInfFly = bInfFtn = bInfSct = FALSE;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	BOOL bIsInTab = FALSE;
/*N*/ #endif
/*N*/ 
/*N*/ 	SwFrm *pFrm = this;
/*N*/ 	if( IsFtnContFrm() )
/*N*/ 		bInfFtn = TRUE;
/*N*/ 	do
/*N*/ 	{   // bInfBody wird nur am Seitenbody, nicht im ColumnBody gesetzt
/*N*/ 		if ( pFrm->IsBodyFrm() && !bInfFtn && pFrm->GetUpper()
/*N*/ 			 && pFrm->GetUpper()->IsPageFrm() )
/*N*/ 			bInfBody = TRUE;
/*N*/ 		else if ( pFrm->IsTabFrm() || pFrm->IsCellFrm() )
/*N*/ 		{
/*N*/ #ifdef DBG_UTIL
/*N*/ 			if( pFrm->IsTabFrm() )
/*N*/ 			{
/*N*/ 				ASSERT( !bIsInTab, "Table in table: Not implemented." );
/*N*/ 				bIsInTab = TRUE;
/*N*/ 			}
/*N*/ #endif
/*N*/ 			bInfTab = TRUE;
/*N*/ 		}
/*N*/ 		else if ( pFrm->IsFlyFrm() )
/*N*/ 			bInfFly = TRUE;
/*N*/ 		else if ( pFrm->IsSctFrm() )
/*N*/ 			bInfSct = TRUE;
/*N*/ 		else if ( pFrm->IsFtnFrm() )
/*N*/ 			bInfFtn = TRUE;
/*N*/ 
/*N*/ //MA: 06. Apr. 94, oberhalb eines Fly geht es nicht weiter!
/*N*/ //		if ( pFrm->IsFlyFrm() )
/*N*/ //			pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
/*N*/ //		else
/*N*/ 			pFrm = pFrm->GetUpper();
/*N*/ 
/*N*/ 	} while ( pFrm && !pFrm->IsPageFrm() ); //Oberhalb der Seite kommt nix
/*N*/ }

/*-----------------22.8.2001 14:30------------------
 * SwFrm::SetDirFlags( BOOL )
 * actualizes the vertical or the righttoleft-flags.
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 * --------------------------------------------------*/

/*N*/ void SwFrm::SetDirFlags( BOOL bVert )
/*N*/ {
/*N*/     if( bVert )
/*N*/     {
/*N*/         USHORT bInv = 0;
/*N*/         if( bDerivedVert )
/*N*/         {
/*N*/             SwFrm* pAsk = IsFlyFrm() ?
/*N*/                           ((SwFlyFrm*)this)->GetAnchor() : GetUpper();
/*N*/             if( pAsk )
/*N*/             {
/*N*/                 bVertical = pAsk->IsVertical() ? 1 : 0;
/*N*/                 bReverse  = pAsk->IsReverse()  ? 1 : 0;
/*N*/             }
/*N*/             if( !pAsk || pAsk->bInvalidVert )
/*N*/                 bInv = bInvalidVert;
/*N*/         }
/*N*/         else
/*N*/             CheckDirection( bVert );
/*N*/         bInvalidVert = bInv;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         BOOL bInv = 0;
/*N*/         if( !bDerivedR2L ) // CheckDirection is able to set bDerivedR2L!
/*N*/             CheckDirection( bVert );
/*N*/         if( bDerivedR2L )
/*N*/         {
/*N*/             SwFrm* pAsk = IsFlyFrm() ?
/*N*/                           ((SwFlyFrm*)this)->GetAnchor() : GetUpper();
/*N*/             if( pAsk )
/*N*/                 bRightToLeft = pAsk->IsRightToLeft() ? 1 : 0;
/*N*/             if( !pAsk || pAsk->bInvalidR2L )
/*N*/                 bInv = bInvalidR2L;
/*N*/         }
/*N*/         bInvalidR2L = bInv;
/*N*/     }
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
