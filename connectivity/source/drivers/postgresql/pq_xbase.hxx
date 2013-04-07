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
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#ifndef _PQ_REFL_BASE_HXX_
#define _PQ_REFL_BASE_HXX_
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "pq_xcontainer.hxx"

namespace pq_sdbc_driver
{

class ReflectionBase :
        public cppu::OComponentHelper,
        public cppu::OPropertySetHelper,
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::sdbcx::XDataDescriptorFactory,
        public com::sun::star::container::XNamed
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
        const ::rtl::Reference< RefCountedMutex > refMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > &conn,
        ConnectionSettings *pSettings,
        cppu::IPropertyArrayHelper & props /* must survive this object !*/ );

public:
    void copyValuesFrom( const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > &set );

public: // for initialization purposes only, not exported via an interface !
    void setPropertyValue_NoBroadcast_public(
        const OUString & name, const com::sun::star::uno::Any & value );

public: //XInterface
    virtual void SAL_CALL acquire() throw() { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() { OComponentHelper::release(); }
    virtual com::sun::star::uno::Any  SAL_CALL queryInterface(
        const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException);

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const;

    // XPropertySet
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException);

public: // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException);

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException );

public: // XDataDescriptorFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
    createDataDescriptor(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

public: // XNamed
    virtual OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

};

}
#endif
