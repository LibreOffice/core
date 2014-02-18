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

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include "pq_xtables.hxx"
#include "pq_xviews.hxx"
#include "pq_xtable.hxx"
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
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XEnumeration;
using com::sun::star::lang::WrappedTargetException;

using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XDatabaseMetaData;
using com::sun::star::sdbcx::XColumnsSupplier;
using com::sun::star::sdbcx::XKeysSupplier;
using com::sun::star::sdbcx::XViewsSupplier;

namespace pq_sdbc_driver
{
Tables::Tables(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings )
    : Container( refMutex, origin, pSettings,  getStatics().TABLE )
{}

Tables::~Tables()
{}

void Tables::refresh()
    throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        osl::MutexGuard guard( m_refMutex->mutex );
        Statics & st = getStatics();

        Reference< XDatabaseMetaData > meta = m_origin->getMetaData();

        Reference< XResultSet > rs =
            meta->getTables( Any(), st.cPERCENT,st.cPERCENT, Sequence< OUString > () );

        Reference< XRow > xRow( rs , UNO_QUERY );

        String2IntMap map;

        m_values = Sequence< com::sun::star::uno::Any > ();
        sal_Int32 tableIndex = 0;
        while( rs->next() )
        {
            // if creating all these tables turns out to have too bad performance, we might
            // instead offer a factory interface
            Table * pTable =
                new Table( m_refMutex, m_origin, m_pSettings );
            Reference< com::sun::star::beans::XPropertySet > prop = pTable;

            OUString name = xRow->getString( TABLE_INDEX_NAME+1);
            OUString schema = xRow->getString( TABLE_INDEX_SCHEMA+1);
            pTable->setPropertyValue_NoBroadcast_public(
                st.CATALOG_NAME , makeAny(xRow->getString( TABLE_INDEX_CATALOG+1) ) );
            pTable->setPropertyValue_NoBroadcast_public( st.NAME , makeAny( name ) );
            pTable->setPropertyValue_NoBroadcast_public( st.SCHEMA_NAME , makeAny( schema ));
            pTable->setPropertyValue_NoBroadcast_public(
                st.TYPE , makeAny( xRow->getString( TABLE_INDEX_TYPE+1) ) );
            pTable->setPropertyValue_NoBroadcast_public(
                st.DESCRIPTION , makeAny( xRow->getString( TABLE_INDEX_REMARKS+1) ) );
            pTable->setPropertyValue_NoBroadcast_public(
                st.PRIVILEGES ,
                makeAny( (sal_Int32)
                         ( com::sun::star::sdbcx::Privilege::SELECT |
                           com::sun::star::sdbcx::Privilege::INSERT |
                           com::sun::star::sdbcx::Privilege::UPDATE |
                           com::sun::star::sdbcx::Privilege::DELETE |
                           com::sun::star::sdbcx::Privilege::READ |
                           com::sun::star::sdbcx::Privilege::CREATE |
                           com::sun::star::sdbcx::Privilege::ALTER |
                           com::sun::star::sdbcx::Privilege::REFERENCE |
                           com::sun::star::sdbcx::Privilege::DROP ) ) );

            {
                const int currentTableIndex = tableIndex++;
                assert(currentTableIndex  == m_values.getLength());
                m_values.realloc( tableIndex );
                m_values[currentTableIndex] = makeAny( prop );
                OUStringBuffer buf( name.getLength() + schema.getLength() + 1);
                buf.append( schema + "." + name );
                map[ buf.makeStringAndClear() ] = currentTableIndex;
            }
        }
        m_name2index.swap( map );
    }
    catch ( const com::sun::star::sdbc::SQLException & e )
    {
        throw RuntimeException( e.Message , e.Context );
    }

    fire( RefreshedBroadcaster( *this ) );
}


static void appendColumnList(
    OUStringBuffer &buf, const Reference< XColumnsSupplier > & columnSupplier, ConnectionSettings *settings )
{
    if( columnSupplier.is() )
    {
        Reference< XEnumerationAccess > columns( columnSupplier->getColumns(),UNO_QUERY );
        if( columns.is() )
        {
            Reference< XEnumeration > xEnum( columns->createEnumeration() );
            bool first = true;
            Statics & st = getStatics();

            while( xEnum.is() && xEnum->hasMoreElements() )
            {
                if( first )
                {
                    first = false;
                }
                else
                {
                    buf.append( ", " );
                }
                Reference< XPropertySet > column( xEnum->nextElement(), UNO_QUERY );
                OUString name = extractStringProperty( column, st.NAME );
                OUString defaultValue = extractStringProperty( column, st.DEFAULT_VALUE );
                sal_Bool isNullable = extractBoolProperty( column, st.IS_NULLABLE );
                sal_Bool isAutoIncrement = extractBoolProperty( column, st.IS_AUTO_INCREMENT );

                bufferQuoteIdentifier( buf, name, settings );

                OUString type = sqltype2string( column );
                if( isAutoIncrement )
                {
                    sal_Int32 dataType = 0;
                    column->getPropertyValue( st.TYPE ) >>= dataType;
                    if( com::sun::star::sdbc::DataType::INTEGER == dataType )
                    {
                        buf.append( " serial  ");
                        isNullable = sal_False;
                    }
                    else if( com::sun::star::sdbc::DataType::BIGINT == dataType )
                    {
                        buf.append( " serial8 " );
                        isNullable = sal_False;
                    }
                    else
                        buf.append( type );
                }
                else
                {
                    buf.append( type );
                }
                if( !defaultValue.isEmpty() )
                {
                    bufferQuoteConstant( buf, defaultValue, settings );
                }

                if( ! isNullable )
                    buf.append( " NOT NULL " );

            }
        }
    }
}

