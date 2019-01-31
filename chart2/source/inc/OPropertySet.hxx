/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CHART2_SOURCE_INC_OPROPERTYSET_HXX
#define INCLUDED_CHART2_SOURCE_INC_OPROPERTYSET_HXX

// helper classes
#include <cppuhelper/propshlp.hxx>

// interfaces and types
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/style/XStyleSupplier.hpp>
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

    public css::lang::XTypeProvider,
    public css::beans::XPropertyState,
    public css::beans::XMultiPropertyStates,
    public css::style::XStyleSupplier
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

        @throws css::beans::UnknownPropertyException
        @throws css::uno::RuntimeException
     */
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const = 0;

    /** The InfoHelper table contains all property names and types of
        this object.

        @return the object that provides information for the
                PropertySetInfo

        @see ::cppu::OPropertySetHelper
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override = 0;

    /** Try to convert the value <code>rValue</code> to the type required by the
        property associated with <code>nHandle</code>.

        Override this method to take influence in modification of properties.

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
        ( css::uno::Any & rConvertedValue,
          css::uno::Any & rOldValue,
          sal_Int32 nHandle,
          const css::uno::Any& rValue ) override;

    /** The same as setFastPropertyValue; nHandle is always valid.
        The changes must not be broadcasted in this method.

        @attention
        Although you are permitted to throw any UNO exception, only the following
        are valid for usage:
        -- css::beans::UnknownPropertyException
        -- css::beans::PropertyVetoException
        -- css::lang::IllegalArgumentException
        -- css::lang::WrappedTargetException
        -- css::uno::RuntimeException

        @param nHandle handle
        @param rValue  value

        @see ::cppu::OPropertySetHelper
    */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const css::uno::Any& rValue ) override;

    /**
       The same as getFastPropertyValue, but return the value through rValue and
       nHandle is always valid.

        @see ::cppu::OPropertySetHelper
     */
    virtual void SAL_CALL getFastPropertyValue
        ( css::uno::Any& rValue,
          sal_Int32 nHandle ) const override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    /** implement this method in derived classes to get called when properties
        change.
     */
    virtual void firePropertyChangeEvent();

    // Interfaces

    // ____ XInterface ____
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    // ____ XTypeProvider ____
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId() override;

    // ____ XPropertyState ____
    virtual css::beans::PropertyState SAL_CALL
        getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
        getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL
        setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL
        getPropertyDefault( const OUString& aPropertyName ) override;

    // ____ XMultiPropertyStates ____
    // Note: getPropertyStates() is already implemented in XPropertyState with the
    // same signature
    virtual void SAL_CALL
        setAllPropertiesToDefault() override;
    virtual void SAL_CALL
        setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
        getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    // ____ XStyleSupplier ____
    virtual css::uno::Reference< css::style::XStyle > SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( const css::uno::Reference< css::style::XStyle >& xStyle ) override;

    // ____ XMultiPropertySet ____
    virtual void SAL_CALL setPropertyValues(
        const css::uno::Sequence< OUString >& PropertyNames,
        const css::uno::Sequence< css::uno::Any >& Values ) override;

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue ) override;

    // Note: it is assumed that the base class implements setPropertyValue by
    // using setFastPropertyValue

private:
    /// reference to mutex of class deriving from here
    ::osl::Mutex &   m_rMutex;

    /// pImpl idiom implementation
    std::unique_ptr< impl::ImplOPropertySet > m_pImplProperties;
    bool m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault;
};

} //  namespace property

// INCLUDED_CHART2_SOURCE_INC_OPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
