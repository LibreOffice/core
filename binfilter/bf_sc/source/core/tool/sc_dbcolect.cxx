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

#include <tools/debug.hxx>
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#include "dbcolect.hxx"
#include "rechead.hxx"
#include "document.hxx"
#include "globstr.hrc"
namespace binfilter {


//---------------------------------------------------------------------------------------

/*N*/ ScDBData::ScDBData( const String& rName,
/*N*/ 					USHORT nTab,
/*N*/ 					USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
/*N*/ 					BOOL bByR, BOOL bHasH) :
/*N*/ 	aName		(rName),
/*N*/ 	nTable		(nTab),
/*N*/ 	nStartCol	(nCol1),
/*N*/ 	nStartRow	(nRow1),
/*N*/ 	nEndCol		(nCol2),
/*N*/ 	nEndRow		(nRow2),
/*N*/ 	bByRow		(bByR),
/*N*/ 	bDoSize		(FALSE),
/*N*/ 	bKeepFmt	(FALSE),
/*N*/ 	bStripData	(FALSE),
/*N*/ 	bHasHeader	(bHasH),
/*N*/ 	bDBSelection(FALSE),
/*N*/ 	nIndex 		(0),
/*N*/ 	nExportIndex(0),
/*N*/ 	bIsAdvanced	(FALSE),
/*N*/ 	bAutoFilter (FALSE),
/*N*/ 	bModified	(FALSE)
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	ScSortParam aSortParam;
/*N*/ 	ScQueryParam aQueryParam;
/*N*/ 	ScSubTotalParam aSubTotalParam;
/*N*/ 	ScImportParam aImportParam;
/*N*/ 
/*N*/ 	for (i=0; i<MAXQUERY; i++)
/*N*/ 		pQueryStr[i] = new String;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		nSubTotals[i] = 0;
/*N*/ 		pSubTotals[i] = NULL;
/*N*/ 		pFunctions[i] = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetSortParam( aSortParam );
/*N*/ 	SetQueryParam( aQueryParam );
/*N*/ 	SetSubTotalParam( aSubTotalParam );
/*N*/ 	SetImportParam( aImportParam );
/*N*/ }

