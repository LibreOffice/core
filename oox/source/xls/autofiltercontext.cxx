/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: autofiltercontext.cxx,v $
 * $Revision: 1.5.4.1 $
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

#include "oox/xls/autofiltercontext.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/FilterOperator.hpp>
#include <com/sun/star/sheet/FilterConnection.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/addressconverter.hxx"

#define DEBUG_OOX_AUTOFILTER 0

#if USE_SC_MULTI_STRING_FILTER_PATCH
#include <com/sun/star/sheet/XExtendedSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/TableFilterFieldNormal.hpp>
#include <com/sun/star/sheet/TableFilterFieldMultiString.hpp>
using ::com::sun::star::sheet::TableFilterFieldNormal;
using ::com::sun::star::sheet::TableFilterFieldMultiString;
using ::com::sun::star::sheet::XExtendedSheetFilterDescriptor;
#else
#include <com/sun/star/sheet/TableFilterField.hpp>
using ::com::sun::star::sheet::TableFilterField;
#endif

#if DEBUG_OOX_AUTOFILTER
#include <stdio.h>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::XCellRange;
using ::com::sun::star::sheet::XDatabaseRange;
using ::com::sun::star::sheet::XDatabaseRanges;
using ::com::sun::star::sheet::XSheetFilterDescriptor;
using ::com::sun::star::i18n::LocaleDataItem;
using ::com::sun::star::i18n::XLocaleData;
using ::com::sun::star::lang::Locale;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

// ============================================================================

FilterFieldItem::FilterFieldItem() :
#if USE_SC_MULTI_STRING_FILTER_PATCH
    mpField(new TableFilterFieldNormal),
#else
    mpField(new TableFilterField),
#endif
    meType(NORMAL)
{
}

FilterFieldItem::FilterFieldItem(Type eType) :
    meType(eType)
{
#if USE_SC_MULTI_STRING_FILTER_PATCH
    switch ( eType )
    {
        case MULTI_STRING:
            mpField.reset(new TableFilterFieldMultiString);
        break;
        case NORMAL:
            mpField.reset(new TableFilterFieldNormal);
        break;
        default:
            mpField.reset(new TableFilterFieldNormal);
    }
#else
    mpField.reset(new TableFilterField);
    meType = NORMAL;
#endif
}

// ============================================================================

OoxAutoFilterContext::OoxAutoFilterContext( OoxWorksheetFragmentBase& rFragment ) :
    OoxWorksheetContextBase( rFragment ),
    mbValidAddress( false ),
    mbUseRegex( false ),
    mbShowBlank( false ),
    mbConnectionAnd( false )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxAutoFilterContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( autoFilter ):
            switch( nElement )
            {
                case XLS_TOKEN( filterColumn ):     return this;
            }
        break;

        case XLS_TOKEN( filterColumn ):
            switch( nElement )
            {
                case XLS_TOKEN( filters ):
                case XLS_TOKEN( customFilters ):
                case XLS_TOKEN( top10 ):
                case XLS_TOKEN( dynamicFilter ):    return this;
            }
        break;

        case XLS_TOKEN( filters ):
            switch( nElement )
            {
                case XLS_TOKEN( filter ):           return this;
            }
        break;

        case XLS_TOKEN( customFilters ):
            switch( nElement )
            {
                case XLS_TOKEN( customFilter ):     return this;
            }
        break;
    }
    return 0;
}

void OoxAutoFilterContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( autoFilter ):
            importAutoFilter( rAttribs );
        break;
        case XLS_TOKEN( filterColumn ):
            if ( mbValidAddress )
                importFilterColumn( rAttribs );
        break;
        case XLS_TOKEN( filters ):
            if ( mbValidAddress )
                importFilters( rAttribs );
        break;
        case XLS_TOKEN( filter ):
            if ( mbValidAddress )
                importFilter( rAttribs );
        break;
        case XLS_TOKEN( customFilters ):
            if ( mbValidAddress )
                importCustomFilters( rAttribs );
        break;
        case XLS_TOKEN( customFilter ):
            if ( mbValidAddress )
                importCustomFilter( rAttribs );
        break;
        case XLS_TOKEN( top10 ):
            if ( mbValidAddress )
                importTop10( rAttribs );
        break;
        case XLS_TOKEN( dynamicFilter ):
            if ( mbValidAddress )
                importDynamicFilter( rAttribs );
        break;
    }
}

void OoxAutoFilterContext::onEndElement( const OUString& /*rChars*/ )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( autoFilter ):
            maybeShowBlank();
            setAutoFilter();
        break;
        case XLS_TOKEN( filters ):
            setFilterNames();
        break;
    }
}

#if DEBUG_OOX_AUTOFILTER
static void lclPrintNormalField(
#if USE_SC_MULTI_STRING_FILTER_PATCH
    TableFilterFieldNormal* pField
#else
    TableFilterField* pField
#endif
)
{
    using namespace ::com::sun::star::sheet;

    printf("  Operator: ");
    switch ( pField->Operator )
    {
        case FilterOperator_EQUAL:
            printf("EQUAL");
            break;
        case FilterOperator_NOT_EQUAL:
            printf("NOT_EQUAL");
            break;
        case com::sun::star::sheet::FilterOperator_GREATER:
            printf("GREATER");
            break;
        case com::sun::star::sheet::FilterOperator_GREATER_EQUAL:
            printf("GREATER_EQUAL");
            break;
        case FilterOperator_LESS:
            printf("LESS");
            break;
        case FilterOperator_LESS_EQUAL:
            printf("LESS_EQUAL");
            break;
        case FilterOperator_NOT_EMPTY:
            printf("NOT_EMPTY");
            break;
        case FilterOperator_EMPTY:
            printf("EMPTY");
            break;
        case FilterOperator_BOTTOM_PERCENT:
            printf("BOTTOM_PERCENT");
            break;
        case FilterOperator_BOTTOM_VALUES:
            printf("BOTTOM_VALUES");
            break;
        case FilterOperator_TOP_PERCENT:
            printf("TOP_PERCENT");
            break;
        case FilterOperator_TOP_VALUES:
            printf("TOP_VALUES");
            break;
        default:
            printf("other");
    }
    printf("\n");

    printf("  StringValue: %s\n",
           OUStringToOString(pField->StringValue, RTL_TEXTENCODING_UTF8).getStr());

    printf("  NumericValue: %g\n", pField->NumericValue);

    printf("  IsNumeric: ");
    if (pField->IsNumeric)
        printf("yes\n");
    else
        printf("no\n");
}

static void lclPrintFieldConnection( ::com::sun::star::sheet::FilterConnection eConn )
{
    using namespace ::com::sun::star::sheet;

    printf("  Connection: ");
    switch ( eConn )
    {
        case FilterConnection_AND:
            printf("AND");
            break;
        case FilterConnection_OR:
            printf("OR");
            break;
        case FilterConnection_MAKE_FIXED_SIZE:
            printf("MAKE_FIXED_SIZE");
            break;
        default:
            printf("other");
    }
    printf("\n");
}

