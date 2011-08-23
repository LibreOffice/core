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

#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/objsh.hxx>
#include <unotools/textsearch.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/useroptions.hxx>
#include <tools/urlobj.hxx>
#include <stdlib.h>
#include <ctype.h>

#include <bf_svtools/syslocale.hxx>

#include "document.hxx"
#include "rangeutl.hxx"
#include "pivot.hxx"
#include "rechead.hxx"
#include "compiler.hxx"

#include "bf_sc.hrc"
#include "globstr.hrc"
namespace binfilter {


// -----------------------------------------------------------------------



#define MAX_LABELS 256 //!!! aus fieldwnd.hxx, muss noch nach global.hxx ???

//------------------------------------------------------------------------
// struct ScImportParam:

/*N*/ ScImportParam::ScImportParam() :
/*N*/ 	nCol1(0),
/*N*/ 	nRow1(0),
/*N*/ 	nCol2(0),
/*N*/ 	nRow2(0),
/*N*/ 	bImport(FALSE),
/*N*/ 	bNative(FALSE),
/*N*/ 	bSql(TRUE),
/*N*/ 	nType(ScDbTable)
/*N*/ {
/*N*/ }

/*N*/ ScImportParam::ScImportParam( const ScImportParam& r ) :
/*N*/ 	nCol1		(r.nCol1),
/*N*/ 	nRow1		(r.nRow1),
/*N*/ 	nCol2		(r.nCol2),
/*N*/ 	nRow2		(r.nRow2),
/*N*/ 	bImport		(r.bImport),
/*N*/ 	aDBName		(r.aDBName),
/*N*/ 	aStatement	(r.aStatement),
/*N*/ 	bNative		(r.bNative),
/*N*/ 	bSql		(r.bSql),
/*N*/ 	nType		(r.nType)
/*N*/ {
/*N*/ }

/*N*/ ScImportParam::~ScImportParam()
/*N*/ {
/*N*/ }


/*N*/ ScImportParam& ScImportParam::operator=( const ScImportParam& r )
/*N*/ {
/*N*/ 	nCol1			= r.nCol1;
/*N*/ 	nRow1			= r.nRow1;
/*N*/ 	nCol2			= r.nCol2;
/*N*/ 	nRow2			= r.nRow2;
/*N*/ 	bImport			= r.bImport;
/*N*/ 	aDBName			= r.aDBName;
/*N*/ 	aStatement		= r.aStatement;
/*N*/ 	bNative			= r.bNative;
/*N*/ 	bSql			= r.bSql;
/*N*/ 	nType			= r.nType;
/*N*/ 
/*N*/ 	return *this;
/*N*/ }



//------------------------------------------------------------------------
// struct ScQueryParam:

/*N*/ ScQueryEntry::ScQueryEntry()
/*N*/ {
/*N*/ 	bDoQuery		= FALSE;
/*N*/ 	bQueryByString	= FALSE;
/*N*/ 	eOp				= SC_EQUAL;
/*N*/ 	eConnect		= SC_AND;
/*N*/ 	nField			= 0;
/*N*/ 	nVal			= 0.0;
/*N*/ 	pStr			= new String;
/*N*/ 	pSearchParam	= NULL;
/*N*/ 	pSearchText		= NULL;
/*N*/ }

/*N*/ ScQueryEntry::ScQueryEntry(const ScQueryEntry& r)
/*N*/ {
/*N*/ 	bDoQuery		= r.bDoQuery;
/*N*/ 	bQueryByString	= r.bQueryByString;
/*N*/ 	eOp				= r.eOp;
/*N*/ 	eConnect		= r.eConnect;
/*N*/ 	nField			= r.nField;
/*N*/ 	nVal			= r.nVal;
/*N*/ 	pStr			= new String(*r.pStr);
/*N*/ 	pSearchParam	= NULL;
/*N*/ 	pSearchText		= NULL;
/*N*/ }

/*N*/ ScQueryEntry::~ScQueryEntry()
/*N*/ {
/*N*/ 	delete pStr;
/*N*/ 	if ( pSearchParam )
/*N*/ 	{
/*N*/ 		delete pSearchParam;
/*N*/ 		delete pSearchText;
/*N*/ 	}
/*N*/ }

/*N*/ ScQueryEntry& ScQueryEntry::operator=( const ScQueryEntry& r )
/*N*/ {
/*N*/ 	bDoQuery		= r.bDoQuery;
/*N*/ 	bQueryByString	= r.bQueryByString;
/*N*/ 	eOp				= r.eOp;
/*N*/ 	eConnect		= r.eConnect;
/*N*/ 	nField			= r.nField;
/*N*/ 	nVal			= r.nVal;
/*N*/ 	*pStr			= *r.pStr;
/*N*/ 	if ( pSearchParam )
/*N*/ 	{
/*?*/ 		delete pSearchParam;
/*?*/ 		delete pSearchText;
/*N*/ 	}
/*N*/ 	pSearchParam	= NULL;
/*N*/ 	pSearchText		= NULL;
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

/*N*/ void ScQueryEntry::Clear()
/*N*/ {
/*N*/ 	bDoQuery		= FALSE;
/*N*/ 	bQueryByString	= FALSE;
/*N*/ 	eOp				= SC_EQUAL;
/*N*/ 	eConnect		= SC_AND;
/*N*/ 	nField			= 0;
/*N*/ 	nVal			= 0.0;
/*N*/ 	pStr->Erase();
/*N*/ 	if ( pSearchParam )
/*N*/ 	{
/*?*/ 		delete pSearchParam;
/*?*/ 		delete pSearchText;
/*N*/ 	}
/*N*/ 	pSearchParam	= NULL;
/*N*/ 	pSearchText		= NULL;
/*N*/ }

/*N*/ BOOL ScQueryEntry::operator==( const ScQueryEntry& r ) const
/*N*/ {
/*N*/ 	return bDoQuery			== r.bDoQuery
/*N*/ 		&& bQueryByString	== r.bQueryByString
/*N*/ 		&& eOp				== r.eOp
/*N*/ 		&& eConnect			== r.eConnect
/*N*/ 		&& nField			== r.nField
/*N*/ 		&& nVal				== r.nVal
/*N*/ 		&& *pStr			== *r.pStr;
/*N*/ 	//! pSearchParam und pSearchText nicht vergleichen
/*N*/ }
/*N*/ 
/*N*/ void ScQueryEntry::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	BYTE cOp, cConnect;
/*N*/ 	rStream >> bDoQuery
/*N*/ 			>> bQueryByString
/*N*/ 			>> cOp
/*N*/ 			>> cConnect
/*N*/ 			>> nField
/*N*/ 			>> nVal;
/*N*/ 	rStream.ReadByteString( *pStr, rStream.GetStreamCharSet() );
/*N*/ 	eOp = (ScQueryOp) cOp;
/*N*/ 	eConnect = (ScQueryConnect) cConnect;
/*N*/ }
/*N*/ 
/*N*/ void ScQueryEntry::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	rStream << bDoQuery
/*N*/ 			<< bQueryByString
/*N*/ 			<< (BYTE) eOp
/*N*/ 			<< (BYTE) eConnect
/*N*/ 			<< nField
/*N*/ 			<< nVal;
/*N*/ 	rStream.WriteByteString( *pStr, rStream.GetStreamCharSet() );
/*N*/ }

