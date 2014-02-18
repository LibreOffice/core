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

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
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
using com::sun::star::uno::Type;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;

using com::sun::star::container::NoSuchElementException;
using com::sun::star::lang::WrappedTargetException;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XDatabaseMetaData;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

KeyColumns::KeyColumns(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
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
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        if( isLog( m_pSettings, LogLevel::INFO ) )
        {
            OStringBuffer buf;
            buf.append( "sdbcx.KeyColumns get refreshed for table " );
            buf.append( OUStringToOString( m_schemaName, m_pSettings->encoding ) );
            buf.append( "." );
            buf.append( OUStringToOString( m_tableName, m_pSettings->encoding ) );
            log( m_pSettings, LogLevel::INFO, buf.makeStringAndClear().getStr() );
        }

        osl::MutexGuard guard( m_refMutex->mutex );

        Statics &st = getStatics();
        Reference< XDatabaseMetaData > meta = m_origin->getMetaData();

        Reference< XResultSet > rs =
            meta->getColumns( Any(), m_schemaName, m_tableName, st.cPERCENT );

        DisposeGuard disposeIt( rs );
        Reference< XRow > xRow( rs , UNO_QUERY );

        String2IntMap map;

        m_values = Sequence< com::sun::star::uno::Any > ();
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
            Reference< com::sun::star::beans::XPropertySet > prop = pKeyColumn;

            OUString name = columnMetaData2SDBCX( pKeyColumn, xRow );
            if( keyindex < m_foreignColumnNames.getLength() )
            {
                pKeyColumn->setPropertyValue_NoBroadcast_public(
                    st.RELATED_COLUMN, makeAny( m_foreignColumnNames[keyindex]) );
            }

            {
                const int currentColumnIndex = columnIndex++;
                assert(currentColumnIndex  == m_values.getLength());
                m_values.realloc( columnIndex );
                m_values[currentColumnIndex] = makeAny( prop );
                map[ name ] = currentColumnIndex;
            }
        }
        m_name2index.swap( map );
    }
    catch ( com::sun::star::sdbc::SQLException & e )
    {
        throw RuntimeException( e.Message , e.Context );
    }

    fire( RefreshedBroadcaster( *this ) );
}


// void alterColumnByDescriptor(
//     const OUString & schemaName,
//     const OUString & tableName,
//     rtl_TextEncoding encoding,
//     const Reference< XStatement > &stmt,
//     const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & past,
//     const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & future)
// {
//     Statics & st  = getStatics();

// //     if( past->getPropertyValue( st.TABLE_NAME ) != future->getPropertyValue( st.TABLE_NAME ) ||
// //         past->getPropertyValue( st.SCHEMA_NAME ) != future->getPropertyValue( st.SCHEMA_NAME ))
// //     {
// //         OUStringBuffer buf(128);
// //         buf.append( "Can't move column " );
// //         buf.append( extractStringProperty( past, st.COLUMN_NAME ) );
// //         buf.append( " from table " );
// //         buf.append( extractStringProperty( past, st.TABLE_NAME ) );
// //         buf.append( " to table " );
// //         buf.append( extractStringProperty( past, st.TABLE_NAME ) );
// //         throw SQLException( buf.makeStringAndClear(), Reference< XInterface > () );
// //     }

// //     OUString tableName = extractStringProperty( past, st.TABLE_NAME );
// //     OUString schemaName = extractStringProperty( past, st.SCHEMA_NAME );
//     OUString pastColumnName = extractStringProperty( past, st.NAME );
//     OUString futureColumnName = extractStringProperty( future, st.NAME );
//     OUString pastTypeName = sqltype2string( past );
//     OUString futureTypeName = sqltype2string( future );

//     TransactionGuard transaction( stmt );

//     OUStringBuffer buf( 128 );
//     if( ! pastColumnName.getLength())
//     {
//         // create a new column
//         buf.append( "ALTER TABLE" );
//         bufferQuoteQualifiedIdentifier( buf, schemaName, tableName );
//         buf.append( "ADD COLUMN" );
//         bufferQuoteIdentifier( buf, futureColumnName );
//         buf.append( futureTypeName );
//         transaction.executeUpdate( buf.makeStringAndClear() );
//     }
//     else
//     {
//         if( pastTypeName != futureTypeName )
//         {
//             throw RuntimeException(
//                 "Can't modify column types, drop the column and create a new one",
//                 Reference< XInterface > () );
//         }

