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

#include <tools/debug.hxx>
#include <math.h>
#include <string.h>
#include "consoli.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "subtotal.hxx"
#include "cell.hxx"
namespace binfilter {

#define SC_CONS_NOTFOUND	0xFFFF

// STATIC DATA -----------------------------------------------------------

/*	Strings bei Gelegenheit ganz raus...
static USHORT nFuncRes[] = {				//	Reihenfolge wie bei enum ScSubTotalFunc
        0,									//	none
        STR_PIVOTFUNC_AVG,
        STR_PIVOTFUNC_COUNT,
        STR_PIVOTFUNC_COUNT2,
        STR_PIVOTFUNC_MAX,
        STR_PIVOTFUNC_MIN,
        STR_PIVOTFUNC_PROD,
        STR_PIVOTFUNC_STDDEV,
        STR_PIVOTFUNC_STDDEV2,
        STR_PIVOTFUNC_SUM,
        STR_PIVOTFUNC_VAR,
        STR_PIVOTFUNC_VAR2 };
*/

/*N*/ static OpCode eOpCodeTable[] = {			//	Reihenfolge wie bei enum ScSubTotalFunc
/*N*/ 		ocBad,								//	none
/*N*/ 		ocAverage,
/*N*/ 		ocCount,
/*N*/ 		ocCount2,
/*N*/ 		ocMax,
/*N*/ 		ocMin,
/*N*/ 		ocProduct,
/*N*/ 		ocStDev,
/*N*/ 		ocStDevP,
/*N*/ 		ocSum,
/*N*/ 		ocVar,
/*N*/ 		ocVarP };

// -----------------------------------------------------------------------

/*N*/ void lcl_AddString( String**& pData, USHORT& nCount, const String& rInsert )
/*N*/ {
/*N*/ 	String** pOldData = pData;
/*N*/ 	pData = new String*[ nCount+1 ];
/*N*/ 	if (pOldData)
/*N*/ 	{
/*N*/ 		memmove( pData, pOldData, nCount * sizeof(String*) );
/*N*/ 		delete[] pOldData;
/*N*/ 	}
/*N*/ 	pData[nCount] = new String(rInsert);
/*N*/ 	++nCount;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ ScConsData::ScConsData() :
/*N*/ 	eFunction(SUBTOTAL_FUNC_SUM),
/*N*/ 	bReference(FALSE),
/*N*/ 	bColByName(FALSE),
/*N*/ 	bRowByName(FALSE),
/*N*/ 	bSubTitles(FALSE),
/*N*/ 	nColCount(0),
/*N*/ 	ppColHeaders(NULL),
/*N*/ 	nRowCount(0),
/*N*/ 	ppRowHeaders(NULL),
/*N*/ 	ppCount(NULL),
/*N*/ 	ppSum(NULL),
/*N*/ 	ppSumSqr(NULL),
/*N*/ 	ppRefs(NULL),
/*N*/ 	ppUsed(NULL),
/*N*/ 	nDataCount(0),
/*N*/ 	nTitleCount(0),
/*N*/ 	ppTitles(NULL),
/*N*/ 	ppTitlePos(NULL),
/*N*/ 	bCornerUsed(FALSE)
/*N*/ {
/*N*/ }

/*N*/ ScConsData::~ScConsData()
/*N*/ {
/*N*/ 	DeleteData();
/*N*/ }


/*N*/ #define DELETEARR(ppArray,nCount)	\
/*N*/ {									\
/*N*/ 	USHORT i; 						\
/*N*/ 	if (ppArray) 					\
/*N*/ 		for(i=0; i<nCount; i++)		\
/*N*/ 			delete[] ppArray[i];	\
/*N*/ 	delete[] ppArray;				\
/*N*/ 	ppArray = NULL;					\
/*N*/ }
/*N*/ 
/*N*/ #define DELETESTR(ppArray,nCount)	\
/*N*/ {									\
/*N*/ 	USHORT i; 						\
/*N*/ 	if (ppArray) 					\
/*N*/ 		for(i=0; i<nCount; i++)		\
/*N*/ 			delete ppArray[i];		\
/*N*/ 	delete[] ppArray;				\
/*N*/ 	ppArray = NULL;					\
/*N*/ }

/*N*/ void ScConsData::DeleteData()
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	if (ppRefs)
/*N*/ 		for (i=0; i<nColCount; i++)
/*N*/ 		{
/*N*/ 			for (USHORT j=0; j<nRowCount; j++)
/*N*/ 				if (ppUsed[i][j])
/*N*/ 					ppRefs[i][j].Clear();
/*N*/ 			delete[] ppRefs[i];
/*N*/ 		}
/*N*/ 	delete[] ppRefs;
/*N*/ 	ppRefs = NULL;
/*N*/ 
/*N*/ //	DELETEARR( ppData1, nColCount );
/*N*/ //	DELETEARR( ppData2, nColCount );
/*N*/ 	DELETEARR( ppCount, nColCount );
/*N*/ 	DELETEARR( ppSum,   nColCount );
/*N*/ 	DELETEARR( ppSumSqr,nColCount );
/*N*/ 	DELETEARR( ppUsed,  nColCount );				// erst nach ppRefs !!!
/*N*/ 	DELETEARR( ppTitlePos, nRowCount );
/*N*/ 	DELETESTR( ppColHeaders, nColCount );
/*N*/ 	DELETESTR( ppRowHeaders, nRowCount );
/*N*/ 	DELETESTR( ppTitles, nTitleCount );
/*N*/ 	nTitleCount = 0;
/*N*/ 	nDataCount = 0;
/*N*/ 
/*N*/ 	if (bColByName) nColCount = 0;					// sonst stimmt ppColHeaders nicht
/*N*/ 	if (bRowByName) nRowCount = 0;
/*N*/ 
/*N*/ 	bCornerUsed = FALSE;
/*N*/ 	aCornerText.Erase();
/*N*/ }

/*N*/ #undef DELETEARR
/*N*/ #undef DELETESTR

/*N*/ void ScConsData::InitData( BOOL bDelete )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	if (bDelete)
/*N*/ 		DeleteData();
/*N*/ 
/*N*/ 	if (bReference && nColCount && !ppRefs)
/*N*/ 	{
/*N*/ 		ppRefs = new ScReferenceList*[nColCount];
/*N*/ 		for (i=0; i<nColCount; i++)
/*N*/ 			ppRefs[i] = new ScReferenceList[nRowCount];
/*N*/ 	}
/*N*/ 	else if (nColCount && !ppCount)
/*N*/ 	{
/*N*/ 		ppCount  = new double*[nColCount];
/*N*/ 		ppSum    = new double*[nColCount];
/*N*/ 		ppSumSqr = new double*[nColCount];
/*N*/ 		for (i=0; i<nColCount; i++)
/*N*/ 		{
/*N*/ 			ppCount[i]  = new double[nRowCount];
/*N*/ 			ppSum[i]    = new double[nRowCount];
/*N*/ 			ppSumSqr[i] = new double[nRowCount];
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nColCount && !ppUsed)
/*N*/ 	{
/*N*/ 		ppUsed = new BOOL*[nColCount];
/*N*/ 		for (i=0; i<nColCount; i++)
/*N*/ 		{
/*N*/ 			ppUsed[i] = new BOOL[nRowCount];
/*N*/ 			memset( ppUsed[i], 0, nRowCount * sizeof(BOOL) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (nRowCount && nDataCount && !ppTitlePos)
/*N*/ 	{
/*N*/ 		ppTitlePos = new USHORT*[nRowCount];
/*N*/ 		for (i=0; i<nRowCount; i++)
/*N*/ 		{
/*N*/ 			ppTitlePos[i] = new USHORT[nDataCount];
/*N*/ 			memset( ppTitlePos[i], 0, nDataCount * sizeof(USHORT) );	//! unnoetig ?
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//	CornerText: einzelner String
/*N*/ }

/*N*/ void ScConsData::DoneFields()
/*N*/ {
/*N*/ 	InitData(FALSE);
/*N*/ }

/*N*/ void ScConsData::SetSize( USHORT nCols, USHORT nRows )
/*N*/ {
/*N*/ 	DeleteData();
/*N*/ 	nColCount = nCols;
/*N*/ 	nRowCount = nRows;
/*N*/ }

/*N*/ void ScConsData::GetSize( USHORT& rCols, USHORT& rRows ) const
/*N*/ {
/*N*/ 	rCols = nColCount;
/*N*/ 	rRows = nRowCount;
/*N*/ }

/*N*/ void ScConsData::SetFlags( ScSubTotalFunc eFunc, BOOL bColName, BOOL bRowName, BOOL bRef )
/*N*/ {
/*N*/ 	DeleteData();
/*N*/ 	bReference = bRef;
/*N*/ 	bColByName = bColName;
/*N*/ 	if (bColName) nColCount = 0;
/*N*/ 	bRowByName = bRowName;
/*N*/ 	if (bRowName) nRowCount = 0;
/*N*/ 	eFunction = eFunc;
/*N*/ }

/*N*/ void ScConsData::AddFields( ScDocument* pSrcDoc, USHORT nTab,
/*N*/ 							USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
/*N*/ {
/*N*/ 	++nDataCount;
/*N*/ 
/*N*/ 	String aTitle;
/*N*/ 
/*N*/ 	USHORT nStartCol = nCol1;
/*N*/ 	USHORT nStartRow = nRow1;
/*N*/ 	if (bColByName)	++nStartRow;
/*N*/ 	if (bRowByName)	++nStartCol;
/*N*/ 
/*N*/ 	if (bColByName)
/*N*/ 	{
/*N*/ 		for (USHORT nCol=nStartCol; nCol<=nCol2; nCol++)
/*N*/ 		{
/*N*/ 			pSrcDoc->GetString( nCol, nRow1, nTab, aTitle );
/*N*/ 			if (aTitle.Len())
/*N*/ 			{
/*N*/ 				BOOL bFound = FALSE;
/*N*/ 				for (USHORT i=0; i<nColCount && !bFound; i++)
/*N*/ 					if ( *ppColHeaders[i] == aTitle )
/*N*/ 						bFound = TRUE;
/*N*/ 				if (!bFound)
/*N*/ 					lcl_AddString( ppColHeaders, nColCount, aTitle );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bRowByName)
/*N*/ 	{
/*N*/ 		for (USHORT nRow=nStartRow; nRow<=nRow2; nRow++)
/*N*/ 		{
/*N*/ 			pSrcDoc->GetString( nCol1, nRow, nTab, aTitle );
/*N*/ 			if (aTitle.Len())
/*N*/ 			{
/*N*/ 				BOOL bFound = FALSE;
/*N*/ 				for (USHORT i=0; i<nRowCount && !bFound; i++)
/*N*/ 					if ( *ppRowHeaders[i] == aTitle )
/*N*/ 						bFound = TRUE;
/*N*/ 				if (!bFound)
/*N*/ 					lcl_AddString( ppRowHeaders, nRowCount, aTitle );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScConsData::AddName( const String& rName )
/*N*/ {
/*N*/ 	USHORT nArrX;
/*N*/ 	USHORT nArrY;
/*N*/ 
/*N*/ 	if (bReference)
/*N*/ 	{
/*N*/ 		lcl_AddString( ppTitles, nTitleCount, rName );
/*N*/ 
/*N*/ 		for (nArrY=0; nArrY<nRowCount; nArrY++)
/*N*/ 		{
/*N*/ 			//	Daten auf gleiche Laenge bringen
/*N*/ 
/*N*/ 			USHORT nMax = 0;
/*N*/ 			for (nArrX=0; nArrX<nColCount; nArrX++)
/*N*/ 				if (ppUsed[nArrX][nArrY])
/*N*/ 					nMax = Max( nMax, ppRefs[nArrX][nArrY].GetCount() );
/*N*/ 
/*N*/ 			for (nArrX=0; nArrX<nColCount; nArrX++)
/*N*/ 			{
/*N*/ 				if (!ppUsed[nArrX][nArrY])
/*N*/ 				{
/*N*/ 					ppUsed[nArrX][nArrY] = TRUE;
/*N*/ 					ppRefs[nArrX][nArrY].Init();
/*N*/ 				}
/*N*/ 				ppRefs[nArrX][nArrY].SetFullSize(nMax);
/*N*/ 			}
/*N*/ 
/*N*/ 			//	Positionen eintragen
/*N*/ 
/*N*/ 			if (ppTitlePos)
/*N*/ 				if (nTitleCount < nDataCount)
/*N*/ 					ppTitlePos[nArrY][nTitleCount] = nMax;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ double lcl_CalcData( ScSubTotalFunc eFunc,
/*N*/ 						double fCount, double fSum, double fSumSqr)
/*N*/ {
/*N*/ 	if (fCount < 0.0)
/*N*/ 		return 0.0;
/*N*/ 	double fVal = 0.0;
/*N*/ 	switch (eFunc)
/*N*/ 	{
/*N*/ 		case SUBTOTAL_FUNC_CNT:
/*N*/ 		case SUBTOTAL_FUNC_CNT2:
/*N*/ 			fVal = fCount;
/*N*/ 			break;
/*N*/ 		case SUBTOTAL_FUNC_SUM:
/*N*/ 		case SUBTOTAL_FUNC_MAX:
/*N*/ 		case SUBTOTAL_FUNC_MIN:
/*N*/ 		case SUBTOTAL_FUNC_PROD:
/*N*/ 			fVal = fSum;
/*N*/ 			break;
/*N*/ 		case SUBTOTAL_FUNC_AVE:
/*N*/ 			if (fCount > 0.0)
/*N*/ 				fVal = fSum / fCount;
/*N*/ 			else
/*N*/ 				fCount = -MAXDOUBLE;
/*N*/ 			break;
/*N*/ 		case SUBTOTAL_FUNC_STD:
/*N*/ 		{
/*N*/ 			if (fCount > 1 && SubTotal::SafeMult(fSum, fSum))
/*N*/ 				fVal = sqrt((fSumSqr - fSum/fCount)/(fCount-1.0));
/*N*/ 			else
/*N*/ 				fCount = -MAXDOUBLE;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case SUBTOTAL_FUNC_STDP:
/*N*/ 		{
/*N*/ 			if (fCount > 0 && SubTotal::SafeMult(fSum, fSum))
/*N*/ 				fVal = sqrt((fSumSqr - fSum/fCount)/fCount);
/*N*/ 			else
/*N*/ 				fCount = -MAXDOUBLE;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case SUBTOTAL_FUNC_VAR:
/*N*/ 		{
/*N*/ 			if (fCount > 1 && SubTotal::SafeMult(fSum, fSum))
/*N*/ 				fVal = (fSumSqr - fSum/fCount)/(fCount-1.0);
/*N*/ 			else
/*N*/ 				fCount = -MAXDOUBLE;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case SUBTOTAL_FUNC_VARP:
/*N*/ 		{
/*N*/ 			if (fCount > 0 && SubTotal::SafeMult(fSum, fSum))
/*N*/ 				fVal = (fSumSqr - fSum/fCount)/fCount;
/*N*/ 			else
/*N*/ 				fCount = -MAXDOUBLE;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 		{
/*N*/ 			DBG_ERROR("unbekannte Funktion bei Consoli::CalcData");
/*N*/ 			fCount = -MAXDOUBLE;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return fVal;
/*N*/ }

/*N*/ void ScConsData::AddData( ScDocument* pSrcDoc, USHORT nTab,
/*N*/ 							USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
/*N*/ {
/*N*/ 	PutInOrder(nCol1,nCol2);
/*N*/ 	PutInOrder(nRow1,nRow2);
/*N*/ 	if ( nCol2 >= nCol1 + nColCount && !bColByName )
/*N*/ 	{
/*N*/ 		DBG_ASSERT(0,"Bereich zu gross");
/*N*/ 		nCol2 = nCol1 + nColCount - 1;
/*N*/ 	}
/*N*/ 	if ( nRow2 >= nRow1 + nRowCount && !bRowByName )
/*N*/ 	{
/*N*/ 		DBG_ASSERT(0,"Bereich zu gross");
/*N*/ 		nRow2 = nRow1 + nRowCount - 1;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nCol;
/*N*/ 	USHORT nRow;
/*N*/ 
/*N*/ 	//		Ecke links oben
/*N*/ 
/*N*/ 	if ( bColByName && bRowByName )
/*N*/ 	{
/*N*/ 		String aThisCorner;
/*N*/ 		pSrcDoc->GetString(nCol1,nRow1,nTab,aThisCorner);
/*N*/ 		if (bCornerUsed)
/*N*/ 		{
/*N*/ 			if (aCornerText != aThisCorner)
/*N*/ 				aCornerText.Erase();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aCornerText = aThisCorner;
/*N*/ 			bCornerUsed = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//		Titel suchen
/*N*/ 
/*N*/ 	USHORT nStartCol = nCol1;
/*N*/ 	USHORT nStartRow = nRow1;
/*N*/ 	if (bColByName)	++nStartRow;
/*N*/ 	if (bRowByName)	++nStartCol;
/*N*/ 	String aTitle;
/*N*/ 	USHORT*	pDestCols = NULL;
/*N*/ 	USHORT*	pDestRows = NULL;
/*N*/ 	if (bColByName)
/*N*/ 	{
/*N*/ 		pDestCols = new USHORT[nCol2-nStartCol+1];
/*N*/ 		for (nCol=nStartCol; nCol<=nCol2; nCol++)
/*N*/ 		{
/*N*/ 			pSrcDoc->GetString(nCol,nRow1,nTab,aTitle);
/*N*/ 			USHORT nPos = SC_CONS_NOTFOUND;
/*N*/ 			if (aTitle.Len())
/*N*/ 			{
/*N*/ 				BOOL bFound = FALSE;
/*N*/ 				for (USHORT i=0; i<nColCount && !bFound; i++)
/*N*/ 					if ( *ppColHeaders[i] == aTitle )
/*N*/ 					{
/*N*/ 						nPos = i;
/*N*/ 						bFound = TRUE;
/*N*/ 					}
/*N*/ 				DBG_ASSERT(bFound, "Spalte nicht gefunden");
/*N*/ 			}
/*N*/ 			pDestCols[nCol-nStartCol] = nPos;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bRowByName)
/*N*/ 	{
/*N*/ 		pDestRows = new USHORT[nRow2-nStartRow+1];
/*N*/ 		for (nRow=nStartRow; nRow<=nRow2; nRow++)
/*N*/ 		{
/*N*/ 			pSrcDoc->GetString(nCol1,nRow,nTab,aTitle);
/*N*/ 			USHORT nPos = SC_CONS_NOTFOUND;
/*N*/ 			if (aTitle.Len())
/*N*/ 			{
/*N*/ 				BOOL bFound = FALSE;
/*N*/ 				for (USHORT i=0; i<nRowCount && !bFound; i++)
/*N*/ 					if ( *ppRowHeaders[i] == aTitle )
/*N*/ 					{
/*N*/ 						nPos = i;
/*N*/ 						bFound = TRUE;
/*N*/ 					}
/*N*/ 				DBG_ASSERT(bFound, "Zeile nicht gefunden");
/*N*/ 			}
/*N*/ 			pDestRows[nRow-nStartRow] = nPos;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	nCol1 = nStartCol;
/*N*/ 	nRow1 = nStartRow;
/*N*/ 
/*N*/ 	//		Daten
/*N*/ 
/*N*/ 	BOOL bAnyCell = ( eFunction == SUBTOTAL_FUNC_CNT2 );
/*N*/ 	for (nCol=nCol1; nCol<=nCol2; nCol++)
/*N*/ 	{
/*N*/ 		USHORT nArrX = nCol-nCol1;
/*N*/ 		if (bColByName)	nArrX = pDestCols[nArrX];
/*N*/ 		if (nArrX != SC_CONS_NOTFOUND)
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 for (nRow=nRow1; nRow<=nRow2; nRow++)
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	delete[] pDestCols;
/*N*/ 	delete[] pDestRows;
/*N*/ }

//	fertige Daten ins Dokument schreiben
//!	optimieren nach Spalten?

/*N*/ void ScConsData::OutputToDocument( ScDocument* pDestDoc, USHORT nCol, USHORT nRow, USHORT nTab )
/*N*/ {
/*N*/ 	OpCode eOpCode = eOpCodeTable[eFunction];
/*N*/ 
/*N*/ 	USHORT nArrX;
/*N*/ 	USHORT nArrY;
/*N*/ 	USHORT nCount;
/*N*/ 	USHORT nPos;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	//	Ecke links oben
/*N*/ 
/*N*/ 	if ( bColByName && bRowByName && aCornerText.Len() )
/*N*/ 		pDestDoc->SetString( nCol, nRow, nTab, aCornerText );
/*N*/ 
/*N*/ 	//	Titel
/*N*/ 
/*N*/ 	USHORT nStartCol = nCol;
/*N*/ 	USHORT nStartRow = nRow;
/*N*/ 	if (bColByName)	++nStartRow;
/*N*/ 	if (bRowByName)	++nStartCol;
/*N*/ 
/*N*/ 	if (bColByName)
/*N*/ 		for (i=0; i<nColCount; i++)
/*N*/ 			pDestDoc->SetString( nStartCol+i, nRow, nTab, *ppColHeaders[i] );
/*N*/ 	if (bRowByName)
/*N*/ 		for (i=0; i<nRowCount; i++)
/*N*/ 			pDestDoc->SetString( nCol, nStartRow+i, nTab, *ppRowHeaders[i] );
/*N*/ 
/*N*/ 	nCol = nStartCol;
/*N*/ 	nRow = nStartRow;
/*N*/ 
/*N*/ 	//	Daten
/*N*/ 
/*N*/ 	if ( ppCount && ppUsed )							// Werte direkt einfuegen
/*N*/ 	{
/*N*/ 		for (nArrX=0; nArrX<nColCount; nArrX++)
/*N*/ 			for (nArrY=0; nArrY<nRowCount; nArrY++)
/*N*/ 				if (ppUsed[nArrX][nArrY])
/*N*/ 				{
/*N*/ 					double fVal = lcl_CalcData( eFunction, ppCount[nArrX][nArrY],
/*N*/ 												ppSum[nArrX][nArrY],
/*N*/ 												ppSumSqr[nArrX][nArrY]);
/*N*/ 					if (ppCount[nArrX][nArrY] < 0.0)
/*N*/ 						pDestDoc->SetError( nCol+nArrX, nRow+nArrY, nTab, errNoValue );
/*N*/ 					else
/*N*/ 						pDestDoc->SetValue( nCol+nArrX, nRow+nArrY, nTab, fVal );
/*N*/ 				}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( ppRefs && ppUsed )								// Referenzen einfuegen
/*N*/ 	{
/*N*/ 								//! unterscheiden, ob nach Kategorien aufgeteilt
/*N*/ 		String aString;
/*N*/ 
/*N*/ 		SingleRefData aSRef;		// Daten fuer Referenz-Formelzellen
/*N*/ 		aSRef.InitFlags();
/*N*/ 		aSRef.SetFlag3D(TRUE);
/*N*/ 
/*N*/ 		ComplRefData aCRef;			// Daten fuer Summen-Zellen
/*N*/ 		aCRef.InitFlags();
/*N*/ 		aCRef.Ref1.SetColRel(TRUE); aCRef.Ref1.SetRowRel(TRUE); aCRef.Ref1.SetTabRel(TRUE);
/*N*/ 		aCRef.Ref2.SetColRel(TRUE); aCRef.Ref2.SetRowRel(TRUE); aCRef.Ref2.SetTabRel(TRUE);
/*N*/ 
/*N*/ 		for (nArrY=0; nArrY<nRowCount; nArrY++)
/*N*/ 		{
/*N*/ 			USHORT nNeeded = 0;
/*N*/ 			for (nArrX=0; nArrX<nColCount; nArrX++)
/*N*/ 				if (ppUsed[nArrX][nArrY])
/*N*/ 					nNeeded = Max( nNeeded, ppRefs[nArrX][nArrY].GetCount() );
/*N*/ 
/*N*/ 			if (nNeeded)
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pDestDoc->InsertRow( 0,nTab, MAXCOL,nTab, nRow+nArrY, nNeeded );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }





}
