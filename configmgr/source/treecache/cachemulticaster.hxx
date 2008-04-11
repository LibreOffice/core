/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachemulticaster.hxx,v $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX
#define CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX

#include "cacheddataprovider.hxx"
#include <osl/mutex.hxx>

#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------

    /** Interface providing a multicasting service for changes to the cache
        managed by a <type>ICachedDataProvider</type>
    */
    class CacheChangeMulticaster : public ICachedDataNotifier
    {
    public:
        CacheChangeMulticaster();
        virtual ~CacheChangeMulticaster();

    // ICachedDataNotifier implementation
    public:
        /// notify all registered listeners and close down this notifier
        virtual void dispose(ICachedDataProvider & _rProvider) CFG_NOTHROW();

        /** notify a new component to all registered listeners.
            <p> Must be called after the component has been created in the cache.</p>
        */
        virtual void notifyCreated(ComponentRequest const & _aComponentName) CFG_NOTHROW();

        /** notify changed data to all registered listeners.
            <p> Must be called after the change has been applied to the cache
                and before any subsequent changes to the same component.</p>
        */
        virtual void notifyChanged(UpdateRequest const & _anUpdate) CFG_NOTHROW();

    // notification support.
        /// register a listener for observing changes to the cached data
        virtual void addListener(ListenerRef _xListener) CFG_NOTHROW();
        /// unregister a listener previously registered
        virtual void removeListener(ListenerRef _xListener) CFG_NOTHROW();
    private:
        typedef std::list<ListenerRef> ListenerList;

        ListenerList copyListenerList();

        osl::Mutex   m_aMutex;
        ListenerList m_aListeners;
    };
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

