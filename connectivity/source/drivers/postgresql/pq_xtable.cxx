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

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include "pq_xtable.hxx"
#include "pq_xtables.hxx"
#include "pq_xviews.hxx"
#include "pq_xindexes.hxx"
#include "pq_xkeys.hxx"
#include "pq_xcolumns.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

using osl::MutexGuard;
using osl::Mutex;

using com::sun::star::container::XNameAccess;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::NoSuchElementException;

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::beans::XPropertySet;

using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{
Table::Table( const ::rtl::Reference< RefCountedMutex > & refMutex,
              const Reference< css::sdbc::XConnection > & connection,
              ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.table.implName,
        getStatics().refl.table.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.table.pProps ),
      m_pColumns( nullptr )
{}

Reference< XPropertySet > Table::createDataDescriptor(  )
{
    TableDescriptor * pTable = new TableDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pTable->copyValuesFrom( this );

    return Reference< XPropertySet > ( pTable );
}

Reference< XNameAccess > Table::getColumns(  )
{
    if( ! m_columns.is() )
    {
        m_columns = Columns::create(
            m_refMutex,
            m_conn,
            m_pSettings,
            extractStringProperty( this, getStatics().SCHEMA_NAME ),
            extractStringProperty( this, getStatics().NAME ),
            &m_pColumns);
    }
    return m_columns;
}

Reference< XNameAccess > Table::getIndexes()
{
    if( ! m_indexes.is() )
    {
        m_indexes = ::pq_sdbc_driver::Indexes::create(
            m_refMutex,
            m_conn,
            m_pSettings,
            extractStringProperty( this, getStatics().SCHEMA_NAME ),
            extractStringProperty( this, getStatics().NAME ) );
    }
    return m_indexes;
}

Reference< XIndexAccess > Table::getKeys(  )
{
    if( ! m_keys.is() )
    {
        m_keys = ::pq_sdbc_driver::Keys::create(
            m_refMutex,
            m_conn,
            m_pSettings,
            extractStringProperty( this, getStatics().SCHEMA_NAME ),
            extractStringProperty( this, getStatics().NAME ) );
    }
    return m_keys;
}

void Table::rename( const OUString& newName )
{
    MutexGuard guard( m_refMutex->mutex );
    Statics & st = getStatics();

    OUString oldName = extractStringProperty(this,st.NAME );
    OUString schema = extractStringProperty(this,st.SCHEMA_NAME );
    OUString fullOldName = concatQualified( schema, oldName );

    OUString newTableName;
    OUString newSchemaName;
    // OOo2.0 passes schema + dot + new-table-name while
    // OO1.1.x passes new Name without schema
    // in case name contains a dot, it is interpreted as schema.tablename
    if( newName.indexOf( '.' ) >= 0 )
    {
        splitConcatenatedIdentifier( newName, &newSchemaName, &newTableName );
    }
    else
    {
        newTableName = newName;
        newSchemaName = schema;
    }
    OUString fullNewName = concatQualified( newSchemaName, newTableName );

    if( extractStringProperty( this, st.TYPE ).equals( st.VIEW ) && m_pSettings->views.is() )
    {
        // maintain view list (really strange API !)
        Any a = m_pSettings->pViewsImpl->getByName( fullOldName );
        Reference< css::sdbcx::XRename > Xrename;
        a >>= Xrename;
        if( Xrename.is() )
        {
            Xrename->rename( newName );
            setPropertyValue_NoBroadcast_public( st.SCHEMA_NAME, makeAny(newSchemaName) );
        }
    }
    else
    {
        if( ! newSchemaName.equals(schema) )
        {
            // try new schema name first
            try
            {
                OUStringBuffer buf(128);
                buf.append( "ALTER TABLE" );
                bufferQuoteQualifiedIdentifier(buf, schema, oldName, m_pSettings );
                buf.append( "SET SCHEMA" );
                bufferQuoteIdentifier( buf, newSchemaName, m_pSettings );
                Reference< XStatement > statement = m_conn->createStatement();
                statement->executeUpdate( buf.makeStringAndClear() );
                setPropertyValue_NoBroadcast_public( st.SCHEMA_NAME, makeAny(newSchemaName) );
                disposeNoThrow( statement );
                schema = newSchemaName;
            }
            catch( css::sdbc::SQLException &e )
            {
                OUString buf( e.Message + "(NOTE: Only postgresql server >= V8.1 support changing a table's schema)" );
                e.Message = buf;
                throw;
            }

        }
        if( ! newTableName.equals( oldName ) ) // might also be just the change of a schema name
        {
            OUStringBuffer buf(128);
            buf.append( "ALTER TABLE" );
            bufferQuoteQualifiedIdentifier(buf, schema, oldName, m_pSettings );
            buf.append( "RENAME TO" );
            bufferQuoteIdentifier( buf, newTableName, m_pSettings );
            Reference< XStatement > statement = m_conn->createStatement();
            statement->executeUpdate( buf.makeStringAndClear() );
            disposeNoThrow( statement );
        }
    }
    setPropertyValue_NoBroadcast_public( st.NAME, makeAny(newTableName) );
    // inform the container of the name change !
    if( m_pSettings->tables.is() )
    {
        m_pSettings->pTablesImpl->rename( fullOldName, fullNewName );
    }
}

void Table::alterColumnByName(
    const OUString& colName,
    const Reference< XPropertySet >& descriptor )
{
    Reference< css::container::XNameAccess > columns =
        Reference< css::container::XNameAccess > ( getColumns(), UNO_QUERY );

    OUString newName = extractStringProperty(descriptor, getStatics().NAME );
    ::pq_sdbc_driver::alterColumnByDescriptor(
        extractStringProperty( this, getStatics().SCHEMA_NAME ),
        extractStringProperty( this, getStatics().NAME ),
        m_pSettings,
        m_conn->createStatement(),
        Reference< css::beans::XPropertySet>( columns->getByName( colName ), UNO_QUERY) ,
        descriptor );

    if( colName !=  newName )
    {
//         m_pColumns->rename( colName, newName );
        m_pColumns->refresh();
    }
}

void Table::alterColumnByIndex(
    sal_Int32 index,
    const css::uno::Reference< css::beans::XPropertySet >& descriptor )
{
    Reference< css::container::XIndexAccess > columns =
        Reference< css::container::XIndexAccess>( getColumns(), UNO_QUERY );
    Reference< css::beans::XPropertySet> column(columns->getByIndex( index ), UNO_QUERY );
    ::pq_sdbc_driver::alterColumnByDescriptor(
        extractStringProperty( this, getStatics().SCHEMA_NAME ),
        extractStringProperty( this, getStatics().NAME ),
        m_pSettings,
        m_conn->createStatement(),
        column,
        descriptor );
    m_pColumns->refresh();
}

Sequence<Type > Table::getTypes()
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<css::sdbcx::XIndexesSupplier>::get(),
                cppu::UnoType<css::sdbcx::XKeysSupplier>::get(),
                cppu::UnoType<css::sdbcx::XColumnsSupplier>::get(),
                cppu::UnoType<css::sdbcx::XRename>::get(),
                cppu::UnoType<css::sdbcx::XAlterTable>::get(),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> Table::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Any Table::queryInterface( const Type & reqType )
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< css::sdbcx::XIndexesSupplier * > ( this ),
            static_cast< css::sdbcx::XKeysSupplier * > ( this ),
            static_cast< css::sdbcx::XColumnsSupplier * > ( this ),
            static_cast< css::sdbcx::XRename * > ( this ),
            static_cast< css::sdbcx::XAlterTable * > ( this )
            );
    return ret;
}

