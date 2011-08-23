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

//------------------------------------------------------------------------

#include <tools/debug.hxx>
#include <math.h>

#include "scmatrix.hxx"

#include <rtl/math.hxx>
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ void ScMatrix::CreateMatrix(USHORT nC, USHORT nR)		// nur fuer ctor
/*N*/ {
/*N*/ 	nAnzCol = nC;
/*N*/ 	nAnzRow = nR;
/*N*/ 	ULONG nCount = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( !nCount || nCount > GetElementsMax() )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE("ScMatrix::CreateMatrix: dimension error");
/*N*/ 		nAnzCol = nAnzRow = 1;
/*N*/ 		pMat = new MatValue[1];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pMat = new MatValue[nCount];
/*N*/ 	bIsString = NULL;
/*N*/ }

/*N*/ ScMatrix::~ScMatrix()
/*N*/ {
/*N*/ 	DeleteIsString();
/*N*/ 	delete [] pMat;
/*N*/ }

/*N*/ ScMatrix* ScMatrix::Clone() const
/*N*/ {
/*N*/ 	ScMatrix* pScMat = new ScMatrix(nAnzCol, nAnzRow);
/*N*/ 	MatCopy(*pScMat);
/*N*/ 	return pScMat;
/*N*/ }

//
//	File format: USHORT columns, USHORT rows, (columns*rows) entries:
//	BYTE type ( CELLTYPE_NONE, CELLTYPE_VALUE, CELLTYPE_STRING ); nothing, double or String
//

