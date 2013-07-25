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

#include "pq_resultsetmetadata.hxx"
#include "pq_resultset.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <string.h>

using osl::Mutex;
using osl::MutexGuard;


using com::sun::star::uno::Any;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;

using com::sun::star::lang::IllegalArgumentException;

using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbcx::XColumnsSupplier;
using com::sun::star::sdbcx::XTablesSupplier;

using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameAccess;


namespace pq_sdbc_driver
{

// struct ColumnMetaData
// {
//     OUString tableName;
//     OUString schemaTableName;
//     OUString typeName;
//     com::sun::star::sdbc::DataType type;
//     sal_Int32 precision;
//     sal_Int32 scale;
//     sal_Bool isCurrency;
//     sal_Bool isNullable;
//     sal_Bool isAutoIncrement;
//     sal_Bool isReadOnly;
//     sal_Bool isSigned;
// };

// is not exported by the postgres header
const static int PQ_VARHDRSZ = sizeof( sal_Int32 );

static void extractPrecisionAndScale( sal_Int32 atttypmod, sal_Int32 *precision, sal_Int32 *scale )
{
    if( atttypmod < PQ_VARHDRSZ )
    {
        *precision = 0;
        *scale = 0;
    }
    else
    {
        if( atttypmod & 0xffff0000 )
        {
            *precision = ( ( atttypmod - PQ_VARHDRSZ ) >> 16 ) & 0xffff;
            *scale = (atttypmod - PQ_VARHDRSZ ) & 0xffff;
        }
        else
        {
            *precision = atttypmod - PQ_VARHDRSZ;
            *scale = 0;
        }
    }
}

ResultSetMetaData::ResultSetMetaData(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >  & origin,
    ResultSet * pResultSet,
    ConnectionSettings **ppSettings,
    PGresult *pResult,
    const OUString &schemaName,
    const OUString &tableName ) :
    m_refMutex( refMutex ),
    m_ppSettings( ppSettings ),
    m_origin( origin ),
    m_tableName( tableName ),
    m_schemaName( schemaName ),
    m_colDesc( PQnfields( pResult ) ),
    m_pResultSet( pResultSet ),
    m_checkedForTable( false ),
    m_checkedForTypes( false ),
    m_colCount( PQnfields( pResult ) )
{

    // extract all needed information from the result object, so that we don't
    // need it anymore after this call !
    for( int col = 0; col < m_colCount ; col ++ )
    {
        sal_Int32 size = PQfsize( pResult, col );
        size = -1 == size ? 25 : size;
        m_colDesc[col].displaySize = size;

        extractPrecisionAndScale(
            PQfmod( pResult, col ),
            & ( m_colDesc[col].precision ),
            & ( m_colDesc[col].scale ) );
        char *name = PQfname( pResult, col );
        m_colDesc[col].name = OUString( name, strlen(name) , (*m_ppSettings)->encoding );
        m_colDesc[col].typeOid = PQftype( pResult, col );
        m_colDesc[col].type = com::sun::star::sdbc::DataType::LONGVARCHAR;
    }
}

void ResultSetMetaData::checkForTypes()
{
    if( ! m_checkedForTypes )
    {
        Reference< XStatement > stmt =
            extractConnectionFromStatement( m_origin->getStatement() )->createStatement();
        DisposeGuard guard( stmt );
        OUStringBuffer buf(128);
        buf.appendAscii( "SELECT oid, typname, typtype FROM pg_type WHERE ");
        for( int i  = 0 ; i < m_colCount ; i ++ )
        {
            if( i > 0 )
                buf.appendAscii( " OR " );
            int oid = m_colDesc[i].typeOid;
            buf.appendAscii( "oid=" );
            buf.append( (sal_Int32) oid, 10 );
        }
        Reference< XResultSet > rs = stmt->executeQuery( buf.makeStringAndClear() );
        Reference< XRow > xRow( rs, UNO_QUERY );
        while( rs->next() )
        {
            Oid oid = xRow->getInt( 1 );
            OUString typeName = xRow->getString( 2 );
            OUString typType = xRow->getString( 3 );

            sal_Int32 type = typeNameToDataType( typeName, typType );

            for( sal_Int32 j = 0; j < m_colCount ; j ++ )
            {
                if( m_colDesc[j].typeOid == oid )
                {
                    m_colDesc[j].typeName = typeName;
                    m_colDesc[j].type = type;
                }
            }
        }
        m_checkedForTypes = true;
    }
}

void ResultSetMetaData::checkTable()
{
    if( ! m_checkedForTable )
    {
        m_checkedForTable = true;
        if( m_tableName.getLength() )
        {

            Reference< com::sun::star::container::XNameAccess > tables = (*m_ppSettings)->tables;
            if( ! tables.is() )
            {

                Reference< XTablesSupplier > supplier =
                    Reference< XTablesSupplier > (
                        extractConnectionFromStatement( m_origin->getStatement() ), UNO_QUERY);
                if( supplier.is() )
                    tables = supplier->getTables();
            }
            if( tables.is() )
            {
                const OUString name   (getTableName ( 1 ));
                const OUString schema (getSchemaName( 1 ));
                const OUString composedName( schema.isEmpty() ? name : (schema + "." + name) );
                tables->getByName( composedName ) >>= m_table;
            }
        }
    }
}

sal_Int32 ResultSetMetaData::getIntColumnProperty( const OUString & name, int index, int def )
{
    sal_Int32 ret = def; // give defensive answers, when data is not available
    try
    {
        MutexGuard guard( m_refMutex->mutex );
        checkColumnIndex( index );
        Reference< XPropertySet > set = getColumnByIndex( index );

        if( set.is() )
        {
            set->getPropertyValue( name ) >>= ret;
        }
    }
    catch( com::sun::star::uno::Exception & )
    {
    }
    return ret;
}

sal_Bool ResultSetMetaData::getBoolColumnProperty( const OUString & name, int index, sal_Bool def )
{
    sal_Bool ret = def;
    try
    {
        MutexGuard guard( m_refMutex->mutex );
        checkColumnIndex( index );
        Reference< XPropertySet > set = getColumnByIndex( index );
        if( set.is() )
        {
            set->getPropertyValue( name ) >>= ret;
        }
    }
    catch( com::sun::star::uno::Exception & )
    {
    }

    return ret;
}

Reference< com::sun::star::beans::XPropertySet > ResultSetMetaData::getColumnByIndex( int index )
{
    Reference< XPropertySet > ret;
    checkTable();
    if( m_table.is() )
    {
        OUString columnName = getColumnName( index );
        Reference< XColumnsSupplier > supplier( m_table, UNO_QUERY );
        if( supplier.is() )
        {
            Reference< XNameAccess > columns = supplier->getColumns();
            if( columns.is() && columns->hasByName( columnName ) )
            {
                columns->getByName( columnName ) >>= ret;
            }
        }
    }
    return ret;
}

// Methods
sal_Int32 ResultSetMetaData::getColumnCount(  )
    throw (SQLException, RuntimeException)
{
    return m_colCount;
}

sal_Bool ResultSetMetaData::isAutoIncrement( sal_Int32 column )
    throw (SQLException, RuntimeException)
{

    sal_Bool ret = getBoolColumnProperty( getStatics().IS_AUTO_INCREMENT, column, sal_False );
    return ret;
}

sal_Bool ResultSetMetaData::isCaseSensitive( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    (void) column;
    return sal_True; // ??? hmm, numeric types or
}

sal_Bool ResultSetMetaData::isSearchable( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    (void) column;
    return sal_True; // mmm, what types are not searchable ?
}

sal_Bool ResultSetMetaData::isCurrency( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    return getBoolColumnProperty( getStatics().IS_CURRENCY, column, sal_False );
}

sal_Int32 ResultSetMetaData::isNullable( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return getIntColumnProperty(
        getStatics().IS_NULLABLE, column, com::sun::star::sdbc::ColumnValue::NULLABLE_UNKNOWN );
}

sal_Bool ResultSetMetaData::isSigned( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    (void) column;
    return sal_True;
}

sal_Int32 ResultSetMetaData::getColumnDisplaySize( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( column );
    return m_colDesc[column-1].displaySize;
}

OUString ResultSetMetaData::getColumnLabel( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return getColumnName( column);
}

OUString ResultSetMetaData::getColumnName( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( column );

    return m_colDesc[column-1].name;
}

OUString ResultSetMetaData::getSchemaName( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    (void) column;
    return m_schemaName;
}

sal_Int32 ResultSetMetaData::getPrecision( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( column );
    return m_colDesc[column-1].precision;
}

sal_Int32 ResultSetMetaData::getScale( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( column );
    return m_colDesc[column-1].scale;
}

OUString ResultSetMetaData::getTableName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    (void) column;
// LEM TODO This is very fishy.. Should probably return the table to which that column belongs!
    return m_tableName;
}

