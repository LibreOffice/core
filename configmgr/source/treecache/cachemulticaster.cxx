/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachemulticaster.cxx,v $
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
#include "precompiled_configmgr.hxx"

#include "cachemulticaster.hxx"
#include "treemanager.hxx"

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------
namespace
{
// manually implemented helpers, as rtl::References don't work well with std binders
// ---------------------------------------------------------------------------

    // replacing  std::bind2nd( std::mem_fun(&aFunc), aArg )
    struct NotifyDisposing : std::unary_function<rtl::Reference<TreeManager>,void>
    {
        CacheController & m_arg;

        NotifyDisposing(CacheController * _pProvider) SAL_THROW(())
        : m_arg(*_pProvider)
        {}

        void operator()(rtl::Reference<TreeManager> const & _xListener) const SAL_THROW(())
        { _xListener->disposing(m_arg); }
    };
// ---------------------------------------------------------------------------

    // replacing  std::bind2nd( std::mem_fun(&TreeManager::componentCreated), _aComponentName )
    struct NotifyCreated : std::unary_function<rtl::Reference<TreeManager>,void>
    {
        ComponentRequest const & m_arg;

        NotifyCreated(ComponentRequest const * _pComponent) SAL_THROW(())
        : m_arg(*_pComponent)
        {}

        void operator()(rtl::Reference<TreeManager> const & _xListener) const SAL_THROW(())
        { _xListener->componentCreated(m_arg); }
    };
// ---------------------------------------------------------------------------

    // replacing  std::bind2nd( std::mem_fun(&TreeManager::componentChanged), _aComponentName )
    struct NotifyChanged : std::unary_function<rtl::Reference<TreeManager>,void>
    {
        UpdateRequest const & m_arg;

        NotifyChanged(UpdateRequest const * _pUpdate) SAL_THROW(())
        : m_arg(*_pUpdate)
        {}

        void operator()(rtl::Reference<TreeManager> const & _xListener) const SAL_THROW(())
        { _xListener->componentChanged(m_arg); }
    };
// ---------------------------------------------------------------------------
} // anonymous namespace
//----------------------------------------------------------------------------

CacheChangeMulticaster::CacheChangeMulticaster()
: m_aMutex()
, m_aListeners()
{
}
// ---------------------------------------------------------------------------

CacheChangeMulticaster::~CacheChangeMulticaster()
{
    OSL_ENSURE( m_aListeners.empty(), "Forgot to dispose multicaster" );
}
// ---------------------------------------------------------------------------

inline std::list< rtl::Reference<TreeManager> > CacheChangeMulticaster::copyListenerList()
{
    osl::MutexGuard aListGuard(m_aMutex);
    return m_aListeners;
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::notifyCreated(ComponentRequest const & _aComponent) SAL_THROW(())
{
    std::list< rtl::Reference<TreeManager> > aNotifyListeners( this->copyListenerList() );

    std::for_each( aNotifyListeners.begin(), aNotifyListeners.end(), NotifyCreated(&_aComponent) );
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::notifyChanged(UpdateRequest const & _anUpdate) SAL_THROW(())
{
    std::list< rtl::Reference<TreeManager> > aNotifyListeners( this->copyListenerList() );

    std::for_each( aNotifyListeners.begin(), aNotifyListeners.end(), NotifyChanged(&_anUpdate) );
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::addListener(rtl::Reference<TreeManager> _xListener) SAL_THROW(())
{
    osl::MutexGuard aListGuard(m_aMutex);

    OSL_PRECOND(std::find(m_aListeners.begin(),m_aListeners.end(),_xListener) == m_aListeners.end(),
                "WARNING: Cache Change Listener was already registered - will be notified multiply.");

    OSL_PRECOND(_xListener.is(), "ERROR: trying to register a NULL listener");

    if (_xListener.is())
        m_aListeners.push_front(_xListener);
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::removeListener(rtl::Reference<TreeManager> _xListener) SAL_THROW(())
{
    osl::MutexGuard aListGuard(m_aMutex);
    m_aListeners.remove(_xListener);
}
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr
