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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <tools/urlobj.hxx>
#include <math.h>
#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <bf_svtools/PasswordHelper.hxx>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "attarray.hxx"		// Iterator
#include "markdata.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "conditio.hxx"
#include "chartlis.hxx"
#include "globstr.hrc"
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

void lcl_LoadRange( SvStream& rStream, ScRange** ppRange );
void lcl_SaveRange( SvStream& rStream, ScRange* pRange );



/*N*/ BOOL ScTable::SetOutlineTable( const ScOutlineTable* pNewOutline )
/*N*/ {
/*N*/ 	USHORT nOldSizeX = 0;
/*N*/ 	USHORT nOldSizeY = 0;
/*N*/ 	USHORT nNewSizeX = 0;
/*N*/ 	USHORT nNewSizeY = 0;
/*N*/ 
/*N*/ 	if (pOutlineTable)
/*N*/ 	{
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		nOldSizeX = pOutlineTable->GetColArray()->GetDepth();
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pNewOutline)
/*N*/ 	{
/*N*/ 		pOutlineTable = new ScOutlineTable( *pNewOutline );
/*N*/ 		nNewSizeX = pOutlineTable->GetColArray()->GetDepth();
/*N*/ 		nNewSizeY = pOutlineTable->GetRowArray()->GetDepth();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pOutlineTable = NULL;
/*N*/ 
/*N*/ 	return ( nNewSizeX != nOldSizeX || nNewSizeY != nOldSizeY );		// Groesse geaendert ?
/*N*/ }


/*N*/ void ScTable::StartOutlineTable()
/*N*/ {
/*N*/ 	if (!pOutlineTable)
/*N*/ 		pOutlineTable = new ScOutlineTable;
/*N*/ }


/*N*/ BOOL ScTable::TestInsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nSize )
/*N*/ {
/*N*/ 	BOOL bTest = TRUE;
/*N*/ 
/*N*/ 	if ( nStartCol==0 && nEndCol==MAXCOL && pOutlineTable )
/*N*/ 		bTest = pOutlineTable->TestInsertRow(nSize);
/*N*/ 
/*N*/ 	for (USHORT i=nStartCol; (i<=nEndCol) && bTest; i++)
/*N*/ 		bTest = aCol[i].TestInsertRow( nSize );
/*N*/ 
/*N*/ 	return bTest;
/*N*/ }


/*N*/ void ScTable::InsertRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	nRecalcLvl++;
/*N*/ 	if (nStartCol==0 && nEndCol==MAXCOL)
/*N*/ 	{
/*N*/ 		if (pRowHeight && pRowFlags)
/*N*/ 		{
/*N*/             memmove( &pRowHeight[nStartRow+nSize], &pRowHeight[nStartRow],
/*N*/                     (MAXROW - nStartRow + 1 - nSize) * sizeof(pRowHeight[0]) );
/*N*/             memmove( &pRowFlags[nStartRow+nSize], &pRowFlags[nStartRow],
/*N*/                     (MAXROW - nStartRow + 1 - nSize) * sizeof(pRowFlags[0]) );
/*N*/ 
/*N*/ 			//	#67451# copy row height from row above
/*N*/ 			USHORT nSourceRow = ( nStartRow > 0 ) ? ( nStartRow - 1 ) : 0;
/*N*/ 			BYTE nNewFlags = pRowFlags[nSourceRow] & CR_MANUALSIZE;
/*N*/ 			USHORT nNewHeight = pRowHeight[nSourceRow];
/*N*/ 			for (i=nStartRow; i<nStartRow+nSize; i++)
/*N*/ 			{
/*N*/ 				pRowHeight[i] = nNewHeight;
/*N*/ 				pRowFlags[i] = nNewFlags;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (pOutlineTable)
/*N*/ 			pOutlineTable->InsertRow( nStartRow, nSize );
/*N*/ 	}
/*N*/ 
/*N*/ 	for (i=nStartCol; i<=nEndCol; i++)
/*N*/ 		aCol[i].InsertRow( nStartRow, nSize );
/*N*/ 	if( !--nRecalcLvl )
/*N*/ 		SetDrawPageSize();
/*N*/ }


/*N*/ void ScTable::DeleteRow( USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nSize,
/*N*/ 							BOOL* pUndoOutline )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	nRecalcLvl++;
/*N*/ 	if (nStartCol==0 && nEndCol==MAXCOL)
/*N*/ 	{
/*N*/ 		if (pRowHeight && pRowFlags)
/*N*/         {
/*N*/             memmove( &pRowHeight[nStartRow], &pRowHeight[nStartRow+nSize],
/*N*/                     (MAXROW - nStartRow + 1 - nSize) * sizeof(pRowHeight[0]) );
/*N*/             memmove( &pRowFlags[nStartRow], &pRowFlags[nStartRow+nSize],
/*N*/                     (MAXROW - nStartRow + 1 - nSize) * sizeof(pRowFlags[0]) );
/*N*/         }
/*N*/ 		if (pOutlineTable)
/*N*/ 			if (pOutlineTable->DeleteRow( nStartRow, nSize ))
/*N*/ 				if (pUndoOutline)
/*N*/ 					*pUndoOutline = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	for (i=nStartCol; i<=nEndCol; i++)
/*N*/ 		aCol[i].DeleteRow( nStartRow, nSize );
/*N*/ 	if( !--nRecalcLvl )
/*N*/ 		SetDrawPageSize();
/*N*/ }


/*N*/ BOOL ScTable::TestInsertCol( USHORT nStartRow, USHORT nEndRow, USHORT nSize )
/*N*/ {
/*N*/ 	BOOL bTest = TRUE;
/*N*/ 
/*N*/ 	if ( nStartRow==0 && nEndRow==MAXROW && pOutlineTable )
/*N*/ 		bTest = pOutlineTable->TestInsertCol(nSize);
/*N*/ 
/*N*/ 	if ( nSize > MAXCOL )
/*N*/ 		bTest = FALSE;
/*N*/ 
/*N*/ 	for (USHORT i=MAXCOL; (i+nSize>MAXCOL) && bTest; i--)
/*N*/ 		bTest = aCol[i].TestInsertCol(nStartRow, nEndRow);
/*N*/ 
/*N*/ 	return bTest;
/*N*/ }


/*N*/ void ScTable::InsertCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	nRecalcLvl++;
/*N*/ 	if (nStartRow==0 && nEndRow==MAXROW)
/*N*/ 	{
/*N*/ 		if (pColWidth && pColFlags)
/*N*/         {
/*N*/             memmove( &pColWidth[nStartCol+nSize], &pColWidth[nStartCol],
/*N*/                     (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColWidth[0]) );
/*N*/             memmove( &pColFlags[nStartCol+nSize], &pColFlags[nStartCol],
/*N*/                     (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColFlags[0]) );
/*N*/         }
/*N*/ 		if (pOutlineTable)
/*N*/ 			pOutlineTable->InsertCol( nStartCol, nSize );
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	if ((nStartRow == 0) && (nEndRow == MAXROW))
/*N*/ 	{
/*N*/ 		for (i=0; i < nSize; i++)
/*N*/ 			for (USHORT nCol = MAXCOL; nCol > nStartCol; nCol--)
/*N*/ 				aCol[nCol].SwapCol(aCol[nCol-1]);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (i=0; i+nSize+nStartCol <= MAXCOL; i++)
/*N*/ 			aCol[MAXCOL - nSize - i].MoveTo(nStartRow, nEndRow, aCol[MAXCOL - i]);
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nStartCol>0)						// copy old attributes
/*N*/ 	{
/*N*/ 		USHORT nWhichArray[2];
/*N*/ 		nWhichArray[0] = ATTR_MERGE;
/*N*/ 		nWhichArray[1] = 0;
/*N*/ 
/*N*/ 		for (i=0; i<nSize; i++)
/*N*/ 		{
/*N*/ 			aCol[nStartCol-1].CopyToColumn( nStartRow, nEndRow, IDF_ATTRIB,
/*N*/ 												FALSE, aCol[nStartCol+i] );
/*N*/ 			aCol[nStartCol+i].RemoveFlags( nStartRow, nEndRow,
/*N*/ 												SC_MF_HOR | SC_MF_VER | SC_MF_AUTO );
/*N*/ 			aCol[nStartCol+i].ClearItems( nStartRow, nEndRow, nWhichArray );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !--nRecalcLvl )
/*N*/ 		SetDrawPageSize();
/*N*/ }


/*N*/ void ScTable::DeleteCol( USHORT nStartCol, USHORT nStartRow, USHORT nEndRow, USHORT nSize,
/*N*/ 							BOOL* pUndoOutline )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	nRecalcLvl++;
/*N*/ 	if (nStartRow==0 && nEndRow==MAXROW)
/*N*/ 	{
/*N*/ 		if (pColWidth && pColFlags)
/*N*/         {
/*N*/             memmove( &pColWidth[nStartCol], &pColWidth[nStartCol+nSize],
/*N*/                     (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColWidth[0]) );
/*N*/             memmove( &pColFlags[nStartCol], &pColFlags[nStartCol+nSize],
/*N*/                     (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColFlags[0]) );
/*N*/         }
/*N*/ 		if (pOutlineTable)
/*N*/ 			if (pOutlineTable->DeleteCol( nStartCol, nSize ))
/*N*/ 				if (pUndoOutline)
/*N*/ 					*pUndoOutline = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	for (i = 0; i < nSize; i++)
/*N*/ 		aCol[nStartCol + i].DeleteArea(nStartRow, nEndRow, IDF_ALL);
/*N*/ 
/*N*/ 	if ((nStartRow == 0) && (nEndRow == MAXROW))
/*N*/ 	{
/*N*/ 		for (i=0; i < nSize; i++)
/*N*/ 			for (USHORT nCol = nStartCol; nCol < MAXCOL; nCol++)
/*N*/ 				aCol[nCol].SwapCol(aCol[nCol+1]);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for (i=0; i+nSize+nStartCol <= MAXCOL; i++)
/*N*/ 			aCol[nStartCol + nSize + i].MoveTo(nStartRow, nEndRow, aCol[nStartCol + i]);
/*N*/ 	}
/*N*/ 	if( !--nRecalcLvl )
/*N*/ 		SetDrawPageSize();
/*N*/ }


/*N*/ void ScTable::DeleteArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nDelFlag)
/*N*/ {
/*N*/ 	if (nCol2 > MAXCOL) nCol2 = MAXCOL;
/*N*/ 	if (nRow2 > MAXROW) nRow2 = MAXROW;
/*N*/ 	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
/*N*/ 	{
/*N*/ //		nRecalcLvl++;
/*N*/ 
/*N*/ 		for (USHORT i = nCol1; i <= nCol2; i++)
/*N*/ 			aCol[i].DeleteArea(nRow1, nRow2, nDelFlag);
/*N*/ 
/*N*/ 			//
/*N*/ 			// Zellschutz auf geschuetzter Tabelle nicht setzen
/*N*/ 			//
/*N*/ 
/*N*/ 		if ( bProtected && (nDelFlag & IDF_ATTRIB) )
/*N*/ 		{
/*N*/ 			ScPatternAttr aPattern(pDocument->GetPool());
/*N*/ 			aPattern.GetItemSet().Put( ScProtectionAttr( FALSE ) );
/*N*/ 			ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
/*N*/ 		}
/*N*/ 
/*		if( !--nRecalcLvl )
            SetDrawPageSize();
*/
/*N*/ 	}
/*N*/ }




//	pTable = Clipboard






//	Markierung von diesem Dokument




/*N*/ void ScTable::StartAllListeners()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].StartAllListeners();
/*N*/ }


/*N*/ void ScTable::StartNameListeners( BOOL bOnlyRelNames )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/         aCol[i].StartNameListeners( bOnlyRelNames );
/*N*/ }






/*N*/ void ScTable::CopyToTable(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
/*N*/ 							USHORT nFlags, BOOL bMarked, ScTable* pDestTab,
/*N*/ 							const ScMarkData* pMarkData,
/*N*/ 							BOOL bAsLink, BOOL bColRowFlags)
/*N*/ {
/*N*/ 	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
/*N*/ 	{
/*N*/ 		USHORT i;
/*N*/ 
/*N*/ 		if (nFlags)
/*N*/ 			for (i = nCol1; i <= nCol2; i++)
/*N*/ 				aCol[i].CopyToColumn(nRow1, nRow2, nFlags, bMarked,
/*N*/ 								pDestTab->aCol[i], pMarkData, bAsLink);
/*N*/ 
/*N*/ 		if (bColRowFlags)		// Spaltenbreiten/Zeilenhoehen/Flags
/*N*/ 		{
/*N*/ 			//	Charts muessen beim Ein-/Ausblenden angepasst werden
/*N*/ 			ScChartListenerCollection* pCharts = pDestTab->pDocument->GetChartListenerCollection();
/*N*/ 			if ( pCharts && !pCharts->GetCount() )
/*N*/ 				pCharts = NULL;
/*N*/ 
/*N*/ 			if (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth)
/*N*/ 				for (i=nCol1; i<=nCol2; i++)
/*N*/ 				{
/*N*/ 					BOOL bChange = pCharts &&
/*N*/ 						( pDestTab->pColFlags[i] & CR_HIDDEN ) != ( pColFlags[i] & CR_HIDDEN );
/*N*/ 					pDestTab->pColWidth[i] = pColWidth[i];
/*N*/ 					pDestTab->pColFlags[i] = pColFlags[i];
/*N*/ 					//!	Aenderungen zusammenfassen?
/*N*/ 					if (bChange)
/*?*/ 					{	DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pCharts->SetRangeDirty(ScRange( i, 0, nTab, i, MAXROW, nTab ));
/*N*/ 				}
/*N*/ 
/*N*/ 			if (nCol1==0 && nCol2==MAXCOL && pRowHeight && pDestTab->pRowHeight)
/*N*/ 				for (i=nRow1; i<=nRow2; i++)
/*N*/ 				{
/*N*/ 					BOOL bChange = pCharts &&
/*N*/ 						( pDestTab->pRowFlags[i] & CR_HIDDEN ) != ( pRowFlags[i] & CR_HIDDEN );
/*N*/ 					pDestTab->pRowHeight[i] = pRowHeight[i];
/*N*/ 					pDestTab->pRowFlags[i]  = pRowFlags[i];
/*N*/ 					//!	Aenderungen zusammenfassen?
/*N*/ 					if (bChange)
/*?*/ 					{	DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pCharts->SetRangeDirty(ScRange( 0, i, nTab, MAXCOL, i, nTab ));
/*N*/ 				}
/*N*/ 
/*N*/ 			pDestTab->SetOutlineTable( pOutlineTable );		// auch nur wenn bColRowFlags
/*N*/ 		}
/*N*/ 	}
/*N*/ }







/*N*/ void ScTable::MarkScenarioIn( ScMarkData& rDestMark, USHORT nNeededBits ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( bScenario, "bScenario == FALSE" );
/*N*/ 
/*N*/ 	if ( ( nScenarioFlags & nNeededBits ) != nNeededBits )	// alle Bits gesetzt?
/*N*/ 		return;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].MarkScenarioIn( rDestMark );
/*N*/ }


/*N*/ void ScTable::InvalidateScenarioRanges()
/*N*/ {
/*N*/ 	delete pScenarioRanges;
/*N*/ 	pScenarioRanges = NULL;
/*N*/ }

/*N*/ const ScRangeList* ScTable::GetScenarioRanges() const
/*N*/ {
/*N*/ 	DBG_ASSERT( bScenario, "bScenario == FALSE" );
/*N*/ 
/*N*/ 	if (!pScenarioRanges)
/*N*/ 	{
/*N*/ 		((ScTable*)this)->pScenarioRanges = new ScRangeList;
/*N*/ 		ScMarkData aMark;
/*N*/ 		MarkScenarioIn( aMark, 0 );		// immer
/*N*/ 		aMark.FillRangeListWithMarks( pScenarioRanges, FALSE );
/*N*/ 	}
/*N*/ 	return pScenarioRanges;
/*N*/ }


/*N*/ void ScTable::PutCell( USHORT nCol, USHORT nRow, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 	{
/*N*/ 		if (pCell)
/*N*/ 			aCol[nCol].Insert( nRow, pCell );
/*N*/ 		else
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			aCol[nCol].Delete( nRow );
/*N*/ 	}
/*N*/ }


/*N*/ void ScTable::PutCell( USHORT nCol, USHORT nRow, ULONG nFormatIndex, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 	{
/*N*/ 		if (pCell)
/*N*/ 			aCol[nCol].Insert( nRow, nFormatIndex, pCell );
/*N*/ 		else
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			aCol[nCol].Delete( nRow );
/*N*/ 	}
/*N*/ }


/*N*/ void ScTable::PutCell( const ScAddress& rPos, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	if (pCell)
/*N*/ 		aCol[rPos.Col()].Insert( rPos.Row(), pCell );
/*N*/ 	else
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		aCol[rPos.Col()].Delete( rPos.Row() );
/*N*/ }


/*N*/ BOOL ScTable::SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].SetString( nRow, nTab, rString );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ void ScTable::SetValue( USHORT nCol, USHORT nRow, const double& rVal )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol, nRow))
/*N*/ 		aCol[nCol].SetValue( nRow, rVal );
/*N*/ }


/*N*/ void ScTable::SetNote( USHORT nCol, USHORT nRow, const ScPostIt& rNote)
/*N*/ {
/*N*/ 	if (ValidColRow(nCol, nRow))
/*N*/ 		aCol[nCol].SetNote(nRow, rNote);
/*N*/ }


/*N*/ void ScTable::GetString( USHORT nCol, USHORT nRow, String& rString )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		aCol[nCol].GetString( nRow, rString );
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }


/*N*/ void ScTable::GetInputString( USHORT nCol, USHORT nRow, String& rString )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		aCol[nCol].GetInputString( nRow, rString );
/*N*/ 	else
/*N*/ 		rString.Erase();
/*N*/ }

/*N*/ double ScTable::GetValue( USHORT nCol, USHORT nRow )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return 0.0;//STRIP001 if (ValidColRow( nCol, nRow ))
/*N*/ }

/*N*/ BOOL ScTable::GetNote( USHORT nCol, USHORT nRow, ScPostIt& rNote)
/*N*/ {
/*N*/ 	BOOL bHasNote = FALSE;
/*N*/ 
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		bHasNote = aCol[nCol].GetNote( nRow, rNote );
/*N*/ 	else
/*?*/ 		rNote.Clear();
/*N*/ 
/*N*/ 	return bHasNote;
/*N*/ }


