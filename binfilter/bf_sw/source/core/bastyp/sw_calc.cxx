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


#include <ctype.h>
#if defined(MAC) || defined(MACOSX)
#include <stdlib.h>
#else
#include <search.h>
#endif
#include <limits.h>
// #include <math.h>
#include <float.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifdef WNT
#include <tools/svwin.h>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <bf_svx/adritem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <bf_svx/scripttypeitem.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
// tippt sich schneller
#define RESOURCE ViewShell::GetShellRes()

const sal_Char __FAR_DATA sCalc_Add[] 	= 	"add";
const sal_Char __FAR_DATA sCalc_Sub[]	=	"sub";
const sal_Char __FAR_DATA sCalc_Mul[]	=	"mul";
const sal_Char __FAR_DATA sCalc_Div[]	=	"div";
const sal_Char __FAR_DATA sCalc_Phd[]	=	"phd";
const sal_Char __FAR_DATA sCalc_Sqrt[]	=	"sqrt";
const sal_Char __FAR_DATA sCalc_Pow[]	=	"pow";
const sal_Char __FAR_DATA sCalc_Or[]	=	"or";
const sal_Char __FAR_DATA sCalc_Xor[]	=	"xor";
const sal_Char __FAR_DATA sCalc_And[]	=	"and";
const sal_Char __FAR_DATA sCalc_Not[]	=	"not";
const sal_Char __FAR_DATA sCalc_Eq[]	=	"eq";
const sal_Char __FAR_DATA sCalc_Neq[]	=	"neq";
const sal_Char __FAR_DATA sCalc_Leq[]	=	"leq";
const sal_Char __FAR_DATA sCalc_Geq[]	=	"geq";
const sal_Char __FAR_DATA sCalc_L[]		=	"l";
const sal_Char __FAR_DATA sCalc_G[]		=	"g";
const sal_Char __FAR_DATA sCalc_Sum[]	=	"sum";
const sal_Char __FAR_DATA sCalc_Mean[]	=	"mean";
const sal_Char __FAR_DATA sCalc_Min[]	=	"min";
const sal_Char __FAR_DATA sCalc_Max[]	=	"max";
const sal_Char __FAR_DATA sCalc_Sin[]	=	"sin";
const sal_Char __FAR_DATA sCalc_Cos[]	=	"cos";
const sal_Char __FAR_DATA sCalc_Tan[]	=	"tan";
const sal_Char __FAR_DATA sCalc_Asin[]	=	"asin";
const sal_Char __FAR_DATA sCalc_Acos[]	=	"acos";
const sal_Char __FAR_DATA sCalc_Atan[]	=	"atan";
const sal_Char __FAR_DATA sCalc_Round[]	=	"round";



//!!!!! ACHTUNG - Sortierte Liste aller Operatoren !!!!!
struct _CalcOp
{
    union{
        const sal_Char* pName;
        const String* pUName;
    };
    SwCalcOper eOp;
};

_CalcOp	__READONLY_DATA aOpTable[] = {
/* ACOS */    {sCalc_Acos,		CALC_ACOS},  // Arcuscosinus
/* ADD */     {sCalc_Add,        CALC_PLUS},  // Addition
/* AND */     {sCalc_And,        CALC_AND},  	// log. und
/* ASIN */    {sCalc_Asin,       CALC_ASIN},  // Arcussinus
/* ATAN */    {sCalc_Atan,       CALC_ATAN},  // Arcustangens
/* COS */     {sCalc_Cos,        CALC_COS},  	// Cosinus
/* DIV */     {sCalc_Div,        CALC_DIV},   // Dividieren
/* EQ */      {sCalc_Eq,         CALC_EQ},   	// gleich
/* G */       {sCalc_G,          CALC_GRE},  	// groesser
/* GEQ */     {sCalc_Geq,        CALC_GEQ},  	// groesser gleich
/* L */       {sCalc_L,          CALC_LES},  	// kleiner
/* LEQ */     {sCalc_Leq,        CALC_LEQ},  	// kleiner gleich
/* MAX */     {sCalc_Max,        CALC_MAX},  	// Maximalwert
/* MEAN */    {sCalc_Mean,       CALC_MEAN},  // Mittelwert
/* MIN */     {sCalc_Min,        CALC_MIN},  	// Minimalwert
/* MUL */     {sCalc_Mul,        CALC_MUL},  	// Multiplizieren
/* NEQ */     {sCalc_Neq,        CALC_NEQ},  	// nicht gleich
/* NOT */     {sCalc_Not,        CALC_NOT},  	// log. nicht
/* OR */      {sCalc_Or,         CALC_OR},   	// log. oder
/* PHD */     {sCalc_Phd,        CALC_PHD},   // Prozent
/* POW */     {sCalc_Pow,        CALC_POW},	// Potenzieren
/* ROUND */   {sCalc_Round,      CALC_ROUND},	// Runden
/* SIN */     {sCalc_Sin,        CALC_SIN},  	// Sinus
/* SQRT */    {sCalc_Sqrt,       CALC_SQRT},	// Wurzel
/* SUB */     {sCalc_Sub,        CALC_MINUS},	// Subtraktion
/* SUM */     {sCalc_Sum,        CALC_SUM},  	// Summe
/* TAN */     {sCalc_Tan,        CALC_TAN},  	// Tangens
/* XOR */     {sCalc_Xor,        CALC_XOR}   	// log. xoder
};

double __READONLY_DATA nRoundVal[] = {
    5.0e+0,	0.5e+0,	0.5e-1,	0.5e-2,	0.5e-3,	0.5e-4,	0.5e-5,	0.5e-6,
    0.5e-7,	0.5e-8,	0.5e-9,	0.5e-10,0.5e-11,0.5e-12,0.5e-13,0.5e-14,
    0.5e-15,0.5e-16
};

double __READONLY_DATA nKorrVal[] = {
    9, 9e-1, 9e-2, 9e-3, 9e-4, 9e-5, 9e-6, 9e-7, 9e-8,
    9e-9, 9e-10, 9e-11, 9e-12, 9e-13, 9e-14
};

    // First character may be any alphabetic or underscore.
const sal_Int32 coStartFlags =
        ::com::sun::star::i18n::KParseTokens::ANY_LETTER_OR_NUMBER |
        ::com::sun::star::i18n::KParseTokens::ASC_UNDERSCORE |
        ::com::sun::star::i18n::KParseTokens::IGNORE_LEADING_WS;

    // Continuing characters may be any alphanumeric or underscore or dot.
const sal_Int32 coContFlags =
    ( coStartFlags | ::com::sun::star::i18n::KParseTokens::ASC_DOT )
        & ~::com::sun::star::i18n::KParseTokens::IGNORE_LEADING_WS;


extern "C" {
static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
/*N*/ 	OperatorCompare( const void *pFirst, const void *pSecond)
/*N*/ {
/*N*/     int nRet = 0;
/*N*/     if( CALC_NAME == ((_CalcOp*)pFirst)->eOp )
/*N*/     {
/*N*/         if( CALC_NAME == ((_CalcOp*)pSecond)->eOp )
/*?*/             nRet = ((_CalcOp*)pFirst)->pUName->CompareTo(
/*?*/                             *((_CalcOp*)pSecond)->pUName );
/*N*/         else
/*N*/             nRet = ((_CalcOp*)pFirst)->pUName->CompareToAscii(
/*N*/                             ((_CalcOp*)pSecond)->pName );
/*N*/     }
/*N*/     else
/*N*/     {
/*?*/         if( CALC_NAME == ((_CalcOp*)pSecond)->eOp )
/*?*/             nRet = -1 * ((_CalcOp*)pSecond)->pUName->CompareToAscii(
/*?*/                             ((_CalcOp*)pFirst)->pName );
/*?*/         else
/*?*/             nRet = strcmp( ((_CalcOp*)pFirst)->pName,
/*?*/                             ((_CalcOp*)pSecond)->pName );
/*N*/     }
/*N*/ 	return nRet;
/*N*/ }

}// extern "C"

/*N*/ _CalcOp* FindOperator( const String& rSrch )
/*N*/ {
/*N*/ 	_CalcOp aSrch;
/*N*/ 	aSrch.pUName = &rSrch;
/*N*/ 	aSrch.eOp = CALC_NAME;
/*N*/ 
/*N*/ 	return (_CalcOp*)bsearch(   (void*) &aSrch,
/*N*/ 								(void*) aOpTable,
/*N*/ 								sizeof( aOpTable ) / sizeof( _CalcOp ),
/*N*/ 								sizeof( _CalcOp ),
/*N*/ 								OperatorCompare );
/*N*/ }
/*N*/ 

//-----------------------------------------------------------------------------

/*N*/ SwHash* Find( const String& rStr, SwHash** ppTable, USHORT nTblSize,
/*N*/ 				USHORT* pPos )
/*N*/ {
/*N*/ 	ULONG ii = 0, n;
/*N*/ 	for( n = 0; n < rStr.Len(); ++n )
/*N*/ 		ii = ii << 1 ^ rStr.GetChar( n );
/*N*/ 	ii %= nTblSize;
/*N*/ 
/*N*/ 	if( pPos )
/*N*/ 		*pPos = (USHORT)ii;
/*N*/ 
/*N*/ 	for( SwHash* pEntry = *(ppTable+ii); pEntry; pEntry = pEntry->pNext )
/*N*/ 		if( rStr == pEntry->aStr )
/*N*/ 			return pEntry;
/*N*/ 	return 0;
/*N*/ }

