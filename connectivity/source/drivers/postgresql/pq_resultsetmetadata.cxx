/*************************************************************************
 *
 *  $RCSfile: pq_resultsetmetadata.cxx,v $
 *
 *  $Revision: 1.1.2.7 $
 *
 *  last change: $Author: jbu $ $Date: 2006/05/27 11:32:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
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


using osl::Mutex;
using osl::MutexGuard;

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OString;

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
// using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbcx::XColumnsSupplier;
using com::sun::star::sdbcx::XTablesSupplier;

using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameAccess;


#define ASCII_STR(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

namespace pq_sdbc_driver
{

// struct ColumnMetaData
// {
//     rtl::OUString tableName;
//     rtl::OUString schemaTableName;
//     rtl::OUString typeName;
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
    const rtl::OUString &schemaName,
    const rtl::OUString &tableName ) :
    m_refMutex( refMutex ),
    m_origin( origin ),
    m_ppSettings( ppSettings ),
    m_colCount( PQnfields( pResult ) ),
    m_tableName( tableName ),
    m_schemaName( schemaName ),
    m_checkedForTable( false ),
    m_checkedForTypes( false ),
    m_colDesc( PQnfields( pResult ) ),
    m_pResultSet( pResultSet )
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

// typedef std::hash_map<
//     Oid,
//     rtl::OUString,
//     std::hash< Oid >,
//     std::equal_to< Oid >,
//     Allocator < std::pair< Oid, rtl::OUString > > >  PqTypeMap;


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
            sal_Int32 oid = xRow->getInt( 1 );
            OUString typeName = xRow->getString( 2 );
            OUString typType = xRow->getString( 3 );

            sal_Int32 type = typeNameToDataType( typeName, typType );

            for( int j = 0; j < m_colCount ; j ++ )
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
                OUString name = getTableName( 1 );
//                 if( tables->hasByName( name ) )
                tables->getByName( name ) >>= m_table;
            }
        }
    }
}

sal_Int32 ResultSetMetaData::getIntColumnProperty( const rtl::OUString & name, int index, int def )
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
    catch( com::sun::star::uno::Exception & e )
    {
    }
    return ret;
}

sal_Bool ResultSetMetaData::getBoolColumnProperty( const rtl::OUString & name, int index, sal_Bool def )
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
    catch( com::sun::star::uno::Exception & e )
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

    return sal_True; // ??? hmm, numeric types or
}

sal_Bool ResultSetMetaData::isSearchable( sal_Int32 column ) throw (SQLException, RuntimeException)
{
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
    return sal_True; //
}

sal_Int32 ResultSetMetaData::getColumnDisplaySize( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( column );
    return m_colDesc[column-1].displaySize;
}

::rtl::OUString ResultSetMetaData::getColumnLabel( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return getColumnName( column);
}

::rtl::OUString ResultSetMetaData::getColumnName( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( column );

    return m_colDesc[column-1].name;
}

::rtl::OUString ResultSetMetaData::getSchemaName( sal_Int32 column ) throw (SQLException, RuntimeException)
{
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

::rtl::OUString ResultSetMetaData::getTableName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    rtl::OUString ret;
    if( m_tableName.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.append( m_schemaName );
        buf.appendAscii( "." );
        buf.append( m_tableName );
        ret = buf.makeStringAndClear();
    }
    return ret;
}

::rtl::OUString ResultSetMetaData::getCatalogName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
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

::rtl::OUString ResultSetMetaData::getColumnTypeName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    ::rtl::OUString ret; // give defensive answers, when data is not available
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
    catch( com::sun::star::uno::Exception & e )
    {
    }
    return ret;
}


sal_Bool ResultSetMetaData::isReadOnly( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
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
::rtl::OUString ResultSetMetaData::getColumnServiceName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
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
