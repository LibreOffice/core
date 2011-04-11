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

#include "XMLConverter.hxx"
#include <com/sun/star/util/DateTime.hpp>
#include <tools/datetime.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "rangelst.hxx"
#include "rangeutl.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "document.hxx"
#include "ftools.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace xmloff::token;


//___________________________________________________________________

ScDocument* ScXMLConverter::GetScDocument( uno::Reference< frame::XModel > xModel )
{
    if (xModel.is())
    {
        ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
        return pDocObj ? pDocObj->GetDocument() : NULL;
    }
    return NULL;
}


//___________________________________________________________________
sheet::GeneralFunction ScXMLConverter::GetFunctionFromString( const OUString& sFunction )
{
    if( IsXMLToken(sFunction, XML_SUM ) )
        return sheet::GeneralFunction_SUM;
    if( IsXMLToken(sFunction, XML_AUTO ) )
        return sheet::GeneralFunction_AUTO;
    if( IsXMLToken(sFunction, XML_COUNT ) )
        return sheet::GeneralFunction_COUNT;
    if( IsXMLToken(sFunction, XML_COUNTNUMS ) )
        return sheet::GeneralFunction_COUNTNUMS;
    if( IsXMLToken(sFunction, XML_PRODUCT ) )
        return sheet::GeneralFunction_PRODUCT;
    if( IsXMLToken(sFunction, XML_AVERAGE ) )
        return sheet::GeneralFunction_AVERAGE;
    if( IsXMLToken(sFunction, XML_MAX ) )
        return sheet::GeneralFunction_MAX;
    if( IsXMLToken(sFunction, XML_MIN ) )
        return sheet::GeneralFunction_MIN;
    if( IsXMLToken(sFunction, XML_STDEV ) )
        return sheet::GeneralFunction_STDEV;
    if( IsXMLToken(sFunction, XML_STDEVP ) )
        return sheet::GeneralFunction_STDEVP;
    if( IsXMLToken(sFunction, XML_VAR ) )
        return sheet::GeneralFunction_VAR;
    if( IsXMLToken(sFunction, XML_VARP ) )
        return sheet::GeneralFunction_VARP;
    return sheet::GeneralFunction_NONE;
}