/*N*/ inline LanguageType GetDocAppScriptLang( SwDoc& rDoc )
/*N*/ {
/*N*/ 	return ((SvxLanguageItem&)rDoc.GetDefault(
/*N*/ 							GetWhichOfScript( RES_CHRATR_LANGUAGE,
/*N*/ 								            GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() ))
/*N*/ 			)).GetLanguage();
/*N*/ }

//-----------------------------------------------------------------------------

/******************************************************************************
|*
|*	SwCalc::SwCalc( SwDoc* pD ) :
|*
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	JP 03.11.95
|*
|******************************************************************************/
/*N*/ 
/*N*/ SwCalc::SwCalc( SwDoc& rD )
/*N*/ 	: rDoc( rD ),
/*N*/ 	eError( CALC_NOERR ),
/*N*/ 	nListPor( 0 ),
/*N*/ 	aErrExpr( aEmptyStr, SwSbxValue(), 0 ),
/*N*/ 	pLclData( &GetAppLocaleData() ),
/*N*/ 	pCharClass( &GetAppCharClass() )
/*N*/ {
/*N*/ 	aErrExpr.aStr.AssignAscii( "~C_ERR~" );
/*N*/ 	memset( VarTable, 0, sizeof(VarTable) );
/*N*/ 	LanguageType eLang = GetDocAppScriptLang( rDoc );
/*N*/ 
/*N*/     if( eLang != SvxLocaleToLanguage( pLclData->getLocale() ) ||
/*N*/         eLang != SvxLocaleToLanguage( pCharClass->getLocale() ) )
/*N*/ 	{
/*N*/ 		STAR_NMSPC::lang::Locale aLocale( SvxCreateLocale( eLang ));
/*N*/ 		STAR_REFERENCE( lang::XMultiServiceFactory ) xMSF(
/*N*/ 							  ::legacy_binfilters::getLegacyProcessServiceFactory() );  //STRIP002 ::comphelper::getProcessServiceFactory());
/*N*/ 		pCharClass = new CharClass( xMSF, aLocale );
/*N*/ 		pLclData = new LocaleDataWrapper( xMSF, aLocale );
/*N*/ 	}
/*N*/ 
/*N*/ 	sCurrSym = pLclData->getCurrSymbol();
/*N*/ 	sCurrSym.EraseLeadingChars().EraseTrailingChars();
/*N*/ 	pCharClass->toLower( sCurrSym );
/*N*/ 
/*N*/ static sal_Char __READONLY_DATA
/*N*/ 	sNType0[] = "false",
/*N*/ 	sNType1[] = "true",
/*N*/ 	sNType2[] = "pi",
/*N*/ 	sNType3[] = "e",
/*N*/ 	sNType4[] = "tables",
/*N*/ 	sNType5[] = "graf",
/*N*/ 	sNType6[] = "ole",
/*N*/ 	sNType7[] = "page",
/*N*/ 	sNType8[] = "para",
/*N*/ 	sNType9[] = "word",
/*N*/ 	sNType10[]= "char",
/*N*/ 
/*N*/ 	sNType11[] = "user_firstname" ,
/*N*/ 	sNType12[] = "user_lastname" ,
/*N*/ 	sNType13[] = "user_initials" ,
/*N*/ 	sNType14[] = "user_company" ,
/*N*/ 	sNType15[] = "user_street" ,
/*N*/ 	sNType16[] = "user_country" ,
/*N*/ 	sNType17[] = "user_zipcode" ,
/*N*/ 	sNType18[] = "user_city" ,
/*N*/ 	sNType19[] = "user_title" ,
/*N*/ 	sNType20[] = "user_position" ,
/*N*/ 	sNType21[] = "user_tel_work" ,
/*N*/ 	sNType22[] = "user_tel_home" ,
/*N*/ 	sNType23[] = "user_fax" ,
/*N*/ 	sNType24[] = "user_email" ,
/*N*/ 	sNType25[] = "user_state" ,
/*N*/ 	sNType26[] = "graph"
/*N*/ 	;
/*N*/ 
/*N*/ static const sal_Char* __READONLY_DATA sNTypeTab[ 27 ] =
/*N*/ {
/*N*/ 	sNType0, sNType1, sNType2, sNType3, sNType4, sNType5,
/*N*/ 	sNType6, sNType7, sNType8, sNType9, sNType10, sNType11,
/*N*/ 	sNType12, sNType13, sNType14, sNType15, sNType16, sNType17,
/*N*/ 	sNType18, sNType19, sNType20, sNType21, sNType22, sNType23,
/*N*/ 	sNType24,
/*N*/ 
/*N*/ 	// diese sind mit doppelten HashIds
/*N*/ 	sNType25, sNType26
/*N*/ };
/*N*/ static USHORT __READONLY_DATA aHashValue[ 27 ] =
/*N*/ {
/*N*/ 	34, 38, 43,  7, 18, 32, 22, 29, 30, 33,  3,
/*N*/ 	28, 24, 40,  9, 11, 26, 45,  4, 23, 36, 44, 19,  5,  1,
/*N*/ 	// diese sind mit doppelten HashIds
/*N*/ 	11, 38
/*N*/ };
/*N*/ static USHORT __READONLY_DATA aAdrToken[ 12 ] =
/*N*/ {
/*N*/ 	POS_COMPANY, POS_STREET, POS_COUNTRY, POS_PLZ,
/*N*/ 	POS_CITY, POS_TITLE, POS_POSITION, POS_TEL_COMPANY,
/*N*/ 	POS_TEL_PRIVATE, POS_FAX, POS_EMAIL, POS_STATE
/*N*/ };
/*N*/ 
/*N*/ static USHORT SwDocStat::* __READONLY_DATA aDocStat1[ 3 ] =
/*N*/ {
/*N*/ 	&SwDocStat::nTbl, &SwDocStat::nGrf, &SwDocStat::nOLE
/*N*/ };
/*N*/ static ULONG SwDocStat::* __READONLY_DATA aDocStat2[ 4 ] =
/*N*/ {
/*N*/ 	&SwDocStat::nPage, &SwDocStat::nPara,
/*N*/ 	&SwDocStat::nWord, &SwDocStat::nChar
/*N*/ };
/*N*/ 
/*N*/ #if TBLSZ != 47
/*N*/ #error Alle Hashwerte angepasst?
/*N*/ #endif
/*N*/ 
/*N*/ 	const SwDocStat& rDocStat = rDoc.GetDocStat();
/*N*/ 
/*N*/ 	SwSbxValue nVal;
/*N*/ 	String sTmpStr;
        USHORT n=0;
/*N*/ 	for( n = 0; n < 25; ++n )
/*N*/ 	{
/*N*/ 		sTmpStr.AssignAscii( sNTypeTab[ n ] );
/*N*/ 		VarTable[ aHashValue[ n ] ] = new SwCalcExp( sTmpStr, nVal, 0 );
/*N*/ 	}
/*N*/ 
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 0 ] ])->nValue.PutBool( FALSE );
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 1 ] ])->nValue.PutBool( TRUE );
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 2 ] ])->nValue.PutDouble( F_PI );
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 3 ] ])->nValue.PutDouble( 2.7182818284590452354 );
/*N*/ 
/*N*/ 	for( n = 0; n < 3; ++n )
/*N*/ 		((SwCalcExp*)VarTable[ aHashValue[ n + 4 ] ])->nValue.PutLong( rDocStat.*aDocStat1[ n ]  );
/*N*/ 	for( n = 0; n < 4; ++n )
/*N*/ 		((SwCalcExp*)VarTable[ aHashValue[ n + 7 ] ])->nValue.PutLong( rDocStat.*aDocStat2[ n ]  );
/*N*/ 
/*N*/ 	SvxAddressItem aAdr;
/*N*/ 
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 11 ] ])->nValue.PutString( aAdr.GetFirstName() );
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 12 ] ])->nValue.PutString( aAdr.GetName() );
/*N*/ 	((SwCalcExp*)VarTable[ aHashValue[ 13 ] ])->nValue.PutString( aAdr.GetShortName() );
/*N*/ 
/*N*/ 	for( n = 0; n < 11; ++n )
/*N*/ 		((SwCalcExp*)VarTable[ aHashValue[ n + 14 ] ])->nValue.PutString(
/*N*/ 										aAdr.GetToken( aAdrToken[ n ] ));
/*N*/ 
/*N*/ 	nVal.PutString( aAdr.GetToken( aAdrToken[ 11 ] ));
/*N*/ 	sTmpStr.AssignAscii( sNTypeTab[ 25 ] );
/*N*/ 	VarTable[ aHashValue[ 25 ] ]->pNext = new SwCalcExp( sTmpStr, nVal, 0 );
/*N*/ 
/*N*/ // at time its better not to use "graph", because then the im-/export have
/*N*/ // to change in all formulas this name.
/*N*/ //	nVal.PutLong( rDocStat.*aDocStat1[ 1 ]  );
/*N*/ //	VarTable[ aHashValue[ 26 ] ]->pNext = new SwCalcExp(
/*N*/ //												sNTypeTab[ 26 ], nVal, 0 );
/*N*/ }

