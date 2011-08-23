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

#include <stdlib.h>

#include <stdio.h>

#include <tools/resid.hxx>
#include <bf_svtools/macitem.hxx>
#include <bf_svtools/zformat.hxx>
#include <bf_svtools/urihelper.hxx>
#include <bf_so3/linkmgr.hxx>

#ifndef _SVSTDARR_USHORTS_DECL
#define _SVSTDARR_USHORTS
#endif

#include <fmtinfmt.hxx>
#include <fmtfld.hxx>

#include <horiornt.hxx>

#include "doc.hxx"
#include "sw3imp.hxx"
#include "flddat.hxx"
#include "docufld.hxx"
#include "chpfld.hxx"
#include "ddefld.hxx"
#include "expfld.hxx"
#include "reffld.hxx"
#include "usrfld.hxx"
#include "dbfld.hxx"
#include "txtfld.hxx"
#include <authfld.hxx>
#include "ndtxt.hxx"

#include "poolfmt.hxx"		// fuer InSetExpField
#include "poolfmt.hrc"		// fuer InSetExpField
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

#if !defined(UNX) && !defined(MSC) && !defined(PPC) && !defined(CSET) && !defined(WTC) && !defined(__MINGW32__) && !defined(OS2)

#define FIELDFNTAB_SIZE 37
#if FIELDFNTAB_SIZE != RES_FIELDS_END - RES_FIELDS_BEGIN
#error Feld-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif

#endif

#define SWG_AUTHORITY_ENTRY_LCL	'E'

sal_Char __FAR_DATA sSW3IO_FixedField[] = "FixedExport";
sal_Char __FAR_DATA sSW3IO_AuthorityField[] = "AuthorityExport";
/* #108791# */
sal_Char __FAR_DATA sSW3IO_DropDownField[] = "DropDownExport";

struct OldFormats
{
    NfIndexTableOffset	eFormatIdx;
    USHORT				nOldFormat;
};

static OldFormats aOldDateFmt40[] =
{
    // Datumsfelder:
    NF_DATE_SYSTEM_SHORT,			DFF_SSYS,		// Kurzes Systemdatum
    NF_DATE_SYSTEM_LONG,			DFF_LSYS,		// Langes Systemdatum
    NF_DATE_SYS_DDMMYY,				DFF_DMY,        // 06.10.64
    NF_DATE_SYS_DDMMYYYY,			DFF_DMYY,		// 06.10.1964
    NF_DATE_SYS_DMMMYY,				DFF_DMMY,		// 06. Okt 64
    NF_DATE_SYS_DMMMYYYY,			DFF_DMMYY,		// 06. Okt 1964
    NF_DATE_DIN_DMMMMYYYY,			DFF_DMMMYY,		// 06. Oktober 1964
    NF_DATE_DIN_DMMMMYYYY,			DFF_DMMMY,		// 06. Oktober 64
    NF_DATE_SYS_NNDMMMYY,			DFF_DDMMY,		// Di, 06. Okt 64
    NF_DATE_SYS_NNDMMMMYYYY,		DFF_DDMMMY,		// Di, 06. Oktober 64
    NF_DATE_SYS_NNDMMMMYYYY,		DFF_DDMMMYY,	// Di, 06. Oktober 1964
    NF_DATE_SYS_NNNNDMMMMYYYY,		DFF_DDDMMMYY,	// Dienstag, 06. Oktober 1964
    NF_DATE_SYS_NNNNDMMMMYYYY,		DFF_DDDMMMY,	// Dienstag, 06. Oktober 64
    NF_DATE_SYS_MMYY,				DFF_MY,			// 10.64
    NF_DATE_DIN_MMDD,				DFF_MD,			// 10-06
    NF_DATE_DIN_YYMMDD,				DFF_YMD,		// 64-10-06
    NF_DATE_DIN_YYYYMMDD,			DFF_YYMD,		// 1964-10-06

    NF_NUMERIC_START,				0				// Tabellenende
};

static OldFormats aOldDateFmt30[] =
{
    // Datumsfelder:
    NF_DATE_SYSTEM_SHORT,			DFF_SSYS,		// Kurzes Systemdatum
    NF_DATE_SYSTEM_LONG,			DFF_LSYS,		// Langes Systemdatum
    NF_DATE_SYS_DDMMYY,				DFF_DMY,        // 06.10.64
    NF_DATE_SYS_DDMMYYYY,			DFF_DMYY,		// 06.10.1964
    NF_DATE_SYS_DMMMYY,				DFF_DMMY,		// 06. Okt 64
    NF_DATE_SYS_DMMMYYYY,			4 /*DFF_DMMYY*/,	// 06. Okt 1964
    NF_DATE_DIN_DMMMMYYYY,			5 /*DFF_DMMMYY*/,	// 06. Oktober 1964
    NF_DATE_DIN_DMMMMYYYY,			5 /*DFF_DMMMY*/,	// 06. Oktober 64
    NF_DATE_SYS_NNDMMMMYYYY,		6 /*DFF_DDMMMYY*/,	// Di, 06. Oktober 1964
    NF_DATE_SYS_NNDMMMYY,			6 /*DFF_DDMMY*/,	// Di, 06. Okt 64
    NF_DATE_SYS_NNDMMMMYYYY,		6 /*DFF_DDMMMY*/,	// Di, 06. Oktober 64
    NF_DATE_SYS_NNNNDMMMMYYYY,		7 /*DFF_DDDMMMYY*/,	// Dienstag, 06. Oktober 1964
    NF_DATE_SYS_NNNNDMMMMYYYY,		7 /*DFF_DDDMMMY*/,	// Dienstag, 06. Oktober 64
    NF_DATE_SYS_MMYY,				2 /*DFF_MY*/,		// 10.64
    NF_DATE_DIN_MMDD,				DFF_MD,			// 10-06
    NF_DATE_DIN_YYMMDD,				DFF_YMD,		// 64-10-06
    NF_DATE_DIN_YYYYMMDD,			DFF_YYMD,		// 1964-10-06

    NF_NUMERIC_START,				0				// Tabellenende
};

static OldFormats aOldTimeFmt[] =
{
    // Zeitfelder:
    NF_TIME_HHMMSS,					TF_SYSTEM,		// Systemzeit
    NF_TIME_HHMM,					TF_SSMM_24,		// 23:25
    NF_TIME_HHMMAMPM,				TF_SSMM_12,		// 11:25 PM

    NF_NUMERIC_START,				0				// Tabellenende
};

static OldFormats aOldGetSetExpFmt40[] =
{
    NF_TEXT,						VVF_CMD,		// Kommando anzeigen
    NF_TEXT,						VVF_INVISIBLE,	// unsichtbar
    NF_PERCENT_INT,					VVF_XXP,		// 1234%
    NF_PERCENT_DEC2,				VVF_XX_XXP,   	// 1.234,56%
    NF_TEXT,						VVF_CLEAR,		// ???

    NF_NUMBER_SYSTEM,				VVF_SYS, 		// Zahlenformat aus der
                                                    // Systemeinstellung
    NF_NUMBER_INT,					VVF_X,			// 1234
    NF_NUMBER_DEC2,					VVF_X_X,   		// 1234,5
    NF_NUMBER_DEC2,					VVF_X_XX,   	// 1245,56
    NF_NUMBER_1000DEC2,				VVF_XX_XX,   	// 1.234,56
    NF_NUMBER_1000DEC2,				VVF_XX_X,   	// 1.234,5
    NF_NUMBER_1000DEC2,				VVF_XX_XXX,		// 1.234,567
    NF_CURRENCY_1000DEC2,			VVF_SYS_CUR,	// W„hrungsformat aus der
                                                    // Systemeinstellung
                                                    // (1.234,00 DM)
    NF_CURRENCY_1000INT,			VVF_X_CUR,		// 1234 DM
    NF_CURRENCY_1000DEC2,			VVF_XX_XX_CUR,  // 1234,56 DM 1234,00 DM
    NF_CURRENCY_1000DEC2_DASHED,	VVF_XX_X0_CUR,  // 1234,56 DM 1234,-- DM
    NF_CURRENCY_1000INT,			VVF_CUR_X,   	// DM 1234
    NF_CURRENCY_1000DEC2,			VVF_CUR_XX_XX,  // DM 1234,56 DM 1234,00
    NF_CURRENCY_1000DEC2_DASHED,	VVF_CUR_XX_X0,  // DM 1234,56 DM 1234,--

    NF_NUMERIC_START, 				0				// Tabellenende
};

static OldFormats aOldGetSetExpFmt30[] =
{
    NF_TEXT,						VVF_CMD,		// Kommando anzeigen
    NF_TEXT,						VVF_INVISIBLE,	// unsichtbar
    NF_PERCENT_INT,					VVF_XXP,		// 1234%
    NF_PERCENT_DEC2,				VVF_XX_XXP,   	// 1.234,56%
    NF_TEXT,						VVF_CLEAR,		// ???

    NF_NUMBER_SYSTEM,				0x0020, 		// Zahlenformat aus der
                                                    // Systemeinstellung
    NF_NUMBER_INT,					0x0080,			// 1234
    NF_NUMBER_1000DEC2,				0x0100,   		// 1.234,56
    NF_NUMBER_DEC2,					0x0100,   		// 1234,5
    NF_NUMBER_DEC2,					0x0100,  		// 1245,56
    NF_NUMBER_1000DEC2,				0x0100,   		// 1.234,5
    NF_NUMBER_1000DEC2,				0x0100,			// 1.234,567
    NF_CURRENCY_1000DEC2,			0x0200,			// W„hrungsformat aus der
                                                    // Systemeinstellung
                                                    // (1.234,00 DM)
    NF_CURRENCY_1000INT,			0x1000,			// 1234 DM
    NF_CURRENCY_1000DEC2,			0x1000,  		// 1234,56 DM 1234,00 DM
    NF_CURRENCY_1000DEC2_DASHED,	0x1000,  		// 1234,56 DM 1234,-- DM
    NF_CURRENCY_1000INT,			0x1000,   		// DM 1234
    NF_CURRENCY_1000DEC2,			0x1000,  		// DM 1234,56 DM 1234,00
    NF_CURRENCY_1000DEC2_DASHED,	0x1000, 		// DM 1234,56 DM 1234,--

    NF_NUMERIC_START, 				0				// Tabellenende
};

/*N*/ void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
/*N*/ 								USHORT& rSubType, UINT32 &rFmt,
/*N*/ 								USHORT nVersion )
/*N*/ {
/*N*/ 	const OldFormats *pOldFmt = 0L;
/*N*/ 
/*N*/ 	switch( rWhich )
/*N*/ 	{
/*N*/ 		case RES_DATEFLD:
/*N*/ 		case RES_FIXDATEFLD:
/*N*/ 			if( nVersion < SWG_NEWFIELDS )
/*N*/ 			{
/*N*/ 				rSubType = DATEFLD;
/*N*/ 				if( RES_FIXDATEFLD == rWhich )
/*N*/ 					rSubType |= FIXEDFLD;
/*N*/ 				rWhich = RES_DATETIMEFLD;
/*N*/ 				pOldFmt = nVersion<SWG_INETBROWSER ? aOldDateFmt30
/*N*/ 												   : aOldDateFmt40;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_TIMEFLD:
/*?*/ 		case RES_FIXTIMEFLD:
/*?*/ 			if( nVersion < SWG_NEWFIELDS )
/*?*/ 			{
/*?*/ 				rSubType = TIMEFLD;
/*?*/ 				if( RES_FIXTIMEFLD == rWhich )
/*?*/ 					rSubType |= FIXEDFLD;
/*?*/ 				rWhich = RES_DATETIMEFLD;
/*?*/ 				pOldFmt = aOldTimeFmt;
/*?*/ 			}
/*?*/ 			break;
/*N*/ 
/*N*/ 		case RES_DBFLD:
/*N*/ 			if( nVersion < SWG_NEWFIELDS )
/*N*/ 			{
/*N*/ 				rSubType = SUB_OWN_FMT;
/*N*/ 				pOldFmt = nVersion<SWG_INETBROWSER ? aOldGetSetExpFmt30
/*N*/ 													: aOldGetSetExpFmt40;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_TABLEFLD:
/*N*/ 		case RES_GETEXPFLD:
/*N*/ 		case RES_SETEXPFLD:
/*N*/ 		case RES_USERFLD:
/*N*/ 			if( nVersion < SWG_NEWFIELDS )
/*N*/ 			{
/*N*/ 				if( rFmt == VVF_INVISIBLE )
/*N*/ 				{
/*N*/ 					rSubType = SUB_INVISIBLE;
/*N*/ 					rFmt = 0;
/*N*/ 				}
/*N*/ 				else if( rFmt == VVF_CMD )
/*N*/ 				{
/*N*/ 					rSubType = SUB_CMD;
/*N*/ 					rFmt = 0;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Kleiner Hack: Bei Numernkreisen wird das
/*N*/ 					// unkonvertierte Format noch benoetigt. Wir merken es
/*N*/ 					// uns voruebergehend mal im Subtyp, sofern es
/*N*/ 					// ueberhaupt als entsprechendes Format in Frage kommt.
/*N*/ 					if( RES_SETEXPFLD==rWhich &&
/*N*/ 						rFmt >= (USHORT)SVX_NUM_CHARS_UPPER_LETTER &&
/*N*/ 						rFmt <= (USHORT)SVX_NUM_BITMAP )
/*N*/ 					{
/*N*/ 						rSubType = (USHORT)rFmt;
/*N*/ 					}
/*N*/ 					pOldFmt = nVersion<SWG_INETBROWSER ? aOldGetSetExpFmt30
/*N*/ 													   : aOldGetSetExpFmt40;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		case RES_DOCINFOFLD:
/*N*/ 			if( nVersion < SWG_NEWFIELDS )
/*N*/ 			{
/*N*/ 				switch( rFmt )
/*N*/ 				{
/*N*/ 				case RF_AUTHOR:	rSubType = DI_SUB_AUTHOR;	break;
/*N*/ 				case RF_TIME:	rSubType = DI_SUB_TIME;	break;
/*N*/ 				case RF_DATE:	rSubType = DI_SUB_DATE;	break;
/*N*/ 				case RF_ALL:	rSubType = DI_SUB_DATE;	break;
/*N*/ 				}
/*N*/ 				rFmt = 0;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pOldFmt )
/*N*/ 	{
/*N*/ 		SvNumberFormatter *pFormatter = rDoc.GetNumberFormatter();
/*N*/ 		USHORT i = 0;
/*N*/ 
/*N*/ 		while( pOldFmt[i].eFormatIdx != NF_NUMERIC_START ||
/*N*/ 			   pOldFmt[i].nOldFormat)
/*N*/ 		{
/*N*/ 			if( rFmt == pOldFmt[i].nOldFormat )
/*N*/ 			{
/*N*/ 				rFmt = pFormatter->GetFormatIndex(pOldFmt[i].eFormatIdx, LANGUAGE_SYSTEM);
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			i++;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void sw3io_ConvertToOldField( const SwField* pFld, USHORT& rWhich,
/*N*/ 							  UINT32& rFmt, ULONG nFFVersion )
/*N*/ {
/*N*/ 	const OldFormats *pOldFmt = 0L;
/*N*/ 	UINT32 nOldFmt = rFmt;
/*N*/ 
/*N*/ 	switch( rWhich )
/*N*/ 	{
/*N*/ 		case RES_DOCINFOFLD:
/*N*/ 			if( SOFFICE_FILEFORMAT_40 >= nFFVersion )
/*N*/ 			{
/*N*/ 				switch (pFld->GetSubType() & 0xff00)
/*N*/ 				{
/*N*/ 				case DI_SUB_AUTHOR:	rFmt = RF_AUTHOR;	break;
/*N*/ 				case DI_SUB_TIME:	rFmt = RF_TIME;		break;
/*N*/ 				case DI_SUB_DATE:	rFmt = RF_DATE;		break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_DATETIMEFLD:
/*N*/ 			if( SOFFICE_FILEFORMAT_40 >= nFFVersion )
/*N*/ 			{
/*N*/ 				USHORT nSubType = ((SwDateTimeField*) pFld)->GetSubType();
/*N*/ 				switch( nSubType )
/*N*/ 				{
/*N*/ 				case DATEFLD:			rWhich = RES_DATEFLD;		break;
/*N*/ 				case TIMEFLD:			rWhich = RES_TIMEFLD;		break;
/*N*/ 				case DATEFLD|FIXEDFLD: 	rWhich = RES_FIXDATEFLD;	break;
/*N*/ 				case TIMEFLD|FIXEDFLD:	rWhich = RES_FIXTIMEFLD;	break;
/*N*/ 				}
/*N*/ 
/*N*/ 				if( nSubType & DATEFLD )
/*N*/ 				{
/*N*/ 					rFmt = DFF_DMY;
/*N*/ 					pOldFmt = aOldDateFmt40;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					rFmt = TF_SYSTEM;
/*N*/ 					pOldFmt = aOldTimeFmt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_DBFLD:
/*N*/ 		case RES_TABLEFLD:
/*N*/ 		case RES_GETEXPFLD:
/*N*/ 		case RES_SETEXPFLD:
/*N*/ 		case RES_USERFLD:
/*N*/ 			if( SOFFICE_FILEFORMAT_40 >= nFFVersion )
/*N*/ 			{
/*N*/ 				USHORT nSubType = pFld->GetSubType();
/*N*/ 
/*N*/ 				if (nSubType & SUB_INVISIBLE)
/*N*/ 					rFmt = VVF_INVISIBLE;
/*N*/ 				else if (nSubType & SUB_CMD)
/*N*/ 					rFmt = VVF_CMD;
/*N*/ 				else if( !(GSE_SEQ & nSubType) )
/*N*/ 				{
/*N*/ 					pOldFmt = aOldGetSetExpFmt40;
/*N*/ 					rFmt = VVF_SYS;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_GETREFFLD:
/*N*/ 			if( SOFFICE_FILEFORMAT_31 == nFFVersion )
/*N*/ 			{
/*N*/ 				switch( rFmt )
/*N*/ 				{
/*N*/ 				case REF_PAGE:
/*N*/ 				case REF_CHAPTER:
/*N*/ 				case REF_CONTENT:
/*N*/ 					break;
/*N*/ 
/*N*/ 				default:
/*N*/ 				// case REF_UPDOWN:
/*N*/ 				// case REF_PAGE_PGDESC:
/*N*/ 					rFmt = REF_PAGE;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pOldFmt && nOldFmt )
/*N*/ 	{
/*N*/ 		USHORT i = 0;
/*N*/ 
/*N*/ 		SvNumberFormatter *pFormatter = ((SwValueField*)pFld)->GetDoc()->GetNumberFormatter();
/*N*/ 		const SvNumberformat* pEntry = pFormatter->GetEntry( nOldFmt );
/*N*/ 
/*N*/ 		if( pEntry )
/*N*/ 		{
/*N*/ 			while( pOldFmt[i].eFormatIdx != NF_NUMERIC_START ||
/*N*/ 				   pOldFmt[i].nOldFormat )
/*N*/ 			{
/*N*/ 				sal_uInt32 nKey = pFormatter->GetFormatIndex(
/*N*/ 							pOldFmt[i].eFormatIdx, pEntry->GetLanguage() );
/*N*/ 
/*N*/ 				if( nOldFmt == nKey )
/*N*/ 				{
/*N*/ 					rFmt = pOldFmt[i].nOldFormat;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				i++;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void lcl_sw3io_FillSetExpFieldName( Sw3IoImp& rIo, USHORT nStrId,
/*N*/ 									String& rName )
/*N*/ {
/*N*/ 	USHORT nPoolId = rIo.aStringPool.FindPoolId( nStrId );
/*N*/ 	USHORT nResId = USHRT_MAX;
/*N*/ 	switch( nPoolId )
/*N*/ 	{
/*N*/ 	case RES_POOLCOLL_LABEL_ABB:
/*N*/ 		nResId = STR_POOLCOLL_LABEL_ABB;
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_LABEL_TABLE:
/*N*/ 		nResId = STR_POOLCOLL_LABEL_TABLE;
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_LABEL_FRAME:
/*N*/ 		nResId = STR_POOLCOLL_LABEL_FRAME;
/*N*/ 		break;
/*N*/ 	case RES_POOLCOLL_LABEL_DRAWING:
/*N*/ 		nResId = STR_POOLCOLL_LABEL_DRAWING;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	if( nResId != USHRT_MAX )
/*N*/ 		rName = SW_RESSTR( nResId );
/*N*/ 	else
/*N*/ 		rName = rIo.aStringPool.Find( nStrId );
/*N*/ }

/*N*/ USHORT lcl_sw3io_GetSetExpFieldPoolId( const String& rName )
/*N*/ {
/*N*/ 	if( rName == String( SW_RES(STR_POOLCOLL_LABEL_ABB) ) )
/*N*/ 		return RES_POOLCOLL_LABEL_ABB;
/*N*/ 	else if( rName == String( SW_RES(STR_POOLCOLL_LABEL_TABLE) ) )
/*N*/ 		return RES_POOLCOLL_LABEL_TABLE;
/*N*/ 	else if( rName == String( SW_RES(STR_POOLCOLL_LABEL_FRAME) ) )
/*N*/ 		return RES_POOLCOLL_LABEL_FRAME;
/*N*/ 	else if( rName == String( SW_RES(STR_POOLCOLL_LABEL_DRAWING) ) )
/*N*/ 		return RES_POOLCOLL_LABEL_DRAWING;
/*N*/ 	else
/*N*/ 		return USHRT_MAX;
/*N*/ }

/*  */

//////////////////////////////////////////////////////////////////////////////

// Ausgabe der Feldtypen

/*N*/ SwDBFieldType* lcl_sw3io_InDBFieldType( Sw3IoImp& rIo )
/*N*/ {
/*N*/ 	String aText, aDBName;
/*N*/ 
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		rIo.InString( *rIo.pStrm, aText );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aText = rIo.aStringPool.Find( nPoolId );
/*N*/ 		if( rIo.IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 		{
/*N*/ 			*rIo.pStrm >> nPoolId;
/*N*/ 			aDBName = rIo.aStringPool.Find( nPoolId );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !aText.Len() && !aDBName.Len() )
/*N*/ 	{
/*?*/ 		rIo.Warning();
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDBData aData;
/*N*/ 	aData.sDataSource = aDBName.GetToken(0, DB_DELIM);
/*N*/ 	aData.sCommand = aDBName.GetToken(1, DB_DELIM);
/*N*/ 	SwDBFieldType aType( rIo.pDoc, aText, aData );
/*N*/ 	return (SwDBFieldType*) rIo.pDoc->InsertFldType( aType );
/*N*/ }

/*N*/ void lcl_sw3io_OutDBFieldType( Sw3IoImp& rIo, SwDBFieldType* pType )
/*N*/ {
/*N*/ 	*rIo.pStrm << (UINT16) rIo.aStringPool.Find( pType->GetColumnName(), USHRT_MAX );
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		SwDBData aData = pType->GetDBData();
/*N*/ 		String sDBName(aData.sDataSource);
/*N*/ 		sDBName += DB_DELIM;
/*N*/ 		sDBName += (String)aData.sCommand;
/*N*/ 		*rIo.pStrm << (UINT16) rIo.aStringPool.Find( sDBName, IDX_NOCONV_FF );
/*N*/ 	}
/*N*/ }

/*N*/ SwUserFieldType* lcl_sw3io_InUserFieldType40( Sw3IoImp& rIo )
/*N*/ {
/*N*/ 	String aName, aContent, aValue;
/*N*/ 	UINT16 nType;
/*N*/ 
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		rIo.InString( *rIo.pStrm, aName );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aName = rIo.aStringPool.Find( nPoolId );
/*N*/ 		if( !aName.Len() )
/*N*/ 		{
/*?*/ 			rIo.Warning();
/*?*/ 			return NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	rIo.InString( *rIo.pStrm, aContent );
/*N*/ 	rIo.InString( *rIo.pStrm, aValue );
/*N*/ 	*rIo.pStrm >> nType;
/*N*/ 	SwUserFieldType* p = (SwUserFieldType*) rIo.pDoc->InsertFldType(
/*N*/ 											SwUserFieldType( rIo.pDoc, aName ) );
/*N*/ 	//JP 07.04.97: beim Einfuegen darf an bestehenden FeldType nichts
/*N*/ 	//				veraendert werden
/*N*/ 	if( rIo.bInsert && p->GetDepends() )
/*N*/ 		return p;
/*N*/ 
/*N*/ 	p->SetContent( aContent );
/*N*/ 	sal_Char* dummy;
/*N*/ 	ByteString sTmp( aValue, RTL_TEXTENCODING_ASCII_US );
/*N*/ 	p->SetValue( strtod( sTmp.GetBuffer(), &dummy ) );
/*N*/ 	if( !nType )
/*N*/ 		nType = GSE_STRING;
/*N*/ 	p->SetType( nType );
/*N*/ 	return p;
/*N*/ }

/*N*/ SwUserFieldType* lcl_sw3io_InUserFieldType( Sw3IoImp& rIo )
/*N*/ {
/*N*/ 	String aName, aContent;
/*N*/ 	double dVal;
/*N*/ 	UINT16 nType;
/*N*/ 
/*N*/ 	USHORT nPoolId;
/*N*/ 	*rIo.pStrm >> nPoolId;
/*N*/ 	aName = rIo.aStringPool.Find( nPoolId );
/*N*/ 	if( !aName.Len() )
/*N*/ 	{
/*?*/ 		rIo.Warning();
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	rIo.InString( *rIo.pStrm, aContent );
/*N*/ 	*rIo.pStrm >> dVal >> nType;
/*N*/ 	SwUserFieldType* p = (SwUserFieldType*) rIo.pDoc->InsertFldType(
/*N*/ 											SwUserFieldType( rIo.pDoc, aName ) );
/*N*/ 	//JP 07.04.97: beim Einfuegen darf an bestehenden FeldType nichts
/*N*/ 	//				veraendert werden
/*N*/ 	if( rIo.bInsert && p->GetDepends() )
/*N*/ 		return p;
/*N*/ 
/*N*/ 	p->SetContent( aContent );
/*N*/ 	p->SetValue( dVal );
/*N*/ 	if( !nType )
/*N*/ 		nType = GSE_STRING;
/*N*/ 	p->SetType( nType );
/*N*/ 	return p;
/*N*/ }

/*N*/ void lcl_sw3io_OutUserFieldType40( Sw3IoImp& rIo, SwUserFieldType* pType )
/*N*/ {
/*N*/     String aValue(String::CreateFromDouble(pType->GetValue()));
/*N*/ 	*rIo.pStrm << (UINT16) rIo.aStringPool.Find( pType->GetName(), USHRT_MAX );
/*N*/    	rIo.OutString( *rIo.pStrm, pType->GetContent() );
/*N*/    	rIo.OutString( *rIo.pStrm, aValue );
/*N*/ 	*rIo.pStrm << (UINT16) pType->GetType();
/*N*/ }

/*N*/ void lcl_sw3io_OutUserFieldType( Sw3IoImp& rIo, SwUserFieldType* pType )
/*N*/ {
/*N*/ 	*rIo.pStrm << (UINT16) rIo.aStringPool.Find( pType->GetName(), USHRT_MAX );
/*N*/    	rIo.OutString( *rIo.pStrm, pType->GetContent() );
/*N*/ 	*rIo.pStrm << (double)pType->GetValue()
/*N*/ 			   << (UINT16) pType->GetType();
/*N*/ }

/*  */

/*N*/ SwDDEFieldType* lcl_sw3io_InDDEFieldType( Sw3IoImp& rIo )
/*N*/ {
/*N*/ 	UINT16 nType;
/*N*/ 	String aName, aCmd;
/*N*/ 
/*N*/ 	*rIo.pStrm >> nType;
/*N*/ 
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		rIo.InString( *rIo.pStrm, aName );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aName = rIo.aStringPool.Find( nPoolId );
/*N*/ 		if( !aName.Len() )
/*N*/ 		{
/*?*/ 			rIo.Warning();
/*?*/ 			return NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ByteString s8;
/*N*/ 	rIo.pStrm->ReadByteString( s8 );
/*N*/     sal_Char cSrch = rIo.nVersion < SWG_DDESEP ? ' ' : ::binfilter::cTokenSeperator;
/*N*/ 
/*N*/ 	{
/*N*/ 		// die ersten beiden Blanks gegen den neuen Trenner austauschen
/*N*/ 		xub_StrLen nFnd = s8.Search( cSrch );
/*N*/ 		aCmd = String( s8, 0, nFnd, rIo.eSrcSet );
/*N*/ 		if( STRING_NOTFOUND != nFnd++ )
/*N*/ 		{
/*N*/ 			xub_StrLen nFnd2 = s8.Search( cSrch, nFnd );
/*N*/             ( aCmd += ::binfilter::cTokenSeperator) +=
/*N*/ 				String( s8, nFnd, nFnd2 - nFnd, rIo.eSrcSet );
/*N*/ 			if( STRING_NOTFOUND != nFnd2++ )
/*N*/                 ( aCmd += ::binfilter::cTokenSeperator) +=
/*N*/ 					String( s8, nFnd2, aCmd.Len() - nFnd2, rIo.eSrcSet );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// JP 15.08.00: our dialog have set the wrong format id's
/*N*/     if( ::binfilter::LINKUPDATE_ALWAYS != nType && ::binfilter::LINKUPDATE_ONCALL != nType )
/*?*/         nType = ::binfilter::LINKUPDATE_ONCALL;
/*N*/ 
/*N*/ 	SwDDEFieldType aType( aName, aCmd, nType );
/*N*/ 	return (SwDDEFieldType*) rIo.pDoc->InsertFldType( aType );
/*N*/ }

/*N*/ void lcl_sw3io_OutDDEFieldType( Sw3IoImp& rIo, SwDDEFieldType* pType )
/*N*/ {
/*N*/ 	*rIo.pStrm << (USHORT) pType->GetType()
/*N*/ 			   << (UINT16) rIo.aStringPool.Find( pType->GetName(), USHRT_MAX );
/*N*/     ByteString s8 = rIo.ConvertStringNoDelim( pType->GetCmd(),
/*N*/                         ::binfilter::cTokenSeperator, '\xff', rIo.eSrcSet );
/*N*/ 	rIo.pStrm->WriteByteString( s8 );
/*N*/ }

/*N*/ SwSetExpFieldType* lcl_sw3io_InSetExpFieldType( Sw3IoImp& rIo )
/*N*/ {
/*N*/ 	UINT16 nType;
/*N*/ 	String aName;
/*N*/ 
/*N*/ 	*rIo.pStrm >> nType;
/*N*/ 
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		rIo.InString( *rIo.pStrm, aName );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		lcl_sw3io_FillSetExpFieldName( rIo, nPoolId, aName );
/*N*/ 		if( !aName.Len() )
/*N*/ 		{
/*?*/ 			rIo.Warning();
/*?*/ 			return NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nFldTypeCount = rIo.pDoc->GetFldTypes()->Count();
/*N*/ 	SwSetExpFieldType* pFldType = (SwSetExpFieldType*) rIo.pDoc->
/*N*/ 			InsertFldType( SwSetExpFieldType( rIo.pDoc, aName, nType ));
/*N*/ 
/*N*/ 	if( SWG_SETEXPFLDCHG <= rIo.nVersion && GSE_SEQ & nType )
/*N*/ 	{
/*N*/ 		BYTE cDelim, nLevel;
/*N*/ 		*rIo.pStrm >> cDelim >> nLevel;
/*N*/ 
/*N*/ 		//JP 17.06.98: nicht setzen, wenn es ein Standard FeldType ist und
/*N*/ 		//				man am Einfuegen ist.
/*N*/ 		if( !rIo.bInsert ||
/*N*/ 			nFldTypeCount != rIo.pDoc->GetFldTypes()->Count() )
/*N*/ 		{
/*N*/ 			pFldType->SetDelimiter( ByteString::ConvertToUnicode( cDelim,
/*N*/ 																  rIo.eSrcSet));
/*N*/ 			pFldType->SetOutlineLvl( nLevel );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pFldType;
/*N*/ }

/*N*/ void lcl_sw3io_OutSetExpFieldType( Sw3IoImp& rIo, SwSetExpFieldType* pType )
/*N*/ {
/*N*/ 	const String& rNm = pType->GetSetRefName();
/*N*/ 
/*N*/ 	*rIo.pStrm << (UINT16) pType->GetType()
/*N*/ 			   << (UINT16) rIo.aStringPool.Find( rNm,
/*N*/ 								lcl_sw3io_GetSetExpFieldPoolId( rNm ) );
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Or40Export() && GSE_SEQ & pType->GetType() )
/*N*/ 	{
/*N*/ 		sal_Char cDelim = ByteString::ConvertFromUnicode( pType->GetDelimiter(),
/*N*/ 														  rIo.eSrcSet );
/*N*/ 		*rIo.pStrm  << (BYTE)cDelim
/*N*/ 					<< (BYTE)pType->GetOutlineLvl();
/*N*/ 	}
/*N*/ }

SwAuthorityFieldType* lcl_sw3io_InAuthorityFieldType( Sw3IoImp& rIo )
{
    SwAuthorityFieldType* pFldType = (SwAuthorityFieldType*) rIo.pDoc->
            InsertFldType( SwAuthorityFieldType(rIo.pDoc) );

    UINT16 nCount, nSortCount;
    BYTE cPrefix, cSuffix;
    BYTE cFlags = rIo.OpenFlagRec();
    *rIo.pStrm  >> nCount
                >> cPrefix
                >> cSuffix
                >> nSortCount;
    rIo.CloseFlagRec();
    if( 0 == pFldType->GetEntryCount() || (rIo.bNormal && !rIo.bInsert) )
    {
        pFldType->SetPreSuffix( ByteString::ConvertToUnicode( cPrefix,
                                                              rIo.eSrcSet ),
                                ByteString::ConvertToUnicode( cSuffix,
                                                              rIo.eSrcSet ) );
        pFldType->SetSequence( (cFlags & 0x10) != 0 );
        pFldType->SetSortByDocument( (cFlags & 0x20) != 0);
    }

    ASSERT( !rIo.pAuthorityMap, "authority map is already existing" );
    if( nCount > 0 )
        rIo.pAuthorityMap = new SvUShorts;

    USHORT i;
    for( i=0; i<nCount; i++ )
    {
        rIo.OpenRec( SWG_AUTHORITY_ENTRY_LCL );

        SwAuthEntry aEntry;

        while( rIo.BytesLeft() )
        {
            UINT16 nPos;
            String sEntry;
            *rIo.pStrm  >> nPos;
            rIo.InString( *rIo.pStrm, sEntry );

            aEntry.SetAuthorField( (ToxAuthorityField)nPos, sEntry );
        }

        USHORT nNewPos = pFldType->AppendField( aEntry );
        ASSERT( !rIo.bNormal || rIo.bInsert || nNewPos == i,
                "unexpected authority entry position" );
        rIo.pAuthorityMap->Insert( nNewPos, rIo.pAuthorityMap->Count() );

        rIo.CloseRec( SWG_AUTHORITY_ENTRY_LCL );
    }
    SwTOXSortKey* pSortKeys = nSortCount ? new SwTOXSortKey[nSortCount] : 0;
    for( i=0; i<nSortCount; i++ )
    {
        BYTE cFlag;
        UINT16 nType;
        *rIo.pStrm  >> cFlag
                    >> nType;
        pSortKeys[i].bSortAscending = 0 != (cFlag & 0x01);
        pSortKeys[i].eField = (ToxAuthorityField)nType;
    }
    pFldType->SetSortKeys(nSortCount, pSortKeys);
    delete pSortKeys;

    return pFldType;
}

/*N*/ void lcl_sw3io_OutAuthorityFieldType( Sw3IoImp& rIo,
/*N*/ 									  SwAuthorityFieldType *pType )
/*N*/ {
/*?*/   BYTE cFlags = 0x06;
/*?*/   if( pType->IsSequence() )
/*?*/       cFlags |= 0x10;
/*?*/   if( pType->IsSortByDocument() )
/*?*/       cFlags|= 0x20;
/*?*/ 
/*?*/   USHORT nCount = pType->GetEntryCount();
/*?*/   *rIo.pStrm  << cFlags
/*?*/               << (UINT16)nCount
/*?*/               << (BYTE)ByteString::ConvertFromUnicode( pType->GetPrefix(),
/*?*/                                                        rIo.eSrcSet )
/*?*/               << (BYTE)ByteString::ConvertFromUnicode( pType->GetSuffix(),
/*?*/                                                        rIo.eSrcSet )
/*?*/               << (UINT16)pType->GetSortKeyCount();
/*?*/ 
/*?*/   USHORT i;
/*?*/   for( i=0; i<nCount; i++ )
/*?*/   {
/*?*/       const SwAuthEntry *pEntry = pType->GetEntryByPosition( i );
/*?*/       rIo.OpenRec( SWG_AUTHORITY_ENTRY_LCL );
/*?*/ 
/*?*/       USHORT nPos = 0;
/*?*/       String sEntry;
/*?*/       BOOL bHasEntry = pEntry->GetFirstAuthorField( nPos, sEntry );
/*?*/       while( bHasEntry )
/*?*/       {
/*?*/           *rIo.pStrm  << (UINT16)nPos;
/*?*/           rIo.OutString(*rIo.pStrm, sEntry );
/*?*/           bHasEntry = pEntry->GetNextAuthorField( nPos, sEntry );
/*?*/       }
/*?*/ 
/*?*/       rIo.CloseRec( SWG_AUTHORITY_ENTRY_LCL );
/*?*/   }
/*?*/   for( i=0; i < pType->GetSortKeyCount(); i++ )
/*?*/   {
/*?*/       const SwTOXSortKey* pKey = pType->GetSortKey(i);
/*?*/       *rIo.pStrm  << (BYTE)(pKey->bSortAscending ? 0X01 : 0x00)
/*?*/                   << (UINT16)pKey->eField;
/*?*/   }
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDBField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 	String aName;
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		pType = lcl_sw3io_InDBFieldType( rIo );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aName = rIo.aStringPool.Find(nPoolId);
/*N*/ 	}
/*N*/ 
/*N*/ 	String aExpand;
/*N*/ 	rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 	BYTE cFlag = 0;
/*N*/ 	if( rIo.nVersion >= SWG_SHORTFIELDS )
/*N*/ 		*rIo.pStrm >> cFlag;
/*N*/ 
/*N*/ 	if( rIo.IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*?*/ 		USHORT nPoolId;
/*?*/ 		*rIo.pStrm >> nPoolId;
/*?*/ 		String aDBName( rIo.aStringPool.Find( nPoolId ) );
/*?*/ 		if (aDBName.Len())
/*?*/ 		{
/*?*/ 			aDBName += DB_DELIM;
/*?*/ 			aDBName += aName;
/*?*/ 			aName = aDBName;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( aName.Len() )
/*N*/ 		pType = rIo.pDoc->GetFldType( RES_DBFLD, aName);
/*N*/ 
/*N*/ 	if( !pType )
/*N*/ 	{
/*?*/ 		rIo.Warning();
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDBField* pFld = new SwDBField( (SwDBFieldType *)pType, rFmt );
/*N*/ 	pFld->SetSubType(nSubType);
/*N*/ 
/*N*/ 	if( rIo.nVersion >= SWG_SHORTFIELDS && (0x01 & cFlag) )
/*N*/ 	{
            sal_Char* dummy;
/*?*/       ByteString sTmp( aExpand, RTL_TEXTENCODING_ASCII_US );
/*?*/       pFld->ChgValue( strtod( sTmp.GetBuffer(), &dummy ), TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pFld->InitContent( aExpand );
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDBField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 							  USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 	String aExpand;
/*N*/ 	UINT16 nColNamePoolId, nDBNamePoolId;
/*N*/ 	BYTE cFlag;
/*N*/ 	*rIo.pStrm >> cFlag >> nColNamePoolId >> nDBNamePoolId;
/*N*/ 
/*N*/ 	String aColName( rIo.aStringPool.Find( nColNamePoolId ) );
/*N*/ 	String aDBName( rIo.aStringPool.Find( nDBNamePoolId ) );
/*N*/ 	if (aDBName.Len())
/*N*/ 	{
/*N*/ 		aDBName += DB_DELIM;
/*N*/ 		aDBName += aColName;
/*N*/ 		aColName = aDBName;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( aColName.Len() )
/*N*/ 		pType = rIo.pDoc->GetFldType( RES_DBFLD, aColName );
/*N*/ 
/*N*/ 	if( !pType )
/*N*/ 	{
/*?*/ 		rIo.Warning();
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDBField* pFld = new SwDBField( (SwDBFieldType *)pType, rFmt );
/*N*/ 	pFld->SetSubType(nSubType);
/*N*/ 
/*N*/ 	if( 0x01 & cFlag )
/*N*/ 	{
/*?*/       double dVal;
/*?*/       *rIo.pStrm >> dVal;
/*?*/       pFld->ChgValue( dVal, TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		String aExpand;
/*N*/ 		rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 		pFld->InitContent( aExpand );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutDBField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	BYTE cFlag = 0;
/*N*/ 	String sTxt( ((SwDBField *)pFld)->GetOldContent() );
/*N*/ 	if( ((SwDBField *)pFld)->IsValidValue() )
/*N*/ 	{
/*?*/ 		cFlag = 0x01;
/*?*/         ByteString sValue(ByteString::CreateFromDouble(((SwDBField *)pFld)->GetValue()));
/*?*/         sTxt.AssignAscii( sValue.GetBuffer() );
/*N*/ 	}
/*N*/ 	*rIo.pStrm << (UINT16)rIo.aStringPool.Find( ((SwDBFieldType *)pFld->GetTyp())->GetColumnName(), USHRT_MAX );
/*N*/ 	rIo.OutString( *rIo.pStrm, sTxt );
/*N*/ 	*rIo.pStrm  << cFlag;
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		SwDBData aData = ((SwDBFieldType *)pFld->GetTyp())->GetDBData();
/*N*/ 		String sDBName;
/*N*/ 		if(aData.sDataSource.getLength() || aData.sCommand.getLength())
/*N*/ 		{
/*?*/ 			sDBName = aData.sDataSource;
/*?*/ 			sDBName += DB_DELIM;
/*?*/ 			sDBName += (String)aData.sCommand;
/*N*/ 		}
/*N*/ 		*rIo.pStrm << (UINT16)rIo.aStringPool.Find( sDBName, IDX_NOCONV_FF );
/*N*/ 	}
/*N*/ }

/*N*/ void lcl_sw3io_OutDBField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	BYTE cFlag = ((SwDBField *)pFld)->IsValidValue() ? 0x01 : 0x00;
/*N*/ 	SwDBData aData = ((SwDBFieldType *)pFld->GetTyp())->GetDBData();
/*N*/ 	String sDBName;
/*N*/ 	if(aData.sDataSource.getLength() || aData.sCommand.getLength())
/*N*/ 	{
/*N*/ 		sDBName = aData.sDataSource;
/*N*/ 		sDBName += DB_DELIM;
/*N*/ 		sDBName += (String)aData.sCommand;
/*N*/ 	}
/*N*/ 
/*N*/ 	*rIo.pStrm << cFlag
/*N*/ 			   << (UINT16)rIo.aStringPool.Find(
/*N*/ 						((SwDBFieldType *)pFld->GetTyp())->GetColumnName(),
/*N*/ 						USHRT_MAX )
/*N*/ 			   << (UINT16)rIo.aStringPool.Find(
/*N*/ 						sDBName,
/*N*/ 						IDX_NOCONV_FF );
/*N*/ 
/*N*/ 	if( ((SwDBField *)pFld)->IsValidValue() )
/*N*/ 	{
/*?*/ 		*rIo.pStrm << (double)((SwDBField *)pFld)->GetValue();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rIo.OutString( *rIo.pStrm, ((SwDBField *)pFld)->GetOldContent() );
/*N*/ 	}
/*N*/ }

/*N*/ SwField* lcl_sw3io_InFileNameField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									USHORT, UINT32& rFmt )
/*N*/ {
/*N*/ 	// Das fixe Feld gibt es erst in der 5.1. Da das Fileformat zur 5.0
/*N*/ 	// kompatibel geblieben ist und die 5.0 das Fixed-Flag nicht loescht,
/*N*/ 	// kann es auch mal in einer aelteren Datei gesetzt sein.
/*N*/ 	if( !rIo.IsVersion(SWG_FIXEDFNFLD) && (rFmt & FF_FIXED) != 0 )
/*?*/ 		rFmt = (rFmt & ~FF_FIXED);
/*N*/ 
/*N*/ 	SwFileNameField *pFld =
/*N*/ 		new SwFileNameField( (SwFileNameFieldType *)pType );
/*N*/ 	if( (rFmt & FF_FIXED) != 0 )
/*N*/ 	{
/*?*/ 		String aContent;
/*?*/ 		rIo.InString( *rIo.pStrm, aContent );
/*?*/ 		pFld->SetExpansion( aContent );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

// Wird for 4.0-Export gar nicht erst aufgerufen!
/*N*/ void lcl_sw3io_OutFileNameField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	if( ((SwFileNameField *)pFld)->IsFixed() )
/*?*/ 		rIo.OutString( *rIo.pStrm, ((SwFileNameField *)pFld)->GetContent() );
/*N*/ }

/*  */

/*N*/ SwField* lcl_sw3io_InDBNameField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aDBName;
/*N*/ 	if( rIo.IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aDBName = rIo.aStringPool.Find( nPoolId );
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDBData aData;
/*N*/ 	aData.sDataSource = aDBName.GetToken(0, DB_DELIM);
/*N*/ 	aData.sCommand = aDBName.GetToken(1, DB_DELIM);
/*N*/ 	return new SwDBNameField( (SwDBNameFieldType *)pType, aData );
/*N*/ }

/*N*/ void lcl_sw3io_OutDBNameField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		SwDBData aData(((SwDBNameField*)pFld)->GetRealDBData());
/*N*/ 		String sDBName(aData.sDataSource);
/*N*/ 		sDBName += DB_DELIM;
/*N*/ 		sDBName += (String)aData.sCommand;
/*N*/ 		*rIo.pStrm << (UINT16)rIo.aStringPool.Find( sDBName, IDX_NOCONV_FF );
/*N*/ 	}
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDateField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT nSubType, UINT32& )
/*N*/ { //SW40.SDW  
/*N*/ 	SwDateTimeField* pFld =
/*N*/ 		new SwDateTimeField( (SwDateTimeFieldType *)pType, DATEFLD );
/*N*/ 	pFld->SetSubType(nSubType);
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InTimeField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT nSubType, UINT32& )
/*N*/ {
            SwDateTimeField* pFld =
                new SwDateTimeField( (SwDateTimeFieldType*)pType, TIMEFLD );
            pFld->SetSubType(nSubType);

            return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InPageNumberField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 										USHORT, UINT32& rFmt )
/*N*/ {
/*N*/ 	INT16 nOff;
/*N*/ 	UINT16 nSub;
/*N*/ 	*rIo.pStrm >> nOff >> nSub;
/*N*/ 	String sUserStr;
/*N*/ 	if( rIo.nVersion >= SWG_OLENAME )
/*N*/ 	{
/*N*/ 		rIo.InString( *rIo.pStrm, sUserStr );
/*N*/ 		if( rIo.IsVersion( SWG_NEXTPREVPAGE, SWG_EXPORT31 ) &&
/*N*/ 			( PG_NEXT == nSub || PG_PREV == nSub ))
/*?*/ 			*rIo.pStrm >> nOff;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwPageNumberField* pFld =
/*N*/ 		new SwPageNumberField( (SwPageNumberFieldType*)pType, nSub, rFmt, nOff );
/*N*/ 	if( sUserStr.Len() )
/*?*/ 		pFld->SetUserString( sUserStr );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InPageNumberField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									  USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	INT16 nOff;
/*N*/ 	String sUserStr;
/*N*/ 	*rIo.pStrm >> nOff;
/*N*/ 	rIo.InString( *rIo.pStrm, sUserStr );
/*N*/ 
/*N*/ 	SwPageNumberField* pFld =
/*N*/ 		new SwPageNumberField( (SwPageNumberFieldType*)pType, nSubType,
/*N*/ 							   rFmt, (short)nOff );
/*N*/ 	if( sUserStr.Len() )
/*?*/ 		pFld->SetUserString( sUserStr );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutPageNumberField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	INT16 nOff = (INT16)pFld->GetPar2().ToInt32();
/*N*/ 	UINT16 nSub = pFld->GetSubType();
/*N*/ 
/*N*/ 	if( rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		if( PG_NEXT == nSub )
/*N*/ 			nOff = 1;
/*N*/ 		else if( PG_PREV == nSub )
/*N*/ 			nOff = -1;
/*N*/ 	}
/*N*/ 
/*N*/ 	*rIo.pStrm << nOff
/*N*/ 			   << nSub;
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwPageNumberField*)pFld)->GetUserString() );
/*N*/ }

/*N*/ void lcl_sw3io_OutPageNumberField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	// nur Offset, deshalb kein long noetig!
/*N*/ 	INT16 nOff = (INT16)pFld->GetPar2().ToInt32();
/*N*/ 	*rIo.pStrm << nOff;
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwPageNumberField*)pFld)->GetUserString() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InUserField40( Sw3IoImp& rIo, SwFieldType *pType,
/*N*/ 								  USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		pType = lcl_sw3io_InUserFieldType40( rIo );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		pType = rIo.pDoc->GetFldType( RES_USERFLD,
/*N*/ 									  rIo.aStringPool.Find(nPoolId));
/*N*/ 		if( !pType )
/*N*/ 		{
/*?*/ 			rIo.Warning();
/*?*/ 			return NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SwUserField* pFld = new SwUserField( (SwUserFieldType *)pType );
/*N*/ 
/*N*/ 	USHORT nType = ((SwUserFieldType *)pType)->GetType();
/*N*/ 	nSubType |= nType;
/*N*/ 	if( UF_STRING & nType )
/*N*/ 		rFmt = 0;	// Warum auch immer!
/*N*/ 
/*N*/ 	pFld->SetSubType(nSubType);
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InUserField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 
/*N*/ 	USHORT nPoolId;
/*N*/ 	*rIo.pStrm >> nPoolId;
/*N*/ 	pType = rIo.pDoc->GetFldType( RES_USERFLD,
/*N*/ 								  rIo.aStringPool.Find(nPoolId));
/*N*/ 	if( !pType )
/*N*/ 	{
/*?*/ 		rIo.Warning();
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwUserField* pFld = new SwUserField( (SwUserFieldType *)pType );
/*N*/ 	pFld->SetSubType( nSubType );
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutUserField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	*rIo.pStrm << (UINT16)rIo.aStringPool.Find(
/*N*/ 					((SwUserField*)pFld)->GetTyp()->GetName(), USHRT_MAX );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InGetRefField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									USHORT, UINT32& rFmt )
/*N*/ {
/*N*/ 	String aName, aExpand;
/*N*/ 	UINT16 nFmt16 = 0, nSubType, nSeqNo;
/*N*/ 
/*N*/ 	rIo.InString( *rIo.pStrm, aName );
/*N*/ 	rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 	if( rIo.IsVersion( SWG_FMTGETREFFLD, SWG_EXPORT31 ) )
/*N*/ 	{
/*N*/ 		*rIo.pStrm >> nFmt16 >> nSubType >> nSeqNo;
/*N*/ 		rFmt = nFmt16;
/*N*/ 	}
/*N*/ 	else if( rIo.IsVersion( SWG_DESKTOP40 ) )
/*N*/ 	{
/*?*/ 		*rIo.pStrm >> nSubType >> nSeqNo;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nSubType = nSeqNo = 0;
/*N*/ 
/*N*/ 	SwGetRefField* pFld = new SwGetRefField( (SwGetRefFieldType*)pType,
/*N*/ 												aName, nSubType,
/*N*/ 												nSeqNo, rFmt );
/*N*/ 	pFld->SetExpand( aExpand );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InGetRefField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	String aName, aExpand;
/*N*/ 	UINT16 nFmt = 0, nSeqNo;
/*N*/ 
/*N*/ 	rIo.InString( *rIo.pStrm, aName );
/*N*/ 	rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 	*rIo.pStrm >> nSeqNo;
/*N*/ 
/*N*/ 	SwGetRefField* pFld = new SwGetRefField( (SwGetRefFieldType*)pType,
/*N*/ 												aName, nSubType,
/*N*/ 												nSeqNo, nFmt );
/*N*/ 	pFld->SetExpand( aExpand );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutGetRefField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwGetRefField*)pFld)->GetSetRefName() );
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->Expand() );
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		*rIo.pStrm << (UINT16)pFld->GetSubType()
/*N*/ 				   << (UINT16)((SwGetRefField*)pFld)->GetSeqNo();
/*N*/ 	}
/*N*/ }

/*N*/ void lcl_sw3io_OutGetRefField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwGetRefField*)pFld)->GetSetRefName() );
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->Expand() );
/*N*/ 	*rIo.pStrm << (UINT16)((SwGetRefField*)pFld)->GetSeqNo();
/*N*/ }

/*N*/ void lcl_sw3io_ChkHiddenExp( String& rCond ) //SW40.SDW 
/*N*/ {
/*N*/ 	// die Expression wurde bei 4.0 Export einmal gedreht, beim erneuten
/*N*/ 	// Einlesen sollte diese nicht noch mal gedreht werden.
/*N*/ 	xub_StrLen nLen = rCond.Len(), nPos = nLen, nCnt = 1;
/*N*/ 	if( 3 < nPos-- && ')' == rCond.GetChar( nPos ) &&
/*N*/ 		'!' == rCond.GetChar( nPos = 0 ) && '(' == rCond.GetChar( ++nPos ))
/*N*/ 	{
/*N*/ 		// dann teste mal ob es dann eine komplette Klammerung ist
/*N*/ 		--nLen; ++nPos;
/*N*/ 		nCnt = 0;
/*N*/ 		while( nPos < nLen )
/*N*/ 			switch( rCond.GetChar( nPos++ ) )
/*N*/ 			{
/*N*/ 			case '(':		++nCnt;		break;
/*N*/ 			case ')':		if( !nCnt-- )
/*N*/ 								nPos = nLen;
/*N*/ 							break;
/*N*/ 			}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !nCnt )
/*N*/ 		rCond = rCond.Copy( 2, rCond.Len() - 3);
/*N*/ 	else
/*N*/ 		rCond.InsertAscii( "!(", 0 ) += ')';
/*N*/ }

/*N*/ SwField* lcl_sw3io_InHiddenTxtField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									   USHORT, UINT32& )
/*N*/ { //SW40.SDW  
/*N*/ 	BYTE cFlags;
/*N*/ 	USHORT nSubType;
/*N*/ 	String aText, aCond;
/*N*/ 	*rIo.pStrm >> cFlags;
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	rIo.InString( *rIo.pStrm, aCond );
/*N*/ 	*rIo.pStrm >> nSubType;
/*N*/ 	BOOL bCond = BOOL( ( cFlags & 0x20 ) != 0 );
/*N*/ 	BOOL bIsHidden = BOOL( ( cFlags & 0x10 ) != 0 );
/*N*/ 
/*N*/ 	if( bCond && TYP_CONDTXTFLD != nSubType )
/*N*/ 	{
/*N*/ 		lcl_sw3io_ChkHiddenExp( aCond );
/*N*/ 		bIsHidden = !bIsHidden;
/*N*/ 	}
/*N*/ 	SwHiddenTxtField* pFld = new SwHiddenTxtField( (SwHiddenTxtFieldType*)pType,
/*N*/ 				bCond,
/*N*/ 				aEmptyStr, aText,
/*N*/ 				bIsHidden, nSubType );
/*N*/ 	pFld->SetPar1( aCond );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InHiddenTxtField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	BYTE cFlags;
/*N*/ 	String aText, aCond;
/*N*/ 	*rIo.pStrm >> cFlags;
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	rIo.InString( *rIo.pStrm, aCond );
/*N*/ 	BOOL bCond = BOOL( ( cFlags & 0x20 ) != 0 );
/*N*/ 
/*N*/ 	SwHiddenTxtField* pFld = new SwHiddenTxtField( (SwHiddenTxtFieldType*)pType,
/*N*/ 				bCond,
/*N*/ 				aEmptyStr, aText,
/*N*/ 				BOOL( ( cFlags & 0x10 ) != 0 ), nSubType );
/*N*/ 	pFld->SetPar1( aCond );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutHiddenTxtField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
    String aText(pFld->GetPar2());
    BYTE cFlags = ((SwHiddenTxtField*)pFld)->GetValue() ? 0x10 : 0;

    if( ((SwHiddenTxtField*)pFld)->IsValid() )
    {
        if( !rIo.IsSw31Export() )
        {
            aText = pFld->GetPar2();
            aText += '|';
            aText += ((SwHiddenTxtField*)pFld)->GetCntnt();
        }
        else
        {
            if (((SwHiddenTxtField*)pFld)->GetValue())
            {
                aText = ((SwHiddenTxtField*)pFld)->GetPar2().GetToken(0, '|');
                aText += '|';
                aText += ((SwHiddenTxtField*)pFld)->GetCntnt();
            }
            else
            {
                aText = ((SwHiddenTxtField*)pFld)->GetCntnt();
                aText += '|';
                aText += pFld->GetPar2().GetToken(1, '|');
            }
        }
    }
    else
        aText = pFld->GetPar2();

    if( ((SwHiddenTxtField*)pFld)->IsConditional() )
        cFlags |= 0x20;

    String sCond( pFld->GetPar1() );
    USHORT nSubType = pFld->GetSubType();
    if( 0x20 & cFlags && TYP_CONDTXTFLD != nSubType )
    {
        lcl_sw3io_ChkHiddenExp( sCond );
        if( 0x10 & cFlags )
            cFlags &= ~0x10;
        else
            cFlags |= 0x10;
    }

    *rIo.pStrm << cFlags;
    rIo.OutString( *rIo.pStrm, aText ); // text
    rIo.OutString( *rIo.pStrm, sCond ); // condition
    *rIo.pStrm << nSubType;
/*N*/ }

/*N*/ void lcl_sw3io_OutHiddenTxtField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	String aText(pFld->GetPar2());
/*N*/ 	BYTE cFlags = ((SwHiddenTxtField*)pFld)->GetValue() ? 0x10 : 0;
/*N*/ 
/*N*/ 	if( ((SwHiddenTxtField*)pFld)->IsValid() )
/*N*/ 	{
/*N*/ 		aText = pFld->GetPar2();
/*N*/ 		aText += '|';
/*N*/ 		aText += ((SwHiddenTxtField*)pFld)->GetCntnt();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aText = pFld->GetPar2();
/*N*/ 
/*N*/ 	if( ((SwHiddenTxtField*)pFld)->IsConditional() )
/*N*/ 		cFlags |= 0x20;
/*N*/ 	*rIo.pStrm << cFlags;
/*N*/ 	rIo.OutString( *rIo.pStrm, aText );	// text
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar1() ); 	// condition
/*N*/ }

/*N*/ SwField* lcl_sw3io_InPostItField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT, UINT32& )
/*N*/ {
        INT32 nDate;
        String aAuthor, aText;
        *rIo.pStrm >> nDate;
        rIo.InString( *rIo.pStrm, aAuthor );
        rIo.InString( *rIo.pStrm, aText );
        return new SwPostItField( (SwPostItFieldType*)pType, aAuthor, aText, Date( nDate ) );
/*N*/ }

/*N*/ void lcl_sw3io_OutPostItField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        Date aDate = ((SwPostItField*)pFld)->GetDate();
        *rIo.pStrm << (INT32) aDate.GetDate();
        rIo.OutString( *rIo.pStrm, pFld->GetPar1() );    // Author
        rIo.OutString( *rIo.pStrm, pFld->GetPar2() );     // Text
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDateTimeField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	double	fVal;
/*N*/ 
/*N*/ 	*rIo.pStrm >> fVal;
/*N*/ 
/*N*/ 	SwDateTimeField* pFld = new SwDateTimeField( (SwDateTimeFieldType*)pType, nSubType );
/*N*/ 	pFld->SetValue( fVal );
/*N*/ 
/*N*/ 	if (rIo.IsVersion(SWG_DATEOFFSET))
/*N*/ 	{
/*N*/ 		INT32 nOffset;
/*N*/ 
/*N*/ 		*rIo.pStrm >> nOffset;
/*N*/ 		pFld->SetOffset(nOffset);
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutDateTimeField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	*rIo.pStrm	<< ((SwDateTimeField*)pFld)->GetValue()
/*N*/ 				<< (INT32)((SwDateTimeField*)pFld)->GetOffset();
/*N*/ }

/*N*/ SwField* lcl_sw3io_InFixDateField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT, UINT32& )
/*N*/ {
/*N*/ 	INT32 nVal;
/*N*/ 	*rIo.pStrm >> nVal;
/*N*/ 	SwDateTimeField* pFld = new SwDateTimeField( (SwDateTimeFieldType*)pType, DATEFLD|FIXEDFLD );
/*N*/     Time aTmpTime; 
/*N*/     Date aTmpDate(nVal);
/*N*/     DateTime aDT(aTmpDate, aTmpTime);
/*N*/     pFld->SetDateTime( aDT );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutFixDateField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/     *rIo.pStrm << (INT32) ((SwDateTimeField*)pFld)->GetDate(TRUE).GetDate();
/*N*/ }

/*N*/ SwField* lcl_sw3io_InFixTimeField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT, UINT32& )
/*N*/ {
        INT32 nVal;
        *rIo.pStrm >> nVal;
        SwDateTimeField* pFld = new SwDateTimeField( (SwDateTimeFieldType*)pType, TIMEFLD|FIXEDFLD );
         Date aTmpDate;
        DateTime aDT(aTmpDate, Time(nVal));
           pFld->SetDateTime( aDT );
        return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutFixTimeField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
            *rIo.pStrm << (INT32)((SwDateTimeField*)pFld)->GetTime(TRUE).GetTime();
/*N*/ }

/*N*/ SwField* lcl_sw3io_InAuthorField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT, UINT32& )
/*N*/ {
/*N*/ 	SwAuthorField *pFld =
/*N*/ 		new SwAuthorField( (SwAuthorFieldType*)pType );
/*N*/ 
/*N*/ 	if( rIo.IsVersion( SWG_FIXEDFLDS ) )
/*N*/ 	{
/*N*/ 		String aExpand;
/*N*/ 		rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 		pFld->SetExpansion( aExpand );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

// Wird for 4.0-Export gar nicht erst aufgerufen!
/*N*/ void lcl_sw3io_OutAuthorField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwAuthorField *)pFld)->GetContent() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InChapterField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								   USHORT, UINT32& )
/*N*/ {
/*N*/ 	SwChapterField* pFld = new SwChapterField( (SwChapterFieldType*)pType );
/*N*/ 	if( rIo.nVersion >= SWG_OLENAME )
/*N*/ 	{
/*N*/ 		BYTE cLvl;
/*N*/ 		*rIo.pStrm >> cLvl;
/*N*/ 		if( cLvl >= MAXLEVEL )
/*?*/ 			cLvl = MAXLEVEL - 1;
/*N*/ 		pFld->SetLevel( cLvl );
/*N*/ 	}
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutChapterField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	BYTE cLvl = ((SwChapterField*)pFld)->GetLevel();
/*N*/ 	if( rIo.IsSw31Or40Export() && cLvl >= OLD_MAXLEVEL)
/*N*/ 		cLvl = OLD_MAXLEVEL - 1;
/*N*/ 
/*N*/ 	*rIo.pStrm << cLvl;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDocStatField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT, UINT32& rFmt )
/*N*/ {
/*N*/ 	UINT16 nSubType;
/*N*/ 	*rIo.pStrm >> nSubType;
/*N*/ 	return new SwDocStatField( (SwDocStatFieldType*)pType, nSubType, rFmt );
/*N*/ }
/*N*/ 
/*N*/ SwField* lcl_sw3io_InDocStatField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								   USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	return new SwDocStatField( (SwDocStatFieldType*)pType, nSubType, rFmt );
/*N*/ }

/*N*/ void lcl_sw3io_OutDocStatField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	*rIo.pStrm << (UINT16) pFld->GetSubType();
/*N*/ }

// Im 5.0-Format bleibt nix, was geschrieben werden muesste.

/*N*/ SwField* lcl_sw3io_InDDEField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 							   USHORT, UINT32& )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*?*/ 		pType = lcl_sw3io_InDDEFieldType( rIo );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		pType = rIo.pDoc->GetFldType( RES_DDEFLD,
/*N*/ 											rIo.aStringPool.Find(nPoolId));
/*N*/ 		if( !pType )
/*N*/ 		{
/*?*/ 			rIo.Warning();
/*?*/ 			return NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return new SwDDEField( (SwDDEFieldType*)pType );
/*N*/ }

/*N*/ void lcl_sw3io_OutDDEField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	*rIo.pStrm << (UINT16)rIo.aStringPool.Find( ((SwDDEField*)pFld)->GetTyp()->GetName(), USHRT_MAX );
/*N*/ 	rIo.nFileFlags |= SWGF_HAS_DDELNK;
/*N*/ }

/*  */

/*N*/ SwField* lcl_sw3io_InInputField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								 USHORT, UINT32& )
/*N*/ { //SW40.SDW  
/*N*/ 	String aContent, aPrompt;
/*N*/ 	UINT16 nSubType;
/*N*/ 	rIo.InString( *rIo.pStrm, aContent );
/*N*/ 	rIo.InString( *rIo.pStrm, aPrompt );
/*N*/ 	*rIo.pStrm >> nSubType;
/*N*/ 	return new SwInputField( (SwInputFieldType*)pType, aContent, aPrompt, nSubType );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InInputField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								 USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	String aContent, aPrompt;
/*N*/ 	rIo.InString( *rIo.pStrm, aContent );
/*N*/ 	rIo.InString( *rIo.pStrm, aPrompt );
/*N*/ 	return new SwInputField( (SwInputFieldType*)pType, aContent, aPrompt, nSubType );
/*N*/ }

/*N*/ void lcl_sw3io_OutInputField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        rIo.OutString( *rIo.pStrm, pFld->GetPar1() );// Content oder SwUserFieldName
        rIo.OutString( *rIo.pStrm, pFld->GetPar2() );// PromptText
        *rIo.pStrm << (UINT16) pFld->GetSubType();
/*N*/ }

/*N*/ void lcl_sw3io_OutInputField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar1() );// Content oder SwUserFieldName
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar2() );// PromptText
/*N*/ }

/*N*/ SwField* lcl_sw3io_InMacroField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								 USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aName;
/*N*/ 	String aText;
/*N*/ 	rIo.InString( *rIo.pStrm, aName );
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	return new SwMacroField( (SwMacroFieldType*)pType, aName, aText );
/*N*/ }

/*N*/ void lcl_sw3io_OutMacroField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar2() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InTblField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 							   USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	String aFormula, aText;
/*N*/ 	UINT16 nSub = 0;
/*N*/ 	rIo.InString( *rIo.pStrm, aFormula );
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	if( !rIo.IsVersion(SWG_NEWERFIELDS) )
/*N*/ 	{
/*N*/ 		*rIo.pStrm >> nSub;
/*N*/ 		if( !rIo.IsVersion(SWG_NEWFIELDS) )
/*N*/ 			nSubType |= nSub;
/*N*/ 	}
/*N*/ 	SwTblField* pFld = new SwTblField( (SwTblFieldType*)pType,
/*N*/ 									   aFormula, nSubType );
/*N*/ 
/*N*/ 
/*N*/ 	pFld->ChgExpStr( aText );
/*N*/ 	return pFld;
/*N*/ }

// Die Variable rIo.pCurTbl wird in Sw3IoImp::OutTable()
// besetzt und enthaelt die zur Zeit ausgegebene Tabelle

/*N*/ void lcl_sw3io_OutTblField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	if( rIo.pCurTbl )
/*N*/ 		((SwTblField*)pFld)->PtrToBoxNm( rIo.pCurTbl );
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar2() );
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwTblField*)pFld)->GetExpStr() );
/*N*/ 	if( rIo.IsSw31Or40Export() )
/*N*/ 		*rIo.pStrm << (UINT16) ((SwTblField*)pFld)->GetSubType();
/*N*/ }

/*N*/ SwField *lcl_sw3io_InGetExpField40( Sw3IoImp& rIo, SwFieldType *pType,
/*N*/ 									USHORT nSubType, UINT32& rFmt )
/*N*/ { //SW40.SDW  
/*N*/ 	String aText, aExpand;
/*N*/ 	UINT16 nSub;
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 	*rIo.pStrm >> nSub;
/*N*/ 
/*N*/ 	SwGetExpField* pFld =
/*N*/ 		new SwGetExpField( (SwGetExpFieldType *)pType, aText );
/*N*/ 	pFld->ChgExpStr( aExpand );
/*N*/ 	pFld->SetSubType( nSub | nSubType );
/*N*/ 
/*N*/ 	if( GSE_STRING & nSub )
/*N*/ 		rFmt = 0;	// Warum auch immer!
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InGetExpField( Sw3IoImp& rIo, SwFieldType *pType,
/*N*/ 								  USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	String aText, aExpand;
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 
/*N*/ 	SwGetExpField* pFld =
/*N*/ 		new SwGetExpField( (SwGetExpFieldType *)pType, aText );
/*N*/ 	pFld->ChgExpStr( aExpand );
/*N*/ 	pFld->SetSubType( nSubType );
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutGetExpField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        rIo.OutString( *rIo.pStrm, ((SwGetExpField*)pFld)->GetFormula() );
        rIo.OutString( *rIo.pStrm, ((SwGetExpField*)pFld)->GetExpStr() );
        *rIo.pStrm << (UINT16) pFld->GetSubType();
/*N*/ }

/*N*/ void lcl_sw3io_OutGetExpField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwGetExpField*)pFld)->GetFormula() );
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwGetExpField*)pFld)->GetExpStr() );
/*N*/ }

