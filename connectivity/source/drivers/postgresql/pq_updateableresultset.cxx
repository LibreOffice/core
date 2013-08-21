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
 *    Copyright: 200? by Sun Microsystems, Inc.
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

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>

#include "pq_updateableresultset.hxx"
#include "pq_resultsetmetadata.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

#include <string.h>

#include <connectivity/dbconversion.hxx>

using osl::MutexGuard;


using com::sun::star::uno::Reference;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::sdbc::XGeneratedResultSet;
using com::sun::star::sdbc::XResultSetMetaDataSupplier;
using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XColumnLocate;
using com::sun::star::sdbc::XResultSetUpdate;
using com::sun::star::sdbc::XRowUpdate;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XStatement;

using com::sun::star::beans::XFastPropertySet;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XMultiPropertySet;

using namespace dbtools;

namespace pq_sdbc_driver
{


com::sun::star::uno::Reference< com::sun::star::sdbc::XCloseable > UpdateableResultSet::createFromPGResultSet(
    const ::rtl::Reference< RefCountedMutex > & mutex,
    const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
    ConnectionSettings **ppSettings,
    PGresult *result,
    const OUString &schema,
    const OUString &table,
    const com::sun::star::uno::Sequence< OUString > &primaryKey )
{
    ConnectionSettings *pSettings = *ppSettings;
    sal_Int32 columnCount = PQnfields( result );
    sal_Int32 rowCount = PQntuples( result );
    Sequence< OUString > columnNames( columnCount );
    for( int i = 0 ; i < columnCount ; i ++ )
    {
        char * name = PQfname( result, i );
        columnNames[i] = OUString( name, strlen(name), pSettings->encoding );
    }
    Sequence< Sequence< Any > > data( rowCount );

    // copy all the data into unicode strings (also binaries, as we yet
    // don't know, what a binary is and what not!)
    for( int row = 0 ; row < rowCount ; row ++ )
    {
        Sequence< Any > aRow( columnCount );
        for( int col = 0 ; col < columnCount ; col ++ )
        {
            if( ! PQgetisnull( result, row, col ) )
            {
                char * val = PQgetvalue( result, row, col );

                aRow[col] = makeAny(
                    OUString( val, strlen( val ) , (*ppSettings)->encoding ) );
            }
        }
        data[row] = aRow;
    }

    UpdateableResultSet *pRS =  new UpdateableResultSet(
        mutex, owner, columnNames, data, ppSettings, schema, table , primaryKey );

    Reference <XCloseable > ret = pRS; // give it an refcount

    pRS->m_meta = new ResultSetMetaData( mutex, pRS,0, ppSettings, result, schema, table );

    PQclear( result ); // we don't need it anymore

    return ret;
}

com::sun::star::uno::Any  UpdateableResultSet::queryInterface(
    const com::sun::star::uno::Type & reqType )
    throw (com::sun::star::uno::RuntimeException)
{
    Any ret = SequenceResultSet::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< XResultSetUpdate * > ( this ),
            static_cast< XRowUpdate * > ( this ) );
    return ret;
}


com::sun::star::uno::Sequence< com::sun::star::uno::Type > UpdateableResultSet::getTypes()
        throw( com::sun::star::uno::RuntimeException )
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( (Reference< XResultSetUpdate> *) 0 ),
                getCppuType( (Reference< XRowUpdate> *) 0 ),
                SequenceResultSet::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();

}

com::sun::star::uno::Sequence< sal_Int8> UpdateableResultSet::getImplementationId()
        throw( com::sun::star::uno::RuntimeException )
{
    static cppu::OImplementationId *pId;
    if( ! pId )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static cppu::OImplementationId id(sal_False);
            pId = &id;
        }
    }
    return pId->getImplementationId();
}

OUString UpdateableResultSet::buildWhereClause()
{
    OUString ret;
    if( m_primaryKey.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " WHERE " ) );
        for( int i = 0 ; i < m_primaryKey.getLength() ; i ++ )
        {
            if( i > 0 )
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " AND " ) );
            sal_Int32 index = findColumn( m_primaryKey[i] );
            bufferQuoteIdentifier( buf, m_primaryKey[i], *m_ppSettings );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " = " ) );
            bufferQuoteConstant( buf, getString( index ), *m_ppSettings );
        }
        ret = buf.makeStringAndClear();
    }
    return ret;
}


