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

#include <autofilterbuffer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/FilterFieldType.hpp>
#include <com/sun/star/sheet/FilterConnection.hpp>
#include <com/sun/star/sheet/FilterOperator2.hpp>
#include <com/sun/star/sheet/TableFilterField3.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor3.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <comphelper/sequence.hxx>
#include <editeng/brushitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <addressconverter.hxx>
#include <defnamesbuffer.hxx>
#include <biffhelper.hxx>
#include <document.hxx>
#include <dbdata.hxx>
#include <scitems.hxx>
#include <sortparam.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <stylesbuffer.hxx>
#include <userlist.hxx>

namespace oox::xls {

using namespace css;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

namespace {

const sal_uInt8 BIFF12_TOP10FILTER_TOP              = 0x01;
const sal_uInt8 BIFF12_TOP10FILTER_PERCENT          = 0x02;

const sal_uInt16 BIFF12_FILTERCOLUMN_HIDDENBUTTON   = 0x0001;
const sal_uInt16 BIFF12_FILTERCOLUMN_SHOWBUTTON     = 0x0002;

const sal_uInt8 BIFF_FILTER_DATATYPE_NONE           = 0;
const sal_uInt8 BIFF_FILTER_DATATYPE_DOUBLE         = 4;
const sal_uInt8 BIFF_FILTER_DATATYPE_STRING         = 6;
const sal_uInt8 BIFF_FILTER_DATATYPE_BOOLEAN        = 8;
const sal_uInt8 BIFF_FILTER_DATATYPE_EMPTY          = 12;
const sal_uInt8 BIFF_FILTER_DATATYPE_NOTEMPTY       = 14;

bool lclGetApiOperatorFromToken( sal_Int32& rnApiOperator, sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_lessThan:              rnApiOperator = FilterOperator2::LESS;          return true;
        case XML_equal:                 rnApiOperator = FilterOperator2::EQUAL;         return true;
        case XML_lessThanOrEqual:       rnApiOperator = FilterOperator2::LESS_EQUAL;    return true;
        case XML_greaterThan:           rnApiOperator = FilterOperator2::GREATER;       return true;
        case XML_notEqual:              rnApiOperator = FilterOperator2::NOT_EQUAL;     return true;
        case XML_greaterThanOrEqual:    rnApiOperator = FilterOperator2::GREATER_EQUAL; return true;
    }
    return false;
}

/** Removes leading asterisk characters from the passed string.
    @return  True = at least one asterisk character has been removed. */
bool lclTrimLeadingAsterisks( OUString& rValue )
{
    sal_Int32 nLength = rValue.getLength();
    sal_Int32 nPos = 0;
    while( (nPos < nLength) && (rValue[ nPos ] == '*') )
        ++nPos;
    if( nPos > 0 )
    {
        rValue = rValue.copy( nPos );
        return true;
    }
    return false;
}

/** Removes trailing asterisk characters from the passed string.
    @return  True = at least one asterisk character has been removed. */
bool lclTrimTrailingAsterisks( OUString& rValue )
{
    sal_Int32 nLength = rValue.getLength();
    sal_Int32 nPos = nLength;
    while( (nPos > 0) && (rValue[ nPos - 1 ] == '*') )
        --nPos;
    if( nPos < nLength )
    {
        rValue = rValue.copy( 0, nPos );
        return true;
    }
    return false;
}

/** Converts wildcard characters '*' and '?' to regular expressions and quotes
    RE meta characters.
    @return  True = passed string has been changed (RE needs to be enabled). */
bool lclConvertWildcardsToRegExp( OUString& rValue )
{
    // check existence of the wildcard characters '*' and '?'
    if( !rValue.isEmpty() && ((rValue.indexOf( '*' ) >= 0) || (rValue.indexOf( '?' ) >= 0)) )
    {
        OUStringBuffer aBuffer;
        aBuffer.ensureCapacity( rValue.getLength() + 5 );
        const sal_Unicode* pcChar = rValue.getStr();
        const sal_Unicode* pcEnd = pcChar + rValue.getLength();
        for( ; pcChar < pcEnd; ++pcChar )
        {
            switch( *pcChar )
            {
                case '?':
                    aBuffer.append( '.' );
                break;
                case '*':
                    aBuffer.append( ".*" );
                break;
                case '\\': case '.': case '|': case '(': case ')': case '^': case '$':
                    // quote RE meta characters
                    aBuffer.append( "\\" + OUStringChar(*pcChar) );
                break;
                default:
                    aBuffer.append( *pcChar );
            }
        }
        rValue = aBuffer.makeStringAndClear();
        return true;
    }
    return false;
}

} // namespace