/******************************************************************************
|*
|*	SwCalc::~SwCalc()
|*
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	OK 12-02-93 11:04am
|*
|******************************************************************************/

/*N*/ SwCalc::~SwCalc()
/*N*/ {
/*N*/ 	for( USHORT n = 0; n < TBLSZ; ++n )
/*N*/ 		delete VarTable[n];
/*N*/ 	if( pLclData != &GetAppLocaleData() )
/*N*/ 		delete pLclData;
/*N*/ 	if( pCharClass != &GetAppCharClass() )
/*N*/ 		delete pCharClass;
/*N*/ }

/******************************************************************************
|*
|*	SwSbxValue SwCalc::Calculate( const String& rStr )
|*
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	OK 12-02-93 11:04am
|*
|******************************************************************************/

/*N*/ SwSbxValue SwCalc::Calculate( const String& rStr )
/*N*/ {
/*N*/ 	eError = CALC_NOERR;
/*N*/ 	SwSbxValue nResult;
/*N*/ 
/*N*/ 	if( !rStr.Len() )
/*N*/ 		return nResult;
/*N*/ 
/*N*/ 	nListPor = 0;
/*N*/ 	eCurrListOper = CALC_PLUS;			// defaulten auf Summe
/*N*/ 
/*N*/ 	sCommand = rStr;
/*N*/ 	nCommandPos = 0;
/*N*/ 
/*N*/ 	while( (eCurrOper = GetToken()) != CALC_ENDCALC && eError == CALC_NOERR )
/*N*/ 		nResult = Expr();
/*N*/ 
/*N*/ 	if( eError )
/*?*/ 		nResult.PutDouble( DBL_MAX );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	SbxDataType eResDType = nResult.GetType();
/*N*/ 	const String& rResStr = nResult.GetString();
/*N*/ #endif
/*N*/ 	return nResult;
/*N*/ }

/******************************************************************************
|*
|*	String SwCalc::GetStrResult( SwSbxValue nValue, BOOL bRound = TRUE )
|*	Beschreibung		Der Parameter bRound ist auf TRUE defaultet und darf
|*						nur beim errechnen von Tabellenzellen auf FALSE gesetzt
|*						werden, damit keine Rundungsfehler beim zusammenstellen
|*						der Formel entstehen.
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	JP 19.02.98
|*
|******************************************************************************/



/*N*/ String SwCalc::GetStrResult( double nValue, BOOL bRound )
/*N*/ {
/*N*/ 	if( nValue >= DBL_MAX )
/*N*/ 		switch( eError )
/*N*/ 		{
/*?*/ 			case CALC_SYNTAX	:	return RESOURCE->aCalc_Syntax;
/*?*/ 			case CALC_ZERODIV	:	return RESOURCE->aCalc_ZeroDiv;
/*?*/ 			case CALC_BRACK		:	return RESOURCE->aCalc_Brack;
/*?*/ 			case CALC_POW		:	return RESOURCE->aCalc_Pow;
/*?*/ 			case CALC_VARNFND	:	return RESOURCE->aCalc_VarNFnd;
/*?*/ 			case CALC_OVERFLOW	:	return RESOURCE->aCalc_Overflow;
/*?*/ 			case CALC_WRONGTIME :	return RESOURCE->aCalc_WrongTime;
/*?*/ 			default				:	return RESOURCE->aCalc_Default;
/*N*/ 		}
/*N*/ 
/*N*/ 	USHORT	nDec = 15; //pLclData->getNumDigits();
/*N*/ 	String	aRetStr( ::rtl::math::doubleToUString( nValue,
/*N*/                 rtl_math_StringFormat_Automatic,
/*N*/                 nDec,
/*N*/                 pLclData->getNumDecimalSep().GetChar(0),
/*N*/                 true ));
/*N*/ 
/*N*/ 	return aRetStr;
/*N*/ }

/******************************************************************************
|*
|*	SwCalcExp* SwCalc::VarLook( const String& )
|*
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	JP 15.11.99
|*
|******************************************************************************/

/*N*/ SwCalcExp* SwCalc::VarInsert( const String &rStr )
/*N*/ {
/*N*/ 	String aStr( rStr );
/*N*/ 	pCharClass->toLower( aStr );
/*N*/ 	return VarLook( aStr, 1 );
/*N*/ }

