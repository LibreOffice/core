/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>

#include "formulaopt.hxx"
#include "miscuno.hxx"
#include "global.hxx"
#include "formulagroup.hxx"

using namespace utl;
using namespace com::sun::star::uno;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::i18n::LocaleDataItem;

TYPEINIT1(ScTpFormulaItem, SfxPoolItem);

ScFormulaOptions::ScFormulaOptions()
{
    SetDefaults();
}

ScFormulaOptions::ScFormulaOptions( const ScFormulaOptions& rCpy ) :
    bUseEnglishFuncName ( rCpy.bUseEnglishFuncName ),
    eFormulaGrammar     ( rCpy.eFormulaGrammar ),
    aCalcConfig(rCpy.aCalcConfig),
    aFormulaSepArg      ( rCpy.aFormulaSepArg ),
    aFormulaSepArrayRow ( rCpy.aFormulaSepArrayRow ),
    aFormulaSepArrayCol ( rCpy.aFormulaSepArrayCol ),
    meOOXMLRecalc       ( rCpy.meOOXMLRecalc ),
    meODFRecalc         ( rCpy.meODFRecalc )
{
}

ScFormulaOptions::~ScFormulaOptions()
{
}

void ScFormulaOptions::SetDefaults()
{
    bUseEnglishFuncName = false;
    eFormulaGrammar     = ::formula::FormulaGrammar::GRAM_NATIVE;
    meOOXMLRecalc = RECALC_ASK;
    meODFRecalc = RECALC_ASK;

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
    rSepArg = OUString(";");
    rSepArrayCol = OUString(";");
    rSepArrayRow = OUString("|");

    const Locale& rLocale = *ScGlobal::GetLocale();
    const OUString& rLang = rLocale.Language;
    if (rLang.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ru")))
        // Don't do automatic guess for these languages, and fall back to
        // the old separator set.
        return;

    const LocaleDataWrapper& rLocaleData = GetLocaleDataWrapper();
    const OUString& rDecSep  = rLocaleData.getNumDecimalSep();
    const OUString& rListSep = rLocaleData.getListSep();

    if (rDecSep.isEmpty() || rListSep.isEmpty())
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
    rSepArg = OUString(cListSep);

    if (cDecSep == cListSep && cDecSep != sal_Unicode(';'))
        // if the decimal and list separators are equal, set the
        // parameter separator to be ';', unless they are both
        // semicolon in which case don't change the decimal separator.
        rSepArg = OUString(";");

    rSepArrayCol = OUString(",");
    if (cDecSep == sal_Unicode(','))
        rSepArrayCol = OUString(".");
    rSepArrayRow = OUString(";");
}

const LocaleDataWrapper& ScFormulaOptions::GetLocaleDataWrapper()
{
    return *ScGlobal::pLocaleData;
}

ScFormulaOptions& ScFormulaOptions::operator=( const ScFormulaOptions& rCpy )
{
    bUseEnglishFuncName = rCpy.bUseEnglishFuncName;
    eFormulaGrammar     = rCpy.eFormulaGrammar;
    aCalcConfig = rCpy.aCalcConfig;
    aFormulaSepArg      = rCpy.aFormulaSepArg;
    aFormulaSepArrayRow = rCpy.aFormulaSepArrayRow;
    aFormulaSepArrayCol = rCpy.aFormulaSepArrayCol;
    meOOXMLRecalc       = rCpy.meOOXMLRecalc;
    meODFRecalc         = rCpy.meODFRecalc;
    return *this;
}

bool ScFormulaOptions::operator==( const ScFormulaOptions& rOpt ) const
{
    return bUseEnglishFuncName == rOpt.bUseEnglishFuncName
        && eFormulaGrammar     == rOpt.eFormulaGrammar
        && aCalcConfig == rOpt.aCalcConfig
        && aFormulaSepArg      == rOpt.aFormulaSepArg
        && aFormulaSepArrayRow == rOpt.aFormulaSepArrayRow
        && aFormulaSepArrayCol == rOpt.aFormulaSepArrayCol
        && meOOXMLRecalc       == rOpt.meOOXMLRecalc
        && meODFRecalc         == rOpt.meODFRecalc;
}

bool ScFormulaOptions::operator!=( const ScFormulaOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

ScTpFormulaItem::ScTpFormulaItem( sal_uInt16 nWhichP, const ScFormulaOptions& rOpt ) :
    SfxPoolItem ( nWhichP ),
    theOptions  ( rOpt )
{
}

ScTpFormulaItem::ScTpFormulaItem( const ScTpFormulaItem& rItem ) :
    SfxPoolItem ( rItem ),
    theOptions  ( rItem.theOptions )
{
}

ScTpFormulaItem::~ScTpFormulaItem()
{
}

OUString ScTpFormulaItem::GetValueText() const
{
    return OUString("ScTpFormulaItem");
}

int ScTpFormulaItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpFormulaItem& rPItem = (const ScTpFormulaItem&)rItem;
    return ( theOptions == rPItem.theOptions );
}

SfxPoolItem* ScTpFormulaItem::Clone( SfxItemPool * ) const
{
    return new ScTpFormulaItem( *this );
}

#define CFGPATH_FORMULA           "Office.Calc/Formula"

#define SCFORMULAOPT_GRAMMAR              0
#define SCFORMULAOPT_ENGLISH_FUNCNAME     1
#define SCFORMULAOPT_SEP_ARG              2
#define SCFORMULAOPT_SEP_ARRAY_ROW        3
#define SCFORMULAOPT_SEP_ARRAY_COL        4
#define SCFORMULAOPT_STRING_REF_SYNTAX    5
#define SCFORMULAOPT_EMPTY_STRING_AS_ZERO 6
#define SCFORMULAOPT_OOXML_RECALC         7
#define SCFORMULAOPT_ODF_RECALC           8
#define SCFORMULAOPT_OPENCL_ENABLED       9
#define SCFORMULAOPT_OPENCL_AUTOSELECT   10
#define SCFORMULAOPT_OPENCL_DEVICE       11
#define SCFORMULAOPT_COUNT               12

Sequence<OUString> ScFormulaCfg::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Syntax/Grammar",                // SCFORMULAOPT_GRAMMAR
        "Syntax/EnglishFunctionName",    // SCFORMULAOPT_ENGLISH_FUNCNAME
        "Syntax/SeparatorArg",           // SCFORMULAOPT_SEP_ARG
        "Syntax/SeparatorArrayRow",      // SCFORMULAOPT_SEP_ARRAY_ROW
        "Syntax/SeparatorArrayCol",      // SCFORMULAOPT_SEP_ARRAY_COL
        "Syntax/StringRefAddressSyntax", // SCFORMULAOPT_STRING_REF_SYNTAX
        "Syntax/EmptyStringAsZero",      // SCFORMULAOPT_EMPTY_STRING_AS_ZERO
        "Load/OOXMLRecalcMode",          // SCFORMULAOPT_OOXML_RECALC
        "Load/ODFRecalcMode",            // SCFORMULAOPT_ODF_RECALC
        "Calculation/OpenCL",            // SCFORMULAOPT_OPENCL_ENABLED
        "Calculation/OpenCLAutoSelect",  // SCFORMULAOPT_OPENCL_AUTOSELECT
        "Calculation/OpenCLDevice"   // SCFORMULAOPT_OPENCL_DEVICE
    };
    Sequence<OUString> aNames(SCFORMULAOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for (int i = 0; i < SCFORMULAOPT_COUNT; ++i)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScFormulaCfg::PropsToIds ScFormulaCfg::GetPropNamesToId()
{
    Sequence<OUString> aPropNames = GetPropertyNames();
    static sal_uInt16 aVals[] = { SCFORMULAOPT_GRAMMAR, SCFORMULAOPT_ENGLISH_FUNCNAME, SCFORMULAOPT_SEP_ARG, SCFORMULAOPT_SEP_ARRAY_ROW, SCFORMULAOPT_SEP_ARRAY_COL, SCFORMULAOPT_STRING_REF_SYNTAX, SCFORMULAOPT_EMPTY_STRING_AS_ZERO, SCFORMULAOPT_OOXML_RECALC, SCFORMULAOPT_ODF_RECALC, SCFORMULAOPT_OPENCL_ENABLED, SCFORMULAOPT_OPENCL_AUTOSELECT, SCFORMULAOPT_OPENCL_DEVICE };
    OSL_ENSURE( SAL_N_ELEMENTS(aVals) == aPropNames.getLength(), "Properties and ids are out of Sync");
    PropsToIds aPropIdMap;
    for ( sal_uInt16 i=0; i<aPropNames.getLength(); ++i )
        aPropIdMap[aPropNames[i]] = aVals[ i ];
    return aPropIdMap;
}

ScFormulaCfg::ScFormulaCfg() :
    ConfigItem( OUString( CFGPATH_FORMULA ) )
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
    if(aValues.getLength() == aNames.getLength())
    {
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
                            // extractino failed.
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
                            default:
                                ;
                        }
                    }
                    while (false);
                    GetCalcConfig().meStringRefAddressSyntax = eConv;
                }
                break;
                case SCFORMULAOPT_EMPTY_STRING_AS_ZERO:
                {
                    sal_Bool bVal = GetCalcConfig().mbEmptyStringAsZero;
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
                case SCFORMULAOPT_OPENCL_ENABLED:
                {
                    sal_Bool bVal = GetCalcConfig().mbOpenCLEnabled;
                    pValues[nProp] >>= bVal;
                    GetCalcConfig().mbOpenCLEnabled = bVal;
                }
                break;
                case SCFORMULAOPT_OPENCL_AUTOSELECT:
                {
                    sal_Bool bVal = GetCalcConfig().mbOpenCLAutoSelect;
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
                default:
                    ;
                }
            }
        }
    }
}