ApiFilterSettings::ApiFilterSettings()
{
}

void ApiFilterSettings::appendField( bool bAnd, sal_Int32 nOperator, double fValue )
{
    maFilterFields.emplace_back();
    TableFilterField3& rFilterField = maFilterFields.back();
    rFilterField.Connection = bAnd ? FilterConnection_AND : FilterConnection_OR;
    rFilterField.Operator = nOperator;
    rFilterField.Values.realloc(1);
    auto pValues = rFilterField.Values.getArray();
    pValues[0].FilterType = FilterFieldType::NUMERIC;
    pValues[0].NumericValue = fValue;
}

void ApiFilterSettings::appendField( bool bAnd, sal_Int32 nOperator, const OUString& rValue )
{
    maFilterFields.emplace_back();
    TableFilterField3& rFilterField = maFilterFields.back();
    rFilterField.Connection = bAnd ? FilterConnection_AND : FilterConnection_OR;
    rFilterField.Operator = nOperator;
    rFilterField.Values.realloc(1);
    auto pValues = rFilterField.Values.getArray();
    pValues[0].FilterType = FilterFieldType::STRING;
    pValues[0].StringValue = rValue;
}

void ApiFilterSettings::appendField(bool bAnd, util::Color aColor, bool bIsBackgroundColor)
{
    maFilterFields.emplace_back();
    TableFilterField3& rFilterField = maFilterFields.back();
    rFilterField.Connection = bAnd ? FilterConnection_AND : FilterConnection_OR;
    rFilterField.Operator = FilterOperator2::EQUAL;
    rFilterField.Values.realloc(1);
    auto pValues = rFilterField.Values.getArray();
    pValues[0].FilterType
        = bIsBackgroundColor ? FilterFieldType::BACKGROUND_COLOR : FilterFieldType::TEXT_COLOR;
    pValues[0].ColorValue = aColor;
}

void ApiFilterSettings::appendField( bool bAnd, const std::vector<std::pair<OUString, bool>>& rValues )
{
    maFilterFields.emplace_back();
    TableFilterField3& rFilterField = maFilterFields.back();
    rFilterField.Connection = bAnd ? FilterConnection_AND : FilterConnection_OR;
    rFilterField.Operator = FilterOperator2::EQUAL;
    rFilterField.Values.realloc(rValues.size());
    auto pValues = rFilterField.Values.getArray();
    size_t i = 0;

    for( auto const& it : rValues )
    {
        pValues[i].StringValue = it.first;
        pValues[i++].FilterType
            = it.second ? FilterFieldType::DATE : FilterFieldType::STRING;
    }
}

FilterSettingsBase::FilterSettingsBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void FilterSettingsBase::importAttribs( sal_Int32 /*nElement*/, const AttributeList& /*rAttribs*/ )
{
}

void FilterSettingsBase::importRecord( sal_Int32 /*nRecId*/, SequenceInputStream& /*rStrm*/ )
{
}

ApiFilterSettings FilterSettingsBase::finalizeImport()
{
    return ApiFilterSettings();
}

DiscreteFilter::DiscreteFilter( const WorkbookHelper& rHelper ) :
    FilterSettingsBase( rHelper ),
    mnCalendarType( XML_none ),
    mbShowBlank( false )
{
}