/*  */

/*N*/ SwField* lcl_sw3io_InSetExpField40( Sw3IoImp& rIo, SwFieldType *pType,
/*N*/ 									USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 	if( rIo.nVersion < SWG_SHORTFIELDS )
/*N*/ 		pType = lcl_sw3io_InSetExpFieldType( rIo );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 
/*N*/ 		// fix #26064#: Namen der 3 Label-Numernkreise: Hier wird die
/*N*/ 		// Pool-Id des dazugehoerigen Formats aus dem Str-Pool geholt
/*N*/ 		String aName;
/*N*/ 		lcl_sw3io_FillSetExpFieldName( rIo, nPoolId, aName );
/*N*/ 
/*N*/ 		pType = rIo.pDoc->GetFldType( RES_SETEXPFLD, aName );
/*N*/ 		if( !pType )
/*N*/ 		{
/*N*/ 			rIo.Warning();
/*N*/ 			return NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	UINT16 nSeqNo;
/*N*/ 	BYTE cFlags;
/*N*/ 	String aFormula, aExpand, aPrompt;
/*N*/ 	*rIo.pStrm >> cFlags;
/*N*/ 	rIo.InString( *rIo.pStrm, aFormula );
/*N*/ 	rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 	if( (cFlags & 0x10) && rIo.nVersion >= SWG_SHORTFIELDS )
/*N*/ 		rIo.InString( *rIo.pStrm, aPrompt );
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		*rIo.pStrm >> nSeqNo;
/*N*/ 
/*N*/ 	SwSetExpField* pFld =
/*N*/ 		new SwSetExpField( (SwSetExpFieldType *)pType, aFormula, rFmt );
/*N*/ 
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 	{
/*N*/ 		pFld->SetInputFlag( TRUE );
/*N*/ 		pFld->SetPromptText( aPrompt );
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nType = ((SwSetExpFieldType *)pType)->GetType();
/*N*/ 	// Hack: fuer Seq-Felder wurde das Original-Format im Subtyp uebergeben,
/*N*/ 	// aber nur, wenn es auch als entsprechendes Format in Frage kommt.
/*N*/ 	// (SUB_VISIBLE und SUB_CMD sind disjunkt).
/*N*/ 	if( nSubType >= (USHORT)SVX_NUM_CHARS_UPPER_LETTER &&
/*N*/ 		nSubType <= (USHORT)SVX_NUM_BITMAP )
/*N*/ 	{
/*N*/ 		if( GSE_SEQ & nType )
/*N*/ 			rFmt = nSubType;
/*N*/ 		nSubType = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	nSubType |= nType;
/*N*/ 	pFld->SetSubType( nSubType );
/*N*/ 
/*N*/ 	if( GSE_STRING & nType )
/*N*/ 		rFmt = 0;	// Warum auch immer!
/*N*/ 
/*N*/ 	if( GSE_SEQ & nType )
/*N*/ 	{
/*N*/ 		sal_Char* dummy;
/*N*/ 		ByteString sTmp( aExpand, RTL_TEXTENCODING_ASCII_US );
/*N*/ 		pFld->SetValue( strtod( sTmp.GetBuffer(), &dummy ) );
/*N*/ 
/*N*/ 		USHORT n = (USHORT)pFld->GetValue();
/*N*/ 
/*N*/ 		aExpand = ::binfilter::FormatNumber( n, rFmt );
/*N*/ 
/*N*/ 		if( cFlags & 0x20 )
/*N*/ 			pFld->SetSeqNumber( nSeqNo );
/*N*/ 	}
/*N*/ 	pFld->ChgExpStr( aExpand );
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InSetExpField( Sw3IoImp& rIo, SwFieldType *pType,
/*N*/ 								  USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	pType = 0;
/*N*/ 
/*N*/ 	BYTE cFlags;
/*N*/ 	UINT16 nPoolId, nSeqNo=0, nSeqVal=0;
/*N*/ 	String aFormula, aPrompt, aExpand;
/*N*/ 	*rIo.pStrm  >> cFlags >> nPoolId;
/*N*/ 	rIo.InString( *rIo.pStrm, aFormula );
/*N*/ 
/*N*/ 	// fix #26064#: Namen der 3 Label-Numernkreise: Hier wird die
/*N*/ 	// Pool-Id des dazugehoerigen Formats aus dem Str-Pool geholt
/*N*/ 	String aName;
/*N*/ 	lcl_sw3io_FillSetExpFieldName( rIo, nPoolId, aName );
/*N*/ 	pType = rIo.pDoc->GetFldType( RES_SETEXPFLD, aName );
/*N*/ 
/*N*/ 	if( !pType )
/*N*/ 	{
/*?*/ 		rIo.Warning();
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( cFlags & 0x10 )
/*?*/ 		rIo.InString( *rIo.pStrm, aPrompt );
/*N*/ 
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		*rIo.pStrm >> nSeqVal >> nSeqNo;
/*N*/ 
/*N*/ 	if( cFlags & 0x40 || !(cFlags & 0x20) )
/*N*/ 		rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 
/*N*/ 	SwSetExpField* pFld =
/*N*/ 		new SwSetExpField( (SwSetExpFieldType *)pType, aFormula, rFmt );
/*N*/ 
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 	{
/*?*/ 		pFld->SetInputFlag( TRUE );
/*?*/ 		pFld->SetPromptText( aPrompt );
/*N*/ 	}
/*N*/ 
/*N*/ 	pFld->SetSubType( nSubType );
/*N*/ 
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 	{
/*N*/ 		ASSERT( GSE_SEQ & ((SwSetExpFieldType *)pType)->GetType(),
/*N*/ 				"Kein Sequence-Number-Feld" );
/*N*/ 
/*N*/ 		pFld->SetValue( nSeqVal );
/*N*/ 		if( !(cFlags & 0x40) )
/*N*/ 			aExpand = ::binfilter::FormatNumber( nSeqVal, rFmt );
/*N*/ 
/*N*/ 		pFld->SetSeqNumber( nSeqNo );
/*N*/ 	}
/*N*/ 
/*N*/ 	pFld->ChgExpStr( aExpand );
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }


/*N*/ void lcl_sw3io_OutSetExpField40( Sw3IoImp& rIo, SwField *pFld)
/*N*/ {
/*N*/ 	USHORT nPoolId = USHRT_MAX;
/*N*/ 	const String& rName = ((SwSetExpField *)pFld)->GetTyp()->GetName();
/*N*/ 	if( GSE_SEQ & ((SwSetExpFieldType *)pFld->GetTyp())->GetType() )
/*N*/ 		nPoolId = lcl_sw3io_GetSetExpFieldPoolId( rName );
/*N*/ 
/*N*/ 	*rIo.pStrm << (UINT16)rIo.aStringPool.Find( rName, nPoolId );
/*N*/ 
/*N*/ 	BYTE cFlags = ((SwSetExpField *)pFld)->GetInputFlag() ? 0x10 : 0;
/*N*/ 	String sStr( ((SwSetExpField *)pFld)->GetExpStr() );
/*N*/ 
/*N*/ 	if( GSE_SEQ & ((SwSetExpFieldType *)pFld->GetTyp())->GetType() )
/*N*/ 	{
/*N*/ 		USHORT n = (USHORT)((SwSetExpField*)pFld)->GetValue();
/*N*/ 		sStr = ::binfilter::FormatNumber( n, SVX_NUM_ARABIC );
/*N*/ 		if( !rIo.IsSw31Export() )
/*N*/ 			cFlags |= 0x20;
/*N*/ 	}
/*N*/ 
/*N*/ 	*rIo.pStrm << cFlags;
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwSetExpField*)pFld)->GetFormula() );
/*N*/ 	rIo.OutString( *rIo.pStrm, sStr );
/*N*/ 
/*N*/ 	if( cFlags & 0x10 )
/*?*/ 		rIo.OutString( *rIo.pStrm, ((SwSetExpField *)pFld)->GetPromptText() );
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		*rIo.pStrm << (UINT16)((SwSetExpField *)pFld)->GetSeqNumber();
/*N*/ }

/*N*/ void lcl_sw3io_OutSetExpField( Sw3IoImp& rIo, SwField *pFld )
/*N*/ {
/*N*/ 	USHORT nPoolId = USHRT_MAX;
/*N*/ 	BYTE cFlags = ((SwSetExpField *)pFld)->GetInputFlag() ? 0x10 : 0;
/*N*/ 
/*N*/ 	const String& rName = ((SwSetExpField *)pFld)->GetTyp()->GetName();
/*N*/ 	if( ((SwSetExpField *)pFld)->IsSequenceFld() )
/*N*/ 	{
/*N*/ 		nPoolId = lcl_sw3io_GetSetExpFieldPoolId( rName );
/*N*/ 		cFlags |= 0x20;
/*N*/ 		if( MAXLEVEL > ((SwSetExpFieldType *)pFld->GetTyp())->GetOutlineLvl() )
/*N*/ 			cFlags |= 0x40;
/*N*/ 	}
/*N*/ 
/*N*/ 	*rIo.pStrm << cFlags
/*N*/ 			   << (UINT16)rIo.aStringPool.Find( rName, nPoolId );
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwSetExpField*)pFld)->GetFormula() );
/*N*/ 
/*N*/ 	if( cFlags & 0x10 )
/*N*/ 		rIo.OutString( *rIo.pStrm, ((SwSetExpField *)pFld)->GetPromptText() );
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		*rIo.pStrm << (UINT16)((SwSetExpField*)pFld)->GetValue()
/*N*/ 				   << (UINT16)((SwSetExpField *)pFld)->GetSeqNumber();
/*N*/ 
/*N*/ 	if( cFlags & 0x40 || !(cFlags & 0x20) )
/*N*/ 		rIo.OutString( *rIo.pStrm, ((SwSetExpField *)pFld)->GetExpStr() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InHiddenParaField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									  USHORT, UINT32& )
/*N*/ {
/*N*/ 	BYTE bHidden;
/*N*/ 	String aCond;
/*N*/ 	*rIo.pStrm >> bHidden;
/*N*/ 	rIo.InString( *rIo.pStrm, aCond );
/*N*/ 	SwHiddenParaField* pFld = new SwHiddenParaField( (SwHiddenParaFieldType*)pType, aCond );
/*N*/ 	pFld->SetHidden( (BOOL) bHidden );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutHiddenParaField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	*rIo.pStrm << (BYTE)((SwHiddenParaField*)pFld)->IsHidden();
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDocInfoField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT nSubType, UINT32& rFmt )
/*N*/ { 
/*N*/ 	UINT16 nSub;
/*N*/ 	*rIo.pStrm >> nSub;
/*N*/ 	nSubType |= nSub;
/*N*/ 
/*N*/ 	SwDocInfoField *pFld = new SwDocInfoField( (SwDocInfoFieldType*)pType,
/*N*/ 											   nSubType, rFmt );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDocInfoField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								   USHORT nSubType, UINT32& rFmt )
/*N*/ {
/*N*/ 	BYTE cFlags;
/*N*/ 	SwDocInfoField *pFld = new SwDocInfoField( (SwDocInfoFieldType*)pType,
/*N*/ 											   nSubType, rFmt );
/*N*/ 
/*N*/ 	String aContent;
/*N*/ 	*rIo.pStrm >> cFlags;
/*N*/ 	rIo.InString( *rIo.pStrm, aContent );
/*N*/ 	pFld->SetExpansion(aContent);
/*N*/ 	if( cFlags & 0x01 )
/*N*/ 	{
/*?*/ 		double dVal;
/*?*/ 		*rIo.pStrm >> dVal;
/*?*/ 		pFld->SetValue( dVal );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutDocInfoField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
            UINT16 nSubType = pFld->GetSubType();
            nSubType &= 0x00ff;

            *rIo.pStrm << nSubType;
/*N*/ }

/*N*/ void lcl_sw3io_OutDocInfoField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	BYTE cFlags = 0x00;
/*N*/ 	if( ((SwDocInfoField*)pFld)->IsFixed() )
/*N*/ 	{
/*?*/ 		USHORT nSub = pFld->GetSubType();
/*?*/ 		switch( nSub & 0x00ff )
/*?*/ 		{
/*?*/ 		case DI_EDIT:
/*?*/ 			cFlags = 0x01;
/*?*/ 			break;
/*?*/ 		case DI_CREATE:
/*?*/ 		case DI_CHANGE:
/*?*/ 		case DI_PRINT:
/*?*/ 			switch( nSub & ~(DI_SUB_FIXED|0x00ff) )
/*?*/ 			{
/*?*/ 			case DI_SUB_TIME:
/*?*/ 			case DI_SUB_DATE:
/*?*/ 				cFlags = 0x01;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	*rIo.pStrm << cFlags;
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->Expand() );
/*N*/ 	if( cFlags & 0x01 )
/*N*/ 		*rIo.pStrm << ((SwDocInfoField*)pFld)->GetValue();
/*N*/ }

/*N*/ SwField* lcl_sw3io_InTemplNameField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT, UINT32& rFmt )
/*N*/ {
/*N*/ 	return new SwTemplNameField( (SwTemplNameFieldType*)pType, rFmt );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDBNextSetField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aName, aCond, aDBName;
/*N*/ 	rIo.InString( *rIo.pStrm, aCond );
/*N*/ 	rIo.InString( *rIo.pStrm, aName );
/*N*/ 	if( rIo.IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aDBName = rIo.aStringPool.Find( nPoolId );
/*N*/ 	}
/*N*/ 	SwDBData aData;
/*N*/ 	aData.sDataSource = aDBName.GetToken(0, DB_DELIM);
/*N*/ 	aData.sCommand = aDBName.GetToken(1, DB_DELIM);
/*N*/ 	return new SwDBNextSetField( (SwDBNextSetFieldType*)pType, aCond, aName, aData );
/*N*/ }

/*N*/ void lcl_sw3io_OutDBNextSetField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar2() );
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		SwDBData aData(((SwDBNextSetField*)pFld)->GetRealDBData());
/*N*/ 		String sDBName(aData.sDataSource);
/*N*/ 		sDBName += DB_DELIM;
/*N*/ 		sDBName += (String)aData.sCommand;
/*N*/ 		*rIo.pStrm << (UINT16)rIo.aStringPool.Find( sDBName, IDX_NOCONV_FF );
/*N*/ 	}
/*N*/ }

