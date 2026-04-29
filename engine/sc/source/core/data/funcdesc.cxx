/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <funcdesc.hxx>

#include <addincol.hxx>
#include <appoptio.hxx>
#include <callform.hxx>
#include <compiler.hxx>
#include <compiler.hrc>
#include <global.hxx>
#include <scfuncs.hrc>
#include <scmod.hxx>
#include <scresid.hxx>
#include <helpids.h>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/collatorwrapper.hxx>
#include <formula/funcvarargs.h>
#include <formula/opcode.hxx>
#include <osl/diagnose.h>

#include <memory>

namespace {

struct ScFuncDescCore
{
    /*
     * An opcode from include/formula/opcode.hxx
     */
    OpCode nOpCode;
    /*
     * Pointer to list of strings
     */
    const TranslateId* pResource;
    /*
     * Count of list of strings
     */
    size_t nResourceLen;
    /*
     * 16-bit value:
     *
     * Bit 1: boolean flag whether function is suppressed. Usually 0. This
     * may be used to add UI string resources before UI freeze if
     * implementation isn't ready yet without displaying them in the
     * function wizard, most recent used list and other UI elements. Also
     * not available via API then.
     *
     * Bit 2: boolean flag whether function is hidden in the Function
     * Wizard unless used in an expression.
     */
    sal_uInt16 nFunctionFlags;
    /*
     * Function group (text, math, ...), one of ID_FUNCTION_GRP_...
     */
    sal_uInt16 nCategory;
    /*
     * Help ID, HID_FUNC_...
     */
    OUString pHelpId;
    /*
     * Number of parameters. VAR_ARGS if variable number, or
     * VAR_ARGS+number if number of fixed parameters and variable
     * arguments following. Or PAIRED_VAR_ARGS if variable number of
     * paired parameters, or PAIRED_VAR_ARGS+number if number of fixed
     * parameters and variable paired arguments following.
     */
    sal_uInt16 nArgs;
    /*
     * For every parameter:
     *     Boolean flag whether the parameter is optional.
     */
    sal_uInt8 aOptionalArgs[7];
    /*
     * Limited number of maximum (variable) parameters, or 0 if no specific
     * limit other than the general VAR_ARGS-1 value.
     */
    sal_uInt16 nVarArgsLimit;
};

}

static void ScFuncRes(const ScFuncDescCore &rEntry, ScFuncDesc*, bool& rbSuppressed);

ScFuncDesc::ScFuncDesc() :
        pDefArgFlags    (nullptr),
        nFIndex         (0),
        nCategory       (0),
        nArgCount       (0),
        nVarArgsStart   (0),
        nVarArgsLimit   (0),
        bIncomplete     (false),
        mbHidden        (false)
{}

ScFuncDesc::~ScFuncDesc()
{
    Clear();
}

void ScFuncDesc::Clear()
{
    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= PAIRED_VAR_ARGS)
        nArgs -= PAIRED_VAR_ARGS - 2;
    else if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    if (nArgs)
    {
        delete [] pDefArgFlags;
    }
    nArgCount = 0;
    nVarArgsStart = 0;
    nVarArgsLimit = 0;
    maDefArgNames.clear();
    maDefArgDescs.clear();
    pDefArgFlags = nullptr;

    mxFuncName.reset();
    mxFuncDesc.reset();

    nFIndex = 0;
    nCategory = 0;
    sHelpId.clear();
    bIncomplete = false;
    mbHidden = false;
}

OUString ScFuncDesc::GetParamList() const
{
    OUString sep(ScCompiler::GetNativeSymbol(ocSep));

    OUStringBuffer aSig;

    if ( nArgCount > 0 )
    {
        if ( nArgCount < VAR_ARGS )
        {
            sal_uInt16 nLastSuppressed = nArgCount;
            sal_uInt16 nLastAdded = nArgCount;
            for ( sal_uInt16 i=0; i<nArgCount; i++ )
            {
                nLastAdded = i;
                aSig.append(maDefArgNames[i]);
                if ( i != nArgCount-1 )
                {
                    aSig.append(sep + " " );
                }
            }
            // If only suppressed parameters follow the last added parameter,
            // remove one "; "
            if (nLastSuppressed < nArgCount && nLastAdded < nLastSuppressed &&
                    aSig.getLength() >= 2)
                aSig.setLength(aSig.getLength() - 2);
        }
        else if ( nArgCount < PAIRED_VAR_ARGS)
        {
            for ( sal_uInt16 nArg = 0; nArg < nVarArgsStart; nArg++ )
            {
                aSig.append(maDefArgNames[nArg] + sep + " ");
            }
            /* NOTE: Currently there are no suppressed var args parameters. If
             * there were, we'd have to cope with it here and above for the fix
             * parameters. For now parameters are always added, so no special
             * treatment of a trailing "; " necessary. */
            aSig.append(maDefArgNames[nVarArgsStart]
                + "1"
                + sep + " "
                + maDefArgNames[nVarArgsStart]
                + "2"
                + sep + " ... ");
        }
        else
        {
            for ( sal_uInt16 nArg = 0; nArg < nVarArgsStart; nArg++ )
            {
                aSig.append(maDefArgNames[nArg] + sep + " ");
            }

            aSig.append(maDefArgNames[nVarArgsStart]
                + "1" + sep
                + maDefArgNames[nVarArgsStart+1]
                + (mxFuncName != "LET" ? "1" : "2") + sep
                + " "
                + maDefArgNames[nVarArgsStart]
                + "2" + sep
                + maDefArgNames[nVarArgsStart+1]
                + (mxFuncName != "LET" ? "2" : "3") + sep + " ... " );
        }
    }

    return aSig.makeStringAndClear();
}

OUString ScFuncDesc::getSignature() const
{
    OUStringBuffer aSig;

    if(mxFuncName)
    {
        aSig.append(*mxFuncName);

        OUString aParamList = GetParamList();
        if( !aParamList.isEmpty() )
        {
            aSig.append( "( " + aParamList
            // U+00A0 (NBSP) prevents automatic line break
                + u"\x00A0" ")" );
        }
        else
            aSig.append( "()" );
    }
    return aSig.makeStringAndClear();
}

OUString ScFuncDesc::getFormula( const ::std::vector< OUString >& _aArguments ) const
{
    OUString sep = ScCompiler::GetNativeSymbol(ocSep);

    OUStringBuffer aFormula;

    if(mxFuncName)
    {
        aFormula.append( *mxFuncName + "(" );
        if ( nArgCount > 0 && !_aArguments.empty() && !_aArguments[0].isEmpty())
        {
            ::std::vector< OUString >::const_iterator aIter = _aArguments.begin();
            ::std::vector< OUString >::const_iterator aEnd = _aArguments.end();

            aFormula.append( *aIter );
            ++aIter;
            while( aIter != aEnd && !aIter->isEmpty() )
            {
                aFormula.append( sep + *aIter );
                ++aIter;
            }
        }

        aFormula.append( ")" );
    }
    return aFormula.makeStringAndClear();
}

sal_uInt16 ScFuncDesc::GetSuppressedArgCount() const
{
    return nArgCount;
}

OUString ScFuncDesc::getFunctionName() const
{
    OUString sRet;
    if ( mxFuncName )
        sRet = *mxFuncName;
    return sRet;
}

const formula::IFunctionCategory* ScFuncDesc::getCategory() const
{
    return ScGlobal::GetStarCalcFunctionMgr()->getCategory(nCategory - 1);
}

OUString ScFuncDesc::getDescription() const
{
    OUString sRet;
    if ( mxFuncDesc )
        sRet = *mxFuncDesc;
    return sRet;
}

sal_Int32 ScFuncDesc::getSuppressedArgumentCount() const
{
    return GetSuppressedArgCount();
}

void ScFuncDesc::fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const
{
    _rArguments.resize( nArgCount);
    sal_uInt16 value = 0;
    for (auto & argument : _rArguments)
        argument = value++;

    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= PAIRED_VAR_ARGS)
        nArgs -= PAIRED_VAR_ARGS - 2;
    else if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        _rArguments.push_back(i);
    }
}

void ScFuncDesc::initArgumentInfo()  const
{
    // get the full argument description
    // (add-in has to be instantiated to get the type information)

    if ( !(bIncomplete && mxFuncName) )
        return;

    ScUnoAddInCollection& rAddIns = *ScGlobal::GetAddInCollection();
    OUString aIntName(rAddIns.FindFunction( *mxFuncName, true ));         // pFuncName is upper-case

    if ( !aIntName.isEmpty() )
    {
        // GetFuncData with bComplete=true loads the component and updates
        // the global function list if needed.

        rAddIns.GetFuncData( aIntName, true );
    }

    if ( bIncomplete )
    {
        OSL_FAIL( "couldn't initialize add-in function" );
        const_cast<ScFuncDesc*>(this)->bIncomplete = false;         // even if there was an error, don't try again
    }
}

OUString ScFuncDesc::getHelpId() const
{
    return sHelpId;
}

bool ScFuncDesc::isHidden() const
{
    return mbHidden;
}

sal_uInt32 ScFuncDesc::getParameterCount() const
{
    return nArgCount;
}

sal_uInt32 ScFuncDesc::getVarArgsStart() const
{
    return nVarArgsStart;
}

sal_uInt32 ScFuncDesc::getVarArgsLimit() const
{
    return nVarArgsLimit;
}

OUString ScFuncDesc::getParameterName(sal_uInt32 _nPos) const
{
    return maDefArgNames[_nPos];
}

OUString ScFuncDesc::getParameterDescription(sal_uInt32 _nPos) const
{
    return maDefArgDescs[_nPos];
}

bool ScFuncDesc::isParameterOptional(sal_uInt32 _nPos) const
{
    return pDefArgFlags[_nPos].bOptional;
}

bool ScFuncDesc::compareByName(const ScFuncDesc* a, const ScFuncDesc* b)
{
    return (ScGlobal::GetCaseCollator().compareString(*a->mxFuncName, *b->mxFuncName ) < 0);
}

#define ENTRY(CODE) CODE, SAL_N_ELEMENTS(CODE)

