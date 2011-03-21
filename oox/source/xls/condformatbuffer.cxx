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

#include "oox/xls/condformatbuffer.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/stylesbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_Int32 BIFF12_CFRULE_TYPE_CELLIS           = 1;
const sal_Int32 BIFF12_CFRULE_TYPE_EXPRESSION       = 2;
const sal_Int32 BIFF12_CFRULE_TYPE_COLORSCALE       = 3;
const sal_Int32 BIFF12_CFRULE_TYPE_DATABAR          = 4;
const sal_Int32 BIFF12_CFRULE_TYPE_TOPTEN           = 5;
const sal_Int32 BIFF12_CFRULE_TYPE_ICONSET          = 6;

const sal_Int32 BIFF12_CFRULE_SUB_CELLIS            = 0;
const sal_Int32 BIFF12_CFRULE_SUB_EXPRESSION        = 1;
const sal_Int32 BIFF12_CFRULE_SUB_COLORSCALE        = 2;
const sal_Int32 BIFF12_CFRULE_SUB_DATABAR           = 3;
const sal_Int32 BIFF12_CFRULE_SUB_ICONSET           = 4;
const sal_Int32 BIFF12_CFRULE_SUB_TOPTEN            = 5;
const sal_Int32 BIFF12_CFRULE_SUB_UNIQUE            = 7;
const sal_Int32 BIFF12_CFRULE_SUB_TEXT              = 8;
const sal_Int32 BIFF12_CFRULE_SUB_BLANK             = 9;
const sal_Int32 BIFF12_CFRULE_SUB_NOTBLANK          = 10;
const sal_Int32 BIFF12_CFRULE_SUB_ERROR             = 11;
const sal_Int32 BIFF12_CFRULE_SUB_NOTERROR          = 12;
const sal_Int32 BIFF12_CFRULE_SUB_TODAY             = 15;
const sal_Int32 BIFF12_CFRULE_SUB_TOMORROW          = 16;
const sal_Int32 BIFF12_CFRULE_SUB_YESTERDAY         = 17;
const sal_Int32 BIFF12_CFRULE_SUB_LAST7DAYS         = 18;
const sal_Int32 BIFF12_CFRULE_SUB_LASTMONTH         = 19;
const sal_Int32 BIFF12_CFRULE_SUB_NEXTMONTH         = 20;
const sal_Int32 BIFF12_CFRULE_SUB_THISWEEK          = 21;
const sal_Int32 BIFF12_CFRULE_SUB_NEXTWEEK          = 22;
const sal_Int32 BIFF12_CFRULE_SUB_LASTWEEK          = 23;
const sal_Int32 BIFF12_CFRULE_SUB_THISMONTH         = 24;
const sal_Int32 BIFF12_CFRULE_SUB_ABOVEAVERAGE      = 25;
const sal_Int32 BIFF12_CFRULE_SUB_BELOWAVERAGE      = 26;
const sal_Int32 BIFF12_CFRULE_SUB_DUPLICATE         = 27;
const sal_Int32 BIFF12_CFRULE_SUB_EQABOVEAVERAGE    = 29;
const sal_Int32 BIFF12_CFRULE_SUB_EQBELOWAVERAGE    = 30;

const sal_Int32 BIFF12_CFRULE_TIMEOP_TODAY          = 0;
const sal_Int32 BIFF12_CFRULE_TIMEOP_YESTERDAY      = 1;
const sal_Int32 BIFF12_CFRULE_TIMEOP_LAST7DAYS      = 2;
const sal_Int32 BIFF12_CFRULE_TIMEOP_THISWEEK       = 3;
const sal_Int32 BIFF12_CFRULE_TIMEOP_LASTWEEK       = 4;
const sal_Int32 BIFF12_CFRULE_TIMEOP_LASTMONTH      = 5;
const sal_Int32 BIFF12_CFRULE_TIMEOP_TOMORROW       = 6;
const sal_Int32 BIFF12_CFRULE_TIMEOP_NEXTWEEK       = 7;
const sal_Int32 BIFF12_CFRULE_TIMEOP_NEXTMONTH      = 8;
const sal_Int32 BIFF12_CFRULE_TIMEOP_THISMONTH      = 9;

