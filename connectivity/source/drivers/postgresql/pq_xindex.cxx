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

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>

#include "pq_xindex.hxx"
#include "pq_xindexcolumns.hxx"
#include "pq_tools.hxx"
#include "pq_statics.hxx"

using osl::MutexGuard;
using osl::Mutex;


using com::sun::star::container::XNameAccess;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::ElementExistException;
using com::sun::star::container::NoSuchElementException;

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::IndexOutOfBoundsException;

using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::XFastPropertySet;
using com::sun::star::beans::XMultiPropertySet;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::Property;

using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XPreparedStatement;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XParameters;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{
Index::Index( const ::rtl::Reference< RefCountedMutex > & refMutex,
          const Reference< com::sun::star::sdbc::XConnection > & connection,
          ConnectionSettings *pSettings,
          const OUString & schemaName,
          const OUString & tableName )
    : ReflectionBase(
        getStatics().refl.index.implName,
        getStatics().refl.index.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.index.pProps ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{}

Reference< XPropertySet > Index::createDataDescriptor(  ) throw (RuntimeException)
{
    IndexDescriptor * pIndex = new IndexDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pIndex->copyValuesFrom( this );

    return Reference< XPropertySet > ( pIndex );
}

Reference< XNameAccess > Index::getColumns(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if( ! m_indexColumns.is() )
    {
        Sequence< OUString > columnNames;
        getPropertyValue( getStatics().PRIVATE_COLUMN_INDEXES ) >>= columnNames;
        OUString indexName = extractStringProperty( this, getStatics().NAME );
        m_indexColumns = IndexColumns::create(
             m_refMutex, m_conn, m_pSettings, m_schemaName,
             m_tableName, indexName, columnNames );
    }
    return m_indexColumns;
}

Sequence<Type > Index::getTypes() throw( RuntimeException )
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( (Reference< com::sun::star::sdbcx::XColumnsSupplier> *) 0 ),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> Index::getImplementationId() throw( RuntimeException )
{
    return getStatics().refl.index.implementationId;
}

Any Index::queryInterface( const Type & reqType ) throw (RuntimeException)
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< com::sun::star::sdbcx::XColumnsSupplier * > ( this ) );
    return ret;
}



IndexDescriptor::IndexDescriptor(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const Reference< com::sun::star::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings )
    : ReflectionBase(
        getStatics().refl.indexDescriptor.implName,
        getStatics().refl.indexDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.indexDescriptor.pProps )
{}

Reference< XPropertySet > IndexDescriptor::createDataDescriptor(  ) throw (RuntimeException)
{
    IndexDescriptor * pIndex = new IndexDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pIndex->copyValuesFrom( this );
    return Reference< XPropertySet > ( pIndex );
}

Reference< XNameAccess > IndexDescriptor::getColumns(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if( ! m_indexColumns.is() )
    {
        m_indexColumns = IndexColumnDescriptors::create(
            m_refMutex, m_conn, m_pSettings );
//         Sequence< OUString > columnNames;
//         getPropertyValue( getStatics().PRIVATE_COLUMN_INDEXES ) >>= columnNames;
//         OUString indexName = extractStringProperty( this, getStatics().NAME );
//         m_indexColumns = IndexColumns::create(
//              m_refMutex, m_conn, m_pSettings, m_schemaName,
//              m_tableName, indexName, columnNames );
    }
    return m_indexColumns;
}

Sequence<Type > IndexDescriptor::getTypes() throw( RuntimeException )
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( (Reference< com::sun::star::sdbcx::XColumnsSupplier> *) 0 ),
                ReflectionBase::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> IndexDescriptor::getImplementationId() throw( RuntimeException )
{
    return getStatics().refl.indexDescriptor.implementationId;
}

Any IndexDescriptor::queryInterface( const Type & reqType ) throw (RuntimeException)
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< com::sun::star::sdbcx::XColumnsSupplier * > ( this ) );
    return ret;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
