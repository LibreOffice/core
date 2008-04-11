/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SelectionMultiplex.cxx,v $
 * $Revision: 1.3 $
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
void OSelectionChangeListener::disposeAdapter()
{
    if ( m_pAdapter )
        m_pAdapter->dispose();

    // will automatically set a new adapter
    OSL_ENSURE( !m_pAdapter, "OSelectionChangeListener::disposeAdapter: what did dispose do?" );
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
    osl_incrementInterlockedCount(&m_refCount);
    {
        Reference< XSelectionChangeListener> xPreventDelete(this);
        m_xSet->addSelectionChangeListener(xPreventDelete);
    }
    osl_decrementInterlockedCount(&m_refCount);
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

