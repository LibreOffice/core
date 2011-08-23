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


#ifndef _SFXITEMITER_HXX //autogen
#include <bf_svtools/itemiter.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen
#include <bf_svx/svdmark.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _DCONTACT_HXX //autogen
#include <dcontact.hxx>
#endif

#include <ndgrf.hxx>
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _TXTFRM_HXX //autogen
#include <txtfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen
#include <rootfrm.hxx>
#endif
#ifndef _FLYFRMS_HXX //autogen
#include <flyfrms.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
namespace binfilter {

extern USHORT GetHtmlMode( const SwDocShell* );

/*-----------------17.02.98 08:35-------------------

--------------------------------------------------*/
/*N*/ USHORT SwDoc::GetFlyCount(FlyCntType eType ) const
/*N*/ {
/*N*/ 	const SwSpzFrmFmts& rFmts = *GetSpzFrmFmts();
/*N*/ 	USHORT nSize = rFmts.Count();
/*N*/ 	USHORT nCount = 0;
/*N*/ 	const SwNodeIndex* pIdx;
/*N*/ 	for ( USHORT i = 0; i < nSize; i++)
/*N*/ 	{
/*N*/ 		const SwFrmFmt* pFlyFmt = rFmts[ i ];
/*N*/ 		if( RES_FLYFRMFMT == pFlyFmt->Which()
/*N*/ 			&& 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
/*N*/ 			&& pIdx->GetNodes().IsDocNodes()
/*N*/ 			)
/*N*/ 		{
/*N*/ 			const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
/*N*/
/*N*/ 			switch( eType )
/*N*/ 			{
/*N*/ 			case FLYCNTTYPE_FRM:
/*N*/ 				if(!pNd->IsNoTxtNode())
/*N*/ 					nCount++;
/*N*/ 				break;
/*N*/
/*N*/ 			case FLYCNTTYPE_GRF:
/*N*/ 				if( pNd->IsGrfNode() )
/*N*/ 					nCount++;
/*N*/ 				break;
/*N*/
/*N*/ 			case FLYCNTTYPE_OLE:
/*N*/ 				if(pNd->IsOLENode())
/*N*/ 					nCount++;
/*N*/ 				break;
/*N*/
/*N*/ 			default:
/*N*/ 				nCount++;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nCount;
/*N*/ }

/*-----------------17.02.98 08:35-------------------

--------------------------------------------------*/
/*N*/ SwFrmFmt* SwDoc::GetFlyNum( USHORT nIdx, FlyCntType eType )
/*N*/ {
/*N*/ 	SwSpzFrmFmts& rFmts = *GetSpzFrmFmts();
/*N*/ 	SwFrmFmt* pRetFmt = 0;
/*N*/ 	USHORT nSize = rFmts.Count();
/*N*/ 	const SwNodeIndex* pIdx;
/*N*/ 	USHORT nCount = 0;
/*N*/ 	for( USHORT i = 0; !pRetFmt && i < nSize; ++i )
/*N*/ 	{
/*N*/ 		SwFrmFmt* pFlyFmt = rFmts[ i ];
/*N*/ 		if( RES_FLYFRMFMT == pFlyFmt->Which()
/*N*/ 			&& 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
/*N*/ 			&& pIdx->GetNodes().IsDocNodes()
/*N*/ 			)
/*N*/ 		{
/*N*/ 			const SwNode* pNd = GetNodes()[ pIdx->GetIndex() + 1 ];
/*N*/ 			switch( eType )
/*N*/ 			{
/*N*/ 			case FLYCNTTYPE_FRM:
/*N*/ 				if( !pNd->IsNoTxtNode() && nIdx == nCount++)
/*N*/ 					pRetFmt = pFlyFmt;
/*N*/ 				break;
/*N*/ 			case FLYCNTTYPE_GRF:
/*N*/ 				if(pNd->IsGrfNode() && nIdx == nCount++ )
/*N*/ 					pRetFmt = pFlyFmt;
/*N*/ 				break;
/*N*/ 			case FLYCNTTYPE_OLE:
/*N*/ 				if(pNd->IsOLENode() && nIdx == nCount++)
/*N*/ 					pRetFmt = pFlyFmt;
/*N*/ 				break;
/*N*/ 			default:
/*?*/ 				if(nIdx == nCount++)
/*?*/ 					pRetFmt = pFlyFmt;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRetFmt;
/*N*/ }

/*  */

/***********************************************************************
#*	Class	   	:  SwDoc
#*	Methode	   	:  SetFlyFrmAnchor
#*	Beschreibung:  Das Ankerattribut des FlyFrms aendert sich.
#*	Datum	   	:  MA 01. Feb. 94
#*	Update	   	:  JP 09.03.98
#***********************************************************************/

/*N*/ Point lcl_FindAnchorLayPos( SwDoc& rDoc, const SwFmtAnchor& rAnch,
/*N*/ 							const SwFrmFmt* pFlyFmt )
/*N*/ {
/*N*/ 	Point aRet;
/*N*/ 	if( rDoc.GetRootFrm() )
/*N*/ 		switch( rAnch.GetAnchorId() )
/*N*/ 		{
/*N*/ 		case FLY_IN_CNTNT:
/*N*/ 			if( pFlyFmt && rAnch.GetCntntAnchor() )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const SwFrm* pOld = ((SwFlyFrmFmt*)pFlyFmt)->GetFrm( &aRet, FALSE );
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case FLY_AT_CNTNT:
/*N*/ 		case FLY_AUTO_CNTNT: // LAYER_IMPL
/*N*/ 			if( rAnch.GetCntntAnchor() )
/*N*/ 			{
/*N*/ 				const SwPosition *pPos = rAnch.GetCntntAnchor();
/*N*/ 				const SwCntntNode* pNd = pPos->nNode.GetNode().GetCntntNode();
/*N*/ 				const SwFrm* pOld = pNd ? pNd->GetFrm( &aRet, 0, FALSE ) : 0;
/*N*/ 				if( pOld )
/*N*/ 					aRet = pOld->Frm().Pos();
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case FLY_AT_FLY: // LAYER_IMPL
/*?*/ 			if( rAnch.GetCntntAnchor() )
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const SwFlyFrmFmt* pFmt = (SwFlyFrmFmt*)rAnch.GetCntntAnchor()->
/*?*/ 			}
/*?*/ 			break;
/*?*/
/*N*/ 		case FLY_PAGE:
/*?*/ 			{
/*?*/ 				USHORT nPgNum = rAnch.GetPageNum();
/*?*/ 				const SwPageFrm *pPage = (SwPageFrm*)rDoc.GetRootFrm()->Lower();
/*?*/ 				for( USHORT i = 1; (i <= nPgNum) && pPage; ++i,
/*?*/ 									pPage = (const SwPageFrm*)pPage->GetNext() )
/*?*/ 					if( i == nPgNum )
/*?*/ 					{
/*?*/ 						aRet = pPage->Frm().Pos();
/*?*/ 						break;
/*?*/ 					}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		}
/*N*/ 	return aRet;
/*N*/ }

#define MAKEFRMS 0
#define IGNOREANCHOR 1
#define DONTMAKEFRMS 2

/*M*/ sal_Int8 SwDoc::SetFlyFrmAnchor( SwFrmFmt& rFmt, SfxItemSet& rSet, BOOL bNewFrms )
/*M*/ {
/*M*/ 	//Ankerwechsel sind fast immer in alle 'Richtungen' erlaubt.
/*M*/ 	//Ausnahme: Absatz- bzw. Zeichengebundene Rahmen duerfen wenn sie in
/*M*/ 	//Kopf-/Fusszeilen stehen nicht Seitengebunden werden.
/*M*/ 	const SwFmtAnchor &rOldAnch = rFmt.GetAnchor();
/*M*/ 	const RndStdIds nOld = rOldAnch.GetAnchorId();
/*M*/
/*M*/ 	SwFmtAnchor aNewAnch( (SwFmtAnchor&)rSet.Get( RES_ANCHOR ) );
/*M*/ 	RndStdIds nNew = aNewAnch.GetAnchorId();
/*M*/
/*M*/ 	// ist der neue ein gueltiger Anker?
/*M*/ 	if( !aNewAnch.GetCntntAnchor() && (FLY_AT_FLY == nNew ||
/*M*/ 		FLY_AT_CNTNT == nNew || FLY_IN_CNTNT == nNew ||
/*M*/ 		FLY_AUTO_CNTNT == nNew ))
/*M*/         return IGNOREANCHOR;
/*M*/
/*M*/ 	if( nOld == nNew )
/*M*/         return DONTMAKEFRMS;
/*M*/
/*M*/
/*M*/ 	Point aOldAnchorPos( ::binfilter::lcl_FindAnchorLayPos( *this, rOldAnch, &rFmt ));
/*M*/ 	Point aNewAnchorPos( ::binfilter::lcl_FindAnchorLayPos( *this, aNewAnch, 0 ));
/*M*/
/*M*/ 	//Die alten Frms vernichten. Dabei werden die Views implizit gehidet und
/*M*/ 	//doppeltes hiden waere so eine art Show!
/*M*/ 	rFmt.DelFrms();
/*M*/
/*M*/ 	if( FLY_IN_CNTNT == nOld )
/*M*/ 	{
/*M*/ 		//Bei InCntnt's wird es spannend: Das TxtAttribut muss vernichtet
/*M*/ 		//werden. Leider reisst dies neben den Frms auch noch das Format mit
/*M*/ 		//in sein Grab. Um dass zu unterbinden loesen wir vorher die
/*M*/ 		//Verbindung zwischen Attribut und Format.
/*M*/ 		const SwPosition *pPos = rOldAnch.GetCntntAnchor();
/*M*/ 		SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
/*M*/ 		ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
/*M*/ 		const xub_StrLen nIdx = pPos->nContent.GetIndex();
/*M*/ 		SwTxtAttr * pHnt = pTxtNode->GetTxtAttr( nIdx, RES_TXTATR_FLYCNT );
/*M*/ #ifdef DBG_UTIL
/*M*/ 		ASSERT( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
/*M*/ 					"Missing FlyInCnt-Hint." );
/*M*/ 		ASSERT( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == &rFmt,
/*M*/ 					"Wrong TxtFlyCnt-Hint." );
/*M*/ #endif
/*M*/ 		((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt();
/*M*/
/*M*/ 		//Die Verbindung ist geloest, jetzt muss noch das Attribut vernichtet
/*M*/ 		//werden.
/*M*/ 		pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
/*M*/ 	}
/*M*/
/*M*/ 	//Endlich kann das Attribut gesetzt werden. Es muss das erste Attribut
/*M*/ 	//sein; Undo depends on it!
/*M*/ 	rFmt.SetAttr( aNewAnch );
/*M*/
/*M*/ 	//Positionskorrekturen
/*M*/ 	const SfxPoolItem* pItem;
/*M*/ 	switch( nNew )
/*M*/ 	{
/*M*/ 	case FLY_IN_CNTNT:
/*M*/ 			//Wenn keine Positionsattribute hereinkommen, dann muss dafuer
/*M*/ 			//gesorgt werden, das keine unerlaubte automatische Ausrichtung
/*M*/ 			//bleibt.
/*M*/ 		{
/*M*/ 			const SwPosition *pPos = aNewAnch.GetCntntAnchor();
/*M*/ 			SwTxtNode *pNd = pPos->nNode.GetNode().GetTxtNode();
/*M*/ 			ASSERT( pNd, "Crsr steht nicht auf TxtNode." );
/*M*/
/*M*/ 			pNd->Insert( SwFmtFlyCnt( (SwFlyFrmFmt*)&rFmt ),
/*M*/ 									   pPos->nContent.GetIndex(), 0 );
/*M*/ 		}
/*M*/
/*M*/ 		if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, FALSE, &pItem ))
/*M*/ 		{
/*M*/ 			SwFmtVertOrient aOldV( rFmt.GetVertOrient() );
/*M*/ 			BOOL bSet = TRUE;
/*M*/ 			switch( aOldV.GetVertOrient() )
/*M*/ 			{
/*M*/ 			case VERT_LINE_TOP:   	aOldV.SetVertOrient( VERT_TOP );   break;
/*M*/ 			case VERT_LINE_CENTER:	aOldV.SetVertOrient( VERT_CENTER); break;
/*M*/ 			case VERT_LINE_BOTTOM:	aOldV.SetVertOrient( VERT_BOTTOM); break;
/*M*/ 			case VERT_NONE:			aOldV.SetVertOrient( VERT_CENTER); break;
/*M*/ 			default:
/*M*/ 				bSet = FALSE;
/*M*/ 			}
/*M*/ 			if( bSet )
/*M*/ 				rSet.Put( aOldV );
/*M*/ 		}
/*M*/ 		break;
/*M*/
/*M*/ 	case FLY_AT_CNTNT:
/*M*/ 	case FLY_AUTO_CNTNT: // LAYER_IMPL
/*M*/ 	case FLY_AT_FLY: // LAYER_IMPL
/*M*/ 	case FLY_PAGE:
/*M*/ 		{
/*M*/ 			//Wenn keine Positionsattribute hereinschneien korrigieren wir
/*M*/ 			//die Position so, dass die Dokumentkoordinaten des Flys erhalten
/*M*/ 			//bleiben.
/*M*/ 			//Chg: Wenn sich in den Positionsattributen lediglich die
/*M*/ 			//Ausrichtung veraendert (FRAME vs. PRTAREA), dann wird die
/*M*/ 			//Position ebenfalls korrigiert.
/*M*/ 			if( SFX_ITEM_SET != rSet.GetItemState( RES_HORI_ORIENT, FALSE, &pItem ))
/*M*/ 				pItem = 0;
/*M*/
/*M*/ 			SwFmtHoriOrient aOldH( rFmt.GetHoriOrient() );
/*M*/
/*M*/ 			if( HORI_NONE == aOldH.GetHoriOrient() && ( !pItem ||
/*M*/ 				aOldH.GetPos() == ((SwFmtHoriOrient*)pItem)->GetPos() ))
/*M*/ 			{
/*M*/ 				SwTwips nPos = FLY_IN_CNTNT == nOld ? 0 : aOldH.GetPos();
/*M*/ 				nPos += aOldAnchorPos.X() - aNewAnchorPos.X();
/*M*/
/*M*/ 				if( pItem )
/*M*/ 				{
/*M*/ 					SwFmtHoriOrient* pH = (SwFmtHoriOrient*)pItem;
/*M*/ 					aOldH.SetHoriOrient( pH->GetHoriOrient() );
/*M*/ 					aOldH.SetRelationOrient( pH->GetRelationOrient() );
/*M*/ 				}
/*M*/ 				aOldH.SetPos( nPos );
/*M*/ 				rSet.Put( aOldH );
/*M*/ 			}
/*M*/
/*M*/ 			if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, FALSE, &pItem ))
/*M*/ 				pItem = 0;
/*M*/ 			SwFmtVertOrient aOldV( rFmt.GetVertOrient() );
/*M*/
/*M*/ 			if( HORI_NONE == aOldV.GetVertOrient() && (!pItem ||
/*M*/ 				aOldV.GetPos() == ((SwFmtVertOrient*)pItem)->GetPos() ) )
/*M*/ 			{
/*M*/ 				SwTwips nPos = FLY_IN_CNTNT == nOld ? 0 : aOldV.GetPos();
/*M*/ 				nPos += aOldAnchorPos.Y() - aNewAnchorPos.Y();
/*M*/ 				if( pItem )
/*M*/ 				{
/*M*/ 					SwFmtVertOrient* pV = (SwFmtVertOrient*)pItem;
/*M*/ 					aOldV.SetVertOrient( pV->GetVertOrient() );
/*M*/ 					aOldV.SetRelationOrient( pV->GetRelationOrient() );
/*M*/ 				}
/*M*/ 				aOldV.SetPos( nPos );
/*M*/ 				rSet.Put( aOldV );
/*M*/ 			}
/*M*/ 		}
/*M*/ 		break;
/*M*/ 	}
/*M*/
/*M*/ 	if( bNewFrms )
/*M*/ 		rFmt.MakeFrms();
/*M*/
/*M*/     return MAKEFRMS;
/*M*/ }

/*N*/ BOOL SwDoc::SetFlyFrmAttr( SwFrmFmt& rFlyFmt, SfxItemSet& rSet )
/*N*/ {
/*N*/ 	if( !rSet.Count() )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	//Ist das Ankerattribut dabei? Falls ja ueberlassen wir die Verarbeitung
/*N*/ 	//desselben einer Spezialmethode. Sie Returnt TRUE wenn der Fly neu
/*N*/ 	//erzeugt werden muss (z.B. weil ein Wechsel des FlyTyps vorliegt).
/*N*/     sal_Int8 nMakeFrms = SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, FALSE )?
/*N*/                          SetFlyFrmAnchor( rFlyFmt, rSet, FALSE ) : DONTMAKEFRMS;
/*N*/
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	SfxItemIter aIter( rSet );
/*N*/ 	SfxItemSet aTmpSet( GetAttrPool(), aFrmFmtSetRange );
/*N*/ 	USHORT nWhich = aIter.GetCurItem()->Which();
/*N*/ 	do {
/*N*/ 		switch( nWhich )
/*N*/ 		{
/*N*/ 		case RES_FILL_ORDER:
/*N*/ 		case RES_BREAK:
/*N*/ 		case RES_PAGEDESC:
/*N*/ 		case RES_CNTNT:
/*N*/ 		case RES_FOOTER:
/*N*/ 			ASSERT( !this, ":-) Unbekanntes Attribut fuer Fly." );
/*N*/ 			// kein break;
/*N*/ 		case RES_CHAIN:
/*?*/ 			rSet.ClearItem( nWhich );
/*?*/ 			break;
/*N*/ 		case RES_ANCHOR:
/*N*/             if( DONTMAKEFRMS != nMakeFrms )
/*N*/ 				break;
/*N*/
/*N*/ 		default:
/*N*/ 			if( !IsInvalidItem( aIter.GetCurItem() ) && ( SFX_ITEM_SET !=
/*N*/ 				rFlyFmt.GetAttrSet().GetItemState( nWhich, TRUE, &pItem ) ||
/*N*/ 				*pItem != *aIter.GetCurItem() ))
/*N*/ 				aTmpSet.Put( *aIter.GetCurItem() );
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		if( aIter.IsAtEnd() )
/*N*/ 			break;
/*N*/
/*?*/ 	} while( 0 != ( nWhich = aIter.NextItem()->Which() ) );
/*N*/
/*N*/ 	if( aTmpSet.Count() )
/*?*/ 		rFlyFmt.SetAttr( aTmpSet );
/*N*/
/*N*/     if( MAKEFRMS == nMakeFrms )
/*N*/ 		rFlyFmt.MakeFrms();
/*N*/
/*N*/
/*N*/ 	SetModified();
/*N*/
/*N*/     return aTmpSet.Count() || MAKEFRMS == nMakeFrms;
/*N*/ }


/***************************************************************************
 *	Methode		:	BOOL SwDoc::SetFrmFmtToFly( SwFlyFrm&, SwFrmFmt& )
 *	Beschreibung:
 *	Erstellt	:	OK 14.04.94 15:40
 *	Aenderung	:	JP 23.04.98
 ***************************************************************************/

/*M*/ BOOL SwDoc::SetFrmFmtToFly( SwFrmFmt& rFmt, SwFrmFmt& rNewFmt,
/*M*/ 							SfxItemSet* pSet, BOOL bKeepOrient )
/*M*/ {
/*M*/ 	BOOL bChgAnchor = FALSE, bFrmSz = FALSE;
/*M*/
/*M*/ 	const SwFmtFrmSize aFrmSz( rFmt.GetFrmSize() );
/*M*/ 	const SwFmtVertOrient aVert( rFmt.GetVertOrient() );
/*M*/ 	const SwFmtHoriOrient aHori( rFmt.GetHoriOrient() );
/*M*/
/*M*/
/*M*/ 	//Erstmal die Spalten setzen, sonst gibts nix als Aerger mit dem
/*M*/ 	//Set/Reset/Abgleich usw.
/*M*/ 	const SfxPoolItem* pItem;
/*M*/ 	if( SFX_ITEM_SET != rNewFmt.GetAttrSet().GetItemState( RES_COL ))
/*M*/ 		rFmt.ResetAttr( RES_COL );
/*M*/
/*M*/ 	if( rFmt.DerivedFrom() != &rNewFmt )
/*M*/ 	{
/*M*/ 		rFmt.SetDerivedFrom( &rNewFmt );
/*M*/
/*M*/ 		// 1. wenn nicht automatisch -> ignorieren, sonst -> wech
/*M*/ 		// 2. wech damit, MB!
/*M*/ 		if( SFX_ITEM_SET == rNewFmt.GetAttrSet().GetItemState( RES_FRM_SIZE, FALSE ))
/*M*/ 		{
/*M*/ 			rFmt.ResetAttr( RES_FRM_SIZE );
/*M*/ 			bFrmSz = TRUE;
/*M*/ 		}
/*M*/
/*M*/ 		const SfxItemSet* pAsk = pSet;
/*M*/ 		if( !pAsk ) pAsk = &rNewFmt.GetAttrSet();
/*M*/ 		if( SFX_ITEM_SET == pAsk->GetItemState( RES_ANCHOR, FALSE, &pItem )
/*M*/ 			&& ((SwFmtAnchor*)pItem)->GetAnchorId() !=
/*M*/ 				rFmt.GetAnchor().GetAnchorId() )
/*M*/ 		{
/*M*/
/*M*/ 			if( pSet )
/*M*/                 bChgAnchor = MAKEFRMS == SetFlyFrmAnchor( rFmt, *pSet, FALSE );
/*M*/ 			else
/*M*/ 			{
/*M*/ 				//JP 23.04.98: muss den FlyFmt-Range haben, denn im SetFlyFrmAnchor
/*M*/ 				//				werden Attribute in diesen gesetzt!
/*M*/ 				SfxItemSet aFlySet( *rNewFmt.GetAttrSet().GetPool(),
/*M*/ 									rNewFmt.GetAttrSet().GetRanges() );
/*M*/ 				aFlySet.Put( *pItem );
/*M*/                 bChgAnchor = MAKEFRMS == SetFlyFrmAnchor( rFmt, aFlySet, FALSE);
/*M*/ 			}
/*M*/
/*M*/ 		}
/*M*/ 	}
/*M*/
/*M*/ 	//Hori und Vert nur dann resetten, wenn in der Vorlage eine
/*M*/ 	//automatische Ausrichtung eingestellt ist, anderfalls den alten Wert
/*M*/ 	//wieder hineinstopfen.
/*M*/ 	//JP 09.06.98: beim Update der RahmenVorlage sollte der Fly NICHT
/*M*/ 	//				seine Orientierng verlieren (diese wird nicht geupdatet!)
/*M*/     //OS: #96584# HORI_NONE and VERT_NONE are allowed now
/*M*/ 	if( !bKeepOrient )
/*M*/ 	{
/*M*/ 		const SwFmtVertOrient &rVert = rNewFmt.GetVertOrient();
/*M*/         rFmt.ResetAttr( RES_VERT_ORIENT );
/*M*/
/*M*/ 		const SwFmtHoriOrient &rHori = rNewFmt.GetHoriOrient();
/*M*/         rFmt.ResetAttr( RES_HORI_ORIENT );
/*M*/ 	}
/*M*/
/*M*/ 	rFmt.ResetAttr( RES_PRINT, RES_SURROUND );
/*M*/ 	rFmt.ResetAttr( RES_LR_SPACE, RES_UL_SPACE );
/*M*/ 	rFmt.ResetAttr( RES_BACKGROUND, RES_COL );
/*M*/ 	rFmt.ResetAttr( RES_URL, RES_EDIT_IN_READONLY );
/*M*/
/*M*/ 	if( !bFrmSz )
/*M*/ 		rFmt.SetAttr( aFrmSz );
/*M*/
/*M*/ 	if( bChgAnchor )
/*M*/ 		rFmt.MakeFrms();
/*M*/
/*M*/
/*M*/ 	SetModified();
/*M*/ 	return bChgAnchor;
/*M*/ }

/*N*/ void SwDoc::GetGrfNms( const SwFlyFrmFmt& rFmt, String* pGrfName,
/*N*/ 						String* pFltName ) const
/*N*/ {
/*N*/ 	SwNodeIndex aIdx( *rFmt.GetCntnt().GetCntntIdx(), 1 );
/*N*/ 	const SwGrfNode* pGrfNd = aIdx.GetNode().GetGrfNode();
/*N*/ 	if( pGrfNd && pGrfNd->IsLinkedFile() )
/*N*/ 		pGrfNd->GetFileFilterNms( pGrfName, pFltName );
/*N*/ }

/*************************************************************************
|*
|*	SwDoc::ChgAnchor()
|*
|*	Ersterstellung		MA 10. Jan. 95
|*	Letzte Aenderung	JP 08.07.98
|*
*************************************************************************/

/*?*/ BOOL SwDoc::ChgAnchor( const SdrMarkList& rMrkList, int eAnchorId,
/*?*/ 						BOOL bSameOnly, BOOL bPosCorr )
/*?*/ {
/*?*/ 	ASSERT( GetRootFrm(), "Ohne Layout geht gar nichts" );
/*?*/
/*?*/ 	if( !rMrkList.GetMarkCount() ||
/*?*/ 		rMrkList.GetMark( 0 )->GetObj()->GetUpGroup() )
/*?*/ 		return FALSE;			// Kein Ankerwechsel innerhalb von Gruppen
/*?*/
/*?*/ 	BOOL bUnmark = FALSE;
/*?*/ 	for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
/*?*/ 	{
/*?*/ 		SdrObject *pObj = rMrkList.GetMark( i )->GetObj();
/*?*/ 		if ( !pObj->IsWriterFlyFrame() )
/*?*/ 		{
/*?*/             SwDrawContact* pContact = (SwDrawContact*)GetUserCall(pObj);
/*?*/
/*?*/             // OD 27.06.2003 #108784# - consider, that drawing object has
/*?*/             // no user call. E.g.: a 'virtual' drawing object is disconnected by
/*?*/             // the anchor type change of the 'master' drawing object.
/*?*/             // Continue with next selected object and assert, if this isn't excepted.
/*?*/             if ( !pContact )
/*?*/             {
/*?*/ #ifdef DBG_UTIL
/*?*/                 bool bNoUserCallExcepted =
/*?*/                         pObj->ISA(SwDrawVirtObj) &&
/*?*/                         !static_cast<SwDrawVirtObj*>(pObj)->IsConnected();
/*?*/                 ASSERT( bNoUserCallExcepted, "SwDoc::ChgAnchor(..) - no contact at selected drawing object" );
/*?*/ #endif
/*?*/                 continue;
/*?*/             }
/*?*/
/*?*/             // OD 17.06.2003 #108784# - determine correct 'old' anchor frame,
/*?*/             // considering 'virtual' drawing objects.
/*?*/             const SwFrm* pOldAnch = 0L;
/*?*/             if ( pObj->ISA(SwDrawVirtObj) )
/*?*/             {
/*?*/                 pOldAnch = static_cast<SwDrawVirtObj*>(pObj)->GetAnchorFrm();
/*?*/             }
/*?*/             else
/*?*/             {
/*?*/                 pOldAnch = pContact->GetAnchor();
/*?*/             }
/*?*/ 			const SwFrm *pNewAnch = pOldAnch;
/*?*/
/*?*/ 			BOOL bChanges = TRUE;
/*?*/ 			xub_StrLen nIndx = STRING_NOTFOUND;
/*?*/ 			SwTxtNode *pTxtNode;
/*?*/ 			int nOld = pContact->GetFmt()->GetAnchor().GetAnchorId();
/*?*/ 			if( !bSameOnly && FLY_IN_CNTNT == nOld )
/*?*/ 			{
/*?*/ 				const SwPosition *pPos =
/*?*/ 					pContact->GetFmt()->GetAnchor().GetCntntAnchor();
/*?*/ 				pTxtNode = pPos->nNode.GetNode().GetTxtNode();
/*?*/ 				ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
/*?*/ 				nIndx = pPos->nContent.GetIndex();
/*?*/ 				if( !pOldAnch )
/*?*/ 				{
/*?*/ 					pContact->ConnectToLayout();
/*?*/ 					pOldAnch = pContact->GetAnchor();
/*?*/ 				}
/*?*/ 				pOldAnch->Calc();
/*?*/                 pObj->ImpSetAnchorPos( pOldAnch->GetFrmAnchorPos( ::binfilter::HasWrap( pObj ) ) );
/*?*/ 			}
/*?*/
/*?*/ 			if ( bSameOnly )
/*?*/ 				eAnchorId = nOld;
/*?*/
/*?*/ 			bChanges = FLY_IN_CNTNT != eAnchorId;
/*?*/ 			SwFmtAnchor  aNewAnch( (RndStdIds)eAnchorId );
/*?*/             const Point aPt( pObj->GetAnchorPos() + pObj->GetRelativePos() );
/*?*/
/*?*/ 			switch ( eAnchorId )
/*?*/ 			{
/*?*/ 			case FLY_AT_CNTNT:
/*?*/ 			case FLY_AUTO_CNTNT:
/*?*/                 {
/*?*/                     const Point aNewPoint = pOldAnch &&
/*?*/                                              ( pOldAnch->IsVertical() ||
/*?*/                                                pOldAnch->IsRightToLeft() ) ?
/*?*/                                              pObj->GetBoundRect().TopRight() :
/*?*/                                              aPt;
/*?*/
/*?*/                     // OD 18.06.2003 #108784# - allow drawing objects in header/footer
/*?*/                     pNewAnch = ::binfilter::FindAnchor( pOldAnch, aNewPoint, false );
/*?*/                     if( pNewAnch->IsTxtFrm() && ((SwTxtFrm*)pNewAnch)->IsFollow() )
/*?*/                         pNewAnch = ((SwTxtFrm*)pNewAnch)->FindMaster();
/*?*/                     if( pNewAnch->IsProtected() )
/*?*/                         pNewAnch = 0;
/*?*/                     else
/*?*/                     {
/*?*/                         SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
/*?*/                         aNewAnch.SetType( (RndStdIds)eAnchorId );
/*?*/                         aNewAnch.SetAnchor( &aPos );
/*?*/                     }
/*?*/                 }
/*?*/ 				break;
/*?*/
/*?*/ 			case FLY_AT_FLY: // LAYER_IMPL
/*?*/ 				{
/*?*/ 					//Ausgehend von der linken oberen Ecke des Fly den
/*?*/ 					//dichtesten SwFlyFrm suchen.
/*?*/ 					SwFrm *pTxtFrm;
/*?*/ 					{
/*?*/ 						SwCrsrMoveState aState( MV_SETONLYTEXT );
/*?*/ 						SwPosition aPos( GetNodes() );
/*?*/ 						Point aPoint( aPt );
/*?*/ 						aPoint.X() -= 1;
/*?*/ 						GetRootFrm()->GetCrsrOfst( &aPos, aPoint, &aState );
/*?*/                         // OD 20.06.2003 #108784# - consider that drawing objects
/*?*/                         // can be in header/footer. Thus, <GetFrm()> by left-top-corner
/*?*/                         pTxtFrm = aPos.nNode.GetNode().
/*?*/                                         GetCntntNode()->GetFrm( &aPt, 0, FALSE );
/*?*/ 					}
/*?*/ 					const SwFrm *pTmp = ::binfilter::FindAnchor( pTxtFrm, aPt );
/*?*/ 					pNewAnch = pTmp->FindFlyFrm();
/*?*/ 					if( pNewAnch && !pNewAnch->IsProtected() )
/*?*/ 					{
/*?*/ 						const SwFrmFmt *pTmpFmt = ((SwFlyFrm*)pNewAnch)->GetFmt();
/*?*/ 						const SwFmtCntnt& rCntnt = pTmpFmt->GetCntnt();
/*?*/ 						SwPosition aPos( *rCntnt.GetCntntIdx() );
/*?*/ 						aNewAnch.SetAnchor( &aPos );
/*?*/ 						break;
/*?*/ 					}
/*?*/
/*?*/ 					aNewAnch.SetType( FLY_PAGE );
/*?*/ 					// no break
/*?*/ 				}
/*?*/ 			case FLY_PAGE:
/*?*/ 				{
/*?*/ 					pNewAnch = GetRootFrm()->Lower();
/*?*/ 					while ( pNewAnch && !pNewAnch->Frm().IsInside( aPt ) )
/*?*/ 						pNewAnch = pNewAnch->GetNext();
/*?*/ 					if ( !pNewAnch )
/*?*/ 						continue;
/*?*/
/*?*/ 					aNewAnch.SetPageNum( ((SwPageFrm*)pNewAnch)->GetPhyPageNum());
/*?*/ 				}
/*?*/ 				break;
/*?*/ 			case FLY_IN_CNTNT:
/*?*/ 				if( bSameOnly )	// Positions/Groessenaenderung
/*?*/ 				{
/*?*/ 					SwDrawFrmFmt *pFmt = (SwDrawFrmFmt*)pContact->GetFmt();
/*?*/ 					const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
/*?*/                     SwTwips nRelPos = pObj->GetRelativePos().Y();
/*?*/                     const SwFrm *pTmp = pContact->GetAnchor();
/*?*/                     if( pTmp && pTmp->IsVertical() )
/*?*/                     {
/*?*/                         nRelPos = pObj->GetRelativePos().X();
/*?*/                         if( !pTmp->IsReverse() )
/*?*/                             nRelPos = -nRelPos -pObj->GetSnapRect().GetWidth();
/*?*/                     }
/*?*/                     if ( rVert.GetPos() != nRelPos ||
/*?*/ 							VERT_NONE != rVert.GetVertOrient() )
/*?*/ 					{
/*?*/ 						SwFmtVertOrient aVert( rVert );
/*?*/ 						aVert.SetVertOrient( VERT_NONE );
/*?*/                         aVert.SetPos( nRelPos );
/*?*/ 						SetAttr( aVert, *pFmt );
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						if( !pOldAnch )
/*?*/ 						{
/*?*/ 							pContact->ConnectToLayout();
/*?*/ 							pOldAnch = pContact->GetAnchor();
/*?*/ 						}
/*?*/ 						((SwTxtFrm*)pOldAnch)->Prepare();
/*?*/ 					}
/*?*/ 				}
/*?*/ 				else 			// Ankerwechsel
/*?*/ 				{
/*?*/                     // OD 18.06.2003 #108784# - allow drawing objects in header/footer
/*?*/                     pNewAnch = ::binfilter::FindAnchor( pOldAnch, aPt, false );
/*?*/ 					if( pNewAnch->IsProtected() )
/*?*/ 					{
/*?*/ 						pNewAnch = 0;
/*?*/ 						break;
/*?*/ 					}
/*?*/
/*?*/ 					bUnmark = ( 0 != i );
/*?*/ 					Point aPoint( aPt );
/*?*/ 					aPoint.X() -= 1;	// nicht im DrawObj landen!!
/*?*/ 					aNewAnch.SetType( FLY_IN_CNTNT );
/*?*/ 					SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
/*?*/ 					if ( pNewAnch->Frm().IsInside( aPoint ) )
/*?*/ 					{
/*?*/ 					// es muss ein TextNode gefunden werden, denn nur dort
/*?*/ 					// ist ein inhaltsgebundenes DrawObjekt zu verankern
/*?*/ 						SwCrsrMoveState aState( MV_SETONLYTEXT );
/*?*/ 						GetRootFrm()->GetCrsrOfst( &aPos, aPoint, &aState );
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						SwCntntNode &rCNd = (SwCntntNode&)
/*?*/ 							*((SwCntntFrm*)pNewAnch)->GetNode();
/*?*/ 						if ( pNewAnch->Frm().Bottom() < aPt.Y() )
/*?*/ 							rCNd.MakeStartIndex( &aPos.nContent );
/*?*/ 						else
/*?*/ 							rCNd.MakeEndIndex( &aPos.nContent );
/*?*/ 					}
/*?*/ 					aNewAnch.SetAnchor( &aPos );
/*?*/ 					SetAttr( aNewAnch, *pContact->GetFmt() );
/*?*/ 					SwTxtNode *pNd = aPos.nNode.GetNode().GetTxtNode();
/*?*/ 					ASSERT( pNd, "Crsr steht nicht auf TxtNode." );
/*?*/
/*?*/ 					pNd->Insert( SwFmtFlyCnt( pContact->GetFmt() ),
/*?*/ 									aPos.nContent.GetIndex(), 0 );
/*?*/ 				}
/*?*/ 				break;
/*?*/ 			default:
/*?*/ 				ASSERT( !this, "unexpected AnchorId." );
/*?*/ 			}
/*?*/
/*?*/ 			if( bChanges && pNewAnch )
/*?*/ 			{
/*?*/                 // OD 20.06.2003 #108784# - consider that a 'virtual' drawing
/*?*/                 // object is disconnected from layout, e.g. caused by an anchor
/*?*/                 // type change.
/*?*/                 if ( pObj->ISA(SwDrawVirtObj) )
/*?*/                 {
/*?*/                     SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pObj);
/*?*/                     if ( !pDrawVirtObj->IsConnected() )
/*?*/                     {
/*?*/                         // 'virtual' drawing object disconnected from layout.
/*?*/                         // Thus, change to 'master' drawing object
/*?*/                         pObj = &(pDrawVirtObj->ReferencedObj());
/*?*/                     }
/*?*/                 }
/*?*/ #ifdef DBG_UTIL
/*?*/                 // SetAttr() removes the ParaPortion of pNewAnch, which is required by
/*?*/                 // GetFrmAnchorPos. Therefore aTmpPoint has to be calculated before
/*?*/                 // the call of SetAttr().
/*?*/                 // OD 20.06.2003 #108784# - refine for assertion:
/*?*/                 // consider anchor change from page to something in header/footer
/*?*/                 Point aProposedAnchorPos;
/*?*/                 if ( nOld == FLY_PAGE &&
/*?*/                      pContact->GetAnchor()->FindFooterOrHeader() )
/*?*/                 {
/*?*/                     aProposedAnchorPos = pContact->GetAnchor()->GetFrmAnchorPos( ::binfilter::HasWrap( pObj ) );
/*?*/                 }
/*?*/                 else
/*?*/                 {
/*?*/                     // SetAttr() removes the ParaPortion of pNewAnch, which is required by
/*?*/                     // GetFrmAnchorPos. Therefore aTmpPoint has to be calculated before
/*?*/                     // the call of SetAttr().
/*?*/                     aProposedAnchorPos = pNewAnch->GetFrmAnchorPos( ::binfilter::HasWrap( pObj ) );
/*?*/                 }
/*?*/ #endif
/*?*/                 SetAttr( aNewAnch, *pContact->GetFmt() );
/*?*/                 if( bPosCorr )
/*?*/                 {
/*?*/                     const Point aTmpRel( aPt - pObj->GetAnchorPos() );
/*?*/
/*?*/                     // #102344# Use SetRelativePos here so that eventually
/*?*/                     // connectors cobnnected to this object get the necessary refresh.
/*?*/                     pObj->SetRelativePos( aTmpRel );
/*?*/                 }
/*?*/
/*?*/ #ifdef DBG_UTIL
/*?*/ 		{
/*?*/                 	const Point aIstA( pObj->GetAnchorPos() );
/*?*/                 	ASSERT( pOldAnch == pNewAnch || aIstA == aProposedAnchorPos,
/*?*/ 				"SwDoc::ChgAnchor(..): Wrong Anchor-Pos." );
/*?*/ 		}
/*?*/ #endif
/*?*/ 			}
/*?*/
/*?*/ 			if ( pNewAnch && STRING_NOTFOUND != nIndx )
/*?*/ 			{
/*?*/ 				//Bei InCntnt's wird es spannend: Das TxtAttribut muss vernichtet
/*?*/ 				//werden. Leider reisst dies neben den Frms auch noch das Format mit
/*?*/ 				//in sein Grab. Um dass zu unterbinden loesen wir vorher die
/*?*/ 				//Verbindung zwischen Attribut und Format.
/*?*/ 				SwTxtAttr *pHnt = pTxtNode->GetTxtAttr( nIndx, RES_TXTATR_FLYCNT );
/*?*/ 				((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt();
/*?*/
/*?*/ 				//Die Verbindung ist geloest, jetzt muss noch das Attribut vernichtet
/*?*/ 				//werden.
/*?*/ 				pTxtNode->Delete( RES_TXTATR_FLYCNT, nIndx, nIndx );
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*?*/
/*?*/ 	SetModified();
/*?*/
/*?*/ 	return bUnmark;
/*?*/ }


/* -----------------23.07.98 13:56-------------------
 *
 * --------------------------------------------------*/
/* -----------------23.07.98 13:56-------------------
 *
 * --------------------------------------------------*/
/*N*/ int SwDoc::Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest )
/*N*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 int nErr = Chainable( rSource, rDest );
/*N*/ }
/* -----------------23.07.98 13:56-------------------
 *
 * --------------------------------------------------*/
/*N*/ void SwDoc::Unchain( SwFrmFmt &rFmt )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwFmtChain aChain( rFmt.GetChain() );
/*N*/ }



}