void UpdateableResultSet::insertRow(  ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    if( isLog( *m_ppSettings, LogLevel::INFO ) )
    {
        log( *m_ppSettings, LogLevel::INFO,"UpdateableResultSet::insertRow got called"  );
    }
    if( ! m_insertRow )
        throw SQLException(
            "pq_resultset.insertRow: moveToInsertRow has not been called !",
            *this, OUString(), 1, Any() );

    OUStringBuffer buf( 128 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "INSERT INTO " ) );
    bufferQuoteQualifiedIdentifier( buf, m_schema, m_table, *m_ppSettings );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " ( " ) );

    int columns = 0;
    for( UpdateableFieldVector::size_type i = 0 ; i < m_updateableField.size() ; i++ )
    {
        if( m_updateableField[i].isTouched )
        {
            if( columns > 0 )
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
            columns ++;
            bufferQuoteIdentifier( buf, m_columnNames[i], *m_ppSettings);
        }
    }
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " ) VALUES ( " ) );

    columns = 0;
    for( UpdateableFieldVector::size_type i = 0 ; i < m_updateableField.size() ; i ++ )
    {
        if( m_updateableField[i].isTouched )
        {
            if( columns > 0 )
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " , " ) );
            columns ++;
            bufferQuoteAnyConstant( buf, m_updateableField[i].value, *m_ppSettings );

//             OUString val;
//             m_updateableField[i].value >>= val;
//             buf.append( val );
//                 OStringToOUString(val, (*m_ppSettings)->encoding ) );
        }
    }

    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " )" ) );

    Reference< XStatement > stmt =
        extractConnectionFromStatement(m_owner)->createStatement();
    DisposeGuard dispGuard( stmt );
    stmt->executeUpdate( buf.makeStringAndClear() );

    // reflect the changes !
    m_rowCount ++;
    m_data.realloc( m_rowCount );
    m_data[m_rowCount-1] = Sequence< Any > ( m_fieldCount );
    Reference< XGeneratedResultSet > result( stmt, UNO_QUERY );
    if( result.is() )
    {
        Reference< XResultSet > rs = result->getGeneratedValues();
        if( rs.is() && rs->next() )
        {
            Reference< XColumnLocate > columnLocate( rs, UNO_QUERY );
            Reference< XRow> xRow ( rs, UNO_QUERY );
            for( int i = 0 ; i < m_fieldCount ; i++ )
            {
                int field = columnLocate->findColumn( m_columnNames[i] );
                if( field >= 1 )
                {
                    m_data[m_rowCount-1][i] <<=  xRow->getString( field );
//                     printf( "adding %s %s\n" ,
//                             OUStringToOString( m_columnNames[i], RTL_TEXTENCODING_ASCII_US).getStr(),
//                             OUStringToOString( xRow->getString( field ), RTL_TEXTENCODING_ASCII_US).getStr() );

                }
            }
        }
        else
        {
            // do the best we can ( DEFAULT and AUTO increment values fail ! )
            for( int i = 0 ; i < m_fieldCount ; i ++ )
            {
                if( m_updateableField[i].isTouched )
                    m_data[m_rowCount-1][i] = m_updateableField[i].value;
            }
        }
    }

    // cleanup
    m_updateableField = UpdateableFieldVector();
}

void UpdateableResultSet::updateRow(  ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    if( isLog( *m_ppSettings, LogLevel::INFO ) )
    {
        log( *m_ppSettings, LogLevel::INFO,"UpdateableResultSet::updateRow got called"  );
    }
    if( m_insertRow )
        throw SQLException(
            "pq_resultset.updateRow: moveToCurrentRow has not been called !",
            *this, OUString(), 1, Any() );

    OUStringBuffer buf( 128 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "UPDATE " ) );
    bufferQuoteQualifiedIdentifier( buf, m_schema, m_table, *m_ppSettings );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "SET " ) );

    int columns = 0;
    for( UpdateableFieldVector::size_type i = 0; i < m_updateableField.size() ; i ++ )
    {
        if( m_updateableField[i].isTouched )
        {
            if( columns > 0 )
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );
            columns ++;

            buf.append( m_columnNames[i] );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" = " ) );
            bufferQuoteAnyConstant( buf, m_updateableField[i].value, *m_ppSettings );
