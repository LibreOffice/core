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
#pragma once


#include <config_options.h>
// helper classes
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propshlp.hxx>

// interfaces and types
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/style/XStyleSupplier.hpp>

#include <unordered_map>

namespace property
{

class SAL_DLLPUBLIC_RTTI OPropertySet :
    protected cppu::BaseMutex,
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
    OPropertySet();
    virtual ~OPropertySet();

protected:
    explicit OPropertySet( const OPropertySet & rOther );

    void SetNewValuesExplicitlyEvenIfTheyEqualDefault();

    /** implement this method to provide default values for all properties
        supporting defaults.  If a property does not have a default value, you
        may throw an UnknownPropertyException.
        We pass the any by reference because this code is very hot and doing
        it this way is cheaper than the two step process of constructing a new
        any and then assigning to via a return value.

        @throws css::beans::UnknownPropertyException
        @throws css::uno::RuntimeException
     */
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const = 0;

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
          const css::uno::Any& rValue ) override final;

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

    /** implement this method in derived classes to get called when properties
        change.
     */
    virtual void firePropertyChangeEvent();

public:
    /// make original interface function visible again
    using ::cppu::OPropertySetHelper::getFastPropertyValue;

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
        getPropertyState( const OUString& PropertyName ) override final;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
        getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override final;
    virtual void SAL_CALL
        setPropertyToDefault( const OUString& PropertyName ) override final;
    virtual css::uno::Any SAL_CALL
        getPropertyDefault( const OUString& aPropertyName ) override final;

    // ____ XMultiPropertyStates ____
    // Note: getPropertyStates() is already implemented in XPropertyState with the
    // same signature
    virtual void SAL_CALL
        setAllPropertiesToDefault() override final;
    virtual void SAL_CALL
        setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override final;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
        getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override final;

    // ____ XStyleSupplier ____
    virtual css::uno::Reference< css::style::XStyle > SAL_CALL getStyle() override final;
    virtual void SAL_CALL setStyle( const css::uno::Reference< css::style::XStyle >& xStyle ) override final;

    // ____ XMultiPropertySet ____
    virtual void SAL_CALL setPropertyValues(
        const css::uno::Sequence< OUString >& PropertyNames,
        const css::uno::Sequence< css::uno::Any >& Values ) override final;

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue ) override final;

    // Note: it is assumed that the base class implements setPropertyValue by
    // using setFastPropertyValue

private:
    /** supports states DIRECT_VALUE and DEFAULT_VALUE
     */
    css::beans::PropertyState
        GetPropertyStateByHandle( sal_Int32 nHandle ) const;

    css::uno::Sequence< css::beans::PropertyState >
        GetPropertyStatesByHandle( const std::vector< sal_Int32 > & aHandles ) const;

    void SetPropertyToDefault( sal_Int32 nHandle );
    void SetPropertiesToDefault( const std::vector< sal_Int32 > & aHandles );
    void SetAllPropertiesToDefault();

    /** @param rValue is set to the value for the property given in nHandle.  If
               the property is not set, the style chain is searched for any
               instance set there.  If there was no value found either in the
               property set itself or any of its styles, rValue remains
               unchanged and false is returned.

        @return false if the property is default, true otherwise.
     */
    bool GetPropertyValueByHandle(
        css::uno::Any & rValue,
        sal_Int32 nHandle ) const;

    void SetPropertyValueByHandle( sal_Int32 nHandle,
                                   const css::uno::Any & rValue );

    bool SetStyle( const css::uno::Reference< css::style::XStyle > & xStyle );

    bool m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault;
    std::unordered_map< sal_Int32, css::uno::Any >    m_aProperties;
    css::uno::Reference< css::style::XStyle > m_xStyle;
};

} //  namespace property

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
