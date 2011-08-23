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

#include "swerror.h"

#include <horiornt.hxx>

#include "doc.hxx"

#include <fmtfsize.hxx>

#include <errhdl.hxx>

#define _SVSTDARR_STRINGSDTOR
#include "sw3imp.hxx"
#include "ddefld.hxx"
#include "swddetbl.hxx"
#include "swtblfmt.hxx"
#include "ndindex.hxx"
namespace binfilter {


#ifdef DBG_UTIL
static ULONG nCntntBox = 0;
#endif


/////////////////////////////////////////////////////////////////////////////

// Das Attribut darf im Prinzip nicht > 65535 werden, da sonst MAs
// Rechenroutinen baden gehen. Da einige alte Tabellen groessere Werte
// enthalten, setzt InTable() ggf. einen Divisionsfaktor, um den
// die gelesenen Werte heruntergeteilt werden. Normalerweise ist er 1.


/*N*/ void lcl_sw3io_AdjustFrmSize( SwFrmFmt* pFmt, long nAdj )
/*N*/ {
/*N*/ 	if( pFmt && nAdj > 1
/*N*/ 	 && pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE, FALSE ) == SFX_ITEM_SET )
/*N*/ 	{
/*?*/ 		SwFmtFrmSize aSz( pFmt->GetFrmSize() );
/*?*/ 		aSz.SetWidth( aSz.GetWidth() / nAdj );
/*?*/ 		aSz.SetHeight( aSz.GetHeight() / nAdj );
/*?*/ 		pFmt->SetAttr( aSz );
/*N*/ 	}
/*N*/ }

/*  */

// Line- und Box-Formate fuer den 3.1-/4.0-Export sammeln

BOOL lcl_sw3io_CollectLineFmts( const SwTableLine*& rpLine, void* pPara );

