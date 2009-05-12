/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysethelper.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "propertysethelper.hxx"
#include <com/sun/star/lang/XTypeProvider.hpp>

#include <cppuhelper/typeprovider.hxx>

//..........................................................................
namespace extensions {
    namespace apihelper {
//..........................................................................
        namespace uno   = com::sun::star::uno;
        namespace lang  = com::sun::star::lang;
        namespace beans = com::sun::star::beans;
//..........................................................................
PropertySetHelper::PropertySetHelper()
: BroadcasterBase()
, cppu::OWeakObject()
, cppu::OPropertySetHelper( BroadcasterBase::getBroadcastHelper() )
, m_pHelper(0)
{
}

//..........................................................................
PropertySetHelper::~PropertySetHelper()
{
    delete m_pHelper;
}

//..........................................................................
// XInterface
uno::Any SAL_CALL PropertySetHelper::queryInterface( uno::Type const & rType ) throw (uno::RuntimeException)
{
    uno::Any aResult = cppu::OPropertySetHelper::queryInterface(rType);
    if (!aResult.hasValue())
        aResult = OWeakObject::queryInterface(rType);
    return aResult;
}

void SAL_CALL PropertySetHelper::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL PropertySetHelper::release() throw ()
{
    if (m_refCount == 1)
        this->disposing();

    OWeakObject::release();
}

//..........................................................................
// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL PropertySetHelper::getTypes() throw (uno::RuntimeException)
{
    // could be static instance
    cppu::OTypeCollection aTypes(
        ::getCppuType( static_cast< uno::Reference< beans::XPropertySet > const * >(0) ),
        ::getCppuType( static_cast< uno::Reference< beans::XMultiPropertySet > const * >(0) ),
        ::getCppuType( static_cast< uno::Reference< beans::XFastPropertySet > const * >(0) ),
        ::getCppuType( static_cast< uno::Reference< lang::XTypeProvider > const * >(0) ) );

    return aTypes.getTypes();
}

//..........................................................................
// cppu::OPropertySetHelper
uno::Reference< beans::XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo(  )
    throw (uno::RuntimeException)
{
    return createPropertySetInfo(getInfoHelper());
}

//..........................................................................
cppu::IPropertyArrayHelper & SAL_CALL PropertySetHelper::getInfoHelper()
{
    osl::MutexGuard aGuard( getBroadcastMutex() );
    if (!m_pHelper)
        m_pHelper = newInfoHelper();

    OSL_ENSURE(m_pHelper,"Derived class did not create new PropertyInfoHelper");
    if (!m_pHelper)
        throw uno::RuntimeException(rtl::OUString::createFromAscii("No PropertyArrayHelper available"),*this);

    return *m_pHelper;
}

//..........................................................................
sal_Bool SAL_CALL PropertySetHelper::convertFastPropertyValue(
    uno::Any & rConvertedValue, uno::Any & rOldValue, sal_Int32 nHandle, const uno::Any& rValue )
        throw (lang::IllegalArgumentException)
{
    this->getFastPropertyValue(rOldValue, nHandle);
    rConvertedValue = rValue;
    return rValue.isExtractableTo( rOldValue.getValueType() );
}

//..........................................................................
void SAL_CALL ReadOnlyPropertySetHelper::setFastPropertyValue_NoBroadcast(
    sal_Int32 /*nHandle*/, const uno::Any& /*rValue*/ )
        throw (uno::Exception)
{
    OSL_ENSURE(false, "Attempt to set value in read-only property set");
    throw beans::PropertyVetoException(rtl::OUString::createFromAscii("Attempt to set value in Read-Only property set"),*this);
}

//..........................................................................
sal_Bool SAL_CALL ReadOnlyPropertySetHelper::convertFastPropertyValue(
    uno::Any & /*rConvertedValue*/, uno::Any & /*rOldValue*/, sal_Int32 /*nHandle*/, const uno::Any& /*rValue*/ )
        throw (lang::IllegalArgumentException)
{
    OSL_ENSURE(false, "Attempt to convert value in read-only property set");
    return false;
}
//..........................................................................
//..........................................................................
    }   // namespace apihelper
}   // namespace extensions
//..........................................................................


