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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>

#include "cfgids.hxx"
#include "docoptio.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "miscuno.hxx"
#include "global.hxx"

using namespace utl;
using namespace com::sun::star::uno;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::i18n::LocaleDataItem;
using ::rtl::OUString;

//------------------------------------------------------------------------

#define SC_VERSION ((USHORT)251)

TYPEINIT1(ScTpCalcItem, SfxPoolItem);

//------------------------------------------------------------------------

//! these functions should be moved to some header file
inline long TwipsToHMM(long nTwips) { return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)   { return (nHMM * 72 + 63) / 127; }

inline long TwipsToEvenHMM(long nTwips) { return ( (nTwips * 127 + 72) / 144 ) * 2; }

//------------------------------------------------------------------------

USHORT lcl_GetDefaultTabDist()
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
            eKeyBindingType( rCpy.eKeyBindingType ),
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
            bFormulaRegexEnabled( rCpy.bFormulaRegexEnabled ),
            bUseEnglishFuncName( rCpy.bUseEnglishFuncName ),
            eFormulaGrammar( rCpy.eFormulaGrammar ),
            aFormulaSepArg( rCpy.aFormulaSepArg ),
            aFormulaSepArrayRow( rCpy.aFormulaSepArrayRow ),
            aFormulaSepArrayCol( rCpy.aFormulaSepArrayCol )
{
}

//------------------------------------------------------------------------

ScDocOptions::~ScDocOptions()
{
}

//------------------------------------------------------------------------

void ScDocOptions::ResetDocOptions()
{
    bIsIgnoreCase       = FALSE;
    bIsIter             = FALSE;
    nIterCount          = 100;
    fIterEps            = 1.0E-3;
    nPrecStandardFormat = SvNumberFormatter::UNLIMITED_PRECISION;
    eKeyBindingType     = ScOptionsUtil::KEY_DEFAULT;
    nDay                = 30;
    nMonth              = 12;
    nYear               = 1899;
    nYear2000           = SvNumberFormatter::GetYear2000Default();
    nTabDistance        = lcl_GetDefaultTabDist();
    bCalcAsShown        = FALSE;
    bMatchWholeCell     = TRUE;
    bDoAutoSpell        = FALSE;
    bLookUpColRowNames  = TRUE;
    bFormulaRegexEnabled= TRUE;
    bUseEnglishFuncName = false;
    eFormulaGrammar     = ::formula::FormulaGrammar::GRAM_NATIVE;

    ResetFormulaSeparators();
}

void ScDocOptions::ResetFormulaSeparators()
{
    // Defaults to the old separator values.
    aFormulaSepArg = OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
    aFormulaSepArrayCol = OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
    aFormulaSepArrayRow = OUString(RTL_CONSTASCII_USTRINGPARAM("|"));

    const Locale& rLocale = *ScGlobal::GetLocale();
    const OUString& rLang = rLocale.Language;
    if (rLang.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ru")))
        // Don't do automatic guess for these languages, and fall back to
        // the old separator set.
        return;

    const LocaleDataWrapper& rLocaleData = GetLocaleDataWrapper();
    const OUString& rDecSep  = rLocaleData.getNumDecimalSep();
    const OUString& rListSep = rLocaleData.getListSep();

    if (!rDecSep.getLength() || !rListSep.getLength())
        // Something is wrong.  Stick with the default separators.
        return;

    sal_Unicode cDecSep  = rDecSep.getStr()[0];
    sal_Unicode cListSep = rListSep.getStr()[0];

    // Excel by default uses system's list separator as the parameter
    // separator, which in English locales is a comma.  However, OOo's list
    // separator value is set to ';' for all English locales.  Because of this
    // discrepancy, we will hardcode the separator value here, for now.
    if (cDecSep == sal_Unicode('.'))
        cListSep = sal_Unicode(',');

    // Special case for de_CH locale.
    if (rLocale.Language.equalsAsciiL("de", 2) && rLocale.Country.equalsAsciiL("CH", 2))
        cListSep = sal_Unicode(';');

    // by default, the parameter separator equals the locale-specific
    // list separator.
    aFormulaSepArg = OUString(cListSep);

    if (cDecSep == cListSep && cDecSep != sal_Unicode(';'))
        // if the decimal and list separators are equal, set the
        // parameter separator to be ';', unless they are both
        // semicolon in which case don't change the decimal separator.
        aFormulaSepArg = OUString(RTL_CONSTASCII_USTRINGPARAM(";"));

    aFormulaSepArrayCol = OUString(RTL_CONSTASCII_USTRINGPARAM(","));
    if (cDecSep == sal_Unicode(','))
        aFormulaSepArrayCol = OUString(RTL_CONSTASCII_USTRINGPARAM("."));
    aFormulaSepArrayRow = OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
}

const LocaleDataWrapper& ScDocOptions::GetLocaleDataWrapper()
{
    return *ScGlobal::pLocaleData;
}

//========================================================================
//      ScTpCalcItem - Daten fuer die CalcOptions-TabPage
//========================================================================

//------------------------------------------------------------------------

ScTpCalcItem::ScTpCalcItem( USHORT nWhichP, const ScDocOptions& rOpt )
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

ScTpCalcItem::~ScTpCalcItem()
{
}

//------------------------------------------------------------------------

String ScTpCalcItem::GetValueText() const
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("ScTpCalcItem") );
}

