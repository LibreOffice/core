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

#include <bf_sfx2/app.hxx>
#include <bf_sfx2/objsh.hxx>
#include "bf_basic/sbmeth.hxx"
#include "bf_basic/sbstar.hxx"
#include <bf_svtools/zforlist.hxx>
#include <tools/rcid.h>
#include <tools/solar.h>
#include <tools/urlobj.hxx>
#include <rtl/math.hxx>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "document.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "refupdat.hxx"
#include "scresid.hxx"
#include "bf_sc.hrc"
#include "globstr.hrc"
#include "cell.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
namespace binfilter {


/*N*/ String* ScCompiler::pSymbolTableNative = NULL;
/*N*/ String* ScCompiler::pSymbolTableEnglish = NULL;
/*N*/ USHORT  ScCompiler::nAnzStrings = 0;
/*N*/ ULONG* ScCompiler::pCharTable = 0;
/*N*/ ScOpCodeHashMap* ScCompiler::pSymbolHashMapNative = NULL;
/*N*/ ScOpCodeHashMap* ScCompiler::pSymbolHashMapEnglish = NULL;

/*N*/ enum ScanState
/*N*/ {
/*N*/     ssGetChar,
/*N*/     ssGetBool,
/*N*/     ssGetString,
/*N*/     ssSkipString, 
/*N*/     ssGetIdent,
/*N*/     ssStop
/*N*/ };

/*N*/ struct ScArrayStack
/*N*/ {
/*N*/ 	ScArrayStack* pNext;
/*N*/ 	ScTokenArray* pArr;
/*N*/ 	BOOL bTemp;
/*N*/ };

/*N*/ static sal_Char* pInternal[ 5 ] = { "GAME", "SPEW", "TTT", "STARCALCTEAM", "ANTWORT" };


/////////////////////////////////////////////////////////////////////////

/*N*/ short lcl_GetRetFormat( OpCode eOpCode )
/*N*/ {
/*N*/ 	switch (eOpCode)
/*N*/ 	{
/*N*/ 		case ocEqual:
/*N*/ 		case ocNotEqual:
/*N*/ 		case ocLess:
/*N*/ 		case ocGreater:
/*N*/ 		case ocLessEqual:
/*N*/ 		case ocGreaterEqual:
/*N*/ 		case ocAnd:
/*N*/ 		case ocOr:
/*N*/ 		case ocNot:
/*N*/ 		case ocTrue:
/*N*/ 		case ocFalse:
/*N*/ 		case ocIsEmpty:
/*N*/ 		case ocIsString:
/*N*/ 		case ocIsNonString:
/*N*/ 		case ocIsLogical:
/*N*/ 		case ocIsRef:
/*N*/ 		case ocIsValue:
/*N*/ 		case ocIsFormula:
/*N*/ 		case ocIsNV:
/*N*/ 		case ocIsErr:
/*N*/ 		case ocIsError:
/*N*/ 		case ocIsEven:
/*N*/ 		case ocIsOdd:
/*N*/ 		case ocExact:
/*N*/ 			return NUMBERFORMAT_LOGICAL;
/*N*/ 		case ocGetActDate:
/*N*/ 		case ocGetDate:
/*N*/ 		case ocEasterSunday :
/*N*/ 			return NUMBERFORMAT_DATE;
/*N*/ 		case ocGetActTime:
/*N*/ 			return NUMBERFORMAT_DATETIME;
/*N*/ 		case ocGetTime:
/*N*/ 			return NUMBERFORMAT_TIME;
/*N*/ 		case ocNBW:
/*N*/ 		case ocBW:
/*N*/ 		case ocDIA:
/*N*/ 		case ocGDA:
/*N*/ 		case ocGDA2:
/*N*/ 		case ocVBD:
/*N*/ 		case ocLIA:
/*N*/ 		case ocRMZ:
/*N*/ 		case ocZW:
/*N*/ 		case ocZinsZ:
/*N*/ 		case ocKapz:
/*N*/ 		case ocKumZinsZ:
/*N*/ 		case ocKumKapZ:
/*N*/ 			return NUMBERFORMAT_CURRENCY;
/*N*/ 		case ocZins:
/*N*/ 		case ocIKV:
/*N*/ 		case ocMIRR:
/*N*/ 		case ocZGZ:
/*N*/ 		case ocEffektiv:
/*N*/ 		case ocNominal:
/*N*/         case ocPercentSign:
/*N*/ 			return NUMBERFORMAT_PERCENT;
/*N*/ //		case ocSum:
/*N*/ //		case ocSumSQ:
/*N*/ //		case ocProduct:
/*N*/ //		case ocAverage:
/*N*/ //			return -1;
/*N*/ 		default:
/*N*/ 			return NUMBERFORMAT_NUMBER;
/*N*/ 	}
/*N*/ 	return NUMBERFORMAT_NUMBER;
/*N*/ }

/////////////////////////////////////////////////////////////////////////

/*N*/ class ScOpCodeList : public Resource		// temp object fuer Resource
/*N*/ {
/*N*/ public:
/*N*/     ScOpCodeList( USHORT, String[], ScOpCodeHashMap& );
/*N*/ };

/*N*/ ScOpCodeList::ScOpCodeList( USHORT nRID, String pSymbolTable[], ScOpCodeHashMap& rHashMap )
/*N*/ 		:
/*N*/ 		Resource( ScResId( nRID ) )
/*N*/ {
/*N*/ 	for (USHORT i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; i++)
/*N*/ 	{
/*N*/ 		ScResId aRes(i);
/*N*/ 		aRes.SetRT(RSC_STRING);
/*N*/ 		if (IsAvailableRes(aRes))
/*N*/         {
/*N*/             pSymbolTable[i] = aRes;
/*N*/             rHashMap.insert( ScOpCodeHashMap::value_type( pSymbolTable[i], (OpCode) i ) );
/*N*/         }
/*N*/ 	}
/*N*/ 	FreeResource();
/*N*/ }


/*N*/ class ScCompilerRecursionGuard
/*N*/ {
/*N*/ private:
/*N*/             short&              rRecursion;
/*N*/ public:
/*N*/                                 ScCompilerRecursionGuard( short& rRec )
/*N*/                                     : rRecursion( rRec ) { ++rRecursion; }
/*N*/                                 ~ScCompilerRecursionGuard() { --rRecursion; }
/*N*/ };


/*N*/ void ScCompiler::Init()
/*N*/ {
/*N*/     pSymbolTableNative = new String[SC_OPCODE_LAST_OPCODE_ID+1];
/*N*/     pSymbolHashMapNative = new ScOpCodeHashMap( SC_OPCODE_LAST_OPCODE_ID+1 );
/*N*/     ScOpCodeList aOpCodeListNative( RID_SC_FUNCTION_NAMES, pSymbolTableNative,
/*N*/             *pSymbolHashMapNative );
/*N*/     nAnzStrings = SC_OPCODE_LAST_OPCODE_ID+1;
/*N*/ 
/*N*/     pCharTable = new ULONG [128];
/*N*/     USHORT i;
/*N*/     for (i = 0; i < 128; i++)
/*N*/         pCharTable[i] = SC_COMPILER_C_ILLEGAL;
/*N*/     /*   */     pCharTable[32] = SC_COMPILER_C_CHAR_DONTCARE | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* ! */     pCharTable[33] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* " */     pCharTable[34] = SC_COMPILER_C_CHAR_STRING | SC_COMPILER_C_STRING_SEP;
/*N*/     /* # */     pCharTable[35] = SC_COMPILER_C_WORD_SEP;
/*N*/     /* $ */     pCharTable[36] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT;
/*N*/     /* % */     pCharTable[37] = SC_COMPILER_C_VALUE;
/*N*/     /* & */     pCharTable[38] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* ' */     pCharTable[39] = SC_COMPILER_C_NAME_SEP;
/*N*/     /* ( */     pCharTable[40] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* ) */     pCharTable[41] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* * */     pCharTable[42] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* + */     pCharTable[43] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_SIGN;
/*N*/     /* , */     pCharTable[44] = SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_VALUE;
/*N*/     /* - */     pCharTable[45] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_SIGN;
/*N*/     /* . */     pCharTable[46] = SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_VALUE | SC_COMPILER_C_IDENT;
/*N*/     /* / */     pCharTable[47] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     for (i = 48; i < 58; i++)
/*N*/     /* 0-9 */   pCharTable[i] = SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_WORD | SC_COMPILER_C_VALUE | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_VALUE | SC_COMPILER_C_IDENT;
/*N*/     /* : */     pCharTable[58] = SC_COMPILER_C_WORD | SC_COMPILER_C_IDENT;
/*N*/     /* ; */     pCharTable[59] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* < */     pCharTable[60] = SC_COMPILER_C_CHAR_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* = */     pCharTable[61] = SC_COMPILER_C_CHAR | SC_COMPILER_C_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* > */     pCharTable[62] = SC_COMPILER_C_CHAR_BOOL | SC_COMPILER_C_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* ? */     pCharTable[63] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD;
/*N*/     /* @ */     // FREI
/*N*/     for (i = 65; i < 91; i++)
/*N*/     /* A-Z */   pCharTable[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT;
/*N*/     /* [ */     // FREI
/*N*/     /* \ */     // FREI
/*N*/     /* ] */     // FREI
/*N*/     /* ^ */     pCharTable[94] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/*N*/     /* _ */     pCharTable[95] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT;
/*N*/     /* ` */     // FREI
/*N*/     for (i = 97; i < 123; i++)
/*N*/     /* a-z */   pCharTable[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT;
/*N*/     /* { */     // FREI
/*N*/     /* | */     // FREI
/*N*/     /* } */     // FREI
/*N*/     /* ~ */     // FREI
/*N*/     /* 127 */   // FREI
/*N*/ }

/*N*/ void ScCompiler::DeInit()
/*N*/ {
/*N*/ 	if (pSymbolTableNative)
/*N*/ 	{
/*N*/ 		delete [] pSymbolTableNative;
/*N*/ 		pSymbolTableNative = NULL;
/*N*/ 	}
/*N*/ 	if (pSymbolTableEnglish)
/*N*/ 	{
/*?*/ 		delete [] pSymbolTableEnglish;
/*?*/ 		pSymbolTableEnglish = NULL;
/*N*/ 	}
/*N*/     if ( pSymbolHashMapNative )
/*N*/     {
/*?*/         pSymbolHashMapNative->clear();
/*?*/         delete pSymbolHashMapNative;
/*?*/         pSymbolHashMapNative = NULL;
/*N*/     }
/*N*/     if ( pSymbolHashMapEnglish )
/*N*/     {
/*?*/         pSymbolHashMapEnglish->clear();
/*?*/         delete pSymbolHashMapEnglish;
/*?*/         pSymbolHashMapEnglish = NULL;
/*N*/     }
/*N*/ 	delete [] pCharTable;
/*N*/ 	pCharTable = NULL;
/*N*/ }

/*N*/ void ScCompiler::SetCompileEnglish( BOOL bCompileEnglish )
/*N*/ {
/*N*/ 	if ( bCompileEnglish )
/*N*/ 	{
/*N*/ 		if ( !pSymbolTableEnglish )
/*N*/ 		{
/*N*/ 			pSymbolTableEnglish = new String[SC_OPCODE_LAST_OPCODE_ID+1];
/*N*/             pSymbolHashMapEnglish = new ScOpCodeHashMap( SC_OPCODE_LAST_OPCODE_ID+1 );
/*N*/             ScOpCodeList aOpCodeListEnglish( RID_SC_FUNCTION_NAMES_ENGLISH,
/*N*/                 pSymbolTableEnglish, *pSymbolHashMapEnglish );
/*N*/         }
/*N*/         pSymbolTable = pSymbolTableEnglish;
/*N*/         pSymbolHashMap = pSymbolHashMapEnglish;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         pSymbolTable = pSymbolTableNative;
/*N*/         pSymbolHashMap = pSymbolHashMapNative;
/*N*/     }
/*N*/ }

//-----------------------Funktionen der Klasse ScCompiler----------------------

/*N*/ ScCompiler::ScCompiler( ScDocument* pDocument, const ScAddress& rPos,
/*N*/ 						const ScTokenArray& rArr )
/*N*/ 		:
/*N*/ 		aPos( rPos ),
/*N*/ 		pSymbolTable( pSymbolTableNative ),
/*N*/         pSymbolHashMap( pSymbolHashMapNative ),
/*N*/ 		nRecursion(0),
/*N*/ 		bAutoCorrect( FALSE ),
/*N*/ 		bCorrected( FALSE ),
/*N*/ 		bCompileForFAP( FALSE ),
/*N*/ 		bIgnoreErrors( FALSE ),
/*N*/ 		bCompileXML( FALSE ),
/*N*/ 		bImportXML ( FALSE )
/*N*/ {
/*N*/ 	if (!nAnzStrings)
/*?*/ 		Init();
/*N*/ 	pArr = (ScTokenArray*) &rArr;
/*N*/ 	pDoc = pDocument;
/*N*/ 	nMaxTab = pDoc->GetTableCount() - 1;
/*N*/ 	pStack = NULL;
/*N*/ 	nNumFmt = NUMBERFORMAT_UNDEFINED;
/*N*/ }

/*N*/ ScCompiler::ScCompiler(ScDocument* pDocument, const ScAddress& rPos )
/*N*/ 		:
/*N*/ 		aPos( rPos ),
/*N*/ 		pSymbolTable( pSymbolTableNative ),
/*N*/         pSymbolHashMap( pSymbolHashMapNative ),
/*N*/ 		nRecursion(0),
/*N*/ 		bAutoCorrect( FALSE ),
/*N*/ 		bCorrected( FALSE ),
/*N*/ 		bCompileForFAP( FALSE ),
/*N*/ 		bIgnoreErrors( FALSE ),
/*N*/ 		bCompileXML( FALSE ),
/*N*/ 		bImportXML ( FALSE )
/*N*/ {
/*N*/ 	if (!nAnzStrings)
/*?*/ 		Init();
/*N*/ 	pDoc = pDocument;
/*N*/ 	nMaxTab = pDoc->GetTableCount() - 1;
/*N*/ 	pStack = NULL;
/*N*/ 	nNumFmt = NUMBERFORMAT_UNDEFINED;
/*N*/ }


/*N*/ void ScCompiler::MakeColStr( ::rtl::OUStringBuffer& rBuffer, USHORT nCol )
/*N*/ {
/*N*/ 	if ( nCol > MAXCOL )
/*?*/ 		rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (nCol < 26)
/*N*/ 			rBuffer.append(  sal_Unicode('A' + (sal_uChar) nCol));
/*N*/ 		else
/*N*/ 		{
/*?*/ 			USHORT nLoCol = nCol % 26;
/*?*/ 			USHORT nHiCol = (nCol / 26) - 1;
/*?*/             rBuffer.append( sal_Unicode('A' + (sal_uChar)nHiCol) );
/*?*/             rBuffer.append( sal_Unicode('A' + (sal_uChar)nLoCol) );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScCompiler::MakeRowStr( ::rtl::OUStringBuffer& rBuffer, USHORT nRow )
/*N*/ {
/*N*/ 	if ( nRow > MAXROW )
/*?*/ 		rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*N*/ 	else
/*N*/ 		rBuffer.append(sal_Int32(nRow + 1));
/*N*/ }

/*N*/ String ScCompiler::MakeTabStr( USHORT nTab, String& aDoc )
/*N*/ {
/*N*/ 	String aString;
/*N*/ 	if (!pDoc->GetName(nTab, aString))
/*?*/ 		aString = ScGlobal::GetRscString(STR_NO_REF_TABLE);
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( aString.GetChar(0) == '\'' )
/*N*/ 		{	// "'Doc'#Tab"
/*N*/ 			xub_StrLen nPos, nLen = 1;
/*N*/ 			while( (nPos = aString.Search( '\'', nLen )) != STRING_NOTFOUND )
/*N*/ 				nLen = nPos + 1;
/*N*/ 			if ( aString.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
/*N*/ 			{
/*N*/ 				aDoc = aString.Copy( 0, nLen + 1 );
/*N*/ 				aString.Erase( 0, nLen + 1 );
/*N*/ 				aDoc = INetURLObject::decode( aDoc, INET_HEX_ESCAPE,
/*N*/ 					INetURLObject::DECODE_UNAMBIGUOUS );
/*N*/ 			}
/*N*/ 			else
/*?*/ 				aDoc.Erase();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aDoc.Erase();
/*N*/ 		CheckTabQuotes( aString );
/*N*/ 	}
/*N*/ 	aString += '.';
/*N*/ 	return aString;
/*N*/ }

/*N*/ void ScCompiler::CheckTabQuotes( String& rString )
/*N*/ {
/*N*/ 	register const sal_Unicode* p = rString.GetBuffer();
/*N*/ 	register const sal_Unicode* const pEnd = p + rString.Len();
/*N*/ 	while ( p < pEnd )
/*N*/ 	{
/*N*/ 		if( !IsWordChar( *p ) )
/*N*/ 		{
/*N*/ 			rString.Insert( '\'', 0 );
/*N*/ 			rString += '\'';
/*N*/             return ;
/*N*/ 		}
/*N*/ 		p++;
/*N*/ 	}
/*N*/     if ( CharClass::isAsciiNumeric( rString ) )
/*N*/     {
/*N*/         rString.Insert( '\'', 0 );
/*N*/         rString += '\'';
/*N*/     }
/*N*/ }

/*N*/ void ScCompiler::MakeRefStr( ::rtl::OUStringBuffer& rBuffer, ComplRefData& rRef, BOOL bSingleRef )
/*N*/ {
/*N*/ 	if (bCompileXML)
/*N*/ 		rBuffer.append(sal_Unicode('['));
/*N*/ 	ComplRefData aRef( rRef );
/*N*/ 	// falls abs/rel nicht separat: Relativ- in Abs-Referenzen wandeln!
/*N*/ //	AdjustReference( aRef.Ref1 );
/*N*/ //	if( !bSingleRef )
/*N*/ //		AdjustReference( aRef.Ref2 );
/*N*/ 	aRef.Ref1.CalcAbsIfRel( aPos );
/*N*/ 	if( !bSingleRef )
/*N*/ 		aRef.Ref2.CalcAbsIfRel( aPos );
/*N*/ 	if( aRef.Ref1.IsFlag3D() )
/*N*/ 	{
/*N*/ 		if (aRef.Ref1.IsTabDeleted())
/*N*/ 		{
/*?*/ 			if (!aRef.Ref1.IsTabRel())
/*?*/ 				rBuffer.append(sal_Unicode('$'));
/*?*/ 			rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*?*/ 			rBuffer.append(sal_Unicode('.'));
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aDoc;
/*N*/ 			String aRefStr( MakeTabStr( aRef.Ref1.nTab, aDoc ) );
/*N*/ 			rBuffer.append(aDoc);
/*N*/ 			if (!aRef.Ref1.IsTabRel()) rBuffer.append(sal_Unicode('$'));
/*N*/ 			rBuffer.append(aRefStr);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (bCompileXML)
/*N*/ 		rBuffer.append(sal_Unicode('.'));
/*N*/ 	if (!aRef.Ref1.IsColRel())
/*N*/ 		rBuffer.append(sal_Unicode('$'));
/*N*/ 	if ( aRef.Ref1.IsColDeleted() )
/*?*/ 		rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*N*/ 	else
/*N*/ 		MakeColStr(rBuffer, aRef.Ref1.nCol );
/*N*/ 	if (!aRef.Ref1.IsRowRel())
/*N*/ 		rBuffer.append(sal_Unicode('$'));
/*N*/ 	if ( aRef.Ref1.IsRowDeleted() )
/*?*/ 		rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*N*/ 	else
/*N*/ 		MakeRowStr( rBuffer, aRef.Ref1.nRow );
/*N*/ 	if (!bSingleRef)
/*N*/ 	{
/*N*/ 		rBuffer.append(sal_Unicode(':'));
/*N*/ 		if (aRef.Ref2.IsFlag3D() || aRef.Ref2.nTab != aRef.Ref1.nTab)
/*N*/ 		{
/*?*/ 			if (aRef.Ref2.IsTabDeleted())
/*?*/ 			{
/*?*/ 				if (!aRef.Ref2.IsTabRel())
/*?*/ 					rBuffer.append(sal_Unicode('$'));
/*?*/ 				rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*?*/ 				rBuffer.append(sal_Unicode('.'));
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				String aDoc;
/*?*/ 				String aRefStr( MakeTabStr( aRef.Ref2.nTab, aDoc ) );
/*?*/ 				rBuffer.append(aDoc);
/*?*/ 				if (!aRef.Ref2.IsTabRel()) rBuffer.append(sal_Unicode('$'));
/*?*/ 				rBuffer.append(aRefStr);
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if (bCompileXML)
/*N*/ 			rBuffer.append(sal_Unicode('.'));
/*N*/ 		if (!aRef.Ref2.IsColRel())
/*N*/ 			rBuffer.append(sal_Unicode('$'));
/*N*/ 		if ( aRef.Ref2.IsColDeleted() )
/*N*/ 			rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*N*/ 		else
/*N*/ 			MakeColStr( rBuffer, aRef.Ref2.nCol );
/*N*/ 		if (!aRef.Ref2.IsRowRel())
/*N*/ 			rBuffer.append(sal_Unicode('$'));
/*N*/ 		if ( aRef.Ref2.IsRowDeleted() )
/*N*/ 			rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
/*N*/ 		else
/*N*/ 			MakeRowStr( rBuffer, aRef.Ref2.nRow );
/*N*/ 	}
/*N*/ 	if (bCompileXML)
/*N*/ 		rBuffer.append(sal_Unicode(']'));
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::SetError(USHORT nError)
/*N*/ {
/*N*/ 	if( !pArr->GetError() )
/*N*/ 		pArr->nError = nError;
/*N*/ }


/*N*/ sal_Unicode* lcl_UnicodeStrNCpy( sal_Unicode* pDst, const sal_Unicode* pSrc, xub_StrLen nMax )
/*N*/ {
/*N*/ 	const sal_Unicode* const pStop = pDst + nMax;
/*N*/ 	while ( *pSrc && pDst < pStop )
/*N*/ 	{
/*N*/ 		*pDst++ = *pSrc++;
/*N*/ 	}
/*N*/ 	*pDst = 0;
/*N*/ 	return pDst;
/*N*/ }


//---------------------------------------------------------------------------
// NextSymbol
//---------------------------------------------------------------------------
// Zerlegt die Formel in einzelne Symbole fuer die weitere
// Verarbeitung (Turing-Maschine).
//---------------------------------------------------------------------------
// Ausgangs Zustand = GetChar
//---------------+-------------------+-----------------------+---------------
// Alter Zustand | gelesenes Zeichen | Aktion                | Neuer Zustand
//---------------+-------------------+-----------------------+---------------
// GetChar       | ;()+-*/^=&        | Symbol=Zeichen        | Stop
//               | <>                | Symbol=Zeichen        | GetBool
//               | $ Buchstabe       | Symbol=Zeichen        | GetWord
//               | Ziffer            | Symbol=Zeichen        | GetValue
//               | "                 | Keine                 | GetString
//               | Sonst             | Keine                 | GetChar
//---------------+-------------------+-----------------------+---------------
// GetBool       | =>                | Symbol=Symbol+Zeichen | Stop
//               | Sonst             | Dec(CharPos)          | Stop
//---------------+-------------------+-----------------------+---------------
// GetWord       | SepSymbol         | Dec(CharPos)          | Stop
//               | ()+-*/^=<>&~      |                       |
//               | Leerzeichen       | Dec(CharPos)          | Stop
//               | $_:.              |                       |
//               | Buchstabe,Ziffer  | Symbol=Symbol+Zeichen | GetWord
//               | Sonst             | Fehler                | Stop
//---------------|-------------------+-----------------------+---------------
// GetValue      | ;()*/^=<>&        |                       |
//               | Leerzeichen       | Dec(CharPos)          | Stop
//               | Ziffer E+-%,.     | Symbol=Symbol+Zeichen | GetValue
//               | Sonst             | Fehler                | Stop
//---------------+-------------------+-----------------------+---------------
// GetString     | "                 | Keine                 | Stop
//               | Sonst             | Symbol=Symbol+Zeichen | GetString
//---------------+-------------------+-----------------------+---------------

/*N*/ xub_StrLen ScCompiler::NextSymbol()
/*N*/ {
/*N*/ 	cSymbol[MAXSTRLEN-1] = 0;		// Stopper
/*N*/ 	sal_Unicode* pSym = cSymbol;
/*N*/ 	const sal_Unicode* const pStart = aFormula.GetBuffer();
/*N*/ 	const sal_Unicode* pSrc = pStart + nSrcPos;
/*N*/ 	BOOL bi18n = FALSE;
/*N*/ 	sal_Unicode c = *pSrc;
/*N*/ 	sal_Unicode cLast = 0;
/*N*/ 	BOOL bQuote = FALSE;
/*N*/ 	ScanState eState = ssGetChar;
/*N*/ 	xub_StrLen nSpaces = 0;
/*N*/ 	// try to parse simple tokens before calling i18n parser
/*N*/ 	while ((c != 0) && (eState != ssStop) )
/*N*/ 	{
/*N*/ 		pSrc++;
/*N*/         ULONG nMask = GetCharTableFlags( c );
/*N*/ 		switch (eState)
/*N*/ 		{
/*N*/ 			case ssGetChar :
/*N*/ 			{
/*N*/ 				if( nMask & SC_COMPILER_C_CHAR )
/*N*/ 				{
/*N*/ 					*pSym++ = c;
/*N*/ 					eState = ssStop;
/*N*/ 				}
/*N*/ 				else if( nMask & SC_COMPILER_C_CHAR_BOOL )
/*N*/ 				{
/*N*/ 					*pSym++ = c;
/*N*/ 					eState = ssGetBool;
/*N*/ 				}
/*N*/ 				else if( nMask & SC_COMPILER_C_CHAR_STRING )
/*N*/ 				{
/*N*/ 					*pSym++ = c;
/*N*/ 					eState = ssGetString;
/*N*/ 				}
/*N*/ 				else if( nMask & SC_COMPILER_C_CHAR_DONTCARE )
/*N*/ 				{
/*N*/ 					nSpaces++;
/*N*/ 				}
/*N*/                 else if( nMask & SC_COMPILER_C_CHAR_IDENT )
/*N*/                 {   // try to get a simple ASCII identifier before calling
/*N*/                     // i18n, to gain performance during import
/*N*/                     *pSym++ = c;
/*N*/                     eState = ssGetIdent;
/*N*/                 }
/*N*/ 				else
/*N*/ 				{
/*N*/ 					bi18n = TRUE;
/*N*/                     eState = ssStop;
/*N*/                 }
/*N*/             }
/*N*/             break;
/*N*/             case ssGetIdent:
/*N*/             {
/*N*/                 if ( nMask & SC_COMPILER_C_IDENT )
/*N*/                 {   // this catches also $Sheet1.A1:A$2, for example
/*N*/                     *pSym++ = c;
/*N*/                 }
/*N*/                 else if ( 128 <= c || '\'' == c )
/*N*/                 {   // High values need reparsing with i18n, 
/*N*/                     // single quoted $'sheet' names too (otherwise we'd had to
/*N*/                     // implement everything twice).
/*N*/                     pSrc = pStart + nSrcPos + nSpaces;
/*N*/                     pSym = cSymbol;
/*N*/                     c = *pSrc;
/*N*/                     bi18n = TRUE;
/*N*/                     eState = ssStop;
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     pSrc--;
/*N*/ 					eState = ssStop;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case ssGetBool :
/*N*/ 			{
/*N*/ 				if( nMask & SC_COMPILER_C_BOOL )
/*N*/ 				{
/*N*/ 					*pSym++ = c;
/*N*/ 					eState = ssStop;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pSrc--;
/*N*/ 					eState = ssStop;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case ssGetString :
/*N*/ 			{
/*N*/ 				if( nMask & SC_COMPILER_C_STRING_SEP )
/*N*/ 				{
/*N*/ 					if ( !bQuote )
/*N*/ 					{
/*N*/ 						if ( *pSrc == '"' )
/*N*/ 							bQuote = TRUE;		// "" => literal "
/*N*/ 						else
/*N*/ 							eState = ssStop;
/*N*/ 					}
/*N*/ 					else
/*N*/ 						bQuote = FALSE;
/*N*/ 				}
/*N*/ 				if ( !bQuote )
/*N*/ 				{
/*N*/ 					if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
/*N*/ 					{
/*N*/ 						SetError(errStringOverflow);
/*N*/ 						eState = ssSkipString;
/*N*/ 					}
/*N*/ 					else
/*N*/ 						*pSym++ = c;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case ssSkipString:
/*N*/ 				if( nMask & SC_COMPILER_C_STRING_SEP )
/*N*/ 					eState = ssStop;
/*N*/ 				break;
/*N*/ 		}
/*N*/ 		cLast = c;
/*N*/ 		c = *pSrc;
/*N*/ 	}
/*N*/ 	if ( bi18n )
/*N*/ 	{
/*N*/ 		using namespace ::com::sun::star::i18n;
/*N*/ 		nSrcPos += nSpaces;
/*N*/ 		sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
/*N*/ 			KParseTokens::ASC_UNDERSCORE | KParseTokens::ASC_DOLLAR;
/*N*/ 		sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT |
/*N*/ 			KParseTokens::ASC_COLON;
/*N*/ 		// '?' allowed in range names because of Xcl :-/
/*N*/         static const String aAddAllowed( '?' );
/*N*/ 		String aSymbol;
/*N*/ 		USHORT nErr = 0;
/*N*/ 		do
/*N*/ 		{
/*N*/ 			bi18n = FALSE;
/*N*/ 			// special case  $'sheetname'
/*N*/ 			if ( pStart[nSrcPos] == '$' && pStart[nSrcPos+1] == '\'' )
/*N*/ 				aSymbol += pStart[nSrcPos++];
/*N*/ 
/*N*/ 			ParseResult aRes = ScGlobal::pCharClass->parseAnyToken( aFormula,
/*N*/ 				nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
/*N*/ 
/*N*/ 			if ( !aRes.TokenType )
/*N*/ 				SetError( nErr = errIllegalChar );		// parsed chars as string
/*N*/ 			if ( aRes.EndPos <= nSrcPos )
/*N*/ 			{	// ?!?
/*?*/ 				SetError( nErr = errIllegalChar );
/*?*/ 				nSrcPos = aFormula.Len();
/*?*/ 				aSymbol.Erase();
/*?*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				aSymbol.Append( pStart + nSrcPos, aRes.EndPos - nSrcPos );
/*N*/ 				nSrcPos = (xub_StrLen) aRes.EndPos;
/*N*/ 				if ( aRes.TokenType & KParseType::SINGLE_QUOTE_NAME )
/*N*/ 				{	// special cases  'sheetname'.  'filename'#
/*N*/ 					c = pStart[nSrcPos];
/*N*/ 					bi18n = (c == '.' || c == SC_COMPILER_FILE_TAB_SEP);
/*N*/ 					if ( bi18n )
/*N*/ 						aSymbol += pStart[nSrcPos++];
/*N*/ 				}
/*N*/ 				else if ( aRes.TokenType & KParseType::IDENTNAME )
/*N*/ 				{	// special cases  reference:[$]'sheetname'
/*N*/ 					c = aSymbol.GetChar( aSymbol.Len()-1 );
/*N*/ 					bi18n = ((c == ':' || c == '$') && pStart[nSrcPos] == '\'');
/*N*/ 				}
/*N*/ 			}
/*N*/ 		} while ( bi18n && !nErr );
/*N*/ 		xub_StrLen nLen = aSymbol.Len();
/*N*/ 		if ( nLen >= MAXSTRLEN )
/*N*/ 		{
/*?*/ 			SetError( errStringOverflow );
/*?*/ 			nLen = MAXSTRLEN-1;
/*N*/ 		}
/*N*/ 		lcl_UnicodeStrNCpy( cSymbol, aSymbol.GetBuffer(), nLen );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nSrcPos = pSrc - pStart;
/*N*/ 		*pSym = 0;
/*N*/ 	}
/*N*/ 	if ( bAutoCorrect )
/*N*/ 		aCorrectedSymbol = cSymbol;
/*N*/ 	return nSpaces;
/*N*/ }

//---------------------------------------------------------------------------
// Symbol in Token Umwandeln
//---------------------------------------------------------------------------

/*N*/ BOOL ScCompiler::IsOpCode( const String& rName )
/*N*/ {
/*N*/     ScOpCodeHashMap::const_iterator iLook( pSymbolHashMap->find( rName ) );
/*N*/     BOOL bFound = (iLook != pSymbolHashMap->end());
/*N*/     if (bFound)
/*N*/     {
/*N*/         ScRawToken aToken;
/*N*/         aToken.SetOpCode( iLook->second );
/*N*/         pRawToken = aToken.Clone();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nIndex;
/*N*/ 		bFound = ScGlobal::GetFuncCollection()->SearchFunc(cSymbol, nIndex);
/*N*/ 		if( bFound )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); /*N*/  ScRawToken aToken;
/*N*/  /*?*/ 			aToken.SetExternal( cSymbol );
/*N*/  /*?*/ 			pRawToken = aToken.Clone();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// bLocalFirst=FALSE for english
/*N*/ 			String aIntName = ScGlobal::GetAddInCollection()->
/*N*/                     FindFunction( rName, ( pSymbolTable != pSymbolTableEnglish ) );
/*N*/ 			if (aIntName.Len())
/*N*/ 			{
/*N*/ 			 ScRawToken aToken;
/*N*/  				aToken.SetExternal( aIntName.GetBuffer() );		// international name
/*N*/  				pRawToken = aToken.Clone();
/*N*/  				bFound = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( bFound && pRawToken->GetOpCode() == ocSub &&
/*N*/ 		(eLastOp == ocOpen || eLastOp == ocSep ||
/*N*/ 		 (eLastOp > ocEndDiv && eLastOp < ocEndBinOp /*ocEndUnOp*/)))
/*N*/ 		pRawToken->NewOpCode( ocNegSub );
/*N*/ 	return bFound;
/*N*/ }

/*N*/ BOOL ScCompiler::IsOpCode2( const String& rName )
/*N*/ {
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	USHORT i = 0;
/*N*/ 	for( i = ocInternalBegin; i <= ocInternalEnd && !bFound; i++ )
/*N*/ 		bFound = rName.EqualsAscii( pInternal[ i-ocInternalBegin ] );
/*N*/ 
/*N*/ 	if (bFound)
/*N*/ 	{
/*?*/ 		ScRawToken aToken;
/*?*/ 		aToken.SetOpCode( (OpCode) --i );
/*?*/ 		pRawToken = aToken.Clone();
/*N*/ 	}
/*N*/ 	return bFound;
/*N*/ }

/*N*/ BOOL ScCompiler::IsValue( const String& rSym )
/*N*/ {
/*N*/ 	double fVal;
/*N*/ 	sal_uInt32 nIndex = ( pSymbolTable == pSymbolTableEnglish ?
/*N*/ 		pDoc->GetFormatTable()->GetStandardIndex( LANGUAGE_ENGLISH_US ) : 0 );
/*N*/ //	ULONG nIndex = 0;
/*N*/ ////	ULONG nIndex = pDoc->GetFormatTable()->GetStandardIndex(ScGlobal::eLnge);
/*N*/ 	if (pDoc->GetFormatTable()->IsNumberFormat( rSym, nIndex, fVal ) )
/*N*/ 	{
/*N*/ 		USHORT nType = pDoc->GetFormatTable()->GetType(nIndex);
/*N*/ 		const sal_Unicode* p = aFormula.GetBuffer() + nSrcPos;
/*N*/ 		while( *p == ' ' )
/*N*/ 			p++;
/*N*/ 		if ( *p == '(' && nType == NUMBERFORMAT_LOGICAL)
/*N*/ 			return FALSE;
/*N*/ 		else if( aFormula.GetChar(nSrcPos) == '.' )
/*N*/ 			// Numerischer Tabellenname?
/*N*/ 			return FALSE;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( nType == NUMBERFORMAT_TEXT )
/*N*/ 				// HACK: Die Zahl ist zu gross!
/*N*/ 				SetError( errIllegalArgument );
/*N*/ 			ScRawToken aToken;
/*N*/ 			aToken.SetDouble( fVal );
/*N*/ 			pRawToken = aToken.Clone();
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ BOOL ScCompiler::IsString()
/*N*/ {
/*N*/ 	register const sal_Unicode* p = cSymbol;
/*N*/ 	while ( *p )
/*N*/ 		p++;
/*N*/ 	xub_StrLen nLen = p - cSymbol - 1;
/*N*/ 	BOOL bQuote = ((cSymbol[0] == '"') && (cSymbol[nLen] == '"'));
/*N*/ 	if ((bQuote ? nLen-2 : nLen) > MAXSTRLEN-1)
/*N*/ 	{
/*?*/ 		SetError(errStringOverflow);
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	if ( bQuote )
/*N*/ 	{
/*?*/ 		cSymbol[nLen] = '\0';
/*?*/ 		ScRawToken aToken;
/*?*/ 		aToken.SetString( cSymbol+1 );
/*?*/ 		pRawToken = aToken.Clone();
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL ScCompiler::IsReference( const String& rName )
/*N*/ {
/*N*/     // Has to be called before IsValue
/*N*/ 	sal_Unicode ch1 = rName.GetChar(0);
/*N*/ 	sal_Unicode cDecSep = ( pSymbolTable == pSymbolTableEnglish ? '.' :
/*N*/         ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0) );
/*N*/ 	if ( ch1 == cDecSep )
/*N*/ 		return FALSE;
/*N*/     // Who was that imbecile introducing '.' as the sheet name separator!?!
/*N*/     if ( CharClass::isAsciiNumeric( ch1 ) )
/*N*/     {   // Numerical sheet name is valid.
/*N*/         // But English 1.E2 or 1.E+2 is value 100, 1.E-2 is 0.01
/*N*/         // Don't create a #REF! of values.
/*N*/         const xub_StrLen nPos = rName.Search( '.' );
/*N*/         if ( nPos == STRING_NOTFOUND )
/*N*/             return FALSE;
/*N*/         sal_Unicode const * const pTabSep = rName.GetBuffer() + nPos;
/*N*/         sal_Unicode ch2 = pTabSep[1];   // maybe a column identifier
/*N*/         if ( !(ch2 == '$' || CharClass::isAsciiAlpha( ch2 )) )
/*N*/             return FALSE;
/*?*/         if ( cDecSep == '.' && (ch2 == 'E' || ch2 == 'e')   // E + - digit
/*?*/                 && (GetCharTableFlags( pTabSep[2] ) & SC_COMPILER_C_VALUE_EXP) )
/*?*/         {   // #91053#
/*?*/             // If it is an 1.E2 expression check if "1" is an existent sheet
/*?*/             // name. If so, a desired value 1.E2 would have to be entered as
/*?*/             // 1E2 or 1.0E2 or 1.E+2, sorry. Another possibility would be to
/*?*/             // require numerical sheet names always being entered quoted, which
/*?*/             // is not desirable (too many 1999, 2000, 2001 sheets in use).
/*?*/             // Furthermore, XML files created with versions prior to SRC640e
/*?*/             // wouldn't contain the quotes added by MakeTabStr()/CheckTabQuotes()
/*?*/             // and would produce wrong formulas if the conditions here are met.
/*?*/             // If you can live with these restrictions you may remove the
/*?*/             // check and return an unconditional FALSE.
/*?*/             String aTabName( rName.Copy( 0, nPos ) );
/*?*/             USHORT nTab;
/*?*/             if ( !pDoc->GetTable( aTabName, nTab ) )
/*?*/                 return FALSE;
/*?*/             // If sheet "1" exists and the expression is 1.E+2 continue as
/*?*/             // usual, the ScRange/ScAddress parser will take care of it.
/*?*/         }
/*N*/     }
/*N*/ 	ScRange aRange( aPos, aPos );
/*N*/ 	USHORT nFlags = aRange.Parse( rName, pDoc );
/*N*/ 	if( nFlags & SCA_VALID )
/*N*/ 	{
/*N*/ 		ScRawToken aToken;
/*N*/ 		ComplRefData aRef;
/*N*/ 		aRef.InitRange( aRange );
/*N*/ 		aRef.Ref1.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
/*N*/ 		aRef.Ref1.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
/*N*/ 		aRef.Ref1.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
/*N*/ 		if ( !(nFlags & SCA_VALID_TAB) )
/*?*/ 			aRef.Ref1.SetTabDeleted( TRUE );		// #REF!
/*N*/ 		aRef.Ref1.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
/*N*/ 		aRef.Ref2.SetColRel( (nFlags & SCA_COL2_ABSOLUTE) == 0 );
/*N*/ 		aRef.Ref2.SetRowRel( (nFlags & SCA_ROW2_ABSOLUTE) == 0 );
/*N*/ 		aRef.Ref2.SetTabRel( (nFlags & SCA_TAB2_ABSOLUTE) == 0 );
/*N*/ 		if ( !(nFlags & SCA_VALID_TAB2) )
/*?*/ 			aRef.Ref2.SetTabDeleted( TRUE );		// #REF!
/*N*/ 		aRef.Ref2.SetFlag3D( ( nFlags & SCA_TAB2_3D ) != 0 );
/*N*/ 		aRef.CalcRelFromAbs( aPos );
/*N*/ 		aToken.SetDoubleReference( aRef );
/*N*/ 		pRawToken = aToken.Clone();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScAddress aAddr( aPos );
/*N*/ 		nFlags = aAddr.Parse( rName, pDoc );
/*N*/ 		// Irgend etwas muss gueltig sein,
/*N*/ 		// damit Tabelle1.blah oder blah.a1 als (falsche) ref erkannt wird
/*N*/ 		if( nFlags & ( SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB ) )
/*N*/ 		{
/*N*/ 			ScRawToken aToken;
/*N*/ 			SingleRefData aRef;
/*N*/ 			aRef.InitAddress( aAddr );
/*N*/ 			aRef.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
/*N*/ 			aRef.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
/*N*/ 			aRef.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
/*N*/ 			aRef.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
/*N*/ 			// Die Referenz ist wirklich ungueltig!
/*N*/ 			if( !( nFlags & SCA_VALID ) )
/*N*/ 			{
/*N*/ 				if( !( nFlags & SCA_VALID_COL ) )
/*N*/ 					aRef.nCol = MAXCOL+1;
/*N*/ 				if( !( nFlags & SCA_VALID_ROW ) )
/*N*/ 					aRef.nRow = MAXROW+1;
/*N*/ 				if( !( nFlags & SCA_VALID_TAB ) )
/*N*/ 					aRef.nTab = MAXTAB+3;
/*N*/ 				nFlags |= SCA_VALID;
/*N*/ 			}
/*N*/ 			aRef.CalcRelFromAbs( aPos );
/*N*/ 			aToken.SetSingleReference( aRef );
/*N*/ 			pRawToken = aToken.Clone();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return ( nFlags & SCA_VALID ) != 0;
/*N*/ }

/*N*/ BOOL ScCompiler::IsMacro( const String& rName )
/*N*/ {
/*N*/ 	StarBASIC* pObj = 0;
/*N*/ 	SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
/*N*/ 
/*N*/ 	SfxApplication* pSfxApp = SFX_APP();
/*N*/ 	pSfxApp->EnterBasicCall();				// Dok-Basic anlegen etc.
/*N*/ 
/*N*/ 	if( pDocSh )//XXX
/*N*/ 		pObj = pDocSh->GetBasic();
/*N*/ 	else
/*?*/ 		pObj = pSfxApp->GetBasic();
/*N*/ 
/*N*/ 	SbxMethod* pMeth = (SbxMethod*) pObj->Find( rName, SbxCLASS_METHOD );
/*N*/ 	if( !pMeth )
/*N*/ 	{
/*N*/ 		pSfxApp->LeaveBasicCall();
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	// Es sollte schon eine BASIC-Function sein!
/*N*/ 	if( pMeth->GetType() == SbxVOID
/*N*/ 	 || ( pMeth->IsFixed() && pMeth->GetType() == SbxEMPTY )
/*N*/ 	 || !pMeth->ISA(SbMethod) )
/*N*/ 	{
/*N*/ 		pSfxApp->LeaveBasicCall();
/*N*/ 		return FALSE;
/*N*/ 	}
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); /*N*/  ScRawToken aToken;
/*N*/  /*?*/ 	aToken.SetExternal( rName.GetBuffer() );
/*N*/  /*?*/ 	aToken.eOp = ocMacro;
/*N*/  /*?*/ 	pRawToken = aToken.Clone();
/*N*/  /*?*/ 	pSfxApp->LeaveBasicCall();
/*?*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL ScCompiler::IsNamedRange( const String& rName )
/*N*/ {
/*N*/ 	USHORT n;
/*N*/ 	ScRangeName* pRangeName = pDoc->GetRangeName();
/*N*/ 	if (pRangeName->SearchName( rName, n ) )
/*N*/ 	{
/*N*/ 		ScRangeData* pData = (*pRangeName)[n];
/*N*/ 		ScRawToken aToken;
/*N*/ 		aToken.SetName( pData->GetIndex() );
/*N*/ 		pRawToken = aToken.Clone();
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ BOOL ScCompiler::IsDBRange( const String& rName )
/*N*/ {
/*N*/ 	USHORT n;
/*N*/ 	ScDBCollection* pDBColl = pDoc->GetDBCollection();
/*N*/ 	if (pDBColl->SearchName( rName, n ) )
/*N*/ 	{
/*?*/ 		ScDBData* pData = (*pDBColl)[n];
/*?*/ 		ScRawToken aToken;
/*?*/ 		aToken.SetName( pData->GetIndex() );
/*?*/ 		aToken.eOp = ocDBArea;
/*?*/ 		pRawToken = aToken.Clone();
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ BOOL ScCompiler::IsColRowName( const String& rName )
/*N*/ {
/*N*/ 	BOOL bInList = FALSE;
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 	SingleRefData aRef;
/*N*/ 	String aName( rName );
/*N*/ 	DeQuote( aName );
/*N*/ 	USHORT nThisTab = aPos.Tab();
/*N*/ 	for ( short jThisTab = 1; jThisTab >= 0 && !bInList; jThisTab-- )
/*N*/ 	{	// #50300# zuerst Bereiche auf dieser Tabelle pruefen, falls doppelte Namen
/*N*/ 		for ( short jRow=0; jRow<2 && !bInList; jRow++ )
/*N*/ 		{
/*N*/ 			ScRangePairList* pRL;
/*N*/ 			if ( !jRow )
/*N*/ 				pRL = pDoc->GetColNameRanges();
/*N*/ 			else
/*N*/ 				pRL = pDoc->GetRowNameRanges();
/*N*/ 			for ( ScRangePair* pR = pRL->First(); pR && !bInList; pR = pRL->Next() )
/*N*/ 			{
/*?*/ 				const ScRange& rNameRange = pR->GetRange(0);
/*?*/ 				if ( jThisTab && !(rNameRange.aStart.Tab() <= nThisTab &&
/*?*/ 						nThisTab <= rNameRange.aEnd.Tab()) )
/*?*/ 					continue;	// for
/*?*/ 				ScCellIterator aIter( pDoc, rNameRange );
/*?*/ 				for ( ScBaseCell* pCell = aIter.GetFirst(); pCell && !bInList;
/*?*/ 						pCell = aIter.GetNext() )
/*?*/ 				{
/*?*/ 					// GPF wenn Zelle via CompileNameFormula auf Zelle ohne Code
/*?*/ 					// trifft und HasStringData/Interpret/Compile ausgefuehrt wird
/*?*/ 					// und das ganze dann auch noch rekursiv..
/*?*/ 					// ausserdem wird *diese* Zelle hier nicht angefasst, da noch
/*?*/ 					// kein RPN existiert
/*?*/ 					CellType eType = pCell->GetCellType();
/*?*/ 					BOOL bOk = (eType == CELLTYPE_FORMULA ?
/*?*/ 						((ScFormulaCell*)pCell)->GetCode()->GetCodeLen() > 0
/*?*/ 						&& ((ScFormulaCell*)pCell)->aPos != aPos	// noIter
/*?*/ 						: TRUE );
/*?*/ 					if ( bOk && pCell->HasStringData() )
/*?*/ 					{
/*?*/ 						String aStr;
/*?*/ 						switch ( eType )
/*?*/ 						{
/*?*/ 							case CELLTYPE_STRING:
/*?*/ 								((ScStringCell*)pCell)->GetString( aStr );
/*?*/ 							break;
/*?*/ 							case CELLTYPE_FORMULA:
/*?*/ 								((ScFormulaCell*)pCell)->GetString( aStr );
/*?*/ 							break;
/*?*/ 							case CELLTYPE_EDIT:
/*?*/ 								((ScEditCell*)pCell)->GetString( aStr );
/*?*/ 							break;
/*?*/ 						}
/*?*/                         if ( ScGlobal::pTransliteration->isEqual( aStr, aName ) )
/*?*/ 						{
/*?*/ 							aRef.InitFlags();
/*?*/ 							aRef.nCol = aIter.GetCol();
/*?*/ 							aRef.nRow = aIter.GetRow();
/*?*/ 							aRef.nTab = aIter.GetTab();
/*?*/ 							if ( !jRow )
/*?*/ 								aRef.SetColRel( TRUE );		// ColName
/*?*/ 							else
/*?*/ 								aRef.SetRowRel( TRUE );		// RowName
/*?*/ 							aRef.CalcRelFromAbs( aPos );
/*?*/ 							bInList = bFound = TRUE;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
/*N*/ 	{	// in der aktuellen Tabelle suchen
/*N*/ 		long nDistance, nMax;
/*N*/ 		long nMyCol = (long) aPos.Col();
/*N*/ 		long nMyRow = (long) aPos.Row();
/*N*/ 		BOOL bTwo = FALSE;
/*N*/ 		ScAddress aOne( 0, 0, aPos.Tab() );
/*N*/ 		ScAddress aTwo( MAXCOL, MAXROW, aPos.Tab() );
/*N*/ 		ScCellIterator aIter( pDoc, ScRange( aOne, aTwo ) );
/*N*/ 		for ( ScBaseCell* pCell = aIter.GetFirst(); pCell; pCell = aIter.GetNext() )
/*N*/ 		{
/*N*/ 			if ( bFound )
/*N*/ 			{	// aufhoeren wenn alles andere weiter liegt
/*N*/ 				if ( nMax < (long)aIter.GetCol() )
/*N*/ 					break;		// aIter
/*N*/ 			}
/*N*/ 			CellType eType = pCell->GetCellType();
/*N*/ 			BOOL bOk = (eType == CELLTYPE_FORMULA ?
/*N*/ 				((ScFormulaCell*)pCell)->GetCode()->GetCodeLen() > 0
/*N*/ 				&& ((ScFormulaCell*)pCell)->aPos != aPos	// noIter
/*N*/ 				: TRUE );
/*N*/ 			if ( bOk && pCell->HasStringData() )
/*N*/ 			{
/*N*/ 				String aStr;
/*N*/ 				switch ( eType )
/*N*/ 				{
/*N*/ 					case CELLTYPE_STRING:
/*N*/ 						((ScStringCell*)pCell)->GetString( aStr );
/*N*/ 					break;
/*N*/ 					case CELLTYPE_FORMULA:
/*N*/ 						((ScFormulaCell*)pCell)->GetString( aStr );
/*N*/ 					break;
/*N*/ 					case CELLTYPE_EDIT:
/*N*/ 						((ScEditCell*)pCell)->GetString( aStr );
/*N*/ 					break;
/*N*/ 				}
/*N*/                 if ( ScGlobal::pTransliteration->isEqual( aStr, aName ) )
/*N*/ 				{
/*N*/ 					USHORT nCol = aIter.GetCol();
/*N*/ 					USHORT nRow = aIter.GetRow();
/*N*/ 					long nC = nMyCol - nCol;
/*N*/ 					long nR = nMyRow - nRow;
/*N*/ 					if ( bFound )
/*N*/ 					{
/*?*/ 						long nD = nC * nC + nR * nR;
/*?*/ 						if ( nD < nDistance )
/*?*/ 						{
/*?*/ 							if ( nC < 0 || nR < 0 )
/*?*/ 							{	// rechts oder unterhalb
/*?*/ 								bTwo = TRUE;
/*?*/ 								aTwo.Set( nCol, nRow, aIter.GetTab() );
/*?*/ 								nMax = Max( nMyCol + Abs( nC ), nMyRow + Abs( nR ) );
/*?*/ 								nDistance = nD;
/*?*/ 							}
/*?*/ 							else if ( !(nRow < aOne.Row() && nMyRow >= (long)aOne.Row()) )
/*?*/ 							{	// links oben, nur wenn nicht weiter oberhalb
/*?*/ 								// des bisherigen und nMyRow darunter
/*?*/ 								// (CellIter geht spaltenweise!)
/*?*/ 								bTwo = FALSE;
/*?*/ 								aOne.Set( nCol, nRow, aIter.GetTab() );
/*?*/ 								nMax = Max( nMyCol + nC, nMyRow + nR );
/*?*/ 								nDistance = nD;
/*?*/ 							}
/*?*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						aOne.Set( nCol, nRow, aIter.GetTab() );
/*N*/ 						nDistance = nC * nC + nR * nR;
/*N*/ 						nMax = Max( nMyCol + Abs( nC ), nMyRow + Abs( nR ) );
/*N*/ 					}
/*N*/ 					bFound = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bFound )
/*N*/ 		{
/*N*/ 			ScAddress aAdr;
/*N*/ 			if ( bTwo )
/*N*/ 			{
/*?*/ 				if ( nMyCol >= (long)aOne.Col() && nMyRow >= (long)aOne.Row() )
/*?*/ 					aAdr = aOne;		// links oben hat Vorrang
/*?*/ 				else
/*?*/ 				{
/*?*/ 					if ( nMyCol < (long)aOne.Col() )
/*?*/ 					{	// zwei rechts
/*?*/ 						if ( nMyRow >= (long)aTwo.Row() )
/*?*/ 							aAdr = aTwo;		// direkt rechts
/*?*/ 						else
/*?*/ 							aAdr = aOne;
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{	// zwei unten oder unten und rechts, der naechstgelegene
/*?*/ 						long nC1 = nMyCol - aOne.Col();
/*?*/ 						long nR1 = nMyRow - aOne.Row();
/*?*/ 						long nC2 = nMyCol - aTwo.Col();
/*?*/ 						long nR2 = nMyRow - aTwo.Row();
/*?*/ 						if ( nC1 * nC1 + nR1 * nR1 <= nC2 * nC2 + nR2 * nR2 )
/*?*/ 							aAdr = aOne;
/*?*/ 						else
/*?*/ 							aAdr = aTwo;
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				aAdr = aOne;
/*N*/ 			aRef.InitAddress( aAdr );
/*N*/ 			if ( (aRef.nRow != MAXROW && pDoc->HasStringData(
/*N*/ 					aRef.nCol, aRef.nRow + 1, aRef.nTab ))
/*N*/ 			  || (aRef.nRow != 0 && pDoc->HasStringData(
/*N*/ 					aRef.nCol, aRef.nRow - 1, aRef.nTab )) )
/*N*/ 				aRef.SetRowRel( TRUE );		// RowName
/*N*/ 			else
/*N*/ 				aRef.SetColRel( TRUE );		// ColName
/*N*/ 			aRef.CalcRelFromAbs( aPos );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( bFound )
/*N*/ 	{
/*N*/ 		ScRawToken aToken;
/*N*/ 		aToken.SetSingleReference( aRef );
/*N*/ 		aToken.eOp = ocColRowName;
/*N*/ 		pRawToken = aToken.Clone();
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

//---------------------------------------------------------------------------

/*N*/  void ScCompiler::AutoCorrectParsedSymbol()
/*N*/  {
/*N*/  	xub_StrLen nPos = aCorrectedSymbol.Len();
/*N*/  	if ( nPos )
/*N*/  	{
/*N*/  		nPos--;
/*N*/  		const sal_Unicode cQuote = '\"';
/*N*/  		const sal_Unicode cx = 'x';
/*N*/  		const sal_Unicode cX = 'X';
/*N*/  		sal_Unicode c1 = aCorrectedSymbol.GetChar( 0 );
/*N*/  		sal_Unicode c2 = aCorrectedSymbol.GetChar( nPos );
/*N*/  		if ( c1 == cQuote && c2 != cQuote  )
/*N*/  		{	// "...
/*N*/  			// was kein Wort bildet gehoert nicht dazu.
/*N*/  			// Don't be pedantic: c < 128 should be sufficient here.
/*N*/  			while ( nPos && ((aCorrectedSymbol.GetChar(nPos) < 128) &&
/*N*/  					((GetCharTableFlags( aCorrectedSymbol.GetChar(nPos) ) &
/*N*/  					(SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_DONTCARE)) == 0)) )
/*N*/  				nPos--;
/*N*/  			if ( nPos == MAXSTRLEN - 2 )
/*N*/  				aCorrectedSymbol.SetChar( nPos, cQuote );	// '"' als 255. Zeichen
/*N*/  			else
/*N*/  				aCorrectedSymbol.Insert( cQuote, nPos + 1 );
/*N*/  			bCorrected = TRUE;
/*N*/  		}
/*N*/  		else if ( c1 != cQuote && c2 == cQuote )
/*N*/  		{	// ..."
/*N*/  			aCorrectedSymbol.Insert( cQuote, 0 );
/*N*/  			bCorrected = TRUE;
/*N*/  		}
/*N*/  		else if ( nPos == 0 && (c1 == cx || c1 == cX) )
/*N*/  		{	// x => *
/*N*/  			aCorrectedSymbol = pSymbolTable[ocMul];
/*N*/  			bCorrected = TRUE;
/*N*/  		}
/*N*/  		else if ( (GetCharTableFlags( c1 ) & SC_COMPILER_C_CHAR_VALUE)
/*N*/  			   && (GetCharTableFlags( c2 ) & SC_COMPILER_C_CHAR_VALUE) )
/*N*/  		{
/*N*/  			xub_StrLen nXcount;
/*N*/  			if ( (nXcount = aCorrectedSymbol.GetTokenCount( cx )) > 1 )
/*N*/  			{	// x => *
/*N*/  				xub_StrLen nIndex = 0;
/*N*/  				sal_Unicode c = pSymbolTable[ocMul].GetChar(0);
/*N*/  				while ( (nIndex = aCorrectedSymbol.SearchAndReplace(
/*N*/  						cx, c, nIndex )) != STRING_NOTFOUND )
/*N*/  					nIndex++;
/*N*/  				bCorrected = TRUE;
/*N*/  			}
/*N*/  			if ( (nXcount = aCorrectedSymbol.GetTokenCount( cX )) > 1 )
/*N*/  			{	// X => *
/*N*/  				xub_StrLen nIndex = 0;
/*N*/  				sal_Unicode c = pSymbolTable[ocMul].GetChar(0);
/*N*/  				while ( (nIndex = aCorrectedSymbol.SearchAndReplace(
/*N*/  						cX, c, nIndex )) != STRING_NOTFOUND )
/*N*/  					nIndex++;
/*N*/  				bCorrected = TRUE;
/*N*/  			}
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			String aSymbol( aCorrectedSymbol );
/*N*/  			String aDoc;
/*N*/  			xub_StrLen nPos;
/*N*/  			if ( aSymbol.GetChar(0) == '\''
/*N*/  			  && ((nPos = aSymbol.SearchAscii( "'#" )) != STRING_NOTFOUND) )
/*N*/  			{	// 'Doc'# abspalten, kann d:\... und sonstwas sein
/*N*/  				aDoc = aSymbol.Copy( 0, nPos + 2 );
/*N*/  				aSymbol.Erase( 0, nPos + 2 );
/*N*/  			}
/*N*/  			xub_StrLen nRefs = aSymbol.GetTokenCount( ':' );
/*N*/  			BOOL bColons;
/*N*/  			if ( nRefs > 2 )
/*N*/  			{	// doppelte oder zuviele ':'? B:2::C10 => B2:C10
/*N*/  				bColons = TRUE;
/*N*/  				xub_StrLen nIndex = 0;
/*N*/  				String aTmp1( aSymbol.GetToken( 0, ':', nIndex ) );
/*N*/  				xub_StrLen nLen1 = aTmp1.Len();
/*N*/  				String aSym, aTmp2;
/*N*/  				BOOL bLastAlp, bNextNum;
/*N*/  				bLastAlp = bNextNum = TRUE;
/*N*/  				xub_StrLen nStrip = 0;
/*N*/  				xub_StrLen nCount = nRefs;
/*N*/  				for ( xub_StrLen j=1; j<nCount; j++ )
/*N*/  				{
/*N*/  					aTmp2 = aSymbol.GetToken( 0, ':', nIndex );
/*N*/  					xub_StrLen nLen2 = aTmp2.Len();
/*N*/  					if ( nLen1 || nLen2 )
/*N*/  					{
/*N*/  						if ( nLen1 )
/*N*/  						{
/*N*/  							aSym += aTmp1;
/*N*/  							bLastAlp = CharClass::isAsciiAlpha( aTmp1 );
/*N*/  						}
/*N*/  						if ( nLen2 )
/*N*/  						{
/*N*/  							bNextNum = CharClass::isAsciiNumeric( aTmp2 );
/*N*/  							if ( bLastAlp == bNextNum && nStrip < 1 )
/*N*/  							{	// muss abwechselnd nur Zahl/String sein,
/*N*/  								// nur innerhalb einer Ref strippen
/*N*/  								nRefs--;
/*N*/  								nStrip++;
/*N*/  							}
/*N*/  							else
/*N*/  							{
/*N*/  								xub_StrLen nSymLen = aSym.Len();
/*N*/  								if ( nSymLen
/*N*/  								  && (aSym.GetChar( nSymLen - 1 ) != ':') )
/*N*/  									aSym += ':';
/*N*/  								nStrip = 0;
/*N*/  							}
/*N*/  							bLastAlp = !bNextNum;
/*N*/  						}
/*N*/  						else
/*N*/  						{	// ::
/*N*/  							nRefs--;
/*N*/  							if ( nLen1 )
/*N*/  							{	// B10::C10 ? naechste Runde ':' anhaengen
/*N*/  								if ( !bLastAlp && !CharClass::isAsciiNumeric( aTmp1 ) )
/*N*/  									nStrip++;
/*N*/  							}
/*N*/  							bNextNum = !bLastAlp;
/*N*/  						}
/*N*/  						aTmp1 = aTmp2;
/*N*/  						nLen1 = nLen2;
/*N*/  					}
/*N*/  					else
/*N*/  						nRefs--;
/*N*/  				}
/*N*/  				aSymbol = aSym;
/*N*/  				aSymbol += aTmp1;
/*N*/  			}
/*N*/  			else
/*N*/  				bColons = FALSE;
/*N*/  			if ( nRefs && nRefs <= 2 )
/*N*/  			{	// Referenzdreher? 4A => A4 etc.
/*N*/  				String aTab[2], aRef[2];
/*N*/  				if ( nRefs == 2 )
/*N*/  				{
/*N*/  					aRef[0] = aSymbol.GetToken( 0, ':' );
/*N*/  					aRef[1] = aSymbol.GetToken( 1, ':' );
/*N*/  				}
/*N*/  				else
/*N*/  					aRef[0] = aSymbol;
/*N*/  
/*N*/  				BOOL bChanged = FALSE;
/*N*/  				BOOL bOk = TRUE;
/*N*/  				USHORT nMask = SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW;
/*N*/  				for ( int j=0; j<nRefs; j++ )
/*N*/  				{
/*N*/  					xub_StrLen nTmp = 0;
/*N*/  					xub_StrLen nPos = STRING_NOTFOUND;
/*N*/  					while ( (nTmp = aRef[j].Search( '.', nTmp )) != STRING_NOTFOUND )
/*N*/  						nPos = nTmp++;		// der letzte zaehlt
/*N*/  					if ( nPos != STRING_NOTFOUND )
/*N*/  					{
/*N*/  						aTab[j] = aRef[j].Copy( 0, nPos + 1 );	// mit '.'
/*N*/  						aRef[j].Erase( 0, nPos + 1 );
/*N*/  					}
/*N*/  					String aOld( aRef[j] );
/*N*/  					String aStr2;
/*N*/  					const sal_Unicode* p = aRef[j].GetBuffer();
/*N*/  					while ( *p && CharClass::isAsciiNumeric( *p ) )
/*N*/  						aStr2 += *p++;
/*N*/  					aRef[j] = String( p );
/*N*/  					aRef[j] += aStr2;
/*N*/  					if ( bColons || aRef[j] != aOld )
/*N*/  					{
/*N*/  						bChanged = TRUE;
/*N*/  						ScAddress aAdr;
/*N*/  						bOk &= ((aAdr.Parse( aRef[j], pDoc ) & nMask) == nMask);
/*N*/  					}
/*N*/  				}
/*N*/  				if ( bChanged && bOk )
/*N*/  				{
/*N*/  					aCorrectedSymbol = aDoc;
/*N*/  					aCorrectedSymbol += aTab[0];
/*N*/  					aCorrectedSymbol += aRef[0];
/*N*/  					if ( nRefs == 2 )
/*N*/  					{
/*N*/  						aCorrectedSymbol += ':';
/*N*/  						aCorrectedSymbol += aTab[1];
/*N*/  						aCorrectedSymbol += aRef[1];
/*N*/  					}
/*N*/  					bCorrected = TRUE;
/*N*/  				}
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  }

/*N*/ BOOL ScCompiler::NextNewToken()
/*N*/ {
/*N*/ 	xub_StrLen nSpaces = NextSymbol();
/*N*/ 	ScRawToken aToken;
/*N*/ 	if( cSymbol[0] )
/*N*/ 	{
/*N*/ 		if( nSpaces )
/*N*/ 		{
/*N*/ 			aToken.SetOpCode( ocSpaces );
/*N*/ 			aToken.cByte = (BYTE) ( nSpaces > 255 ? 255 : nSpaces );
/*N*/ 			if( !pArr->AddToken( aToken ) )
/*N*/ 			{
/*?*/ 				SetError(errCodeOverflow); return FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/         if ( (cSymbol[0] == '#' || cSymbol[0] == '$') && cSymbol[1] == 0 &&
/*N*/                 !bAutoCorrect )
/*N*/         {   // #101100# special case to speed up broken [$]#REF documents
/*N*/             String aBad( aFormula.Copy( nSrcPos-1 ) );
/*N*/             eLastOp = pArr->AddBad( aBad )->GetOpCode();
/*N*/             return FALSE;
/*N*/         }
/*N*/         if( !IsString() )
/*N*/         {
/*N*/             BOOL bMayBeFuncName;
/*N*/             if ( cSymbol[0] < 128 )
/*N*/                 bMayBeFuncName = CharClass::isAsciiAlpha( cSymbol[0] );
/*N*/             else
/*N*/             {
/*N*/                 String aTmpStr( cSymbol[0] );
/*N*/                 bMayBeFuncName = ScGlobal::pCharClass->isLetter( aTmpStr, 0 );
/*N*/             }
/*N*/             if ( bMayBeFuncName )
/*N*/             {   // a function name must be followed by a parenthesis
/*N*/                 const sal_Unicode* p = aFormula.GetBuffer() + nSrcPos;
/*N*/                 while( *p == ' ' )
/*N*/                     p++;
/*N*/                 bMayBeFuncName = ( *p == '(' );
/*N*/             }
/*N*/             else
/*N*/                 bMayBeFuncName = TRUE;      // operators and other opcodes
/*N*/ 
/*N*/             String aOrg( cSymbol ); // evtl. Dateinamen in IsReference erhalten
/*N*/             String aUpper( ScGlobal::pCharClass->upper( aOrg ) );
/*N*/             // Spalte DM konnte nicht referiert werden, IsReference vor IsValue
/*N*/             // #42016# italian ARCTAN.2 gab #REF! => IsOpCode vor IsReference
/*N*/             if ( !(bMayBeFuncName && IsOpCode( aUpper ))
/*N*/               && !IsReference( aOrg )
/*N*/               && !IsValue( aUpper )
/*N*/               && !IsNamedRange( aUpper )
/*N*/               && !IsDBRange( aUpper )
/*N*/               && !IsColRowName( aUpper )
/*N*/               && !(bMayBeFuncName && IsMacro( aUpper ))
/*N*/               && !(bMayBeFuncName && IsOpCode2( aUpper )) )
/*N*/             {
/*N*/                 SetError( errNoName );
/*N*/                 if ( bAutoCorrect )
/*N*/                 {   // provide single token information and continue
/*?*/                     ScGlobal::pCharClass->toLower( aUpper );
/*?*/                     aToken.SetString( aUpper.GetBuffer() );
/*?*/                     aToken.NewOpCode( ocBad );
/*?*/                     pRawToken = aToken.Clone();
/*?*/                     AutoCorrectParsedSymbol();
/*N*/                 }
/*N*/                 else
/*N*/                 {   // we don't need single token information, just a bad formula
/*N*/                     String aBad( aFormula.Copy( nSrcPos - aOrg.Len() ) );
/*N*/                     eLastOp = pArr->AddBad( aBad )->GetOpCode();
/*N*/                     return FALSE;
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/         return TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }

/*N*/ ScTokenArray* ScCompiler::CompileString( const String& rFormula )
/*N*/ {
/*N*/ 	ScTokenArray aArr;
/*N*/ 	pArr = &aArr;
/*N*/ 	aFormula = rFormula;
/*N*/ 	aFormula.EraseLeadingChars();
/*N*/ 	aFormula.EraseTrailingChars();
/*N*/ 	nSrcPos = 0;
/*N*/ 	bCorrected = FALSE;
/*N*/ 	if ( bAutoCorrect )
/*N*/ 	{
/*?*/ 		aCorrectedFormula.Erase();
/*?*/ 		aCorrectedSymbol.Erase();
/*N*/ 	}
/*N*/ 	BYTE nForced = 0;		// ==Formel forciert Recalc auch wenn nicht sichtbar
/*N*/ 	if( aFormula.GetChar(nSrcPos) == '=' )
/*N*/ 	{
/*N*/ 		nSrcPos++;
/*N*/ 		nForced++;
/*N*/ 		if ( bAutoCorrect )
/*N*/ 			aCorrectedFormula += '=';
/*N*/ 	}
/*N*/ 	if( aFormula.GetChar(nSrcPos) == '=' )
/*N*/ 	{
/*N*/ 		nSrcPos++;
/*N*/ 		nForced++;
/*N*/ 		if ( bAutoCorrect )
/*N*/ 			aCorrectedFormula += '=';
/*N*/ 	}
/*N*/ 	short nBrackets = 0;
/*N*/ 	eLastOp = ocOpen;
/*N*/ 	while( NextNewToken() )
/*N*/ 	{
/*N*/ 		if( pRawToken->GetOpCode() == ocOpen )
/*N*/ 			nBrackets++;
/*N*/ 		else if( pRawToken->GetOpCode() == ocClose )
/*N*/ 		{
/*N*/ 			if( !nBrackets )
/*N*/ 			{
/*?*/ 				SetError( errPairExpected );
/*?*/ 				if ( bAutoCorrect )
/*?*/ 				{
/*?*/ 					bCorrected = TRUE;
/*?*/ 					aCorrectedSymbol.Erase();
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nBrackets--;
/*N*/ 		}
/*N*/ 		if( !pArr->Add( pRawToken->CreateToken() ) )
/*N*/ 		{
/*?*/ 			SetError(errCodeOverflow); break;
/*N*/ 		}
/*N*/ 		eLastOp = pRawToken->GetOpCode();
/*N*/ 		if ( bAutoCorrect )
/*N*/ 			aCorrectedFormula += aCorrectedSymbol;
/*N*/ 	}
/*N*/ 	if ( eLastOp != ocBad )
/*N*/ 	{	// bei ocBad ist der Rest der Formel String, es wuerden zuviele
/*N*/ 		// Klammern erscheinen
/*N*/ 		ScByteToken aToken( ocClose );
/*N*/ 		while( nBrackets-- )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); /*N*/  if( !pArr->AddToken( aToken ) )
/*N*/  /*?*/ 			{
/*N*/  /*?*/ 				SetError(errCodeOverflow); break;
/*N*/  /*?*/ 			}
/*N*/  /*?*/ 			if ( bAutoCorrect )
/*N*/  /*?*/ 				aCorrectedFormula += pSymbolTable[ocClose];
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( nForced >= 2 )
/*?*/ 		pArr->SetRecalcModeForced();
/*N*/ 	// pArr merken, falls danach CompileTokenArray() kommt
/*N*/ 	return pArr = new ScTokenArray( aArr );
/*N*/ }

/*N*/ void ScCompiler::PushTokenArray( ScTokenArray* pa, BOOL bTemp )
/*N*/ {
/*N*/ 	if ( bAutoCorrect && !pStack )
/*N*/ 	{	// #61426# don't merge stacked subroutine code into entered formula
/*?*/ 		aCorrectedFormula += aCorrectedSymbol;
/*?*/ 		aCorrectedSymbol.Erase();
/*N*/ 	}
/*N*/ 	ScArrayStack* p = new ScArrayStack;
/*N*/ 	p->pNext 	  = pStack;
/*N*/ 	p->pArr  	  = pArr;
/*N*/ 	p->bTemp	  = bTemp;
/*N*/ 	pStack 		  = p;
/*N*/ 	pArr 		  = pa;
/*N*/ }

/*N*/ void ScCompiler::PopTokenArray()
/*N*/ {
/*N*/ 	if( pStack )
/*N*/ 	{
/*N*/ 		ScArrayStack* p = pStack;
/*N*/ 		pStack = p->pNext;
/*N*/ 		p->pArr->nRefs += pArr->nRefs;
/*N*/ 		// special RecalcMode aus SharedFormula uebernehmen
/*N*/ 		if ( pArr->IsRecalcModeAlways() )
/*?*/ 			p->pArr->SetRecalcModeAlways();
/*N*/ 		else if ( !pArr->IsRecalcModeNormal() && p->pArr->IsRecalcModeNormal() )
/*?*/ 			p->pArr->SetMaskedRecalcMode( pArr->GetRecalcMode() );
/*N*/ 		p->pArr->SetCombinedBitsRecalcMode( pArr->GetRecalcMode() );
/*N*/ 		if( p->bTemp )
/*N*/ 			delete pArr;
/*N*/ 		pArr = p->pArr;
/*N*/ 		delete p;
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScCompiler::GetToken()
/*N*/ {
/*N*/ 	static const short nRecursionMax = 42;
/*N*/     ScCompilerRecursionGuard aRecursionGuard( nRecursion );
/*N*/     if ( nRecursion > nRecursionMax )
/*N*/ 	{
/*?*/ 		SetError( errStackOverflow );
/*?*/ 		pToken = new ScByteToken( ocStop );
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	if ( bAutoCorrect && !pStack )
/*N*/ 	{	// #61426# don't merge stacked subroutine code into entered formula
/*?*/ 		aCorrectedFormula += aCorrectedSymbol;
/*?*/ 		aCorrectedSymbol.Erase();
/*N*/ 	}
/*N*/ 	BOOL bStop = FALSE;
/*N*/ 	if( pArr->GetError() && !bIgnoreErrors )
/*N*/ 		bStop = TRUE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		short nWasColRowName;
/*N*/ 		if ( pArr->nIndex
/*N*/ 		  && pArr->pCode[ pArr->nIndex-1 ]->GetOpCode() == ocColRowName )
/*N*/ 			 nWasColRowName = 1;
/*N*/ 		else
/*N*/ 			 nWasColRowName = 0;
/*N*/ 		pToken = pArr->Next();
/*N*/ 		while( pToken && pToken->GetOpCode() == ocSpaces )
/*N*/ 		{
/*N*/ 			if ( nWasColRowName )
/*N*/ 				nWasColRowName++;
/*N*/ 			if ( bAutoCorrect && !pStack )
/*?*/ 				CreateStringFromToken( aCorrectedFormula, pToken, FALSE );
/*N*/ 			pToken = pArr->Next();
/*N*/ 		}
/*N*/ 		if ( bAutoCorrect && !pStack && pToken )
/*?*/ 			CreateStringFromToken( aCorrectedSymbol, pToken, FALSE );
/*N*/ 		if( !pToken )
/*N*/ 		{
/*N*/ 			if( pStack )
/*N*/ 			{
/*N*/ 				PopTokenArray();
/*N*/ 				return GetToken();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bStop = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( nWasColRowName >= 2 && pToken->GetOpCode() == ocColRowName )
/*N*/ 			{	// aus einem ocSpaces ein ocIntersect im RPN machen
/*?*/ 				pToken = new ScByteToken( ocIntersect );
/*?*/ 				pArr->nIndex--;		// ganz schweinisch..
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( bStop )
/*N*/ 	{
/*N*/ 		pToken = new ScByteToken( ocStop );
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	if( pToken->GetOpCode() == ocSubTotal )
/*N*/ 		glSubTotal = TRUE;
/*N*/ 	else if( pToken->GetOpCode() == ocName )
/*N*/ 	{
/*N*/ 		ScRangeData* pRangeData = pDoc->GetRangeName()->FindIndex( pToken->GetIndex() );
/*N*/ 		if (pRangeData)
/*N*/ 		{
/*N*/ 			USHORT nErr = pRangeData->GetErrCode();
/*N*/ 			if( nErr )
/*N*/ 				SetError( errNoName );
/*N*/ 			else if ( !bCompileForFAP )
/*N*/ 			{
/*N*/ 				ScTokenArray* pNew;
/*N*/ 				// #35168# Bereichsformel klammern
/*N*/ 				// #37680# aber nur wenn nicht schon Klammern da,
/*N*/ 				// geklammerte ocSep geht nicht, z.B. SUMME((...;...))
/*N*/ 				// und wenn nicht direkt zwischen ocSep/Klammer,
/*N*/ 				// z.B. SUMME(...;(...;...)) nicht, SUMME(...;(...)*3) ja
/*N*/ 				// kurz: wenn kein eigenstaendiger Ausdruck
/*N*/ 				ScToken* p1 = pArr->PeekPrevNoSpaces();
/*N*/ 				ScToken* p2 = pArr->PeekNextNoSpaces();
/*N*/ 				OpCode eOp1 = (p1 ? p1->GetOpCode() : ocSep);
/*N*/ 				OpCode eOp2 = (p2 ? p2->GetOpCode() : ocSep);
/*N*/ 				BOOL bBorder1 = (eOp1 == ocSep || eOp1 == ocOpen);
/*N*/ 				BOOL bBorder2 = (eOp2 == ocSep || eOp2 == ocClose);
/*N*/ 				BOOL bAddPair = !(bBorder1 && bBorder2);
/*N*/ 				if ( bAddPair )
/*N*/ 				{
/*N*/ 					pNew = new ScTokenArray;
/*N*/ 					pNew->AddOpCode( ocClose );
/*N*/ 					PushTokenArray( pNew, TRUE );
/*N*/ 					pNew->Reset();
/*N*/ 				}
/*N*/ 				pNew = pRangeData->GetCode()->Clone();
/*N*/ 				PushTokenArray( pNew, TRUE );
/*N*/ 				if( pRangeData->HasReferences() )
/*N*/ 				{
/*N*/ 					SetRelNameReference();
/*N*/ 					MoveRelWrap();
/*N*/ 				}
/*N*/ 				pNew->Reset();
/*N*/ 				if ( bAddPair )
/*N*/ 				{
/*N*/ 					pNew = new ScTokenArray;
/*N*/ 					pNew->AddOpCode( ocOpen );
/*N*/ 					PushTokenArray( pNew, TRUE );
/*N*/ 					pNew->Reset();
/*N*/ 				}
/*N*/ 				return GetToken();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			SetError(errNoName);
/*N*/ 	}
/*N*/ 	else if( pToken->GetOpCode() == ocColRowName )
/*N*/ 	{
/*N*/ 		SingleRefData& rRef = pToken->GetSingleRef();
/*N*/ 		rRef.CalcAbsIfRel( aPos );
/*N*/ 		if ( !rRef.Valid() )
/*N*/ 		{
/*N*/ 			SetError( errNoRef );
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 		USHORT nCol = rRef.nCol;
/*N*/ 		USHORT nRow = rRef.nRow;
/*N*/ 		USHORT nTab = rRef.nTab;
/*N*/ 		ScAddress aLook( nCol, nRow, nTab );
/*N*/ 		BOOL bColName = rRef.IsColRel();
/*N*/ 		USHORT nMyCol = aPos.Col();
/*N*/ 		USHORT nMyRow = aPos.Row();
/*N*/ 		BOOL bInList = FALSE;
/*N*/ 		BOOL bValidName = FALSE;
/*N*/ 		ScRangePairList* pRL = (bColName ?
/*N*/ 			pDoc->GetColNameRanges() : pDoc->GetRowNameRanges());
/*N*/ 		ScRange aRange;
/*N*/ 		for ( ScRangePair* pR = pRL->First(); pR; pR = pRL->Next() )
/*N*/ 		{
/*?*/ 			if ( pR->GetRange(0).In( aLook ) )
/*?*/ 			{
/*?*/ 				bInList = bValidName = TRUE;
/*?*/ 				aRange = pR->GetRange(1);
/*?*/ 				if ( bColName )
/*?*/ 				{
/*?*/ 					aRange.aStart.SetCol( nCol );
/*?*/ 					aRange.aEnd.SetCol( nCol );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					aRange.aStart.SetRow( nRow );
/*?*/ 					aRange.aEnd.SetRow( nRow );
/*?*/ 				}
/*?*/ 				break;	// for
/*?*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
/*N*/ 		{	// automagically oder durch kopieren entstanden und NamePos nicht in Liste
/*N*/ 			BOOL bString = pDoc->HasStringData( nCol, nRow, nTab );
/*N*/ 			if ( !bString && !pDoc->GetCell( aLook ) )
/*N*/ 				bString = TRUE;		// leere Zelle ist ok
/*N*/ 			if ( bString )
/*N*/ 			{	//! korrespondiert mit ScInterpreter::ScColRowNameAuto
/*N*/ 				bValidName = TRUE;
/*N*/ 				if ( bColName )
/*N*/ 				{	// ColName
/*N*/ 					USHORT nStartRow = nRow + 1;
/*N*/ 					if ( nStartRow > MAXROW )
/*N*/ 						nStartRow = MAXROW;
/*N*/ 					USHORT nMaxRow = MAXROW;
/*N*/ 					if ( nMyCol == nCol )
/*N*/ 					{	// Formelzelle in gleicher Col
/*?*/ 						if ( nMyRow == nStartRow )
/*?*/ 						{	// direkt unter dem Namen den Rest nehmen
/*?*/ 							nStartRow++;
/*?*/ 							if ( nStartRow > MAXROW )
/*?*/ 								nStartRow = MAXROW;
/*?*/ 						}
/*?*/ 						else if ( nMyRow > nStartRow )
/*?*/ 						{	// weiter unten vom Namen bis zur Formelzelle
/*?*/ 							nMaxRow = nMyRow - 1;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					for ( ScRangePair* pR = pRL->First(); pR; pR = pRL->Next() )
/*N*/ 					{	// naechster definierter ColNameRange unten ist Row-Begrenzung
/*?*/ 						const ScRange& rRange = pR->GetRange(1);
/*?*/ 						if ( rRange.aStart.Col() <= nCol && nCol <= rRange.aEnd.Col() )
/*?*/ 						{	// gleicher Col Bereich
/*?*/ 							USHORT nTmp = rRange.aStart.Row();
/*?*/ 							if ( nStartRow < nTmp && nTmp <= nMaxRow )
/*?*/ 								nMaxRow = nTmp - 1;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					aRange.aStart.Set( nCol, nStartRow, nTab );
/*N*/ 					aRange.aEnd.Set( nCol, nMaxRow, nTab );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{	// RowName
/*N*/ 					USHORT nStartCol = nCol + 1;
/*N*/ 					if ( nStartCol > MAXCOL )
/*N*/ 						nStartCol = MAXCOL;
/*N*/ 					USHORT nMaxCol = MAXCOL;
/*N*/ 					if ( nMyRow == nRow )
/*N*/ 					{	// Formelzelle in gleicher Row
/*N*/ 						if ( nMyCol == nStartCol )
/*N*/ 						{	// direkt neben dem Namen den Rest nehmen
/*N*/ 							nStartCol++;
/*N*/ 							if ( nStartCol > MAXCOL )
/*N*/ 								nStartCol = MAXCOL;
/*N*/ 						}
/*N*/ 						else if ( nMyCol > nStartCol )
/*N*/ 						{	// weiter rechts vom Namen bis zur Formelzelle
/*N*/ 							nMaxCol = nMyCol - 1;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					for ( ScRangePair* pR = pRL->First(); pR; pR = pRL->Next() )
/*N*/ 					{	// naechster definierter RowNameRange rechts ist Col-Begrenzung
/*?*/ 						const ScRange& rRange = pR->GetRange(1);
/*?*/ 						if ( rRange.aStart.Row() <= nRow && nRow <= rRange.aEnd.Row() )
/*?*/ 						{	// gleicher Row Bereich
/*?*/ 							USHORT nTmp = rRange.aStart.Col();
/*?*/ 							if ( nStartCol < nTmp && nTmp <= nMaxCol )
/*?*/ 								nMaxCol = nTmp - 1;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					aRange.aStart.Set( nStartCol, nRow, nTab );
/*N*/ 					aRange.aEnd.Set( nMaxCol, nRow, nTab );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bValidName )
/*N*/ 		{
/*N*/ 			// Und nun der Zauber zur Unterscheidung zwischen
/*N*/ 			// Bereich und einer einzelnen Zelle daraus, die
/*N*/ 			// positionsabhaengig von der Formelzelle gewaehlt wird.
/*N*/ 			// Ist ein direkter Nachbar ein binaerer Operator (ocAdd etc.)
/*N*/ 			// so wird eine SingleRef passend zur Col/Row generiert,
/*N*/ 			// ocColRowName bzw. ocIntersect als Nachbar => Range.
/*N*/ 			// Spezialfall: Beschriftung gilt fuer eine einzelne Zelle,
/*N*/ 			// dann wird eine positionsunabhaengige SingleRef generiert.
/*N*/ 			BOOL bSingle = (aRange.aStart == aRange.aEnd);
/*N*/ 			BOOL bFound;
/*N*/ 			if ( bSingle )
/*N*/ 				bFound = TRUE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				ScToken* p1 = pArr->PeekPrevNoSpaces();
/*N*/ 				ScToken* p2 = pArr->PeekNextNoSpaces();
/*N*/ 				// Anfang/Ende einer Formel => Single
/*N*/ 				OpCode eOp1 = p1 ? p1->GetOpCode() : ocAdd;
/*N*/ 				OpCode eOp2 = p2 ? p2->GetOpCode() : ocAdd;
/*N*/ 				if ( eOp1 != ocColRowName && eOp1 != ocIntersect
/*N*/ 					&& eOp2 != ocColRowName && eOp2 != ocIntersect )
/*N*/ 				{
/*N*/ 					if ( (ocEndDiv < eOp1 && eOp1 < ocEndBinOp)
/*N*/ 						|| (ocEndDiv < eOp2 && eOp2 < ocEndBinOp) )
/*N*/ 						bSingle = TRUE;
/*N*/ 				}
/*N*/ 				if ( bSingle )
/*N*/ 				{	// Col bzw. Row muss zum Range passen
/*N*/ 					if ( bColName )
/*N*/ 					{
/*N*/ 						bFound = (aRange.aStart.Row() <= nMyRow
/*N*/ 							&& nMyRow <= aRange.aEnd.Row());
/*N*/ 						if ( bFound )
/*N*/ 							aRange.aStart.SetRow( nMyRow );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bFound = (aRange.aStart.Col() <= nMyCol
/*N*/ 							&& nMyCol <= aRange.aEnd.Col());
/*N*/ 						if ( bFound )
/*?*/ 							aRange.aStart.SetCol( nMyCol );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bFound = TRUE;
/*N*/ 			}
/*N*/ 			if ( !bFound )
/*N*/ 				SetError(errNoRef);
/*N*/ 			else if ( !bCompileForFAP )
/*N*/ 			{
/*N*/ 				ScTokenArray* pNew = new ScTokenArray;
/*N*/ 				if ( bSingle )
/*N*/ 				{
/*N*/ 					SingleRefData aRefData;
/*N*/ 					aRefData.InitAddress( aRange.aStart );
/*N*/ 					if ( bColName )
/*N*/ 						aRefData.SetColRel( TRUE );
/*N*/ 					else
/*?*/ 						aRefData.SetRowRel( TRUE );
/*N*/ 					aRefData.CalcRelFromAbs( aPos );
/*N*/ 					pNew->AddSingleReference( aRefData );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*?*/ 					ComplRefData aRefData;
/*?*/ 					aRefData.InitRange( aRange );
/*?*/ 					if ( bColName )
/*?*/ 					{
/*?*/ 						aRefData.Ref1.SetColRel( TRUE );
/*?*/ 						aRefData.Ref2.SetColRel( TRUE );
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						aRefData.Ref1.SetRowRel( TRUE );
/*?*/ 						aRefData.Ref2.SetRowRel( TRUE );
/*?*/ 					}
/*?*/ 					aRefData.CalcRelFromAbs( aPos );
/*?*/ 					if ( bInList )
/*?*/ 						pNew->AddDoubleReference( aRefData );
/*?*/ 					else
/*?*/ 					{	// automagically
/*?*/ 						pNew->Add( new ScDoubleRefToken( ocColRowNameAuto,
/*?*/ 							aRefData ) );
/*?*/ 					}
/*N*/ 				}
/*N*/ 				PushTokenArray( pNew, TRUE );
/*N*/ 				pNew->Reset();
/*N*/ 				return GetToken();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			SetError(errNoName);
/*N*/ 	}
/*N*/ 	else if( pToken->GetOpCode() == ocDBArea )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); /*N*/  ScDBData* pDBData = pDoc->GetDBCollection()->FindIndex( pToken->GetIndex() );
/*N*/  /*?*/ 		if ( !pDBData )
/*N*/  /*?*/ 			SetError(errNoName);
/*N*/  /*?*/ 		else if ( !bCompileForFAP )
/*N*/  /*?*/ 		{
/*N*/  /*?*/ 			ComplRefData aRefData;
/*N*/  /*?*/ 			aRefData.InitFlags();
/*N*/  /*?*/ 			pDBData->GetArea( 	(USHORT&) aRefData.Ref1.nTab,
/*N*/  /*?*/ 								(USHORT&) aRefData.Ref1.nCol,
/*N*/  /*?*/ 								(USHORT&) aRefData.Ref1.nRow,
/*N*/  /*?*/ 								(USHORT&) aRefData.Ref2.nCol,
/*N*/  /*?*/ 								(USHORT&) aRefData.Ref2.nRow);
/*N*/  /*?*/ 			aRefData.Ref2.nTab    = aRefData.Ref1.nTab;
/*N*/  /*?*/ 			aRefData.CalcRelFromAbs( aPos );
/*N*/  /*?*/ 			ScTokenArray* pNew = new ScTokenArray;
/*N*/  /*?*/ 			pNew->AddDoubleReference( aRefData );
/*N*/  /*?*/ 			PushTokenArray( pNew, TRUE );
/*N*/  /*?*/ 			pNew->Reset();
/*N*/  /*?*/ 			return GetToken();
/*N*/  /*?*/ 		}
/*N*/ 	}
/*N*/ 	else if( pToken->GetType() == svSingleRef )
/*N*/ 	{
/*N*/ //		if (!pDoc->HasTable( pToken->aRef.Ref1.nTab ) )
/*N*/ //			SetError(errNoRef);
/*N*/ 		pArr->nRefs++;
/*N*/ 	}
/*N*/ 	else if( pToken->GetType() == svDoubleRef )
/*N*/ 	{
/*N*/ //		if (!pDoc->HasTable( pToken->aRef.Ref1.nTab ) ||
/*N*/ //			!pDoc->HasTable( pToken->aRef.Ref2.nTab ))
/*N*/ //			SetError(errNoRef);
/*N*/ 		pArr->nRefs++;
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ OpCode ScCompiler::NextToken()
/*N*/ {
/*N*/ 	if( !GetToken() )
/*N*/ 		return ocStop;
/*N*/ 	OpCode eOp = pToken->GetOpCode();
/*N*/ 	// #38815# CompileTokenArray mit zurueckgesetztem Fehler gibt wieder Fehler
/*N*/ 	if ( eOp == ocBad )
/*N*/ 		SetError( errNoName );
/*N*/ 	// Vor einem Push muss ein Operator kommen
/*N*/ 	if ( (eOp == ocPush || eOp == ocColRowNameAuto) &&
/*N*/ 			!( (eLastOp == ocOpen) || (eLastOp == ocSep) ||
/*N*/ 				((eLastOp > ocEndDiv) && (eLastOp < ocEndUnOp))) )
/*?*/ 		SetError(errOperatorExpected);
/*N*/ 	// Operator und Plus = Operator
/*N*/ 	BOOL bLastOp = ( eLastOp == ocOpen || eLastOp == ocSep ||
/*N*/ 		  (eLastOp > ocEndDiv && eLastOp < ocEndUnOp)
/*N*/ 		);
/*N*/ 	if( bLastOp && eOp == ocAdd )
/*N*/ 		eOp = NextToken();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Vor einem Operator darf kein weiterer Operator stehen
/*N*/ 		// Aber AND, OR ist OK
/*N*/ 		if ( eOp != ocAnd && eOp != ocOr
/*N*/ 		  && ( eOp > ocEndDiv && eOp < ocEndBinOp )
/*N*/ 		  && ( eLastOp == ocOpen || eLastOp == ocSep
/*N*/ 			|| (eLastOp > ocEndDiv && eLastOp < ocEndUnOp)) )
/*N*/ 		{
/*?*/ 			SetError(errVariableExpected);
/*?*/ 			if ( bAutoCorrect && !pStack )
/*?*/ 			{
/*?*/ 				if ( eOp == eLastOp || eLastOp == ocOpen )
/*?*/ 				{	// doppelten Operator verwerfen
/*?*/ 					aCorrectedSymbol.Erase();
/*?*/ 					bCorrected = TRUE;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					xub_StrLen nPos = aCorrectedFormula.Len();
/*?*/ 					if ( nPos )
/*?*/ 					{
/*?*/ 						nPos--;
/*?*/ 						sal_Unicode c = aCorrectedFormula.GetChar( nPos );
/*?*/ 						switch ( eOp )
/*?*/ 						{	// Operatoren vertauschen
/*?*/ 							case ocGreater:
/*?*/ 								if ( c == pSymbolTable[ocEqual].GetChar(0) )
/*?*/ 								{	// >= ist richtig statt =>
/*?*/ 									aCorrectedFormula.SetChar( nPos,
/*?*/ 										pSymbolTable[ocGreater].GetChar(0) );
/*?*/ 									aCorrectedSymbol = c;
/*?*/ 									bCorrected = TRUE;
/*?*/ 								}
/*?*/ 							break;
/*?*/ 							case ocLess:
/*?*/ 								if ( c == pSymbolTable[ocEqual].GetChar(0) )
/*?*/ 								{	// <= ist richtig statt =<
/*?*/ 									aCorrectedFormula.SetChar( nPos,
/*?*/ 										pSymbolTable[ocLess].GetChar(0) );
/*?*/ 									aCorrectedSymbol = c;
/*?*/ 									bCorrected = TRUE;
/*?*/ 								}
/*?*/ 								else if ( c == pSymbolTable[ocGreater].GetChar(0) )
/*?*/ 								{	// <> ist richtig statt ><
/*?*/ 									aCorrectedFormula.SetChar( nPos,
/*?*/ 										pSymbolTable[ocLess].GetChar(0) );
/*?*/ 									aCorrectedSymbol = c;
/*?*/ 									bCorrected = TRUE;
/*?*/ 								}
/*?*/ 							break;
/*?*/ 							case ocMul:
/*?*/ 								if ( c == pSymbolTable[ocSub].GetChar(0) )
/*?*/ 								{	// *- statt -*
/*?*/ 									aCorrectedFormula.SetChar( nPos,
/*?*/ 										pSymbolTable[ocMul].GetChar(0) );
/*?*/ 									aCorrectedSymbol = c;
/*?*/ 									bCorrected = TRUE;
/*?*/ 								}
/*?*/ 							break;
/*?*/ 							case ocDiv:
/*?*/ 								if ( c == pSymbolTable[ocSub].GetChar(0) )
/*?*/ 								{	// /- statt -/
/*?*/ 									aCorrectedFormula.SetChar( nPos,
/*?*/ 										pSymbolTable[ocDiv].GetChar(0) );
/*?*/ 									aCorrectedSymbol = c;
/*?*/ 									bCorrected = TRUE;
/*?*/ 								}
/*?*/ 							break;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		eLastOp = eOp;
/*N*/ 	}
/*N*/ 	return eOp;
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ BOOL ScCompiler::CompileTokenArray()
/*N*/ {
/*N*/ 	glSubTotal = FALSE;
/*N*/ 	bCorrected = FALSE;
/*N*/ 	if( !pArr->nError || bIgnoreErrors )
/*N*/ 	{
/*N*/ 		if ( bAutoCorrect )
/*N*/ 		{
/*?*/ 			aCorrectedFormula.Erase();
/*?*/ 			aCorrectedSymbol.Erase();
/*N*/ 		}
/*N*/ 		pArr->nRefs = 0;	// wird neu gezaehlt
/*N*/ 		pArr->DelRPN();
/*N*/ 		pStack = NULL;
/*N*/ 		ScToken* pData[ MAXCODE ];
/*N*/ 		pCode = pData;
/*N*/ 		BOOL bWasForced = pArr->IsRecalcModeForced();
/*N*/ 		if ( bWasForced )
/*N*/ 		{
/*N*/ 			if ( bAutoCorrect )
/*N*/ 				aCorrectedFormula = '=';
/*N*/ 		}
/*N*/ 		pArr->ClearRecalcMode();
/*N*/ 		pArr->Reset();
/*N*/ 		eLastOp = ocOpen;
/*N*/ 		pc = 0;
/*N*/ 		NextToken();
/*N*/ 		Expression();
/*N*/ 
/*N*/ 		USHORT nErrorBeforePop = pArr->nError;
/*N*/ 
/*N*/ 		while( pStack )
/*?*/ 			PopTokenArray();
/*N*/ 		if( pc )
/*N*/ 		{
/*N*/ 			pArr->pRPN = new ScToken*[ pc ];
/*N*/ 			pArr->nRPN = pc;
/*N*/ 			memcpy( pArr->pRPN, pData, pc * sizeof( ScToken* ) );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !pArr->nError && nErrorBeforePop )
/*N*/ 			pArr->nError = nErrorBeforePop;				// einmal Fehler, immer Fehler
/*N*/ 
/*N*/ 		if( pArr->nError && !bIgnoreErrors )
/*N*/ 			pArr->DelRPN();
/*N*/ 
/*N*/ 		if ( bWasForced )
/*?*/ 			pArr->SetRecalcModeForced();
/*N*/ 	}
/*N*/ 	if( nNumFmt == NUMBERFORMAT_UNDEFINED )
/*N*/ 		nNumFmt = NUMBERFORMAT_NUMBER;
/*N*/ 	return glSubTotal;
/*N*/ }

//---------------------------------------------------------------------------
// Token in den Code Eintragen
//---------------------------------------------------------------------------

/*N*/ void ScCompiler::PutCode( ScToken* p )
/*N*/ {
/*N*/     if( pc >= MAXCODE-1 )
/*N*/     {
/*?*/         if ( pc == MAXCODE-1 )
/*?*/         {
/*?*/             p = new ScByteToken( ocStop );
/*?*/             *pCode++ = p;
/*?*/             ++pc;
/*?*/             p->IncRef();
/*?*/         }
/*?*/         SetError(errCodeOverflow);
/*?*/         return;
/*N*/     }
/*N*/ 	if( pArr->GetError() && !bCompileForFAP )
/*N*/ 		return;
/*N*/ 	*pCode++ = p; pc++;
/*N*/ 	p->IncRef();
/*N*/ }

//---------------------------------------------------------------------------
// UPN-Erzeugung (durch Rekursion)
//---------------------------------------------------------------------------

/*N*/ void ScCompiler::Factor()
/*N*/ {
/*N*/ 	if ( pArr->GetError() && !bIgnoreErrors )
/*N*/ 		return;
/*N*/ 	ScTokenRef pFacToken;
/*N*/ 	OpCode eOp = pToken->GetOpCode();
/*N*/ 	if( eOp == ocPush || eOp == ocColRowNameAuto || eOp == ocMatRef ||
/*N*/ 			eOp == ocDBArea
/*N*/ 			|| (bCompileForFAP && ((eOp == ocName) || (eOp == ocDBArea)
/*N*/ 			|| (eOp == ocColRowName) || (eOp == ocBad)))
/*N*/ 		)
/*N*/ 	{
/*N*/ 		PutCode( pToken );
/*N*/ 		eOp = NextToken();
/*N*/ 		if( eOp == ocOpen )
/*N*/ 		{
/*?*/ 			// PUSH( ist ein Fehler, der durch eine unbekannte
/*?*/ 			// Funktion hervorgerufen wird.
/*?*/ 			SetError(
/*?*/ 				( pToken->GetType() == svString
/*?*/ 			   || pToken->GetType() == svSingleRef )
/*?*/ 			   ? errNoName : errOperatorExpected );
/*?*/ 			if ( bAutoCorrect && !pStack )
/*?*/ 			{	// Multiplikation annehmen
/*?*/ 				aCorrectedFormula += pSymbolTable[ocMul];
/*?*/ 				bCorrected = TRUE;
/*?*/ 				NextToken();
/*?*/ 				eOp = Expression();
/*?*/ 				if( eOp != ocClose )
/*?*/ 					SetError(errPairExpected);
/*?*/ 				else
/*?*/ 					eOp = NextToken();
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( eOp == ocOpen )
/*N*/ 	{
/*N*/ 		NextToken();
/*N*/ 		eOp = Expression();
/*N*/ 		if( eOp != ocClose )
/*N*/ 			SetError(errPairExpected);
/*N*/ 		else
/*N*/ 			eOp = NextToken();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( nNumFmt == NUMBERFORMAT_UNDEFINED )
/*N*/ 			nNumFmt = lcl_GetRetFormat( eOp );
/*N*/ 		if( eOp > ocEndUnOp && eOp < ocEndNoPar)
/*N*/ 		{
/*N*/ 			// Diese Funktionen muessen immer neu berechnet werden
/*N*/ 			switch( eOp )
/*N*/ 			{
/*N*/ 				case ocRandom:
/*N*/ 				case ocGetActDate:
/*N*/ 				case ocGetActTime:
/*N*/ 					pArr->SetRecalcModeAlways();
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			pFacToken = pToken;
/*N*/ 			eOp = NextToken();
/*N*/ 			if (eOp != ocOpen)
/*N*/ 			{
/*?*/ 				SetError(errPairExpected);
/*?*/ 				PutCode( pFacToken );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				eOp = NextToken();
/*N*/ 				if (eOp != ocClose)
/*?*/ 					SetError(errPairExpected);
/*N*/ 				PutCode(pFacToken);
/*N*/ 				eOp = NextToken();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		// Spezialfall NICHT() und NEG()
/*N*/ 		else if( eOp == ocNot || eOp == ocNeg
/*N*/ 			  || ( eOp > ocEndNoPar && eOp < ocEnd1Par) )
/*N*/ 		{
/*N*/             // Functions that have to be always recalculated
/*N*/             switch( eOp )
/*N*/             {
/*N*/                 case ocFormula:
/*?*/                     pArr->SetRecalcModeAlways();
/*N*/                 break;
/*N*/             }
/*N*/ 			pFacToken = pToken;
/*N*/ 			eOp = NextToken();
/*N*/ 			if( nNumFmt == NUMBERFORMAT_UNDEFINED && eOp == ocNot )
/*N*/ 				nNumFmt = NUMBERFORMAT_LOGICAL;
/*N*/ 			if (eOp == ocOpen)
/*N*/ 			{
/*N*/ 				NextToken();
/*N*/ 				eOp = Expression();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				SetError(errPairExpected);
/*N*/ 			if (eOp != ocClose)
/*N*/ 				SetError(errPairExpected);
/*N*/ 			else if ( !pArr->GetError() )
/*N*/ 				pFacToken->SetByte( 1 );
/*N*/ 			PutCode( pFacToken );
/*N*/ 			eOp = NextToken();
/*N*/ 		}
/*N*/ 		else if ((eOp > ocEnd1Par && eOp < ocEnd2Par)
/*N*/ 				|| eOp == ocExternal
/*N*/ 				|| eOp == ocMacro
/*N*/ 				|| eOp == ocAnd
/*N*/ 				|| eOp == ocOr
/*N*/ 				|| ( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
/*N*/ 				|| (bCompileForFAP && ((eOp == ocIf) || (eOp == ocChose)))
/*N*/ 			)
/*N*/ 		{
/*N*/ 			OpCode eFuncOp = eOp;
/*N*/ 			pFacToken = pToken;
/*N*/ 			eOp = NextToken();
/*N*/ 			BOOL bNoParam = FALSE;
/*N*/ 			BOOL bNoPair = FALSE;
/*N*/ 			BYTE nMultiAreaSep = 0;
/*N*/ 			if (eOp == ocOpen)
/*N*/ 			{
/*N*/ 				eOp = NextToken();
/*N*/ 				if ( eFuncOp == ocIndex && eOp == ocOpen )
/*N*/ 				{	// Mehrfachbereiche
/*?*/ 					BYTE SepCount = 0;
/*?*/ 					do
/*?*/ 					{
/*?*/ 						eOp = NextToken();
/*?*/ 						if ( eOp != ocClose )
/*?*/ 						{
/*?*/ 							SepCount++;
/*?*/ 							eOp = Expression();
/*?*/ 						}
/*?*/ 					} while ( (eOp == ocSep) && (!pArr->GetError() || bIgnoreErrors) );
/*?*/ 					if ( eOp != ocClose )
/*?*/ 						SetError(errPairExpected);
/*?*/ 					eOp = NextToken();
/*?*/ 					if ( eOp == ocSep )
/*?*/ 					{
/*?*/ 						nMultiAreaSep = 1;
/*?*/ 						eOp = NextToken();
/*?*/ 					}
/*?*/ 					if ( SepCount == 0 )
/*?*/ 					{
/*?*/ 						if ( eOp == ocClose )
/*?*/ 							bNoParam = TRUE;
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						PutCode( new ScByteToken( ocPush, SepCount ) );
/*?*/ 						if ( eOp != ocClose )
/*?*/ 							eOp = Expression();
/*?*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if (eOp == ocClose)
/*N*/ 						bNoParam = TRUE;
/*N*/ 					else
/*N*/ 						eOp = Expression();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/ 				if (pFacToken->GetOpCode() == ocMultiArea)
/*?*/ 				{
/*?*/ 					bNoPair = TRUE;
/*?*/ 					NextToken();
/*?*/ 					eOp = Expression();
/*?*/ 				}
/*?*/ 				else
/*?*/ 					SetError(errPairExpected);
/*N*/ 			}
/*N*/ 			BYTE SepCount = nMultiAreaSep;
/*N*/ 			if( !bNoParam )
/*N*/ 			{
/*N*/ 				SepCount++;
/*N*/ 				while ( (eOp == ocSep) && (!pArr->GetError() || bIgnoreErrors) )
/*N*/ 				{
/*N*/ 					SepCount++;
/*N*/ 					NextToken();
/*N*/ 					eOp = Expression();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ((eOp != ocClose) && !bNoPair )
/*N*/ 				SetError(errPairExpected);
/*N*/ 			else
/*N*/ 				eOp = NextToken();
/*N*/ 			// Jumps are just normal functions for the FunctionAutoPilot tree view
/*N*/ 			if ( bCompileForFAP && pFacToken->GetType() == svJump )
/*?*/ 				pFacToken = new ScFAPToken( pFacToken->GetOpCode(), SepCount, pFacToken );
/*N*/ 			else
/*N*/ 				pFacToken->SetByte( SepCount );
/*N*/ 			PutCode( pFacToken );
/*N*/ 			// Diese Funktionen muessen immer neu berechnet werden
/*N*/ 			switch( eFuncOp )
/*N*/ 			{
/*?*/ 				// ocIndirect muesste sonst bei jedem Interpret StopListening
/*?*/ 				// und StartListening fuer ein RefTripel ausfuehren
/*?*/ 				case ocIndirect:
/*?*/ 				// ocOffset und ocIndex liefern indirekte Refs
/*?*/ 				case ocOffset:
/*?*/ 				case ocIndex:
/*?*/ 					pArr->SetRecalcModeAlways();
/*?*/ 				break;
/*?*/ 				// Functions recalculated on every document load.
/*?*/ 				// Don't use SetRecalcModeOnLoad which would override ModeAlways
/*N*/ 				case ocConvert :
/*N*/					pArr->AddRecalcMode( RECALCMODE_ONLOAD );
/*N*/ 				break;
/*?*/ 				// wird die referierte verschoben, aendert sich der Wert
/*?*/ 				case ocColumn :
/*?*/ 				case ocRow :
/*?*/                 case ocCell :   // CELL needs recalc on move for some possible type values
/*?*/ 					pArr->SetRecalcModeOnRefMove();
/*?*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if (eOp == ocIf || eOp == ocChose)
/*N*/ 		{
/*N*/ 			// Die PC-Staende sind -1
/*N*/ 			pFacToken = pToken;
/*N*/ 			if ( eOp == ocIf )
/*N*/ 				pFacToken->GetJump()[ 0 ] = 3;	// if, else, behind
/*N*/ 			else
/*?*/ 				pFacToken->GetJump()[ 0 ] = MAXJUMPCOUNT+1;
/*N*/ 			eOp = NextToken();
/*N*/ 			if (eOp == ocOpen)
/*N*/ 			{
/*N*/ 				NextToken();
/*N*/ 				eOp = Expression();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				SetError(errPairExpected);
/*N*/ 			short nJumpCount = 0;
/*N*/ 			PutCode( pFacToken );
/*N*/ 			// #36253# bei AutoCorrect (da pArr->nError ignoriert wird)
/*N*/ 			// unbegrenztes ocIf gibt GPF weil ScRawToken::Clone den JumpBuffer
/*N*/ 			// anhand von nJump[0]*2+2 alloziert, was bei ocIf 3*2+2 ist
/*N*/ 			const short nJumpMax =
/*N*/ 				(pFacToken->GetOpCode() == ocIf ? 3 : MAXJUMPCOUNT);
/*N*/ 			while ( (nJumpCount < (MAXJUMPCOUNT - 1)) && (eOp == ocSep)
/*N*/ 					&& (!pArr->GetError() || bIgnoreErrors) )
/*N*/ 			{
/*N*/ 				if ( ++nJumpCount <= nJumpMax )
/*N*/ 					pFacToken->GetJump()[nJumpCount] = pc-1;
/*N*/ 				NextToken();
/*N*/ 				eOp = Expression();
/*N*/ 				PutCode( pToken );		// Als Terminator des Teilausdrucks
/*N*/ 			}
/*N*/ 			if (eOp != ocClose)
/*N*/ 				SetError(errPairExpected);
/*N*/ 			else
/*N*/ 			{
/*N*/ 				eOp = NextToken();
/*N*/ 				// auch ohne AutoCorrect gibt es hier ohne nJumpMax bei
/*N*/ 				// mehr als 3 Parametern in ocIf einen Ueberschreiber,
/*N*/ 				// das war auch schon in der 312 so (jaja, die Tester..)
/*N*/ 				if ( ++nJumpCount <= nJumpMax )
/*N*/ 					pFacToken->GetJump()[ nJumpCount ] = pc-1;
/*N*/ 				if ((pFacToken->GetOpCode() == ocIf && (nJumpCount > 3)) ||
/*N*/ 								 (nJumpCount >= MAXJUMPCOUNT))
/*N*/ 					SetError(errIllegalParameter);
/*N*/ 				else
/*N*/ 					pFacToken->GetJump()[ 0 ] = nJumpCount;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( eOp == ocBad )
/*N*/ 		{
/*?*/ 			SetError( errNoName );
/*N*/ 		}
/*N*/ 		else if ( eOp == ocClose )
/*N*/ 		{
/*?*/ 			SetError( errParameterExpected );
/*N*/ 		}
/*N*/ 		else if ( eOp == ocMissing )
/*N*/ 		{	// #84460# May occur if imported from Xcl.
/*?*/ 			// The real value for missing parameters depends on the function
/*?*/ 			// where it is used, interpreter would have to handle this.
/*?*/ 			// If it does remove this error case here, that could also be the
/*?*/ 			// time to generate ocMissing in between subsequent ocSep.
/*?*/ 			// Xcl import should map missings to values if possible.
/*?*/ 			SetError( errParameterExpected );
/*N*/ 		}
/*N*/ 		else if ( eOp == ocSep )
/*N*/ 		{	// Subsequent ocSep
/*N*/ 			SetError( errParameterExpected );
/*N*/ 			if ( bAutoCorrect && !pStack )
/*N*/ 			{
/*?*/ 				aCorrectedSymbol.Erase();
/*?*/ 				bCorrected = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			SetError( errUnknownToken );
/*?*/ 			if ( bAutoCorrect && !pStack )
/*?*/ 			{
/*?*/ 				if ( eOp == ocStop )
/*?*/ 				{	// trailing operator w/o operand
/*?*/ 					xub_StrLen nLen = aCorrectedFormula.Len();
/*?*/ 					if ( nLen )
/*?*/ 						aCorrectedFormula.Erase( nLen - 1 );
/*?*/ 					aCorrectedSymbol.Erase();
/*?*/ 					bCorrected = TRUE;
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::Unary()
/*N*/ {
/*N*/ 	if( pToken->GetOpCode() == ocAdd )
/*?*/ 		GetToken();
/*N*/ 	else if ( pToken->GetOpCode() > ocNot && pToken->GetOpCode() < ocEndUnOp )
/*N*/ 	{
/*?*/ 		ScTokenRef p = pToken;
/*?*/ 		NextToken();
/*?*/ 		Factor();
/*?*/ 		PutCode( p );
/*N*/ 	}
/*N*/ 	else
/*N*/     {
/*N*/ 		Factor();
/*N*/         while ( pToken->GetOpCode() == ocPercentSign )
/*N*/         {   // this operator _follows_ its operand
/*?*/             PutCode( pToken );
/*?*/             NextToken();
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ void ScCompiler::PowLine()
/*N*/ {
/*N*/ 	Unary();
/*N*/ 	while (pToken->GetOpCode() == ocPow)
/*N*/ 	{
/*?*/ 		ScTokenRef p = pToken;
/*?*/ 		NextToken();
/*?*/ 		Unary();
/*?*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::UnionCutLine()
/*N*/ {
/*N*/ 	PowLine();
/*N*/ 	while (pToken->GetOpCode() == ocIntersect)
/*N*/ 	{
/*?*/ 		ScTokenRef p = pToken;
/*?*/ 		NextToken();
/*?*/ 		PowLine();
/*?*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::MulDivLine()
/*N*/ {
/*N*/ 	UnionCutLine();
/*N*/ 	while (pToken->GetOpCode() == ocMul || pToken->GetOpCode() == ocDiv)
/*N*/ 	{
/*N*/ 		ScTokenRef p = pToken;
/*N*/ 		NextToken();
/*N*/ 		UnionCutLine();
/*N*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::AddSubLine()
/*N*/ {
/*N*/ 	MulDivLine();
/*N*/     while (pToken->GetOpCode() == ocAdd || pToken->GetOpCode() == ocSub)
/*N*/ 	{
/*N*/ 		ScTokenRef p = pToken;
/*N*/ 		NextToken();
/*N*/ 		MulDivLine();
/*N*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::ConcatLine()
/*N*/ {
/*N*/ 	AddSubLine();
/*N*/     while (pToken->GetOpCode() == ocAmpersand)
/*N*/ 	{
/*N*/ 		ScTokenRef p = pToken;
/*N*/ 		NextToken();
/*N*/         AddSubLine();
/*N*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::CompareLine()
/*N*/ {
/*N*/     ConcatLine();
/*N*/ 	while (pToken->GetOpCode() >= ocEqual && pToken->GetOpCode() <= ocGreaterEqual)
/*N*/ 	{
/*N*/ 		ScTokenRef p = pToken;
/*N*/ 		NextToken();
/*N*/         ConcatLine();
/*N*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void ScCompiler::NotLine()
/*N*/ {
/*N*/ 	CompareLine();
/*N*/ 	while (pToken->GetOpCode() == ocNot)
/*N*/ 	{
/*?*/ 		ScTokenRef p = pToken;
/*?*/ 		NextToken();
/*?*/ 		CompareLine();
/*?*/ 		PutCode(p);
/*N*/ 	}
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ OpCode ScCompiler::Expression()
/*N*/ {
/*N*/ 	static const short nRecursionMax = 42;
/*N*/     ScCompilerRecursionGuard aRecursionGuard( nRecursion );
/*N*/     if ( nRecursion > nRecursionMax )
/*N*/ 	{
/*?*/ 		SetError( errStackOverflow );
/*?*/ 		return ocStop;		//! stattdessen Token generieren?
/*N*/ 	}
/*N*/ 	NotLine();
/*N*/ 	while (pToken->GetOpCode() == ocAnd || pToken->GetOpCode() == ocOr)
/*N*/ 	{
/*?*/ 	 ScTokenRef p = pToken;
/*?*/ 		pToken->SetByte( 2 );		// 2 parameters!
/*?*/ 		NextToken();
/*?*/ 		NotLine();
/*?*/ 		PutCode(p);
/*N*/ 	}
/*N*/ 	return pToken->GetOpCode();
/*N*/ }

// Referenz aus benanntem Bereich mit relativen Angaben

/*N*/ void ScCompiler::SetRelNameReference()
/*N*/ {
/*N*/ 	pArr->Reset();
/*N*/ 	for( ScToken* t = pArr->GetNextReference(); t;
/*N*/ 				  t = pArr->GetNextReference() )
/*N*/ 	{
/*N*/ 		SingleRefData& rRef1 = t->GetSingleRef();
/*N*/ 		if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
/*?*/ 			rRef1.SetRelName( TRUE );
/*N*/ 		if ( t->GetType() == svDoubleRef )
/*N*/ 		{
/*?*/ 			SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
/*?*/ 			if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
/*?*/ 				rRef2.SetRelName( TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// nur relative aus RangeName mit Wrap an Position anpassen
/*N*/ void ScCompiler::MoveRelWrap()
/*N*/ {
/*N*/ 	pArr->Reset();
/*N*/ 	for( ScToken* t = pArr->GetNextReference(); t;
/*N*/ 				  t = pArr->GetNextReference() )
/*N*/ 	{
/*N*/ 		if ( t->GetType() == svSingleRef )
/*N*/ 			ScRefUpdate::MoveRelWrap( pDoc, aPos, SingleDoubleRefModifier( t->GetSingleRef() ).Ref() );
/*N*/ 		else
/*?*/ 			ScRefUpdate::MoveRelWrap( pDoc, aPos, t->GetDoubleRef() );
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScCompiler::UpdateNameReference(UpdateRefMode eUpdateRefMode,
/*N*/ 									 const ScRange& r,
/*N*/ 									 short nDx, short nDy, short nDz,
/*N*/ 									 BOOL& rChanged)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;						// wird gesetzt, wenn rel-Ref
/*N*/ 	rChanged = FALSE;
/*N*/ 	pArr->Reset();
/*N*/ 	for( ScToken* t = pArr->GetNextReference(); t;
/*N*/ 				  t = pArr->GetNextReference() )
/*N*/ 	{
/*N*/         SingleDoubleRefModifier aMod( *t );
/*N*/         ComplRefData& rRef = aMod.Ref();
/*N*/ 		if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() &&
/*N*/ 				(!rRef.Ref1.IsFlag3D() || !rRef.Ref1.IsTabRel()) &&
/*N*/ 			( t->GetType() == svSingleRef ||
/*N*/ 			(!rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel() &&
/*N*/ 				(!rRef.Ref2.IsFlag3D() || !rRef.Ref2.IsTabRel()))))
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); /*N*/  if (ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
/*N*/  /*?*/ 									 r, nDx, nDy, nDz, rRef ) != UR_NOTHING )
/*N*/  /*?*/ 				rChanged = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bRet = TRUE;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ ScRangeData* ScCompiler::UpdateInsertTab( USHORT nTable, BOOL bIsName )
/*N*/ {
/*N*/ 	ScRangeData* pRangeData = NULL;
/*N*/     short nTab;
/*N*/ 	USHORT nPosTab = aPos.Tab();	// _nach_ evtl. Increment!
/*N*/ 	USHORT nOldPosTab = ((nPosTab > nTable) ? (nPosTab - 1) : nPosTab);
/*N*/ 	BOOL bIsRel = FALSE;
/*N*/ 	ScToken* t;
/*N*/ 	pArr->Reset();
/*N*/ 	if (bIsName)
/*?*/ 		t = pArr->GetNextReference();
/*N*/ 	else
/*N*/ 		t = pArr->GetNextReferenceOrName();
/*N*/ 	while( t )
/*N*/ 	{
/*N*/ 		if( t->GetOpCode() == ocName )
/*N*/ 		{
/*?*/ 			if (!bIsName)
/*?*/ 			{
/*?*/ 				ScRangeData* pName = pDoc->GetRangeName()->FindIndex(t->GetIndex());
/*?*/ 				if (pName && pName->HasType(RT_SHAREDMOD))
/*?*/ 					pRangeData = pName;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if( t->GetType() != svIndex )	// es kann ein DB-Bereich sein !!!
/*N*/ 		{
/*N*/ 			if ( !(bIsName && t->GetSingleRef().IsTabRel()) )
/*N*/ 			{	// Namen nur absolute anpassen
/*N*/ 				SingleRefData& rRef = t->GetSingleRef();
/*N*/ 				if ( rRef.IsTabRel() )
/*N*/                 {
/*N*/ 					nTab = rRef.nRelTab + nOldPosTab;
/*N*/                     if ( nTab < 0 )
/*?*/                         nTab += pDoc->GetTableCount();  // was a wrap
/*N*/                 }
/*N*/ 				else
/*N*/ 					nTab = rRef.nTab;
/*N*/ 				if ( nTable <= nTab )
/*N*/ 					rRef.nTab = nTab + 1;
/*N*/ 				rRef.nRelTab = rRef.nTab - nPosTab;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bIsRel = TRUE;
/*N*/ 			if ( t->GetType() == svDoubleRef )
/*N*/ 			{
/*N*/ 				if ( !(bIsName && t->GetDoubleRef().Ref2.IsTabRel()) )
/*N*/ 				{	// Namen nur absolute anpassen
/*N*/ 					SingleRefData& rRef = t->GetDoubleRef().Ref2;
/*N*/ 					if ( rRef.IsTabRel() )
/*N*/                     {
/*N*/                         nTab = rRef.nRelTab + nOldPosTab;
/*N*/                         if ( nTab < 0 )
/*?*/                             nTab += pDoc->GetTableCount();  // was a wrap
/*N*/                     }
/*N*/ 					else
/*N*/ 						nTab = rRef.nTab;
/*N*/ 					if ( nTable <= nTab )
/*N*/ 						rRef.nTab = nTab + 1;
/*N*/ 					rRef.nRelTab = rRef.nTab - nPosTab;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bIsRel = TRUE;
/*N*/ 			}
/*N*/ 			if ( bIsName && bIsRel )
/*N*/ 				pRangeData = (ScRangeData*) this;	// wird in rangenam nicht dereferenziert
/*N*/ 		}
/*N*/ 		if (bIsName)
/*?*/ 			t = pArr->GetNextReference();
/*N*/ 		else
/*N*/ 			t = pArr->GetNextReferenceOrName();
/*N*/ 	}
/*N*/ 	if ( !bIsName )
/*N*/ 	{
/*N*/ 		pArr->Reset();
/*N*/ 		for ( t = pArr->GetNextReferenceRPN(); t;
/*N*/ 			  t = pArr->GetNextReferenceRPN() )
/*N*/ 		{
/*N*/ 			if ( t->GetRef() == 1 )
/*N*/ 			{
/*N*/ 				SingleRefData& rRef1 = t->GetSingleRef();
/*N*/ 				if ( !(rRef1.IsRelName() && rRef1.IsTabRel()) )
/*N*/ 				{	// Namen nur absolute anpassen
/*N*/ 					if ( rRef1.IsTabRel() )
/*N*/                     {
/*?*/                         nTab = rRef1.nRelTab + nOldPosTab;
/*?*/                         if ( nTab < 0 )
/*?*/                             nTab += pDoc->GetTableCount();  // was a wrap
/*N*/                     }
/*N*/ 					else
/*N*/ 						nTab = rRef1.nTab;
/*N*/ 					if ( nTable <= nTab )
/*N*/ 						rRef1.nTab = nTab + 1;
/*N*/ 					rRef1.nRelTab = rRef1.nTab - nPosTab;
/*N*/ 				}
/*N*/ 				if ( t->GetType() == svDoubleRef )
/*N*/ 				{
/*?*/ 					SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
/*?*/ 					if ( !(rRef2.IsRelName() && rRef2.IsTabRel()) )
/*?*/ 					{	// Namen nur absolute anpassen
/*?*/ 						if ( rRef2.IsTabRel() )
/*?*/                         {
/*?*/                             nTab = rRef2.nRelTab + nOldPosTab;
/*?*/                             if ( nTab < 0 )
/*?*/                                 nTab += pDoc->GetTableCount();  // was a wrap
/*?*/                         }
/*?*/ 						else
/*?*/ 							nTab = rRef2.nTab;
/*?*/ 						if ( nTable <= nTab )
/*?*/ 							rRef2.nTab = nTab + 1;
/*?*/ 						rRef2.nRelTab = rRef2.nTab - nPosTab;
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRangeData;
/*N*/ }

/*N*/  ScRangeData* ScCompiler::UpdateDeleteTab(USHORT nTable, BOOL bIsMove, BOOL bIsName,
/*N*/  								 BOOL& rChanged)
/*N*/  {
/*N*/  	ScRangeData* pRangeData = NULL;
/*N*/  	USHORT nTab, nTab2;
/*N*/  	USHORT nPosTab = aPos.Tab();		 // _nach_ evtl. Decrement!
/*N*/  	USHORT nOldPosTab = ((nPosTab >= nTable) ? (nPosTab + 1) : nPosTab);
/*N*/  	rChanged = FALSE;
/*N*/  	BOOL bIsRel = FALSE;
/*N*/  	ScToken* t;
/*N*/  	pArr->Reset();
/*N*/  	if (bIsName)
/*N*/  		t = pArr->GetNextReference();
/*N*/  	else
/*N*/  		t = pArr->GetNextReferenceOrName();
/*N*/  	while( t )
/*N*/  	{
/*N*/  		if( t->GetOpCode() == ocName )
/*N*/  		{
/*N*/  			if (!bIsName)
/*N*/  			{
/*N*/  				ScRangeData* pName = pDoc->GetRangeName()->FindIndex(t->GetIndex());
/*N*/  				if (pName && pName->HasType(RT_SHAREDMOD))
/*N*/  					pRangeData = pName;
/*N*/  			}
/*N*/  			rChanged = TRUE;
/*N*/  		}
/*N*/  		else if( t->GetType() != svIndex )	// es kann ein DB-Bereich sein !!!
/*N*/  		{
/*N*/  			if ( !(bIsName && t->GetSingleRef().IsTabRel()) )
/*N*/  			{	// Namen nur absolute anpassen
/*N*/  				SingleRefData& rRef = t->GetSingleRef();
/*N*/  				if ( rRef.IsTabRel() )
/*N*/  					nTab = rRef.nRelTab + nOldPosTab;
/*N*/  				else
/*N*/  					nTab = rRef.nTab;
/*N*/  				if ( nTable < nTab )
/*N*/  				{
/*N*/  					rRef.nTab = nTab - 1;
/*N*/  					rChanged = TRUE;
/*N*/  				}
/*N*/  				else if ( nTable == nTab )
/*N*/  				{
/*N*/  					if ( t->GetType() == svDoubleRef )
/*N*/  					{
/*N*/  						SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
/*N*/  						if ( rRef2.IsTabRel() )
/*N*/  							nTab2 = rRef2.nRelTab + nOldPosTab;
/*N*/  						else
/*N*/  							nTab2 = rRef2.nTab;
/*N*/  						if ( nTab == nTab2
/*N*/  						  || (nTab+1) >= pDoc->GetTableCount() )
/*N*/  						{
/*N*/  							rRef.nTab = MAXTAB+1;
/*N*/  							rRef.SetTabDeleted( TRUE );
/*N*/  						}
/*N*/  						// else: nTab zeigt spaeter auf jetziges nTable+1
/*N*/  						// => Bereich verkleinert
/*N*/  					}
/*N*/  					else
/*N*/  					{
/*N*/  						rRef.nTab = MAXTAB+1;
/*N*/  						rRef.SetTabDeleted( TRUE );
/*N*/  					}
/*N*/  					rChanged = TRUE;
/*N*/  				}
/*N*/  				rRef.nRelTab = rRef.nTab - nPosTab;
/*N*/  			}
/*N*/  			else
/*N*/  				bIsRel = TRUE;
/*N*/  			if ( t->GetType() == svDoubleRef )
/*N*/  			{
/*N*/  				if ( !(bIsName && t->GetDoubleRef().Ref2.IsTabRel()) )
/*N*/  				{	// Namen nur absolute anpassen
/*N*/  					SingleRefData& rRef = t->GetDoubleRef().Ref2;
/*N*/  					if ( rRef.IsTabRel() )
/*N*/  						nTab = rRef.nRelTab + nOldPosTab;
/*N*/  					else
/*N*/  						nTab = rRef.nTab;
/*N*/  					if ( nTable < nTab )
/*N*/  					{
/*N*/  						rRef.nTab = nTab - 1;
/*N*/  						rChanged = TRUE;
/*N*/  					}
/*N*/  					else if ( nTable == nTab )
/*N*/  					{
/*N*/  						if ( !t->GetDoubleRef().Ref1.IsTabDeleted() )
/*N*/  							rRef.nTab = nTab - 1;	// Bereich verkleinern
/*N*/  						else
/*N*/  						{
/*N*/  							rRef.nTab = MAXTAB+1;
/*N*/  							rRef.SetTabDeleted( TRUE );
/*N*/  						}
/*N*/  						rChanged = TRUE;
/*N*/  					}
/*N*/  					rRef.nRelTab = rRef.nTab - nPosTab;
/*N*/  				}
/*N*/  				else
/*N*/  					bIsRel = TRUE;
/*N*/  			}
/*N*/  			if ( bIsName && bIsRel )
/*N*/  				pRangeData = (ScRangeData*) this;	// wird in rangenam nicht dereferenziert
/*N*/  		}
/*N*/  		if (bIsName)
/*N*/  			t = pArr->GetNextReference();
/*N*/  		else
/*N*/  			t = pArr->GetNextReferenceOrName();
/*N*/  	}
/*N*/  	if ( !bIsName )
/*N*/  	{
/*N*/  		pArr->Reset();
/*N*/  		for ( t = pArr->GetNextReferenceRPN(); t;
/*N*/  			  t = pArr->GetNextReferenceRPN() )
/*N*/  		{
/*N*/  			if ( t->GetRef() == 1 )
/*N*/  			{
/*N*/  				SingleRefData& rRef1 = t->GetSingleRef();
/*N*/  				if ( !(rRef1.IsRelName() && rRef1.IsTabRel()) )
/*N*/  				{	// Namen nur absolute anpassen
/*N*/  					if ( rRef1.IsTabRel() )
/*N*/  						nTab = rRef1.nRelTab + nOldPosTab;
/*N*/  					else
/*N*/  						nTab = rRef1.nTab;
/*N*/  					if ( nTable < nTab )
/*N*/  					{
/*N*/  						rRef1.nTab = nTab - 1;
/*N*/  						rChanged = TRUE;
/*N*/  					}
/*N*/  					else if ( nTable == nTab )
/*N*/  					{
/*N*/  						if ( t->GetType() == svDoubleRef )
/*N*/  						{
/*N*/  							SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
/*N*/  							if ( rRef2.IsTabRel() )
/*N*/  								nTab2 = rRef2.nRelTab + nOldPosTab;
/*N*/  							else
/*N*/  								nTab2 = rRef2.nTab;
/*N*/  							if ( nTab == nTab2
/*N*/  							  || (nTab+1) >= pDoc->GetTableCount() )
/*N*/  							{
/*N*/  								rRef1.nTab = MAXTAB+1;
/*N*/  								rRef1.SetTabDeleted( TRUE );
/*N*/  							}
/*N*/  							// else: nTab zeigt spaeter auf jetziges nTable+1
/*N*/  							// => Bereich verkleinert
/*N*/  						}
/*N*/  						else
/*N*/  						{
/*N*/  							rRef1.nTab = MAXTAB+1;
/*N*/  							rRef1.SetTabDeleted( TRUE );
/*N*/  						}
/*N*/  						rChanged = TRUE;
/*N*/  					}
/*N*/  					rRef1.nRelTab = rRef1.nTab - nPosTab;
/*N*/  				}
/*N*/  				if ( t->GetType() == svDoubleRef )
/*N*/  				{
/*N*/  					SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
/*N*/  					if ( !(rRef2.IsRelName() && rRef2.IsTabRel()) )
/*N*/  					{	// Namen nur absolute anpassen
/*N*/  						if ( rRef2.IsTabRel() )
/*N*/  							nTab = rRef2.nRelTab + nOldPosTab;
/*N*/  						else
/*N*/  							nTab = rRef2.nTab;
/*N*/  						if ( nTable < nTab )
/*N*/  						{
/*N*/  							rRef2.nTab = nTab - 1;
/*N*/  							rChanged = TRUE;
/*N*/  						}
/*N*/  						else if ( nTable == nTab )
/*N*/  						{
/*N*/  							if ( !rRef1.IsTabDeleted() )
/*N*/  								rRef2.nTab = nTab - 1;	// Bereich verkleinern
/*N*/  							else
/*N*/  							{
/*N*/  								rRef2.nTab = MAXTAB+1;
/*N*/  								rRef2.SetTabDeleted( TRUE );
/*N*/  							}
/*N*/  							rChanged = TRUE;
/*N*/  						}
/*N*/  						rRef2.nRelTab = rRef2.nTab - nPosTab;
/*N*/  					}
/*N*/  				}
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  	return pRangeData;
/*N*/  }

/*N*/  ScToken* ScCompiler::CreateStringFromToken( String& rFormula, ScToken* pToken,
/*N*/  		BOOL bAllowArrAdvance )
/*N*/  {
/*N*/      ::rtl::OUStringBuffer aBuffer;
/*N*/      ScToken* p = CreateStringFromToken( aBuffer, pToken, bAllowArrAdvance );
/*N*/      rFormula += aBuffer;
/*N*/      return p;
/*N*/  }

/*N*/ ScToken* ScCompiler::CreateStringFromToken( ::rtl::OUStringBuffer& rBuffer, ScToken* pToken,
/*N*/ 		BOOL bAllowArrAdvance )
/*N*/ {
/*N*/ 	BOOL bNext = TRUE;
/*N*/ 	BOOL bSpaces = FALSE;
/*N*/ 	ScToken* t = pToken;
/*N*/ 	OpCode eOp = t->GetOpCode();
/*N*/ 	if( eOp >= ocAnd && eOp <= ocOr )
/*N*/ 	{
/*N*/ 		// AND, OR infix?
/*N*/ 		 if ( bAllowArrAdvance )
/*N*/ 			t = pArr->Next();
/*N*/ 		else
/*N*/ 			t = pArr->PeekNext();
/*N*/ 		bNext = FALSE;
/*N*/ 		bSpaces = ( !t || t->GetOpCode() != ocOpen );
/*N*/ 	}
/*N*/ 	if( bSpaces )
/*?*/ 		rBuffer.append(sal_Unicode(' '));
/*N*/ 
/*N*/ 	if( eOp == ocSpaces )
/*N*/     {   // most times it's just one blank
/*N*/         BYTE n = t->GetByte();
/*N*/         for ( BYTE j=0; j<n; ++j )
/*N*/         {
/*N*/             rBuffer.append(sal_Unicode(' '));
/*N*/         }
/*N*/     }
/*N*/ 	else if( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
/*?*/ 		rBuffer.appendAscii( pInternal[ eOp - ocInternalBegin ] );
/*N*/ 	else if( (USHORT) eOp < nAnzStrings)		// Keyword:
/*N*/ 		rBuffer.append(pSymbolTable[eOp]);
/*N*/ 	else
/*N*/ 	{
/*?*/ 		DBG_ERROR("Unbekannter OpCode");
/*?*/ 		rBuffer.append(ScGlobal::GetRscString(STR_NO_NAME_REF));
/*N*/ 	}
/*N*/ 	if( bNext ) switch( t->GetType() )
/*N*/ 	{
/*N*/ 		case svDouble:
/*N*/ 		{
/*N*/             if ( pSymbolTable == pSymbolTableEnglish )
/*N*/             {   // Don't go via number formatter, slows down XML export
/*N*/                 // significantly because on every formula the number formatter
/*N*/                 // has to switch to/from English/native language.
/*N*/                 ::rtl::math::doubleToUStringBuffer( rBuffer, t->GetDouble(),
/*N*/                         rtl_math_StringFormat_Automatic,
/*N*/                         rtl_math_DecimalPlaces_Max, '.', TRUE );
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 ::rtl::math::doubleToUStringBuffer( rBuffer, t->GetDouble(),
/*N*/                         rtl_math_StringFormat_Automatic,
/*N*/                         rtl_math_DecimalPlaces_Max,
/*N*/                         ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0),
/*N*/                         TRUE );
/*N*/             }
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case svString:
/*N*/ 			if( eOp == ocBad )
/*N*/ 				rBuffer.append(t->GetString());
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if (bImportXML)
/*?*/ 					rBuffer.append(t->GetString());
/*N*/ 				else
/*N*/ 				{
/*N*/ 					rBuffer.append(sal_Unicode('"'));
/*N*/ 					if ( ScGlobal::UnicodeStrChr( t->GetString().GetBuffer(), '"' ) == NULL )
/*N*/ 						rBuffer.append(t->GetString());
/*N*/ 					else
/*N*/ 					{
/*?*/ 						String aStr( t->GetString() );
/*?*/ 						xub_StrLen nPos = 0;
/*?*/ 						while ( (nPos = aStr.Search( '"', nPos)) != STRING_NOTFOUND )
/*?*/ 						{
/*?*/ 							aStr.Insert( '"', nPos );
/*?*/ 							nPos += 2;
/*?*/ 						}
/*?*/ 						rBuffer.append(aStr);
/*N*/ 					}
/*N*/ 					rBuffer.append(sal_Unicode('"'));
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case svSingleRef:
/*N*/ 		{
/*N*/ 			SingleRefData& rRef = t->GetSingleRef();
/*N*/ 			ComplRefData aRef;
/*N*/ 			aRef.Ref1 = aRef.Ref2 = rRef;
/*N*/ 			if ( eOp == ocColRowName )
/*N*/ 			{
/*N*/ 				rRef.CalcAbsIfRel( aPos );
/*N*/ 				if ( pDoc->HasStringData( rRef.nCol, rRef.nRow, rRef.nTab ) )
/*N*/ 				{
/*N*/ 					String aStr;
/*N*/ 					pDoc->GetString( rRef.nCol, rRef.nRow, rRef.nTab, aStr );
/*N*/ 					EnQuote( aStr );
/*N*/ 					rBuffer.append(aStr);
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*?*/ 					rBuffer.append(ScGlobal::GetRscString(STR_NO_NAME_REF));
/*?*/ 					MakeRefStr( rBuffer, aRef, TRUE );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				MakeRefStr( rBuffer, aRef, TRUE );
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case svDoubleRef:
/*N*/ 			MakeRefStr( rBuffer, t->GetDoubleRef(), FALSE );
/*N*/ 			break;
/*N*/ 		case svIndex:
/*N*/ 		{
/*N*/ 			::rtl::OUStringBuffer aBuffer;
/*N*/ 			switch ( eOp )
/*N*/ 			{
/*N*/ 				case ocName:
/*N*/ 				{
/*N*/ 					ScRangeData* pData = pDoc->GetRangeName()->FindIndex(t->GetIndex());
/*N*/ 					if (pData)
/*N*/ 					{
/*N*/ 						if (pData->HasType(RT_SHARED))
/*N*/ 							 pData->UpdateSymbol( aBuffer, aPos,
/*N*/ 										pSymbolTable == pSymbolTableEnglish,
/*N*/ 										bCompileXML );
/*N*/ 						else
/*N*/ 							aBuffer.append(pData->GetName());
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*?*/ 				case ocDBArea:
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); /*N*/  ScDBData* pDBData = pDoc->GetDBCollection()->FindIndex(t->GetIndex());
/*N*/  /*?*/ 					if (pDBData)
/*N*/  /*?*/ 						aBuffer.append(pDBData->GetName());
/*?*/ 				}
/*?*/ 				break;
/*?*/ 			}
/*N*/ 			if ( aBuffer.getLength() )
/*N*/ 				rBuffer.append(aBuffer);
/*N*/ 			else
/*?*/ 				rBuffer.append(ScGlobal::GetRscString(STR_NO_NAME_REF));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case svExternal:
/*N*/ 		{
/*N*/ 			//	show translated name of StarOne AddIns
/*N*/ 			String aAddIn( t->GetExternal() );
/*N*/ 			if ( pSymbolTable != pSymbolTableEnglish )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001   ScGlobal::GetAddInCollection()->LocalizeString( aAddIn );
/*N*/ 			rBuffer.append(aAddIn);
/*N*/ 		}
/*N*/ 			break;
/*N*/ 		case svByte:
/*N*/ 		case svJump:
/*N*/         case svFAP:
/*N*/ 		case svMissing:
/*N*/ 			break;		// Opcodes
/*N*/ 		default:
/*N*/ 			DBG_ERROR("ScCompiler:: GetStringFromToken errUnknownVariable");
/*N*/ 	}											// of switch
/*N*/ 	if( bSpaces )
/*?*/ 		rBuffer.append(sal_Unicode(' '));
/*N*/ 	if ( bAllowArrAdvance )
/*N*/ 	{
/*N*/ 		if( bNext )
/*N*/ 			t = pArr->Next();
/*N*/ 		return t;
/*N*/ 	}
/*N*/ 	return pToken;
/*N*/ }

/*N*/ void ScCompiler::CreateStringFromTokenArray( String& rFormula )
/*N*/ {
/*N*/     ::rtl::OUStringBuffer aBuffer( pArr->GetLen() * 2 );
/*N*/     CreateStringFromTokenArray( aBuffer );
/*N*/     rFormula = aBuffer;
/*N*/ }

/*N*/ void ScCompiler::CreateStringFromTokenArray( ::rtl::OUStringBuffer& rBuffer )
/*N*/ {
/*N*/ 	rBuffer.setLength(0);
/*N*/ 	if( !pArr->GetLen() )
/*N*/ 		return;
/*N*/ 
/*N*/     // at least one char per token, plus some are references, some are function names
/*N*/     rBuffer.ensureCapacity( pArr->GetLen() * 2 );
/*N*/ 
/*N*/ 	if ( pArr->IsRecalcModeForced() )
/*?*/ 		rBuffer.append(sal_Unicode('='));
/*N*/ 	ScToken* t = pArr->First();
/*N*/ 	while( t )
/*N*/ 		t = CreateStringFromToken( rBuffer, t, TRUE );
/*N*/ }

/*N*/ BOOL ScCompiler::EnQuote( String& rStr )
/*N*/ {
/*N*/ 	sal_Int32 nType = ScGlobal::pCharClass->getStringType( rStr, 0, rStr.Len() );
/*N*/ 	if ( !CharClass::isNumericType( nType )
/*N*/ 			&& CharClass::isAlphaNumericType( nType ) )
/*N*/ 		return FALSE;
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	while ( (nPos = rStr.Search( '\'', nPos)) != STRING_NOTFOUND )
/*N*/ 	{
/*N*/ 		rStr.Insert( '\\', nPos );
/*N*/ 		nPos += 2;
/*N*/ 	}
/*N*/ 	rStr.Insert( '\'', 0 );
/*N*/ 	rStr += '\'';
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL ScCompiler::DeQuote( String& rStr )
/*N*/ {
/*N*/ 	xub_StrLen nLen = rStr.Len();
/*N*/ 	if ( nLen > 1 && rStr.GetChar(0) == '\'' && rStr.GetChar( nLen-1 ) == '\'' )
/*N*/ 	{
/*N*/ 		rStr.Erase( nLen-1, 1 );
/*N*/ 		rStr.Erase( 0, 1 );
/*N*/ 		xub_StrLen nPos = 0;
/*N*/ 		while ( (nPos = rStr.SearchAscii( "\\\'", nPos)) != STRING_NOTFOUND )
/*N*/ 		{
/*N*/ 			rStr.Erase( nPos, 1 );
/*N*/ 			++nPos;
/*N*/ 		}
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
