/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <comphelper/propmultiplex.hxx>
#include <osl/diagnose.h>

//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//========================================================================
//= OPropertyChangeListener
//========================================================================
//------------------------------------------------------------------------
OPropertyChangeListener::~OPropertyChangeListener()
{
    if (m_pAdapter)
        m_pAdapter->dispose();
}

//------------------------------------------------------------------
void OPropertyChangeListener::_disposing(const EventObject&) throw( RuntimeException)
{
    // nothing to do here
}

//------------------------------------------------------------------
void OPropertyChangeListener::disposeAdapter()
{
    if ( m_pAdapter )
        m_pAdapter->dispose();

    // will automatically set a new adapter
    OSL_ENSURE( !m_pAdapter, "OPropertyChangeListener::disposeAdapter: what did dispose do?" );
}

//------------------------------------------------------------------
void OPropertyChangeListener::setAdapter(OPropertyChangeMultiplexer* pAdapter)
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
//= OPropertyChangeMultiplexer
//========================================================================
//------------------------------------------------------------------
OPropertyChangeMultiplexer::OPropertyChangeMultiplexer(OPropertyChangeListener* _pListener, const  Reference< XPropertySet>& _rxSet, sal_Bool _bAutoReleaseSet)
            :m_xSet(_rxSet)
            ,m_pListener(_pListener)
            ,m_nLockCount(0)
            ,m_bListening(sal_False)
            ,m_bAutoSetRelease(_bAutoReleaseSet)
{
    m_pListener->setAdapter(this);
}

//------------------------------------------------------------------
OPropertyChangeMultiplexer::~OPropertyChangeMultiplexer()
{
}

//------------------------------------------------------------------
void OPropertyChangeMultiplexer::lock()
{
    ++m_nLockCount;
}

//------------------------------------------------------------------
void OPropertyChangeMultiplexer::unlock()
{
    --m_nLockCount;
}

//------------------------------------------------------------------
void OPropertyChangeMultiplexer::dispose()
{
    if (m_bListening)
    {
        Reference< XPropertyChangeListener> xPreventDelete(this);

        const ::rtl::OUString* pProperties = m_aProperties.getConstArray();
        for (sal_Int32 i = 0; i < m_aProperties.getLength(); ++i, ++pProperties)
            m_xSet->removePropertyChangeListener(*pProperties, static_cast< XPropertyChangeListener*>(this));

        m_pListener->setAdapter(NULL);

        m_pListener = NULL;
        m_bListening = sal_False;

        if (m_bAutoSetRelease)
            m_xSet = NULL;
    }
}

// XEventListener
//------------------------------------------------------------------
void SAL_CALL OPropertyChangeMultiplexer::disposing( const  EventObject& _rSource) throw( RuntimeException)
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

// XPropertyChangeListener
//------------------------------------------------------------------
void SAL_CALL OPropertyChangeMultiplexer::propertyChange( const  PropertyChangeEvent& _rEvent ) throw( RuntimeException)
{
    if (m_pListener && !locked())
        m_pListener->_propertyChanged(_rEvent);
}

//------------------------------------------------------------------
void OPropertyChangeMultiplexer::addProperty(const ::rtl::OUString& _sPropertyName)
{
    if (m_xSet.is())
    {
        m_xSet->addPropertyChangeListener(_sPropertyName, static_cast< XPropertyChangeListener*>(this));
        m_aProperties.realloc(m_aProperties.getLength() + 1);
        m_aProperties.getArray()[m_aProperties.getLength()-1] = _sPropertyName;
        m_bListening = sal_True;
    }
}

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
