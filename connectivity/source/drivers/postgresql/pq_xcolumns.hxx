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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XCOLUMNS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XCOLUMNS_HXX

#include "pq_xcontainer.hxx"
#include "pq_xbase.hxx"

namespace com { namespace sun { namespace star { namespace sdbc { class XRow; } } } }

namespace pq_sdbc_driver
{

void alterColumnByDescriptor(
    const OUString & schemaName,
    const OUString & tableName,
    ConnectionSettings *settings,
    const com::sun::star::uno::Reference< com::sun::star::sdbc::XStatement > &stmt,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & past,
    const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > & future);

OUString columnMetaData2SDBCX(
    ReflectionBase *pBase, const com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > &xRow );

class Columns : public Container
{
    OUString m_schemaName;
    OUString m_tableName;

public: // instances Columns 'exception safe'
    static com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > create(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName,
        Columns **pColumns);

protected:
    Columns(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings,
        const OUString &schemaName,
        const OUString &tableName);


    virtual ~Columns();

public: // XAppend
    virtual void SAL_CALL appendByDescriptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::container::ElementExistException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

// public: // XDrop
//     virtual void SAL_CALL dropByName( const OUString& elementName )
//         throw (::com::sun::star::sdbc::SQLException,
//                ::com::sun::star::container::NoSuchElementException,
//                ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dropByIndex( sal_Int32 index )
        throw (::com::sun::star::sdbc::SQLException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XRefreshable
    virtual void SAL_CALL refresh(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};


class ColumnDescriptors : public Container
{
public:
    ColumnDescriptors(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection >  & origin,
        ConnectionSettings *pSettings );

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
