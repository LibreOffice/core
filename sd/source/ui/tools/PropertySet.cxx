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

#include "tools/PropertySet.hxx"
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace tools {

PropertySet::PropertySet()
    : PropertySetInterfaceBase(m_aMutex),
      mpChangeListeners(new ChangeListenerContainer())
{
}

PropertySet::~PropertySet()
{
}

void SAL_CALL PropertySet::disposing()
{
}

//----- XPropertySet ----------------------------------------------------------

Reference<beans::XPropertySetInfo> SAL_CALL PropertySet::getPropertySetInfo()
    throw(RuntimeException, std::exception)
{
    return NULL;
}

void SAL_CALL PropertySet::setPropertyValue (
    const OUString& rsPropertyName,
    const css::uno::Any& rsPropertyValue)
    throw(css::beans::UnknownPropertyException,
        css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    Any aOldValue (SetPropertyValue(rsPropertyName,rsPropertyValue));
    if (aOldValue != rsPropertyValue)
    {
        // Inform listeners that are registered specifically for the
        // property and those registered for any property.
        beans::PropertyChangeEvent aEvent(
            static_cast<XWeak*>(this),
            rsPropertyName,
            sal_False,
            -1,
            aOldValue,
            rsPropertyValue);
        CallListeners(rsPropertyName, aEvent);
        CallListeners(OUString(), aEvent);
    }
}

Any SAL_CALL PropertySet::getPropertyValue (const OUString& rsPropertyName)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return GetPropertyValue(rsPropertyName);
}

void SAL_CALL PropertySet::addPropertyChangeListener (
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    if ( ! rxListener.is())
        throw lang::IllegalArgumentException();

    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;

    mpChangeListeners->insert(
        ChangeListenerContainer::value_type(
            rsPropertyName,
            rxListener));
}

void SAL_CALL PropertySet::removePropertyChangeListener (
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
    throw(beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    ::std::pair<ChangeListenerContainer::iterator,ChangeListenerContainer::iterator>
        aRange (mpChangeListeners->equal_range(rsPropertyName));

    ChangeListenerContainer::iterator iListener (
        ::std::find_if(
            aRange.first,
            aRange.second,
            [&rxListener] (const ChangeListenerContainer::value_type& listener) {
                return listener.second == rxListener;
            }));

    if (iListener != mpChangeListeners->end())
    {
        mpChangeListeners->erase(iListener);
    }
    else
    {
        throw lang::IllegalArgumentException();
    }
}

void SAL_CALL PropertySet::addVetoableChangeListener (
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    // Constraint properties are not supported and thus no vetoable
    // listeners.
    (void)rsPropertyName;
    (void)rxListener;
}

void SAL_CALL PropertySet::removeVetoableChangeListener (
    const OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    // Constraint properties are not supported and thus no vetoable
    // listeners.
    (void)rsPropertyName;
    (void)rxListener;
}

void PropertySet::CallListeners (
    const OUString& rsPropertyName,
    const beans::PropertyChangeEvent& rEvent)
{
    ::std::pair<ChangeListenerContainer::iterator,ChangeListenerContainer::iterator>
        aRange (mpChangeListeners->equal_range(rsPropertyName));
    ChangeListenerContainer::const_iterator iListener;
    for (iListener=aRange.first; iListener!=aRange.second; ++iListener)
    {
        if (iListener->second.is())
            iListener->second->propertyChange(rEvent);
    }
}

void PropertySet::ThrowIfDisposed()
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
                "PropertySet object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