// der 3.1-Writer hat beim Einlesen Condition und Number vertauscht.
// Deshalb exportieren wir diese beiden Werte vertauscht und lesen sie
// in der exportierten Version auch verkehrt herum wieder ein.

/*N*/ SwField* lcl_sw3io_InDBNumSetField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aNumber, aCond, aDBName;
/*N*/ 
/*N*/ 	if( rIo.IsVersion( SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*?*/ 		rIo.InString( *rIo.pStrm, aNumber );
/*?*/ 		rIo.InString( *rIo.pStrm, aCond );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rIo.InString( *rIo.pStrm, aCond );
/*N*/ 		rIo.InString( *rIo.pStrm, aNumber );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( rIo.IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aDBName = rIo.aStringPool.Find( nPoolId );
/*N*/ 	}
/*N*/ 	SwDBData aData;
/*N*/ 	aData.sDataSource = aDBName.GetToken(0, DB_DELIM);
/*N*/ 	aData.sCommand = aDBName.GetToken(1, DB_DELIM);
/*N*/ 	return new SwDBNumSetField( (SwDBNumSetFieldType*)pType, aCond, aNumber, aData );
/*N*/ }

/*N*/ void lcl_sw3io_OutDBNumSetField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	if( rIo.IsSw31Export() )
/*N*/ 	{
/*?*/ 		rIo.OutString( *rIo.pStrm, pFld->GetPar2() );
/*?*/ 		rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
/*N*/ 		rIo.OutString( *rIo.pStrm, pFld->GetPar2() );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		SwDBData aData(((SwDBNumSetField*)pFld)->GetRealDBData());
/*N*/ 		String sDBName(aData.sDataSource);
/*N*/ 		sDBName += DB_DELIM;
/*N*/ 		sDBName += (String)aData.sCommand;
/*N*/ 		*rIo.pStrm << (UINT16)rIo.aStringPool.Find( sDBName, IDX_NOCONV_FF );
/*N*/ 	}
/*N*/ }

