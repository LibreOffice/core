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
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::sheet::XDatabaseRanges;
using ::com::sun::star::sheet::XDatabaseRange;

namespace oox {
namespace xls {

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

void Table::importTable( RecordInputStream& rStrm, sal_Int16 nSheet )
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
    // validate cell range
    maDestRange = maModel.maRange;
    bool bValidRange = getAddressConverter().validateCellRange( maDestRange, true, true );

    // create database range
    if( bValidRange && (maModel.mnId > 0) && (maModel.maDisplayName.getLength() > 0) ) try
    {
        // find an unused name
        Reference< XDatabaseRanges > xDatabaseRanges = getDatabaseRanges();
        Reference< XNameAccess > xNameAccess( xDatabaseRanges, UNO_QUERY_THROW );
        OUString aName = ContainerHelper::getUnusedName( xNameAccess, maModel.maDisplayName, '_' );
        xDatabaseRanges->addNewByName( aName, maModel.maRange );
        Reference< XDatabaseRange > xDatabaseRange( xDatabaseRanges->getByName( aName ), UNO_QUERY_THROW );
        PropertySet aPropSet( xDatabaseRange );
        if( !aPropSet.getProperty( mnTokenIndex, PROP_TokenIndex ) )
            mnTokenIndex = -1;
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

TableRef TableBuffer::importTable( const AttributeList& rAttribs, sal_Int16 nSheet )
{
    TableRef xTable( new Table( *this ) );
    xTable->importTable( rAttribs, nSheet );
    insertTable( xTable );
    return xTable;
}

TableRef TableBuffer::importTable( RecordInputStream& rStrm, sal_Int16 nSheet )
{
    TableRef xTable( new Table( *this ) );
    xTable->importTable( rStrm, nSheet );
    insertTable( xTable );
    return xTable;
}

void TableBuffer::finalizeImport()
{
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

void TableBuffer::insertTable( TableRef xTable )
{
    sal_Int32 nTableId = xTable->getTableId();
    const OUString& rDispName = xTable->getDisplayName();
    if( (nTableId > 0) && (rDispName.getLength() > 0) )
    {
        OSL_ENSURE( maIdTables.find( nTableId ) == maIdTables.end(), "TableBuffer::insertTable - multiple table identifier" );
        maIdTables[ nTableId ] = xTable;
        OSL_ENSURE( maNameTables.find( rDispName ) == maNameTables.end(), "TableBuffer::insertTable - multiple table name" );
        maNameTables[ rDispName ] = xTable;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