void DiscreteFilter::importAttribs( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case XLS_TOKEN( filters ):
            mnCalendarType = rAttribs.getToken( XML_calendarType, XML_none );
            mbShowBlank = rAttribs.getBool( XML_blank, false );
        break;

        case XLS_TOKEN( filter ):
        {
            OUString aValue = rAttribs.getXString( XML_val, OUString() );
            if( !aValue.isEmpty() )
                maValues.push_back( std::make_pair(aValue, false) );
        }
        break;

        case XLS_TOKEN( dateGroupItem ):
        {
            OUString aDateValue;
            // it is just a fallback, we do not need the XML_day as default value,
            // because if the dateGroupItem exists also XML_dateTimeGrouping exists!
            sal_uInt16 nToken = rAttribs.getToken(XML_dateTimeGrouping, XML_day);
            if( nToken == XML_year || nToken == XML_month || nToken == XML_day ||
                nToken == XML_hour || nToken == XML_minute || nToken == XML_second )
            {
                aDateValue = rAttribs.getString(XML_year, OUString());

                if( nToken == XML_month || nToken == XML_day || nToken == XML_hour ||
                    nToken == XML_minute || nToken == XML_second )
                {
                    OUString aMonthName = rAttribs.getString(XML_month, OUString());
                    if( aMonthName.getLength() == 1 )
                        aMonthName = "0" + aMonthName;
                    aDateValue += "-" + aMonthName;

                    if( nToken == XML_day || nToken == XML_hour || nToken == XML_minute ||
                        nToken == XML_second )
                    {
                        OUString aDayName = rAttribs.getString(XML_day, OUString());
                        if( aDayName.getLength() == 1 )
                            aDayName = "0" + aDayName;
                        aDateValue += "-" + aDayName;

                        if( nToken == XML_hour || nToken == XML_minute || nToken == XML_second )
                        {
                            OUString aHourName = rAttribs.getString(XML_hour, OUString());
                            if( aHourName.getLength() == 1 )
                                aHourName = "0" + aHourName;
                            aDateValue += " " + aHourName;

                            if( nToken == XML_minute || nToken == XML_second )
                            {
                                OUString aMinName = rAttribs.getString(XML_minute, OUString());
                                if( aMinName.getLength() == 1 )
                                    aMinName = "0" + aMinName;
                                aDateValue += ":" + aMinName;

                                if( nToken == XML_second )
                                {
                                    OUString aSecName = rAttribs.getString(XML_second, OUString());
                                    if( aSecName.getLength() == 1 )
                                        aSecName = "0" + aSecName;
                                    aDateValue += ":" + aSecName;
                                }
                            }
                        }
                    }
                }
            }
            if( !aDateValue.isEmpty() )
                maValues.push_back( std::make_pair(aDateValue, true) );
        }
        break;
    }
}

void DiscreteFilter::importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( nRecId )
    {
        case BIFF12_ID_DISCRETEFILTERS:
        {
            sal_Int32 nShowBlank, nCalendarType;
            nShowBlank = rStrm.readInt32();
            nCalendarType = rStrm.readInt32();

            static const sal_Int32 spnCalendarTypes[] = {
                XML_none, XML_gregorian, XML_gregorianUs, XML_japan, XML_taiwan, XML_korea, XML_hijri, XML_thai, XML_hebrew,
                XML_gregorianMeFrench, XML_gregorianArabic, XML_gregorianXlitEnglish, XML_gregorianXlitFrench };
            mnCalendarType = STATIC_ARRAY_SELECT( spnCalendarTypes, nCalendarType, XML_none );
            mbShowBlank = nShowBlank != 0;
        }
        break;

        case BIFF12_ID_DISCRETEFILTER:
        {
            OUString aValue = BiffHelper::readString( rStrm );
            if( !aValue.isEmpty() )
                maValues.push_back( std::make_pair(aValue, false) );
        }
        break;
    }
}

ApiFilterSettings DiscreteFilter::finalizeImport()
{
    ApiFilterSettings aSettings;
    aSettings.maFilterFields.reserve( maValues.size() );

    // insert all filter values
    aSettings.appendField( true, maValues );

    // extra field for 'show empty'
    if( mbShowBlank )
        aSettings.appendField( false, FilterOperator2::EMPTY, OUString() );

    /*  Require disabled regular expressions, filter entries may contain
        any RE meta characters. */
    if( !maValues.empty() )
        aSettings.mobNeedsRegExp = false;

    return aSettings;
}

Top10Filter::Top10Filter( const WorkbookHelper& rHelper ) :
    FilterSettingsBase( rHelper ),
    mfValue( 0.0 ),
    mbTop( true ),
    mbPercent( false )
{
}

void Top10Filter::importAttribs( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( nElement == XLS_TOKEN( top10 ) )
    {
        mfValue = rAttribs.getDouble( XML_val, 0.0 );
        mbTop = rAttribs.getBool( XML_top, true );
        mbPercent = rAttribs.getBool( XML_percent, false );
    }
}

void Top10Filter::importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    if( nRecId == BIFF12_ID_TOP10FILTER )
    {
        sal_uInt8 nFlags;
        nFlags = rStrm.readuChar();
        mfValue = rStrm.readDouble();
        mbTop = getFlag( nFlags, BIFF12_TOP10FILTER_TOP );
        mbPercent = getFlag( nFlags, BIFF12_TOP10FILTER_PERCENT );
    }
}

