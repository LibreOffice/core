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

#include "oox/xls/autofilterbuffer.hxx"

#include <com/sun/star/sheet/FilterConnection.hpp>
#include <com/sun/star/sheet/FilterOperator2.hpp>
#include <com/sun/star/sheet/TableFilterField2.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor2.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/defnamesbuffer.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_uInt8 BIFF12_TOP10FILTER_TOP              = 0x01;
const sal_uInt8 BIFF12_TOP10FILTER_PERCENT          = 0x02;

const sal_uInt16 BIFF12_FILTERCOLUMN_HIDDENBUTTON   = 0x0001;
const sal_uInt16 BIFF12_FILTERCOLUMN_SHOWBUTTON     = 0x0002;

const sal_uInt16 BIFF_FILTERCOLUMN_OR               = 0x0001;
const sal_uInt16 BIFF_FILTERCOLUMN_TOP10FILTER      = 0x0010;
const sal_uInt16 BIFF_FILTERCOLUMN_TOP              = 0x0020;
const sal_uInt16 BIFF_FILTERCOLUMN_PERCENT          = 0x0040;

const sal_uInt8 BIFF_FILTER_DATATYPE_NONE           = 0;
const sal_uInt8 BIFF_FILTER_DATATYPE_RK             = 2;
const sal_uInt8 BIFF_FILTER_DATATYPE_DOUBLE         = 4;
const sal_uInt8 BIFF_FILTER_DATATYPE_STRING         = 6;
const sal_uInt8 BIFF_FILTER_DATATYPE_BOOLEAN        = 8;
const sal_uInt8 BIFF_FILTER_DATATYPE_EMPTY          = 12;
const sal_uInt8 BIFF_FILTER_DATATYPE_NOTEMPTY       = 14;

// ----------------------------------------------------------------------------

bool lclGetApiOperatorFromToken( sal_Int32& rnApiOperator, sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_lessThan:              rnApiOperator = FilterOperator2::NOT_EQUAL;     return true;
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
    if( (rValue.getLength() > 0) && ((rValue.indexOf( '*' ) >= 0) || (rValue.indexOf( '?' ) >= 0)) )
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
                    aBuffer.append( sal_Unicode( '.' ) );
                break;
                case '*':
                    aBuffer.append( sal_Unicode( '.' ) ).append( sal_Unicode( '*' ) );
                break;
                case '\\': case '.': case '|': case '(': case ')': case '^': case '$':
                    // quote RE meta characters
                    aBuffer.append( sal_Unicode( '\\' ) ).append( *pcChar );
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

// ============================================================================

ApiFilterSettings::ApiFilterSettings()
{
}

void ApiFilterSettings::appendField( bool bAnd, sal_Int32 nOperator, double fValue )
{
    maFilterFields.resize( maFilterFields.size() + 1 );
    TableFilterField2& rFilterField = maFilterFields.back();
    rFilterField.Connection = bAnd ? FilterConnection_AND : FilterConnection_OR;
    rFilterField.Operator = nOperator;
    rFilterField.IsNumeric = sal_True;
    rFilterField.NumericValue = fValue;
}

void ApiFilterSettings::appendField( bool bAnd, sal_Int32 nOperator, const OUString& rValue )
{
    maFilterFields.resize( maFilterFields.size() + 1 );
    TableFilterField2& rFilterField = maFilterFields.back();
    rFilterField.Connection = bAnd ? FilterConnection_AND : FilterConnection_OR;
    rFilterField.Operator = nOperator;
    rFilterField.IsNumeric = sal_False;
    rFilterField.StringValue = rValue;
}

// ============================================================================

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

void FilterSettingsBase::importBiffRecord( BiffInputStream& /*rStrm*/, sal_uInt16 /*nFlags*/ )
{
}

ApiFilterSettings FilterSettingsBase::finalizeImport( sal_Int32 /*nMaxCount*/ )
{
    return ApiFilterSettings();
}

// ============================================================================

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
            if( aValue.getLength() > 0 )
                maValues.push_back( aValue );
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
            rStrm >> nShowBlank >> nCalendarType;

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
            if( aValue.getLength() > 0 )
                maValues.push_back( aValue );
        }
        break;
    }
}

