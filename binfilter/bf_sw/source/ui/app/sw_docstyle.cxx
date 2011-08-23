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

#define _SVSTDARR_USHORTS

#include <uiparam.hxx>
#include <hintids.hxx>

#include <bf_svtools/itemiter.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svx/boxitem.hxx>

#include <errhdl.hxx>

#include <fmtcol.hxx>
#include <uitool.hxx>

#include <horiornt.hxx>

#include <wrtsh.hxx>
#include <docsh.hxx>
#include <errhdl.hxx>
#include <charfmt.hxx>
#include <poolfmt.hxx>
#include <docstyle.hxx>
#include <docary.hxx>
#include <ccoll.hxx>
#include <doc.hxx>
#include <uiitems.hxx>

#include <cmdid.h>
#include <swstyle.h>
#include <app.hrc>
#include <SwStyleNameMapper.hxx>
namespace binfilter {

// MD 06.02.95: Die Formatnamen in der Liste aller Namen haben als
// erstes Zeichen die Familie:

#define	cCHAR		(sal_Unicode)'c'
#define	cPARA		(sal_Unicode)'p'
#define	cFRAME		(sal_Unicode)'f'
#define	cPAGE		(sal_Unicode)'g'
#define cNUMRULE	(sal_Unicode)'n'

// Dieses Zeichen wird bei der Herausgabe der Namen wieder entfernt und
// die Familie wird neu generiert.

// Ausserdem gibt es jetzt zusaetzlich das Bit bPhysical. Ist dieses Bit
// TRUE, werden die Pool-Formatnamen NICHT mit eingetragen.


/*N*/ class SwImplShellAction
/*N*/ {
/*N*/ 	SwWrtShell* pSh;
/*N*/ 	CurrShell* pCurrSh;
/*N*/ public:
/*N*/ 	SwImplShellAction( SwDoc& rDoc );
/*N*/ 	~SwImplShellAction();
/*N*/ 
/*N*/ 	SwWrtShell* GetSh() { return pSh; }
/*N*/ };

/*N*/ SwImplShellAction::SwImplShellAction( SwDoc& rDoc )
/*N*/ 	: pCurrSh( 0 )
/*N*/ {
/*N*/ 	if( rDoc.GetDocShell() )
/*N*/ 		pSh = rDoc.GetDocShell()->GetWrtShell();
/*N*/ 	else
/*N*/ 		pSh = 0;
/*N*/ 
/*N*/ 	if( pSh )
/*N*/ 	{
/*N*/ 		pCurrSh = new CurrShell( pSh );
/*N*/ 		pSh->StartAllAction();
/*N*/ 	}
/*N*/ }

/*N*/ SwImplShellAction::~SwImplShellAction()
/*N*/ {
/*N*/ 	if( pCurrSh )
/*N*/ 	{
/*N*/ 		pSh->EndAllAction();
/*N*/ 		delete pCurrSh;
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	SwCharFormate finden/anlegen
                    evtl. Style fuellen
 --------------------------------------------------------------------*/

/*N*/ SwCharFmt* lcl_FindCharFmt(	SwDoc& rDoc,
/*N*/ 							const String& rName,
/*N*/ 							SwDocStyleSheet* pStyle = 0,
/*N*/ 							FASTBOOL bCreate = TRUE )
/*N*/ {
/*N*/ 	SwCharFmt* 	pFmt = 0;
/*N*/ 	if( rName.Len() )
/*N*/ 	{
/*N*/ 		pFmt = rDoc.FindCharFmtByName( rName );
/*N*/ 		if( !pFmt && rName == *SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
/*N*/ 												RES_POOLCOLL_TEXT_BEGIN ] )
/*N*/ 		{
/*N*/ 			// Standard-Zeichenvorlage
/*?*/ 			pFmt = (SwCharFmt*)rDoc.GetDfltCharFmt();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !pFmt && bCreate )
/*N*/ 		{	// Pool abklappern
/*N*/ 			const USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, GET_POOLID_CHRFMT);
/*N*/ 			if(nId != USHRT_MAX)
/*N*/ 				pFmt = rDoc.GetCharFmtFromPool(nId);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if(pStyle)
/*N*/ 	{
/*N*/ 		if(pFmt)
/*N*/ 		{
/*N*/ 			pStyle->SetPhysical(TRUE);
/*N*/ 			SwFmt* p = pFmt->DerivedFrom();
/*N*/ 			if( p && !p->IsDefault() )
/*N*/ 				pStyle->PresetParent( p->GetName() );
/*N*/ 			else
/*N*/ 				pStyle->PresetParent( aEmptyStr );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pStyle->SetPhysical(FALSE);
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: 	ParaFormate finden/erzeugen
                    Style fuellen
 --------------------------------------------------------------------*/

/*N*/ SwTxtFmtColl* lcl_FindParaFmt(	SwDoc& rDoc,
/*N*/ 								const String& rName,
/*N*/ 								SwDocStyleSheet* pStyle = 0,
/*N*/ 								FASTBOOL bCreate = TRUE )
/*N*/ {
/*N*/ 	SwTxtFmtColl* 	pColl = 0;
/*N*/ 
/*N*/ 	if( rName.Len() )
/*N*/ 	{
/*N*/ 		pColl = rDoc.FindTxtFmtCollByName( rName );
/*N*/ 		if( !pColl && bCreate )
/*N*/ 		{	// Pool abklappern
/*N*/ 			const USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, GET_POOLID_TXTCOLL);
/*N*/ 			if(nId != USHRT_MAX)
/*N*/ 				pColl = rDoc.GetTxtCollFromPool(nId);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(pStyle)
/*N*/ 	{
/*N*/ 		if(pColl)
/*N*/ 		{
/*N*/ 			pStyle->SetPhysical(TRUE);
/*N*/ 			if( pColl->DerivedFrom() && !pColl->DerivedFrom()->IsDefault() )
/*N*/ 				pStyle->PresetParent( pColl->DerivedFrom()->GetName() );
/*N*/ 			else
/*N*/ 				pStyle->PresetParent( aEmptyStr );
/*N*/ 
/*N*/ 			SwTxtFmtColl& rNext = pColl->GetNextTxtFmtColl();
/*N*/ 			pStyle->PresetFollow(rNext.GetName());
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pStyle->SetPhysical(FALSE);
/*N*/ 	}
/*N*/ 	return pColl;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung:	Rahmenformate
 --------------------------------------------------------------------*/


/*N*/ SwFrmFmt* lcl_FindFrmFmt(	SwDoc& rDoc,
/*N*/ 							const String& rName,
/*N*/ 							SwDocStyleSheet* pStyle = 0,
/*N*/ 							FASTBOOL bCreate = TRUE )
/*N*/ {
/*N*/ 	SwFrmFmt* pFmt = 0;
/*N*/ 	if( rName.Len() )
/*N*/ 	{
/*N*/ 		pFmt = rDoc.FindFrmFmtByName( rName );
/*N*/ 		if( !pFmt && bCreate )
/*N*/ 		{	// Pool abklappern
/*N*/ 			const USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, GET_POOLID_FRMFMT);
/*N*/ 			if(nId != USHRT_MAX)
/*N*/ 				pFmt = rDoc.GetFrmFmtFromPool(nId);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(pStyle)
/*N*/ 	{
/*N*/ 		if(pFmt)
/*N*/ 		{
/*N*/ 			pStyle->SetPhysical(TRUE);
/*N*/ 			if( pFmt->DerivedFrom() && !pFmt->DerivedFrom()->IsDefault() )
/*?*/ 				pStyle->PresetParent( pFmt->DerivedFrom()->GetName() );
/*N*/ 			else
/*N*/ 				pStyle->PresetParent( aEmptyStr );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pStyle->SetPhysical(FALSE);
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Seitendescriptoren
 --------------------------------------------------------------------*/


/*N*/ const SwPageDesc* lcl_FindPageDesc(	SwDoc&	rDoc,
/*N*/ 									const String& 	 rName,
/*N*/ 									SwDocStyleSheet* pStyle = 0,
/*N*/ 									FASTBOOL bCreate = TRUE )
/*N*/ {
/*N*/ 	const SwPageDesc* pDesc = 0;
/*N*/ 
/*N*/ 	if( rName.Len() )
/*N*/ 	{
/*N*/ 		pDesc = rDoc.FindPageDescByName( rName );
/*N*/ 		if( !pDesc && bCreate )
/*N*/ 		{
/*N*/ 			USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, GET_POOLID_PAGEDESC);
/*N*/ 			if(nId != USHRT_MAX)
/*N*/ 				pDesc = rDoc.GetPageDescFromPool(nId);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(pStyle)
/*N*/ 	{
/*N*/ 		if(pDesc)
/*N*/ 		{
/*N*/ 			pStyle->SetPhysical(TRUE);
/*N*/ 			if(pDesc->GetFollow())
/*N*/ 				pStyle->PresetFollow(pDesc->GetFollow()->GetName());
/*N*/ 			else
/*?*/ 				pStyle->PresetParent( aEmptyStr );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pStyle->SetPhysical(FALSE);
/*N*/ 	}
/*N*/ 	return pDesc;
/*N*/ }

/*N*/ const SwNumRule* lcl_FindNumRule(	SwDoc&	rDoc,
/*N*/ 									const String& 	 rName,
/*N*/ 									SwDocStyleSheet* pStyle = 0,
/*N*/ 									FASTBOOL bCreate = TRUE )
/*N*/ {
/*N*/ 	const SwNumRule* pRule = 0;
/*N*/ 
/*N*/ 	if( rName.Len() )
/*N*/ 	{
/*N*/ 		pRule = rDoc.FindNumRulePtr( rName );
/*N*/ 		if( !pRule && bCreate )
/*N*/ 		{
/*N*/ 			USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, GET_POOLID_NUMRULE);
/*N*/ 			if(nId != USHRT_MAX)
/*N*/ 				pRule = rDoc.GetNumRuleFromPool(nId);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(pStyle)
/*N*/ 	{
/*N*/ 		if(pRule)
/*N*/ 		{
/*N*/ 			pStyle->SetPhysical(TRUE);
/*N*/ 			pStyle->PresetParent( aEmptyStr );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pStyle->SetPhysical(FALSE);
/*N*/ 	}
/*N*/ 	return pRule;
/*N*/ }





/*--------------------------------------------------------------------
    Beschreibung:	Einfuegen von Strings in die Liste der Vorlagen
 --------------------------------------------------------------------*/


/*N*/ void SwPoolFmtList::Append( char cChar, const String& rStr )
/*N*/ {
/*N*/ 	String* pStr = new String( cChar );
/*N*/ 	*pStr += rStr;
/*N*/ 	for ( USHORT i=0; i < Count(); ++i )
/*N*/ 	{
/*N*/ 		if( *operator[](i) == *pStr )
/*N*/ 		{
/*N*/ 			delete pStr;
/*N*/ 			return;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	Insert( pStr, Count() );
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Liste kompletti loeschen
 --------------------------------------------------------------------*/


/*N*/ void SwPoolFmtList::Erase()
/*N*/ {
/*N*/ 	DeleteAndDestroy( 0, Count() );
/*N*/ }

/*  */

/*--------------------------------------------------------------------
    Beschreibung:  UI-seitige implementierung von StyleSheets
                   greift auf die Core-Engine zu
 --------------------------------------------------------------------*/

/*N*/ SwDocStyleSheet::SwDocStyleSheet(	SwDoc&			rDocument,
/*N*/ 									const String& 			rName,
/*N*/ 									SwDocStyleSheetPool& 	rPool,
/*N*/ 									SfxStyleFamily 			eFam,
/*N*/ 									USHORT 					nMask) :
/*N*/ 
/*N*/ 	SfxStyleSheetBase( rName, rPool, eFam, nMask ),
/*N*/ 	rDoc(rDocument),
/*N*/ 	pCharFmt(0),
/*N*/ 	pFrmFmt(0),
/*N*/ 	pColl(0),
/*N*/ 	pDesc(0),
/*N*/ 	pNumRule(0),
/*N*/ 	bPhysical(FALSE),
/*N*/ 	aCoreSet(GetPool().GetPool(),
/*N*/ 			RES_CHRATR_BEGIN,		RES_CHRATR_END - 1,
/*N*/ 			RES_PARATR_BEGIN, 		RES_PARATR_END - 1,
/*N*/ 			RES_FRMATR_BEGIN, 		RES_FRMATR_END - 1,
/*N*/ 			RES_UNKNOWNATR_BEGIN, 	RES_UNKNOWNATR_END-1,
/*N*/ 			SID_ATTR_PAGE, 			SID_ATTR_PAGE_EXT1,
/*N*/ 			SID_ATTR_PAGE_HEADERSET,SID_ATTR_PAGE_FOOTERSET,
/*N*/ 			SID_ATTR_BORDER_INNER, 	SID_ATTR_BORDER_INNER,
/*N*/ 			FN_PARAM_FTN_INFO, 		FN_PARAM_FTN_INFO,
/*N*/ 			SID_ATTR_PARA_MODEL,	SID_ATTR_PARA_MODEL,
/*N*/ 			SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM,
/*N*/ 			SID_SWREGISTER_MODE,	SID_SWREGISTER_COLLECTION,
/*N*/ 			FN_COND_COLL,			FN_COND_COLL,
/*N*/ 			SID_ATTR_AUTO_STYLE_UPDATE,	SID_ATTR_AUTO_STYLE_UPDATE,
/*N*/ 			SID_ATTR_NUMBERING_RULE,	SID_ATTR_NUMBERING_RULE,
/*N*/ 			SID_PARA_BACKGRND_DESTINATION,	SID_ATTR_BRUSH_CHAR,
/*N*/ 			SID_ATTR_NUMBERING_RULE, 	SID_ATTR_NUMBERING_RULE,
/*N*/ 			0)
/*N*/ {
/*N*/ 	nHelpId = UCHAR_MAX;
/*N*/ }


/*N*/ SwDocStyleSheet::SwDocStyleSheet( const SwDocStyleSheet& rOrg) :
/*N*/ 	SfxStyleSheetBase(rOrg),
/*N*/ 	pCharFmt(rOrg.pCharFmt),
/*N*/ 	pFrmFmt(rOrg.pFrmFmt),
/*N*/ 	pColl(rOrg.pColl),
/*N*/ 	pDesc(rOrg.pDesc),
/*N*/ 	pNumRule(rOrg.pNumRule),
/*N*/ 	rDoc(rOrg.rDoc),
/*N*/ 	bPhysical(rOrg.bPhysical),
/*N*/ 	aCoreSet(rOrg.aCoreSet)
/*N*/ {
/*N*/ }


/*N*/  SwDocStyleSheet::~SwDocStyleSheet()
/*N*/ {
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Zuruecksetzen
 --------------------------------------------------------------------*/


/*N*/ void  SwDocStyleSheet::Reset()
/*N*/ {
/*N*/ 	aName.Erase();
/*N*/ 	aFollow.Erase();
/*N*/ 	aParent.Erase();
/*N*/ 	SetPhysical(FALSE);
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	virtuelle Methoden
 --------------------------------------------------------------------*/


/*N*/ const String&  SwDocStyleSheet::GetParent() const
/*N*/ {
/*N*/ 	if( !bPhysical )
/*N*/ 	{
/*?*/ 		// dann pruefe, ob schon im Doc vorhanden
/*?*/ 		SwFmt* pFmt = 0;
/*?*/ 		SwGetPoolIdFromName eGetType;
/*?*/ 		switch(nFamily)
/*?*/ 		{
/*?*/ 		case SFX_STYLE_FAMILY_CHAR:
/*?*/ 			pFmt = rDoc.FindCharFmtByName( aName );
/*?*/ 			eGetType = GET_POOLID_CHRFMT;
/*?*/ 			break;
/*?*/ 
/*?*/ 		case SFX_STYLE_FAMILY_PARA:
/*?*/ 			pFmt = rDoc.FindTxtFmtCollByName( aName );
/*?*/ 			eGetType = GET_POOLID_TXTCOLL;
/*?*/ 			break;
/*?*/ 
/*?*/ 		case SFX_STYLE_FAMILY_FRAME:
/*?*/ 			pFmt = rDoc.FindFrmFmtByName( aName );
/*?*/ 			eGetType = GET_POOLID_FRMFMT;
/*?*/ 			break;
/*?*/ 
/*?*/ 		case SFX_STYLE_FAMILY_PAGE:
/*?*/ 		case SFX_STYLE_FAMILY_PSEUDO:
/*?*/ 		default:
/*?*/ 			return aEmptyStr;		// es gibt keinen Parent
/*?*/ 		}
/*?*/ 
/*?*/ 		String sTmp;
/*?*/ 		if( !pFmt )			// noch nicht vorhanden, also dflt. Parent
/*?*/ 		{
/*?*/ 			USHORT i = SwStyleNameMapper::GetPoolIdFromUIName( aName, eGetType );
/*?*/ 			i = ::binfilter::GetPoolParent( i );
/*?*/ 			if( i && USHRT_MAX != i )
/*?*/ 				SwStyleNameMapper::FillUIName( i, sTmp );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			SwFmt* p = pFmt->DerivedFrom();
/*?*/ 			if( p && !p->IsDefault() )
/*?*/ 				sTmp = p->GetName();
/*?*/ 		}
/*?*/ 		SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
/*?*/ 		pThis->aParent = sTmp;
/*N*/ 	}
/*N*/ 	return aParent;
/*N*/ }

/*--------------------------------------------------------------------
   Beschreibung:	Nachfolger
 --------------------------------------------------------------------*/


/*N*/ const String&  SwDocStyleSheet::GetFollow() const
/*N*/ {
/*N*/ 	if( !bPhysical )
/*N*/ 	{
/*?*/ 		SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
/*?*/ 		pThis->FillStyleSheet( FillAllInfo );
/*N*/ 	}
/*N*/ 	return aFollow;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Welche Verkettung ist moeglich
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung:	Parent ?
 --------------------------------------------------------------------*/





/*--------------------------------------------------------------------
    Beschreibung:	textuelle Beschreibung ermitteln
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung:	Namen setzen
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung:	Ableitungshirachie
 --------------------------------------------------------------------*/


/*N*/ BOOL   SwDocStyleSheet::SetParent( const String& rStr)
/*N*/ {
/*N*/ 	SwFmt* pFmt = 0, *pParent = 0;
/*N*/ 	switch(nFamily)
/*N*/ 	{
/*N*/ 		case SFX_STYLE_FAMILY_CHAR :
/*N*/ 			ASSERT( pCharFmt, "SwCharFormat fehlt!" )
/*N*/ 			if( 0 != ( pFmt = pCharFmt ) && rStr.Len() )
/*N*/ 				pParent = lcl_FindCharFmt(rDoc, rStr);
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PARA :
/*N*/ 			ASSERT( pColl, "Collektion fehlt!")
/*N*/ 			if( 0 != ( pFmt = pColl ) && rStr.Len() )
/*N*/ 				pParent = lcl_FindParaFmt( rDoc, rStr );
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_FRAME:
/*?*/ 			ASSERT(pFrmFmt, "FrameFormat fehlt!");
/*?*/ 			if( 0 != ( pFmt = pFrmFmt ) && rStr.Len() )
/*?*/ 				pParent = lcl_FindFrmFmt( rDoc, rStr );
/*?*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PAGE:
/*N*/ 		case SFX_STYLE_FAMILY_PSEUDO:
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			ASSERT(!this, "unbekannte Style-Familie");
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if( pFmt && pFmt->DerivedFrom() &&
/*N*/ 		pFmt->DerivedFrom()->GetName() != rStr )
/*N*/ 	{
/*N*/ 		{
/*N*/ 			SwImplShellAction aTmp( rDoc );
/*N*/ 			bRet = pFmt->SetDerivedFrom( pParent );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bRet )
/*N*/ 		{
/*N*/ 			aParent = rStr;
/*N*/ 			rPool.Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED,
/*N*/ 							*this ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Nachfolger detzen
 --------------------------------------------------------------------*/


/*N*/ BOOL   SwDocStyleSheet::SetFollow( const String& rStr)
/*N*/ {
/*N*/ 	if( rStr.Len() && !SfxStyleSheetBase::SetFollow( rStr ))
/*?*/ 		return FALSE;
/*N*/ 
/*N*/ 	SwImplShellAction aTmpSh( rDoc );
/*N*/ 	switch(nFamily)
/*N*/ 	{
/*N*/ 	case SFX_STYLE_FAMILY_PARA :
/*N*/ 	{
/*N*/ 		ASSERT(pColl, "Collection fehlt!");
/*N*/ 		if( pColl )
/*N*/ 		{
/*N*/ 			SwTxtFmtColl* pFollow = pColl;
/*N*/ 			if( rStr.Len() && 0 == (pFollow = lcl_FindParaFmt(rDoc, rStr) ))
/*?*/ 				pFollow = pColl;
/*N*/ 
/*N*/ 			pColl->SetNextTxtFmtColl(*pFollow);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	case SFX_STYLE_FAMILY_PAGE :
/*N*/ 	{
/*N*/ 		ASSERT(pDesc, "PageDesc fehlt!");
/*N*/ 		if( pDesc )
/*N*/ 		{
/*N*/ 			const SwPageDesc* pFollowDesc = rStr.Len()
/*N*/ 											? lcl_FindPageDesc(rDoc, rStr)
/*N*/ 											: 0;
/*N*/ 			USHORT nId;
/*N*/ 			if( rDoc.FindPageDescByName( pDesc->GetName(), &nId ))
/*N*/ 			{
/*N*/ 				SwPageDesc aDesc( *pDesc );
/*N*/ 				aDesc.SetFollow( pFollowDesc );
/*N*/ 				rDoc.ChgPageDesc( nId, aDesc );
/*N*/ 				pDesc = &rDoc.GetPageDesc( nId );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	case SFX_STYLE_FAMILY_CHAR:
/*N*/ 	case SFX_STYLE_FAMILY_FRAME:
/*N*/ 	case SFX_STYLE_FAMILY_PSEUDO:
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		ASSERT(!this, "unbekannte Style-Familie");
/*N*/ 	}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	ueber Name und Family, Mask den ItemSet rausholen
 --------------------------------------------------------------------*/

/*N*/ SfxItemSet&   SwDocStyleSheet::GetItemSet()
/*N*/ {
/*N*/ 	if(!bPhysical)
/*?*/ 		FillStyleSheet( FillPhysical );
/*N*/ 
/*N*/ 	switch(nFamily)
/*N*/ 	{
/*N*/ 		case SFX_STYLE_FAMILY_CHAR:
/*N*/ 			{
/*N*/ 				ASSERT(pCharFmt, "Wo ist das SwCharFmt");
/*N*/ 				aCoreSet.Put(pCharFmt->GetAttrSet());
/*N*/ 				if(pCharFmt->DerivedFrom())
/*N*/ 					aCoreSet.SetParent(&pCharFmt->DerivedFrom()->GetAttrSet());
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case SFX_STYLE_FAMILY_PARA :
/*N*/ 		case SFX_STYLE_FAMILY_FRAME:
/*N*/ 			{
/*N*/ 				SvxBoxInfoItem aBoxInfo;
/*N*/ 				aBoxInfo.SetTable( FALSE );
/*N*/ 				aBoxInfo.SetDist( TRUE);	// Abstandsfeld immer anzeigen
/*N*/ 				aBoxInfo.SetMinDist( TRUE );// Minimalgroesse in Tabellen und Absaetzen setzen
/*N*/ 				aBoxInfo.SetDefDist( MIN_BORDER_DIST );// Default-Abstand immer setzen
/*N*/ 					// Einzelne Linien koennen nur in Tabellen DontCare-Status haben
/*N*/ 				aBoxInfo.SetValid( VALID_DISABLE, TRUE );
/*N*/ 				if ( nFamily == SFX_STYLE_FAMILY_PARA )
/*N*/ 				{
/*N*/ 					ASSERT(pColl, "Wo ist die Collektion");
/*N*/ 					aCoreSet.Put(pColl->GetAttrSet());
/*N*/ 					aCoreSet.Put( aBoxInfo );
/*N*/ 					aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pColl->IsAutoUpdateFmt()));
/*N*/ 					if(pColl->DerivedFrom())
/*N*/ 						aCoreSet.SetParent(&pColl->DerivedFrom()->GetAttrSet());
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					ASSERT(pFrmFmt, "Wo ist das FrmFmt");
/*N*/ 					aCoreSet.Put(pFrmFmt->GetAttrSet());
/*N*/ 					aCoreSet.Put( aBoxInfo );
/*N*/ 					aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pFrmFmt->IsAutoUpdateFmt()));
/*N*/ 					if(pFrmFmt->DerivedFrom())
/*N*/ 						aCoreSet.SetParent(&pFrmFmt->DerivedFrom()->GetAttrSet());
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PAGE :
/*N*/ 			{
/*N*/ 				ASSERT(pDesc, "Kein PageDescriptor");
/*N*/ 				::binfilter::PageDescToItemSet(*((SwPageDesc*)pDesc), aCoreSet);
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PSEUDO:
/*N*/ 			{
/*N*/ 				ASSERT(pNumRule, "Keine NumRule");
/*N*/ 				SvxNumRule aRule = pNumRule->MakeSvxNumRule();
/*N*/ 				aCoreSet.Put(SvxNumBulletItem(aRule));
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		default:
/*N*/ 			ASSERT(!this, "unbekannte Style-Familie");
/*N*/ #endif
/*N*/ 	}
/*N*/ 	// Member der Basisklasse
/*N*/ 	pSet = &aCoreSet;
/*N*/ 
/*N*/ 	return aCoreSet;
/*N*/ }
/*N*/ 
/*--------------------------------------------------------------------
    Beschreibung:	ItemSet setzen
 --------------------------------------------------------------------*/


/*N*/ void   SwDocStyleSheet::SetItemSet(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// gegebenenfalls Format erst ermitteln
/*N*/ 	if(!bPhysical)
/*?*/ 		FillStyleSheet( FillPhysical );
/*N*/ 
/*N*/ 	SwImplShellAction aTmpSh( rDoc );
/*N*/ 
/*N*/ 	ASSERT( &rSet != &aCoreSet, "SetItemSet mit eigenem Set ist nicht erlaubt" );
/*N*/ 
/*N*/ 	SwFmt* pFmt = 0;
/*N*/ 	SwPageDesc* pNewDsc = 0;
/*N*/ 	USHORT nPgDscPos;
/*N*/ 
/*N*/ 	switch(nFamily)
/*N*/ 	{
/*N*/ 		case SFX_STYLE_FAMILY_CHAR :
/*N*/ 			{
/*N*/ 				ASSERT(pCharFmt, "Wo ist das CharFormat");
/*N*/ 				pFmt = pCharFmt;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PARA :
/*N*/ 		{
/*N*/ 			ASSERT(pColl, "Wo ist die Collection");
/*N*/ 			const SfxPoolItem* pAutoUpdate;
/*N*/ 			if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,FALSE, &pAutoUpdate ))
/*N*/ 			{
/*N*/ 				pColl->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
/*N*/ 			}
/*N*/ 
/*N*/ 			const SwCondCollItem* pCondItem;
/*N*/ 			if( SFX_ITEM_SET != rSet.GetItemState( FN_COND_COLL, FALSE,
/*N*/ 				(const SfxPoolItem**)&pCondItem ))
/*N*/ 				pCondItem = 0;
/*N*/ 
/*N*/ 			if( RES_CONDTXTFMTCOLL == pColl->Which() && pCondItem )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwFmt* pFindFmt;
/*N*/ 			}
/*N*/ 			else if( pCondItem && !pColl->GetDepends() )
/*N*/ 			{
/*?*/ 				// keine bedingte Vorlage, dann erstmal erzeugen und
/*?*/ 				// alle wichtigen Werte uebernehmen
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwConditionTxtFmtColl* pCColl = rDoc.MakeCondTxtFmtColl(
/*N*/ 			}
/*N*/ 			pFmt = pColl;

/*N*/ 			USHORT nId = pColl->GetPoolFmtId() &
/*N*/ 							~ ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID );
/*N*/ 			switch( GetMask() & ( 0x0fff & ~SWSTYLEBIT_CONDCOLL ) )
/*N*/ 			{
/*N*/ 				case SWSTYLEBIT_TEXT:
/*N*/ 					nId |= COLL_TEXT_BITS;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_CHAPTER:
/*N*/ 					nId |= COLL_DOC_BITS;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_LIST:
/*N*/ 					nId |= COLL_LISTS_BITS;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_IDX:
/*N*/ 					nId |= COLL_REGISTER_BITS;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_EXTRA:
/*N*/ 					nId |= COLL_EXTRA_BITS;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_HTML:
/*N*/ 					nId |= COLL_HTML_BITS;
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			pColl->SetPoolFmtId( nId );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SFX_STYLE_FAMILY_FRAME:
/*N*/ 		{
/*N*/ 			ASSERT(pFrmFmt, "Wo ist das FrmFmt");
/*N*/ 			const SfxPoolItem* pAutoUpdate;
/*N*/ 			if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,FALSE, &pAutoUpdate ))
/*N*/ 			{
/*N*/ 				pFrmFmt->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
/*N*/ 			}
/*N*/ 			pFmt = pFrmFmt;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PAGE :
/*N*/ 			{
/*N*/ 				ASSERT(pDesc, "Wo ist der PageDescriptor");
/*N*/ 
/*N*/ 				if( rDoc.FindPageDescByName( pDesc->GetName(), &nPgDscPos ))
/*N*/ 				{
/*N*/ 					pNewDsc = new SwPageDesc( *pDesc );
/*N*/ 					pFmt = &pNewDsc->GetMaster();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PSEUDO:
/*N*/ 			{
/*N*/ 				ASSERT(pNumRule, "Wo ist die NumRule");
/*N*/ 				const SfxPoolItem* pItem;
/*N*/ 				switch( rSet.GetItemState( SID_ATTR_NUMBERING_RULE, FALSE, &pItem ))
/*N*/ 				{
/*N*/ 				case SFX_ITEM_SET:
/*N*/ 				{
/*N*/ 					SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
/*N*/ 					pSetRule->UnLinkGraphics();
/*N*/ 					//SwNumRule aSetRule(rDoc.GetUniqueNumRuleName());
/*N*/ 					SwNumRule aSetRule(pNumRule->GetName());
/*N*/ 					aSetRule.SetSvxRule(*pSetRule, &rDoc);
/*N*/ 					rDoc.ChgNumRuleFmts( aSetRule );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 				case SFX_ITEM_DONTCARE:
/*N*/ 					// NumRule auf default Werte
/*N*/ 					// was sind die default Werte?
/*N*/ 					{
/*?*/ 						SwNumRule aRule( pNumRule->GetName() );
/*?*/ 						rDoc.ChgNumRuleFmts( aRule );
/*?*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		default:
/*N*/ 			ASSERT(!this, "unbekannte Style-Familie");
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFmt && rSet.Count())
/*N*/ 	{
/*N*/ 		SfxItemIter aIter( rSet );
/*N*/ 		const SfxPoolItem* pItem = aIter.GetCurItem();
/*N*/ 		while( TRUE )
/*N*/ 		{
/*N*/ 			if( IsInvalidItem( pItem ) )			// Clearen
/*?*/ 				pFmt->ResetAttr( rSet.GetWhichByPos(aIter.GetCurPos()));
/*N*/ 
/*N*/ 			if( aIter.IsAtEnd() )
/*N*/ 				break;
/*N*/ 			pItem = aIter.NextItem();
/*N*/ 		}
/*N*/ 		SfxItemSet aSet(rSet);
/*N*/ 		aSet.ClearInvalidItems();
/*N*/ 
/*N*/ 		aCoreSet.ClearItem();
/*N*/ 
/*N*/ 		if( pNewDsc )
/*N*/ 		{
/*N*/ 			::binfilter::ItemSetToPageDesc( aSet, *pNewDsc );
/*N*/ 			rDoc.ChgPageDesc( nPgDscPos, *pNewDsc );
/*N*/ 			pDesc = &rDoc.GetPageDesc( nPgDscPos );
/*N*/ 			delete pNewDsc;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFmt->SetAttr( aSet );		// alles gesetzten Putten
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aCoreSet.ClearItem();
/*N*/ 		if( pNewDsc ) 			// den muessen wir noch vernichten!!
/*?*/ 			delete pNewDsc;
/*N*/ 	}
/*N*/ }



/*--------------------------------------------------------------------
    Beschreibung:	Das Format ermitteln
 --------------------------------------------------------------------*/

/*N*/ BOOL SwDocStyleSheet::FillStyleSheet( FillStyleType eFType )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	USHORT nPoolId = USHRT_MAX;
/*N*/ 	SwFmt* pFmt = 0;
/*N*/ 
/*N*/ 	BOOL bCreate = FillPhysical == eFType;
/*N*/ 	BOOL bDeleteInfo = FALSE;
/*N*/ 	BOOL bFillOnlyInfo = FillAllInfo == eFType;
/*N*/ 	SvPtrarr aDelArr;
/*N*/ 
/*N*/ 	switch(nFamily)
/*N*/ 	{
/*N*/ 	case SFX_STYLE_FAMILY_CHAR:
/*N*/ 		pCharFmt = lcl_FindCharFmt(rDoc, aName, this, bCreate );
/*N*/ 		bPhysical = 0 != pCharFmt;
/*N*/ 		if( bFillOnlyInfo && !bPhysical )
/*N*/ 		{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	bDeleteInfo = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		pFmt = pCharFmt;
/*N*/ 		if( !bCreate && !pFmt )
/*N*/ 		{
/*N*/ 			if( aName == *SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
/*N*/ 											RES_POOLCOLL_TEXT_BEGIN ] )
/*N*/ 				nPoolId = 0;
/*N*/ 			else
/*N*/ 				nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_CHRFMT );
/*N*/ 		}
/*N*/ 
/*N*/ 		bRet = 0 != pCharFmt || USHRT_MAX != nPoolId;
/*N*/ 
/*N*/ 		if( bDeleteInfo )
/*?*/ 			pCharFmt = 0;
/*N*/ 		break;
/*N*/ 
/*N*/ 	case SFX_STYLE_FAMILY_PARA:
/*N*/ 		{
/*N*/ 			pColl = lcl_FindParaFmt(rDoc, aName, this, bCreate);
/*N*/ 			bPhysical = 0 != pColl;
/*N*/ 			if( bFillOnlyInfo && !bPhysical )
/*N*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	bDeleteInfo = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/ 			pFmt = pColl;
/*N*/ 			if( pColl )
/*N*/ 				PresetFollow( pColl->GetNextTxtFmtColl().GetName() );
/*N*/ 			else if( !bCreate )
/*N*/ 				nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_TXTCOLL );
/*N*/ 
/*N*/ 			bRet = 0 != pColl || USHRT_MAX != nPoolId;
/*N*/ 
/*N*/ 			if( bDeleteInfo )
/*N*/ 				pColl = 0;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case SFX_STYLE_FAMILY_FRAME:
/*N*/ 		pFrmFmt = lcl_FindFrmFmt(rDoc,  aName, this, bCreate);
/*N*/ 		bPhysical = 0 != pFrmFmt;
/*N*/ 		if( bFillOnlyInfo && bPhysical )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bDeleteInfo = TRUE;
/*N*/ 		}
/*N*/ 		pFmt = pFrmFmt;
/*N*/ 		if( !bCreate && !pFmt )
/*N*/ 			nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_FRMFMT );
/*N*/ 
/*N*/ 		bRet = 0 != pFrmFmt || USHRT_MAX != nPoolId;
/*N*/ 
/*N*/ 		if( bDeleteInfo )
/*N*/ 			pFrmFmt = 0;
/*N*/ 		break;
/*N*/ 
/*N*/ 	case SFX_STYLE_FAMILY_PAGE:
/*N*/ 		pDesc = lcl_FindPageDesc(rDoc, aName, this, bCreate);
/*N*/ 		bPhysical = 0 != pDesc;
/*N*/ 		if( bFillOnlyInfo && !pDesc )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bDeleteInfo = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pDesc )
/*N*/ 		{
/*N*/ 			nPoolId = pDesc->GetPoolFmtId();
/*N*/ 			nHelpId = pDesc->GetPoolHelpId();
/*N*/ 			if( pDesc->GetPoolHlpFileId() != UCHAR_MAX )
/*?*/ 				aHelpFile = *rDoc.GetDocPattern( pDesc->GetPoolHlpFileId() );
/*N*/ 			else
/*N*/ 				aHelpFile.Erase();
/*N*/ 		}
/*N*/ 		else if( !bCreate )
/*N*/ 			nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_PAGEDESC );
/*N*/ 		SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );
/*N*/ 
/*N*/ 		bRet = 0 != pDesc || USHRT_MAX != nPoolId;
/*N*/ 		if( bDeleteInfo )
/*N*/ 			pDesc = 0;
/*N*/ 		break;
/*N*/ 
/*N*/ 	case SFX_STYLE_FAMILY_PSEUDO:
/*N*/ 		pNumRule = lcl_FindNumRule(rDoc, aName, this, bCreate);
/*N*/ 		bPhysical = 0 != pNumRule;
/*N*/ 		if( bFillOnlyInfo && !pNumRule )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bDeleteInfo = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pNumRule )
/*N*/ 		{
/*N*/ 			nPoolId = pNumRule->GetPoolFmtId();
/*N*/ 			nHelpId = pNumRule->GetPoolHelpId();
/*N*/ 			if( pNumRule->GetPoolHlpFileId() != UCHAR_MAX )
/*N*/ 				aHelpFile = *rDoc.GetDocPattern( pNumRule->GetPoolHlpFileId() );
/*N*/ 			else
/*N*/ 				aHelpFile.Erase();
/*N*/ 		}
/*N*/ 		else if( !bCreate )
/*N*/ 			nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_NUMRULE );
/*N*/ 		SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );
/*N*/ 
/*N*/ 		bRet = 0 != pNumRule || USHRT_MAX != nPoolId;
/*N*/ 
/*N*/ 		if( bDeleteInfo )
/*N*/ 			pNumRule = 0;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( SFX_STYLE_FAMILY_CHAR == nFamily ||
/*N*/ 		SFX_STYLE_FAMILY_PARA == nFamily ||
/*N*/ 		SFX_STYLE_FAMILY_FRAME == nFamily )
/*N*/ 	{
/*N*/ 		if( pFmt )
/*N*/ 			nPoolId = pFmt->GetPoolFmtId();
/*N*/ 
/*N*/ 		USHORT nMask = 0;
/*N*/ 		if( pFmt == rDoc.GetDfltCharFmt() )
/*N*/ 			nMask |= SFXSTYLEBIT_READONLY;
/*N*/ 		else if( USER_FMT & nPoolId )
/*N*/ 			nMask |= SFXSTYLEBIT_USERDEF;
/*N*/ 
/*N*/ 		switch ( COLL_GET_RANGE_BITS & nPoolId )
/*N*/ 		{
/*N*/ 		case COLL_TEXT_BITS:	 nMask |= SWSTYLEBIT_TEXT;	  break;
/*N*/ 		case COLL_DOC_BITS :	 nMask |= SWSTYLEBIT_CHAPTER; break;
/*N*/ 		case COLL_LISTS_BITS:	 nMask |= SWSTYLEBIT_LIST;	  break;
/*N*/ 		case COLL_REGISTER_BITS: nMask |= SWSTYLEBIT_IDX;	  break;
/*N*/ 		case COLL_EXTRA_BITS:	 nMask |= SWSTYLEBIT_EXTRA;	  break;
/*N*/ 		case COLL_HTML_BITS:	 nMask |= SWSTYLEBIT_HTML;	  break;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pFmt )
/*N*/ 		{
/*N*/ 			ASSERT( bPhysical, "Format nicht gefunden" );
/*N*/ 
/*N*/ 			nHelpId = pFmt->GetPoolHelpId();
/*N*/ 			if( pFmt->GetPoolHlpFileId() != UCHAR_MAX )
/*?*/ 				aHelpFile = *rDoc.GetDocPattern( pFmt->GetPoolHlpFileId() );
/*N*/ 			else
/*N*/ 				aHelpFile.Erase();
/*N*/ 
/*N*/ 			if( RES_CONDTXTFMTCOLL == pFmt->Which() )
/*N*/ 				nMask |= SWSTYLEBIT_CONDCOLL;
/*N*/ 		}
/*N*/ 
/*N*/ 		SetMask( nMask );
/*N*/ 	}
/*N*/ 	if( bDeleteInfo && bFillOnlyInfo )
/*?*/ {	DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 	::lcl_DeleteInfoStyles( nFamily, aDelArr, rDoc );
/*N*/ 	return bRet;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Neues Format in der Core anlegen
 --------------------------------------------------------------------*/


/*N*/ void   SwDocStyleSheet::Create()
/*N*/ {
/*N*/ 	switch(nFamily)
/*N*/ 	{
/*N*/ 		case SFX_STYLE_FAMILY_CHAR :
/*N*/ 			pCharFmt = lcl_FindCharFmt( rDoc, aName );
/*N*/ 			if( !pCharFmt )
/*N*/ 				pCharFmt = rDoc.MakeCharFmt(aName,
/*N*/ 											rDoc.GetDfltCharFmt());
/*N*/ 			pCharFmt->SetAuto( FALSE );
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PARA :
/*N*/ 			pColl = lcl_FindParaFmt( rDoc, aName );
/*N*/ 			if( !pColl )
/*N*/ 			{
/*N*/ 				SwTxtFmtColl *pPar = (*rDoc.GetTxtFmtColls())[0];
/*N*/ 				if( nMask & SWSTYLEBIT_CONDCOLL )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pColl = rDoc.MakeCondTxtFmtColl( aName, pPar );
/*N*/ 				else
/*N*/ 					pColl = rDoc.MakeTxtFmtColl( aName, pPar );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_FRAME:
/*N*/ 			pFrmFmt = lcl_FindFrmFmt( rDoc, aName );
/*N*/ 			if( !pFrmFmt )
/*N*/ 				pFrmFmt = rDoc.MakeFrmFmt(aName, rDoc.GetDfltFrmFmt());
/*N*/ 			pFrmFmt->SetAuto( FALSE );
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PAGE :
/*N*/ 			pDesc = lcl_FindPageDesc( rDoc, aName );
/*N*/ 			if( !pDesc )
/*N*/ 			{
/*N*/ 				USHORT nId = rDoc.MakePageDesc(aName);
/*N*/ 				pDesc = &rDoc.GetPageDesc(nId);
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PSEUDO:
/*?*/ 			pNumRule = lcl_FindNumRule( rDoc, aName );
/*?*/ 			if( !pNumRule )
/*?*/ 			{
/*?*/ 				//JP 05.02.99: temp Namen erzeugen, damit kein ASSERT kommt
/*?*/ 				String sTmpNm( aName );
/*?*/ 				if( !aName.Len() )
/*?*/ 					sTmpNm = rDoc.GetUniqueNumRuleName();
/*?*/ 
/*?*/ 				SwNumRule* pRule = rDoc.GetNumRuleTbl()[
/*?*/ 										rDoc.MakeNumRule( sTmpNm ) ];
/*?*/ 				pRule->SetAutoRule( FALSE );
/*?*/ 				if( !aName.Len() )
/*?*/ 					pRule->SetName( aName );
/*?*/ 				pNumRule = pRule;
/*?*/ 			}
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	bPhysical = TRUE;
/*N*/ 	aCoreSet.ClearItem();
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Konkrete Formate rausholen
 --------------------------------------------------------------------*/



/*N*/ SwCharFmt* SwDocStyleSheet::GetCharFmt()
/*N*/ {
/*N*/ 	if(!bPhysical)
/*N*/ 		FillStyleSheet( FillPhysical );
/*N*/ 	return pCharFmt;
/*N*/ }


/*N*/ SwTxtFmtColl* SwDocStyleSheet::GetCollection()
/*N*/ {
/*N*/ 	if(!bPhysical)
/*N*/ 		FillStyleSheet( FillPhysical );
/*N*/ 	return pColl;
/*N*/ }


/*N*/ const SwPageDesc* SwDocStyleSheet::GetPageDesc()
/*N*/ {
/*N*/ 	if(!bPhysical)
/*N*/ 		FillStyleSheet( FillPhysical );
/*N*/ 	return pDesc;
/*N*/ }

/*N*/ const SwNumRule * SwDocStyleSheet::GetNumRule()
/*N*/ {
/*N*/ 	if(!bPhysical)
/*N*/ 		FillStyleSheet( FillPhysical );
/*N*/ 	return pNumRule;
/*N*/ }

/*N*/ void SwDocStyleSheet::SetNumRule(const SwNumRule& rRule)
/*N*/ {
/*N*/ 	DBG_ASSERT(pNumRule, "Wo ist die NumRule");
/*N*/ 	rDoc.ChgNumRuleFmts( rRule );
/*N*/ }

// Namen UND Familie aus String re-generieren
// First() und Next() (s.u.) fuegen einen Kennbuchstaben an Pos.1 ein

/*N*/ void SwDocStyleSheet::PresetNameAndFamily(const String& rName)
/*N*/ {
/*N*/ 	switch( rName.GetChar(0) )
/*N*/ 	{
/*N*/ 		case cPARA:		nFamily = SFX_STYLE_FAMILY_PARA; break;
/*N*/ 		case cFRAME:	nFamily = SFX_STYLE_FAMILY_FRAME; break;
/*N*/ 		case cPAGE:		nFamily = SFX_STYLE_FAMILY_PAGE; break;
/*N*/ 		case cNUMRULE:	nFamily = SFX_STYLE_FAMILY_PSEUDO; break;
/*N*/ 		default:   		nFamily = SFX_STYLE_FAMILY_CHAR; break;
/*N*/ 	}
/*N*/ 	aName = rName;
/*N*/ 	aName.Erase( 0, 1 );
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Ist das Format physikalisch schon vorhanden
 --------------------------------------------------------------------*/


/*N*/ void SwDocStyleSheet::SetPhysical(BOOL bPhys)
/*N*/ {
/*N*/ 	bPhysical = bPhys;
/*N*/ 
/*N*/ 	if(!bPhys)
/*N*/ 	{
/*N*/ 		pCharFmt = 0;
/*N*/ 		pColl 	 = 0;
/*N*/ 		pFrmFmt  = 0;
/*N*/ 		pDesc 	 = 0;
/*N*/ 	}
/*N*/ }

/*N*/ SwFrmFmt* SwDocStyleSheet::GetFrmFmt()
/*N*/ {
/*N*/ 	if(!bPhysical)
/*N*/ 		FillStyleSheet( FillPhysical );
/*N*/ 	return pFrmFmt;
/*N*/ }








/*  */

/*--------------------------------------------------------------------
    Beschreibung:	Methoden fuer den DocStyleSheetPool
 --------------------------------------------------------------------*/

/*N*/ SwDocStyleSheetPool::SwDocStyleSheetPool( SwDoc& rDocument, BOOL bOrg )
/*N*/ 	: SfxStyleSheetBasePool( rDocument.GetAttrPool() ),
/*N*/ 	aStyleSheet( rDocument, aEmptyStr, *this, SFX_STYLE_FAMILY_CHAR, 0 ),
/*N*/ 	rDoc( rDocument )
/*N*/ {
/*N*/ 	bOrganizer = bOrg;
/*N*/ }

/*N*/  SwDocStyleSheetPool::~SwDocStyleSheetPool()
/*N*/ {
/*N*/ }

/*N*/ SfxStyleSheetBase&	 SwDocStyleSheetPool::Make(
/*N*/ 		const String& 	rName,
/*N*/ 		SfxStyleFamily	eFam,
/*N*/ 		USHORT 			nMask,
/*N*/ 		USHORT			nPos )
/*N*/ {
/*N*/ 	aStyleSheet.PresetName(rName);
/*N*/ 	aStyleSheet.PresetParent(aEmptyStr);
/*N*/ 	aStyleSheet.PresetFollow(aEmptyStr);
/*N*/ 	aStyleSheet.SetMask(nMask) ;
/*N*/ 	aStyleSheet.SetFamily(eFam);
/*N*/ 	aStyleSheet.SetPhysical(TRUE);
/*N*/ 	aStyleSheet.Create();
/*N*/ 
/*N*/ 	return aStyleSheet;
/*N*/ }






/*N*/ SfxStyleSheetIterator*  SwDocStyleSheetPool::CreateIterator(
/*N*/ 						SfxStyleFamily eFam, USHORT nMask )
/*N*/ {
/*N*/ 	return new SwStyleSheetIterator( this, eFam, nMask );
/*N*/ }




/*N*/ BOOL  SwDocStyleSheetPool::SetParent( SfxStyleFamily eFam,
/*N*/ 								const String &rStyle, const String &rParent )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

/*N*/ SfxStyleSheetBase* SwDocStyleSheetPool::Find( const String& rName,
/*N*/ 											SfxStyleFamily eFam, USHORT n )
/*N*/ {
/*N*/ 	USHORT nSMask = n;
/*N*/ 	if( SFX_STYLE_FAMILY_PARA == eFam && rDoc.IsHTMLMode() )
/*N*/ 	{
/*?*/ 		// dann sind nur HTML-Vorlagen von Interesse
/*?*/ 		if( USHRT_MAX == nSMask )
/*?*/ 			nSMask = SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED;
/*?*/ 		else
/*?*/ 			nSMask &= SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF |
/*?*/ 								SWSTYLEBIT_CONDCOLL | SWSTYLEBIT_HTML;
/*?*/ 		if( !nSMask )
/*?*/ 			nSMask = SWSTYLEBIT_HTML;
/*N*/ 	}

/*N*/ 	const BOOL bSearchUsed = ( n != SFXSTYLEBIT_ALL &&
/*N*/ 							 n & SFXSTYLEBIT_USED ) ? TRUE : FALSE;
/*N*/ 	const SwModify* pMod = 0;
/*N*/ 
/*N*/ 	aStyleSheet.SetPhysical( FALSE );
/*N*/ 	aStyleSheet.PresetName( rName );
/*N*/ 	aStyleSheet.SetFamily( eFam );
/*N*/ 	BOOL bFnd = aStyleSheet.FillStyleSheet( SwDocStyleSheet::FillOnlyName );
/*N*/ 
/*N*/ 	if( aStyleSheet.IsPhysical() )
/*N*/ 	{
/*N*/ 		switch( eFam )
/*N*/ 		{
/*N*/ 		case SFX_STYLE_FAMILY_CHAR:
/*N*/ 			pMod = aStyleSheet.GetCharFmt();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PARA:
/*N*/ 			pMod = aStyleSheet.GetCollection();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_FRAME:
/*N*/ 			pMod = aStyleSheet.GetFrmFmt();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PAGE:
/*N*/ 			pMod = aStyleSheet.GetPageDesc();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case SFX_STYLE_FAMILY_PSEUDO:
/*N*/ 			{
/*N*/ 				const SwNumRule* pRule = aStyleSheet.GetNumRule();
/*N*/ 				if( pRule &&
/*N*/ 					!(bSearchUsed && (bOrganizer || rDoc.IsUsed(*pRule)) ) &&
/*N*/ 					(( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
/*N*/ 							? !(pRule->GetPoolFmtId() & USER_FMT)
/*N*/ 								// benutzte gesucht und keine gefunden
/*N*/ 							: bSearchUsed ))
/*N*/ 					bFnd = FALSE;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		default:
/*N*/ 			ASSERT(!this, "unbekannte Style-Familie");
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// dann noch die Maske auswerten:
/*N*/ 	if( pMod && !(bSearchUsed && (bOrganizer || rDoc.IsUsed(*pMod)) ) )
/*N*/ 	{
/*N*/ 		const USHORT nId = SFX_STYLE_FAMILY_PAGE == eFam
/*N*/ 						? ((SwPageDesc*)pMod)->GetPoolFmtId()
/*N*/ 						: ((SwFmt*)pMod)->GetPoolFmtId();
/*N*/ 
/*N*/ 		if( ( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
/*N*/ 			? !(nId & USER_FMT)
/*N*/ 				// benutzte gesucht und keine gefunden
/*N*/ 			: bSearchUsed )
/*N*/ 			bFnd = FALSE;
/*N*/ 	}
/*N*/ 	return bFnd ? &aStyleSheet : 0;
/*N*/ }

/*  */

/*N*/ SwStyleSheetIterator::SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
/*N*/ 								SfxStyleFamily eFam, USHORT n )
/*N*/ 	: SfxStyleSheetIterator( pBase, eFam, n ),
/*N*/ 	aIterSheet( pBase->GetDoc(), aEmptyStr, *pBase, SFX_STYLE_FAMILY_CHAR, 0 ),
/*N*/ 	aStyleSheet( pBase->GetDoc(), aEmptyStr, *pBase, SFX_STYLE_FAMILY_CHAR, 0 )
/*N*/ {
/*N*/ 	bFirstCalled = FALSE;
/*N*/ 	nLastPos = 0;
/*N*/ 	StartListening( *pBase );
/*N*/ }

/*N*/  SwStyleSheetIterator::~SwStyleSheetIterator()
/*N*/ {
/*N*/ 	EndListening( aIterSheet.GetPool() );
/*N*/ }



/*N*/ SfxStyleSheetBase*  SwStyleSheetIterator::First()
/*N*/ {
/*N*/ 	// Alte Liste loeschen
/*N*/ 	bFirstCalled = TRUE;
/*N*/ 	nLastPos = 0;
/*N*/ 	aLst.Erase();
/*N*/ 
/*N*/ 	// aktuellen loeschen
/*N*/ 	aIterSheet.Reset();
/*N*/ 
/*N*/ 	SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
/*N*/ 	const USHORT nSrchMask = nMask;
/*N*/ 	const BOOL bSearchUsed = SearchUsed();
/*N*/ 
/*N*/ 	const BOOL bOrganizer = ((SwDocStyleSheetPool*)pBasePool)->IsOrganizerMode();
/*N*/ 
/*N*/ 	if( nSearchFamily == SFX_STYLE_FAMILY_CHAR
/*N*/ 	 || nSearchFamily == SFX_STYLE_FAMILY_ALL )
/*N*/ 	{
/*?*/ 		const USHORT nArrLen = rDoc.GetCharFmts()->Count();
/*?*/ 		for( USHORT i = 0; i < nArrLen; i++ )
/*?*/ 		{
/*?*/ 			SwCharFmt* pFmt = (*rDoc.GetCharFmts())[ i ];
/*?*/ 			if( pFmt->IsDefault() && pFmt != rDoc.GetDfltCharFmt() )
/*?*/ 				continue;
/*?*/ 
/*?*/ 			const BOOL	bUsed = bSearchUsed && (bOrganizer || rDoc.IsUsed(*pFmt));
/*?*/ 			if( !bUsed )
/*?*/ 			{
/*?*/ 				// Standard ist keine Benutzervorlage #46181#
/*?*/ 				const USHORT nId = rDoc.GetDfltCharFmt() == pFmt ?
/*?*/ 						RES_POOLCHR_INET_NORMAL :
/*?*/ 								pFmt->GetPoolFmtId();
/*?*/ 				if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
/*?*/ 					? !(nId & USER_FMT)
/*?*/ 						// benutzte gesucht und keine gefunden
/*?*/ 					: bSearchUsed )
/*?*/ 				continue;
/*?*/ 
/*?*/ 				if( rDoc.IsHTMLMode() && !(nId & USER_FMT) &&
/*?*/ 					!( RES_POOLCHR_HTML_BEGIN <= nId &&
/*?*/ 						  nId < RES_POOLCHR_HTML_END ) &&
/*?*/ 					RES_POOLCHR_INET_NORMAL != nId &&
/*?*/ 					RES_POOLCHR_INET_VISIT != nId &&
/*?*/ 					RES_POOLCHR_FOOTNOTE  != nId &&
/*?*/ 					RES_POOLCHR_ENDNOTE != nId )
/*?*/ 					continue;
/*?*/ 			}
/*?*/ 
/*?*/ 			aLst.Append( cCHAR, pFmt == rDoc.GetDfltCharFmt()
/*?*/ 						? (const String&) *SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
/*?*/ 												RES_POOLCOLL_TEXT_BEGIN ]
/*?*/ 						: pFmt->GetName() );
/*?*/ 		}
/*?*/ 
/*?*/ 		// PoolFormate
/*?*/ 		//
/*?*/ 		if( nSrchMask == SFXSTYLEBIT_ALL )
/*?*/ 		{
/*?*/ 			if( !rDoc.IsHTMLMode() )
/*?*/ 				AppendStyleList(SwStyleNameMapper::GetChrFmtUINameArray(),
/*?*/ 								bSearchUsed, GET_POOLID_CHRFMT, cCHAR);
/*?*/ 			else
/*?*/ 			{
/*?*/ 				aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
/*?*/ 						RES_POOLCHR_INET_NORMAL - RES_POOLCHR_BEGIN ] );
/*?*/ 				aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
/*?*/ 						RES_POOLCHR_INET_VISIT - RES_POOLCHR_BEGIN ] );
/*?*/ 				aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
/*?*/ 						RES_POOLCHR_ENDNOTE - RES_POOLCHR_BEGIN ] );
/*?*/ 				aLst.Append( cCHAR, *SwStyleNameMapper::GetChrFmtUINameArray()[
/*?*/ 						RES_POOLCHR_FOOTNOTE - RES_POOLCHR_BEGIN ] );
/*?*/ 			}
/*?*/ 			AppendStyleList(SwStyleNameMapper::GetHTMLChrFmtUINameArray(),
/*?*/ 								bSearchUsed, GET_POOLID_CHRFMT, cCHAR);
/*N*/ 		}
/*N*/ 	}

/*N*/ 	if( nSearchFamily == SFX_STYLE_FAMILY_PARA ||
/*N*/ 		nSearchFamily == SFX_STYLE_FAMILY_ALL )
/*N*/ 	{
/*N*/ 		USHORT nSMask = nSrchMask;
/*N*/ 		if( rDoc.IsHTMLMode() )
/*N*/ 		{
/*?*/ 			// dann sind nur HTML-Vorlagen von Interesse
/*?*/ 			if( USHRT_MAX == nSMask )
/*?*/ 				nSMask = SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF |
/*?*/ 							SFXSTYLEBIT_USED;
/*?*/ 			else
/*?*/ 				nSMask &= SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF |
/*?*/ 								SWSTYLEBIT_CONDCOLL | SWSTYLEBIT_HTML;
/*?*/ 			if( !nSMask )
/*?*/ 				nSMask = SWSTYLEBIT_HTML;
/*N*/ 		}
/*N*/ 
/*N*/ 		const USHORT nArrLen = rDoc.GetTxtFmtColls()->Count();
/*N*/ 		for( USHORT i = 0; i < nArrLen; i++ )
/*N*/ 		{
/*N*/ 			SwTxtFmtColl* pColl = (*rDoc.GetTxtFmtColls())[ i ];
/*N*/ 
/*N*/ 			if(pColl->IsDefault())
/*N*/ 				continue;
/*N*/ 
/*N*/ 			const BOOL bUsed = bOrganizer || rDoc.IsUsed(*pColl);
/*N*/ 			if( !(bSearchUsed && bUsed ))
/*N*/ 			{
/*N*/ 				const USHORT nId = pColl->GetPoolFmtId();
/*N*/ 				switch ( (nSMask & ~SFXSTYLEBIT_USED) )
/*N*/ 				{
/*N*/ 				case SFXSTYLEBIT_USERDEF:
/*N*/ 					if(!IsPoolUserFmt(nId)) continue;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_TEXT:
/*?*/ 					if((nId & COLL_GET_RANGE_BITS) != COLL_TEXT_BITS) continue;
/*?*/ 					break;
/*N*/ 				case SWSTYLEBIT_CHAPTER:
/*?*/ 					if((nId  & COLL_GET_RANGE_BITS) != COLL_DOC_BITS) continue;
/*?*/ 					break;
/*N*/ 				case SWSTYLEBIT_LIST:
/*?*/ 					if((nId  & COLL_GET_RANGE_BITS) != COLL_LISTS_BITS) continue;
/*?*/ 					break;
/*N*/ 				case SWSTYLEBIT_IDX:
/*N*/ 					if((nId  & COLL_GET_RANGE_BITS) != COLL_REGISTER_BITS) continue;
/*N*/ 					break;
/*N*/ 				case SWSTYLEBIT_EXTRA:
/*N*/ 					if((nId  & COLL_GET_RANGE_BITS) != COLL_EXTRA_BITS) continue;
/*N*/ 					break;
/*N*/ 
/*N*/ 				case SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF:
/*?*/ 					if(IsPoolUserFmt(nId))
/*?*/ 						break;
/*N*/ 					// ansonten weiter
/*N*/ 				case SWSTYLEBIT_HTML:
/*?*/ 					if( (nId  & COLL_GET_RANGE_BITS) != COLL_HTML_BITS)
/*?*/ 					{
/*?*/ 						// einige wollen wir aber auch in dieser Section sehen
/*?*/ 						BOOL bWeiter = TRUE;
/*?*/ 						switch( nId )
/*?*/ 						{
/*?*/ 						case RES_POOLCOLL_SENDADRESS:	//	--> ADDRESS
/*?*/ 						case RES_POOLCOLL_TABLE_HDLN:	//	--> TH
/*?*/ 						case RES_POOLCOLL_TABLE:		//	--> TD
/*?*/ 						case RES_POOLCOLL_TEXT:			// 	--> P
/*?*/ 						case RES_POOLCOLL_HEADLINE_BASE://	--> H
/*?*/ 						case RES_POOLCOLL_HEADLINE1:	//	--> H1
/*?*/ 						case RES_POOLCOLL_HEADLINE2:	//	--> H2
/*?*/ 						case RES_POOLCOLL_HEADLINE3:	//	--> H3
/*?*/ 						case RES_POOLCOLL_HEADLINE4:	//	--> H4
/*?*/ 						case RES_POOLCOLL_HEADLINE5:	//	--> H5
/*?*/ 						case RES_POOLCOLL_HEADLINE6:	//	--> H6
/*?*/ 						case RES_POOLCOLL_STANDARD:		//	--> P
/*?*/ 						case RES_POOLCOLL_FOOTNOTE:
/*?*/ 						case RES_POOLCOLL_ENDNOTE:
/*?*/ 							bWeiter = FALSE;
/*?*/ 							break;
/*?*/ 						}
/*?*/ 						if( bWeiter )
/*?*/ 							continue;
/*?*/ 					}
/*?*/ 					break;
/*?*/ 				case SWSTYLEBIT_CONDCOLL:
/*?*/ 					if( RES_CONDTXTFMTCOLL != pColl->Which() ) continue;
/*?*/ 					break;
/*N*/ 				default:
/*N*/ 					// benutzte gesucht und keine gefunden
/*N*/ 					if( bSearchUsed )
/*N*/ 						continue;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			aLst.Append( cPARA, pColl->GetName() );
/*N*/ 		}
/*N*/ 
/*N*/ 		const BOOL bAll = nSMask == SFXSTYLEBIT_ALL;
/*N*/ 		if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_TEXT )
/*N*/ 			AppendStyleList(SwStyleNameMapper::GetTextUINameArray(),
/*N*/ 							bSearchUsed, GET_POOLID_TXTCOLL, cPARA );
/*N*/ 		if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CHAPTER )
/*N*/ 			AppendStyleList(SwStyleNameMapper::GetDocUINameArray(),
/*N*/ 							bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
/*N*/ 		if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_LIST )
/*N*/ 			AppendStyleList(SwStyleNameMapper::GetListsUINameArray(),
/*N*/ 							bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
/*N*/ 		if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_IDX )
/*N*/ 			AppendStyleList(SwStyleNameMapper::GetRegisterUINameArray(),
/*N*/ 							bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
/*N*/ 		if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_EXTRA )
/*N*/ 			AppendStyleList(SwStyleNameMapper::GetExtraUINameArray(),
/*N*/ 							bSearchUsed, GET_POOLID_TXTCOLL, cPARA ) ;
/*N*/ 		if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CONDCOLL )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( !bSearchUsed ||
/*N*/ 		}
/*N*/ 		if ( bAll ||
/*N*/ 			(nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_HTML ||
/*N*/ 			(nSMask & ~SFXSTYLEBIT_USED) ==
/*N*/ 						(SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF) )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 AppendStyleList(SwStyleNameMapper::GetHTMLUINameArray(),
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nSearchFamily == SFX_STYLE_FAMILY_FRAME ||
/*N*/ 		nSearchFamily == SFX_STYLE_FAMILY_ALL )
/*?*/ 	{
/*?*/ 		const USHORT nArrLen = rDoc.GetFrmFmts()->Count();
/*?*/ 		for( USHORT i = 0; i < nArrLen; i++ )
/*?*/ 		{
/*?*/ 			SwFrmFmt* pFmt = (*rDoc.GetFrmFmts())[ i ];
/*?*/ 			if(pFmt->IsDefault() || pFmt->IsAuto())
/*?*/ 				continue;
/*?*/ 
/*?*/ 			const USHORT nId = pFmt->GetPoolFmtId();
/*?*/ 			BOOL bUsed = bSearchUsed && ( bOrganizer || rDoc.IsUsed(*pFmt));
/*?*/ 			if( !bUsed )
/*?*/ 			{
/*?*/ 				if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
/*?*/ 					? !(nId & USER_FMT)
/*?*/ 					// benutzte gesucht und keine gefunden
/*?*/ 					: bSearchUsed )
/*?*/ 					continue;
/*?*/ 			}
/*?*/ 
/*?*/ 			aLst.Append( cFRAME, pFmt->GetName() );
/*?*/ 		}
/*?*/ 
/*?*/ 		// PoolFormate
/*?*/ 		//
/*?*/ 		if ( nSrchMask == SFXSTYLEBIT_ALL )
/*?*/ 			AppendStyleList(SwStyleNameMapper::GetFrmFmtUINameArray(),
/*?*/ 									bSearchUsed, GET_POOLID_FRMFMT, cFRAME);
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nSearchFamily == SFX_STYLE_FAMILY_PAGE ||
/*N*/ 		nSearchFamily == SFX_STYLE_FAMILY_ALL )
/*N*/ 	{
/*N*/ 		const USHORT nCount = rDoc.GetPageDescCnt();
/*N*/ 		for(USHORT i = 0; i < nCount; ++i)
/*N*/ 		{
/*N*/ 			const SwPageDesc& rDesc = rDoc.GetPageDesc(i);
/*N*/ 			const USHORT nId = rDesc.GetPoolFmtId();
/*N*/ 			BOOL bUsed = bSearchUsed && ( bOrganizer || rDoc.IsUsed(rDesc));
/*N*/ 			if( !bUsed )
/*N*/ 			{
/*N*/ 				if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
/*N*/ 					? !(nId & USER_FMT)
/*N*/ 					// benutzte gesucht und keine gefunden
/*N*/ 					: bSearchUsed )
/*?*/ 					continue;
/*N*/ 			}
/*N*/ 
/*N*/ 			aLst.Append( cPAGE, rDesc.GetName() );
/*N*/ 		}
/*N*/ 		if ( nSrchMask == SFXSTYLEBIT_ALL )
/*N*/ 			AppendStyleList(SwStyleNameMapper::GetPageDescUINameArray(),
/*N*/ 							bSearchUsed, GET_POOLID_PAGEDESC, cPAGE);
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nSearchFamily == SFX_STYLE_FAMILY_PSEUDO ||
/*N*/ 		nSearchFamily == SFX_STYLE_FAMILY_ALL )
/*N*/ 	{
/*?*/ 		const SwNumRuleTbl& rNumTbl = rDoc.GetNumRuleTbl();
/*?*/ 		for(USHORT i = 0; i < rNumTbl.Count(); ++i)
/*?*/ 		{
/*?*/ 			const SwNumRule& rRule = *rNumTbl[ i ];
/*?*/ 			if( !rRule.IsAutoRule() )
/*?*/ 			{
/*?*/ 				BOOL bUsed = bSearchUsed && ( bOrganizer || rDoc.IsUsed(rRule) );
/*?*/ 				if( !bUsed )
/*?*/ 				{
/*?*/ 					if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
/*?*/ 						? !(rRule.GetPoolFmtId() & USER_FMT)
/*?*/ 						// benutzte gesucht und keine gefunden
/*?*/ 						: bSearchUsed )
/*?*/ 						continue;
/*?*/ 				}
/*?*/ 
/*?*/ 				aLst.Append( cNUMRULE, rRule.GetName() );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if ( nSrchMask == SFXSTYLEBIT_ALL )
/*?*/ 			AppendStyleList(SwStyleNameMapper::GetNumRuleUINameArray(),
/*?*/ 							bSearchUsed, GET_POOLID_NUMRULE, cNUMRULE);
/*N*/ 	}
/*N*/ 
/*N*/ 	if(aLst.Count() > 0)
/*N*/ 	{
/*N*/ 		nLastPos = USHRT_MAX;
/*N*/ 		return Next();
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ SfxStyleSheetBase*  SwStyleSheetIterator::Next()
/*N*/ {
/*N*/ 	nLastPos++;
/*N*/ 	if(aLst.Count() > 0 && nLastPos < aLst.Count())
/*N*/ 	{
/*N*/ 		aIterSheet.PresetNameAndFamily(*aLst[nLastPos]);
/*N*/ 		aIterSheet.SetPhysical( FALSE );
/*N*/ 		aIterSheet.SetMask( nMask );
/*N*/ 		if(aIterSheet.pSet)
/*N*/ 		{
/*?*/ 			aIterSheet.pSet->ClearItem(0);
/*?*/ 			aIterSheet.pSet= 0;
/*N*/ 		}
/*N*/ 		return &aIterSheet;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }


/*N*/ void SwStyleSheetIterator::AppendStyleList(const SvStringsDtor& rList,
/*N*/ 											BOOL 	bTestUsed,
/*N*/ 											USHORT nSection, char cType )
/*N*/ {
/*N*/ 	if( bTestUsed )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		for ( USHORT i=0; i < rList.Count(); ++i )
/*N*/ 			aLst.Append( cType, *rList[i] );
/*N*/ }

/*N*/ void  SwStyleSheetIterator::Notify( SfxBroadcaster&, const SfxHint& rHint )
/*N*/ {
/*N*/ 	// suchen und aus der Anzeige-Liste entfernen !!
/*N*/ 	if( rHint.ISA( SfxStyleSheetHint ) &&
/*N*/ 		SFX_STYLESHEET_ERASED == ((SfxStyleSheetHint&) rHint).GetHint() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SfxStyleSheetBase* pStyle = ((SfxStyleSheetHint&)rHint).GetStyleSheet();
/*N*/ 	}
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
