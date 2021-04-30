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
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "pq_xcontainer.hxx"

namespace pq_sdbc_driver
{

typedef ::cppu::WeakComponentImplHelper< css::lang::XServiceInfo,
                                         css::sdbcx::XDataDescriptorFactory,
                                         css::container::XNamed
                                          > ReflectionBase_BASE;

class ReflectionBase :
        public ReflectionBase_BASE,
        public cppu::OPropertySetHelper
{
protected:
    const OUString m_implName;
    const css::uno::Sequence< OUString > m_supportedServices;
    ::rtl::Reference< comphelper::RefCountedMutex > m_xMutex;
    css::uno::Reference< css::sdbc::XConnection > m_conn;
    ConnectionSettings *m_pSettings;
    cppu::IPropertyArrayHelper & m_propsDesc;
    std::vector< css::uno::Any > m_values;
public:
    ReflectionBase(
        const OUString &implName,
        const css::uno::Sequence< OUString > &supportedServices,
        const ::rtl::Reference< comphelper::RefCountedMutex >& refMutex,
        const css::uno::Reference< css::sdbc::XConnection > &conn,
        ConnectionSettings *pSettings,
        cppu::IPropertyArrayHelper & props /* must survive this object !*/ );

public:
    void copyValuesFrom( const css::uno::Reference< css::beans::XPropertySet > &set );

public: // for initialization purposes only, not exported via an interface !
    void setPropertyValue_NoBroadcast_public(
        const OUString & name, const css::uno::Any & value );

public: //XInterface
    virtual void SAL_CALL acquire() noexcept override { ReflectionBase_BASE::acquire(); }
    virtual void SAL_CALL release() noexcept override { ReflectionBase_BASE::release(); }
    virtual css::uno::Any  SAL_CALL queryInterface(
        const css::uno::Type & reqType ) override;

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any & rConvertedValue,
        css::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const css::uno::Any& rValue ) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const css::uno::Any& rValue ) override;

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    // XPropertySet
    css::uno::Reference < css::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() override;

public: // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId() override;

public: // XDataDescriptorFactory
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) override = 0;

public: // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
