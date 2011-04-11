/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef CHART_OPROPERTYSET_HXX
#define CHART_OPROPERTYSET_HXX

// helper classes
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>

// interfaces and types
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/style/XStyleSupplier.hpp>
#include <osl/mutex.hxx>
#include "charttoolsdllapi.hxx"

#include <memory>

namespace property
{

namespace impl
{ class ImplOPropertySet; }


class OOO_DLLPUBLIC_CHARTTOOLS OPropertySet :
    public ::cppu::OBroadcastHelper,
    // includes beans::XPropertySet, XMultiPropertySet and XFastPropertySet
    public ::cppu::OPropertySetHelper,
    // includes uno::XWeak (and XInterface, esp. ref-counting)

    public ::com::sun::star::lang::XTypeProvider,
    public ::com::sun::star::beans::XPropertyState,
    public ::com::sun::star::beans::XMultiPropertyStates,
    public ::com::sun::star::style::XStyleSupplier
{
public:
    OPropertySet( ::osl::Mutex & rMutex );
    virtual ~OPropertySet();

protected:
    explicit OPropertySet( const OPropertySet & rOther, ::osl::Mutex & rMutex );

    void SetNewValuesExplicitlyEvenIfTheyEqualDefault();

    /** implement this method to provide default values for all properties
        supporting defaults.  If a property does not have a default value, you
        may throw an UnknownPropertyException.
     */
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) = 0;

    /** The InfoHelper table contains all property names and types of
        this object.

        @return the object that provides information for the
                PropertySetInfo

        @see ::cppu::OPropertySetHelper
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() = 0;


    /** Try to convert the value <code>rValue</code> to the type required by the
        property associated with <code>nHandle</code>.

        Overload this method to take influence in modification of properties.

        If the conversion changed , </sal_True> is returned and the converted value
        is in <code>rConvertedValue</code>.  The former value is contained in
        <code>rOldValue</code>.

        After this call returns successfully, the vetoable listeners are
        notified.

        @throws IllegalArgumentException, if the conversion was not successful,
                or if there is no corresponding property to the given handle.

        @param rConvertedValue the converted value. Only set if return is true.
        @param rOldValue the old value. Only set if return is true.
        @param nHandle the handle of the property.

        @return true, if the conversion was successful and converted value
                differs from the old value.

        @see ::cppu::OPropertySetHelper
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue
        ( ::com::sun::star::uno::Any & rConvertedValue,
          ::com::sun::star::uno::Any & rOldValue,
          sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    /** The same as setFastProperyValue; nHandle is always valid.
        The changes must not be broadcasted in this method.

        @attention
        Although you are permitted to throw any UNO exception, only the following
        are valid for usage:
        -- ::com::sun::star::beans::UnknownPropertyException
        -- ::com::sun::star::beans::PropertyVetoException
        -- ::com::sun::star::lang::IllegalArgumentException
        -- ::com::sun::star::lang::WrappedTargetException
        -- ::com::sun::star::uno::RuntimeException

        @param nHandle handle
        @param rValue  value

        @see ::cppu::OPropertySetHelper
    */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    /**
       The same as getFastProperyValue, but return the value through rValue and
       nHandle is always valid.

        @see ::cppu::OPropertySetHelper
     */
    virtual void SAL_CALL getFastPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    /** implement this method in derived classes to get called when properties
        change.
     */
    virtual void firePropertyChangeEvent();

    /// call this when a derived component is disposed
    virtual void disposePropertySet();

    // ========================================
    // Interfaces
    // ========================================

    // ____ XInterface ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTypeProvider ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XPropertyState ____
    virtual ::com::sun::star::beans::PropertyState SAL_CALL
        getPropertyState( const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
        getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setPropertyToDefault( const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL
        getPropertyDefault( const ::rtl::OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XMultiPropertyStates ____
    // Note: getPropertyStates() is already implemented in XPropertyState with the
    // same signature
    virtual void SAL_CALL
        setAllPropertiesToDefault()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setPropertiesToDefault( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
        getPropertyDefaults( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XStyleSupplier ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > SAL_CALL getStyle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >& xStyle )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XMultiPropertySet ____
    virtual void SAL_CALL setPropertyValues(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values )
        throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Note: it is assumed that the base class implements setPropertyValue by
    // using setFastPropertyValue

private:
    /// reference to mutex of class deriving from here
    ::osl::Mutex &   m_rMutex;

    /// pImpl idiom implementation
    ::std::auto_ptr< impl::ImplOPropertySet > m_pImplProperties;
    bool m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault;
};

} //  namespace property

// CHART_OPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