const sal_uInt16 BIFF12_CFRULE_STOPIFTRUE           = 0x0002;
const sal_uInt16 BIFF12_CFRULE_ABOVEAVERAGE         = 0x0004;
const sal_uInt16 BIFF12_CFRULE_BOTTOM               = 0x0008;
const sal_uInt16 BIFF12_CFRULE_PERCENT              = 0x0010;

// ----------------------------------------------------------------------------

template< typename Type >
void lclAppendProperty( ::std::vector< PropertyValue >& orProps, const OUString& rPropName, const Type& rValue )
{
    orProps.push_back( PropertyValue() );
    orProps.back().Name = rPropName;
    orProps.back().Value <<= rValue;
}

} // namespace

// ============================================================================

CondFormatRuleModel::CondFormatRuleModel() :
    mnPriority( -1 ),
    mnType( XML_TOKEN_INVALID ),
    mnOperator( XML_TOKEN_INVALID ),
    mnTimePeriod( XML_TOKEN_INVALID ),
    mnRank( 0 ),
    mnStdDev( 0 ),
    mnDxfId( -1 ),
    mbStopIfTrue( false ),
    mbBottom( false ),
    mbPercent( false ),
    mbAboveAverage( true ),
    mbEqualAverage( false )
{
}

void CondFormatRuleModel::setBiffOperator( sal_Int32 nOperator )
{
    static const sal_Int32 spnOperators[] = {
        XML_TOKEN_INVALID, XML_between, XML_notBetween, XML_equal, XML_notEqual,
        XML_greaterThan, XML_lessThan, XML_greaterThanOrEqual, XML_lessThanOrEqual };
    mnOperator = STATIC_ARRAY_SELECT( spnOperators, nOperator, XML_TOKEN_INVALID );
}

void CondFormatRuleModel::setBiff12TextType( sal_Int32 nOperator )
{
    // note: type XML_notContainsText vs. operator XML_notContains
    static const sal_Int32 spnTypes[] = { XML_containsText, XML_notContainsText, XML_beginsWith, XML_endsWith };
    mnType = STATIC_ARRAY_SELECT( spnTypes, nOperator, XML_TOKEN_INVALID );
    static const sal_Int32 spnOperators[] = { XML_containsText, XML_notContains, XML_beginsWith, XML_endsWith };
    mnOperator = STATIC_ARRAY_SELECT( spnOperators, nOperator, XML_TOKEN_INVALID );
}

// ============================================================================

CondFormatRule::CondFormatRule( const CondFormat& rCondFormat ) :
    WorksheetHelper( rCondFormat ),
    mrCondFormat( rCondFormat )
{
}

void CondFormatRule::importCfRule( const AttributeList& rAttribs )
{
    maModel.maText         = rAttribs.getString( XML_text, OUString() );
    maModel.mnPriority     = rAttribs.getInteger( XML_priority, -1 );
    maModel.mnType         = rAttribs.getToken( XML_type, XML_TOKEN_INVALID );
    maModel.mnOperator     = rAttribs.getToken( XML_operator, XML_TOKEN_INVALID );
    maModel.mnTimePeriod   = rAttribs.getToken( XML_timePeriod, XML_TOKEN_INVALID );
    maModel.mnRank         = rAttribs.getInteger( XML_rank, 0 );
    maModel.mnStdDev       = rAttribs.getInteger( XML_stdDev, 0 );
    maModel.mnDxfId        = rAttribs.getInteger( XML_dxfId, -1 );
    maModel.mbStopIfTrue   = rAttribs.getBool( XML_stopIfTrue, false );
    maModel.mbBottom       = rAttribs.getBool( XML_bottom, false );
    maModel.mbPercent      = rAttribs.getBool( XML_percent, false );
    maModel.mbAboveAverage = rAttribs.getBool( XML_aboveAverage, true );
    maModel.mbEqualAverage = rAttribs.getBool( XML_equalAverage, false );
}

void CondFormatRule::appendFormula( const OUString& rFormula )
{
    CellAddress aBaseAddr = mrCondFormat.getRanges().getBaseAddress();
    ApiTokenSequence aTokens = getFormulaParser().importFormula( aBaseAddr, rFormula );
    maModel.maFormulas.push_back( aTokens );
}

