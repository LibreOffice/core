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
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ref.hxx>
#include <mutex>
#include <vector>

namespace com::sun::star::beans
{
class XPropertySet;
}

//= property helper classes

namespace comphelper
{
class OPropertyChangeMultiplexer2;

//= OPropertyChangeListener

/// simple listener adapter for property sets
class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OPropertyChangeListener2
{
    friend class OPropertyChangeMultiplexer2;

    rtl::Reference<OPropertyChangeMultiplexer2> m_xAdapter;

public:
    virtual ~OPropertyChangeListener2();

    /// @throws css::uno::RuntimeException
    virtual void _propertyChanged(const css::beans::PropertyChangeEvent& _rEvent) = 0;

protected:
    /** If the derivee also owns the mutex which we know as reference, then call this within your
            derivee's dtor.
        */
    void disposeAdapter(std::unique_lock<std::mutex>& rGuard);

private:
    void setAdapter(std::unique_lock<std::mutex>& rGuard, OPropertyChangeMultiplexer2* _pAdapter);
};

//= OPropertyChangeMultiplexer2
// A copy of OPropertyChangeMultiplexer except that it uses std::mutex instead osl::Mutex

/// multiplexer for property changes
// workaround for incremental linking bugs in MSVC2019
class SAL_DLLPUBLIC_TEMPLATE OPropertyChangeMultiplexer_Base2
    : public cppu::WeakImplHelper<css::beans::XPropertyChangeListener>
{
};
class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OPropertyChangeMultiplexer2 final
    : public OPropertyChangeMultiplexer_Base2
{
    friend class OPropertyChangeListener2;
    std::mutex& m_rMutex;
    std::vector<OUString> m_aProperties;
    css::uno::Reference<css::beans::XPropertySet> m_xSet;
    OPropertyChangeListener2* m_pListener;
    sal_Int32 m_nLockCount;
    bool m_bListening : 1;

    void onListenerDestruction();
    virtual ~OPropertyChangeMultiplexer2() override;

public:
    OPropertyChangeMultiplexer2(std::mutex& rMutex, std::unique_lock<std::mutex>& rGuard,
                                OPropertyChangeListener2* _pListener,
                                const css::uno::Reference<css::beans::XPropertySet>& _rxSet);

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

    /// incremental lock
    void lock();
    /// incremental unlock
    void unlock();
    /// get the lock count
    sal_Int32 locked() const { return m_nLockCount; }

    void addProperty(const OUString& aPropertyName);
    void dispose(std::unique_lock<std::mutex>& rGuard);
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
