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

//------------------------------------------------------------------

#include "scitems.hxx"
#include <bf_sfx2/objsh.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svtools/zforlist.hxx>
#include <rtl/math.hxx>


#include "conditio.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "hints.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "rangenam.hxx"
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ SV_IMPL_OP_PTRARR_SORT( ScConditionalFormats_Impl, ScConditionalFormatPtr );

//------------------------------------------------------------------------

/*N*/ BOOL lcl_HasRelRef( ScDocument* pDoc, ScTokenArray* pFormula, USHORT nRecursion = 0 )
/*N*/ {
/*N*/ 	if (pFormula)
/*N*/ 	{
/*N*/ 		pFormula->Reset();
/*N*/ 		ScToken* t;
/*N*/ 		for( t = pFormula->GetNextReferenceOrName(); t; t = pFormula->GetNextReferenceOrName() )
/*N*/ 		{
/*N*/ 			if( t->GetType() == svIndex )
/*N*/ 			{
/*?*/ 				ScRangeData* pRangeData = pDoc->GetRangeName()->FindIndex( t->GetIndex() );
/*?*/ 				if( (t->GetOpCode() == ocName) && (nRecursion < 42) && pRangeData &&
/*?*/ 					lcl_HasRelRef( pDoc, pRangeData->GetCode(), nRecursion + 1 ) )
/*?*/ 					return TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SingleRefData& rRef1 = t->GetSingleRef();
/*N*/ 				if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
/*N*/ 					return TRUE;
/*?*/ 				if ( t->GetType() == svDoubleRef )
/*?*/ 				{
/*?*/ 					SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
/*?*/ 					if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
/*?*/ 						return TRUE;
/*?*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ ScConditionEntry::ScConditionEntry( const ScConditionEntry& r ) :
/*N*/ 	eOp(r.eOp),
/*N*/ 	nOptions(r.nOptions),
/*N*/ 	nVal1(r.nVal1),
/*N*/ 	nVal2(r.nVal2),
/*N*/ 	aStrVal1(r.aStrVal1),
/*N*/ 	aStrVal2(r.aStrVal2),
/*N*/ 	bIsStr1(r.bIsStr1),
/*N*/ 	bIsStr2(r.bIsStr2),
/*N*/ 	bRelRef1(r.bRelRef1),
/*N*/ 	bRelRef2(r.bRelRef2),
/*N*/ 	pFormula1(NULL),
/*N*/ 	pFormula2(NULL),
/*N*/ 	pFCell1(NULL),
/*N*/ 	pFCell2(NULL),
/*N*/ 	pDoc(r.pDoc),
/*N*/ 	aSrcPos(r.aSrcPos),
/*N*/ 	bFirstRun(TRUE)
/*N*/ {
/*N*/ 	//	ScTokenArray copy ctor erzeugt flache Kopie
/*N*/ 
/*N*/ 	if (r.pFormula1)
/*N*/ 		pFormula1 = new ScTokenArray( *r.pFormula1 );
/*N*/ 	if (r.pFormula2)
/*N*/ 		pFormula2 = new ScTokenArray( *r.pFormula2 );
/*N*/ 
/*N*/ 	//	Formelzellen werden erst bei IsValid angelegt
/*N*/ }

/*N*/ ScConditionEntry::ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r ) :
/*N*/ 	eOp(r.eOp),
/*N*/ 	nOptions(r.nOptions),
/*N*/ 	nVal1(r.nVal1),
/*N*/ 	nVal2(r.nVal2),
/*N*/ 	aStrVal1(r.aStrVal1),
/*N*/ 	aStrVal2(r.aStrVal2),
/*N*/ 	bIsStr1(r.bIsStr1),
/*N*/ 	bIsStr2(r.bIsStr2),
/*N*/ 	bRelRef1(r.bRelRef1),
/*N*/ 	bRelRef2(r.bRelRef2),
/*N*/ 	pFormula1(NULL),
/*N*/ 	pFormula2(NULL),
/*N*/ 	pFCell1(NULL),
/*N*/ 	pFCell2(NULL),
/*N*/ 	pDoc(pDocument),
/*N*/ 	aSrcPos(r.aSrcPos),
/*N*/ 	bFirstRun(TRUE)
/*N*/ {
/*N*/ 	// echte Kopie der Formeln (fuer Ref-Undo)
/*N*/ 
/*N*/ 	if (r.pFormula1)
/*N*/ 		pFormula1 = r.pFormula1->Clone();
/*N*/ 	if (r.pFormula2)
/*N*/ 		pFormula2 = r.pFormula2->Clone();
/*N*/ 
/*N*/ 	//	Formelzellen werden erst bei IsValid angelegt
/*N*/ 	//!	im Clipboard nicht - dann vorher interpretieren !!!
/*N*/ }

/*N*/ ScConditionEntry::ScConditionEntry( ScConditionMode eOper,
/*N*/ 								const String& rExpr1, const String& rExpr2,
/*N*/ 								ScDocument* pDocument, const ScAddress& rPos,
/*N*/ 								BOOL bCompileEnglish, BOOL bCompileXML ) :
/*N*/ 	eOp(eOper),
/*N*/ 	nOptions(0),	// spaeter...
/*N*/ 	nVal1(0.0),
/*N*/ 	nVal2(0.0),
/*N*/ 	bIsStr1(FALSE),
/*N*/ 	bIsStr2(FALSE),
/*N*/ 	bRelRef1(FALSE),
/*N*/ 	bRelRef2(FALSE),
/*N*/ 	pFormula1(NULL),
/*N*/ 	pFormula2(NULL),
/*N*/ 	pFCell1(NULL),
/*N*/ 	pFCell2(NULL),
/*N*/ 	pDoc(pDocument),
/*N*/ 	aSrcPos(rPos),
/*N*/ 	bFirstRun(TRUE)
/*N*/ {
/*N*/ 	Compile( rExpr1, rExpr2, bCompileEnglish, bCompileXML, FALSE );
/*N*/ 
/*N*/ 	//	Formelzellen werden erst bei IsValid angelegt
/*N*/ }

/*N*/ ScConditionEntry::~ScConditionEntry()
/*N*/ {
/*N*/ 	delete pFCell1;
/*N*/ 	delete pFCell2;
/*N*/ 
/*N*/ 	delete pFormula1;
/*N*/ 	delete pFormula2;
/*N*/ }

/*N*/ ScConditionEntry::ScConditionEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
/*N*/ 									ScDocument* pDocument ) :
/*N*/ 	nVal1(0.0),
/*N*/ 	nVal2(0.0),
/*N*/ 	bIsStr1(FALSE),
/*N*/ 	bIsStr2(FALSE),
/*N*/ 	bRelRef1(FALSE),
/*N*/ 	bRelRef2(FALSE),
/*N*/ 	pFormula1(NULL),
/*N*/ 	pFormula2(NULL),
/*N*/ 	pFCell1(NULL),
/*N*/ 	pFCell2(NULL),
/*N*/ 	pDoc(pDocument),
/*N*/ 	bFirstRun(TRUE)
/*N*/ {
/*N*/ 	USHORT nVer = (USHORT) pDoc->GetSrcVersion();
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	BYTE nOpByte;
/*N*/ 	rStream >> nOpByte;
/*N*/ 	eOp = (ScConditionMode) nOpByte;
/*N*/ 
/*N*/ 	rStream >> nOptions;
/*N*/ 
/*N*/ 	ScAddress aPos;
/*N*/ 	BYTE nTypeByte;
/*N*/ 	rStream >> nTypeByte;
/*N*/ 	ScConditionValType eType = (ScConditionValType) nTypeByte;
/*N*/ 	if ( eType == SC_VAL_FORMULA )
/*N*/ 	{
/*N*/ 		rStream >> aPos;
/*N*/ 		pFormula1 = new ScTokenArray;
/*N*/ 		pFormula1->Load( rStream, nVer, aPos );
/*N*/ 		bRelRef1 = lcl_HasRelRef( pDoc, pFormula1 );
/*N*/ 	}
/*N*/ 	else if ( eType == SC_VAL_VALUE )
/*N*/ 		rStream >> nVal1;
/*N*/ 	else
/*N*/ 	{
/*?*/ 		bIsStr1 = TRUE;
/*?*/ 		rStream.ReadByteString( aStrVal1, rStream.GetStreamCharSet() );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
/*N*/ 	{
/*?*/ 		rStream >> nTypeByte;
/*?*/ 		eType = (ScConditionValType) nTypeByte;
/*?*/ 		if ( eType == SC_VAL_FORMULA )
/*?*/ 		{
/*?*/ 			rStream >> aPos;
/*?*/ 			pFormula2 = new ScTokenArray;
/*?*/ 			pFormula2->Load( rStream, nVer, aPos );
/*?*/ 			bRelRef2 = lcl_HasRelRef( pDoc, pFormula2 );
/*?*/ 		}
/*?*/ 		else if ( eType == SC_VAL_VALUE )
/*?*/ 			rStream >> nVal2;
/*?*/ 		else
/*?*/ 		{
/*?*/ 			bIsStr2 = TRUE;
/*?*/ 			rStream.ReadByteString( aStrVal2, rStream.GetStreamCharSet() );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 	aSrcPos = aPos;
/*N*/ 
/*N*/ 	//	Formelzellen werden erst bei IsValid angelegt
/*N*/ }

/*N*/ void ScConditionEntry::StoreCondition(SvStream& rStream, ScMultipleWriteHeader& rHdr) const
/*N*/ {
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	//	1) Byte fuer die Operation
/*N*/ 	//	2) USHORT fuer Optionen
/*N*/ 	//	3) Byte, ob Wert, String oder Formel folgt
/*N*/ 	//	4) double, String oder TokenArray
/*N*/ 	//	5) je nach Operation 3 und 4 nochmal
/*N*/ 	//	vor jedem TokenArray noch die Position als ScAddress
/*N*/ 
/*N*/ 	rStream << (BYTE) eOp;
/*N*/ 	rStream << nOptions;
/*N*/ 
/*N*/ 	ScConditionValType eType =
/*N*/ 			pFormula1 ? SC_VAL_FORMULA : ( bIsStr1 ? SC_VAL_STRING : SC_VAL_VALUE );
/*N*/ 	rStream << (BYTE) eType;
/*N*/ 	if ( eType == SC_VAL_FORMULA )
/*N*/ 	{
/*N*/ 		rStream << aSrcPos;
/*N*/ 		pFormula1->Store( rStream, aSrcPos );
/*N*/ 	}
/*N*/ 	else if ( eType == SC_VAL_VALUE )
/*N*/ 		rStream << nVal1;
/*N*/ 	else
/*?*/ 		rStream.WriteByteString( aStrVal1, rStream.GetStreamCharSet() );
/*N*/ 
/*N*/ 	if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
/*N*/ 	{
/*?*/ 		eType = pFormula2 ? SC_VAL_FORMULA : ( bIsStr2 ? SC_VAL_STRING : SC_VAL_VALUE );
/*?*/ 		rStream << (BYTE) eType;
/*?*/ 		if ( eType == SC_VAL_FORMULA )
/*?*/ 		{
/*?*/ 			rStream << aSrcPos;
/*?*/ 			pFormula2->Store( rStream, aSrcPos );
/*?*/ 		}
/*?*/ 		else if ( eType == SC_VAL_VALUE )
/*?*/ 			rStream << nVal2;
/*?*/ 		else
/*?*/ 			rStream.WriteByteString( aStrVal2, rStream.GetStreamCharSet() );
/*N*/ 	}
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ }

/*N*/ void ScConditionEntry::Compile( const String& rExpr1, const String& rExpr2,
/*N*/ 								BOOL bEnglish, BOOL bCompileXML, BOOL bTextToReal )
/*N*/ {
/*N*/ 	if ( rExpr1.Len() || rExpr2.Len() )
/*N*/ 	{
/*N*/ 		ScCompiler aComp( pDoc, aSrcPos );
/*N*/ 		aComp.SetCompileEnglish( bEnglish );
/*N*/ 		aComp.SetCompileXML( bCompileXML );
/*N*/ 
/*N*/ 		if ( rExpr1.Len() )
/*N*/ 		{
/*N*/ 			if ( pDoc->IsImportingXML() && !bTextToReal )
/*N*/ 			{
/*?*/ 				//	temporary formula string as string tokens
/*?*/ 				//!	merge with lcl_ScDocFunc_CreateTokenArrayXML
/*?*/ 				pFormula1 = new ScTokenArray;
/*?*/ 				pFormula1->AddString( rExpr1 );
/*?*/ 				// bRelRef1 is set when the formula is compiled again (CompileXML)
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pFormula1 = aComp.CompileString( rExpr1 );
/*N*/ 				if ( pFormula1->GetLen() == 1 )
/*N*/ 				{
/*?*/ 					// einzelne (konstante Zahl) ?
/*?*/ 					ScToken* pToken = pFormula1->First();
/*?*/ 					if ( pToken->GetOpCode() == ocPush )
/*?*/ 					{
/*?*/ 						if ( pToken->GetType() == svDouble )
/*?*/ 						{
/*?*/ 							nVal1 = pToken->GetDouble();
/*?*/ 							DELETEZ(pFormula1);				// nicht als Formel merken
/*?*/ 						}
/*?*/ 						else if ( pToken->GetType() == svString )
/*?*/ 						{
/*?*/ 							bIsStr1 = TRUE;
/*?*/ 							aStrVal1 = pToken->GetString();
/*?*/ 							DELETEZ(pFormula1);				// nicht als Formel merken
/*?*/ 						}
/*?*/ 					}
/*N*/ 				}
/*N*/ 				bRelRef1 = lcl_HasRelRef( pDoc, pFormula1 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( rExpr2.Len() )
/*N*/ 		{
/*N*/ 			if ( pDoc->IsImportingXML() && !bTextToReal )
/*N*/ 			{
/*?*/ 				//	temporary formula string as string tokens
/*?*/ 				//!	merge with lcl_ScDocFunc_CreateTokenArrayXML
/*?*/ 				pFormula2 = new ScTokenArray;
/*?*/ 				pFormula2->AddString( rExpr2 );
/*?*/ 				// bRelRef2 is set when the formula is compiled again (CompileXML)
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pFormula2 = aComp.CompileString( rExpr2 );
/*N*/ 				if ( pFormula2->GetLen() == 1 )
/*N*/ 				{
/*N*/ 					// einzelne (konstante Zahl) ?
/*N*/ 					ScToken* pToken = pFormula2->First();
/*N*/ 					if ( pToken->GetOpCode() == ocPush )
/*N*/ 					{
/*N*/ 						if ( pToken->GetType() == svDouble )
/*N*/ 						{
/*N*/ 							nVal2 = pToken->GetDouble();
/*N*/ 							DELETEZ(pFormula2);				// nicht als Formel merken
/*N*/ 						}
/*N*/ 						else if ( pToken->GetType() == svString )
/*N*/ 						{
/*?*/ 							bIsStr2 = TRUE;
/*?*/ 							aStrVal2 = pToken->GetString();
/*?*/ 							DELETEZ(pFormula2);				// nicht als Formel merken
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				bRelRef2 = lcl_HasRelRef( pDoc, pFormula2 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScConditionEntry::MakeCells( const ScAddress& rPos )			// Formelzellen anlegen
/*N*/ {
/*N*/ 	if ( !pDoc->IsClipOrUndo() )			// nie im Clipboard rechnen!
/*N*/ 	{
/*N*/ 		if ( pFormula1 && !pFCell1 && !bRelRef1 )
/*N*/ 		{
/*?*/ 			pFCell1 = new ScFormulaCell( pDoc, rPos, pFormula1 );
/*?*/ 			pFCell1->StartListeningTo( pDoc );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pFormula2 && !pFCell2 && !bRelRef2 )
/*N*/ 		{
/*?*/ 			pFCell2 = new ScFormulaCell( pDoc, rPos, pFormula2 );
/*?*/ 			pFCell2->StartListeningTo( pDoc );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScConditionEntry::SetIgnoreBlank(BOOL bSet)
/*N*/ {
/*N*/ 	//	Das Bit SC_COND_NOBLANKS wird gesetzt, wenn Blanks nicht ignoriert werden
/*N*/ 	//	(nur bei Gueltigkeit)
/*N*/ 
/*N*/ 	if (bSet)
/*N*/ 		nOptions &= ~SC_COND_NOBLANKS;
/*N*/ 	else
/*N*/ 		nOptions |= SC_COND_NOBLANKS;
/*N*/ }


/*N*/ void ScConditionEntry::CompileXML()
/*N*/ {
/*N*/ 	//	Convert the text tokens that were created during XML import into real tokens:
/*N*/ 	//	The stored string tokens contain english function names, but no XML-style references
/*N*/ 
/*N*/ 	Compile( GetExpression(aSrcPos, 0, 0, TRUE, FALSE, TRUE),
/*N*/ 			 GetExpression(aSrcPos, 1, 0, TRUE, FALSE, TRUE),
/*N*/ 			 TRUE, FALSE, TRUE );
/*N*/ }

/*N*/ void lcl_CondUpdateInsertTab( ScTokenArray& rCode, USHORT nInsTab, USHORT nPosTab, BOOL& rChanged )
/*N*/ {
/*N*/     //  Insert table: only update absolute table references.
/*N*/     //  (Similar to ScCompiler::UpdateInsertTab with bIsName=TRUE, result is the same as for named ranges)
/*N*/     //  For deleting, ScCompiler::UpdateDeleteTab is used because of the handling of invalid references.
/*N*/ 
/*N*/     rCode.Reset();
/*N*/     ScToken* p = rCode.GetNextReference();
/*N*/     while( p )
/*N*/     {
/*N*/         SingleRefData& rRef1 = p->GetSingleRef();
/*N*/         if ( !rRef1.IsTabRel() && nInsTab <= rRef1.nTab )
/*N*/         {
/*?*/             rRef1.nTab += 1;
/*?*/             rRef1.nRelTab = rRef1.nTab - nPosTab;
/*?*/             rChanged = TRUE;
/*N*/         }
/*N*/         if( p->GetType() == svDoubleRef )
/*N*/         {
/*?*/             SingleRefData& rRef2 = p->GetDoubleRef().Ref2;
/*?*/             if ( !rRef2.IsTabRel() && nInsTab <= rRef2.nTab )
/*?*/             {
/*?*/                 rRef2.nTab += 1;
/*?*/                 rRef2.nRelTab = rRef2.nTab - nPosTab;
/*?*/                 rChanged = TRUE;
/*N*/             }
/*N*/         }
/*N*/         p = rCode.GetNextReference();
/*N*/     }
/*N*/ }

/*N*/ void ScConditionEntry::UpdateReference( UpdateRefMode eUpdateRefMode,
/*N*/ 								const ScRange& rRange, short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	BOOL bInsertTab = ( eUpdateRefMode == URM_INSDEL && nDz == 1 );
/*N*/ 	BOOL bDeleteTab = ( eUpdateRefMode == URM_INSDEL && nDz == -1 );
/*N*/ 
/*N*/ 	BOOL bChanged1 = FALSE;
/*N*/ 	BOOL bChanged2 = FALSE;
/*N*/ 
/*N*/ 	if (pFormula1)
/*N*/ 	{
/*N*/ 		if ( bInsertTab )
/*N*/ 			lcl_CondUpdateInsertTab( *pFormula1, rRange.aStart.Tab(), aSrcPos.Tab(), bChanged1 );
/*N*/ 		else
/*N*/ 		{
/*?*/ 			ScCompiler aComp( pDoc, aSrcPos, *pFormula1 );
/*?*/ 			if ( bDeleteTab )
/*?*/ 				aComp.UpdateDeleteTab( rRange.aStart.Tab(), FALSE, TRUE, bChanged1 );
/*?*/ 			else
/*?*/ 				aComp.UpdateNameReference( eUpdateRefMode, rRange, nDx, nDy, nDz, bChanged1 );
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bChanged1)
/*?*/ 			DELETEZ(pFCell1);		// is created again in IsValid
/*N*/ 	}
/*N*/ 	if (pFormula2)
/*N*/ 	{
/*?*/ 		if ( bInsertTab )
/*?*/ 			lcl_CondUpdateInsertTab( *pFormula2, rRange.aStart.Tab(), aSrcPos.Tab(), bChanged2 );
/*?*/ 		else
/*?*/ 		{
/*?*/ 			ScCompiler aComp( pDoc, aSrcPos, *pFormula2 );
/*?*/ 			if ( bDeleteTab )
/*?*/ 				aComp.UpdateDeleteTab( rRange.aStart.Tab(), FALSE, TRUE, bChanged2 );
/*?*/ 			else
/*?*/ 				aComp.UpdateNameReference( eUpdateRefMode, rRange, nDx, nDy, nDz, bChanged2 );
/*?*/ 		}
/*?*/ 
/*?*/ 		if (bChanged2)
/*?*/ 			DELETEZ(pFCell2);		// is created again in IsValid
/*N*/ 	}
/*N*/ }


//!	als Vergleichsoperator ans TokenArray ???

/*N*/ BOOL lcl_IsEqual( const ScTokenArray* pArr1, const ScTokenArray* pArr2 )
/*N*/ {
/*N*/ 	//	verglichen wird nur das nicht-UPN Array
/*N*/ 
/*N*/ 	if ( pArr1 && pArr2 )
/*N*/ 	{
/*N*/ 		USHORT nLen = pArr1->GetLen();
/*N*/ 		if ( pArr2->GetLen() != nLen )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 		ScToken** ppToken1 = pArr1->GetArray();
/*N*/ 		ScToken** ppToken2 = pArr2->GetArray();
/*N*/ 		for (USHORT i=0; i<nLen; i++)
/*N*/ 		{
/*N*/ 			if ( ppToken1[i] != ppToken2[i] &&
/*N*/ 				 !(*ppToken1[i] == *ppToken2[i]) )
/*N*/ 				return FALSE;						// Unterschied
/*N*/ 		}
/*N*/ 		return TRUE;					// alle Eintraege gleich
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return !pArr1 && !pArr2;		// beide 0 -> gleich
/*N*/ }

/*N*/ int ScConditionEntry::operator== ( const ScConditionEntry& r ) const
/*N*/ {
/*N*/ 	BOOL bEq = (eOp == r.eOp && nOptions == r.nOptions &&
/*N*/ 				lcl_IsEqual( pFormula1, r.pFormula1 ) &&
/*N*/ 				lcl_IsEqual( pFormula2, r.pFormula2 ));
/*N*/ 	if (bEq)
/*N*/ 	{
/*N*/ 		//	bei Formeln auch die Referenz-Tabelle vergleichen
/*N*/ 		if ( ( pFormula1 || pFormula2 ) && aSrcPos != r.aSrcPos )
/*N*/ 			bEq = FALSE;
/*N*/ 
/*N*/ 		//	wenn keine Formeln, Werte vergleichen
/*N*/ 		if ( !pFormula1 && ( nVal1 != r.nVal1 || aStrVal1 != r.aStrVal1 || bIsStr1 != r.bIsStr1 ) )
/*N*/ 			bEq = FALSE;
/*N*/ 		if ( !pFormula2 && ( nVal2 != r.nVal2 || aStrVal2 != r.aStrVal2 || bIsStr2 != r.bIsStr2 ) )
/*N*/ 			bEq = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bEq;
/*N*/ }

/*N*/ void ScConditionEntry::Interpret( const ScAddress& rPos )
/*N*/ {
/*N*/ 	//	Formelzellen anlegen
/*N*/ 	//	dabei koennen neue Broadcaster (Note-Zellen) ins Dokument eingefuegt werden !!!!
/*N*/ 
/*N*/ 	if ( ( pFormula1 && !pFCell1 ) || ( pFormula2 && !pFCell2 ) )
/*N*/ 		MakeCells( rPos );
/*N*/ 
/*N*/ 	//	Formeln auswerten
/*N*/ 
/*N*/ 	BOOL bDirty = FALSE;		//! 1 und 2 getrennt ???
/*N*/ 
/*N*/ 	ScFormulaCell* pTemp1 = NULL;
/*N*/ 	ScFormulaCell* pEff1 = pFCell1;
/*N*/ 	if ( bRelRef1 )
/*N*/ 	{
/*N*/ 		pTemp1 = new ScFormulaCell( pDoc, rPos, pFormula1 );	// ohne Listening
/*N*/ 		pEff1 = pTemp1;
/*N*/ 	}
/*N*/ 	if ( pEff1 )
/*N*/ 	{
/*N*/ 		if (!pEff1->IsRunning())		// keine 522 erzeugen
/*N*/ 		{
/*N*/ 			//!	Changed statt Dirty abfragen !!!
/*N*/ 			if (pEff1->GetDirty() && !bRelRef1)
/*N*/ 				bDirty = TRUE;
/*N*/ 			if (pEff1->IsValue())
/*N*/ 			{
/*N*/ 				bIsStr1 = FALSE;
/*N*/ 				nVal1 = pEff1->GetValue();
/*N*/ 				aStrVal1.Erase();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				bIsStr1 = TRUE;
/*?*/ 				pEff1->GetString( aStrVal1 );
/*?*/ 				nVal1 = 0.0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete pTemp1;
/*N*/ 
/*N*/ 	ScFormulaCell* pTemp2 = NULL;
/*N*/ 	ScFormulaCell* pEff2 = pFCell2; //@ 1!=2
/*N*/ 	if ( bRelRef2 )
/*N*/ 	{
/*N*/ 		pTemp2 = new ScFormulaCell( pDoc, rPos, pFormula2 );	// ohne Listening
/*N*/ 		pEff2 = pTemp2;
/*N*/ 	}
/*N*/ 	if ( pEff2 )
/*N*/ 	{
/*N*/ 		if (!pEff2->IsRunning())		// keine 522 erzeugen
/*N*/ 		{
/*N*/ 			if (pEff2->GetDirty() && !bRelRef2)
/*N*/ 				bDirty = TRUE;
/*N*/ 			if (pEff2->IsValue())
/*N*/ 			{
/*N*/ 				bIsStr2 = FALSE;
/*N*/ 				nVal2 = pEff2->GetValue();
/*N*/ 				aStrVal2.Erase();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				bIsStr2 = TRUE;
/*N*/ 				pEff2->GetString( aStrVal2 );
/*N*/ 				nVal2 = 0.0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	delete pTemp2;
/*N*/ 
/*N*/ 	//	wenn IsRunning, bleiben die letzten Werte erhalten
/*N*/ 
/*N*/ 	if (bDirty && !bFirstRun)
/*N*/ 	{
/*N*/ 		// 	bei bedingten Formaten neu painten
/*N*/ 
/*N*/ 		DataChanged( NULL );	// alles
/*N*/ 	}
/*N*/ 
/*N*/ 	bFirstRun = FALSE;
/*N*/ }

/*N*/ BOOL ScConditionEntry::IsValid( double nArg ) const
/*N*/ {
/*N*/ 	//	Interpret muss schon gerufen sein
/*N*/ 
/*N*/ 	if ( bIsStr1 )
/*N*/ 	{
/*N*/ 		// wenn auf String getestet wird, bei Zahlen immer FALSE, ausser bei "ungleich"
/*N*/ 
/*N*/ 		return ( eOp == SC_COND_NOTEQUAL );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
/*N*/ 		if ( bIsStr2 )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 	double nComp1 = nVal1;		// Kopie, damit vertauscht werden kann
/*N*/ 	double nComp2 = nVal2;
/*N*/ 
/*N*/ 	if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
/*N*/ 		if ( nComp1 > nComp2 )
/*N*/ 		{
/*N*/ 			//	richtige Reihenfolge fuer Wertebereich
/*N*/ 			double nTemp = nComp1; nComp1 = nComp2; nComp2 = nTemp;
/*N*/ 		}
/*N*/ 
/*N*/ 	//	Alle Grenzfaelle muessen per ::rtl::math::approxEqual getestet werden!
/*N*/ 
/*N*/ 	BOOL bValid = FALSE;
/*N*/ 	switch (eOp)
/*N*/ 	{
/*?*/ 		case SC_COND_NONE:
/*?*/ 			break;					// immer FALSE;
/*?*/ 		case SC_COND_EQUAL:
/*?*/ 			bValid = ::rtl::math::approxEqual( nArg, nComp1 );
/*?*/ 			break;
/*?*/ 		case SC_COND_NOTEQUAL:
/*?*/ 			bValid = !::rtl::math::approxEqual( nArg, nComp1 );
/*?*/ 			break;
/*?*/ 		case SC_COND_GREATER:
/*?*/ 			bValid = ( nArg > nComp1 ) && !::rtl::math::approxEqual( nArg, nComp1 );
/*?*/ 			break;
/*?*/ 		case SC_COND_EQGREATER:
/*?*/ 			bValid = ( nArg >= nComp1 ) || ::rtl::math::approxEqual( nArg, nComp1 );
/*?*/ 			break;
/*?*/ 		case SC_COND_LESS:
/*?*/ 			bValid = ( nArg < nComp1 ) && !::rtl::math::approxEqual( nArg, nComp1 );
/*?*/ 			break;
/*?*/ 		case SC_COND_EQLESS:
/*?*/ 			bValid = ( nArg <= nComp1 ) || ::rtl::math::approxEqual( nArg, nComp1 );
/*?*/ 			break;
/*?*/ 		case SC_COND_BETWEEN:
/*?*/ 			bValid = ( nArg >= nComp1 && nArg <= nComp2 ) ||
/*?*/ 					 ::rtl::math::approxEqual( nArg, nComp1 ) || ::rtl::math::approxEqual( nArg, nComp2 );
/*?*/ 			break;
/*?*/ 		case SC_COND_NOTBETWEEN:
/*?*/ 			bValid = ( nArg < nComp1 || nArg > nComp2 ) &&
/*?*/ 					 !::rtl::math::approxEqual( nArg, nComp1 ) && !::rtl::math::approxEqual( nArg, nComp2 );
/*?*/ 			break;
/*N*/ 		case SC_COND_DIRECT:
/*N*/ 			bValid = !::rtl::math::approxEqual( nComp1, 0.0 );
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR("unbekannte Operation bei ScConditionEntry");
/*N*/ 			break;
/*N*/ 	}
/*N*/ 	return bValid;
/*N*/ }


/*N*/ BOOL ScConditionEntry::IsCellValid( ScBaseCell* pCell, const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	((ScConditionEntry*)this)->Interpret(rPos);			// Formeln auswerten
/*N*/ 
/*N*/ 	double nArg = 0.0;
/*N*/ 	String aArgStr;
/*N*/ 	BOOL bVal = TRUE;
/*N*/ 
/*N*/ 	if ( pCell )
/*N*/ 	{
/*N*/ 		CellType eType = pCell->GetCellType();
/*N*/ 		switch (eType)
/*N*/ 		{
/*?*/ 			case CELLTYPE_VALUE:
/*?*/ 				nArg = ((ScValueCell*)pCell)->GetValue();
/*?*/ 				break;
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 				{
/*N*/ 					ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 					bVal = pFCell->IsValue();
/*N*/ 					if (bVal)
/*N*/ 						nArg = pFCell->GetValue();
/*N*/ 					else
/*?*/ 						pFCell->GetString(aArgStr);
/*N*/ 				}
/*N*/ 				break;
/*?*/ 			case CELLTYPE_STRING:
/*?*/ 			case CELLTYPE_EDIT:
/*?*/ 				bVal = FALSE;
/*?*/ 				if ( eType == CELLTYPE_STRING )
/*?*/ 					((ScStringCell*)pCell)->GetString(aArgStr);
/*?*/ 				else
/*?*/ 					((ScEditCell*)pCell)->GetString(aArgStr);
/*?*/ 				break;
/*N*/ 
/*N*/ 			default:
/*N*/ 				pCell = NULL;			// Note-Zellen wie leere
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!pCell)
/*N*/ 		if (bIsStr1)
/*N*/ 			bVal = FALSE;				// leere Zellen je nach Bedingung
/*N*/ 
/*N*/ 	if (bVal)
/*N*/ 		return IsValid( nArg );
/*N*/ 	else
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); return FALSE;} //STRIP001 return IsValidStr( aArgStr );
/*N*/ }

/*N*/ String ScConditionEntry::GetExpression( const ScAddress& rCursor, USHORT nIndex,
/*N*/ 										ULONG nNumFmt, BOOL bEnglish,
/*N*/ 										BOOL bCompileXML, BOOL bTextToReal ) const
/*N*/ {
/*N*/ 	String aRet;
/*N*/ 
/*N*/ 	if ( bEnglish && nNumFmt == 0 )
/*N*/ 		nNumFmt = pDoc->GetFormatTable()->GetStandardIndex( LANGUAGE_ENGLISH_US );
/*N*/ 
/*N*/ 	if ( nIndex==0 )
/*N*/ 	{
/*N*/ 		if ( pFormula1 )
/*N*/ 		{
/*N*/ 			ScCompiler aComp(pDoc, rCursor, *pFormula1);
/*N*/ 			aComp.SetCompileEnglish( bEnglish );
/*N*/ 			aComp.SetCompileXML( bCompileXML );
/*N*/ 			aComp.SetImportXML( bTextToReal );			// set only from CompileXML method
/*N*/ 			aComp.CreateStringFromTokenArray( aRet );
/*N*/ 		}
/*N*/ 		else if (bIsStr1)
/*N*/ 		{
/*N*/ 			aRet = '"';
/*N*/ 			aRet += aStrVal1;
/*N*/ 			aRet += '"';
/*N*/ 		}
/*N*/ 		else
/*?*/ 			pDoc->GetFormatTable()->GetInputLineString(nVal1, nNumFmt, aRet);
/*N*/ 	}
/*N*/ 	else if ( nIndex==1 )
/*N*/ 	{
/*N*/ 		if ( pFormula2 )
/*N*/ 		{
/*?*/ 			ScCompiler aComp(pDoc, rCursor, *pFormula2);
/*?*/ 			aComp.SetCompileEnglish( bEnglish );
/*?*/ 			aComp.SetCompileXML( bCompileXML );
/*M*/ 			aComp.SetImportXML( bTextToReal );			// set only from CompileXML method
/*?*/ 			aComp.CreateStringFromTokenArray( aRet );
/*N*/ 		}
/*N*/ 		else if (bIsStr2)
/*N*/ 		{
/*N*/ 			aRet = '"';
/*N*/ 			aRet += aStrVal2;
/*N*/ 			aRet += '"';
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pDoc->GetFormatTable()->GetInputLineString(nVal2, nNumFmt, aRet);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("GetExpression: falscher Index");
/*N*/ 
/*N*/ 	return aRet;
/*N*/ }


/*N*/ void ScConditionEntry::SourceChanged( const ScAddress& rChanged )
/*N*/ {
/*N*/ 	for (USHORT nPass = 0; nPass < 2; nPass++)
/*N*/ 	{
/*N*/ 		ScTokenArray* pFormula = nPass ? pFormula2 : pFormula1;
/*N*/ 		if (pFormula)
/*N*/ 		{
/*N*/ 			pFormula->Reset();
/*N*/ 			ScToken* t;
/*N*/ 			for( t = pFormula->GetNextReference(); t; t = pFormula->GetNextReference() )
/*N*/ 			{
/*N*/ 				SingleDoubleRefProvider aProv( *t );
/*N*/ 				if ( aProv.Ref1.IsColRel() || aProv.Ref1.IsRowRel() || aProv.Ref1.IsTabRel() ||
/*N*/ 					 aProv.Ref2.IsColRel() || aProv.Ref2.IsRowRel() || aProv.Ref2.IsTabRel() )
/*N*/ 				{
/*N*/ 					//	absolut muss getroffen sein, relativ bestimmt Bereich
/*N*/ 
/*N*/ 					BOOL bHit = TRUE;
/*N*/ 					INT16 nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ 
/*N*/ 					if ( aProv.Ref1.IsColRel() )
/*N*/ 						nCol2 = rChanged.Col() - aProv.Ref1.nRelCol;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bHit &= ( rChanged.Col() >= aProv.Ref1.nCol );
/*N*/ 						nCol2 = MAXCOL;
/*N*/ 					}
/*N*/ 					if ( aProv.Ref1.IsRowRel() )
/*N*/ 						nRow2 = rChanged.Row() - aProv.Ref1.nRelRow;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bHit &= ( rChanged.Row() >= aProv.Ref1.nRow );
/*N*/ 						nRow2 = MAXROW;
/*N*/ 					}
/*N*/ 					if ( aProv.Ref1.IsTabRel() )
/*N*/ 						nTab2 = rChanged.Tab() - aProv.Ref1.nRelTab;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bHit &= ( rChanged.Tab() >= aProv.Ref1.nTab );
/*N*/ 						nTab2 = MAXTAB;
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( aProv.Ref2.IsColRel() )
/*N*/ 						nCol1 = rChanged.Col() - aProv.Ref2.nRelCol;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bHit &= ( rChanged.Col() <= aProv.Ref2.nCol );
/*N*/ 						nCol1 = 0;
/*N*/ 					}
/*N*/ 					if ( aProv.Ref2.IsRowRel() )
/*N*/ 						nRow1 = rChanged.Row() - aProv.Ref2.nRelRow;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bHit &= ( rChanged.Row() <= aProv.Ref2.nRow );
/*N*/ 						nRow1 = 0;
/*N*/ 					}
/*N*/ 					if ( aProv.Ref2.IsTabRel() )
/*N*/ 						nTab1 = rChanged.Tab() - aProv.Ref2.nRelTab;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bHit &= ( rChanged.Tab() <= aProv.Ref2.nTab );
/*N*/ 						nTab1 = 0;
/*N*/ 					}
/*N*/ 
/*N*/ 					if ( bHit )
/*N*/ 					{
/*N*/ 						//!	begrenzen
/*N*/ 
/*N*/ 						ScRange aPaint( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 );
/*N*/ 
/*N*/ 						//	kein Paint, wenn es nur die Zelle selber ist
/*N*/ 						if ( aPaint.aStart != rChanged || aPaint.aEnd != rChanged )
/*N*/ 							DataChanged( &aPaint );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScConditionEntry::DataChanged( const ScRange* pModified ) const
/*N*/ {
/*N*/ 	// nix
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScCondFormatEntry::ScCondFormatEntry( ScConditionMode eOper,
/*N*/ 										const String& rExpr1, const String& rExpr2,
/*N*/ 										ScDocument* pDocument, const ScAddress& rPos,
/*N*/ 										const String& rStyle,
/*N*/ 										BOOL bCompileEnglish, BOOL bCompileXML ) :
/*N*/ 	ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, bCompileEnglish, bCompileXML ),
/*N*/ 	aStyleName( rStyle ),
/*N*/ 	pParent( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScCondFormatEntry::ScCondFormatEntry( const ScCondFormatEntry& r ) :
/*N*/ 	ScConditionEntry( r ),
/*N*/ 	aStyleName( r.aStyleName ),
/*N*/ 	pParent( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScCondFormatEntry::ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r ) :
/*N*/ 	ScConditionEntry( pDocument, r ),
/*N*/ 	aStyleName( r.aStyleName ),
/*N*/ 	pParent( NULL )
/*N*/ {
/*N*/ }

/*N*/ ScCondFormatEntry::ScCondFormatEntry( SvStream& rStream, ScMultipleReadHeader& rHdr,
/*N*/ 										ScDocument* pDocument ) :
/*N*/ 	ScConditionEntry( rStream, rHdr, pDocument ),
/*N*/ 	pParent( NULL )
/*N*/ {
/*N*/ 	//	im Datei-Header sind getrennte Eintraege fuer ScConditionEntry und ScCondFormatEntry
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 	rStream.ReadByteString( aStyleName, rStream.GetStreamCharSet() );
/*N*/ 	rHdr.EndEntry();
/*N*/ }

/*N*/ void ScCondFormatEntry::Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const
/*N*/ {
/*N*/ 	//	im Datei-Header sind getrennte Eintraege fuer ScConditionEntry und ScCondFormatEntry
/*N*/ 
/*N*/ 	StoreCondition( rStream, rHdr );
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 	rStream.WriteByteString( aStyleName, rStream.GetStreamCharSet() );
/*N*/ 	rHdr.EndEntry();
/*N*/ }


/*N*/ int ScCondFormatEntry::operator== ( const ScCondFormatEntry& r ) const
/*N*/ {
/*N*/ 	return ScConditionEntry::operator==( r ) &&
/*N*/ 			aStyleName == r.aStyleName;
/*N*/ 
/*N*/ 	//	Range wird nicht verglichen
/*N*/ }

/*N*/ ScCondFormatEntry::~ScCondFormatEntry()
/*N*/ {
/*N*/ }

/*N*/ void ScCondFormatEntry::DataChanged( const ScRange* pModified ) const
/*N*/ {
/*N*/ 	if ( pParent )
/*N*/ 		pParent->DoRepaint( pModified );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScConditionalFormat::ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	pAreas( NULL ),
/*N*/ 	nKey( nNewKey ),
/*N*/ 	ppEntries( NULL ),
/*N*/ 	nEntryCount( 0 )
/*N*/ {
/*N*/ }

/*N*/ ScConditionalFormat::ScConditionalFormat(const ScConditionalFormat& r) :
/*N*/ 	pDoc( r.pDoc ),
/*N*/ 	pAreas( NULL ),
/*N*/ 	nKey( r.nKey ),
/*N*/ 	ppEntries( NULL ),
/*N*/ 	nEntryCount( r.nEntryCount )
/*N*/ {
/*N*/ 	if (nEntryCount)
/*N*/ 	{
/*N*/ 		ppEntries = new ScCondFormatEntry*[nEntryCount];
/*N*/ 		for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		{
/*N*/ 			ppEntries[i] = new ScCondFormatEntry(*r.ppEntries[i]);
/*N*/ 			ppEntries[i]->SetParent(this);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ ScConditionalFormat* ScConditionalFormat::Clone(ScDocument* pNewDoc) const
/*N*/ {
/*N*/ 	// echte Kopie der Formeln (fuer Ref-Undo / zwischen Dokumenten)
/*N*/ 
/*N*/ 	if (!pNewDoc)
/*N*/ 		pNewDoc = pDoc;
/*N*/ 
/*N*/ 	ScConditionalFormat* pNew = new ScConditionalFormat(nKey, pNewDoc);
/*N*/ 	DBG_ASSERT(!pNew->ppEntries, "wo kommen die Eintraege her?");
/*N*/ 
/*N*/ 	if (nEntryCount)
/*N*/ 	{
/*N*/ 		pNew->ppEntries = new ScCondFormatEntry*[nEntryCount];
/*N*/ 		for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		{
/*N*/ 			pNew->ppEntries[i] = new ScCondFormatEntry( pNewDoc, *ppEntries[i] );
/*N*/ 			pNew->ppEntries[i]->SetParent(pNew);
/*N*/ 		}
/*N*/ 		pNew->nEntryCount = nEntryCount;
/*N*/ 	}
/*N*/ 
/*N*/ 	return pNew;
/*N*/ }

/*N*/ ScConditionalFormat::ScConditionalFormat(SvStream& rStream, ScMultipleReadHeader& rHdr,
/*N*/ 											ScDocument* pDocument) :
/*N*/ 	pDoc( pDocument ),
/*N*/ 	pAreas( NULL ),
/*N*/ 	ppEntries( NULL ),
/*N*/ 	nEntryCount( 0 )
/*N*/ {
/*N*/ 	//	ein Eintrag im Header fuer die ScConditionalFormat-Daten,
/*N*/ 	//	je zwei Eintraege fuer jede Bedingung (ScConditionEntry und ScCondFormatEntry)
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	rStream >> nKey;
/*N*/ 	rStream >> nEntryCount;
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 		//	Eintraege laden
/*N*/ 
/*N*/ 	if (nEntryCount)
/*N*/ 	{
/*N*/ 		ppEntries = new ScCondFormatEntry*[nEntryCount];
/*N*/ 		for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		{
/*N*/ 			ppEntries[i] = new ScCondFormatEntry(rStream, rHdr, pDocument);
/*N*/ 			ppEntries[i]->SetParent(this);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScConditionalFormat::Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const
/*N*/ {
/*N*/ 	//	ein Eintrag im Header fuer die ScConditionalFormat-Daten,
/*N*/ 	//	je zwei Eintraege fuer jede Bedingung (ScConditionEntry und ScCondFormatEntry)
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	rStream << nKey;
/*N*/ 	rStream << nEntryCount;
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 		//	Eintraege speichern
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		ppEntries[i]->Store(rStream, rHdr);
/*N*/ }

/*N*/ BOOL ScConditionalFormat::EqualEntries( const ScConditionalFormat& r ) const
/*N*/ {
/*N*/ 	if ( nEntryCount != r.nEntryCount )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	//!	auf gleiche Eintraege in anderer Reihenfolge testen ???
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		if ( ! (*ppEntries[i] == *r.ppEntries[i]) )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void ScConditionalFormat::AddEntry( const ScCondFormatEntry& rNew )
/*N*/ {
/*N*/ 	ScCondFormatEntry** ppNew = new ScCondFormatEntry*[nEntryCount+1];
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		ppNew[i] = ppEntries[i];
/*N*/ 	ppNew[nEntryCount] = new ScCondFormatEntry(rNew);
/*N*/ 	ppNew[nEntryCount]->SetParent(this);
/*N*/ 	++nEntryCount;
/*N*/ 	delete[] ppEntries;
/*N*/ 	ppEntries = ppNew;
/*N*/ }

/*N*/ ScConditionalFormat::~ScConditionalFormat()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		delete ppEntries[i];
/*N*/ 	delete[] ppEntries;
/*N*/ 
/*N*/ 	delete pAreas;
/*N*/ }

/*N*/ const ScCondFormatEntry* ScConditionalFormat::GetEntry( USHORT nPos ) const
/*N*/ {
/*N*/ 	if ( nPos < nEntryCount )
/*N*/ 		return ppEntries[nPos];
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ const String& ScConditionalFormat::GetCellStyle( ScBaseCell* pCell, const ScAddress& rPos ) const
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		if ( ppEntries[i]->IsCellValid( pCell, rPos ) )
/*N*/ 			return ppEntries[i]->GetStyle();
/*N*/ 
/*N*/ 	return EMPTY_STRING;
/*N*/ }

/*N*/ void lcl_Extend( ScRange& rRange, ScDocument* pDoc, BOOL bLines )
/*N*/ {
/*N*/ 	USHORT nTab = rRange.aStart.Tab();
/*N*/ 	DBG_ASSERT(rRange.aEnd.Tab() == nTab, "lcl_Extend - mehrere Tabellen?");
/*N*/ 
/*N*/ 	USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	USHORT nStartRow = rRange.aStart.Row();
/*N*/ 	USHORT nEndCol = rRange.aEnd.Col();
/*N*/ 	USHORT nEndRow = rRange.aEnd.Row();
/*N*/ 
/*N*/ 	BOOL bEx = pDoc->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
/*N*/ 
/*N*/ 	if (bLines)
/*N*/ 	{
/*N*/ 		if (nStartCol > 0)	  --nStartCol;
/*N*/ 		if (nStartRow > 0)	  --nStartRow;
/*N*/ 		if (nEndCol < MAXCOL) ++nEndCol;
/*N*/ 		if (nEndRow < MAXROW) ++nEndRow;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bEx || bLines )
/*N*/ 	{
/*N*/ 		rRange.aStart.Set( nStartCol, nStartRow, nTab );
/*N*/ 		rRange.aEnd.Set( nEndCol, nEndRow, nTab );
/*N*/ 	}
/*N*/ }

/*N*/ BOOL lcl_CutRange( ScRange& rRange, const ScRange& rOther )
/*N*/ {
/*N*/ 	rRange.Justify();
/*N*/ 	ScRange aCmpRange = rOther;
/*N*/ 	aCmpRange.Justify();
/*N*/ 
/*N*/ 	if ( rRange.aStart.Col() <= aCmpRange.aEnd.Col() &&
/*N*/ 		 rRange.aEnd.Col() >= aCmpRange.aStart.Col() &&
/*N*/ 		 rRange.aStart.Row() <= aCmpRange.aEnd.Row() &&
/*N*/ 		 rRange.aEnd.Row() >= aCmpRange.aStart.Row() &&
/*N*/ 		 rRange.aStart.Tab() <= aCmpRange.aEnd.Tab() &&
/*N*/ 		 rRange.aEnd.Tab() >= aCmpRange.aStart.Tab() )
/*N*/ 	{
/*N*/ 		if ( rRange.aStart.Col() < aCmpRange.aStart.Col() )
/*N*/ 			rRange.aStart.SetCol( aCmpRange.aStart.Col() );
/*N*/ 		if ( rRange.aStart.Row() < aCmpRange.aStart.Row() )
/*N*/ 			rRange.aStart.SetRow( aCmpRange.aStart.Row() );
/*N*/ 		if ( rRange.aStart.Tab() < aCmpRange.aStart.Tab() )
/*N*/ 			rRange.aStart.SetTab( aCmpRange.aStart.Tab() );
/*N*/ 		if ( rRange.aEnd.Col() > aCmpRange.aEnd.Col() )
/*N*/ 			rRange.aEnd.SetCol( aCmpRange.aEnd.Col() );
/*N*/ 		if ( rRange.aEnd.Row() > aCmpRange.aEnd.Row() )
/*N*/ 			rRange.aEnd.SetRow( aCmpRange.aEnd.Row() );
/*N*/ 		if ( rRange.aEnd.Tab() > aCmpRange.aEnd.Tab() )
/*N*/ 			rRange.aEnd.SetTab( aCmpRange.aEnd.Tab() );
/*N*/ 
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;		// ausserhalb
/*N*/ }

/*N*/ void ScConditionalFormat::DoRepaint( const ScRange* pModified )
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	SfxObjectShell* pSh = pDoc->GetDocumentShell();
/*N*/ 	if (pSh)
/*N*/ 	{
/*N*/ 		//	Rahmen/Schatten enthalten?
/*N*/ 		//	(alle Bedingungen testen)
/*N*/ 		BOOL bExtend = FALSE;
/*N*/ 		BOOL bRotate = FALSE;
/*N*/ 		for (i=0; i<nEntryCount; i++)
/*N*/ 		{
/*N*/ 			String aStyle = ppEntries[i]->GetStyle();
/*N*/ 			if (aStyle.Len())
/*N*/ 			{
/*N*/ 				SfxStyleSheetBase* pStyleSheet =
/*N*/ 					pDoc->GetStyleSheetPool()->Find( aStyle, SFX_STYLE_FAMILY_PARA );
/*N*/ 				if ( pStyleSheet )
/*N*/ 				{
/*N*/ 					const SfxItemSet& rSet = pStyleSheet->GetItemSet();
/*N*/ 					if (rSet.GetItemState( ATTR_BORDER, TRUE ) == SFX_ITEM_SET ||
/*N*/ 						rSet.GetItemState( ATTR_SHADOW, TRUE ) == SFX_ITEM_SET)
/*N*/ 					{
/*N*/ 						bExtend = TRUE;
/*N*/ 					}
/*N*/ 					if (rSet.GetItemState( ATTR_ROTATE_VALUE, TRUE ) == SFX_ITEM_SET ||
/*N*/ 						rSet.GetItemState( ATTR_ROTATE_MODE, TRUE ) == SFX_ITEM_SET)
/*N*/ 					{
/*N*/ 						bRotate = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if (!pAreas)		//	RangeList ggf. holen
/*N*/ 		{
/*N*/ 			pAreas = new ScRangeList;
/*N*/ 			pDoc->FindConditionalFormat( nKey, *pAreas );
/*N*/ 		}
/*N*/ 		USHORT nCount = (USHORT) pAreas->Count();
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 		{
/*N*/ 			ScRange aRange = *pAreas->GetObject(i);
/*N*/ 			BOOL bDo = TRUE;
/*N*/ 			if ( pModified )
/*N*/ 			{
/*N*/ 				if ( !lcl_CutRange( aRange, *pModified ) )
/*N*/ 					bDo = FALSE;
/*N*/ 			}
/*N*/ 			if (bDo)
/*N*/ 			{
/*N*/ 				lcl_Extend( aRange, pDoc, bExtend );		// zusammengefasste und bExtend
/*N*/ 				if ( bRotate )
/*N*/ 				{
/*?*/ 					aRange.aStart.SetCol(0);
/*?*/ 					aRange.aEnd.SetCol(MAXCOL);		// gedreht: ganze Zeilen
/*N*/ 				}
/*N*/ 
/*N*/ 				// gedreht -> ganze Zeilen
/*N*/ 				if ( aRange.aStart.Col() != 0 || aRange.aEnd.Col() != MAXCOL )
/*N*/ 				{
/*N*/ 					if ( pDoc->HasAttrib( 0,aRange.aStart.Row(),aRange.aStart.Tab(),
/*N*/ 											MAXCOL,aRange.aEnd.Row(),aRange.aEnd.Tab(),
/*N*/ 											HASATTR_ROTATE ) )
/*N*/ 					{
/*?*/ 						aRange.aStart.SetCol(0);
/*?*/ 						aRange.aEnd.SetCol(MAXCOL);
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				pSh->Broadcast( ScPaintHint( aRange, PAINT_GRID ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScConditionalFormat::InvalidateArea()
/*N*/ {
/*N*/ 	delete pAreas;
/*N*/ 	pAreas = NULL;
/*N*/ }


/*N*/ void ScConditionalFormat::CompileXML()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		ppEntries[i]->CompileXML();
/*N*/ }

/*N*/ void ScConditionalFormat::UpdateReference( UpdateRefMode eUpdateRefMode,
/*N*/ 								const ScRange& rRange, short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		ppEntries[i]->UpdateReference(eUpdateRefMode, rRange, nDx, nDy, nDz);
/*N*/ 
/*N*/ 	delete pAreas;		// aus dem AttrArray kommt beim Einfuegen/Loeschen kein Aufruf
/*N*/ 	pAreas = NULL;
/*N*/ }


/*N*/ void ScConditionalFormat::SourceChanged( const ScAddress& rAddr )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		ppEntries[i]->SourceChanged( rAddr );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScConditionalFormatList::ScConditionalFormatList(const ScConditionalFormatList& rList)
/*N*/ {
/*N*/ 	//	fuer Ref-Undo - echte Kopie mit neuen Tokens!
/*N*/ 
/*N*/ 	USHORT nCount = rList.Count();
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		InsertNew( rList[i]->Clone() );
/*N*/ 
/*N*/ 	//!		sortierte Eintraege aus rList schneller einfuegen ???
/*N*/ }

/*N*/ void ScConditionalFormatList::Load( SvStream& rStream, ScDocument* pDocument )
/*N*/ {
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nNewCount;
/*N*/ 	rStream >> nNewCount;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nNewCount; i++)
/*N*/ 	{
/*N*/ 		ScConditionalFormat* pNew = new ScConditionalFormat( rStream, aHdr, pDocument );
/*N*/ 		InsertNew( pNew );
/*N*/ 	}
/*N*/ }

/*N*/ void ScConditionalFormatList::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nCount = Count();
/*N*/ 	USHORT nUsed = 0;
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 		if ((*this)[i]->IsUsed())
/*N*/ 			++nUsed;
/*N*/ 
/*N*/ 	rStream << nUsed;		// Anzahl der gespeicherten
/*N*/ 
/*N*/ 	for (i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const ScConditionalFormat* pForm = (*this)[i];
/*N*/ 		if (pForm->IsUsed())
/*N*/ 			pForm->Store( rStream, aHdr );
/*N*/ 	}
/*N*/ }

/*N*/ ScConditionalFormat* ScConditionalFormatList::GetFormat( sal_uInt32 nKey )
/*N*/ {
/*N*/ 	//!	binaer suchen
/*N*/ 
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		if ((*this)[i]->GetKey() == nKey)
/*N*/ 			return (*this)[i];
/*N*/ 
/*N*/ 	DBG_ERROR("ScConditionalFormatList: Eintrag nicht gefunden");
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void ScConditionalFormatList::ResetUsed()
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		(*this)[i]->SetUsed(FALSE);
/*N*/ }


/*N*/ void ScConditionalFormatList::CompileXML()
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		(*this)[i]->CompileXML();
/*N*/ }

/*N*/ void ScConditionalFormatList::UpdateReference( UpdateRefMode eUpdateRefMode,
/*N*/ 								const ScRange& rRange, short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		(*this)[i]->UpdateReference( eUpdateRefMode, rRange, nDx, nDy, nDz );
/*N*/ }


/*N*/ void ScConditionalFormatList::SourceChanged( const ScAddress& rAddr )
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		(*this)[i]->SourceChanged( rAddr );
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