void CondFormatRule::importCfRule( SequenceInputStream& rStrm )
{
    sal_Int32 nType, nSubType, nOperator, nFmla1Size, nFmla2Size, nFmla3Size;
    sal_uInt16 nFlags;
    rStrm >> nType >> nSubType >> maModel.mnDxfId >> maModel.mnPriority >> nOperator;
    rStrm.skip( 8 );
    rStrm >> nFlags >> nFmla1Size >> nFmla2Size >> nFmla3Size >> maModel.maText;

    /*  Import the formulas. For no obvious reason, the sizes of the formulas
        are already stored before. Nevertheless the following formulas contain
        their own sizes. */

    // first formula
    OSL_ENSURE( (nFmla1Size >= 0) || ((nFmla2Size == 0) && (nFmla3Size == 0)), "CondFormatRule::importCfRule - missing first formula" );
    OSL_ENSURE( (nFmla1Size > 0) == (rStrm.getRemaining() >= 8), "CondFormatRule::importCfRule - formula size mismatch" );
    if( rStrm.getRemaining() >= 8 )
    {
        CellAddress aBaseAddr = mrCondFormat.getRanges().getBaseAddress();
        ApiTokenSequence aTokens = getFormulaParser().importFormula( aBaseAddr, FORMULATYPE_CONDFORMAT, rStrm );
        maModel.maFormulas.push_back( aTokens );

        // second formula
        OSL_ENSURE( (nFmla2Size >= 0) || (nFmla3Size == 0), "CondFormatRule::importCfRule - missing second formula" );
        OSL_ENSURE( (nFmla2Size > 0) == (rStrm.getRemaining() >= 8), "CondFormatRule::importCfRule - formula size mismatch" );
        if( rStrm.getRemaining() >= 8 )
        {
            aTokens = getFormulaParser().importFormula( aBaseAddr, FORMULATYPE_CONDFORMAT, rStrm );
            maModel.maFormulas.push_back( aTokens );

            // third formula
            OSL_ENSURE( (nFmla3Size > 0) == (rStrm.getRemaining() >= 8), "CondFormatRule::importCfRule - formula size mismatch" );
            if( rStrm.getRemaining() >= 8 )
            {
                aTokens = getFormulaParser().importFormula( aBaseAddr, FORMULATYPE_CONDFORMAT, rStrm );
                maModel.maFormulas.push_back( aTokens );
            }
        }
    }

    // flags
    maModel.mbStopIfTrue   = getFlag( nFlags, BIFF12_CFRULE_STOPIFTRUE );
    maModel.mbBottom       = getFlag( nFlags, BIFF12_CFRULE_BOTTOM );
    maModel.mbPercent      = getFlag( nFlags, BIFF12_CFRULE_PERCENT );
    maModel.mbAboveAverage = getFlag( nFlags, BIFF12_CFRULE_ABOVEAVERAGE );
    // no flag for equalAverage, must be determined from subtype below...

    // Convert the type/operator settings. This is a real mess...
    switch( nType )
    {
        case BIFF12_CFRULE_TYPE_CELLIS:
            OSL_ENSURE( nSubType == BIFF12_CFRULE_SUB_CELLIS, "CondFormatRule::importCfRule - rule type/subtype mismatch" );
            maModel.mnType = XML_cellIs;
            maModel.setBiffOperator( nOperator );
            OSL_ENSURE( maModel.mnOperator != XML_TOKEN_INVALID, "CondFormatRule::importCfRule - unknown operator" );
        break;
        case BIFF12_CFRULE_TYPE_EXPRESSION:
            // here we have to look at the subtype to find the real type...
            switch( nSubType )
            {
                case BIFF12_CFRULE_SUB_EXPRESSION:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_expression;
                break;
                case BIFF12_CFRULE_SUB_UNIQUE:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_uniqueValues;
                break;
                case BIFF12_CFRULE_SUB_TEXT:
                    maModel.setBiff12TextType( nOperator );
                    OSL_ENSURE( maModel.mnType != XML_TOKEN_INVALID, "CondFormatRule::importCfRule - unexpected operator value" );
                break;
                case BIFF12_CFRULE_SUB_BLANK:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_containsBlanks;
                break;
                case BIFF12_CFRULE_SUB_NOTBLANK:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_notContainsBlanks;
                break;
                case BIFF12_CFRULE_SUB_ERROR:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_containsErrors;
                break;
                case BIFF12_CFRULE_SUB_NOTERROR:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_notContainsErrors;
                break;
                case BIFF12_CFRULE_SUB_TODAY:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_TODAY, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_today;
                break;
                case BIFF12_CFRULE_SUB_TOMORROW:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_TOMORROW, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_tomorrow;
                break;
                case BIFF12_CFRULE_SUB_YESTERDAY:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_YESTERDAY, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_yesterday;
                break;
                case BIFF12_CFRULE_SUB_LAST7DAYS:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_LAST7DAYS, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_last7Days;
                break;
                case BIFF12_CFRULE_SUB_LASTMONTH:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_LASTMONTH, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_lastMonth;
                break;
                case BIFF12_CFRULE_SUB_NEXTMONTH:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_NEXTMONTH, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_nextMonth;
                break;
                case BIFF12_CFRULE_SUB_THISWEEK:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_THISWEEK, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_thisWeek;
                break;
                case BIFF12_CFRULE_SUB_NEXTWEEK:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_NEXTWEEK, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_nextWeek;
                break;
                case BIFF12_CFRULE_SUB_LASTWEEK:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_LASTWEEK, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_lastWeek;
                break;
                case BIFF12_CFRULE_SUB_THISMONTH:
                    OSL_ENSURE( nOperator == BIFF12_CFRULE_TIMEOP_THISMONTH, "CondFormatRule::importCfRule - unexpected time operator value" );
                    maModel.mnType = XML_timePeriod;
                    maModel.mnTimePeriod = XML_thisMonth;
                break;
                case BIFF12_CFRULE_SUB_ABOVEAVERAGE:
                    OSL_ENSURE( maModel.mbAboveAverage, "CondFormatRule::importCfRule - wrong above-average flag" );
                    maModel.mnType = XML_aboveAverage;
                    maModel.mnStdDev = nOperator;     // operator field used for standard deviation
                    maModel.mbAboveAverage = true;
                    maModel.mbEqualAverage = false;   // does not exist as real flag...
                break;
                case BIFF12_CFRULE_SUB_BELOWAVERAGE:
                    OSL_ENSURE( !maModel.mbAboveAverage, "CondFormatRule::importCfRule - wrong above-average flag" );
                    maModel.mnType = XML_aboveAverage;
                    maModel.mnStdDev = nOperator;     // operator field used for standard deviation
                    maModel.mbAboveAverage = false;
                    maModel.mbEqualAverage = false;   // does not exist as real flag...
                break;
                case BIFF12_CFRULE_SUB_DUPLICATE:
                    OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
                    maModel.mnType = XML_duplicateValues;
                break;
                case BIFF12_CFRULE_SUB_EQABOVEAVERAGE:
                    OSL_ENSURE( maModel.mbAboveAverage, "CondFormatRule::importCfRule - wrong above-average flag" );
                    maModel.mnType = XML_aboveAverage;
                    maModel.mnStdDev = nOperator;     // operator field used for standard deviation
                    maModel.mbAboveAverage = true;
                    maModel.mbEqualAverage = true;    // does not exist as real flag...
                break;
                case BIFF12_CFRULE_SUB_EQBELOWAVERAGE:
                    OSL_ENSURE( !maModel.mbAboveAverage, "CondFormatRule::importCfRule - wrong above-average flag" );
                    maModel.mnType = XML_aboveAverage;
                    maModel.mnStdDev = nOperator;     // operator field used for standard deviation
                    maModel.mbAboveAverage = false;
                    maModel.mbEqualAverage = true;    // does not exist as real flag...
                break;
            }
        break;
        case BIFF12_CFRULE_TYPE_COLORSCALE:
            OSL_ENSURE( nSubType == BIFF12_CFRULE_SUB_COLORSCALE, "CondFormatRule::importCfRule - rule type/subtype mismatch" );
            OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
            maModel.mnType = XML_colorScale;
        break;
        case BIFF12_CFRULE_TYPE_DATABAR:
            OSL_ENSURE( nSubType == BIFF12_CFRULE_SUB_DATABAR, "CondFormatRule::importCfRule - rule type/subtype mismatch" );
            OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
            maModel.mnType = XML_dataBar;
        break;
        case BIFF12_CFRULE_TYPE_TOPTEN:
            OSL_ENSURE( nSubType == BIFF12_CFRULE_SUB_TOPTEN, "CondFormatRule::importCfRule - rule type/subtype mismatch" );
            maModel.mnType = XML_top10;
            maModel.mnRank = nOperator;   // operator field used for rank value
        break;
        case BIFF12_CFRULE_TYPE_ICONSET:
            OSL_ENSURE( nSubType == BIFF12_CFRULE_SUB_ICONSET, "CondFormatRule::importCfRule - rule type/subtype mismatch" );
            OSL_ENSURE( nOperator == 0, "CondFormatRule::importCfRule - unexpected operator value" );
            maModel.mnType = XML_iconSet;
        break;
        default:
            OSL_ENSURE( false, "CondFormatRule::importCfRule - unknown rule type" );
    }
}

