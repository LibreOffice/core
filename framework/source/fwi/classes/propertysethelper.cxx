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

#include <sal/config.h>

#include <vcl/svapp.hxx>

#include <classes/propertysethelper.hxx>
#include <threadhelp/transactionguard.hxx>

namespace framework{

PropertySetHelper::PropertySetHelper(      osl::Mutex & mutex,
                                           TransactionManager*                                     pExternalTransactionManager ,
                                           bool                                                bReleaseLockOnCall          )
    : m_lSimpleChangeListener(mutex)
    , m_lVetoChangeListener  (mutex)
    , m_bReleaseLockOnCall   (bReleaseLockOnCall                   )
    , m_rTransactionManager  (*pExternalTransactionManager         )
{
}

PropertySetHelper::~PropertySetHelper()
{
}

void PropertySetHelper::impl_setPropertyChangeBroadcaster(const css::uno::Reference< css::uno::XInterface >& xBroadcaster)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    SolarMutexGuard g;
    m_xBroadcaster = xBroadcaster;
}

void SAL_CALL PropertySetHelper::impl_addPropertyInfo(const css::beans::Property& aProperty)
    throw(css::beans::PropertyExistException,
          css::uno::Exception               )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    SolarMutexGuard g;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(aProperty.Name);
    if (pIt != m_lProps.end())
        throw css::beans::PropertyExistException();

    m_lProps[aProperty.Name] = aProperty;
}

void SAL_CALL PropertySetHelper::impl_disablePropertySet()
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    SolarMutexGuard g;

    css::uno::Reference< css::uno::XInterface > xThis(static_cast< css::beans::XPropertySet* >(this), css::uno::UNO_QUERY);
    css::lang::EventObject aEvent(xThis);

    m_lSimpleChangeListener.disposeAndClear(aEvent);
    m_lVetoChangeListener.disposeAndClear(aEvent);
    m_lProps.free();
}

bool PropertySetHelper::impl_existsVeto(const css::beans::PropertyChangeEvent& aEvent)
{
    /*  Don't use the lock here!
        The used helper is threadsafe and it lives for the whole lifetime of
        our own object.
    */
    ::cppu::OInterfaceContainerHelper* pVetoListener = m_lVetoChangeListener.getContainer(aEvent.PropertyName);
    if (! pVetoListener)
        return false;

    ::cppu::OInterfaceIteratorHelper pListener(*pVetoListener);
    while (pListener.hasMoreElements())
    {
        try
        {
            css::uno::Reference< css::beans::XVetoableChangeListener > xListener(
                static_cast<css::beans::XVetoableChangeListener*>(pListener.next()),
                css::uno::UNO_QUERY_THROW);
            xListener->vetoableChange(aEvent);
        }
        catch(const css::uno::RuntimeException&)
            { pListener.remove(); }
        catch(const css::beans::PropertyVetoException&)
            { return true; }
    }

    return false;
}