ApiFilterSettings DiscreteFilter::finalizeImport( sal_Int32 nMaxCount )
{
    ApiFilterSettings aSettings;
    if( static_cast< sal_Int32 >( maValues.size() ) <= nMaxCount )
    {
        aSettings.maFilterFields.reserve( maValues.size() );

        // insert all filter values
        for( FilterValueVector::iterator aIt = maValues.begin(), aEnd = maValues.end(); aIt != aEnd; ++aIt )
            aSettings.appendField( false, FilterOperator2::EQUAL, *aIt );

        // extra field for 'show empty'
        if( mbShowBlank )
            aSettings.appendField( false, FilterOperator2::EMPTY, OUString() );

        /*  Require disabled regular expressions, filter entries may contain
            any RE meta characters. */
        if( !maValues.empty() )
            aSettings.mobNeedsRegExp = false;
    }
    return aSettings;
}

// ============================================================================

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
        rStrm >> nFlags >> mfValue;
        mbTop = getFlag( nFlags, BIFF12_TOP10FILTER_TOP );
        mbPercent = getFlag( nFlags, BIFF12_TOP10FILTER_PERCENT );
    }
}

void Top10Filter::importBiffRecord( BiffInputStream& /*rStrm*/, sal_uInt16 nFlags )
{
    mfValue = extractValue< sal_uInt16 >( nFlags, 7, 9 );
    mbTop = getFlag( nFlags, BIFF_FILTERCOLUMN_TOP );
    mbPercent = getFlag( nFlags, BIFF_FILTERCOLUMN_PERCENT );
}

ApiFilterSettings Top10Filter::finalizeImport( sal_Int32 /*nMaxCount*/ )
{
    sal_Int32 nOperator = mbTop ?
        (mbPercent ? FilterOperator2::TOP_PERCENT : FilterOperator2::TOP_VALUES) :
        (mbPercent ? FilterOperator2::BOTTOM_PERCENT : FilterOperator2::BOTTOM_VALUES);
    ApiFilterSettings aSettings;
    aSettings.appendField( true, nOperator, mfValue );
    return aSettings;
}

// ============================================================================

FilterCriterionModel::FilterCriterionModel() :
    mnOperator( XML_equal ),
    mnDataType( BIFF_FILTER_DATATYPE_NONE ),
    mnStrLen( 0 )
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
    rStrm >> mnDataType >> nOperator;
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
            if( aValue.getLength() > 0 )
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