/*N*/ ::utl::TextSearch* ScQueryEntry::GetSearchTextPtr( BOOL bCaseSens )
/*N*/ {
/*N*/ 	if ( !pSearchParam )
/*N*/ 	{
/*N*/ 		pSearchParam = new ::utl::SearchParam( *pStr, utl::SearchParam::SRCH_REGEXP,
/*N*/ 			bCaseSens, FALSE, FALSE );
/*N*/ 		pSearchText = new ::utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
/*N*/ 	}
/*N*/ 	return pSearchText;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScQueryParam::ScQueryParam()
/*N*/ {
/*N*/ 	nEntryCount = 0;
/*N*/ 	Clear();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScQueryParam::ScQueryParam( const ScQueryParam& r ) :
/*N*/ 		nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),nTab(r.nTab),
/*N*/ 		nDestTab(r.nDestTab),nDestCol(r.nDestCol),nDestRow(r.nDestRow),
/*N*/ 		bHasHeader(r.bHasHeader),bInplace(r.bInplace),bCaseSens(r.bCaseSens),
/*N*/ 		bRegExp(r.bRegExp),bDuplicate(r.bDuplicate),bByRow(r.bByRow),
/*N*/ 		bDestPers(r.bDestPers)
/*N*/ {
/*N*/ 	nEntryCount = 0;
/*N*/ 
/*N*/ 	Resize( r.nEntryCount );
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		pEntries[i] = r.pEntries[i];
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScQueryParam::~ScQueryParam()
/*N*/ {
/*N*/ 	delete[] pEntries;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScQueryParam::Clear()
/*N*/ {
/*N*/ 	nCol1=nRow1=nCol2=nRow2=
/*N*/ 	nDestTab=nDestCol=nDestRow = 0;
/*N*/ 	nTab = USHRT_MAX;
/*N*/ 	bHasHeader=bCaseSens=bRegExp = FALSE;
/*N*/ 	bInplace=bByRow=bDuplicate=bDestPers = TRUE;
/*N*/ 
/*N*/ 	Resize( MAXQUERY );
/*N*/ 	for (USHORT i=0; i<MAXQUERY; i++)
/*N*/ 		pEntries[i].Clear();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScQueryParam& ScQueryParam::operator=( const ScQueryParam& r )
/*N*/ {
/*N*/ 	nCol1		= r.nCol1;
/*N*/ 	nRow1		= r.nRow1;
/*N*/ 	nCol2		= r.nCol2;
/*N*/ 	nRow2		= r.nRow2;
/*N*/ 	nTab		= r.nTab;
/*N*/ 	nDestTab	= r.nDestTab;
/*N*/ 	nDestCol	= r.nDestCol;
/*N*/ 	nDestRow	= r.nDestRow;
/*N*/ 	bHasHeader	= r.bHasHeader;
/*N*/ 	bInplace	= r.bInplace;
/*N*/ 	bCaseSens	= r.bCaseSens;
/*N*/ 	bRegExp		= r.bRegExp;
/*N*/ 	bDuplicate	= r.bDuplicate;
/*N*/ 	bByRow		= r.bByRow;
/*N*/ 	bDestPers	= r.bDestPers;
/*N*/ 
/*N*/ 	Resize( r.nEntryCount );
/*N*/ 	for (USHORT i=0; i<nEntryCount; i++)
/*N*/ 		pEntries[i] = r.pEntries[i];
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------

BOOL ScQueryParam::operator==( const ScQueryParam& rOther ) const
{
    BOOL bEqual = FALSE;
 
    // Anzahl der Queries gleich?
     DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nUsed 	  = 0;
    return bEqual;
}

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ void ScQueryParam::Resize(USHORT nNew)
/*N*/ {
/*N*/ 	if ( nNew < MAXQUERY )
/*N*/ 		nNew = MAXQUERY;				// nie weniger als MAXQUERY
/*N*/ 
/*N*/ 	ScQueryEntry* pNewEntries = NULL;
/*N*/ 	if ( nNew )
/*N*/ 		pNewEntries = new ScQueryEntry[nNew];
/*N*/ 
/*N*/ 	USHORT nCopy = Min( nEntryCount, nNew );
/*N*/ 	for (USHORT i=0; i<nCopy; i++)
/*N*/ 		pNewEntries[i] = pEntries[i];
/*N*/ 
/*N*/ 	if ( nEntryCount )
/*N*/ 		delete[] pEntries;
/*N*/ 	nEntryCount = nNew;
/*N*/ 	pEntries = pNewEntries;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/  void ScQueryParam::FillInExcelSyntax(String& aCellStr, USHORT nIndex)
/*N*/  {
/*N*/  	if (aCellStr.Len() > 0)
/*N*/  	{
/*N*/  		if ( nIndex >= nEntryCount )
/*N*/  			Resize( nIndex+1 );
/*N*/  
/*N*/  		ScQueryEntry& rEntry = pEntries[nIndex];
/*N*/  
/*N*/  		rEntry.bDoQuery = TRUE;
/*N*/  		// Operatoren herausfiltern
/*N*/  		if (aCellStr.GetChar(0) == '<')
/*N*/  		{
/*N*/  			if (aCellStr.GetChar(1) == '>')
/*N*/  			{
/*N*/  				*rEntry.pStr = aCellStr.Copy(2);
/*N*/  				rEntry.eOp   = SC_NOT_EQUAL;
/*N*/  			}
/*N*/  			else if (aCellStr.GetChar(1) == '=')
/*N*/  			{
/*N*/  				*rEntry.pStr = aCellStr.Copy(2);
/*N*/  				rEntry.eOp   = SC_LESS_EQUAL;
/*N*/  			}
/*N*/  			else
/*N*/  			{
/*N*/  				*rEntry.pStr = aCellStr.Copy(1);
/*N*/  				rEntry.eOp   = SC_LESS;
/*N*/  			}
/*N*/  		}
/*N*/  		else if (aCellStr.GetChar(0) == '>')
/*N*/  		{
/*N*/  			if (aCellStr.GetChar(1) == '=')
/*N*/  			{
/*N*/  				*rEntry.pStr = aCellStr.Copy(2);
/*N*/  				rEntry.eOp   = SC_GREATER_EQUAL;
/*N*/  			}
/*N*/  			else
/*N*/  			{
/*N*/  				*rEntry.pStr = aCellStr.Copy(1);
/*N*/  				rEntry.eOp   = SC_GREATER;
/*N*/  			}
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			if (aCellStr.GetChar(0) == '=')
/*N*/  				*rEntry.pStr = aCellStr.Copy(1);
/*N*/  			else
/*N*/  				*rEntry.pStr = aCellStr;
/*N*/  			rEntry.eOp = SC_EQUAL;
/*N*/  		}
/*N*/  	}
/*N*/  }

//------------------------------------------------------------------------

/*N*/ void ScQueryParam::Load( SvStream& rStream )		// z.B. fuer Pivot-Tabelle
/*N*/ {
/*N*/ 	// bDestPers wird nicht geladen/gespeichert
/*N*/ 
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream >> nCol1
/*N*/ 			>> nRow1
/*N*/ 			>> nCol2
/*N*/ 			>> nRow2
/*N*/ 			>> nDestTab
/*N*/ 			>> nDestCol
/*N*/ 			>> nDestRow
/*N*/ 			>> bHasHeader
/*N*/ 			>> bInplace
/*N*/ 			>> bCaseSens
/*N*/ 			>> bRegExp
/*N*/ 			>> bDuplicate
/*N*/ 			>> bByRow;
/*N*/ 
/*N*/ 	Resize( MAXQUERY );
/*N*/ 
/*N*/ 	for (USHORT i=0; i<MAXQUERY; i++)
/*N*/ 		pEntries[i].Load(rStream);
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScQueryParam::Store( SvStream& rStream ) const		// z.B. fuer Pivot-Tabelle
/*N*/ {
/*N*/ 	// bDestPers wird nicht geladen/gespeichert
/*N*/ 
/*N*/ 	ScWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	DBG_ASSERT( nEntryCount <= MAXQUERY || !pEntries[MAXQUERY].bDoQuery,
/*N*/ 					"zuviele Eintraege bei ScQueryParam::Store" );
/*N*/ 
/*N*/ 
/*N*/ 	if ( nEntryCount < MAXQUERY )
/*N*/ 	{
/*N*/ 		DBG_ERROR("ScQueryParam::Store - zuwenig Eintraege");
/*N*/ 		((ScQueryParam*)this)->Resize( MAXQUERY );
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream << nCol1
/*N*/ 			<< nRow1
/*N*/ 			<< nCol2
/*N*/ 			<< nRow2
/*N*/ 			<< nDestTab
/*N*/ 			<< nDestCol
/*N*/ 			<< nDestRow
/*N*/ 			<< bHasHeader
/*N*/ 			<< bInplace
/*N*/ 			<< bCaseSens
/*N*/ 			<< bRegExp
/*N*/ 			<< bDuplicate
/*N*/ 			<< bByRow;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<MAXQUERY; i++)
/*N*/ 		pEntries[i].Store(rStream);
/*N*/ }

//------------------------------------------------------------------------
// struct ScSubTotalParam:

/*N*/ ScSubTotalParam::ScSubTotalParam()
/*N*/ {
/*N*/ 	for ( USHORT i=0; i<MAXSUBTOTAL; i++ )
/*N*/ 	{
/*N*/ 		nSubTotals[i] = 0;
/*N*/ 		pSubTotals[i] = NULL;
/*N*/ 		pFunctions[i] = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	Clear();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSubTotalParam::ScSubTotalParam( const ScSubTotalParam& r ) :
/*N*/ 		nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),
/*N*/ 		bReplace(r.bReplace),bPagebreak(r.bPagebreak),bCaseSens(r.bCaseSens),
/*N*/ 		bDoSort(r.bDoSort),bAscending(r.bAscending),bUserDef(r.bUserDef),nUserIndex(r.nUserIndex),
/*N*/ 		bIncludePattern(r.bIncludePattern),bRemoveOnly(r.bRemoveOnly)
/*N*/ {
/*N*/ 	for (USHORT i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		bGroupActive[i]	= r.bGroupActive[i];
/*N*/ 		nField[i]		= r.nField[i];
/*N*/ 
/*N*/ 		if ( (r.nSubTotals[i] > 0) && r.pSubTotals[i] && r.pFunctions[i] )
/*N*/ 		{
/*N*/ 			nSubTotals[i] = r.nSubTotals[i];
/*N*/ 			pSubTotals[i] = new USHORT			[r.nSubTotals[i]];
/*N*/ 			pFunctions[i] = new ScSubTotalFunc	[r.nSubTotals[i]];
/*N*/ 
/*N*/ 			for (USHORT j=0; j<r.nSubTotals[i]; j++)
/*N*/ 			{
/*N*/ 				pSubTotals[i][j] = r.pSubTotals[i][j];
/*N*/ 				pFunctions[i][j] = r.pFunctions[i][j];
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nSubTotals[i] = 0;
/*N*/ 			pSubTotals[i] = NULL;
/*N*/ 			pFunctions[i] = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScSubTotalParam::Clear()
/*N*/ {
/*N*/ 	nCol1=nRow1=nCol2=nRow2=nUserIndex = 0;
/*N*/ 	bPagebreak=bCaseSens=bUserDef=bIncludePattern=bRemoveOnly = FALSE;
/*N*/ 	bAscending=bReplace=bDoSort = TRUE;
/*N*/ 
/*N*/ 	USHORT i=0;
/*N*/ 	for ( i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		bGroupActive[i]	= FALSE;
/*N*/ 		nField[i]		= 0;
/*N*/ 
/*N*/ 		if ( (nSubTotals[i] > 0) && pSubTotals[i] && pFunctions[i] )
/*N*/ 		{
/*?*/ 			USHORT j=0;
/*?*/ 			for ( j=0; j<nSubTotals[i]; j++ )
/*?*/ 				pSubTotals[i][j] = 0;
/*?*/ 				pFunctions[i][j] = SUBTOTAL_FUNC_NONE;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSubTotalParam& ScSubTotalParam::operator=( const ScSubTotalParam& r )
/*N*/ {
/*N*/ 	nCol1			= r.nCol1;
/*N*/ 	nRow1			= r.nRow1;
/*N*/ 	nCol2			= r.nCol2;
/*N*/ 	nRow2			= r.nRow2;
/*N*/ 	bRemoveOnly		= r.bRemoveOnly;
/*N*/ 	bReplace		= r.bReplace;
/*N*/ 	bPagebreak		= r.bPagebreak;
/*N*/ 	bCaseSens		= r.bCaseSens;
/*N*/ 	bDoSort			= r.bDoSort;
/*N*/ 	bAscending		= r.bAscending;
/*N*/ 	bUserDef		= r.bUserDef;
/*N*/ 	nUserIndex		= r.nUserIndex;
/*N*/ 	bIncludePattern	= r.bIncludePattern;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		bGroupActive[i]	= r.bGroupActive[i];
/*N*/ 		nField[i]		= r.nField[i];
/*N*/ 		nSubTotals[i]	= r.nSubTotals[i];
/*N*/ 
/*N*/ 		if ( pSubTotals[i] ) delete [] pSubTotals[i];
/*N*/ 		if ( pFunctions[i] ) delete [] pFunctions[i];
/*N*/ 
/*N*/ 		if ( r.nSubTotals[i] > 0 )
/*N*/ 		{
/*N*/ 			pSubTotals[i] = new USHORT			[r.nSubTotals[i]];
/*N*/ 			pFunctions[i] = new ScSubTotalFunc	[r.nSubTotals[i]];
/*N*/ 
/*N*/ 			for (USHORT j=0; j<r.nSubTotals[i]; j++)
/*N*/ 			{
/*N*/ 				pSubTotals[i][j] = r.pSubTotals[i][j];
/*N*/ 				pFunctions[i][j] = r.pFunctions[i][j];
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nSubTotals[i] = 0;
/*N*/ 			pSubTotals[i] = NULL;
/*N*/ 			pFunctions[i] = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------
// struct ScConsolidateParam:

/*N*/ ScConsolidateParam::ScConsolidateParam() :
/*N*/ 	ppDataAreas( NULL )
/*N*/ {
/*N*/ 	Clear();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScConsolidateParam::ScConsolidateParam( const ScConsolidateParam& r ) :
/*N*/ 		ppDataAreas( NULL ),
/*N*/ 		nCol(r.nCol),nRow(r.nRow),nTab(r.nTab),
/*N*/ 		bByCol(r.bByCol),bByRow(r.bByRow),bReferenceData(r.bReferenceData),
/*N*/ 		nDataAreaCount(0),eFunction(r.eFunction)
/*N*/ {
/*N*/ 	if ( r.nDataAreaCount > 0 )
/*N*/ 	{
/*N*/ 		nDataAreaCount = r.nDataAreaCount;
/*N*/ 		ppDataAreas = new ScArea*[nDataAreaCount];
/*N*/ 		for ( USHORT i=0; i<nDataAreaCount; i++ )
/*N*/ 			ppDataAreas[i] = new ScArea( *(r.ppDataAreas[i]) );
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScConsolidateParam::~ScConsolidateParam()
/*N*/ {
/*N*/ 	ClearDataAreas();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void __EXPORT ScConsolidateParam::ClearDataAreas()
/*N*/ {
/*N*/ 	if ( ppDataAreas )
/*N*/ 	{
/*?*/ 		for ( USHORT i=0; i<nDataAreaCount; i++ )
/*?*/ 			delete ppDataAreas[i];
/*?*/ 		delete [] ppDataAreas;
/*?*/ 		ppDataAreas = NULL;
/*N*/ 	}
/*N*/ 	nDataAreaCount = 0;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void __EXPORT ScConsolidateParam::Clear()
/*N*/ {
/*N*/ 	ClearDataAreas();
/*N*/ 
/*N*/ 	nCol = nRow = nTab					= 0;
/*N*/ 	bByCol = bByRow = bReferenceData	= FALSE;
/*N*/ 	eFunction							= SUBTOTAL_FUNC_SUM;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScConsolidateParam& __EXPORT ScConsolidateParam::operator=( const ScConsolidateParam& r )
/*N*/ {
/*N*/ 	nCol			= r.nCol;
/*N*/ 	nRow			= r.nRow;
/*N*/ 	nTab			= r.nTab;
/*N*/ 	bByCol			= r.bByCol;
/*N*/ 	bByRow			= r.bByRow;
/*N*/ 	bReferenceData	= r.bReferenceData;
/*N*/ 	eFunction		= r.eFunction;
/*N*/ 	SetAreas( r.ppDataAreas, r.nDataAreaCount );
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ void __EXPORT ScConsolidateParam::SetAreas( ScArea* const* ppAreas, USHORT nCount )
/*N*/ {
/*N*/ 	ClearDataAreas();
/*N*/ 	if ( ppAreas && nCount > 0 )
/*N*/ 	{
/*N*/ 		ppDataAreas = new ScArea*[nCount];
/*N*/ 		for ( USHORT i=0; i<nCount; i++ )
/*N*/ 			ppDataAreas[i] = new ScArea( *(ppAreas[i]) );
/*N*/ 		nDataAreaCount = nCount;
/*N*/ 	}
/*N*/ }

/*N*/ void ScConsolidateParam::Load( SvStream& rStream )
/*N*/ {
/*N#116571#*/ 	ClearDataAreas();
/*N#116571#*/ 
/*N#116571#*/ 	ScReadHeader aHdr( rStream );
/*N#116571#*/ 
/*N#116571#*/ 	BYTE nByte;
/*N#116571#*/ 	rStream >> nCol >> nRow >> nTab
/*N#116571#*/ 			>> bByCol >> bByRow >> bReferenceData >> nByte;
/*N#116571#*/ 	eFunction = (ScSubTotalFunc) nByte;
/*N#116571#*/ 
/*N#116571#*/ 	rStream >> nDataAreaCount;
/*N#116571#*/ 	if ( nDataAreaCount )
/*N#116571#*/ 	{
/*N#116571#*/ 		ppDataAreas = new ScArea*[nDataAreaCount];
/*N#116571#*/ 		for ( USHORT i=0; i<nDataAreaCount; i++ )
/*N#116571#*/ 		{
/*N#116571#*/ 			ppDataAreas[i] = new ScArea();
/*N#116571#*/ 			rStream >> *ppDataAreas[i];
/*N#116571#*/ 		}
/*N#116571#*/ 	}
/*N*/ }

/*N*/ void ScConsolidateParam::Store( SvStream& rStream ) const
/*N*/ {
/*N#116571#*/ 	ScWriteHeader aHdr( rStream, 12+10*nDataAreaCount );
/*N#116571#*/ 
/*N#116571#*/ 	rStream << nCol << nRow << nTab
/*N#116571#*/ 			<< bByCol << bByRow << bReferenceData << (BYTE) eFunction;
/*N#116571#*/ 
/*N#116571#*/ 	rStream << nDataAreaCount;
/*N#116571#*/ 	for (USHORT i=0; i<nDataAreaCount; i++)
/*N#116571#*/ 		rStream << *ppDataAreas[i];
/*N*/ }

//------------------------------------------------------------------------
// struct ScPivotParam:

/*N*/ ScPivotParam::ScPivotParam()
/*N*/ 	:	nCol(0), nRow(0), nTab(0),
/*N*/ 		ppLabelArr( NULL ), nLabels(0),
/*N*/ 		nColCount(0), nRowCount(0), nDataCount(0),
/*N*/ 		bIgnoreEmptyRows(FALSE), bDetectCategories(FALSE),
/*N*/ 		bMakeTotalCol(TRUE), bMakeTotalRow(TRUE)
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScPivotParam::ScPivotParam( const ScPivotParam& r )
/*N*/ 	:	nCol( r.nCol ), nRow( r.nRow ), nTab( r.nTab ),
/*N*/ 		bIgnoreEmptyRows(r.bIgnoreEmptyRows),
/*N*/ 		bDetectCategories(r.bDetectCategories),
/*N*/ 		bMakeTotalCol(r.bMakeTotalCol),
/*N*/ 		bMakeTotalRow(r.bMakeTotalRow),
/*N*/ 		ppLabelArr( NULL ), nLabels(0),
/*N*/ 		nColCount(0), nRowCount(0), nDataCount(0)
/*N*/ {
/*N*/ 	SetLabelData	( r.ppLabelArr, r.nLabels );
/*N*/ 	SetPivotArrays	( r.aColArr, r.aRowArr, r.aDataArr,
/*N*/ 					  r.nColCount, r.nRowCount, r.nDataCount );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScPivotParam::~ScPivotParam()
/*N*/ {
/*N*/ 	ClearLabelData();
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ void __EXPORT ScPivotParam::ClearLabelData()
/*N*/ {
/*N*/ 	if ( (nLabels > 0) && ppLabelArr )
/*N*/ 	{
/*N*/ 		for ( USHORT i=0; i<nLabels; i++ )
/*N*/ 			delete ppLabelArr[i];
/*N*/ 		delete [] ppLabelArr;
/*N*/ 		ppLabelArr = NULL;
/*N*/ 		nLabels = 0;
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ void __EXPORT ScPivotParam::SetLabelData( LabelData**	pLabArr,
/*N*/ 										  USHORT		nLab )
/*N*/ {
/*N*/ 	ClearLabelData();
/*N*/ 
/*N*/ 	if ( (nLab > 0) && pLabArr )
/*N*/ 	{
/*N*/ 		nLabels = (nLab>MAX_LABELS) ? MAX_LABELS : nLab;
/*N*/ 		ppLabelArr = new LabelData*[nLabels];
/*N*/ 		for ( USHORT i=0; i<nLabels; i++ )
/*N*/ 			ppLabelArr[i] = new LabelData( *(pLabArr[i]) );
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void __EXPORT ScPivotParam::SetPivotArrays	( const PivotField*	pColArr,
/*N*/ 											  const PivotField*	pRowArr,
/*N*/ 											  const PivotField*	pDataArr,
/*N*/ 											  USHORT			nColCnt,
/*N*/ 											  USHORT			nRowCnt,
/*N*/ 											  USHORT			nDataCnt )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001  	ClearPivotArrays();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScPivotParam& __EXPORT ScPivotParam::operator=( const ScPivotParam& r )
/*N*/ {
/*N*/ 	nCol			  = r.nCol;
/*N*/ 	nRow			  = r.nRow;
/*N*/ 	nTab			  = r.nTab;
/*N*/ 	bIgnoreEmptyRows  = r.bIgnoreEmptyRows;
/*N*/ 	bDetectCategories = r.bDetectCategories;
/*N*/ 	bMakeTotalCol	  = r.bMakeTotalCol;
/*N*/ 	bMakeTotalRow	  = r.bMakeTotalRow;
/*N*/ 
/*N*/ 	SetLabelData	( r.ppLabelArr, r.nLabels );
/*N*/ 	SetPivotArrays	( r.aColArr, r.aRowArr, r.aDataArr,
/*N*/ 					  r.nColCount, r.nRowCount, r.nDataCount );
/*N*/ 
/*N*/ 	return *this;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------
// struct ScSolveParam

/*N*/ ScSolveParam::ScSolveParam()
/*N*/ 	:	pStrTargetVal( NULL )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSolveParam::ScSolveParam( const ScSolveParam& r )
/*N*/ 	:	aRefFormulaCell	( r.aRefFormulaCell ),
/*N*/ 		aRefVariableCell( r.aRefVariableCell ),
/*N*/ 		pStrTargetVal	( r.pStrTargetVal
/*N*/ 							? new String(*r.pStrTargetVal)
/*N*/ 							: NULL )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSolveParam::~ScSolveParam()
/*N*/ {
/*N*/ 	delete pStrTargetVal;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------



//------------------------------------------------------------------------
// struct ScTabOpParam

/*N*/ ScTabOpParam::ScTabOpParam( const ScTabOpParam& r )
/*N*/ 	:	aRefFormulaCell	( r.aRefFormulaCell ),
/*N*/ 		aRefFormulaEnd	( r.aRefFormulaEnd ),
/*N*/ 		aRefRowCell		( r.aRefRowCell ),
/*N*/ 		aRefColCell		( r.aRefColCell ),
/*N*/ 		nMode			( r.nMode )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

//========================================================================
// class ScPostIt
//========================================================================

/*N*/ ScPostIt::ScPostIt()
/*N*/ {
/*N*/ 	bShown = FALSE;
/*N*/ }

/*N*/ ScPostIt::ScPostIt( const String& rText, const String& rDate, const String& rAuthor )
/*N*/ 	:	aStrText	( rText ),
/*N*/ 		aStrDate	( rDate ),
/*N*/ 		aStrAuthor	( rAuthor )
/*N*/ {
/*N*/ 	bShown = FALSE;
/*N*/ }

/*N*/ ScPostIt::ScPostIt( const ScPostIt& rCpy )
/*N*/ 	:	aStrText	( rCpy.aStrText ),
/*N*/ 		aStrDate	( rCpy.aStrDate ),
/*N*/ 		aStrAuthor	( rCpy.aStrAuthor )
/*N*/ {
/*N*/ 	bShown = rCpy.bShown;
/*N*/ }

/*N*/ __EXPORT ScPostIt::~ScPostIt()
/*N*/ {
/*N*/ }

/*N*/ void ScPostIt::AutoSetText( const String& rNewText )
/*N*/ {
/*N*/ 	aStrText   = rNewText;
/*N*/     aStrDate   = ScGlobal::pLocaleData->getDate( Date() );
/*N*/ 
/*N*/ 	//	Der Einheitlichkeit halber das Datum immer ohne Uhrzeit (wie im Writer)
/*N*/ //	aStrDate  += ", ";
/*N*/ //  aStrDate  += ScGlobal::pLocaleData->getTime( Time() );
/*N*/ 
/*N*/ 	SvtUserOptions aUserOpt;
/*N*/ 	aStrAuthor = aUserOpt.GetID();
/*N*/ }

//========================================================================

/*N*/ String ScGlobal::GetAbsDocName( const String& rFileName,
/*N*/ 								SfxObjectShell* pShell )
/*N*/ {
/*N*/ 	String aAbsName;
/*N*/ 	if ( !pShell->HasName() )
/*N*/ 	{	// maybe relative to document path working directory
/*N*/ 		INetURLObject aObj;
/*N*/ 		SvtPathOptions aPathOpt;
/*N*/ 		aObj.SetSmartURL( aPathOpt.GetWorkPath() );
/*N*/ 		aObj.setFinalSlash();		// it IS a path
/*N*/ 		bool bWasAbs = true;
/*N*/ 		aAbsName = aObj.smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
/*N*/ 		//	returned string must be encoded because it's used directly to create SfxMedium
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SfxMedium* pMedium = pShell->GetMedium();
/*N*/ 		if ( pMedium )
/*N*/ 		{
/*N*/ 			bool bWasAbs = true;
/*N*/ 			aAbsName = pMedium->GetURLObject().smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// This can't happen, but ...
/*?*/ 			// just to be sure to have the same encoding
/*?*/ 			INetURLObject aObj;
/*?*/ 			aObj.SetSmartURL( aAbsName );
/*?*/ 			aAbsName = aObj.GetMainURL(INetURLObject::NO_DECODE);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aAbsName;
/*N*/ }


/*N*/ String ScGlobal::GetDocTabName( const String& rFileName,
/*N*/ 								const String& rTabName )
/*N*/ {
/*N*/ 	String aDocTab( '\'' );
/*N*/ 	aDocTab += rFileName;
/*N*/ 	xub_StrLen nPos = 1;
/*N*/ 	while( (nPos = aDocTab.Search( '\'', nPos ))
/*N*/ 			!= STRING_NOTFOUND )
/*N*/ 	{	// escape Quotes
/*N*/ 		aDocTab.Insert( '\\', nPos );
/*N*/ 		nPos += 2;
/*N*/ 	}
/*N*/ 	aDocTab += '\'';
/*N*/ 	aDocTab += SC_COMPILER_FILE_TAB_SEP;
/*N*/ 	aDocTab += rTabName;  	// "'Doc'#Tab"
/*N*/ 	return aDocTab;
/*N*/ }


/*N*/ USHORT lcl_ConvertSingleRef( BOOL& bExternal, const sal_Unicode* p,
/*N*/ 			ScDocument* pDoc, ScAddress& rAddr )
/*N*/ {
/*N*/ 	if ( !*p )
/*N*/ 		return 0;
/*N*/ 	USHORT	nRes = 0;
/*N*/ 	String	aDocName;		// der pure Dokumentenname
/*N*/ 	String	aDocTab;		// zusammengesetzt fuer Table
/*N*/ 	String	aTab;
/*N*/ 	BOOL	bExtDoc = FALSE;
/*N*/ 	BOOL	bNeedExtTab = FALSE;
/*N*/ 
/*N*/ 	if ( *p == '\'' && ScGlobal::UnicodeStrChr( p, SC_COMPILER_FILE_TAB_SEP ) )
/*N*/ 	{
/*?*/ 		BOOL bQuote = TRUE;			// Dokumentenname ist immer quoted
/*?*/ 		aDocTab += *p++;
/*?*/ 		while ( bQuote && *p )
/*?*/ 		{
/*?*/ 			if ( *p == '\'' && *(p-1) != '\\' )
/*?*/ 				bQuote = FALSE;
/*?*/ 			else if( !(*p == '\\' && *(p+1) == '\'') )
/*?*/ 				aDocName += *p;		// falls escaped Quote: nur Quote in den Namen
/*?*/ 			aDocTab += *p++;
/*?*/ 		}
/*?*/ 		aDocTab += *p;				// den SC_COMPILER_FILE_TAB_SEP mitnehmen
/*?*/ 		if( *p++ == SC_COMPILER_FILE_TAB_SEP )
/*?*/ 			bExtDoc = TRUE;
/*?*/ 		else
/*?*/ 			return nRes;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT	nCol = 0;
/*N*/ 	USHORT	nRow = 0;
/*N*/ 	USHORT	nTab = 0;
/*N*/ 	USHORT	nBits = SCA_VALID_TAB;
/*N*/ 	const sal_Unicode* q;
/*N*/ 	if ( ScGlobal::UnicodeStrChr( p, '.') )
/*N*/ 	{
/*N*/ 		nRes |= SCA_TAB_3D;
/*N*/ 		if ( bExtDoc )
/*N*/ 			nRes |= SCA_TAB_ABSOLUTE;
/*N*/ 		if (*p == '$')
/*N*/ 			nRes |= SCA_TAB_ABSOLUTE, p++;
/*N*/ 		BOOL bQuote = FALSE;
/*N*/ 		if( *p == '\'' )
/*N*/ 			p++, bQuote = TRUE;
/*N*/ 		while (*p && (*p != '.'))
/*N*/ 		{
/*N*/ 			if( bQuote && *p == '\'' )
/*N*/ 			{
/*N*/ 				p++; break;
/*N*/ 			}
/*N*/ 			aTab += *p++;
/*N*/ 		}
/*N*/ 		if( *p++ != '.' )
/*N*/ 			nBits = 0;
/*N*/ 		if ( pDoc )
/*N*/ 		{
/*N*/ 			if ( bExtDoc )
/*N*/ 			{
/*N*/ 				bExternal = TRUE;
/*N*/ 				aDocTab += aTab;  	// "'Doc'#Tab"
/*N*/ 				if ( !pDoc->GetTable( aDocTab, nTab ) )
/*N*/ 				{
/*N*/ 					if ( pDoc->ValidTabName( aTab ) )
/*N*/ 					{
/*N*/ 						aDocName = ScGlobal::GetAbsDocName( aDocName,
/*N*/ 							pDoc->GetDocumentShell() );
/*N*/ 						aDocTab = ScGlobal::GetDocTabName( aDocName, aTab );
/*N*/ 						if ( !pDoc->GetTable( aDocTab, nTab ) )
/*N*/ 						{
/*N*/ 							// erst einfuegen, wenn Rest der Ref ok
/*N*/ 							bNeedExtTab = TRUE;
/*N*/ 							nBits = 0;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 						nBits = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if ( !pDoc->GetTable( aTab, nTab ) )
/*N*/ 					nBits = 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nBits = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( bExtDoc )
/*N*/ 			return nRes;		// nach Dokument muss Tabelle folgen
/*N*/ 		nTab = rAddr.Tab();
/*N*/ 	}
/*N*/ 	nRes |= nBits;
/*N*/ 
/*N*/ 	q = p;
/*N*/ 	if (*p)
/*N*/ 	{
/*N*/ 		nBits = SCA_VALID_COL;
/*N*/ 		if (*p == '$')
/*N*/ 			nBits |= SCA_COL_ABSOLUTE, p++;
/*N*/ 		if( CharClass::isAsciiAlpha( *p ) )
/*N*/ 			nCol = toupper( char(*p++) ) - 'A';
/*N*/ 		else
/*N*/ 			nBits = 0;
/*N*/ 		if( CharClass::isAsciiAlpha( *p ) )
/*N*/ 			nCol = ((nCol + 1) * 26) + (toupper( char(*p++) ) - 'A');
/*N*/ 		if( nCol > MAXCOL )
/*N*/ 			nBits = 0;
/*N*/ 		while( CharClass::isAsciiAlpha( *p ) )
/*N*/ 			p++, nBits = 0;
/*N*/ 		nRes |= nBits;
/*N*/ 		if( !nBits )
/*N*/ 			p = q;
/*N*/ 	}
/*N*/ 
/*N*/ 	q = p;
/*N*/ 	if (*p)
/*N*/ 	{
/*N*/ 		nBits = SCA_VALID_ROW;
/*N*/ 		if (*p == '$')
/*N*/ 			nBits |= SCA_ROW_ABSOLUTE, p++;
/*N*/ 		if( !CharClass::isAsciiDigit( *p ) )
/*N*/ 		{
/*N*/ 			nBits = 0;
/*N*/ 			nRow = USHORT(-1);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			String aTmp( p );
/*N*/ 			long n = aTmp.ToInt32() - 1;
/*N*/ 			while (CharClass::isAsciiDigit( *p ))
/*N*/ 				p++;
/*N*/ 			if( n < 0 || n > MAXROW )
/*N*/ 				nBits = 0;
/*N*/ 			nRow = (USHORT) n;
/*N*/ 		}
/*N*/ 		nRes |= nBits;
/*N*/ 		if( !nBits )
/*N*/ 			p = q;
/*N*/ 	}
/*N*/ 	if ( bNeedExtTab )
/*N*/ 	{
/*N*/ 		if ( (nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
/*N*/ 		  && pDoc->LinkExternalTab( nTab, aDocTab, aDocName, aTab ) )
/*N*/ 		{
/*N*/ 			nRes |= SCA_VALID_TAB;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nRes = 0;	// #NAME? statt #REF!, Dateiname bleibt erhalten
/*N*/ 	}
/*N*/ 	if ( !(nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
/*N*/ 			&& !( (nRes & SCA_TAB_3D) && (nRes & SCA_VALID_TAB)) )
/*N*/ 	{	// keine Row, keine Tab, aber Col => DM (...), B (...) o.ae.
/*N*/ 		nRes = 0;
/*N*/ 	}
/*N*/ 	if( !*p )
/*N*/ 	{
/*N*/ 		USHORT nMask = nRes & ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
/*N*/ 		if( nMask == ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB ) )
/*N*/ 			nRes |= SCA_VALID;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nRes = 0;
/*N*/ 	rAddr.Set( nCol, nRow, nTab );
/*N*/ 	return nRes;
/*N*/ }


//	ConvertSingleRef mit sal_Unicode* muss vorher stehen!!!

/*N*/ BOOL ConvertSingleRef( ScDocument* pDoc, const String& rRefString,
/*N*/ 			USHORT nDefTab, ScRefTripel& rRefTripel )
/*N*/ {
/*N*/ 	BOOL bExternal = FALSE;
/*N*/ 	ScAddress aAddr( 0, 0, nDefTab );
/*N*/ 	USHORT nRes = lcl_ConvertSingleRef( bExternal, rRefString.GetBuffer(), pDoc, aAddr );
/*N*/ 	if( nRes & SCA_VALID )
/*N*/ 	{
/*N*/ 		rRefTripel.Put( aAddr.Col(), aAddr.Row(), aAddr.Tab(),
/*N*/ 						BOOL( ( nRes & SCA_COL_ABSOLUTE ) == 0 ),
/*N*/ 						BOOL( ( nRes & SCA_ROW_ABSOLUTE ) == 0 ),
/*N*/ 						BOOL( ( nRes & SCA_TAB_ABSOLUTE ) == 0 ) );
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return FALSE;
/*N*/ }


/*N*/ BOOL ConvertDoubleRef( ScDocument* pDoc, const String& rRefString, USHORT nDefTab,
/*N*/ 			ScRefTripel& rStartRefTripel, ScRefTripel& rEndRefTripel )
/*N*/ {
/*N*/ 	 BOOL bRet = FALSE;
/*N*/  	xub_StrLen nPos = rRefString.Search(':');
/*N*/  	if (nPos != STRING_NOTFOUND)
/*N*/  	{
/*N*/  		String aTmp( rRefString );
/*N*/  		sal_Unicode* p = aTmp.GetBufferAccess();
/*N*/  		p[ nPos ] = 0;
/*N*/  		if ( ConvertSingleRef( pDoc, p, nDefTab, rStartRefTripel ) )
/*N*/  		{
/*N*/  			nDefTab = rStartRefTripel.GetTab();
/*N*/  			bRet = ConvertSingleRef( pDoc, p + nPos + 1, nDefTab, rEndRefTripel );
/*N*/  		}
/*N*/  	}
/*N*/  	return bRet;
/*N*/ }


/*N*/ USHORT ScAddress::Parse( const String& r, ScDocument* pDoc )
/*N*/ {
/*N*/ 	BOOL bExternal = FALSE;
/*N*/ 	return lcl_ConvertSingleRef( bExternal, r.GetBuffer(), pDoc, *this );
/*N*/ }

/*N*/ BOOL ScRange::Intersects( const ScRange& r ) const
/*N*/ {
/*N*/ 	return !(
/*N*/ 		Min( aEnd.Col(), r.aEnd.Col() ) < Max( aStart.Col(), r.aStart.Col() )
/*N*/ 	 || Min( aEnd.Row(), r.aEnd.Row() ) < Max( aStart.Row(), r.aStart.Row() )
/*N*/ 	 || Min( aEnd.Tab(), r.aEnd.Tab() ) < Max( aStart.Tab(), r.aStart.Tab() )
/*N*/ 		);
/*N*/ }

/*N*/ void ScRange::Justify()
/*N*/ {
/*N*/ 	USHORT nTemp;
/*N*/ 	if ( aEnd.Col() < (nTemp = aStart.Col()) )
/*N*/ 	{
/*?*/ 		aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTemp);
/*N*/ 	}
/*N*/ 	if ( aEnd.Row() < (nTemp = aStart.Row()) )
/*N*/ 	{
/*?*/ 		aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTemp);
/*N*/ 	}
/*N*/ 	if ( aEnd.Tab() < (nTemp = aStart.Tab()) )
/*N*/ 	{
/*?*/ 		aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTemp);
/*N*/ 	}
/*N*/ }


/*N*/ USHORT ScRange::Parse( const String& r, ScDocument* pDoc )
/*N*/ {
/*N*/ 	USHORT nRes1 = 0, nRes2 = 0;
/*N*/ 	xub_StrLen nTmp = 0;
/*N*/ 	xub_StrLen nPos = STRING_NOTFOUND;
/*N*/ 	while ( (nTmp = r.Search( ':', nTmp )) != STRING_NOTFOUND )
/*N*/ 		nPos = nTmp++;		// der letzte zaehlt, koennte 'd:\...'!a1:a2 sein
/*N*/ 	if (nPos != STRING_NOTFOUND)
/*N*/ 	{
/*N*/ 		String aTmp( r );
/*N*/ 		sal_Unicode* p = aTmp.GetBufferAccess();
/*N*/ 		p[ nPos ] = 0;
/*N*/ 		BOOL bExternal = FALSE;
/*N*/ 		if( nRes1 = lcl_ConvertSingleRef( bExternal, p, pDoc, aStart ) )
/*N*/ 		{
/*N*/ 			aEnd = aStart;	// die Tab _muss_ gleich sein, so ist`s weniger Code
/*N*/ 			if ( nRes2 = lcl_ConvertSingleRef( bExternal, p + nPos+ 1, pDoc, aEnd ) )
/*N*/ 			{
/*N*/ 				if ( bExternal && aStart.Tab() != aEnd.Tab() )
/*N*/ 					nRes2 &= ~SCA_VALID_TAB;	// #REF!
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// PutInOrder / Justify
/*N*/ 					USHORT nTemp, nMask, nBits1, nBits2;
/*N*/ 					if ( aEnd.Col() < (nTemp = aStart.Col()) )
/*N*/ 					{
/*?*/ 						aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTemp);
/*?*/ 						nMask = (SCA_VALID_COL | SCA_COL_ABSOLUTE);
/*?*/ 						nBits1 = nRes1 & nMask;
/*?*/ 						nBits2 = nRes2 & nMask;
/*?*/ 						nRes1 = (nRes1 & ~nMask) | nBits2;
/*?*/ 						nRes2 = (nRes2 & ~nMask) | nBits1;
/*N*/ 					}
/*N*/ 					if ( aEnd.Row() < (nTemp = aStart.Row()) )
/*N*/ 					{
/*?*/ 						aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTemp);
/*?*/ 						nMask = (SCA_VALID_ROW | SCA_ROW_ABSOLUTE);
/*?*/ 						nBits1 = nRes1 & nMask;
/*?*/ 						nBits2 = nRes2 & nMask;
/*?*/ 						nRes1 = (nRes1 & ~nMask) | nBits2;
/*?*/ 						nRes2 = (nRes2 & ~nMask) | nBits1;
/*N*/ 					}
/*N*/ 					if ( aEnd.Tab() < (nTemp = aStart.Tab()) )
/*N*/ 					{
/*?*/ 						aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTemp);
/*?*/ 						nMask = (SCA_VALID_TAB | SCA_TAB_ABSOLUTE | SCA_TAB_3D);
/*?*/ 						nBits1 = nRes1 & nMask;
/*?*/ 						nBits2 = nRes2 & nMask;
/*?*/ 						nRes1 = (nRes1 & ~nMask) | nBits2;
/*?*/ 						nRes2 = (nRes2 & ~nMask) | nBits1;
/*N*/ 					}
/*N*/ 					if ( ((nRes1 & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
/*N*/ 							== ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
/*N*/ 							&& !(nRes2 & SCA_TAB_3D) )
/*N*/ 						nRes2 |= SCA_TAB_ABSOLUTE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nRes1 = 0;		// #38840# keine Tokens aus halben Sachen
/*N*/ 		}
/*N*/ 	}
/*N*/ 	nRes1 = ( ( nRes1 | nRes2 ) & SCA_VALID )
/*N*/ 		  | nRes1
/*N*/ 		  | ( ( nRes2 & 0x070F ) << 4 );
/*N*/ 	return nRes1;
/*N*/ }

/*N*/ USHORT ScRange::ParseAny( const String& r, ScDocument* pDoc )
/*N*/ {
/*N*/ 	USHORT nRet = Parse( r, pDoc );
/*N*/ 	const USHORT nValid = SCA_VALID | SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2;
/*N*/ 	if ( (nRet & nValid) != nValid )
/*N*/ 	{
/*N*/ 		ScAddress aAdr;
/*N*/ 		nRet = aAdr.Parse( r, pDoc );
/*N*/ 		if ( nRet & SCA_VALID )
/*N*/ 			aStart = aEnd = aAdr;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ void ScAddress::Format( String& r, USHORT nFlags, ScDocument* pDoc ) const
/*N*/ {
/*N*/ 	USHORT nTab = Tab();
/*N*/ 	USHORT nCol = Col();
/*N*/ 	r.Erase();
/*N*/ 	if( nFlags & SCA_VALID )
/*N*/ 		nFlags |= ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
/*N*/ 	if( pDoc && (nFlags & SCA_VALID_TAB ) )
/*N*/ 	{
/*N*/ 		if ( nTab >= pDoc->GetTableCount() )
/*N*/ 		{
/*?*/ 			r = ScGlobal::GetRscString( STR_NOREF_STR );
/*?*/ 			return;
/*N*/ 		}
/*N*/ //		if( nFlags & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ) )
/*N*/ 		if( nFlags & SCA_TAB_3D )
/*N*/ 		{
/*N*/ 			String aTabName;
/*N*/ 			pDoc->GetName( nTab, aTabName );
/*N*/ 
/*N*/ 			//	externe Referenzen (wie in ScCompiler::MakeTabStr)
/*N*/ 			String aDoc;
/*N*/ 			if ( aTabName.GetChar(0) == '\'' )
/*N*/ 			{	// "'Doc'#Tab"
/*?*/ 				xub_StrLen nPos, nLen = 1;
/*?*/ 				while( (nPos = aTabName.Search( '\'', nLen ))
/*?*/ 						!= STRING_NOTFOUND )
/*?*/ 					nLen = nPos + 1;
/*?*/ 				if ( aTabName.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
/*?*/ 				{
/*?*/ 					aDoc = aTabName.Copy( 0, nLen + 1 );
/*?*/ 					aTabName.Erase( 0, nLen + 1 );
/*?*/ 				}
/*N*/ 			}
/*N*/ 			r += aDoc;
/*N*/ 
/*N*/ 			if( nFlags & SCA_TAB_ABSOLUTE )
/*N*/ 				r += '$';
/*N*/ 			ScCompiler::CheckTabQuotes( aTabName );
/*N*/ 			r += aTabName;
/*N*/ 			r += '.';
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nFlags & SCA_VALID_COL )
/*N*/ 	{
/*N*/ 		if( nFlags & SCA_COL_ABSOLUTE )
/*N*/ 			r += '$';
/*N*/ 		if ( nCol < 26 )
/*N*/ 			r += (sal_Unicode) ( 'A' + nCol );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			r += (sal_Unicode) ( 'A' + ( nCol / 26 ) - 1 );
/*N*/ 			r += (sal_Unicode) ( 'A' + ( nCol % 26 ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nFlags & SCA_VALID_ROW )
/*N*/ 	{
/*N*/ 		if ( nFlags & SCA_ROW_ABSOLUTE )
/*N*/ 			r += '$';
/*N*/ 		r += String::CreateFromInt32( Row()+1 );
/*N*/ 	}
/*N*/ }


/*N*/ void ScRange::Format( String& r, USHORT nFlags, ScDocument* pDoc ) const
/*N*/ {
/*N*/ 	if( !( nFlags & SCA_VALID ) )
/*N*/ 		r = ScGlobal::GetRscString( STR_NOREF_STR );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bOneTab = (aStart.Tab() == aEnd.Tab());
/*N*/ 		if ( !bOneTab )
/*N*/ 			nFlags |= SCA_TAB_3D;
/*N*/ 		aStart.Format( r, nFlags, pDoc );
/*N*/ 		if( aStart != aEnd )
/*N*/ 		{
/*N*/ 			String aName;
/*N*/ 			nFlags = ( nFlags & SCA_VALID ) | ( ( nFlags >> 4 ) & 0x070F );
/*N*/ 			if ( bOneTab )
/*N*/ 				pDoc = NULL;
/*N*/ 			else
/*N*/ 				nFlags |= SCA_TAB_3D;
/*N*/ 			aEnd.Format( aName, nFlags, pDoc );
/*N*/ 			r += ':';
/*N*/ 			r += aName;
/*N*/ 		}
/*N*/ 	}
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
