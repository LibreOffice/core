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

#include <bf_svtools/intitem.hxx>
#include <vcl/sound.hxx>

#include "globstr.hrc"
#include "subtotal.hxx"
#include "interpre.hxx"
#include "markdata.hxx"
#include "validat.hxx"
#include "scitems.hxx"
#include "stlpool.hxx"
#include "poolhelp.hxx"
#include "detdata.hxx"
namespace binfilter {

// -----------------------------------------------------------------------

// Nach der Regula Falsi Methode
/*N*/ BOOL ScDocument::Solver(USHORT nFCol, USHORT nFRow, USHORT nFTab,
/*N*/ 						USHORT nVCol, USHORT nVRow, USHORT nVTab,
/*N*/ 						const String& sValStr, double& nX)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	nX = 0.0;
/*N*/ 	if (ValidColRow(nFCol, nFRow) && ValidColRow(nVCol, nVRow) &&
/*N*/ 		VALIDTAB(nFTab) && VALIDTAB(nVTab) && pTab[nFTab] && pTab[nVTab])
/*N*/ 	{
/*N*/ 		CellType eFType, eVType;
/*N*/ 		GetCellType(nFCol, nFRow, nFTab, eFType);
/*N*/ 		GetCellType(nVCol, nVRow, nVTab, eVType);
/*N*/ 		// CELLTYPE_NOTE: kein Value aber von Formel referiert
/*N*/ 		if (eFType == CELLTYPE_FORMULA && (eVType == CELLTYPE_VALUE
/*N*/ 				|| eVType == CELLTYPE_NOTE) )
/*N*/ 		{
/*N*/ 			SingleRefData aRefData;
/*N*/ 			aRefData.InitFlags();
/*N*/ 			aRefData.nCol = nVCol;
/*N*/ 			aRefData.nRow = nVRow;
/*N*/ 			aRefData.nTab = nVTab;
/*N*/ 
/*N*/ 			ScTokenArray aArr;
/*N*/ 			aArr.AddOpCode( ocBackSolver );
/*N*/ 			aArr.AddOpCode( ocOpen );
/*N*/ 			aArr.AddSingleReference( aRefData );
/*N*/ 			aArr.AddOpCode( ocSep );
/*N*/ 
/*N*/ 			aRefData.nCol = nFCol;
/*N*/ 			aRefData.nRow = nFRow;
/*N*/ 			aRefData.nTab = nFTab;
/*N*/ 
/*N*/ 			aArr.AddSingleReference( aRefData );
/*N*/ 			aArr.AddOpCode( ocSep );
/*N*/ 			aArr.AddString( sValStr.GetBuffer() );
/*N*/ 			aArr.AddOpCode( ocClose );
/*N*/ 			aArr.AddOpCode( ocStop );
/*N*/ 
/*N*/ 			ScFormulaCell* pCell = new ScFormulaCell( this, ScAddress(), &aArr );
/*N*/ 
/*N*/ 			if (pCell)
/*N*/ 			{
/*N*/ 				pCell->Interpret();
/*N*/ 				USHORT nErrCode = pCell->GetErrCode();
/*N*/ 				nX = pCell->GetValueAlways();
/*N*/ 				if (nErrCode == 0)					// kein fehler beim Rechnen
/*N*/ 					bRet = TRUE;
/*N*/ 				delete pCell;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void ScDocument::InsertMatrixFormula(USHORT nCol1, USHORT nRow1,
/*N*/ 									 USHORT nCol2, USHORT nRow2,
/*N*/ 									 const ScMarkData& rMark,
/*N*/ 									 const String& rFormula,
/*N*/ 									 const ScTokenArray* pArr )
/*N*/ {
/*N*/ 	PutInOrder(nCol1, nCol2);
/*N*/ 	PutInOrder(nRow1, nRow2);
/*N*/ 	USHORT i, j, k, nTab1;
/*N*/ 	i = 0;
/*N*/ 	BOOL bStop = FALSE;
/*N*/ 	while (i <= MAXTAB && !bStop)				// erste markierte Tabelle finden
/*N*/ 	{
/*N*/ 		if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 			bStop = TRUE;
/*N*/ 		else
/*N*/ 			i++;
/*N*/ 	}
/*N*/ 	nTab1 = i;
/*N*/ 	if (i == MAXTAB + 1)
/*N*/ 	{
/*N*/ 		Sound::Beep();
/*N*/ 		DBG_ERROR("ScDocument::InsertMatrixFormula Keine Tabelle markiert");
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	ScFormulaCell* pCell;
/*N*/ 	ScAddress aPos( nCol1, nRow1, nTab1 );
/*N*/ 	if (pArr)
/*N*/ 		pCell = new ScFormulaCell( this, aPos, pArr, MM_FORMULA );
/*N*/ 	else
/*N*/ 		pCell = new ScFormulaCell( this, aPos, rFormula, MM_FORMULA );
/*N*/ 	pCell->SetMatColsRows( nCol2 - nCol1 + 1, nRow2 - nRow1 + 1 );
/*N*/ 	for (i = 0; i <= MAXTAB; i++)
/*N*/ 	{
/*N*/ 		if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 		{
/*N*/ 			if (i == nTab1)
/*N*/ 				pTab[i]->PutCell(nCol1, nRow1, pCell);
/*N*/ 			else
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				pTab[i]->PutCell(nCol1, nRow1, pCell->Clone(this, ScAddress( nCol1, nRow1, i)));
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SingleRefData aRefData;
/*N*/ 	aRefData.InitFlags();
/*N*/ 	aRefData.nCol = nCol1;
/*N*/ 	aRefData.nRow = nRow1;
/*N*/ 	aRefData.nTab = nTab1;
/*N*/ 	aRefData.SetColRel( TRUE );
/*N*/ 	aRefData.SetRowRel( TRUE );
/*N*/ 	aRefData.SetTabRel( TRUE );
/*N*/ 	aRefData.CalcRelFromAbs( ScAddress( nCol1, nRow1, nTab1 ) );
/*N*/ 
/*N*/ 	ScTokenArray aArr;
/*N*/ 	ScToken* t = aArr.AddSingleReference(aRefData);
/*N*/ 	t->NewOpCode( ocMatRef );
/*N*/ 
/*N*/ 	for (i = 0; i <= MAXTAB; i++)
/*N*/ 	{
/*N*/ 		if (pTab[i] && rMark.GetTableSelect(i))
/*N*/ 		{
/*N*/ 			pTab[i]->DoColResize( nCol1, nCol2, nRow2 - nRow1 + 1 );
/*N*/ 			if (i != nTab1)
/*N*/ 			{
/*?*/ 				aRefData.nTab = i;
/*?*/ 				aRefData.nRelTab = i - nTab1;
/*?*/ 				t->GetSingleRef() = aRefData;
/*N*/ 			}
/*N*/ 			for (j = nCol1; j <= nCol2; j++)
/*N*/ 			{
/*N*/ 				for (k = nRow1; k <= nRow2; k++)
/*N*/ 				{
/*N*/ 					if (j != nCol1 || k != nRow1)		// nicht in der ersten Zelle
/*N*/ 					{
/*N*/ 						// Array muss geklont werden, damit jede
/*N*/ 						// Zelle ein eigenes Array erhaelt!
/*N*/ 						aPos = ScAddress( j, k, i );
/*N*/ 						t->CalcRelFromAbs( aPos );
/*N*/ 						pCell = new ScFormulaCell( this, aPos, aArr.Clone(), MM_REFERENCE );
/*N*/ 						pTab[i]->PutCell(j, k, (ScBaseCell*) pCell);
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }




/*N*/ BOOL ScDocument::GetNextMarkedCell( USHORT& rCol, USHORT& rRow, USHORT nTab,
/*N*/ 										const ScMarkData& rMark )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		return pTab[nTab]->GetNextMarkedCell( rCol, rRow, rMark );
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ void ScDocument::CompileDBFormula()
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ }

/*N*/ void ScDocument::CompileDBFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 	{
/*N*/ 		if (pTab[i]) pTab[i]->CompileDBFormula( bCreateFormulaString );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::CompileNameFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	if ( pCondFormList )
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		pCondFormList->CompileAll();	// nach ScNameDlg noetig
/*N*/ 
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 	{
/*N*/ 		if (pTab[i]) pTab[i]->CompileNameFormula( bCreateFormulaString );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::CompileColRowNameFormula()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXTAB; i++)
/*N*/ 	{
/*N*/ 		if (pTab[i]) pTab[i]->CompileColRowNameFormula();
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::DoColResize( USHORT nTab, USHORT nCol1, USHORT nCol2, USHORT nAdd )
/*N*/ {
/*N*/ 	if (nTab<=MAXTAB && pTab[nTab])
/*N*/ 		pTab[nTab]->DoColResize( nCol1, nCol2, nAdd );
/*N*/ 	else
/*N*/ 		DBG_ERROR("DoColResize: falsche Tabelle");
/*N*/ }

/*N*/ void ScDocument::InvalidateTableArea()
/*N*/ {
/*N*/ 	for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
/*N*/ 	{
/*N*/ 		pTab[nTab]->InvalidateTableArea();
/*N*/ 		if ( pTab[nTab]->IsScenario() )
/*N*/ 			pTab[nTab]->InvalidateScenarioRanges();
/*N*/ 	}
/*N*/ }



/*N*/ BOOL ScDocument::GetSelectionFunction( ScSubTotalFunc eFunc,
/*N*/ 										const ScAddress& rCursor, const ScMarkData& rMark,
/*N*/ 										double& rResult )
/*N*/ {
/*N*/ 	ScFunctionData aData(eFunc);
/*N*/ 
/*N*/ 	ScRange aSingle( rCursor );
/*N*/ 	if ( rMark.IsMarked() )
/*N*/ 		rMark.GetMarkArea(aSingle);
/*N*/ 
/*N*/ 	USHORT nStartCol = aSingle.aStart.Col();
/*N*/ 	USHORT nStartRow = aSingle.aStart.Row();
/*N*/ 	USHORT nEndCol = aSingle.aEnd.Col();
/*N*/ 	USHORT nEndRow = aSingle.aEnd.Row();
/*N*/ 
/*N*/ 	for (USHORT nTab=0; nTab<=MAXTAB && !aData.bError; nTab++)
/*N*/ 		if (pTab[nTab] && rMark.GetTableSelect(nTab))
/*N*/ 			pTab[nTab]->UpdateSelectionFunction( aData,
/*N*/ 							nStartCol, nStartRow, nEndCol, nEndRow, rMark );
/*N*/ 
/*N*/ 			//!	rMark an UpdateSelectionFunction uebergeben !!!!!
/*N*/ 
/*N*/ 	if (!aData.bError)
/*N*/ 		switch (eFunc)
/*N*/ 		{
/*N*/ 			case SUBTOTAL_FUNC_SUM:
/*N*/ 				rResult = aData.nVal;
/*N*/ 				break;
/*N*/ 			case SUBTOTAL_FUNC_CNT:
/*N*/ 			case SUBTOTAL_FUNC_CNT2:
/*N*/ 				rResult = aData.nCount;
/*N*/ 				break;
/*N*/ 			case SUBTOTAL_FUNC_AVE:
/*N*/ 				if (aData.nCount)
/*N*/ 					rResult = aData.nVal / (double) aData.nCount;
/*N*/ 				else
/*N*/ 					aData.bError = TRUE;
/*N*/ 				break;
/*N*/ 			case SUBTOTAL_FUNC_MAX:
/*N*/ 			case SUBTOTAL_FUNC_MIN:
/*N*/ 				if (aData.nCount)
/*N*/ 					rResult = aData.nVal;
/*N*/ 				else
/*N*/ 					aData.bError = TRUE;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 	if (aData.bError)
/*N*/ 		rResult = 0.0;
/*N*/ 
/*N*/ 	return !aData.bError;
/*N*/ }

/*N*/ double ScDocument::RoundValueAsShown( double fVal, ULONG nFormat )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	short nType;
        return 0;//STRIP001 return fVal;
}

//
//			bedingte Formate und Gueltigkeitsbereiche
//

/*N*/ ULONG ScDocument::AddCondFormat( const ScConditionalFormat& rNew )
/*N*/ {
/*N*/ 	if (rNew.IsEmpty())
/*N*/ 		return 0;					// leer ist immer 0
/*N*/ 
/*N*/ 	if (!pCondFormList)
/*N*/ 		pCondFormList = new ScConditionalFormatList;
/*N*/ 
/*N*/ 	ULONG nMax = 0;
/*N*/ 	USHORT nCount = pCondFormList->Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const ScConditionalFormat* pForm = (*pCondFormList)[i];
/*N*/ 		ULONG nKey = pForm->GetKey();
/*N*/ 		if ( pForm->EqualEntries( rNew ) )
/*N*/ 			return nKey;
/*N*/ 		if ( nKey > nMax )
/*N*/ 			nMax = nKey;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Der Aufruf kann aus ScPatternAttr::PutInPool kommen, darum Clone (echte Kopie)
/*N*/ 
/*N*/ 	ULONG nNewKey = nMax + 1;
/*N*/ 	ScConditionalFormat* pInsert = rNew.Clone(this);
/*N*/ 	pInsert->SetKey( nNewKey );
/*N*/ 	pCondFormList->InsertNew( pInsert );
/*N*/ 	return nNewKey;
/*N*/ }

/*N*/ ULONG ScDocument::AddValidationEntry( const ScValidationData& rNew )
/*N*/ {
/*N*/ 	if (rNew.IsEmpty())
/*N*/ 		return 0;					// leer ist immer 0
/*N*/ 
/*N*/ 	if (!pValidationList)
/*N*/ 		pValidationList = new ScValidationDataList;
/*N*/ 
/*N*/ 	ULONG nMax = 0;
/*N*/ 	USHORT nCount = pValidationList->Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const ScValidationData* pData = (*pValidationList)[i];
/*N*/ 		ULONG nKey = pData->GetKey();
/*N*/ 		if ( pData->EqualEntries( rNew ) )
/*N*/ 			return nKey;
/*N*/ 		if ( nKey > nMax )
/*N*/ 			nMax = nKey;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Der Aufruf kann aus ScPatternAttr::PutInPool kommen, darum Clone (echte Kopie)
/*N*/ 
/*N*/ 	ULONG nNewKey = nMax + 1;
/*N*/ 	ScValidationData* pInsert = rNew.Clone(this);
/*N*/ 	pInsert->SetKey( nNewKey );
/*N*/ 	pValidationList->InsertNew( pInsert );
/*N*/ 	return nNewKey;
/*N*/ }


/*N*/ const SfxItemSet* ScDocument::GetCondResult( USHORT nCol, USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	const ScConditionalFormat* pForm = GetCondFormat( nCol, nRow, nTab );
/*N*/ 	if ( pForm )
/*N*/ 	{
/*N*/ 		ScBaseCell* pCell = ((ScDocument*)this)->GetCell(ScAddress(nCol,nRow,nTab));
/*N*/ 		String aStyle = pForm->GetCellStyle( pCell, ScAddress(nCol, nRow, nTab) );
/*N*/ 		if (aStyle.Len())
/*N*/ 		{
/*N*/ 			SfxStyleSheetBase* pStyleSheet = xPoolHelper->GetStylePool()->Find( aStyle, SFX_STYLE_FAMILY_PARA );
/*N*/ 			if ( pStyleSheet )
/*N*/ 				return &pStyleSheet->GetItemSet();
/*N*/ 			// if style is not there, treat like no condition
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ const ScConditionalFormat* ScDocument::GetCondFormat(
/*N*/ 							USHORT nCol, USHORT nRow, USHORT nTab ) const
/*N*/ {
/*N*/ 	ULONG nIndex = ((const SfxUInt32Item*)GetAttr(nCol,nRow,nTab,ATTR_CONDITIONAL))->GetValue();
/*N*/ 	if (nIndex)
/*N*/ 	{
/*N*/ 		if (pCondFormList)
/*N*/ 			return pCondFormList->GetFormat( nIndex );
/*N*/ 		else
/*N*/ 			DBG_ERROR("pCondFormList ist 0");
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;
/*N*/ }

/*N*/ const ScValidationData*	ScDocument::GetValidationEntry( ULONG nIndex ) const
/*N*/ {
/*N*/ 	if ( pValidationList )
/*N*/ 		return pValidationList->GetData( nIndex );
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ void ScDocument::FindConditionalFormat( ULONG nKey, ScRangeList& rRanges )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXTAB && pTab[i]; i++)
/*N*/ 		pTab[i]->FindConditionalFormat( nKey, rRanges );
/*N*/ }


/*N*/ void ScDocument::ConditionalChanged( ULONG nKey )
/*N*/ {
/*N*/ 	if ( nKey && pCondFormList && !bIsClip && !bIsUndo )		// nKey==0 -> noop
/*N*/ 	{
/*N*/ 		ScConditionalFormat* pForm = pCondFormList->GetFormat( nKey );
/*N*/ 		if (pForm)
/*N*/ 			pForm->InvalidateArea();
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::SetConditionalUsed( ULONG nKey )	// aus dem Speichern der Tabellen
/*N*/ {
/*N*/ 	if ( nKey && pCondFormList )		// nKey==0 -> noop
/*N*/ 	{
/*N*/ 		ScConditionalFormat* pForm = pCondFormList->GetFormat( nKey );
/*N*/ 		if (pForm)
/*N*/ 			pForm->SetUsed(TRUE);
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocument::SetValidationUsed( ULONG nKey )	// aus dem Speichern der Tabellen
/*N*/ {
/*N*/ 	if ( nKey && pValidationList )		// nKey==0 -> noop
/*N*/ 	{
/*N*/ 		ScValidationData* pData = pValidationList->GetData( nKey );
/*N*/ 		if (pData)
/*N*/ 			pData->SetUsed(TRUE);
/*N*/ 	}
/*N*/ }


//------------------------------------------------------------------------


/*N*/ void ScDocument::AddDetectiveOperation( const ScDetOpData& rData )
/*N*/ {
/*N*/ 	if (!pDetOpList)
/*N*/ 		pDetOpList = new ScDetOpList;
/*N*/ 
/*N*/ 	pDetOpList->Append( new ScDetOpData( rData ) );
/*N*/ }

/*N*/ void ScDocument::ClearDetectiveOperations()
/*N*/ {
/*N*/ 	delete pDetOpList;		// loescht auch die Eintraege
/*N*/ 	pDetOpList = NULL;
/*N*/ }


//------------------------------------------------------------------------
//
//		Vergleich von Dokumenten
//
//------------------------------------------------------------------------

//	Pfriemel-Faktoren
#define SC_DOCCOMP_MAXDIFF	256
#define SC_DOCCOMP_MINGOOD	128
#define SC_DOCCOMP_COLUMNS	10
#define SC_DOCCOMP_ROWS		100










}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
