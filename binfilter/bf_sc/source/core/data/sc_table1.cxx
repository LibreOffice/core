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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

//------------------------------------------------------------------------

#ifdef WIN

// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//#define _SI_HXX
//#define SI_NODRW
#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW		  //
#define _SI_NOCONTROL
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
#define _VCATTR_HXX


#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

#endif	//WIN

// INCLUDE ---------------------------------------------------------------

#include <unotools/textsearch.hxx>
#include <bf_sfx2/objsh.hxx>

#include "cell.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "globstr.hrc"
#include "refupdat.hxx"
#include "markdata.hxx"
#include "progress.hxx"
#include "hints.hxx"		// fuer Paint-Broadcast
#include "prnsave.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

extern BOOL bIsOlk, bOderSo;

// -----------------------------------------------------------------------

/*N*/ ScTable::ScTable( ScDocument* pDoc, USHORT nNewTab, const String& rNewName,
/*N*/ 					BOOL bColInfo, BOOL bRowInfo ) :
/*N*/ 	pDocument( pDoc ),
/*N*/ 	aName( rNewName ),
/*N*/ 	nTab( nNewTab ),
/*N*/ 	bScenario( FALSE ),
/*N*/ 	bActiveScenario( FALSE ),
/*N*/ 	nScenarioFlags( 0 ),
/*N*/ 	aScenarioColor( COL_LIGHTGRAY ),
/*N*/ 	nLinkMode( 0 ),
/*N*/ 	pColWidth( NULL ),
/*N*/ 	pColFlags( NULL ),
/*N*/ 	pRowHeight( NULL ),
/*N*/ 	pRowFlags( NULL ),
/*N*/ 	pOutlineTable( NULL ),
/*N*/ 	bVisible( TRUE ),
/*N*/ 	pSearchParam( NULL ),
/*N*/ 	pSearchText ( NULL ),
/*N*/ 	bProtected( FALSE ),
/*N*/ 	nRecalcLvl( 0 ),
/*N*/ 	bPageSizeValid( FALSE ),
/*N*/ 	nRepeatStartX( REPEAT_NONE ),
/*N*/ 	nRepeatStartY( REPEAT_NONE ),
/*N*/ 	aPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) ),
/*N*/ 	bTableAreaValid( FALSE ),
/*N*/ 	nPrintRangeCount( 0 ),
/*N*/ 	pPrintRanges( NULL ),
/*N*/ 	pRepeatColRange( NULL ),
/*N*/ 	pRepeatRowRange( NULL ),
/*N*/ 	nLockCount( 0 ),
/*N*/ 	pScenarioRanges( NULL ),
/*N*/ 	pSortCollator( NULL )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	if (bColInfo)
/*N*/ 	{
/*N*/ 		pColWidth  = new USHORT[ MAXCOL+1 ];
/*N*/ 		pColFlags  = new BYTE[ MAXCOL+1 ];
/*N*/ 
/*N*/ 		for (i=0; i<=MAXCOL; i++)
/*N*/ 		{
/*N*/ 			pColWidth[i] = STD_COL_WIDTH;
/*N*/ 			pColFlags[i] = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bRowInfo)
/*N*/ 	{
/*N*/ 		pRowHeight = new USHORT[ MAXROW+1 ];
/*N*/ 		pRowFlags  = new BYTE[ MAXROW+1 ];
/*N*/ 
/*N*/ 		for (i=0; i<=MAXROW; i++)
/*N*/ 		{
/*N*/ 			pRowHeight[i] = ScGlobal::nStdRowHeight;
/*N*/ 			pRowFlags[i] = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 	if (pDrawLayer)
/*N*/ 	{
/*N*/ 		pDrawLayer->ScAddPage( nTab );
/*N*/ 		pDrawLayer->ScRenamePage( nTab, aName );
/*N*/ 		ULONG nx = (ULONG) ((double) (MAXCOL+1) * STD_COL_WIDTH			  * HMM_PER_TWIPS );
/*N*/ 		ULONG ny = (ULONG) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
/*N*/ 		pDrawLayer->SetPageSize( nTab, Size( nx, ny ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	for (i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].Init( i, nTab, pDocument );
/*N*/ }

/*N*/ ScTable::~ScTable()
/*N*/ {
/*N*/ 	if (!pDocument->IsInDtorClear())
/*N*/ 	{
/*?*/ 		//	nicht im dtor die Pages in der falschen Reihenfolge loeschen
/*?*/ 		//	(nTab stimmt dann als Page-Number nicht!)
/*?*/ 		//	In ScDocument::Clear wird hinterher per Clear am DrawLayer alles geloescht.
/*?*/ 
/*?*/ 		ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*?*/ 		if (pDrawLayer)
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			pDrawLayer->ScRemovePage( nTab );
/*N*/ 	}
/*N*/ 
/*N*/ 	delete[] pColWidth;
/*N*/ 	delete[] pRowHeight;
/*N*/ 	delete[] pColFlags;
/*N*/ 	delete[] pRowFlags;
/*N*/ 	delete pOutlineTable;
/*N*/ 	delete pSearchParam;
/*N*/ 	delete pSearchText;
/*N*/ 	delete[] pPrintRanges;
/*N*/ 	delete pRepeatColRange;
/*N*/ 	delete pRepeatRowRange;
/*N*/ 	delete pScenarioRanges;
/*N*/ 	DestroySortCollator();
/*N*/ }

/*N*/ void ScTable::GetName( String& rName ) const
/*N*/ {
/*N*/ 	rName = aName;
/*N*/ }

/*N*/ void ScTable::SetName( const String& rNewName )
/*N*/ {
/*N*/ 	String aMd( "D\344umling", RTL_TEXTENCODING_MS_1252 );	// ANSI
/*N*/ 	if( rNewName == aMd )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 bIsOlk = bOderSo = TRUE;
/*N*/ 	aName = rNewName;
/*N*/ }

/*N*/ void ScTable::SetVisible( BOOL bVis )
/*N*/ {
/*N*/ 	bVisible = bVis;
/*N*/ }

/*N*/ void ScTable::SetScenario( BOOL bFlag )
/*N*/ {
/*N*/ 	bScenario = bFlag;
/*N*/ }

/*N*/ void ScTable::SetLink( BYTE nMode,
/*N*/ 						const String& rDoc, const String& rFlt, const String& rOpt,
/*N*/ 						const String& rTab, ULONG nRefreshDelay )
/*N*/ {
/*N*/ 	nLinkMode = nMode;
/*N*/ 	aLinkDoc = rDoc;		// Datei
/*N*/ 	aLinkFlt = rFlt;		// Filter
/*N*/ 	aLinkOpt = rOpt;		// Filter-Optionen
/*N*/ 	aLinkTab = rTab;		// Tabellenname in Quelldatei
/*N*/ 	nLinkRefreshDelay = nRefreshDelay;	// refresh delay in seconds, 0==off
/*N*/ }

/*N*/ USHORT ScTable::GetOptimalColWidth( USHORT nCol, OutputDevice* pDev,
/*N*/ 									double nPPTX, double nPPTY,
/*N*/ 									const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 									BOOL bFormula, const ScMarkData* pMarkData,
/*N*/ 									BOOL bSimpleTextImport )
/*N*/ {
/*N*/ 	return aCol[nCol].GetOptimalColWidth( pDev, nPPTX, nPPTY, rZoomX, rZoomY,
/*N*/ 		bFormula, STD_COL_WIDTH - STD_EXTRA_WIDTH, pMarkData, bSimpleTextImport );
/*N*/ }


/*N*/ BOOL ScTable::SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nExtra,
/*N*/ 								OutputDevice* pDev,
/*N*/ 								double nPPTX, double nPPTY,
/*N*/ 								const Fraction& rZoomX, const Fraction& rZoomY,
/*N*/ 								BOOL bForce )
/*N*/ {
/*N*/ 	DBG_ASSERT( nExtra==0 || bForce, "autom. OptimalHeight mit Extra" );
/*N*/ 
/*N*/ 	BOOL    bChanged = FALSE;
/*N*/ 	USHORT  nCount = nEndRow-nStartRow+1;
/*N*/ 
/*N*/ 	ScProgress* pProgress = NULL;
/*N*/ 	if ( nCount > 1 )
/*N*/ 		pProgress = new ScProgress( pDocument->GetDocumentShell(),
/*N*/ 							ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), GetWeightedCount() );
/*N*/ 
/*N*/ 	USHORT* pHeight = new USHORT[nCount];                   // Twips !
/*N*/     memset( pHeight, 0, sizeof(USHORT) * nCount );
/*N*/ 
/*N*/ 	//	zuerst einmal ueber den ganzen Bereich
/*N*/ 	//	(mit der letzten Spalte in der Hoffnung, dass die am ehesten noch auf
/*N*/ 	//	 Standard formatiert ist)
/*N*/ 
/*N*/ 	aCol[MAXCOL].GetOptimalHeight(
/*N*/ 			nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce, 0, 0 );
/*N*/ 
/*N*/ 	//	daraus Standardhoehe suchen, die im unteren Bereich gilt
/*N*/ 
/*N*/ 	USHORT nMinHeight = pHeight[nCount-1];
/*N*/ 	USHORT nPos = nCount-1;
/*N*/ 	while ( nPos && pHeight[nPos-1] >= nMinHeight )
/*N*/ 		--nPos;
/*N*/ 	USHORT nMinStart = nStartRow + nPos;
/*N*/ 
/*N*/ 	long nWeightedCount = 0;
/*N*/ 	for (USHORT nCol=0; nCol<MAXCOL; nCol++)		// MAXCOL schon oben
/*N*/ 	{
/*N*/ 		aCol[nCol].GetOptimalHeight(
/*N*/ 			nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
/*N*/ 			nMinHeight, nMinStart );
/*N*/ 
/*N*/ 		if (pProgress)
/*N*/ 		{
/*N*/ 			long nWeight = aCol[nCol].GetWeightedCount();
/*N*/ 			if (nWeight)		// nochmal denselben Status muss auch nicht sein
/*N*/ 			{
/*N*/ 				nWeightedCount += nWeight;
/*N*/ 				pProgress->SetState( nWeightedCount );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nRngStart;
/*N*/ 	USHORT nRngEnd;
/*N*/ 	USHORT nLast = 0;
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		if ( (pRowFlags[nStartRow+i] & CR_MANUALSIZE) == 0 || bForce )
/*N*/ 		{
/*N*/ 			if (nExtra)
/*N*/ 				pRowFlags[nStartRow+i] |= CR_MANUALSIZE;
/*N*/ 			else
/*N*/ 				pRowFlags[nStartRow+i] &= ~CR_MANUALSIZE;
/*N*/ 
/*N*/ 			if (nLast)
/*N*/ 			{
/*N*/ 				if (pHeight[i]+nExtra == nLast)
/*N*/ 					nRngEnd = nStartRow+i;
/*N*/ 				else
/*N*/ 				{
/*N*/ 					bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
/*N*/ 					nLast = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (!nLast)
/*N*/ 			{
/*N*/ 				nLast = pHeight[i]+nExtra;
/*N*/ 				nRngStart = nStartRow+i;
/*N*/ 				nRngEnd = nStartRow+i;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (nLast)
/*N*/ 				bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
/*N*/ 			nLast = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (nLast)
/*N*/ 		bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
/*N*/ 
/*N*/ 	delete[] pHeight;
/*N*/ 	delete pProgress;
/*N*/ 
/*N*/ 	return bChanged;
/*N*/ }

/*N*/ BOOL ScTable::GetCellArea( USHORT& rEndCol, USHORT& rEndRow ) const
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT nMaxX = 0;
/*N*/ 	USHORT nMaxY = 0;
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		if (!aCol[i].IsEmptyVisData(TRUE))		// TRUE = Notizen zaehlen auch
/*N*/ 		{
/*N*/ 			bFound = TRUE;
/*N*/ 			nMaxX = i;
/*N*/ 			USHORT nColY = aCol[i].GetLastVisDataPos(TRUE);
/*N*/ 			if (nColY > nMaxY)
/*N*/ 				nMaxY = nColY;
/*N*/ 		}
/*N*/ 
/*N*/ 	rEndCol = nMaxX;
/*N*/ 	rEndRow = nMaxY;
/*N*/ 	return bFound;
/*N*/ }

/*N*/ BOOL ScTable::GetTableArea( USHORT& rEndCol, USHORT& rEndRow ) const
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;				//! merken?
/*N*/ 	if (!bTableAreaValid)
/*N*/ 	{
/*N*/ 		bRet = GetPrintArea( ((ScTable*)this)->nTableAreaX,
/*N*/ 								((ScTable*)this)->nTableAreaY, TRUE );
/*N*/ 		((ScTable*)this)->bTableAreaValid = TRUE;
/*N*/ 	}
/*N*/ 	rEndCol = nTableAreaX;
/*N*/ 	rEndRow = nTableAreaY;
/*N*/ 	return bRet;
/*N*/ }

/*		vorher:

    BOOL bFound = FALSE;
    USHORT nMaxX = 0;
    USHORT nMaxY = 0;
    for (USHORT i=0; i<=MAXCOL; i++)
        if (!aCol[i].IsEmpty())
        {
            bFound = TRUE;
            nMaxX = i;
            USHORT nColY = aCol[i].GetLastEntryPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
*/

/*N*/ BOOL ScTable::GetPrintArea( USHORT& rEndCol, USHORT& rEndRow, BOOL bNotes ) const
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT nMaxX = 0;
/*N*/ 	USHORT nMaxY = 0;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for (i=0; i<=MAXCOL; i++)				// Attribute testen
/*N*/ 	{
/*N*/ 		USHORT nFirstRow,nLastRow;
/*N*/ 		if (aCol[i].HasVisibleAttr( nFirstRow,nLastRow, FALSE ))
/*N*/ 		{
/*N*/ 			bFound = TRUE;
/*N*/ 			nMaxX = i;
/*N*/ 			if (nLastRow > nMaxY)
/*N*/ 				nMaxY = nLastRow;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nMaxX == MAXCOL)					// Attribute rechts weglassen
/*N*/ 	{
/*N*/ 		--nMaxX;
/*N*/ 		while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1]) )
/*N*/ 			--nMaxX;
/*N*/ 	}
/*N*/ 
/*N*/ 	for (i=0; i<=MAXCOL; i++)				// Daten testen
/*N*/ 		if (!aCol[i].IsEmptyVisData(bNotes))
/*N*/ 		{
/*N*/ 			bFound = TRUE;
/*N*/ 			if (i>nMaxX)
/*N*/ 				nMaxX = i;
/*N*/ 			USHORT nColY = aCol[i].GetLastVisDataPos(bNotes);
/*N*/ 			if (nColY > nMaxY)
/*N*/ 				nMaxY = nColY;
/*N*/ 		}
/*N*/ 
/*N*/ 	rEndCol = nMaxX;
/*N*/ 	rEndRow = nMaxY;
/*N*/ 	return bFound;
/*N*/ }



/*N*/ BOOL ScTable::GetDataStart( USHORT& rStartCol, USHORT& rStartRow ) const
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT nMinX = MAXCOL;
/*N*/ 	USHORT nMinY = MAXROW;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for (i=0; i<=MAXCOL; i++)					// Attribute testen
/*N*/ 	{
/*N*/ 		USHORT nFirstRow,nLastRow;
/*N*/ 		if (aCol[i].HasVisibleAttr( nFirstRow,nLastRow, TRUE ))
/*N*/ 		{
/*N*/ 			if (!bFound)
/*N*/ 				nMinX = i;
/*N*/ 			bFound = TRUE;
/*N*/ 			if (nFirstRow < nMinY)
/*N*/ 				nMinY = nFirstRow;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nMinX == 0)										// Attribute links weglassen
/*N*/ 	{
/*N*/ 		if ( aCol[0].IsVisibleAttrEqual(aCol[1]) )		// keine einzelnen
/*N*/ 		{
/*?*/ 			++nMinX;
/*?*/ 			while ( nMinX<MAXCOL && aCol[nMinX].IsVisibleAttrEqual(aCol[nMinX-1]) )
/*?*/ 				++nMinX;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bDatFound = FALSE;
/*N*/ 	for (i=0; i<=MAXCOL; i++)					// Daten testen
/*N*/ 		if (!aCol[i].IsEmptyVisData(TRUE))
/*N*/ 		{
/*N*/ 			if (!bDatFound && i<nMinX)
/*N*/ 				nMinX = i;
/*N*/ 			bFound = bDatFound = TRUE;
/*N*/ 			USHORT nColY = aCol[i].GetFirstVisDataPos(TRUE);
/*N*/ 			if (nColY < nMinY)
/*N*/ 				nMinY = nColY;
/*N*/ 		}
/*N*/ 
/*N*/ 	rStartCol = nMinX;
/*N*/ 	rStartRow = nMinY;
/*N*/ 	return bFound;
/*N*/ }

/*N*/ void ScTable::GetDataArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rEndCol, USHORT& rEndRow,
/*N*/ 							BOOL bIncludeOld )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	BOOL bLeft       = FALSE;
/*N*/ }






/*N*/ void ScTable::GetNextPos( USHORT& rCol, USHORT& rRow, short nMovX, short nMovY,
/*N*/ 								BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if (bUnprotected && !IsProtected())		// Tabelle ueberhaupt geschuetzt?
/*N*/ }

/*N*/ BOOL ScTable::GetNextMarkedCell( USHORT& rCol, USHORT& rRow, const ScMarkData& rMark )
/*N*/ {
/*N*/ 	const ScMarkArray* pMarkArray = rMark.GetArray();
/*N*/ 	DBG_ASSERT(pMarkArray,"GetNextMarkedCell ohne MarkArray");
/*N*/ 	if ( !pMarkArray )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	++rRow;					// naechste Zelle ist gesucht
/*N*/ 
/*N*/ 	while ( rCol <= MAXCOL )
/*N*/ 	{
/*N*/ 		const ScMarkArray& rArray = pMarkArray[rCol];
/*N*/ 		while ( rRow <= MAXROW )
/*N*/ 		{
/*N*/ 			USHORT nStart = (USHORT) rArray.GetNextMarked( (short) rRow, FALSE );
/*N*/ 			if ( nStart <= MAXROW )
/*N*/ 			{
/*N*/ 				USHORT nEnd = rArray.GetMarkEnd( nStart, FALSE );
/*N*/ 				ScColumnIterator aColIter( &aCol[rCol], nStart, nEnd );
/*N*/ 				USHORT nCellRow;
/*N*/ 				ScBaseCell* pCell = NULL;
/*N*/ 				while ( aColIter.Next( nCellRow, pCell ) )
/*N*/ 				{
/*N*/ 					if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
/*N*/ 					{
/*N*/ 						rRow = nCellRow;
/*N*/ 						return TRUE;			// Zelle gefunden
/*N*/ 					}
/*N*/ 				}
/*N*/ 				rRow = nEnd + 1;				// naechsten markierten Bereich suchen
/*N*/ 			}
/*N*/ 			else
/*N*/ 				rRow = MAXROW + 1;				// Ende der Spalte
/*N*/ 		}
/*N*/ 		rRow = 0;
/*N*/ 		++rCol;									// naechste Spalte testen
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;								// alle Spalten durch
/*N*/ }

/*N*/ void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 									USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 									short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )		// only within the table
/*N*/ 	{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 	}
/*N*/ }

/*N*/ void ScTable::UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 					 USHORT nCol2, USHORT nRow2, USHORT nTab2, short nDx, short nDy, short nDz,
/*N*/ 					 ScDocument* pUndoDoc, BOOL bIncludeDraw )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	USHORT iMax;
/*N*/ 	if ( eUpdateRefMode == URM_COPY )
/*N*/ 	{
/*N*/ 		i = nCol1;
/*N*/ 		iMax = nCol2;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		i = 0;
/*N*/ 		iMax = MAXCOL;
/*N*/ 	}
/*N*/ 	for ( ; i<=iMax; i++)
/*N*/ 		aCol[i].UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
/*N*/ 									nDx, nDy, nDz, pUndoDoc );
/*N*/ 
/*N*/ 	if ( bIncludeDraw )
/*N*/ 		UpdateDrawRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
/*N*/ 
/*N*/ 	if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )		// print ranges: only within the table
/*N*/ 	{
/*?*/ 		USHORT nSTab,nETab,nSCol,nSRow,nECol,nERow;
/*?*/ 		BOOL bRecalcPages = FALSE;
/*?*/ 
/*?*/ 		if ( pPrintRanges && nPrintRangeCount )
/*?*/ 			for ( i=0; i<nPrintRangeCount; i++ )
/*?*/ 			{
/*?*/ 				nSTab = nETab = pPrintRanges[i].aStart.Tab();
/*?*/ 				nSCol = pPrintRanges[i].aStart.Col();
/*?*/ 				nSRow = pPrintRanges[i].aStart.Row();
/*?*/ 				nECol = pPrintRanges[i].aEnd.Col();
/*?*/ 				nERow = pPrintRanges[i].aEnd.Row();
/*?*/ 
/*?*/ 				if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
/*?*/ 										  nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
/*?*/ 										  nDx,nDy,nDz,
/*?*/ 										  nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
/*?*/ 				{
/*?*/ 					pPrintRanges[i] = ScRange( nSCol, nSRow, nSTab, nECol, nERow, nSTab );
/*?*/ 					bRecalcPages = TRUE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 
/*?*/ 		if ( pRepeatColRange )
/*?*/ 		{
/*?*/ 			nSTab = nETab = pRepeatColRange->aStart.Tab();
/*?*/ 			nSCol = pRepeatColRange->aStart.Col();
/*?*/ 			nSRow = pRepeatColRange->aStart.Row();
/*?*/ 			nECol = pRepeatColRange->aEnd.Col();
/*?*/ 			nERow = pRepeatColRange->aEnd.Row();
/*?*/ 
/*?*/ 			if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
/*?*/ 									  nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
/*?*/ 									  nDx,nDy,nDz,
/*?*/ 									  nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
/*?*/ 			{
/*?*/ 				*pRepeatColRange = ScRange( nSCol, nSRow, nSTab, nECol, nERow, nSTab );
/*?*/ 				bRecalcPages = TRUE;
/*?*/ 				nRepeatStartX = nSCol;	// fuer UpdatePageBreaks
/*?*/ 				nRepeatEndX = nECol;
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/ 		if ( pRepeatRowRange )
/*?*/ 		{
/*?*/ 			nSTab = nETab = pRepeatRowRange->aStart.Tab();
/*?*/ 			nSCol = pRepeatRowRange->aStart.Col();
/*?*/ 			nSRow = pRepeatRowRange->aStart.Row();
/*?*/ 			nECol = pRepeatRowRange->aEnd.Col();
/*?*/ 			nERow = pRepeatRowRange->aEnd.Row();
/*?*/ 
/*?*/ 			if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
/*?*/ 									  nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
/*?*/ 									  nDx,nDy,nDz,
/*?*/ 									  nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
/*?*/ 			{
/*?*/ 				*pRepeatRowRange = ScRange( nSCol, nSRow, nSTab, nECol, nERow, nSTab );
/*?*/ 				bRecalcPages = TRUE;
/*?*/ 				nRepeatStartY = nSRow;	// fuer UpdatePageBreaks
/*?*/ 				nRepeatEndY = nERow;
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/ 		//	updating print ranges is not necessary with multiple print ranges
/*?*/ 		if ( bRecalcPages && GetPrintRangeCount() <= 1 )
/*?*/ 		{
/*?*/ 			UpdatePageBreaks(NULL);
/*?*/ 
/*?*/ 			SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
/*?*/ 			if (pDocSh)
/*?*/ 				pDocSh->Broadcast( ScPaintHint(
/*?*/ 									ScRange(0,0,nTab,MAXCOL,MAXROW,nTab),
/*?*/ 									PAINT_GRID ) );
/*?*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ void ScTable::UpdateInsertTab(USHORT nTable)
/*N*/ {
/*N*/ 	if (nTab >= nTable) nTab++;
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTab(nTable);
/*N*/ }


/*N*/  void ScTable::UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScTable* pRefUndo )
/*N*/  {
/*N*/  	if (nTab > nTable) nTab--;
/*N*/  
/*N*/  	USHORT i;
/*N*/  	if (pRefUndo)
/*N*/  		for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, &pRefUndo->aCol[i]);
/*N*/  	else
/*N*/  		for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, NULL);
/*N*/  }

/*N*/ void ScTable::UpdateCompile( BOOL bForceIfNameInUse )
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++)
/*N*/ 	{
/*N*/ 		aCol[i].UpdateCompile( bForceIfNameInUse );
/*N*/ 	}
/*N*/ }

/*N*/ void ScTable::SetTabNo(USHORT nNewTab)
/*N*/ {
/*N*/ 	nTab = nNewTab;
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++) aCol[i].SetTabNo(nNewTab);
/*N*/ }

/*N*/ BOOL ScTable::IsRangeNameInUse(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                               USHORT nIndex) const
/*N*/ {
/*N*/ 	BOOL bInUse = FALSE;
/*N*/ 	for (USHORT i = nCol1; !bInUse && (i <= nCol2) && (i <= MAXCOL); i++)
/*N*/ 		bInUse = aCol[i].IsRangeNameInUse(nRow1, nRow2, nIndex);
/*N*/ 	return bInUse;
/*N*/ }

/*N*/ void ScTable::ReplaceRangeNamesInUse(USHORT nCol1, USHORT nRow1,
/*N*/ 								USHORT nCol2, USHORT nRow2,
/*N*/ 									const ScIndexMap& rMap )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	for (USHORT i = nCol1; i <= nCol2 && (i <= MAXCOL); i++)
/*N*/ }

/*N*/ void ScTable::ExtendPrintArea( OutputDevice* pDev,
/*N*/ 					USHORT nStartCol, USHORT nStartRow, USHORT& rEndCol, USHORT nEndRow )
/*N*/ {
/*N*/ 	if ( !pColFlags || !pRowFlags )
/*N*/ 	{
/*N*/ 		DBG_ERROR("keine ColInfo oder RowInfo in ExtendPrintArea");
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
/*N*/ 	double nPPTX = aPix1000.X() / 1000.0;
/*N*/ 	double nPPTY = aPix1000.Y() / 1000.0;
/*N*/ 
/*N*/ 	BOOL bEmpty[MAXCOL+1];
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		bEmpty[i] = ( aCol[i].GetCellCount() == 0 );
/*N*/ 
/*N*/ 	USHORT nIndex;
/*N*/ 	USHORT nPrintCol = rEndCol;
/*N*/ 	for (USHORT nRow = nStartRow; nRow<=nEndRow; nRow++)
/*N*/ 	{
/*N*/ 		if ( ( pRowFlags[nRow] & CR_HIDDEN ) == 0 )
/*N*/ 		{
/*N*/ 			USHORT nDataCol = rEndCol;
/*N*/ 			while (nDataCol > 0 && ( bEmpty[nDataCol] || !aCol[nDataCol].Search(nRow,nIndex) ) )
/*N*/ 				--nDataCol;
/*N*/ 			if ( ( pColFlags[nDataCol] & CR_HIDDEN ) == 0 )
/*N*/ 			{
/*N*/ 				ScBaseCell* pCell = aCol[nDataCol].GetCell(nRow);
/*N*/ 				if (pCell)
/*N*/ 				{
/*N*/ 					CellType eType = pCell->GetCellType();
/*N*/ 					if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT
/*N*/ 						|| (eType == CELLTYPE_FORMULA && !((ScFormulaCell*)pCell)->IsValue()) )
/*N*/ 					{
/*N*/ 						BOOL bFormula = FALSE;	//! uebergeben
/*N*/ 						long nPixel = pCell->GetTextWidth();
/*N*/ 
/*N*/ 						// Breite bereits im Idle-Handler berechnet?
/*N*/ 						if ( TEXTWIDTH_DIRTY == nPixel )
/*N*/ 						{
/*N*/ 							ScNeededSizeOptions aOptions;
/*N*/ 							aOptions.bTotalSize	 = TRUE;
/*N*/ 							aOptions.bFormula	 = bFormula;
/*N*/ 							aOptions.bSkipMerged = FALSE;
/*N*/ 
/*N*/ 							Fraction aZoom(1,1);
/*N*/ 							nPixel = aCol[nDataCol].GetNeededSize( nRow,
/*N*/ 														pDev,nPPTX,nPPTY,aZoom,aZoom,
/*N*/ 														TRUE, aOptions );
/*N*/ 							pCell->SetTextWidth( (USHORT)nPixel );
/*N*/ 						}
/*N*/ 
/*N*/ 						long nTwips = (long) (nPixel / nPPTX);
/*N*/ 						long nDocW = GetColWidth( nDataCol );
/*N*/ 						USHORT nCol = nDataCol;
/*N*/ 						while (nTwips > nDocW && nCol < MAXCOL)
/*N*/ 						{
/*N*/ 							++nCol;
/*N*/ 							nDocW += GetColWidth( nCol );
/*N*/ 						}
/*N*/ 						if (nCol>nPrintCol)
/*N*/ 							nPrintCol = nCol;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rEndCol = nPrintCol;
/*N*/ }

/*N*/ void ScTable::DoColResize( USHORT nCol1, USHORT nCol2, USHORT nAdd )
/*N*/ {
/*N*/ 	for (USHORT nCol=nCol1; nCol<=nCol2; nCol++)
/*N*/ 		aCol[nCol].Resize(aCol[nCol].GetCellCount() + nAdd);
/*N*/ }

/*N*/ #define SET_PRINTRANGE( p1, p2 ) \
/*N*/ 	if ( (p2) )								\
/*N*/ 	{										\
/*N*/ 		if ( (p1) )							\
/*N*/ 			*(p1) = *(p2);					\
/*N*/ 		else								\
/*N*/ 			(p1) = new ScRange( *(p2) );	\
/*N*/ 	}										\
/*N*/ 	else									\
/*N*/ 		DELETEZ( (p1) )

/*N*/ void ScTable::SetRepeatColRange( const ScRange* pNew )
/*N*/ {
/*N*/ 	SET_PRINTRANGE( pRepeatColRange, pNew );
/*N*/ }

/*N*/ void ScTable::SetRepeatRowRange( const ScRange* pNew )
/*N*/ {
/*N*/ 	SET_PRINTRANGE( pRepeatRowRange, pNew );
/*N*/ }

// #42845# zeroptimiert
#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif
/*N*/ void ScTable::SetPrintRangeCount( USHORT nNew )
/*N*/ {
/*N*/ 	ScRange* pNewRanges;
/*N*/ 	if (nNew)
/*N*/ 		pNewRanges = new ScRange[nNew];
/*N*/ 	else
/*N*/ 		pNewRanges = NULL;
/*N*/ 
/*N*/ 	if ( pPrintRanges && nNew >= nPrintRangeCount )		//	Anzahl vergroessert?
/*?*/ 		for ( USHORT i=0; i<nPrintRangeCount; i++ )		//	(fuer "Hinzufuegen")
/*?*/ 			pNewRanges[i] = pPrintRanges[i];			//	alte Ranges kopieren
/*N*/ 
/*N*/ 	delete[] pPrintRanges;
/*N*/ 	pPrintRanges = pNewRanges;
/*N*/ 	nPrintRangeCount = nNew;
/*N*/ }
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif

/*N*/ void ScTable::SetPrintRange( USHORT nPos, const ScRange& rNew )
/*N*/ {
/*N*/ 	if (nPos < nPrintRangeCount && pPrintRanges)
/*N*/ 		pPrintRanges[nPos] = rNew;
/*N*/ 	else
/*N*/ 		DBG_ERROR("SetPrintRange falsch");
/*N*/ }

/*N*/ const ScRange* ScTable::GetPrintRange(USHORT nPos) const
/*N*/ {
/*N*/ 	if (nPos < nPrintRangeCount && pPrintRanges)
/*N*/ 		return pPrintRanges+nPos;
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ void ScTable::FillPrintSaver( ScPrintSaverTab& rSaveTab ) const
/*N*/ {
/*N*/ 	rSaveTab.SetAreas( nPrintRangeCount, pPrintRanges );
/*N*/ 	rSaveTab.SetRepeat( pRepeatColRange, pRepeatRowRange );
/*N*/ }






}