void CondFormatRule::importCfRule( BiffInputStream& rStrm, sal_Int32 nPriority )
{
    sal_uInt8 nType, nOperator;
    sal_uInt16 nFmla1Size, nFmla2Size;
    sal_uInt32 nFlags;
    rStrm >> nType >> nOperator >> nFmla1Size >> nFmla2Size >> nFlags;
    rStrm.skip( 2 );

    static const sal_Int32 spnTypeIds[] = { XML_TOKEN_INVALID, XML_cellIs, XML_expression };
    maModel.mnType = STATIC_ARRAY_SELECT( spnTypeIds, nType, XML_TOKEN_INVALID );

    maModel.setBiffOperator( nOperator );
    maModel.mnPriority = nPriority;
    maModel.mbStopIfTrue = true;

    DxfRef xDxf = getStyles().createDxf( &maModel.mnDxfId );
    xDxf->importCfRule( rStrm, nFlags );
    xDxf->finalizeImport();

    // import the formulas
    OSL_ENSURE( (nFmla1Size > 0) || (nFmla2Size == 0), "CondFormatRule::importCfRule - missing first formula" );
    if( nFmla1Size > 0 )
    {
        CellAddress aBaseAddr = mrCondFormat.getRanges().getBaseAddress();
        ApiTokenSequence aTokens = getFormulaParser().importFormula( aBaseAddr, FORMULATYPE_CONDFORMAT, rStrm, &nFmla1Size );
        maModel.maFormulas.push_back( aTokens );
        if( nFmla2Size > 0 )
        {
            aTokens = getFormulaParser().importFormula( aBaseAddr, FORMULATYPE_CONDFORMAT, rStrm, &nFmla2Size );
            maModel.maFormulas.push_back( aTokens );
        }
    }
}