/*N*/ ScDBData::ScDBData( SvStream& rStream, ScMultipleReadHeader& rHdr ) :
/*N*/ 					// nicht in der Datei:
/*N*/ 	bAutoFilter		(FALSE),
/*N*/ 	bModified		(FALSE),
/*N*/ 	nExportIndex 	(0),
/*N*/ 					// nicht in alten Versionen:
/*N*/ 	bDoSize			(FALSE),
/*N*/ 	bKeepFmt		(FALSE),
/*N*/ 	bStripData		(FALSE),
/*N*/ 	nIndex 			(0),
/*N*/ 	bIsAdvanced		(FALSE),
/*N*/ 	bDBSelection	(FALSE),
/*N*/ 	bDBSql			(TRUE),
/*N*/ 	nDBType			(ScDbTable),
/*N*/ 	nSubUserIndex	(0),
/*N*/ 	bSortUserDef	(FALSE),
/*N*/ 	nSortUserIndex	(0)
/*N*/ {
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	USHORT j;
/*N*/ 	BYTE nDummy;
/*N*/ 	rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
/*N*/ 
/*N*/ 	rStream.ReadByteString( aName, eCharSet );
/*N*/ 	rStream >> nTable;
/*N*/ 	rStream >> nStartCol;
/*N*/ 	rStream >> nStartRow;
/*N*/ 	rStream >> nEndCol;
/*N*/ 	rStream >> nEndRow;
/*N*/ 	rStream >> bByRow;
/*N*/ 	rStream >> bHasHeader;
/*N*/ 	rStream >> bSortCaseSens;
/*N*/ 	rStream >> bIncludePattern;
/*N*/ 	rStream	>> bSortInplace;
/*N*/ 	rStream	>> nSortDestTab;
/*N*/ 	rStream	>> nSortDestCol;
/*N*/ 	rStream	>> nSortDestRow;
/*N*/ 	rStream >> bQueryInplace;
/*N*/ 	rStream >> bQueryCaseSens;
/*N*/ 	rStream >> bQueryRegExp;
/*N*/ 	rStream >> bQueryDuplicate;
/*N*/ 	rStream >> nQueryDestTab;
/*N*/ 	rStream >> nQueryDestCol;
/*N*/ 	rStream >> nQueryDestRow;
/*N*/ 	rStream >> bSubRemoveOnly;
/*N*/ 	rStream >> bSubReplace;
/*N*/ 	rStream >> bSubPagebreak;
/*N*/ 	rStream >> bSubCaseSens;
/*N*/ 	rStream >> bSubDoSort;
/*N*/ 	rStream >> bSubAscending;
/*N*/ 	rStream >> bSubIncludePattern;
/*N*/ 	rStream >> bSubUserDef;
/*N*/ 	rStream >> bDBImport;
/*N*/ 
/*N*/ 	rStream.ReadByteString( aDBName, eCharSet );
/*N*/ 	rStream.ReadByteString( aDBStatement, eCharSet );
/*N*/ 	rStream >> bDBNative;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		rStream >> bDoSort[i];
/*N*/ 		rStream >> nSortField[i];
/*N*/ 		rStream >> bAscending[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXQUERY; i++)
/*N*/ 	{
/*N*/ 		rStream >> bDoQuery[i];
/*N*/ 		rStream >> nQueryField[i];
/*N*/ 		rStream >> nDummy; eQueryOp[i] = (ScQueryOp) nDummy;
/*N*/ 		rStream >> bQueryByString[i];
/*N*/ 		pQueryStr[i] = new String;
/*N*/ 		rStream.ReadByteString( *pQueryStr[i], eCharSet );
/*N*/ 		rStream >> nQueryVal[i];
/*N*/ 		rStream >> nDummy; eQueryConnect[i] = (ScQueryConnect) nDummy;
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		rStream >> bDoSubTotal[i];
/*N*/ 		rStream >> nSubField[i];
/*N*/ 
/*N*/ 		USHORT nCount;
/*N*/ 		rStream >> nCount;
/*N*/ 		nSubTotals[i] = nCount;
/*N*/ 
/*N*/ 		pSubTotals[i] = nCount ? new USHORT			[nCount] : NULL;
/*N*/ 		pFunctions[i] = nCount ? new ScSubTotalFunc	[nCount] : NULL;
/*N*/ 
/*N*/ 		for (j=0; j<nCount; j++)
/*N*/ 		{
/*N*/ 			rStream >> pSubTotals[i][j];
/*N*/ 			rStream >> nDummy; pFunctions[i][j] = (ScSubTotalFunc)nDummy;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 		rStream >> nIndex;
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 		rStream >> bDBSelection;
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 		rStream >> bDBSql;				// Default = TRUE
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 	{
/*N*/ 		rStream >> nSubUserIndex;
/*N*/ 		rStream >> bSortUserDef;
/*N*/ 		rStream >> nSortUserIndex;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 	{
/*N*/ 		rStream >> bDoSize;
/*N*/ 		rStream >> bKeepFmt;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 		rStream >> bStripData;
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 		rStream >> nDBType;				// Default = ScDbTable
/*N*/ 
/*N*/ 	if (rHdr.BytesLeft())
/*N*/ 	{
/*N*/ 		rStream >> bIsAdvanced;			// Default = FALSE
/*N*/ 		if (bIsAdvanced)
/*N*/ 			rStream >> aAdvSource;
/*N*/ 	}
/*N*/ 
/*N*/ 	// aSortLocale / aSortAlgorithm are not in binary file format
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 	// #43070# rottes Dokument?!?
/*N*/ 	// nEndCol war 258
/*N*/ 	// und auch die CellInfo pPattern in ScOutputData FindRotated waren NULL
/*N*/ 	if ( nStartCol > MAXCOL )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "nStartCol > MAXCOL" );
/*N*/ 		nStartCol = MAXCOL;
/*N*/ 	}
/*N*/ 	if ( nStartRow > MAXROW )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "nStartRow > MAXROW" );
/*N*/ 		nStartRow = MAXROW;
/*N*/ 	}
/*N*/ 	if ( nEndCol > MAXCOL )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "nEndCol > MAXCOL" );
/*N*/ 		nEndCol = MAXCOL;
/*N*/ 	}
/*N*/ 	if ( nEndRow > MAXROW )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "nEndRow > MAXROW" );
/*N*/ 		nEndRow = MAXROW;
/*N*/ 	}
/*N*/ 	if ( nQueryDestCol > MAXCOL )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "nQueryDestCol > MAXCOL" );
/*N*/ 		nQueryDestCol = MAXCOL;
/*N*/ 	}
/*N*/ 	if ( nQueryDestRow > MAXROW )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "nQueryDestRow > MAXROW" );
/*N*/ 		nQueryDestRow = MAXROW;
/*N*/ 	}
/*N*/ }