void ScFormulaCfg::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
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
            case SCFORMULAOPT_STRING_REF_SYNTAX:
            {
                sal_Int32 nVal = -1;
                switch (GetCalcConfig().meStringRefAddressSyntax)
                {
                    case ::formula::FormulaGrammar::CONV_OOO:     nVal = 0; break;
                    case ::formula::FormulaGrammar::CONV_XL_A1:   nVal = 1; break;
                    case ::formula::FormulaGrammar::CONV_XL_R1C1: nVal = 2; break;
                    default: break;
                }
                pValues[nProp] <<= nVal;
            }
            break;
            case SCFORMULAOPT_EMPTY_STRING_AS_ZERO:
            {
                sal_Bool bVal = GetCalcConfig().mbEmptyStringAsZero;
                pValues[nProp] <<= bVal;
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
            case SCFORMULAOPT_OPENCL_ENABLED:
            {
                sal_Bool bVal = GetCalcConfig().mbOpenCLEnabled;
                pValues[nProp] <<= bVal;
            }
            break;
            case SCFORMULAOPT_OPENCL_AUTOSELECT:
            {
                sal_Bool bVal = GetCalcConfig().mbOpenCLAutoSelect;
                pValues[nProp] <<= bVal;
                sc::FormulaGroupInterpreter::switchOpenCLDevice(
                        GetCalcConfig().maOpenCLDevice, bVal);
            }
            break;
            case SCFORMULAOPT_OPENCL_DEVICE:
            {
                OUString aOpenCLDevice = GetCalcConfig().maOpenCLDevice;
                pValues[nProp] <<= aOpenCLDevice;
                sc::FormulaGroupInterpreter::switchOpenCLDevice(
                        aOpenCLDevice, GetCalcConfig().mbOpenCLAutoSelect);
            }
            break;
            default:
                ;
        }
    }
    PutProperties(aNames, aValues);
}

void ScFormulaCfg::SetOptions( const ScFormulaOptions& rNew )
{
    *(ScFormulaOptions*)this = rNew;
    SetModified();
}

void ScFormulaCfg::Notify( const ::com::sun::star::uno::Sequence< OUString >& rNames)
{
    UpdateFromProperties( rNames );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
