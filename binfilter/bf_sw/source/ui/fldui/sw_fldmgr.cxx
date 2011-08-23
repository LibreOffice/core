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

#include <cmdid.h>
#include <hintids.hxx>

#include <horiornt.hxx>

#include <dbmgr.hxx>
#include <shellres.hxx>
#include <docufld.hxx>
#include <fldmgr.hxx>
#include <fldui.hrc>
namespace binfilter {
using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::sdbc;

/*--------------------------------------------------------------------
    Beschreibung: Gruppen der Felder
 --------------------------------------------------------------------*/
/*N*/ enum
/*N*/ {
/*N*/ 	GRP_DOC_BEGIN	=  0,
/*N*/ 	GRP_DOC_END   	=  GRP_DOC_BEGIN + 11,
/*N*/
/*N*/ 	GRP_FKT_BEGIN 	=  GRP_DOC_END,
    GRP_FKT_END     =  GRP_FKT_BEGIN + 8,
/*N*/
/*N*/ 	GRP_REF_BEGIN 	=  GRP_FKT_END,
/*N*/ 	GRP_REF_END   	=  GRP_REF_BEGIN + 2,
/*N*/
/*N*/ 	GRP_REG_BEGIN 	=  GRP_REF_END,
/*N*/ 	GRP_REG_END     =  GRP_REG_BEGIN + 1,
/*N*/
/*N*/ 	GRP_DB_BEGIN  	=  GRP_REG_END,
/*N*/ 	GRP_DB_END    	=  GRP_DB_BEGIN  + 5,
/*N*/
/*N*/ 	GRP_VAR_BEGIN 	=  GRP_DB_END,
/*N*/ 	GRP_VAR_END   	=  GRP_VAR_BEGIN + 9
/*N*/ };
/*N*/
/*N*/ enum
/*N*/ {
/*N*/ 	GRP_WEB_DOC_BEGIN	=  0,
/*N*/ 	GRP_WEB_DOC_END   	=  GRP_WEB_DOC_BEGIN + 9,
/*N*/
/*N*/ 	GRP_WEB_FKT_BEGIN 	=  GRP_WEB_DOC_END + 2,
/*N*/ 	GRP_WEB_FKT_END   	=  GRP_WEB_FKT_BEGIN + 0,	// Die Gruppe ist leer!
/*N*/
/*N*/ 	GRP_WEB_REF_BEGIN 	=  GRP_WEB_FKT_END + 6,		// Die Gruppe ist leer!
/*N*/ 	GRP_WEB_REF_END   	=  GRP_WEB_REF_BEGIN + 0,
/*N*/
/*N*/ 	GRP_WEB_REG_BEGIN 	=  GRP_WEB_REF_END + 2,
/*N*/ 	GRP_WEB_REG_END     =  GRP_WEB_REG_BEGIN + 1,
/*N*/
/*N*/ 	GRP_WEB_DB_BEGIN  	=  GRP_WEB_REG_END,			// Die Gruppe ist leer!
/*N*/ 	GRP_WEB_DB_END    	=  GRP_WEB_DB_BEGIN  + 0,
/*N*/
/*N*/ 	GRP_WEB_VAR_BEGIN 	=  GRP_WEB_DB_END + 5,
/*N*/ 	GRP_WEB_VAR_END   	=  GRP_WEB_VAR_BEGIN + 1
/*N*/ };

/*--------------------------------------------------------------------
     Beschreibung: Formate in der richtigen Reihenfolge
--------------------------------------------------------------------*/
/*N*/ static const USHORT __FAR_DATA aSetFmt[] =
/*N*/ {
/*N*/ // die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_SETVAR_???
/*N*/ // uebereinstimmen
/*N*/ 	0,
/*N*/ 	0
/*N*/ };
/*N*/
/*N*/ static const USHORT __FAR_DATA aGetFmt[] =
/*N*/ {
/*N*/ // die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_GETVAR_???
/*N*/ // uebereinstimmen
/*N*/ 	0
/*N*/ };
/*N*/
/*N*/ static const USHORT __FAR_DATA aUsrFmt[] =
/*N*/ {
/*N*/ // die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_SETVAR_???
/*N*/ // uebereinstimmen
/*N*/ 	0,
/*N*/ 	SUB_CMD
/*N*/ };
/*N*/
/*N*/ static const USHORT __FAR_DATA aDBFmt[] =
/*N*/ {
/*N*/ // die Reihenfolge muss zu Beginn mit den ResourceIds fuer FMT_DBFLD_???
/*N*/ // uebereinstimmen
/*N*/ 	SUB_OWN_FMT
/*N*/ };
/*N*/
/*N*/ static const USHORT VF_COUNT		= sizeof(aGetFmt) / sizeof(USHORT);
/*N*/ static const USHORT VF_USR_COUNT	= sizeof(aUsrFmt) / sizeof(USHORT);
/*N*/ static const USHORT VF_DB_COUNT		= sizeof(aDBFmt)  / sizeof(USHORT);
/*N*/
/*--------------------------------------------------------------------
    Beschreibung: Feldtypen und Subtypes
 --------------------------------------------------------------------*/
struct SwFldPack
{
    USHORT  nTypeId;

