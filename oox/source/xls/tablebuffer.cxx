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

#include "oox/xls/tablebuffer.hxx"

#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/addressconverter.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

TableModel::TableModel() :
    mnId( -1 ),
    mnType( XML_worksheet ),
    mnHeaderRows( 1 ),
    mnTotalsRows( 0 )
{
}

// ============================================================================

Table::Table( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maAutoFilters( rHelper ),
    mnTokenIndex( -1 )
{
}

void Table::importTable( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    getAddressConverter().convertToCellRangeUnchecked( maModel.maRange, rAttribs.getString( XML_ref, OUString() ), nSheet );
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
    rStrm >> aBinRange >> nType >> maModel.mnId >> maModel.mnHeaderRows >> maModel.mnTotalsRows;
    rStrm.skip( 32 );
    rStrm >> maModel.maProgName >> maModel.maDisplayName;

    getAddressConverter().convertToCellRangeUnchecked( maModel.maRange, aBinRange, nSheet );
    static const sal_Int32 spnTypes[] = { XML_worksheet, XML_TOKEN_INVALID, XML_TOKEN_INVALID, XML_queryTable };
    maModel.mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
}

void Table::finalizeImport()
{
    // create database range
    if( (maModel.mnId > 0) && (maModel.maDisplayName.getLength() > 0) ) try
    {
        maDBRangeName = maModel.maDisplayName;
        Reference< XDatabaseRange > xDatabaseRange( createDatabaseRangeObject( maDBRangeName, maModel.maRange ), UNO_SET_THROW );
        maDestRange = xDatabaseRange->getDataArea();

        // get formula token index of the database range
        PropertySet aPropSet( xDatabaseRange );
        if( !aPropSet.getProperty( mnTokenIndex, PROP_TokenIndex ) )
            mnTokenIndex = -1;

        // filter settings
        maAutoFilters.finalizeImport( xDatabaseRange );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "Table::finalizeImport - cannot create database range" );
    }
}

// ============================================================================

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
    for( TableVector::iterator aIt = maTables.begin(), aEnd = maTables.end(); aIt != aEnd; ++aIt )
        insertTableToMaps( *aIt );
    // finalize all valid tables
    maIdTables.forEachMem( &Table::finalizeImport );
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
    if( (nTableId > 0) && (rDispName.getLength() > 0) )
    {
        OSL_ENSURE( !maIdTables.has( nTableId ), "TableBuffer::insertTableToMaps - multiple table identifier" );
        maIdTables[ nTableId ] = rxTable;
        OSL_ENSURE( !maNameTables.has( rDispName ), "TableBuffer::insertTableToMaps - multiple table name" );
        maNameTables[ rDispName ] = rxTable;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */