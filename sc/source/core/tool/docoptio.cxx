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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "cfgids.hxx"
#include "docoptio.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

//------------------------------------------------------------------------

#define SC_VERSION ((sal_uInt16)251)

TYPEINIT1(ScTpCalcItem, SfxPoolItem);

//------------------------------------------------------------------------

//! these functions should be moved to some header file
inline long TwipsToHMM(long nTwips) { return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)   { return (nHMM * 72 + 63) / 127; }

inline long TwipsToEvenHMM(long nTwips) { return ( (nTwips * 127 + 72) / 144 ) * 2; }

//------------------------------------------------------------------------

sal_uInt16 lcl_GetDefaultTabDist()
{
    if ( ScOptionsUtil::IsMetricSystem() )
        return 709;                 // 1,25 cm
    else
        return 720;                 // 1/2"
}

//========================================================================
//      ScDocOptions - Dokument-Optionen
//========================================================================

ScDocOptions::ScDocOptions()
{
    ResetDocOptions();
}

//------------------------------------------------------------------------

ScDocOptions::ScDocOptions( const ScDocOptions& rCpy )
        :   fIterEps( rCpy.fIterEps ),
            nIterCount( rCpy.nIterCount ),
            nPrecStandardFormat( rCpy.nPrecStandardFormat ),
            nDay( rCpy.nDay ),
            nMonth( rCpy.nMonth ),
            nYear( rCpy.nYear ),
            nYear2000( rCpy.nYear2000 ),
            nTabDistance( rCpy.nTabDistance ),
            bIsIgnoreCase( rCpy.bIsIgnoreCase ),
            bIsIter( rCpy.bIsIter ),
            bCalcAsShown( rCpy.bCalcAsShown ),
            bMatchWholeCell( rCpy.bMatchWholeCell ),
            bDoAutoSpell( rCpy.bDoAutoSpell ),
            bLookUpColRowNames( rCpy.bLookUpColRowNames ),
            bFormulaRegexEnabled( rCpy.bFormulaRegexEnabled )
{
}

//------------------------------------------------------------------------

ScDocOptions::~ScDocOptions()
{
}

//------------------------------------------------------------------------

void ScDocOptions::ResetDocOptions()
{
    bIsIgnoreCase       = sal_False;
    bIsIter             = sal_False;
    nIterCount          = 100;
    fIterEps            = 1.0E-3;
    nPrecStandardFormat = SvNumberFormatter::UNLIMITED_PRECISION;
    nDay                = 30;
    nMonth              = 12;
    nYear               = 1899;
    nYear2000           = SvNumberFormatter::GetYear2000Default();
    nTabDistance        = lcl_GetDefaultTabDist();
    bCalcAsShown        = sal_False;
    bMatchWholeCell     = sal_True;
    bDoAutoSpell        = sal_False;
    bLookUpColRowNames  = sal_True;
    bFormulaRegexEnabled= sal_True;
}

//========================================================================
//      ScTpCalcItem - Daten fuer die CalcOptions-TabPage
//========================================================================

//UNUSED2008-05  ScTpCalcItem::ScTpCalcItem( sal_uInt16 nWhichP ) : SfxPoolItem( nWhichP )
//UNUSED2008-05  {
//UNUSED2008-05  }

//------------------------------------------------------------------------

ScTpCalcItem::ScTpCalcItem( sal_uInt16 nWhichP, const ScDocOptions& rOpt )
    :   SfxPoolItem ( nWhichP ),
        theOptions  ( rOpt )
{
}

//------------------------------------------------------------------------

ScTpCalcItem::ScTpCalcItem( const ScTpCalcItem& rItem )
    :   SfxPoolItem ( rItem ),
        theOptions  ( rItem.theOptions )
{
}

//------------------------------------------------------------------------

__EXPORT ScTpCalcItem::~ScTpCalcItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScTpCalcItem::GetValueText() const
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("ScTpCalcItem") );
}

//------------------------------------------------------------------------