/*N*/ SwField* lcl_sw3io_InDBSetNumberField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									   USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aDBName;
/*N*/ 	INT32 n;
/*N*/ 	*rIo.pStrm >> n;
/*N*/ 	if( rIo.IsVersion( SWG_MULTIDB, SWG_EXPORT31, SWG_DESKTOP40 ) )
/*N*/ 	{
/*N*/ 		USHORT nPoolId;
/*N*/ 		*rIo.pStrm >> nPoolId;
/*N*/ 		aDBName = rIo.aStringPool.Find( nPoolId );
/*N*/ 	}
/*N*/ 	SwDBData aData;
/*N*/ 	aData.sDataSource = aDBName.GetToken(0, DB_DELIM);
/*N*/ 	aData.sCommand = aDBName.GetToken(1, DB_DELIM);
/*N*/ 	SwDBSetNumberField* pFld = new SwDBSetNumberField( (SwDBSetNumberFieldType*)pType, aData );
/*N*/ 	pFld->SetSetNumber( n );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutDBSetNumberField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	*rIo.pStrm << (INT32) ((SwDBSetNumberField*)pFld)->GetSetNumber();
/*N*/ 
/*N*/ 	if( !rIo.IsSw31Export() )
/*N*/ 	{
/*N*/ 		SwDBData aData(((SwDBSetNumberField*)pFld)->GetRealDBData());
/*N*/ 		String sDBName(aData.sDataSource);
/*N*/ 		sDBName += DB_DELIM;
/*N*/ 		sDBName += (String)aData.sCommand;
/*N*/ 		*rIo.pStrm<< (UINT16)rIo.aStringPool.Find( sDBName, IDX_NOCONV_FF );
/*N*/ 	}
/*N*/ }