void FilterCriterionModel::readBiffData( BiffInputStream& rStrm )
{
    sal_uInt8 nOperator;
    rStrm >> mnDataType >> nOperator;
    setBiffOperator( nOperator );

    switch( mnDataType )
    {
        case BIFF_FILTER_DATATYPE_NONE:
            rStrm.skip( 8 );
        break;
        case BIFF_FILTER_DATATYPE_RK:
            maValue <<= BiffHelper::calcDoubleFromRk( rStrm.readInt32() );
            rStrm.skip( 4 );
        break;
        case BIFF_FILTER_DATATYPE_DOUBLE:
            maValue <<= rStrm.readDouble();
        break;
        case BIFF_FILTER_DATATYPE_STRING:
            rStrm.skip( 4 );
            rStrm >> mnStrLen;
            rStrm.skip( 3 );
        break;
        case BIFF_FILTER_DATATYPE_BOOLEAN:
        {
            sal_uInt8 nValue, nType;
            rStrm >> nValue >> nType;
            rStrm.skip( 6 );
            switch( nType )
            {
                case BIFF_BOOLERR_BOOL:     maValue <<= (nValue != 0);                              break;
                case BIFF_BOOLERR_ERROR:    maValue <<= BiffHelper::calcDoubleFromError( nValue );  break;
                default:                    OSL_ENSURE( false, "FilterCriterionModel::readBiffData - unknown type" );
            }
        }
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

void FilterCriterionModel::readString( BiffInputStream& rStrm, BiffType eBiff, rtl_TextEncoding eTextEnc )
{
    if( (mnDataType == BIFF_FILTER_DATATYPE_STRING) && (mnStrLen > 0) )
    {
        OUString aValue = (eBiff == BIFF8) ?
            rStrm.readUniStringBody( mnStrLen, true ) :
            rStrm.readCharArrayUC( mnStrLen, eTextEnc, true );
        aValue = aValue.trim();
        if( aValue.getLength() > 0 )
            maValue <<= aValue;
    }
}

// ----------------------------------------------------------------------------

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
            if( (aCriterion.mnOperator == XML_equal) || (aCriterion.mnOperator == XML_notEqual) || (aValue.getLength() > 0) )
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

void CustomFilter::importBiffRecord( BiffInputStream& rStrm, sal_uInt16 nFlags )
{
    mbAnd = !getFlag( nFlags, BIFF_FILTERCOLUMN_OR );

    FilterCriterionModel aCriterion1, aCriterion2;
    aCriterion1.readBiffData( rStrm );
    aCriterion2.readBiffData( rStrm );
    aCriterion1.readString( rStrm, getBiff(), getTextEncoding() );
    aCriterion2.readString( rStrm, getBiff(), getTextEncoding() );
    appendCriterion( aCriterion1 );
    appendCriterion( aCriterion2 );
}

ApiFilterSettings CustomFilter::finalizeImport( sal_Int32 /*nMaxCount*/ )
{
    ApiFilterSettings aSettings;
    OSL_ENSURE( maCriteria.size() <= 2, "CustomFilter::finalizeImport - too many filter criteria" );
    for( FilterCriterionVector::iterator aIt = maCriteria.begin(), aEnd = maCriteria.end(); aIt != aEnd; ++aIt )
    {
        // first extract the filter operator
        sal_Int32 nOperator = 0;
        bool bValidOperator = lclGetApiOperatorFromToken( nOperator, aIt->mnOperator );
        if( bValidOperator )
        {
            if( aIt->maValue.has< OUString >() )
            {
                // string argument
                OUString aValue;
                aIt->maValue >>= aValue;
                // check for 'empty', 'contains', 'begins with', or 'ends with' text filters
                bool bEqual = nOperator == FilterOperator2::EQUAL;
                bool bNotEqual = nOperator == FilterOperator2::NOT_EQUAL;
                if( bEqual || bNotEqual )
                {
                    if( aValue.getLength() == 0 )
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
                        bValidOperator = aValue.getLength() > 0;
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
            else if( aIt->maValue.has< double >() )
            {
                // floating-point argument
                double fValue = 0.0;
                aIt->maValue >>= fValue;
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

// ============================================================================

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
    rStrm >> mnColId >> nFlags;
    mbHiddenButton = getFlag( nFlags, BIFF12_FILTERCOLUMN_HIDDENBUTTON );
    mbShowButton = getFlag( nFlags, BIFF12_FILTERCOLUMN_SHOWBUTTON );
}

void FilterColumn::importFilterColumn( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags;
    mnColId = rStrm.readuInt16();
    rStrm >> nFlags;

    // BIFF5/BIFF8 support top-10 filters and custom filters
    if( getFlag( nFlags, BIFF_FILTERCOLUMN_TOP10FILTER ) )
        createFilterSettings< Top10Filter >().importBiffRecord( rStrm, nFlags );
    else
        createFilterSettings< CustomFilter >().importBiffRecord( rStrm, nFlags );
}

ApiFilterSettings FilterColumn::finalizeImport( sal_Int32 nMaxCount )
{
    ApiFilterSettings aSettings;
    if( (0 <= mnColId) && mxSettings.get() )
    {
        // filter settings object creates a sequence of filter fields
        aSettings = mxSettings->finalizeImport( nMaxCount );
        // add column index to all filter fields
        for( ApiFilterSettings::FilterFieldVector::iterator aIt = aSettings.maFilterFields.begin(), aEnd = aSettings.maFilterFields.end(); aIt != aEnd; ++aIt )
            aIt->Field = mnColId;
    }
    return aSettings;
}

// ============================================================================

AutoFilter::AutoFilter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void AutoFilter::importAutoFilter( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    OUString aRangeStr = rAttribs.getString( XML_ref, OUString() );
    getAddressConverter().convertToCellRangeUnchecked( maRange, aRangeStr, nSheet );
}

void AutoFilter::importAutoFilter( SequenceInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    getAddressConverter().convertToCellRangeUnchecked( maRange, aBinRange, nSheet );
}

FilterColumn& AutoFilter::createFilterColumn()
{
    FilterColumnVector::value_type xFilterColumn( new FilterColumn( *this ) );
    maFilterColumns.push_back( xFilterColumn );
    return *xFilterColumn;
}

void AutoFilter::finalizeImport( const Reference< XSheetFilterDescriptor2 >& rxFilterDesc )
{
    if( rxFilterDesc.is() )
    {
        // set some common properties for the auto filter range
        PropertySet aDescProps( rxFilterDesc );
        aDescProps.setProperty( PROP_IsCaseSensitive, false );
        aDescProps.setProperty( PROP_SkipDuplicates, false );
        aDescProps.setProperty( PROP_Orientation, TableOrientation_ROWS );
        aDescProps.setProperty( PROP_ContainsHeader, true );
        aDescProps.setProperty( PROP_CopyOutputData, false );

        // maximum number of UNO API filter fields
        sal_Int32 nMaxCount = 0;
        aDescProps.getProperty( nMaxCount, PROP_MaxFieldCount );
        OSL_ENSURE( nMaxCount > 0, "AutoFilter::finalizeImport - invalid maximum filter field count" );

        // resulting list of all UNO API filter fields
        ::std::vector< TableFilterField2 > aFilterFields;

        // track if columns require to enable or disable regular expressions
        OptValue< bool > obNeedsRegExp;

        /*  Track whether the filter fields of the first filter column are
            connected with 'or'. In this case, other filter fields cannot be
            inserted without altering the result of the entire filter, due to
            Calc's precedence for the 'and' connection operator. Example:
            Excel's filter conditions 'A1 and (B1 or B2) and C1' where B1 and
            B2 belong to filter column B, will be evaluated by Calc as
            '(A1 and B1) or (B2 and C1)'. */
        bool bHasOrConnection = false;

        // process all filter column objects, exit when 'or' connection exists
        for( FilterColumnVector::iterator aIt = maFilterColumns.begin(), aEnd = maFilterColumns.end(); !bHasOrConnection && (aIt != aEnd); ++aIt )
        {
            // the filter settings object creates a list of filter fields
            ApiFilterSettings aSettings = (*aIt)->finalizeImport( nMaxCount );
            ApiFilterSettings::FilterFieldVector& rColumnFields = aSettings.maFilterFields;

            // new total number of filter fields
            sal_Int32 nNewCount = static_cast< sal_Int32 >( aFilterFields.size() + rColumnFields.size() );

            /*  Check whether mode for regular expressions is compatible with
                the global mode in obNeedsRegExp. If either one is still in
                don't-care state, all is fine. If both are set, they must be
                equal. */
            bool bRegExpCompatible = !obNeedsRegExp || !aSettings.mobNeedsRegExp || (obNeedsRegExp.get() == aSettings.mobNeedsRegExp.get());

            // check whether fields are connected by 'or' (see comments above).
            if( rColumnFields.size() >= 2 )
                for( ApiFilterSettings::FilterFieldVector::iterator aSIt = rColumnFields.begin() + 1, aSEnd = rColumnFields.end(); !bHasOrConnection && (aSIt != aSEnd); ++aSIt )
                    bHasOrConnection = aSIt->Connection == FilterConnection_OR;

            /*  Skip the column filter, if no filter fields have been created,
                if the number of new filter fields would exceed the total limit
                of filter fields, or if the mode for regular expressions of the
                filter column does not fit. */
            if( !rColumnFields.empty() && (nNewCount <= nMaxCount) && bRegExpCompatible )
            {
                /*  Add 'and' connection to the first filter field to connect
                    it to the existing filter fields of other columns. */
                rColumnFields[ 0 ].Connection = FilterConnection_AND;

                // insert the new filter fields
                aFilterFields.insert( aFilterFields.end(), rColumnFields.begin(), rColumnFields.end() );

                // update the regular expressions mode
                obNeedsRegExp.assignIfUsed( aSettings.mobNeedsRegExp );
            }
        }

        // insert all filter fields to the filter descriptor
        if( !aFilterFields.empty() )
            rxFilterDesc->setFilterFields2( ContainerHelper::vectorToSequence( aFilterFields ) );

        // regular expressions
        bool bUseRegExp = obNeedsRegExp.get( false );
        aDescProps.setProperty( PROP_UseRegularExpressions, bUseRegExp );
    }
}

// ============================================================================

AutoFilterBuffer::AutoFilterBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

AutoFilter& AutoFilterBuffer::createAutoFilter()
{
    AutoFilterVector::value_type xAutoFilter( new AutoFilter( *this ) );
    maAutoFilters.push_back( xAutoFilter );
    return *xAutoFilter;
}

void AutoFilterBuffer::finalizeImport( sal_Int16 nSheet )
{
    // rely on existence of the defined name '_FilterDatabase' containing the range address of the filtered area
    if( const DefinedName* pFilterDBName = getDefinedNames().getByBuiltinId( BIFF_DEFNAME_FILTERDATABASE, nSheet ).get() )
    {
        CellRangeAddress aFilterRange;
        if( pFilterDBName->getAbsoluteRange( aFilterRange ) && (aFilterRange.Sheet == nSheet) )
        {
            // use the same name for the database range as used for the defined name '_FilterDatabase'
            OUString aDBRangeName = pFilterDBName->getCalcName();
            Reference< XDatabaseRange > xDatabaseRange = createDatabaseRangeObject( aDBRangeName, aFilterRange );
            // first, try to create an auto filter
            bool bHasAutoFilter = finalizeImport( xDatabaseRange );
            // no success: try to create an advanced filter
            if( !bHasAutoFilter && xDatabaseRange.is() )
            {
                // the built-in defined name 'Criteria' must exist
                if( const DefinedName* pCriteriaName = getDefinedNames().getByBuiltinId( BIFF_DEFNAME_CRITERIA, nSheet ).get() )
                {
                    CellRangeAddress aCriteriaRange;
                    if( pCriteriaName->getAbsoluteRange( aCriteriaRange ) )
                    {
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
                        CellRangeAddress aOutputRange;
                        bool bHasOutputRange = xExtractName.get() && xExtractName->getAbsoluteRange( aOutputRange );
                        aDescProps.setProperty( PROP_CopyOutputData, bHasOutputRange );
                        if( bHasOutputRange )
                        {
                            aDescProps.setProperty( PROP_SaveOutputPosition, true );
                            aDescProps.setProperty( PROP_OutputPosition, CellAddress( aOutputRange.Sheet, aOutputRange.StartColumn, aOutputRange.StartRow ) );
                        }

                        /*  Properties of the database range (must be set after
                            modifying properties of the filter descriptor,
                            otherwise the 'FilterCriteriaSource' property gets
                            deleted). */
                        PropertySet aRangeProps( xDatabaseRange );
                        aRangeProps.setProperty( PROP_AutoFilter, false );
                        aRangeProps.setProperty( PROP_FilterCriteriaSource, aCriteriaRange );
                    }
                }
            }
        }
    }
}

bool AutoFilterBuffer::finalizeImport( const Reference< XDatabaseRange >& rxDatabaseRange )
{
    AutoFilter* pAutoFilter = getActiveAutoFilter();
    if( pAutoFilter && rxDatabaseRange.is() ) try
    {
        // the property 'AutoFilter' enables the drop-down buttons
        PropertySet aRangeProps( rxDatabaseRange );
        aRangeProps.setProperty( PROP_AutoFilter, true );
        // convert filter settings using the filter descriptor of the database range
        Reference< XSheetFilterDescriptor2 > xFilterDesc( rxDatabaseRange->getFilterDescriptor(), UNO_QUERY_THROW );
        pAutoFilter->finalizeImport( xFilterDesc );
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
    return maAutoFilters.empty() ? 0 : maAutoFilters.back().get();
}

// ============================================================================

} // namespace xls
} // namespace oox
