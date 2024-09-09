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
#include <comphelper/propmultiplex.hxx>
#include <osl/diagnose.h>


namespace comphelper
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

OPropertyChangeListener::~OPropertyChangeListener()
{
    rtl::Reference<OPropertyChangeMultiplexer> xTmpAdapter;
    {
        std::unique_lock aGuard(m_aAdapterMutex);
        xTmpAdapter = std::move(m_xAdapter);
    }
    if ( xTmpAdapter.is() )
        xTmpAdapter->dispose();
}


void OPropertyChangeListener::_disposing(const EventObject&)
{
    // nothing to do here
}


void OPropertyChangeListener::disposeAdapter()
{
    rtl::Reference<OPropertyChangeMultiplexer> xTmpAdapter;
    {
        std::unique_lock aGuard(m_aAdapterMutex);
        xTmpAdapter = std::move(m_xAdapter);
    }
    if ( xTmpAdapter.is() )
        xTmpAdapter->dispose();

    // will automatically set a new adapter
    OSL_ENSURE( !m_xAdapter.is(), "OPropertyChangeListener::disposeAdapter: what did dispose do?" );
}


void OPropertyChangeListener::setAdapter(OPropertyChangeMultiplexer* pAdapter)
{
    std::unique_lock aGuard(m_aAdapterMutex);
    m_xAdapter = pAdapter;
}

OPropertyChangeMultiplexer::OPropertyChangeMultiplexer(OPropertyChangeListener* _pListener, const  Reference< XPropertySet>& _rxSet, bool _bAutoReleaseSet)
            :m_xSet(_rxSet)
            ,m_pListener(_pListener)
            ,m_nLockCount(0)
            ,m_bListening(false)
            ,m_bAutoSetRelease(_bAutoReleaseSet)
{
    m_pListener->setAdapter(this);
}


OPropertyChangeMultiplexer::~OPropertyChangeMultiplexer()
{
}


void OPropertyChangeMultiplexer::lock()
{
    ++m_nLockCount;
}


void OPropertyChangeMultiplexer::unlock()
{
    --m_nLockCount;
}


void OPropertyChangeMultiplexer::dispose()
{
    if (!m_bListening)
        return;

    Reference< XPropertyChangeListener> xPreventDelete(this);

    for (const OUString& rProp : m_aProperties)
        m_xSet->removePropertyChangeListener(rProp, static_cast< XPropertyChangeListener*>(this));

    m_pListener->setAdapter(nullptr);

    m_pListener = nullptr;
    m_bListening = false;

    if (m_bAutoSetRelease)
        m_xSet = nullptr;
}

// XEventListener

void SAL_CALL OPropertyChangeMultiplexer::disposing( const  EventObject& _rSource)
{
    if (m_pListener)
    {
         // tell the listener
        if (!locked())
            m_pListener->_disposing(_rSource);
        // disconnect the listener
        if (m_pListener)    // may have been reset whilst calling into _disposing
            m_pListener->setAdapter(nullptr);
    }

    m_pListener = nullptr;
    m_bListening = false;

    if (m_bAutoSetRelease)
        m_xSet = nullptr;
}

// XPropertyChangeListener

void SAL_CALL OPropertyChangeMultiplexer::propertyChange( const  PropertyChangeEvent& _rEvent )
{
    if (m_pListener && !locked())
        m_pListener->_propertyChanged(_rEvent);
}


void OPropertyChangeMultiplexer::addProperty(const OUString& _sPropertyName)
{
    if (m_xSet.is())
    {
        m_xSet->addPropertyChangeListener(_sPropertyName, static_cast< XPropertyChangeListener*>(this));
        m_aProperties.push_back(_sPropertyName);
        m_bListening = true;
    }
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