static void lclPrintFilterField( const FilterFieldItem& aItem )
{
    using namespace ::com::sun::star::sheet;

    printf("----------------------------------------\n");
#if USE_SC_MULTI_STRING_FILTER_PATCH
    {
        // Print common fields first.

        TableFilterFieldBase* pField = aItem.mpField.get();
        printf("  Field: %ld\n", pField->Field);
        lclPrintFieldConnection(pField->Connection);
    }
    switch ( aItem.meType )
    {
        case FilterFieldItem::NORMAL:
        {
            TableFilterFieldNormal* pField = static_cast<TableFilterFieldNormal*>(aItem.mpField.get());
            lclPrintNormalField(pField);
        }
        break;
        case FilterFieldItem::MULTI_STRING:
        {
            TableFilterFieldMultiString* pMultiStrField = static_cast<TableFilterFieldMultiString*>(aItem.mpField.get());
            sal_Int32 nSize = pMultiStrField->StringSet.getLength();
            printf("  StringSet:\n");
            for ( sal_Int32 i = 0; i < nSize; ++i )
            {
                printf("    * %s\n",
                       OUStringToOString(pMultiStrField->StringSet[i], RTL_TEXTENCODING_UTF8).getStr());
            }
        }
        break;
    }
#else
    TableFilterField* pField = aItem.mpField.get();
    printf("  Field: %ld\n", pField->Field);
    lclPrintFieldConnection(pField->Connection);
    lclPrintNormalField(pField);

#endif
    fflush(stdout);
}
#endif

void OoxAutoFilterContext::initialize()
{
    maFields.clear();
    maFilterNames.clear();
    mbValidAddress = mbShowBlank = mbUseRegex = mbConnectionAnd = false;
}

void OoxAutoFilterContext::setAutoFilter()
{
    using namespace ::com::sun::star::sheet;

    // Name this built-in database.
    OUStringBuffer sDataAreaNameBuf( CREATE_OUSTRING("Excel_BuiltIn__FilterDatabase_ ") );
    sDataAreaNameBuf.append( static_cast<sal_Int32>(getSheetIndex()+1) );

    OUString sDataAreaName = sDataAreaNameBuf.makeStringAndClear();
    Reference< XCellRange > xCellRange = getCellRange( maAutoFilterRange );

    // Create a new database range, add filters to it and refresh the database
    // for that to take effect.

    Reference< XDatabaseRanges > xDBRanges = getDatabaseRanges();
    if ( !xDBRanges.is() )
    {
        OSL_ENSURE( false, "OoxAutoFilterContext::setAutoFilter: DBRange empty" );
        return;
    }

    Reference< XNameAccess > xNA( xDBRanges, UNO_QUERY_THROW );
    if ( !xNA->hasByName( sDataAreaName ) )
        xDBRanges->addNewByName( sDataAreaName, maAutoFilterRange );

    Reference< XDatabaseRange > xDB( xNA->getByName( sDataAreaName ), UNO_QUERY );
    if ( xDB.is() )
    {
        PropertySet aProp( xDB );
        aProp.setProperty( PROP_AutoFilter, true );
    }

    sal_Int32 nSize = maFields.size();
    sal_Int32 nMaxFieldCount = nSize;
    Reference< XSheetFilterDescriptor > xDescriptor = xDB->getFilterDescriptor();
    if ( xDescriptor.is() )
    {
        PropertySet aProp( xDescriptor );
        aProp.setProperty( PROP_ContainsHeader, true );
        aProp.setProperty( PROP_UseRegularExpressions, mbUseRegex );
        aProp.getProperty( nMaxFieldCount, PROP_MaxFieldCount );
    }
    else
    {
        OSL_ENSURE(false, "OoxAutoFilterContext::setAutoFilter: descriptor is empty");
        return;
    }

    // Unpack all column field items into a sequence.
#if USE_SC_MULTI_STRING_FILTER_PATCH
    Reference< XExtendedSheetFilterDescriptor > xExtDescriptor( xDescriptor, UNO_QUERY );
    if ( !xExtDescriptor.is() )
    {
        OSL_ENSURE(false, "OoxAutoFilterContext::setAutoFilter: extended descriptor is empty");
        return;
    }

    xExtDescriptor->begin();

    ::std::list< FilterFieldItem >::const_iterator itr = maFields.begin(), itrEnd = maFields.end();
    for (sal_Int32 i = 0; itr != itrEnd && i < nMaxFieldCount; ++itr, ++i)
    {
#if DEBUG_OOX_AUTOFILTER
        lclPrintFilterField(*itr);
#endif
        switch ( itr->meType )
        {
            case oox::xls::FilterFieldItem::MULTI_STRING:
            {
                // multi-string filter type
                TableFilterFieldMultiString* pField = static_cast<TableFilterFieldMultiString*>( itr->mpField.get() );
                xExtDescriptor->addFilterFieldMultiString( *pField );
            }
            break;
            case oox::xls::FilterFieldItem::NORMAL:
            default:
                // normal filter type
                TableFilterFieldNormal* pField = static_cast<TableFilterFieldNormal*>( itr->mpField.get() );
                xExtDescriptor->addFilterFieldNormal( *pField );
        }
    }
    xExtDescriptor->commit();

#else
    Sequence< TableFilterField > aFields(nSize);
    ::std::list< FilterFieldItem >::const_iterator itr = maFields.begin(), itrEnd = maFields.end();
    for (sal_Int32 i = 0; itr != itrEnd && i < nMaxFieldCount; ++itr, ++i)
    {
#if DEBUG_OOX_AUTOFILTER
        lclPrintFilterField( *itr );
#endif
        aFields[i] = *itr->mpField;
    }
    xDescriptor->setFilterFields( aFields );
#endif
    xDB->refresh();
}