//         if( pastColumnName != futureColumnName )
//         {
//             buf.append( "ALTER TABLE" );
//             bufferQuoteQualifiedIdentifier( buf, schemaName, tableName );
//             buf.append( "RENAME COLUMN" );
//             bufferQuoteIdentifier( buf, pastColumnName );
//             buf.append( "TO" );
//             bufferQuoteIdentifier( buf, futureColumnName );
//             transaction.executeUpdate( buf.makeStringAndClear() );
//         }
//     }

//     OUString futureDefaultValue = extractStringProperty( future, st.DEFAULT_VALUE );
//     OUString pastDefaultValue = extractStringProperty( past, st.DEFAULT_VALUE );
//     if( futureDefaultValue != pastDefaultValue )
//     {
//         buf = OUStringBuffer( 128 );
//         buf.append( "ALTER TABLE" );
//         bufferQuoteQualifiedIdentifier( buf, schemaName, tableName );
//         buf.append( "ALTER COLUMN" );
//         bufferQuoteIdentifier( buf, futureColumnName );
//         buf.append( "SET DEFAULT " );
//         // default value is not quoted, caller needs to quote himself (otherwise
//         // how to pass e.g. nextval('something' ) ????
//         buf.append( futureDefaultValue );
// //        bufferQuoteConstant( buf, defaultValue, encoding );
//         transaction.executeUpdate( buf.makeStringAndClear() );
//     }

//     sal_Int32 futureNullable = extractIntProperty( future, st.IS_NULLABLE );
//     sal_Int32 pastNullable = extractIntProperty( past, st.IS_NULLABLE );
//     if( futureNullable != pastNullable )
//     {
//         buf = OUStringBuffer( 128 );
//         buf.append( "ALTER TABLE" );
//         bufferQuoteQualifiedIdentifier( buf, schemaName, tableName );
//         buf.append( "ALTER COLUMN" );
//         bufferQuoteIdentifier( buf, futureColumnName );
//         if( futureNullable == com::sun::star::sdbc::ColumnValue::NO_NULLS )
//         {
//             buf.append( "SET" );
//         }
//         else
//         {
//             buf.append( "DROP" );
//         }
//         buf.append( " NOT NULL" );
//         transaction.executeUpdate( buf.makeStringAndClear() );
//     }

//     OUString futureComment = extractStringProperty( future, st.DESCRIPTION );
//     OUString pastComment = extractStringProperty( past, st.DESCRIPTION );
//     if( futureComment != pastComment )
//     {
//         buf = OUStringBuffer( 128 );
//         buf.append( "COMMENT ON COLUMN" );
//         bufferQuoteQualifiedIdentifier( buf, schemaName, tableName , futureColumnName );
//         buf.append( "IS " );
//         bufferQuoteConstant( buf, futureComment,encoding);
//         transaction.executeUpdate( buf.makeStringAndClear() );
//     }
//     transaction.commit();
// }

void KeyColumns::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& future )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    (void) future;
    throw com::sun::star::sdbc::SQLException(
        "KeyColumns::appendByDescriptor not implemented yet",
        *this, OUString(), 1, Any() );

//     osl::MutexGuard guard( m_refMutex->mutex );
//     Statics & st = getStatics();
//     Reference< XPropertySet > past = createDataDescriptor();
//     past->setPropertyValue( st.IS_NULLABLE, makeAny( com::sun::star::sdbc::ColumnValue::NULLABLE ) );
//     alterColumnByDescriptor(
//         m_schemaName, m_tableName, m_pSettings->encoding, m_origin->createStatement() , past, future  );

}


void KeyColumns::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{
    (void) index;
    throw com::sun::star::sdbc::SQLException(
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
//         throw com::sun::star::lang::IndexOutOfBoundsException(
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


Reference< ::com::sun::star::beans::XPropertySet > KeyColumns::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new KeyColumnDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< com::sun::star::container::XNameAccess > KeyColumns::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString &schemaName,
    const OUString &tableName,
    const Sequence< OUString > &columnNames ,
    const Sequence< OUString > &foreignColumnNames )
{
    KeyColumns *pKeyColumns = new KeyColumns(
        refMutex, origin, pSettings, schemaName, tableName, columnNames, foreignColumnNames );
    Reference< com::sun::star::container::XNameAccess > ret = pKeyColumns;
    pKeyColumns->refresh();

    return ret;
}

//_______________________________________________________________________________________
KeyColumnDescriptors::KeyColumnDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings )
    : Container( refMutex, origin, pSettings,  "KEY_COLUMN" )
{}

Reference< ::com::sun::star::beans::XPropertySet > KeyColumnDescriptors::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new KeyColumnDescriptor( m_refMutex, m_origin, m_pSettings );
}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
