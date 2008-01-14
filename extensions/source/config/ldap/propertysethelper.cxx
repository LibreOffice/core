/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertysethelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:42:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "propertysethelper.hxx"

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

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


