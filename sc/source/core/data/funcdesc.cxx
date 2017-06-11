/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include "funcdesc.hxx"

#include "addincol.hxx"
#include "appoptio.hxx"
#include "callform.hxx"
#include "compiler.hxx"
#include "compiler.hrc"
#include "global.hxx"
#include "sc.hrc"
#include "scfuncs.hrc"
#include "scmod.hxx"
#include "scresid.hxx"
#include "helpids.h"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <tools/resmgr.hxx>
#include <unotools/collatorwrapper.hxx>

#include <memory>
#include <numeric>

struct ScFuncDescCore
{
    /*
     * An opcode from include/formula/compiler.hrc
     */
    sal_uInt16 nOpCode;
    /*
     * Pointer to nullptr terminated list of strings
     */
    const char** pResource;
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
    const char* pHelpId;
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
};

class ScFuncRes
{
public:
    ScFuncRes(const ScFuncDescCore &rEntry, ScFuncDesc*, bool& rbSuppressed);
};

// class ScFuncDesc:
ScFuncDesc::ScFuncDesc() :
        pFuncName       (nullptr),
        pFuncDesc       (nullptr),
        pDefArgFlags    (nullptr),
        nFIndex         (0),
        nCategory       (0),
        nArgCount       (0),
        nVarArgsStart   (0),
        bIncomplete     (false),
        bHasSuppressedArgs(false),
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
    maDefArgNames.clear();
    maDefArgDescs.clear();
    pDefArgFlags = nullptr;

    delete pFuncName;
    pFuncName = nullptr;

    delete pFuncDesc;
    pFuncDesc = nullptr;

    nFIndex = 0;
    nCategory = 0;
    sHelpId.clear();
    bIncomplete = false;
    bHasSuppressedArgs = false;
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
                    aSig.append(sep);
                    aSig.append( " " );
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
                aSig.append(maDefArgNames[nArg]);
                aSig.append(sep);
                aSig.append( " " );
            }
            /* NOTE: Currently there are no suppressed var args parameters. If
             * there were, we'd have to cope with it here and above for the fix
             * parameters. For now parameters are always added, so no special
             * treatment of a trailing "; " necessary. */
            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('1');
            aSig.append(sep);
            aSig.append(' ');
            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('2');
            aSig.append(sep);
            aSig.append(" ... ");
        }
        else
        {
            for ( sal_uInt16 nArg = 0; nArg < nVarArgsStart; nArg++ )
            {
                aSig.append(maDefArgNames[nArg]);
                aSig.append(sep);
                aSig.append( " " );
            }

            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('1');
            aSig.append(sep);
            aSig.append(maDefArgNames[nVarArgsStart+1]);
            aSig.append('1');
            aSig.append(sep);
            aSig.append( " " );
            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('2');
            aSig.append(sep);
            aSig.append(maDefArgNames[nVarArgsStart+1]);
            aSig.append('2');
            aSig.append(sep);
            aSig.append( " ... " );
        }
    }

    return aSig.makeStringAndClear();
}

OUString ScFuncDesc::getSignature() const
{
    OUStringBuffer aSig;

    if(pFuncName)
    {
        aSig.append(*pFuncName);

        OUString aParamList = GetParamList();
        if( !aParamList.isEmpty() )
        {
            aSig.append( "( " );
            aSig.append(aParamList);
            // U+00A0 (NBSP) prevents automatic line break
            aSig.append( u'\x00A0' );
            aSig.append( ")" );
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

    if(pFuncName)
    {
        aFormula.append( *pFuncName );

        aFormula.append( "(" );
        if ( nArgCount > 0 && !_aArguments.empty() && !_aArguments[0].isEmpty())
        {
            ::std::vector< OUString >::const_iterator aIter = _aArguments.begin();
            ::std::vector< OUString >::const_iterator aEnd = _aArguments.end();

            aFormula.append( *aIter );
            ++aIter;
            while( aIter != aEnd && !aIter->isEmpty() )
            {
                aFormula.append( sep );
                aFormula.append( *aIter );
                ++aIter;
            }
        }

        aFormula.append( ")" );
    }
    return aFormula.makeStringAndClear();
}

sal_uInt16 ScFuncDesc::GetSuppressedArgCount() const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
        return nArgCount;

    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= PAIRED_VAR_ARGS)
        nArgs -= PAIRED_VAR_ARGS - 2;
    else if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    sal_uInt16 nCount = nArgs;
    if (nArgCount >= PAIRED_VAR_ARGS)
        nCount += PAIRED_VAR_ARGS - 2;
    else if (nArgCount >= VAR_ARGS)
        nCount += VAR_ARGS - 1;
    return nCount;
}

OUString ScFuncDesc::getFunctionName() const
{
    OUString sRet;
    if ( pFuncName )
        sRet = *pFuncName;
    return sRet;
}

const formula::IFunctionCategory* ScFuncDesc::getCategory() const
{
    return ScGlobal::GetStarCalcFunctionMgr()->getCategory(nCategory);
}

OUString ScFuncDesc::getDescription() const
{
    OUString sRet;
    if ( pFuncDesc )
        sRet = *pFuncDesc;
    return sRet;
}

sal_Int32 ScFuncDesc::getSuppressedArgumentCount() const
{
    return GetSuppressedArgCount();
}

void ScFuncDesc::fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
    {
        _rArguments.resize( nArgCount);
        ::std::vector<sal_uInt16>::iterator iter = _rArguments.begin();
        sal_uInt16 value = 0;
        while (iter != _rArguments.end())
            *iter++ = value++;
    }

    _rArguments.reserve( nArgCount);
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

    if ( bIncomplete && pFuncName )
    {
        ScUnoAddInCollection& rAddIns = *ScGlobal::GetAddInCollection();
        OUString aIntName(rAddIns.FindFunction( *pFuncName, true ));         // pFuncName is upper-case

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
}

OString ScFuncDesc::getHelpId() const
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
    return (ScGlobal::GetCaseCollator()->compareString(*a->pFuncName, *b->pFuncName ) < 0);
}