int __EXPORT ScTpCalcItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpCalcItem& rPItem = (const ScTpCalcItem&)rItem;

    return ( theOptions == rPItem.theOptions );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScTpCalcItem::Clone( SfxItemPool * ) const
{
    return new ScTpCalcItem( *this );
}

//==================================================================
//  Config Item containing document options
//==================================================================

#define CFGPATH_CALC        "Office.Calc/Calculate"

#define SCCALCOPT_ITER_ITER         0
#define SCCALCOPT_ITER_STEPS        1
#define SCCALCOPT_ITER_MINCHG       2
#define SCCALCOPT_DATE_DAY          3
#define SCCALCOPT_DATE_MONTH        4
#define SCCALCOPT_DATE_YEAR         5
#define SCCALCOPT_DECIMALS          6
#define SCCALCOPT_CASESENSITIVE     7
#define SCCALCOPT_PRECISION         8
#define SCCALCOPT_SEARCHCRIT        9
#define SCCALCOPT_FINDLABEL         10
#define SCCALCOPT_REGEX             11
#define SCCALCOPT_COUNT             12

#define CFGPATH_DOCLAYOUT   "Office.Calc/Layout/Other"

#define SCDOCLAYOUTOPT_TABSTOP      0
#define SCDOCLAYOUTOPT_COUNT        1