void OoxAutoFilterContext::maybeShowBlank()
{
    using namespace ::com::sun::star::sheet;

    if ( !mbShowBlank )
        return;

#if USE_SC_MULTI_STRING_FILTER_PATCH
    FilterFieldItem aItem(FilterFieldItem::NORMAL);
    TableFilterFieldNormal* pField = static_cast<TableFilterFieldNormal*>(aItem.mpField.get());
    pField->Field = mnCurColID;
    pField->Operator = FilterOperator_EMPTY;
    pField->Connection = FilterConnection_AND;
    pField->IsNumeric = false;
#else
    FilterFieldItem aItem;
    aItem.mpField->Field = mnCurColID;
    aItem.mpField->Operator = FilterOperator_EMPTY;
    aItem.mpField->Connection = FilterConnection_AND;
    aItem.mpField->IsNumeric = false;
#endif
    maFields.push_back(aItem);
}

void OoxAutoFilterContext::setFilterNames()
{
    using namespace ::com::sun::star::sheet;


    sal_Int32 size = maFilterNames.size();
    if ( !size )
        return;

#if USE_SC_MULTI_STRING_FILTER_PATCH
    Sequence< OUString > aStrList(size);
    ::std::list< OUString >::const_iterator itr = maFilterNames.begin(), itrEnd = maFilterNames.end();
    for (sal_Int32 i = 0; itr != itrEnd; ++itr, ++i)
        aStrList[i] = *itr;

    FilterFieldItem aItem(FilterFieldItem::MULTI_STRING);
    TableFilterFieldMultiString* pField = static_cast<TableFilterFieldMultiString*>( aItem.mpField.get() );
    pField->Field = mnCurColID;
    pField->Connection = FilterConnection_AND;
    pField->StringSet = aStrList;

    maFields.push_back(aItem);
#else
    static const OUString sSep = CREATE_OUSTRING("|");

    OUStringBuffer buf;
    if ( size > 1 )
    {
        buf.append( CREATE_OUSTRING("^(") );
        mbUseRegex = true;
    }

    ::std::list< OUString >::const_iterator itr = maFilterNames.begin(), itrEnd = maFilterNames.end();
    bool bFirst = true;
    for (; itr != itrEnd; ++itr)
    {
        if (bFirst)
            bFirst = false;
        else
            buf.append( sSep );
        buf.append( *itr );
    }
    if ( size > 1 )
        buf.append( CREATE_OUSTRING(")$") );

    FilterFieldItem aItem;
    aItem.mpField->Field = mnCurColID;
    aItem.mpField->StringValue = buf.makeStringAndClear();
    aItem.mpField->Operator = FilterOperator_EQUAL;
    aItem.mpField->Connection = FilterConnection_AND;
    aItem.mpField->IsNumeric = false;
    maFields.push_back(aItem);
#endif
}

