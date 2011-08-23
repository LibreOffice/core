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
#include <bf_svtools/itemiter.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/svdpage.hxx>

#include <fmtfsize.hxx>

#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <fmtcntnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtcnct.hxx>
#include <frmatr.hxx>
#include <swtblfmt.hxx>

// OD 27.06.2003 #108784#
#include <fmtflcnt.hxx>

#include <errhdl.hxx>

#include <ndtxt.hxx>
#include "swerror.h"
#include "doc.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "pam.hxx"
#include "section.hxx"
#include "flypos.hxx"
#include "sw3imp.hxx"
#include "poolfmt.hxx"
#include "dcontact.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "hints.hxx"
#include "ndnotxt.hxx"
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////

// Finden eines AttrSets nach Index

SwFmt* lcl_sw3io__GetUserPoolFmt( USHORT nId, const SvPtrarr* pFmtArr )
{
    SwFmt* pFmt;
    for( USHORT n = 0; n < pFmtArr->Count(); ++n )
        if( nId == (pFmt = (SwFmt*)(*pFmtArr)[n])->GetPoolFmtId() )
            return pFmt;
    return 0;
}

// Whichwerte werden nach Gruppen gespreizt, so dass die einzelnen
// Gruppen getrennt um neue Attribute erweitert werden koennen.

// Umwandeln eines normalen Whichwertes in einen gespreizten Wert

/*N*/ USHORT lcl_sw3io__ExpandWhich( USHORT nWhich )
/*N*/ {
/*N*/ 	// #95500#: There is no seperate range for box attributes in the file
/*N*/ 	// format, but they are treated as graphic attributes!
/*N*/ 	// Since we have extended the range of graphic attributes, we have
/*N*/ 	// to move them back to RES_GRFATR_ROTATION, because that's where the
/*N*/ 	// box attributes reange started in the 5.2..
/*N*/ 	if( nWhich >= RES_BOXATR_BEGIN )
/*N*/ 		return (nWhich <= RES_BOXATR_VALUE) ? (nWhich - RES_BOXATR_BEGIN + RES_GRFATR_ROTATION - RES_GRFATR_BEGIN + 0x6000) : 0;
/*N*/ 	if( nWhich >= RES_GRFATR_BEGIN )
/*N*/ 		return (nWhich <= RES_GRFATR_CROPGRF) ? (nWhich - RES_GRFATR_BEGIN + 0x6000) : 0;
/*N*/ 	if( nWhich >= RES_FRMATR_BEGIN )
/*N*/ 		return (nWhich <= RES_COLUMNBALANCE) ? (nWhich - RES_FRMATR_BEGIN + 0x5000) : 0;
/*N*/ 	if( nWhich >= RES_PARATR_BEGIN )
/*N*/ 		return (nWhich <= RES_PARATR_NUMRULE) ? (nWhich - RES_PARATR_BEGIN + 0x4000) : 0;
/*N*/ 	if( nWhich >= RES_TXTATR_NOEND_BEGIN )
/*N*/ 		return (nWhich <= RES_TXTATR_HARDBLANK) ? (nWhich - RES_TXTATR_NOEND_BEGIN + 0x3000) : 0;
/*N*/ 	if( nWhich >= RES_TXTATR_WITHEND_BEGIN )
/*N*/ 		return (nWhich <= RES_TXTATR_CHARFMT) ? (nWhich - RES_TXTATR_WITHEND_BEGIN + 0x2000) : 0;
/*N*/ 	return (nWhich <= RES_CHRATR_BACKGROUND) ? (nWhich - RES_CHRATR_BEGIN + 0x1000) : 0;
/*N*/ }

// Umwandeln eines gespreizten Whichwertes in den normalen Wert

/*N*/ USHORT lcl_sw3io__CompressWhich( USHORT nWhich, USHORT nVersion )
/*N*/ {
/*N*/ 	if( nWhich >= 0x6000 )
/*N*/ 	{
/*N*/ 		// #95500#: Files written with the SWG_NEWGRFATTR version have the
/*N*/ 		// extended grapic attribute range, but didn't correct it while
/*N*/ 		// saving: File before that version didn't have the extended graphic
/*N*/ 		// attribute range and need a correction. Files written with later
/*N*/ 		// version did correct the range while saving and therfor need a
/*N*/ 		// correction while loading, too.that version didn't have the extended graphic
/*N*/ 		// attribute range and need a correction. Files written with later
/*N*/ 		// version did correct the range while saving and therfor need a
/*N*/ 		// correction while loading, too.
/*N*/ 
/*N*/ 		(nWhich -= 0x6000 ) += RES_GRFATR_BEGIN;
/*N*/ 		if( SWG_NEWGRFATTR != nVersion && nWhich > RES_GRFATR_CROPGRF )
/*N*/ 			nWhich += RES_GRFATR_END - RES_GRFATR_CROPGRF - 1;
/*N*/ 		return nWhich;
/*N*/ 	}
/*N*/ 	if( nWhich >= 0x5000 )
/*N*/ 		return nWhich - 0x5000 + RES_FRMATR_BEGIN;
/*N*/ 	if( nWhich >= 0x4000 )
/*N*/ 		return nWhich - 0x4000 + RES_PARATR_BEGIN;
/*N*/ 	if( nWhich >= 0x3000 )
/*N*/ 		return nWhich - 0x3000 + RES_TXTATR_NOEND_BEGIN;
/*N*/ 	if( nWhich >= 0x2000 )
/*N*/ 		return nWhich - 0x2000 + RES_TXTATR_WITHEND_BEGIN;
/*N*/ 	if( nWhich >= 0x1000 )
/*N*/ 		return nWhich - 0x1000 + RES_CHRATR_BEGIN;
/*N*/ 	// Alte Dokumente vor dem 21.04.95
/*N*/ 	return nWhich;
/*N*/ }

sal_Bool lcl_sw3io_insFtn( const SwTxtNode *pTxtNd )
{
    ASSERT( pTxtNd, "There is the text node?" );
    if( !pTxtNd )
        return FALSE;

    const SwStartNode *pSttNd = pTxtNd->FindStartNode();
    while( pSttNd && 
           (pSttNd->IsTableNode() || pSttNd->IsSectionNode() ||
            SwTableBoxStartNode == pSttNd->GetStartNodeType() ) )
        pSttNd = pSttNd->FindStartNode();
    return !pSttNd || SwNormalStartNode == pSttNd->GetStartNodeType();
}

//////////////////////////////////////////////////////////////////////////////

// Einlesen eines Attributs
// Falls Bgn und End nicht besetzt sind, wird STRING_LEN zurueckgeliefert.
// Fall nur Bgn besetzt ist, ist End == Bgn

// BYTE		Flags
//			0x10 - Beginn ist gueltig
//			0x20 - Ende ist gueltig
// UINT16	Which
// UINT16	Attributversion
// UINT16	Beginn (opt)
// UINT16	Ende (opt)
// Attribut

