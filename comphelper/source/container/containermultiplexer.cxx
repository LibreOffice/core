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

#include <comphelper/containermultiplexer.hxx>
#include <osl/diagnose.h>

namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

    OContainerListener::OContainerListener(::osl::Mutex& _rMutex)
        :m_pAdapter(nullptr)
        ,m_rMutex(_rMutex)
    {
    }


    OContainerListener::~OContainerListener()
    {
        if (m_pAdapter)
        {
            m_pAdapter->dispose();
            m_pAdapter = nullptr;
        }
    }


    void OContainerListener::_elementInserted( const ContainerEvent& /*_rEvent*/ )
        throw (RuntimeException, std::exception)
    {
    }


    void OContainerListener::_elementRemoved( const ContainerEvent& )
        throw (RuntimeException, std::exception)
    {
    }


    void OContainerListener::_elementReplaced( const ContainerEvent& /*_rEvent*/ )
        throw (RuntimeException, std::exception)
    {
    }


    void OContainerListener::_disposing(const EventObject& )
        throw (RuntimeException, std::exception)
    {
    }


    void OContainerListener::setAdapter(OContainerListenerAdapter* pAdapter)
    {
        if (m_pAdapter)
        {
            ::osl::MutexGuard aGuard(m_rMutex);
            m_pAdapter->release();
            m_pAdapter = nullptr;
        }

        if (pAdapter)
        {
            ::osl::MutexGuard aGuard(m_rMutex);
            m_pAdapter = pAdapter;
            m_pAdapter->acquire();
        }
    }

    OContainerListenerAdapter::OContainerListenerAdapter(OContainerListener* _pListener,
            const  Reference< XContainer >& _rxContainer)
        :m_xContainer(_rxContainer)
        ,m_pListener(_pListener)
        ,m_nLockCount(0)
    {
        if (m_pListener)
            m_pListener->setAdapter(this);

        osl_atomic_increment(&m_refCount);
        try
        {
            m_xContainer->addContainerListener(this);
        }
        catch(const Exception&)
        {
            OSL_FAIL("Exception caught!");
        }
        osl_atomic_decrement(&m_refCount);
    }


    OContainerListenerAdapter::~OContainerListenerAdapter()
    {
    }


    void OContainerListenerAdapter::dispose()
    {
        if (m_xContainer.is())
        {
            try
            {
                Reference< XContainerListener > xPreventDelete(this);
                m_xContainer->removeContainerListener(xPreventDelete);
                m_pListener->setAdapter(nullptr);
            }
            catch(const Exception&)
            {
                OSL_FAIL("Exception caught!");
            }
            m_xContainer = nullptr;
            m_pListener = nullptr;
        }
    }


    void SAL_CALL OContainerListenerAdapter::disposing( const  EventObject& _rSource) throw(RuntimeException, std::exception)
    {
        if (m_pListener)
        {
             // tell the listener
            if (!locked())
                m_pListener->_disposing(_rSource);
            // disconnect the listener
            if ( m_pListener )
                m_pListener->setAdapter(nullptr);
        }

        m_xContainer = nullptr;
        m_pListener = nullptr;
    }


    void SAL_CALL OContainerListenerAdapter::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException, std::exception)
    {
        if (m_pListener && !locked())
            m_pListener->_elementInserted(_rEvent);
    }


    void SAL_CALL OContainerListenerAdapter::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException, std::exception)
    {
        if (m_pListener && !locked())
            m_pListener->_elementRemoved(_rEvent);
    }


    void SAL_CALL OContainerListenerAdapter::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException, std::exception)
    {
        if (m_pListener && !locked())
            m_pListener->_elementReplaced(_rEvent);
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