ScSubTotalFunc ScXMLConverter::GetSubTotalFuncFromString( const OUString& sFunction )
{
    if( IsXMLToken(sFunction, XML_SUM ) )
        return SUBTOTAL_FUNC_SUM;
    if( IsXMLToken(sFunction, XML_COUNT ) )
        return SUBTOTAL_FUNC_CNT;
    if( IsXMLToken(sFunction, XML_COUNTNUMS ) )
        return SUBTOTAL_FUNC_CNT2;
    if( IsXMLToken(sFunction, XML_PRODUCT ) )
        return SUBTOTAL_FUNC_PROD;
    if( IsXMLToken(sFunction, XML_AVERAGE ) )
        return SUBTOTAL_FUNC_AVE;
    if( IsXMLToken(sFunction, XML_MAX ) )
        return SUBTOTAL_FUNC_MAX;
    if( IsXMLToken(sFunction, XML_MIN ) )
        return SUBTOTAL_FUNC_MIN;
    if( IsXMLToken(sFunction, XML_STDEV ) )
        return SUBTOTAL_FUNC_STD;
    if( IsXMLToken(sFunction, XML_STDEVP ) )
        return SUBTOTAL_FUNC_STDP;
    if( IsXMLToken(sFunction, XML_VAR ) )
        return SUBTOTAL_FUNC_VAR;
    if( IsXMLToken(sFunction, XML_VARP ) )
        return SUBTOTAL_FUNC_VARP;
    return SUBTOTAL_FUNC_NONE;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromFunction(
        OUString& rString,
        const sheet::GeneralFunction eFunction,
        sal_Bool bAppendStr )
{
    OUString sFuncStr;
    switch( eFunction )
    {
        case sheet::GeneralFunction_AUTO:       sFuncStr = GetXMLToken( XML_AUTO );         break;
        case sheet::GeneralFunction_AVERAGE:    sFuncStr = GetXMLToken( XML_AVERAGE );      break;
        case sheet::GeneralFunction_COUNT:      sFuncStr = GetXMLToken( XML_COUNT );        break;
        case sheet::GeneralFunction_COUNTNUMS:  sFuncStr = GetXMLToken( XML_COUNTNUMS );    break;
        case sheet::GeneralFunction_MAX:        sFuncStr = GetXMLToken( XML_MAX );          break;
        case sheet::GeneralFunction_MIN:        sFuncStr = GetXMLToken( XML_MIN );          break;
        case sheet::GeneralFunction_NONE:       sFuncStr = GetXMLToken( XML_NONE );         break;
        case sheet::GeneralFunction_PRODUCT:    sFuncStr = GetXMLToken( XML_PRODUCT );      break;
        case sheet::GeneralFunction_STDEV:      sFuncStr = GetXMLToken( XML_STDEV );        break;
        case sheet::GeneralFunction_STDEVP:     sFuncStr = GetXMLToken( XML_STDEVP );       break;
        case sheet::GeneralFunction_SUM:        sFuncStr = GetXMLToken( XML_SUM );          break;
        case sheet::GeneralFunction_VAR:        sFuncStr = GetXMLToken( XML_VAR );          break;
        case sheet::GeneralFunction_VARP:       sFuncStr = GetXMLToken( XML_VARP );         break;
        default:
        {
            // added to avoid warnings
        }
    }
    ScRangeStringConverter::AssignString( rString, sFuncStr, bAppendStr );
}

void ScXMLConverter::GetStringFromFunction(
        OUString& rString,
        const ScSubTotalFunc eFunction,
        sal_Bool bAppendStr )
{
    OUString sFuncStr;
    switch( eFunction )
    {
        case SUBTOTAL_FUNC_AVE:     sFuncStr = GetXMLToken( XML_AVERAGE );      break;
        case SUBTOTAL_FUNC_CNT:     sFuncStr = GetXMLToken( XML_COUNT );        break;
        case SUBTOTAL_FUNC_CNT2:    sFuncStr = GetXMLToken( XML_COUNTNUMS );    break;
        case SUBTOTAL_FUNC_MAX:     sFuncStr = GetXMLToken( XML_MAX );          break;
        case SUBTOTAL_FUNC_MIN:     sFuncStr = GetXMLToken( XML_MIN );          break;
        case SUBTOTAL_FUNC_NONE:    sFuncStr = GetXMLToken( XML_NONE );         break;
        case SUBTOTAL_FUNC_PROD:    sFuncStr = GetXMLToken( XML_PRODUCT );      break;
        case SUBTOTAL_FUNC_STD:     sFuncStr = GetXMLToken( XML_STDEV );        break;
        case SUBTOTAL_FUNC_STDP:    sFuncStr = GetXMLToken( XML_STDEVP );       break;
        case SUBTOTAL_FUNC_SUM:     sFuncStr = GetXMLToken( XML_SUM );          break;
        case SUBTOTAL_FUNC_VAR:     sFuncStr = GetXMLToken( XML_VAR );          break;
        case SUBTOTAL_FUNC_VARP:    sFuncStr = GetXMLToken( XML_VARP );         break;
    }
    ScRangeStringConverter::AssignString( rString, sFuncStr, bAppendStr );
}


//___________________________________________________________________

sheet::DataPilotFieldOrientation ScXMLConverter::GetOrientationFromString(
    const OUString& rString )
{
    if( IsXMLToken(rString, XML_COLUMN ) )
        return sheet::DataPilotFieldOrientation_COLUMN;
    if( IsXMLToken(rString, XML_ROW ) )
        return sheet::DataPilotFieldOrientation_ROW;
    if( IsXMLToken(rString, XML_PAGE ) )
        return sheet::DataPilotFieldOrientation_PAGE;
    if( IsXMLToken(rString, XML_DATA ) )
        return sheet::DataPilotFieldOrientation_DATA;
    return sheet::DataPilotFieldOrientation_HIDDEN;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromOrientation(
    OUString& rString,
    const sheet::DataPilotFieldOrientation eOrientation,
    sal_Bool bAppendStr )
{
    OUString sOrientStr;
    switch( eOrientation )
    {
        case sheet::DataPilotFieldOrientation_HIDDEN:
            sOrientStr = GetXMLToken( XML_HIDDEN );
        break;
        case sheet::DataPilotFieldOrientation_COLUMN:
            sOrientStr = GetXMLToken( XML_COLUMN );
        break;
        case sheet::DataPilotFieldOrientation_ROW:
            sOrientStr = GetXMLToken( XML_ROW );
        break;
        case sheet::DataPilotFieldOrientation_PAGE:
            sOrientStr = GetXMLToken( XML_PAGE );
        break;
        case sheet::DataPilotFieldOrientation_DATA:
            sOrientStr = GetXMLToken( XML_DATA );
        break;
        default:
        {
            // added to avoid warnings
        }
    }
    ScRangeStringConverter::AssignString( rString, sOrientStr, bAppendStr );
}


//___________________________________________________________________

ScDetectiveObjType ScXMLConverter::GetDetObjTypeFromString( const OUString& rString )
{
    if( IsXMLToken(rString, XML_FROM_SAME_TABLE ) )
        return SC_DETOBJ_ARROW;
    if( IsXMLToken(rString, XML_FROM_ANOTHER_TABLE ) )
        return SC_DETOBJ_FROMOTHERTAB;
    if( IsXMLToken(rString, XML_TO_ANOTHER_TABLE ) )
        return SC_DETOBJ_TOOTHERTAB;
    return SC_DETOBJ_NONE;
}

sal_Bool ScXMLConverter::GetDetOpTypeFromString( ScDetOpType& rDetOpType, const OUString& rString )
{
    if( IsXMLToken(rString, XML_TRACE_DEPENDENTS ) )
        rDetOpType = SCDETOP_ADDSUCC;
    else if( IsXMLToken(rString, XML_TRACE_PRECEDENTS ) )
        rDetOpType = SCDETOP_ADDPRED;
    else if( IsXMLToken(rString, XML_TRACE_ERRORS ) )
        rDetOpType = SCDETOP_ADDERROR;
    else if( IsXMLToken(rString, XML_REMOVE_DEPENDENTS ) )
        rDetOpType = SCDETOP_DELSUCC;
    else if( IsXMLToken(rString, XML_REMOVE_PRECEDENTS ) )
        rDetOpType = SCDETOP_DELPRED;
    else
        return false;
    return sal_True;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromDetObjType(
        OUString& rString,
        const ScDetectiveObjType eObjType,
        sal_Bool bAppendStr )
{
    OUString sTypeStr;
    switch( eObjType )
    {
        case SC_DETOBJ_ARROW:
            sTypeStr = GetXMLToken( XML_FROM_SAME_TABLE );
        break;
        case SC_DETOBJ_FROMOTHERTAB:
            sTypeStr = GetXMLToken( XML_FROM_ANOTHER_TABLE );
        break;
        case SC_DETOBJ_TOOTHERTAB:
            sTypeStr = GetXMLToken( XML_TO_ANOTHER_TABLE );
        break;
        default:
        {
            // added to avoid warnings
        }
    }
    ScRangeStringConverter::AssignString( rString, sTypeStr, bAppendStr );
}

void ScXMLConverter::GetStringFromDetOpType(
        OUString& rString,
        const ScDetOpType eOpType,
        sal_Bool bAppendStr )
{
    OUString sTypeStr;
    switch( eOpType )
    {
        case SCDETOP_ADDSUCC:
            sTypeStr = GetXMLToken( XML_TRACE_DEPENDENTS );
        break;
        case SCDETOP_ADDPRED:
            sTypeStr = GetXMLToken( XML_TRACE_PRECEDENTS );
        break;
        case SCDETOP_ADDERROR:
            sTypeStr = GetXMLToken( XML_TRACE_ERRORS );
        break;
        case SCDETOP_DELSUCC:
            sTypeStr = GetXMLToken( XML_REMOVE_DEPENDENTS );
        break;
        case SCDETOP_DELPRED:
            sTypeStr = GetXMLToken( XML_REMOVE_PRECEDENTS );
        break;
    }
    ScRangeStringConverter::AssignString( rString, sTypeStr, bAppendStr );
}


//___________________________________________________________________

void ScXMLConverter::ParseFormula(OUString& sFormula, const sal_Bool bIsFormula)
{
    OUStringBuffer sBuffer(sFormula.getLength());
    sal_Bool bInQuotationMarks(false);
    sal_Bool bInDoubleQuotationMarks(false);
    sal_Int16 nCountBraces(0);
    sal_Unicode chPrevious('=');
    for (sal_Int32 i = 0; i < sFormula.getLength(); ++i)
    {
        if (sFormula[i] == '\'' && !bInDoubleQuotationMarks &&
            chPrevious != '\\')
            bInQuotationMarks = !bInQuotationMarks;
        else if (sFormula[i] == '"' && !bInQuotationMarks)
            bInDoubleQuotationMarks = !bInDoubleQuotationMarks;
        if (bInQuotationMarks || bInDoubleQuotationMarks)
            sBuffer.append(sFormula[i]);
        else if (sFormula[i] == '[')
            ++nCountBraces;
        else if (sFormula[i] == ']')
            nCountBraces--;
        else if ((sFormula[i] != '.') ||
                ((nCountBraces == 0) && bIsFormula) ||
                !((chPrevious == '[') || (chPrevious == ':') || (chPrevious == ' ') || (chPrevious == '=')))
                sBuffer.append(sFormula[i]);
        chPrevious = sFormula[i];
    }

    DBG_ASSERT(nCountBraces == 0, "there are some braces still open");
    sFormula = sBuffer.makeStringAndClear();
}


//_____________________________________________________________________

void ScXMLConverter::ConvertDateTimeToString(const DateTime& aDateTime, rtl::OUStringBuffer& sDate)
{
    util::DateTime aAPIDateTime;
    ConvertCoreToAPIDateTime(aDateTime, aAPIDateTime);
    SvXMLUnitConverter::convertDateTime(sDate, aAPIDateTime);
}

void ScXMLConverter::ConvertCoreToAPIDateTime(const DateTime& aDateTime, util::DateTime& rDateTime)
{
    rDateTime.Year = aDateTime.GetYear();
    rDateTime.Month = aDateTime.GetMonth();
    rDateTime.Day = aDateTime.GetDay();
    rDateTime.Hours = aDateTime.GetHour();
    rDateTime.Minutes = aDateTime.GetMin();
    rDateTime.Seconds = aDateTime.GetSec();
    rDateTime.HundredthSeconds = aDateTime.Get100Sec();
}

void ScXMLConverter::ConvertAPIToCoreDateTime(const util::DateTime& aDateTime, DateTime& rDateTime)
{
    Date aDate(aDateTime.Day, aDateTime.Month, aDateTime.Year);
    Time aTime(aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.HundredthSeconds);
    DateTime aTempDateTime (aDate, aTime);
    rDateTime = aTempDateTime;
}

// ============================================================================

namespace {

/** Enumerates different types of condition tokens. */
enum ScXMLConditionTokenType
{
    XML_COND_TYPE_KEYWORD,          /// Simple keyword without parentheses, e.g. 'and'.
    XML_COND_TYPE_COMPARISON,       /// Comparison rule, e.g. 'cell-content()<=2'.
    XML_COND_TYPE_FUNCTION0,        /// Function without parameters, e.g. 'cell-content-is-whole-number()'.
    XML_COND_TYPE_FUNCTION1,        /// Function with 1 parameter, e.g. 'is-true-formula(1+1=2)'.
    XML_COND_TYPE_FUNCTION2         /// Function with 2 parameters, e.g. 'cell-content-is-between(1,2)'.
};

struct ScXMLConditionInfo
{
    ScXMLConditionToken meToken;
    ScXMLConditionTokenType meType;
    sheet::ValidationType meValidation;
    sheet::ConditionOperator meOperator;
    const sal_Char*     mpcIdentifier;
    sal_Int32           mnIdentLength;
};

static const ScXMLConditionInfo spConditionInfos[] =
{
    { XML_COND_AND,                     XML_COND_TYPE_KEYWORD,    sheet::ValidationType_ANY,      sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "and" ) },
    { XML_COND_CELLCONTENT,             XML_COND_TYPE_COMPARISON, sheet::ValidationType_ANY,      sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "cell-content" ) },
    { XML_COND_ISBETWEEN,               XML_COND_TYPE_FUNCTION2,  sheet::ValidationType_ANY,      sheet::ConditionOperator_BETWEEN,     RTL_CONSTASCII_STRINGPARAM( "cell-content-is-between" ) },
    { XML_COND_ISNOTBETWEEN,            XML_COND_TYPE_FUNCTION2,  sheet::ValidationType_ANY,      sheet::ConditionOperator_NOT_BETWEEN, RTL_CONSTASCII_STRINGPARAM( "cell-content-is-not-between" ) },
    { XML_COND_ISWHOLENUMBER,           XML_COND_TYPE_FUNCTION0,  sheet::ValidationType_WHOLE,    sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "cell-content-is-whole-number" ) },
    { XML_COND_ISDECIMALNUMBER,         XML_COND_TYPE_FUNCTION0,  sheet::ValidationType_DECIMAL,  sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "cell-content-is-decimal-number" ) },
    { XML_COND_ISDATE,                  XML_COND_TYPE_FUNCTION0,  sheet::ValidationType_DATE,     sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "cell-content-is-date" ) },
    { XML_COND_ISTIME,                  XML_COND_TYPE_FUNCTION0,  sheet::ValidationType_TIME,     sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "cell-content-is-time" ) },
    { XML_COND_ISINLIST,                XML_COND_TYPE_FUNCTION1,  sheet::ValidationType_LIST,     sheet::ConditionOperator_EQUAL,       RTL_CONSTASCII_STRINGPARAM( "cell-content-is-in-list" ) },
    { XML_COND_TEXTLENGTH,              XML_COND_TYPE_COMPARISON, sheet::ValidationType_TEXT_LEN, sheet::ConditionOperator_NONE,        RTL_CONSTASCII_STRINGPARAM( "cell-content-text-length" ) },
    { XML_COND_TEXTLENGTH_ISBETWEEN,    XML_COND_TYPE_FUNCTION2,  sheet::ValidationType_TEXT_LEN, sheet::ConditionOperator_BETWEEN,     RTL_CONSTASCII_STRINGPARAM( "cell-content-text-length-is-between" ) },
    { XML_COND_TEXTLENGTH_ISNOTBETWEEN, XML_COND_TYPE_FUNCTION2,  sheet::ValidationType_TEXT_LEN, sheet::ConditionOperator_NOT_BETWEEN, RTL_CONSTASCII_STRINGPARAM( "cell-content-text-length-is-not-between" ) },
    { XML_COND_ISTRUEFORMULA,           XML_COND_TYPE_FUNCTION1,  sheet::ValidationType_CUSTOM,   sheet::ConditionOperator_FORMULA,     RTL_CONSTASCII_STRINGPARAM( "is-true-formula" ) }
};

