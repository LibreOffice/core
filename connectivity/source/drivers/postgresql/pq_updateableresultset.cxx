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
#include <com/sun/star/sdbc/SQLException.hpp>
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


css::uno::Reference< css::sdbc::XCloseable > UpdateableResultSet::createFromPGResultSet(
    const ::rtl::Reference< RefCountedMutex > & mutex,
    const css::uno::Reference< css::uno::XInterface > &owner,
    ConnectionSettings **ppSettings,
    PGresult *result,
    const OUString &schema,
    const OUString &table,
    const std::vector< OUString > &primaryKey )
{
    sal_Int32 columnCount = PQnfields( result );
    sal_Int32 rowCount = PQntuples( result );
    std::vector< OUString > columnNames( columnCount );
    for( int i = 0 ; i < columnCount ; i ++ )
    {
        char * name = PQfname( result, i );
        columnNames[i] = OUString( name, strlen(name), ConnectionSettings::encoding );
    }
    std::vector< std::vector< Any > > data( rowCount );

    // copy all the data into unicode strings (also binaries, as we yet
    // don't know, what a binary is and what not!)
    for( int row = 0 ; row < rowCount ; row ++ )
    {
        std::vector< Any > aRow( columnCount );
        for( int col = 0 ; col < columnCount ; col ++ )
        {
            if( ! PQgetisnull( result, row, col ) )
            {
                char * val = PQgetvalue( result, row, col );

                aRow[col] = makeAny(
                    OUString( val, strlen( val ), ConnectionSettings::encoding ) );
            }
        }
        data[row] = aRow;
    }

    UpdateableResultSet *pRS =  new UpdateableResultSet(
        mutex, owner, columnNames, data, ppSettings, schema, table, primaryKey );

    Reference <XCloseable > ret = pRS; // give it an refcount

    pRS->m_meta = new ResultSetMetaData( mutex, pRS,nullptr, ppSettings, result, schema, table );

    PQclear( result ); // we don't need it anymore

    return ret;
}

css::uno::Any  UpdateableResultSet::queryInterface(
    const css::uno::Type & reqType )
{
    Any ret = SequenceResultSet::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< XResultSetUpdate * > ( this ),
            static_cast< XRowUpdate * > ( this ) );
    return ret;
}


css::uno::Sequence< css::uno::Type > UpdateableResultSet::getTypes()
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<XResultSetUpdate>::get(),
                cppu::UnoType<XRowUpdate>::get(),
                SequenceResultSet::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();

}

css::uno::Sequence< sal_Int8> UpdateableResultSet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString UpdateableResultSet::buildWhereClause()
{
    OUString ret;
    if( m_primaryKey.size() )
    {
        OUStringBuffer buf( 128 );
        buf.append( " WHERE " );
        for( size_t i = 0 ; i < m_primaryKey.size() ; i ++ )
        {
            if( i > 0 )
                buf.append( " AND " );
            sal_Int32 index = findColumn( m_primaryKey[i] );
            bufferQuoteIdentifier( buf, m_primaryKey[i], *m_ppSettings );
            buf.append( " = " );
            bufferQuoteConstant( buf, getString( index ), *m_ppSettings );
        }
        ret = buf.makeStringAndClear();
    }
    return ret;
}