ScFunctionList::ScFunctionList( bool bEnglishFunctionNames )
    : mbEnglishFunctionNames( bEnglishFunctionNames )
{
    sal_Int32  nMaxFuncNameLen = 0; // Length of longest function name

    // See ScFuncDescCore definition for format details.
    // This list must be sorted in order of the opcode, dbgutil builds enable _GLIBCXX_DEBUG
    // which will concept check that the list is sorted on first use to ensure this holds
    static const ScFuncDescCore aDescs[] =
    {
        { ocIf, ENTRY(ocIF_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_WENN, 3, { 0, 1, 1 }, 0 },
        { ocIfError, ENTRY(ocIF_ERROR_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_IFERROR, 2, { 0, 0 }, 0 },
        { ocIfNA, ENTRY(ocIF_NA_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_IFNA, 2, { 0, 0 }, 0 },
        { ocChoose, ENTRY(ocCHOOSE_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_WAHL, VAR_ARGS+1, { 0, 0 }, 31 },
        { ocLet, ENTRY(ocLET_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_LET_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 }, 0 },
        { ocAnd, ENTRY(ocAND_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_UND, VAR_ARGS, { 0 }, 0 },
        { ocOr, ENTRY(ocOR_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_ODER, VAR_ARGS, { 0 }, 0 },
        { ocPi, ENTRY(ocPI_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_PI, 0, { }, 0 },
        { ocRandom, ENTRY(ocRANDOM_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ZUFALLSZAHL, 0, { }, 0 },
        { ocTrue, ENTRY(ocTRUE_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_WAHR, 0, { }, 0 },
        { ocFalse, ENTRY(ocFALSE_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_FALSCH, 0, { }, 0 },
        { ocGetActDate, ENTRY(ocGET_ACT_DATE_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_HEUTE, 0, { }, 0 },
        { ocGetActTime, ENTRY(ocGET_ACT_TIME_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_JETZT, 0, { }, 0 },
        { ocNotAvail, ENTRY(ocNO_VALUE_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_NV, 0, { }, 0 },
        { ocCurrent, ENTRY(ocCURRENT_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_AKTUELL, 0, { }, 0 },
        { ocRandomNV, ENTRY(ocRANDOM_NV_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RAND_NV, 0, { }, 0 },
        { ocDeg, ENTRY(ocDEG_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_DEG, 1, { 0 }, 0 },
        { ocRad, ENTRY(ocRAD_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RAD, 1, { 0 }, 0 },
        { ocSin, ENTRY(ocSIN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SIN, 1, { 0 }, 0 },
        { ocCos, ENTRY(ocCOS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COS, 1, { 0 }, 0 },
        { ocTan, ENTRY(ocTAN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_TAN, 1, { 0 }, 0 },
        { ocCot, ENTRY(ocCOT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COT, 1, { 0 }, 0 },
        { ocArcSin, ENTRY(ocARC_SIN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCSIN, 1, { 0 }, 0 },
        { ocArcCos, ENTRY(ocARC_COS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCCOS, 1, { 0 }, 0 },
        { ocArcTan, ENTRY(ocARC_TAN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCTAN, 1, { 0 }, 0 },
        { ocArcCot, ENTRY(ocARC_COT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCCOT, 1, { 0 }, 0 },
        { ocSinHyp, ENTRY(ocSIN_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SINHYP, 1, { 0 }, 0 },
        { ocCosHyp, ENTRY(ocCOS_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COSHYP, 1, { 0 }, 0 },
        { ocTanHyp, ENTRY(ocTAN_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_TANHYP, 1, { 0 }, 0 },
        { ocCotHyp, ENTRY(ocCOT_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COTHYP, 1, { 0 }, 0 },
        { ocArcSinHyp, ENTRY(ocARC_SIN_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARSINHYP, 1, { 0 }, 0 },
        { ocArcCosHyp, ENTRY(ocARC_COS_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCOSHYP, 1, { 0 }, 0 },
        { ocArcTanHyp, ENTRY(ocARC_TAN_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARTANHYP, 1, { 0 }, 0 },
        { ocArcCotHyp, ENTRY(ocARC_COT_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCOTHYP, 1, { 0 }, 0 },
        { ocCosecant, ENTRY(ocCOSECANT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COSECANT, 1, { 0 }, 0 },
        { ocSecant, ENTRY(ocSECANT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SECANT, 1, { 0 }, 0 },
        { ocCosecantHyp, ENTRY(ocCOSECANT_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COSECANTHYP, 1, { 0 }, 0 },
        { ocSecantHyp, ENTRY(ocSECANT_HYP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SECANTHYP, 1, { 0 }, 0 },
        { ocExp, ENTRY(ocEXP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_EXP, 1, { 0 }, 0 },
        { ocLn, ENTRY(ocLN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_LN, 1, { 0 }, 0 },
        { ocSqrt, ENTRY(ocSQRT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_WURZEL, 1, { 0 }, 0 },
        { ocFact, ENTRY(ocFACT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FAKULTAET, 1, { 0 }, 0 },
        { ocGetYear, ENTRY(ocGET_YEAR_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_JAHR, 1, { 0 }, 0 },
        { ocGetMonth, ENTRY(ocGET_MONTH_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_MONAT, 1, { 0 }, 0 },
        { ocGetDay, ENTRY(ocGET_DAY_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_TAG, 1, { 0 }, 0 },
        { ocGetHour, ENTRY(ocGET_HOUR_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_STUNDE, 1, { 0 }, 0 },
        { ocGetMin, ENTRY(ocGET_MIN_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_MINUTE, 1, { 0 }, 0 },
        { ocGetSec, ENTRY(ocGET_SEC_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_SEKUNDE, 1, { 0 }, 0 },
        { ocPlusMinus, ENTRY(ocPLUS_MINUS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_VORZEICHEN, 1, { 0 }, 0 },
        { ocAbs, ENTRY(ocABS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ABS, 1, { 0 }, 0 },
        { ocInt, ENTRY(ocINT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_GANZZAHL, 1, { 0 }, 0 },
        { ocPhi, ENTRY(ocPHI_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PHI, 1, { 0 }, 0 },
        { ocGauss, ENTRY(ocGAUSS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAUSS, 1, { 0 }, 0 },
        { ocIsEmpty, ENTRY(ocIS_EMPTY_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTLEER, 1, { 0 }, 0 },
        { ocIsString, ENTRY(ocIS_STRING_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTTEXT, 1, { 0 }, 0 },
        { ocIsNonString, ENTRY(ocIS_NON_STRING_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTKTEXT, 1, { 0 }, 0 },
        { ocIsLogical, ENTRY(ocIS_LOGICAL_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTLOG, 1, { 0 }, 0 },
        { ocType, ENTRY(ocTYPE_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_TYP, 1, { 0 }, 0 },
        { ocIsRef, ENTRY(ocIS_REF_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTBEZUG, 1, { 0 }, 0 },
        { ocIsValue, ENTRY(ocIS_VALUE_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTZAHL, 1, { 0 }, 0 },
        { ocIsFormula, ENTRY(ocIS_FORMULA_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTFORMEL, 1, { 0 }, 0 },
        { ocIsNA, ENTRY(ocIS_NV_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTNV, 1, { 0 }, 0 },
        { ocIsErr, ENTRY(ocIS_ERR_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTFEHL, 1, { 0 }, 0 },
        { ocIsError, ENTRY(ocIS_ERROR_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTFEHLER, 1, { 0 }, 0 },
        { ocIsEven, ENTRY(ocIS_EVEN_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTGERADE, 1, { 0 }, 0 },
        { ocIsOdd, ENTRY(ocIS_ODD_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTUNGERADE, 1, { 0 }, 0 },
        { ocN, ENTRY(ocN_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_N, 1, { 0 }, 0 },
        { ocGetDateValue, ENTRY(ocGET_DATE_VALUE_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_DATWERT, 1, { 0 }, 0 },
        { ocGetTimeValue, ENTRY(ocGET_TIME_VALUE_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_ZEITWERT, 1, { 0 }, 0 },
        { ocCode, ENTRY(ocCODE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_CODE, 1, { 0 }, 0 },
        { ocTrim, ENTRY(ocTRIM_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_GLAETTEN, 1, { 0 }, 0 },
        { ocUpper, ENTRY(ocUPPER_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_GROSS, 1, { 0 }, 0 },
        { ocProper, ENTRY(ocPROPER_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_GROSS2, 1, { 0 }, 0 },
        { ocLower, ENTRY(ocLOWER_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_KLEIN, 1, { 0 }, 0 },
        { ocLen, ENTRY(ocLEN_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LAENGE, 1, { 0 }, 0 },
        { ocT, ENTRY(ocT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_T, 1, { 0 }, 0 },
        { ocValue, ENTRY(ocVALUE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_VALUE, 1, { 0 }, 0 },
        { ocClean, ENTRY(ocCLEAN_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_SAEUBERN, 1, { 0 }, 0 },
        { ocChar, ENTRY(ocCHAR_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ZEICHEN, 1, { 0 }, 0 },
        { ocLog10, ENTRY(ocLOG10_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_LOG10, 1, { 0 }, 0 },
        { ocEven, ENTRY(ocEVEN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_GERADE, 1, { 0 }, 0 },
        { ocOdd, ENTRY(ocODD_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_UNGERADE, 1, { 0 }, 0 },
        { ocStdNormDist, ENTRY(ocSTD_NORM_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STANDNORMVERT, 1, { 0 }, 0 },
        { ocFisher, ENTRY(ocFISHER_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FISHER, 1, { 0 }, 0 },
        { ocFisherInv, ENTRY(ocFISHER_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FISHERINV, 1, { 0 }, 0 },
        { ocSNormInv, ENTRY(ocS_NORM_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STANDNORMINV, 1, { 0 }, 0 },
        { ocGammaLn, ENTRY(ocGAMMA_LN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMALN, 1, { 0 }, 0 },
        { ocErrorType, ENTRY(ocERROR_TYPE_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_FEHLERTYP, 1, { 0 }, 0 },
        { ocFormula, ENTRY(ocFORMULA_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_FORMEL, 1, { 0 }, 0 },
        { ocArabic, ENTRY(ocARABIC_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ARABISCH, 1, { 0 }, 0 },
        { ocInfo, ENTRY(ocINFO_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_INFO, 1, { 0 }, 0 },
        { ocBahtText, ENTRY(ocBAHTTEXT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_BAHTTEXT, 1, { 0 }, 0 },
        { ocJis, ENTRY(ocJIS_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_JIS, 1, { 0 }, 0 },
        { ocAsc, ENTRY(ocASC_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ASC, 1, { 0 }, 0 },
        { ocUnicode, ENTRY(ocUNICODE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_UNICODE, 1, { 0 }, 0 },
        { ocUnichar, ENTRY(ocUNICHAR_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_UNICHAR, 1, { 0 }, 0 },
        { ocGamma, ENTRY(ocGAMMA_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMA, 1, { 0 }, 0 },
        { ocGammaLn_MS, ENTRY(ocGAMMA_LN_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMALN_MS, 1, { 0 }, 0 },
        { ocErf_MS, ENTRY(ocERF_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ERF_MS, 1, { 0 }, 0 },
        { ocErfc_MS, ENTRY(ocERFC_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ERFC_MS, 1, { 0 }, 0 },
        { ocErrorType_ODF, ENTRY(ocERROR_TYPE_ODF_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ERROR_TYPE_ODF, 1, { 0 }, 0 },
        { ocEncodeURL, ENTRY(ocENCODEURL_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ENCODEURL, 1, { 0 }, 0 },
        { ocIsoWeeknum, ENTRY(ocISOWEEKNUM_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_ISOWEEKNUM, 1, { 0 }, 0 },
        { ocNot, ENTRY(ocNOT_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_NICHT, 1, { 0 }, 0 },
        { ocArcTan2, ENTRY(ocARC_TAN_2_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCTAN2, 2, { 0, 0 }, 0 },
        { ocCeil, ENTRY(ocCEIL_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_OBERGRENZE, 3, { 0, 1, 1 }, 0 },
        { ocFloor, ENTRY(ocFLOOR_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_UNTERGRENZE, 3, { 0, 1, 1 }, 0 },
        { ocRound, ENTRY(ocROUND_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RUNDEN, 2, { 0, 1 }, 0 },
        { ocRoundUp, ENTRY(ocROUND_UP_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_AUFRUNDEN, 2, { 0, 1 }, 0 },
        { ocRoundDown, ENTRY(ocROUND_DOWN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ABRUNDEN, 2, { 0, 1 }, 0 },
        { ocTrunc, ENTRY(ocTRUNC_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KUERZEN, 2, { 0, 0 }, 0 },
        { ocLog, ENTRY(ocLOG_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_LOG, 2, { 0, 1 }, 0 },
        { ocPower, ENTRY(ocPOWER_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_POTENZ, 2, { 0, 0 }, 0 },
        { ocGCD, ENTRY(ocGCD_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_GGT, VAR_ARGS, { 0 }, 0 },
        { ocLCM, ENTRY(ocLCM_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KGV, VAR_ARGS, { 0 }, 0 },
        { ocMod, ENTRY(ocMOD_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_REST, 2, { 0, 0 }, 0 },
        { ocSumProduct, ENTRY(ocSUM_PRODUCT_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMENPRODUKT, VAR_ARGS, { 0 }, 0 },
        { ocSumSQ, ENTRY(ocSUM_SQ_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_QUADRATESUMME, VAR_ARGS, { 0 }, 0 },
        { ocSumX2MY2, ENTRY(ocSUM_X2MY2_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMEX2MY2, 2, { 0, 0 }, 0 },
        { ocSumX2DY2, ENTRY(ocSUM_X2DY2_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMEX2PY2, 2, { 0, 0 }, 0 },
        { ocSumXMY2, ENTRY(ocSUM_XMY2_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMEXMY2, 2, { 0, 0 }, 0 },
        { ocGetDate, ENTRY(ocGET_DATE_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_DATUM, 3, { 0, 0, 0 }, 0 },
        { ocGetTime, ENTRY(ocGET_TIME_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_ZEIT, 3, { 0, 0, 0 }, 0 },
        { ocGetDiffDate, ENTRY(ocGET_DIFF_DATE_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_TAGE, 2, { 0, 0 }, 0 },
        { ocGetDiffDate360, ENTRY(ocGET_DIFF_DATE_360_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_TAGE360, 3, { 0, 0, 1 }, 0 },
        { ocMin, ENTRY(ocMIN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MIN, VAR_ARGS, { 0 }, 0 },
        { ocMax, ENTRY(ocMAX_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MAX, VAR_ARGS, { 0 }, 0 },
        { ocSum, ENTRY(ocSUM_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SUMME, VAR_ARGS, { 0 }, 0 },
        { ocProduct, ENTRY(ocPRODUCT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_PRODUKT, VAR_ARGS, { 0 }, 0 },
        { ocAverage, ENTRY(ocAVERAGE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MITTELWERT, VAR_ARGS, { 0 }, 0 },
        { ocCount, ENTRY(ocCOUNT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ANZAHL, VAR_ARGS, { 0 }, 0 },
        { ocCount2, ENTRY(ocCOUNT_2_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ANZAHL2, VAR_ARGS, { 0 }, 0 },
        { ocNPV, ENTRY(ocNPV_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_NBW, VAR_ARGS+1, { 0, 0 }, 0 },
        { ocIRR, ENTRY(ocIRR_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_IKV, 2, { 0, 1 }, 0 },
        { ocVar, ENTRY(ocVAR_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZ, VAR_ARGS, { 0 }, 0 },
        { ocVarP, ENTRY(ocVAR_P_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZEN, VAR_ARGS, { 0 }, 0 },
        { ocStDev, ENTRY(ocST_DEV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABW, VAR_ARGS, { 0 }, 0 },
        { ocStDevP, ENTRY(ocST_DEV_P_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABWN, VAR_ARGS, { 0 }, 0 },
        { ocB, ENTRY(ocB_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_B, 4, { 0, 0, 0, 1 }, 0 },
        { ocNormDist, ENTRY(ocNORM_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMVERT, 4, { 0, 0, 0, 1 }, 0 },
        { ocExpDist, ENTRY(ocEXP_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_EXPONVERT, 3, { 0, 0, 0 }, 0 },
        { ocBinomDist, ENTRY(ocBINOM_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BINOMVERT, 4, { 0, 0, 0, 0 }, 0 },
        { ocPoissonDist, ENTRY(ocPOISSON_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_POISSON, 3, { 0, 0, 1 }, 0 },
        { ocCombin, ENTRY(ocCOMBIN_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KOMBINATIONEN, 2, { 0, 0 }, 0 },
        { ocCombinA, ENTRY(ocCOMBIN_A_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KOMBINATIONEN2, 2, { 0, 0 }, 0 },
        { ocPermut, ENTRY(ocPERMUT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIATIONEN, 2, { 0, 0 }, 0 },
        { ocPermutationA, ENTRY(ocPERMUTATION_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIATIONEN2, 2, { 0, 0 }, 0 },
        { ocPV, ENTRY(ocPV_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_BW, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocSYD, ENTRY(ocSYD_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_DIA, 4, { 0, 0, 0, 0 }, 0 },
        { ocDDB, ENTRY(ocDDB_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_GDA, 5, { 0, 0, 0, 0, 1 }, 0 },
        { ocDB, ENTRY(ocDB_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_GDA2, 5, { 0, 0, 0, 0, 1 }, 0 },
        { ocVBD , ENTRY(ocVBD_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_VDB, 7, { 0, 0, 0, 0, 0, 1, 1 }, 0 },
        { ocPDuration, ENTRY(ocPDURATION_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_LAUFZEIT, 3, { 0, 0, 0 }, 0 },
        { ocSLN, ENTRY(ocSLN_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_LIA, 3, { 0, 0, 0 }, 0 },
        { ocPMT, ENTRY(ocPMT_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_RMZ, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocColumns, ENTRY(ocCOLUMNS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SPALTEN, 1, { 0 }, 0 },
        { ocRows, ENTRY(ocROWS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ZEILEN, 1, { 0 }, 0 },
        { ocColumn, ENTRY(ocCOLUMN_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SPALTE, 1, { 1 }, 0 },
        { ocRow, ENTRY(ocROW_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ZEILE, 1, { 1 }, 0 },
        { ocRRI, ENTRY(ocRRI_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_ZGZ, 3, { 0, 0, 0 }, 0 },
        { ocFV, ENTRY(ocFV_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_ZW, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocNper, ENTRY(ocNPER_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_ZZR, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocRate, ENTRY(ocRATE_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_ZINS, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocIpmt, ENTRY(ocIPMT_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_ZINSZ, 6, { 0, 0, 0, 0, 1, 1 }, 0 },
        { ocPpmt, ENTRY(ocPPMT_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_KAPZ, 6, { 0, 0, 0, 0, 1, 1 }, 0 },
        { ocCumIpmt, ENTRY(ocCUM_IPMT_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_KUMZINSZ, 6, { 0, 0, 0, 0, 0, 0 }, 0 },
        { ocCumPrinc, ENTRY(ocCUM_PRINC_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_KUMKAPITAL, 6, { 0, 0, 0, 0, 0, 0 }, 0 },
        { ocEffect, ENTRY(ocEFFECT_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_EFFEKTIV, 2, { 0, 0 }, 0 },
        { ocNominal, ENTRY(ocNOMINAL_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_NOMINAL, 2, { 0, 0 }, 0 },
        { ocSubTotal, ENTRY(ocSUB_TOTAL_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_TEILERGEBNIS, 2, { 0, 0 }, 0 },
        { ocDBSum, ENTRY(ocDB_SUM_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBSUMME, 3, { 0, 0, 0 }, 0 },
        { ocDBCount, ENTRY(ocDB_COUNT_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBANZAHL, 3, { 0, 1, 0 }, 0 },
        { ocDBCount2, ENTRY(ocDB_COUNT_2_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBANZAHL2, 3, { 0, 1, 0 }, 0 },
        { ocDBAverage, ENTRY(ocDB_AVERAGE_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBMITTELWERT, 3, { 0, 0, 0 }, 0 },
        { ocDBGet, ENTRY(ocDB_GET_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBAUSZUG, 3, { 0, 0, 0 }, 0 },
        { ocDBMax, ENTRY(ocDB_MAX_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBMAX, 3, { 0, 0, 0 }, 0 },
        { ocDBMin, ENTRY(ocDB_MIN_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBMIN, 3, { 0, 0, 0 }, 0 },
        { ocDBProduct, ENTRY(ocDB_PRODUCT_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBPRODUKT, 3, { 0, 0, 0 }, 0 },
        { ocDBStdDev, ENTRY(ocDB_STD_DEV_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBSTDABW, 3, { 0, 0, 0 }, 0 },
        { ocDBStdDevP, ENTRY(ocDB_STD_DEV_P_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBSTDABWN, 3, { 0, 0, 0 }, 0 },
        { ocDBVar, ENTRY(ocDB_VAR_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBVARIANZ, 3, { 0, 0, 0 }, 0 },
        { ocDBVarP, ENTRY(ocDB_VAR_P_ARY), 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBVARIANZEN, 3, { 0, 0, 0 }, 0 },
        { ocIndirect, ENTRY(ocINDIRECT_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_INDIREKT, 2, { 0, 1 }, 0 },
        { ocAddress, ENTRY(ocADDRESS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ADRESSE, 5, { 0, 0, 1, 1, 1 }, 0 },
        { ocMatch, ENTRY(ocMATCH_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VERGLEICH, 3, { 0, 0, 1 }, 0 },
        { ocCountEmptyCells, ENTRY(ocCOUNT_EMPTY_CELLS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ANZAHLLEEREZELLEN, 1, { 0 }, 0 },
        { ocCountIf, ENTRY(ocCOUNT_IF_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ZAEHLENWENN, 2, { 0, 0 }, 0 },
        { ocSumIf, ENTRY(ocSUM_IF_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SUMMEWENN, 3, { 0, 0, 1 }, 0 },
        { ocLookup, ENTRY(ocLOOKUP_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VERWEIS, 3, { 0, 0, 1 }, 0 },
        { ocVLookup, ENTRY(ocV_LOOKUP_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SVERWEIS, 4, { 0, 0, 0, 1 }, 0 },
        { ocHLookup, ENTRY(ocH_LOOKUP_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_WVERWEIS, 4, { 0, 0, 0, 1 }, 0 },
        { ocOffset, ENTRY(ocOFFSET_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VERSCHIEBUNG, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocIndex, ENTRY(ocINDEX_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_INDEX, 4, { 0, 1, 1, 1 }, 0 },
        { ocAreas, ENTRY(ocAREAS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_BEREICHE, 1, { 0 }, 0 },
        { ocCurrency, ENTRY(ocCURRENCY_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_DM, 2, { 0, 1 }, 0 },
        { ocReplace, ENTRY(ocREPLACE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ERSETZEN, 4, { 0, 0, 0, 0 }, 0 },
        { ocFixed, ENTRY(ocFIXED_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FEST, 3, { 0, 1, 1 }, 0 },
        { ocFind, ENTRY(ocFIND_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FINDEN, 3, { 0, 0, 1 }, 0 },
        { ocExact, ENTRY(ocEXACT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_IDENTISCH, 2, { 0, 0 }, 0 },
        { ocLeft, ENTRY(ocLEFT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LINKS, 2, { 0, 1 }, 0 },
        { ocRight, ENTRY(ocRIGHT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_RECHTS, 2, { 0, 1 }, 0 },
        { ocSearch, ENTRY(ocSEARCH_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_SUCHEN, 3, { 0, 0, 1 }, 0 },
        { ocMid, ENTRY(ocMID_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEIL, 3, { 0, 0, 0 }, 0 },
        { ocText, ENTRY(ocTEXT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEXT, 2, { 0, 0 }, 0 },
        { ocSubstitute, ENTRY(ocSUBSTITUTE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WECHSELN, 4, { 0, 0, 0, 1 }, 0 },
        { ocRept, ENTRY(ocREPT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WIEDERHOLEN, 2, { 0, 0 }, 0 },
        { ocConcat, ENTRY(ocCONCAT_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_VERKETTEN, VAR_ARGS, { 0 }, 0 },
        { ocMatDet, ENTRY(ocMAT_DET_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MDET, 1, { 0 }, 0 },
        { ocMatInv, ENTRY(ocMAT_INV_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MINV, 1, { 0 }, 0 },
        { ocMatMult, ENTRY(ocMAT_MULT_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MMULT, 2, { 0, 0 }, 0 },
        { ocMatTrans, ENTRY(ocMAT_TRANS_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MTRANS, 1, { 0 }, 0 },
        { ocMatrixUnit, ENTRY(ocMATRIX_UNIT_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_EINHEITSMATRIX, 1, { 0 }, 0 },
        { ocHypGeomDist, ENTRY(ocHYP_GEOM_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_HYPGEOMVERT, 5, { 0, 0, 0, 0, 1 }, 0 },
        { ocLogNormDist, ENTRY(ocLOG_NORM_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGNORMVERT, 4, { 0, 1, 1, 1 }, 0 },
        { ocTDist, ENTRY(ocT_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TVERT, 3, { 0, 0, 0 }, 0 },
        { ocFDist, ENTRY(ocF_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FVERT, 3, { 0, 0, 0 }, 0 },
        { ocChiDist, ENTRY(ocCHI_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIVERT, 2, { 0, 0 }, 0 },
        { ocWeibull, ENTRY(ocWEIBULL_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_WEIBULL, 4, { 0, 0, 0, 0 }, 0 },
        { ocNegBinomVert, ENTRY(ocNEG_BINOM_VERT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NEGBINOMVERT, 3, { 0, 0, 0 }, 0 },
        { ocCritBinom, ENTRY(ocCRIT_BINOM_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KRITBINOM, 3, { 0, 0, 0 }, 0 },
        { ocKurt, ENTRY(ocKURT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KURT, VAR_ARGS, { 0 }, 0 },
        { ocHarMean, ENTRY(ocHAR_MEAN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_HARMITTEL, VAR_ARGS, { 0 }, 0 },
        { ocGeoMean, ENTRY(ocGEO_MEAN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GEOMITTEL, VAR_ARGS, { 0 }, 0 },
        { ocStandard, ENTRY(ocSTANDARD_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STANDARDISIERUNG, 3, { 0, 0, 0 }, 0 },
        { ocAveDev, ENTRY(ocAVE_DEV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MITTELABW, VAR_ARGS, { 0 }, 0 },
        { ocSkew, ENTRY(ocSKEW_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SCHIEFE, VAR_ARGS, { 0 }, 0 },
        { ocDevSq, ENTRY(ocDEV_SQ_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SUMQUADABW, VAR_ARGS, { 0 }, 0 },
        { ocMedian, ENTRY(ocMEDIAN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MEDIAN, VAR_ARGS, { 0 }, 0 },
        { ocModalValue, ENTRY(ocMODAL_VALUE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MODALWERT, VAR_ARGS, { 0 }, 0 },
        { ocZTest, ENTRY(ocZ_TEST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GTEST, 3, { 0, 0, 1 }, 0 },
        { ocTTest, ENTRY(ocT_TEST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TTEST, 4, { 0, 0, 0, 0 }, 0 },
        { ocRank, ENTRY(ocRANK_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_RANG, 3, { 0, 0, 1 }, 0 },
        { ocPercentile, ENTRY(ocPERCENTILE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUANTIL, 2, { 0, 0 }, 0 },
        { ocPercentrank, ENTRY(ocPERCENT_RANK_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUANTILSRANG, 3, { 0, 0, 1 }, 0 },
        { ocLarge, ENTRY(ocLARGE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KGROESSTE, 2, { 0, 0 }, 0 },
        { ocSmall, ENTRY(ocSMALL_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KKLEINSTE, 2, { 0, 0 }, 0 },
        { ocFrequency, ENTRY(ocFREQUENCY_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_HAEUFIGKEIT, 2, { 0, 0 }, 0 },
        { ocQuartile, ENTRY(ocQUARTILE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUARTILE, 2, { 0, 0 }, 0 },
        { ocNormInv, ENTRY(ocNORM_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMINV, 3, { 0, 0, 0 }, 0 },
        { ocConfidence, ENTRY(ocCONFIDENCE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KONFIDENZ, 3, { 0, 0, 0 }, 0 },
        { ocFTest, ENTRY(ocF_TEST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FTEST, 2, { 0, 0 }, 0 },
        { ocTrimMean, ENTRY(ocTRIM_MEAN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GESTUTZTMITTEL, 2, { 0, 0 }, 0 },
        { ocProb, ENTRY(ocPROB_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_WAHRSCHBEREICH, 4, { 0, 0, 0, 1 }, 0 },
        { ocCorrel, ENTRY(ocCORREL_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KORREL, 2, { 0, 0 }, 0 },
        { ocCovar, ENTRY(ocCOVAR_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KOVAR, 2, { 0, 0 }, 0 },
        { ocPearson, ENTRY(ocPEARSON_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PEARSON, 2, { 0, 0 }, 0 },
        { ocRSQ, ENTRY(ocRSQ_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BESTIMMTHEITSMASS, 2, { 0, 0 }, 0 },
        { ocSTEYX, ENTRY(ocSTEYX_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STFEHLERYX, 2, { 0, 0 }, 0 },
        { ocSlope, ENTRY(ocSLOPE_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STEIGUNG, 2, { 0, 0 }, 0 },
        { ocIntercept, ENTRY(ocINTERCEPT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ACHSENABSCHNITT, 2, { 0, 0 }, 0 },
        { ocTrend, ENTRY(ocTREND_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_TREND, 4, { 0, 1, 1, 1 }, 0 },
        { ocGrowth, ENTRY(ocGROWTH_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_VARIATION, 4, { 0, 1, 1, 1 }, 0 },
        { ocLinest, ENTRY(ocLINEST_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_RGP, 4, { 0, 1, 1, 1 }, 0 },
        { ocLogest, ENTRY(ocLOGEST_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_RKP, 4, { 0, 1, 1, 1 }, 0 },
        { ocForecast, ENTRY(ocFORECAST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SCHAETZER, 3, { 0, 0, 0 }, 0 },
        { ocChiInv, ENTRY(ocCHI_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIINV, 2, { 0, 0 }, 0 },
        { ocGammaDist, ENTRY(ocGAMMA_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMAVERT, 4, { 0, 0, 0, 1 }, 0 },
        { ocGammaInv, ENTRY(ocGAMMA_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMAINV, 3, { 0, 0, 0 }, 0 },
        { ocTInv, ENTRY(ocT_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TINV, 2, { 0, 0 }, 0 },
        { ocFInv, ENTRY(ocF_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FINV, 3, { 0, 0, 0 }, 0 },
        { ocChiTest, ENTRY(ocCHI_TEST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHITEST, 2, { 0, 0 }, 0 },
        { ocLogInv, ENTRY(ocLOG_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGINV, 3, { 0, 1, 1 }, 0 },
        { ocBetaDist, ENTRY(ocBETA_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETAVERT, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocBetaInv, ENTRY(ocBETA_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETAINV, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocWeek, ENTRY(ocWEEK_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_KALENDERWOCHE, 2, { 0, 1 }, 0 },
        { ocGetDayOfWeek, ENTRY(ocGET_DAY_OF_WEEK_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_WOCHENTAG, 2, { 0, 1 }, 0 },
        { ocStyle, ENTRY(ocSTYLE_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VORLAGE, 3, { 0, 1, 1 }, 0 },
        { ocDde, ENTRY(ocDDE_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_DDE, 4, { 0, 0, 0, 1 }, 0 },
        { ocBase, ENTRY(ocBASE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_BASIS, 3, { 0, 0, 1 }, 0 },
        { ocSheet, ENTRY(ocSHEET_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TABELLE, 1, { 1 }, 0 },
        { ocSheets, ENTRY(ocSHEETS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TABELLEN, 1, { 1 }, 0 },
        { ocMinA, ENTRY(ocMIN_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MINA, VAR_ARGS, { 0 }, 0 },
        { ocMaxA, ENTRY(ocMAX_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MAXA, VAR_ARGS, { 0 }, 0 },
        { ocAverageA, ENTRY(ocAVERAGE_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MITTELWERTA, VAR_ARGS, { 0 }, 0 },
        { ocStDevA, ENTRY(ocST_DEV_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABWA, VAR_ARGS, { 0 }, 0 },
        { ocStDevPA, ENTRY(ocST_DEV_P_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABWNA, VAR_ARGS, { 0 }, 0 },
        { ocVarA, ENTRY(ocVAR_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZA, VAR_ARGS, { 0 }, 0 },
        { ocVarPA, ENTRY(ocVAR_P_A_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZENA, VAR_ARGS, { 0 }, 0 },
        { ocEasterSunday, ENTRY(ocEASTERSUNDAY_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_OSTERSONNTAG, 1, { 0 }, 0 },
        { ocDecimal, ENTRY(ocDECIMAL_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_DEZIMAL, 2, { 0, 0 }, 0 },
        { ocConvertOOo, ENTRY(ocCONVERT_OOO_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_UMRECHNEN, 3, { 0, 0, 0 }, 0 },
        { ocRoman, ENTRY(ocROMAN_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ROEMISCH, 2, { 0, 1 }, 0 },
        { ocMIRR, ENTRY(ocMIRR_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_QIKV, 3, { 0, 0, 0 }, 0 },
        { ocCell, ENTRY(ocCELL_ARY), 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ZELLE, 2, { 0, 1 }, 0 },
        { ocISPMT, ENTRY(ocISPMT_ARY), 0, ID_FUNCTION_GRP_FINANCIAL, HID_FUNC_ISPMT, 4, { 0, 0, 0, 0 }, 0 },
        { ocHyperLink, ENTRY(ocHYPERLINK_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_HYPERLINK, 2, { 0, 1 }, 0 },
        { ocXLookup, ENTRY(ocX_LOOKUP_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_XLOOKUP_MS, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocXMatch, ENTRY(ocX_MATCH_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_XMATCH_MS, 4, { 0, 0, 1, 1 }, 0 },
        { ocGetPivotData, ENTRY(ocGET_PIVOT_DATA_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_GETPIVOTDATA, VAR_ARGS+2, { 0, 0, 1 }, 0 },
        { ocEuroConvert, ENTRY(ocEUROCONVERT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_EUROCONVERT, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocNumberValue, ENTRY(ocNUMBERVALUE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_NUMBERVALUE, 3, { 0, 1, 1 }, 0 },
        { ocChiSqDist, ENTRY(ocCHISQ_DIST_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQDIST, 3, { 0, 0, 1 }, 0 },
        { ocChiSqInv, ENTRY(ocCHISQ_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQINV, 2, { 0, 0 }, 0 },
        { ocBitAnd, ENTRY(ocBITAND_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITAND, 2, { 0, 0 }, 0 },
        { ocBitOr, ENTRY(ocBITOR_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITOR, 2, { 0, 0 }, 0 },
        { ocBitXor, ENTRY(ocBITXOR_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITXOR, 2, { 0, 0 }, 0 },
        { ocBitRshift, ENTRY(ocBITRSHIFT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITRSHIFT, 2, { 0, 0 }, 0 },
        { ocBitLshift, ENTRY(ocBITLSHIFT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITLSHIFT, 2, { 0, 0 }, 0 },
        { ocGetDateDif, ENTRY(ocGET_DATEDIF_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_DATEDIF, 3, { 0, 0, 0 }, 0 },
        { ocXor, ENTRY(ocXOR_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_XOR, VAR_ARGS, { 0 }, 0 },
        { ocAverageIf, ENTRY(ocAVERAGE_IF_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_AVERAGEIF, 3, { 0, 0, 1 }, 0 },
        { ocSumIfs, ENTRY(ocSUM_IFS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SUMIFS, PAIRED_VAR_ARGS+1, { 0, 0, 0 }, 0 },
        { ocAverageIfs, ENTRY(ocAVERAGE_IFS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_AVERAGEIFS, PAIRED_VAR_ARGS+1, { 0, 0, 0 }, 0 },
        { ocCountIfs, ENTRY(ocCOUNT_IFS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_COUNTIFS, PAIRED_VAR_ARGS, { 0, 0 }, 0 },
        { ocSkewp, ENTRY(ocSKEWP_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SKEWP, VAR_ARGS, { 0 }, 0 },
        { ocLenB, ENTRY(ocLENB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LENB, 1, { 0 }, 0 },
        { ocRightB, ENTRY(ocRIGHTB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_RIGHTB, 2, { 0, 1 }, 0 },
        { ocLeftB, ENTRY(ocLEFTB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LEFTB, 2, { 0, 1 }, 0 },
        { ocMidB, ENTRY(ocMIDB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_MIDB, 3, { 0, 0, 0 }, 0 },
        { ocFilterXML, ENTRY(ocFILTERXML_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FILTERXML, 2, { 0, 0 }, 0 },
        { ocWebservice, ENTRY(ocWEBSERVICE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WEBSERVICE, 1, { 0, 0 }, 0 },
        { ocCovarianceS, ENTRY(ocCOVARIANCE_S_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_COVARIANCE_S, 2, { 0, 0 }, 0 },
        { ocCovarianceP, ENTRY(ocCOVARIANCE_P_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_COVARIANCE_P, 2, { 0, 0 }, 0 },
        { ocStDevP_MS, ENTRY(ocST_DEV_P_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ST_DEV_P_MS, VAR_ARGS, { 0 }, 0 },
        { ocStDevS, ENTRY(ocST_DEV_S_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ST_DEV_S, VAR_ARGS, { 0 }, 0 },
        { ocVarP_MS, ENTRY(ocVAR_P_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VAR_P_MS, VAR_ARGS, { 0 }, 0 },
        { ocVarS, ENTRY(ocVAR_S_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VAR_S, VAR_ARGS, { 0 }, 0 },
        { ocBetaDist_MS, ENTRY(ocBETA_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETADIST_MS, 6, { 0, 0, 0, 0, 1, 1 }, 0 },
        { ocBetaInv_MS, ENTRY(ocBETA_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETAINV_MS, 5, { 0, 0, 0, 1, 1 }, 0 },
        { ocBinomDist_MS, ENTRY(ocBINOM_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BINOM_DIST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocBinomInv, ENTRY(ocBINOM_INV_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BINOM_INV_MS, 3, { 0, 0, 0 }, 0 },
        { ocChiDist_MS, ENTRY(ocCHI_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIVERT_MS, 2, { 0, 0 }, 0 },
        { ocChiInv_MS, ENTRY(ocCHI_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIINV_MS, 2, { 0, 0 }, 0 },
        { ocChiTest_MS, ENTRY(ocCHI_TEST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHITEST_MS, 2, { 0, 0 }, 0 },
        { ocChiSqDist_MS, ENTRY(ocCHISQ_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQDIST_MS, 3, { 0, 0, 0 }, 0 },
        { ocChiSqInv_MS, ENTRY(ocCHISQ_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQINV_MS, 2, { 0, 0 }, 0 },
        { ocConfidence_N, ENTRY(ocCONFIDENCE_N_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CONFIDENCE_N, 3, { 0, 0, 0 }, 0 },
        { ocConfidence_T, ENTRY(ocCONFIDENCE_T_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CONFIDENCE_T, 3, { 0, 0, 0 }, 0 },
        { ocFDist_LT, ENTRY(ocF_DIST_LT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_DIST_LT, 4, { 0, 0, 0, 1 }, 0 },
        { ocFDist_RT, ENTRY(ocF_DIST_RT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_DIST_RT, 3, { 0, 0, 0 }, 0 },
        { ocFInv_LT, ENTRY(ocF_INV_LT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_INV_LT, 3, { 0, 0, 0 }, 0 },
        { ocFInv_RT, ENTRY(ocF_INV_RT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_INV_RT, 3, { 0, 0, 0 }, 0 },
        { ocFTest_MS, ENTRY(ocF_TEST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_TEST_MS, 2, { 0, 0 }, 0 },
        { ocExpDist_MS, ENTRY(ocEXP_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_EXP_DIST_MS, 3, { 0, 0, 0 }, 0 },
        { ocHypGeomDist_MS, ENTRY(ocHYP_GEOM_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_HYP_GEOM_DIST_MS, 5, { 0, 0, 0, 0, 0 }, 0 },
        { ocPoissonDist_MS, ENTRY(ocPOISSON_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_POISSON_DIST_MS, 3, { 0, 0, 0 }, 0 },
        { ocWeibull_MS, ENTRY(ocWEIBULL_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_WEIBULL_DIST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocGammaDist_MS, ENTRY(ocGAMMA_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMADIST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocGammaInv_MS, ENTRY(ocGAMMA_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMAINV_MS, 3, { 0, 0, 0 }, 0 },
        { ocLogNormDist_MS, ENTRY(ocLOG_NORM_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGNORMDIST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocLogInv_MS, ENTRY(ocLOG_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGINV, 3, { 0, 0, 0 }, 0 },
        { ocNormDist_MS, ENTRY(ocNORM_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMDIST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocNormInv_MS, ENTRY(ocNORM_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMINV_MS, 3, { 0, 0, 0 }, 0 },
        { ocStdNormDist_MS, ENTRY(ocSTD_NORM_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STD_NORMDIST_MS, 2, { 0, 0 }, 0 },
        { ocSNormInv_MS, ENTRY(ocS_NORM_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STD_NORMINV_MS, 1, { 0 }, 0 },
        { ocTDist_MS, ENTRY(ocT_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TDIST_MS, 3, { 0, 0, 0 }, 0 },
        { ocTDist_RT, ENTRY(ocT_DIST_RT_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TDIST_RT, 2, { 0, 0 }, 0 },
        { ocTDist_2T, ENTRY(ocT_DIST_2T_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TDIST_2T, 2, { 0, 0 }, 0 },
        { ocTInv_2T, ENTRY(ocT_INV_2T_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TINV_2T, 2, { 0, 0 }, 0 },
        { ocTInv_MS, ENTRY(ocT_INV_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TINV_MS, 2, { 0, 0 }, 0 },
        { ocTTest_MS, ENTRY(ocT_TEST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TTEST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocPercentile_Inc, ENTRY(ocPERCENTILE_INC_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTILE_INC, 2, { 0, 0 }, 0 },
        { ocPercentrank_Inc, ENTRY(ocPERCENT_RANK_INC_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTRANK_INC, 3, { 0, 0, 1 }, 0 },
        { ocQuartile_Inc, ENTRY(ocQUARTILE_INC_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUARTILE_INC, 2, { 0, 0 }, 0 },
        { ocRank_Eq, ENTRY(ocRANK_EQ_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_RANK_EQ, 3, { 0, 0, 1 }, 0 },
        { ocPercentile_Exc, ENTRY(ocPERCENTILE_EXC_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTILE_EXC, 2, { 0, 0 }, 0 },
        { ocPercentrank_Exc, ENTRY(ocPERCENT_RANK_EXC_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTRANK_EXC, 3, { 0, 0, 1 }, 0 },
        { ocQuartile_Exc, ENTRY(ocQUARTILE_EXC_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUARTILE_EXC, 2, { 0, 0 }, 0 },
        { ocRank_Avg, ENTRY(ocRANK_AVG_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_RANK_AVG, 3, { 0, 0, 1 }, 0 },
        { ocModalValue_MS, ENTRY(ocMODAL_VALUE_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MODAL_VALUE_MS, VAR_ARGS, { 0 }, 0 },
        { ocModalValue_Multi, ENTRY(ocMODAL_VALUE_MULTI_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MODAL_VALUE_MULTI, VAR_ARGS, { 0 }, 0 },
        { ocNegBinomDist_MS, ENTRY(ocNEG_BINOM_DIST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NEGBINOMDIST_MS, 4, { 0, 0, 0, 0 }, 0 },
        { ocZTest_MS, ENTRY(ocZ_TEST_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_Z_TEST_MS, 3, { 0, 0, 1 }, 0 },
        { ocCeil_MS, ENTRY(ocCEIL_MS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_MS, 2, { 0, 0 }, 0 },
        { ocCeil_ISO, ENTRY(ocCEIL_ISO_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_ISO, 2, { 0, 1 }, 0 },
        { ocFloor_MS, ENTRY(ocFLOOR_MS_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FLOOR_MS, 2, { 0, 0 }, 0 },
        { ocNetWorkdays_MS, ENTRY(ocNETWORKDAYS_MS_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_NETWORKDAYS_MS, 4, { 0, 0, 1, 1 }, 0 },
        { ocWorkday_MS, ENTRY(ocWORKDAY_MS_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_WORKDAY_MS, 4, { 0, 0, 1, 1 }, 0 },
        { ocAggregate, ENTRY(ocAGGREGATE_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_AGGREGATE, VAR_ARGS+3, { 0, 0, 0, 1 }, 0 },
        { ocColor, ENTRY(ocCOLOR_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COLOR, 4, { 0, 0, 0, 1 }, 0 },
        { ocCeil_Math, ENTRY(ocCEIL_MATH_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_MATH, 3, { 0, 1, 1 }, 0 },
        { ocCeil_Precise, ENTRY(ocCEIL_PRECISE_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_PRECISE, 2, { 0, 1 }, 0 },
        { ocNetWorkdays, ENTRY(ocNETWORKDAYS_ARY), 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_NETWORKDAYS, 4, { 0, 0, 1, 1 }, 0 },
        { ocFloor_Math, ENTRY(ocFLOOR_MATH_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FLOOR_MATH, 3, { 0, 1, 1 }, 0 },
        { ocFloor_Precise, ENTRY(ocFLOOR_PRECISE_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FLOOR_PRECISE, 2, { 0, 1 }, 0 },
        { ocRawSubtract, ENTRY(ocRAWSUBTRACT_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RAWSUBTRACT, VAR_ARGS+2, { 0, 0, 1 }, 0 },
        { ocWeeknumOOo, ENTRY(ocWEEKNUM_OOO_ARY), 2, ID_FUNCTION_GRP_DATETIME, HID_FUNC_WEEKNUM_OOO, 2, { 0, 0 }, 0 },
        { ocForecast_ETS_ADD, ENTRY(ocFORECAST_ETS_ADD_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_ADD, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocForecast_ETS_SEA, ENTRY(ocFORECAST_ETS_SEA_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_SEA, 4, { 0, 0, 1, 1 }, 0 },
        { ocForecast_ETS_MUL, ENTRY(ocFORECAST_ETS_MUL_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_MUL, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocForecast_ETS_PIA, ENTRY(ocFORECAST_ETS_PIA_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_PIA, 7, { 0, 0, 0, 1, 1, 1, 1 }, 0 },
        { ocForecast_ETS_PIM, ENTRY(ocFORECAST_ETS_PIM_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_PIM, 7, { 0, 0, 0, 1, 1, 1, 1 }, 0 },
        { ocForecast_ETS_STA, ENTRY(ocFORECAST_ETS_STA_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_STA, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocForecast_ETS_STM, ENTRY(ocFORECAST_ETS_STM_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_STM, 6, { 0, 0, 0, 1, 1, 1 }, 0 },
        { ocForecast_LIN, ENTRY(ocFORECAST_LIN_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_LIN, 3, { 0, 0, 0 }, 0 },
        { ocConcat_MS, ENTRY(ocCONCAT_MS_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_CONCAT_MS, VAR_ARGS, { 0 }, 0 },
        { ocTextJoin_MS, ENTRY(ocTEXTJOIN_MS_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEXTJOIN_MS, VAR_ARGS + 2, { 0, 0, 0 }, 0 },
        { ocIfs_MS, ENTRY(ocIFS_MS_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_IFS_MS, PAIRED_VAR_ARGS, { 0, 0 }, 0 },
        { ocSwitch_MS, ENTRY(ocSWITCH_MS_ARY), 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_SWITCH_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 }, 0 },
        { ocMinIfs_MS, ENTRY(ocMINIFS_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MINIFS_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 }, 0 },
        { ocMaxIfs_MS, ENTRY(ocMAXIFS_MS_ARY), 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MAXIFS_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 }, 0 },
        { ocRoundSig, ENTRY(ocROUNDSIG_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ROUNDSIG, 2, { 0, 0 }, 0 },
        { ocReplaceB, ENTRY(ocREPLACEB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_REPLACEB, 4, { 0, 0, 0, 0 }, 0 },
        { ocFindB, ENTRY(ocFINDB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FINDB, 3, { 0, 0, 1 }, 0 },
        { ocSearchB, ENTRY(ocSEARCHB_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_SEARCHB, 3, { 0, 0, 1 }, 0 },
        { ocRegex, ENTRY(ocREGEX_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_REGEX, 4, { 0, 0, 1, 1 }, 0 },
        { ocFourier, ENTRY(ocFOURIER_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_FOURIER, 5, { 0, 0, 1, 1, 1 }, 0 },
        { ocRandbetweenNV, ENTRY(ocRANDBETWEEN_NV_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RANDBETWEEN_NV, 2, { 0, 0 }, 0 },
        { ocFilter, ENTRY(ocFILTER_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_FILTER_MS, 3, { 0, 0, 1 }, 0 },
        { ocSort, ENTRY(ocSORT_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SORT_MS, 4, { 0, 1, 1, 1 }, 0 },
        { ocSortBy, ENTRY(ocSORTBY_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SORTBY_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 1 }, 0 },
        { ocMatSequence, ENTRY(ocMAT_SEQUENCE_ARY), 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MSEQUENCE_MS, 4, { 0, 1, 1, 1 }, 0 },
        { ocRandArray, ENTRY(ocRANDARRAY_ARY), 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RANDARRAY_MS, 5, { 1, 1, 1, 1, 1 }, 0 },
        { ocChooseCols, ENTRY(ocCHOOSECOLS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_CHOOSECOLS_MS, VAR_ARGS + 1, { 0, 0 }, 0 },
        { ocChooseRows, ENTRY(ocCHOOSEROWS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_CHOOSEROWS_MS, VAR_ARGS + 1, { 0, 0 }, 0 },
        { ocDrop, ENTRY(ocDROP_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_DROP_MS, 3, { 0, 0, 1 }, 0 },
        { ocExpand, ENTRY(ocEXPAND_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_EXPAND_MS, 4, { 0, 0, 1, 1 }, 0 },
        { ocHStack, ENTRY(ocHSTACK_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_HSTACK_MS, VAR_ARGS + 1, { 0, 0 }, 0 },
        { ocVStack, ENTRY(ocVSTACK_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VSTACK_MS, VAR_ARGS + 1, { 0, 0 }, 0 },
        { ocTake, ENTRY(ocTAKE_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TAKE_MS, 3, { 0, 1, 1 }, 0 },
        { ocTextAfter, ENTRY(ocTEXTAFTER_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEXTAFTER_MS, 6, { 0, 1, 1, 1, 1, 1 }, 0 },
        { ocTextBefore, ENTRY(ocTEXTBEFORE_ARY), 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEXTBEFORE_MS, 6, { 0, 1, 1, 1, 1, 1 }, 0 },
        { ocTextSplit, ENTRY(ocTEXTSPLIT_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TEXTSPLIT_MS, 6, { 0, 1, 1, 1, 1, 1 }, 0 },
        { ocToCol, ENTRY(ocTOCOL_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TOCOL_MS, 3, { 0, 1, 1 }, 0 },
        { ocToRow, ENTRY(ocTOROW_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TOROW_MS, 3, { 0, 1, 1 }, 0 },
        { ocUnique, ENTRY(ocUNIQUE_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_UNIQUE_MS, 3, { 0, 1, 1 }, 0 },
        { ocWrapCols, ENTRY(ocWRAPCOLS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_WRAPCOLS_MS, 3, { 0, 1, 1 }, 0 },
        { ocWrapRows, ENTRY(ocWRAPROWS_ARY), 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_WRAPROWS_MS, 3, { 0, 1, 1 }, 0 },
    };

    ScFuncDesc* pDesc = nullptr;
    sal_Int32 nStrLen = 0;
    ::std::vector<const ScFuncDesc*> tmpFuncVector;

    // Browse for all possible OpCodes. This is not the fastest method, but
    // otherwise the sub resources within the resource blocks and the
    // resource blocks themselves would had to be ordered according to
    // OpCodes, which is utopian...
    ScFuncDescCore const * pDescsEnd = aDescs + SAL_N_ELEMENTS(aDescs);
    for (sal_uInt16 i = 0; i <= ocLastOpcodeId; ++i)
    {
        const ScFuncDescCore *pEntry = std::lower_bound(aDescs, pDescsEnd, i,
            [](const ScFuncDescCore &rItem, sal_uInt16 key)
            {
                return rItem.nOpCode < key;
            }
        );

        // Opcode Resource available?
        if (pEntry != pDescsEnd && pEntry->nOpCode == i && pEntry->pResource)
        {
            pDesc = new ScFuncDesc;
            bool bSuppressed = false;

            ScFuncRes(*pEntry, pDesc, bSuppressed);
            // Instead of dealing with this exceptional case at 1001 places
            // we simply don't add an entirely suppressed function to the
            // list and delete it.
            if (bSuppressed)
                delete pDesc;
            else
            {
                pDesc->nFIndex = i;
                tmpFuncVector.push_back(pDesc);

                nStrLen = pDesc->mxFuncName->getLength();
                if (nStrLen > nMaxFuncNameLen)
                    nMaxFuncNameLen = nStrLen;
            }
        }
    }

    // legacy binary AddIn functions

    sal_uInt16 nNextId = ocLastOpcodeId + 1; // FuncID for AddIn functions

    // Interpretation of AddIn list
    OUString aDefArgNameValue   = u"value"_ustr;
    OUString aDefArgNameString  = u"string"_ustr;
    OUString aDefArgNameValues  = u"values"_ustr;
    OUString aDefArgNameStrings = u"strings"_ustr;
    OUString aDefArgNameCells   = u"cells"_ustr;
    OUString aDefArgNameNone    = u"none"_ustr;
    OUString aDefArgDescValue   = u"a value"_ustr;
    OUString aDefArgDescString  = u"a string"_ustr;
    OUString aDefArgDescValues  = u"array of values"_ustr;
    OUString aDefArgDescStrings = u"array of strings"_ustr;
    OUString aDefArgDescCells   = u"range of cells"_ustr;
    OUString aDefArgDescNone    = u"none"_ustr;

    OUString aArgName, aArgDesc;
    const LegacyFuncCollection& rLegacyFuncColl = *ScGlobal::GetLegacyFuncCollection();
    for (auto const& legacyFunc : rLegacyFuncColl)
    {
        const LegacyFuncData *const pLegacyFuncData = legacyFunc.second.get();
        pDesc = new ScFuncDesc;
        sal_uInt16 nArgs = pLegacyFuncData->GetParamCount() - 1;
        pLegacyFuncData->getParamDesc( aArgName, aArgDesc, 0 );
        pDesc->nFIndex     = nNextId++; //  ??? OpCode vergeben
        pDesc->nCategory   = ID_FUNCTION_GRP_ADDINS;
        pDesc->mxFuncName = pLegacyFuncData->GetInternalName().toAsciiUpperCase();
        pDesc->mxFuncDesc = aArgDesc + "\n"
             "( AddIn: " + pLegacyFuncData->GetModuleName() + " )";
        pDesc->nArgCount   = nArgs;
        if (nArgs)
        {
            pDesc->maDefArgNames.clear();
            pDesc->maDefArgNames.resize(nArgs);
            pDesc->maDefArgDescs.clear();
            pDesc->maDefArgDescs.resize(nArgs);
            pDesc->pDefArgFlags  = new ScFuncDesc::ParameterFlags[nArgs];
            for (sal_uInt16 j = 0; j < nArgs; ++j)
            {
                pDesc->pDefArgFlags[j].bOptional = false;
                pLegacyFuncData->getParamDesc( aArgName, aArgDesc, j+1 );
                if ( !aArgName.isEmpty() )
                    pDesc->maDefArgNames[j] = aArgName;
                else
                {
                    switch (pLegacyFuncData->GetParamType(j+1))
                    {
                        case ParamType::PTR_DOUBLE:
                            pDesc->maDefArgNames[j] = aDefArgNameValue;
                            break;
                        case ParamType::PTR_STRING:
                            pDesc->maDefArgNames[j] = aDefArgNameString;
                            break;
                        case ParamType::PTR_DOUBLE_ARR:
                            pDesc->maDefArgNames[j] = aDefArgNameValues;
                            break;
                        case ParamType::PTR_STRING_ARR:
                            pDesc->maDefArgNames[j] = aDefArgNameStrings;
                            break;
                        case ParamType::PTR_CELL_ARR:
                            pDesc->maDefArgNames[j] = aDefArgNameCells;
                            break;
                        default:
                            pDesc->maDefArgNames[j] = aDefArgNameNone;
                            break;
                    }
                }
                if ( !aArgDesc.isEmpty() )
                    pDesc->maDefArgDescs[j] = aArgDesc;
                else
                {
                    switch (pLegacyFuncData->GetParamType(j+1))
                    {
                        case ParamType::PTR_DOUBLE:
                            pDesc->maDefArgDescs[j] = aDefArgDescValue;
                            break;
                        case ParamType::PTR_STRING:
                            pDesc->maDefArgDescs[j] = aDefArgDescString;
                            break;
                        case ParamType::PTR_DOUBLE_ARR:
                            pDesc->maDefArgDescs[j] = aDefArgDescValues;
                            break;
                        case ParamType::PTR_STRING_ARR:
                            pDesc->maDefArgDescs[j] = aDefArgDescStrings;
                            break;
                        case ParamType::PTR_CELL_ARR:
                            pDesc->maDefArgDescs[j] = aDefArgDescCells;
                            break;
                        default:
                            pDesc->maDefArgDescs[j] = aDefArgDescNone;
                            break;
                    }
                }
            }
        }

        tmpFuncVector.push_back(pDesc);
        nStrLen = pDesc->mxFuncName->getLength();
        if ( nStrLen > nMaxFuncNameLen)
            nMaxFuncNameLen = nStrLen;
    }

    // StarOne AddIns

    ScUnoAddInCollection* pUnoAddIns = ScGlobal::GetAddInCollection();
    tools::Long nUnoCount = pUnoAddIns->GetFuncCount();
    for (tools::Long nFunc=0; nFunc<nUnoCount; nFunc++)
    {
        pDesc = new ScFuncDesc;
        pDesc->nFIndex = nNextId++;

        if ( pUnoAddIns->FillFunctionDesc( nFunc, *pDesc, mbEnglishFunctionNames ) )
        {
            tmpFuncVector.push_back(pDesc);
            nStrLen = pDesc->mxFuncName->getLength();
            if (nStrLen > nMaxFuncNameLen)
                nMaxFuncNameLen = nStrLen;
        }
        else
            delete pDesc;
    }

    aFunctionList.swap(tmpFuncVector);

    //Initialize iterator
    aFunctionListIter = aFunctionList.end();
}

ScFunctionList::~ScFunctionList()
{
    const ScFuncDesc* pDesc = First();
    while (pDesc)
    {
        delete pDesc;
        pDesc = Next();
    }
}

const ScFuncDesc* ScFunctionList::First()
{
    const ScFuncDesc* pDesc = nullptr;
    aFunctionListIter = aFunctionList.begin();
    if(aFunctionListIter != aFunctionList.end())
        pDesc = *aFunctionListIter;

    return pDesc;
}

const ScFuncDesc* ScFunctionList::Next()
{
    const ScFuncDesc* pDesc = nullptr;
    if(aFunctionListIter != aFunctionList.end())
    {
        if((++aFunctionListIter) != aFunctionList.end())
            pDesc = *aFunctionListIter;
    }
    return pDesc;
}

const ScFuncDesc* ScFunctionList::GetFunction( sal_uInt32 nIndex ) const
{
    const ScFuncDesc* pDesc = nullptr;
    if(nIndex < aFunctionList.size())
        pDesc = aFunctionList.at(nIndex);

    return pDesc;
}


sal_uInt32 ScFunctionCategory::getCount() const
{
    return m_rCategory.size();
}

OUString ScFunctionCategory::getName() const
{
    if ( m_sName.isEmpty() )
        m_sName = ScFunctionMgr::GetCategoryName(m_nCategory);
    return m_sName;
}

const formula::IFunctionDescription* ScFunctionCategory::getFunction(sal_uInt32 _nPos) const
{
    const ScFuncDesc* pDesc = nullptr;
    if(_nPos < m_rCategory.size())
        pDesc = m_rCategory.at(_nPos);
    return pDesc;
}

sal_uInt32 ScFunctionCategory::getNumber() const
{
    return m_nCategory;
}


ScFunctionMgr::ScFunctionMgr()
{
    ScFunctionList* pFuncList /**< list of all calc functions */
        = ScGlobal::GetStarCalcFunctionList();

    assert(pFuncList && "Functionlist not found.");
    sal_uInt32 catCount[MAX_FUNCCAT] = {0};

    aCatLists[0].reserve(pFuncList->GetCount());

    // Retrieve all functions, store in cumulative ("All") category, and count
    // number of functions in each category
    for(const ScFuncDesc* pDesc = pFuncList->First(); pDesc; pDesc = pFuncList->Next())
    {
        OSL_ENSURE((pDesc->nCategory) < MAX_FUNCCAT, "Unknown category");
        if ((pDesc->nCategory) < MAX_FUNCCAT)
            ++catCount[pDesc->nCategory];
        aCatLists[0].push_back(pDesc);
    }

    // Sort functions in cumulative category by name
    ::std::sort(aCatLists[0].begin(), aCatLists[0].end(), ScFuncDesc::compareByName);

    // Allocate correct amount of space for categories
    for (sal_uInt16 i = 1; i < MAX_FUNCCAT; ++i)
    {
        aCatLists[i].reserve(catCount[i]);
    }

    // Fill categories with the corresponding functions (still sorted by name)
    for (auto const& elemList : aCatLists[0])
    {
        if ((elemList->nCategory) < MAX_FUNCCAT)
            aCatLists[elemList->nCategory].push_back(elemList);
    }

    // Initialize iterators
    pCurCatListIter = aCatLists[0].end();
    pCurCatListEnd = aCatLists[0].end();
}

ScFunctionMgr::~ScFunctionMgr()
{
}


const formula::IFunctionDescription* ScFunctionMgr::Get( sal_uInt16 nFIndex ) const
{
    const ScFuncDesc* pDesc;
    for (pDesc = First(); pDesc; pDesc = Next())
        if (pDesc->nFIndex == nFIndex)
            break;
    return pDesc;
}

const ScFuncDesc* ScFunctionMgr::First( sal_uInt16 nCategory ) const
{
    OSL_ENSURE( nCategory < MAX_FUNCCAT, "Unknown category" );
    const ScFuncDesc* pDesc = nullptr;
    if ( nCategory < MAX_FUNCCAT )
    {
        pCurCatListIter = aCatLists[nCategory].begin();
        pCurCatListEnd = aCatLists[nCategory].end();
        pDesc = *pCurCatListIter;
    }
    else
    {
        pCurCatListIter = aCatLists[0].end();
        pCurCatListEnd = aCatLists[0].end();
    }
    return pDesc;
}

const ScFuncDesc* ScFunctionMgr::Next() const
{
    const ScFuncDesc* pDesc = nullptr;
    if ( pCurCatListIter != pCurCatListEnd )
    {
        if ( (++pCurCatListIter) != pCurCatListEnd )
        {
            pDesc = *pCurCatListIter;
        }
    }
    return pDesc;
}

sal_uInt32 ScFunctionMgr::getCount() const
{
    return MAX_FUNCCAT - 1;
}

const formula::IFunctionCategory* ScFunctionMgr::getCategory(sal_uInt32 nCategory) const
{
    if ( nCategory < (MAX_FUNCCAT-1) )
    {
        if (m_aCategories.find(nCategory) == m_aCategories.end())
            m_aCategories[nCategory] = std::make_shared<ScFunctionCategory>(aCatLists[nCategory+1],nCategory); // aCatLists[0] is "all"
        return m_aCategories[nCategory].get();
    }
    return nullptr;
}

sal_uInt16 ScFunctionMgr::getFunctionIndex(const formula::IFunctionDescription* _pDesc) const
{
    const ScFuncDesc* pDesc = dynamic_cast<const ScFuncDesc*>(_pDesc);
    return pDesc ? pDesc->nFIndex : 0;
}

void ScFunctionMgr::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const
{
    const ScAppOptions& rAppOpt = ScModule::get()->GetAppOptions();
    sal_uInt16 nLRUFuncCount = std::min( rAppOpt.GetLRUFuncListCount(), sal_uInt16(LRU_MAX) );
    sal_uInt16* pLRUListIds = rAppOpt.GetLRUFuncList();
    _rLastRUFunctions.clear();

    if ( pLRUListIds )
    {
        for (sal_uInt16 i = 0; i < nLRUFuncCount; ++i)
        {
            _rLastRUFunctions.push_back( Get( pLRUListIds[i] ) );
        }
    }
}

void ScFunctionMgr::fillFavouriteFunctions(std::unordered_set<sal_uInt16>& rFavouriteFunctions) const
{
    const ScAppOptions& rAppOpt = ScModule::get()->GetAppOptions();
    rFavouriteFunctions.clear();
    rFavouriteFunctions = rAppOpt.GetFavouritesList();
}

OUString ScFunctionMgr::GetCategoryName(sal_uInt32 _nCategoryNumber )
{
    if (_nCategoryNumber >= SC_FUNCGROUP_COUNT)
    {
        OSL_FAIL("Invalid category number!");
        return OUString();
    }

    return ScResId(RID_FUNCTION_CATEGORIES[_nCategoryNumber]);
}

sal_Unicode ScFunctionMgr::getSingleToken(const formula::IFunctionManager::EToken _eToken) const
{
    switch(_eToken)
    {
        case eOk:
            return ScCompiler::GetNativeSymbolChar(ocOpen);
        case eClose:
            return ScCompiler::GetNativeSymbolChar(ocClose);
        case eSep:
            return ScCompiler::GetNativeSymbolChar(ocSep);
        case eArrayOpen:
            return ScCompiler::GetNativeSymbolChar(ocArrayOpen);
        case eArrayClose:
            return ScCompiler::GetNativeSymbolChar(ocArrayClose);
        case eTableRefOpen:
            return ScCompiler::GetNativeSymbolChar(ocTableRefOpen);
        case eTableRefClose:
            return ScCompiler::GetNativeSymbolChar(ocTableRefClose);
    }
    return 0;
}

static void ScFuncRes(const ScFuncDescCore &rEntry, ScFuncDesc* pDesc, bool& rbSuppressed)
{
    const sal_uInt16 nOpCode = rEntry.nOpCode;
    sal_uInt16 nFunctionFlags = rEntry.nFunctionFlags;
    // Bit 1: entirely suppressed
    // Bit 2: hidden unless used
    rbSuppressed = ((nFunctionFlags & 1) != 0);
    pDesc->mbHidden = ((nFunctionFlags & 2) != 0);
    pDesc->nCategory = rEntry.nCategory;
    pDesc->sHelpId = rEntry.pHelpId;
    pDesc->nArgCount = rEntry.nArgs;
    sal_uInt16 nArgs = pDesc->nArgCount;
    sal_uInt16 nVarArgsSet = 0;
    if (nArgs >= PAIRED_VAR_ARGS)
    {
        nVarArgsSet = 2;
        nArgs -= PAIRED_VAR_ARGS - nVarArgsSet;
    }
    else if (nArgs >= VAR_ARGS)
    {
        nVarArgsSet = 1;
        nArgs -= VAR_ARGS - nVarArgsSet;
    }
    assert(nArgs <= SAL_N_ELEMENTS(rEntry.aOptionalArgs));
    if (nArgs)
    {
        pDesc->nVarArgsStart = nArgs - nVarArgsSet;
        pDesc->nVarArgsLimit = rEntry.nVarArgsLimit;
        pDesc->pDefArgFlags = new ScFuncDesc::ParameterFlags[nArgs];
        for (sal_uInt16 i = 0; i < nArgs; ++i)
        {
            pDesc->pDefArgFlags[i].bOptional = static_cast<bool>(rEntry.aOptionalArgs[i]);
        }
    }

    pDesc->mxFuncName = ScCompiler::GetNativeSymbol(static_cast<OpCode>(nOpCode));
    pDesc->mxFuncDesc = ScResId(rEntry.pResource[0]);

    if (!nArgs)
        return;

    pDesc->maDefArgNames.clear();
    pDesc->maDefArgNames.resize(nArgs);
    pDesc->maDefArgDescs.clear();
    pDesc->maDefArgDescs.resize(nArgs);
    for (sal_uInt16 i = 0; i < nArgs; ++i)
    {
        size_t nIndex = (i * 2) + 1;
        if (nIndex < rEntry.nResourceLen)
            pDesc->maDefArgNames[i] = ScResId(rEntry.pResource[nIndex]);
        if (nIndex + 1 < rEntry.nResourceLen)
            pDesc->maDefArgDescs[i] = ScResId(rEntry.pResource[nIndex + 1]);
        // If empty and variable number of arguments and last parameter and
        // parameter is optional and the previous is not optional, repeat
        // previous parameter name and description.
        if ((pDesc->maDefArgNames[i].isEmpty() || pDesc->maDefArgDescs[i].isEmpty()) &&
                nVarArgsSet > 0 && i > nVarArgsSet && (i == nArgs-1 || i == nArgs-2) &&
                pDesc->pDefArgFlags[i].bOptional)
        {
            sal_uInt16 nPrev = i - nVarArgsSet;
            if (!pDesc->pDefArgFlags[nPrev].bOptional)
            {
                if (pDesc->maDefArgNames[i].isEmpty())
                    pDesc->maDefArgNames[i] = pDesc->maDefArgNames[nPrev];
                if (pDesc->maDefArgDescs[i].isEmpty())
                    pDesc->maDefArgDescs[i] = pDesc->maDefArgDescs[nPrev];
                // This also means that variable arguments start one
                // parameter set earlier.
                pDesc->nVarArgsStart -= nVarArgsSet;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
