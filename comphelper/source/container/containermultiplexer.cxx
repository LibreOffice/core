/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: containermultiplexer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 10:09:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#ifndef _COMPHELPER_CONTAINERMULTIPLEXER_HXX_
#include "comphelper/containermultiplexer.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include "comphelper/uno3.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
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

