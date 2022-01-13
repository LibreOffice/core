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

#include <cppuhelper/propshlp.hxx>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <mutex>
#include <vector>

struct BaseStdMutex
{
    std::mutex m_aMutex;
};

namespace comphelper
{
/**
   This abstract class maps the methods of the interfaces XMultiPropertySet, XFastPropertySet
   and XPropertySet to the methods getInfoHelper, convertFastPropertyValue,
   setFastPropertyValue_NoBroadcast and getFastPropertyValue. You must derive from
   this class and override the methods.
   It provides a standard implementation of the XPropertySetInfo.
   The XPropertiesChangeListener are inserted in the rBHelper.aLC structure.
   The XPropertyChangeListener and XVetoableChangeListener with no names are inserted
   in the rBHelper.aLC structure. So it is possible to advise property listeners with
   the connection point interfaces. But only listeners that listen to all property changes.

 */
class COMPHELPER_DLLPUBLIC OPropertySetHelper : public css::beans::XMultiPropertySet,
                                                public css::beans::XFastPropertySet,
                                                public css::beans::XPropertySet
{
public:
    OPropertySetHelper(std::mutex&);

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
       Send a disposing notification to the listeners in the containers aBoundLC
       and aVetoableLC.

       @see OComponentHelper
     */
    void disposing();

    /**
       Throw UnknownPropertyException or PropertyVetoException if the property with the name
       rPropertyName does not exist or is readonly. Otherwise rPropertyName is changed to its handle
       value and setFastPropertyValue is called.
     */
    virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& rPropertyName,
                                           const css::uno::Any& aValue) override;
    /**
       Throw UnknownPropertyException if the property with the name
       rPropertyName does not exist.
     */
    virtual css::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& aPropertyName) override;
    /** Ignored if the property is not bound. */
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override;

    /** Ignored if the property is not bound. */
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override;

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

    /** Ignored if the property is not constrained. */
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

    /**
       Throw UnknownPropertyException or PropertyVetoException if the property with the name
       rPropertyName does not exist or is readonly. Otherwise the method convertFastPropertyValue
       is called, then the vetoable listeners are notified. After this the value of the property
       is changed with the setFastPropertyValue_NoBroadcast method and the bound listeners are
       notified.
      */
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle,
                                               const css::uno::Any& rValue) override;

    /**
       @exception css::beans::UnknownPropertyException
         if the property with the handle nHandle does not exist.
     */
    virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override;

    // XMultiPropertySet
    virtual void SAL_CALL
    setPropertyValues(const css::uno::Sequence<::rtl::OUString>& PropertyNames,
                      const css::uno::Sequence<css::uno::Any>& Values) override;

    virtual css::uno::Sequence<css::uno::Any> SAL_CALL
    getPropertyValues(const css::uno::Sequence<::rtl::OUString>& PropertyNames) override;

    virtual void SAL_CALL addPropertiesChangeListener(
        const css::uno::Sequence<::rtl::OUString>& PropertyNames,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override;

    virtual void SAL_CALL removePropertiesChangeListener(
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override;

    virtual void SAL_CALL firePropertiesChangeEvent(
        const css::uno::Sequence<::rtl::OUString>& PropertyNames,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override;

    /**
       The property sequence is created in the call. The interface isn't used after the call.
     */
    static css::uno::Reference<css::beans::XPropertySetInfo>
    createPropertySetInfo(cppu::IPropertyArrayHelper& rProperties);

protected:
    /**
       This method fire events to all registered property listeners.
       @param pnHandles     the id's of the properties that changed.
       @param pNewValues    the new values of the properties.
       @param pOldValues    the old values of the properties.
       @param nCount        the number of elements in the arrays pnHandles, pNewValues and pOldValues.
       @param bVetoable true means fire to VetoableChangeListener, false means fire to
                  XPropertyChangedListener and XMultiPropertyChangedListener.
     */
    void fire(sal_Int32* pnHandles, const css::uno::Any* pNewValues,
              const css::uno::Any* pOldValues, sal_Int32 nCount, bool bVetoable);

    /**
       Set multiple properties with the handles.
       @param nSeqLen   the length of the arrays pHandles and Values.
       @param pHandles the handles of the properties. The number of elements
              in the Values sequence is the length of the handle array. A value of -1
              of a handle means invalid property. These are ignored.
       @param pValues the values of the properties.
       @param nHitCount the number of valid entries in the handle array.
     */
    void setFastPropertyValues(sal_Int32 nSeqLen, sal_Int32* pHandles, const css::uno::Any* pValues,
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
    virtual sal_Bool convertFastPropertyValue(css::uno::Any& rConvertedValue,
                                              css::uno::Any& rOldValue, sal_Int32 nHandle,
                                              const css::uno::Any& rValue)
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
    virtual void setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue)
        = 0;
    /**
       The same as getFastPropertyValue, but return the value through rValue and nHandle
       is always valid.
       The method is not implemented in this class.
     */
    virtual void getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const = 0;

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
    void setDependentFastPropertyValue(sal_Int32 i_handle, const css::uno::Any& i_value);

private:
    OPropertySetHelper(const OPropertySetHelper&) = delete;
    OPropertySetHelper& operator=(const OPropertySetHelper&) = delete;

    /** notifies the given changes in property's values, <em>plus</em> all property changes collected during recent
        |setDependentFastPropertyValue| calls.
    */
    void impl_fireAll(sal_Int32* i_handles, const css::uno::Any* i_newValues,
                      const css::uno::Any* i_oldValues, sal_Int32 i_count);

protected:
    /**
       You must call disposing before destruction.
     */
    virtual ~OPropertySetHelper();

private:
    std::mutex& m_rMutex;
    bool m_bFireEvents;

    /**
       Container for the XPropertyChangedListener. The listeners are inserted by handle.
     */
    comphelper::OMultiTypeInterfaceContainerHelperVar4<sal_Int32,
                                                       css::beans::XPropertyChangeListener>
        aBoundLC;
    /**
       Container for the XPropertyVetoableListener. The listeners are inserted by handle.
     */
    comphelper::OMultiTypeInterfaceContainerHelperVar4<sal_Int32,
                                                       css::beans::XVetoableChangeListener>
        aVetoableLC;
    comphelper::OInterfaceContainerHelper4<css::beans::XPropertiesChangeListener>
        maPropertiesChangeListeners;

    std::vector<sal_Int32> m_handles;
    std::vector<css::uno::Any> m_newValues;
    std::vector<css::uno::Any> m_oldValues;
};

} // end namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
