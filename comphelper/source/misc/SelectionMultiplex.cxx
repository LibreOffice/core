/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SelectionMultiplex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 10:20:04 $
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

#include <comphelper/SelectionMultiplex.hxx>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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

