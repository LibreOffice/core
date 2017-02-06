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

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>

#include "pq_xcolumns.hxx"
#include "pq_xkeycolumns.hxx"
#include "pq_xkeycolumn.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;

using com::sun::star::beans::XPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XDatabaseMetaData;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

KeyColumns::KeyColumns(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName,
        const Sequence< OUString > &columnNames,
        const Sequence< OUString > &foreignColumnNames )
    : Container( refMutex, origin, pSettings,  "KEY_COLUMN" ),
      m_schemaName( schemaName ),
      m_tableName( tableName ),
      m_columnNames( columnNames ),
      m_foreignColumnNames( foreignColumnNames )
{}

KeyColumns::~KeyColumns()
{}


void KeyColumns::refresh()
{
    try
    {
        if (isLog(m_pSettings, LogLevel::Info))
        {
            OStringBuffer buf;
            buf.append( "sdbcx.KeyColumns get refreshed for table " );
            buf.append( OUStringToOString( m_schemaName, ConnectionSettings::encoding ) );
            buf.append( "." );
            buf.append( OUStringToOString( m_tableName, ConnectionSettings::encoding ) );
            log( m_pSettings, LogLevel::Info, buf.makeStringAndClear().getStr() );
        }

        osl::MutexGuard guard( m_refMutex->mutex );

        Statics &st = getStatics();
        Reference< XDatabaseMetaData > meta = m_origin->getMetaData();

        Reference< XResultSet > rs =
            meta->getColumns( Any(), m_schemaName, m_tableName, st.cPERCENT );

        DisposeGuard disposeIt( rs );
        Reference< XRow > xRow( rs , UNO_QUERY );

        String2IntMap map;

        m_values.clear();
        sal_Int32 columnIndex = 0;
        while( rs->next() )
        {
            OUString columnName = xRow->getString( 4 );

            int keyindex;
            for( keyindex = 0 ; keyindex < m_columnNames.getLength() ; keyindex ++ )
            {
                if( columnName == m_columnNames[keyindex] )
                    break;
            }
            if( m_columnNames.getLength() == keyindex )
                continue;

            KeyColumn * pKeyColumn =
                new KeyColumn( m_refMutex, m_origin, m_pSettings );
            Reference< css::beans::XPropertySet > prop = pKeyColumn;

            OUString name = columnMetaData2SDBCX( pKeyColumn, xRow );
            if( keyindex < m_foreignColumnNames.getLength() )
            {
                pKeyColumn->setPropertyValue_NoBroadcast_public(
                    st.RELATED_COLUMN, makeAny( m_foreignColumnNames[keyindex]) );
            }

            {
                m_values.push_back( makeAny( prop ) );
                map[ name ] = columnIndex;
                ++columnIndex;
            }
        }
        m_name2index.swap( map );
    }
    catch ( css::sdbc::SQLException & e )
    {
        throw RuntimeException( e.Message , e.Context );
    }

    fire( RefreshedBroadcaster( *this ) );
}


void KeyColumns::appendByDescriptor(
    const css::uno::Reference< css::beans::XPropertySet >& future )
{
    (void) future;
    throw css::sdbc::SQLException(
        "KeyColumns::appendByDescriptor not implemented yet",
        *this, OUString(), 1, Any() );

//     osl::MutexGuard guard( m_refMutex->mutex );
//     Statics & st = getStatics();
//     Reference< XPropertySet > past = createDataDescriptor();
//     past->setPropertyValue( st.IS_NULLABLE, makeAny( css::sdbc::ColumnValue::NULLABLE ) );
//     alterColumnByDescriptor(
//         m_schemaName, m_tableName, m_pSettings->encoding, m_origin->createStatement() , past, future  );

}


void KeyColumns::dropByIndex( sal_Int32 index )
{
    (void) index;
    throw css::sdbc::SQLException(
        "KeyColumns::dropByIndex not implemented yet",
        *this, OUString(), 1, Any() );
//     osl::MutexGuard guard( m_refMutex->mutex );
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


Reference< css::beans::XPropertySet > KeyColumns::createDataDescriptor()
{
    return new KeyColumnDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< css::container::XNameAccess > KeyColumns::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const css::uno::Reference< css::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString &schemaName,
    const OUString &tableName,
    const Sequence< OUString > &columnNames ,
    const Sequence< OUString > &foreignColumnNames )
{
    KeyColumns *pKeyColumns = new KeyColumns(
        refMutex, origin, pSettings, schemaName, tableName, columnNames, foreignColumnNames );
    Reference< css::container::XNameAccess > ret = pKeyColumns;
    pKeyColumns->refresh();

    return ret;
}


KeyColumnDescriptors::KeyColumnDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings )
    : Container( refMutex, origin, pSettings,  "KEY_COLUMN" )
{}

Reference< css::beans::XPropertySet > KeyColumnDescriptors::createDataDescriptor()
{
    return new KeyColumnDescriptor( m_refMutex, m_origin, m_pSettings );
}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