// class ScFunctionList:
ScFunctionList::ScFunctionList()
    : nMaxFuncNameLen(0)
{
    // See ScFuncDescCore definition for format details.
    // This list must be sorted in order of the opcode, dbgutil builds enable _GLIBCXX_DEBUG
    // which will concept check that the list is sorted on first use to ensure this holds
    ScFuncDescCore aDescs[] =
    {
        { SC_OPCODE_IF, SC_OPCODE_IF_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_WENN, 3, { 0, 1, 1 } },
        { SC_OPCODE_IF_ERROR, SC_OPCODE_IF_ERROR_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_IFERROR, 2, { 0, 0 } },
        { SC_OPCODE_IF_NA, SC_OPCODE_IF_NA_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_IFNA, 2, { 0, 0 } },
        { SC_OPCODE_CHOOSE, SC_OPCODE_CHOOSE_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_WAHL, VAR_ARGS+1, { 0, 0 } },
        { SC_OPCODE_AND, SC_OPCODE_AND_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_UND, VAR_ARGS, { 0 } },
        { SC_OPCODE_OR, SC_OPCODE_OR_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_ODER, VAR_ARGS, { 0 } },
        { SC_OPCODE_NOT, SC_OPCODE_NOT_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_NICHT, 1, { 0 } },
        { SC_OPCODE_PI, SC_OPCODE_PI_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_PI, 0, { } },
        { SC_OPCODE_RANDOM, SC_OPCODE_RANDOM_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ZUFALLSZAHL, 0, { } },
        { SC_OPCODE_TRUE, SC_OPCODE_TRUE_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_WAHR, 0, { } },
        { SC_OPCODE_FALSE, SC_OPCODE_FALSE_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_FALSCH, 0, { } },
        { SC_OPCODE_GET_ACT_DATE, SC_OPCODE_GET_ACT_DATE_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_HEUTE, 0, { } },
        { SC_OPCODE_GET_ACT_TIME, SC_OPCODE_GET_ACT_TIME_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_JETZT, 0, { } },
        { SC_OPCODE_NO_VALUE, SC_OPCODE_NO_VALUE_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_NV, 0, { } },
        { SC_OPCODE_CURRENT, SC_OPCODE_CURRENT_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_AKTUELL, 0, { } },
        { SC_OPCODE_DEG, SC_OPCODE_DEG_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_DEG, 1, { 0 } },
        { SC_OPCODE_RAD, SC_OPCODE_RAD_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RAD, 1, { 0 } },
        { SC_OPCODE_SIN, SC_OPCODE_SIN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SIN, 1, { 0 } },
        { SC_OPCODE_COS, SC_OPCODE_COS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COS, 1, { 0 } },
        { SC_OPCODE_TAN, SC_OPCODE_TAN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_TAN, 1, { 0 } },
        { SC_OPCODE_COT, SC_OPCODE_COT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COT, 1, { 0 } },
        { SC_OPCODE_ARC_SIN, SC_OPCODE_ARC_SIN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCSIN, 1, { 0 } },
        { SC_OPCODE_ARC_COS, SC_OPCODE_ARC_COS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCCOS, 1, { 0 } },
        { SC_OPCODE_ARC_TAN, SC_OPCODE_ARC_TAN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCTAN, 1, { 0 } },
        { SC_OPCODE_ARC_COT, SC_OPCODE_ARC_COT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCCOT, 1, { 0 } },
        { SC_OPCODE_SIN_HYP, SC_OPCODE_SIN_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SINHYP, 1, { 0 } },
        { SC_OPCODE_COS_HYP, SC_OPCODE_COS_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COSHYP, 1, { 0 } },
        { SC_OPCODE_TAN_HYP, SC_OPCODE_TAN_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_TANHYP, 1, { 0 } },
        { SC_OPCODE_COT_HYP, SC_OPCODE_COT_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COTHYP, 1, { 0 } },
        { SC_OPCODE_ARC_SIN_HYP, SC_OPCODE_ARC_SIN_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARSINHYP, 1, { 0 } },
        { SC_OPCODE_ARC_COS_HYP, SC_OPCODE_ARC_COS_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCOSHYP, 1, { 0 } },
        { SC_OPCODE_ARC_TAN_HYP, SC_OPCODE_ARC_TAN_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARTANHYP, 1, { 0 } },
        { SC_OPCODE_ARC_COT_HYP, SC_OPCODE_ARC_COT_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCOTHYP, 1, { 0 } },
        { SC_OPCODE_COSECANT, SC_OPCODE_COSECANT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COSECANT, 1, { 0 } },
        { SC_OPCODE_SECANT, SC_OPCODE_SECANT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SECANT, 1, { 0 } },
        { SC_OPCODE_COSECANT_HYP, SC_OPCODE_COSECANT_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COSECANTHYP, 1, { 0 } },
        { SC_OPCODE_SECANT_HYP, SC_OPCODE_SECANT_HYP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SECANTHYP, 1, { 0 } },
        { SC_OPCODE_EXP, SC_OPCODE_EXP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_EXP, 1, { 0 } },
        { SC_OPCODE_LN, SC_OPCODE_LN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_LN, 1, { 0 } },
        { SC_OPCODE_SQRT, SC_OPCODE_SQRT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_WURZEL, 1, { 0 } },
        { SC_OPCODE_FACT, SC_OPCODE_FACT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FAKULTAET, 1, { 0 } },
        { SC_OPCODE_GET_YEAR, SC_OPCODE_GET_YEAR_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_JAHR, 1, { 0 } },
        { SC_OPCODE_GET_MONTH, SC_OPCODE_GET_MONTH_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_MONAT, 1, { 0 } },
        { SC_OPCODE_GET_DAY, SC_OPCODE_GET_DAY_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_TAG, 1, { 0 } },
        { SC_OPCODE_GET_HOUR, SC_OPCODE_GET_HOUR_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_STUNDE, 1, { 0 } },
        { SC_OPCODE_GET_MIN, SC_OPCODE_GET_MIN_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_MINUTE, 1, { 0 } },
        { SC_OPCODE_GET_SEC, SC_OPCODE_GET_SEC_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_SEKUNDE, 1, { 0 } },
        { SC_OPCODE_PLUS_MINUS, SC_OPCODE_PLUS_MINUS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_VORZEICHEN, 1, { 0 } },
        { SC_OPCODE_ABS, SC_OPCODE_ABS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ABS, 1, { 0 } },
        { SC_OPCODE_INT, SC_OPCODE_INT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_GANZZAHL, 1, { 0 } },
        { SC_OPCODE_PHI, SC_OPCODE_PHI_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PHI, 1, { 0 } },
        { SC_OPCODE_GAUSS, SC_OPCODE_GAUSS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAUSS, 1, { 0 } },
        { SC_OPCODE_IS_EMPTY, SC_OPCODE_IS_EMPTY_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTLEER, 1, { 0 } },
        { SC_OPCODE_IS_STRING, SC_OPCODE_IS_STRING_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTTEXT, 1, { 0 } },
        { SC_OPCODE_IS_NON_STRING, SC_OPCODE_IS_NON_STRING_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTKTEXT, 1, { 0 } },
        { SC_OPCODE_IS_LOGICAL, SC_OPCODE_IS_LOGICAL_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTLOG, 1, { 0 } },
        { SC_OPCODE_TYPE, SC_OPCODE_TYPE_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_TYP, 1, { 0 } },
        { SC_OPCODE_IS_REF, SC_OPCODE_IS_REF_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTBEZUG, 1, { 0 } },
        { SC_OPCODE_IS_VALUE, SC_OPCODE_IS_VALUE_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTZAHL, 1, { 0 } },
        { SC_OPCODE_IS_FORMULA, SC_OPCODE_IS_FORMULA_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTFORMEL, 1, { 0 } },
        { SC_OPCODE_IS_NV, SC_OPCODE_IS_NV_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTNV, 1, { 0 } },
        { SC_OPCODE_IS_ERR, SC_OPCODE_IS_ERR_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTFEHL, 1, { 0 } },
        { SC_OPCODE_IS_ERROR, SC_OPCODE_IS_ERROR_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTFEHLER, 1, { 0 } },
        { SC_OPCODE_IS_EVEN, SC_OPCODE_IS_EVEN_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTGERADE, 1, { 0 } },
        { SC_OPCODE_IS_ODD, SC_OPCODE_IS_ODD_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ISTUNGERADE, 1, { 0 } },
        { SC_OPCODE_N, SC_OPCODE_N_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_N, 1, { 0 } },
        { SC_OPCODE_GET_DATE_VALUE, SC_OPCODE_GET_DATE_VALUE_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_DATWERT, 1, { 0 } },
        { SC_OPCODE_GET_TIME_VALUE, SC_OPCODE_GET_TIME_VALUE_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_ZEITWERT, 1, { 0 } },
        { SC_OPCODE_CODE, SC_OPCODE_CODE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_CODE, 1, { 0 } },
        { SC_OPCODE_TRIM, SC_OPCODE_TRIM_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_GLAETTEN, 1, { 0 } },
        { SC_OPCODE_UPPER, SC_OPCODE_UPPER_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_GROSS, 1, { 0 } },
        { SC_OPCODE_PROPER, SC_OPCODE_PROPER_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_GROSS2, 1, { 0 } },
        { SC_OPCODE_LOWER, SC_OPCODE_LOWER_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_KLEIN, 1, { 0 } },
        { SC_OPCODE_LEN, SC_OPCODE_LEN_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LAENGE, 1, { 0 } },
        { SC_OPCODE_T, SC_OPCODE_T_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_T, 1, { 0 } },
        { SC_OPCODE_VALUE, SC_OPCODE_VALUE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WERT, 1, { 0 } },
        { SC_OPCODE_CLEAN, SC_OPCODE_CLEAN_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_SAEUBERN, 1, { 0 } },
        { SC_OPCODE_CHAR, SC_OPCODE_CHAR_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ZEICHEN, 1, { 0 } },
        { SC_OPCODE_LOG10, SC_OPCODE_LOG10_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_LOG10, 1, { 0 } },
        { SC_OPCODE_EVEN, SC_OPCODE_EVEN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_GERADE, 1, { 0 } },
        { SC_OPCODE_ODD, SC_OPCODE_ODD_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_UNGERADE, 1, { 0 } },
        { SC_OPCODE_STD_NORM_DIST, SC_OPCODE_STD_NORM_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STANDNORMVERT, 1, { 0 } },
        { SC_OPCODE_FISHER, SC_OPCODE_FISHER_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FISHER, 1, { 0 } },
        { SC_OPCODE_FISHER_INV, SC_OPCODE_FISHER_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FISHERINV, 1, { 0 } },
        { SC_OPCODE_S_NORM_INV, SC_OPCODE_S_NORM_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STANDNORMINV, 1, { 0 } },
        { SC_OPCODE_GAMMA_LN, SC_OPCODE_GAMMA_LN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMALN, 1, { 0 } },
        { SC_OPCODE_ERROR_TYPE, SC_OPCODE_ERROR_TYPE_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_FEHLERTYP, 1, { 0 } },
        { SC_OPCODE_FORMULA, SC_OPCODE_FORMULA_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_FORMEL, 1, { 0 } },
        { SC_OPCODE_ARABIC, SC_OPCODE_ARABIC_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ARABISCH, 1, { 0 } },
        { SC_OPCODE_INFO, SC_OPCODE_INFO_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_INFO, 1, { 0 } },
        { SC_OPCODE_BAHTTEXT, SC_OPCODE_BAHTTEXT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_BAHTTEXT, 1, { 0 } },
        { SC_OPCODE_JIS, SC_OPCODE_JIS_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_JIS, 1, { 0 } },
        { SC_OPCODE_ASC, SC_OPCODE_ASC_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ASC, 1, { 0 } },
        { SC_OPCODE_UNICODE, SC_OPCODE_UNICODE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_UNICODE, 1, { 0 } },
        { SC_OPCODE_UNICHAR, SC_OPCODE_UNICHAR_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_UNICHAR, 1, { 0 } },
        { SC_OPCODE_GAMMA, SC_OPCODE_GAMMA_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMA, 1, { 0 } },
        { SC_OPCODE_GAMMA_LN_MS, SC_OPCODE_GAMMA_LN_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMALN_MS, 1, { 0 } },
        { SC_OPCODE_ERF_MS, SC_OPCODE_ERF_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ERF_MS, 1, { 0 } },
        { SC_OPCODE_ERFC_MS, SC_OPCODE_ERFC_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ERFC_MS, 1, { 0 } },
        { SC_OPCODE_ERROR_TYPE_ODF, SC_OPCODE_ERROR_TYPE_ODF_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ERROR_TYPE_ODF, 1, { 0 } },
        { SC_OPCODE_ENCODEURL, SC_OPCODE_ENCODEURL_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ENCODEURL, 1, { 0 } },
        { SC_OPCODE_ISOWEEKNUM, SC_OPCODE_ISOWEEKNUM_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_ISOWEEKNUM, 1, { 0 } },
        { SC_OPCODE_ARC_TAN_2, SC_OPCODE_ARC_TAN_2_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ARCTAN2, 2, { 0, 0 } },
        { SC_OPCODE_CEIL, SC_OPCODE_CEIL_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_OBERGRENZE, 3, { 0, 1, 1 } },
        { SC_OPCODE_FLOOR, SC_OPCODE_FLOOR_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_UNTERGRENZE, 3, { 0, 1, 1 } },
        { SC_OPCODE_ROUND, SC_OPCODE_ROUND_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RUNDEN, 2, { 0, 1 } },
        { SC_OPCODE_ROUND_UP, SC_OPCODE_ROUND_UP_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_AUFRUNDEN, 2, { 0, 1 } },
        { SC_OPCODE_ROUND_DOWN, SC_OPCODE_ROUND_DOWN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ABRUNDEN, 2, { 0, 1 } },
        { SC_OPCODE_TRUNC, SC_OPCODE_TRUNC_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KUERZEN, 2, { 0, 0 } },
        { SC_OPCODE_LOG, SC_OPCODE_LOG_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_LOG, 2, { 0, 1 } },
        { SC_OPCODE_POWER, SC_OPCODE_POWER_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_POTENZ, 2, { 0, 0 } },
        { SC_OPCODE_GCD, SC_OPCODE_GCD_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_GGT, VAR_ARGS, { 0 } },
        { SC_OPCODE_LCM, SC_OPCODE_LCM_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KGV, VAR_ARGS, { 0 } },
        { SC_OPCODE_MOD, SC_OPCODE_MOD_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_REST, 2, { 0, 0 } },
        { SC_OPCODE_SUM_PRODUCT, SC_OPCODE_SUM_PRODUCT_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMENPRODUKT, VAR_ARGS, { 0 } },
        { SC_OPCODE_SUM_SQ, SC_OPCODE_SUM_SQ_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_QUADRATESUMME, VAR_ARGS, { 0 } },
        { SC_OPCODE_SUM_X2MY2, SC_OPCODE_SUM_X2MY2_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMEX2MY2, 2, { 0, 0 } },
        { SC_OPCODE_SUM_X2DY2, SC_OPCODE_SUM_X2DY2_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMEX2PY2, 2, { 0, 0 } },
        { SC_OPCODE_SUM_XMY2, SC_OPCODE_SUM_XMY2_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_SUMMEXMY2, 2, { 0, 0 } },
        { SC_OPCODE_GET_DATE, SC_OPCODE_GET_DATE_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_DATUM, 3, { 0, 0, 0 } },
        { SC_OPCODE_GET_TIME, SC_OPCODE_GET_TIME_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_ZEIT, 3, { 0, 0, 0 } },
        { SC_OPCODE_GET_DIFF_DATE, SC_OPCODE_GET_DIFF_DATE_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_TAGE, 2, { 0, 0 } },
        { SC_OPCODE_GET_DIFF_DATE_360, SC_OPCODE_GET_DIFF_DATE_360_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_TAGE360, 3, { 0, 0, 1 } },
        { SC_OPCODE_MIN, SC_OPCODE_MIN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MIN, VAR_ARGS, { 0 } },
        { SC_OPCODE_MAX, SC_OPCODE_MAX_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MAX, VAR_ARGS, { 0 } },
        { SC_OPCODE_SUM, SC_OPCODE_SUM_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SUMME, VAR_ARGS, { 0 } },
        { SC_OPCODE_PRODUCT, SC_OPCODE_PRODUCT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_PRODUKT, VAR_ARGS, { 0 } },
        { SC_OPCODE_AVERAGE, SC_OPCODE_AVERAGE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MITTELWERT, VAR_ARGS, { 0 } },
        { SC_OPCODE_COUNT, SC_OPCODE_COUNT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ANZAHL, VAR_ARGS, { 0 } },
        { SC_OPCODE_COUNT_2, SC_OPCODE_COUNT_2_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ANZAHL2, VAR_ARGS, { 0 } },
        { SC_OPCODE_NPV, SC_OPCODE_NPV_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_NBW, VAR_ARGS+1, { 0, 0 } },
        { SC_OPCODE_IRR, SC_OPCODE_IRR_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_IKV, 2, { 0, 1 } },
        { SC_OPCODE_VAR, SC_OPCODE_VAR_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZ, VAR_ARGS, { 0 } },
        { SC_OPCODE_VAR_P, SC_OPCODE_VAR_P_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZEN, VAR_ARGS, { 0 } },
        { SC_OPCODE_ST_DEV, SC_OPCODE_ST_DEV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABW, VAR_ARGS, { 0 } },
        { SC_OPCODE_ST_DEV_P, SC_OPCODE_ST_DEV_P_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABWN, VAR_ARGS, { 0 } },
        { SC_OPCODE_B, SC_OPCODE_B_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_B, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_NORM_DIST, SC_OPCODE_NORM_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMVERT, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_EXP_DIST, SC_OPCODE_EXP_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_EXPONVERT, 3, { 0, 0, 0 } },
        { SC_OPCODE_BINOM_DIST, SC_OPCODE_BINOM_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BINOMVERT, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_POISSON_DIST, SC_OPCODE_POISSON_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_POISSON, 3, { 0, 0, 1 } },
        { SC_OPCODE_COMBIN, SC_OPCODE_COMBIN_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KOMBINATIONEN, 2, { 0, 0 } },
        { SC_OPCODE_COMBIN_A, SC_OPCODE_COMBIN_A_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_KOMBINATIONEN2, 2, { 0, 0 } },
        { SC_OPCODE_PERMUT, SC_OPCODE_PERMUT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIATIONEN, 2, { 0, 0 } },
        { SC_OPCODE_PERMUTATION_A, SC_OPCODE_PERMUTATION_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIATIONEN2, 2, { 0, 0 } },
        { SC_OPCODE_PV, SC_OPCODE_PV_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_BW, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_SYD, SC_OPCODE_SYD_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_DIA, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_DDB, SC_OPCODE_DDB_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_GDA, 5, { 0, 0, 0, 0, 1 } },
        { SC_OPCODE_DB, SC_OPCODE_DB_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_GDA2, 5, { 0, 0, 0, 0, 1 } },
        { SC_OPCODE_VBD , SC_OPCODE_VBD_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_VDB, 7, { 0, 0, 0, 0, 0, 1, 1 } },
        { SC_OPCODE_PDURATION, SC_OPCODE_PDURATION_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_LAUFZEIT, 3, { 0, 0, 0 } },
        { SC_OPCODE_SLN, SC_OPCODE_SLN_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_LIA, 3, { 0, 0, 0 } },
        { SC_OPCODE_PMT, SC_OPCODE_PMT_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_RMZ, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_COLUMNS, SC_OPCODE_COLUMNS_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SPALTEN, 1, { 0 } },
        { SC_OPCODE_ROWS, SC_OPCODE_ROWS_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ZEILEN, 1, { 0 } },
        { SC_OPCODE_COLUMN, SC_OPCODE_COLUMN_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SPALTE, 1, { 1 } },
        { SC_OPCODE_ROW, SC_OPCODE_ROW_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ZEILE, 1, { 1 } },
        { SC_OPCODE_RRI, SC_OPCODE_RRI_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_ZGZ, 3, { 0, 0, 0 } },
        { SC_OPCODE_FV, SC_OPCODE_FV_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_ZW, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_NPER, SC_OPCODE_NPER_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_ZZR, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_RATE, SC_OPCODE_RATE_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_ZINS, 6, { 0, 0, 0, 1, 1, 1 } },
        { SC_OPCODE_IPMT, SC_OPCODE_IPMT_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_ZINSZ, 6, { 0, 0, 0, 0, 1, 1 } },
        { SC_OPCODE_PPMT, SC_OPCODE_PPMT_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_KAPZ, 6, { 0, 0, 0, 0, 1, 1 } },
        { SC_OPCODE_CUM_IPMT, SC_OPCODE_CUM_IPMT_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_KUMZINSZ, 6, { 0, 0, 0, 0, 0, 0 } },
        { SC_OPCODE_CUM_PRINC, SC_OPCODE_CUM_PRINC_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_KUMKAPITAL, 6, { 0, 0, 0, 0, 0, 0 } },
        { SC_OPCODE_EFFECT, SC_OPCODE_EFFECT_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_EFFEKTIV, 2, { 0, 0 } },
        { SC_OPCODE_NOMINAL, SC_OPCODE_NOMINAL_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_NOMINAL, 2, { 0, 0 } },
        { SC_OPCODE_SUB_TOTAL, SC_OPCODE_SUB_TOTAL_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_TEILERGEBNIS, 2, { 0, 0 } },
        { SC_OPCODE_DB_SUM, SC_OPCODE_DB_SUM_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBSUMME, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_COUNT, SC_OPCODE_DB_COUNT_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBANZAHL, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_COUNT_2, SC_OPCODE_DB_COUNT_2_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBANZAHL2, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_AVERAGE, SC_OPCODE_DB_AVERAGE_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBMITTELWERT, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_GET, SC_OPCODE_DB_GET_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBAUSZUG, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_MAX, SC_OPCODE_DB_MAX_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBMAX, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_MIN, SC_OPCODE_DB_MIN_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBMIN, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_PRODUCT, SC_OPCODE_DB_PRODUCT_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBPRODUKT, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_STD_DEV, SC_OPCODE_DB_STD_DEV_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBSTDABW, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_STD_DEV_P, SC_OPCODE_DB_STD_DEV_P_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBSTDABWN, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_VAR, SC_OPCODE_DB_VAR_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBVARIANZ, 3, { 0, 0, 0 } },
        { SC_OPCODE_DB_VAR_P, SC_OPCODE_DB_VAR_P_ARY, 0, ID_FUNCTION_GRP_DATABASE, HID_FUNC_DBVARIANZEN, 3, { 0, 0, 0 } },
        { SC_OPCODE_INDIRECT, SC_OPCODE_INDIRECT_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_INDIREKT, 2, { 0, 1 } },
        { SC_OPCODE_ADDRESS, SC_OPCODE_ADDRESS_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_ADRESSE, 5, { 0, 0, 1, 1, 1 } },
        { SC_OPCODE_MATCH, SC_OPCODE_MATCH_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VERGLEICH, 3, { 0, 0, 1 } },
        { SC_OPCODE_COUNT_EMPTY_CELLS, SC_OPCODE_COUNT_EMPTY_CELLS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ANZAHLLEEREZELLEN, 1, { 0 } },
        { SC_OPCODE_COUNT_IF, SC_OPCODE_COUNT_IF_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ZAEHLENWENN, 2, { 0, 0 } },
        { SC_OPCODE_SUM_IF, SC_OPCODE_SUM_IF_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SUMMEWENN, 3, { 0, 0, 1 } },
        { SC_OPCODE_LOOKUP, SC_OPCODE_LOOKUP_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VERWEIS, 3, { 0, 0, 1 } },
        { SC_OPCODE_V_LOOKUP, SC_OPCODE_V_LOOKUP_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_SVERWEIS, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_H_LOOKUP, SC_OPCODE_H_LOOKUP_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_WVERWEIS, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_OFFSET, SC_OPCODE_OFFSET_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VERSCHIEBUNG, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_INDEX, SC_OPCODE_INDEX_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_INDEX, 4, { 0, 1, 1, 1 } },
        { SC_OPCODE_AREAS, SC_OPCODE_AREAS_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_BEREICHE, 1, { 0 } },
        { SC_OPCODE_CURRENCY, SC_OPCODE_CURRENCY_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_DM, 2, { 0, 1 } },
        { SC_OPCODE_REPLACE, SC_OPCODE_REPLACE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ERSETZEN, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_FIXED, SC_OPCODE_FIXED_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FEST, 3, { 0, 0, 1 } },
        { SC_OPCODE_FIND, SC_OPCODE_FIND_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FINDEN, 3, { 0, 0, 1 } },
        { SC_OPCODE_EXACT, SC_OPCODE_EXACT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_IDENTISCH, 2, { 0, 0 } },
        { SC_OPCODE_LEFT, SC_OPCODE_LEFT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LINKS, 2, { 0, 1 } },
        { SC_OPCODE_RIGHT, SC_OPCODE_RIGHT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_RECHTS, 2, { 0, 1 } },
        { SC_OPCODE_SEARCH, SC_OPCODE_SEARCH_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_SUCHEN, 3, { 0, 0, 1 } },
        { SC_OPCODE_MID, SC_OPCODE_MID_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEIL, 3, { 0, 0, 0 } },
        { SC_OPCODE_TEXT, SC_OPCODE_TEXT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEXT, 2, { 0, 0 } },
        { SC_OPCODE_SUBSTITUTE, SC_OPCODE_SUBSTITUTE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WECHSELN, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_REPT, SC_OPCODE_REPT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WIEDERHOLEN, 2, { 0, 0 } },
        { SC_OPCODE_CONCAT, SC_OPCODE_CONCAT_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_VERKETTEN, VAR_ARGS, { 0 } },
        { SC_OPCODE_MAT_DET, SC_OPCODE_MAT_DET_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MDET, 1, { 0 } },
        { SC_OPCODE_MAT_INV, SC_OPCODE_MAT_INV_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MINV, 1, { 0 } },
        { SC_OPCODE_MAT_MULT, SC_OPCODE_MAT_MULT_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MMULT, 2, { 0, 0 } },
        { SC_OPCODE_MAT_TRANS, SC_OPCODE_MAT_TRANS_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_MTRANS, 1, { 0 } },
        { SC_OPCODE_MATRIX_UNIT, SC_OPCODE_MATRIX_UNIT_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_EINHEITSMATRIX, 1, { 0 } },
        { SC_OPCODE_HYP_GEOM_DIST, SC_OPCODE_HYP_GEOM_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_HYPGEOMVERT, 5, { 0, 0, 0, 0, 1 } },
        { SC_OPCODE_LOG_NORM_DIST, SC_OPCODE_LOG_NORM_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGNORMVERT, 4, { 0, 1, 1, 1 } },
        { SC_OPCODE_T_DIST, SC_OPCODE_T_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TVERT, 3, { 0, 0, 0 } },
        { SC_OPCODE_F_DIST, SC_OPCODE_F_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FVERT, 3, { 0, 0, 0 } },
        { SC_OPCODE_CHI_DIST, SC_OPCODE_CHI_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIVERT, 2, { 0, 0 } },
        { SC_OPCODE_WEIBULL, SC_OPCODE_WEIBULL_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_WEIBULL, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_NEG_BINOM_VERT, SC_OPCODE_NEG_BINOM_VERT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NEGBINOMVERT, 3, { 0, 0, 0 } },
        { SC_OPCODE_CRIT_BINOM, SC_OPCODE_CRIT_BINOM_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KRITBINOM, 3, { 0, 0, 0 } },
        { SC_OPCODE_KURT, SC_OPCODE_KURT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KURT, VAR_ARGS, { 0 } },
        { SC_OPCODE_HAR_MEAN, SC_OPCODE_HAR_MEAN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_HARMITTEL, VAR_ARGS, { 0 } },
        { SC_OPCODE_GEO_MEAN, SC_OPCODE_GEO_MEAN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GEOMITTEL, VAR_ARGS, { 0 } },
        { SC_OPCODE_STANDARD, SC_OPCODE_STANDARD_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STANDARDISIERUNG, 3, { 0, 0, 0 } },
        { SC_OPCODE_AVE_DEV, SC_OPCODE_AVE_DEV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MITTELABW, VAR_ARGS, { 0 } },
        { SC_OPCODE_SKEW, SC_OPCODE_SKEW_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SCHIEFE, VAR_ARGS, { 0 } },
        { SC_OPCODE_DEV_SQ, SC_OPCODE_DEV_SQ_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SUMQUADABW, VAR_ARGS, { 0 } },
        { SC_OPCODE_MEDIAN, SC_OPCODE_MEDIAN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MEDIAN, VAR_ARGS, { 0 } },
        { SC_OPCODE_MODAL_VALUE, SC_OPCODE_MODAL_VALUE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MODALWERT, VAR_ARGS, { 0 } },
        { SC_OPCODE_Z_TEST, SC_OPCODE_Z_TEST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GTEST, 3, { 0, 0, 1 } },
        { SC_OPCODE_T_TEST, SC_OPCODE_T_TEST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TTEST, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_RANK, SC_OPCODE_RANK_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_RANG, 3, { 0, 0, 1 } },
        { SC_OPCODE_PERCENTILE, SC_OPCODE_PERCENTILE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUANTIL, 2, { 0, 0 } },
        { SC_OPCODE_PERCENT_RANK, SC_OPCODE_PERCENT_RANK_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUANTILSRANG, 3, { 0, 0, 1 } },
        { SC_OPCODE_LARGE, SC_OPCODE_LARGE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KGROESSTE, 2, { 0, 0 } },
        { SC_OPCODE_SMALL, SC_OPCODE_SMALL_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KKLEINSTE, 2, { 0, 0 } },
        { SC_OPCODE_FREQUENCY, SC_OPCODE_FREQUENCY_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_HAEUFIGKEIT, 2, { 0, 0 } },
        { SC_OPCODE_QUARTILE, SC_OPCODE_QUARTILE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUARTILE, 2, { 0, 0 } },
        { SC_OPCODE_NORM_INV, SC_OPCODE_NORM_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMINV, 3, { 0, 0, 0 } },
        { SC_OPCODE_CONFIDENCE, SC_OPCODE_CONFIDENCE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KONFIDENZ, 3, { 0, 0, 0 } },
        { SC_OPCODE_F_TEST, SC_OPCODE_F_TEST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FTEST, 2, { 0, 0 } },
        { SC_OPCODE_TRIM_MEAN, SC_OPCODE_TRIM_MEAN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GESTUTZTMITTEL, 2, { 0, 0 } },
        { SC_OPCODE_PROB, SC_OPCODE_PROB_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_WAHRSCHBEREICH, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_CORREL, SC_OPCODE_CORREL_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KORREL, 2, { 0, 0 } },
        { SC_OPCODE_COVAR, SC_OPCODE_COVAR_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_KOVAR, 2, { 0, 0 } },
        { SC_OPCODE_PEARSON, SC_OPCODE_PEARSON_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PEARSON, 2, { 0, 0 } },
        { SC_OPCODE_RSQ, SC_OPCODE_RSQ_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BESTIMMTHEITSMASS, 2, { 0, 0 } },
        { SC_OPCODE_STEYX, SC_OPCODE_STEYX_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STFEHLERYX, 2, { 0, 0 } },
        { SC_OPCODE_SLOPE, SC_OPCODE_SLOPE_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STEIGUNG, 2, { 0, 0 } },
        { SC_OPCODE_INTERCEPT, SC_OPCODE_INTERCEPT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ACHSENABSCHNITT, 2, { 0, 0 } },
        { SC_OPCODE_TREND, SC_OPCODE_TREND_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_TREND, 4, { 0, 1, 1, 1 } },
        { SC_OPCODE_GROWTH, SC_OPCODE_GROWTH_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_VARIATION, 4, { 0, 1, 1, 1 } },
        { SC_OPCODE_LINEST, SC_OPCODE_LINEST_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_RGP, 4, { 0, 1, 1, 1 } },
        { SC_OPCODE_LOGEST, SC_OPCODE_LOGEST_ARY, 0, ID_FUNCTION_GRP_MATRIX, HID_FUNC_RKP, 4, { 0, 1, 1, 1 } },
        { SC_OPCODE_FORECAST, SC_OPCODE_FORECAST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SCHAETZER, 3, { 0, 0, 0 } },
        { SC_OPCODE_CHI_INV, SC_OPCODE_CHI_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIINV, 2, { 0, 0 } },
        { SC_OPCODE_GAMMA_DIST, SC_OPCODE_GAMMA_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMAVERT, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_GAMMA_INV, SC_OPCODE_GAMMA_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMAINV, 3, { 0, 0, 0 } },
        { SC_OPCODE_T_INV, SC_OPCODE_T_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TINV, 2, { 0, 0 } },
        { SC_OPCODE_F_INV, SC_OPCODE_F_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FINV, 3, { 0, 0, 0 } },
        { SC_OPCODE_CHI_TEST, SC_OPCODE_CHI_TEST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHITEST, 2, { 0, 0 } },
        { SC_OPCODE_LOG_INV, SC_OPCODE_LOG_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGINV, 3, { 0, 1, 1 } },
        { SC_OPCODE_BETA_DIST, SC_OPCODE_BETA_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETAVERT, 6, { 0, 0, 0, 1, 1, 1 } },
        { SC_OPCODE_BETA_INV, SC_OPCODE_BETA_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETAINV, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_WEEK, SC_OPCODE_WEEK_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_KALENDERWOCHE, 2, { 0, 1 } },
        { SC_OPCODE_GET_DAY_OF_WEEK, SC_OPCODE_GET_DAY_OF_WEEK_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_WOCHENTAG, 2, { 0, 1 } },
        { SC_OPCODE_STYLE, SC_OPCODE_STYLE_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_VORLAGE, 3, { 0, 1, 1 } },
        { SC_OPCODE_DDE, SC_OPCODE_DDE_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_DDE, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_BASE, SC_OPCODE_BASE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_BASIS, 3, { 0, 0, 1 } },
        { SC_OPCODE_SHEET, SC_OPCODE_SHEET_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TABELLE, 1, { 1 } },
        { SC_OPCODE_SHEETS, SC_OPCODE_SHEETS_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_TABELLEN, 1, { 1 } },
        { SC_OPCODE_MIN_A, SC_OPCODE_MIN_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MINA, VAR_ARGS, { 0 } },
        { SC_OPCODE_MAX_A, SC_OPCODE_MAX_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MAXA, VAR_ARGS, { 0 } },
        { SC_OPCODE_AVERAGE_A, SC_OPCODE_AVERAGE_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MITTELWERTA, VAR_ARGS, { 0 } },
        { SC_OPCODE_ST_DEV_A, SC_OPCODE_ST_DEV_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABWA, VAR_ARGS, { 0 } },
        { SC_OPCODE_ST_DEV_P_A, SC_OPCODE_ST_DEV_P_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STABWNA, VAR_ARGS, { 0 } },
        { SC_OPCODE_VAR_A, SC_OPCODE_VAR_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZA, VAR_ARGS, { 0 } },
        { SC_OPCODE_VAR_P_A, SC_OPCODE_VAR_P_A_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VARIANZENA, VAR_ARGS, { 0 } },
        { SC_OPCODE_EASTERSUNDAY, SC_OPCODE_EASTERSUNDAY_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_OSTERSONNTAG, 1, { 0 } },
        { SC_OPCODE_DECIMAL, SC_OPCODE_DECIMAL_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_DEZIMAL, 2, { 0, 0 } },
        { SC_OPCODE_CONVERT_OOO, SC_OPCODE_CONVERT_OOO_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_UMRECHNEN, 3, { 0, 0, 0 } },
        { SC_OPCODE_ROMAN, SC_OPCODE_ROMAN_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_ROEMISCH, 2, { 0, 1 } },
        { SC_OPCODE_MIRR, SC_OPCODE_MIRR_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_QIKV, 3, { 0, 0, 0 } },
        { SC_OPCODE_CELL, SC_OPCODE_CELL_ARY, 0, ID_FUNCTION_GRP_INFO, HID_FUNC_ZELLE, 2, { 0, 1 } },
        { SC_OPCODE_ISPMT, SC_OPCODE_ISPMT_ARY, 0, ID_FUNCTION_GRP_FINANZ, HID_FUNC_ISPMT, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_HYPERLINK, SC_OPCODE_HYPERLINK_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_HYPERLINK, 2, { 0, 1 } },
        { SC_OPCODE_GET_PIVOT_DATA, SC_OPCODE_GET_PIVOT_DATA_ARY, 0, ID_FUNCTION_GRP_TABLE, HID_FUNC_GETPIVOTDATA, VAR_ARGS+2, { 0, 0, 1 } },
        { SC_OPCODE_EUROCONVERT, SC_OPCODE_EUROCONVERT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_EUROCONVERT, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_NUMBERVALUE, SC_OPCODE_NUMBERVALUE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_NUMBERVALUE, 3, { 0, 1, 1 } },
        { SC_OPCODE_CHISQ_DIST, SC_OPCODE_CHISQ_DIST_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQDIST, 3, { 0, 0, 1 } },
        { SC_OPCODE_CHISQ_INV, SC_OPCODE_CHISQ_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQINV, 2, { 0, 0 } },
        { SC_OPCODE_BITAND, SC_OPCODE_BITAND_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITAND, 2, { 0, 0 } },
        { SC_OPCODE_BITOR, SC_OPCODE_BITOR_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITOR, 2, { 0, 0 } },
        { SC_OPCODE_BITXOR, SC_OPCODE_BITXOR_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITXOR, 2, { 0, 0 } },
        { SC_OPCODE_BITRSHIFT, SC_OPCODE_BITRSHIFT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITRSHIFT, 2, { 0, 0 } },
        { SC_OPCODE_BITLSHIFT, SC_OPCODE_BITLSHIFT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_BITLSHIFT, 2, { 0, 0 } },
        { SC_OPCODE_GET_DATEDIF, SC_OPCODE_GET_DATEDIF_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_DATEDIF, 3, { 0, 0, 0 } },
        { SC_OPCODE_XOR, SC_OPCODE_XOR_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_XOR, VAR_ARGS, { 0 } },
        { SC_OPCODE_AVERAGE_IF, SC_OPCODE_AVERAGE_IF_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_AVERAGEIF, 3, { 0, 0, 1 } },
        { SC_OPCODE_SUM_IFS, SC_OPCODE_SUM_IFS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_SUMIFS, PAIRED_VAR_ARGS+1, { 0, 0, 0 } },
        { SC_OPCODE_AVERAGE_IFS, SC_OPCODE_AVERAGE_IFS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_AVERAGEIFS, PAIRED_VAR_ARGS+1, { 0, 0, 0 } },
        { SC_OPCODE_COUNT_IFS, SC_OPCODE_COUNT_IFS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_COUNTIFS, PAIRED_VAR_ARGS, { 0, 0 } },
        { SC_OPCODE_SKEWP, SC_OPCODE_SKEWP_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_SKEWP, VAR_ARGS, { 0 } },
        { SC_OPCODE_LENB, SC_OPCODE_LENB_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LENB, 1, { 0 } },
        { SC_OPCODE_RIGHTB, SC_OPCODE_RIGHTB_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_RIGHTB, 2, { 0, 1 } },
        { SC_OPCODE_LEFTB, SC_OPCODE_LEFTB_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_LEFTB, 2, { 0, 1 } },
        { SC_OPCODE_MIDB, SC_OPCODE_MIDB_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_MIDB, 3, { 0, 0, 0 } },
        { SC_OPCODE_FILTERXML, SC_OPCODE_FILTERXML_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_FILTERXML, 2, { 0, 0 } },
        { SC_OPCODE_WEBSERVICE, SC_OPCODE_WEBSERVICE_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_WEBSERVICE, 1, { 0, 0 } },
        { SC_OPCODE_COVARIANCE_S, SC_OPCODE_COVARIANCE_S_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_COVARIANCE_S, 2, { 0, 0 } },
        { SC_OPCODE_COVARIANCE_P, SC_OPCODE_COVARIANCE_P_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_COVARIANCE_P, 2, { 0, 0 } },
        { SC_OPCODE_ST_DEV_P_MS, SC_OPCODE_ST_DEV_P_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ST_DEV_P_MS, VAR_ARGS, { 0 } },
        { SC_OPCODE_ST_DEV_S, SC_OPCODE_ST_DEV_S_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_ST_DEV_S, VAR_ARGS, { 0 } },
        { SC_OPCODE_VAR_P_MS, SC_OPCODE_VAR_P_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VAR_P_MS, VAR_ARGS, { 0 } },
        { SC_OPCODE_VAR_S, SC_OPCODE_VAR_S_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_VAR_S, VAR_ARGS, { 0 } },
        { SC_OPCODE_BETA_DIST_MS, SC_OPCODE_BETA_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETADIST_MS, 6, { 0, 0, 0, 0, 1, 1 } },
        { SC_OPCODE_BETA_INV_MS, SC_OPCODE_BETA_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BETAINV_MS, 5, { 0, 0, 0, 1, 1 } },
        { SC_OPCODE_BINOM_DIST_MS, SC_OPCODE_BINOM_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BINOM_DIST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_BINOM_INV, SC_OPCODE_BINOM_INV_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_BINOM_INV_MS, 3, { 0, 0, 0 } },
        { SC_OPCODE_CHI_DIST_MS, SC_OPCODE_CHI_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIVERT_MS, 2, { 0, 0 } },
        { SC_OPCODE_CHI_INV_MS, SC_OPCODE_CHI_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHIINV_MS, 2, { 0, 0 } },
        { SC_OPCODE_CHI_TEST_MS, SC_OPCODE_CHI_TEST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHITEST_MS, 2, { 0, 0 } },
        { SC_OPCODE_CHISQ_DIST_MS, SC_OPCODE_CHISQ_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQDIST_MS, 3, { 0, 0, 0 } },
        { SC_OPCODE_CHISQ_INV_MS, SC_OPCODE_CHISQ_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CHISQINV_MS, 2, { 0, 0 } },
        { SC_OPCODE_CONFIDENCE_N, SC_OPCODE_CONFIDENCE_N_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CONFIDENCE_N, 3, { 0, 0, 0 } },
        { SC_OPCODE_CONFIDENCE_T, SC_OPCODE_CONFIDENCE_T_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_CONFIDENCE_T, 3, { 0, 0, 0 } },
        { SC_OPCODE_F_DIST_LT, SC_OPCODE_F_DIST_LT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_DIST_LT, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_F_DIST_RT, SC_OPCODE_F_DIST_RT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_DIST_RT, 3, { 0, 0, 0 } },
        { SC_OPCODE_F_INV_LT, SC_OPCODE_F_INV_LT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_INV_LT, 3, { 0, 0, 0 } },
        { SC_OPCODE_F_INV_RT, SC_OPCODE_F_INV_RT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_INV_RT, 3, { 0, 0, 0 } },
        { SC_OPCODE_F_TEST_MS, SC_OPCODE_F_TEST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_F_TEST_MS, 2, { 0, 0 } },
        { SC_OPCODE_EXP_DIST_MS, SC_OPCODE_EXP_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_EXP_DIST_MS, 3, { 0, 0, 0 } },
        { SC_OPCODE_HYP_GEOM_DIST_MS, SC_OPCODE_HYP_GEOM_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_HYP_GEOM_DIST_MS, 5, { 0, 0, 0, 0, 0 } },
        { SC_OPCODE_POISSON_DIST_MS, SC_OPCODE_POISSON_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_POISSON_DIST_MS, 3, { 0, 0, 1 } },
        { SC_OPCODE_WEIBULL_MS, SC_OPCODE_WEIBULL_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_WEIBULL_DIST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_GAMMA_DIST_MS, SC_OPCODE_GAMMA_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMADIST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_GAMMA_INV_MS, SC_OPCODE_GAMMA_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_GAMMAINV_MS, 3, { 0, 0, 0 } },
        { SC_OPCODE_LOG_NORM_DIST_MS, SC_OPCODE_LOG_NORM_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGNORMDIST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_LOG_INV_MS, SC_OPCODE_LOG_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_LOGINV, 3, { 0, 0, 0 } },
        { SC_OPCODE_NORM_DIST_MS, SC_OPCODE_NORM_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMDIST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_NORM_INV_MS, SC_OPCODE_NORM_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NORMINV_MS, 3, { 0, 0, 0 } },
        { SC_OPCODE_STD_NORM_DIST_MS, SC_OPCODE_STD_NORM_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STD_NORMDIST_MS, 2, { 0, 0 } },
        { SC_OPCODE_S_NORM_INV_MS, SC_OPCODE_S_NORM_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_STD_NORMINV_MS, 1, { 0 } },
        { SC_OPCODE_T_DIST_MS, SC_OPCODE_T_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TDIST_MS, 3, { 0, 0, 0 } },
        { SC_OPCODE_T_DIST_RT, SC_OPCODE_T_DIST_RT_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TDIST_RT, 2, { 0, 0 } },
        { SC_OPCODE_T_DIST_2T, SC_OPCODE_T_DIST_2T_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TDIST_2T, 2, { 0, 0 } },
        { SC_OPCODE_T_INV_2T, SC_OPCODE_T_INV_2T_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TINV_2T, 2, { 0, 0 } },
        { SC_OPCODE_T_INV_MS, SC_OPCODE_T_INV_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TINV_MS, 2, { 0, 0 } },
        { SC_OPCODE_T_TEST_MS, SC_OPCODE_T_TEST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_TTEST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_PERCENTILE_INC, SC_OPCODE_PERCENTILE_INC_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTILE_INC, 2, { 0, 0 } },
        { SC_OPCODE_PERCENT_RANK_INC, SC_OPCODE_PERCENT_RANK_INC_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTRANK_INC, 3, { 0, 0, 1 } },
        { SC_OPCODE_QUARTILE_INC, SC_OPCODE_QUARTILE_INC_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUARTILE_INC, 2, { 0, 0 } },
        { SC_OPCODE_RANK_EQ, SC_OPCODE_RANK_EQ_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_RANK_EQ, 3, { 0, 0, 1 } },
        { SC_OPCODE_PERCENTILE_EXC, SC_OPCODE_PERCENTILE_EXC_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTILE_EXC, 2, { 0, 0 } },
        { SC_OPCODE_PERCENT_RANK_EXC, SC_OPCODE_PERCENT_RANK_EXC_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_PERCENTRANK_EXC, 3, { 0, 0, 1 } },
        { SC_OPCODE_QUARTILE_EXC, SC_OPCODE_QUARTILE_EXC_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_QUARTILE_EXC, 2, { 0, 0 } },
        { SC_OPCODE_RANK_AVG, SC_OPCODE_RANK_AVG_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_RANK_AVG, 3, { 0, 0, 1 } },
        { SC_OPCODE_MODAL_VALUE_MS, SC_OPCODE_MODAL_VALUE_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MODAL_VALUE_MS, VAR_ARGS, { 0 } },
        { SC_OPCODE_MODAL_VALUE_MULTI, SC_OPCODE_MODAL_VALUE_MULTI_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MODAL_VALUE_MULTI, VAR_ARGS, { 0 } },
        { SC_OPCODE_NEG_BINOM_DIST_MS, SC_OPCODE_NEG_BINOM_DIST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_NEGBINOMDIST_MS, 4, { 0, 0, 0, 0 } },
        { SC_OPCODE_Z_TEST_MS, SC_OPCODE_Z_TEST_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_Z_TEST_MS, 3, { 0, 0, 1 } },
        { SC_OPCODE_CEIL_MS, SC_OPCODE_CEIL_MS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_MS, 2, { 0, 0 } },
        { SC_OPCODE_CEIL_ISO, SC_OPCODE_CEIL_ISO_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_ISO, 2, { 0, 1 } },
        { SC_OPCODE_FLOOR_MS, SC_OPCODE_FLOOR_MS_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FLOOR_MS, 2, { 0, 0 } },
        { SC_OPCODE_NETWORKDAYS_MS, SC_OPCODE_NETWORKDAYS_MS_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_NETWORKDAYS_MS, 4, { 0, 0, 1, 1 } },
        { SC_OPCODE_WORKDAY_MS, SC_OPCODE_WORKDAY_MS_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_WORKDAY_MS, 4, { 0, 0, 1, 1 } },
        { SC_OPCODE_AGGREGATE, SC_OPCODE_AGGREGATE_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_AGGREGATE, VAR_ARGS+3, { 0, 0, 0, 1 } },
        { SC_OPCODE_COLOR, SC_OPCODE_COLOR_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_COLOR, 4, { 0, 0, 0, 1 } },
        { SC_OPCODE_CEIL_MATH, SC_OPCODE_CEIL_MATH_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_MATH, 3, { 0, 1, 1 } },
        { SC_OPCODE_CEIL_PRECISE, SC_OPCODE_CEIL_PRECISE_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_CEIL_PRECISE, 2, { 0, 1 } },
        { SC_OPCODE_NETWORKDAYS, SC_OPCODE_NETWORKDAYS_ARY, 0, ID_FUNCTION_GRP_DATETIME, HID_FUNC_NETWORKDAYS, 4, { 0, 0, 1, 1 } },
        { SC_OPCODE_FLOOR_MATH, SC_OPCODE_FLOOR_MATH_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FLOOR_MATH, 3, { 0, 1, 1 } },
        { SC_OPCODE_FLOOR_PRECISE, SC_OPCODE_FLOOR_PRECISE_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_FLOOR_PRECISE, 2, { 0, 1 } },
        { SC_OPCODE_RAWSUBTRACT, SC_OPCODE_RAWSUBTRACT_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_RAWSUBTRACT, VAR_ARGS+2, { 0, 0, 1 } },
        { SC_OPCODE_WEEKNUM_OOO, SC_OPCODE_WEEKNUM_OOO_ARY, 2, ID_FUNCTION_GRP_DATETIME, HID_FUNC_WEEKNUM_OOO, 2, { 0, 0 } },
        { SC_OPCODE_FORECAST_ETS_ADD, SC_OPCODE_FORECAST_ETS_ADD_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_ADD, 6, { 0, 0, 0, 1, 1, 1 } },
        { SC_OPCODE_FORECAST_ETS_SEA, SC_OPCODE_FORECAST_ETS_SEA_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_SEA, 4, { 0, 0, 1, 1 } },
        { SC_OPCODE_FORECAST_ETS_MUL, SC_OPCODE_FORECAST_ETS_MUL_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_MUL, 6, { 0, 0, 0, 1, 1, 1 } },
        { SC_OPCODE_FORECAST_ETS_PIA, SC_OPCODE_FORECAST_ETS_PIA_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_PIA, 7, { 0, 0, 0, 1, 1, 1, 1 } },
        { SC_OPCODE_FORECAST_ETS_PIM, SC_OPCODE_FORECAST_ETS_PIM_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_PIM, 7, { 0, 0, 0, 1, 1, 1, 1 } },
        { SC_OPCODE_FORECAST_ETS_STA, SC_OPCODE_FORECAST_ETS_STA_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_STA, 6, { 0, 0, 0, 1, 1, 1 } },
        { SC_OPCODE_FORECAST_ETS_STM, SC_OPCODE_FORECAST_ETS_STM_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_ETS_STM, 6, { 0, 0, 0, 1, 1, 1 } },
        { SC_OPCODE_FORECAST_LIN, SC_OPCODE_FORECAST_LIN_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_FORECAST_LIN, 3, { 0, 0, 0 } },
        { SC_OPCODE_CONCAT_MS, SC_OPCODE_CONCAT_MS_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_CONCAT_MS, VAR_ARGS, { 0 } },
        { SC_OPCODE_TEXTJOIN_MS, SC_OPCODE_TEXTJOIN_MS_ARY, 0, ID_FUNCTION_GRP_TEXT, HID_FUNC_TEXTJOIN_MS, VAR_ARGS + 2, { 0, 0, 0 } },
        { SC_OPCODE_IFS_MS, SC_OPCODE_IFS_MS_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_IFS_MS, PAIRED_VAR_ARGS, { 0, 0 } },
        { SC_OPCODE_SWITCH_MS, SC_OPCODE_SWITCH_MS_ARY, 0, ID_FUNCTION_GRP_LOGIC, HID_FUNC_SWITCH_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 } },
        { SC_OPCODE_MINIFS_MS, SC_OPCODE_MINIFS_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MINIFS_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 } },
        { SC_OPCODE_MAXIFS_MS, SC_OPCODE_MAXIFS_MS_ARY, 0, ID_FUNCTION_GRP_STATISTIC, HID_FUNC_MAXIFS_MS, PAIRED_VAR_ARGS + 1, { 0, 0, 0 } },
        { SC_OPCODE_ROUNDSIG, SC_OPCODE_ROUNDSIG_ARY, 0, ID_FUNCTION_GRP_MATH, HID_FUNC_ROUNDSIG, 2, { 0, 0 } }
    };

    ScFuncDesc* pDesc = nullptr;
    sal_Int32 nStrLen = 0;
    ::std::list<ScFuncDesc*> tmpFuncList;

    // Browse for all possible OpCodes. This is not the fastest method, but
    // otherwise the sub resources within the resource blocks and the
    // resource blocks themselves would had to be ordered according to
    // OpCodes, which is utopian...
    ScFuncDescCore* pDescsEnd = aDescs + SAL_N_ELEMENTS(aDescs);
    for (sal_uInt16 i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; ++i)
    {
        ScFuncDescCore *pEntry = std::lower_bound(aDescs, pDescsEnd, i,
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

            ScFuncRes aSubRes(*pEntry, pDesc, bSuppressed);
            // Instead of dealing with this exceptional case at 1001 places
            // we simply don't add an entirely suppressed function to the
            // list and delete it.
            if (bSuppressed)
                delete pDesc;
            else
            {
                pDesc->nFIndex = i;
                tmpFuncList.push_back(pDesc);

                nStrLen = (*(pDesc->pFuncName)).getLength();
                if (nStrLen > nMaxFuncNameLen)
                    nMaxFuncNameLen = nStrLen;
            }
        }
    }

    // legacy binary AddIn functions

    sal_uInt16 nNextId = SC_OPCODE_LAST_OPCODE_ID + 1; // FuncID for AddIn functions

    // Interpretation of AddIn list
    OUString aDefArgNameValue   = "value";
    OUString aDefArgNameString  = "string";
    OUString aDefArgNameValues  = "values";
    OUString aDefArgNameStrings = "strings";
    OUString aDefArgNameCells   = "cells";
    OUString aDefArgNameNone    = "none";
    OUString aDefArgDescValue   = "a value";
    OUString aDefArgDescString  = "a string";
    OUString aDefArgDescValues  = "array of values";
    OUString aDefArgDescStrings = "array of strings";
    OUString aDefArgDescCells   = "range of cells";
    OUString aDefArgDescNone    = "none";

    OUString aArgName, aArgDesc;
    const LegacyFuncCollection& rLegacyFuncColl = *ScGlobal::GetLegacyFuncCollection();
    LegacyFuncCollection::const_iterator it = rLegacyFuncColl.begin(), itEnd = rLegacyFuncColl.end();
    for (; it != itEnd; ++it)
    {
        const LegacyFuncData *const pLegacyFuncData = it->second.get();
        pDesc = new ScFuncDesc;
        sal_uInt16 nArgs = pLegacyFuncData->GetParamCount() - 1;
        pLegacyFuncData->getParamDesc( aArgName, aArgDesc, 0 );
        pDesc->nFIndex     = nNextId++; //  ??? OpCode vergeben
        pDesc->nCategory   = ID_FUNCTION_GRP_ADDINS;
        pDesc->pFuncName   = new OUString(pLegacyFuncData->GetInternalName().toAsciiUpperCase());

        OUStringBuffer aBuf(aArgDesc);
        aBuf.append('\n');
        aBuf.append("( AddIn: ");
        aBuf.append(pLegacyFuncData->GetModuleName());
        aBuf.append(" )");
        pDesc->pFuncDesc = new OUString(aBuf.makeStringAndClear());

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

        tmpFuncList.push_back(pDesc);
        nStrLen = (*(pDesc->pFuncName)).getLength();
        if ( nStrLen > nMaxFuncNameLen)
            nMaxFuncNameLen = nStrLen;
    }

    // StarOne AddIns

    ScUnoAddInCollection* pUnoAddIns = ScGlobal::GetAddInCollection();
    long nUnoCount = pUnoAddIns->GetFuncCount();
    for (long nFunc=0; nFunc<nUnoCount; nFunc++)
    {
        pDesc = new ScFuncDesc;
        pDesc->nFIndex = nNextId++;

        if ( pUnoAddIns->FillFunctionDesc( nFunc, *pDesc ) )
        {
            tmpFuncList.push_back(pDesc);
            nStrLen = (*(pDesc->pFuncName)).getLength();
            if (nStrLen > nMaxFuncNameLen)
                nMaxFuncNameLen = nStrLen;
        }
        else
            delete pDesc;
    }

    //Move list to vector for better random access performance
    ::std::vector<const ScFuncDesc*> tmp(tmpFuncList.begin(), tmpFuncList.end());
    tmpFuncList.clear();
    aFunctionList.swap(tmp);

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