OUString Table::getName(  )
{
    Statics & st = getStatics();
    return concatQualified(
        extractStringProperty( this, st.SCHEMA_NAME ),
        extractStringProperty( this, st.NAME ) );
}

void Table::setName( const OUString& aName )
{
    rename( aName );
}


TableDescriptor::TableDescriptor(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const Reference< css::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.tableDescriptor.implName,
        getStatics().refl.tableDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.tableDescriptor.pProps )
{
}

Reference< XNameAccess > TableDescriptor::getColumns(  )
{
    if( ! m_columns.is() )
    {
        m_columns = new ColumnDescriptors(m_refMutex, m_conn, m_pSettings );
    }
    return m_columns;
}

Reference< XNameAccess > TableDescriptor::getIndexes()
{
    if( ! m_indexes.is() )
    {
        m_indexes = ::pq_sdbc_driver::IndexDescriptors::create(
            m_refMutex,
            m_conn,
            m_pSettings);
    }
    return m_indexes;
}

Reference< XIndexAccess > TableDescriptor::getKeys(  )
{
    if( ! m_keys.is() )
    {
        m_keys = ::pq_sdbc_driver::KeyDescriptors::create(
            m_refMutex,
            m_conn,
            m_pSettings );
    }
    return m_keys;
}


Sequence<Type > TableDescriptor::getTypes()
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<css::sdbcx::XIndexesSupplier>::get(),
                cppu::UnoType<css::sdbcx::XKeysSupplier>::get(),
                cppu::UnoType<css::sdbcx::XColumnsSupplier>::get(),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> TableDescriptor::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Any TableDescriptor::queryInterface( const Type & reqType )
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< css::sdbcx::XIndexesSupplier * > ( this ),
            static_cast< css::sdbcx::XKeysSupplier * > ( this ),
            static_cast< css::sdbcx::XColumnsSupplier * > ( this ));
    return ret;
}


Reference< XPropertySet > TableDescriptor::createDataDescriptor(  )
{
    TableDescriptor * pTable = new TableDescriptor(
        m_refMutex, m_conn, m_pSettings );

    // TODO: deep copies
    pTable->m_values = m_values;

    return Reference< XPropertySet > ( pTable );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
