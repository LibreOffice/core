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


#include <comphelper/SelectionMultiplex.hxx>
#include <osl/diagnose.h>

//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;

//========================================================================
//= OSelectionChangeListener
//========================================================================
//------------------------------------------------------------------------
OSelectionChangeListener::~OSelectionChangeListener()
{
    if (m_pAdapter)
        m_pAdapter->dispose();
}

//------------------------------------------------------------------
void OSelectionChangeListener::_disposing(const EventObject&) throw( RuntimeException)
{
    // nothing to do here
}

//------------------------------------------------------------------
void OSelectionChangeListener::setAdapter(OSelectionChangeMultiplexer* pAdapter)
{
    if (m_pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter->release();
        m_pAdapter = NULL;
    }

    if (pAdapter)
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        m_pAdapter = pAdapter;
        m_pAdapter->acquire();
    }
}

//========================================================================
//= OSelectionChangeMultiplexer
//========================================================================
//------------------------------------------------------------------
OSelectionChangeMultiplexer::OSelectionChangeMultiplexer(OSelectionChangeListener* _pListener, const  Reference< XSelectionSupplier>& _rxSet, sal_Bool _bAutoReleaseSet)
            :m_xSet(_rxSet)
            ,m_pListener(_pListener)
            ,m_nLockCount(0)
            ,m_bListening(sal_False)
            ,m_bAutoSetRelease(_bAutoReleaseSet)
{
    m_pListener->setAdapter(this);
    osl_atomic_increment(&m_refCount);
    {
        Reference< XSelectionChangeListener> xPreventDelete(this);
        m_xSet->addSelectionChangeListener(xPreventDelete);
    }
    osl_atomic_decrement(&m_refCount);
}

//------------------------------------------------------------------
OSelectionChangeMultiplexer::~OSelectionChangeMultiplexer()
{
}

//------------------------------------------------------------------
void OSelectionChangeMultiplexer::lock()
{
    ++m_nLockCount;
}

//------------------------------------------------------------------
void OSelectionChangeMultiplexer::unlock()
{
    --m_nLockCount;
}

//------------------------------------------------------------------
void OSelectionChangeMultiplexer::dispose()
{
    if (m_bListening)
    {
        Reference< XSelectionChangeListener> xPreventDelete(this);

        m_xSet->removeSelectionChangeListener(xPreventDelete);

        m_pListener->setAdapter(NULL);

        m_pListener = NULL;
        m_bListening = sal_False;

        if (m_bAutoSetRelease)
            m_xSet = NULL;
    }
}

// XEventListener
//------------------------------------------------------------------
void SAL_CALL OSelectionChangeMultiplexer::disposing( const  EventObject& _rSource) throw( RuntimeException)
{
    if (m_pListener)
    {
         // tell the listener
        if (!locked())
            m_pListener->_disposing(_rSource);
        // disconnect the listener
        if (m_pListener)    // may have been reset whilest calling into _disposing
            m_pListener->setAdapter(NULL);
    }

    m_pListener = NULL;
    m_bListening = sal_False;

    if (m_bAutoSetRelease)
        m_xSet = NULL;
}

// XSelectionChangeListener
//------------------------------------------------------------------
void SAL_CALL OSelectionChangeMultiplexer::selectionChanged( const  EventObject& _rEvent ) throw( RuntimeException)
{
    if (m_pListener && !locked())
        m_pListener->_selectionChanged(_rEvent);
}
//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