ApiFilterSettings Top10Filter::finalizeImport()
{
    sal_Int32 nOperator = mbTop ?
        (mbPercent ? FilterOperator2::TOP_PERCENT : FilterOperator2::TOP_VALUES) :
        (mbPercent ? FilterOperator2::BOTTOM_PERCENT : FilterOperator2::BOTTOM_VALUES);
    ApiFilterSettings aSettings;
    aSettings.appendField( true, nOperator, mfValue );
    return aSettings;
}

ColorFilter::ColorFilter(const WorkbookHelper& rHelper)
    : FilterSettingsBase(rHelper)
    , mbIsBackgroundColor(false)
{
}

void ColorFilter::importAttribs(sal_Int32 nElement, const AttributeList& rAttribs)
{
    if (nElement == XLS_TOKEN(colorFilter))
    {
        // When cellColor attribute not found, it means cellColor = true
        // cellColor = 0 (false) -> TextColor
        // cellColor = 1 (true)  -> BackgroundColor
        mbIsBackgroundColor = rAttribs.getBool(XML_cellColor, true);
        msStyleName = getStyles().createDxfStyle( rAttribs.getInteger(XML_dxfId, -1) );
    }
}

void ColorFilter::importRecord(sal_Int32 /* nRecId */, SequenceInputStream& /* rStrm */)
{
    // TODO
}

ApiFilterSettings ColorFilter::finalizeImport()
{
    ApiFilterSettings aSettings;
    ScDocument& rDoc = getScDocument();
    ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>(
        rDoc.GetStyleSheetPool()->Find(msStyleName, SfxStyleFamily::Para));
    if (!pStyleSheet)
        return aSettings;

    const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
    // Color (whether text or background color) is always stored in ATTR_BACKGROUND
    if (const SvxBrushItem* pItem = rItemSet.GetItem<SvxBrushItem>(ATTR_BACKGROUND))
    {
        ::Color aColor = pItem->GetFiltColor();
        util::Color nColor(aColor);
        aSettings.appendField(true, nColor, mbIsBackgroundColor);
    }
    return aSettings;
}

FilterCriterionModel::FilterCriterionModel() :
    mnOperator( XML_equal ),
    mnDataType( BIFF_FILTER_DATATYPE_NONE )
{
}

void FilterCriterionModel::setBiffOperator( sal_uInt8 nOperator )
{
    static const sal_Int32 spnOperators[] = { XML_TOKEN_INVALID,
        XML_lessThan, XML_equal, XML_lessThanOrEqual, XML_greaterThan, XML_notEqual, XML_greaterThanOrEqual };
    mnOperator = STATIC_ARRAY_SELECT( spnOperators, nOperator, XML_TOKEN_INVALID );
}

void FilterCriterionModel::readBiffData( SequenceInputStream& rStrm )
{
    sal_uInt8 nOperator;
    mnDataType = rStrm.readuChar();
    nOperator = rStrm.readuChar();
    setBiffOperator( nOperator );

    switch( mnDataType )
    {
        case BIFF_FILTER_DATATYPE_DOUBLE:
            maValue <<= rStrm.readDouble();
        break;
        case BIFF_FILTER_DATATYPE_STRING:
        {
            rStrm.skip( 8 );
            OUString aValue = BiffHelper::readString( rStrm ).trim();
            if( !aValue.isEmpty() )
                maValue <<= aValue;
        }
        break;
        case BIFF_FILTER_DATATYPE_BOOLEAN:
            maValue <<= (rStrm.readuInt8() != 0);
            rStrm.skip( 7 );
        break;
        case BIFF_FILTER_DATATYPE_EMPTY:
            rStrm.skip( 8 );
            if( mnOperator == XML_equal )
                maValue <<= OUString();
        break;
        case BIFF_FILTER_DATATYPE_NOTEMPTY:
            rStrm.skip( 8 );
            if( mnOperator == XML_notEqual )
                maValue <<= OUString();
        break;
        default:
            OSL_ENSURE( false, "FilterCriterionModel::readBiffData - unexpected data type" );
            rStrm.skip( 8 );
    }
}

CustomFilter::CustomFilter( const WorkbookHelper& rHelper ) :
    FilterSettingsBase( rHelper ),
    mbAnd( false )
{
}

