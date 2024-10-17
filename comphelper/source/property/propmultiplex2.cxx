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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/propmultiplex2.hxx>
#include <osl/diagnose.h>

namespace comphelper
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

OPropertyChangeListener2::~OPropertyChangeListener2()
{
    if (m_xAdapter.is())
        m_xAdapter->onListenerDestruction();
}

void OPropertyChangeListener2::disposeAdapter(std::unique_lock<std::mutex>& rGuard)
{
    if (m_xAdapter.is())
        m_xAdapter->dispose(rGuard);

    // will automatically set a new adapter
    OSL_ENSURE(!m_xAdapter.is(), "OPropertyChangeListener::disposeAdapter: what did dispose do?");
}

void OPropertyChangeListener2::setAdapter(std::unique_lock<std::mutex>& /*rGuard*/,
                                          OPropertyChangeMultiplexer2* pAdapter)
{
    m_xAdapter = pAdapter;
}

OPropertyChangeMultiplexer2::OPropertyChangeMultiplexer2(std::mutex& rMutex,
                                                         std::unique_lock<std::mutex>& rGuard,
                                                         OPropertyChangeListener2* _pListener,
                                                         const Reference<XPropertySet>& _rxSet)
    : m_rMutex(rMutex)
    , m_xSet(_rxSet)
    , m_pListener(_pListener)
    , m_nLockCount(0)
    , m_bListening(false)
{
    m_pListener->setAdapter(rGuard, this);
}

OPropertyChangeMultiplexer2::~OPropertyChangeMultiplexer2() {}

void OPropertyChangeMultiplexer2::lock() { ++m_nLockCount; }

void OPropertyChangeMultiplexer2::unlock() { --m_nLockCount; }

void OPropertyChangeMultiplexer2::dispose(std::unique_lock<std::mutex>& rGuard)
{
    if (!m_bListening)
        return;

    Reference<XPropertyChangeListener> xPreventDelete(this);

    for (const OUString& rProp : m_aProperties)
        m_xSet->removePropertyChangeListener(rProp, static_cast<XPropertyChangeListener*>(this));

    m_pListener->setAdapter(rGuard, nullptr);

    m_pListener = nullptr;
    m_bListening = false;

    m_xSet = nullptr;
}

void OPropertyChangeMultiplexer2::onListenerDestruction()
{
    if (!m_bListening)
        return;

    Reference<XPropertyChangeListener> xPreventDelete(this);

    for (const OUString& rProp : m_aProperties)
        m_xSet->removePropertyChangeListener(rProp, static_cast<XPropertyChangeListener*>(this));
}

// XEventListener

void SAL_CALL OPropertyChangeMultiplexer2::disposing(const EventObject& /*_rSource*/)
{
    std::unique_lock g(m_rMutex);
    // disconnect the listener
    if (m_pListener)
        m_pListener->setAdapter(g, nullptr);

    m_pListener = nullptr;
    m_bListening = false;

    m_xSet = nullptr;
}

// XPropertyChangeListener

void SAL_CALL OPropertyChangeMultiplexer2::propertyChange(const PropertyChangeEvent& _rEvent)
{
    if (m_pListener && !locked())
        m_pListener->_propertyChanged(_rEvent);
}

void OPropertyChangeMultiplexer2::addProperty(const OUString& _sPropertyName)
{
    if (m_xSet.is())
    {
        m_xSet->addPropertyChangeListener(_sPropertyName,
                                          static_cast<XPropertyChangeListener*>(this));
        m_aProperties.push_back(_sPropertyName);
        m_bListening = true;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
