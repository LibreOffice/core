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

// INCLUDE ---------------------------------------------------------------

#include <bf_svtools/zforlist.hxx>

#include "dociter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "attarray.hxx"
#include "docoptio.hxx"
namespace binfilter {

//------------------------------------------------------------------------
//------------------------------------------------------------------------
void lcl_IterGetNumberFormat( ULONG& nFormat, const ScAttrArray*& rpArr,
        USHORT& nAttrEndRow, const ScAttrArray* pNewArr, USHORT nRow,
        ScDocument* pDoc )
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( rpArr != pNewArr || nAttrEndRow < nRow )
}

/*N*/ ScValueIterator::ScValueIterator( ScDocument* pDocument, const ScRange& rRange,
/*N*/ 			BOOL bSTotal, BOOL bTextZero ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nStartCol( rRange.aStart.Col() ),
/*N*/ 	nStartRow( rRange.aStart.Row() ),
/*N*/ 	nStartTab( rRange.aStart.Tab() ),
/*N*/ 	nEndCol( rRange.aEnd.Col() ),
/*N*/ 	nEndRow( rRange.aEnd.Row() ),
/*N*/ 	nEndTab( rRange.aEnd.Tab() ),
/*N*/ 	bSubTotal(bSTotal),
/*N*/ 	nNumFmtType( NUMBERFORMAT_UNDEFINED ),
/*N*/ 	nNumFmtIndex(0),
/*N*/ 	bNumValid( FALSE ),
/*N*/ 	bNextValid( FALSE ),
/*N*/ 	bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
/*N*/ 	bTextAsZero( bTextZero )
/*N*/ {
/*N*/ 	PutInOrder( nStartCol, nEndCol);
/*N*/ 	PutInOrder( nStartRow, nEndRow);
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 
/*N*/ 	if (nStartCol > MAXCOL) nStartCol = MAXCOL;
/*N*/ 	if (nEndCol > MAXCOL) nEndCol = MAXCOL;
/*N*/ 	if (nStartRow > MAXROW) nStartRow = MAXROW;
/*N*/ 	if (nEndRow > MAXROW) nEndRow = MAXROW;
/*N*/ 	if (nStartTab > MAXTAB) nStartTab = MAXTAB;
/*N*/ 	if (nEndTab > MAXTAB) nEndTab = MAXTAB;
/*N*/ 
/*N*/ 	nCol = nStartCol;
/*N*/ 	nRow = nStartRow;
/*N*/ 	nTab = nStartTab;
/*N*/ 
/*N*/ 	nColRow = 0;					// wird bei GetFirst initialisiert
/*N*/ 
/*N*/ 	nNumFormat = 0;					// werden bei GetNumberFormat initialisiert
/*N*/ 	pAttrArray = 0;
/*N*/ 	nAttrEndRow = 0;
/*N*/ }

/*N*/ BOOL ScValueIterator::GetThis(double& rValue, USHORT& rErr)
/*N*/ {
/*N*/ 	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 	for (;;)
/*N*/ 	{
/*N*/ 		if ( nRow > nEndRow )
/*N*/ 		{
/*N*/ 			nRow = nStartRow;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				nCol++;
/*N*/ 				if ( nCol > nEndCol )
/*N*/ 				{
/*N*/ 					nCol = nStartCol;
/*N*/ 					nTab++;
/*N*/ 					if ( nTab > nEndTab )
/*N*/ 					{
/*N*/ 						rValue = 0.0;
/*N*/ 						rErr = 0;
/*N*/ 						return FALSE;				// Ende und Aus
/*N*/ 					}
/*N*/ 				}
/*N*/ 				pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 			} while ( pCol->nCount == 0 );
/*N*/ 			pCol->Search( nRow, nColRow );
/*N*/ 		}
/*N*/ 
/*N*/ 		while (( nColRow < pCol->nCount ) && ( pCol->pItems[nColRow].nRow < nRow ))
/*N*/ 			nColRow++;
/*N*/ 
/*N*/ 		if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= nEndRow )
/*N*/ 		{
/*N*/ 			nRow = pCol->pItems[nColRow].nRow + 1;
/*N*/ 			if ( !bSubTotal || !pDoc->pTab[nTab]->IsFiltered( nRow-1 ) )
/*N*/ 			{
/*N*/ 				ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
/*N*/ 				++nColRow;
/*N*/ 				switch (pCell->GetCellType())
/*N*/ 				{
/*N*/ 					case CELLTYPE_VALUE:
/*N*/ 					{
/*N*/ 						bNumValid = FALSE;
/*N*/ 						rValue = ((ScValueCell*)pCell)->GetValue();
/*N*/ 						rErr = 0;
/*N*/ 						--nRow;
/*N*/ 						if ( bCalcAsShown )
/*N*/ 						{
/*?*/ #ifndef WTC
/*?*/ 							lcl_IterGetNumberFormat( nNumFormat,pAttrArray,
/*?*/ #else
/*?*/ 							lcl_IterGetNumberFormat( nNumFormat,
/*?*/ 								(ScAttrArray const *&)pAttrArray,
/*?*/ #endif
/*?*/ 								nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
/*?*/ 							rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
/*N*/ 						}
/*N*/ 						//
/*N*/ 						//	wenn in der selben Spalte gleich noch eine Value-Cell folgt, die
/*N*/ 						//	auch noch im Block liegt, den Wert jetzt schon holen
/*N*/ 						//
/*N*/ 						if ( nColRow < pCol->nCount &&
/*N*/ 							 pCol->pItems[nColRow].nRow <= nEndRow &&
/*N*/ 							 pCol->pItems[nColRow].pCell->GetCellType() == CELLTYPE_VALUE &&
/*N*/ 							 !bSubTotal )
/*N*/ 						{
/*N*/ 							fNextValue = ((ScValueCell*)pCol->pItems[nColRow].pCell)->GetValue();
/*N*/ 							nNextRow = pCol->pItems[nColRow].nRow;
/*N*/ 							bNextValid = TRUE;
/*N*/ 							if ( bCalcAsShown )
/*N*/ 							{
/*?*/ #ifndef WTC
/*?*/ 								lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
/*?*/ #else
/*?*/ 								lcl_IterGetNumberFormat( nNumFormat,
/*?*/ 									(ScAttrArray const *&)pAttrArray,
/*?*/ #endif
/*?*/ 									nAttrEndRow, pCol->pAttrArray, nNextRow, pDoc );
/*?*/ 								fNextValue = pDoc->RoundValueAsShown( fNextValue, nNumFormat );
/*N*/ 							}
/*N*/ 						}
/*N*/ 
/*N*/ 						return TRUE;									// gefunden
/*N*/ 					}
/*N*/ 					break;
/*N*/ 					case CELLTYPE_FORMULA:
/*N*/ 					{
/*N*/ 						if (!bSubTotal || !((ScFormulaCell*)pCell)->IsSubTotal())
/*N*/ 						{
/*N*/ 							rErr = ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/ 							if ( rErr || ((ScFormulaCell*)pCell)->IsValue() )
/*N*/ 							{
/*N*/ 								rValue = ((ScFormulaCell*)pCell)->GetValue();
/*N*/ 								nRow--;
/*N*/ 								bNumValid = FALSE;
/*N*/ 								return TRUE;							// gefunden
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 					case CELLTYPE_STRING :
/*N*/ 					case CELLTYPE_EDIT :
/*N*/ 					{
/*N*/ 						if ( bTextAsZero )
/*N*/ 						{
/*N*/ 							rErr = 0;
/*N*/ 							rValue = 0.0;
/*N*/ 							nNumFmtType = NUMBERFORMAT_NUMBER;
/*N*/ 							nNumFmtIndex = 0;
/*N*/ 							bNumValid = TRUE;
/*N*/ 							--nRow;
/*N*/ 							return TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nRow = nEndRow + 1;			// naechste Spalte
/*N*/ 	}
/*N*/ }

/*N*/ void ScValueIterator::GetCurNumFmtInfo( short& nType, ULONG& nIndex )
/*N*/ {
/*N*/ 	if (!bNumValid)
/*N*/ 	{
/*N*/ 		const ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 		nNumFmtIndex = pCol->GetNumberFormat( nRow );
/*N*/ 		if ( (nNumFmtIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
/*N*/ 		{
/*N*/ 			const ScBaseCell* pCell;
/*N*/ 			USHORT nIdx = nColRow - 1;
/*N*/ 			// there might be rearranged something, so be on the safe side
/*N*/ 			if ( nIdx < pCol->nCount && pCol->pItems[nIdx].nRow == nRow )
/*N*/ 				pCell = pCol->pItems[nIdx].pCell;
/*N*/ 			else
/*N*/ 			{
/*?*/ 				if ( pCol->Search( nRow, nIdx ) )
/*?*/ 					pCell = pCol->pItems[nIdx].pCell;
/*?*/ 				else
/*?*/ 					pCell = NULL;
/*N*/ 			}
/*N*/ 			if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/ 				((const ScFormulaCell*)pCell)->GetFormatInfo( nNumFmtType, nNumFmtIndex );
/*N*/ 			else
/*N*/ 				nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
/*N*/ 		bNumValid = TRUE;
/*N*/ 	}
/*N*/ 	nType = nNumFmtType;
/*N*/ 	nIndex = nNumFmtIndex;
/*N*/ }

/*N*/ BOOL ScValueIterator::GetFirst(double& rValue, USHORT& rErr)
/*N*/ {
/*N*/ 	nCol = nStartCol;
/*N*/ 	nRow = nStartRow;
/*N*/ 	nTab = nStartTab;
/*N*/ 
/*N*/ //	nColRow = 0;
/*N*/ 	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 	pCol->Search( nRow, nColRow );
/*N*/ 
/*N*/ 	nNumFormat = 0;					// werden bei GetNumberFormat initialisiert
/*N*/ 	pAttrArray = 0;
/*N*/ 	nAttrEndRow = 0;
/*N*/ 
/*N*/ 	return GetThis(rValue, rErr);
/*N*/ }

/*	ist inline:
BOOL ScValueIterator::GetNext(double& rValue, USHORT& rErr)
{
    ++nRow;
    return GetThis(rValue, rErr);
}
*/

//------------------------------------------------------------------------
//------------------------------------------------------------------------

/*N*/  ScQueryValueIterator::ScQueryValueIterator(ScDocument* pDocument, USHORT nTable, const ScQueryParam& rParam) :
/*N*/  	pDoc( pDocument ),
/*N*/  	nTab( nTable),
/*N*/  	aParam (rParam),
/*N*/  	nNumFmtType( NUMBERFORMAT_UNDEFINED ),
/*N*/  	nNumFmtIndex(0),
/*N*/  	bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() )
/*N*/  {
/*N*/  	nCol = aParam.nCol1;
/*N*/  	nRow = aParam.nRow1;
/*N*/  	nColRow = 0;					// wird bei GetFirst initialisiert
/*N*/  	USHORT i;
/*N*/  	USHORT nCount = aParam.GetEntryCount();
/*N*/  	for (i=0; (i<nCount) && (aParam.GetEntry(i).bDoQuery); i++)
/*N*/  	{
/*N*/  		ScQueryEntry& rEntry = aParam.GetEntry(i);
/*N*/  		sal_uInt32 nIndex = 0;
/*N*/  		rEntry.bQueryByString =
/*N*/  				!(pDoc->GetFormatTable()->IsNumberFormat(*rEntry.pStr, nIndex, rEntry.nVal));
/*N*/  	}
/*N*/  	nNumFormat = 0;					// werden bei GetNumberFormat initialisiert
/*N*/  	pAttrArray = 0;
/*N*/  	nAttrEndRow = 0;
/*N*/  }
/*N*/  
/*N*/  BOOL ScQueryValueIterator::GetThis(double& rValue, USHORT& rErr)
/*N*/  {
/*N*/  	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/      USHORT nFirstQueryField = aParam.GetEntry(0).nField;
/*N*/  	for ( ;; )
/*N*/  	{
/*N*/  		if ( nRow > aParam.nRow2 )
/*N*/  		{
/*N*/  			nRow = aParam.nRow1;
/*N*/  			if (aParam.bHasHeader)
/*N*/  				nRow++;
/*N*/  			do
/*N*/  			{
/*N*/  				nCol++;
/*N*/  				if ( nCol > aParam.nCol2 )
/*N*/  				{
/*N*/  					rValue = 0.0;
/*N*/  					rErr = 0;
/*N*/  					return FALSE;				// Ende und Aus
/*N*/  				}
/*N*/  				pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/  			} while ( pCol->nCount == 0 );
/*N*/  			pCol->Search( nRow, nColRow );
/*N*/  		}
/*N*/  
/*N*/  		while ( (nColRow < pCol->nCount) && (pCol->pItems[nColRow].nRow < nRow) )
/*N*/  			nColRow++;
/*N*/  
/*N*/  		if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= aParam.nRow2 )
/*N*/  		{
/*N*/  			nRow = pCol->pItems[nColRow].nRow;
/*N*/              ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
/*N*/              if ( (pDoc->pTab[nTab])->ValidQuery( nRow, aParam, NULL,
/*N*/                      (nCol == nFirstQueryField ? pCell : NULL) ) )
/*N*/  			{
/*N*/  				switch (pCell->GetCellType())
/*N*/  				{
/*N*/  					case CELLTYPE_VALUE:
/*N*/  						{
/*N*/  							rValue = ((ScValueCell*)pCell)->GetValue();
/*N*/  							if ( bCalcAsShown )
/*N*/  							{
/*N*/  #if ! ( defined WTC || defined ICC || defined HPUX || defined C50 || defined C52  || ( defined GCC && __GNUC__ >= 3 ) || ( defined WNT && _MSC_VER >= 1400 ) )
/*N*/  								lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
/*N*/  #else
/*N*/  								lcl_IterGetNumberFormat( nNumFormat,
/*N*/  									(ScAttrArray const *&)pAttrArray,
/*N*/  #endif
/*N*/  									nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
/*N*/  								rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
/*N*/  							}
/*N*/  							nNumFmtType = NUMBERFORMAT_NUMBER;
/*N*/  							nNumFmtIndex = 0;
/*N*/  							rErr = 0;
/*N*/  							return TRUE;		// gefunden
/*N*/  						}
/*N*/  						break;
/*N*/  					case CELLTYPE_FORMULA:
/*N*/  						{
/*N*/  							if (((ScFormulaCell*)pCell)->IsValue())
/*N*/  							{
/*N*/  								rValue = ((ScFormulaCell*)pCell)->GetValue();
/*N*/  								pDoc->GetNumberFormatInfo( nNumFmtType,
/*N*/  									nNumFmtIndex, ScAddress( nCol, nRow, nTab ),
/*N*/  									*((ScFormulaCell*)pCell) );
/*N*/  								rErr = ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/  								return TRUE;	// gefunden
/*N*/  							}
/*N*/  							else
/*N*/  								nRow++;
/*N*/  						}
/*N*/  						break;
/*N*/  					default:
/*N*/  						nRow++;
/*N*/  						break;
/*N*/  				}
/*N*/  			}
/*N*/  			else
/*N*/  				nRow++;
/*N*/  		}
/*N*/  		else
/*N*/  			nRow = aParam.nRow2 + 1; // Naechste Spalte
/*N*/  	}
/*N*/  	return FALSE;
/*N*/  }
/*N*/  
/*N*/  BOOL ScQueryValueIterator::GetFirst(double& rValue, USHORT& rErr)
/*N*/  {
/*N*/  	nCol = aParam.nCol1;
/*N*/  	nRow = aParam.nRow1;
/*N*/  	if (aParam.bHasHeader)
/*N*/  		nRow++;
/*N*/  //	nColRow = 0;
/*N*/  	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/  	pCol->Search( nRow, nColRow );
/*N*/  	return GetThis(rValue, rErr);
/*N*/  }
/*N*/  
/*N*/  BOOL ScQueryValueIterator::GetNext(double& rValue, USHORT& rErr)
/*N*/  {
/*N*/  	++nRow;
/*N*/  	return GetThis(rValue, rErr);
/*N*/  }

//-------------------------------------------------------------------------------

/*N*/ ScCellIterator::ScCellIterator( ScDocument* pDocument,
/*N*/ 								USHORT nSCol, USHORT nSRow, USHORT nSTab,
/*N*/ 								USHORT nECol, USHORT nERow, USHORT nETab, BOOL bSTotal ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nStartCol( nSCol),
/*N*/ 	nStartRow( nSRow),
/*N*/ 	nStartTab( nSTab ),
/*N*/ 	nEndCol( nECol ),
/*N*/ 	nEndRow( nERow),
/*N*/ 	nEndTab( nETab ),
/*N*/ 	bSubTotal(bSTotal)
/*N*/ 
/*N*/ {
/*N*/ 	PutInOrder( nStartCol, nEndCol);
/*N*/ 	PutInOrder( nStartRow, nEndRow);
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 
/*N*/ 	if (nStartCol > MAXCOL) nStartCol = MAXCOL;
/*N*/ 	if (nEndCol > MAXCOL) nEndCol = MAXCOL;
/*N*/ 	if (nStartRow > MAXROW) nStartRow = MAXROW;
/*N*/ 	if (nEndRow > MAXROW) nEndRow = MAXROW;
/*N*/ 	if (nStartTab > MAXTAB) nStartTab = MAXTAB;
/*N*/ 	if (nEndTab > MAXTAB) nEndTab = MAXTAB;
/*N*/ 
/*N*/ 	while (nEndTab>0 && !pDoc->pTab[nEndTab])
/*N*/ 		--nEndTab;										// nur benutzte Tabellen
/*N*/ 	if (nStartTab>nEndTab)
/*N*/ 		nStartTab = nEndTab;
/*N*/ 
/*N*/ 	nCol = nStartCol;
/*N*/ 	nRow = nStartRow;
/*N*/ 	nTab = nStartTab;
/*N*/ 	nColRow = 0;					// wird bei GetFirst initialisiert
/*N*/ 
/*N*/ 	if (!pDoc->pTab[nTab])
/*N*/ 	{
/*N*/ 		DBG_ERROR("Tabelle nicht gefunden");
/*N*/ 		nStartCol = nCol = MAXCOL+1;
/*N*/ 		nStartRow = nRow = MAXROW+1;
/*N*/ 		nStartTab = nTab = MAXTAB+1;	// -> Abbruch bei GetFirst
/*N*/ 	}
/*N*/ }

/*N*/ ScCellIterator::ScCellIterator
/*N*/ 	( ScDocument* pDocument, const ScRange& rRange, BOOL bSTotal ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nStartCol( rRange.aStart.Col() ),
/*N*/ 	nStartRow( rRange.aStart.Row() ),
/*N*/ 	nStartTab( rRange.aStart.Tab() ),
/*N*/ 	nEndCol( rRange.aEnd.Col() ),
/*N*/ 	nEndRow( rRange.aEnd.Row() ),
/*N*/ 	nEndTab( rRange.aEnd.Tab() ),
/*N*/ 	bSubTotal(bSTotal)
/*N*/ 
/*N*/ {
/*N*/ 	PutInOrder( nStartCol, nEndCol);
/*N*/ 	PutInOrder( nStartRow, nEndRow);
/*N*/ 	PutInOrder( nStartTab, nEndTab );
/*N*/ 
/*N*/ 	if (nStartCol > MAXCOL) nStartCol = MAXCOL;
/*N*/ 	if (nEndCol > MAXCOL) nEndCol = MAXCOL;
/*N*/ 	if (nStartRow > MAXROW) nStartRow = MAXROW;
/*N*/ 	if (nEndRow > MAXROW) nEndRow = MAXROW;
/*N*/ 	if (nStartTab > MAXTAB) nStartTab = MAXTAB;
/*N*/ 	if (nEndTab > MAXTAB) nEndTab = MAXTAB;
/*N*/ 
/*N*/ 	while (nEndTab>0 && !pDoc->pTab[nEndTab])
/*N*/ 		--nEndTab;										// nur benutzte Tabellen
/*N*/ 	if (nStartTab>nEndTab)
/*N*/ 		nStartTab = nEndTab;
/*N*/ 
/*N*/ 	nCol = nStartCol;
/*N*/ 	nRow = nStartRow;
/*N*/ 	nTab = nStartTab;
/*N*/ 	nColRow = 0;					// wird bei GetFirst initialisiert
/*N*/ 
/*N*/ 	if (!pDoc->pTab[nTab])
/*N*/ 	{
/*N*/ 		DBG_ERROR("Tabelle nicht gefunden");
/*N*/ 		nStartCol = nCol = MAXCOL+1;
/*N*/ 		nStartRow = nRow = MAXROW+1;
/*N*/ 		nStartTab = nTab = MAXTAB+1;	// -> Abbruch bei GetFirst
/*N*/ 	}
/*N*/ }

/*N*/ ScBaseCell* ScCellIterator::GetThis()
/*N*/ {
/*N*/ 	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 	for ( ;; )
/*N*/ 	{
/*N*/ 		if ( nRow > nEndRow )
/*N*/ 		{
/*N*/ 			nRow = nStartRow;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				nCol++;
/*N*/ 				if ( nCol > nEndCol )
/*N*/ 				{
/*N*/ 					nCol = nStartCol;
/*N*/ 					nTab++;
/*N*/ 					if ( nTab > nEndTab )
/*N*/ 						return NULL;				// Ende und Aus
/*N*/ 				}
/*N*/ 				pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 			} while ( pCol->nCount == 0 );
/*N*/ 			pCol->Search( nRow, nColRow );
/*N*/ 		}
/*N*/ 
/*N*/ 		while ( (nColRow < pCol->nCount) && (pCol->pItems[nColRow].nRow < nRow) )
/*N*/ 			nColRow++;
/*N*/ 
/*N*/ 		if ( nColRow < pCol->nCount	&& pCol->pItems[nColRow].nRow <= nEndRow )
/*N*/ 		{
/*N*/ 			nRow = pCol->pItems[nColRow].nRow;
/*N*/ 			if ( !bSubTotal || !pDoc->pTab[nTab]->IsFiltered( nRow ) )
/*N*/ 			{
/*N*/ 				ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
/*N*/ 
/*N*/ 				if ( bSubTotal && pCell->GetCellType() == CELLTYPE_FORMULA
/*N*/ 								&& ((ScFormulaCell*)pCell)->IsSubTotal() )
/*N*/ 					nRow++;				// Sub-Total-Zeilen nicht
/*N*/ 				else
/*N*/ 					return pCell;		// gefunden
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nRow++;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nRow = nEndRow + 1; // Naechste Spalte
/*N*/ 	}
/*N*/ }

/*N*/ ScBaseCell* ScCellIterator::GetFirst()
/*N*/ {
/*N*/ 	if ( nTab > MAXTAB )
/*N*/ 		return NULL;
/*N*/ 	nCol = nStartCol;
/*N*/ 	nRow = nStartRow;
/*N*/ 	nTab = nStartTab;
/*N*/ //	nColRow = 0;
/*N*/ 	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 	pCol->Search( nRow, nColRow );
/*N*/ 	return GetThis();
/*N*/ }

/*N*/ ScBaseCell* ScCellIterator::GetNext()
/*N*/ {
/*N*/ 	++nRow;
/*N*/ 	return GetThis();
/*N*/ }

//-------------------------------------------------------------------------------

/*N*/ ScQueryCellIterator::ScQueryCellIterator(ScDocument* pDocument, USHORT nTable,
/*N*/ 			 const ScQueryParam& rParam, BOOL bMod ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nTab( nTable),
/*N*/ 	aParam (rParam),
/*N*/     nStopOnMismatch( nStopOnMismatchDisabled ),
/*N*/     nTestEqualCondition( nTestEqualConditionDisabled ),
/*N*/     bAdvanceQuery( FALSE )
/*N*/ {
/*N*/ 	nCol = aParam.nCol1;
/*N*/ 	nRow = aParam.nRow1;
/*N*/ 	nColRow = 0;					// wird bei GetFirst initialisiert
/*N*/ 	USHORT i;
/*N*/ 	if (bMod)								// sonst schon eingetragen
/*N*/ 	{
/*N*/ 		for (i=0; (i<MAXQUERY) && (aParam.GetEntry(i).bDoQuery); i++)
/*N*/ 		{
/*?*/ 			ScQueryEntry& rEntry = aParam.GetEntry(i);
/*?*/ 			sal_uInt32 nIndex = 0;
/*?*/ 			rEntry.bQueryByString =
/*?*/ 					 !(pDoc->GetFormatTable()->IsNumberFormat(*rEntry.pStr,
/*?*/ 															  nIndex, rEntry.nVal));
/*N*/ 		}
/*N*/ 	}
/*N*/ 	nNumFormat = 0;					// werden bei GetNumberFormat initialisiert
/*N*/ 	pAttrArray = 0;
/*N*/ 	nAttrEndRow = 0;
/*N*/ }
/*N*/ 
/*N*/ ScBaseCell* ScQueryCellIterator::GetThis()
/*N*/ {
/*N*/ 	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/     USHORT nFirstQueryField = aParam.GetEntry(0).nField;
/*N*/ 	for ( ;; )
/*N*/ 	{
/*N*/ 		if ( nRow > aParam.nRow2 )
/*N*/ 		{
/*N*/ 			nRow = aParam.nRow1;
/*N*/ 			if (aParam.bHasHeader)
/*?*/ 				nRow++;
/*N*/ 			do
/*M*/ 			{
/*M*/ 				if ( ++nCol > aParam.nCol2 )
/*M*/ 					return NULL;				// Ende und Aus
/*M*/ 				if ( bAdvanceQuery )
/*M*/                 {
/*M*/ 					 AdvanceQueryParamEntryField();
/*M*/                     nFirstQueryField = aParam.GetEntry(0).nField;
/*M*/                 }
/*M*/ 				pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*M*/ 			} while ( pCol->nCount == 0 );
/*N*/ 			pCol->Search( nRow, nColRow );
/*N*/ 		}
/*N*/ 
/*N*/ 		while ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow < nRow )
/*N*/ 			nColRow++;
/*N*/ 
/*N*/ 		if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= aParam.nRow2 )
/*N*/ 		{
/*N*/ 			if ( pCol->pItems[nColRow].pCell->GetCellType() == CELLTYPE_NOTE )
/*?*/ 				nRow++;
/*N*/ 			else
/*N*/ 			{
/*N*/                 BOOL bTestEqualCondition;
/*N*/ 				nRow = pCol->pItems[nColRow].nRow;
/*N*/                 ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
/*N*/                 if ( (pDoc->pTab[nTab])->ValidQuery( nRow, aParam, NULL,
/*N*/                         (nCol == nFirstQueryField ? pCell : NULL),
/*N*/                         (nTestEqualCondition ? &bTestEqualCondition : NULL) ) )
/*N*/                 {
/*N*/                     if ( nTestEqualCondition && bTestEqualCondition )
/*N*/                         nTestEqualCondition |= nTestEqualConditionMatched;
/*N*/                     return pCell;     // found
/*N*/                 }
/*N*/                 else if ( nStopOnMismatch )
/*N*/                 {
/*N*/                     nStopOnMismatch |= nStopOnMismatchOccured;
/*N*/                     // Yes, even a mismatch may have a fulfilled equal
/*N*/                     // condition if regular expressions were involved and
/*N*/                     // SC_LESS_EQUAL or SC_GREATER_EQUAL were queried.
/*N*/                     if ( nTestEqualCondition && bTestEqualCondition )
/*?*/                         nTestEqualCondition |= nTestEqualConditionMatched;
/*N*/                     return NULL;
/*N*/                 }
/*N*/ 				else
/*N*/ 					nRow++;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			nRow = aParam.nRow2 + 1; // Naechste Spalte
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ ScBaseCell* ScQueryCellIterator::GetFirst()
/*N*/ {
/*N*/ 	nCol = aParam.nCol1;
/*N*/ 	nRow = aParam.nRow1;
/*N*/ 	if (aParam.bHasHeader)
/*?*/ 		nRow++;
/*N*/ //	nColRow = 0;
/*N*/ 	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/ 	pCol->Search( nRow, nColRow );
/*N*/ 	return GetThis();
/*N*/ }
/*N*/ 
/*N*/ ScBaseCell* ScQueryCellIterator::GetNext()
/*N*/ {
/*N*/ 	++nRow;
/*N*/     if ( nStopOnMismatch )
/*N*/         nStopOnMismatch = nStopOnMismatchEnabled;
/*N*/     if ( nTestEqualCondition )
/*N*/         nTestEqualCondition = nTestEqualConditionEnabled;
/*N*/ 	return GetThis();
/*N*/ }
/*N*/ 
/*N*/  ULONG ScQueryCellIterator::GetNumberFormat()
/*N*/  {
/*N*/  	ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
/*N*/  #if ! ( defined WTC || defined ICC || defined HPUX || defined C50 || defined C52 || ( defined GCC && __GNUC__ >= 3 ) || ( defined WNT && _MSC_VER >= 1400 ) )
/*N*/  	lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
/*N*/  #else
/*N*/  	lcl_IterGetNumberFormat( nNumFormat,
/*N*/  		(ScAttrArray const *&)pAttrArray,
/*N*/  #endif
/*N*/  		nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
/*N*/  	return nNumFormat;
/*N*/  }

/*N*/  void ScQueryCellIterator::AdvanceQueryParamEntryField()
/*N*/  {
/*N*/  	USHORT nEntries = aParam.GetEntryCount();
/*N*/  	for ( USHORT j = 0; j < nEntries; j++  )
/*N*/  	{
/*N*/  		ScQueryEntry& rEntry = aParam.GetEntry( j );
/*N*/  		if ( rEntry.bDoQuery )
/*N*/  		{
/*N*/  			if ( rEntry.nField < MAXCOL )
/*N*/  				rEntry.nField++;
/*N*/  			else
/*N*/  			{
/*N*/  				DBG_ERRORFILE( "AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL" );
/*N*/  			}
/*N*/  		}
/*N*/  		else
/*N*/  			break;	// for
/*N*/  	}
/*N*/  }


/*N*/ BOOL ScQueryCellIterator::FindEqualOrSortedLastInRange( USHORT& nFoundCol, USHORT& nFoundRow )
/*N*/ {
/*N*/     nFoundCol = MAXCOL+1;
/*N*/     nFoundRow = MAXROW+1;
/*N*/     SetStopOnMismatch( TRUE );      // assume sorted keys
/*N*/     SetTestEqualCondition( TRUE );
/*N*/     if ( GetFirst() )
/*N*/     {
/*N*/         do
/*N*/         {
/*N*/             nFoundCol = GetCol();
/*N*/             nFoundRow = GetRow();
/*N*/         } while ( !IsEqualConditionFulfilled() && GetNext() );
/*N*/     }
/*N*/     if ( IsEqualConditionFulfilled() )
/*N*/     {
/*N*/         nFoundCol = GetCol();
/*N*/         nFoundRow = GetRow();
/*N*/         return TRUE;
/*N*/     }
/*N*/     if ( StoppedOnMismatch() )
/*N*/     {   // Assume found entry to be the last value less than or equal to query.
/*N*/         // But keep on searching for an equal match.
/*N*/         SetStopOnMismatch( FALSE );
/*N*/         SetTestEqualCondition( FALSE );
/*N*/         USHORT nEntries = aParam.GetEntryCount();
/*N*/         for ( USHORT j = 0; j < nEntries; j++  )
/*N*/         {
/*N*/             ScQueryEntry& rEntry = aParam.GetEntry( j );
/*N*/             if ( rEntry.bDoQuery )
/*N*/             {
/*N*/                 switch ( rEntry.eOp )
/*N*/                 {
/*N*/                     case SC_LESS_EQUAL :
/*N*/                     case SC_GREATER_EQUAL :
/*N*/                         rEntry.eOp = SC_EQUAL;
/*N*/                     break;
/*N*/                 }
/*N*/             }
/*N*/             else
/*N*/                 break;  // for
/*N*/         }
/*N*/         if ( GetNext() )
/*N*/         {
/*?*/             nFoundCol = GetCol();
/*?*/             nFoundRow = GetRow();
/*N*/         }
/*N*/     }
/*N*/     return (nFoundCol <= MAXCOL) && (nFoundRow <= MAXROW);
/*N*/ }


//-------------------------------------------------------------------------------

/*N*/ ScHorizontalCellIterator::ScHorizontalCellIterator(ScDocument* pDocument, USHORT nTable,
/*N*/ 									USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nTab( nTable ),
/*N*/ 	nStartCol( nCol1 ),
/*N*/ 	nEndCol( nCol2 ),
/*N*/ 	nEndRow( nRow2 ),
/*N*/ 	nCol( nCol1 ),
/*N*/ 	nRow( nRow1 ),
/*N*/ 	bMore( TRUE )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	USHORT nIndex;
/*N*/ 
/*N*/ 	pNextRows = new USHORT[ nCol2-nCol1+1 ];
/*N*/ 	pNextIndices = new USHORT[ nCol2-nCol1+1 ];
/*N*/ 
/*N*/ 	for (i=nStartCol; i<=nEndCol; i++)
/*N*/ 	{
/*N*/ 		ScColumn* pCol = &pDoc->pTab[nTab]->aCol[i];
/*N*/ 
/*N*/ 		pCol->Search( nRow1, nIndex );
/*N*/ 		if ( nIndex < pCol->nCount )
/*N*/ 		{
/*N*/ 			pNextRows[i-nStartCol] = pCol->pItems[nIndex].nRow;
/*N*/ 			pNextIndices[i-nStartCol] = nIndex;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pNextRows[i-nStartCol] = MAXROW+1;		// nichts gefunden
/*N*/ 			pNextIndices[i-nStartCol] = MAXROW+1;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pNextRows[0] != nRow1)
/*N*/ 		Advance();
/*N*/ }

/*N*/ ScHorizontalCellIterator::~ScHorizontalCellIterator()
/*N*/ {
/*N*/ 	delete [] pNextRows;
/*N*/ 	delete [] pNextIndices;
/*N*/ }

/*N*/ ScBaseCell* ScHorizontalCellIterator::GetNext( USHORT& rCol, USHORT& rRow )
/*N*/ {
/*N*/ 	if ( bMore )
/*N*/ 	{
/*N*/ 		rCol = nCol;
/*N*/ 		rRow = nRow;
/*N*/ 
/*N*/ 		ScColumn* pCol = &pDoc->pTab[nTab]->aCol[nCol];
/*N*/ 		USHORT nIndex = pNextIndices[nCol-nStartCol];
/*N*/ 		DBG_ASSERT( nIndex < pCol->nCount, "ScHorizontalCellIterator::GetNext: nIndex out of range" );
/*N*/ 		ScBaseCell* pCell = pCol->pItems[nIndex].pCell;
/*N*/ 		if ( ++nIndex < pCol->nCount )
/*N*/ 		{
/*N*/ 			pNextRows[nCol-nStartCol] = pCol->pItems[nIndex].nRow;
/*N*/ 			pNextIndices[nCol-nStartCol] = nIndex;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pNextRows[nCol-nStartCol] = MAXROW+1;		// nichts gefunden
/*N*/ 			pNextIndices[nCol-nStartCol] = MAXROW+1;
/*N*/ 		}
/*N*/ 
/*N*/ 		Advance();
/*N*/ 		return pCell;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ BOOL ScHorizontalCellIterator::ReturnNext( USHORT& rCol, USHORT& rRow )
/*N*/ {
/*N*/ 	rCol = nCol;
/*N*/ 	rRow = nRow;
/*N*/ 	return bMore;
/*N*/ }

/*N*/ void ScHorizontalCellIterator::Advance()
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for (i=nCol+1; i<=nEndCol && !bFound; i++)
/*N*/ 		if (pNextRows[i-nStartCol] == nRow)
/*N*/ 		{
/*N*/ 			nCol = i;
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/ 	if (!bFound)
/*N*/ 	{
/*N*/ 		USHORT nMinRow = MAXROW+1;
/*N*/ 		for (i=nStartCol; i<=nEndCol; i++)
/*N*/ 			if (pNextRows[i-nStartCol] < nMinRow)
/*N*/ 			{
/*N*/ 				nCol = i;
/*N*/ 				nMinRow = pNextRows[i-nStartCol];
/*N*/ 			}
/*N*/ 
/*N*/ 		if (nMinRow <= nEndRow)
/*N*/ 		{
/*N*/ 			nRow = nMinRow;
/*N*/ 			bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bFound )
/*N*/ 		bMore = FALSE;
/*N*/ }

//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------

/*N*/ ScDocAttrIterator::ScDocAttrIterator(ScDocument* pDocument, USHORT nTable,
/*N*/ 									USHORT nCol1, USHORT nRow1,
/*N*/ 									USHORT nCol2, USHORT nRow2) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nTab( nTable ),
/*N*/ 	nCol( nCol1 ),
/*N*/ 	nEndCol( nCol2 ),
/*N*/ 	nStartRow( nRow1 ),
/*N*/ 	nEndRow( nRow2 )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pDoc->pTab[nTab] )
/*N*/ 		pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
/*N*/ 	else
/*N*/ 		pColIter = NULL;
/*N*/ }

/*N*/ ScDocAttrIterator::~ScDocAttrIterator()
/*N*/ {
/*N*/ 	delete pColIter;
/*N*/ }

/*N*/ const ScPatternAttr* ScDocAttrIterator::GetNext( USHORT& rCol, USHORT& rRow1, USHORT& rRow2 )
/*N*/ {
/*N*/ 	while ( pColIter )
/*N*/ 	{
/*N*/ 		const ScPatternAttr* pPattern = pColIter->Next( rRow1, rRow2 );
/*N*/ 		if ( pPattern )
/*N*/ 		{
/*N*/ 			rCol = nCol;
/*N*/ 			return pPattern;
/*N*/ 		}
/*N*/ 
/*N*/ 		delete pColIter;
/*N*/ 		++nCol;
/*N*/ 		if ( nCol <= nEndCol )
/*N*/ 			pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
/*N*/ 		else
/*N*/ 			pColIter = NULL;
/*N*/ 	}
/*N*/ 	return NULL;		// is nix mehr
/*N*/ }

//-------------------------------------------------------------------------------

/*N*/ ScAttrRectIterator::ScAttrRectIterator(ScDocument* pDocument, USHORT nTable,
/*N*/ 									USHORT nCol1, USHORT nRow1,
/*N*/ 									USHORT nCol2, USHORT nRow2) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	nTab( nTable ),
/*N*/ 	nEndCol( nCol2 ),
/*N*/ 	nStartRow( nRow1 ),
/*N*/ 	nEndRow( nRow2 ),
/*N*/ 	nIterStartCol( nCol1 ),
/*N*/ 	nIterEndCol( nCol1 )
/*N*/ {
/*N*/ 	if ( nTab<=MAXTAB && pDoc->pTab[nTab] )
/*N*/ 	{
/*N*/ 		pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
/*N*/ 		while ( nIterEndCol < nEndCol &&
/*N*/ 				pDoc->pTab[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
/*N*/ 					pDoc->pTab[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
/*N*/ 			++nIterEndCol;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pColIter = NULL;
/*N*/ }

/*N*/ ScAttrRectIterator::~ScAttrRectIterator()
/*N*/ {
/*N*/ 	delete pColIter;
/*N*/ }

/*N*/ void ScAttrRectIterator::DataChanged()
/*N*/ {
/*N*/ 	if (pColIter)
/*N*/ 	{
/*N*/ 		USHORT nNextRow = pColIter->GetNextRow();
/*N*/ 		delete pColIter;
/*N*/ 		pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nNextRow, nEndRow );
/*N*/ 	}
/*N*/ }

/*N*/ const ScPatternAttr* ScAttrRectIterator::GetNext( USHORT& rCol1, USHORT& rCol2,
/*N*/ 													USHORT& rRow1, USHORT& rRow2 )
/*N*/ {
/*N*/ 	while ( pColIter )
/*N*/ 	{
/*N*/ 		const ScPatternAttr* pPattern = pColIter->Next( rRow1, rRow2 );
/*N*/ 		if ( pPattern )
/*N*/ 		{
/*N*/ 			rCol1 = nIterStartCol;
/*N*/ 			rCol2 = nIterEndCol;
/*N*/ 			return pPattern;
/*N*/ 		}
/*N*/ 
/*N*/ 		delete pColIter;
/*N*/ 		nIterStartCol = nIterEndCol+1;
/*N*/ 		if ( nIterStartCol <= nEndCol )
/*N*/ 		{
/*N*/ 			nIterEndCol = nIterStartCol;
/*N*/ 			pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
/*N*/ 			while ( nIterEndCol < nEndCol &&
/*N*/ 					pDoc->pTab[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
/*N*/ 						pDoc->pTab[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
/*N*/ 				++nIterEndCol;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pColIter = NULL;
/*N*/ 	}
/*N*/ 	return NULL;		// is nix mehr
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