void CustomFilter::importAttribs( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case XLS_TOKEN( customFilters ):
            mbAnd = rAttribs.getBool( XML_and, false );
        break;

        case XLS_TOKEN( customFilter ):
        {
            FilterCriterionModel aCriterion;
            aCriterion.mnOperator = rAttribs.getToken( XML_operator, XML_equal );
            OUString aValue = rAttribs.getXString( XML_val, OUString() ).trim();
            if( (aCriterion.mnOperator == XML_equal) || (aCriterion.mnOperator == XML_notEqual) || (!aValue.isEmpty()) )
                aCriterion.maValue <<= aValue;
            appendCriterion( aCriterion );
        }
        break;
    }
}

void CustomFilter::importRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( nRecId )
    {
        case BIFF12_ID_CUSTOMFILTERS:
            mbAnd = rStrm.readInt32() == 0;
        break;

        case BIFF12_ID_CUSTOMFILTER:
        {
            FilterCriterionModel aCriterion;
            aCriterion.readBiffData( rStrm );
            appendCriterion( aCriterion );
        }
        break;
    }
}

ApiFilterSettings CustomFilter::finalizeImport()
{
    ApiFilterSettings aSettings;
    OSL_ENSURE( maCriteria.size() <= 2, "CustomFilter::finalizeImport - too many filter criteria" );
    for( const auto& rCriterion : maCriteria )
    {
        // first extract the filter operator
        sal_Int32 nOperator = 0;
        bool bValidOperator = lclGetApiOperatorFromToken( nOperator, rCriterion.mnOperator );
        if( bValidOperator )
        {
            if( rCriterion.maValue.has< OUString >() )
            {
                // string argument
                OUString aValue;
                rCriterion.maValue >>= aValue;
                // check for 'empty', 'contains', 'begins with', or 'ends with' text filters
                bool bEqual = nOperator == FilterOperator2::EQUAL;
                bool bNotEqual = nOperator == FilterOperator2::NOT_EQUAL;
                if( bEqual || bNotEqual )
                {
                    if( aValue.isEmpty() )
                    {
                        // empty comparison string: create empty/not empty filters
                        nOperator = bNotEqual ? FilterOperator2::NOT_EMPTY : FilterOperator2::EMPTY;
                    }
                    else
                    {
                        // compare to something: try to find begins/ends/contains
                        bool bHasLeadingAsterisk = lclTrimLeadingAsterisks( aValue );
                        bool bHasTrailingAsterisk = lclTrimTrailingAsterisks( aValue );
                        // just '***' matches everything, do not create a filter field
                        bValidOperator = !aValue.isEmpty();
                        if( bValidOperator )
                        {
                            if( bHasLeadingAsterisk && bHasTrailingAsterisk )
                                nOperator = bNotEqual ? FilterOperator2::DOES_NOT_CONTAIN : FilterOperator2::CONTAINS;
                            else if( bHasLeadingAsterisk )
                                nOperator = bNotEqual ? FilterOperator2::DOES_NOT_END_WITH : FilterOperator2::ENDS_WITH;
                            else if( bHasTrailingAsterisk )
                                nOperator = bNotEqual ? FilterOperator2::DOES_NOT_BEGIN_WITH : FilterOperator2::BEGINS_WITH;
                            // else: no asterisks, stick to equal/not equal
                        }
                    }
                }

                if( bValidOperator )
                {
                    // if wildcards are present, require RE mode, otherwise keep don't care state
                    if( lclConvertWildcardsToRegExp( aValue ) )
                        aSettings.mobNeedsRegExp = true;
                    // create a new UNO API filter field
                    aSettings.appendField( mbAnd, nOperator, aValue );
                }
            }
            else if( rCriterion.maValue.has< double >() )
            {
                // floating-point argument
                double fValue = 0.0;
                rCriterion.maValue >>= fValue;
                aSettings.appendField( mbAnd, nOperator, fValue );
            }
        }
    }
    return aSettings;
}

void CustomFilter::appendCriterion( const FilterCriterionModel& rCriterion )
{
    if( (rCriterion.mnOperator != XML_TOKEN_INVALID) && rCriterion.maValue.hasValue() )
        maCriteria.push_back( rCriterion );
}