void OoxAutoFilterContext::importAutoFilter( const AttributeList& rAttribs )
{
    initialize();

    mbValidAddress = getAddressConverter().convertToCellRange(
        maAutoFilterRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true, true );
}

void OoxAutoFilterContext::importFilterColumn( const AttributeList& rAttribs )
{
    // hiddenButton and showButton attributes are not used for now.
    mnCurColID = rAttribs.getInteger( XML_colId, -1 );
}

void OoxAutoFilterContext::importTop10( const AttributeList& rAttribs )
{
    using namespace ::com::sun::star::sheet;

    // filterVal attribute is not necessarily, since Calc also supports top 10
    // and top 10% filter type.
    FilterFieldItem aItem;
#if USE_SC_MULTI_STRING_FILTER_PATCH
    TableFilterFieldNormal* pField = static_cast<TableFilterFieldNormal*>(aItem.mpField.get());
#else
    TableFilterField* pField = aItem.mpField.get();
#endif
    pField->Field = mnCurColID;

    bool bPercent = rAttribs.getBool( XML_percent, false );
    bool bTop = rAttribs.getBool( XML_top, true );
    pField->NumericValue = rAttribs.getDouble( XML_val, 0.0 );
    pField->IsNumeric = true;

    // When top10 filter item is present, that's the only filter item for that column.
    if ( bTop )
        if ( bPercent )
            pField->Operator = FilterOperator_TOP_PERCENT;
        else
            pField->Operator = FilterOperator_TOP_VALUES;
    else
        if ( bPercent )
            pField->Operator = FilterOperator_BOTTOM_PERCENT;
        else
            pField->Operator = FilterOperator_BOTTOM_VALUES;

    maFields.push_back(aItem);
}

void OoxAutoFilterContext::importCustomFilters( const AttributeList& rAttribs )
{
    // OR is default when the 'and' attribute is absent.
    mbConnectionAnd = rAttribs.getBool( XML_and, false );
}

/** Do a best-effort guess of whether or not the given string is numerical. */
static bool lclIsNumeric( const OUString& _str, const LocaleDataItem& aLocaleItem )
{
    OUString str = _str.trim();
    sal_Int32 size = str.getLength();

    if ( !size )
        // Empty string.  This can't be a number.
        return false;

    // Get the decimal separator for the current locale.
    const OUString& sep = aLocaleItem.decimalSeparator;

    bool bDecimalSep = false;
    for (sal_Int32 i = 0; i < size; ++i)
    {
        OUString c = str.copy(i, 1);
        if ( !c.compareTo(sep) )
        {
            if ( bDecimalSep )
                return false;
            else
            {
                bDecimalSep = true;
                continue;
            }
        }
        if ( (0 > c.compareToAscii("0") || 0 < c.compareToAscii("9")) )
            return false;
    }

    return true;
}

/** Convert wildcard characters to regex equivalent. Returns true if any
    wildcard character is found. */
static bool lclWildcard2Regex( OUString& str )
{
    bool bWCFound = false;
    OUStringBuffer buf;
    sal_Int32 size = str.getLength();
    buf.ensureCapacity(size + 6); // pure heuristics.

    sal_Unicode dot = '.', star = '*', hat = '^', dollar = '$';
    buf.append(hat);
    for (sal_Int32 i = 0; i < size; ++i)
    {
        OUString c = str.copy(i, 1);
        if ( !c.compareToAscii("?") )
        {
            buf.append(dot);
            bWCFound = true;
        }
        else if ( !c.compareToAscii("*") )
        {
            buf.append(dot);
            buf.append(star);
            bWCFound = true;
        }
        else
            buf.append(c);
    }
    buf.append(dollar);

    if (bWCFound)
        str = buf.makeStringAndClear();

    return bWCFound;
}

