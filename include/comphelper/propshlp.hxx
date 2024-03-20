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
#include <comphelper/multiinterfacecontainer4.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <comphelper/comphelperdllapi.h>
#include <comphelper/unoimplbase.hxx>
#include <cppuhelper/propshlp.hxx>

namespace comphelper
{
/*************************************************************************
*************************************************************************/

/**
   This abstract class maps the methods of the interfaces XMultiPropertySet, XFastPropertySet
   and XPropertySet to the methods getInfoHelper, convertFastPropertyValue,
   setFastPropertyValue_NoBroadcast and getFastPropertyValue. You must derive from
   this class and override the methods.
   It provides a standard implementation of the XPropertySetInfo.

   This is a modified copy of the cppuhelper::OPropertySetHelper class, except
   that is uses std::mutex instead of osl::Mutex.
 */
class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OPropertySetHelper
    : public virtual comphelper::UnoImplBase,
      public css::beans::XMultiPropertySet,
      public css::beans::XFastPropertySet,
      public css::beans::XPropertySet
{
public:
    OPropertySetHelper();

    /** Constructor.

        @param bIgnoreRuntimeExceptionsWhileFiring
                        indicates whether occurring RuntimeExceptions will be
                        ignored when firing notifications
                        (vetoableChange(), propertyChange())
                        to listeners.
                        PropertyVetoExceptions may still be thrown.
                        This flag is useful in an inter-process scenario when
                        remote bridges may break down
                        (firing DisposedExceptions).
    */
    OPropertySetHelper(bool bIgnoreRuntimeExceptionsWhileFiring);

    /**
       Only returns a reference to XMultiPropertySet, XFastPropertySet, XPropertySet and
       XEventListener.
     */
    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType) override;

    /** eases implementing XTypeProvider::getTypes, returns the types of XMultiPropertySet, XFastPropertySet, XPropertySet

        @throws css::uno::RuntimeException
     */
    static css::uno::Sequence<css::uno::Type> getTypes();

    /**
       Send a disposing notification to the listeners

       @see OComponentHelper
     */
    void disposing(std::unique_lock<std::mutex>& rGuard);

