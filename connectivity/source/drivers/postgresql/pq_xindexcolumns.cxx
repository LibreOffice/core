/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#include <sal/log.hxx>

#include <string_view>
#include <vector>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ref.hxx>

#include "pq_xcolumns.hxx"
#include "pq_xindexcolumns.hxx"
#include "pq_xindexcolumn.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;

using com::sun::star::beans::XPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XDatabaseMetaData;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

IndexColumns::IndexColumns(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName,
        const OUString &indexName,
        const css::uno::Sequence< OUString > &columns )
    : Container( refMutex, origin, pSettings,  "INDEX_COLUMN" ),
      m_schemaName( schemaName ),
      m_tableName( tableName ),
      m_indexName( indexName ),
      m_columns( columns )
{}

IndexColumns::~IndexColumns()
{}

static sal_Int32 findInSequence( const Sequence< OUString > & seq , std::u16string_view str)
{
    int index;
    for( index = 0 ; index < seq.getLength() ; index ++ )
    {
        if( str == seq[index] )
            break;
    }
    return index;
}

void IndexColumns::refresh()
{
    try
    {
        SAL_INFO("connectivity.postgresql", "sdbcx.IndexColumns get refreshed for index " << m_indexName);

        osl::MutexGuard guard( m_xMutex->GetMutex() );

        Statics &st = getStatics();
        Reference< XDatabaseMetaData > meta = m_origin->getMetaData();

        Reference< XResultSet > rs =
            meta->getColumns( Any(), m_schemaName, m_tableName, st.cPERCENT );

        DisposeGuard disposeIt( rs );
        Reference< XRow > xRow( rs , UNO_QUERY );
        m_values.clear();
        m_values.resize( m_columns.getLength() );

        while( rs->next() )
        {
            OUString columnName = xRow->getString( 4 );

            sal_Int32 index = findInSequence( m_columns, columnName );
            if( index >= m_columns.getLength() )
                continue;

            rtl::Reference<IndexColumn> pIndexColumn =
                new IndexColumn( m_xMutex, m_origin, m_pSettings );
            Reference< css::beans::XPropertySet > prop = pIndexColumn;

            columnMetaData2SDBCX( pIndexColumn.get(), xRow );
            pIndexColumn->setPropertyValue_NoBroadcast_public(
                st.IS_ASCENDING , makeAny( false ) );

            m_values[ index ] <<= prop;
            m_name2index[ columnName ] = index;
        }
    }
    catch ( css::sdbc::SQLException & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException( e.Message,
                        e.Context, anyEx );
    }

    fire( RefreshedBroadcaster( *this ) );
}


void IndexColumns::appendByDescriptor(
    const css::uno::Reference< css::beans::XPropertySet >& /*future*/ )
{
    throw css::sdbc::SQLException(
        "SDBC-POSTGRESQL: IndexesColumns.appendByDescriptor not yet implemented",
        *this, OUString(), 1, Any() );
//     osl::MutexGuard guard( m_xMutex->GetMutex() );
//     Statics & st = getStatics();
//     Reference< XPropertySet > past = createDataDescriptor();
//     past->setPropertyValue( st.IS_NULLABLE, makeAny( css::sdbc::ColumnValue::NULLABLE ) );
//     alterColumnByDescriptor(
//         m_schemaName, m_tableName, m_pSettings->encoding, m_origin->createStatement() , past, future  );

}

void IndexColumns::dropByName( const OUString& )
{
    throw css::sdbc::SQLException(
        "SDBC-POSTGRESQL: IndexesColumns.dropByName not yet implemented",
        *this, OUString(), 1, Any() );
//     String2IntMap::const_iterator ii = m_name2index.find( elementName );
//     if( ii == m_name2index.end() )
//     {
//         OUStringBuffer buf( 128 );
//         buf.appendAscii( "Column " );
//         buf.append( elementName );
//         buf.appendAscii( " is unknown in table " );
//         buf.append( m_schemaName );
//         buf.appendAscii( "." );
//         buf.append( m_tableName );
//         buf.appendAscii( ", so it can't be dropped" );
//         throw css::container::NoSuchElementException(
//             buf.makeStringAndClear(), *this );
//     }
//     dropByIndex( ii->second );
}

void IndexColumns::dropByIndex( sal_Int32 )
{
    throw css::sdbc::SQLException(
        "SDBC-POSTGRESQL: IndexesColumns.dropByIndex not yet implemented",
        *this, OUString(), 1, Any() );
//     osl::MutexGuard guard( m_xMutex->GetMutex() );
//     if( index < 0 ||  index >= m_values.getLength() )
//     {
//         OUStringBuffer buf( 128 );
//         buf.appendAscii( "COLUMNS: Index out of range (allowed 0 to " );
//         buf.append((sal_Int32)(m_values.getLength() -1) );
//         buf.appendAscii( ", got " );
//         buf.append( index );
//         buf.appendAscii( ")" );
//         throw css::lang::IndexOutOfBoundsException(
//             buf.makeStringAndClear(), *this );
//     }

//     Reference< XPropertySet > set;
//     m_values[index] >>= set;
//     Statics &st = getStatics();
//     OUString name;
//     set->getPropertyValue( st.NAME ) >>= name;

//     OUStringBuffer update( 128 );
//     update.appendAscii( "ALTER TABLE ONLY");
//     bufferQuoteQualifiedIdentifier( update, m_schemaName, m_tableName );
//     update.appendAscii( "DROP COLUMN" );
//     bufferQuoteIdentifier( update, name );
//     Reference< XStatement > stmt = m_origin->createStatement( );
//     DisposeGuard disposeIt( stmt );
//     stmt->executeUpdate( update.makeStringAndClear() );

}


Reference< css::beans::XPropertySet > IndexColumns::createDataDescriptor()
{
    return new IndexColumnDescriptor( m_xMutex, m_origin, m_pSettings );
}

Reference< css::container::XNameAccess > IndexColumns::create(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString &schemaName,
    const OUString &tableName,
    const OUString &indexName,
    const Sequence< OUString > &columns )
{
    rtl::Reference<IndexColumns> pIndexColumns = new IndexColumns(
        refMutex, origin, pSettings, schemaName, tableName, indexName, columns );
    pIndexColumns->refresh();

    return pIndexColumns;
}


IndexColumnDescriptors::IndexColumnDescriptors(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings)
    : Container( refMutex, origin, pSettings,  getStatics().INDEX_COLUMN )
{}

Reference< css::container::XNameAccess > IndexColumnDescriptors::create(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings)
{
    return new IndexColumnDescriptors( refMutex, origin, pSettings );
}

css::uno::Reference< css::beans::XPropertySet > IndexColumnDescriptors::createDataDescriptor()
{
    return new IndexColumnDescriptor( m_xMutex, m_origin, m_pSettings );
}

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