// class ScFunctionCategory:

sal_uInt32 ScFunctionCategory::getCount() const
{
    return m_pCategory->size();
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
    if(_nPos < m_pCategory->size())
        pDesc = m_pCategory->at(_nPos);
    return pDesc;
}

sal_uInt32 ScFunctionCategory::getNumber() const
{
    return m_nCategory;
}

// class ScFunctionMgr:

ScFunctionMgr::ScFunctionMgr() :
    pFuncList( ScGlobal::GetStarCalcFunctionList() )
{
    OSL_ENSURE( pFuncList, "Functionlist not found." );
    sal_uInt32 catCount[MAX_FUNCCAT] = {0};

    aCatLists[0] = new ::std::vector<const ScFuncDesc*>;
    aCatLists[0]->reserve(pFuncList->GetCount());

    // Retrieve all functions, store in cumulative ("All") category, and count
    // number of functions in each category
    for(const ScFuncDesc* pDesc = pFuncList->First(); pDesc; pDesc = pFuncList->Next())
    {
        OSL_ENSURE((pDesc->nCategory) < MAX_FUNCCAT, "Unknown category");
        if ((pDesc->nCategory) < MAX_FUNCCAT)
            ++catCount[pDesc->nCategory];
        aCatLists[0]->push_back(pDesc);
    }

    // Sort functions in cumulative category by name
    ::std::sort(aCatLists[0]->begin(), aCatLists[0]->end(), ScFuncDesc::compareByName);

    // Allocate correct amount of space for categories
    for (sal_uInt16 i = 1; i < MAX_FUNCCAT; ++i)
    {
        aCatLists[i] = new ::std::vector<const ScFuncDesc*>;
        aCatLists[i]->reserve(catCount[i]);
    }

    // Fill categories with the corresponding functions (still sorted by name)
    for(::std::vector<const ScFuncDesc*>::iterator iter = aCatLists[0]->begin(); iter!=aCatLists[0]->end(); ++iter)
    {
        if (((*iter)->nCategory) < MAX_FUNCCAT)
            aCatLists[(*iter)->nCategory]->push_back(*iter);
    }

    // Initialize iterators
    pCurCatListIter = aCatLists[0]->end();
    pCurCatListEnd = aCatLists[0]->end();
}

