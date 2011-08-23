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

// System - Includes -----------------------------------------------------

class StarBASIC;

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef PCH
#include "bf_sc.hrc"
#define GLOBALOVERFLOW
#endif

// INCLUDE ---------------------------------------------------------------

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <i18npool/mslangid.hxx>
#include <bf_svtools/zforlist.hxx>
#define _SVSTDARR_ULONGS
#include <sot/formats.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>

#include "docsh.hxx"
#include "asciiopt.hxx"
#include "cell.hxx"
#include "progress.hxx"

#include "impex.hxx"

#include "globstr.hrc"


#include <osl/endian.h>
namespace binfilter {

//========================================================================


// Gesamtdokument ohne Undo


/*N*/ ScImportExport::ScImportExport( ScDocument* p )
/*N*/ 	: pDoc( p ), pDocSh( PTR_CAST(ScDocShell,p->GetDocumentShell()) ),
/*N*/ 	  nSizeLimit( 0 ), bSingle( TRUE ), bAll( TRUE ), bUndo( FALSE ),
/*N*/ 	  cSep( '\t' ), cStr( '"' ), bFormulas( FALSE ), bIncludeFiltered( TRUE ),
/*N*/ 	  bOverflow( FALSE )
/*N*/ {
/*N*/ 	pUndoDoc = NULL;
/*N*/ 	pExtOptions = NULL;
/*N*/ }

// Insert am Punkt ohne Bereichschecks




//	ctor with a range is only used for export
//!	ctor with a string (and bSingle=TRUE) is also used for DdeSetData


// String auswerten: Entweder Bereich, Punkt oder Gesamtdoc (bei Fehler)
// Falls eine View existiert, wird die TabNo der View entnommen!




/*N*/ ScImportExport::~ScImportExport()
/*N*/ {
/*N*/ 	delete pUndoDoc;
/*N*/ 	delete pExtOptions;
/*N*/ }


/*N*/ void ScImportExport::SetExtOptions( const ScAsciiOptions& rOpt )
/*N*/ {
/*N*/ 	if ( pExtOptions )
/*N*/ 		*pExtOptions = rOpt;
/*N*/ 	else
/*N*/ 		pExtOptions = new ScAsciiOptions( rOpt );
/*N*/ 
/*N*/ 	//	"normale" Optionen uebernehmen
/*N*/ 
/*N*/ 	cSep = rOpt.GetFieldSeps().GetChar(0);
/*N*/ 	cStr = rOpt.GetTextSep();
/*N*/ }




//////////////////////////////////////////////////////////////////////////////

// Vorbereitung fuer Undo: Undo-Dokument erzeugen



// Nachbereitung Insert: Undo/Redo-Aktionen erzeugen, Invalidate/Repaint



/////////////////////////////////////////////////////////////////////////////






// static








/*N*/ BOOL ScImportExport::ImportStream( SvStream& rStrm, ULONG nFmt )
/*N*/ {
/*N*/ 	if( nFmt == FORMAT_STRING )
/*N*/ 	{
/*N*/ 		if( ExtText2Doc( rStrm ) )		// pExtOptions auswerten
/*N*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if( nFmt == SOT_FORMATSTR_ID_SYLK )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( Sylk2Doc( rStrm ) )
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if( nFmt == SOT_FORMATSTR_ID_DIF )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( Dif2Doc( rStrm ) )
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if( nFmt == FORMAT_RTF )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( RTF2Doc( rStrm ) )
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if( nFmt == SOT_FORMATSTR_ID_LINK )
/*N*/ 		return TRUE;			// Link-Import?
/*N*/ 	if ( nFmt == SOT_FORMATSTR_ID_HTML )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( HTML2Doc( rStrm ) )
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if ( nFmt == SOT_FORMATSTR_ID_HTML_SIMPLE )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 MSE40HTMLClipFormatObj aMSE40ClpObj;				// needed to skip the header data
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }




//static


// static


enum DoubledQuoteMode
{
    DQM_KEEP,		// both are taken
    DQM_ESCAPE,		// escaped quote, one is taken, one ignored
    DQM_CONCAT,		// first is end, next is start, both ignored => strings combined
    DQM_SEPARATE	// end one string and begin next
};

/*N*/ const sal_Unicode* lcl_ScanString( const sal_Unicode* p, String& rString,
/*N*/ 			sal_Unicode cStr, DoubledQuoteMode eMode )
/*N*/ {
/*N*/ 	p++;	//! jump over opening quote
/*N*/ 	BOOL bCont;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		bCont = FALSE;
/*N*/ 		const sal_Unicode* p0 = p;
/*N*/ 		for( ;; )
/*N*/ 		{
/*N*/ 			if( !*p )
/*N*/ 				break;
/*N*/ 			if( *p == cStr )
/*N*/ 			{
/*N*/ 				if ( *++p != cStr )
/*N*/ 					break;
/*N*/ 				// doubled quote char
/*N*/ 				switch ( eMode )
/*N*/ 				{
/*N*/ 					case DQM_KEEP :
/*N*/ 						p++;			// both for us (not breaking for-loop)
/*N*/ 					break;
/*N*/ 					case DQM_ESCAPE :
/*N*/ 						p++;			// one for us (breaking for-loop)
/*N*/ 						bCont = TRUE;	// and more
/*N*/ 					break;
/*N*/ 					case DQM_CONCAT :
/*N*/ 						if ( p0+1 < p )
/*N*/ 							rString.Append( p0, (p-1) - p0 );	// first part
/*N*/ 						p0 = ++p;		// text of next part starts here
/*N*/ 					break;
/*N*/ 					case DQM_SEPARATE :
/*N*/ 										// positioned on next opening quote
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				if ( eMode == DQM_ESCAPE || eMode == DQM_SEPARATE )
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				p++;
/*N*/ 		}
/*N*/ 		if ( p0 < p )
/*N*/ 			rString.Append( p0, ((*p || *(p-1) == cStr) ? p-1 : p) - p0 );
/*N*/ 	} while ( bCont );
/*N*/ 	return p;
/*N*/ }




//////////////////////////////////////////////////////////////////////////////



        //
        //	erweiterter Ascii-Import
        //


/*M*/ void lcl_PutString( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
/*M*/ 					const String& rStr, BYTE nColFormat,
/*M*/                     ::utl::TransliterationWrapper& rTransliteration,
/*M*/ 					CalendarWrapper& rCalendar,
/*M*/                     ::utl::TransliterationWrapper* pSecondTransliteration,
/*M*/ 					CalendarWrapper* pSecondCalendar )
/*M*/ {
/*M*/ 	if ( nColFormat == SC_COL_SKIP || !rStr.Len() || nCol > MAXCOL || nRow > MAXROW )
/*M*/ 		return;
/*M*/ 
/*M*/ 	if ( nColFormat == SC_COL_TEXT )
/*M*/ 	{
/*M*/ 		pDoc->PutCell( nCol, nRow, nTab, new ScStringCell( rStr ) );
/*M*/ 		return;
/*M*/ 	}
/*M*/ 
/*M*/ 	if ( nColFormat == SC_COL_ENGLISH )
/*M*/ 	{
/*M*/ 		//!	SetString mit Extra-Flag ???
/*M*/ 
/*M*/ 		SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
/*M*/ 		sal_uInt32 nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
/*M*/ 		double fVal;
/*M*/ 		if ( pFormatter->IsNumberFormat( rStr, nEnglish, fVal ) )
/*M*/ 		{
/*M*/ 			//	Zahlformat wird nicht auf englisch gesetzt
/*M*/ 			pDoc->SetValue( nCol, nRow, nTab, fVal );
/*M*/ 			return;
/*M*/ 		}
/*M*/ 		//	sonst weiter mit SetString
/*M*/ 	}
/*M*/ 	else if ( nColFormat != SC_COL_STANDARD )					// Datumsformate
/*M*/ 	{
/*M*/ 		//	nach genau drei Teilen suchen
/*M*/ 
/*M*/ 		xub_StrLen nLen = rStr.Len();
/*M*/ 		xub_StrLen nStart[3];
/*M*/ 		xub_StrLen nEnd[3];
/*M*/ 		USHORT nFound = 0;
/*M*/ 		BOOL bInNum = FALSE;
/*M*/ 		for ( xub_StrLen nPos=0; nPos<nLen; nPos++ )
/*M*/ 		{
/*M*/ 			if ( ScGlobal::pCharClass->isLetterNumeric( rStr, nPos ) )
/*M*/ 			{
/*M*/ 				if (!bInNum)
/*M*/ 				{
/*M*/ 					if ( nFound >= 3 )
/*M*/ 						break;					// zuviele Teile
/*M*/ 					bInNum = TRUE;
/*M*/ 					nStart[nFound] = nPos;
/*M*/ 					++nFound;
/*M*/ 				}
/*M*/ 				nEnd[nFound-1] = nPos;
/*M*/ 			}
/*M*/ 			else
/*M*/ 				bInNum = FALSE;
/*M*/ 		}
/*M*/ 
/*M*/ 		USHORT nDP, nMP, nYP;
/*M*/ 		switch ( nColFormat )
/*M*/ 		{
/*M*/ 			case SC_COL_YMD: nDP = 2; nMP = 1; nYP = 0; break;
/*M*/ 			case SC_COL_MDY: nDP = 1; nMP = 0; nYP = 2; break;
/*M*/ 			case SC_COL_DMY:
/*M*/ 			default:		 nDP = 0; nMP = 1; nYP = 2; break;
/*M*/ 		}
/*M*/ 
/*M*/ 		if ( nFound == 1 )
/*M*/ 		{
/*M*/ 			//	try to break one number (without separators) into date fields
/*M*/ 
/*M*/ 			xub_StrLen nDateStart = nStart[0];
/*M*/ 			xub_StrLen nDateLen = nEnd[0] + 1 - nDateStart;
/*M*/ 
/*M*/ 			if ( nDateLen >= 5 && nDateLen <= 8 &&
/*M*/ 					ScGlobal::pCharClass->isNumeric( rStr.Copy( nDateStart, nDateLen ) ) )
/*M*/ 			{
/*M*/ 				//	6 digits: 2 each for day, month, year
/*M*/ 				//	8 digits: 4 for year, 2 each for day and month
/*M*/ 				//	5 or 7 digits: first field is shortened by 1
/*M*/ 
/*M*/ 				BOOL bLongYear = ( nDateLen >= 7 );
/*M*/ 				BOOL bShortFirst = ( nDateLen == 5 || nDateLen == 7 );
/*M*/ 
/*M*/ 				USHORT nFieldStart = nDateStart;
/*M*/ 				for (USHORT nPos=0; nPos<3; nPos++)
/*M*/ 				{
/*M*/ 					USHORT nFieldEnd = nFieldStart + 1;		// default: 2 digits
/*M*/ 					if ( bLongYear && nPos == nYP )
/*M*/ 						nFieldEnd += 2;						// 2 extra digits for long year
/*M*/ 					if ( bShortFirst && nPos == 0 )
/*M*/ 						--nFieldEnd;						// first field shortened?
/*M*/ 
/*M*/ 					nStart[nPos] = nFieldStart;
/*M*/ 					nEnd[nPos]   = nFieldEnd;
/*M*/ 					nFieldStart  = nFieldEnd + 1;
/*M*/ 				}
/*M*/ 				nFound = 3;
/*M*/ 			}
/*M*/ 		}
/*M*/ 
/*M*/ 		if ( nFound == 3 )				// exactly 3 parts found?
/*M*/ 		{
/*M*/ 			using namespace ::com::sun::star;
/*M*/ 			BOOL bSecondCal = FALSE;
/*M*/ 			USHORT nDay  = (USHORT) rStr.Copy( nStart[nDP], nEnd[nDP]+1-nStart[nDP] ).ToInt32();
/*M*/ 			USHORT nYear = (USHORT) rStr.Copy( nStart[nYP], nEnd[nYP]+1-nStart[nYP] ).ToInt32();
/*M*/ 			String aMStr = rStr.Copy( nStart[nMP], nEnd[nMP]+1-nStart[nMP] );
/*M*/ 			sal_Int16 nMonth = (sal_Int16) aMStr.ToInt32();
/*M*/ 			if (!nMonth)
/*M*/ 			{
/*M*/                 static const String aSeptCorrect( RTL_CONSTASCII_USTRINGPARAM( "SEPT" ) );
/*M*/                 static const String aSepShortened( RTL_CONSTASCII_USTRINGPARAM( "SEP" ) );
/*M*/ 				uno::Sequence< i18n::CalendarItem > xMonths;
/*M*/ 				sal_Int32 i, nLen;
/*M*/ 				//	first test all month names from local international
/*M*/ 				xMonths = rCalendar.getMonths();
/*M*/ 				nLen = xMonths.getLength();
/*M*/ 				for (i=0; i<nLen && !nMonth; i++)
/*M*/ 				{
/*M*/                     if ( rTransliteration.isEqual( aMStr, xMonths[i].FullName ) ||
/*M*/                          rTransliteration.isEqual( aMStr, xMonths[i].AbbrevName ) )
/*M*/ 						nMonth = i+1;
/*M*/                     else if ( i == 8 && rTransliteration.isEqual( aSeptCorrect,
/*M*/                                 xMonths[i].AbbrevName ) &&
/*M*/                             rTransliteration.isEqual( aMStr, aSepShortened ) )
/*M*/                     {   // #102136# correct English abbreviation is SEPT,
/*M*/                         // but data mostly contains SEP only
/*M*/                         nMonth = i+1;
/*M*/                     }
/*M*/ 				}
/*M*/ 				//	if none found, then test english month names
/*M*/                 if ( !nMonth && pSecondCalendar && pSecondTransliteration )
/*M*/ 				{
/*M*/ 					xMonths = pSecondCalendar->getMonths();
/*M*/ 					nLen = xMonths.getLength();
/*M*/ 					for (i=0; i<nLen && !nMonth; i++)
/*M*/ 					{
/*M*/                         if ( pSecondTransliteration->isEqual( aMStr, xMonths[i].FullName ) ||
/*M*/                              pSecondTransliteration->isEqual( aMStr, xMonths[i].AbbrevName ) )
/*M*/ 						{
/*M*/ 							nMonth = i+1;
/*M*/ 							bSecondCal = TRUE;
/*M*/ 						}
/*M*/                         else if ( i == 8 && pSecondTransliteration->isEqual(
/*M*/                                     aMStr, aSepShortened ) )
/*M*/                         {   // #102136# correct English abbreviation is SEPT,
/*M*/                             // but data mostly contains SEP only
/*M*/ 							nMonth = i+1;
/*M*/ 							bSecondCal = TRUE;
/*M*/                         }
/*M*/ 					}
/*M*/ 				}
/*M*/ 			}
/*M*/ 
/*M*/ 			SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
/*M*/ 			if ( nYear < 100 )
/*M*/ 				nYear = pFormatter->ExpandTwoDigitYear( nYear );
/*M*/ 
/*M*/ 			CalendarWrapper* pCalendar = (bSecondCal ? pSecondCalendar : &rCalendar);
/*M*/ 			sal_Int16 nNumMonths = pCalendar->getNumberOfMonthsInYear();
/*M*/ 			if ( nDay && nMonth && nDay<=31 && nMonth<=nNumMonths )
/*M*/ 			{
/*M*/ 				--nMonth;
/*M*/ 				pCalendar->setValue( i18n::CalendarFieldIndex::DAY_OF_MONTH, nDay );
/*M*/ 				pCalendar->setValue( i18n::CalendarFieldIndex::MONTH, nMonth );
/*M*/ 				pCalendar->setValue( i18n::CalendarFieldIndex::YEAR, nYear );
/*N*/                 // #i14974# The imported value should have no fractional value, so set the
/*N*/                 // time fields to zero (ICU calendar instance defaults to current date/time)
/*N*/                 pCalendar->setValue( i18n::CalendarFieldIndex::HOUR, 0 );
/*N*/                 pCalendar->setValue( i18n::CalendarFieldIndex::MINUTE, 0 );
/*N*/                 pCalendar->setValue( i18n::CalendarFieldIndex::SECOND, 0 );
/*N*/                 pCalendar->setValue( i18n::CalendarFieldIndex::MILLISECOND, 0 );
/*M*/                 if ( pCalendar->isValid() )
/*M*/                 {
/*M*/                     double fDiff = DateTime(*pFormatter->GetNullDate()) -
/*M*/                         pCalendar->getEpochStart();
/*N*/                     // #i14974# must use getLocalDateTime to get the same
/*N*/                     // date values as set above
/*N*/                     double fDays = pCalendar->getLocalDateTime();
/*M*/                     fDays -= fDiff;
/*M*/ 
/*M*/                     LanguageType eLatin, eCjk, eCtl;
/*M*/                     pDoc->GetLanguage( eLatin, eCjk, eCtl );
/*M*/                     LanguageType eDocLang = eLatin;     //! which language for date formats?
/*M*/ 
/*M*/                     long nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, eDocLang );
/*M*/ 
/*M*/                     pDoc->PutCell( nCol, nRow, nTab, new ScValueCell(fDays), nFormat, FALSE );
/*M*/ 
/*M*/                     return;     // success
/*M*/                 }
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	//	Standard oder Datum nicht erkannt -> SetString
/*M*/ 
/*M*/ 	pDoc->SetString( nCol, nRow, nTab, rStr );
/*M*/ }




/*N*/ BOOL ScImportExport::ExtText2Doc( SvStream& rStrm )
/*N*/ {
/*N*/ 	if (!pExtOptions)
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 return Text2Doc( rStrm );
/*N*/ 
/*N*/ 	ULONG nOldPos = rStrm.Tell();
/*N*/ 	rStrm.Seek( STREAM_SEEK_TO_END );
/*N*/ 	ScProgress aProgress( pDocSh, ScGlobal::GetRscString( STR_LOAD_DOC ), rStrm.Tell() - nOldPos );
/*N*/ 	rStrm.Seek( nOldPos );
/*N*/ 	if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
/*?*/ 		rStrm.StartReadingUnicodeText();
/*N*/ 
/*N*/ 	BOOL bOld = ScColumn::bDoubleAlloc;
/*N*/ 	ScColumn::bDoubleAlloc = TRUE;
/*N*/ 
/*N*/ 	DBG_ASSERT( !bUndo, "ExtText2Doc mit Undo noch nicht implementiert!" );
/*N*/ 	USHORT nStartCol = aRange.aStart.Col();
/*N*/ 	USHORT nStartRow = aRange.aStart.Row();
/*N*/ 	USHORT nTab = aRange.aStart.Tab();
/*N*/ 
/*N*/ 	BOOL	bFixed			= pExtOptions->IsFixedLen();
/*N*/ 	const sal_Unicode* pSeps = pExtOptions->GetFieldSeps().GetBuffer();
/*N*/ 	BOOL	bMerge			= pExtOptions->IsMergeSeps();
/*N*/ 	USHORT	nInfoCount		= pExtOptions->GetInfoCount();
/*N*/ 	const xub_StrLen* pColStart = pExtOptions->GetColStart();
/*N*/ 	const BYTE* pColFormat  = pExtOptions->GetColFormat();
/*N*/ 	long nSkipLines	= pExtOptions->GetStartRow();
/*N*/ 
/*N*/ 	LanguageType eLatin, eCjk, eCtl;
/*N*/ 	pDoc->GetLanguage( eLatin, eCjk, eCtl );
/*N*/ 	LanguageType eDocLang = eLatin;					//! which language for date formats?
/*N*/ 
/*N*/ 	// For date recognition
/*N*/     ::utl::TransliterationWrapper aTransliteration(
/*N*/         pDoc->GetServiceManager(), SC_TRANSLITERATION_IGNORECASE );
/*N*/     aTransliteration.loadModuleIfNeeded( eDocLang );
/*N*/ 	CalendarWrapper aCalendar( pDoc->GetServiceManager() );
/*N*/ 	aCalendar.loadDefaultCalendar(
/*N*/ 		MsLangId::convertLanguageToLocale( eDocLang ) );
/*N*/     ::utl::TransliterationWrapper* pEnglishTransliteration = NULL;
/*N*/ 	CalendarWrapper* pEnglishCalendar = NULL;
/*N*/ 	if ( eDocLang != LANGUAGE_ENGLISH_US )
/*N*/ 	{
/*?*/         pEnglishTransliteration = new ::utl::TransliterationWrapper (
/*?*/             pDoc->GetServiceManager(), SC_TRANSLITERATION_IGNORECASE );
/*?*/         aTransliteration.loadModuleIfNeeded( LANGUAGE_ENGLISH_US );
/*?*/ 		pEnglishCalendar = new CalendarWrapper ( pDoc->GetServiceManager() );
/*?*/ 		pEnglishCalendar->loadDefaultCalendar(
/*?*/ 			MsLangId::convertLanguageToLocale( LANGUAGE_ENGLISH_US ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	String aLine, aCell;
/*N*/ 	USHORT i;
/*N*/ 	USHORT nRow = nStartRow;
/*N*/ 
/*N*/ 	while(--nSkipLines>0)
/*N*/ 	{
/*?*/ 		rStrm.ReadUniOrByteStringLine( aLine );		// content is ignored
/*?*/ 		if ( rStrm.IsEof() )
/*?*/ 			break;
/*N*/ 	}
/*N*/ 	for( ;; )
/*N*/ 	{
/*N*/ 		rStrm.ReadUniOrByteStringLine( aLine );
/*N*/ 		if ( rStrm.IsEof() )
/*N*/ 			break;
/*N*/ 
/*N*/ 		xub_StrLen nLineLen = aLine.Len();
/*N*/ 		USHORT nCol = nStartCol;
/*N*/ 		if ( bFixed )				//	Feste Satzlaenge
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 for ( i=0; i<nInfoCount; i++ )
/*N*/ 		}
/*N*/ 		else						//	Nach Trennzeichen suchen
/*N*/ 		{
/*N*/ 			USHORT nSourceCol = 0;
/*N*/ 			USHORT nInfoStart = 0;
/*N*/ 			const sal_Unicode* p = aLine.GetBuffer();
/*N*/ 			while (*p)
/*N*/ 			{
/*N*/ 				p = ScImportExport::ScanNextFieldFromString( p, aCell, cStr, pSeps, bMerge );
/*N*/ 
/*N*/ 				BYTE nFmt = SC_COL_STANDARD;
/*N*/ 				for ( i=nInfoStart; i<nInfoCount; i++ )
/*N*/ 				{
/*N*/ 					if ( pColStart[i] == nSourceCol + 1 )		// pColStart ist 1-basiert
/*N*/ 					{
/*N*/ 						nFmt = pColFormat[i];
/*N*/ 						nInfoStart = i + 1;		// ColInfos sind in Reihenfolge
/*N*/ 						break;	// for
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if ( nFmt != SC_COL_SKIP )
/*N*/ 				{
/*N*/ 					lcl_PutString( pDoc, nCol, nRow, nTab, aCell, nFmt,
/*N*/                         aTransliteration, aCalendar, pEnglishTransliteration, pEnglishCalendar );
/*N*/ 					++nCol;
/*N*/ 				}
/*N*/ 
/*N*/ 				++nSourceCol;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		aProgress.SetStateOnPercent( rStrm.Tell() - nOldPos );
/*N*/ 		++nRow;
/*N*/ 		if ( nRow > MAXROW )
/*N*/ 		{
/*N*/ 			bOverflow = TRUE;			// beim Import Warnung ausgeben
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ScColumn::bDoubleAlloc = bOld;
/*N*/ 	pDoc->DoColResize( nTab, 0, MAXCOL, 0 );
/*N*/ 
/*N*/     delete pEnglishTransliteration;
/*N*/ 	delete pEnglishCalendar;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }


// static
/*N*/ const sal_Unicode* ScImportExport::ScanNextFieldFromString( const sal_Unicode* p,
/*N*/ 		String& rField, sal_Unicode cStr, const sal_Unicode* pSeps, BOOL bMergeSeps )
/*N*/ {
/*N*/ 	rField.Erase();
/*N*/ 	if ( *p == cStr )			// String in Anfuehrungszeichen
/*N*/ 	{
/*N*/ 		p = lcl_ScanString( p, rField, cStr, DQM_ESCAPE );
/*N*/ 		while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
/*N*/ 			p++;
/*N*/ 		if( *p )
/*N*/ 			p++;
/*N*/ 	}
/*N*/ 	else						// bis zum Trennzeichen
/*N*/ 	{
/*N*/ 		const sal_Unicode* p0 = p;
/*N*/ 		while ( *p && !ScGlobal::UnicodeStrChr( pSeps, *p ) )
/*N*/ 			p++;
/*N*/ 		rField.Append( p0, p - p0 );
/*N*/ 		if( *p )
/*N*/ 			p++;
/*N*/ 	}
/*N*/ 	if ( bMergeSeps ) 			// folgende Trennzeichen ueberspringen
/*N*/ 	{
/*?*/ 		while ( *p && ScGlobal::UnicodeStrChr( pSeps, *p ) )
/*?*/ 			p++;
/*N*/ 	}
/*N*/ 	return p;
/*N*/ }

        //
        //
        //



















}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