/******************************************************************************
|*
|*	SwCalcExp* SwCalc::VarLook( const String& , USHORT ins )
|*
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	JP 15.11.99
|*
|******************************************************************************/
/*N*/ SwCalcExp* SwCalc::VarLook( const String& rStr, USHORT ins )
/*N*/ {
/*N*/ 	USHORT ii = 0;
/*N*/     String aStr( rStr );
/*N*/ 	pCharClass->toLower( aStr );
/*N*/ 
/*N*/     SwHash* pFnd = Find( aStr, VarTable, TBLSZ, &ii );
/*N*/ 
/*N*/ 	if( !pFnd )
/*N*/ 	{
/*N*/ 		// dann sehen wir mal im Doc nach:
/*N*/ 		SwHash** ppDocTbl = rDoc.GetUpdtFlds().GetFldTypeTable();
/*N*/ 		for( SwHash* pEntry = *(ppDocTbl+ii); pEntry; pEntry = pEntry->pNext )
/*N*/             if( aStr == pEntry->aStr )
/*N*/ 			{
/*N*/ 				// dann hier zufuegen
/*N*/                 pFnd = new SwCalcExp( aStr, SwSbxValue(),
/*N*/ 									((SwCalcFldType*)pEntry)->pFldType );
/*N*/ 				pFnd->pNext = *(VarTable+ii);
/*N*/ 				*(VarTable+ii) = pFnd;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFnd )
/*N*/ 	{
/*N*/ 		SwCalcExp* pFndExp = (SwCalcExp*)pFnd;
/*N*/ 
/*N*/ 		if( pFndExp->pFldType && pFndExp->pFldType->Which() == RES_USERFLD )
/*N*/ 		{
/*N*/ 			SwUserFieldType* pUFld = (SwUserFieldType*)pFndExp->pFldType;
/*N*/ 			if( GSE_STRING & pUFld->GetType() )
/*N*/ 				pFndExp->nValue.PutString( pUFld->GetContent() );
/*N*/ 			else if( !pUFld->IsValid() )
/*N*/ 			{
/*N*/ 				// Die aktuellen Werte sichern . . .
/*N*/ 				USHORT			nOld_ListPor		= nListPor;
/*N*/ 				SwSbxValue		nOld_LastLeft		= nLastLeft;
/*N*/ 				SwSbxValue		nOld_NumberValue	= nNumberValue;
/*N*/ 				xub_StrLen		nOld_CommandPos		= nCommandPos;
/*N*/ 				SwCalcOper		eOld_CurrOper	  	= eCurrOper;
/*N*/ 				SwCalcOper		eOld_CurrListOper	= eCurrListOper;
/*N*/ 
/*N*/ 				pFndExp->nValue.PutDouble( pUFld->GetValue( *this ) );
/*N*/ 
/*N*/ 				// . . . und zurueck damit.
/*N*/ 				nListPor		= nOld_ListPor;
/*N*/ 				nLastLeft		= nOld_LastLeft;
/*N*/ 				nNumberValue	= nOld_NumberValue;
/*N*/ 				nCommandPos		= nOld_CommandPos;
/*N*/ 				eCurrOper		= eOld_CurrOper;
/*N*/ 				eCurrListOper	= eOld_CurrListOper;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pFndExp->nValue.PutDouble( pUFld->GetValue() );
/*N*/ 		}
/*N*/ 		return pFndExp;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Name(p)=Adress.PLZ oder Adress.DATENSATZNUMMER
/*N*/ 	// DBSETNUMBERFLD = DatenSATZ-nummernfeld (NICHT "setze Datensatznummer!!!")
/*N*/     // #101436#: At this point the "real" case variable has to be used
/*N*/     String sTmpName( rStr );
/*N*/ 	::binfilter::ReplacePoint( sTmpName );
/*N*/ 
/*N*/ 	if( !ins )
/*N*/ 	{
/*N*/ 		SwNewDBMgr *pMgr = rDoc.GetNewDBMgr();
/*N*/ 
/*N*/ 		// Name(p)=Adress.PLZ oder Adress.DATENSATZNUMMER
/*N*/ 		// DBSETNUMBERFLD = DatenSATZ-nummernfeld (NICHT "setze Datensatznummer!!!")
/*N*/ 		String sDBName(GetDBName( sTmpName ));
/*N*/ 		String sSourceName(sDBName.GetToken(0, DB_DELIM));
/*N*/ 		String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));
/*N*/ 		if( pMgr && sSourceName.Len() && sTableName.Len() &&
/*N*/ 			pMgr->OpenDataSource(sSourceName, sTableName))
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 		// auf keinen fall eintragen!!
/*N*/ 		return &aErrExpr;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/     SwCalcExp* pNewExp = new SwCalcExp( aStr, SwSbxValue(), 0 );
/*N*/ 	pNewExp->pNext = VarTable[ ii ];
/*N*/ 	VarTable[ ii ] = pNewExp;
/*N*/ 
/*N*/ 	String sColumnName( GetColumnName( sTmpName ));
/*N*/ 	ASSERT( sColumnName.Len(), "DB-Spaltenname fehlt!" );
/*N*/ 	if( sColumnName.EqualsIgnoreCaseAscii(
/*N*/ 							SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) ))
/*N*/ 	{
/*?*/ 		SwNewDBMgr *pMgr = rDoc.GetNewDBMgr();
/*?*/ 		String sDBName(GetDBName( sTmpName ));
/*?*/ 		String sSourceName(sDBName.GetToken(0, DB_DELIM));
/*?*/ 		String sTableName(sDBName.GetToken(0).GetToken(1, DB_DELIM));
/*?*/ 		if( pMgr && sSourceName.Len() && sTableName.Len() &&
/*?*/ 			pMgr->OpenDataSource(sSourceName, sTableName) &&
/*?*/ 			!pMgr->IsInMerge())
/*?*/ 			pNewExp->nValue.PutULong( pMgr->GetSelectedRecordId(sSourceName, sTableName));
/*N*/ 	}
/*N*/ 
/*N*/ 	return pNewExp;
/*N*/ }

/******************************************************************************
|*
|*	BOOL SwCalc::VarChange( const String& rStr, const SwSbxValue nValue )
|*
|*	Erstellung			OK 12-02-93 11:04am
|*	Letzte Aenderung	OK 12-02-93 11:04am
|*
|******************************************************************************/

/*?*/ void SwCalc::VarChange( const String& rStr, double nValue )
/*?*/ {
/*?*/ 	SwSbxValue aVal( nValue );
/*?*/ 	VarChange( rStr, aVal );
/*?*/ }

/*?*/ void SwCalc::VarChange( const String& rStr, const SwSbxValue& rValue )
/*?*/ {
/*?*/ 	String aStr( rStr );
/*?*/ 	pCharClass->toLower( aStr );
/*?*/ 
/*?*/ 	USHORT nPos = 0;
/*?*/ 	SwCalcExp* pFnd = (SwCalcExp*)Find( aStr, VarTable, TBLSZ, &nPos );
/*?*/ 
/*?*/ 	if( !pFnd )
/*?*/ 	{
/*?*/ 		pFnd = new SwCalcExp( aStr, SwSbxValue( rValue ), 0 );
/*?*/ 		pFnd->pNext = VarTable[ nPos ];
/*?*/ 		VarTable[ nPos ] = pFnd;
/*?*/ 	}
/*?*/ 	else
/*?*/ 		pFnd->nValue = rValue;
/*?*/ }

/******************************************************************************
|*
|*	BOOL SwCalc::Push( const void* pPtr )
|*
|*	Erstellung			OK 12-02-93 11:05am
|*	Letzte Aenderung	OK 12-02-93 11:05am
|*
|******************************************************************************/

/*N*/ BOOL SwCalc::Push( const VoidPtr pPtr )
/*N*/ {
/*N*/ 	if( USHRT_MAX != aRekurStk.GetPos( pPtr ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	aRekurStk.Insert( pPtr, aRekurStk.Count() );
/*N*/ 	return TRUE;
/*N*/ }

/******************************************************************************
|*
|*	void SwCalc::Pop( const void* pPtr )
|*
|*	Erstellung			OK 12-02-93 11:05am
|*	Letzte Aenderung	OK 12-02-93 11:05am
|*
|******************************************************************************/

/*N*/ void SwCalc::Pop( const VoidPtr pPtr )
/*N*/ {
/*N*/ 	ASSERT( aRekurStk.Count() && aRekurStk.GetPos( pPtr ) ==
/*N*/ 			(aRekurStk.Count() - 1 ), "SwCalc: Pop auf ungueltigen Ptr" );
/*N*/ 
/*N*/ 	aRekurStk.Remove( aRekurStk.Count() - 1 );
/*N*/ }


/******************************************************************************
|*
|*	SwCalcOper SwCalc::GetToken()
|*
|*	Erstellung			OK 12-02-93 11:05am
|*	Letzte Aenderung	JP 03.11.95
|*
|******************************************************************************/

/*M*/ SwCalcOper SwCalc::GetToken()
/*M*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ //JP 25.01.2001: static for switch back to the "old" implementation of the
/*M*/ //				calculator, which don't use the I18N routines.
/*M*/ static int nUseOld = 0;
/*M*/ if( !nUseOld )
/*M*/ {
/*M*/ #endif
/*M*/ 
/*M*/ 	if( nCommandPos >= sCommand.Len() )
/*M*/ 		return eCurrOper = CALC_ENDCALC;
/*M*/ 
/*M*/ 	using namespace ::com::sun::star::i18n;
/*M*/ 	{
/*M*/ 		// Parse any token.
/*M*/ 		ParseResult aRes = pCharClass->parseAnyToken( sCommand, nCommandPos,
/*M*/ 													coStartFlags, aEmptyStr,
/*M*/ 													coContFlags, aEmptyStr );
/*M*/ 
/*M*/ 		BOOL bSetError = TRUE;
/*M*/ 		xub_StrLen nRealStt = nCommandPos + (xub_StrLen)aRes.LeadingWhiteSpace;
/*M*/ 		if( aRes.TokenType & (KParseType::ASC_NUMBER | KParseType::UNI_NUMBER) )
/*M*/ 		{
/*M*/ 			nNumberValue.PutDouble( aRes.Value );
/*M*/ 			eCurrOper = CALC_NUMBER;
/*M*/ 			bSetError = FALSE;
/*M*/ 		}
/*M*/ 		else if( aRes.TokenType & KParseType::IDENTNAME )
/*M*/ 		{
/*M*/ 			String aName( sCommand.Copy( nRealStt, aRes.EndPos - nRealStt ));
/*M*/             //#101436#: the variable may contain a database name it must not be converted to lower case
/*M*/             // instead all further comparisons must be done case-insensitive
/*M*/             //pCharClass->toLower( aName );
/*M*/             String sLowerCaseName(aName);
/*M*/             pCharClass->toLower( sLowerCaseName );
/*M*/ 			// Currency-Symbol abfangen
/*M*/             if( sLowerCaseName == sCurrSym )
/*M*/ 			{
/*M*/ 				nCommandPos = (xub_StrLen)aRes.EndPos;
/*M*/ 				return GetToken();	// also nochmal aufrufen
/*M*/ 			}
/*M*/ 
/*M*/ 			// Operations abfangen
/*M*/             _CalcOp* pFnd = ::binfilter::FindOperator( sLowerCaseName );
/*M*/ 			if( pFnd )
/*M*/ 			{
/*M*/ 				switch( ( eCurrOper = ((_CalcOp*)pFnd)->eOp ) )
/*M*/ 				{
/*M*/ 					case CALC_SUM  :
/*M*/ 					case CALC_MEAN : eCurrListOper = CALC_PLUS;		break;
/*M*/ 					case CALC_MIN  : eCurrListOper = CALC_MIN_IN;	break;
/*M*/ 					case CALC_MAX  : eCurrListOper = CALC_MAX_IN;	break;
/*M*/ 				}
/*M*/ 				nCommandPos = (xub_StrLen)aRes.EndPos;
/*M*/ 				return eCurrOper;
/*M*/ 			}
/*M*/ 			aVarName = aName;
/*M*/ 			eCurrOper = CALC_NAME;
/*M*/ 			bSetError = FALSE;
/*M*/ 		}
/*M*/ 		else if ( aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING )
/*M*/ 		{
/*M*/ 			nNumberValue.PutString( String( aRes.DequotedNameOrString ));
/*M*/ 			eCurrOper = CALC_NUMBER;
/*M*/ 			bSetError = FALSE;
/*M*/ 		}
/*M*/ 		else if( aRes.TokenType & KParseType::ONE_SINGLE_CHAR )
/*M*/ 		{
/*M*/ 			String aName( sCommand.Copy( nRealStt, aRes.EndPos - nRealStt ));
/*M*/ 			if( 1 == aName.Len() )
/*M*/ 			{
/*M*/ 				bSetError = FALSE;
/*M*/ 				sal_Unicode ch = aName.GetChar( 0 );
/*M*/ 				switch( ch )
/*M*/ 				{
/*M*/ 				case ';':
/*M*/ 				case '\n':
/*M*/ 							eCurrOper = CALC_PRINT;
/*M*/ 							break;
/*M*/ 				case '%':
/*M*/ 				case '^':
/*M*/ 				case '*':
/*M*/ 				case '/':
/*M*/ 				case '+':
/*M*/ 				case '-':
/*M*/ 				case '(':
/*M*/ 				case ')':	eCurrOper = SwCalcOper(ch);
/*M*/ 							break;
/*M*/ 
/*M*/ 				case '=':
/*M*/ 				case '!':
/*M*/ 						{
/*M*/ 							SwCalcOper eTmp2;
/*M*/ 							if( '=' == ch )
/*M*/ 								eCurrOper = SwCalcOper('='), eTmp2 = CALC_EQ;
/*M*/ 							else
/*M*/ 								eCurrOper = CALC_NOT, eTmp2 = CALC_NEQ;
/*M*/ 
/*M*/ 							if( aRes.EndPos < sCommand.Len() &&
/*M*/ 								'=' == sCommand.GetChar( (xub_StrLen)aRes.EndPos ) )
/*M*/ 							{
/*M*/ 								eCurrOper = eTmp2;
/*M*/ 								++aRes.EndPos;
/*M*/ 							}
/*M*/ 						}
/*M*/ 						break;
/*M*/ 
/*M*/ 				case cListDelim :
/*M*/ 						eCurrOper = eCurrListOper;
/*M*/ 						break;
/*M*/ 
/*M*/ 				case '[':
/*M*/ 						if( aRes.EndPos < sCommand.Len() )
/*M*/ 						{
/*M*/ 							aVarName.Erase();
/*M*/ 							xub_StrLen nFndPos = (xub_StrLen)aRes.EndPos,
/*M*/ 										nSttPos = nFndPos;
/*M*/ 
/*M*/ 							do{
/*M*/ 								if( STRING_NOTFOUND != ( nFndPos =
/*M*/ 									sCommand.Search( ']', nFndPos )) )
/*M*/ 								{
/*M*/ 									// ignore the ]
/*M*/ 									if( '\\' == sCommand.GetChar(nFndPos-1))
/*M*/ 									{
/*M*/ 										aVarName += sCommand.Copy( nSttPos,
/*M*/ 													nFndPos - nSttPos - 1 );
/*M*/ 										nSttPos = ++nFndPos;
/*M*/ 									}
/*M*/ 									else
/*M*/ 										break;
/*M*/ 								}
/*M*/ 							} while( STRING_NOTFOUND != nFndPos );
/*M*/ 
/*M*/ 							if( STRING_NOTFOUND != nFndPos )
/*M*/ 							{
/*M*/ 								if( nSttPos != nFndPos )
/*M*/ 									aVarName += sCommand.Copy( nSttPos,
/*M*/ 													nFndPos - nSttPos );
/*M*/ 								aRes.EndPos = nFndPos + 1;
/*M*/ 								eCurrOper = CALC_NAME;
/*M*/ 							}
/*M*/ 							else
/*M*/ 								bSetError = TRUE;
/*M*/ 						}
/*M*/ 						else
/*M*/ 							bSetError = TRUE;
/*M*/ 						break;
/*M*/ 
/*M*/ 				default:
/*M*/ 					bSetError = TRUE;
/*M*/ 					break;
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else if( aRes.TokenType & KParseType::BOOLEAN )
/*M*/ 		{
/*M*/ 			String aName( sCommand.Copy( nRealStt, aRes.EndPos - nRealStt ));
/*M*/ 			if( aName.Len() )
/*M*/ 			{
/*M*/ 				bSetError = FALSE;
/*M*/ 				sal_Unicode ch = aName.GetChar(0);
/*M*/ 				SwCalcOper eTmp2;
/*M*/ 				if( '<' == ch )
/*M*/ 					eCurrOper = CALC_LES, eTmp2 = CALC_LEQ;
/*M*/ 				else if( '>' == ch )
/*M*/ 					eCurrOper = CALC_GRE, eTmp2 = CALC_GEQ;
/*M*/ 				else
/*M*/ 					bSetError = TRUE;
/*M*/ 
/*M*/ 				if( !bSetError )
/*M*/ 				{
/*M*/ 					if( 2 == aName.Len() && '=' == aName.GetChar(1) )
/*M*/ 						eCurrOper = eTmp2;
/*M*/ 					else if( 1 != aName.Len() )
/*M*/ 						bSetError = TRUE;
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else if( nRealStt == sCommand.Len() )
/*M*/ 		{
/*M*/ 			eCurrOper = CALC_ENDCALC;
/*M*/ 			bSetError = FALSE;
/*M*/ 		}
/*M*/ 
/*M*/ 		if( bSetError )
/*M*/ 		{
/*M*/ 			eError = CALC_SYNTAX;
/*M*/ 			eCurrOper = CALC_PRINT;
/*M*/ 		}
/*M*/ 		nCommandPos = (xub_StrLen)aRes.EndPos;
/*M*/ 	};
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ 
/*M*/ #define NextCh( s, n )	(nCommandPos < sCommand.Len() ? sCommand.GetChar( nCommandPos++ ) : 0)
/*M*/ 
/*M*/ }
/*M*/ else
/*M*/ {
/*M*/ 	sal_Unicode ch;
/*M*/ 	sal_Unicode cTSep = pLclData->getNumThousandSep().GetChar(0),
/*M*/ 				cDSep = pLclData->getNumDecimalSep().GetChar(0);
/*M*/ 
/*M*/ 	do {
/*M*/ 		if( 0 == ( ch = NextCh( sCommand, nCommandPos ) ) )
/*M*/ 			return eCurrOper = CALC_ENDCALC;
/*M*/ 	} while ( ch == '\t' || ch == ' ' || ch == cTSep );
/*M*/ 
/*M*/ 	if( ch == cDSep )
/*M*/ 		ch = '.';
/*M*/ 
/*M*/ 	switch( ch )
/*M*/ 	{
/*M*/ 		case ';':
/*M*/ 		case '\n':
/*M*/ 					{
/*M*/ 						sal_Unicode c;
/*M*/ 						while( nCommandPos < sCommand.Len() && ( ( c =
/*M*/ 								sCommand.GetChar( nCommandPos ) ) == ' ' ||
/*M*/ 								c == '\t' || c == '\x0a' || c == '\x0d' ))
/*M*/ 							++nCommandPos;
/*M*/ 						eCurrOper = CALC_PRINT;
/*M*/ 					}
/*M*/ 					break;
/*M*/ 		case '%':
/*M*/ 		case '^':
/*M*/ 		case '*':
/*M*/ 		case '/':
/*M*/ 		case '+':
/*M*/ 		case '-':
/*M*/ 		case '(':
/*M*/ 		case ')':	eCurrOper = SwCalcOper(ch);
/*M*/ 					break;
/*M*/ 
/*M*/ 		case '=':   if( '=' == sCommand.GetChar( nCommandPos ) )
/*M*/ 					{
/*M*/ 						++nCommandPos;
/*M*/ 						eCurrOper = CALC_EQ;
/*M*/ 					}
/*M*/ 					else
/*M*/ 						eCurrOper = SwCalcOper(ch);
/*M*/ 					break;
/*M*/ 
/*M*/ 		case '!':   if( '=' == sCommand.GetChar( nCommandPos ) )
/*M*/ 					{
/*M*/ 						++nCommandPos;
/*M*/ 						eCurrOper = CALC_NEQ;
/*M*/ 					}
/*M*/ 					else
/*M*/ 						eCurrOper = CALC_NOT;
/*M*/ 					break;
/*M*/ 
/*M*/ 		case '>':
/*M*/ 		case '<':   eCurrOper = '>' == ch  ? CALC_GRE : CALC_LES;
/*M*/ 					if( '=' == (ch = sCommand.GetChar( nCommandPos ) ) )
/*M*/ 					{
/*M*/ 						++nCommandPos;
/*M*/ 						eCurrOper = CALC_GRE == eCurrOper ? CALC_GEQ : CALC_LEQ;
/*M*/ 					}
/*M*/ 					else if( ' ' != ch )
/*M*/ 					{
/*M*/ 						eError = CALC_SYNTAX;
/*M*/ 						eCurrOper = CALC_PRINT;
/*M*/ 					}
/*M*/ 					break;
/*M*/ 
/*M*/ 		case cListDelim :
/*M*/ 					eCurrOper = eCurrListOper;
/*M*/ 					break;
/*M*/ 
/*M*/ 		case '0':	case '1':	case '2':	case '3':	case '4':
/*M*/ 		case '5':	case '6':	case '7':	case '8':	case '9':
/*M*/ 		case ',':
/*M*/ 		case '.':	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*M*/ 					}
/*M*/ 					break;
/*M*/ 
/*M*/ 		case '[':	{
/*M*/ 						String aStr;
/*M*/ 						BOOL bIgnore = FALSE;
/*M*/ 						do {
/*M*/ 							while( ( ch = NextCh( sCommand, nCommandPos  ))
/*M*/ 									&& ch != ']' )
/*M*/ 							{
/*M*/ 								if( !bIgnore && '\\' == ch )
/*M*/ 									bIgnore = TRUE;
/*M*/ 								else if( bIgnore )
/*M*/ 									bIgnore = FALSE;
/*M*/ 								aStr += ch;
/*M*/ 							}
/*M*/ 
/*M*/ 							if( !bIgnore )
/*M*/ 								break;
/*M*/ 
/*M*/ 							aStr.SetChar( aStr.Len() - 1, ch );
/*M*/ 						} while( ch );
/*M*/ 
/*M*/ 						aVarName = aStr;
/*M*/ 						eCurrOper = CALC_NAME;
/*M*/ 					}
/*M*/ 					break;
/*M*/ 
/*M*/ 		case '"':	{
/*M*/ 						xub_StrLen nStt = nCommandPos;
/*M*/ 						while( ( ch = NextCh( sCommand, nCommandPos ))
/*M*/ 								&& '"' != ch )
/*M*/ 							;
/*M*/ 
/*M*/ 						xub_StrLen nLen = nCommandPos - nStt;
/*M*/ 						if( '"' == ch )
/*M*/ 							--nLen;
/*M*/ 						nNumberValue.PutString( sCommand.Copy( nStt, nLen ));
/*M*/ 						eCurrOper = CALC_NUMBER;
/*M*/ 					}
/*M*/ 					break;
/*M*/ 
/*M*/ 		default:	if( ch && pCharClass->isLetter( sCommand, nCommandPos - 1)
/*M*/ 							|| '_' == ch )
/*M*/ 					{
/*M*/ 						xub_StrLen nStt = nCommandPos-1;
/*M*/ 						while( 0 != (ch = NextCh( sCommand, nCommandPos )) &&
/*M*/ 							   (pCharClass->isLetterNumeric(
/*M*/ 							   					sCommand, nCommandPos - 1) ||
/*M*/ 								ch == '_' || ch == '.' ) )
/*M*/ 							;
/*M*/ 
/*M*/ 						if( ch )
/*M*/ 							--nCommandPos;
/*M*/ 
/*M*/ 						String aStr( sCommand.Copy( nStt, nCommandPos-nStt ));
/*M*/ 						pCharClass->toLower( aStr );
/*M*/ 
/*M*/ 
/*M*/ 						// Currency-Symbol abfangen
/*M*/ 						if( aStr == sCurrSym )
/*M*/ 							return GetToken();	// also nochmal aufrufen
/*M*/ 
/*M*/ 						// Operations abfangen
/*M*/ 						_CalcOp* pFnd = ::binfilter::FindOperator( aStr );
/*M*/ 						if( pFnd )
/*M*/ 						{
/*M*/ 							switch( ( eCurrOper = ((_CalcOp*)pFnd)->eOp ) )
/*M*/ 							{
/*M*/ 								case CALC_SUM  :
/*M*/ 								case CALC_MEAN : eCurrListOper = CALC_PLUS;
/*M*/ 													break;
/*M*/ 								case CALC_MIN  : eCurrListOper = CALC_MIN_IN;
/*M*/ 													break;
/*M*/ 								case CALC_MAX  : eCurrListOper = CALC_MAX_IN;
/*M*/ 													break;
/*M*/ 							}
/*M*/ 							return eCurrOper;
/*M*/ 						}
/*M*/ 						aVarName = aStr;
/*M*/ 						eCurrOper = CALC_NAME;
/*M*/ 					}
/*M*/ 					else
/*M*/ 					{
/*M*/ 						eError = CALC_SYNTAX;
/*M*/ 						eCurrOper = CALC_PRINT;
/*M*/ 					}
/*M*/ 					break;
/*M*/ 	}
/*M*/ 
/*M*/ }
/*M*/ #endif
/*M*/ 	return eCurrOper;
/*M*/ }

/******************************************************************************
|*
|*	SwSbxValue SwCalc::Term()
|*
|*	Erstellung			OK 12-02-93 11:05am
|*	Letzte Aenderung	JP 16.01.96
|*
|******************************************************************************/

/*N*/ SwSbxValue SwCalc::Term()
/*N*/ {
/*N*/ 	SwSbxValue left( Prim() );
/*N*/ 	nLastLeft = left;
/*N*/ 	for(;;)
/*N*/ 	{
/*N*/ 		SbxOperator eSbxOper = (SbxOperator)USHRT_MAX;
/*N*/ 
/*N*/ 		switch( eCurrOper )
/*N*/ 		{
/*N*/ // wir haben kein Bitweises verodern, oder ?
/*N*/ //			case CALC_AND:	eSbxOper = SbxAND;	break;
/*N*/ //			case CALC_OR:	eSbxOper = SbxOR;	break;
/*N*/ //			case CALC_XOR:	eSbxOper = SbxXOR;	break;
/*?*/ 			case CALC_AND:	{
/*?*/ 								GetToken();
/*?*/ 								BOOL bB = Prim().GetBool();
/*?*/ 								left.PutBool( left.GetBool() && bB );
/*?*/ 							}
/*?*/ 							break;
/*?*/ 			case CALC_OR:	{
/*?*/ 								GetToken();
/*?*/ 								BOOL bB = Prim().GetBool();
/*?*/ 								left.PutBool( left.GetBool() || bB );
/*?*/ 							}
/*?*/ 							break;
/*?*/ 			case CALC_XOR:	{
/*?*/ 								GetToken();
/*?*/ 								BOOL bR = Prim().GetBool();
/*?*/ 								BOOL bL = left.GetBool();
/*?*/ 								left.PutBool( (bL && !bR) || (!bL && bR) );
/*?*/ 							}
/*?*/ 							break;
/*?*/ 
/*?*/ 			case CALC_EQ:	eSbxOper = SbxEQ;	break;
/*?*/ 			case CALC_NEQ:	eSbxOper = SbxNE;	break;
/*?*/ 			case CALC_LEQ:	eSbxOper = SbxLE;	break;
/*?*/ 			case CALC_GEQ:	eSbxOper = SbxGE;	break;
/*?*/ 			case CALC_GRE:	eSbxOper = SbxGT;	break;
/*?*/ 			case CALC_LES:	eSbxOper = SbxLT;	break;
/*N*/ 
/*N*/ 			case CALC_MUL:	eSbxOper = SbxMUL; 	break;
/*?*/ 			case CALC_DIV:	eSbxOper = SbxDIV;  break;
/*?*/ 
/*?*/ 			case CALC_MIN_IN:
/*?*/ 							{
/*?*/ 								GetToken();
/*?*/ 								SwSbxValue e = Prim();
/*?*/ 								left = left.GetDouble() < e.GetDouble()
/*?*/ 											? left : e;
/*?*/ 							}
/*?*/ 							break;
/*?*/ 			case CALC_MAX_IN:
/*?*/ 							{
/*?*/ 								GetToken();
/*?*/ 								SwSbxValue e = Prim();
/*?*/ 								left = left.GetDouble() > e.GetDouble()
/*?*/ 											? left : e;
/*?*/ 							}
/*?*/ 							break;
/*?*/ 			case CALC_ROUND:
/*?*/ 							{
/*?*/ 								GetToken();
/*?*/ 								SwSbxValue e = Prim();
/*?*/ 
/*?*/ 								double fVal = 0;
/*?*/ 								double fFac = 1;
/*?*/ 								INT32 nDec = (INT32) floor( e.GetDouble() );
/*?*/ 								if( nDec < -20 || nDec > 20 )
/*?*/ 								{
/*?*/ 									eError = CALC_OVERFLOW;
/*?*/ 									left.Clear();
/*?*/ 									return left;
/*?*/ 								}
/*?*/ 								fVal = left.GetDouble();
/*?*/ 								USHORT i;
/*?*/ 								if( nDec >= 0)
/*?*/ 									for (i = 0; i < (USHORT) nDec; ++i )
/*?*/ 										fFac *= 10.0;
/*?*/ 								else
/*?*/ 									for (i = 0; i < (USHORT) -nDec; ++i )
/*?*/ 										fFac /= 10.0;
/*?*/ 
/*?*/ 								fVal *= fFac;
/*?*/ 
/*?*/ 								BOOL bSign;
/*?*/ 								if (fVal < 0.0)
/*?*/ 								{
/*?*/ 									fVal *= -1.0;
/*?*/ 									bSign = TRUE;
/*?*/ 								}
/*?*/ 								else
/*?*/ 									bSign = FALSE;
/*?*/ 
/*?*/ 								// runden
/*?*/ 								double fNum = fVal;				// find the exponent
/*?*/ 								int nExp = 0;
/*?*/ 								if( fNum > 0 )
/*?*/ 								{
/*?*/ 									while( fNum < 1.0 ) fNum *= 10.0, --nExp;
/*?*/ 									while( fNum >= 10.0 ) fNum /= 10.0, ++nExp;
/*?*/ 								}
/*?*/ 								nExp = 15 - nExp;
/*?*/ 								if( nExp > 15 )
/*?*/ 									nExp = 15;
/*?*/ 								else if( nExp <= 1 )
/*?*/ 									nExp = 0;
/*?*/ 								fVal = floor( fVal+ 0.5 + nRoundVal[ nExp ] );
/*?*/ 
/*?*/ 								if (bSign)
/*?*/ 									fVal *= -1.0;
/*?*/ 
/*?*/ 								fVal /= fFac;
/*?*/ 
/*?*/ 								left.PutDouble( fVal );
/*?*/ 							}
/*?*/ 							break;
/*N*/ 
/*
// removed here because of #77448# (=2*3^2 != 18)
            case CALC_POW:  {
                                 GetToken();
 #if defined(MAC) && !defined(__powerc)
                                 long double fraction, integer;
 #else
                                 double fraction, integer;
 #endif
                                 double right = Prim().GetDouble(),
                                         dleft = left.GetDouble();
 
                                 fraction = modf( right, &integer );
                                 if( ( dleft < 0.0 && 0.0 != fraction ) ||
                                     ( 0.0 == dleft && right < 0.0 ) )
                                 {
                                     eError = CALC_OVERFLOW;
                                     left.Clear();
                                     return left;
                                 }
                                 dleft = pow(dleft, right );
                                 if( dleft == HUGE_VAL )
                                 {
                                     eError = CALC_POWERR;
                                     left.Clear();
                                     return left;
                                 }
                                 left.PutDouble( dleft );
                             }
                             break;
 */
/*N*/ 			default:		return left;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( USHRT_MAX != (USHORT)eSbxOper )
/*N*/ 		{
/*N*/ 			GetToken();
/*N*/ 			if( SbxEQ <= eSbxOper && eSbxOper <= SbxGE )
/*?*/ 				left.PutBool( left.Compare( eSbxOper, Prim() ));
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwSbxValue aRight( Prim() );
/*N*/ 				aRight.MakeDouble();
/*N*/ 				left.MakeDouble();
/*N*/ 
/*N*/ 				if( SbxDIV == eSbxOper && !aRight.GetDouble() )
/*N*/ 					eError = CALC_ZERODIV;
/*N*/ 				else
/*N*/ 					left.Compute( eSbxOper, aRight );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	left.Clear();
/*N*/ 	return left;
/*N*/ }

/******************************************************************************
|*
|*	SwSbxValue SwCalc::Prim()
|*
|*	Erstellung			OK 12-02-93 11:05am
|*	Letzte Aenderung	JP 03.11.95
|*
|******************************************************************************/

/*N*/ SwSbxValue SwCalc::Prim()
/*N*/ {
/*N*/ 	SwSbxValue nErg;
/*N*/ 
/*N*/ 	double (
/*N*/ #ifdef WNT
/*N*/ 			__cdecl
/*N*/ #endif
/*N*/ 
/*N*/ 			*pFnc)( double ) = 0;
/*N*/ 
/*N*/ 	BOOL bChkTrig = FALSE, bChkPow = FALSE;
/*N*/ 
/*N*/ 	switch( eCurrOper )
/*N*/ 	{
/*?*/ 		case CALC_SIN:		pFnc = &sin;						break;
/*?*/ 		case CALC_COS:		pFnc = &cos;						break;
/*?*/ 		case CALC_TAN:		pFnc = &tan;						break;
/*?*/ 		case CALC_ATAN:		pFnc = &atan;						break;
/*?*/ 		case CALC_ASIN:		pFnc = &asin; 	bChkTrig = TRUE;	break;
/*?*/ 		case CALC_ACOS:		pFnc = &acos; 	bChkTrig = TRUE;	break;
/*?*/ 
/*?*/ 		case CALC_NOT:		{
/*?*/ 								GetToken();
/*?*/ 								nErg = Prim();
/*?*/ 								if( SbxSTRING == nErg.GetType() )
/*?*/ 									nErg.PutBool( 0 != nErg.GetString().Len() );
/*?*/ 								nErg.Compute( SbxNOT, nErg );
/*?*/ 							}
/*?*/ 							break;
/*N*/ 
/*N*/ 		case CALC_NUMBER:	if( GetToken() == CALC_PHD )
/*N*/ 							{
/*?*/ 								double aTmp = nNumberValue.GetDouble();
/*?*/ 								aTmp *= 0.01;
/*?*/ 								nErg.PutDouble( aTmp );
/*?*/ 								GetToken();
/*N*/ 							}
/*N*/ 							else if( eCurrOper == CALC_NAME )
/*N*/ 								eError = CALC_SYNTAX;
/*N*/ 							else
/*N*/ 							{
/*N*/ 								nErg = nNumberValue;
/*N*/ 								bChkPow = TRUE;
/*N*/ 							}
/*N*/ 							break;
/*N*/ 
/*N*/ 		case CALC_NAME: 	if( GetToken() == CALC_ASSIGN )
/*N*/ 							{
/*N*/ 								SwCalcExp* n = VarInsert( aVarName );
/*N*/ 								GetToken();
/*N*/ 								nErg = n->nValue = Expr();
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								nErg = VarLook( aVarName )->nValue;
/*N*/ 								bChkPow = TRUE;
/*N*/ 							}
/*N*/ 							break;
/*N*/ 
/*?*/ 		case CALC_MINUS:	GetToken();
/*?*/ 							nErg.PutDouble( -(Prim().GetDouble()) );
/*?*/ 							break;
/*N*/ 
/*N*/ 		case CALC_LP:		{
/*N*/ 								GetToken();
/*N*/ 								nErg = Expr();
/*N*/ 								if( eCurrOper != CALC_RP )
/*?*/ 									eError = CALC_BRACK;
/*N*/ 								else
/*N*/                               {
/*N*/ 									GetToken();
/*N*/                                   bChkPow = TRUE; // in order for =(7)^2 to work
/*N*/                               }
/*N*/ 							}
/*N*/ 							break;
/*N*/ 
/*N*/ 		case CALC_MEAN:		{
/*?*/ 								nListPor = 1;
/*?*/ 								GetToken();
/*?*/ 								nErg = Expr();
/*?*/ 								double aTmp = nErg.GetDouble();
/*?*/ 								aTmp /= nListPor;
/*?*/ 								nErg.PutDouble( aTmp );
/*?*/ 							}
/*?*/ 							break;
/*N*/ 
/*?*/ 		case CALC_SQRT:		{
/*?*/ 								GetToken();
/*?*/ 								nErg = Prim();
/*?*/ 								if( nErg.GetDouble() < 0 )
/*?*/ 									eError = CALC_OVERFLOW;
/*?*/ 								else
/*?*/ 									nErg.PutDouble( sqrt( nErg.GetDouble() ));
/*?*/ 							}
/*?*/ 							break;
/*N*/ 
/*N*/ 		case CALC_SUM:
/*N*/ 		case CALC_MIN:
/*N*/ 		case CALC_MAX:		GetToken();
/*N*/ 							nErg = Expr();
/*N*/ 							break;
/*N*/ 
/*?*/ 		case CALC_ENDCALC:	nErg.Clear();
/*?*/ 							break;
/*N*/ 
/*N*/ 		default:			eError = CALC_SYNTAX;
/*N*/ 							break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFnc )
/*N*/ 	{
/*?*/ 		GetToken();
/*?*/ 		double nVal = Prim().GetDouble();
/*?*/ 		if( !bChkTrig || ( nVal > -1 && nVal < 1 ) )
/*?*/ 			nErg.PutDouble( (*pFnc)( nVal ) );
/*?*/ 		else
/*?*/ 			eError = CALC_OVERFLOW;
/*N*/ 	}

    // added here because of #77448# (=2*3^2 should be 18)
/*N*/ 	if( bChkPow && eCurrOper == CALC_POW )
/*N*/ 	{
/*?*/ 		double dleft = nErg.GetDouble();
/*?*/ 		GetToken();
/*?*/ 		double right = Prim().GetDouble();
/*?*/ 
/*?*/ #if defined(MAC) && !defined(__powerc)
/*?*/ 		long double fraction, integer;
/*?*/ #else
/*?*/ 		double fraction, integer;
/*?*/ #endif
/*?*/ 		fraction = modf( right, &integer );
/*?*/ 		if( ( dleft < 0.0 && 0.0 != fraction ) ||
/*?*/ 			( 0.0 == dleft && right < 0.0 ) )
/*?*/ 		{
/*?*/ 			eError = CALC_OVERFLOW;
/*?*/ 			nErg.Clear();
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			dleft = pow(dleft, right );
/*?*/ 			if( dleft == HUGE_VAL )
/*?*/ 			{
/*?*/ 				eError = CALC_POWERR;
/*?*/ 				nErg.Clear();
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				nErg.PutDouble( dleft );
/*?*/ //				GetToken();
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return nErg;
/*N*/ }

/******************************************************************************
|*
|*	SwSbxValue	SwCalc::Expr()
|*
|*	Erstellung			OK 12-02-93 11:06am
|*	Letzte Aenderung	JP 03.11.95
|*
|******************************************************************************/

/*N*/ SwSbxValue	SwCalc::Expr()
/*N*/ {
/*N*/ 	SwSbxValue left = Term(), right;
/*N*/ 	nLastLeft = left;
/*N*/ 	for(;;)
/*N*/ 		switch(eCurrOper)
/*N*/ 		{
/*N*/ 			case CALC_PLUS:		GetToken();
/*N*/ 								// erzeuge zum addieren auf jedenfall einen
/*N*/ 								// Double-Wert
/*N*/ 								left.MakeDouble();
/*N*/ 								( right = Term() ).MakeDouble();
/*N*/ 								left.Compute( SbxPLUS, right );
/*N*/ 								nListPor++;
/*N*/ 								break;
/*N*/ 
/*?*/ 			case CALC_MINUS:	GetToken();
/*?*/ 								// erzeuge zum addieren auf jedenfall einen
/*?*/ 								// Double-Wert
/*?*/ 								left.MakeDouble();
/*?*/ 								( right = Term() ).MakeDouble();
/*?*/ 								left.Compute( SbxMINUS, right );
/*?*/ 								break;
/*N*/ 
/*N*/ 			default:			return left;
/*N*/ 		}
/*N*/ 	left.Clear();
/*N*/ 	return left;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ String SwCalc::GetColumnName(const String& rName)
/*N*/ {
/*N*/ 	xub_StrLen nPos = rName.Search(DB_DELIM);
/*N*/ 	if( STRING_NOTFOUND != nPos )
/*N*/ 	{
/*?*/ 		nPos = rName.Search(DB_DELIM, nPos + 1);
/*?*/ 
/*?*/ 		if( STRING_NOTFOUND != nPos )
/*?*/ 			return rName.Copy(nPos + 1);
/*N*/ 	}
/*N*/ 	return rName;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ String SwCalc::GetDBName(const String& rName)
/*N*/ {
/*N*/ 	xub_StrLen nPos = rName.Search(DB_DELIM);
/*N*/ 	if( STRING_NOTFOUND != nPos )
/*N*/ 	{
/*?*/ 		nPos = rName.Search(DB_DELIM, nPos + 1);
/*?*/ 
/*?*/ 		if( STRING_NOTFOUND != nPos )
/*?*/ 			return rName.Copy( 0, nPos );
/*N*/ 	}
/*N*/ 	SwDBData aData = rDoc.GetDBData();
/*N*/ 	String sRet = aData.sDataSource;
/*N*/ 	sRet += DB_DELIM;
/*N*/ 	sRet += String(aData.sCommand);
/*N*/ 	return sRet;
/*N*/ }

//------------------------------------------------------------------------------

/******************************************************************************
 *	Methode		:	FASTBOOL SwCalc::Str2Double( double& )
 *	Beschreibung:
 *	Erstellt	:	OK 07.06.94 12:56
 *	Aenderung	: 	JP 27.10.98
 ******************************************************************************/
/*N*/ FASTBOOL SwCalc::Str2Double( const String& rCommand, xub_StrLen& rCommandPos,
/*N*/ 							double& rVal, const LocaleDataWrapper* pLclData )
/*N*/ {
/*N*/ 	const LocaleDataWrapper* pLclD = pLclData;
/*N*/ 	if( !pLclD )
/*N*/ 		pLclD = &GetAppLocaleData();
/*N*/ 
/*N*/ 	const xub_Unicode nCurrCmdPos = rCommandPos;
/*N*/     rtl_math_ConversionStatus eStatus;
/*N*/     const sal_Unicode* pEnd;
/*N*/     rVal = rtl_math_uStringToDouble( rCommand.GetBuffer() + rCommandPos,
/*N*/             rCommand.GetBuffer() + rCommand.Len(),
/*N*/             pLclD->getNumDecimalSep().GetChar(0),
/*N*/             pLclD->getNumThousandSep().GetChar(0),
/*N*/             &eStatus, &pEnd );
/*N*/ 	rCommandPos = pEnd - rCommand.GetBuffer();
/*N*/ 
/*N*/ 	if( !pLclData && pLclD != &GetAppLocaleData() )
/*N*/ 		delete (LocaleDataWrapper*)pLclD;
/*N*/ 
/*N*/ 	return rtl_math_ConversionStatus_Ok == eStatus && nCurrCmdPos != rCommandPos;
/*N*/ }

/*N*/ FASTBOOL SwCalc::Str2Double( const String& rCommand, xub_StrLen& rCommandPos,
/*N*/ 							double& rVal, SwDoc* pDoc )
/*N*/ {
/*N*/ 	const LocaleDataWrapper* pLclD = &GetAppLocaleData();
/*N*/ 	if( pDoc )
/*N*/ 	{
/*N*/ 
/*N*/ 		LanguageType eLang = GetDocAppScriptLang( *pDoc );
/*N*/ 		if( eLang != SvxLocaleToLanguage( pLclD->getLocale() ) )
/*N*/ 			pLclD = new LocaleDataWrapper(
/*N*/ 							 ::legacy_binfilters::getLegacyProcessServiceFactory(), //STRIP002  ::comphelper::getProcessServiceFactory(),
/*N*/ 							SvxCreateLocale( eLang ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	const xub_Unicode nCurrCmdPos = rCommandPos;
/*N*/     rtl_math_ConversionStatus eStatus;
/*N*/     const sal_Unicode* pEnd;
/*N*/     rVal = rtl_math_uStringToDouble( rCommand.GetBuffer() + rCommandPos,
/*N*/             rCommand.GetBuffer() + rCommand.Len(),
/*N*/             pLclD->getNumDecimalSep().GetChar(0),
/*N*/             pLclD->getNumThousandSep().GetChar(0),
/*N*/             &eStatus, &pEnd );
/*N*/ 	rCommandPos = pEnd - rCommand.GetBuffer();
/*N*/ 
/*N*/ 	if( pLclD != &GetAppLocaleData() )
/*N*/ 		delete (LocaleDataWrapper*)pLclD;
/*N*/ 
/*N*/ 	return rtl_math_ConversionStatus_Ok == eStatus && nCurrCmdPos != rCommandPos;
/*N*/ }

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

/******************************************************************************
|*
|*	CTOR DTOR der SwHash classes
|*
|*	Ersterstellung		OK 25.06.93 12:20
|*	Letzte Aenderung	OK 25.06.93 12:20
|*
******************************************************************************/

/*N*/ SwHash::SwHash( const String& rStr ) :
/*N*/ 	aStr( rStr ),
/*N*/ 	pNext( 0 )
/*N*/ {}

/*N*/ SwHash::~SwHash()
/*N*/ {
/*N*/ 	if( pNext )
/*N*/ 		delete pNext;
/*N*/ }

/*N*/ void DeleteHashTable( SwHash **ppHashTable, USHORT nCount )
/*N*/ {
/*N*/ 	for ( USHORT i = 0; i < nCount; ++i )
/*N*/ 		delete *(ppHashTable+i);
/*N*/ 	delete [] ppHashTable;
/*N*/ }

/*N*/ SwCalcExp::SwCalcExp( const String& rStr, const SwSbxValue& rVal,
/*N*/ 					const SwFieldType* pType )
/*N*/ 	: SwHash( rStr ),
/*N*/ 	nValue( rVal ),
/*N*/ 	pFldType( pType )
/*N*/ {
/*N*/ }


/*N*/ SwSbxValue::~SwSbxValue()
/*N*/ {
/*N*/ }


/*N*/ double SwSbxValue::GetDouble() const
/*N*/ {
/*N*/ 	double nRet;
/*N*/ 	if( SbxSTRING == GetType() )
/*N*/ 	{
/*?*/ 		xub_StrLen nStt = 0;
/*?*/ 		SwCalc::Str2Double( GetString(), nStt, nRet );
/*N*/ 	}
/*N*/ 	else if (IsBool())
/*N*/ 	{
/*N*/ 	    nRet = 0 != GetBool() ? 1.0 : 0.0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nRet = SbxValue::GetDouble();
/*N*/ 	return nRet;
/*N*/ }

/*N*/ SwSbxValue& SwSbxValue::MakeDouble()
/*N*/ {
/*N*/ 	if( SbxSTRING == GetType() )
/*?*/ 		PutDouble( GetDouble() );
/*N*/ 	return *this;
/*N*/ }

#ifdef STANDALONE_HASHCALC

// dies ist der Beispielcode zu erzeugen der HashValues im CTOR:

#include <stdio.h>

void main()
{
static sal_Char
    sNType0[] = "false", 	sNType1[] = "true",		sNType2[] = "pi",
    sNType3[] = "e",		sNType4[] = "tables",	sNType5[] = "graf",
    sNType6[] = "ole",		sNType7[] = "page",		sNType8[] = "para",
    sNType9[] = "word",		sNType10[]= "char",
    sNType11[] = "user_company" ,		sNType12[] = "user_firstname" ,
    sNType13[] = "user_lastname" ,		sNType14[] = "user_initials",
    sNType15[] = "user_street" ,        sNType16[] = "user_country" ,
    sNType17[] = "user_zipcode" ,       sNType18[] = "user_city" ,
    sNType19[] = "user_title" ,         sNType20[] = "user_position" ,
    sNType21[] = "user_tel_home",	    sNType22[] = "user_tel_work",
    sNType23[] = "user_fax" ,           sNType24[] = "user_email" ,
    sNType25[] = "user_state",			sNType26[] = "graph"
    ;

static const sal_Char* sNTypeTab[ 27 ] =
{
    sNType0, sNType1, sNType2, sNType3, sNType4, sNType5,
    sNType6, sNType7, sNType8, sNType9, sNType10, sNType11,
    sNType12, sNType13, sNType14, sNType15, sNType16, sNType17,
    sNType18, sNType19, sNType20, sNType21, sNType22, sNType23,
    sNType24, sNType25, sNType26
};

    const unsigned short nTblSize = 47;
    int aArr[ nTblSize ] = { 0 };
    sal_Char ch;

    for( int n = 0; n < 27; ++n )
    {
        unsigned long ii = 0;
        const sal_Char* pp = sNTypeTab[ n ];

        while( *pp )
            ii = ii << 1 ^ *pp++;
        ii %= nTblSize;

        ch = aArr[ ii ] ? 'X' : ' ';
        aArr[ ii ] = 1;
        printf( "%-20s -> %3d [%c]\n", sNTypeTab[ n ], ii, ch );
    }
}

#endif



}