void UpdateableResultSet::insertRow(  )
{
    MutexGuard guard( m_refMutex->mutex );
    if (isLog(*m_ppSettings, LogLevel::Info))
    {
        log(*m_ppSettings, LogLevel::Info, "UpdateableResultSet::insertRow got called");
    }
    if( ! m_insertRow )
        throw SQLException(
            "pq_resultset.insertRow: moveToInsertRow has not been called !",
            *this, OUString(), 1, Any() );

    OUStringBuffer buf( 128 );
    buf.append( "INSERT INTO " );
    bufferQuoteQualifiedIdentifier( buf, m_schema, m_table, *m_ppSettings );
    buf.append( " ( " );

    int columns = 0;
    for( UpdateableFieldVector::size_type i = 0 ; i < m_updateableField.size() ; i++ )
    {
        if( m_updateableField[i].isTouched )
        {
            if( columns > 0 )
                buf.append( ", " );
            columns ++;
            bufferQuoteIdentifier( buf, m_columnNames[i], *m_ppSettings);
        }
    }
    buf.append( " ) VALUES ( " );

    columns = 0;
    for(UpdateableField & i : m_updateableField)
    {
        if( i.isTouched )
        {
            if( columns > 0 )
                buf.append( " , " );
            columns ++;
            bufferQuoteAnyConstant( buf, i.value, *m_ppSettings );

//             OUString val;
//             m_updateableField[i].value >>= val;
//             buf.append( val );
//                 OStringToOUString(val, (*m_ppSettings)->encoding ) );
        }
    }

    buf.append( " )" );

    Reference< XStatement > stmt =
        extractConnectionFromStatement(m_owner)->createStatement();
    DisposeGuard dispGuard( stmt );
    stmt->executeUpdate( buf.makeStringAndClear() );

    // reflect the changes !
    m_rowCount ++;
    m_data.resize( m_rowCount );
    m_data[m_rowCount-1] = std::vector< Any > ( m_fieldCount );
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

void UpdateableResultSet::updateRow(  )
{
    MutexGuard guard( m_refMutex->mutex );
    if (isLog(*m_ppSettings, LogLevel::Info))
    {
        log(*m_ppSettings, LogLevel::Info, "UpdateableResultSet::updateRow got called");
    }
    if( m_insertRow )
        throw SQLException(
            "pq_resultset.updateRow: moveToCurrentRow has not been called !",
            *this, OUString(), 1, Any() );

    OUStringBuffer buf( 128 );
    buf.append( "UPDATE " );
    bufferQuoteQualifiedIdentifier( buf, m_schema, m_table, *m_ppSettings );
    buf.append( "SET " );

    int columns = 0;
    for( UpdateableFieldVector::size_type i = 0; i < m_updateableField.size() ; i ++ )
    {
        if( m_updateableField[i].isTouched )
        {
            if( columns > 0 )
                buf.append( ", " );
            columns ++;

            buf.append( m_columnNames[i] );
            buf.append( " = " );
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

void UpdateableResultSet::deleteRow(  )
{
    if (isLog(*m_ppSettings, LogLevel::Info))
    {
        log(*m_ppSettings, LogLevel::Info, "UpdateableResultSet::deleteRow got called");
    }
    if( m_insertRow )
        throw SQLException(
            "pq_resultset.deleteRow: deleteRow cannot be called when on insert row !",
            *this, OUString(), 1, Any() );

    if( m_row < 0 || m_row >= m_rowCount )
    {
        throw SQLException(
            "deleteRow cannot be called on invalid row ("
            + OUString::number(m_row) + ")",
            *this, OUString(), 0, Any() );
    }

    Reference< XStatement > stmt = extractConnectionFromStatement(m_owner)->createStatement();
    DisposeGuard dispGuard( stmt );
    OUStringBuffer buf( 128 );
    buf.append( "DELETE FROM " );
    bufferQuoteQualifiedIdentifier( buf, m_schema, m_table, *m_ppSettings );
    buf.append( " " );
    buf.append( buildWhereClause() );

    stmt->executeUpdate( buf.makeStringAndClear() );

    // reflect the changes !
    for( int i = m_row + 1; i < m_row ; i ++ )
    {
        m_data[i-1] = m_data[i];
    }
    m_rowCount --;
    m_data.resize( m_rowCount );
 }

void UpdateableResultSet::cancelRowUpdates(  )
{
    MutexGuard guard( m_refMutex->mutex );
    m_updateableField = UpdateableFieldVector();
}

void UpdateableResultSet::moveToInsertRow(  )
{
    m_insertRow = true;
}

void UpdateableResultSet::moveToCurrentRow(  )
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

void UpdateableResultSet::updateNull( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );
    m_updateableField[columnIndex-1].value = Any();
}

void UpdateableResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    Statics &st = getStatics();
    m_updateableField[columnIndex-1].value <<=  ( x ? st.TRUE : st.FALSE );

}

void UpdateableResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x )
{
    updateInt(columnIndex,x);
}

void UpdateableResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x )
{
    updateInt( columnIndex, x );
}

void UpdateableResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x )
{
    updateLong( columnIndex, x );
//     MutexGuard guard( m_refMutex->mutex );
//     checkClosed();
//     checkUpdate( columnIndex );

//     m_updateableField[columnIndex-1].value <<= OUString::valueOf( x );

}

void UpdateableResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x )
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

void UpdateableResultSet::updateFloat( sal_Int32 columnIndex, float x )
{

    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    m_updateableField[columnIndex-1].value <<= OUString::number( x );
}

void UpdateableResultSet::updateDouble( sal_Int32 columnIndex, double x )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    m_updateableField[columnIndex-1].value <<= OUString::number( x );
}

void UpdateableResultSet::updateString( sal_Int32 columnIndex, const OUString& x )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    m_updateableField[columnIndex-1].value <<= x;
}

void UpdateableResultSet::updateBytes( sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkUpdate( columnIndex );

    size_t len;
    unsigned char * escapedString =
        PQescapeBytea( reinterpret_cast<unsigned char const *>(x.getConstArray()), x.getLength(), &len);
    if( ! escapedString )
    {
        throw SQLException(
            "pq_preparedstatement.setBytes: Error during converting bytesequence to an SQL conform string",
            *this, OUString(), 1, Any() );
    }
//     buf.append( (const sal_Char *)escapedString, len -1 );

    m_updateableField[columnIndex-1].value <<=
        OUString( reinterpret_cast<char*>(escapedString), len, RTL_TEXTENCODING_ASCII_US );
    free( escapedString );
}

void UpdateableResultSet::updateDate( sal_Int32 columnIndex, const css::util::Date& x )
{
    updateString( columnIndex, DBTypeConversion::toDateString( x ) );
}

void UpdateableResultSet::updateTime( sal_Int32 columnIndex, const css::util::Time& x )
{
    updateString( columnIndex, DBTypeConversion::toTimeString( x ) );
}

void UpdateableResultSet::updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x )
{
    updateString( columnIndex, DBTypeConversion::toDateTimeString( x ) );
}

void UpdateableResultSet::updateBinaryStream( sal_Int32 /* columnIndex */, const css::uno::Reference< css::io::XInputStream >& /* x */, sal_Int32 /* length */ )
{
}

void UpdateableResultSet::updateCharacterStream( sal_Int32 /* columnIndex */, const css::uno::Reference< css::io::XInputStream >& /* x */, sal_Int32 /* length */ )
{
}

void UpdateableResultSet::updateObject( sal_Int32 /* columnIndex */, const css::uno::Any& /* x */ )
{
}

void UpdateableResultSet::updateNumericObject( sal_Int32 /* columnIndex */, const css::uno::Any& /* x */, sal_Int32 /* scale */ )
{
}


Sequence< Type > UpdateableResultSet::getStaticTypes( bool updateable )
{
    if( updateable )
    {
        cppu::OTypeCollection collection(
            cppu::UnoType<XResultSetUpdate>::get(),
            cppu::UnoType<XRowUpdate>::get(),
//             cppu::UnoType<css::sdbcx::XRowLocate>::get(),
            getStaticTypes( false /* updateable */ ) );
        return collection.getTypes();
    }
    else
    {
        cppu::OTypeCollection collection(
            cppu::UnoType<XResultSet>::get(),
            cppu::UnoType<XResultSetMetaDataSupplier>::get(),
            cppu::UnoType<XRow>::get(),
            cppu::UnoType<XColumnLocate>::get(),
            cppu::UnoType<XCloseable>::get(),
            cppu::UnoType<XPropertySet>::get(),
            cppu::UnoType<XFastPropertySet>::get(),
            cppu::UnoType<XMultiPropertySet>::get(),
            cppu::UnoType<css::lang::XComponent>::get(),  // OComponentHelper
            cppu::UnoType<css::lang::XTypeProvider>::get(),
            cppu::UnoType<css::uno::XAggregation>::get(),
            cppu::UnoType<css::uno::XWeak>::get());
        return collection.getTypes();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
