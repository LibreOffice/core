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

#include "pq_xkey.hxx"
#include "pq_xkeycolumns.hxx"
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
Key::Key( const ::rtl::Reference< RefCountedMutex > & refMutex,
          const Reference< com::sun::star::sdbc::XConnection > & connection,
          ConnectionSettings *pSettings,
          const OUString & schemaName,
          const OUString & tableName )
    : ReflectionBase(
        getStatics().refl.key.implName,
        getStatics().refl.key.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.key.pProps ),
      m_schemaName( schemaName ),
      m_tableName( tableName )
{}

Reference< XPropertySet > Key::createDataDescriptor(  ) throw (RuntimeException)
{
    KeyDescriptor * pKeyDescriptor = new KeyDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pKeyDescriptor->copyValuesFrom( this );

    return Reference< XPropertySet > ( pKeyDescriptor );
}

Reference< XNameAccess > Key::getColumns(  ) throw (::com::sun::star::uno::RuntimeException)
{
    // TODO: cash columns object !
    if( !m_keyColumns.is() )
    {
        Sequence< OUString > columnNames, foreignColumnNames;
        getPropertyValue( getStatics().PRIVATE_COLUMNS ) >>= columnNames;
        getPropertyValue( getStatics().PRIVATE_FOREIGN_COLUMNS ) >>= foreignColumnNames;

        m_keyColumns = KeyColumns::create(
            m_refMutex, m_conn, m_pSettings, m_schemaName,
            m_tableName, columnNames, foreignColumnNames );
    }
    return m_keyColumns;
}

Sequence<Type > Key::getTypes() throw( RuntimeException )
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

Sequence< sal_Int8> Key::getImplementationId() throw( RuntimeException )
{
    return getStatics().refl.key.implementationId;
}

Any Key::queryInterface( const Type & reqType ) throw (RuntimeException)
{
    Any ret;

    ret = ReflectionBase::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< com::sun::star::sdbcx::XColumnsSupplier * > ( this ) );
    return ret;
}



KeyDescriptor::KeyDescriptor( const ::rtl::Reference< RefCountedMutex > & refMutex,
          const Reference< com::sun::star::sdbc::XConnection > & connection,
          ConnectionSettings *pSettings )
    : ReflectionBase(
        getStatics().refl.keyDescriptor.implName,
        getStatics().refl.keyDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.keyDescriptor.pProps )
{
}

Reference< XPropertySet > KeyDescriptor::createDataDescriptor(  ) throw (RuntimeException)
{
    KeyDescriptor * pKeyDescriptor = new KeyDescriptor(
        m_refMutex, m_conn, m_pSettings );
    pKeyDescriptor->copyValuesFrom( this );

    return Reference< XPropertySet > ( pKeyDescriptor );
}

Reference< XNameAccess > KeyDescriptor::getColumns(  ) throw (::com::sun::star::uno::RuntimeException)
{
    // TODO: cash columns object !
    if( !m_keyColumns.is() )
    {
        m_keyColumns = new KeyColumnDescriptors( m_refMutex, m_conn, m_pSettings );
    }
    return m_keyColumns;
}

Sequence<Type > KeyDescriptor::getTypes() throw( RuntimeException )
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

Sequence< sal_Int8> KeyDescriptor::getImplementationId() throw( RuntimeException )
{
    return getStatics().refl.keyDescriptor.implementationId;
}

Any KeyDescriptor::queryInterface( const Type & reqType ) throw (RuntimeException)
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