ScFunctionMgr::~ScFunctionMgr()
{
    for (std::vector<const ScFuncDesc*> * pCatList : aCatLists)
        delete pCatList;
}


const ScFuncDesc* ScFunctionMgr::Get( sal_uInt16 nFIndex ) const
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
        pCurCatListIter = aCatLists[nCategory]->begin();
        pCurCatListEnd = aCatLists[nCategory]->end();
        pDesc = *pCurCatListIter;
    }
    else
    {
        pCurCatListIter = aCatLists[0]->end();
        pCurCatListEnd = aCatLists[0]->end();
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
            m_aCategories[nCategory].reset(new ScFunctionCategory(aCatLists[nCategory+1],nCategory)); // aCatLists[0] is "all"
        return m_aCategories[nCategory].get();
    }
    return nullptr;
}

void ScFunctionMgr::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nLRUFuncCount = std::min( rAppOpt.GetLRUFuncListCount(), (sal_uInt16)LRU_MAX );
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
    }
    return 0;
}

// class ScFuncRes:
ScFuncRes::ScFuncRes(const ScFuncDescCore &rEntry, ScFuncDesc* pDesc, bool& rbSuppressed)
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
        pDesc->pDefArgFlags = new ScFuncDesc::ParameterFlags[nArgs];
        for (sal_uInt16 i = 0; i < nArgs; ++i)
        {
            pDesc->pDefArgFlags[i].bOptional = (bool)rEntry.aOptionalArgs[i];
        }
    }

    pDesc->pFuncName = new OUString(ScCompiler::GetNativeSymbol(static_cast<OpCode>(nOpCode)));
    pDesc->pFuncDesc = new OUString(ScResId(rEntry.pResource[0]));

    if (nArgs)
    {
        pDesc->maDefArgNames.clear();
        pDesc->maDefArgNames.resize(nArgs);
        pDesc->maDefArgDescs.clear();
        pDesc->maDefArgDescs.resize(nArgs);
        for (sal_uInt16 i = 0; i < nArgs; ++i)
        {
            pDesc->maDefArgNames[i] = ScResId(rEntry.pResource[(i * 2) + 1]);
            pDesc->maDefArgDescs[i] = ScResId(rEntry.pResource[(i * 2) + 2]);
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