void CondFormatRule::finalizeImport( const Reference< XSheetConditionalEntries >& rxEntries )
{
    ConditionOperator eOperator = ConditionOperator_NONE;

    /*  Replacement formula for unsupported rule types (text comparison rules,
        time period rules, cell type rules). The replacement formulas below may
        contain several placeholders:
        - '#B' will be replaced by the current relative base address (may occur
            several times).
        - '#R' will be replaced by the entire range list of the conditional
            formatting (absolute addresses).
        - '#T' will be replaced by the quoted comparison text.
        - '#L' will be replaced by the length of the comparison text (from
            the 'text' attribute) used in text comparison rules.
        - '#K' will be replaced by the rank (from the 'rank' attribute) used in
            top-10 rules.
        - '#M' will be replaced by the top/bottom flag (from the 'bottom'
            attribute) used in the RANK function in top-10 rules.
        - '#C' will be replaced by one of the comparison operators <, >, <=, or
            >=, according to the 'aboveAverage' and 'equalAverage' flags.
     */
    OUString aReplaceFormula;

    switch( maModel.mnType )
    {
        case XML_cellIs:
            eOperator = CondFormatBuffer::convertToApiOperator( maModel.mnOperator );
        break;
        case XML_expression:
            eOperator = ConditionOperator_FORMULA;
        break;
        case XML_containsText:
            OSL_ENSURE( maModel.mnOperator == XML_containsText, "CondFormatRule::finalizeImport - unexpected operator" );
            aReplaceFormula = CREATE_OUSTRING( "NOT(ISERROR(SEARCH(#T,#B)))" );
        break;
        case XML_notContainsText:
            // note: type XML_notContainsText vs. operator XML_notContains
            OSL_ENSURE( maModel.mnOperator == XML_notContains, "CondFormatRule::finalizeImport - unexpected operator" );
            aReplaceFormula = CREATE_OUSTRING( "ISERROR(SEARCH(#T,#B))" );
        break;
        case XML_beginsWith:
            OSL_ENSURE( maModel.mnOperator == XML_beginsWith, "CondFormatRule::finalizeImport - unexpected operator" );
            aReplaceFormula = CREATE_OUSTRING( "LEFT(#B,#L)=#T" );
        break;
        case XML_endsWith:
            OSL_ENSURE( maModel.mnOperator == XML_endsWith, "CondFormatRule::finalizeImport - unexpected operator" );
            aReplaceFormula = CREATE_OUSTRING( "RIGHT(#B,#L)=#T" );
        break;
        case XML_timePeriod:
            switch( maModel.mnTimePeriod )
            {
                case XML_yesterday:
                    aReplaceFormula = CREATE_OUSTRING( "FLOOR(#B,1)=TODAY()-1" );
                break;
                case XML_today:
                    aReplaceFormula = CREATE_OUSTRING( "FLOOR(#B,1)=TODAY()" );
                break;
                case XML_tomorrow:
                    aReplaceFormula = CREATE_OUSTRING( "FLOOR(#B,1)=TODAY()+1" );
                break;
                case XML_last7Days:
                    aReplaceFormula = CREATE_OUSTRING( "AND(TODAY()-7<FLOOR(#B,1),FLOOR(#B,1)<=TODAY())" );
                break;
                case XML_lastWeek:
                    aReplaceFormula = CREATE_OUSTRING( "AND(TODAY()-WEEKDAY(TODAY())-7<FLOOR(#B,1),FLOOR(#B,1)<=TODAY()-WEEKDAY(TODAY()))" );
                break;
                case XML_thisWeek:
                    aReplaceFormula = CREATE_OUSTRING( "AND(TODAY()-WEEKDAY(TODAY())<FLOOR(#B,1),FLOOR(#B,1)<=TODAY()-WEEKDAY(TODAY())+7)" );
                break;
                case XML_nextWeek:
                    aReplaceFormula = CREATE_OUSTRING( "AND(TODAY()-WEEKDAY(TODAY())+7<FLOOR(#B,1),FLOOR(#B,1)<=TODAY()-WEEKDAY(TODAY())+14)" );
                break;
                case XML_lastMonth:
                    aReplaceFormula = CREATE_OUSTRING( "OR(AND(MONTH(#B)=MONTH(TODAY())-1,YEAR(#B)=YEAR(TODAY())),AND(MONTH(#B)=12,MONTH(TODAY())=1,YEAR(#B)=YEAR(TODAY())-1))" );
                break;
                case XML_thisMonth:
                    aReplaceFormula = CREATE_OUSTRING( "AND(MONTH(#B)=MONTH(TODAY()),YEAR(#B)=YEAR(TODAY()))" );
                break;
                case XML_nextMonth:
                    aReplaceFormula = CREATE_OUSTRING( "OR(AND(MONTH(#B)=MONTH(TODAY())+1,YEAR(#B)=YEAR(TODAY())),AND(MONTH(#B)=1,MONTH(TODAY())=12,YEAR(#B)=YEAR(TODAY())+1))" );
                break;
                default:
                    OSL_ENSURE( false, "CondFormatRule::finalizeImport - unknown time period type" );
            }
        break;
        case XML_containsBlanks:
            aReplaceFormula = CREATE_OUSTRING( "LEN(TRIM(#B))=0" );
        break;
        case XML_notContainsBlanks:
            aReplaceFormula = CREATE_OUSTRING( "LEN(TRIM(#B))>0" );
        break;
        case XML_containsErrors:
            aReplaceFormula = CREATE_OUSTRING( "ISERROR(#B)" );
        break;
        case XML_notContainsErrors:
            aReplaceFormula = CREATE_OUSTRING( "NOT(ISERROR(#B))" );
        break;
        case XML_top10:
            if( maModel.mbPercent )
                aReplaceFormula = CREATE_OUSTRING( "RANK(#B,#R,#M)/COUNT(#R)<=#K%" );
            else
                aReplaceFormula = CREATE_OUSTRING( "RANK(#B,#R,#M)<=#K" );
        break;
        case XML_aboveAverage:
            if( maModel.mnStdDev == 0 )
                aReplaceFormula = CREATE_OUSTRING( "#B#CAVERAGE(#R)" );
        break;
    }

    if( aReplaceFormula.getLength() > 0 )
    {
        OUString aAddress, aRanges, aText, aComp;
        sal_Int32 nStrPos = aReplaceFormula.getLength();
        while( (nStrPos = aReplaceFormula.lastIndexOf( '#', nStrPos )) >= 0 )
        {
            switch( aReplaceFormula[ nStrPos + 1 ] )
            {
                case 'B':       // current base address
                    if( aAddress.getLength() == 0 )
                        aAddress = FormulaProcessorBase::generateAddress2dString( mrCondFormat.getRanges().getBaseAddress(), false );
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2, aAddress );
                break;
                case 'R':       // range list of conditional formatting
                    if( aRanges.getLength() == 0 )
                        aRanges = FormulaProcessorBase::generateRangeList2dString( mrCondFormat.getRanges(), true, ',', true );
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2, aRanges );
                break;
                case 'T':       // comparison text
                    if( aText.getLength() == 0 )
                        // quote the comparison text, and handle embedded quote characters
                        aText = FormulaProcessorBase::generateApiString( maModel.maText );
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2, aText );
                break;
                case 'L':       // length of comparison text
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2,
                        OUString::valueOf( maModel.maText.getLength() ) );
                break;
                case 'K':       // top-10 rank
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2,
                        OUString::valueOf( maModel.mnRank ) );
                break;
                case 'M':       // top-10 top/bottom flag
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2,
                        OUString::valueOf( static_cast< sal_Int32 >( maModel.mbBottom ? 1 : 0 ) ) );
                break;
                case 'C':       // average comparison operator
                    if( aComp.getLength() == 0 )
                        aComp = maModel.mbAboveAverage ?
                            (maModel.mbEqualAverage ? CREATE_OUSTRING( ">=" ) : CREATE_OUSTRING( ">" )) :
                            (maModel.mbEqualAverage ? CREATE_OUSTRING( "<=" ) : CREATE_OUSTRING( "<" ));
                    aReplaceFormula = aReplaceFormula.replaceAt( nStrPos, 2, aComp );
                break;
                default:
                    OSL_ENSURE( false, "CondFormatRule::finalizeImport - unknown placeholder" );
            }
        }

        // set the replacement formula
        maModel.maFormulas.clear();
        appendFormula( aReplaceFormula );
        eOperator = ConditionOperator_FORMULA;
    }

    if( rxEntries.is() && (eOperator != ConditionOperator_NONE) && !maModel.maFormulas.empty() )
    {
        ::std::vector< PropertyValue > aProps;
        // create condition properties
        lclAppendProperty( aProps, CREATE_OUSTRING( "Operator" ), eOperator );
        lclAppendProperty( aProps, CREATE_OUSTRING( "Formula1" ), maModel.maFormulas[ 0 ] );
        if( maModel.maFormulas.size() >= 2 )
            lclAppendProperty( aProps, CREATE_OUSTRING( "Formula2" ), maModel.maFormulas[ 1 ] );

        // style name for the formatting attributes
        OUString aStyleName = getStyles().createDxfStyle( maModel.mnDxfId );
        if( aStyleName.getLength() > 0 )
            lclAppendProperty( aProps, CREATE_OUSTRING( "StyleName" ), aStyleName );

        // append the new rule
        try
        {
            rxEntries->addNew( ContainerHelper::vectorToSequence( aProps ) );
        }
        catch( Exception& )
        {
        }
    }
}

