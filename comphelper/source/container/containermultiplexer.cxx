/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: containermultiplexer.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include "comphelper/containermultiplexer.hxx"
#include "comphelper/uno3.hxx"
#include <osl/diagnose.h>
//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

    //=====================================================================
    //= OContainerListener
    //=====================================================================
    //---------------------------------------------------------------------
    OContainerListener::OContainerListener(::osl::Mutex& _rMutex)
        :m_pAdapter(NULL)
        ,m_rMutex(_rMutex)
    {
    }

    //---------------------------------------------------------------------
    OContainerListener::~OContainerListener()
    {
        if (m_pAdapter)
        {
            m_pAdapter->dispose();
            m_pAdapter = NULL;
        }
    }

    //---------------------------------------------------------------------
    void OContainerListener::_elementInserted( const ContainerEvent& /*_rEvent*/ ) throw(RuntimeException)
    {
    }

    //---------------------------------------------------------------------
    void OContainerListener::_elementRemoved( const ContainerEvent& ) throw(RuntimeException)
    {
    }

    //---------------------------------------------------------------------
    void OContainerListener::_elementReplaced( const ContainerEvent& /*_rEvent*/ ) throw(RuntimeException)
    {
    }

    //---------------------------------------------------------------------
    void OContainerListener::_disposing(const EventObject& ) throw( RuntimeException)
    {
    }

    //------------------------------------------------------------------
    void OContainerListener::setAdapter(OContainerListenerAdapter* pAdapter)
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

    //=====================================================================
    //= OContainerListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OContainerListenerAdapter::OContainerListenerAdapter(OContainerListener* _pListener,
            const  Reference< XContainer >& _rxContainer)
        :m_xContainer(_rxContainer)
        ,m_pListener(_pListener)
        ,m_nLockCount(0)
    {
        if (m_pListener)
            m_pListener->setAdapter(this);

        ::comphelper::increment(m_refCount);
        try
        {
            m_xContainer->addContainerListener(this);
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exceptiopn catched!");
        }
        ::comphelper::decrement(m_refCount);
    }

    //---------------------------------------------------------------------
    OContainerListenerAdapter::~OContainerListenerAdapter()
    {
    }

    //------------------------------------------------------------------
    void OContainerListenerAdapter::lock()
    {
        ++m_nLockCount;
    }

    //------------------------------------------------------------------
    void OContainerListenerAdapter::unlock()
    {
        --m_nLockCount;
    }

    //------------------------------------------------------------------
    void OContainerListenerAdapter::dispose()
    {
        if (m_xContainer.is())
        {
            try
            {
                Reference< XContainerListener > xPreventDelete(this);
                m_xContainer->removeContainerListener(xPreventDelete);
                m_pListener->setAdapter(NULL);
            }
            catch(const Exception&)
            {
                OSL_ENSURE(0,"Exception catched!");
            }
            m_xContainer = NULL;
            m_pListener = NULL;
        }
    }

    //------------------------------------------------------------------
    void SAL_CALL OContainerListenerAdapter::disposing( const  EventObject& _rSource) throw(RuntimeException)
    {
        if (m_pListener)
        {
             // tell the listener
            if (!locked())
                m_pListener->_disposing(_rSource);
            // disconnect the listener
            if ( m_pListener )
                m_pListener->setAdapter(NULL);
        }

        m_xContainer = NULL;
        m_pListener = NULL;
    }

    //------------------------------------------------------------------
    void SAL_CALL OContainerListenerAdapter::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
    {
        if (m_pListener && !locked())
            m_pListener->_elementInserted(_rEvent);
    }

    //------------------------------------------------------------------
    void SAL_CALL OContainerListenerAdapter::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
    {
        if (m_pListener && !locked())
            m_pListener->_elementRemoved(_rEvent);
    }

    //------------------------------------------------------------------
    void SAL_CALL OContainerListenerAdapter::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
    {
        if (m_pListener && !locked())
            m_pListener->_elementReplaced(_rEvent);
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

