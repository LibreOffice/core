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

#include <tablebuffer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <addressconverter.hxx>
#include <biffhelper.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;

TableModel::TableModel() :
    mnId( -1 ),
    mnType( XML_worksheet ),
    mnHeaderRows( 1 ),
    mnTotalsRows( 0 )
{
}

Table::Table( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maAutoFilters( rHelper ),
    maTableColumns( rHelper ),
    mnTokenIndex( -1 )
{
}

void Table::importTable( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    AddressConverter::convertToCellRangeUnchecked( maModel.maRange, rAttribs.getString( XML_ref, OUString() ), nSheet );
    maModel.maProgName    = rAttribs.getXString( XML_name, OUString() );
    maModel.maDisplayName = rAttribs.getXString( XML_displayName, OUString() );
    maModel.mnId          = rAttribs.getInteger( XML_id, -1 );
    maModel.mnType        = rAttribs.getToken( XML_tableType, XML_worksheet );
    maModel.mnHeaderRows  = rAttribs.getInteger( XML_headerRowCount, 1 );
    maModel.mnTotalsRows  = rAttribs.getInteger( XML_totalsRowCount, 0 );
}

void Table::importTable( SequenceInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    sal_Int32 nType;
    rStrm >> aBinRange;
    nType = rStrm.readInt32();
    maModel.mnId = rStrm.readInt32();
    maModel.mnHeaderRows = rStrm.readInt32();
    maModel.mnTotalsRows = rStrm.readInt32();
    rStrm.skip( 32 );
    rStrm >> maModel.maProgName >> maModel.maDisplayName;

    AddressConverter::convertToCellRangeUnchecked( maModel.maRange, aBinRange, nSheet );
    static const sal_Int32 spnTypes[] = { XML_worksheet, XML_TOKEN_INVALID, XML_TOKEN_INVALID, XML_queryTable };
    maModel.mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
}

void Table::finalizeImport()
{
    // Create database range.  Note that Excel 2007 and later names database
    // ranges (or tables in their terminology) as Table1, Table2 etc.  We need
    // to import them as named db ranges because they may be referenced by
    // name in formula expressions.
    if( (maModel.mnId > 0) && !maModel.maDisplayName.isEmpty() ) try
    {
        maDBRangeName = maModel.maDisplayName;

        Reference< XDatabaseRange > xDatabaseRange(
            createDatabaseRangeObject( maDBRangeName, maModel.maRange ), UNO_SET_THROW);
        ::css::table::CellRangeAddress aAddressRange = xDatabaseRange->getDataArea();
        maDestRange = ScRange( aAddressRange.StartColumn, aAddressRange.StartRow, aAddressRange.Sheet,
                               aAddressRange.EndColumn, aAddressRange.EndRow, aAddressRange.Sheet );

        PropertySet aPropSet( xDatabaseRange );

        // Default HasHeader is true at ScDBData.
        if (maModel.mnHeaderRows != 1)
        {
            SAL_WARN_IF( maModel.mnHeaderRows > 1, "sc.filter",
                    "Table HeaderRows > 1 not supported: " << maModel.mnHeaderRows);
            if (maModel.mnHeaderRows == 0)
                aPropSet.setProperty( PROP_ContainsHeader, false);
        }

        if (maModel.mnTotalsRows > 0)
        {
            SAL_WARN_IF( maModel.mnTotalsRows > 1, "sc.filter",
                    "Table TotalsRows > 1 not supported: " << maModel.mnTotalsRows);
            aPropSet.setProperty( PROP_TotalsRow, true);
        }

        // get formula token index of the database range
        if( !aPropSet.getProperty( mnTokenIndex, PROP_TokenIndex ) )
            mnTokenIndex = -1;
    }
    catch( Exception& )
    {
        OSL_FAIL( "Table::finalizeImport - cannot create database range" );
    }
}

void Table::applyAutoFilters()
{
    if( !maDBRangeName.isEmpty() )
    {
        try
        {
            // get the range ( maybe we should cache the xDatabaseRange from finalizeImport )
            PropertySet aDocProps( getDocument() );
            Reference< XDatabaseRanges > xDatabaseRanges( aDocProps.getAnyProperty( PROP_DatabaseRanges ), UNO_QUERY_THROW );
            Reference< XDatabaseRange > xDatabaseRange( xDatabaseRanges->getByName( maDBRangeName ), UNO_QUERY );
            maAutoFilters.finalizeImport( xDatabaseRange );
        }
        catch( Exception& )
        {
            OSL_FAIL( "Table::applyAutofilters - cannot create filter" );
        }
    }
}

void Table::applyTableColumns()
{
    maTableColumns.finalizeImport( findDatabaseRangeByIndex( mnTokenIndex ));
}

TableBuffer::TableBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

Table& TableBuffer::createTable()
{
    TableVector::value_type xTable( new Table( *this ) );
    maTables.push_back( xTable );
    return *xTable;
}

void TableBuffer::finalizeImport()
{
    // map all tables by identifier and display name
    for( const auto& rxTable : maTables )
        insertTableToMaps( rxTable );
    // finalize all valid tables
    maIdTables.forEachMem( &Table::finalizeImport );
}

void TableBuffer::applyAutoFilters()
{
    maIdTables.forEachMem( &Table::applyAutoFilters );
}

void TableBuffer::applyTableColumns()
{
    maIdTables.forEachMem( &Table::applyTableColumns );
}

TableRef TableBuffer::getTable( sal_Int32 nTableId ) const
{
    return maIdTables.get( nTableId );
}

TableRef TableBuffer::getTable( const OUString& rDispName ) const
{
    return maNameTables.get( rDispName );
}

// private --------------------------------------------------------------------

void TableBuffer::insertTableToMaps( const TableRef& rxTable )
{
    sal_Int32 nTableId = rxTable->getTableId();
    const OUString& rDispName = rxTable->getDisplayName();
    if( (nTableId > 0) && !rDispName.isEmpty() )
    {
        OSL_ENSURE( !maIdTables.has( nTableId ), "TableBuffer::insertTableToMaps - multiple table identifier" );
        maIdTables[ nTableId ] = rxTable;
        OSL_ENSURE( !maNameTables.has( rDispName ), "TableBuffer::insertTableToMaps - multiple table name" );
        maNameTables[ rDispName ] = rxTable;
    }
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