FilterColumn::FilterColumn( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnColId( -1 ),
    mbHiddenButton( false ),
    mbShowButton( true )
{
}

void FilterColumn::importFilterColumn( const AttributeList& rAttribs )
{
    mnColId = rAttribs.getInteger( XML_colId, -1 );
    mbHiddenButton = rAttribs.getBool( XML_hiddenButton, false );
    mbShowButton = rAttribs.getBool( XML_showButton, true );
}

void FilterColumn::importFilterColumn( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    mnColId = rStrm.readInt32();
    nFlags = rStrm.readuInt16();
    mbHiddenButton = getFlag( nFlags, BIFF12_FILTERCOLUMN_HIDDENBUTTON );
    mbShowButton = getFlag( nFlags, BIFF12_FILTERCOLUMN_SHOWBUTTON );
}

ApiFilterSettings FilterColumn::finalizeImport()
{
    ApiFilterSettings aSettings;
    if( (0 <= mnColId) && mxSettings )
    {
        // filter settings object creates a sequence of filter fields
        aSettings = mxSettings->finalizeImport();
        // add column index to all filter fields
        for( auto& rFilterField : aSettings.maFilterFields )
            rFilterField.Field = mnColId;
    }
    return aSettings;
}

bool FilterColumn::isButtonHidden()
{
    return (mbShowButton == false) || (mbHiddenButton == true);
}

// SortCondition

SortCondition::SortCondition( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbDescending( false )
{
}

void SortCondition::importSortCondition( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    OUString aRangeStr = rAttribs.getString( XML_ref, OUString() );
    AddressConverter::convertToCellRangeUnchecked(maRange, aRangeStr, nSheet, getScDocument());

    maSortCustomList = rAttribs.getString( XML_customList, OUString() );
    mbDescending = rAttribs.getBool( XML_descending, false );
}

// AutoFilter

AutoFilter::AutoFilter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void AutoFilter::importAutoFilter( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    OUString aRangeStr = rAttribs.getString( XML_ref, OUString() );
    AddressConverter::convertToCellRangeUnchecked(maRange, aRangeStr, nSheet, getScDocument());
}

void AutoFilter::importAutoFilter( SequenceInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    AddressConverter::convertToCellRangeUnchecked( maRange, aBinRange, nSheet );
}

void AutoFilter::importSortState( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    OUString aRangeStr = rAttribs.getString( XML_ref, OUString() );
    AddressConverter::convertToCellRangeUnchecked(maSortRange, aRangeStr, nSheet, getScDocument());
}

FilterColumn& AutoFilter::createFilterColumn()
{
    FilterColumnVector::value_type xFilterColumn = std::make_shared<FilterColumn>( *this );
    maFilterColumns.push_back( xFilterColumn );
    return *xFilterColumn;
}

SortCondition& AutoFilter::createSortCondition()
{
    SortConditionVector::value_type xSortCondition = std::make_shared<SortCondition>( *this );
    maSortConditions.push_back( xSortCondition );
    return *xSortCondition;
}

void AutoFilter::finalizeImport( const Reference< XDatabaseRange >& rxDatabaseRange, sal_Int16 nSheet )
{
    // convert filter settings using the filter descriptor of the database range
    const Reference<XSheetFilterDescriptor3> xFilterDesc( rxDatabaseRange->getFilterDescriptor(), UNO_QUERY_THROW );
    if( !xFilterDesc.is() )
        return;

    // set some common properties for the auto filter range
    PropertySet aDescProps( xFilterDesc );
    aDescProps.setProperty( PROP_IsCaseSensitive, false );
    aDescProps.setProperty( PROP_SkipDuplicates, false );
    aDescProps.setProperty( PROP_Orientation, TableOrientation_ROWS );
    aDescProps.setProperty( PROP_ContainsHeader, true );
    aDescProps.setProperty( PROP_CopyOutputData, false );

    // resulting list of all UNO API filter fields
    ::std::vector<TableFilterField3> aFilterFields;

    // track if columns require to enable or disable regular expressions
    std::optional< bool > obNeedsRegExp;

    /*  Track whether the filter fields of the first filter column are
        connected with 'or'. In this case, other filter fields cannot be
        inserted without altering the result of the entire filter, due to
        Calc's precedence for the 'and' connection operator. Example:
        Excel's filter conditions 'A1 and (B1 or B2) and C1' where B1 and
        B2 belong to filter column B, will be evaluated by Calc as
        '(A1 and B1) or (B2 and C1)'. */
    bool bHasOrConnection = false;

    ScDocument& rDoc = getScDocument();
    SCCOL nCol = maRange.aStart.Col();
    SCROW nRow = maRange.aStart.Row();
    SCTAB nTab = maRange.aStart.Tab();

    // process all filter column objects, exit when 'or' connection exists
    for( const auto& rxFilterColumn : maFilterColumns )
    {
        // the filter settings object creates a list of filter fields
        ApiFilterSettings aSettings = rxFilterColumn->finalizeImport();
        ApiFilterSettings::FilterFieldVector& rColumnFields = aSettings.maFilterFields;

        if (rxFilterColumn->isButtonHidden())
        {
            auto nFlag = rDoc.GetAttr(nCol, nRow, nTab, ATTR_MERGE_FLAG)->GetValue();
            rDoc.ApplyAttr(nCol, nRow, nTab, ScMergeFlagAttr(nFlag & ~ScMF::Auto));
        }
        nCol++;

        /*  Check whether mode for regular expressions is compatible with
            the global mode in obNeedsRegExp. If either one is still in
            don't-care state, all is fine. If both are set, they must be
            equal. */
        bool bRegExpCompatible = !obNeedsRegExp.has_value() || !aSettings.mobNeedsRegExp.has_value() || (obNeedsRegExp.value() == aSettings.mobNeedsRegExp.value());

        // check whether fields are connected by 'or' (see comments above).
        if( rColumnFields.size() >= 2 )
            bHasOrConnection = std::any_of(rColumnFields.begin() + 1, rColumnFields.end(),
                [](const css::sheet::TableFilterField3& rColumnField) { return rColumnField.Connection == FilterConnection_OR; });

        /*  Skip the column filter, if no filter fields have been created,
            and if the mode for regular expressions of the
            filter column does not fit. */
        if( !rColumnFields.empty() && bRegExpCompatible )
        {
            /*  Add 'and' connection to the first filter field to connect
                it to the existing filter fields of other columns. */
            rColumnFields[ 0 ].Connection = FilterConnection_AND;

            // insert the new filter fields
            aFilterFields.insert( aFilterFields.end(), rColumnFields.begin(), rColumnFields.end() );

            // update the regular expressions mode
            assignIfUsed( obNeedsRegExp, aSettings.mobNeedsRegExp );
        }

        if( bHasOrConnection )
            break;
    }

    // insert all filter fields to the filter descriptor
    if( !aFilterFields.empty() )
        xFilterDesc->setFilterFields3( comphelper::containerToSequence( aFilterFields ) );

    // regular expressions
    bool bUseRegExp = obNeedsRegExp.value_or( false );
    aDescProps.setProperty( PROP_UseRegularExpressions, bUseRegExp );

    // sort
    if (maSortConditions.empty())
        return;

    const SortConditionVector::value_type& xSortConditionPointer = *maSortConditions.begin();
    const SortCondition& rSorConditionLoaded = *xSortConditionPointer;

    ScSortParam aParam;
    aParam.bUserDef = false;
    aParam.nUserIndex = 0;
    aParam.bByRow = false;

    ScUserList* pUserList = ScGlobal::GetUserList();
    if (!rSorConditionLoaded.maSortCustomList.isEmpty())
    {
        for (size_t i=0; pUserList && i < pUserList->size(); i++)
        {
            const OUString aEntry((*pUserList)[i].GetString());
            if (aEntry.equalsIgnoreAsciiCase(rSorConditionLoaded.maSortCustomList))
            {
                aParam.bUserDef = true;
                aParam.nUserIndex = i;
                break;
            }
        }
    }

    if (!aParam.bUserDef)
    {
        pUserList->emplace_back(rSorConditionLoaded.maSortCustomList);
        aParam.bUserDef = true;
        aParam.nUserIndex = pUserList->size()-1;
    }

    // set sort parameter if we have detected it
    if (!aParam.bUserDef)
        return;

    SCCOLROW nStartPos = aParam.bByRow ? maRange.aStart.Col() : maRange.aStart.Row();
    if (rSorConditionLoaded.mbDescending)
    {
        // descending sort - need to enable 1st SortParam slot
        assert(aParam.GetSortKeyCount() == DEFSORT);

        aParam.maKeyState[0].bDoSort = true;
        aParam.maKeyState[0].bAscending = false;
        aParam.maKeyState[0].nField += nStartPos;
    }

    ScDBData* pDBData = rDoc.GetDBAtArea(
        nSheet,
        maRange.aStart.Col(), maRange.aStart.Row(),
        maRange.aEnd.Col(), maRange.aEnd.Row());

    if (pDBData)
        pDBData->SetSortParam(aParam);
    else
        OSL_FAIL("AutoFilter::finalizeImport(): cannot find matching DBData");
}

AutoFilterBuffer::AutoFilterBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

AutoFilter& AutoFilterBuffer::createAutoFilter()
{
    AutoFilterVector::value_type xAutoFilter = std::make_shared<AutoFilter>( *this );
    maAutoFilters.push_back( xAutoFilter );
    return *xAutoFilter;
}

void AutoFilterBuffer::finalizeImport( sal_Int16 nSheet )
{
    // rely on existence of the defined name '_FilterDatabase' containing the range address of the filtered area
    const DefinedName* pFilterDBName = getDefinedNames().getByBuiltinId( BIFF_DEFNAME_FILTERDATABASE, nSheet ).get();
    if(!pFilterDBName)
        return;

    ScRange aFilterRange;
    if( !(pFilterDBName->getAbsoluteRange( aFilterRange ) && (aFilterRange.aStart.Tab() == nSheet)) )
        return;

    // use the same name for the database range as used for the defined name '_FilterDatabase'
    Reference< XDatabaseRange > xDatabaseRange = createUnnamedDatabaseRangeObject( aFilterRange );
    // first, try to create an auto filter
    bool bHasAutoFilter = finalizeImport( xDatabaseRange, nSheet );
    // no success: try to create an advanced filter
    if( bHasAutoFilter || !xDatabaseRange.is() )
        return;

    // the built-in defined name 'Criteria' must exist
    const DefinedName* pCriteriaName = getDefinedNames().getByBuiltinId( BIFF_DEFNAME_CRITERIA, nSheet ).get();
    if( !pCriteriaName )
        return;

    ScRange aCriteriaRange;
    if( !pCriteriaName->getAbsoluteRange( aCriteriaRange ) )
        return;

    // set some common properties for the filter descriptor
    PropertySet aDescProps( xDatabaseRange->getFilterDescriptor() );
    aDescProps.setProperty( PROP_IsCaseSensitive, false );
    aDescProps.setProperty( PROP_SkipDuplicates, false );
    aDescProps.setProperty( PROP_Orientation, TableOrientation_ROWS );
    aDescProps.setProperty( PROP_ContainsHeader, true );
    // criteria range may contain wildcards, but these are incompatible with REs
    aDescProps.setProperty( PROP_UseRegularExpressions, false );

    // position of output data (if built-in defined name 'Extract' exists)
    DefinedNameRef xExtractName = getDefinedNames().getByBuiltinId( BIFF_DEFNAME_EXTRACT, nSheet );
    ScRange aOutputRange;
    bool bHasOutputRange = xExtractName && xExtractName->getAbsoluteRange( aOutputRange );
    aDescProps.setProperty( PROP_CopyOutputData, bHasOutputRange );
    if( bHasOutputRange )
    {
        aDescProps.setProperty( PROP_SaveOutputPosition, true );
        aDescProps.setProperty( PROP_OutputPosition, CellAddress( aOutputRange.aStart.Tab(), aOutputRange.aStart.Col(), aOutputRange.aStart.Row() ) );
    }

    /*  Properties of the database range (must be set after
        modifying properties of the filter descriptor,
        otherwise the 'FilterCriteriaSource' property gets
        deleted). */
    PropertySet aRangeProps( xDatabaseRange );
    aRangeProps.setProperty( PROP_AutoFilter, false );
    aRangeProps.setProperty( PROP_FilterCriteriaSource,
                             CellRangeAddress( aCriteriaRange.aStart.Tab(),
                                               aCriteriaRange.aStart.Col(), aCriteriaRange.aStart.Row(),
                                               aCriteriaRange.aEnd.Col(), aCriteriaRange.aEnd.Row() ));
}

bool AutoFilterBuffer::finalizeImport( const Reference< XDatabaseRange >& rxDatabaseRange, sal_Int16 nSheet )
{
    AutoFilter* pAutoFilter = getActiveAutoFilter();
    if( pAutoFilter && rxDatabaseRange.is() ) try
    {
        // the property 'AutoFilter' enables the drop-down buttons
        PropertySet aRangeProps( rxDatabaseRange );
        aRangeProps.setProperty( PROP_AutoFilter, true );

        pAutoFilter->finalizeImport( rxDatabaseRange, nSheet );

        // return true to indicate enabled autofilter
        return true;
    }
    catch( Exception& )
    {
    }
    return false;
}

AutoFilter* AutoFilterBuffer::getActiveAutoFilter()
{
    // Excel expects not more than one auto filter per sheet or table
    OSL_ENSURE( maAutoFilters.size() <= 1, "AutoFilterBuffer::getActiveAutoFilter - too many auto filters" );
    // stick to the last imported auto filter
    return maAutoFilters.empty() ? nullptr : maAutoFilters.back().get();
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