/*N*/ SwField* lcl_sw3io_InExtUserField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									 USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aData;
/*N*/ 	UINT16 nSubType;
/*N*/ 	rIo.InString( *rIo.pStrm, aData );
/*N*/ 	*rIo.pStrm >> nSubType;
/*N*/ 	SwExtUserField* pFld = new SwExtUserField( (SwExtUserFieldType*)pType, nSubType );
/*N*/ 	((SwExtUserFieldType*)pType)->SetData( aData );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InExtUserField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								   USHORT nSubType, UINT32& )
/*N*/ {
/*N*/ 	String aData;
/*N*/ 	rIo.InString( *rIo.pStrm, aData );
/*N*/ 
/*N*/ 	SwExtUserField* pFld = new SwExtUserField( (SwExtUserFieldType*)pType, nSubType );
/*N*/ 	((SwExtUserFieldType*)pType)->SetData( aData );
/*N*/ 
/*N*/ 	if( rIo.IsVersion( SWG_FIXEDFLDS ) )
/*N*/ 	{
/*N*/ 		String aExpand;
/*N*/ 		rIo.InString( *rIo.pStrm, aExpand );
/*N*/ 		pFld->SetExpansion( aExpand );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutExtUserField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	SwExtUserFieldType* pType = (SwExtUserFieldType*) pFld->GetTyp();
/*N*/ 	rIo.OutString( *rIo.pStrm, pType->GetData() );
/*N*/ 	*rIo.pStrm << (UINT16) pFld->GetSubType();
/*N*/ }

/*N*/ void lcl_sw3io_OutExtUserField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	SwExtUserFieldType* pType = (SwExtUserFieldType*) pFld->GetTyp();
/*N*/ 	rIo.OutString( *rIo.pStrm, pType->GetData() );
/*N*/ 	rIo.OutString( *rIo.pStrm, ((SwExtUserField *)pFld)->GetContent() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InRefPageSetField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									  USHORT, UINT32& )
/*N*/ {
            INT16 nOffset;
            BYTE cIsOn;
            *rIo.pStrm >> nOffset >> cIsOn;
            return new SwRefPageSetField( (SwRefPageSetFieldType*)pType, nOffset, cIsOn!=0 );
/*N*/ }

/*N*/ void lcl_sw3io_OutRefPageSetField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        *rIo.pStrm << (INT16)((SwRefPageSetField*)pFld)->GetOffset()
                   << (BYTE)((SwRefPageSetField*)pFld)->IsOn();
/*N*/ }

/*N*/ SwField* lcl_sw3io_InRefPageGetField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									  USHORT, UINT32& )
/*N*/ {
            String aString;
            SwRefPageGetField *pFld = new SwRefPageGetField( (SwRefPageGetFieldType*)pType, 0 );
            rIo.InString( *rIo.pStrm, aString );
            pFld->SetText( aString );
            return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutRefPageGetField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        rIo.OutString( *rIo.pStrm, ((SwRefPageGetField*)pFld)->GetText() );
/*N*/ }