    /**
       Throw UnknownPropertyException or PropertyVetoException if the property with the name
       rPropertyName does not exist or is readonly. Otherwise rPropertyName is changed to its handle
       value and setFastPropertyValue is called.
     */
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& rPropertyName,
                                           const css::uno::Any& aValue) override final;
    /**
       Throw UnknownPropertyException if the property with the name
       rPropertyName does not exist.
     */
    virtual css::uno::Any SAL_CALL
    getPropertyValue(const ::rtl::OUString& aPropertyName) override final;

    /** Ignored if the property is not bound. */
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override final;

    /** Ignored if the property is not bound. */
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override final;

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override final;

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override final;

    /**
       Throw UnknownPropertyException or PropertyVetoException if the property with the name
       rPropertyName does not exist or is readonly. Otherwise the method convertFastPropertyValue
       is called, then the vetoable listeners are notified. After this the value of the property
       is changed with the setFastPropertyValue_NoBroadcast method and the bound listeners are
       notified.
      */
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle,
                                               const css::uno::Any& rValue) override final;

    /**
       @exception css::beans::UnknownPropertyException
         if the property with the handle nHandle does not exist.
     */
    virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override final;

    // XMultiPropertySet
    virtual void SAL_CALL
    setPropertyValues(const css::uno::Sequence<::rtl::OUString>& PropertyNames,
                      const css::uno::Sequence<css::uno::Any>& Values) override;

    virtual css::uno::Sequence<css::uno::Any> SAL_CALL
    getPropertyValues(const css::uno::Sequence<::rtl::OUString>& PropertyNames) override final;

    virtual void SAL_CALL addPropertiesChangeListener(
        const css::uno::Sequence<::rtl::OUString>& PropertyNames,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override final;

    virtual void SAL_CALL removePropertiesChangeListener(
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override final;

    virtual void SAL_CALL firePropertiesChangeEvent(
        const css::uno::Sequence<::rtl::OUString>& PropertyNames,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override final;

    /**
       The property sequence is created in the call. The interface isn't used after the call.
     */
    static css::uno::Reference<css::beans::XPropertySetInfo>
    createPropertySetInfo(cppu::IPropertyArrayHelper& rProperties);

protected:
    /**
       You must call disposing() before destruction.
     */
    ~OPropertySetHelper();

    /** Override this if you need to do something special during setFastPropertyValue */
    virtual void setFastPropertyValueImpl(std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle,
                                          const css::uno::Any& rValue);
    /** Override this if you need to do something special during getPropertyValue */
    virtual css::uno::Any getPropertyValueImpl(std::unique_lock<std::mutex>& rGuard,
                                               const ::rtl::OUString& aPropertyName) final;

    /**
       This method fire events to all registered property listeners.
       @param pnHandles     the id's of the properties that changed.
       @param pNewValues    the new values of the properties.
       @param pOldValues    the old values of the properties.
       @param nCount        the number of elements in the arrays pnHandles, pNewValues and pOldValues.
       @param bVetoable true means fire to VetoableChangeListener, false means fire to
                  XPropertyChangedListener and XMultiPropertyChangedListener.
     */
    void fire(std::unique_lock<std::mutex>& rGuard, sal_Int32* pnHandles,
              const css::uno::Any* pNewValues, const css::uno::Any* pOldValues, sal_Int32 nCount,
              bool bVetoable);

    /**
       Set multiple properties with the handles.
       @param nSeqLen   the length of the arrays pHandles and Values.
       @param pHandles the handles of the properties. The number of elements
              in the Values sequence is the length of the handle array. A value of -1
              of a handle means invalid property. These are ignored.
       @param pValues the values of the properties.
       @param nHitCount the number of valid entries in the handle array.
     */
    void setFastPropertyValues(std::unique_lock<std::mutex>& rGuard, sal_Int32 nSeqLen,
                               sal_Int32* pHandles, const css::uno::Any* pValues,
                               sal_Int32 nHitCount);

    /**
       This abstract method must return the name to index table. This table contains all property
       names and types of this object. The method is not implemented in this class.
     */
    virtual cppu::IPropertyArrayHelper& getInfoHelper() = 0;

    /**
       Converted the value rValue and return the result in rConvertedValue and the
       old value in rOldValue. An IllegalArgumentException is thrown.
       The method is not implemented in this class. After this call the vetoable
       listeners are notified.

       @param rConvertedValue the converted value. Only set if return is true.
       @param rOldValue the old value. Only set if return is true.
       @param nHandle the handle of the property.
       @param rValue the value to be converted
       @return true if the value converted.
       @throws css::lang::IllegalArgumentException
       @throws css::beans::UnknownPropertyException
       @throws css::uno::RuntimeException
     */
    virtual bool convertFastPropertyValue(std::unique_lock<std::mutex>& rGuard,
                                          css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue)
        = 0;

    /** The same as setFastPropertyValue; nHandle is always valid.
        The changes must not be broadcasted in this method.
        The method is implemented in a derived class.

        @attention
        Although you are permitted to throw any UNO exception, only the following
        are valid for usage:
        -- css::beans::UnknownPropertyException
        -- css::beans::PropertyVetoException
        -- css::lang::IllegalArgumentException
        -- css::lang::WrappedTargetException
        -- css::uno::RuntimeException

        @param nHandle
               handle
        @param rValue
               value
        @throws css::uno::Exception
    */
    virtual void setFastPropertyValue_NoBroadcast(std::unique_lock<std::mutex>& rGuard,
                                                  sal_Int32 nHandle, const css::uno::Any& rValue)
        = 0;
    /**
       The same as getFastPropertyValue, but return the value through rValue and nHandle
       is always valid.
       The method is not implemented in this class.
     */
    virtual void getFastPropertyValue(std::unique_lock<std::mutex>& rGuard, css::uno::Any& rValue,
                                      sal_Int32 nHandle) const = 0;

    /** sets an dependent property's value

        <p>Sometimes setting a given property needs to implicitly modify another property's value. Calling |setPropertyValue|
        from within |setFastPropertyValue_NoBroadcast| is not an option here, as it would notify the property listeners
        while our mutex is still locked. Setting the dependent property's value directly (e.g. by calling |setFastPropertyValue_NoBroadcast|
        recursively) is not an option, too, since it would miss firing the property change event.</p>

        <p>So, in such cases, you use |setDependentFastPropertyValue| from within |setFastPropertyValue_NoBroadcast|.
        It will convert and actually set the property value (invoking |convertFastPropertyValue| and |setFastPropertyValue_NoBroadcast|
        for the given handle and value), and add the property change event to the list of events to be notified
        when the bottom-most |setFastPropertyValue_NoBroadcast| on the stack returns.</p>

        <p><strong>Note</strong>: The method will <em>not</em> invoke veto listeners for the property.</p>

        <p><strong>Note</strong>: It's the caller's responsibility to ensure that our mutex is locked. This is
        canonically given when the method is invoked from within |setFastPropertyValue_NoBroadcast|, in other
        contexts, you might need to take own measures.</p>
    */
    void setDependentFastPropertyValue(std::unique_lock<std::mutex>& rGuard, sal_Int32 i_handle,
                                       const css::uno::Any& i_value);

private:
    /**
       Container for the XPropertyChangedListener. The listeners are inserted by handle.
     */
    OMultiTypeInterfaceContainerHelperVar4<sal_Int32, css::beans::XPropertyChangeListener> aBoundLC;
    /**
       Container for the XPropertyVetoableListener. The listeners are inserted by handle.
     */
    OMultiTypeInterfaceContainerHelperVar4<sal_Int32, css::beans::XVetoableChangeListener>
        aVetoableLC;
    /**
       Container for the XPropertyChangedListener where the listeners want to listen to all properties.
     */
    comphelper::OInterfaceContainerHelper4<css::beans::XPropertyChangeListener>
        maPropertyChangeListeners;
    comphelper::OInterfaceContainerHelper4<css::beans::XPropertiesChangeListener>
        maPropertiesChangeListeners;
    /**
       Container for the XVetoableChangeListener where the listeners want to listen to all properties.
     */
    comphelper::OInterfaceContainerHelper4<css::beans::XVetoableChangeListener>
        maVetoableChangeListeners;
    std::vector<sal_Int32> m_handles;
    std::vector<css::uno::Any> m_newValues;
    std::vector<css::uno::Any> m_oldValues;
    bool m_bIgnoreRuntimeExceptionsWhileFiring = false;

    /** notifies the given changes in property's values, <em>plus</em> all property changes collected during recent
        |setDependentFastPropertyValue| calls.
    */
    void impl_fireAll(std::unique_lock<std::mutex>& rGuard, sal_Int32* i_handles,
                      const css::uno::Any* i_newValues, const css::uno::Any* i_oldValues,
                      sal_Int32 i_count);

    void fireVetoableChangeListeners(
        std::unique_lock<std::mutex>& rGuard,
        comphelper::OInterfaceContainerHelper4<css::beans::XVetoableChangeListener>* pListeners,
        const css::beans::PropertyChangeEvent& rChangeEvent);
    void firePropertyChangeListeners(
        std::unique_lock<std::mutex>& rGuard,
        comphelper::OInterfaceContainerHelper4<css::beans::XPropertyChangeListener>* pListeners,
        const css::beans::PropertyChangeEvent& rChangeEvent);
};

} // end namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