void lclSkipWhitespace( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd )
{
    while( (rpcString < pcEnd) && (*rpcString <= ' ') ) ++rpcString;
}

const ScXMLConditionInfo* lclGetConditionInfo( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd )
{
    lclSkipWhitespace( rpcString, pcEnd );
    /*  Search the end of an identifier name; assuming that valid identifiers
        consist of [a-z-] only. */
    const sal_Unicode* pcIdStart = rpcString;
    while( (rpcString < pcEnd) && (((*rpcString >= 'a') && (*rpcString <= 'z')) || (*rpcString == '-')) ) ++rpcString;
    sal_Int32 nLength = static_cast< sal_Int32 >( rpcString - pcIdStart );

    // search the table for an entry
    if( nLength > 0 )
        for( const ScXMLConditionInfo* pInfo = spConditionInfos; pInfo < STATIC_TABLE_END( spConditionInfos ); ++pInfo )
            if( (nLength == pInfo->mnIdentLength) && (::rtl_ustr_ascii_shortenedCompare_WithLength( pcIdStart, nLength, pInfo->mpcIdentifier, nLength ) == 0) )
                return pInfo;

    return 0;
}

sheet::ConditionOperator lclGetConditionOperator( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd )
{
    // check for double-char operators
    if( (rpcString + 1 < pcEnd) && (rpcString[ 1 ] == '=') )
    {
        sheet::ConditionOperator eOperator = sheet::ConditionOperator_NONE;
        switch( *rpcString )
        {
            case '!':   eOperator = sheet::ConditionOperator_NOT_EQUAL;     break;
            case '<':   eOperator = sheet::ConditionOperator_LESS_EQUAL;    break;
            case '>':   eOperator = sheet::ConditionOperator_GREATER_EQUAL; break;
        }
        if( eOperator != sheet::ConditionOperator_NONE )
        {
            rpcString += 2;
            return eOperator;
        }
    }

    // check for single-char operators
    if( rpcString < pcEnd )
    {
        sheet::ConditionOperator eOperator = sheet::ConditionOperator_NONE;
        switch( *rpcString )
        {
            case '=':   eOperator = sheet::ConditionOperator_EQUAL;     break;
            case '<':   eOperator = sheet::ConditionOperator_LESS;      break;
            case '>':   eOperator = sheet::ConditionOperator_GREATER;   break;
        }
        if( eOperator != sheet::ConditionOperator_NONE )
        {
            ++rpcString;
            return eOperator;
        }
    }

    return sheet::ConditionOperator_NONE;
}

