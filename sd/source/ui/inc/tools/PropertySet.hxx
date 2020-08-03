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

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <map>
#include <memory>

namespace sd::tools {

typedef ::cppu::WeakComponentImplHelper <
    css::beans::XPropertySet
> PropertySetInterfaceBase;

/** A very simple implementation of the XPropertySet interface.  It does not
    support constrained properties and thus does not support vetoable
    listeners.  It does not support the optional property set info.

    In order to use it you have to derive from this class and implement the
    GetPropertyValue() and SetPropertyValue() methods.
*/
class PropertySet
    : protected ::cppu::BaseMutex,
      public PropertySetInterfaceBase
{
public:
    explicit PropertySet();
    virtual ~PropertySet() override;

    virtual void SAL_CALL disposing() override;

    // XPropertySet

    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue (
        const OUString& rsPropertyName,
        const css::uno::Any& rsPropertyValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue (const OUString& rsPropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener (
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;

    virtual void SAL_CALL removePropertyChangeListener (
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;

    virtual void SAL_CALL addVetoableChangeListener (
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;

    virtual void SAL_CALL removeVetoableChangeListener (
        const OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;

protected:
    /** Return the requested property value.
        @throw css::beans::UnknownPropertyException when the
            property is not supported.
    */
    virtual css::uno::Any GetPropertyValue (const OUString& rsPropertyName) = 0;
    /** Set the given property value.
        @return the old value.
        @throw css::beans::UnknownPropertyException when the
            property is not supported.
    */
    virtual css::uno::Any SetPropertyValue (
        const OUString& rsPropertyName,
        const css::uno::Any& rValue) = 0;

private:
    typedef ::std::multimap<OUString,
        css::uno::Reference<css::beans::XPropertyChangeListener> > ChangeListenerContainer;
    std::unique_ptr<ChangeListenerContainer> mpChangeListeners;

    /** Call all listeners that are registered for the given property name.
        Call this method with an empty property name to call listeners that
        are registered for all properties.
    */
    void CallListeners (
        const OUString& rsPropertyName,
        const css::beans::PropertyChangeEvent& rEvent);

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();
};

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