/*N*/ ScMatrix::ScMatrix(SvStream& rStream)
/*N*/ {
/*N*/ 	USHORT nC, nR;
/*N*/ 
/*N*/ 	rStream >> nC;
/*N*/ 	rStream >> nR;
/*N*/ 
/*N*/ 	CreateMatrix(nC, nR);
/*N*/ 	DBG_ASSERT( pMat, "pMat == NULL" );
/*N*/ 
/*N*/ 	String aMatStr;
/*N*/ 	double fVal;
/*N*/ 	rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
/*N*/ 	ULONG nCount = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	ULONG nReadCount = (ULONG) nC * nR;
/*N*/ 	for (ULONG i=0; i<nReadCount; i++)
/*N*/ 	{
/*N*/ 		BYTE nType;
/*N*/ 		rStream >> nType;
/*N*/ 		if ( nType == CELLTYPE_VALUE )
/*N*/ 		{
/*N*/ 			if ( i < nCount )
/*N*/ 				rStream >> pMat[i].fVal;
/*N*/ 			else
/*N*/ 				rStream >> fVal;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// For unknown types read and forget string (upwards compatibility)
/*N*/ 
/*N*/ 			if ( nType != CELLTYPE_NONE )
/*N*/ 				rStream.ReadByteString( aMatStr, eCharSet );
/*N*/ 
/*N*/ 			if ( i < nCount )
/*N*/ 			{
/*N*/ 				if (!bIsString)
/*N*/ 					ResetIsString();		// init string flags
/*N*/ 				bIsString[i] = ( nType == CELLTYPE_NONE ? SC_MATVAL_EMPTY : SC_MATVAL_STRING );
/*N*/ 
/*N*/ 				if ( nType == CELLTYPE_STRING )
/*N*/ 					pMat[i].pS = new String(aMatStr);
/*N*/ 				else
/*N*/ 					pMat[i].pS = NULL;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::Store(SvStream& rStream) const
/*N*/ {
/*N*/ 	ULONG nCount = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	// Don't store matrix with more than USHORT max elements, old versions
/*N*/ 	// might get confused in loops for(USHORT i=0; i<nC*nR; i++)
/*N*/ 	if ( !pMat || nCount > ((USHORT)(~0)) )
/*N*/ 	{
/*N*/ 		DBG_ASSERT( pMat, "ScMatrix::Store: pMat == NULL" );
/*N*/ 		// We can't store a 0 dimension because old versions rely on some
/*N*/ 		// matrix being present, e.g. DDE link results, and old versions didn't
/*N*/ 		// create a matrix if dimension was 0. Store an error result.
/*N*/ 		rStream << (USHORT) 1;
/*N*/ 		rStream << (USHORT) 1;
/*N*/ 		rStream << (BYTE) CELLTYPE_VALUE;
/*N*/ 		double fVal;
/*N*/ 		::rtl::math::setNan( &fVal );
/*N*/ 		rStream << fVal;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream << nAnzCol;
/*N*/ 	rStream << nAnzRow;
/*N*/ 
/*N*/ 	String aMatStr;
/*N*/ 	rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
/*N*/ 	for (ULONG i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		BYTE nType = CELLTYPE_VALUE;
/*N*/ 		if ( bIsString && bIsString[i] )
/*N*/ 		{
/*N*/ 			if ( pMat[i].pS )
/*N*/ 				aMatStr = *pMat[i].pS;
/*N*/ 			else
/*N*/ 				aMatStr.Erase();
/*N*/ 
/*N*/ 			if ( bIsString[i] == SC_MATVAL_STRING )
/*N*/ 				nType = CELLTYPE_STRING;
/*N*/ 			else
/*N*/ 				nType = CELLTYPE_NONE;
/*N*/ 		}
/*N*/ 		rStream << nType;
/*N*/ 		if ( nType == CELLTYPE_VALUE )
/*N*/ 			rStream << pMat[i].fVal;
/*N*/ 		else if ( nType == CELLTYPE_STRING )
/*N*/ 			rStream.WriteByteString( aMatStr, eCharSet );
/*N*/ 	}
/*N*/ }

/*N*/  void ScMatrix::ResetIsString()
/*N*/  {
/*N*/  	ULONG nCount = (ULONG) nAnzCol * nAnzRow;
/*N*/  	if (bIsString)
/*N*/  	{
/*N*/  		for (ULONG i = 0; i < nCount; i++)
/*N*/  		{
/*N*/  			if ( bIsString[i] )
/*N*/  				delete pMat[i].pS;
/*N*/  		}
/*N*/  	}
/*N*/  	else
/*N*/  		bIsString = new BYTE[nCount];
/*N*/  	memset( bIsString, 0, nCount * sizeof( BYTE ) );
/*N*/  }

/*N*/ void ScMatrix::DeleteIsString()
/*N*/ {
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*?*/ 		ULONG nCount = (ULONG) nAnzCol * nAnzRow;
/*?*/ 		for ( ULONG i = 0; i < nCount; i++ )
/*?*/ 		{
/*?*/ 			if ( bIsString[i] )
/*?*/ 				delete pMat[i].pS;
/*?*/ 		}
/*?*/ 		delete [] bIsString;
/*?*/ 		bIsString = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::PutDouble(double fVal, USHORT nC, USHORT nR)
/*N*/ {
/*N*/ 	if (nC < nAnzCol && nR < nAnzRow)
/*N*/ 		PutDouble( fVal, (ULONG) nC * nAnzRow + nR );
/*N*/ 	else
/*N*/ 		DBG_ERRORFILE("ScMatrix::PutDouble: dimension error");
/*N*/ }



/*N*/ void ScMatrix::PutString(const String& rStr, USHORT nC, USHORT nR)
/*N*/ {
/*N*/ 	if (nC < nAnzCol && nR < nAnzRow)
/*N*/ 		PutString( rStr, (ULONG) nC * nAnzRow + nR );
/*N*/ 	else
/*N*/ 		DBG_ERRORFILE("ScMatrix::PutString: dimension error");
/*N*/ }

/*N*/ void ScMatrix::PutString(const String& rStr, ULONG nIndex)
/*N*/ {
/*N*/ 	if (bIsString == NULL)
/*N*/ 		ResetIsString();
/*N*/ 	if ( bIsString[nIndex] && pMat[nIndex].pS )
/*N*/ 		*(pMat[nIndex].pS) = rStr;
/*N*/ 	else
/*N*/ 		pMat[nIndex].pS = new String(rStr);
/*N*/ 	bIsString[nIndex] = SC_MATVAL_STRING;
/*N*/ }

/*N*/  void ScMatrix::PutStringEntry( const String* pStr, BYTE bFlag, ULONG nIndex )
/*N*/  {
/*N*/  	DBG_ASSERT( bFlag, "ScMatrix::PutStringEntry: bFlag == 0" );
/*N*/  	if (bIsString == NULL)
/*N*/  		ResetIsString();
/*N*/  	if ( bIsString[nIndex] && pMat[nIndex].pS )
/*N*/  	{
/*N*/  		if ( pStr )
/*N*/  			*(pMat[nIndex].pS) = *pStr;
/*N*/  		else
/*N*/  			pMat[nIndex].pS->Erase();
/*N*/  	}
/*N*/  	else
/*N*/  		pMat[nIndex].pS = (pStr ? new String(*pStr) : NULL);
/*N*/  	bIsString[nIndex] = bFlag;
/*N*/  }

/*N*/ void ScMatrix::PutEmpty(USHORT nC, USHORT nR)
/*N*/ {
/*N*/ 	if (nC < nAnzCol && nR < nAnzRow)
/*N*/ 		PutEmpty( (ULONG) nC * nAnzRow + nR );
/*N*/ 	else
/*N*/ 		DBG_ERRORFILE("ScMatrix::PutString: dimension error");
/*N*/ }

/*N*/ void ScMatrix::PutEmpty(ULONG nIndex)
/*N*/ {
/*N*/ 	if (bIsString == NULL)
/*N*/ 		ResetIsString();
/*N*/ 	if ( bIsString[nIndex] && pMat[nIndex].pS )
/*N*/ 		delete pMat[nIndex].pS;
/*N*/ 	bIsString[nIndex] = SC_MATVAL_EMPTY;
/*N*/ 	pMat[nIndex].pS = NULL;
/*N*/ 	pMat[nIndex].fVal = 0.0;
/*N*/ }

/*N*/ double ScMatrix::GetDouble(USHORT nC, USHORT nR) const
/*N*/ {
/*N*/ 	if (nC < nAnzCol && nR < nAnzRow)
/*N*/ 		return GetDouble( (ULONG) nC * nAnzRow + nR );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE("ScMatrix::GetDouble: dimension error");
/*N*/ 		return 0.0;
/*N*/ 	}
/*N*/ }

/*N*/ const String& ScMatrix::GetString(USHORT nC, USHORT nR) const
/*N*/ {
/*N*/ 	if (nC < nAnzCol && nR < nAnzRow)
/*N*/ 	{
/*N*/ 		ULONG nIndex = (ULONG) nC * nAnzRow + nR;
/*N*/ 		if ( IsString( nIndex ) )
/*N*/ 			return GetString( nIndex );
/*N*/ 		else
/*N*/ 			DBG_ERRORFILE("ScMatrix::GetString: access error, no string");
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERRORFILE("ScMatrix::GetString: dimension error");
/*N*/ 	return ScGlobal::GetEmptyString();
/*N*/ }

/*N*/ const MatValue* ScMatrix::Get(USHORT nC, USHORT nR, BOOL& bString) const
/*N*/ {
/*N*/ 	if (nC < nAnzCol && nR < nAnzRow)
/*N*/ 	{
/*N*/ 		ULONG nIndex = (ULONG) nC * nAnzRow + nR;
/*N*/ 		if (bIsString && bIsString[nIndex])
/*N*/ 			bString = TRUE;
/*N*/ 		else
/*N*/ 			bString = FALSE;
/*N*/ 		return &pMat[nIndex];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERRORFILE("ScMatrix::Get: dimension error");
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void ScMatrix::MatCopy(ScMatrix& mRes) const
/*N*/ {
/*N*/ 	if (nAnzCol != mRes.nAnzCol || nAnzRow != mRes.nAnzRow)
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE("ScMatrix::MatCopy: dimension error");
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (bIsString)
/*N*/ 		{
/*N*/ 			mRes.ResetIsString();
/*N*/ 			for (USHORT i = 0; i < nAnzCol; i++)
/*N*/ 			{
/*N*/ 				ULONG nStart = (ULONG) i * nAnzRow;
/*N*/ 				for (USHORT j = 0; j < nAnzRow; j++)
/*N*/ 				{
/*N*/ 					if ( bIsString[nStart+j] )
/*N*/ 						mRes.PutStringEntry( pMat[nStart+j].pS,
/*N*/ 							bIsString[nStart+j], nStart+j );
/*N*/ 					else
/*N*/ 						mRes.pMat[nStart+j].fVal = pMat[nStart+j].fVal;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			mRes.DeleteIsString();
/*N*/ 			ULONG nCount = (ULONG) nAnzCol * nAnzRow;
/*N*/ 			for (ULONG i = 0; i < nCount; i++)
/*N*/ 				mRes.pMat[i].fVal = pMat[i].fVal;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/  void ScMatrix::MatTrans(ScMatrix& mRes) const
/*N*/  {
/*N*/  	if (nAnzCol != mRes.nAnzRow || nAnzRow != mRes.nAnzCol)
/*N*/  	{
/*N*/  		DBG_ERRORFILE("ScMatrix::MatTrans: dimension error");
/*N*/  	}
/*N*/  	else
/*N*/  	{
/*N*/  		if (bIsString)
/*N*/  		{
/*N*/  			mRes.ResetIsString();
/*N*/  			for ( ULONG i = 0; i < nAnzCol; i++ )
/*N*/  			{
/*N*/  				ULONG nStart = i * nAnzRow;
/*N*/  				for ( ULONG j = 0; j < nAnzRow; j++ )
/*N*/  				{
/*N*/  					if ( bIsString[nStart+j] )
/*N*/  						mRes.PutStringEntry( pMat[nStart+j].pS,
/*N*/  							bIsString[nStart+j], j*mRes.nAnzRow+i );
/*N*/  					else
/*N*/  						mRes.pMat[j*mRes.nAnzRow+i].fVal = pMat[nStart+j].fVal;
/*N*/  				}
/*N*/  			}
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			mRes.DeleteIsString();
/*N*/  			for ( ULONG i = 0; i < nAnzCol; i++ )
/*N*/  			{
/*N*/  				ULONG nStart = i * nAnzRow;
/*N*/  				for ( ULONG j = 0; j < nAnzRow; j++ )
/*N*/  				{
/*N*/  					mRes.pMat[j*mRes.nAnzRow+i].fVal = pMat[nStart+j].fVal;
/*N*/  				}
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  }


/*N*/  void ScMatrix::FillDouble( double fVal, USHORT nC1, USHORT nR1,
/*N*/  							USHORT nC2, USHORT nR2 )
/*N*/  {
/*N*/  	if (nC2 < nAnzCol && nR2 < nAnzRow)
/*N*/  	{
/*N*/  		if ( nC1 == 0 && nR1 == 0 && nC2 == nAnzCol-1 && nR2 == nAnzRow-1 )
/*N*/  		{
/*N*/  			ULONG nEnd = (ULONG) nAnzCol * nAnzRow;
/*N*/  			for ( ULONG j=0; j<nEnd; j++ )
/*N*/  				pMat[j].fVal = fVal;
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			for ( USHORT i=nC1; i<=nC2; i++ )
/*N*/  			{
/*N*/  				ULONG nOff1 = (ULONG) i * nAnzRow + nR1;
/*N*/  				ULONG nOff2 = nOff1 + nR2 - nR1;
/*N*/  				for ( ULONG j=nOff1; j<=nOff2; j++ )
/*N*/  					pMat[j].fVal = fVal;
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  	else
/*N*/  		DBG_ERRORFILE("ScMatrix::FillDouble: dimension error");
/*N*/  }

/*N*/  void ScMatrix::FillDoubleLowerLeft( double fVal, USHORT nC2 )
/*N*/  {
/*N*/  	if (nC2 < nAnzCol && nC2 < nAnzRow)
/*N*/  	{
/*N*/  		for ( USHORT i=1; i<=nC2; i++ )
/*N*/  		{
/*N*/  			ULONG nOff1 = (ULONG) i * nAnzRow;
/*N*/  			ULONG nOff2 = nOff1 + i;
/*N*/  			for ( ULONG j=nOff1; j<nOff2; j++ )
/*N*/  				pMat[j].fVal = fVal;
/*N*/  		}
/*N*/  	}
/*N*/  	else
/*N*/  		DBG_ERRORFILE("ScMatrix::FillDoubleLowerLeft: dimension error");
/*N*/  }

/*N*/ void ScMatrix::CompareEqual()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			if ( !bIsString[j])		// else: #WERT!
/*N*/ 				pMat[j].fVal = (pMat[j].fVal == 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			pMat[j].fVal = (pMat[j].fVal == 0.0);
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::CompareNotEqual()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			if ( !bIsString[j])		// else: #WERT!
/*N*/ 				pMat[j].fVal = (pMat[j].fVal != 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			pMat[j].fVal = (pMat[j].fVal != 0.0);
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::CompareLess()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			if ( !bIsString[j])		// else: #WERT!
/*N*/ 				pMat[j].fVal = (pMat[j].fVal < 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			pMat[j].fVal = (pMat[j].fVal < 0.0);
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::CompareGreater()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			if ( !bIsString[j])		// else: #WERT!
/*N*/ 				pMat[j].fVal = (pMat[j].fVal > 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			pMat[j].fVal = (pMat[j].fVal > 0.0);
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::CompareLessEqual()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			if ( !bIsString[j])		// else: #WERT!
/*N*/ 				pMat[j].fVal = (pMat[j].fVal <= 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			pMat[j].fVal = (pMat[j].fVal <= 0.0);
/*N*/ 	}
/*N*/ }

/*N*/ void ScMatrix::CompareGreaterEqual()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			if ( !bIsString[j])		// else: #WERT!
/*N*/ 				pMat[j].fVal = (pMat[j].fVal >= 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; j<n; j++ )
/*N*/ 			pMat[j].fVal = (pMat[j].fVal >= 0.0);
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScMatrix::And()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	BOOL bAnd = TRUE;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; bAnd && j<n; j++ )
/*N*/ 			if ( bIsString[j] )
/*N*/ 				bAnd = FALSE;		// we're assuming a CompareMat
/*N*/ 			else
/*N*/ 				bAnd = (pMat[j].fVal != 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; bAnd && j<n; j++ )
/*N*/ 			bAnd = (pMat[j].fVal != 0.0);
/*N*/ 	}
/*N*/ 	return bAnd;
/*N*/ }

/*N*/ BOOL ScMatrix::Or()
/*N*/ {
/*N*/ 	ULONG n = (ULONG) nAnzCol * nAnzRow;
/*N*/ 	BOOL bOr = FALSE;
/*N*/ 	if ( bIsString )
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; !bOr && j<n; j++ )
/*N*/ 			if ( !bIsString[j] )
/*N*/ 				bOr = (pMat[j].fVal != 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		for ( ULONG j=0; !bOr && j<n; j++ )
/*N*/ 			bOr = (pMat[j].fVal != 0.0);
/*N*/ 	}
/*N*/ 	return bOr;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
