/*************************************************************************
 *
 *  $RCSfile: cachemulticaster.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-15 11:48:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "cachemulticaster.hxx"

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
    typedef CacheChangeMulticaster::ListenerRef ListenerRef;
// ---------------------------------------------------------------------------

    // replacing  std::bind2nd( std::mem_fun(&aFunc), aArg )
    struct NotifyDisposing : std::unary_function<ListenerRef,void>
    {
        typedef ICachedDataProvider Arg;
        Arg & m_arg;

        NotifyDisposing(Arg * _pProvider) CFG_NOTHROW()
        : m_arg(*_pProvider)
        {}

        void operator()(ListenerRef const & _xListener) const CFG_NOTHROW()
        { _xListener->disposing(m_arg); }
    };
// ---------------------------------------------------------------------------

    // replacing  std::bind2nd( std::mem_fun(&ICachedDataListener::componentCreated), _aComponentName )
    struct NotifyCreated : std::unary_function<ListenerRef,void>
    {
        typedef ComponentRequest const Arg;
        Arg & m_arg;

        NotifyCreated(Arg * _pComponent) CFG_NOTHROW()
        : m_arg(*_pComponent)
        {}

        void operator()(ListenerRef const & _xListener) const CFG_NOTHROW()
        { _xListener->componentCreated(m_arg); }
    };
// ---------------------------------------------------------------------------

    // replacing  std::bind2nd( std::mem_fun(&ICachedDataListener::componentChanged), _aComponentName )
    struct NotifyChanged : std::unary_function<ListenerRef,void>
    {
        typedef UpdateRequest const Arg;
        Arg & m_arg;

        NotifyChanged(Arg * _pUpdate) CFG_NOTHROW()
        : m_arg(*_pUpdate)
        {}

        void operator()(ListenerRef const & _xListener) const CFG_NOTHROW()
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

inline CacheChangeMulticaster::ListenerList CacheChangeMulticaster::copyListenerList()
{
    osl::MutexGuard aListGuard(m_aMutex);
    return m_aListeners;
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::dispose(ICachedDataProvider & _rProvider) CFG_NOTHROW()
{
    osl::ClearableMutexGuard aListGuard(m_aMutex);

    ListenerList aNotifyListeners;
    aNotifyListeners.swap(m_aListeners);

    aListGuard.clear();

    std::for_each( aNotifyListeners.begin(), aNotifyListeners.end(), NotifyDisposing(&_rProvider) );
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::notifyCreated(ComponentRequest const & _aComponent) CFG_NOTHROW()
{
    ListenerList aNotifyListeners( this->copyListenerList() );

    std::for_each( aNotifyListeners.begin(), aNotifyListeners.end(), NotifyCreated(&_aComponent) );
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::notifyChanged(UpdateRequest const & _anUpdate) CFG_NOTHROW()
{
    ListenerList aNotifyListeners( this->copyListenerList() );

    std::for_each( aNotifyListeners.begin(), aNotifyListeners.end(), NotifyChanged(&_anUpdate) );
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::addListener(ListenerRef _xListener) CFG_NOTHROW()
{
    osl::MutexGuard aListGuard(m_aMutex);

    OSL_PRECOND(std::find(m_aListeners.begin(),m_aListeners.end(),_xListener) == m_aListeners.end(),
                "WARNING: Cache Change Listener was already registered - will be notified multiply.");

    OSL_PRECOND(_xListener.is(), "ERROR: trying to register a NULL listener");

    if (_xListener.is())
        m_aListeners.push_front(_xListener);
}
// ---------------------------------------------------------------------------

void CacheChangeMulticaster::removeListener(ListenerRef _xListener) CFG_NOTHROW()
{
    osl::MutexGuard aListGuard(m_aMutex);
    m_aListeners.remove(_xListener);
}
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr
