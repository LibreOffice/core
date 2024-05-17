/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <formulaopt.hxx>
#include <global.hxx>
#include <formulagroup.hxx>
#include <sc.hrc>
#include <utility>

using namespace utl;
using namespace com::sun::star::uno;
namespace lang = ::com::sun::star::lang;


ScFormulaOptions::ScFormulaOptions()
{
    SetDefaults();
}

void ScFormulaOptions::SetDefaults()
{
    bUseEnglishFuncName = false;
    eFormulaGrammar     = ::formula::FormulaGrammar::GRAM_NATIVE;
    mbWriteCalcConfig = true;
    meOOXMLRecalc = RECALC_ASK;
    meODFRecalc = RECALC_ASK;
    meReCalcOptiRowHeights = RECALC_ASK;

    // unspecified means use the current formula syntax.
    aCalcConfig.reset();

    ResetFormulaSeparators();
}

void ScFormulaOptions::ResetFormulaSeparators()
{
    GetDefaultFormulaSeparators(aFormulaSepArg, aFormulaSepArrayCol, aFormulaSepArrayRow);
}

void ScFormulaOptions::GetDefaultFormulaSeparators(
    OUString& rSepArg, OUString& rSepArrayCol, OUString& rSepArrayRow)
{
    // Defaults to the old separator values.
    rSepArg = ";";
    rSepArrayCol = ";";
    rSepArrayRow = "|";

    const lang::Locale& rLocale = ScGlobal::GetLocale();
    const OUString& rLang = rLocale.Language;
    if (rLang == "ru")
        // Don't do automatic guess for these languages, and fall back to
        // the old separator set.
        return;

    const LocaleDataWrapper& rLocaleData = ScGlobal::getLocaleData();
    const OUString& rDecSep  = rLocaleData.getNumDecimalSep();
    const OUString& rListSep = rLocaleData.getListSep();

    if (rDecSep.isEmpty() || rListSep.isEmpty())
        // Something is wrong.  Stick with the default separators.
        return;

    sal_Unicode cDecSep  = rDecSep[0];
    sal_Unicode cListSep = rListSep[0];
    sal_Unicode cDecSepAlt = rLocaleData.getNumDecimalSepAlt().toChar();    // usually 0 (empty)

    // Excel by default uses system's list separator as the parameter
    // separator, which in English locales is a comma.  However, OOo's list
    // separator value is set to ';' for all English locales.  Because of this
    // discrepancy, we will hardcode the separator value here, for now.
    // Similar for decimal separator alternative.
    // However, if the decimal separator alternative is '.' and the decimal
    // separator is ',' this makes no sense, fall back to ';' in that case.
    if (cDecSep == '.' || (cDecSepAlt == '.' && cDecSep != ','))
        cListSep = ',';
    else if (cDecSep == ',' && cDecSepAlt == '.')
        cListSep = ';';

    // Special case for de_CH locale.
    if (rLocale.Language == "de" && rLocale.Country == "CH")
        cListSep = ';';

    // by default, the parameter separator equals the locale-specific
    // list separator.
    rSepArg = OUString(cListSep);

    if (cDecSep == cListSep && cDecSep != ';')
        // if the decimal and list separators are equal, set the
        // parameter separator to be ';', unless they are both
        // semicolon in which case don't change the decimal separator.
        rSepArg = ";";

    rSepArrayCol = ",";
    if (cDecSep == ',')
        rSepArrayCol = ".";
    rSepArrayRow = ";";
}

bool ScFormulaOptions::operator==( const ScFormulaOptions& rOpt ) const
{
    return bUseEnglishFuncName == rOpt.bUseEnglishFuncName
        && eFormulaGrammar     == rOpt.eFormulaGrammar
        && aCalcConfig == rOpt.aCalcConfig
        && mbWriteCalcConfig   == rOpt.mbWriteCalcConfig
        && aFormulaSepArg      == rOpt.aFormulaSepArg
        && aFormulaSepArrayRow == rOpt.aFormulaSepArrayRow
        && aFormulaSepArrayCol == rOpt.aFormulaSepArrayCol
        && meOOXMLRecalc       == rOpt.meOOXMLRecalc
        && meODFRecalc         == rOpt.meODFRecalc
        && meReCalcOptiRowHeights == rOpt.meReCalcOptiRowHeights;
}

bool ScFormulaOptions::operator!=( const ScFormulaOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

ScTpFormulaItem::ScTpFormulaItem( ScFormulaOptions aOpt ) :
    SfxPoolItem ( SID_SCFORMULAOPTIONS ),
    theOptions  (std::move( aOpt ))
{
}

ScTpFormulaItem::~ScTpFormulaItem()
{
}

bool ScTpFormulaItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScTpFormulaItem& rPItem = static_cast<const ScTpFormulaItem&>(rItem);
    return ( theOptions == rPItem.theOptions );
}

ScTpFormulaItem* ScTpFormulaItem::Clone( SfxItemPool * ) const
{
    return new ScTpFormulaItem( *this );
}

constexpr OUStringLiteral CFGPATH_FORMULA = u"Office.Calc/Formula";

#define SCFORMULAOPT_GRAMMAR              0
#define SCFORMULAOPT_ENGLISH_FUNCNAME     1
#define SCFORMULAOPT_SEP_ARG              2
#define SCFORMULAOPT_SEP_ARRAY_ROW        3
#define SCFORMULAOPT_SEP_ARRAY_COL        4
#define SCFORMULAOPT_STRING_REF_SYNTAX    5
#define SCFORMULAOPT_STRING_CONVERSION    6
#define SCFORMULAOPT_EMPTY_OUSTRING_AS_ZERO 7
#define SCFORMULAOPT_OOXML_RECALC         8
#define SCFORMULAOPT_ODF_RECALC           9
#define SCFORMULAOPT_ROW_HEIGHT_RECALC   10
#define SCFORMULAOPT_OPENCL_AUTOSELECT   11
#define SCFORMULAOPT_OPENCL_DEVICE       12
#define SCFORMULAOPT_OPENCL_SUBSET_ONLY  13
#define SCFORMULAOPT_OPENCL_MIN_SIZE     14
#define SCFORMULAOPT_OPENCL_SUBSET_OPS   15

Sequence<OUString> ScFormulaCfg::GetPropertyNames()
{
    return {u"Syntax/Grammar"_ustr,                       // SCFORMULAOPT_GRAMMAR
            u"Syntax/EnglishFunctionName"_ustr,           // SCFORMULAOPT_ENGLISH_FUNCNAME
            u"Syntax/SeparatorArg"_ustr,                  // SCFORMULAOPT_SEP_ARG
            u"Syntax/SeparatorArrayRow"_ustr,             // SCFORMULAOPT_SEP_ARRAY_ROW
            u"Syntax/SeparatorArrayCol"_ustr,             // SCFORMULAOPT_SEP_ARRAY_COL
            u"Syntax/StringRefAddressSyntax"_ustr,        // SCFORMULAOPT_STRING_REF_SYNTAX
            u"Syntax/StringConversion"_ustr,              // SCFORMULAOPT_STRING_CONVERSION
            u"Syntax/EmptyStringAsZero"_ustr,             // SCFORMULAOPT_EMPTY_OUSTRING_AS_ZERO
            u"Load/OOXMLRecalcMode"_ustr,                 // SCFORMULAOPT_OOXML_RECALC
            u"Load/ODFRecalcMode"_ustr,                   // SCFORMULAOPT_ODF_RECALC
            u"Load/RecalcOptimalRowHeightMode"_ustr,      // SCFORMULAOPT_ROW_HEIGHT_RECALC
            u"Calculation/OpenCLAutoSelect"_ustr,         // SCFORMULAOPT_OPENCL_AUTOSELECT
            u"Calculation/OpenCLDevice"_ustr,             // SCFORMULAOPT_OPENCL_DEVICE
            u"Calculation/OpenCLSubsetOnly"_ustr,         // SCFORMULAOPT_OPENCL_SUBSET_ONLY
            u"Calculation/OpenCLMinimumDataSize"_ustr,    // SCFORMULAOPT_OPENCL_MIN_SIZE
            u"Calculation/OpenCLSubsetOpCodes"_ustr};     // SCFORMULAOPT_OPENCL_SUBSET_OPS
}

ScFormulaCfg::PropsToIds ScFormulaCfg::GetPropNamesToId()
{
    Sequence<OUString> aPropNames = GetPropertyNames();
    static sal_uInt16 aVals[] = {
        SCFORMULAOPT_GRAMMAR,
        SCFORMULAOPT_ENGLISH_FUNCNAME,
        SCFORMULAOPT_SEP_ARG,
        SCFORMULAOPT_SEP_ARRAY_ROW,
        SCFORMULAOPT_SEP_ARRAY_COL,
        SCFORMULAOPT_STRING_REF_SYNTAX,
        SCFORMULAOPT_STRING_CONVERSION,
        SCFORMULAOPT_EMPTY_OUSTRING_AS_ZERO,
        SCFORMULAOPT_OOXML_RECALC,
        SCFORMULAOPT_ODF_RECALC,
        SCFORMULAOPT_ROW_HEIGHT_RECALC,
        SCFORMULAOPT_OPENCL_AUTOSELECT,
        SCFORMULAOPT_OPENCL_DEVICE,
        SCFORMULAOPT_OPENCL_SUBSET_ONLY,
        SCFORMULAOPT_OPENCL_MIN_SIZE,
        SCFORMULAOPT_OPENCL_SUBSET_OPS,
    };
    OSL_ENSURE( SAL_N_ELEMENTS(aVals) == aPropNames.getLength(), "Properties and ids are out of Sync");
    PropsToIds aPropIdMap;
    for ( sal_Int32 i=0; i<aPropNames.getLength(); ++i )
        aPropIdMap[aPropNames[i]] = aVals[ i ];
    return aPropIdMap;
}

