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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XUSER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XUSER_HXX

#include <cppuhelper/component.hxx>
#include <cppuhelper/propshlp.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XUser.hpp>

#include "pq_xbase.hxx"

namespace pq_sdbc_driver
{

class User : public ReflectionBase,
             public com::sun::star::sdbcx::XUser
{

public:
    User( const ::rtl::Reference< RefCountedMutex > & refMutex,
          const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > & connection,
          ConnectionSettings *pSettings);

    // XInterface
    virtual void SAL_CALL acquire() throw() override { ReflectionBase::acquire(); }
    virtual void SAL_CALL release() throw() override { ReflectionBase::release(); }
    virtual com::sun::star::uno::Any  SAL_CALL queryInterface(
        const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XTypeProvider, first implemented by OPropertySetHelper
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XUser : XAuthorizable
    virtual sal_Int32 SAL_CALL getPrivileges( const OUString& objName, sal_Int32 objType ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getGrantablePrivileges( const OUString& objName, sal_Int32 objType ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL changePassword( const OUString& oldPassword, const OUString& newPassword ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};

class UserDescriptor : public ReflectionBase
{
public:
    UserDescriptor(
        const ::rtl::Reference< RefCountedMutex > & refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > & connection,
        ConnectionSettings *pSettings);

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