/*N*/ CellType ScTable::GetCellType( USHORT nCol, USHORT nRow ) const
/*N*/ {
/*N*/ 	if (ValidColRow( nCol, nRow ))
/*N*/ 		return aCol[nCol].GetCellType( nRow );
/*N*/ 	return CELLTYPE_NONE;
/*N*/ }


/*N*/ ScBaseCell* ScTable::GetCell( USHORT nCol, USHORT nRow ) const
/*N*/ {
/*N*/ 	if (ValidColRow( nCol, nRow ))
/*N*/ 		return aCol[nCol].GetCell( nRow );
/*N*/ 
/*N*/ 	DBG_ERROR("GetCell ausserhalb");
/*N*/ 	return NULL;
/*N*/ }




/*N*/ BOOL ScTable::HasData( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].HasDataAt( nRow );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ BOOL ScTable::HasStringData( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].HasStringData( nRow );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ BOOL ScTable::HasValueData( USHORT nCol, USHORT nRow )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].HasValueData( nRow );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ void ScTable::SetDirtyVar()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].SetDirtyVar();
/*N*/ }


/*N*/ void ScTable::SetDirty()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].SetDirty();
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScTable::SetDirty( const ScRange& rRange )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ }


/*N*/  void ScTable::SetTableOpDirty( const ScRange& rRange )
/*N*/  {
/*N*/  	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/  	pDocument->SetAutoCalc( FALSE );	// no multiple recalculation
/*N*/  	USHORT nCol2 = rRange.aEnd.Col();
/*N*/  	for (USHORT i=rRange.aStart.Col(); i<=nCol2; i++)
/*N*/  		aCol[i].SetTableOpDirty( rRange );
/*N*/  	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/  }


/*N*/ void ScTable::SetDirtyAfterLoad()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].SetDirtyAfterLoad();
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScTable::SetRelNameDirty()
/*N*/ {
/*N*/ 	BOOL bOldAutoCalc = pDocument->GetAutoCalc();
/*N*/ 	pDocument->SetAutoCalc( FALSE );	// Mehrfachberechnungen vermeiden
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].SetRelNameDirty();
/*N*/ 	pDocument->SetAutoCalc( bOldAutoCalc );
/*N*/ }


/*N*/ void ScTable::CalcAll()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++) aCol[i].CalcAll();
/*N*/ }


/*N*/ void ScTable::CompileAll()
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++) aCol[i].CompileAll();
/*N*/ }


/*N*/ void ScTable::CompileXML( ScProgress& rProgress )
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++)
/*N*/ 	{
/*N*/ 		aCol[i].CompileXML( rProgress );
/*N*/ 	}
/*N*/ }


/*N*/ void ScTable::CalcAfterLoad()
/*N*/ {
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++) aCol[i].CalcAfterLoad();
/*N*/ }


/*N*/ void ScTable::ResetChanged( const ScRange& rRange )
/*N*/ {
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 
/*N*/ 	for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
/*N*/ 		aCol[nCol].ResetChanged(nStartRow, nEndRow);
/*N*/ }

//	Attribute

/*N*/ const SfxPoolItem* ScTable::GetAttr( USHORT nCol, USHORT nRow, USHORT nWhich ) const
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].GetAttr( nRow, nWhich );
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }


/*N*/ ULONG ScTable::GetNumberFormat( USHORT nCol, USHORT nRow ) const
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].GetNumberFormat( nRow );
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }


/*N*/ const ScPatternAttr* ScTable::GetPattern( USHORT nCol, USHORT nRow ) const
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		return aCol[nCol].GetPattern( nRow );
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }


/*N*/ BOOL ScTable::HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, USHORT nMask ) const
/*N*/ {
/*N*/ 	BOOL bFound=FALSE;
/*N*/ 	for (USHORT i=nCol1; i<=nCol2 && !bFound; i++)
/*N*/ 		bFound |= aCol[i].HasAttrib( nRow1, nRow2, nMask );
/*N*/ 	return bFound;
/*N*/ }






/*N*/ BOOL ScTable::ExtendMerge( USHORT nStartCol, USHORT nStartRow,
/*N*/ 						   USHORT& rEndCol, USHORT& rEndRow,
/*N*/ 						   BOOL bRefresh, BOOL bAttrs )
/*N*/ {
/*N*/ 	BOOL bFound=FALSE;
/*N*/ 	USHORT nOldEndX = rEndCol;
/*N*/ 	USHORT nOldEndY = rEndRow;
/*N*/ 	for (USHORT i=nStartCol; i<=nOldEndX; i++)
/*N*/ 		bFound |= aCol[i].ExtendMerge( i, nStartRow, nOldEndY, rEndCol, rEndRow, bRefresh, bAttrs );
/*N*/ 	return bFound;
/*N*/ }