Sequence<OUString> ScDocCfg::GetCalcPropertyNames()
{
    static const char* aPropNames[] =
    {
        "IterativeReference/Iteration",     // SCCALCOPT_ITER_ITER
        "IterativeReference/Steps",         // SCCALCOPT_ITER_STEPS
        "IterativeReference/MinimumChange", // SCCALCOPT_ITER_MINCHG
        "Other/Date/DD",                    // SCCALCOPT_DATE_DAY
        "Other/Date/MM",                    // SCCALCOPT_DATE_MONTH
        "Other/Date/YY",                    // SCCALCOPT_DATE_YEAR
        "Other/DecimalPlaces",              // SCCALCOPT_DECIMALS
        "Other/CaseSensitive",              // SCCALCOPT_CASESENSITIVE
        "Other/Precision",                  // SCCALCOPT_PRECISION
        "Other/SearchCriteria",             // SCCALCOPT_SEARCHCRIT
        "Other/FindLabel",                  // SCCALCOPT_FINDLABEL
        "Other/RegularExpressions"          // SCCALCOPT_REGEX
    };
    Sequence<OUString> aNames(SCCALCOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCCALCOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScDocCfg::GetLayoutPropertyNames()
{
    static const char* aPropNames[] =
    {
        "TabStop/NonMetric"         // SCDOCLAYOUTOPT_TABSTOP
    };
    Sequence<OUString> aNames(SCDOCLAYOUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCDOCLAYOUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    //  adjust for metric system
    if (ScOptionsUtil::IsMetricSystem())
        pNames[SCDOCLAYOUTOPT_TABSTOP] = OUString::createFromAscii( "TabStop/Metric" );

    return aNames;
}

ScDocCfg::ScDocCfg() :
    aCalcItem( OUString::createFromAscii( CFGPATH_CALC ) ),
    aLayoutItem( OUString::createFromAscii( CFGPATH_DOCLAYOUT ) )
{
    sal_Int32 nIntVal = 0;
    double fDoubleVal = 0;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = NULL;

    sal_uInt16 nDateDay, nDateMonth, nDateYear;
    GetDate( nDateDay, nDateMonth, nDateYear );

    aNames = GetCalcPropertyNames();
    aValues = aCalcItem.GetProperties(aNames);
    aCalcItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCCALCOPT_ITER_ITER:
                        SetIter( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_ITER_STEPS:
                        if (pValues[nProp] >>= nIntVal) SetIterCount( (sal_uInt16) nIntVal );
                        break;
                    case SCCALCOPT_ITER_MINCHG:
                        if (pValues[nProp] >>= fDoubleVal) SetIterEps( fDoubleVal );
                        break;
                    case SCCALCOPT_DATE_DAY:
                        if (pValues[nProp] >>= nIntVal) nDateDay = (sal_uInt16) nIntVal;
                        break;
                    case SCCALCOPT_DATE_MONTH:
                        if (pValues[nProp] >>= nIntVal) nDateMonth = (sal_uInt16) nIntVal;
                        break;
                    case SCCALCOPT_DATE_YEAR:
                        if (pValues[nProp] >>= nIntVal) nDateYear = (sal_uInt16) nIntVal;
                        break;
                    case SCCALCOPT_DECIMALS:
                        if (pValues[nProp] >>= nIntVal) SetStdPrecision( (sal_uInt16) nIntVal );
                        break;
                    case SCCALCOPT_CASESENSITIVE:
                        // content is reversed
                        SetIgnoreCase( !ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_PRECISION:
                        SetCalcAsShown( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_SEARCHCRIT:
                        SetMatchWholeCell( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_FINDLABEL:
                        SetLookUpColRowNames( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_REGEX :
                        SetFormulaRegexEnabled( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
    aCalcItem.SetCommitLink( LINK( this, ScDocCfg, CalcCommitHdl ) );

    SetDate( nDateDay, nDateMonth, nDateYear );

    aNames = GetLayoutPropertyNames();
    aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCDOCLAYOUTOPT_TABSTOP:
                        // TabDistance in ScDocOptions is in twips
                        if (pValues[nProp] >>= nIntVal)
                            SetTabDistance( (sal_uInt16) HMMToTwips( nIntVal ) );
                        break;
                }
            }
        }
    }
    aLayoutItem.SetCommitLink( LINK( this, ScDocCfg, LayoutCommitHdl ) );
}

IMPL_LINK( ScDocCfg, CalcCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetCalcPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    sal_uInt16 nDateDay, nDateMonth, nDateYear;
    GetDate( nDateDay, nDateMonth, nDateYear );

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCCALCOPT_ITER_ITER:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsIter() );
                break;
            case SCCALCOPT_ITER_STEPS:
                pValues[nProp] <<= (sal_Int32) GetIterCount();
                break;
            case SCCALCOPT_ITER_MINCHG:
                pValues[nProp] <<= (double) GetIterEps();
                break;
            case SCCALCOPT_DATE_DAY:
                pValues[nProp] <<= (sal_Int32) nDateDay;
                break;
            case SCCALCOPT_DATE_MONTH:
                pValues[nProp] <<= (sal_Int32) nDateMonth;
                break;
            case SCCALCOPT_DATE_YEAR:
                pValues[nProp] <<= (sal_Int32) nDateYear;
                break;
            case SCCALCOPT_DECIMALS:
                pValues[nProp] <<= (sal_Int32) GetStdPrecision();
                break;
            case SCCALCOPT_CASESENSITIVE:
                // content is reversed
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], !IsIgnoreCase() );
                break;
            case SCCALCOPT_PRECISION:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsCalcAsShown() );
                break;
            case SCCALCOPT_SEARCHCRIT:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsMatchWholeCell() );
                break;
            case SCCALCOPT_FINDLABEL:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsLookUpColRowNames() );
                break;
            case SCCALCOPT_REGEX :
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsFormulaRegexEnabled() );
        }
    }
    aCalcItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK( ScDocCfg, LayoutCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetLayoutPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCDOCLAYOUTOPT_TABSTOP:
                //  TabDistance in ScDocOptions is in twips
                //  use only even numbers, so defaults don't get changed
                //  by modifying other settings in the same config item
                pValues[nProp] <<= (sal_Int32) TwipsToEvenHMM( GetTabDistance() );
                break;
        }
    }
    aLayoutItem.PutProperties(aNames, aValues);

    return 0;
}


void ScDocCfg::SetOptions( const ScDocOptions& rNew )
{
    *(ScDocOptions*)this = rNew;

    aCalcItem.SetModified();
    aLayoutItem.SetModified();
}