ScFormulaCfg::ScFormulaCfg() :
    ConfigItem( CFGPATH_FORMULA )
{
    Sequence<OUString> aNames = GetPropertyNames();
    UpdateFromProperties( aNames );
    EnableNotification( aNames );
}

void ScFormulaCfg::UpdateFromProperties( const Sequence<OUString>& aNames )
{
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    PropsToIds aPropMap = GetPropNamesToId();
    if(aValues.getLength() != aNames.getLength())
        return;

    sal_Int32 nIntVal = 0;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        PropsToIds::iterator it_end = aPropMap.end();
        PropsToIds::iterator it = aPropMap.find( aNames[nProp] );
        if(pValues[nProp].hasValue() && it != it_end )
        {
            switch(it->second)
            {
            case SCFORMULAOPT_GRAMMAR:
            {
                // Get default value in case this option is not set.
                ::formula::FormulaGrammar::Grammar eGram = GetFormulaSyntax();

                do
                {
                    if (!(pValues[nProp] >>= nIntVal))
                        // extracting failed.
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
                bool bEnglish = false;
                if (pValues[nProp] >>= bEnglish)
                    SetUseEnglishFuncName(bEnglish);
            }
            break;
            case SCFORMULAOPT_SEP_ARG:
            {
                OUString aSep;
                if ((pValues[nProp] >>= aSep) && !aSep.isEmpty())
                    SetFormulaSepArg(aSep);
            }
            break;
            case SCFORMULAOPT_SEP_ARRAY_ROW:
            {
                OUString aSep;
                if ((pValues[nProp] >>= aSep) && !aSep.isEmpty())
                    SetFormulaSepArrayRow(aSep);
            }
            break;
            case SCFORMULAOPT_SEP_ARRAY_COL:
            {
                OUString aSep;
                if ((pValues[nProp] >>= aSep) && !aSep.isEmpty())
                    SetFormulaSepArrayCol(aSep);
            }
            break;
            case SCFORMULAOPT_STRING_REF_SYNTAX:
            {
                // Get default value in case this option is not set.
                ::formula::FormulaGrammar::AddressConvention eConv = GetCalcConfig().meStringRefAddressSyntax;

                do
                {
                    if (!(pValues[nProp] >>= nIntVal))
                        // extraction failed.
                        break;

                    switch (nIntVal)
                    {
                        case -1: // Same as the formula grammar.
                            eConv = formula::FormulaGrammar::CONV_UNSPECIFIED;
                        break;
                        case 0: // Calc A1
                            eConv = formula::FormulaGrammar::CONV_OOO;
                        break;
                        case 1: // Excel A1
                            eConv = formula::FormulaGrammar::CONV_XL_A1;
                        break;
                        case 2: // Excel R1C1
                            eConv = formula::FormulaGrammar::CONV_XL_R1C1;
                        break;
                        case 3: // Calc A1 | Excel A1
                            eConv = formula::FormulaGrammar::CONV_A1_XL_A1;
                        break;
                        default:
                            ;
                    }
                }
                while (false);
                GetCalcConfig().meStringRefAddressSyntax = eConv;
            }
            break;
            case SCFORMULAOPT_STRING_CONVERSION:
            {
                // Get default value in case this option is not set.
                ScCalcConfig::StringConversion eConv = GetCalcConfig().meStringConversion;

                do
                {
                    if (!(pValues[nProp] >>= nIntVal))
                        // extraction failed.
                        break;

                    switch (nIntVal)
                    {
                        case 0:
                            eConv = ScCalcConfig::StringConversion::ILLEGAL;
                        break;
                        case 1:
                            eConv = ScCalcConfig::StringConversion::ZERO;
                        break;
                        case 2:
                            eConv = ScCalcConfig::StringConversion::UNAMBIGUOUS;
                        break;
                        case 3:
                            eConv = ScCalcConfig::StringConversion::LOCALE;
                        break;
                        default:
                            SAL_WARN("sc", "unknown string conversion option!");
                    }
                }
                while (false);
                GetCalcConfig().meStringConversion = eConv;
            }
            break;
            case SCFORMULAOPT_EMPTY_OUSTRING_AS_ZERO:
            {
                bool bVal = GetCalcConfig().mbEmptyStringAsZero;
                pValues[nProp] >>= bVal;
                GetCalcConfig().mbEmptyStringAsZero = bVal;
            }
            break;
            case SCFORMULAOPT_OOXML_RECALC:
            {
                ScRecalcOptions eOpt = RECALC_ASK;
                if (pValues[nProp] >>= nIntVal)
                {
                    switch (nIntVal)
                    {
                        case 0:
                            eOpt = RECALC_ALWAYS;
                            break;
                        case 1:
                            eOpt = RECALC_NEVER;
                            break;
                        case 2:
                            eOpt = RECALC_ASK;
                            break;
                        default:
                            SAL_WARN("sc", "unknown ooxml recalc option!");
                    }
                }

                SetOOXMLRecalcOptions(eOpt);
            }
            break;
            case SCFORMULAOPT_ODF_RECALC:
            {
                ScRecalcOptions eOpt = RECALC_ASK;
                if (pValues[nProp] >>= nIntVal)
                {
                    switch (nIntVal)
                    {
                        case 0:
                            eOpt = RECALC_ALWAYS;
                            break;
                        case 1:
                            eOpt = RECALC_NEVER;
                            break;
                        case 2:
                            eOpt = RECALC_ASK;
                            break;
                        default:
                            SAL_WARN("sc", "unknown odf recalc option!");
                    }
                }

                SetODFRecalcOptions(eOpt);
            }
            break;
            case SCFORMULAOPT_ROW_HEIGHT_RECALC:
            {
                ScRecalcOptions eOpt = RECALC_ASK;
                if (pValues[nProp] >>= nIntVal)
                {
                    switch (nIntVal)
                    {
                    case 0:
                        eOpt = RECALC_ALWAYS;
                        break;
                    case 1:
                        eOpt = RECALC_NEVER;
                        break;
                    case 2:
                        eOpt = RECALC_ASK;
                        break;
                    default:
                        SAL_WARN("sc", "unknown optimal row height recalc option!");
                    }
                }

                SetReCalcOptiRowHeights(eOpt);
            }
            break;
            case SCFORMULAOPT_OPENCL_AUTOSELECT:
            {
                bool bVal = GetCalcConfig().mbOpenCLAutoSelect;
                pValues[nProp] >>= bVal;
                GetCalcConfig().mbOpenCLAutoSelect = bVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_DEVICE:
            {
                OUString aOpenCLDevice = GetCalcConfig().maOpenCLDevice;
                pValues[nProp] >>= aOpenCLDevice;
                GetCalcConfig().maOpenCLDevice = aOpenCLDevice;
            }
            break;
            case SCFORMULAOPT_OPENCL_SUBSET_ONLY:
            {
                bool bVal = GetCalcConfig().mbOpenCLSubsetOnly;
                pValues[nProp] >>= bVal;
                GetCalcConfig().mbOpenCLSubsetOnly = bVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_MIN_SIZE:
            {
                sal_Int32 nVal = GetCalcConfig().mnOpenCLMinimumFormulaGroupSize;
                pValues[nProp] >>= nVal;
                GetCalcConfig().mnOpenCLMinimumFormulaGroupSize = nVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_SUBSET_OPS:
            {
                OUString sVal = ScOpCodeSetToSymbolicString(GetCalcConfig().mpOpenCLSubsetOpCodes);
                pValues[nProp] >>= sVal;
                GetCalcConfig().mpOpenCLSubsetOpCodes = ScStringToOpCodeSet(sVal);
            }
            break;
            }
        }
    }
}

void ScFormulaCfg::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    Sequence<Any> aOldValues = GetProperties(aNames);
    Any* pOldValues = aOldValues.getArray();

    bool bSetOpenCL = false;

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
                bool b = GetUseEnglishFuncName();
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
            case SCFORMULAOPT_STRING_REF_SYNTAX:
            {
                sal_Int32 nVal = -1;

                if (GetWriteCalcConfig())
                {
                    switch (GetCalcConfig().meStringRefAddressSyntax)
                    {
                        case ::formula::FormulaGrammar::CONV_OOO:     nVal = 0; break;
                        case ::formula::FormulaGrammar::CONV_XL_A1:   nVal = 1; break;
                        case ::formula::FormulaGrammar::CONV_XL_R1C1: nVal = 2; break;
                        case ::formula::FormulaGrammar::CONV_A1_XL_A1: nVal = 3; break;
                        default: break;
                    }
                    pValues[nProp] <<= nVal;
                }
                else
                {
                    pValues[nProp] = pOldValues[nProp];
                }
            }
            break;
            case SCFORMULAOPT_STRING_CONVERSION:
            {
                if (GetWriteCalcConfig())
                {
                    sal_Int32 nVal = 3;

                    switch (GetCalcConfig().meStringConversion)
                    {
                        case ScCalcConfig::StringConversion::ILLEGAL:     nVal = 0; break;
                        case ScCalcConfig::StringConversion::ZERO:        nVal = 1; break;
                        case ScCalcConfig::StringConversion::UNAMBIGUOUS: nVal = 2; break;
                        case ScCalcConfig::StringConversion::LOCALE:      nVal = 3; break;
                    }
                    pValues[nProp] <<= nVal;
                }
                else
                {
                    pValues[nProp] = pOldValues[nProp];
                }
            }
            break;
            case SCFORMULAOPT_EMPTY_OUSTRING_AS_ZERO:
            {
                if (GetWriteCalcConfig())
                {
                    bool bVal = GetCalcConfig().mbEmptyStringAsZero;
                    pValues[nProp] <<= bVal;
                }
                else
                {
                    pValues[nProp] = pOldValues[nProp];
                }
            }
            break;
            case SCFORMULAOPT_OOXML_RECALC:
            {
                sal_Int32 nVal = 2;
                switch (GetOOXMLRecalcOptions())
                {
                    case RECALC_ALWAYS:
                        nVal = 0;
                        break;
                    case RECALC_NEVER:
                        nVal = 1;
                        break;
                    case RECALC_ASK:
                        nVal = 2;
                        break;
                }

                pValues[nProp] <<= nVal;
            }
            break;
            case SCFORMULAOPT_ODF_RECALC:
            {
                sal_Int32 nVal = 2;
                switch (GetODFRecalcOptions())
                {
                    case RECALC_ALWAYS:
                        nVal = 0;
                        break;
                    case RECALC_NEVER:
                        nVal = 1;
                        break;
                    case RECALC_ASK:
                        nVal = 2;
                        break;
                }

                pValues[nProp] <<= nVal;
            }
            break;
            case SCFORMULAOPT_ROW_HEIGHT_RECALC:
            {
                sal_Int32 nVal = 2;
                switch (GetReCalcOptiRowHeights())
                {
                    case RECALC_ALWAYS:
                        nVal = 0;
                        break;
                    case RECALC_NEVER:
                        nVal = 1;
                        break;
                    case RECALC_ASK:
                        nVal = 2;
                        break;
                    default:
                        SAL_WARN("sc", "unknown optimal row height recalc option!");
                }

                pValues[nProp] <<= nVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_AUTOSELECT:
            {
                bool bVal = GetCalcConfig().mbOpenCLAutoSelect;
                pValues[nProp] <<= bVal;
                bSetOpenCL = true;
            }
            break;
            case SCFORMULAOPT_OPENCL_DEVICE:
            {
                OUString aOpenCLDevice = GetCalcConfig().maOpenCLDevice;
                pValues[nProp] <<= aOpenCLDevice;
                bSetOpenCL = true;
            }
            break;
            case SCFORMULAOPT_OPENCL_SUBSET_ONLY:
            {
                bool bVal = GetCalcConfig().mbOpenCLSubsetOnly;
                pValues[nProp] <<= bVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_MIN_SIZE:
            {
                sal_Int32 nVal = GetCalcConfig().mnOpenCLMinimumFormulaGroupSize;
                pValues[nProp] <<= nVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_SUBSET_OPS:
            {
                OUString sVal = ScOpCodeSetToSymbolicString(GetCalcConfig().mpOpenCLSubsetOpCodes);
                pValues[nProp] <<= sVal;
            }
            break;
        }
    }
#if !HAVE_FEATURE_OPENCL
    (void) bSetOpenCL;
#else
    if(bSetOpenCL)
        sc::FormulaGroupInterpreter::switchOpenCLDevice(
                GetCalcConfig().maOpenCLDevice, GetCalcConfig().mbOpenCLAutoSelect);
#endif
    PutProperties(aNames, aValues);
}

void ScFormulaCfg::SetOptions( const ScFormulaOptions& rNew )
{
    *static_cast<ScFormulaOptions*>(this) = rNew;
    SetModified();
}

void ScFormulaCfg::Notify( const css::uno::Sequence< OUString >& rNames)
{
    UpdateFromProperties( rNames );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