OUString ResultSetMetaData::getCatalogName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    (void) column;
    // can do this through XConnection.getCatalog() !
    return OUString();
}
sal_Int32 ResultSetMetaData::getColumnType( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    int ret = getIntColumnProperty( getStatics().TYPE, column, -100 );
    if( -100 == ret )
    {
        checkForTypes();
        if( com::sun::star::sdbc::DataType::LONGVARCHAR == m_colDesc[column-1].type && m_pResultSet )
            m_colDesc[column-1].type = m_pResultSet->guessDataType( column );
        ret = m_colDesc[column-1].type;
    }
    return ret;
}

OUString ResultSetMetaData::getColumnTypeName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    OUString ret; // give defensive answers, when data is not available
    try
    {
        MutexGuard guard( m_refMutex->mutex );
        checkColumnIndex( column );
        Reference< XPropertySet > set = getColumnByIndex( column );

        if( set.is() )
        {
            set->getPropertyValue( getStatics().TYPE_NAME ) >>= ret;
        }
        else
        {
            checkForTypes();
            ret = m_colDesc[column-1].typeName;
        }
    }
    catch( com::sun::star::uno::Exception & )
    {
    }
    return ret;
}


sal_Bool ResultSetMetaData::isReadOnly( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    (void) column;
    return sal_False;
}

sal_Bool ResultSetMetaData::isWritable( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return ! isReadOnly( column ); // what's the sense if this method ?
}

sal_Bool ResultSetMetaData::isDefinitelyWritable( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return isWritable(column); // uhh, now it becomes really esoteric ....
}
OUString ResultSetMetaData::getColumnServiceName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    (void) column;
    return OUString();
}

void ResultSetMetaData::checkClosed()
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    // we never close
}

void ResultSetMetaData::checkColumnIndex(sal_Int32 columnIndex)
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_colCount )
    {
        OUStringBuffer buf(128);

        buf.appendAscii( "pq_resultsetmetadata: index out of range (expected 1 to " );
        buf.append( m_colCount );
        buf.appendAscii( ", got " );
        buf.append( columnIndex );
        throw SQLException(
            buf.makeStringAndClear(), *this, OUString(), 1, Any() );
    }
}

}
