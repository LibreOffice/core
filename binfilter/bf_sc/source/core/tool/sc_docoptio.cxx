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

#include <bf_svtools/zforlist.hxx>


#include "docoptio.hxx"
#include "rechead.hxx"
#include "bf_sc.hrc"
#include "miscuno.hxx"
namespace binfilter {

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------

#define SC_VERSION ((USHORT)251)


//------------------------------------------------------------------------

//!	these functions should be moved to some header file
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }


//------------------------------------------------------------------------

/*N*/ USHORT lcl_GetDefaultTabDist()
/*N*/ {
/*N*/ 	if ( ScOptionsUtil::IsMetricSystem() )
/*N*/ 		return 709;					// 1,25 cm
/*N*/ 	else
/*N*/ 		return 720;					// 1/2"
/*N*/ }

//========================================================================
//      ScDocOptions - Dokument-Optionen
//========================================================================

/*N*/ ScDocOptions::ScDocOptions()
/*N*/ {
/*N*/ 	ResetDocOptions();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScDocOptions::ScDocOptions( const ScDocOptions& rCpy )
/*N*/ 		:   bIsIgnoreCase( rCpy.bIsIgnoreCase ),
/*N*/ 			bIsIter( rCpy.bIsIter ),
/*N*/ 			nIterCount( rCpy.nIterCount ),
/*N*/ 			fIterEps( rCpy.fIterEps ),
/*N*/ 			nPrecStandardFormat( rCpy.nPrecStandardFormat ),
/*N*/ 			nDay( rCpy.nDay ),
/*N*/ 			nMonth( rCpy.nMonth ),
/*N*/ 			nYear( rCpy.nYear ),
/*N*/ 			nYear2000( rCpy.nYear2000 ),
/*N*/ 			nTabDistance( rCpy.nTabDistance ),
/*N*/ 			bCalcAsShown( rCpy.bCalcAsShown ),
/*N*/ 			bMatchWholeCell( rCpy.bMatchWholeCell ),
/*N*/ 			bDoAutoSpell( rCpy.bDoAutoSpell ),
/*N*/             bLookUpColRowNames( rCpy.bLookUpColRowNames ),
/*N*/             bFormulaRegexEnabled( rCpy.bFormulaRegexEnabled )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScDocOptions::~ScDocOptions()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScDocOptions::Save(SvStream& rStream, BOOL bConfig) const
/*N*/ {
/*N*/ 	ScWriteHeader aHdr( rStream, 28 );
/*N*/ 
/*N*/ 	rStream << bIsIgnoreCase;
/*N*/ 	rStream << bIsIter;
/*N*/ 	rStream << nIterCount;
/*N*/ 	rStream << fIterEps;
/*N*/ 	rStream << nPrecStandardFormat;
/*N*/ 	rStream << nDay;
/*N*/ 	rStream << nMonth;
/*N*/ 	rStream << nYear;
/*N*/ 	rStream << nTabDistance;
/*N*/ 	rStream << bCalcAsShown;
/*N*/ 	rStream << bMatchWholeCell;
/*N*/ 	rStream << bDoAutoSpell;
/*N*/ 	rStream << bLookUpColRowNames;
/*N*/ 
/*N*/ 	if ( bConfig || rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )		// nicht bei 4.0 Export
/*N*/ 	{
/*N*/ 		if ( !bConfig && 1901 <= nYear2000 && nYear2000 <= 1999 )
/*N*/ 		{	// fuer SO5 auf altes Format zweistellig abbilden
/*N*/ 			rStream << (USHORT) (nYear2000 - 1901);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// neues Format vierstellig, beliebiges Jahrhundert
/*N*/ 			// erzeugt in SO5 vor src513e ein Warning beim Laden
/*N*/ 			rStream << (USHORT) 29;		// Dummy, alter SO5 Default
/*N*/ 			rStream << nYear2000;		// echter Wert
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocOptions::Load(SvStream& rStream)
/*N*/ {
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream >> bIsIgnoreCase;
/*N*/ 	rStream >> bIsIter;
/*N*/ 	rStream >> nIterCount;
/*N*/ 	rStream >> fIterEps;
/*N*/ 	rStream >> nPrecStandardFormat;
/*N*/ 	rStream >> nDay;
/*N*/ 	rStream >> nMonth;
/*N*/ 	rStream >> nYear;
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 		rStream >> nTabDistance;
/*N*/ 	else
/*?*/ 		nTabDistance = lcl_GetDefaultTabDist();
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 		rStream >> bCalcAsShown;
/*N*/ 	else
/*N*/ 		bCalcAsShown = FALSE;
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 		rStream >> bMatchWholeCell;
/*N*/ 	else
/*N*/ 		bMatchWholeCell = FALSE;
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 		rStream >> bDoAutoSpell;
/*N*/ 	else
/*N*/ 		bDoAutoSpell = FALSE;
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 		rStream >> bLookUpColRowNames;
/*N*/ 	else
/*N*/ 		bLookUpColRowNames = TRUE;
/*N*/ 	if ( aHdr.BytesLeft() )
/*N*/ 	{
/*N*/ 		rStream >> nYear2000;		// SO5 ab 24.06.98
/*N*/ 		// SO51 ab src513e
/*N*/ 		if ( aHdr.BytesLeft() )
/*N*/ 			rStream >> nYear2000;	// der echte Wert
/*N*/ 		else
/*N*/ 			nYear2000 += 1901;		// altes zweistelliges auf neues vierstelliges
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nYear2000 = 18 + 1901; 		// alter Wert vor SO5
/*N*/ }

/*N*/ void ScDocOptions::ResetDocOptions()
/*N*/ {
/*N*/ 	bIsIgnoreCase		= FALSE;
/*N*/ 	bIsIter				= FALSE;
/*N*/ 	nIterCount			= 100;
/*N*/ 	fIterEps			= 1.0E-3;
/*N*/ 	nPrecStandardFormat	= 2;
/*N*/ 	nDay				= 30;
/*N*/ 	nMonth				= 12;
/*N*/ 	nYear				= 1899;
/*N*/ 	nYear2000			= SvNumberFormatter::GetYear2000Default();
/*N*/ 	nTabDistance 		= lcl_GetDefaultTabDist();
/*N*/ 	bCalcAsShown		= FALSE;
/*N*/ 	bMatchWholeCell		= TRUE;
/*N*/ 	bDoAutoSpell		= FALSE;
/*N*/ 	bLookUpColRowNames	= TRUE;
/*N*/     bFormulaRegexEnabled= TRUE;
/*N*/ }

//========================================================================
//      ScTpCalcItem - Daten fuer die CalcOptions-TabPage
//========================================================================


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//==================================================================
//	Config Item containing document options
//==================================================================

#define CFGPATH_CALC		"Office.Calc/Calculate"

#define SCCALCOPT_ITER_ITER			0
#define SCCALCOPT_ITER_STEPS		1
#define SCCALCOPT_ITER_MINCHG		2
#define SCCALCOPT_DATE_DAY			3
#define SCCALCOPT_DATE_MONTH		4
#define SCCALCOPT_DATE_YEAR			5
#define SCCALCOPT_DECIMALS			6
#define SCCALCOPT_CASESENSITIVE		7
#define SCCALCOPT_PRECISION			8
#define SCCALCOPT_SEARCHCRIT		9
#define SCCALCOPT_FINDLABEL			10
#define SCCALCOPT_REGEX             11
#define SCCALCOPT_COUNT             12

#define CFGPATH_DOCLAYOUT	"Office.Calc/Layout/Other"

#define SCDOCLAYOUTOPT_TABSTOP		0
#define SCDOCLAYOUTOPT_COUNT		1


/*N*/ Sequence<OUString> ScDocCfg::GetCalcPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"IterativeReference/Iteration",		// SCCALCOPT_ITER_ITER
/*N*/ 		"IterativeReference/Steps",			// SCCALCOPT_ITER_STEPS
/*N*/ 		"IterativeReference/MinimumChange",	// SCCALCOPT_ITER_MINCHG
/*N*/ 		"Other/Date/DD",					// SCCALCOPT_DATE_DAY
/*N*/ 		"Other/Date/MM",					// SCCALCOPT_DATE_MONTH
/*N*/ 		"Other/Date/YY",					// SCCALCOPT_DATE_YEAR
/*N*/ 		"Other/DecimalPlaces",				// SCCALCOPT_DECIMALS
/*N*/ 		"Other/CaseSensitive",				// SCCALCOPT_CASESENSITIVE
/*N*/ 		"Other/Precision",					// SCCALCOPT_PRECISION
/*N*/ 		"Other/SearchCriteria",				// SCCALCOPT_SEARCHCRIT
/*N*/         "Other/FindLabel",                  // SCCALCOPT_FINDLABEL
/*N*/         "Other/RegularExpressions"          // SCCALCOPT_REGEX
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCCALCOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCCALCOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ Sequence<OUString> ScDocCfg::GetLayoutPropertyNames()
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"TabStop/NonMetric"			// SCDOCLAYOUTOPT_TABSTOP
/*N*/ 	};
/*N*/ 	Sequence<OUString> aNames(SCDOCLAYOUTOPT_COUNT);
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	for(int i = 0; i < SCDOCLAYOUTOPT_COUNT; i++)
/*N*/ 		pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 
/*N*/ 	//	adjust for metric system
/*N*/ 	if (ScOptionsUtil::IsMetricSystem())
/*N*/ 		pNames[SCDOCLAYOUTOPT_TABSTOP] = OUString::createFromAscii( "TabStop/Metric" );
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

/*N*/ ScDocCfg::ScDocCfg() :
/*N*/ 	aCalcItem( OUString::createFromAscii( CFGPATH_CALC ) ),
/*N*/ 	aLayoutItem( OUString::createFromAscii( CFGPATH_DOCLAYOUT ) )
/*N*/ {
/*N*/ 	sal_Int32 nIntVal;
/*N*/ 	double fDoubleVal;
/*N*/ 
/*N*/ 	Sequence<OUString> aNames;
/*N*/ 	Sequence<Any> aValues;
/*N*/ 	const Any* pValues = NULL;
/*N*/ 
/*N*/ 	USHORT nDateDay, nDateMonth, nDateYear;
/*N*/ 	GetDate( nDateDay, nDateMonth, nDateYear );
/*N*/ 
/*N*/ 	aNames = GetCalcPropertyNames();
/*N*/ 	aValues = aCalcItem.GetProperties(aNames);
/*N*/ 	aCalcItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCCALCOPT_ITER_ITER:
/*N*/ 						SetIter( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_ITER_STEPS:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetIterCount( (USHORT) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_ITER_MINCHG:
/*N*/ 						if (pValues[nProp] >>= fDoubleVal) SetIterEps( fDoubleVal );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_DATE_DAY:
/*N*/ 						if (pValues[nProp] >>= nIntVal) nDateDay = (USHORT) nIntVal;
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_DATE_MONTH:
/*N*/ 						if (pValues[nProp] >>= nIntVal) nDateMonth = (USHORT) nIntVal;
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_DATE_YEAR:
/*N*/ 						if (pValues[nProp] >>= nIntVal) nDateYear = (USHORT) nIntVal;
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_DECIMALS:
/*N*/ 						if (pValues[nProp] >>= nIntVal) SetStdPrecision( (USHORT) nIntVal );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_CASESENSITIVE:
/*N*/ 						// content is reversed
/*N*/ 						SetIgnoreCase( !ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_PRECISION:
/*N*/ 						SetCalcAsShown( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_SEARCHCRIT:
/*N*/ 						SetMatchWholeCell( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/ 					case SCCALCOPT_FINDLABEL:
/*N*/ 						SetLookUpColRowNames( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/ 						break;
/*N*/                     case SCCALCOPT_REGEX :
/*N*/                         SetFormulaRegexEnabled( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
/*N*/                         break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aCalcItem.SetCommitLink( LINK( this, ScDocCfg, CalcCommitHdl ) );
/*N*/ 
/*N*/ 	SetDate( nDateDay, nDateMonth, nDateYear );
/*N*/ 
/*N*/ 	aNames = GetLayoutPropertyNames();
/*N*/ 	aValues = aLayoutItem.GetProperties(aNames);
/*N*/ 	aLayoutItem.EnableNotification(aNames);
/*N*/ 	pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == aNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
/*N*/ 			if(pValues[nProp].hasValue())
/*N*/ 			{
/*N*/ 				switch(nProp)
/*N*/ 				{
/*N*/ 					case SCDOCLAYOUTOPT_TABSTOP:
/*N*/ 						// TabDistance in ScDocOptions is in twips
/*N*/ 						if (pValues[nProp] >>= nIntVal)
/*N*/ 							SetTabDistance( (USHORT) HMMToTwips( nIntVal ) );
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aLayoutItem.SetCommitLink( LINK( this, ScDocCfg, LayoutCommitHdl ) );
/*N*/ }

/*N*/ IMPL_LINK( ScDocCfg, CalcCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetCalcPropertyNames();
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( ScDocCfg, LayoutCommitHdl, void *, EMPTYARG )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Sequence<OUString> aNames = GetLayoutPropertyNames();
/*N*/ 	return 0;
/*N*/ }




}
