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

#ifdef _MSC_VER
#pragma optimize("",off)
#endif
#ifdef _MSC_VER
#pragma optimize("q",off) // p-code off
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#include "globstr.hrc"
#include "rangeutl.hxx"
#include "pivot.hxx"
#include "rechead.hxx"
#include "refupdat.hxx"
#include "stlsheet.hxx"
namespace binfilter {


// STATIC DATA -----------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Hilfsmethoden von ScPivot
//--------------------------------------------------------------------------------------------------










//--------------------------------------------------------------------------------------------------

/*N*/ void ScPivot::GetParam( ScPivotParam& rParam, ScQueryParam& rQuery, ScArea& rSrcArea ) const
/*N*/ {
/*N*/ 	short nCount;
/*N*/ 	USHORT nDummy;
/*N*/ 	GetDestArea( rParam.nCol,rParam.nRow, nDummy,nDummy, rParam.nTab );
/*N*/ 
/*N*/ 	// Row und Col in der Bedeutung vertauscht:
/*N*/ 	GetRowFields( rParam.aColArr, nCount );
/*N*/ 	rParam.nColCount = (USHORT) nCount;
/*N*/ 	GetColFields( rParam.aRowArr, nCount );
/*N*/ 	rParam.nRowCount = (USHORT) nCount;
/*N*/ 	GetDataFields( rParam.aDataArr, nCount );
/*N*/ 	rParam.nDataCount = (USHORT) nCount;
/*N*/ 
/*N*/ 	rParam.bIgnoreEmptyRows  = GetIgnoreEmpty();
/*N*/ 	rParam.bDetectCategories = GetDetectCat();
/*N*/ 	rParam.bMakeTotalCol	 = GetMakeTotalCol();
/*N*/ 	rParam.bMakeTotalRow	 = GetMakeTotalRow();
/*N*/ 
/*N*/ 	GetQuery(rQuery);
/*N*/ 	GetSrcArea( rSrcArea.nColStart, rSrcArea.nRowStart,
/*N*/ 						rSrcArea.nColEnd, rSrcArea.nRowEnd, rSrcArea.nTab );
/*N*/ }

/*N*/ void ScPivot::SetParam( const ScPivotParam& rParam, const ScQueryParam& rQuery,
/*N*/ 							const ScArea& rSrcArea )
/*N*/ {
/*N*/ 	SetQuery( rQuery );
/*N*/ 	SetHeader( TRUE );
/*N*/ 	SetSrcArea( rSrcArea.nColStart, rSrcArea.nRowStart,
/*N*/ 						  rSrcArea.nColEnd, rSrcArea.nRowEnd, rSrcArea.nTab );
/*N*/ 	SetDestPos( rParam.nCol, rParam.nRow, rParam.nTab );
/*N*/ 	SetIgnoreEmpty( rParam.bIgnoreEmptyRows );
/*N*/ 	SetDetectCat( rParam.bDetectCategories );
/*N*/ 	SetMakeTotalCol( rParam.bMakeTotalCol );
/*N*/ 	SetMakeTotalRow( rParam.bMakeTotalRow );
/*N*/ 
/*N*/ 	// Row und Col in der Bedeutung vertauscht:
/*N*/ 	SetRowFields( rParam.aColArr, rParam.nColCount );
/*N*/ 	SetColFields( rParam.aRowArr, rParam.nRowCount );
/*N*/ 	SetDataFields( rParam.aDataArr, rParam.nDataCount );
/*N*/ }

/*N*/ DataObject*	ScPivot::Clone() const
/*N*/ {
/*N*/ 	return new ScPivot(*this);
/*N*/ }

//--------------------------------------------------------------------------------------------------
// PivotStrCollection
//--------------------------------------------------------------------------------------------------


/*N*/ short PivotStrCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
/*N*/ {
/*N*/ 	DBG_ASSERT(pKey1&&pKey2,"0-Zeiger bei PivotStrCollection::Compare");
/*N*/ 
/*N*/ 	short nResult = 0;
/*N*/ 
/*N*/ 	TypedStrData& rData1 = (TypedStrData&)*pKey1;
/*N*/ 	TypedStrData& rData2 = (TypedStrData&)*pKey2;
/*N*/ 
/*N*/ 	if ( rData1.nStrType > rData2.nStrType )
/*N*/ 		nResult = 1;
/*N*/ 	else if ( rData1.nStrType < rData2.nStrType )
/*N*/ 		nResult = -1;
/*N*/ 	else if ( !rData1.nStrType /* && !rData2.nStrType */ )
/*N*/ 	{
/*?*/ 		// Zahlen vergleichen:
/*?*/ 
/*?*/ 		if ( rData1.nValue == rData2.nValue )
/*?*/ 			nResult = 0;
/*?*/ 		else if ( rData1.nValue < rData2.nValue )
/*?*/ 			nResult = -1;
/*?*/ 		else
/*?*/ 			nResult = 1;
/*N*/ 	}
/*N*/ 	else /* if ( rData1.nStrType && rData2.nStrType ) */
/*N*/ 	{
/*N*/ 		// Strings vergleichen:
/*N*/ 
/*N*/ 		if (pUserData)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	nResult = pUserData->ICompare(rData1.aStrValue, rData2.aStrValue);
/*N*/ 		else
/*N*/ 		{
/*N*/             nResult = (short) ScGlobal::pTransliteration->compareString(
/*N*/ 				rData1.aStrValue, rData2.aStrValue );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nResult;
/*N*/ }

/*N*/ short PivotStrCollection::GetIndex(TypedStrData* pData) const
/*N*/ {
/*N*/ 	USHORT nIndex = 0;
/*N*/ 	if (!Search(pData, nIndex))
/*N*/ 		nIndex = 0;
/*N*/ 	return (short)nIndex;
/*N*/ }

//--------------------------------------------------------------------------------------------------
// PivotCollection
//--------------------------------------------------------------------------------------------------

/*N*/ String ScPivotCollection::CreateNewName( USHORT nMin ) const
/*N*/ {
/*N*/     String aBase( RTL_CONSTASCII_USTRINGPARAM("DataPilot") );
/*N*/ 	//!	from Resource?
/*N*/ 
/*N*/ 	for (USHORT nAdd=0; nAdd<=nCount; nAdd++)	//	nCount+1 Versuche
/*N*/ 	{
/*N*/ 		String aNewName = aBase;
/*N*/ 		aNewName += String::CreateFromInt32( nMin + nAdd );
/*N*/ 		BOOL bFound = FALSE;
/*N*/ 		for (USHORT i=0; i<nCount && !bFound; i++)
/*N*/ 			if (((ScPivot*)pItems[i])->GetName() == aNewName)
/*N*/ 				bFound = TRUE;
/*N*/ 		if (!bFound)
/*N*/ 			return aNewName;			// freien Namen gefunden
/*N*/ 	}
/*N*/ 	return String();					// sollte nicht vorkommen
/*N*/ }


/*N*/ BOOL ScPivotCollection::Load(SvStream& rStream)
/*N*/ {
/*N*/ 	BOOL bSuccess = TRUE;
/*N*/ 	USHORT nNewCount, i;
/*N*/ 	FreeAll();
/*N*/ 
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream >> nNewCount;
/*N*/ 	for (i=0; i<nNewCount && bSuccess; i++)
/*N*/ 	{
/*N*/ 		ScPivot* pPivot = new ScPivot( pDoc );
/*N*/ 		if (pPivot)
/*N*/ 		{
/*N*/ 			bSuccess = pPivot->Load(rStream, aHdr);
/*N*/ 			Insert( pPivot );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bSuccess = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	//	fuer alte Dateien: eindeutige Namen vergeben
/*N*/ 
/*N*/ 	if (bSuccess)
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 			if (!((const ScPivot*)At(i))->GetName().Len())
/*N*/ 				((ScPivot*)At(i))->SetName( CreateNewName() );
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }

/*N*/ BOOL ScPivotCollection::Store(SvStream& rStream) const
/*N*/ {
/*N*/ 	BOOL bSuccess = TRUE;
/*N*/ 
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream << nCount;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nCount && bSuccess; i++)
/*?*/ 		bSuccess = ((const ScPivot*)At(i))->Store( rStream, aHdr );
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }

/*N*/ void ScPivotCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
/*N*/ 								USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 								USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 								short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*?*/ 		USHORT theCol1;
/*?*/ 		USHORT theRow1;
/*?*/ 		USHORT theTab1;
/*?*/ 		USHORT theCol2;
/*?*/ 		USHORT theRow2;
/*?*/ 		USHORT theTab2;
/*?*/ 		ScRefUpdateRes eRes;
/*?*/ 		ScPivot* pPivot = (ScPivot*)pItems[i];
/*?*/ 
/*?*/ 		//	Source
/*?*/ 
/*?*/ 		pPivot->GetSrcArea( theCol1, theRow1, theCol2, theRow2, theTab1 );
/*?*/ 		theTab2 = theTab1;
/*?*/ 
/*?*/ 		eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
/*?*/ 												nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
/*?*/ 												theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
/*?*/ 
/*?*/ 		if (eRes != UR_NOTHING)
/*?*/ 			pPivot->MoveSrcArea( theCol1, theRow1, theTab1 );
/*?*/ 
/*?*/ 		//	Dest
/*?*/ 
/*?*/ 		pPivot->GetDestArea( theCol1, theRow1, theCol2, theRow2, theTab1 );
/*?*/ 		theTab2 = theTab1;
/*?*/ 
/*?*/ 		eRes = ScRefUpdate::Update( pDoc, eUpdateRefMode,
/*?*/ 												nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
/*?*/ 												theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
/*?*/ 
/*?*/ 		if (eRes != UR_NOTHING)
/*?*/ 			pPivot->MoveDestArea( theCol1, theRow1, theTab1 );
/*N*/ 	}
/*N*/ }







}