/** Translate Excel's filter operator to Calc's. */
static ::com::sun::star::sheet::FilterOperator lclTranslateFilterOp( sal_Int32 nToken )
{
    using namespace ::com::sun::star::sheet;

    switch ( nToken )
    {
        case XML_equal:
            return FilterOperator_EQUAL;
        case XML_notEqual:
            return FilterOperator_NOT_EQUAL;
        case XML_greaterThan:
            return FilterOperator_GREATER;
        case XML_greaterThanOrEqual:
            return FilterOperator_GREATER_EQUAL;
        case XML_lessThan:
            return FilterOperator_LESS;
        case XML_lessThanOrEqual:
            return FilterOperator_LESS_EQUAL;
    }
    return FilterOperator_EQUAL;
}

void OoxAutoFilterContext::importCustomFilter( const AttributeList& rAttribs )
{
    using namespace ::com::sun::star::sheet;

    sal_Int32 nToken = rAttribs.getToken( XML_operator, XML_equal );
#if USE_SC_MULTI_STRING_FILTER_PATCH
    FilterFieldItem aItem(FilterFieldItem::NORMAL);
    TableFilterFieldNormal* pField = static_cast<TableFilterFieldNormal*>(aItem.mpField.get());
#else
    FilterFieldItem aItem;
    TableFilterField* pField = aItem.mpField.get();
#endif
    pField->Field = mnCurColID;
    pField->StringValue = rAttribs.getString( XML_val, OUString() );
    pField->NumericValue = pField->StringValue.toDouble();
    pField->Operator = lclTranslateFilterOp( nToken );

    if ( nToken == XML_notEqual && !pField->StringValue.compareToAscii(" ") )
    {
        // Special case for hiding blanks.  Excel translates "hide blanks" to
        // (filter if notEqual " ").  So, we need to translate it back.
        pField->Operator = FilterOperator_NOT_EMPTY;
        pField->IsNumeric = false;
        maFields.push_back(aItem);
        return;
    }

    switch ( nToken )
    {
        case XML_equal:
        case XML_notEqual:
        {
            Reference< XLocaleData > xLocale( getGlobalFactory()->createInstance(
                CREATE_OUSTRING("com.sun.star.i18n.LocaleData") ), UNO_QUERY );

            if ( !xLocale.is() )
                return;

            LocaleDataItem aLocaleItem = xLocale->getLocaleItem( ::com::sun::star::lang::Locale() );
            pField->IsNumeric = lclIsNumeric(pField->StringValue, aLocaleItem);

            if ( !pField->IsNumeric && lclWildcard2Regex(pField->StringValue) )
                mbUseRegex = true;

            maFields.push_back(aItem);
        }
        break;

        case XML_greaterThan:
        case XML_greaterThanOrEqual:
        case XML_lessThan:
        case XML_lessThanOrEqual:
        {
            pField->IsNumeric = true;
            maFields.push_back(aItem);
        }
        break;
        default:
            OSL_ENSURE( false, "OoxAutoFilterContext::importCustomFilter: unhandled case" );
    }
}

void OoxAutoFilterContext::importFilters( const AttributeList& rAttribs )
{
    // blank (boolean) and calendarType attributes can be present, but not used for now.

    mbShowBlank = rAttribs.getBool( XML_blank, false );
    maFilterNames.clear();
}

void OoxAutoFilterContext::importFilter( const AttributeList& rAttribs )
{
    if (mnCurColID == -1)
        return;

    OUString value = rAttribs.getString( XML_val, OUString() );
    if ( value.getLength() )
        maFilterNames.push_back(value);
}

void OoxAutoFilterContext::importDynamicFilter( const AttributeList& /*rAttribs*/ )
{
    // not implemented yet - Calc doesn't support this.
}

// ============================================================================

} // namespace xls
} // namespace oox

