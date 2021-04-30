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

#pragma once

#include <cppuhelper/component.hxx>
#include <cppuhelper/propshlp.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "pq_xbase.hxx"

namespace pq_sdbc_driver
{

class Columns;

class Table : public ReflectionBase,
              public css::sdbcx::XColumnsSupplier,
              public css::sdbcx::XIndexesSupplier,
              public css::sdbcx::XKeysSupplier,
              public css::sdbcx::XRename,
              public css::sdbcx::XAlterTable
{
    css::uno::Reference< css::container::XNameAccess > m_columns;
    css::uno::Reference< css::container::XIndexAccess > m_keys;
    css::uno::Reference< css::container::XNameAccess > m_indexes;
    rtl::Reference<Columns> m_pColumns;

public:
    Table( const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
           const css::uno::Reference< css::sdbc::XConnection > & connection,
           ConnectionSettings *pSettings);

    // XInterface
    virtual void SAL_CALL acquire() noexcept override { ReflectionBase::acquire(); }
    virtual void SAL_CALL release() noexcept override { ReflectionBase::release(); }
    virtual css::uno::Any  SAL_CALL queryInterface(
        const css::uno::Type & reqType ) override;

    // XTypeProvider, first implemented by OPropertySetHelper
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId() override;

    // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) override;

    // XColumnsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
    getColumns(  ) override;

    // XIndexesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
    getIndexes(  ) override;

    // XKeysSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
    getKeys(  ) override;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) override;

    // XAlterTable
    virtual void SAL_CALL alterColumnByName(
        const OUString& colName,
        const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;

    virtual void SAL_CALL alterColumnByIndex(
        sal_Int32 index,
        const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;
};


class TableDescriptor
    : public ReflectionBase,
      public css::sdbcx::XColumnsSupplier,
      public css::sdbcx::XIndexesSupplier,
      public css::sdbcx::XKeysSupplier
{
    css::uno::Reference< css::container::XNameAccess > m_columns;
    css::uno::Reference< css::container::XIndexAccess > m_keys;
    css::uno::Reference< css::container::XNameAccess > m_indexes;

public:
    TableDescriptor(
        const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
        const css::uno::Reference< css::sdbc::XConnection > & connection,
        ConnectionSettings *pSettings);

public: // XInterface
    virtual void SAL_CALL acquire() noexcept override { ReflectionBase::acquire(); }
    virtual void SAL_CALL release() noexcept override { ReflectionBase::release(); }
    virtual css::uno::Any  SAL_CALL queryInterface(
        const css::uno::Type & reqType ) override;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId() override;

public: // XColumnsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
    getColumns(  ) override;

public: // XIndexesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
    getIndexes(  ) override;

public: // XKeysSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
    getKeys(  ) override;

public: // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) override;
};


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
