/*************************************************************************
 *
 *  $RCSfile: cachemulticaster.hxx,v $
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
        ~CacheChangeMulticaster();

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