/*N*/ BOOL ScTable::HasBlockMatrixFragment( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) const
/*N*/ {
/*N*/ 	// nix:0, mitte:1, unten:2, links:4, oben:8, rechts:16, offen:32
/*N*/ 	USHORT nEdges;
/*N*/ 
/*N*/ 	if ( nCol1 == nCol2 )
/*N*/ 	{	// linke und rechte Spalte
/*N*/ 		const USHORT n = 4 | 16;
/*N*/ 		nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, n );
/*N*/ 		// nicht (4 und 16) oder 1 oder 32
/*N*/ 		if ( nEdges && (((nEdges & n) != n) || (nEdges & 33)) )
/*N*/ 			return TRUE;		// linke oder rechte Kante fehlt oder offen
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	// linke Spalte
/*N*/ 		nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, 4 );
/*N*/ 		// nicht 4 oder 1 oder 32
/*N*/ 		if ( nEdges && (((nEdges & 4) != 4) || (nEdges & 33)) )
/*N*/ 			return TRUE;		// linke Kante fehlt oder offen
/*N*/ 		// rechte Spalte
/*N*/ 		nEdges = aCol[nCol2].GetBlockMatrixEdges( nRow1, nRow2, 16 );
/*N*/ 		// nicht 16 oder 1 oder 32
/*N*/ 		if ( nEdges && (((nEdges & 16) != 16) || (nEdges & 33)) )
/*N*/ 			return TRUE;		// rechte Kante fehlt oder offen
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( nRow1 == nRow2 )
/*N*/ 	{	// obere und untere Zeile
/*N*/ 		USHORT i;
/*N*/ 		BOOL bOpen = FALSE;
/*N*/ 		const USHORT n = 2 | 8;
/*N*/ 		for ( i=nCol1; i<=nCol2; i++)
/*N*/ 		{
/*N*/ 			nEdges = aCol[i].GetBlockMatrixEdges( nRow1, nRow1, n );
/*N*/ 			if ( nEdges )
/*N*/ 			{
/*N*/ 				if ( (nEdges & n) != n )
/*N*/ 					return TRUE;		// obere oder untere Kante fehlt
/*N*/ 				if ( nEdges & 4 )
/*N*/ 					bOpen = TRUE;		// linke Kante oeffnet, weitersehen
/*N*/ 				else if ( !bOpen )
/*N*/ 					return TRUE;		// es gibt was, was nicht geoeffnet wurde
/*N*/ 				if ( nEdges & 16 )
/*N*/ 					bOpen = FALSE;		// rechte Kante schliesst
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bOpen )
/*N*/ 			return TRUE;				// es geht noch weiter
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT j, nR, n, i;
/*N*/ 		// erst obere Zeile, dann untere Zeile
/*N*/ 		for ( j=0, nR=nRow1, n=8; j<2; j++, nR=nRow2, n=2 )
/*N*/ 		{
/*N*/ 			BOOL bOpen = FALSE;
/*N*/ 			for ( i=nCol1; i<=nCol2; i++)
/*N*/ 			{
/*N*/ 				nEdges = aCol[i].GetBlockMatrixEdges( nR, nR, n );
/*N*/ 				if ( nEdges )
/*N*/ 				{
/*N*/ 					// in oberere Zeile keine obere Kante bzw.
/*N*/ 					// in unterer Zeile keine untere Kante
/*N*/ 					if ( (nEdges & n) != n )
/*N*/ 						return TRUE;
/*N*/ 					if ( nEdges & 4 )
/*N*/ 						bOpen = TRUE;		// linke Kante oeffnet, weitersehen
/*N*/ 					else if ( !bOpen )
/*N*/ 						return TRUE;		// es gibt was, was nicht geoeffnet wurde
/*N*/ 					if ( nEdges & 16 )
/*N*/ 						bOpen = FALSE;		// rechte Kante schliesst
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( bOpen )
/*N*/ 				return TRUE;				// es geht noch weiter
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ BOOL ScTable::HasSelectionMatrixFragment( const ScMarkData& rMark ) const
/*N*/ {
/*N*/ 	BOOL bFound=FALSE;
/*N*/ 	for (USHORT i=0; i<=MAXCOL && !bFound; i++)
/*N*/ 		bFound |= aCol[i].HasSelectionMatrixFragment(rMark);
/*N*/ 	return bFound;
/*N*/ }


/*N*/ BOOL ScTable::IsBlockEditable( USHORT nCol1, USHORT nRow1, USHORT nCol2,
/*N*/ 			USHORT nRow2, BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
/*N*/ {
/*N*/ 	BOOL bIsEditable;
/*N*/ 	if ( nLockCount )
/*N*/ 		bIsEditable = FALSE;
/*N*/ 	else if ( bProtected )
/*N*/ 		bIsEditable = !HasAttrib( nCol1, nRow1, nCol2, nRow2, HASATTR_PROTECTED );
/*N*/ 	else
/*N*/ 		bIsEditable = TRUE;
/*N*/ 	if ( bIsEditable )
/*N*/ 	{
/*N*/ 		if ( HasBlockMatrixFragment( nCol1, nRow1, nCol2, nRow2 ) )
/*N*/ 		{
/*N*/ 			bIsEditable = FALSE;
/*N*/ 			if ( pOnlyNotBecauseOfMatrix )
/*N*/ 				*pOnlyNotBecauseOfMatrix = TRUE;
/*N*/ 		}
/*N*/ 		else if ( pOnlyNotBecauseOfMatrix )
/*N*/ 			*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 	}
/*N*/ 	else if ( pOnlyNotBecauseOfMatrix )
/*N*/ 		*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 	return bIsEditable;
/*N*/ }


/*N*/ BOOL ScTable::IsSelectionEditable( const ScMarkData& rMark,
/*N*/ 			BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
/*N*/ {
/*N*/ 	BOOL bIsEditable;
/*N*/ 	if ( nLockCount )
/*N*/ 		bIsEditable = FALSE;
/*N*/ 	else if ( bProtected )
    {
{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		 bIsEditable = !HasAttribSelection( rMark, HASATTR_PROTECTED );
        bIsEditable = FALSE;
    }
/*N*/ 	else
/*N*/ 		bIsEditable = TRUE;
/*N*/ 	if ( bIsEditable )
/*N*/ 	{
/*N*/ 		if ( HasSelectionMatrixFragment( rMark ) )
/*N*/ 		{
/*N*/ 			bIsEditable = FALSE;
/*N*/ 			if ( pOnlyNotBecauseOfMatrix )
/*N*/ 				*pOnlyNotBecauseOfMatrix = TRUE;
/*N*/ 		}
/*N*/ 		else if ( pOnlyNotBecauseOfMatrix )
/*N*/ 			*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 	}
/*N*/ 	else if ( pOnlyNotBecauseOfMatrix )
/*N*/ 		*pOnlyNotBecauseOfMatrix = FALSE;
/*N*/ 	return bIsEditable;
/*N*/ }



/*N*/ void ScTable::LockTable()
/*N*/ {
/*N*/ 	++nLockCount;
/*N*/ }


/*N*/ void ScTable::UnlockTable()
/*N*/ {
/*N*/ 	if (nLockCount)
/*N*/ 		--nLockCount;
/*N*/ 	else
/*N*/ 		DBG_ERROR("UnlockTable ohne LockTable");
/*N*/ }


/*N*/ void ScTable::MergeSelectionPattern( SfxItemSet** ppSet, const ScMarkData& rMark, BOOL bDeep ) const
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].MergeSelectionPattern( ppSet, rMark, bDeep );
/*N*/ }


/*N*/ void ScTable::MergePatternArea( SfxItemSet** ppSet, USHORT nCol1, USHORT nRow1,
/*N*/ 													USHORT nCol2, USHORT nRow2, BOOL bDeep ) const
/*N*/ {
/*N*/ 	for (USHORT i=nCol1; i<=nCol2; i++)
/*N*/ 		aCol[i].MergePatternArea( ppSet, nRow1, nRow2, bDeep );
/*N*/ }


/*N*/ void ScTable::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
/*N*/ 					USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow ) const
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
/*N*/ }


/*N*/ void ScTable::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
/*N*/ 					USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
/*N*/ }




/*N*/ void ScTable::ApplyPatternArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
/*N*/ 									 const ScPatternAttr& rAttr )
/*N*/ {
/*N*/ 	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
/*N*/ 	{
/*N*/ 		PutInOrder(nStartCol, nEndCol);
/*N*/ 		PutInOrder(nStartRow, nEndRow);
/*N*/ 		for (USHORT i = nStartCol; i <= nEndCol; i++)
/*N*/ 			aCol[i].ApplyPatternArea(nStartRow, nEndRow, rAttr);
/*N*/ 	}
/*N*/ }






/*N*/ void ScTable::ApplyStyleArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow, const ScStyleSheet& rStyle )
/*N*/ {
/*N*/ 	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
/*N*/ 	{
/*N*/ 		PutInOrder(nStartCol, nEndCol);
/*N*/ 		PutInOrder(nStartRow, nEndRow);
/*N*/ 		for (USHORT i = nStartCol; i <= nEndCol; i++)
/*N*/ 			aCol[i].ApplyStyleArea(nStartRow, nEndRow, rStyle);
/*N*/ 	}
/*N*/ }

/*N*/ void ScTable::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].ApplySelectionStyle( rStyle, rMark );
/*N*/ }