//             OUString val;
//             m_updateableField[i].value >>= val;
//             bufferQuoteConstant( buf, val ):
//             buf.append( val );
        }
    }
    buf.append( buildWhereClause() );

    Reference< XStatement > stmt = extractConnectionFromStatement(m_owner)->createStatement();
    DisposeGuard dispGuard( stmt );
    stmt->executeUpdate( buf.makeStringAndClear() );

    // reflect the changes !
    for( int i = 0 ; i < m_fieldCount ; i ++ )
    {
        if( m_updateableField[i].isTouched  )
            m_data[m_row][i] = m_updateableField[i].value;
    }
    m_updateableField = UpdateableFieldVector();
}

void UpdateableResultSet::deleteRow(  ) throw (SQLException, RuntimeException)
{
    if( isLog( *m_ppSettings, LogLevel::INFO ) )
    {
        log( *m_ppSettings, LogLevel::INFO,"UpdateableResultSet::deleteRow got called"  );
    }
    if( m_insertRow )
        throw SQLException(
            "pq_resultset.deleteRow: deleteRow cannot be called when on insert row !",
            *this, OUString(), 1, Any() );

    if( m_row < 0 || m_row >= m_rowCount )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "deleteRow cannot be called on invalid row (" );
        buf.append( m_row );
        buf.appendAscii( ")" );
        throw SQLException( buf.makeStringAndClear() , *this, OUString(), 0, Any() );
    }

    Reference< XStatement > stmt = extractConnectionFromStatement(m_owner)->createStatement();
    DisposeGuard dispGuard( stmt );
    OUStringBuffer buf( 128 );
    buf.appendAscii( "DELETE FROM " );
    bufferQuoteQualifiedIdentifier( buf, m_schema, m_table, *m_ppSettings );
    buf.appendAscii( " " );
    buf.append( buildWhereClause() );

    stmt->executeUpdate( buf.makeStringAndClear() );

    // reflect the changes !
    for( int i = m_row + 1; i < m_row ; i ++ )
    {
        m_data[i-1] = m_data[i];
    }
    m_rowCount --;
    m_data.realloc( m_rowCount );
 }

void UpdateableResultSet::cancelRowUpdates(  ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    m_updateableField = UpdateableFieldVector();
}

void UpdateableResultSet::moveToInsertRow(  ) throw (SQLException, RuntimeException)
{
    m_insertRow = true;
}

void UpdateableResultSet::moveToCurrentRow(  ) throw (SQLException, RuntimeException)
{
    m_insertRow = false;
}

void UpdateableResultSet::checkUpdate( sal_Int32 columnIndex)
{
    checkColumnIndex( columnIndex );
    if( m_updateableField.empty() )
        m_updateableField = UpdateableFieldVector( m_fieldCount );
    m_updateableField[columnIndex-1].isTouched = true;
}

void UpdateableResultSet::updateNull( sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );
    m_updateableField[columnIndex-1].value = Any();
}

void UpdateableResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    Statics &st = getStatics();
    m_updateableField[columnIndex-1].value <<=  ( x ? st.TRUE : st.FALSE );

}

void UpdateableResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw (SQLException, RuntimeException)
{
    updateInt(columnIndex,x);
}

void UpdateableResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw (SQLException, RuntimeException)
{
    updateInt( columnIndex, x );
}

void UpdateableResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw (SQLException, RuntimeException)
{
    updateLong( columnIndex, x );
//     MutexGuard guard( m_refMutex->mutex );
//     checkClosed();
//     checkUpdate( columnIndex );

//     m_updateableField[columnIndex-1].value <<= OUString::valueOf( x );

}

void UpdateableResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

//     OStringBuffer buf( 20 );
//     buf.append( "'" );
//     buf.append( (sal_Int64) x );
//     buf.append( "'" );
    m_updateableField[columnIndex-1].value <<= OUString::number( x );
}

void UpdateableResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw (SQLException, RuntimeException)
{

    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    m_updateableField[columnIndex-1].value <<= OUString::number( x );
}

void UpdateableResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    m_updateableField[columnIndex-1].value <<= OUString::number( x );
}

void UpdateableResultSet::updateString( sal_Int32 columnIndex, const OUString& x ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    m_updateableField[columnIndex-1].value <<= x;
}

void UpdateableResultSet::updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw (SQLException, RuntimeException)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    size_t len;
    unsigned char * escapedString =
        PQescapeBytea( (unsigned char *)x.getConstArray(), x.getLength(), &len);
    if( ! escapedString )
    {
        throw SQLException(
            "pq_preparedstatement.setBytes: Error during converting bytesequence to an SQL conform string",
            *this, OUString(), 1, Any() );
    }
//     buf.append( (const sal_Char *)escapedString, len -1 );

    m_updateableField[columnIndex-1].value <<=
        OUString( (sal_Char*) escapedString, len, RTL_TEXTENCODING_ASCII_US );
    free( escapedString );
}

void UpdateableResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw (SQLException, RuntimeException)
{
    updateString( columnIndex, DBTypeConversion::toDateString( x ) );
}

void UpdateableResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw (SQLException, RuntimeException)
{
    updateString( columnIndex, DBTypeConversion::toTimeString( x ) );
}

void UpdateableResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw (SQLException, RuntimeException)
{
    updateString( columnIndex, DBTypeConversion::toDateTimeString( x ) );
}

void UpdateableResultSet::updateBinaryStream( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& /* x */, sal_Int32 /* length */ ) throw (SQLException, RuntimeException)
{
}

void UpdateableResultSet::updateCharacterStream( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& /* x */, sal_Int32 /* length */ ) throw (SQLException, RuntimeException)
{
}

void UpdateableResultSet::updateObject( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Any& /* x */ ) throw (SQLException, RuntimeException)
{
}

void UpdateableResultSet::updateNumericObject( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Any& /* x */, sal_Int32 /* scale */ ) throw (SQLException, RuntimeException)
{
}


::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > UpdateableResultSet::getMetaData(  )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return m_meta;
}

Sequence< Type > UpdateableResultSet::getStaticTypes( bool updateable )
    throw( com::sun::star::uno::RuntimeException )
{
    if( updateable )
    {
        cppu::OTypeCollection collection(
            getCppuType( (Reference< XResultSetUpdate> *) 0 ),
            getCppuType( (Reference< XRowUpdate> *) 0 ),
//             getCppuType( (Reference< com::sun::star::sdbcx::XRowLocate > *) 0 ),
            getStaticTypes( false /* updateable */ ) );
        return collection.getTypes();
    }
    else
    {
        cppu::OTypeCollection collection(
            getCppuType( (Reference< XResultSet> *) 0 ),
            getCppuType( (Reference< XResultSetMetaDataSupplier> *) 0 ),
            getCppuType( (Reference< XRow> *) 0 ),
            getCppuType( (Reference< XColumnLocate> *) 0 ),
            getCppuType( (Reference< XCloseable> *) 0 ),
            getCppuType( (Reference< XPropertySet >*) 0 ),
            getCppuType( (Reference< XFastPropertySet > *) 0 ),
            getCppuType( (Reference< XMultiPropertySet > *) 0 ),
            getCppuType( (const Reference< com::sun::star::lang::XComponent > *)0 ),  // OComponentHelper
            getCppuType( (const Reference< com::sun::star::lang::XTypeProvider > *)0 ),
            getCppuType( (const Reference< com::sun::star::uno::XAggregation > *)0 ),
            getCppuType( (const Reference< com::sun::star::uno::XWeak > *)0 ) );
        return collection.getTypes();
    }
}

}
