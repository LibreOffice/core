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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XBASE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_XBASE_HXX
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "pq_xcontainer.hxx"

namespace pq_sdbc_driver
{

typedef ::cppu::WeakComponentImplHelper< ::com::sun::star::lang::XServiceInfo,
                                          ::com::sun::star::sdbcx::XDataDescriptorFactory,
                                          ::com::sun::star::container::XNamed
                                          > ReflectionBase_BASE;

class ReflectionBase :
        public ReflectionBase_BASE,
        public cppu::OPropertySetHelper
{
protected:
    const OUString m_implName;
    const ::com::sun::star::uno::Sequence< OUString > m_supportedServices;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > m_conn;
    ConnectionSettings *m_pSettings;
    cppu::IPropertyArrayHelper & m_propsDesc;
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > m_values;
public:
    ReflectionBase(
        const OUString &implName,
        const ::com::sun::star::uno::Sequence< OUString > &supportedServices,
        const ::rtl::Reference< RefCountedMutex >& refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &conn,
        ConnectionSettings *pSettings,
        cppu::IPropertyArrayHelper & props /* must survive this object !*/ );

public:
    void copyValuesFrom( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > &set );

public: // for initialization purposes only, not exported via an interface !
    void setPropertyValue_NoBroadcast_public(
        const OUString & name, const com::sun::star::uno::Any & value );

public: //XInterface
    virtual void SAL_CALL acquire() throw() override { ReflectionBase_BASE::acquire(); }
    virtual void SAL_CALL release() throw() override { ReflectionBase_BASE::release(); }
    virtual com::sun::star::uno::Any  SAL_CALL queryInterface(
        const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception, std::exception) override;

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    // XPropertySet
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

public: // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override = 0;

public: // XNamed
    virtual OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