/*N*/ SfxPoolItem* Sw3IoImp::InAttr( xub_StrLen& nBgn, xub_StrLen& nEnd,
/*N*/ 							   const SwTxtNode *pTxtNd )
/*N*/ {
/*N*/ 	nBgn = nEnd = STRING_LEN;
/*N*/ 	OpenRec( SWG_ATTRIBUTE );
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	UINT16 nWhich, nVer;
/*N*/ 	*pStrm >> nWhich >> nVer;
/*N*/ 	nWhich = lcl_sw3io__CompressWhich( nWhich, nVersion );
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 	{
/*N*/ 		USHORT nBgn16;
/*N*/ 		*pStrm >> nBgn16;
/*N*/ 		nBgn = nBgn16;
/*N*/ 	}
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 	{
/*N*/ 		USHORT nEnd16;
/*N*/ 		*pStrm >> nEnd16;
/*N*/ 		nEnd = nEnd16;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nEnd = nBgn;
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	SfxPoolItem* pItem = 0;
/*N*/ 	// MIB 1.9.97: In der 3.0 geschriebene Crop-Attribute sind inkompatibel,
/*N*/ 	// also ignorieren wir sie.
/*N*/ 	if( POOLATTR_BEGIN <= nWhich && nWhich < POOLATTR_END &&
/*N*/ 		( nWhich!=RES_GRFATR_CROPGRF || IsVersion(SWG_NONUMLEVEL) )  )
/*N*/ 	{
/*N*/ 		if( nWhich != RES_TXTATR_FTN || 
/*N*/ 			(nFlyLevel == 0 && (!bInsert || lcl_sw3io_insFtn(pTxtNd)) ) )
/*N*/ 		{
/*N*/ 			const SfxPoolItem& rDflt = pDoc->GetAttrPool().GetDefaultItem( nWhich );
/*N*/ 			pItem = rDflt.Create( *pStrm, nVer );
/*N*/ 
/*N*/ 			// Don't insert charset color items that have the system charset
/*N*/ //			if( RES_CHRATR_CHARSETCOLOR == nWhich &&
/*N*/ //				((SvxCharSetColorItem *)pItem)->GetCharSet() == eSysSet )
/*N*/ //			{
/*N*/ //				delete pItem;
/*N*/ //				pItem = 0;
/*N*/ //			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ASSERT( nFlyLevel == 0, "Fussnoten im Fly sind nicht erlaubt" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CloseRec( SWG_ATTRIBUTE );
/*N*/ 	return pItem;
/*N*/ }

// Schreiben eines Attributs

/*N*/ void Sw3IoImp::OutAttr( const SfxPoolItem& rAttr, xub_StrLen nBgn,
/*N*/ 						xub_StrLen nEnd )
/*N*/ {
/*N*/ 	USHORT nWhich = rAttr.Which();
/*N*/ 
/*N*/ 	// Items, die als Version USHRT_MAX zurueckgeben, wollen nicht
/*N*/ 	// geschrieben werden
/*N*/ 	long nFFVersion = pStrm->GetVersion();
/*N*/ 	ASSERT( IsSw31Export() ? nFFVersion==SOFFICE_FILEFORMAT_31
/*N*/ 						   : (nFFVersion==SOFFICE_FILEFORMAT_40 ||
/*N*/ 							  nFFVersion==SOFFICE_FILEFORMAT_50),
/*N*/ 			"FF-Version am Stream stimmt nicht" );
/*N*/ 	USHORT nIVer = rAttr.GetVersion( (USHORT)nFFVersion );
/*N*/ 	if( USHRT_MAX == nIVer )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Das Zeichen-Hintergrund-Attribut gab es in der 3.1 auch noch nicht
/*N*/ 	if( IsSw31Export() && RES_CHRATR_BACKGROUND==nWhich)
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Hints that start behind the maximum string length of version 5.2
/*N*/ 	// don't have to be exported.
/*N*/ 	if( nBgn != STRING_LEN && nBgn > STRING_MAXLEN52 )
/*N*/ 		return;
/*N*/ 	if( nEnd != STRING_LEN && nEnd > STRING_MAXLEN52 )
/*N*/ 		nEnd = STRING_MAXLEN52;
/*N*/ 
/*N*/ 	if( nWhich != RES_TXTATR_FTN || nFlyLevel == 0 )
/*N*/ 	{
/*N*/ 		nWhich = lcl_sw3io__ExpandWhich( nWhich );
/*N*/         // OD 27.06.2003 #108784# - disable export of drawing frame format in header/footer.
/*N*/         bool bExport = true;
/*N*/         {
/*N*/             if ( RES_TXTATR_FLYCNT == rAttr.Which() )
/*N*/             {
/*N*/                 bExport = static_cast<const SwFmtFlyCnt&>(rAttr).Sw3ioExportAllowed();
/*N*/             }
/*N*/         }
/*N*/       if( bExport && nWhich )
/*N*/ 		{
/*N*/ 				
/*N*/ 			OpenRec( SWG_ATTRIBUTE );
/*N*/ 			BYTE cFlags = 0x04;			// Which + Version
/*N*/ 			if( nBgn != STRING_LEN )
/*N*/ 				cFlags += 0x12;			// Begin
/*N*/ 			if( nEnd != nBgn && nBgn != STRING_LEN )
/*N*/ 				cFlags += 0x22;			// End
/*N*/ 			*pStrm << (BYTE) cFlags
/*N*/ 				   << (UINT16) nWhich
/*N*/ 				   << (UINT16) nIVer;
/*N*/ 			if( cFlags & 0x10 )
/*N*/ 				*pStrm << (UINT16) nBgn;
/*N*/ 			if( cFlags & 0x20 )
/*N*/ 				*pStrm << (UINT16) nEnd;
/*N*/ 			rAttr.Store( *pStrm, nIVer );
/*N*/ 			CloseRec( SWG_ATTRIBUTE );
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef DBG_UTIL
/*N*/ 	else
/*N*/ 		ASSERT( !this, "Fussnoten im Fly sind nicht erlaubt" );
/*N*/ #endif
/*N*/ }

// Einlesen eines AttrSets

/*N*/ void Sw3IoImp::InAttrSet( SwAttrSet& rSet )
/*N*/ {
/*N*/ 	OpenRec( SWG_ATTRSET );
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		xub_StrLen n1, n2;
/*N*/ 		SfxPoolItem* pItem = InAttr( n1, n2 );
/*N*/ 		if( pItem )
/*N*/ 			rSet.Put( *pItem ), delete pItem;
/*N*/ 	}
/*N*/ 	CloseRec( SWG_ATTRSET );
/*N*/ }

/*N*/ void Sw3IoImp::OutAttrSet( const SfxItemSet& rSet, BOOL bSectionFmt )
/*N*/ {
/*N*/ 	if( rSet.Count() )
/*N*/ 	{
/*N*/ 		// Beim 3.1 Export den aktuellen Attrset merken
/*N*/ 		const SfxItemSet *pOldExportItemSet;
/*N*/ 		if( pExportInfo )
/*N*/ 		{
/*N*/ 			pOldExportItemSet = pExportInfo->pItemSet;
/*N*/ 			pExportInfo->pItemSet = &rSet;
/*N*/ 		}
/*N*/ 
/*N*/ 		OpenRec( SWG_ATTRSET );
/*N*/ 		SfxItemIter aIter( rSet );
/*N*/ 		const SfxPoolItem* pAttr = aIter.GetCurItem();
/*N*/ 		const SfxItemPool *pTmp = SfxItemPool::GetStoringPool();
/*N*/ 		SfxItemPool::SetStoringPool( &pDoc->GetAttrPool() );
/*N*/ 		for( USHORT i = 0; i < rSet.Count() && Good(); i++ )
/*N*/ 		{
/*N*/ 			// Header- und Footer- Attribute werden u.U. nicht rausgeschrieben,
/*N*/ 			// wenn sie im Leftformat einer Seitenvorlage mit HdrShared
/*N*/ 			// oder FtrShared vorliegen. Dann wird nur das Attribut im
/*N*/ 			// MAsterformat gespeichert (siehe auch SW3PAGE.CXX, OutPageDesc())
/*N*/ 			USHORT nWhich = pAttr->Which();
/*N*/ 			BOOL bNoHdr =  RES_HEADER == nWhich &&
/*N*/ 						   ( nGblFlags & SW3F_NOHDRFMT ) != 0;
/*N*/ 			BOOL bNoFtr = RES_FOOTER == nWhich &&
/*N*/ 						  ( nGblFlags & SW3F_NOFTRFMT ) != 0;
/*N*/ 			BOOL bNoCntnt = bSectionFmt && RES_CNTNT == nWhich;
/*N*/ 			if( !( bNoHdr || bNoFtr || bNoCntnt) )
/*N*/ 				OutAttr( *pAttr, STRING_LEN, STRING_LEN );
/*N*/ 			pAttr = aIter.NextItem();
/*N*/ 		}
/*N*/ 		if( IsSw31Export() && pExportInfo && pExportInfo->bFlyFrmFmt )
/*N*/ 		{
/*N*/ 			// Wenn ein FlyFrm-Format exportiert wird, muss beruecksichtigt
/*N*/ 			// werden, dass die Raender jetzt nicht mehr Bestandteil der
/*N*/ 			// Rahmengroesse sind. Weil die Vorlagen nicht angepasst werden,
/*N*/ 			// wird ggf. hart attributiert
/*N*/ 			const SvxLRSpaceItem& rLRSpace =
/*N*/ 						(const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE );
/*N*/ 			const SvxULSpaceItem& rULSpace =
/*N*/ 						(const SvxULSpaceItem&)rSet.Get( RES_UL_SPACE );
/*N*/ 
/*N*/ 			if( SFX_ITEM_SET != rSet.GetItemState( RES_FRM_SIZE, FALSE ) )
/*N*/ 			{
/*N*/ 				const SwFmtFrmSize& rFrmSize =
/*N*/ 							(const SwFmtFrmSize&)rSet.Get( RES_FRM_SIZE );
/*N*/ 				Size aConvSize(
/*N*/ 					rFrmSize.GetSizeConvertedToSw31( &rLRSpace, &rULSpace ) );
/*N*/ 
/*N*/ 				if( rFrmSize.GetSize() != aConvSize )
/*N*/ 				{
/*?*/ 					SwFmtFrmSize aFrmSize( rFrmSize );
/*?*/ 					aFrmSize.SetSize( aConvSize );
/*?*/ 					OutAttr( aFrmSize, STRING_LEN, STRING_LEN );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( SFX_ITEM_SET != rSet.GetItemState( RES_HORI_ORIENT, FALSE ) )
/*N*/ 			{
/*?*/ 				const SwFmtHoriOrient rHori =
/*?*/ 					(const SwFmtHoriOrient&)rSet.Get( RES_HORI_ORIENT );
/*?*/ 
/*?*/ 				if( rHori.GetRelationOrient() > PRTAREA ||
/*?*/ 					rHori.GetPos() != rHori.GetPosConvertedToSw31( &rLRSpace ) )
/*?*/ 					OutAttr( rHori, STRING_LEN, STRING_LEN );
/*N*/ 			}
/*N*/ 
/*N*/ 			if( SFX_ITEM_SET != rSet.GetItemState( RES_VERT_ORIENT, FALSE ) )
/*N*/ 			{
/*?*/ 				const SwFmtVertOrient rVert =
/*?*/ 					(const SwFmtVertOrient&)rSet.Get( RES_VERT_ORIENT );
/*?*/ 
/*?*/ 				SwTwips nConvPos = rVert.GetPosConvertedToSw31( &rULSpace );
/*?*/ 				if( rVert.GetPos() != nConvPos )
/*?*/ 				{
/*?*/ 					SwFmtVertOrient aVert( rVert );
/*?*/ 					aVert.SetPos( nConvPos );
/*?*/ 					OutAttr( aVert, STRING_LEN, STRING_LEN );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		SfxItemPool::SetStoringPool( pTmp );
/*N*/ 		CloseRec( SWG_ATTRSET );
/*N*/ 
/*N*/ 		// Beim 3.1 Export den alten Attrset zuruecksetzen
/*N*/ 		if( pExportInfo )
/*N*/ 			pExportInfo->pItemSet = pOldExportItemSet;
/*N*/ 	}
/*N*/ }

// Ein Format einlesen
// Hier wird das Format auch erzeugt, falls es nicht uebergeben wurde.

/*N*/ SwFmt* Sw3IoImp::InFormat( BYTE cKind, SwFmt* pFmt )
/*N*/ {
/*N*/ 	if( !OpenRec( cKind ) )
/*N*/ 	{
/*N*/ 		CloseRec( SWG_EOF );
/*N*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ist der Drawing Layer korrekt geladen?
/*N*/ 	if( cKind == SWG_SDRFMT && ( nGblFlags & SW3F_NODRAWING ) )
/*N*/ 	{
/*?*/ 		Warning();
/*?*/ 		CloseRec( cKind );
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 	if( cKind == SWG_FLYFMT || cKind == SWG_SDRFMT ) nFlyLevel++;
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	BYTE cMoreFlags = 0;
/*N*/ 	USHORT nDerived, nPoolId, nStrIdx = IDX_NO_VALUE;
/*N*/ 	SdrObject* pSdrObj = NULL;
/*N*/ 	*pStrm >> nDerived >> nPoolId;
/*N*/ 	// 0x0L - Laenge der Daten
/*N*/ 	// 0x10 - Namensindex des Formats folgt
/*N*/ 	// 0x20 - SdrObject-Referenz folgt
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		*pStrm >> nStrIdx;
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 	{
/*N*/ 		// Sdr-Objekt aus SdrModel auslesen
/*N*/ 		UINT32 nObjRef;
/*N*/ 		*pStrm >> nObjRef;
/*N*/ 		nObjRef += nZOrderOff;
/*N*/ 		SdrPage* pPg = pDoc->MakeDrawModel()->GetPage( 0 );
/*N*/ 		switch( cKind )
/*N*/ 		{
/*N*/ 			case SWG_FLYFMT:
/*N*/ 				if( ( nVersion < SWG_ZORDER )
/*N*/ 				 || ( nGblFlags & SW3F_NODRAWING ) )
/*N*/ 				{
/*N*/ 					pSdrObj = new SwFlyDrawObj;
/*N*/ 					pPg->InsertObject( pSdrObj, nObjRef );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				// sonst kann durchgerauscht werden, da
/*N*/ 				// LoadDrawingLayer() die Objekte schon angelegt hat
/*N*/ 			case SWG_SDRFMT:
/*N*/ 				if( nObjRef >= pPg->GetObjCount() )
/*N*/ 				{
/*?*/ 					ASSERT( !this, "Ungueltige SdrObject-Nummer" );
/*?*/ 					nObjRef = 0;
/*?*/ 					Error();
/*N*/ 				}
/*N*/ 				pSdrObj = pPg->GetObj( nObjRef );
/*N*/ 				// All sdr object have to be in the hidden layer initially.
/*N*/ 				// They will be moved in the visible one when the layout
/*N*/ 				// will be created.
/*N*/ 				if( SWG_SDRFMT==cKind )
/*N*/ 					pSdrObj->NbcSetLayer( pDoc->GetInvisibleLayerIdByVisibleOne( pSdrObj->GetLayer() ) );
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bMoreFlags;
/*N*/ 	if( IsVersion(SWG_LONGIDX) )
/*N*/ 		bMoreFlags = (cFlags & 0x80) != 0;
/*N*/ 	else
/*N*/ 		bMoreFlags = (cFlags & 0x40) != 0;
/*N*/ 
/*N*/ 	if( bMoreFlags )
/*N*/ 		*pStrm >> cMoreFlags;
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	BOOL bReadName;
/*N*/ 	if( IsVersion(SWG_LONGIDX) )
/*N*/ 		bReadName = (cMoreFlags & 0x20) != 0;
/*N*/ 	else
/*N*/ 		bReadName = nStrIdx == IDX_NO_VALUE;
/*N*/ 
/*N*/ 	String aName;
/*N*/ 	if( bReadName )
/*N*/ 	{
/*N*/ 		InString( *pStrm, aName );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aName = aStringPool.Find( nStrIdx );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Line und Box-Formate habe keinen Namen mehr.
/*N*/ 	if( !IsVersion(SWG_LONGIDX) && pFmt &&
/*N*/ 		( pFmt->IsA(TYPE(SwTableLineFmt)) || pFmt->IsA(TYPE(SwTableBoxFmt)) ) )
/*N*/ 	{
/*N*/ 		aName = aEmptyStr;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Format erzeugen
/*N*/ 	if( !pFmt )
/*N*/ 	{
/*N*/ 		switch( cKind )
/*N*/ 		{
/*N*/ 			case SWG_FREEFMT:
/*N*/ 				// Freifliegende Frameformate werden von Headers
/*N*/ 				// und Footers verwendet.
/*N*/ 				pFmt = new SwFrmFmt( pDoc->GetAttrPool(), aName,
/*N*/ 									 pDoc->GetDfltFrmFmt() );
/*N*/ 				break;
/*?*/ 			case SWG_FRAMEFMT:
/*?*/ 				if( IDX_NO_VALUE != nPoolId )
/*?*/ 				{
/*?*/ 					if( IsPoolUserFmt( nPoolId ) )
/*?*/ 					{
/*?*/ 						// wir suchen uns das richtige Format
                            if( 0 == ( pFmt = lcl_sw3io__GetUserPoolFmt( nPoolId,
/*?*/                                               pDoc->GetFrmFmts() )) )
/*?*/                           pFmt = lcl_sw3io__GetUserPoolFmt( nPoolId,
/*?*/                                               pDoc->GetSpzFrmFmts() );
/*?*/                       ASSERT( pFmt, "Format not found." );
/*?*/ 					}
/*?*/ 					else
/*?*/ 						pFmt = pDoc->GetFrmFmtFromPool( nPoolId );
/*?*/ 				}
/*?*/ 				if( !pFmt )
/*?*/ 					pFmt = pDoc->MakeFrmFmt( aName,
/*?*/ 						(SwFrmFmt*) FindFmt( nDerived, cKind ) );
/*?*/ 				else
/*?*/ 					pFmt->ResetAllAttr();
/*?*/ 
/*?*/ 				break;
/*N*/ 			case SWG_FLYFMT:
/*N*/ 				if( pSdrObj && pSdrObj->GetUserCall() )
/*N*/ 				{
/*N*/ 					//s.u., und auch bugdoc 28336.sdw beim schliessen nach laden.
/*?*/ 					ASSERT( !this, "More than one Format" );
/*?*/ 					CloseRec( cKind );
/*?*/ 					nFlyLevel--;
/*?*/ 					return NULL;
/*N*/ 				}
/*N*/ 				pFmt = pDoc->MakeFlyFrmFmt( aName,
/*N*/ 						(SwFlyFrmFmt*) FindFmt( nDerived, cKind ) );
/*N*/ 				if ( pSdrObj )
/*N*/ 					new SwFlyDrawContact( (SwFrmFmt*) pFmt, pSdrObj );
/*N*/ 				break;
/*N*/ 			case SWG_SDRFMT:
/*N*/ 				if ( pSdrObj && (pSdrObj->ISA(SwFlyDrawObj) ||
/*N*/ 								 pSdrObj->GetUserCall()) )
/*N*/ 				{
/*N*/ 					// Pruefen, ob das Draw-Objekt nicht etwa zu einem
/*N*/ 					// Fly-Frame-Format gehoert oder schon verwendet wurde.
/*N*/ 					// Kann passieren, weil sich manche Draw-Objekte selber
/*N*/ 					// zerstoeren, ohne dass das Format geloescht wird. Leider
/*N*/ 					// wurden solche Format bisher auch rausgeschreiben.
/*N*/ 					// (bug fix #25354# fuer alte Doks)
/*?*/ 					CloseRec( cKind );
/*?*/ 					nFlyLevel--;
/*?*/ 					return NULL;
/*N*/ 				}
/*N*/ 				pFmt = pDoc->MakeDrawFrmFmt( aName,
/*N*/ 						(SwFrmFmt*) FindFmt( nDerived, cKind ) );
/*N*/ 				if( pSdrObj )
/*N*/ 					new SwDrawContact( (SwFrmFmt*) pFmt, pSdrObj );
/*N*/ 				break;
/*N*/ 			case SWG_SECTFMT:
/*N*/ 				pFmt = pDoc->MakeSectionFmt(
/*N*/ 						(SwSectionFmt*) FindFmt( nDerived, cKind ) );
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Fuellen eines vorhandenen Formats
/*N*/ 		pFmt->ResetAllAttr();
/*N*/ 		if( nDerived != IDX_NO_VALUE )
/*N*/ 			pFmt->SetDerivedFrom( FindFmt( nDerived, cKind ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nPoolId != IDX_NO_VALUE )
/*N*/ 		pFmt->SetPoolFmtId( nPoolId );
/*N*/ 
/*N*/ 	if( cMoreFlags & 0x01 )
/*?*/ 		pFmt->SetAutoUpdateFmt();
/*N*/ 
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		BYTE cSubtype = Peek();
/*N*/ 		switch( cSubtype )
/*N*/ 		{
/*N*/ 			case SWG_ATTRSET:
/*N*/ 				InAttrSet( (SwAttrSet&) pFmt->GetAttrSet() );
/*N*/ 				((SwAttrSet&)pFmt->GetAttrSet()).SetModifyAtAttr( pFmt );
/*N*/ 
/*N*/ 				if( cKind == SWG_FLYFMT &&
/*N*/ 					!IsVersion( SWG_URLANDMAP, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 				{
/*N*/ 					// In 31-FFohne URL-Attribut muss das Attribut noch vom
/*N*/ 					// Node in das Format verschoben werden
/*N*/ 					SwNoTxtNode *pNd = pDoc->GetNodes()[
/*N*/ 						pFmt->GetCntnt().GetCntntIdx()->GetIndex()+1 ]->GetNoTxtNode();
/*N*/ 
/*N*/ 					const SfxPoolItem *pItem;
/*N*/ 					if ( pNd && SFX_ITEM_SET == pNd->GetSwAttrSet().
/*N*/ 										GetItemState( RES_URL, FALSE, &pItem))
/*N*/ 					{
/*N*/ 						pFmt->SetAttr( *pItem );
/*N*/ 						pNd->ResetAttr( RES_URL );
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if( nVersion < SWG_DESKTOP40 && cKind == SWG_FLYFMT )
/*N*/ 				{
/*N*/ 					// In SW31-FlyFrm-Formaten muessen ein par Attrs
/*N*/ 					// korregiert werden. Sie werden immer gesetzt, da sie
/*N*/ 					// in den Vorlagen nicht korregiert wurden.
/*N*/ 					const SvxLRSpaceItem& rLRSpace = pFmt->GetLRSpace();
/*N*/ 					const SvxULSpaceItem& rULSpace = pFmt->GetULSpace();
/*N*/ 
/*N*/ 					const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
/*N*/ 					Size aConvSize(
/*N*/ 						rFrmSize.GetSizeConvertedFromSw31( &rLRSpace,
/*N*/ 														   &rULSpace ) );
/*N*/ 					if( rFrmSize.GetSize() != aConvSize )
/*N*/ 					{
/*?*/ 						SwFmtFrmSize aFrmSize( rFrmSize );
/*?*/ 						aFrmSize.SetSize( aConvSize );
/*?*/ 						pFmt->SetAttr( aFrmSize );
/*N*/ 					}
/*N*/ 
/*N*/ 					const SwFmtHoriOrient& rHori = pFmt->GetHoriOrient();
/*N*/ 					SwTwips nConvPos =
/*N*/ 						rHori.GetPosConvertedFromSw31( &rLRSpace );
/*N*/ 					if( rHori.GetPos() != nConvPos )
/*N*/ 					{
/*?*/ 						SwFmtHoriOrient aHori( rHori );
/*?*/ 						aHori.SetPos( nConvPos );
/*?*/ 						pFmt->SetAttr( aHori );
/*N*/ 					}
/*N*/ 
/*N*/ 					const SwFmtVertOrient& rVert = pFmt->GetVertOrient();
/*N*/ 					nConvPos = rVert.GetPosConvertedFromSw31( &rULSpace );
/*N*/ 					if( rVert.GetPos() != nConvPos )
/*N*/ 					{
/*N*/ 						SwFmtVertOrient aVert( rVert );
/*N*/ 						aVert.SetPos( nConvPos );
/*N*/ 						pFmt->SetAttr( aVert );
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if( cKind == SWG_FLYFMT && IsVersion( SWG_LONGIDX ) )
/*N*/ 				{
/*N*/ 					const SfxPoolItem *pItem;
/*N*/ 					if( SFX_ITEM_SET == pFmt->GetAttrSet().
/*N*/ 								GetItemState( RES_CHAIN, FALSE, &pItem) )
/*N*/ 					{
/*?*/ 						const SwFmtChain *pChain =
/*?*/ 							(const SwFmtChain *)pItem;
/*?*/ 
/*?*/ 						// Den Next des Prev richtig setzen.
/*?*/ 						SwFlyFrmFmt *pFlyFmt = pChain->GetPrev();
/*?*/ 						if( pFlyFmt )
/*?*/ 						{
/*?*/                            SwFmtChain aChain( pFlyFmt->GetChain() );
/*?*/                           ASSERT( !aChain.GetNext(),
/*?*/                                   "Next ist bereits verkettet" );
/*?*/                           aChain.SetNext( (SwFlyFrmFmt *)pFmt );
/*?*/                           pFlyFmt->SetAttr( aChain );
/*?*/ 						}
/*?*/ 
/*?*/ 						// Den Prev des Next richtig setzen.
/*?*/ 						pFlyFmt = pChain->GetNext();
/*?*/ 						if( pFlyFmt )
/*?*/ 						{
/*?*/                            SwFmtChain aChain( pFlyFmt->GetChain() );
/*?*/                           ASSERT( !aChain.GetPrev(),
/*?*/                                   "Prev ist bereits verkettet" );
/*?*/                           aChain.SetPrev( (SwFlyFrmFmt *)pFmt );
/*?*/                           pFlyFmt->SetAttr( aChain );
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if( bInsert )
/*N*/ 				{
/*N*/ 					// das Format hat sich geaendert!
/*?*/ 					SwFmtChg aHint( pFmt );
/*?*/ 					pFmt->Modify( &aHint, &aHint );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			default:
/*?*/ 				SkipRec();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CloseRec( cKind );
/*N*/ 
/*N*/ 	if( cKind == SWG_SDRFMT &&
/*N*/ 		!IsVersion( SWG_WRAPDRAWOBJ, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 		pFmt->SetAttr( SwFmtSurround( SURROUND_THROUGHT ) );
/*N*/ 
/*N*/ 	// beim Einfuegen eines Dokuments muessen Rahmen, Grafiken und
/*N*/ 	// OLE-Nodes noch einen eindeutigen Namen bekommen
/*N*/ 	// This must happen always, because there are lots of documents
/*N*/ 	// that contain frames with the same name.
/*N*/ 	if( cKind == SWG_FLYFMT )
/*N*/ 	{
/*N*/ 		const SwNode* pNd =
/*N*/ 			pDoc->GetNodes()[ pFmt->GetCntnt().GetCntntIdx()->GetIndex()+1 ];
/*N*/ 		BYTE nNdTyp = !pNd->IsNoTxtNode() ? ND_TEXTNODE : pNd->GetNodeType();
/*N*/ 
/*N*/ 		pFmt->SetName( aEmptyStr );
/*N*/ 
/*N*/ 		// If the name is not changed, its extension must be kept to be able
/*N*/ 		// to load the layout.
/*N*/ 		String aSrchName( aName );
/*N*/ 		aStringPool.RemoveExtension( aSrchName );
/*N*/ 		if( bInsert )
/*N*/ 			aName = aSrchName;
/*N*/ 
/*N*/ 		if( pDoc->FindFlyByName( aSrchName, nNdTyp ) )
/*N*/ 		{
/*?*/ 			switch( nNdTyp )
/*?*/ 			{
/*?*/ 			case ND_TEXTNODE:
/*?*/ 				aName = pDoc->GetUniqueFrameName();
/*?*/ 				break;
/*?*/ 			case ND_GRFNODE:
/*?*/ 				aName = pDoc->GetUniqueGrfName();
/*?*/ 				break;
/*?*/ 			case ND_OLENODE:
/*?*/ 				aName = pDoc->GetUniqueOLEName();
/*?*/ 				break;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pFmt->SetName( aName );
/*N*/ 	if( cKind == SWG_FLYFMT || cKind == SWG_SDRFMT )
/*N*/ 		nFlyLevel--;
/*N*/ 
/*N*/ // OPT: Cache fuer Formate im StringPool
/*N*/ 	if( nStrIdx != IDX_NO_VALUE )
/*N*/ 		aStringPool.SetCachedFmt( nStrIdx, pFmt );
/*N*/ // /OPT: Cache fuer Formate im StringPool
/*N*/ 
/*N*/ 	return pFmt;
/*N*/ }

// Ausgabe eines Formats:
// BYTE	   		Flags
// 				0x10 - Namensindex des Formats folgt
// 				0x20 - SdrObject-Referenz folgt
// UINT16		String-ID des Parents (IDX_NO_VALUE: Formatname folgt)
// UINT16		Pool-ID
// UINT16		String-ID des Formatnamens (opt.)
// INT32		Z-Order des Drawing-Objekts (opt.)
// String       Name des Formats, falls nicht im Pool gefunden (opt.)
// SWG_ATTRSET	Attribute (opt.)

/*N*/ void Sw3IoImp::OutFormat( BYTE cType, const SwFmt& rFmt )
/*N*/ {
/*N*/ 	// Freifliegende Formate werden immer geschrieben, da sie niemals
/*N*/ 	// mehrfach referenziert werden!
/*N*/ 	if( cType != SWG_FREEFMT && cType != SWG_SECTFMT && rFmt.IsWritten() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	OpenRec( cType );
/*N*/ 	// Von welchem Format isses abgeleitet?
/*N*/ 	USHORT nDerived = IDX_NO_VALUE;
/*N*/ 	SwFmt* pDerived = rFmt.DerivedFrom();
/*N*/ 	if( pDerived && SWG_SECTFMT == cType )
/*N*/ 	{
/*N*/ 		// The parent format of TOX content sections must not to be stored,
/*N*/ 		// because the parent format will not exist if the document is read
/*N*/ 		// again. In fact, this format is copied to a newly created one only.
/*N*/ 		// The parent format of TOX header sections has not to be stored,
/*N*/ 		// because the section is inserted into the newly created TOX content
/*N*/ 		// section and that for will be replaced always.
/*N*/ 		const SwSectionFmt* pSectFmt = PTR_CAST( SwSectionFmt, &rFmt );
/*N*/ 		ASSERT( pSectFmt, "no section format?" );
/*N*/ 		if( pSectFmt )
/*N*/ 		{
/*N*/ 			const SwSection* pSect = pSectFmt->GetSection();
/*N*/ 			if( pSect && (TOX_CONTENT_SECTION == pSect->GetType() ||
/*N*/ 						  TOX_HEADER_SECTION == pSect->GetType()) )
/*N*/ 			{
/*N*/ 				pDerived = 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pDerived )
/*N*/ 	{
/*N*/ 		if( pDerived->IsDefault() )
/*N*/ 			nDerived = IDX_DFLT_VALUE;
/*N*/ 		else
/*N*/ 			nDerived = aStringPool.Find( pDerived->GetName(),
/*N*/ 										 pDerived->GetPoolFmtId() );
/*N*/ 	}
/*N*/ 	// Flag Bits:
/*N*/ 	// SW3.1/4.0: Byte 1
/*N*/ 	// 0x0L - Laenge der Daten
/*N*/ 	// 0x10 - Namensindex des Formats folgt
/*N*/ 	// 0x20 - SdrObject-Referenz folgt
/*N*/ 	// 0x40 - zusaetzliches Flag-Byte folgt (nur 4.0)
/*N*/ 	// SW3.1/40: Byte 2
/*N*/ 	// 0x01 - Auto-Update-Fmt (nur 4.0)
/*N*/ 	// SW5.0
/*N*/ 	// 0x0L - Laenge der Daten
/*N*/ 	// 0x10 - Namensindex des Formats folgt
/*N*/ 	// 0x20 - SdrObject-Referenz folgt
/*N*/ 	// 0x40 - frei (fuer ein haeufig benutztes Flag gedacht)
/*N*/ 	// 0x80 - zusaetzliches Flag-Byte folgt
/*N*/ 	// SW5.0: Byte 2
/*N*/ 	// 0x01 - Auto-Update-Fmt
/*N*/ 	// 0x02 - Name des Formats ist als String enthalten
/*N*/ 	BYTE cFlags = 0x04;		// Derived, PoolId
/*N*/ 	BYTE cMoreFlags = 0x00;
/*N*/ 
/*N*/ 	const SwFlyFrm* pExportFlyFrm = NULL;
/*N*/ 
/*N*/ 	if( rFmt.IsAutoUpdateFmt() && !IsSw31Export() )
/*N*/ 		cMoreFlags += 0x01;
/*N*/ 
/*N*/ 	ULONG nSdrRef = 0;
/*N*/ 	USHORT nStrIdx = IDX_NO_VALUE;
/*N*/ 	BOOL bWriteName = FALSE;
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		if( SWG_FRAMEFMT==cType && ( rFmt.IsA(TYPE(SwTableLineFmt)) ||
/*N*/ 									 rFmt.IsA(TYPE(SwTableBoxFmt)) ) )
/*N*/ 		{
/*N*/ 			nStrIdx = GetTblLineBoxFmtStrPoolId40( (SwFrmFmt *)&rFmt );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nStrIdx = aStringPool.Find( rFmt.GetName(), rFmt.GetPoolFmtId() );
/*N*/ 		}
/*N*/ 		if( nStrIdx != IDX_NO_VALUE )
/*N*/ 			cFlags += 0x12;
/*N*/ 		else
/*N*/ 			bWriteName = TRUE;
/*N*/ 
/*N*/ 		if( cMoreFlags != 0 )
/*N*/ 			cFlags += 0x41;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( rFmt.GetName().Len() )
/*N*/ 		{
/*N*/ 			nStrIdx = aStringPool.Find( rFmt.GetName(), rFmt.GetPoolFmtId() );
/*N*/ 			if( nStrIdx != IDX_NO_VALUE )
/*N*/ 			{
/*N*/ 				cFlags += 0x12;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				cMoreFlags += 0x20;
/*N*/ 				bWriteName = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( cMoreFlags != 0 )
/*N*/ 			cFlags += 0x81;
/*N*/ 	}
/*N*/ 
/*N*/ 	// #72785#: In organizer mode, no drawing layer will be written. That for
/*N*/ 	// no sdr object references must be written.
/*N*/ 	if( !bOrganizer )
/*N*/ 	{
/*N*/ 		// Gibt es ein SdrObject?
/*N*/ 		// Zur Zeit nur fuer SDR-Formate!!!
/*N*/ 		if( cType == SWG_SDRFMT )
/*N*/ 		{
/*N*/ 			// Schauen, ob es ein SdrObject dafuer gibt
/*N*/ 			SwClientIter aIter( (SwFmt&) rFmt );
/*N*/ 			if( aIter.First( TYPE(SwDrawContact) ) )
/*N*/ 			{
/*N*/ 				nSdrRef = ((SwDrawContact*)aIter())->GetMaster()->GetOrdNum();
/*N*/ 				cFlags += 0x24;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if( cType == SWG_FLYFMT )
/*N*/ 		{
/*N*/ 			// Schauen, ob es ein SdrObject dafuer gibt
/*N*/ 			SwClientIter aIter( (SwFmt&) rFmt );
/*N*/ 			if( aIter.First( TYPE( SwFlyFrm) ) )
/*N*/ 			{
/*N*/ 				pExportFlyFrm = (SwFlyFrm*)aIter();
/*N*/ 				nSdrRef = pExportFlyFrm->GetVirtDrawObj()->GetOrdNum();
/*N*/ 				cFlags += 0x24;
/*N*/ 			}
/*N*/ 			else if( aIter.First( TYPE( SwFlyDrawContact) ) )
/*N*/ 			{
/*N*/ 				// fix #45256#: Wenn Seitenvorlagen eines Doks importiert werden,
/*N*/ 				// werden dafuer u.U. Master-Objekte in die Page eingetragen.
/*N*/ 				// Wenn diese jetzt noch in der Page stehen, muss dafuer gesorgt
/*N*/ 				// werden, dass beim Laden des Dokuments wieder an der gleichen
/*N*/ 				// Stelle Master-Objekte angelegt werden, weil sonst die Indizes
/*N*/ 				// der Zeichen-Objekte in der Page nicht stimmen.
/*N*/ 				const SdrObject *pMaster =
/*N*/ 					((SwFlyDrawContact*)aIter())->GetMaster();
/*N*/ 				if( pMaster->GetPage() )
/*N*/ 				{
/*?*/ 					nSdrRef = pMaster->GetOrdNum();
/*?*/ 					cFlags += 0x24;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if( (cFlags & 0x20) && pRefSdrObjects )
/*N*/ 	{
/*N*/ 		size_t nPos = nSdrRef / 8;
/*N*/ 		BYTE nByte = 1 << (nSdrRef % 8);
/*N*/ 		ASSERT( (pRefSdrObjects[nPos] & nByte) == 0,
/*N*/ 				"Zeichen-Object doppelt referenziert" );
/*N*/ 
/*N*/ 		pRefSdrObjects[nPos] |= nByte;
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	USHORT nPoolId = rFmt.GetPoolFmtId();
/*N*/ 	// In Colls darf nur das FrmFmt einen PoolId haben
/*N*/ 	if( nPoolId == USHRT_MAX )
/*N*/ 		nPoolId = IDX_NO_VALUE;
/*N*/ 	*pStrm << (BYTE)   cFlags
/*N*/ 		   << (UINT16) nDerived
/*N*/ 		   << (UINT16) nPoolId;
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		*pStrm << (UINT16) nStrIdx;
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		*pStrm << (INT32) nSdrRef;
/*N*/ 	if( cMoreFlags != 0 )
/*N*/ 		*pStrm << (BYTE) cMoreFlags;
/*N*/ 
/*N*/ 	// Falls der Name nicht im Pool ist, direkt raus damit!
/*N*/ 	if( bWriteName )
/*N*/ 		OutString( *pStrm, rFmt.GetName() );
/*N*/ 
/*N*/ 	BOOL bOldExportFlyFrmFmt;
/*N*/ 	const SwFlyFrm* pOldExportFlyFrm;
/*N*/ 	if( pExportInfo )
/*N*/ 	{
/*N*/ 		bOldExportFlyFrmFmt = pExportInfo->bFlyFrmFmt;
/*N*/ 		pOldExportFlyFrm = pExportInfo->pFlyFrm;
/*N*/ 		pExportInfo->bFlyFrmFmt = (cType == SWG_FLYFMT);
/*N*/ 		pExportInfo->pFlyFrm = pExportFlyFrm;
/*N*/ 	}
/*N*/ 	OutAttrSet( rFmt.GetAttrSet(), SWG_SECTFMT == cType );
/*N*/ 	if( pExportInfo )
/*N*/ 	{
/*N*/ 		pExportInfo->bFlyFrmFmt = bOldExportFlyFrmFmt;
/*N*/ 		pExportInfo->pFlyFrm = pOldExportFlyFrm;
/*N*/ 	}
/*N*/ 	CloseRec( cType );
/*N*/ 	if( cType != SWG_FREEFMT && cType != SWG_SECTFMT )
/*N*/ 		((SwFmt&)rFmt).SetWritten();
/*N*/ }

////////////////////////////////////////////////////////////////////////////

// globale FlyFrames

/*N*/ void Sw3IoImp::InFlyFrames()
/*N*/ {
/*N*/ 	BOOL bInsIntoHdrFtrSave = bInsIntoHdrFtr;	// In seitengebundenen sind
/*N*/ 	bInsIntoHdrFtr = FALSE;						// Draw-Objekte immer erlaubt
/*N*/ 
/*N*/ 	USHORT nPageNumOff = 0;
/*N*/ 	if( bInsert )		// im einfuege Modus?
/*N*/ 	{
/*N*/ 		// dann binde die Seitengebundenen an die akt. Seite + Nummer
/*N*/ 		// Dazu stelle erstmal fest, ob es ueberhaupt einen entsprechen
/*N*/ 		// Offset gibt.
/*N*/ 		SwFrm* pFrm;
/*?*/ 		SwCntntNode* pCNd = pDoc->GetNodes()[ pCurPaM->GetPoint()->nNode ]->GetCntntNode();
/*?*/ 		if( pCNd && 0 != ( pFrm = pCNd->GetFrm() ))
/*?*/ 			nPageNumOff = pFrm->FindPageFrm()->GetPhyPageNum() - 1;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT eSave_StartNodeType = eStartNodeType;
/*N*/ 	eStartNodeType = SwFlyStartNode;
/*N*/ 
/*N*/ 	OpenRec( SWG_FLYFRAMES );
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		BYTE cType = Peek();
/*N*/ 		if( cType == SWG_FLYFMT || cType == SWG_SDRFMT )
/*N*/ 		{
/*N*/ 			SwFmt* pFmt = InFormat( cType, NULL );
/*N*/ 			ASSERT( !pFmt || FLY_PAGE >= pFmt->GetAnchor().GetAnchorId(),
/*N*/ 						"Rahmen ist ungueltig gebunden" );
/*N*/ 			if( pFmt && nPageNumOff )
/*N*/ 			{
/*?*/ 				const SwFmtAnchor& rOld = pFmt->GetAnchor();
/*?*/ 				if( FLY_PAGE == rOld.GetAnchorId() )
/*?*/ 				{
/*?*/ 					pFmt->SetAttr( SwFmtAnchor( FLY_PAGE,
/*?*/ 								nPageNumOff + rOld.GetPageNum() ) );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			Error();
/*N*/ 	}
/*N*/ 	CloseRec( SWG_FLYFRAMES );
/*N*/ 	eStartNodeType = eSave_StartNodeType;
/*N*/ 	bInsIntoHdrFtr = bInsIntoHdrFtrSave;
/*N*/ }

// Ausgabe von FlyFrames, die nicht an einem Node kleben

/*N*/ void Sw3IoImp::OutFlyFrames( SwPaM& rPaM )
/*N*/ {
/*N*/ 	if( bSaveAll )
/*N*/ 	{
/*N*/ 		// Alle Frames: ueber das Array gehen
/*N*/ 		const SwSpzFrmFmts* pFlys = pDoc->GetSpzFrmFmts();
/*N*/ 		USHORT nArrLen = pFlys->Count();
/*N*/ 		if( nArrLen )
/*N*/ 		{
/*N*/ 			OpenRec( SWG_FLYFRAMES );
/*N*/ 			for( USHORT i = 0; i < nArrLen; i++ )
/*N*/ 			{
/*N*/ 				const SwFrmFmt* pFmt = (*pFlys)[ i ];
/*N*/ 				const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
/*N*/ 				switch( rAnchor.GetAnchorId() )
/*N*/ 				{
/*N*/ 					case FLY_AT_CNTNT:
/*N*/ 					case FLY_AUTO_CNTNT:
/*N*/ 					case FLY_IN_CNTNT:
/*N*/ 						break;	// nicht die, die am Node kleben
/*N*/ 					case FLY_AT_FLY:
/*N*/ 						// Rahmengebunde Rahmen werden als seitengebunde
/*N*/ 						// Rahmen ins 3.1/4.0-Format exportiert. Sonst
/*N*/ 						// werden sie in der Section exportiert.
/*?*/ 						if( SOFFICE_FILEFORMAT_40 < pStrm->GetVersion() )
/*?*/ 							break;
/*N*/ 					default:
/*N*/ 						ASSERT( FLY_PAGE==rAnchor.GetAnchorId() ||
/*N*/ 								FLY_AT_FLY==rAnchor.GetAnchorId(),
/*N*/ 								"Rahmen ist nicht Seitengebunden" );
/*N*/ 						if( !pFmt->IsDefault() )
/*N*/ 						{
/*N*/ 							BYTE cType = SWG_FLYFMT;
/*N*/ 							if( RES_DRAWFRMFMT == pFmt->Which() )
/*N*/ 								cType = SWG_SDRFMT;
/*N*/ 							OutFormat( cType, *pFmt );
/*N*/ 						}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			CloseRec( SWG_FLYFRAMES );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		// Selektion: Frames absammeln
/*?*/ 		USHORT nArrLen = pDoc->GetSpzFrmFmts()->Count();
/*?*/ 		if( nArrLen > 255 )
/*?*/ 			nArrLen = 255;
/*?*/ 		SwPosFlyFrms aFlys( (BYTE)nArrLen, 50 );
/*?*/ 		pDoc->GetAllFlyFmts( aFlys, &rPaM, TRUE );
/*?*/ 		nArrLen = aFlys.Count();
/*?*/ 		if( nArrLen )
/*?*/ 		{
/*?*/ 			OpenRec( SWG_FLYFRAMES );
/*?*/ 			for( USHORT i = 0; i < nArrLen; i++ )
/*?*/ 			{
/*?*/ 				const SwFrmFmt& rFmt = aFlys[ i ]->GetFmt();
/*?*/ 				const SwFmtAnchor& rAnchor = rFmt.GetAnchor();
/*?*/ 				switch( rAnchor.GetAnchorId() )
/*?*/ 				{
/*?*/ 					case FLY_AT_CNTNT:
/*?*/ 					case FLY_IN_CNTNT:
/*?*/ 					case FLY_AUTO_CNTNT:
/*?*/ 						break;	// nicht die, die am Node kleben
/*?*/ 					case FLY_AT_FLY:
/*?*/ 						// Rahmengebunde Rahmen werden als seitengebunde
/*?*/ 						// Rahmen ins 3.1/4.0-Format exportiert. Sonst
/*?*/ 						// werden sie in der Section exportiert.
/*?*/ 						if( SOFFICE_FILEFORMAT_40 < pStrm->GetVersion() )
/*?*/ 							break;
/*?*/ 					default:
/*?*/ 						ASSERT( FLY_PAGE==rAnchor.GetAnchorId() ||
/*?*/ 								FLY_AT_FLY==rAnchor.GetAnchorId(),
/*?*/ 								"Rahmen ist nicht Seitengebunden" );
/*?*/ 						if( !rFmt.IsDefault() )
/*?*/ 						{
/*?*/ 							BYTE cType = SWG_FLYFMT;
/*?*/ 							if( RES_DRAWFRMFMT == rFmt.Which() )
/*?*/ 								cType = SWG_SDRFMT;
/*?*/ 							OutFormat( cType, rFmt );
/*?*/ 						}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			CloseRec( SWG_FLYFRAMES );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// in FLYPOS.CXX

extern BOOL TstFlyRange( const SwPaM* pPam, const SwIndex& rFlyPos );

/*N*/ void Sw3IoImp::CollectFlyFrms( const SwPaM* pPaM )
/*N*/ {
/*N*/ 	if( !pFlyFrms )
/*N*/ 	{
/*N*/ 		pFlyFrms = new SwPosFlyFrms;
/*N*/ 		SwPosFlyFrm *pFPos = 0;
/*N*/ 		const SwPosition* pAPos;
/*N*/ 
/*N*/ 		// alle absatzgebundenen Flys einsammeln
/*N*/ 		for( USHORT n = 0; n < pDoc->GetSpzFrmFmts()->Count(); ++n )
/*N*/ 		{
/*N*/ 			SwFrmFmt *pFly = (*pDoc->GetSpzFrmFmts())[ n ];
/*N*/ 			if( RES_DRAWFRMFMT == pFly->Which() ||
/*N*/ 				RES_FLYFRMFMT  == pFly->Which() )
/*N*/ 			{
/*N*/ 				const SwFmtAnchor& rAnchor = pFly->GetAnchor();
/*N*/ 				if( ( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
/*N*/ 					  FLY_AT_FLY == rAnchor.GetAnchorId() ||
/*N*/ 					  FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ) &&
/*N*/ 					NULL != ( pAPos = rAnchor.GetCntntAnchor()) )
/*N*/ 				{
/*N*/ 					pFPos = new SwPosFlyFrm( pAPos->nNode, pFly,
/*N*/ 												pFlyFrms->Count() );
/*N*/ 					pFlyFrms->Insert( pFPos );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( !pFlyFrms->Count() )
/*N*/ 			delete pFlyFrms, pFlyFrms = NULL;
/*N*/ 	}
/*N*/ }

// Freigabe der gesammelten FlyFrame-Infos nach dem Schreiben

/*N*/ void Sw3IoImp::FreeFlyFrms()
/*N*/ {
/*N*/ 	if( pFlyFrms )
/*N*/ 	{
/*N*/ 		for( USHORT i = 0; i < pFlyFrms->Count(); i++ )
/*N*/ 			delete (*pFlyFrms)[ i ];
/*N*/ 		delete pFlyFrms;
/*N*/ 		pFlyFrms = NULL;
/*N*/ 	}
/*N*/ }

// Suchen eines absatzgebundenen Flys. Wenn gefunden, wird der Fly
// zurueckgeliefert und das Element aus dem Array entfernt.

/*N*/ SwFmt* Sw3IoImp::FindFlyFrm( ULONG nNodeId )
/*N*/ {
/*N*/ 	SwFmt* pFmt = NULL;
/*N*/ 	if( pFlyFrms )
/*N*/ 	{
/*N*/ 		for( USHORT i = 0; i < pFlyFrms->Count(); i++ )
/*N*/ 		{
/*N*/ 			ULONG nId = (*pFlyFrms)[ i ]->GetNdIndex().GetIndex();
/*N*/ 			if( nNodeId == nId )
/*N*/ 			{
/*N*/ 				pFmt = (SwFmt*) &(*pFlyFrms)[ i ]->GetFmt();
/*N*/ 				pFlyFrms->DeleteAndDestroy( i );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			if( nId > nNodeId )
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }

////////////////////////////////////////////////////////////////////////////


/*  */


}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