void PropertySetHelper::impl_notifyChangeListener(const css::beans::PropertyChangeEvent& aEvent)
{
    /*  Don't use the lock here!
        The used helper is threadsafe and it lives for the whole lifetime of
        our own object.
    */
    ::cppu::OInterfaceContainerHelper* pSimpleListener = m_lSimpleChangeListener.getContainer(aEvent.PropertyName);
    if (! pSimpleListener)
        return;

    ::cppu::OInterfaceIteratorHelper pListener(*pSimpleListener);
    while (pListener.hasMoreElements())
    {
        try
        {
            css::uno::Reference< css::beans::XPropertyChangeListener > xListener(
                static_cast<css::beans::XVetoableChangeListener*>(pListener.next()),
                css::uno::UNO_QUERY_THROW);
            xListener->propertyChange(aEvent);
        }
        catch(const css::uno::RuntimeException&)
            { pListener.remove(); }
    }
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo()
    throw(css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    css::uno::Reference< css::beans::XPropertySetInfo > xInfo(static_cast< css::beans::XPropertySetInfo* >(this), css::uno::UNO_QUERY_THROW);
    return xInfo;
}

void SAL_CALL PropertySetHelper::setPropertyValue(const OUString& sProperty,
                                                  const css::uno::Any&   aValue   )
    throw(css::beans::UnknownPropertyException,
          css::beans::PropertyVetoException   ,
          css::lang::IllegalArgumentException ,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException, std::exception          )
{
    // TODO look for e.g. readonly props and reject setProp() call!

    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    SolarMutexResettableGuard aWriteLock;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    css::beans::Property aPropInfo = pIt->second;

    bool bLocked = true;
    if (m_bReleaseLockOnCall)
    {
        aWriteLock.clear();
        bLocked = false;
        // <- SAFE
    }

    css::uno::Any aCurrentValue = impl_getPropertyValue(aPropInfo.Name, aPropInfo.Handle);

    if (! bLocked)
    {
        // SAFE ->
        aWriteLock.reset();
    }

    bool bWillBeChanged = (aCurrentValue != aValue);
    if (! bWillBeChanged)
        return;

    css::beans::PropertyChangeEvent aEvent;
    aEvent.PropertyName   = aPropInfo.Name;
    aEvent.Further        = sal_False;
    aEvent.PropertyHandle = aPropInfo.Handle;
    aEvent.OldValue       = aCurrentValue;
    aEvent.NewValue       = aValue;
    aEvent.Source.set(m_xBroadcaster.get(), css::uno::UNO_QUERY);

    if (m_bReleaseLockOnCall)
    {
        aWriteLock.clear();
        // <- SAFE
    }

    if (impl_existsVeto(aEvent))
        throw css::beans::PropertyVetoException();

    impl_setPropertyValue(aPropInfo.Name, aPropInfo.Handle, aValue);

    impl_notifyChangeListener(aEvent);
}

css::uno::Any SAL_CALL PropertySetHelper::getPropertyValue(const OUString& sProperty)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException, std::exception          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    SolarMutexClearableGuard aReadLock;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    css::beans::Property aPropInfo = pIt->second;

    if (m_bReleaseLockOnCall)
        aReadLock.clear();

    return impl_getPropertyValue(aPropInfo.Name, aPropInfo.Handle);
}

void SAL_CALL PropertySetHelper::addPropertyChangeListener(const OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException, std::exception          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    SolarMutexClearableGuard aReadLock;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.clear();
    // <- SAFE

    m_lSimpleChangeListener.addInterface(sProperty, xListener);
}

void SAL_CALL PropertySetHelper::removePropertyChangeListener(const OUString&                                            sProperty,
                                                              const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException, std::exception          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    SolarMutexClearableGuard aReadLock;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.clear();
    // <- SAFE

    m_lSimpleChangeListener.removeInterface(sProperty, xListener);
}

void SAL_CALL PropertySetHelper::addVetoableChangeListener(const OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException, std::exception          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    SolarMutexClearableGuard aReadLock;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.clear();
    // <- SAFE

    m_lVetoChangeListener.addInterface(sProperty, xListener);
}

void SAL_CALL PropertySetHelper::removeVetoableChangeListener(const OUString&                                            sProperty,
                                                              const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException, std::exception          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    SolarMutexClearableGuard aReadLock;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.clear();
    // <- SAFE

    m_lVetoChangeListener.removeInterface(sProperty, xListener);
}

css::uno::Sequence< css::beans::Property > SAL_CALL PropertySetHelper::getProperties()
    throw(css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    SolarMutexGuard g;

    sal_Int32                                        c     = (sal_Int32)m_lProps.size();
    css::uno::Sequence< css::beans::Property >       lProps(c);
    PropertySetHelper::TPropInfoHash::const_iterator pIt;

    for (  pIt  = m_lProps.begin();
           pIt != m_lProps.end();
         ++pIt                    )
    {
        lProps[--c] = pIt->second;
    }

    return lProps;
}

css::beans::Property SAL_CALL PropertySetHelper::getPropertyByName(const OUString& sName)
    throw(css::beans::UnknownPropertyException,
          css::uno::RuntimeException, std::exception          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    SolarMutexGuard g;

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sName);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    return pIt->second;
}

sal_Bool SAL_CALL PropertySetHelper::hasPropertyByName(const OUString& sName)
    throw(css::uno::RuntimeException, std::exception)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    SolarMutexGuard g;

    PropertySetHelper::TPropInfoHash::iterator pIt    = m_lProps.find(sName);
    bool                                   bExist = (pIt != m_lProps.end());

    return bExist;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