    USHORT	nSubTypeStart;
    USHORT  nSubTypeEnd;

    ULONG	nFmtBegin;
    ULONG	nFmtEnd;
};

/*--------------------------------------------------------------------
    Beschreibung: Strings und Formate
 --------------------------------------------------------------------*/
static const SwFldPack __FAR_DATA aSwFlds[] =
{
    // Dokument
    TYP_EXTUSERFLD,	 	FLD_EU_BEGIN,	 	FLD_EU_END,	    0,     				0,
    TYP_AUTHORFLD,	 	0,					0,				FMT_AUTHOR_BEGIN,	FMT_AUTHOR_END,
    TYP_DATEFLD,		FLD_DATE_BEGIN,		FLD_DATE_END,	0,					0,
    TYP_TIMEFLD,		FLD_TIME_BEGIN,		FLD_TIME_END,	0,					0,
    TYP_PAGENUMBERFLD,	0,					0,				FMT_NUM_BEGIN,   	FMT_NUM_END-1,
    TYP_NEXTPAGEFLD,	0,					0,				FMT_NUM_BEGIN,		FMT_NUM_END,
    TYP_PREVPAGEFLD,	0,					0,				FMT_NUM_BEGIN,		FMT_NUM_END,
    TYP_FILENAMEFLD,	0,					0,				FMT_FF_BEGIN,    	FMT_FF_END,
    TYP_DOCSTATFLD,	 	FLD_STAT_BEGIN,		FLD_STAT_END,	FMT_NUM_BEGIN,		FMT_NUM_END-1,

    TYP_CHAPTERFLD,	 	0,					0,				FMT_CHAPTER_BEGIN, 	FMT_CHAPTER_END,
    TYP_TEMPLNAMEFLD,	0,					0,				FMT_FF_BEGIN,    	FMT_FF_END,

    // Funktion
    TYP_CONDTXTFLD,	 	0,					0,				0,					0,
    TYP_DROPDOWN,       0,                  0,              0,                  0,
    TYP_INPUTFLD,		FLD_INPUT_BEGIN,	FLD_INPUT_END,	0,   				0,
    TYP_MACROFLD,		0,					0,				0,					0,
    TYP_JUMPEDITFLD, 	0,					0,				FMT_MARK_BEGIN,		FMT_MARK_END,
    TYP_COMBINED_CHARS,	 0,					0,				0,					0,
    TYP_HIDDENTXTFLD,	0,					0,				0,					0,
    TYP_HIDDENPARAFLD, 	0,			   		0,				0,					0,

    // Referenzen
    TYP_SETREFFLD,	 	0,					0,				0,					0,
    TYP_GETREFFLD,	 	0,					0,				FMT_REF_BEGIN,		FMT_REF_END,

    // Ablage
    TYP_DOCINFOFLD,	 	0,					0,				FMT_REG_BEGIN, 		FMT_REG_END,

    // Datenbank
    TYP_DBFLD,		 	0,					0,				FMT_DBFLD_BEGIN,	FMT_DBFLD_END,
    TYP_DBNEXTSETFLD,	0,					0,				0,					0,
    TYP_DBNUMSETFLD,	0,					0,				0,					0,
    TYP_DBSETNUMBERFLD, 0,					0,				FMT_NUM_BEGIN,	    FMT_NUM_END-2,
    TYP_DBNAMEFLD, 	 	0,					0,				0,					0,

    // Variablen
    TYP_SETFLD,		 	0,					0,				FMT_SETVAR_BEGIN,   FMT_SETVAR_END,

    TYP_GETFLD,		 	0,					0, 				FMT_GETVAR_BEGIN,	FMT_GETVAR_END,
    TYP_DDEFLD,		 	0,					0,				FMT_DDE_BEGIN, 		FMT_DDE_END,
    TYP_FORMELFLD,  	0,					0,				FMT_GETVAR_BEGIN,	FMT_GETVAR_END,
    TYP_INPUTFLD,		FLD_INPUT_BEGIN,	FLD_INPUT_END,	0,   				0,
    TYP_SEQFLD,		 	0,					0,				FMT_NUM_BEGIN,		FMT_NUM_END-2,
    TYP_SETREFPAGEFLD,	FLD_PAGEREF_BEGIN,	FLD_PAGEREF_END,0,   				0,
    TYP_GETREFPAGEFLD,	0,					0,				FMT_NUM_BEGIN,   	FMT_NUM_END-1,
    TYP_USERFLD,		0,					0,				FMT_USERVAR_BEGIN,	FMT_USERVAR_END
};

String* SwFldMgr::pDate = 0;
String* SwFldMgr::pTime = 0;

/*N*/ void SwFieldType::_GetFldName()
/*N*/ {
/*N*/ 	static const USHORT coFldCnt = STR_TYPE_END	- STR_TYPE_BEGIN;
/*N*/
/*N*/ 	static USHORT __READONLY_DATA coFldNms[ coFldCnt ] = {
/*N*/ 		FLD_DATE_STD,
/*N*/ 		FLD_TIME_STD,
/*N*/ 		STR_FILENAMEFLD,
/*N*/ 		STR_DBNAMEFLD,
/*N*/ 		STR_CHAPTERFLD,
/*N*/ 		STR_PAGENUMBERFLD,
/*N*/ 		STR_DOCSTATFLD,
/*N*/ 		STR_AUTHORFLD,
/*N*/ 		STR_SETFLD,
/*N*/ 		STR_GETFLD,
/*N*/ 		STR_FORMELFLD,
/*N*/ 		STR_HIDDENTXTFLD,
/*N*/ 		STR_SETREFFLD,
/*N*/ 		STR_GETREFFLD,
/*N*/ 		STR_DDEFLD,
/*N*/ 		STR_MACROFLD,
/*N*/ 		STR_INPUTFLD,
/*N*/ 		STR_HIDDENPARAFLD,
/*N*/ 		STR_DOCINFOFLD,
/*N*/ 		STR_DBFLD,
/*N*/ 		STR_USERFLD,
/*N*/ 		STR_POSTITFLD,
/*N*/ 		STR_TEMPLNAMEFLD,
/*N*/ 		STR_SEQFLD,
/*N*/ 		STR_DBNEXTSETFLD,
/*N*/ 		STR_DBNUMSETFLD,
/*N*/ 		STR_DBSETNUMBERFLD,
/*N*/ 		STR_CONDTXTFLD,
/*N*/ 		STR_NEXTPAGEFLD,
/*N*/ 		STR_PREVPAGEFLD,
/*N*/ 		STR_EXTUSERFLD,
/*N*/ 		FLD_DATE_FIX,
/*N*/ 		FLD_TIME_FIX,
/*N*/ 		STR_SETINPUTFLD,
/*N*/ 		STR_USRINPUTFLD,
/*N*/ 		STR_SETREFPAGEFLD,
/*N*/ 		STR_GETREFPAGEFLD,
/*N*/ 		STR_INTERNETFLD,
/*N*/ 		STR_JUMPEDITFLD,
/*N*/ 		STR_SCRIPTFLD,
/*N*/ 		STR_AUTHORITY,
/*N*/       STR_COMBINED_CHARS,
/*N*/       STR_DROPDOWN
/*N*/ 	};
/*N*/
/*N*/ 	// Infos fuer Felder einfuegen
/*N*/ 	SwFieldType::pFldNames = new SvStringsDtor( (BYTE)coFldCnt, 2 );
/*N*/ 	for( USHORT nIdx = 0; nIdx < coFldCnt; ++nIdx )
/*N*/ 	{
/*N*/ 		String* pTmp = new SW_RESSTR( coFldNms[ nIdx ] );
/*N*/ 		pTmp->EraseAllChars('~');
/*N*/ 		SwFieldType::pFldNames->Insert(pTmp, nIdx );
/*N*/ 	}
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