/*N*/ SwField *lcl_sw3io_InINetField31( Sw3IoImp& rIo, SwFieldType *, USHORT, UINT32& )
/*N*/ {
/*N*/ 	ASSERT( !(rIo.pFmtINetFmt || rIo.aINetFldText.Len()),
/*N*/ 			"Da sind noch Rest-Infos vom INet-Feld!" );
/*N*/ 
/*N*/ 	String aURL, aText;
/*N*/ 	rIo.InString( *rIo.pStrm, aURL );
/*N*/ 	rIo.InString( *rIo.pStrm, rIo.aINetFldText );
/*N*/ 
/*N*/ 	// JP 10.04.96: aus rel. URLs wieder absolute machen!
/*N*/ 	aURL = ::binfilter::StaticBaseUrl::SmartRelToAbs( aURL );
/*N*/ 
/*N*/ 	String sTarget;
/*N*/ 	if( rIo.IsVersion( SWG_TARGETFRAME, SWG_EXPORT31 ) )
/*N*/ 	{
/*N*/ 		rIo.InString( *rIo.pStrm, sTarget );
/*N*/ 	}
/*N*/ 
/*N*/ 	rIo.pFmtINetFmt = new SwFmtINetFmt( aURL, sTarget );
/*N*/ 
/*N*/ 	if( rIo.IsVersion( SWG_INETMACROTAB, SWG_EXPORT31 ) )
/*N*/ 	{
/*?*/ 		USHORT nCnt;
/*?*/ 		*rIo.pStrm >> nCnt;
/*?*/ 
/*?*/ 		while( nCnt-- )
/*?*/ 		{
                USHORT nCurKey;
/*?*/           String aLibName, aMacName;
/*?*/           *rIo.pStrm >> nCurKey;
/*?*/           rIo.InString( *rIo.pStrm, aLibName );
/*?*/           rIo.InString( *rIo.pStrm, aMacName );
/*?*/           rIo.pFmtINetFmt->SetMacro( nCurKey, SvxMacro( aMacName, aLibName, STARBASIC ) );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InJumpEditField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									USHORT, UINT32& )
/*N*/ {
/*N*/ 	String aText, aHelp;
/*N*/ 	rIo.InString( *rIo.pStrm, aText );
/*N*/ 	rIo.InString( *rIo.pStrm, aHelp );
/*N*/ 	SwJumpEditField *pFld = new SwJumpEditField( (SwJumpEditFieldType*)pType, 0, aText, aHelp );
/*N*/ 	return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutJumpEditField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
/*N*/ 	rIo.OutString( *rIo.pStrm, pFld->GetPar2() );
/*N*/ }

/*N*/ SwField* lcl_sw3io_InScriptField40( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 									USHORT, UINT32& )
/*N*/ {
        String aType, aCode;
        BYTE cFlags = 0;
        rIo.InString( *rIo.pStrm, aType );
        rIo.InString( *rIo.pStrm, aCode );
 
        // Hier gab es mal eine Version SWG_SCRIPTURLS (0x0121), die jedoch
        // so gut wie keiner benutuzt hat, erst recht nicht mit Script-Feldern.
        // Deshalb wurde die geknickt.
        if( rIo.IsVersion( SWG_NEWFIELDS ) )
            *rIo.pStrm >> cFlags;
        else if( aCode.CompareIgnoreCaseToAscii( "// @url: ", 9 ) == COMPARE_EQUAL )
        {
            // HACK fuer die 363 (4.0 fixpack 2): Script-Links wurden
            // als spezieller Kommentar rausgeschrieben, weil es kein Flag zu
            // Unterscheidung von normalem Code gab.
            aCode.Erase( 0, 9 );
            cFlags = 0x01;
        }
        if( (cFlags & 0x01) != 0 )
            aCode = ::binfilter::StaticBaseUrl::SmartRelToAbs( aCode );
 
        SwScriptField *pFld = new SwScriptField( (SwScriptFieldType*)pType, aType, aCode,
                                                 (cFlags & 0x01) != 0 );
        return pFld;
/*N*/ }

/*N*/ SwField* lcl_sw3io_InScriptField( Sw3IoImp& rIo, SwFieldType* pType,
/*N*/ 								  USHORT, UINT32& )
/*N*/ {
        String aType, aCode;
        BYTE cFlags = 0;
        rIo.InString( *rIo.pStrm, aType );
        rIo.InString( *rIo.pStrm, aCode );
        *rIo.pStrm >> cFlags;

        if( (cFlags & 0x01) != 0 )
            aCode = ::binfilter::StaticBaseUrl::SmartRelToAbs( aCode );

        SwScriptField *pFld = new SwScriptField( (SwScriptFieldType*)pType, aType, aCode,
                                                 (cFlags & 0x01) != 0 );
        return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutScriptField40( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        ASSERT( !rIo.IsSw31Export(),
                "Wer will denn da ein Script-Feld exportieren" );

        BYTE cFlags = ((SwScriptField*)pFld)->IsCodeURL() ? 0x01 : 0x00;

        String aCode;
        if( ((SwScriptField*)pFld)->IsCodeURL() )
        {
            aCode.AssignAscii( "// @url: " );
            aCode += ::binfilter::StaticBaseUrl::AbsToRel( ((SwScriptField*)pFld)->GetCode() );
        }
        else
            aCode = ((SwScriptField*)pFld)->GetCode();

        rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
        rIo.OutString( *rIo.pStrm, aCode );
/*N*/ }

/*N*/ void lcl_sw3io_OutScriptField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
        ASSERT( !rIo.IsSw31Export(),
                "Wer will denn da ein Script-Feld exportieren" );

        BYTE cFlags = ((SwScriptField*)pFld)->IsCodeURL() ? 0x01 : 0x00;

        String aCode;
        if( ((SwScriptField*)pFld)->IsCodeURL() )
            aCode = ::binfilter::StaticBaseUrl::AbsToRel( ((SwScriptField*)pFld)->GetCode() );
        else
            aCode = ((SwScriptField*)pFld)->GetCode();

        rIo.OutString( *rIo.pStrm, pFld->GetPar1() );
        rIo.OutString( *rIo.pStrm, aCode );
        *rIo.pStrm << cFlags;
/*N*/ }

/*  */

/*N*/ SwField* lcl_sw3io_InAuthorityField( Sw3IoImp& rIo, SwFieldType*,
/*N*/ 								  USHORT, UINT32& )
/*N*/ {
        rIo.OpenFlagRec();

        UINT16 nPos;
        *rIo.pStrm >> nPos;

        rIo.CloseFlagRec();

        if( rIo.pAuthorityMap && nPos < rIo.pAuthorityMap->Count() )
            nPos = (*rIo.pAuthorityMap)[nPos];

        SwField *pFld = 0;
        SwFieldType* pType = rIo.pDoc->GetFldType( RES_AUTHORITY, aEmptyStr );
        ASSERT( pType, "missing authority field type" );
        if( pType )
        {
            long nHandle = ((SwAuthorityFieldType *)pType)->GetHandle( nPos );
            pFld = new SwAuthorityField( (SwAuthorityFieldType *)pType, nHandle );
        }

        return pFld;
/*N*/ }

/*N*/ void lcl_sw3io_OutAuthorityField( Sw3IoImp& rIo, SwField* pFld )
/*N*/ {
            BYTE cFlags = 0x02;
            *rIo.pStrm  << cFlags
                        << (UINT16)((SwAuthorityField *)pFld)->GetHandlePosition();
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// Die Beta-1-Version hatte noch eine eigene Kapselung fuer das Feld.
// Da dieser Inhalt als Teil eines SWG_ATTRIBUTE-Records vorkommt,ist
// dies reine Platzverschwendung und wurde geknickt.

#define	SWG_FIELD 'y'

typedef SwField *(*Sw3InFieldFn)( Sw3IoImp&, SwFieldType*, USHORT, UINT32& );

static Sw3InFieldFn aInFieldFnTbl40[] =
{
    &lcl_sw3io_InDBField40,        // RES_DBFLD
    &lcl_sw3io_InUserField40,      // RES_USERFLD
    &lcl_sw3io_InFileNameField,    // RES_FILENAMEFLD
    &lcl_sw3io_InDBNameField,      // RES_DBNAMEFLD
    &lcl_sw3io_InDateField40,      // RES_DATEFLD
    &lcl_sw3io_InTimeField40,      // RES_TIMEFLD
    &lcl_sw3io_InPageNumberField40,// RES_PAGENUMBERFLD
    &lcl_sw3io_InAuthorField,      // RES_AUTHORFLD
    &lcl_sw3io_InChapterField,     // RES_CHAPTERFLD
    &lcl_sw3io_InDocStatField40,   // RES_DOCSTATFLD
    &lcl_sw3io_InGetExpField40,    // RES_GETEXPFLD
    &lcl_sw3io_InSetExpField40,    // RES_SETEXPFLD
    &lcl_sw3io_InGetRefField40,    // RES_GETREFFLD
    &lcl_sw3io_InHiddenTxtField40, // RES_HIDDENTXTFLD
    &lcl_sw3io_InPostItField,      // RES_POSTITFLD
    &lcl_sw3io_InFixDateField40,   // RES_FIXDATEFLD
    &lcl_sw3io_InFixTimeField40,   // RES_FIXTIMEFLD
    0						,      // RES_REGFLD
    0						,      // RES_VARREGFLD
    0						,      // RES_SETREFFLD
    &lcl_sw3io_InInputField40,     // RES_INPUTFLD
    &lcl_sw3io_InMacroField,       // RES_MACROFLD
    &lcl_sw3io_InDDEField,         // RES_DDEFLD
    &lcl_sw3io_InTblField,         // RES_TABLEFLD
    &lcl_sw3io_InHiddenParaField,  // RES_HIDDENPARAFLD
    &lcl_sw3io_InDocInfoField40,   // RES_DOCINFOFLD
    &lcl_sw3io_InTemplNameField,   // RES_TEMPLNAMEFLD
    &lcl_sw3io_InDBNextSetField,   // RES_DBNEXTSETFLD
    &lcl_sw3io_InDBNumSetField,    // RES_DBNUMSETFLD
    &lcl_sw3io_InDBSetNumberField, // RES_DBSETNUMBERFLD
    &lcl_sw3io_InExtUserField40,   // RES_EXTUSERFLD
    &lcl_sw3io_InRefPageSetField,  // RES_REFPAGESETFLD
    &lcl_sw3io_InRefPageGetField,  // RES_REFPAGEGETFLD
    &lcl_sw3io_InINetField31,      // RES_INTERNETFLD
    &lcl_sw3io_InJumpEditField,    // RES_JUMPEDITFLD
    &lcl_sw3io_InScriptField40,    // RES_SCRIPTFLD
    0,                              // RES_DATETIMEFLD
    0,                             	// RES_AUTHORITY
    0,								// RES_COMBINED_CHARS
    0								// RES_DROPDOWN #108791#
};

static Sw3InFieldFn aInFieldFnTbl[] =
{
    &lcl_sw3io_InDBField,		   	// RES_DBFLD			   	 OK (3.1?)
    &lcl_sw3io_InUserField,	   		// RES_USERFLD			  	 OK (3.1?)
    &lcl_sw3io_InFileNameField,  	// RES_FILENAMEFLD		  	 unv.
    &lcl_sw3io_InDBNameField,	   	// RES_DBNAMEFLD		   	 unv.
    0,						   		// RES_DATEFLD			  	 OK (3.1?)
    0,                           	// RES_TIMEFLD			     OK (3.1?)
    &lcl_sw3io_InPageNumberField,	// RES_PAGENUMBERFLD	     OK (3.1?)
    &lcl_sw3io_InAuthorField,	   	// RES_AUTHORFLD		   	 unv.
    &lcl_sw3io_InChapterField,   	// RES_CHAPTERFLD		  	 unv.
    &lcl_sw3io_InDocStatField,   	// RES_DOCSTATFLD		  	 OK
    &lcl_sw3io_InGetExpField,   	// RES_GETEXPFLD		   	 OK (3.1?)
    &lcl_sw3io_InSetExpField,	   	// RES_SETEXPFLD		   	 OK (3.1?)
    &lcl_sw3io_InGetRefField,	   	// RES_GETREFFLD		   	 OK (3.1?)
    &lcl_sw3io_InHiddenTxtField, 	// RES_HIDDENTXTFLD		  	 OK
    &lcl_sw3io_InPostItField,	   	// RES_POSTITFLD		   	 unv.
    0,                           	// RES_FIXDATEFLD		     OK (3.1?)
    0,                           	// RES_FIXTIMEFLD		     OK (3.1?)
    0,							   	// RES_REGFLD			  	 ---
    0,							   	// RES_VARREGFLD 		  	 ---
    0,						    	// RES_SETREFFLD 		     ---
    &lcl_sw3io_InInputField,	   	// RES_INPUTFLD			  	 OK
    &lcl_sw3io_InMacroField,	  	// RES_MACROFLD			  	 unv.
    &lcl_sw3io_InDDEField,	   		// RES_DDEFLD			  	 unv.
    &lcl_sw3io_InTblField,	   		// RES_TABLEFLD              OK (3.1?)
    &lcl_sw3io_InHiddenParaField,	// RES_HIDDENPARAFLD	   	 unv.
    &lcl_sw3io_InDocInfoField,   	// RES_DOCINFOFLD		  	 OK
    &lcl_sw3io_InTemplNameField, 	// RES_TEMPLNAMEFLD		  	 unv.
    &lcl_sw3io_InDBNextSetField, 	// RES_DBNEXTSETFLD		  	 unv.
    &lcl_sw3io_InDBNumSetField,  	// RES_DBNUMSETFLD		  	 unv.
    &lcl_sw3io_InDBSetNumberField,	// RES_DBSETNUMBERFLD	     unv.
    &lcl_sw3io_InExtUserField,   	// RES_EXTUSERFLD		  	 OK
    &lcl_sw3io_InRefPageSetField,	// RES_REFPAGESETFLD	   	 unv.
    &lcl_sw3io_InRefPageGetField,	// RES_REFPAGEGETFLD	   	 unv.
    0,						   		// RES_INTERNETFLD		  	 unv.
    &lcl_sw3io_InJumpEditField,  	// RES_JUMPEDITFLD		  	 unv.
    &lcl_sw3io_InScriptField,	   	// RES_SCRIPTFLD		   	 OK
    &lcl_sw3io_InDateTimeField,  	// RES_DATETIMEFLD		     OK (3.1?)
    &lcl_sw3io_InAuthorityField,	// RES_AUTHORITY
    0,								// RES_COMBINED_CHARS
    0								// RES_DROPDOWN #108791#
};

/*N*/ SwField* Sw3IoImp::InField()
/*N*/ {
/*N*/ 	ASSERT( RES_FIELDS_END-RES_FIELDS_BEGIN ==
/*N*/ 						sizeof(aInFieldFnTbl) / sizeof(Sw3InFieldFn),
/*N*/ 			"Neues Feld? Und tschuess..." );
/*N*/ 	ASSERT( RES_FIELDS_END-RES_FIELDS_BEGIN ==
/*N*/ 						sizeof(aInFieldFnTbl40) / sizeof(Sw3InFieldFn),
/*N*/ 			"Neues Feld? Und tschuess..." );
/*N*/ 
/*N*/ 	BYTE cType = Peek();
/*N*/ 	if( cType == SWG_FIELD )
/*?*/ 		OpenRec( cType );
/*N*/ 	UINT16  nWhich;
/*N*/ 	UINT32  nFldFmt;
/*N*/ 	USHORT nSubType = 0;
/*N*/ 
/*N*/ 	*pStrm >> nWhich;
/*N*/ 	if( IsVersion(SWG_NEWERFIELDS) )
/*N*/ 	{
/*N*/ 		// 5.0
/*N*/ 		*pStrm >> nFldFmt >> nSubType;
/*N*/ 	}
/*N*/ 	else if( IsVersion(SWG_NEWFIELDS) )
/*N*/ 	{
/*N*/ 		// spaete 4.0 und 5.0 vor Umbau
/*?*/ 		*pStrm >> nFldFmt;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// 3.1 und 4.0
/*N*/ 		UINT16 nOldFldFmt;
/*N*/ 		*pStrm >> nOldFldFmt;
/*N*/ 		nFldFmt = nOldFldFmt;
/*N*/ 	}
/*N*/ 
/*N*/ 	nWhich += RES_FIELDS_BEGIN;
/*N*/ 
/*N*/ 	// Beim Import aelter Dokumente wird jetzt erstmal kraeftig
/*N*/ 	// an allem geschraubt.
/*N*/ 	UINT16 nRealWhich = nWhich;
/*N*/ 	if( !IsVersion(SWG_NEWFIELDS) )
/*N*/ 		sw3io_ConvertFromOldField( *pDoc, nRealWhich, nSubType, nFldFmt, nVersion );
/*N*/ 
/*N*/ 	SwField* pFld = NULL;
/*N*/ 	SwFieldType* pType = pDoc->GetSysFldType( (const RES_FIELDS)nRealWhich );
/*N*/ 
/*N*/ 	Sw3InFieldFn *pFnTbl;
/*N*/ 	if( IsVersion(SWG_NEWERFIELDS) )
/*N*/ 		pFnTbl = aInFieldFnTbl;
/*N*/ 	else
/*N*/ 		pFnTbl = aInFieldFnTbl40;
/*N*/ 	Sw3InFieldFn pFn =
/*N*/ 		(nWhich < RES_FIELDS_END) ? pFnTbl[nWhich-RES_FIELDS_BEGIN] : 0;
/*N*/ 
/*N*/ 	ASSERT( pFn, "unbekannte Feld-Which-Id" );
/*N*/ 	if( pFn )
/*N*/ 		pFld = (*pFn)( *this, pType, nSubType, nFldFmt );
/*N*/ 	else
/*?*/ 		Warning();
/*N*/ 
/*N*/ 	if( cType == SWG_FIELD )
/*N*/ 		CloseRec( cType );
/*N*/ 	if( pFld )
/*N*/ 		pFld->ChangeFormat( nFldFmt );
/*N*/ 
/*N*/ 	if( (bOrganizer || bPageDescs) && pFld && pFld->IsFixed() )
/*N*/ 	{
/*?*/ 		switch( nWhich )
/*?*/ 		{
/*?*/ 		case RES_DATETIMEFLD:
/*?*/             ((SwDateTimeField*)pFld)->SetDateTime( DateTime() );
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_EXTUSERFLD:
/*?*/ 			{
/*?*/ 				SwExtUserField* pExtUserFld = (SwExtUserField*)pFld;
/*?*/ 				pExtUserFld->SetExpansion( ((SwExtUserFieldType*)pType)->Expand(
/*?*/ 											pExtUserFld->GetSubType(),
/*?*/ 											pExtUserFld->GetFormat() ) );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_AUTHORFLD:
/*?*/ 			{
/*?*/ 				SwAuthorField* pAuthorFld = (SwAuthorField*)pFld;
/*?*/ 				pAuthorFld->SetExpansion( ((SwAuthorFieldType*)pType)->Expand(
/*?*/ 											pAuthorFld->GetFormat() ) );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_FILENAMEFLD:
/*?*/ 			{
/*?*/ 				SwFileNameField* pFileNameFld = (SwFileNameField*)pFld;
/*?*/ 				pFileNameFld->SetExpansion( ((SwFileNameFieldType*)pType)->Expand(
/*?*/ 											pFileNameFld->GetFormat() ) );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_DOCINFOFLD:
/*?*/ 			{
/*?*/ 				SwDocInfoField* pDocInfFld = (SwDocInfoField*)pFld;
/*?*/ 				pDocInfFld->SetExpansion( ((SwDocInfoFieldType*)pType)->Expand(
/*?*/ 											pDocInfFld->GetSubType(),
/*?*/ 											pDocInfFld->GetFormat(),
/*?*/ 											pDocInfFld->GetLanguage() ) );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }

typedef void (*Sw3OutFieldFn)( Sw3IoImp&, SwField* );

static Sw3OutFieldFn aOutFieldFnTbl40[] =
{
    &lcl_sw3io_OutDBField40,		    // RES_DBFLD
    &lcl_sw3io_OutUserField,            // RES_USERFLD
    0,                                  // RES_FILENAMEFLD
    &lcl_sw3io_OutDBNameField,          // RES_DBNAMEFLD
    0,                                  // RES_DATEFLD
    0,                                  // RES_TIMEFLD
    &lcl_sw3io_OutPageNumberField40,    // RES_PAGENUMBERFLD
    0,                                  // RES_AUTHORFLD
    &lcl_sw3io_OutChapterField,         // RES_CHAPTERFLD
    &lcl_sw3io_OutDocStatField40,       // RES_DOCSTATFLD
    &lcl_sw3io_OutGetExpField40,        // RES_GETEXPFLD
    &lcl_sw3io_OutSetExpField40,        // RES_SETEXPFLD
    &lcl_sw3io_OutGetRefField40,        // RES_GETREFFLD
    &lcl_sw3io_OutHiddenTxtField40,     // RES_HIDDENTXTFLD
    &lcl_sw3io_OutPostItField,          // RES_POSTITFLD
    &lcl_sw3io_OutFixDateField40,       // RES_FIXDATEFLD
    &lcl_sw3io_OutFixTimeField40,       // RES_FIXTIMEFLD
    0,         						    // RES_REGFLD
    0,         						    // RES_VARREGFLD
    0,         						    // RES_SETREFFLD
    &lcl_sw3io_OutInputField40,         // RES_INPUTFLD
    &lcl_sw3io_OutMacroField,           // RES_MACROFLD
    &lcl_sw3io_OutDDEField,             // RES_DDEFLD
    &lcl_sw3io_OutTblField,             // RES_TABLEFLD
    &lcl_sw3io_OutHiddenParaField,      // RES_HIDDENPARAFLD
    &lcl_sw3io_OutDocInfoField40,       // RES_DOCINFOFLD
    0,           						// RES_TEMPLNAMEFLD
    &lcl_sw3io_OutDBNextSetField,       // RES_DBNEXTSETFLD
    &lcl_sw3io_OutDBNumSetField,        // RES_DBNUMSETFLD
    &lcl_sw3io_OutDBSetNumberField,     // RES_DBSETNUMBERFLD
    &lcl_sw3io_OutExtUserField40,       // RES_EXTUSERFLD
    &lcl_sw3io_OutRefPageSetField,      // RES_REFPAGESETFLD
    &lcl_sw3io_OutRefPageGetField,      // RES_REFPAGEGETFLD
    0,         						    // RES_INTERNETFLD
    &lcl_sw3io_OutJumpEditField,        // RES_JUMPEDITFLD
    &lcl_sw3io_OutScriptField40,        // RES_SCRIPTFLD
    0,                              	// RES_DATETIMEFLD
    0,									// RES_AUTHORITY
    0,									// RES_COMBINED_CHARS
    0									// RES_DROPDOWN #108791#
};

static Sw3OutFieldFn aOutFieldFnTbl[] =
{
    &lcl_sw3io_OutDBField,              // RES_DBFLD
    &lcl_sw3io_OutUserField,            // RES_USERFLD
    &lcl_sw3io_OutFileNameField,        // RES_FILENAMEFLD
    &lcl_sw3io_OutDBNameField,          // RES_DBNAMEFLD
    0,                                  // RES_DATEFLD
    0,                                  // RES_TIMEFLD
    &lcl_sw3io_OutPageNumberField,      // RES_PAGENUMBERFLD
    &lcl_sw3io_OutAuthorField,          // RES_AUTHORFLD
    &lcl_sw3io_OutChapterField,         // RES_CHAPTERFLD
    0,                                  // RES_DOCSTATFLD
    &lcl_sw3io_OutGetExpField,          // RES_GETEXPFLD
    &lcl_sw3io_OutSetExpField,          // RES_SETEXPFLD
    &lcl_sw3io_OutGetRefField,          // RES_GETREFFLD
    &lcl_sw3io_OutHiddenTxtField,       // RES_HIDDENTXTFLD
    &lcl_sw3io_OutPostItField,          // RES_POSTITFLD
    0,                                  // RES_FIXDATEFLD
    0,                                  // RES_FIXTIMEFLD
    0,      						    // RES_REGFLD
    0,      						    // RES_VARREGFLD
    0,    						        // RES_SETREFFLD
    &lcl_sw3io_OutInputField,           // RES_INPUTFLD
    &lcl_sw3io_OutMacroField,           // RES_MACROFLD
    &lcl_sw3io_OutDDEField,             // RES_DDEFLD
    &lcl_sw3io_OutTblField,             // RES_TABLEFLD
    &lcl_sw3io_OutHiddenParaField,      // RES_HIDDENPARAFLD
    &lcl_sw3io_OutDocInfoField,         // RES_DOCINFOFLD
    0,						            // RES_TEMPLNAMEFLD
    &lcl_sw3io_OutDBNextSetField,       // RES_DBNEXTSETFLD
    &lcl_sw3io_OutDBNumSetField,        // RES_DBNUMSETFLD
    &lcl_sw3io_OutDBSetNumberField,     // RES_DBSETNUMBERFLD
    &lcl_sw3io_OutExtUserField,         // RES_EXTUSERFLD
    &lcl_sw3io_OutRefPageSetField,      // RES_REFPAGESETFLD
    &lcl_sw3io_OutRefPageGetField,      // RES_REFPAGEGETFLD
    0,      						    // RES_INTERNETFLD
    &lcl_sw3io_OutJumpEditField,        // RES_JUMPEDITFLD
    &lcl_sw3io_OutScriptField,          // RES_SCRIPTFLD
    &lcl_sw3io_OutDateTimeField,         // RES_DATETIMEFLD
    &lcl_sw3io_OutAuthorityField,		// RES_AUTHORITY
    0,									// RES_COMBINED_CHARS
    0									// RES_DROPDOWN #108791#
};


/*N*/ void lcl_sw3io_OutAnySetExpField40( Sw3IoImp& rIo, const sal_Char *pName,
/*N*/ 									const String& rExpand )
/*N*/ {
/*N*/ 	SwSetExpFieldType aType( rIo.pDoc, String::CreateFromAscii(pName),
/*N*/ 							 GSE_STRING );
/*N*/ 	SwSetExpField aFld( &aType, rExpand );
/*N*/ 	aFld.ChgExpStr( rExpand );
/*N*/ 	*rIo.pStrm << (UINT16) ( RES_SETEXPFLD - RES_FIELDS_BEGIN ); // Which
/*N*/ 	*rIo.pStrm << (UINT16) 1; // Format: GSE_STRING
/*N*/ 	lcl_sw3io_OutSetExpField40( rIo, &aFld );
/*N*/ }

/* #108791# */
/*N*/ void lcl_sw3io_OutAnySetExpField( Sw3IoImp& rIo, const sal_Char *pName,
/*N*/ 									const String& rExpand )
/*N*/ {
/*N*/ 	SwSetExpFieldType aType( rIo.pDoc, String::CreateFromAscii(pName),
/*N*/ 							 GSE_STRING );
/*N*/ 	SwSetExpField aFld( &aType, rExpand );
/*N*/ 	aFld.ChgExpStr( rExpand );
/*N*/ 	*rIo.pStrm << (UINT16) ( RES_SETEXPFLD - RES_FIELDS_BEGIN ) // Which
/*N*/                << (UINT32) 1 // Format: GSE_STRING
/*N*/ 			   << (UINT16)aFld.GetSubType();
/*N*/ 	lcl_sw3io_OutSetExpField( rIo, &aFld );
/*N*/ }

/*N*/ void Sw3IoImp::OutField( const SwFmtFld& rAttr )
/*N*/ {
/*N*/ 	ASSERT( RES_FIELDS_END-RES_FIELDS_BEGIN ==
/*N*/ 						sizeof(aOutFieldFnTbl) / sizeof(Sw3OutFieldFn),
/*N*/ 			"Neues Feld? Und tschuess..." );
/*N*/ 	ASSERT( RES_FIELDS_END-RES_FIELDS_BEGIN ==
/*N*/ 						sizeof(aOutFieldFnTbl40) / sizeof(Sw3OutFieldFn),
/*N*/ 			"Neues Feld? Und tschuess..." );
/*N*/ 
/*N*/ 	const SwField* pFld = rAttr.GetFld();
/*N*/ 	ASSERT( pFld, "SWG-Writer: SwAttrFld-Hint ohne Inhalt!" );
/*N*/ 	if( !pFld )
/*N*/ 		return;
/*N*/ 
/*N*/ 	SwFieldType* pType = pFld->GetTyp();
/*N*/ 	if( !pType )
/*N*/ 		return;
/*N*/ 
/*N*/ 	UINT16 nWhich = pType->Which();
/*N*/ 	sal_uInt32 nFmt = pFld->GetFormat();
/*N*/ 
/*N*/ 	if( SOFFICE_FILEFORMAT_40 >= pStrm->GetVersion() )
/*N*/ 	{
/*N*/ 		const sal_Char *pExportSetExpFld = 0;
/*N*/ 		switch( nWhich )
/*N*/ 		{
/*N*/ 		case RES_DOCINFOFLD:
/*N*/ 			if( ((SwDocInfoField*)pFld)->IsFixed() )
/*N*/ 				pExportSetExpFld = sSW3IO_FixedField;
/*N*/ 			break;
/*N*/ 		case RES_AUTHORFLD:
/*N*/ 			if( ((SwAuthorField*)pFld)->IsFixed() )
/*N*/ 				pExportSetExpFld = sSW3IO_FixedField;
/*N*/ 			break;
/*N*/ 		case RES_EXTUSERFLD:
/*N*/ 			if( ((SwExtUserField*)pFld)->IsFixed() )
/*N*/ 				pExportSetExpFld = sSW3IO_FixedField;
/*N*/ 			break;
/*N*/ 		case RES_FILENAMEFLD:
/*N*/ 			if( ((SwFileNameField*)pFld)->IsFixed() )
/*N*/ 				pExportSetExpFld = sSW3IO_FixedField;
/*N*/ 			break;
/*N*/ 		case RES_AUTHORITY:
/*N*/ 			pExportSetExpFld = sSW3IO_AuthorityField;
/*N*/ 			break;
/*N*/ 
/*N*/             /* #108791# */
/*N*/ 		case RES_DROPDOWN:
/*N*/ 			pExportSetExpFld = sSW3IO_DropDownField;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pExportSetExpFld )
/*N*/ 		{
/*N*/ 			lcl_sw3io_OutAnySetExpField40( *this, pExportSetExpFld,
/*N*/ 				   						   pFld->Expand() );
/*N*/ 			return;
/*N*/ 		}
/*N*/ 
/*N*/ 		 // Format wandeln
/*N*/ 		sw3io_ConvertToOldField( pFld, nWhich, nFmt, pStrm->GetVersion() );
/*N*/ 	}
/*N*/     /* #108791# */
/*N*/     else if (RES_DROPDOWN == nWhich)
/*N*/     {
/*N*/         lcl_sw3io_OutAnySetExpField( *this, sSW3IO_DropDownField,
/*N*/                                      pFld->Expand() );
/*N*/ 
/*N*/         return;        
/*N*/     }
/*N*/ 
/*N*/ 	// nField koennte von sw3io_GetOldFieldFormat geaendert worden sein
/*N*/ 	*pStrm << (UINT16) ( nWhich - RES_FIELDS_BEGIN );
/*N*/ 
/*N*/ 	if( SOFFICE_FILEFORMAT_40 >= pStrm->GetVersion() )
/*N*/ 	{
/*N*/ 		*pStrm << (UINT16) nFmt;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		*pStrm << (UINT32) nFmt
/*N*/ 			   << (UINT16)pFld->GetSubType();
/*N*/ 	}
/*N*/ 
/*N*/ 	Sw3OutFieldFn *pFnTbl;
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 		pFnTbl = aOutFieldFnTbl40;
/*N*/ 	else
/*N*/ 		pFnTbl = aOutFieldFnTbl;
/*N*/ 
/*N*/ 	Sw3OutFieldFn pFn = pFnTbl[nWhich-RES_FIELDS_BEGIN];
/*N*/ 
/*N*/ 	if( pFn )
/*N*/ 		(*pFn)( *this, (SwField *)pFld );
/*N*/ }

// Ausgabe aller Feldtypen, die keine Systemtypen sind


/*N*/ BOOL lcl_sw3io_HasFixedFields40( Sw3IoImp& rIo, USHORT nWhich )
/*N*/ {
/*N*/ 	SwFieldType* pFldType = rIo.pDoc->GetSysFldType( nWhich );
/*N*/ 
/*N*/ 	SwClientIter aIter( *pFldType );
/*N*/ 	for( SwFmtFld* pFmtFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
/*N*/ 		 pFmtFld;
/*N*/ 		 pFmtFld = (SwFmtFld*)aIter.Next() )
/*N*/ 	{
/*N*/ 		const SwField *pFld = pFmtFld->GetFld();
/*N*/ 		BOOL bFixed = FALSE;
/*N*/ 		switch( nWhich )
/*N*/ 		{
/*?*/ 		case RES_DOCINFOFLD:
/*?*/ 			bFixed = ((SwDocInfoField*)pFld)->IsFixed();
/*?*/ 			break;
/*?*/ 		case RES_AUTHORFLD:
/*?*/ 			bFixed = ((SwAuthorField*)pFld)->IsFixed();
/*?*/ 			break;
/*N*/ 		case RES_EXTUSERFLD:
/*N*/ 			bFixed = ((SwExtUserField*)pFld)->IsFixed();
/*N*/ 			break;
/*?*/ 		case RES_FILENAMEFLD:
/*?*/ 			bFixed = ((SwFileNameField*)pFld)->IsFixed();
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		const SwTxtFld *pTxtFld = pFmtFld->GetTxtFld();
/*N*/ 		if( bFixed && pTxtFld && pTxtFld->GetpTxtNode() &&
/*N*/ 			pTxtFld->GetpTxtNode()->GetNodes().IsDocNodes() )
/*N*/ 		{
/*N*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void Sw3IoImp::OutFieldTypes()
/*N*/ {
/*N*/ 	const SwFldTypes* p = pDoc->GetFldTypes();
/*N*/ 	for( USHORT i=INIT_FLDTYPES-INIT_SEQ_FLDTYPES; i<p->Count(); ++i )
/*N*/ 		if( !OutFieldType( *(*p)[ i ] ) )
/*N*/ 			break;
/*N*/ 
/*N*/ 	// Fixe DocInfo Author und ExtUser-Felder in SetExpFields wandeln
/*N*/ 	String sFixedField( String::CreateFromAscii(sSW3IO_FixedField) );
/*N*/ 	if( pStrm->GetVersion() <= SOFFICE_FILEFORMAT_40 &&
/*N*/ 		!pDoc->GetFldType( RES_SETEXPFLD, sFixedField ) &&
/*N*/ 		( lcl_sw3io_HasFixedFields40( *this, RES_DOCINFOFLD ) ||
/*N*/ 		  lcl_sw3io_HasFixedFields40( *this, RES_AUTHORFLD ) ||
/*N*/ 		  lcl_sw3io_HasFixedFields40( *this, RES_EXTUSERFLD ) ||
/*N*/ 		  lcl_sw3io_HasFixedFields40( *this, RES_FILENAMEFLD ) ) )
/*N*/ 	{
/*?*/ 		SwSetExpFieldType aType( pDoc, sFixedField, GSE_STRING );
/*?*/ 		OutFieldType( aType );
/*N*/ 	}
/*N*/ 	{
/*N*/         /* #108791# */
/*N*/         String sDropDown( String::CreateFromAscii(sSW3IO_DropDownField) );
/*N*/ 		SwSetExpFieldType aType( pDoc, sDropDown, GSE_STRING );
/*N*/ 		OutFieldType( aType );
/*N*/ 	}
/*N*/ }


/*N*/ SwFieldType* Sw3IoImp::InFieldType()
/*N*/ {
/*N*/ 	BYTE ch;
/*N*/ 	SwFieldType* p = NULL;
/*N*/ 	OpenRec( SWG_FIELDTYPE );
/*N*/ 	*pStrm >> ch;
/*N*/ 	ch += RES_FIELDS_BEGIN;
/*N*/ 	switch( ch )
/*N*/ 	{
/*N*/ 		case RES_DBFLD:
/*N*/ 			p = lcl_sw3io_InDBFieldType( *this ); break;
/*N*/ 		case RES_USERFLD:
/*N*/ 			if( IsVersion(SWG_NEWERFIELDS) )
/*N*/ 				p = lcl_sw3io_InUserFieldType( *this );
/*N*/ 			else
/*N*/ 				p = lcl_sw3io_InUserFieldType40( *this );
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_DDEFLD:
/*N*/ 			p = lcl_sw3io_InDDEFieldType( *this ); break;
/*N*/ 		case RES_SETEXPFLD:
/*N*/ 			p = lcl_sw3io_InSetExpFieldType( *this ); break;
/*?*/ 		case RES_AUTHORITY:
/*?*/          p = lcl_sw3io_InAuthorityFieldType( *this ); break;
/*?*/ 		default:
/*?*/ 			Warning(); break;
/*N*/ 	}
/*N*/ 	CloseRec( SWG_FIELDTYPE );
/*N*/ 	return p;
/*N*/ }

// Schreiben eines Feldtyps



/*N*/ BOOL Sw3IoImp::OutFieldType( const SwFieldType& rType )
/*N*/ {
/*N*/ 	USHORT nFld = rType.Which();
/*N*/ 
/*N*/ 	if( IsSw31Or40Export() && RES_AUTHORITY == nFld )
/*N*/ 	{
/*N*/ 		SwSetExpFieldType aType( pDoc,
/*N*/ 								 String::CreateFromAscii(sSW3IO_AuthorityField),
/*N*/ 								 GSE_STRING );
/*N*/ 		return OutFieldType( aType );
/*N*/ 	}
/*N*/     /* #108791# */
/*N*/     else if (RES_DROPDOWN == nFld)
/*N*/     {
/*N*/         SwSetExpFieldType aType( pDoc,
/*N*/                                  String::CreateFromAscii(sSW3IO_DropDownField),
/*N*/                                  GSE_STRING );
/*N*/         return OutFieldType( aType );
/*N*/     }
/*N*/ 
/*N*/ 	OpenRec( SWG_FIELDTYPE );
/*N*/ 
/*N*/ 	*pStrm << (BYTE) ( nFld - RES_FIELDS_BEGIN );
/*N*/ 
/*N*/ 	switch( nFld )
/*N*/ 	{
/*N*/ 		case RES_DBFLD:
/*N*/ 			lcl_sw3io_OutDBFieldType( *this, (SwDBFieldType*) &rType );
/*N*/ 			break;
/*N*/ 		case RES_USERFLD:
/*N*/ 			if( IsSw31Or40Export() )
/*N*/ 				lcl_sw3io_OutUserFieldType40( *this, (SwUserFieldType*) &rType );
/*N*/ 			else
/*N*/ 				lcl_sw3io_OutUserFieldType( *this, (SwUserFieldType*) &rType );
/*N*/ 			break;
/*N*/ 		case RES_DDEFLD:
/*N*/ 			lcl_sw3io_OutDDEFieldType( *this, (SwDDEFieldType*) &rType );
/*N*/ 			nFileFlags |= SWGF_HAS_DDELNK;
/*N*/ 			break;
/*N*/ 		case RES_SETEXPFLD:
/*N*/ 			lcl_sw3io_OutSetExpFieldType( *this, (SwSetExpFieldType*) &rType );
/*N*/ 			break;
/*N*/ 		case RES_AUTHORITY:
/*N*/ 			lcl_sw3io_OutAuthorityFieldType( *this,
/*N*/ 											(SwAuthorityFieldType *)&rType );
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 			ASSERT( !this, "Unbekannter Feldtyp" );
/*N*/ 			CloseRec( SWG_FIELDTYPE );
/*N*/ 			Error();
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	CloseRec( SWG_FIELDTYPE );
/*N*/ 	return TRUE;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
