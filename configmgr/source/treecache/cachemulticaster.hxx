/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachemulticaster.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:23:57 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX
#define CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX

#ifndef CONFIGMGR_BACKEND_CACHEDDATAPROVIDER_HXX
#include "cacheddataprovider.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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