/** Skips a literal string in a formula expression.

    @param rpcString
        (in-out) On call, must point to the first character of the string
        following the leading string delimiter character. On return, points to
        the trailing string delimiter character if existing, otherwise to
        pcEnd.

    @param pcEnd
        The end of the string to parse.

    @param cQuoteChar
        The string delimiter character enclosing the string.
  */
void lclSkipExpressionString( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd, sal_Unicode cQuoteChar )
{
    if( rpcString < pcEnd )
    {
        sal_Int32 nLength = static_cast< sal_Int32 >( pcEnd - rpcString );
        sal_Int32 nNextQuote = ::rtl_ustr_indexOfChar_WithLength( rpcString, nLength, cQuoteChar );
        if( nNextQuote >= 0 )
            rpcString += nNextQuote;
        else
            rpcString = pcEnd;
    }
}

/** Skips a formula expression. Processes embedded parentheses, braces, and
    literal strings.

    @param rpcString
        (in-out) On call, must point to the first character of the expression.
        On return, points to the passed end character if existing, otherwise to
        pcEnd.

    @param pcEnd
        The end of the string to parse.

    @param cEndChar
        The termination character following the expression.
  */
void lclSkipExpression( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd, sal_Unicode cEndChar )
{
    while( rpcString < pcEnd )
    {
        if( *rpcString == cEndChar )
            return;
        switch( *rpcString )
        {
            case '(':       lclSkipExpression( ++rpcString, pcEnd, ')' );           break;
            case '{':       lclSkipExpression( ++rpcString, pcEnd, '}' );           break;
            case '"':       lclSkipExpressionString( ++rpcString, pcEnd, '"' );     break;
            case '\'':      lclSkipExpressionString( ++rpcString, pcEnd, '\'' );    break;
        }
        if( rpcString < pcEnd ) ++rpcString;
    }
}

