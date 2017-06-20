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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XVIEW_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XVIEW_HXX

#include <cppuhelper/component.hxx>
#include <cppuhelper/propshlp.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "pq_xbase.hxx"

namespace pq_sdbc_driver
{
class View : public ReflectionBase,
             public css::sdbcx::XRename
{
public:
    View( const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
          const css::uno::Reference< css::sdbc::XConnection > & connection,
          ConnectionSettings *pSettings);

    // XInterface
    virtual void SAL_CALL acquire() throw() override { ReflectionBase::acquire(); }
    virtual void SAL_CALL release() throw() override { ReflectionBase::release(); }
    virtual css::uno::Any  SAL_CALL queryInterface(
        const css::uno::Type & reqType ) override;

    // XTypeProvider, first implemented by OPropertySetHelper
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId() override;

    // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) override;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

};


class ViewDescriptor : public ReflectionBase
{
public:
    ViewDescriptor( const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
          const css::uno::Reference< css::sdbc::XConnection > & connection,
          ConnectionSettings *pSettings);

    // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) override;
};

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
