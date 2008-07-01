/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablebuffer.cxx,v $
 * $Revision: 1.4 $
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

OoxTableData::OoxTableData() :
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
    if( getAddressConverter().convertToCellRange( maOoxData.maRange, rAttribs.getString( XML_ref, OUString() ), nSheet, true ) )
    {
        maOoxData.maProgName    = rAttribs.getString( XML_name, OUString() );
        maOoxData.maDisplayName = rAttribs.getString( XML_displayName, OUString() );
        maOoxData.mnId          = rAttribs.getInteger( XML_id, -1 );
        maOoxData.mnType        = rAttribs.getToken( XML_tableType, XML_worksheet );
        maOoxData.mnHeaderRows  = rAttribs.getInteger( XML_headerRowCount, 1 );
        maOoxData.mnTotalsRows  = rAttribs.getInteger( XML_totalsRowCount, 0 );
    }
}

void Table::importTable( RecordInputStream& rStrm, sal_Int16 nSheet )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    if( getAddressConverter().convertToCellRange( maOoxData.maRange, aBinRange, nSheet, true ) )
    {
        sal_Int32 nType;
        rStrm >> nType >> maOoxData.mnId >> maOoxData.mnHeaderRows >> maOoxData.mnTotalsRows;
        rStrm.skip( 32 );
        rStrm >> maOoxData.maProgName >> maOoxData.maDisplayName;

        static const sal_Int32 spnTypes[] = { XML_worksheet, XML_TOKEN_INVALID, XML_TOKEN_INVALID, XML_queryTable };
        maOoxData.mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
    }
}

void Table::finalizeImport()
{
    if( maOoxData.maDisplayName.getLength() > 0 ) try
    {
        // find an unused name
        Reference< XDatabaseRanges > xDatabaseRanges = getDatabaseRanges();
        Reference< XNameAccess > xNameAccess( xDatabaseRanges, UNO_QUERY_THROW );
        OUString aName = ContainerHelper::getUnusedName( xNameAccess, maOoxData.maDisplayName, '_' );
        xDatabaseRanges->addNewByName( aName, maOoxData.maRange );
        Reference< XDatabaseRange > xDatabaseRange( xDatabaseRanges->getByName( aName ), UNO_QUERY_THROW );
        PropertySet aPropSet( xDatabaseRange );
        if( !aPropSet.getProperty( mnTokenIndex, CREATE_OUSTRING( "TokenIndex" ) ) )
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
    maTables.forEachMem( &Table::finalizeImport );
}

TableRef TableBuffer::getTable( sal_Int32 nTableId ) const
{
    return maTables.get( nTableId );
}

// private --------------------------------------------------------------------

void TableBuffer::insertTable( TableRef xTable )
{
    sal_Int32 nTableId = xTable->getTableId();
    if( nTableId > 0 )
    {
        OSL_ENSURE( maTables.find( nTableId ) == maTables.end(), "TableBuffer::insertTable - multiple table identifier" );
        maTables[ nTableId ] = xTable;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