static void appendKeyList(
    OUStringBuffer & buf, const Reference< XKeysSupplier > &keySupplier, ConnectionSettings *settings )
{
    if( keySupplier.is() )
    {
        Reference< XEnumerationAccess > keys( keySupplier->getKeys(), UNO_QUERY );
        if(keys.is() )
        {
            Reference< XEnumeration > xEnum = keys->createEnumeration();
            while( xEnum.is() && xEnum->hasMoreElements() )
            {
                buf.append( ", " );
                Reference< XPropertySet > key( xEnum->nextElement(), UNO_QUERY );
                bufferKey2TableConstraint( buf, key, settings );
            }
        }
    }
}

void Tables::appendByDescriptor(
    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::container::ElementExistException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    Reference< XStatement > stmt =
        m_origin->createStatement();

    Statics &st = getStatics();
    OUString name,schema;
    descriptor->getPropertyValue( st.SCHEMA_NAME ) >>= schema;
    descriptor->getPropertyValue( st.NAME ) >>= name;

    TransactionGuard transaction( stmt );

    OUStringBuffer buf( 128 );
    buf.append( "CREATE TABLE" );
    bufferQuoteQualifiedIdentifier( buf, schema, name , m_pSettings);
    buf.append( "(" );

    // columns
    Reference< XColumnsSupplier > supplier( descriptor, UNO_QUERY );
    appendColumnList( buf, supplier, m_pSettings );

    appendKeyList( buf, Reference< XKeysSupplier >( descriptor, UNO_QUERY ), m_pSettings );

    buf.append( ") " );
    // execute the creation !
    transaction.executeUpdate( buf.makeStringAndClear() );

    // description ....
    OUString description = extractStringProperty( descriptor, st.DESCRIPTION );
    if( !description.isEmpty() )
    {
        buf = OUStringBuffer( 128 );
        buf.append( "COMMENT ON TABLE" );
        bufferQuoteQualifiedIdentifier( buf, schema, name, m_pSettings );
        buf.append( "IS " );
        bufferQuoteConstant( buf, description, m_pSettings);

        transaction.executeUpdate( buf.makeStringAndClear() );
    }

    // column descriptions
    if( supplier.is() )
    {
        Reference< XEnumerationAccess > columns( supplier->getColumns(),UNO_QUERY );
        if( columns.is() )
        {
            Reference< XEnumeration > xEnum( columns->createEnumeration() );
            while( xEnum.is() && xEnum->hasMoreElements() )
            {
                Reference< XPropertySet > column( xEnum->nextElement(), UNO_QUERY );
                description = extractStringProperty( column,st.DESCRIPTION );
                if( !description.isEmpty() )
                {
                    buf = OUStringBuffer( 128 );
                    buf.append( "COMMENT ON COLUMN " );
                    bufferQuoteQualifiedIdentifier(
                        buf, schema, name, extractStringProperty( column, st.NAME ), m_pSettings );
                    buf.append( "IS " );
                    bufferQuoteConstant( buf, description, m_pSettings );
                    transaction.executeUpdate( buf.makeStringAndClear() );
                }
            }
        }
    }

    transaction.commit();

    disposeNoThrow( stmt );
        // TODO: cheaper recalculate
//        Container::append( concatQualified( schema, name ), descriptor ); // maintain the lists
    refresh();
}

void Tables::dropByIndex( sal_Int32 index )
    throw (::com::sun::star::sdbc::SQLException,
           ::com::sun::star::lang::IndexOutOfBoundsException,
           ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard( m_refMutex->mutex );
    if( index < 0 ||  index >= m_values.getLength() )
    {
        OUStringBuffer buf( 128 );
        buf.append( "TABLES: Index out of range (allowed 0 to " + OUString::number(m_values.getLength() -1) +
                    ", got " + OUString::number( index ) + ")" );
        throw com::sun::star::lang::IndexOutOfBoundsException( buf.makeStringAndClear(), *this );
    }

    Reference< XPropertySet > set;
    m_values[index] >>= set;
    Statics &st = getStatics();
    OUString name,schema;
    set->getPropertyValue( st.SCHEMA_NAME ) >>= schema;
    set->getPropertyValue( st.NAME ) >>= name;
    if( extractStringProperty( set, st.TYPE ).equals( st.VIEW ) && m_pSettings->views.is() )
    {
        m_pSettings->pViewsImpl->dropByName( concatQualified( schema, name ) );
    }
    else
    {
        OUStringBuffer update( 128 );
        update.append( "DROP " );
        if( extractStringProperty( set, st.TYPE ).equals( st.VIEW ) )
            update.append( "VIEW " );
        else
            update.append( "TABLE " );
        bufferQuoteQualifiedIdentifier( update, schema, name, m_pSettings );
        Reference< XStatement > stmt = m_origin->createStatement( );
        DisposeGuard dispGuard( stmt );
        stmt->executeUpdate( update.makeStringAndClear() );
    }

    Container::dropByIndex( index );
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > Tables::createDataDescriptor()
        throw (::com::sun::star::uno::RuntimeException)
{
    return new TableDescriptor( m_refMutex, m_origin, m_pSettings );
}

Reference< com::sun::star::container::XNameAccess > Tables::create(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
    ConnectionSettings *pSettings,
    Tables **ppTables)
{
    *ppTables = new Tables( refMutex, origin, pSettings );
    Reference< com::sun::star::container::XNameAccess > ret = *ppTables;
    (*ppTables)->refresh();

    return ret;
}

void Tables::disposing()
{
    Container::disposing();
}

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