// ============================================================================

CondFormatModel::CondFormatModel() :
    mbPivot( false )
{
}

// ============================================================================

CondFormat::CondFormat( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void CondFormat::importConditionalFormatting( const AttributeList& rAttribs )
{
    getAddressConverter().convertToCellRangeList( maModel.maRanges, rAttribs.getString( XML_sqref, OUString() ), getSheetIndex(), true );
    maModel.mbPivot = rAttribs.getBool( XML_pivot, false );
}

CondFormatRuleRef CondFormat::importCfRule( const AttributeList& rAttribs )
{
    CondFormatRuleRef xRule = createRule();
    xRule->importCfRule( rAttribs );
    insertRule( xRule );
    return xRule;
}

void CondFormat::importCondFormatting( SequenceInputStream& rStrm )
{
    BinRangeList aRanges;
    rStrm.skip( 8 );
    rStrm >> aRanges;
    getAddressConverter().convertToCellRangeList( maModel.maRanges, aRanges, getSheetIndex(), true );
}

void CondFormat::importCfRule( SequenceInputStream& rStrm )
{
    CondFormatRuleRef xRule = createRule();
    xRule->importCfRule( rStrm );
    insertRule( xRule );
}

void CondFormat::importCfHeader( BiffInputStream& rStrm )
{
    // import the CFHEADER record
    sal_uInt16 nRuleCount;
    BinRangeList aRanges;
    rStrm >> nRuleCount;
    rStrm.skip( 10 );
    rStrm >> aRanges;
    getAddressConverter().convertToCellRangeList( maModel.maRanges, aRanges, getSheetIndex(), true );

    // import following list of CFRULE records
    for( sal_uInt16 nRule = 0; (nRule < nRuleCount) && (rStrm.getNextRecId() == BIFF_ID_CFRULE) && rStrm.startNextRecord(); ++nRule )
    {
        CondFormatRuleRef xRule = createRule();
        xRule->importCfRule( rStrm, nRule + 1 );
        insertRule( xRule );
    }
}

void CondFormat::finalizeImport()
{
    try
    {
        Reference< XSheetCellRanges > xRanges( getCellRangeList( maModel.maRanges ), UNO_SET_THROW );
        PropertySet aPropSet( xRanges );
        Reference< XSheetConditionalEntries > xEntries( aPropSet.getAnyProperty( PROP_ConditionalFormat ), UNO_QUERY_THROW );
        // maRules is sorted by rule priority
        maRules.forEachMem( &CondFormatRule::finalizeImport, ::boost::cref( xEntries ) );
        aPropSet.setProperty( PROP_ConditionalFormat, xEntries );
    }
    catch( Exception& )
    {
    }
}

CondFormatRuleRef CondFormat::createRule()
{
    return CondFormatRuleRef( new CondFormatRule( *this ) );
}

void CondFormat::insertRule( CondFormatRuleRef xRule )
{
    if( xRule.get() && (xRule->getPriority() > 0) )
    {
        OSL_ENSURE( maRules.find( xRule->getPriority() ) == maRules.end(), "CondFormat::insertRule - multiple rules with equal priority" );
        maRules[ xRule->getPriority() ] = xRule;
    }
}

// ============================================================================

CondFormatBuffer::CondFormatBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

CondFormatRef CondFormatBuffer::importConditionalFormatting( const AttributeList& rAttribs )
{
    CondFormatRef xCondFmt = createCondFormat();
    xCondFmt->importConditionalFormatting( rAttribs );
    return xCondFmt;
}

CondFormatRef CondFormatBuffer::importCondFormatting( SequenceInputStream& rStrm )
{
    CondFormatRef xCondFmt = createCondFormat();
    xCondFmt->importCondFormatting( rStrm );
    return xCondFmt;
}

void CondFormatBuffer::importCfHeader( BiffInputStream& rStrm )
{
    createCondFormat()->importCfHeader( rStrm );
}

void CondFormatBuffer::finalizeImport()
{
    maCondFormats.forEachMem( &CondFormat::finalizeImport );
}

ConditionOperator CondFormatBuffer::convertToApiOperator( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_between:               return ConditionOperator_BETWEEN;
        case XML_equal:                 return ConditionOperator_EQUAL;
        case XML_greaterThan:           return ConditionOperator_GREATER;
        case XML_greaterThanOrEqual:    return ConditionOperator_GREATER_EQUAL;
        case XML_lessThan:              return ConditionOperator_LESS;
        case XML_lessThanOrEqual:       return ConditionOperator_LESS_EQUAL;
        case XML_notBetween:            return ConditionOperator_NOT_BETWEEN;
        case XML_notEqual:              return ConditionOperator_NOT_EQUAL;
    }
    return ConditionOperator_NONE;
}

// private --------------------------------------------------------------------

CondFormatRef CondFormatBuffer::createCondFormat()
{
    CondFormatRef xCondFmt( new CondFormat( *this ) );
    maCondFormats.push_back( xCondFmt );
    return xCondFmt;
}

// ============================================================================

} // namespace xls
} // namespace oox