/*N*/ BOOL lcl_sw3io_CollectBoxFmts( const SwTableBox*& rpBox, void* pPara )
/*N*/ {
/*N*/ 	Sw3FrmFmts *pFrmFmts = (Sw3FrmFmts *)pPara;
/*N*/ 	if( USHRT_MAX == pFrmFmts->GetPos( rpBox->GetFrmFmt() ) )
/*N*/ 		pFrmFmts->Insert( rpBox->GetFrmFmt(), pFrmFmts->Count() );
/*N*/ 
/*N*/ 	if( rpBox->GetTabLines().Count() )
/*N*/ 		((SwTableBox *)rpBox)->GetTabLines()
/*N*/ 			.ForEach( &lcl_sw3io_CollectLineFmts, pPara );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL lcl_sw3io_CollectLineFmts( const SwTableLine*& rpLine, void* pPara )
/*N*/ {
/*N*/ 	Sw3FrmFmts *pFrmFmts = (Sw3FrmFmts *)pPara;
/*N*/ 	if( USHRT_MAX == pFrmFmts->GetPos( rpLine->GetFrmFmt() ) )
/*N*/ 		pFrmFmts->Insert( rpLine->GetFrmFmt(), pFrmFmts->Count() );
/*N*/ 
/*N*/ 	((SwTableLine *)rpLine)->GetTabBoxes()
/*N*/ 		.ForEach( &lcl_sw3io_CollectBoxFmts, pPara );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void Sw3IoImp::CollectTblLineBoxFmts40()
/*N*/ {
/*N*/ 	ASSERT( pExportInfo, "Wo ist die Export-Info?" );
/*N*/ 	for( USHORT i=0; i<pDoc->GetTblFrmFmts()->Count(); i++ )
/*N*/ 	{
/*N*/ 		SwClientIter aIter( *(*pDoc->GetTblFrmFmts())[i] );
/*N*/ 		SwTable *pTable = (SwTable *)aIter.First( TYPE(SwTable) );
/*N*/ 		ASSERT( pTable, "Tabellen-Format ohne Tabelle" );
/*N*/ 		if( !pTable )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		if( !pExportInfo->pTblLineBoxFmts40 )
/*N*/ 			pExportInfo->pTblLineBoxFmts40 = new Sw3FrmFmts;
/*N*/ 		pTable->GetTabLines().ForEach( &lcl_sw3io_CollectLineFmts,
/*N*/ 										 pExportInfo->pTblLineBoxFmts40 );
/*N*/ 	}
/*N*/ }

/*  */

/*N*/ void Sw3IoImp::AddTblLineBoxFmt( SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	if( !pTblLineBoxFmts )
/*N*/ 		pTblLineBoxFmts = new Sw3FrmFmts;
/*N*/ 
/*N*/ 	pTblLineBoxFmts->Insert( pFmt, pTblLineBoxFmts->Count() );
/*N*/ }

/*N*/ USHORT Sw3IoImp::GetTblLineBoxFmtId( SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	USHORT nIdx = pTblLineBoxFmts ? pTblLineBoxFmts->GetPos( pFmt )
/*N*/ 								  : USHRT_MAX;
/*N*/ 	ASSERT( USHRT_MAX != nIdx, "Tabellen-Line/-Box-Format nicht gefunden" );
/*N*/ 	if( USHRT_MAX == nIdx )
/*N*/ 		Error();
/*N*/ 
/*N*/ 	return nIdx;
/*N*/ }
/*N*/ 
/*N*/ USHORT Sw3IoImp::GetTblLineBoxFmtStrPoolId40( SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	ASSERT( pExportInfo, "Wo ist die Export-Info?" );
/*N*/ 	USHORT nPos = pExportInfo->pTblLineBoxFmts40
/*N*/ 						? pExportInfo->pTblLineBoxFmts40->GetPos( pFmt )
/*N*/ 						: USHRT_MAX;
/*N*/ 	ASSERT( USHRT_MAX != nPos, "Tabellen-Line/-Box-Format nicht gefunden" );
/*N*/ 	if( USHRT_MAX == nPos )
/*N*/ 		Error();
/*N*/ 
/*N*/ 	return aStringPool.Find( *((*pExportInfo->pTblLineBoxFmtNames40)[nPos]),
/*N*/ 							 pFmt->GetPoolFmtId() );
/*N*/ }
/*N*/ 
/*N*/ SwFrmFmt *Sw3IoImp::GetTblLineBoxFmt( USHORT nIdx )
/*N*/ {
/*N*/ 	ASSERT( pTblLineBoxFmts && nIdx < pTblLineBoxFmts->Count(),
/*N*/ 			"Tabellen-Line/-Box-Format nicht gefunden" );
/*N*/ 	if( pTblLineBoxFmts && nIdx < pTblLineBoxFmts->Count() )
/*N*/ 		return (*pTblLineBoxFmts)[nIdx];
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

// Lesen einer Tabelle
// Der uebergebene Index zeigt anschliessend hinter die Tabelle
// Flag-Byte
// 	0x10 - Modified
// 	0x20 - HeadLineRepeat
// Anzahl Boxes
// Tabellen-Section-ID
// SWG_FRAMEFMT
// SWG_FIELDTYPE bei DDE-Tabelle
// x-mal SWG_TABLELINE


/*N*/ void Sw3IoImp::InTable( SwNodeIndex& rPos )
/*N*/ {
/*N*/ 	Sw3FrmFmts *pOldTblLineBoxFmts = pTblLineBoxFmts;
/*N*/ 	pTblLineBoxFmts = 0;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ULONG nOldCntntBox = nCntntBox;
/*N*/ #endif
/*N*/ 
/*N*/ 
/*N*/ 	// Bei einer Tabelle hoert eine 3.1-/4.0-Numerierung auf!
/*N*/ 	if( !IsVersion(SWG_LONGIDX) )
/*N*/ 		CloseNumRange40( rPos );
/*N*/ 
/*N*/ 	OpenRec( SWG_TABLE );
/*N*/ 	// Tabelle in Tabelle einfuegen wird nicht gemacht
/*N*/ 	if( !bInsert || !pDoc->IsIdxInTbl( rPos ) )
/*N*/ 	{
/*N*/ 		BYTE cFlags = OpenFlagRec();
/*N*/ 		UINT16 nBoxes, nTblIdDummy;
/*N*/ 		BYTE cChgMode;
/*N*/ 		*pStrm >> nBoxes;
/*N*/ 		if( IsVersion(SWG_LAYFRAMES,SWG_LONGIDX) )
/*N*/ 			*pStrm >> nTblIdDummy;
/*N*/ 		if( nVersion >= SWG_TBLCHGMODE )
/*N*/ 			*pStrm >> cChgMode;
/*N*/ 		CloseFlagRec();
/*N*/ //JP 16.02.99: ueberfluessiges Flag
/*N*/ //		BOOL bModified = BOOL( ( cFlags & 0x10 ) != 0 );
/*N*/ 		BOOL bHdRepeat = BOOL( ( cFlags & 0x20 ) != 0 );
/*N*/ 		// Die Strukturen im Nodes-Array erzeugen
/*N*/ 		// Erzeugen von:
/*N*/ 		// TableNode..StartNode..TxtNode..EndNode.. (weitere Boxes) ..EndNode
/*N*/ 		SwTableNode* pNd = pDoc->GetNodes().InsertTable( rPos, nBoxes,
/*N*/ 								(SwTxtFmtColl*) pDoc->GetDfltTxtFmtColl() );
/*N*/ 		if( pNd )
/*N*/ 		{
/*N*/ 			rPos = *pNd;
/*N*/ 			SwTable* pTbl = &pNd->GetTable();
/*N*/ //JP 16.02.99: ueberfluessiges Flag
/*N*/ //			if( bModified )
/*N*/ //				pTbl->SetModified();
/*N*/ 			pTbl->SetHeadlineRepeat( bHdRepeat );
/*N*/ 			if( nVersion >= SWG_TBLCHGMODE )
/*N*/ 				pTbl->SetTblChgMode( (TblChgMode)cChgMode );
/*N*/ 
/*N*/ 			// Das Frame-Format der Tabelle einlesen
/*N*/ 			SwFrmFmt* pFmt = pDoc->MakeTblFrmFmt( aEmptyStr, 0 );
/*N*/ 			if( Peek() == SWG_FRAMEFMT )
/*N*/ 				InFormat( SWG_FRAMEFMT, pFmt );
/*N*/ 			// Da einige FRMSIZE-Attribute zu grosse Werte enthalten,
/*N*/ 			// muessen die Werte heruntergeteilt werden.
/*N*/ 			nSizeDivFac = 1;
/*N*/ 			if( pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE, FALSE )
/*N*/ 				== SFX_ITEM_SET )
/*N*/ 			{
/*N*/ 				const SwFmtFrmSize &rSz = pFmt->GetFrmSize();
/*N*/ 				if ( rSz.GetHeight() > 65535L || rSz.GetWidth() > 65535L )
/*N*/ 				{
/*?*/ 					SwFmtFrmSize aSz( rSz );
/*?*/ 					SwTwips h = aSz.GetHeight();
/*?*/ 					SwTwips w = aSz.GetWidth();
/*?*/ 					while( h > 65535L || w > 65535L )
/*?*/ 						w /= 2, h /= 2, nSizeDivFac *= 2;
/*?*/ 					aSz.SetWidth( w );
/*?*/ 					aSz.SetHeight( h );
/*?*/ 					pFmt->SetAttr( aSz );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// Falls wir im Insert-Mode sind, muss der Name der Tabelle
/*N*/ 			// auf Eindeutigkeit ueberprueft werden
/*N*/ 			if( bInsert )
/*N*/ 			{
/*?*/ 				String aName( pFmt->GetName() );
/*?*/ 				Sw3StringPool::RemoveExtension( aName );
/*?*/ 				pFmt->SetName( aEmptyStr );
/*?*/ 				if( pDoc->FindTblFmtByName( aName ) )
/*?*/ 					pFmt->SetName( pDoc->GetUniqueTblName() );
/*?*/ 				else
/*?*/ 					pFmt->SetName( aName );
/*N*/ 			}
/*N*/ 			pFmt->Add( pTbl );
/*N*/ 			// Ist es eine DDE-Tabelle?
/*N*/ 			SwDDEFieldType* pDDEFldType = 0;
/*N*/ 			if( Peek() == SWG_FIELDTYPE )
/*?*/ 				pDDEFldType = (SwDDEFieldType*) InFieldType();
/*N*/ 
/*N*/ 			// Vordefinierte Layout-Frames loeschen
/*N*/ 			// beim Einfuegen stoeren diese zur Zeit
/*N*/ 			if( bInsert ) pNd->DelFrms();
/*N*/ 
/*N*/ 			// Redlines am Start- oder End-Node einlesen
/*N*/ 			while( Peek() == SWG_NODEREDLINE )
/*N*/ 			{
/*N*/ /*?*/ 				INT32 nDummy=0; //SW50.SDW
/*N*/						InNodeRedline( rPos, nDummy );
/*N*/ 			}
/*N*/ 
/*N*/ 			// Die einzelnen Zeilen einlesen
/*N*/ 			// aIdx zeigt auf den Startnode der ersten Box
/*N*/ 			rPos = *pNd; rPos++;
/*N*/ #ifdef DBG_UTIL
/*N*/ 			nCntntBox = 0;
/*N*/ #endif
/*N*/ 			USHORT nLine = 0;
/*N*/ 			while( BytesLeft() )
/*N*/ 				InTableLine( pTbl->GetTabLines(), NULL, nLine++, rPos );
/*N*/ 			rPos = pNd->EndOfSectionIndex()+1;
/*N*/ 
/*N*/ 			ASSERT( nCntntBox == nBoxes ,
/*N*/ 				"Anzahl der Boxen stimmt nicht mit den gelesenen ueberein" );
/*N*/ 
/*N*/ 			if( pDDEFldType && !pNd->GetTable().IsTblComplex() )
/*N*/ 			{
/*N*/ 				//DDETabelle, dann tausche am Node den Tabellen-Pointer aus
                    SwDDETable* pNewTable = new SwDDETable( pNd->GetTable(),
/*N*/                                                       pDDEFldType );
/*?*/               pNd->SetNewTable( pNewTable, FALSE );
/*N*/ 			}
/*N*/ 
/*N*/ 			// Layout-Frames wieder erzeugen, falls eingefuegt
/*N*/ 			// und falls (MA #$.!) die Tbl nicht im FlyFrm ist
/*N*/ 			if( bInsert && !nFlyLevel )
/*N*/ 			{
/*?*/ 				SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
/*?*/ 				pDoc->GetNodes().GoNext( &aIdx );
/*?*/ 				pNd->MakeFrms( &aIdx );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else Error();		// kein TblNode
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	CloseRec( SWG_TABLE );
/*N*/ 	// Den Divisionsfaktor fur FRMSIZE wieder zuruecksetzen
/*N*/ 	nSizeDivFac = 1;
/*N*/ 
/*N*/ 	delete pTblLineBoxFmts;
/*N*/ 	pTblLineBoxFmts = pOldTblLineBoxFmts;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	nCntntBox = nOldCntntBox;
/*N*/ #endif
/*N*/ }

// Schreiben einer Tabelle


/*N*/ void Sw3IoImp::OutTable( const SwTableNode& rNd )
/*N*/ {
/*N*/ 	Sw3FrmFmts *pOldTblLineBoxFmts = pTblLineBoxFmts;
/*N*/ 	pTblLineBoxFmts = 0;
/*N*/ 
/*N*/ 	const SwTable& rTbl = rNd.GetTable();
/*N*/ 	const SwTableLines& rLines = rTbl.GetTabLines();
/*N*/ 	USHORT nLines = rLines.Count();
/*N*/ 	SwTable* pOldTbl = pCurTbl;
/*N*/ 	pCurTbl = (SwTable*) &rTbl;
/*N*/ 	// 0x04 - Anzahl Boxes, Tabellen-ID
/*N*/ 	// 0x10 - Modified
/*N*/ 	// 0x20 - HeadLineRepeat
/*N*/ 	BYTE   cFlags;
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 		cFlags = IsSw31Export() ? 0x04 : 0x05;
/*N*/ 	else
/*N*/ 		cFlags = 0x03;
/*N*/ 	UINT16 nBoxes = 0;
/*N*/ //JP 16.02.99: ueberflussiges Flag
/*N*/ //	if( rTbl.IsModified() )
/*N*/ //		cFlags |= 0x10;
/*N*/ 	if( rTbl.IsHeadlineRepeat() )
/*N*/ 		cFlags |= 0x20;
/*N*/ 	OpenRec( SWG_TABLE );
/*N*/ 	*pStrm << (BYTE) cFlags;
/*N*/ 	OpenValuePos16( nBoxes );
/*N*/ 
/*N*/ 	// Im 3.1/4.0-Format stand hier der Index der Section. Bei IDX_NO_VALUE
/*N*/ 	// wurde sie beim Laden nicht registriert.
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 		*pStrm << (UINT16)IDX_NO_VALUE;
/*N*/ 	if( !IsSw31Export() )
/*N*/ 		*pStrm << (BYTE)rTbl.GetTblChgMode();
/*N*/ 	OutFormat( SWG_FRAMEFMT, *rTbl.GetFrmFmt() );
/*N*/ 	// DDE-Tabelle? Dann den DDE-Feldtyp speichern
/*N*/ 	if( IS_TYPE(SwDDETable, &rTbl) )
/*N*/ 	{
/*?*/       SwDDETable* pDDE = (SwDDETable*) &rTbl;
/*?*/       OutFieldType( *pDDE->GetDDEFldType() );
/*N*/ 	}
/*N*/ 	if( !IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		OutNodeRedlines( rNd.GetIndex() );
/*N*/ 		OutNodeRedlines( rNd.EndOfSectionIndex() );
/*N*/ 	}
/*N*/ 
/*N*/ 	for( USHORT i = 0; i < nLines && Good(); i++ )
/*N*/ 		nBoxes += OutTableLine( *(rLines[ i ]) );
/*N*/ 	CloseValuePos16( nBoxes );
/*N*/ 	CloseRec( SWG_TABLE );
/*N*/ 	pCurTbl = pOldTbl;
/*N*/ 	aStat.nTbl++;
/*N*/ 
/*N*/ 	delete pTblLineBoxFmts;
/*N*/ 	pTblLineBoxFmts = pOldTblLineBoxFmts;
/*N*/ }

/**/


// Einlesen einer Zeile
// Flag-Byte (0)
//				0x10 - schon immer unbenutzt. Zur Zeit nicht benutzt,
//					   damit die anderen Flags wie bei Boxen sind.
//				0x20 - FrmFmtId ist vorhanden (seit SW5)
//				0x40 - FrmFmt ist geshared (seit SW5)
// String-ID des Frameformats
// Anzahl Boxes
// SWG_FRAMEFMT (optional)
// x-mal SWG_TABLEBOX


/*N*/ void Sw3IoImp::InTableLine
/*N*/ 	( SwTableLines& rLines, SwTableBox* pUpper, USHORT nPos, SwNodeIndex& rPos )
/*N*/ {
/*N*/ 	OpenRec( SWG_TABLELINE );
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	UINT16 nBoxes, nFmtId = IDX_NO_VALUE;
/*N*/ 	if( !IsVersion(SWG_LONGIDX) || (cFlags & 0x20) != 0 )
/*N*/ 		*pStrm >> nFmtId;
/*N*/ 	*pStrm >> nBoxes;
/*N*/ 	CloseFlagRec();
/*N*/ 	SwTableLineFmt* pFmt = 0;
/*N*/ 	if( Peek() == SWG_FRAMEFMT )
/*N*/ 	{
/*N*/ 		pFmt = (SwTableLineFmt*) InFormat( SWG_FRAMEFMT, pDoc->MakeTableLineFmt() );
/*N*/ 		lcl_sw3io_AdjustFrmSize( pFmt, nSizeDivFac );
/*N*/ 		if( (cFlags & 0x40) != 0 )
/*N*/ 			AddTblLineBoxFmt( pFmt );
/*N*/ 	}
/*N*/ 	else if( (cFlags & 0x20) != 0 )
/*N*/ 	{
/*N*/ 		pFmt = (SwTableLineFmt *)GetTblLineBoxFmt( nFmtId );
/*N*/ 	}
/*N*/ 	else if( !IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		// Ja, das funktioniert. Das Format steht zwar in keinem
/*N*/ 		// Array mehr, aber es ist im String-Pool gecached.
/*N*/ 		pFmt = (SwTableLineFmt *)FindFmt( nFmtId, SWG_FRAMEFMT );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFmt )
/*N*/ 	{
/*N*/ 		// Default-Werte setzen:
/*N*/ 		SwTableLine* pLine = new SwTableLine( pFmt, nBoxes, pUpper );
/*N*/ 		rLines.C40_INSERT( SwTableLine, pLine, nPos );
/*N*/ 		SwTableBoxes& rBoxes = pLine->GetTabBoxes();
/*N*/ 		USHORT nBox = 0;
/*N*/ 		while( BytesLeft() )
/*N*/ 			InTableBox( rBoxes, nBox++, pLine, rPos );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( pFmt, "Line-Format fehlt" );
/*N*/ 		Error(); 	// kein FrameFmt
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_TABLELINE );
/*N*/ }
/*N*/ 
/*N*/ BOOL lcl_sw3io_IsLineFmtShared( SwFrmFmt& rFmt,	const SwTableLine& rLine )
/*N*/ {
/*N*/ 	SwClientIter aIter( rFmt );
/*N*/ 
/*N*/ 	SwClient* pLast;
/*N*/ 
/*N*/ 	for( pLast = aIter.First( TYPE( SwTableLine ));
/*N*/ 		 pLast && pLast == (SwClient *)&rLine;
/*N*/ 		 pLast = aIter.Next() )
/*N*/ 		;
/*N*/ 
/*N*/ 	return pLast != 0;
/*N*/ }

// Schreiben einer Tabellenzeile. Red Returnwert ist die Summe
// aller in der Zeile enthaltenen Zellen.

/*N*/ USHORT Sw3IoImp::OutTableLine( const SwTableLine& rLine )
/*N*/ {
/*N*/ 	USHORT nTotalBoxes = 0;
/*N*/ 	const SwTableBoxes& rBoxes = rLine.GetTabBoxes();
/*N*/ 	UINT16 nBoxes = rBoxes.Count();
/*N*/ 
/*N*/ 	// 0x10: Format-Id ist enthalten (nur SW5)
/*N*/ 	// 0x20: Format ist geshared (nur SW5)
/*N*/ 	BYTE cFlags;
/*N*/ 
/*N*/ 	SwFrmFmt* pFmt = rLine.GetFrmFmt();
/*N*/ 	UINT16 nFmtId;
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		cFlags = 0x04;		// keine Flags, Fmt-Id, Anzahl Boxen
/*N*/ 		nFmtId = GetTblLineBoxFmtStrPoolId40( pFmt );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		cFlags = 0x02; // Anzahl Boxen
/*N*/ 		if( lcl_sw3io_IsLineFmtShared( *pFmt, rLine ) )
/*N*/ 		{
/*N*/ 			cFlags += 0x40;		// Shared-Flag
/*N*/ 			if( pFmt->IsWritten() )
/*N*/ 			{
/*N*/ 				cFlags += 0x22; // FmtId-Flag + FmtId
/*N*/ 				nFmtId = GetTblLineBoxFmtId( pFmt );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				AddTblLineBoxFmt( pFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	OpenRec( SWG_TABLELINE );
/*N*/ 	*pStrm << (BYTE) cFlags;
/*N*/ 	if( IsSw31Or40Export() || (cFlags & 0x20) != 0 )
/*N*/ 		*pStrm << nFmtId;
/*N*/ 
/*N*/ 	*pStrm << nBoxes;
/*N*/ 
/*N*/ 	// OutFormat schreibt nur noch nicht geschriebene Formate raus,
/*N*/ 	// deshalb muss das hier nicht ueberprueft werden.
/*N*/ 	OutFormat( SWG_FRAMEFMT, *pFmt );
/*N*/ 
/*N*/ 	for( USHORT i = 0; i < nBoxes && Good(); i++ )
/*N*/ 		nTotalBoxes += OutTableBox( *( rBoxes[ i ]) );
/*N*/ 	CloseRec( SWG_TABLELINE );
/*N*/ 	return nTotalBoxes;
/*N*/ 
/*N*/ }

/**/


// Einlesen einer Zelle
// BYTE			Flag-Byte
//				0x10 - Zelle enthaelt weitere Zeilen
//				0x20 - Zellen enthaelt leeren Contentbereich (nie exportiert)
//				0x20 - FrmFmtId ist vorhanden (seit SW5)
//				0x40 - FrmFmt ist geshared (seit SW5)
// UINT16		String-ID des Frameformats
// UINT16		Anzahl Zeilen oder Section-ID (opt)
// SWG_FRAMEFMT (optional)
// SWG_CONTENTS	Inhalt oder
// SWG_TABLELINE (optional, mehrfach)


/*N*/ void Sw3IoImp::InTableBox
/*N*/ 	 (SwTableBoxes& rBoxes, USHORT nPos,	// Array und Index
/*N*/ 	  SwTableLine* pUpper,					// uebergeordnete Zeile
/*N*/ 	  SwNodeIndex& rPos )					// Start-Index der Section
/*N*/ {
/*N*/ 	OpenRec( SWG_TABLEBOX );
/*N*/ 	BYTE   cFlags = OpenFlagRec();
/*N*/ 	UINT16 nFmtId = IDX_NO_VALUE, nLines = 0;
/*N*/ 	if( !IsVersion(SWG_LONGIDX) || (cFlags & 0x20) != 0 )
/*N*/ 		*pStrm >> nFmtId;
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		*pStrm >> nLines;
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	// Frame-Format evtl. einlesen
/*N*/ 	SwTableBoxFmt* pFmt = NULL;
/*N*/ 	if( Peek() == SWG_FRAMEFMT )
/*N*/ 	{
/*N*/ 		pFmt = (SwTableBoxFmt*) InFormat( SWG_FRAMEFMT, pDoc->MakeTableBoxFmt() );
/*N*/ 		lcl_sw3io_AdjustFrmSize( pFmt, nSizeDivFac );
/*N*/ 		if( (cFlags & 0x40) != 0 )
/*N*/ 			AddTblLineBoxFmt( pFmt );
/*N*/ 	}
/*N*/ 	else if( (cFlags & 0x20) != 0 )
/*N*/ 	{
/*N*/ 		ASSERT( IsVersion(SWG_LONGIDX), "0x20 doch schon frueher benutzt?" );
/*N*/ 		pFmt = (SwTableBoxFmt *)GetTblLineBoxFmt( nFmtId );
/*N*/ 	}
/*N*/ 	else if( !IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		// Ja, das funktioniert. Das Format steht zwar in keinem
/*N*/ 		// Array mehr, aber es ist im String-Pool gecached.
/*N*/ 		pFmt = (SwTableBoxFmt *)FindFmt( nFmtId, SWG_FRAMEFMT );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFmt )
/*N*/ 	{
/*N*/ 		SwTableBox* pBox;
/*N*/ 		if( Peek() == SWG_CONTENTS )
/*N*/ 		{
/*N*/ 			pBox = new SwTableBox( pFmt, rPos, pUpper );
/*N*/ 			InContents( rPos );
/*N*/ 
/*N*/ 			// JP 12.09.97 - Bug 41223:
/*N*/ 			// falls an der International Einstellung gedreht wurde, so muss
/*N*/ 			// jetzt die entsprechende Aktualisierung erfolgen.
/*N*/ 			pBox->ChgByLanguageSystem();
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			++nCntntBox;
/*N*/ #endif
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pBox = new SwTableBox( pFmt, nLines, pUpper );
/*N*/ 		rBoxes.C40_INSERT( SwTableBox, pBox, nPos );
/*N*/ 		USHORT nBoxPos = 0;
/*N*/ 		if( !BytesLeft() )
/*N*/ 		{
/*N*/ 			// JP 05.05.97: Bug 39569 - Zelle ohne StartNode und ohne Lines
/*N*/ 			if( !pBox->GetSttNd() )
/*N*/ 			{
/*?*/ 				//JP 19.06.98: erst die neue Box erzeugen, damit diese sich
/*?*/ 				//			als abhaengig ins Format eintraegt; sonst wird
/*?*/ 				//			das Format im BoxDTOR geloescht !!!
/*?*/ 				SwTableBox* pNewBox = new SwTableBox( pFmt, rPos, pUpper );
/*?*/ 				rBoxes.C40_REPLACE( SwTableBox, pNewBox, nPos );
/*?*/ 				delete pBox;
/*?*/ 				pBox = pNewBox;
/*?*/ 				rPos = pBox->GetSttNd()->EndOfSectionIndex() + 1;
/*?*/ #ifdef DBG_UTIL
/*?*/ 				ASSERT( !this, "Tabellenzelle ohne Lines und ohne Content" );
/*?*/ 				++nCntntBox;
/*?*/ #endif
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			while( BytesLeft() )
/*N*/ 				InTableLine( pBox->GetTabLines(), pBox, nBoxPos++, rPos );
/*N*/ 
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( pFmt, "Box-Format fehlt" );
/*N*/ 		Error(); 	// kein FrmFmt
/*N*/ 	}
/*N*/ 	CloseRec( SWG_TABLEBOX );
/*N*/ }

/*N*/ BOOL lcl_sw3io_IsBoxFmtShared( SwFrmFmt& rFmt, const SwTableBox& rBox )
/*N*/ {
/*N*/ 	SwClientIter aIter( rFmt );
/*N*/ 
/*N*/ 	SwClient* pLast;
/*N*/ 
/*N*/ 	for( pLast = aIter.First( TYPE( SwTableBox ));
/*N*/ 		 pLast && pLast == (SwClient *)&rBox;
/*N*/ 		 pLast = aIter.Next() )
/*N*/ 		;
/*N*/ 
/*N*/ 	return pLast != 0;
/*N*/ }
/*N*/ 
/*N*/ USHORT Sw3IoImp::OutTableBox( const SwTableBox& rBox )
/*N*/ {
/*N*/ 	const SwTableLines& rLines = rBox.GetTabLines();
/*N*/ 	USHORT nLines = rLines.Count();
/*N*/ 	USHORT nTotalBoxes = 0;
/*N*/ 
/*N*/ 	BYTE cFlags;
/*N*/ 
/*N*/ 	UINT16 nFmtId;
/*N*/ 	SwFrmFmt* pFmt = rBox.GetFrmFmt();
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		cFlags = 0x02;		// keine Flags, FmtId
/*N*/ 		nFmtId = GetTblLineBoxFmtStrPoolId40( pFmt );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		cFlags = 0x00; 		// keine Flags, keine Daten
/*N*/ 		if( lcl_sw3io_IsBoxFmtShared( *pFmt, rBox ) )
/*N*/ 		{
/*N*/ 			cFlags += 0x40;	// Format ist geshared
/*N*/ 			if( pFmt->IsWritten() )
/*N*/ 			{
/*N*/ 				cFlags += 0x22; // FmtId-Flag + FmtId
/*N*/ 				nFmtId = GetTblLineBoxFmtId( pFmt );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				AddTblLineBoxFmt( pFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nLines )
/*N*/ 		cFlags += 0x12;
/*N*/ 	OpenRec( SWG_TABLEBOX );
/*N*/ 	*pStrm << (BYTE)   cFlags;
/*N*/ 	if( IsSw31Or40Export() || (cFlags & 0x20) != 0 )
/*N*/ 		*pStrm << (UINT16) nFmtId;
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		*pStrm << (UINT16) nLines;
/*N*/ 	OutFormat( SWG_FRAMEFMT, *pFmt );
/*N*/ 	if( rBox.GetSttNd() )
/*N*/ 		// Inhalt ausgeben
/*N*/ 		OutContents( SwNodeIndex( *rBox.GetSttNd() ) );
/*N*/ 	if( nLines )
/*N*/ 	{
/*N*/ 		// Falls Zeilen vorhanden, diese ausgeben
/*N*/ 		for( USHORT i = 0; i < nLines; i++)
/*N*/ 			nTotalBoxes += OutTableLine( *( rLines[ i ]) );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nTotalBoxes++;
/*N*/ 	CloseRec( SWG_TABLEBOX );
/*N*/ 	return nTotalBoxes;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