/** Extracts a formula expression. Processes embedded parentheses, braces, and
    literal strings.

    @param rpcString
        (in-out) On call, must point to the first character of the expression.
        On return, points *behind* the passed end character if existing,
        otherwise to pcEnd.

    @param pcEnd
        The end of the string to parse.

    @param cEndChar
        The termination character following the expression.
  */
OUString lclGetExpression( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd, sal_Unicode cEndChar )
{
    OUString aExp;
    const sal_Unicode* pcExpStart = rpcString;
    lclSkipExpression( rpcString, pcEnd, cEndChar );
    if( rpcString < pcEnd )
    {
        aExp = OUString( pcExpStart, static_cast< sal_Int32 >( rpcString - pcExpStart ) ).trim();
        ++rpcString;
    }
    return aExp;
}

/** Tries to skip an empty pair of parentheses (which may contain whitespace
    characters).

    @return
        True on success, rpcString points behind the closing parentheses then.
 */
bool lclSkipEmptyParentheses( const sal_Unicode*& rpcString, const sal_Unicode* pcEnd )
{
    if( (rpcString < pcEnd) && (*rpcString == '(') )
    {
        lclSkipWhitespace( ++rpcString, pcEnd );
        if( (rpcString < pcEnd) && (*rpcString == ')') )
        {
            ++rpcString;
            return true;
        }
    }
    return false;
}

} // namespace