//------------------------------------------------------------------------

int ScTpCalcItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpCalcItem& rPItem = (const ScTpCalcItem&)rItem;

    return ( theOptions == rPItem.theOptions );
}

//------------------------------------------------------------------------

SfxPoolItem* ScTpCalcItem::Clone( SfxItemPool * ) const
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

#define CFGPATH_FORMULA     "Office.Calc/Formula"
#define SCFORMULAOPT_GRAMMAR           0
#define SCFORMULAOPT_ENGLISH_FUNCNAME  1
#define SCFORMULAOPT_SEP_ARG           2
#define SCFORMULAOPT_SEP_ARRAY_ROW     3
#define SCFORMULAOPT_SEP_ARRAY_COL     4
#define SCFORMULAOPT_COUNT             5

#define CFGPATH_DOCLAYOUT   "Office.Calc/Layout/Other"

#define SCDOCLAYOUTOPT_TABSTOP      0
#define SCDOCLAYOUTOPT_COUNT        1

#define CFGPATH_COMPAT      "Office.Calc/Compatibility"
#define SCCOMPATOPT_KEY_BINDING     0
#define SCCOMPATOPT_COUNT           1

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
        "Other/RegularExpressions",         // SCCALCOPT_REGEX
    };
    Sequence<OUString> aNames(SCCALCOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCCALCOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScDocCfg::GetFormulaPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Syntax/Grammar",             // SCFORMULAOPT_GRAMMAR
        "Syntax/EnglishFunctionName", // SCFORMULAOPT_ENGLISH_FUNCNAME
        "Syntax/SeparatorArg",        // SCFORMULAOPT_SEP_ARG
        "Syntax/SeparatorArrayRow",   // SCFORMULAOPT_SEP_ARRAY_ROW
        "Syntax/SeparatorArrayCol",   // SCFORMULAOPT_SEP_ARRAY_COL
    };
    Sequence<OUString> aNames(SCFORMULAOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for (int i = 0; i < SCFORMULAOPT_COUNT; ++i)
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
        pNames[SCDOCLAYOUTOPT_TABSTOP] = OUString(RTL_CONSTASCII_USTRINGPARAM( "TabStop/Metric") );

    return aNames;
}