/*N*/ const ScStyleSheet* ScTable::GetSelectionStyle( const ScMarkData& rMark, BOOL& rFound ) const
/*N*/ {
/*N*/ 	rFound = FALSE;
/*N*/ 
/*N*/ 	BOOL	bEqual = TRUE;
/*N*/ 	BOOL	bColFound;
/*N*/ 	USHORT	i;
/*N*/ 
/*N*/ 	const ScStyleSheet* pStyle = NULL;
/*N*/ 	const ScStyleSheet* pNewStyle;
/*N*/ 
/*N*/ 	for (i=0; i<=MAXCOL && bEqual; i++)
/*N*/ 		if (rMark.HasMultiMarks(i))
/*N*/ 		{
/*N*/ 			pNewStyle = aCol[i].GetSelectionStyle( rMark, bColFound );
/*N*/ 			if (bColFound)
/*N*/ 			{
/*N*/ 				rFound = TRUE;
/*N*/ 				if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
/*N*/ 					bEqual = FALSE;												// unterschiedliche
/*N*/ 				pStyle = pNewStyle;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 	return bEqual ? pStyle : NULL;
/*N*/ }


/*N*/ const ScStyleSheet*	ScTable::GetAreaStyle( BOOL& rFound, USHORT nCol1, USHORT nRow1,
/*N*/ 													USHORT nCol2, USHORT nRow2 ) const
/*N*/ {
/*N*/ 	rFound = FALSE;
/*N*/ 
/*N*/ 	BOOL	bEqual = TRUE;
/*N*/ 	BOOL	bColFound;
/*N*/ 	USHORT	i;
/*N*/ 
/*N*/ 	const ScStyleSheet* pStyle = NULL;
/*N*/ 	const ScStyleSheet* pNewStyle;
/*N*/ 
/*N*/ 	for (i=nCol1; i<=nCol2 && bEqual; i++)
/*N*/ 	{
/*N*/ 		pNewStyle = aCol[i].GetAreaStyle(bColFound, nRow1, nRow2);
/*N*/ 		if (bColFound)
/*N*/ 		{
/*N*/ 			rFound = TRUE;
/*N*/ 			if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
/*N*/ 				bEqual = FALSE;												// unterschiedliche
/*N*/ 			pStyle = pNewStyle;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bEqual ? pStyle : NULL;
/*N*/ }


/*N*/ void ScTable::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
/*N*/ 								OutputDevice* pDev,
/*N*/ 								double nPPTX, double nPPTY,
/*N*/ 								const Fraction& rZoomX, const Fraction& rZoomY )
/*N*/ {
/*N*/ 	BOOL* pUsed = new BOOL[MAXROW+1];
/*N*/     memset( pUsed, 0, sizeof(BOOL) * (MAXROW+1) );
/*N*/ 
/*N*/ 	USHORT nCol;
/*N*/ 	for (nCol=0; nCol<=MAXCOL; nCol++)
/*N*/ 		aCol[nCol].FindStyleSheet( pStyleSheet, pUsed, bRemoved );
/*N*/ 
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT i;
/*N*/ 	USHORT nStart;
/*N*/ 	USHORT nEnd;
/*N*/ 	for (i=0; i<=MAXROW; i++)
/*N*/ 	{
/*N*/ 		if (pUsed[i])
/*N*/ 		{
/*N*/ 			if (!bFound)
/*N*/ 			{
/*N*/ 				nStart = i;
/*N*/ 				bFound = TRUE;
/*N*/ 			}
/*N*/ 			nEnd = i;
/*N*/ 		}
/*N*/ 		else if (bFound)
/*N*/ 		{
/*?*/ 			SetOptimalHeight( nStart, nEnd, 0, pDev, nPPTX, nPPTY, rZoomX, rZoomY, FALSE );
/*?*/ 			bFound = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bFound)
/*N*/ 		SetOptimalHeight( nStart, nEnd, 0, pDev, nPPTX, nPPTY, rZoomX, rZoomY, FALSE );
/*N*/ 
/*N*/ 	delete[] pUsed;
/*N*/ }


/*N*/ BOOL ScTable::ApplyFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
/*N*/ 									INT16 nFlags )
/*N*/ {
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
/*N*/ 		for (USHORT i = nStartCol; i <= nEndCol; i++)
/*N*/ 			bChanged |= aCol[i].ApplyFlags(nStartRow, nEndRow, nFlags);
/*N*/ 	return bChanged;
/*N*/ }






/*N*/ void ScTable::ApplyAttr( USHORT nCol, USHORT nRow, const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	if (ValidColRow(nCol,nRow))
/*N*/ 		aCol[nCol].ApplyAttr( nRow, rAttr );
/*N*/ }


/*N*/ void ScTable::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++)
/*N*/ 		aCol[i].ApplySelectionCache( pCache, rMark );
/*N*/ }






//	Spaltenbreiten / Zeilenhoehen

/*N*/ void ScTable::SetColWidth( USHORT nCol, USHORT nNewWidth )
/*N*/ {
/*N*/ 	if (VALIDCOL(nCol) && pColWidth)
/*N*/ 	{
/*N*/ 		if (!nNewWidth)
/*N*/ 		{
/*N*/ //			DBG_ERROR("Spaltenbreite 0 in SetColWidth");
/*N*/ 			nNewWidth = STD_COL_WIDTH;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( nNewWidth != pColWidth[nCol] )
/*N*/ 		{
/*N*/ 			nRecalcLvl++;
/*N*/ 			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 			if (pDrawLayer)
/*N*/ 				pDrawLayer->WidthChanged( nTab, nCol, ((long) nNewWidth) - (long) pColWidth[nCol] );
/*N*/ 			pColWidth[nCol] = nNewWidth;
/*N*/ 			if( !--nRecalcLvl )
/*N*/ 				SetDrawPageSize();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Spaltennummer oder keine Breiten");
/*N*/ }


/*N*/ void ScTable::SetRowHeight( USHORT nRow, USHORT nNewHeight )
/*N*/ {
/*N*/ 	if (VALIDROW(nRow) && pRowHeight)
/*N*/ 	{
/*N*/ 		if (!nNewHeight)
/*N*/ 		{
/*?*/ 			DBG_ERROR("Zeilenhoehe 0 in SetRowHeight");
/*?*/ 			nNewHeight = ScGlobal::nStdRowHeight;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( nNewHeight != pRowHeight[nRow] )
/*N*/ 		{
/*N*/ 			nRecalcLvl++;
/*N*/ 			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 			if (pDrawLayer)
/*N*/ 				pDrawLayer->HeightChanged( nTab, nRow, ((long) nNewHeight) - (long) pRowHeight[nRow] );
/*N*/ 			pRowHeight[nRow] = nNewHeight;
/*N*/ 			if( !--nRecalcLvl )
/*?*/ 				SetDrawPageSize();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Zeilennummer oder keine Hoehen");
/*N*/ }


/*N*/ BOOL ScTable::SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nNewHeight,
/*N*/ 									double nPPTX,double nPPTY )
/*N*/ {
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 	if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && pRowHeight)
/*N*/ 	{
/*N*/ 		nRecalcLvl++;
/*N*/ 		if (!nNewHeight)
/*N*/ 		{
/*?*/ 			DBG_ERROR("Zeilenhoehe 0 in SetRowHeight");
/*?*/ 			nNewHeight = ScGlobal::nStdRowHeight;
/*N*/ 		}
/*N*/ 
/*N*/ 		long nNewPix = (long) ( nNewHeight * nPPTY );
/*N*/ 
/*N*/ 		BOOL bSingle = FALSE;
/*N*/ 		ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 		if (pDrawLayer)
/*N*/ 			if (pDrawLayer->HasObjectsInRows( nTab, nStartRow, nEndRow ))
/*N*/ 				bSingle = TRUE;
/*N*/ 
/*N*/ 		USHORT nRow;
/*N*/ 		if (bSingle)
/*N*/ 		{
/*N*/ 			BOOL bDiff = FALSE;
/*N*/ 			for (nRow=nStartRow; nRow<=nEndRow && !bDiff; nRow++)
/*N*/ 				bDiff = ( pRowHeight[nRow] != nNewHeight );
/*N*/ 			if (!bDiff)
/*N*/ 				bSingle = FALSE;
/*N*/ 		}
/*N*/ 		if (bSingle)
/*N*/ 		{
/*N*/ 			if (nEndRow-nStartRow < 20)
/*N*/ 				for (nRow=nStartRow; nRow<=nEndRow; nRow++)
/*N*/ 				{
/*N*/ 					if (!bChanged)
/*N*/ 						if ( pRowHeight[nRow] != nNewHeight )
/*N*/ 							bChanged = ( nNewPix != (long) ( pRowHeight[nRow] * nPPTY ) );
/*N*/ 					SetRowHeight( nRow, nNewHeight );
/*N*/ 				}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				USHORT nMid = (nStartRow+nEndRow) / 2;
/*?*/ 				if (SetRowHeightRange( nStartRow, nMid, nNewHeight, 1.0, 1.0 ))
/*?*/ 					bChanged = TRUE;
/*?*/ 				if (SetRowHeightRange( nMid+1, nEndRow, nNewHeight, 1.0, 1.0 ))
/*?*/ 					bChanged = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (pDrawLayer)
/*N*/ 			{
/*N*/ 				long nHeightDif = 0;
/*N*/ 				for (nRow=nStartRow; nRow<=nEndRow; nRow++)
/*N*/ 					nHeightDif += ((long) nNewHeight) - (long) pRowHeight[nRow];
/*N*/ 				pDrawLayer->HeightChanged( nTab, nEndRow, nHeightDif );
/*N*/ 			}
/*N*/ 			for (nRow=nStartRow; nRow<=nEndRow; nRow++)
/*N*/ 			{
/*N*/ 				if (!bChanged)
/*N*/ 					if ( pRowHeight[nRow] != nNewHeight )
/*N*/ 						bChanged = ( nNewPix != (long) ( pRowHeight[nRow] * nPPTY ) );
/*N*/ 				pRowHeight[nRow] = nNewHeight;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( !--nRecalcLvl )
/*N*/ 			SetDrawPageSize();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Zeilennummer oder keine Hoehen");
/*N*/ 
/*N*/ 	return bChanged;
/*N*/ }


/*N*/ void ScTable::SetManualHeight( USHORT nStartRow, USHORT nEndRow, BOOL bManual )
/*N*/ {
/*N*/ 	if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && pRowFlags)
/*N*/ 	{
/*N*/ 		USHORT nRow;
/*N*/ 		if (bManual)
/*N*/ 			for (nRow=nStartRow; nRow<=nEndRow; nRow++)
/*N*/ 				pRowFlags[nRow] |= CR_MANUALSIZE;
/*N*/ 		else
/*N*/ 			for (nRow=nStartRow; nRow<=nEndRow; nRow++)
/*N*/ 				pRowFlags[nRow] &= ~CR_MANUALSIZE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Zeilennummer oder keine Zeilenflags");
/*N*/ }


/*N*/ USHORT ScTable::GetColWidth( USHORT nCol ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(VALIDCOL(nCol),"Falsche Spaltennummer");
/*N*/ 
/*N*/ 	if (VALIDCOL(nCol) && pColFlags && pColWidth)
/*N*/ 	{
/*N*/ 		if ( pColFlags[nCol] & CR_HIDDEN )
/*N*/ 			return 0;
/*N*/ 		else
/*N*/ 			return pColWidth[nCol];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return (USHORT) STD_COL_WIDTH;
/*N*/ }


/*N*/ USHORT ScTable::GetOriginalWidth( USHORT nCol ) const		// immer die eingestellte
/*N*/ {
/*N*/ 	DBG_ASSERT(VALIDCOL(nCol),"Falsche Spaltennummer");
/*N*/ 
/*N*/ 	if (VALIDCOL(nCol) && pColWidth)
/*N*/ 		return pColWidth[nCol];
/*N*/ 	else
/*N*/ 		return (USHORT) STD_COL_WIDTH;
/*N*/ }




/*N*/ USHORT ScTable::GetRowHeight( USHORT nRow ) const
/*N*/ {
/*N*/ 	DBG_ASSERT(VALIDROW(nRow),"Falsche Zeilennummer");
/*N*/ 
/*N*/ 	if (VALIDROW(nRow) && pRowFlags && pRowHeight)
/*N*/ 	{
/*N*/ 		if ( pRowFlags[nRow] & CR_HIDDEN )
/*N*/ 			return 0;
/*N*/ 		else
/*N*/ 			return pRowHeight[nRow];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return (USHORT) ScGlobal::nStdRowHeight;
/*N*/ }


/*N*/ USHORT ScTable::GetOriginalHeight( USHORT nRow ) const		// non-0 even if hidden
/*N*/ {
/*N*/ 	DBG_ASSERT(VALIDROW(nRow),"wrong row number");
/*N*/ 
/*N*/ 	if (VALIDROW(nRow) && pRowHeight)
/*N*/ 		return pRowHeight[nRow];
/*N*/ 	else
/*N*/ 		return (USHORT) ScGlobal::nStdRowHeight;
/*N*/ }


//	Spalten-/Zeilen-Flags


/*N*/ USHORT ScTable::GetHiddenRowCount( USHORT nRow ) const
/*N*/ {
/*N*/ 	USHORT nEndRow = nRow;
/*N*/ 	if ( pRowFlags )
/*N*/ 	{
/*N*/ 		while ( nEndRow <= MAXROW && ( pRowFlags[nEndRow] & CR_HIDDEN ) )
/*N*/ 			++nEndRow;
/*N*/ 	}
/*N*/ 	return nEndRow - nRow;
/*N*/ }


//!		ShowRows / DBShowRows zusammenfassen

/*N*/ void ScTable::ShowCol(USHORT nCol, BOOL bShow)
/*N*/ {
/*N*/ 	if (VALIDCOL(nCol) && pColFlags)
/*N*/ 	{
/*N*/ 		BOOL bWasVis = ( pColFlags[nCol] & CR_HIDDEN ) == 0;
/*N*/ 		if (bWasVis != bShow)
/*N*/ 		{
/*N*/ 			nRecalcLvl++;
/*N*/ 			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 			if (pDrawLayer)
/*N*/ 			{
/*N*/ 				if (bShow)
/*N*/ 					pDrawLayer->WidthChanged( nTab, nCol, (long) pColWidth[nCol] );
/*N*/ 				else
/*N*/ 					pDrawLayer->WidthChanged( nTab, nCol, -(long) pColWidth[nCol] );
/*N*/ 			}
/*N*/ 
/*N*/ 			if (bShow)
/*N*/ 				pColFlags[nCol] &= ~CR_HIDDEN;
/*N*/ 			else
/*N*/ 				pColFlags[nCol] |= CR_HIDDEN;
/*N*/ 			if( !--nRecalcLvl )
/*?*/ 				SetDrawPageSize();
/*N*/ 
/*N*/ 			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
/*N*/ 			if ( pCharts && pCharts->GetCount() )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pCharts->SetRangeDirty(ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ));
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Spaltennummer oder keine Flags");
/*N*/ }


/*N*/ void ScTable::ShowRows(USHORT nRow1, USHORT nRow2, BOOL bShow)
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	USHORT nStartRow = nRow1;
/*N*/ 	nRecalcLvl++;
/*N*/ 	while (nStartRow <= nRow2)
/*N*/ 	{
/*N*/ 		USHORT nEndRow = nStartRow;
/*N*/ 		BYTE nOldFlag = pRowFlags[nStartRow] & CR_HIDDEN;
/*N*/ 		while ( nEndRow < nRow2 && (pRowFlags[nEndRow+1] & CR_HIDDEN) == nOldFlag )
/*N*/ 			++nEndRow;
/*N*/ 
/*N*/ 		BOOL bWasVis = ( nOldFlag == 0 );
/*N*/ 		BOOL bChanged = ( bWasVis != bShow );
/*N*/ 		if ( bChanged )
/*N*/ 		{
/*N*/ 			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 			if (pDrawLayer)
/*N*/ 			{
/*N*/ 				long nHeight = 0;
/*N*/ 				for (i=nStartRow; i<=nEndRow; i++)
/*N*/ 					nHeight += pRowHeight[i];
/*N*/ 
/*N*/ 				if (bShow)
/*N*/ 					pDrawLayer->HeightChanged( nTab, nStartRow, nHeight );
/*N*/ 				else
/*N*/ 					pDrawLayer->HeightChanged( nTab, nStartRow, -nHeight );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bShow)
/*N*/ 			for (i=nStartRow; i<=nEndRow; i++)
/*N*/ 				pRowFlags[i] &= ~(CR_HIDDEN | CR_FILTERED);
/*N*/ 		else
/*N*/ 			for (i=nStartRow; i<=nEndRow; i++)
/*N*/ 				pRowFlags[i] |= CR_HIDDEN;
/*N*/ 
/*N*/ 		if ( bChanged )
/*N*/ 		{
/*N*/ 			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
/*N*/ 			if ( pCharts && pCharts->GetCount() )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));
/*N*/ 		}
/*N*/ 
/*N*/ 		nStartRow = nEndRow + 1;
/*N*/ 	}
/*N*/ 	if( !--nRecalcLvl )
/*?*/ 		SetDrawPageSize();
/*N*/ }


/*N*/ BOOL ScTable::IsFiltered(USHORT nRow) const
/*N*/ {
/*N*/  	if (VALIDROW(nRow) && pRowFlags)
/*N*/  		return ( pRowFlags[nRow] & CR_FILTERED ) != 0;
/*N*/ 
/*N*/ 	DBG_ERROR("Falsche Zeilennummer oder keine Flags");
/*N*/ 	return FALSE;
/*N*/ }




/*N*/ void ScTable::SetRowFlags( USHORT nRow, BYTE nNewFlags )
/*N*/ {
/*N*/ 	if (VALIDROW(nRow) && pRowFlags)
/*N*/ 		pRowFlags[nRow] = nNewFlags;
/*N*/ 	else
/*N*/ 		DBG_ERROR("Falsche Zeilennummer oder keine Flags");
/*N*/ }


/*N*/ BYTE ScTable::GetColFlags( USHORT nCol ) const
/*N*/ {
/*N*/ 	if (VALIDCOL(nCol) && pColFlags)
/*N*/ 		return pColFlags[nCol];
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }


/*N*/ BYTE ScTable::GetRowFlags( USHORT nRow ) const
/*N*/ {
/*N*/ 	if (VALIDROW(nRow) && pRowFlags)
/*N*/ 		return pRowFlags[nRow];
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }






/*N*/ USHORT ScTable::GetLastChangedCol() const
/*N*/ {
/*N*/ 	if ( !pColFlags )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	USHORT nLastFound = 0;
/*N*/ 	for (USHORT nCol = 1; nCol <= MAXCOL; nCol++)
/*N*/ 		if ((pColFlags[nCol] & ~CR_PAGEBREAK) || (pColWidth[nCol] != STD_COL_WIDTH))
/*N*/ 			nLastFound = nCol;
/*N*/ 
/*N*/ 	return nLastFound;
/*N*/ }


/*N*/ USHORT ScTable::GetLastChangedRow() const
/*N*/ {
/*N*/ 	if ( !pRowFlags )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	USHORT nLastFound = 0;
/*N*/ 	for (USHORT nRow = 1; nRow <= MAXROW; nRow++)
/*N*/ 		if ((pRowFlags[nRow] & ~CR_PAGEBREAK) || (pRowHeight[nRow] != ScGlobal::nStdRowHeight))
/*N*/ 			nLastFound = nRow;
/*N*/ 
/*N*/ 	return nLastFound;
/*N*/ }


/*N*/ BOOL ScTable::UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, BOOL bShow )
/*N*/ {
/*N*/ 	if (pOutlineTable && pColFlags)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); return FALSE;} //STRIP001 return pOutlineTable->GetColArray()->ManualAction( nStartCol, nEndCol, bShow, pColFlags );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ BOOL ScTable::UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, BOOL bShow )
/*N*/ {
/*N*/ 	if (pOutlineTable && pRowFlags)
/*N*/ 		return pOutlineTable->GetRowArray()->ManualAction( nStartRow, nEndRow, bShow, pRowFlags );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


//	Auto-Outline

/*N*/ inline short DiffSign( USHORT a, USHORT b )
/*N*/ {
/*N*/ 	return (a<b) ? -1 :
/*N*/ 			(a>b) ? 1 : 0;
/*N*/ }



//
//	Datei-Operationen
//

//	Laden

/*N*/ BOOL ScTable::Load( SvStream& rStream, USHORT nVersion, ScProgress* pProgress )
/*N*/ {
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	while (aHdr.BytesLeft() && rStream.GetError() == SVSTREAM_OK)
/*N*/ 	{
/*N*/ 		USHORT nID;
/*N*/ 		rStream >> nID;
/*N*/ 		switch (nID)
/*N*/ 		{
/*N*/ 			case SCID_COLUMNS:						//	Spalten (Daten)
/*N*/ 				{
/*N*/ 					ScMultipleReadHeader aColHdr( rStream );
/*N*/ 					if( nVersion >= SC_DATABYTES )
/*N*/ 					  while( aColHdr.BytesLeft() )
/*N*/ 					{
/*N*/ 						BYTE nCol;
/*N*/ 						rStream >> nCol;
/*N*/ 						aCol[ nCol ].Load(rStream, aColHdr);
/*N*/ 						if (pProgress)
/*N*/ 							pProgress->SetState( rStream.Tell() );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					  for (i=0; i<=MAXCOL; i++)
/*N*/ 					{
/*N*/ 						aCol[i].Load(rStream, aColHdr);
/*N*/ 						if (pProgress)
/*N*/ 							pProgress->SetState( rStream.Tell() );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SCID_COLROWFLAGS:					//	Spalten-/Zeilenflags
/*N*/ 				{
/*N*/ 					ScReadHeader aFlagsHdr( rStream );
/*N*/ 
/*N*/ 					BYTE	nFlags;
/*N*/ 					USHORT	nVal;
/*N*/ 					short	nRep;
/*N*/ 
/*N*/ 					i=0;
/*N*/ 					while (i<=MAXCOL)
/*N*/ 					{
/*N*/ 						rStream >> nRep;
/*N*/ 						rStream >> nVal;
/*N*/ 						while (nRep-- && i<=MAXCOL)
/*N*/ 							pColWidth[i++] = nVal;
/*N*/ 					}
/*N*/ 					DBG_ASSERT(nRep==-1, "Fehler bei Spaltenbreiten");
/*N*/ 					i=0;
/*N*/ 					while (i<=MAXCOL)
/*N*/ 					{
/*N*/ 						rStream >> nRep;
/*N*/ 						rStream >> nFlags;
/*N*/ 						while (nRep-- && i<=MAXCOL)
/*N*/ 							pColFlags[i++] = nFlags;
/*N*/ 					}
/*N*/ 					DBG_ASSERT(nRep==-1, "Fehler bei Spaltenflags");
/*N*/ 					if (pProgress)
/*N*/ 						pProgress->SetState( rStream.Tell() );
/*N*/ 
/*N*/ 					//	aus der Datei immer soviele Zeilen laden, wie gespeichert wurden
/*N*/ 					USHORT nSrcMaxRow = pDocument->GetSrcMaxRow();
/*N*/ 
/*N*/ 					i=0;
/*N*/ 					while (i<=nSrcMaxRow)		// nSrcMaxRow und MAXROW evtl. unterschiedlich
/*N*/ 					{
/*N*/ 						rStream >> nRep;
/*N*/ 						rStream >> nVal;
/*N*/ 						while (nRep--)
/*N*/ 						{
/*N*/ 							if (i<=MAXROW)
/*N*/ 								pRowHeight[i] = nVal;
/*N*/ 							++i;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					DBG_ASSERT(i==nSrcMaxRow+1, "Fehler bei Zeilenhoehen");
/*N*/ 					if (pProgress)
/*N*/ 						pProgress->SetState( rStream.Tell() );
/*N*/ 					i=0;
/*N*/ 					while (i<=nSrcMaxRow)		// nSrcMaxRow und MAXROW evtl. unterschiedlich
/*N*/ 					{
/*N*/ 						rStream >> nRep;
/*N*/ 						rStream >> nFlags;
/*N*/ 						while (nRep--)
/*N*/ 						{
/*N*/ 							if (i<=MAXROW)
/*N*/ 								pRowFlags[i] = nFlags;
/*N*/ 							++i;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					DBG_ASSERT(i==nSrcMaxRow+1, "Fehler bei Zeilenflags");
/*N*/ 					if (pProgress)
/*N*/ 						pProgress->SetState( rStream.Tell() );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SCID_TABOPTIONS:					//	einzelne Einstellungen
/*N*/ 				{
/*N*/ 					ScReadHeader aFlagsHdr( rStream );
/*N*/ 
/*N*/ 					rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 					rStream >> bScenario;
/*N*/ 					rStream.ReadByteString( aComment, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 					rStream >> bProtected;
/*N*/ 					String aPass;
/*N*/ 					rStream.ReadByteString( aPass, rStream.GetStreamCharSet() );
/*N*/ 					if (aPass.Len())
/*N*/ 						SvPasswordHelper::GetHashPassword(aProtectPass, aPass);
/*N*/ 
/*N*/ 					BOOL bOutline;
/*N*/ 					rStream >> bOutline;
/*N*/ 					if (bOutline)
/*N*/ 					{
/*N*/ 						StartOutlineTable();
/*N*/ 						pOutlineTable->Load( rStream );
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( aFlagsHdr.BytesLeft() )
/*N*/ 					{
/*N*/ 						SfxStyleSheetBasePool* pStylePool =
/*N*/ 								pDocument->GetStyleSheetPool();
/*N*/ 
/*N*/ 						rStream.ReadByteString( aPageStyle, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 						if ( !pStylePool->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE ) )
/*N*/ 						{
/*N*/ 							DBG_TRACE( "PageStyle not found. Using Standard." );
/*N*/ 							aPageStyle = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( aFlagsHdr.BytesLeft() )
/*N*/ 					{
/*N*/ 						BOOL bOneRange;						// einzelner Druckbereich ?
/*N*/ 						rStream >> bOneRange;
/*N*/ 						if ( bOneRange )
/*N*/ 						{
/*N*/ 							ScRange aRange;
/*N*/ 							rStream >> aRange;
/*N*/ 							SetPrintRangeCount( 1 );
/*N*/ 							SetPrintRange( 0, aRange );
/*N*/ 						}
/*N*/ 
/*N*/ 						lcl_LoadRange( rStream, &pRepeatColRange );
/*N*/ 						lcl_LoadRange( rStream, &pRepeatRowRange );
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( aFlagsHdr.BytesLeft() )
/*N*/ 						rStream >> bVisible;
/*N*/ 
/*N*/ 					if ( aFlagsHdr.BytesLeft() )			// Druckbereiche ab Version 314c
/*N*/ 					{
/*N*/ 						USHORT nNewCount;
/*N*/ 						rStream >> nNewCount;
/*N*/ 						if ( nNewCount )
/*N*/ 						{
/*?*/ 							ScRange aTmp;
/*?*/ 							SetPrintRangeCount( nNewCount );
/*?*/ 							for (i=0; i<nNewCount; i++)
/*?*/ 							{
/*?*/ 								rStream >> aTmp;
/*?*/ 								SetPrintRange( i, aTmp );
/*?*/ 							}
/*?*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( aFlagsHdr.BytesLeft() )			// erweiterte Szenario-Flags ab 5.0
/*N*/ 					{
/*N*/ 						rStream >> aScenarioColor;
/*N*/ 						rStream >> nScenarioFlags;
/*N*/ 						rStream >> bActiveScenario;
/*N*/ 					}
/*N*/ 					else if ( bScenario )
/*N*/ 					{
/*?*/ 						//	Default fuer Szenarien aus alten Dateien??
/*?*/ 						//	Wenn die alten Szenarien wirklich benutzt wurden,
/*?*/ 						//	wuerde der Rahmen wahrscheinlich stoeren.
/*?*/ 
/*?*/ 						nScenarioFlags = SC_SCENARIO_COPYALL;
/*N*/ 					}
/*N*/ 
/*N*/ 					if (pProgress)
/*N*/ 						pProgress->SetState( rStream.Tell() );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SCID_TABLINK:						//	Verknuepfung
/*N*/ 				{
/*N*/ 					ScReadHeader aLinkHdr( rStream );
/*N*/ 
/*N*/ 					rStream >> nLinkMode;
/*N*/ 					rStream.ReadByteString( aLinkDoc, rStream.GetStreamCharSet() );
/*N*/ 					aLinkDoc = ::binfilter::StaticBaseUrl::RelToAbs( aLinkDoc );
/*N*/ 					rStream.ReadByteString( aLinkFlt, rStream.GetStreamCharSet() );
/*N*/ 					rStream.ReadByteString( aLinkTab, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 					BOOL bRelURL;
/*N*/ 					if ( aLinkHdr.BytesLeft() )
/*N*/ 						rStream >> bRelURL;
/*N*/ 					else
/*N*/ 						bRelURL = FALSE;
/*N*/ 					// externer Tabellenname relativ zu absolut
/*N*/ 					if ( nLinkMode == SC_LINK_VALUE && bRelURL )
/*N*/ 						aName = ScGlobal::GetDocTabName( aLinkDoc, aLinkTab );
/*N*/ 
/*N*/ 					if ( aLinkHdr.BytesLeft() )		// ab 336 auch Filter-Optionen
/*N*/ 						rStream.ReadByteString( aLinkOpt, rStream.GetStreamCharSet() );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				{
/*N*/ 					DBG_ERROR("unbekannter Sub-Record in ScTable::Load");
/*N*/ 					ScReadHeader aDummyHeader( rStream );
/*N*/ 				}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Neuberechnungen
/*N*/ 
/*N*/ 	ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 	if (pDrawLayer)
/*?*/ 		pDrawLayer->ScRenamePage( nTab, aName );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

//	Speichern


/*N*/ void lcl_SaveValue( SvStream& rStream, USHORT* pValue, USHORT nEnd )
/*N*/ {
/*N*/ 	USHORT nPos = 0;
/*N*/ 	while (nPos<=nEnd)
/*N*/ 	{
/*N*/ 		USHORT nVal = pValue[nPos];
/*N*/ 		USHORT nNextPos = nPos+1;
/*N*/ 		while (nNextPos<=nEnd && pValue[nNextPos]==nVal)
/*N*/ 			++nNextPos;
/*N*/ 		rStream << (USHORT)( nNextPos - nPos );
/*N*/ 		rStream << nVal;
/*N*/ 		nPos = nNextPos;
/*N*/ 	}
/*N*/ }


/*N*/ void lcl_SaveFlags( SvStream& rStream, BYTE* pValue, USHORT nEnd )
/*N*/ {
/*N*/ 	USHORT nPos = 0;
/*N*/ 	while (nPos<=nEnd)
/*N*/ 	{
/*N*/ 		BYTE nVal = pValue[nPos] & CR_SAVEMASK;
/*N*/ 		USHORT nNextPos = nPos+1;
/*N*/ 		while (nNextPos<=nEnd && (pValue[nNextPos] & CR_SAVEMASK)==nVal)
/*N*/ 			++nNextPos;
/*N*/ 		rStream << (USHORT)( nNextPos - nPos );
/*N*/ 		rStream << nVal;
/*N*/ 		nPos = nNextPos;
/*N*/ 	}
/*N*/ }


/*N*/ void lcl_LoadRange( SvStream& rStream, ScRange** ppRange )
/*N*/ {
/*N*/ 	BOOL bIsSet = FALSE;
/*N*/ 
/*N*/ 	rStream >> bIsSet;
/*N*/ 
/*N*/ 	if ( bIsSet )
/*N*/ 	{
/*N*/ 		*ppRange = new ScRange;
/*N*/ 		rStream >> **ppRange;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		*ppRange = NULL;
/*N*/ }


/*N*/ void lcl_SaveRange( SvStream& rStream, ScRange* pRange )
/*N*/ {
/*N*/ 	if ( pRange )
/*N*/ 	{
/*N*/ 		rStream << (BOOL)TRUE;
/*N*/ 		rStream << *pRange;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		rStream << (BOOL)FALSE;
/*N*/ }


/*N*/ BOOL ScTable::Save( SvStream& rStream, long& rSavedDocCells, ScProgress* pProgress ) const
/*N*/ {
/*N*/ 	ScWriteHeader aHdr( rStream );
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	//	Spalten (Daten)
/*N*/ 
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_COLUMNS;
/*N*/ 		ScMultipleWriteHeader aColHdr( rStream );
/*N*/ 
/*N*/ 		for (i=0; i<=MAXCOL; i++)
/*N*/ 		{
/*N*/ 			const ScColumn* pCol = &aCol[ i ];
/*N*/ 			if( !pCol->IsEmptyData() || pCol->NoteCount() || !pCol->IsEmptyAttr())
/*N*/ 			{
/*N*/ 				rStream << (BYTE) i;
/*N*/ 				aCol[i].Save(rStream, aColHdr);
/*N*/ 				rSavedDocCells += aCol[i].GetWeightedCount();
/*N*/ 				if (pProgress)
/*N*/ 					pProgress->SetState( rSavedDocCells );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Spalten-/Zeilenflags
/*N*/ 
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_COLROWFLAGS;
/*N*/ 		ScWriteHeader aFlagsHdr( rStream );
/*N*/ 
/*N*/ 		USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();
/*N*/ 
/*N*/ 		lcl_SaveValue( rStream, pColWidth, MAXCOL );
/*N*/ 		lcl_SaveFlags( rStream, pColFlags, MAXCOL );
/*N*/ 		lcl_SaveValue( rStream, pRowHeight,nSaveMaxRow );
/*N*/ 		lcl_SaveFlags( rStream, pRowFlags, nSaveMaxRow );
/*N*/ 
/*N*/ 		//	wenn dabei weniger Zeilenhoehen als vorhanden gespeichert wurden,
/*N*/ 		//	gibt das noch keine Warnung wegen Datenverlust.
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bRelURL = FALSE;
/*N*/ 	String aLinkDocSaveName( aLinkDoc );
/*N*/ 	String aSaveName( aName );
/*N*/ 	if ( nLinkMode )
/*N*/ 	{
/*N*/ 		aLinkDocSaveName = ::binfilter::StaticBaseUrl::AbsToRel( aLinkDocSaveName );
/*N*/ 		aLinkDocSaveName = INetURLObject::decode( aLinkDocSaveName,
/*N*/ 			INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/         if ( ScGlobal::pTransliteration->isEqual( aLinkDocSaveName,
/*N*/                 INetURLObject::decode( aLinkDoc, INET_HEX_ESCAPE,
/*N*/                 INetURLObject::DECODE_UNAMBIGUOUS ) ) )
/*N*/ 		{
/*N*/ 			aSaveName = INetURLObject::decode( aSaveName,
/*N*/ 				INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			bRelURL = TRUE;
/*N*/ 			// Reference to external sheet, only the sheet name is stored
/*N*/ 			// instead of the absolute DocTabName, will be reconcatenated upon
/*N*/ 			// load time.
/*N*/ 			if ( nLinkMode == SC_LINK_VALUE )
/*N*/ 				aSaveName = aLinkTab;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	einzelne Einstellungen
/*N*/ 
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_TABOPTIONS;
/*N*/ 		ScWriteHeader aFlagsHdr( rStream );
/*N*/ 
/*N*/ 		rStream.WriteByteString( aSaveName, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 		rStream << bScenario;
/*N*/ 		rStream.WriteByteString( aComment, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 		rStream << bProtected;
/*N*/ 		String aPass;
/*N*/ 		//rStream.WriteByteString( aProtectPass, rStream.GetStreamCharSet() );
/*N*/ 		rStream.WriteByteString( aPass, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 		BOOL bOutline = ( pOutlineTable != NULL );
/*N*/ 		rStream << bOutline;
/*N*/ 		if (bOutline)
/*N*/ 			pOutlineTable->Store( rStream );
/*N*/ 
/*N*/ 		rStream.WriteByteString( aPageStyle, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 		if ( pPrintRanges && nPrintRangeCount == 1 )		// kompatibel zu alten Versionen
/*N*/ 			lcl_SaveRange( rStream, pPrintRanges );			// (nur wenn genau ein Bereich)
/*N*/ 		else
/*N*/ 			lcl_SaveRange( rStream, NULL );
/*N*/ 		lcl_SaveRange( rStream, pRepeatColRange );
/*N*/ 		lcl_SaveRange( rStream, pRepeatRowRange );
/*N*/ 
/*N*/ 		rStream << bVisible;
/*N*/ 
/*N*/ 		if ( pPrintRanges && nPrintRangeCount>1 )			// einzelner Bereich schon oben
/*N*/ 		{
/*?*/ 			rStream << nPrintRangeCount;					// ab Version 314c
/*?*/ 			if ( nPrintRangeCount > 1 )
/*?*/ 				for ( i=0; i<nPrintRangeCount; i++)
/*?*/ 					rStream << pPrintRanges[i];
/*N*/ 		}
/*N*/ 		else
/*N*/ 			rStream << (USHORT) 0;
/*N*/ 
/*N*/ 		if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )	// erweiterte Szenario-Flags ab 5.0
/*N*/ 		{
/*N*/ 			rStream << aScenarioColor;
/*N*/ 			rStream << nScenarioFlags;
/*N*/ 			rStream << bActiveScenario;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	Verknuepfung
/*N*/ 
/*N*/ 	if (nLinkMode)
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_TABLINK;
/*N*/ 		ScWriteHeader aLinkHdr( rStream );
/*N*/ 
/*N*/ 		//	Hack, um verknuepfte Tabellen mit der 3.1 laden zu koennen (#35242#)
/*N*/ 		//	bei 3.1 Export Filter von "StarCalc 4.0" auf "StarCalc 3.0" umsetzen
/*N*/ 		//	(4.0 Dateien koennen vom Calc 3.1 gelesen werden)
/*N*/ 		String aSaveFlt = aLinkFlt;
/*N*/ 		if ( rStream.GetVersion() == SOFFICE_FILEFORMAT_31 && aSaveFlt.EqualsAscii("StarCalc 4.0") )
/*?*/ 			aSaveFlt.AssignAscii(RTL_CONSTASCII_STRINGPARAM("StarCalc 3.0"));
/*N*/ 
/*N*/ 		rStream	<< nLinkMode;
/*N*/ 		rStream.WriteByteString( aLinkDocSaveName, rStream.GetStreamCharSet() );
/*N*/ 		rStream.WriteByteString( aSaveFlt, rStream.GetStreamCharSet() );
/*N*/ 		rStream.WriteByteString( aLinkTab, rStream.GetStreamCharSet() );
/*N*/ 		rStream	<< bRelURL;
/*N*/ 		rStream.WriteByteString( aLinkOpt, rStream.GetStreamCharSet() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


                                    //	CopyData - fuer Query in anderen Bereich







// Berechnen der Groesse der Tabelle und setzen der Groesse an der DrawPage

/*N*/ void ScTable::SetDrawPageSize()
/*N*/ {
/*N*/ 	ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
/*N*/ 	if( pDrawLayer )
/*N*/ 	{
/*N*/ 		ULONG x = GetColOffset( MAXCOL + 1 );
/*N*/ 		ULONG y = GetRowOffset( MAXROW + 1 );
/*N*/ 		x = (ULONG) ((double) x * HMM_PER_TWIPS);
/*N*/ 		y = (ULONG) ((double) y * HMM_PER_TWIPS);
/*N*/ 		pDrawLayer->SetPageSize( nTab, Size( x, y ) );
/*N*/ 	}
/*N*/ }


/*N*/ ULONG ScTable::GetRowOffset( USHORT nRow ) const
/*N*/ {
/*N*/ 	ULONG n = 0;
/*N*/ 	if ( pRowFlags && pRowHeight )
/*N*/ 	{
/*N*/ 		USHORT i;
/*N*/ 		BYTE* pFlags = pRowFlags;
/*N*/ 		USHORT* pHeight = pRowHeight;
/*N*/ 		for( i = 0; i < nRow; i++, pFlags++, pHeight++ )
/*N*/ 			if( !( *pFlags & CR_HIDDEN ) )
/*N*/ 				n += *pHeight;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("GetRowOffset: Daten fehlen");
/*N*/ 	return n;
/*N*/ }


/*N*/ ULONG ScTable::GetColOffset( USHORT nCol ) const
/*N*/ {
/*N*/ 	ULONG n = 0;
/*N*/ 	if ( pColFlags && pColWidth )
/*N*/ 	{
/*N*/ 		USHORT i;
/*N*/ 		BYTE* pFlags = pColFlags;
/*N*/ 		USHORT* pWidth = pColWidth;
/*N*/ 		for( i = 0; i < nCol; i++, pFlags++, pWidth++ )
/*N*/ 			if( !( *pFlags & CR_HIDDEN ) )
/*N*/ 				n += *pWidth;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("GetColumnOffset: Daten fehlen");
/*N*/ 	return n;
/*N*/ }

}