// ----------------------------------------------------------------------------

void ScXMLConditionHelper::parseCondition(
        ScXMLConditionParseResult& rParseResult, const OUString& rAttribute, sal_Int32 nStartIndex )
{
    rParseResult.meToken = XML_COND_INVALID;
    if( (nStartIndex < 0) || (nStartIndex >= rAttribute.getLength()) ) return;

    // try to find an identifier
    const sal_Unicode* pcBegin = rAttribute.getStr();
    const sal_Unicode* pcString = pcBegin + nStartIndex;
    const sal_Unicode* pcEnd = pcBegin + rAttribute.getLength();
    if( const ScXMLConditionInfo* pCondInfo = lclGetConditionInfo( pcString, pcEnd ) )
    {
        // insert default values into parse result (may be changed below)
        rParseResult.meValidation = pCondInfo->meValidation;
        rParseResult.meOperator = pCondInfo->meOperator;
        // continue parsing dependent on token type
        switch( pCondInfo->meType )
        {
            case XML_COND_TYPE_KEYWORD:
                // nothing specific has to follow, success
                rParseResult.meToken = pCondInfo->meToken;
            break;

            case XML_COND_TYPE_COMPARISON:
                // format is <condition>()<operator><expression>
                if( lclSkipEmptyParentheses( pcString, pcEnd ) )
                {
                    rParseResult.meOperator = lclGetConditionOperator( pcString, pcEnd );
                    if( rParseResult.meOperator != sheet::ConditionOperator_NONE )
                    {
                        lclSkipWhitespace( pcString, pcEnd );
                        if( pcString < pcEnd )
                        {
                            rParseResult.meToken = pCondInfo->meToken;
                            // comparison must be at end of attribute, remaining text is the formula
                            rParseResult.maOperand1 = OUString( pcString, static_cast< sal_Int32 >( pcEnd - pcString ) );
                        }
                    }
                }
            break;

            case XML_COND_TYPE_FUNCTION0:
                // format is <condition>()
                if( lclSkipEmptyParentheses( pcString, pcEnd ) )
                    rParseResult.meToken = pCondInfo->meToken;
            break;

            case XML_COND_TYPE_FUNCTION1:
                // format is <condition>(<expression>)
                if( (pcString < pcEnd) && (*pcString == '(') )
                {
                    rParseResult.maOperand1 = lclGetExpression( ++pcString, pcEnd, ')' );
                    if( rParseResult.maOperand1.getLength() > 0 )
                        rParseResult.meToken = pCondInfo->meToken;
                }
            break;

            case XML_COND_TYPE_FUNCTION2:
                // format is <condition>(<expression1>,<expression2>)
                if( (pcString < pcEnd) && (*pcString == '(') )
                {
                    rParseResult.maOperand1 = lclGetExpression( ++pcString, pcEnd, ',' );
                    if( rParseResult.maOperand1.getLength() > 0 )
                    {
                        rParseResult.maOperand2 = lclGetExpression( pcString, pcEnd, ')' );
                        if( rParseResult.maOperand2.getLength() > 0 )
                            rParseResult.meToken = pCondInfo->meToken;
                    }
                }
            break;
        }
        rParseResult.mnEndIndex = static_cast< sal_Int32 >( pcString - pcBegin );
    }
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
