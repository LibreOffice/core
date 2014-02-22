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
 *    file, You can obtain one at http:
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>

#include <cppuhelper/implbase1.hxx>

#include "pq_xcolumns.hxx"
#include "pq_xcolumn.hxx"
#include "pq_statics.hxx"
#include "pq_tools.hxx"

using osl::MutexGuard;


using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertyChangeListener;
using com::sun::star::beans::PropertyChangeEvent;

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

static Any isCurrency( const OUString & typeName )
{
    sal_Bool b = typeName.equalsIgnoreAsciiCase("money");
    return Any( &b, getBooleanCppuType() );
}







static Any isAutoIncrement( const OUString & defaultValue )
{
    sal_Bool ret = defaultValue.startsWith( "nextval(" );













    return Any ( &ret, getBooleanCppuType() );
}

Columns::Columns(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName)
    : Container( refMutex, origin, pSettings,  "COLUMN" ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{}

Columns::~Columns()
{}

OUString columnMetaData2SDBCX(
    ReflectionBase *pBase, const com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > &xRow )
{
    Statics & st = getStatics();

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    static const int COLUMN_NAME = 4;
    static const int DATA_TYPE = 5;
    static const int TYPE_NAME = 6;
    static const int COLUMN_SIZE = 7;
    static const int DECIMAL_DIGITS = 9;
    static const int IS_NULLABLE = 11;
    static const int DESCRIPTION = 12;
    static const int DEFAULT_VALUE = 13;

    OUString name = xRow->getString( COLUMN_NAME );
    OUString typeName  = xRow->getString( TYPE_NAME );

    pBase->setPropertyValue_NoBroadcast_public(
        st.NAME, makeAny( name ) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.TYPE, makeAny( xRow->getInt( DATA_TYPE ) ) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.TYPE_NAME, makeAny( typeName ) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.PRECISION, makeAny( xRow->getInt( COLUMN_SIZE ) ) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.SCALE, makeAny( xRow->getInt( DECIMAL_DIGITS ) ) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.IS_NULLABLE, makeAny( xRow->getInt( IS_NULLABLE ) ) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.DEFAULT_VALUE, makeAny( xRow->getString( DEFAULT_VALUE ) ) );








        pBase->setPropertyValue_NoBroadcast_public(
            st.DESCRIPTION, makeAny( xRow->getString( DESCRIPTION ) ) );


    
    pBase->setPropertyValue_NoBroadcast_public(
        st.IS_AUTO_INCREMENT, isAutoIncrement(xRow->getString( DEFAULT_VALUE )) );

    pBase->setPropertyValue_NoBroadcast_public(
        st.IS_CURRENCY, isCurrency( typeName));
    return name;
}






















































void Columns::refresh()
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        if( isLog( m_pSettings, LogLevel::INFO ) )
        {
            OStringBuffer buf;
            buf.append( "sdbcx.Columns get refreshed for table " );
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
        int columnIndex = 0;
        while( rs->next() )
        {
            Column * pColumn =
                new Column( m_refMutex, m_origin, m_pSettings );
            Reference< com::sun::star::beans::XPropertySet > prop = pColumn;

            OUString name = columnMetaData2SDBCX( pColumn, xRow );










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


void alterColumnByDescriptor(
    const OUString & schemaName,
    const OUString & tableName,
    ConnectionSettings *settings,
    const Reference< XStatement > &stmt,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & past,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & future)
{
    Statics & st  = getStatics();
















    OUString pastColumnName = extractStringProperty( past, st.NAME );
    OUString futureColumnName = extractStringProperty( future, st.NAME );
    OUString pastTypeName = sqltype2string( past );
    OUString futureTypeName = sqltype2string( future );

    TransactionGuard transaction( stmt );

    OUStringBuffer buf( 128 );
    if( ! pastColumnName.getLength())
    {
        
        buf.append( "ALTER TABLE" );
        bufferQuoteQualifiedIdentifier( buf, schemaName, tableName, settings );
        buf.append( "ADD COLUMN" );
        bufferQuoteIdentifier( buf, futureColumnName, settings );
        buf.append( futureTypeName );
        transaction.executeUpdate( buf.makeStringAndClear() );
    }
    else
    {
        if( pastTypeName != futureTypeName )
        {
            throw RuntimeException(
                "Can't modify column types, drop the column and create a new one",
                Reference< XInterface > () );
        }

        if( pastColumnName != futureColumnName )
        {
            buf.append( "ALTER TABLE" );
            bufferQuoteQualifiedIdentifier( buf, schemaName, tableName, settings );
            buf.append( "RENAME COLUMN" );
            bufferQuoteIdentifier( buf, pastColumnName, settings );
            buf.append( "TO" );
            bufferQuoteIdentifier( buf, futureColumnName, settings );
            transaction.executeUpdate( buf.makeStringAndClear() );
        }
    }

    OUString futureDefaultValue = extractStringProperty( future, st.DEFAULT_VALUE );
    OUString pastDefaultValue = extractStringProperty( past, st.DEFAULT_VALUE );
    if( futureDefaultValue != pastDefaultValue )
    {
        buf = OUStringBuffer( 128 );
        buf.append( "ALTER TABLE" );
        bufferQuoteQualifiedIdentifier( buf, schemaName, tableName, settings );
        buf.append( "ALTER COLUMN" );
        bufferQuoteIdentifier( buf, futureColumnName, settings );
        buf.append( "SET DEFAULT " );
        
        
        
        buf.append( futureDefaultValue );

        transaction.executeUpdate( buf.makeStringAndClear() );
    }

    sal_Int32 futureNullable = extractIntProperty( future, st.IS_NULLABLE );
    sal_Int32 pastNullable = extractIntProperty( past, st.IS_NULLABLE );
    if( futureNullable != pastNullable )
    {
        buf = OUStringBuffer( 128 );
        buf.append( "ALTER TABLE" );
        bufferQuoteQualifiedIdentifier( buf, schemaName, tableName, settings );
        buf.append( "ALTER COLUMN" );
        bufferQuoteIdentifier( buf, futureColumnName, settings );
        if( futureNullable == com::sun::star::sdbc::ColumnValue::NO_NULLS )
        {
            buf.append( "SET" );
        }
        else
        {
            buf.append( "DROP" );
        }
        buf.append( " NOT NULL" );
        transaction.executeUpdate( buf.makeStringAndClear() );
    }






    OUString futureComment = extractStringProperty( future, st.DESCRIPTION );
    OUString pastComment = extractStringProperty( past, st.DESCRIPTION );

    if( futureComment != pastComment )
    {
        buf = OUStringBuffer( 128 );
        buf.append( "COMMENT ON COLUMN" );
        bufferQuoteQualifiedIdentifier( buf, schemaName, tableName , futureColumnName, settings );
        buf.append( "IS " );
        bufferQuoteConstant( buf, futureComment, settings );
        transaction.executeUpdate( buf.makeStringAndClear() );
    }
    transaction.commit();
}

void Columns::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& future )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    Statics & st = getStatics();
    Reference< XPropertySet > past = createDataDescriptor();
    past->setPropertyValue( st.IS_NULLABLE, makeAny( com::sun::star::sdbc::ColumnValue::NULLABLE ) );
    alterColumnByDescriptor(
        m_schemaName, m_tableName, m_pSettings, m_origin->createStatement() , past, future  );

    refresh();
}























void Columns::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( "COLUMNS: Index out of range (allowed 0 to " );
        buf.append((sal_Int32)(m_values.getLength() -1) );
        buf.appendAscii( ", got " );
        buf.append( index );
        buf.appendAscii( ")" );
        throw com::sun::star::lang::IndexOutOfBoundsException(
            buf.makeStringAndClear(), *this );
    }

    Reference< XPropertySet > set;
    m_values[index] >>= set;
    Statics &st = getStatics();
    OUString name;
    set->getPropertyValue( st.NAME ) >>= name;

    OUStringBuffer update( 128 );
    update.appendAscii( "ALTER TABLE ONLY");
    bufferQuoteQualifiedIdentifier( update, m_schemaName, m_tableName, m_pSettings );
    update.appendAscii( "DROP COLUMN" );
    bufferQuoteIdentifier( update, name, m_pSettings );
    Reference< XStatement > stmt = m_origin->createStatement( );
    DisposeGuard disposeIt( stmt );
    stmt->executeUpdate( update.makeStringAndClear() );

    Container::dropByIndex( index );
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > Columns::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new ColumnDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< com::sun::star::container::XNameAccess > Columns::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    const OUString &schemaName,
    const OUString &tableName,
    Columns **ppColumns)
{
    *ppColumns = new Columns(
        refMutex, origin, pSettings, schemaName, tableName );
    Reference< com::sun::star::container::XNameAccess > ret = *ppColumns;
    (*ppColumns)->refresh();

    return ret;
}



ColumnDescriptors::ColumnDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings )
    : Container( refMutex, origin, pSettings,  "COLUMN-DESCRIPTOR" )
{}


Reference< ::com::sun::star::beans::XPropertySet > ColumnDescriptors::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new ColumnDescriptor( m_refMutex, m_origin, m_pSettings );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