/*N*/ BOOL ScDBData::Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
/*N*/ {
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	USHORT j;
/*N*/ 	rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
/*N*/ 
/*N*/ 	rStream.WriteByteString( aName, eCharSet );
/*N*/ 	rStream << nTable;
/*N*/ 	rStream << nStartCol;
/*N*/ 	rStream << nStartRow;
/*N*/ 	rStream << nEndCol;
/*N*/ 	rStream << nEndRow;
/*N*/ 	rStream << bByRow;
/*N*/ 	rStream << bHasHeader;
/*N*/ 	rStream << bSortCaseSens;
/*N*/ 	rStream << bIncludePattern;
/*N*/ 	rStream	<< bSortInplace;
/*N*/ 	rStream	<< nSortDestTab;
/*N*/ 	rStream	<< nSortDestCol;
/*N*/ 	rStream	<< nSortDestRow;
/*N*/ 	rStream << bQueryInplace;
/*N*/ 	rStream << bQueryCaseSens;
/*N*/ 	rStream << bQueryRegExp;
/*N*/ 	rStream << bQueryDuplicate;
/*N*/ 	rStream << nQueryDestTab;
/*N*/ 	rStream << nQueryDestCol;
/*N*/ 	rStream << nQueryDestRow;
/*N*/ 	rStream << bSubRemoveOnly;
/*N*/ 	rStream << bSubReplace;
/*N*/ 	rStream << bSubPagebreak;
/*N*/ 	rStream << bSubCaseSens;
/*N*/ 	rStream << bSubDoSort;
/*N*/ 	rStream << bSubAscending;
/*N*/ 	rStream << bSubIncludePattern;
/*N*/ 	rStream << bSubUserDef;
/*N*/ 	rStream << bDBImport;
/*N*/ 
/*N*/ 	rStream.WriteByteString( aDBName, eCharSet );
/*N*/ 	rStream.WriteByteString( aDBStatement, eCharSet );
/*N*/ 	rStream << bDBNative;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		rStream << bDoSort[i];
/*N*/ 		rStream << nSortField[i];
/*N*/ 		rStream << bAscending[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXQUERY; i++)
/*N*/ 	{
/*N*/ 		rStream << bDoQuery[i];
/*N*/ 		rStream << nQueryField[i];
/*N*/ 		rStream << (BYTE) eQueryOp[i];
/*N*/ 		rStream << bQueryByString[i];
/*N*/ 		rStream.WriteByteString( *pQueryStr[i], eCharSet );
/*N*/ 		rStream << nQueryVal[i];
/*N*/ 		rStream << (BYTE) eQueryConnect[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		rStream << bDoSubTotal[i];
/*N*/ 		rStream << nSubField[i];
/*N*/ 
/*N*/ 		USHORT nCount = nSubTotals[i];
/*N*/ 		rStream << nCount;
/*N*/ 		for (j=0; j<nCount; j++)
/*N*/ 		{
/*N*/ 			rStream << pSubTotals[i][j];
/*N*/ 			rStream << (BYTE)pFunctions[i][j];
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rStream << nIndex;					// seit 24.10.95
/*N*/ 
/*N*/ 	rStream << bDBSelection;
/*N*/ 
/*N*/ 	rStream << bDBSql;					// seit 4.2.97
/*N*/ 
/*N*/ 	rStream << nSubUserIndex;			// seit 5.2.97
/*N*/ 	rStream << bSortUserDef;
/*N*/ 	rStream << nSortUserIndex;
/*N*/ 
/*N*/ 	rStream << bDoSize;					// seit 13.2.97
/*N*/ 	rStream << bKeepFmt;
/*N*/ 
/*N*/ 	rStream << bStripData;				// seit 23.2.97
/*N*/ 
/*N*/ 	if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		//	folgendes gab's in der 4.0 noch nicht
/*N*/ 
/*N*/ 		//	alte Versionen suchen immer nach Tables und Queries
/*N*/ 		rStream << nDBType;					// seit 20.11.97
/*N*/ 
/*N*/ 		//	starting from 591, store advanced filter source range
/*N*/ 		//	only if set, to avoid unneccessary warnings
/*N*/ 		if (bIsAdvanced)
/*N*/ 		{
/*?*/ 			rStream << (BOOL) TRUE;
/*?*/ 			rStream << aAdvSource;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// aSortLocale / aSortAlgorithm are not in binary file format
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ ScDBData::ScDBData( const ScDBData& rData ) :
/*N*/ 	ScRefreshTimer		( rData ),
/*N*/ 	aName				(rData.aName),
/*N*/ 	nTable				(rData.nTable),
/*N*/ 	nStartCol			(rData.nStartCol),
/*N*/ 	nStartRow			(rData.nStartRow),
/*N*/ 	nEndCol				(rData.nEndCol),
/*N*/ 	nEndRow				(rData.nEndRow),
/*N*/ 	bByRow				(rData.bByRow),
/*N*/ 	bHasHeader			(rData.bHasHeader),
/*N*/ 	bDoSize				(rData.bDoSize),
/*N*/ 	bKeepFmt			(rData.bKeepFmt),
/*N*/ 	bStripData			(rData.bStripData),
/*N*/ 	bSortCaseSens   	(rData.bSortCaseSens),
/*N*/ 	bIncludePattern 	(rData.bIncludePattern),
/*N*/ 	bSortInplace		(rData.bSortInplace),
/*N*/ 	nSortDestTab		(rData.nSortDestTab),
/*N*/ 	nSortDestCol		(rData.nSortDestCol),
/*N*/ 	nSortDestRow		(rData.nSortDestRow),
/*N*/ 	bSortUserDef		(rData.bSortUserDef),
/*N*/ 	nSortUserIndex		(rData.nSortUserIndex),
/*N*/ 	aSortLocale			(rData.aSortLocale),
/*N*/ 	aSortAlgorithm		(rData.aSortAlgorithm),
/*N*/ 	bQueryInplace   	(rData.bQueryInplace),
/*N*/ 	bQueryCaseSens  	(rData.bQueryCaseSens),
/*N*/ 	bQueryRegExp		(rData.bQueryRegExp),
/*N*/ 	bQueryDuplicate		(rData.bQueryDuplicate),
/*N*/ 	nQueryDestTab   	(rData.nQueryDestTab),
/*N*/ 	nQueryDestCol	  	(rData.nQueryDestCol),
/*N*/ 	nQueryDestRow  		(rData.nQueryDestRow),
/*N*/ 	bIsAdvanced			(rData.bIsAdvanced),
/*N*/ 	aAdvSource			(rData.aAdvSource),
/*N*/ 	bSubRemoveOnly		(rData.bSubRemoveOnly),
/*N*/ 	bSubReplace			(rData.bSubReplace),
/*N*/ 	bSubPagebreak		(rData.bSubPagebreak),
/*N*/ 	bSubCaseSens		(rData.bSubCaseSens),
/*N*/ 	bSubDoSort			(rData.bSubDoSort),
/*N*/ 	bSubAscending		(rData.bSubAscending),
/*N*/ 	bSubIncludePattern	(rData.bSubIncludePattern),
/*N*/ 	bSubUserDef			(rData.bSubUserDef),
/*N*/ 	nSubUserIndex		(rData.nSubUserIndex),
/*N*/ 	bDBImport			(rData.bDBImport),
/*N*/ 	aDBName				(rData.aDBName),
/*N*/ 	aDBStatement		(rData.aDBStatement),
/*N*/ 	bDBNative			(rData.bDBNative),
/*N*/ 	bDBSelection		(rData.bDBSelection),
/*N*/ 	bDBSql				(rData.bDBSql),
/*N*/ 	nDBType				(rData.nDBType),
/*N*/ 	nIndex   			(rData.nIndex),
/*N*/ 	nExportIndex   		(rData.nExportIndex),
/*N*/ 	bAutoFilter			(rData.bAutoFilter),
/*N*/ 	bModified			(rData.bModified)
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	USHORT j;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		bDoSort[i]		= rData.bDoSort[i];
/*N*/ 		nSortField[i]	= rData.nSortField[i];
/*N*/ 		bAscending[i]	= rData.bAscending[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXQUERY; i++)
/*N*/ 	{
/*N*/ 		bDoQuery[i]			= rData.bDoQuery[i];
/*N*/ 		nQueryField[i]		= rData.nQueryField[i];
/*N*/ 		eQueryOp[i]			= rData.eQueryOp[i];
/*N*/ 		bQueryByString[i]	= rData.bQueryByString[i];
/*N*/ 		pQueryStr[i]		= new String( *(rData.pQueryStr[i]) );
/*N*/ 		nQueryVal[i]		= rData.nQueryVal[i];
/*N*/ 		eQueryConnect[i]	= rData.eQueryConnect[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		bDoSubTotal[i]		= rData.bDoSubTotal[i];
/*N*/ 		nSubField[i]		= rData.nSubField[i];
/*N*/ 
/*N*/ 		USHORT nCount	= rData.nSubTotals[i];
/*N*/ 		nSubTotals[i]	= nCount;
/*N*/ 		pFunctions[i]	= nCount ? new ScSubTotalFunc [nCount] : NULL;
/*N*/ 		pSubTotals[i]	= nCount ? new USHORT		  [nCount] : NULL;
/*N*/ 
/*N*/ 		for (j=0; j<nCount; j++)
/*N*/ 		{
/*N*/ 			pSubTotals[i][j] = rData.pSubTotals[i][j];
/*N*/ 			pFunctions[i][j] = rData.pFunctions[i][j];
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/  ScDBData& ScDBData::operator= (const ScDBData& rData)
/*N*/  {
/*N*/  	USHORT i;
/*N*/  	USHORT j;
/*N*/  
/*N*/ 	ScRefreshTimer::operator=( rData );
/*N*/ 	aName				= rData.aName;
/*N*/ 	nTable				= rData.nTable;
/*N*/ 	nStartCol			= rData.nStartCol;
/*N*/ 	nStartRow			= rData.nStartRow;
/*N*/ 	nEndCol				= rData.nEndCol;
/*N*/ 	nEndRow				= rData.nEndRow;
/*N*/ 	bByRow				= rData.bByRow;
/*N*/ 	bHasHeader			= rData.bHasHeader;
/*N*/ 	bDoSize				= rData.bDoSize;
/*N*/ 	bKeepFmt			= rData.bKeepFmt;
/*N*/ 	bStripData			= rData.bStripData;
/*N*/ 	bSortCaseSens   	= rData.bSortCaseSens;
/*N*/ 	bIncludePattern 	= rData.bIncludePattern;
/*N*/ 	bSortInplace		= rData.bSortInplace;
/*N*/ 	nSortDestTab		= rData.nSortDestTab;
/*N*/ 	nSortDestCol		= rData.nSortDestCol;
/*N*/ 	nSortDestRow		= rData.nSortDestRow;
/*N*/ 	bSortUserDef		= rData.bSortUserDef;
/*N*/ 	nSortUserIndex		= rData.nSortUserIndex;
/*N*/ 	aSortLocale			= rData.aSortLocale;
/*N*/ 	aSortAlgorithm		= rData.aSortAlgorithm;
/*N*/ 	bQueryInplace   	= rData.bQueryInplace;
/*N*/ 	bQueryCaseSens  	= rData.bQueryCaseSens;
/*N*/ 	bQueryRegExp		= rData.bQueryRegExp;
/*N*/ 	bQueryDuplicate		= rData.bQueryDuplicate;
/*N*/ 	nQueryDestTab   	= rData.nQueryDestTab;
/*N*/ 	nQueryDestCol	  	= rData.nQueryDestCol;
/*N*/ 	nQueryDestRow  		= rData.nQueryDestRow;
/*N*/ 	bIsAdvanced			= rData.bIsAdvanced;
/*N*/ 	aAdvSource			= rData.aAdvSource;
/*N*/ 	bSubRemoveOnly		= rData.bSubRemoveOnly;
/*N*/ 	bSubReplace			= rData.bSubReplace;
/*N*/ 	bSubPagebreak		= rData.bSubPagebreak;
/*N*/ 	bSubCaseSens		= rData.bSubCaseSens;
/*N*/ 	bSubDoSort			= rData.bSubDoSort;
/*N*/ 	bSubAscending		= rData.bSubAscending;
/*N*/ 	bSubIncludePattern	= rData.bSubIncludePattern;
/*N*/ 	bSubUserDef			= rData.bSubUserDef;
/*N*/ 	nSubUserIndex		= rData.nSubUserIndex;
/*N*/ 	bDBImport			= rData.bDBImport;
/*N*/ 	aDBName				= rData.aDBName;
/*N*/ 	aDBStatement		= rData.aDBStatement;
/*N*/ 	bDBNative			= rData.bDBNative;
/*N*/ 	bDBSelection		= rData.bDBSelection;
/*N*/ 	bDBSql				= rData.bDBSql;
/*N*/ 	nDBType				= rData.nDBType;
/*N*/ 	nIndex   			= rData.nIndex;
/*N*/ 	nExportIndex   		= rData.nExportIndex;
/*N*/     bAutoFilter         = rData.bAutoFilter;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		bDoSort[i]		= rData.bDoSort[i];
/*N*/ 		nSortField[i]	= rData.nSortField[i];
/*N*/ 		bAscending[i]	= rData.bAscending[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXQUERY; i++)
/*N*/ 	{
/*N*/ 		bDoQuery[i]			= rData.bDoQuery[i];
/*N*/ 		nQueryField[i]		= rData.nQueryField[i];
/*N*/ 		eQueryOp[i]			= rData.eQueryOp[i];
/*N*/ 		bQueryByString[i]	= rData.bQueryByString[i];
/*N*/ 		*pQueryStr[i]		= *rData.pQueryStr[i];
/*N*/ 		nQueryVal[i]		= rData.nQueryVal[i];
/*N*/ 		eQueryConnect[i]	= rData.eQueryConnect[i];
/*N*/ 	}
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		bDoSubTotal[i]		= rData.bDoSubTotal[i];
/*N*/ 		nSubField[i]		= rData.nSubField[i];
/*N*/ 		USHORT nCount	= rData.nSubTotals[i];
/*N*/ 		nSubTotals[i]	= nCount;
/*N*/ 
/*N*/ 		delete[] pSubTotals[i];
/*N*/ 		delete[] pFunctions[i];
/*N*/ 
/*N*/ 		pSubTotals[i] = nCount ? new USHORT			[nCount] : NULL;
/*N*/ 		pFunctions[i] = nCount ? new ScSubTotalFunc [nCount] : NULL;
/*N*/ 		for (j=0; j<nCount; j++)
/*N*/ 		{
/*N*/ 			pSubTotals[i][j] = rData.pSubTotals[i][j];
/*N*/ 			pFunctions[i][j] = rData.pFunctions[i][j];
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return *this;
/*N*/ }


/*N*/ ScDBData::~ScDBData()
/*N*/ {
/*N*/ 	StopRefreshTimer();
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	for (i=0; i<MAXQUERY; i++)
/*N*/ 		delete pQueryStr[i];
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		delete[] pSubTotals[i];
/*N*/ 		delete[] pFunctions[i];
/*N*/ 	}
/*N*/ }

/*N*/  BOOL ScDBData::IsBeyond(USHORT nMaxRow) const
/*N*/  {
/*N*/  	return ( nStartRow > nMaxRow ||
/*N*/  			 nEndRow > nMaxRow ||
/*N*/  			 nQueryDestRow > nMaxRow );
/*N*/  }



/*N*/ void ScDBData::GetArea(USHORT& rTab, USHORT& rCol1, USHORT& rRow1, USHORT& rCol2, USHORT& rRow2) const
/*N*/ {
/*N*/ 	rTab  = nTable;
/*N*/ 	rCol1 = nStartCol;
/*N*/ 	rRow1 = nStartRow;
/*N*/ 	rCol2 = nEndCol;
/*N*/ 	rRow2 = nEndRow;
/*N*/ }

/*N*/ void ScDBData::GetArea(ScRange& rRange) const
/*N*/ {
/*N*/ 	rRange = ScRange( nStartCol,nStartRow,nTable, nEndCol,nEndRow,nTable );
/*N*/ }

/*N*/ void ScDBData::SetArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
/*N*/ {
/*N*/ 	nTable	= nTab;
/*N*/ 	nStartCol = nCol1;
/*N*/ 	nStartRow = nRow1;
/*N*/ 	nEndCol   = nCol2;
/*N*/ 	nEndRow   = nRow2;
/*N*/ }


/*N*/ void ScDBData::GetSortParam( ScSortParam& rSortParam ) const
/*N*/ {
/*N*/ 	rSortParam.nCol1 = nStartCol;
/*N*/ 	rSortParam.nRow1 = nStartRow;
/*N*/ 	rSortParam.nCol2 = nEndCol;
/*N*/ 	rSortParam.nRow2 = nEndRow;
/*N*/ 	rSortParam.bByRow = bByRow;
/*N*/ 	rSortParam.bHasHeader = bHasHeader;
/*N*/ 	rSortParam.bCaseSens = bSortCaseSens;
/*N*/ 	rSortParam.bInplace = bSortInplace;
/*N*/ 	rSortParam.nDestTab = nSortDestTab;
/*N*/ 	rSortParam.nDestCol = nSortDestCol;
/*N*/ 	rSortParam.nDestRow = nSortDestRow;
/*N*/ 	rSortParam.bIncludePattern = bIncludePattern;
/*N*/ 	rSortParam.bUserDef	= bSortUserDef;
/*N*/ 	rSortParam.nUserIndex = nSortUserIndex;
/*N*/ 	for (USHORT i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		rSortParam.bDoSort[i] 	 = bDoSort[i];
/*N*/ 		rSortParam.nField[i] 	 = nSortField[i];
/*N*/ 		rSortParam.bAscending[i] = bAscending[i];
/*N*/ 	}
/*N*/ 	rSortParam.aCollatorLocale = aSortLocale;
/*N*/ 	rSortParam.aCollatorAlgorithm = aSortAlgorithm;
/*N*/ }

/*N*/ void ScDBData::SetSortParam( const ScSortParam& rSortParam )
/*N*/ {
/*N*/ 	bSortCaseSens = rSortParam.bCaseSens;
/*N*/ 	bIncludePattern = rSortParam.bIncludePattern;
/*N*/ 	bSortInplace = rSortParam.bInplace;
/*N*/ 	nSortDestTab = rSortParam.nDestTab;
/*N*/ 	nSortDestCol = rSortParam.nDestCol;
/*N*/ 	nSortDestRow = rSortParam.nDestRow;
/*N*/ 	bSortUserDef = rSortParam.bUserDef;
/*N*/ 	nSortUserIndex = rSortParam.nUserIndex;
/*N*/ 	for (USHORT i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		bDoSort[i]	  = rSortParam.bDoSort[i];
/*N*/ 		nSortField[i] = rSortParam.nField[i];
/*N*/ 		bAscending[i] = rSortParam.bAscending[i];
/*N*/ 	}
/*N*/ 	aSortLocale = rSortParam.aCollatorLocale;
/*N*/ 	aSortAlgorithm = rSortParam.aCollatorAlgorithm;
/*N*/ 
/*N*/ 	//#98317#; set the orientation
/*N*/ 	bByRow = rSortParam.bByRow;
/*N*/ }

/*N*/ void ScDBData::GetQueryParam( ScQueryParam& rQueryParam ) const
/*N*/ {
/*N*/ 	rQueryParam.nCol1 = nStartCol;
/*N*/ 	rQueryParam.nRow1 = nStartRow;
/*N*/ 	rQueryParam.nCol2 = nEndCol;
/*N*/ 	rQueryParam.nRow2 = nEndRow;
/*N*/ 	rQueryParam.nTab  = nTable;
/*N*/ 	rQueryParam.bByRow = bByRow;
/*N*/ 	rQueryParam.bHasHeader = bHasHeader;
/*N*/ 	rQueryParam.bInplace = bQueryInplace;
/*N*/ 	rQueryParam.bCaseSens = bQueryCaseSens;
/*N*/ 	rQueryParam.bRegExp = bQueryRegExp;
/*N*/ 	rQueryParam.bDuplicate = bQueryDuplicate;
/*N*/ 	rQueryParam.nDestTab = nQueryDestTab;
/*N*/ 	rQueryParam.nDestCol = nQueryDestCol;
/*N*/ 	rQueryParam.nDestRow = nQueryDestRow;
/*N*/ 
/*N*/ 	rQueryParam.Resize( MAXQUERY );
/*N*/ 	for (USHORT i=0; i<MAXQUERY; i++)
/*N*/ 	{
/*N*/ 		ScQueryEntry& rEntry = rQueryParam.GetEntry(i);
/*N*/ 
/*N*/ 		rEntry.bDoQuery = bDoQuery[i];
/*N*/ 		rEntry.nField = nQueryField[i];
/*N*/ 		rEntry.eOp = eQueryOp[i];
/*N*/ 		rEntry.bQueryByString = bQueryByString[i];
/*N*/ 		*rEntry.pStr = *pQueryStr[i];
/*N*/ 		rEntry.nVal = nQueryVal[i];
/*N*/ 		rEntry.eConnect = eQueryConnect[i];
/*N*/ 	}
/*N*/ }

/*N*/ void ScDBData::SetQueryParam(const ScQueryParam& rQueryParam)
/*N*/ {
/*N*/ 	DBG_ASSERT( rQueryParam.GetEntryCount() <= MAXQUERY ||
/*N*/ 				!rQueryParam.GetEntry(MAXQUERY).bDoQuery,
/*N*/ 				"zuviele Eintraege bei ScDBData::SetQueryParam" );
/*N*/ 
/*N*/ 	//	set bIsAdvanced to FALSE for everything that is not from the
/*N*/ 	//	advanced filter dialog
/*N*/ 	bIsAdvanced = FALSE;
/*N*/ 
/*N*/ 	bQueryInplace = rQueryParam.bInplace;
/*N*/ 	bQueryCaseSens = rQueryParam.bCaseSens;
/*N*/ 	bQueryRegExp = rQueryParam.bRegExp;
/*N*/ 	bQueryDuplicate = rQueryParam.bDuplicate;
/*N*/ 	nQueryDestTab = rQueryParam.nDestTab;
/*N*/ 	nQueryDestCol = rQueryParam.nDestCol;
/*N*/ 	nQueryDestRow = rQueryParam.nDestRow;
/*N*/ 	for (USHORT i=0; i<MAXQUERY; i++)
/*N*/ 	{
/*N*/ 		ScQueryEntry& rEntry = rQueryParam.GetEntry(i);
/*N*/ 
/*N*/ 		bDoQuery[i] = rEntry.bDoQuery;
/*N*/ 		nQueryField[i] = rEntry.nField;
/*N*/ 		eQueryOp[i] = rEntry.eOp;
/*N*/ 		bQueryByString[i] = rEntry.bQueryByString;
/*N*/ 		*pQueryStr[i] = *rEntry.pStr;
/*N*/ 		nQueryVal[i] = rEntry.nVal;
/*N*/ 		eQueryConnect[i] = rEntry.eConnect;
/*N*/ 	}
/*N*/ }

/*N*/ void ScDBData::SetAdvancedQuerySource(const ScRange* pSource)
/*N*/ {
/*N*/ 	if (pSource)
/*N*/ 	{
/*N*/ 		aAdvSource = *pSource;
/*N*/ 		bIsAdvanced = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bIsAdvanced = FALSE;
/*N*/ }

/*N*/ BOOL ScDBData::GetAdvancedQuerySource(ScRange& rSource) const
/*N*/ {
/*N*/ 	rSource = aAdvSource;
/*N*/ 	return bIsAdvanced;
/*N*/ }

/*N*/ void ScDBData::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	USHORT j;
/*N*/ 
/*N*/ 	rSubTotalParam.nCol1 = nStartCol;
/*N*/ 	rSubTotalParam.nRow1 = nStartRow;
/*N*/ 	rSubTotalParam.nCol2 = nEndCol;
/*N*/ 	rSubTotalParam.nRow2 = nEndRow;
/*N*/ 
/*N*/ 	rSubTotalParam.bRemoveOnly		= bSubRemoveOnly;
/*N*/ 	rSubTotalParam.bReplace			= bSubReplace;
/*N*/ 	rSubTotalParam.bPagebreak		= bSubPagebreak;
/*N*/ 	rSubTotalParam.bCaseSens		= bSubCaseSens;
/*N*/ 	rSubTotalParam.bDoSort			= bSubDoSort;
/*N*/ 	rSubTotalParam.bAscending		= bSubAscending;
/*N*/ 	rSubTotalParam.bIncludePattern	= bSubIncludePattern;
/*N*/ 	rSubTotalParam.bUserDef			= bSubUserDef;
/*N*/ 	rSubTotalParam.nUserIndex		= nSubUserIndex;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		rSubTotalParam.bGroupActive[i]	= bDoSubTotal[i];
/*N*/ 		rSubTotalParam.nField[i]		= nSubField[i];
/*N*/ 		USHORT nCount = nSubTotals[i];
/*N*/ 
/*N*/ 		rSubTotalParam.nSubTotals[i] = nCount;
/*N*/ 		delete[] rSubTotalParam.pSubTotals[i];
/*N*/ 		delete[] rSubTotalParam.pFunctions[i];
/*N*/ 		rSubTotalParam.pSubTotals[i] = nCount ? new USHORT[nCount] : NULL;
/*N*/ 		rSubTotalParam.pFunctions[i] = nCount ? new ScSubTotalFunc[nCount]
/*N*/ 											  : NULL;
/*N*/ 		for (j=0; j<nCount; j++)
/*N*/ 		{
/*N*/ 			rSubTotalParam.pSubTotals[i][j] = pSubTotals[i][j];
/*N*/ 			rSubTotalParam.pFunctions[i][j] = pFunctions[i][j];
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDBData::SetSubTotalParam(const ScSubTotalParam& rSubTotalParam)
/*N*/ {
/*N*/ 	USHORT i;
/*N*/ 	USHORT j;
/*N*/ 
/*N*/ 	bSubRemoveOnly		= rSubTotalParam.bRemoveOnly;
/*N*/ 	bSubReplace			= rSubTotalParam.bReplace;
/*N*/ 	bSubPagebreak		= rSubTotalParam.bPagebreak;
/*N*/ 	bSubCaseSens		= rSubTotalParam.bCaseSens;
/*N*/ 	bSubDoSort			= rSubTotalParam.bDoSort;
/*N*/ 	bSubAscending		= rSubTotalParam.bAscending;
/*N*/ 	bSubIncludePattern	= rSubTotalParam.bIncludePattern;
/*N*/ 	bSubUserDef			= rSubTotalParam.bUserDef;
/*N*/ 	nSubUserIndex		= rSubTotalParam.nUserIndex;
/*N*/ 
/*N*/ 	for (i=0; i<MAXSUBTOTAL; i++)
/*N*/ 	{
/*N*/ 		bDoSubTotal[i]	= rSubTotalParam.bGroupActive[i];
/*N*/ 		nSubField[i]	= rSubTotalParam.nField[i];
/*N*/ 		USHORT nCount = rSubTotalParam.nSubTotals[i];
/*N*/ 
/*N*/ 		nSubTotals[i] = nCount;
/*N*/ 		delete[] pSubTotals[i];
/*N*/ 		delete[] pFunctions[i];
/*N*/ 		pSubTotals[i] = nCount ? new USHORT			[nCount] : NULL;
/*N*/ 		pFunctions[i] = nCount ? new ScSubTotalFunc [nCount] : NULL;
/*N*/ 		for (j=0; j<nCount; j++)
/*N*/ 		{
/*N*/ 			pSubTotals[i][j] = rSubTotalParam.pSubTotals[i][j];
/*N*/ 			pFunctions[i][j] = rSubTotalParam.pFunctions[i][j];
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDBData::GetImportParam(ScImportParam& rImportParam) const
/*N*/ {
/*N*/ 	rImportParam.nCol1 = nStartCol;
/*N*/ 	rImportParam.nRow1 = nStartRow;
/*N*/ 	rImportParam.nCol2 = nEndCol;
/*N*/ 	rImportParam.nRow2 = nEndRow;
/*N*/ 
/*N*/ 	rImportParam.bImport	= bDBImport;
/*N*/ 	rImportParam.aDBName	= aDBName;
/*N*/ 	rImportParam.aStatement	= aDBStatement;
/*N*/ 	rImportParam.bNative	= bDBNative;
/*N*/ 	rImportParam.bSql		= bDBSql;
/*N*/ 	rImportParam.nType		= nDBType;
/*N*/ }

/*N*/ void ScDBData::SetImportParam(const ScImportParam& rImportParam)
/*N*/ {
/*N*/ 	bDBImport		= rImportParam.bImport;
/*N*/ 	aDBName			= rImportParam.aDBName;
/*N*/ 	aDBStatement	= rImportParam.aStatement;
/*N*/ 	bDBNative		= rImportParam.bNative;
/*N*/ 	bDBSql			= rImportParam.bSql;
/*N*/ 	nDBType			= rImportParam.nType;
/*N*/ }




/*N*/  DataObject*	ScDBData::Clone() const
/*N*/  {
/*N*/  	return new ScDBData(*this);
/*N*/  }


//---------------------------------------------------------------------------------------
//	Compare zum Sortieren

/*N*/ short ScDBCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
/*N*/ {
/*N*/ 	const String& rStr1 = ((ScDBData*)pKey1)->GetName();
/*N*/ 	const String& rStr2 = ((ScDBData*)pKey2)->GetName();
/*N*/     return (short) ScGlobal::pTransliteration->compareString( rStr1, rStr2 );
/*N*/ }

//	IsEqual - alles gleich


/*N*/ ScDBData* ScDBCollection::GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, BOOL bStartOnly) const
/*N*/ {
/*N*/ 	ScDBData* pNoNameData = NULL;
/*N*/ 	if (pItems)
/*N*/ 	{
/*N*/ 		const String& rNoName = ScGlobal::GetRscString( STR_DB_NONAME );
/*N*/ 
/*N*/ 		for (USHORT i = 0; i < nCount; i++)
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if (((ScDBData*)pItems[i])->IsDBAtCursor(nCol, nRow, nTab, bStartOnly))
/*N*/ 	}
/*N*/ 	return pNoNameData;				// "unbenannt" nur zurueck, wenn sonst nichts gefunden
/*N*/ }

/*N*/ ScDBData* ScDBCollection::GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const
/*N*/ {
/*N*/ 	ScDBData* pNoNameData = NULL;
/*N*/ 	if (pItems)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const String& rNoName = ScGlobal::GetRscString( STR_DB_NONAME );
/*N*/ 	}
/*N*/ 	return pNoNameData;				// "unbenannt" nur zurueck, wenn sonst nichts gefunden
/*N*/ }

/*N*/ BOOL ScDBCollection::SearchName( const String& rName, USHORT& rIndex ) const
/*N*/ {
/*N*/ 	ScDBData aDataObj( rName, 0,0,0,0,0 );
/*N*/ 	return Search( &aDataObj, rIndex );
/*N*/ }

/*N*/ BOOL ScDBCollection::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	BOOL bSuccess = TRUE;
/*N*/ 	USHORT nNewCount;
/*N*/ 
/*N*/ 	while( nCount > 0 )
/*?*/ 		AtFree(0);					// alles loeschen
/*N*/ 
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream >> nNewCount;
/*N*/ 	for (USHORT i=0; i<nNewCount && bSuccess; i++)
/*N*/ 	{
/*N*/ 		ScDBData* pData = new ScDBData( rStream, aHdr );
/*N*/ 		Insert( pData );
/*N*/ 	}
/*N*/ 	if (aHdr.BytesLeft())	//	... Erweiterungen
/*N*/ 		rStream >> nEntryIndex;
/*N*/ 	return bSuccess;
/*N*/ }

/*N*/ BOOL ScDBCollection::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	USHORT nSaveCount = nCount;
/*N*/ 	USHORT nSaveMaxRow = pDoc->GetSrcMaxRow();
/*N*/ 	if ( nSaveMaxRow < MAXROW )
/*N*/ 	{
/*N*/ 		nSaveCount = 0;
/*N*/ 		for (i=0; i<nCount; i++)
/*N*/ 			if ( !((const ScDBData*)At(i))->IsBeyond(nSaveMaxRow) )
/*N*/ 				++nSaveCount;
/*N*/ 
/*N*/ 		if ( nSaveCount < nCount )
/*N*/ 			pDoc->SetLostData();			// Warnung ausgeben
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream << nSaveCount;
/*N*/ 
/*N*/ 	BOOL bSuccess = TRUE;
/*N*/ 	for (i=0; i<nCount && bSuccess; i++)
/*N*/ 	{
/*N*/ 		const ScDBData* pDBData = (const ScDBData*)At(i);
/*N*/ 		if ( nSaveMaxRow == MAXROW || !pDBData->IsBeyond(nSaveMaxRow) )
/*N*/ 			bSuccess = pDBData->Store( rStream, aHdr );
/*N*/ 	}
/*N*/ 
/*N*/ 	rStream << nEntryIndex;				// seit 24.10.95
/*N*/ 
/*N*/ 	return bSuccess;
/*N*/ }

/*N*/ void ScDBCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
/*N*/ 								USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 								USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 								short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT theCol1;
/*N*/ 	}
/*N*/ }




/*N*/ ScDBData* ScDBCollection::FindIndex(USHORT nIndex)
/*N*/ {
/*N*/ 	USHORT i = 0;
/*N*/ 	while (i < nCount)
/*N*/ 	{
/*N*/ 		if ((*this)[i]->GetIndex() == nIndex)
/*N*/ 			return (*this)[i];
/*N*/ 		i++;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ BOOL ScDBCollection::Insert(DataObject* pDataObject)
/*N*/ {
/*N*/ 	ScDBData* pData = (ScDBData*) pDataObject;
/*N*/ 	if (!pData->GetIndex())		// schon gesetzt?
/*N*/ 		pData->SetIndex(nEntryIndex++);
/*N*/ 	BOOL bInserted = SortedCollection::Insert(pDataObject);
/*N*/ 	if ( bInserted && pData->HasImportParam() && !pData->HasImportSelection() )
/*N*/ 	{
/*?*/ 		pData->SetRefreshHandler( GetRefreshHandler() );
/*?*/ 		pData->SetRefreshControl( pDoc->GetRefreshTimerControlAddress() );
/*N*/ 	}
/*N*/ 	return bInserted;
/*N*/ }




}