Sequence<OUString> ScDocCfg::GetCompatPropertyNames()
{
    static const char* aPropNames[] =
    {
        "KeyBindings/BaseGroup"             // SCCOMPATOPT_KEY_BINDING
    };
    Sequence<OUString> aNames(SCCOMPATOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for (int i = 0; i < SCCOMPATOPT_COUNT; ++i)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScDocCfg::ScDocCfg() :
    aCalcItem( OUString(RTL_CONSTASCII_USTRINGPARAM( CFGPATH_CALC )) ),
    aFormulaItem(OUString(RTL_CONSTASCII_USTRINGPARAM(CFGPATH_FORMULA))),
    aLayoutItem(OUString(RTL_CONSTASCII_USTRINGPARAM(CFGPATH_DOCLAYOUT))),
    aCompatItem(OUString(RTL_CONSTASCII_USTRINGPARAM(CFGPATH_COMPAT)))
{
    sal_Int32 nIntVal = 0;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = NULL;

    USHORT nDateDay, nDateMonth, nDateYear;
    GetDate( nDateDay, nDateMonth, nDateYear );

    aNames = GetCalcPropertyNames();
    aValues = aCalcItem.GetProperties(aNames);
    aCalcItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        double fDoubleVal = 0;
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
                        if (pValues[nProp] >>= nIntVal) SetIterCount( (USHORT) nIntVal );
                        break;
                    case SCCALCOPT_ITER_MINCHG:
                        if (pValues[nProp] >>= fDoubleVal) SetIterEps( fDoubleVal );
                        break;
                    case SCCALCOPT_DATE_DAY:
                        if (pValues[nProp] >>= nIntVal) nDateDay = (USHORT) nIntVal;
                        break;
                    case SCCALCOPT_DATE_MONTH:
                        if (pValues[nProp] >>= nIntVal) nDateMonth = (USHORT) nIntVal;
                        break;
                    case SCCALCOPT_DATE_YEAR:
                        if (pValues[nProp] >>= nIntVal) nDateYear = (USHORT) nIntVal;
                        break;
                    case SCCALCOPT_DECIMALS:
                        if (pValues[nProp] >>= nIntVal) SetStdPrecision( (USHORT) nIntVal );
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

    aNames = GetFormulaPropertyNames();
    aValues = aFormulaItem.GetProperties(aNames);
    aFormulaItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    if (aValues.getLength() == aNames.getLength())
    {
        for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
        {
            switch (nProp)
            {
                case SCFORMULAOPT_GRAMMAR:
                {
                    // Get default value in case this option is not set.
                    ::formula::FormulaGrammar::Grammar eGram = GetFormulaSyntax();

                    do
                    {
                        if (!(pValues[nProp] >>= nIntVal))
                            // extractino failed.
                            break;

                        switch (nIntVal)
                        {
                            case 0: // Calc A1
                                eGram = ::formula::FormulaGrammar::GRAM_NATIVE;
                            break;
                            case 1: // Excel A1
                                eGram = ::formula::FormulaGrammar::GRAM_NATIVE_XL_A1;
                            break;
                            case 2: // Excel R1C1
                                eGram = ::formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1;
                            break;
                            default:
                                ;
                        }
                    }
                    while (false);
                    SetFormulaSyntax(eGram);
                }
                break;
                case SCFORMULAOPT_ENGLISH_FUNCNAME:
                {
                    sal_Bool bEnglish = false;
                    if (pValues[nProp] >>= bEnglish)
                        SetUseEnglishFuncName(bEnglish);
                }
                break;
                case SCFORMULAOPT_SEP_ARG:
                {
                    OUString aSep;
                    if ((pValues[nProp] >>= aSep) && aSep.getLength())
                        SetFormulaSepArg(aSep);
                }
                break;
                case SCFORMULAOPT_SEP_ARRAY_ROW:
                {
                    OUString aSep;
                    if ((pValues[nProp] >>= aSep) && aSep.getLength())
                        SetFormulaSepArrayRow(aSep);
                }
                break;
                case SCFORMULAOPT_SEP_ARRAY_COL:
                {
                    OUString aSep;
                    if ((pValues[nProp] >>= aSep) && aSep.getLength())
                        SetFormulaSepArrayCol(aSep);
                }
                break;
            }
        }
    }
    aFormulaItem.SetCommitLink( LINK(this, ScDocCfg, FormulaCommitHdl) );

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
                            SetTabDistance( (USHORT) HMMToTwips( nIntVal ) );
                        break;
                }
            }
        }
    }
    aLayoutItem.SetCommitLink( LINK( this, ScDocCfg, LayoutCommitHdl ) );

    aNames = GetCompatPropertyNames();
    aValues = aCompatItem.GetProperties(aNames);
    aCompatItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    if (aValues.getLength() == aNames.getLength())
    {
        for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
        {
            switch (nProp)
            {
                case SCCOMPATOPT_KEY_BINDING:
                {
                    nIntVal = 0; // 0 = 'Default'
                    pValues[nProp] >>= nIntVal;
                    SetKeyBindingType(static_cast<ScOptionsUtil::KeyBindingType>(nIntVal));
                }
                break;
            }
        }
    }
    aCompatItem.SetCommitLink( LINK(this, ScDocCfg, CompatCommitHdl) );
}

IMPL_LINK( ScDocCfg, CalcCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetCalcPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    USHORT nDateDay, nDateMonth, nDateYear;
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

IMPL_LINK( ScDocCfg, FormulaCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetFormulaPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        switch (nProp)
        {
            case SCFORMULAOPT_GRAMMAR :
            {
                sal_Int32 nVal = 0;
                switch (GetFormulaSyntax())
                {
                    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_A1:    nVal = 1; break;
                    case ::formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1:  nVal = 2; break;
                    default: break;
                }
                pValues[nProp] <<= nVal;
            }
            break;
            case SCFORMULAOPT_ENGLISH_FUNCNAME:
            {
                sal_Bool b = GetUseEnglishFuncName();
                pValues[nProp] <<= b;
            }
            break;
            case SCFORMULAOPT_SEP_ARG:
                pValues[nProp] <<= GetFormulaSepArg();
            break;
            case SCFORMULAOPT_SEP_ARRAY_ROW:
                pValues[nProp] <<= GetFormulaSepArrayRow();
            break;
            case SCFORMULAOPT_SEP_ARRAY_COL:
                pValues[nProp] <<= GetFormulaSepArrayCol();
            break;
        }
    }
    aFormulaItem.PutProperties(aNames, aValues);

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

IMPL_LINK( ScDocCfg, CompatCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetCompatPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        switch(nProp)
        {
            case SCCOMPATOPT_KEY_BINDING:
                pValues[nProp] <<= static_cast<sal_Int32>(GetKeyBindingType());
            break;
        }
    }
    aCompatItem.PutProperties(aNames, aValues);
    return 0;
}

void ScDocCfg::SetOptions( const ScDocOptions& rNew )
{
    *(ScDocOptions*)this = rNew;

    aCalcItem.SetModified();
    aFormulaItem.SetModified();
    aLayoutItem.SetModified();
    aCompatItem.SetModified();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
