/*************************************************************************
 *
 *  $RCSfile: cacheddataprovider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-06-18 15:50:04 $
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


#ifndef CONFIGMGR_BACKEND_CACHEDDATAPROVIDER_HXX
#define CONFIGMGR_BACKEND_CACHEDDATAPROVIDER_HXX

#ifndef CONFIGMGR_BACKEND_REQUEST_HXX_
#include "request.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#include "requesttypes.hxx"
#endif
#ifndef CONFIGMGR_SEGMENT_HXX
#include "segment.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------
    struct CacheLocation
    {
        memory::SegmentAddress          segment;
        memory::SegmentHeap::Address    address;

        bool isNull() const { return segment.isNull() || address == 0; }
    };
// ---------------------------------------------------------------------------
    struct IDirectDataProvider;
    struct ICachedDataNotifier;
// ---------------------------------------------------------------------------

    /** Interface providing access to configuration data from some backend,
        which is cached in a shared data cache.
    */
    struct SAL_NO_VTABLE ICachedDataProvider : Refcounted
    {
        /** locates data of a component in the cache.

            <p> If the data isn't in the cache it is loaded from the backend. </p>
            <p> When the caller is done with the data, <method>freeComponent</method>
                should be called. </p>

            @param _aRequest
                identifies the component to be loaded.

            @returns
                data that can be used to locate the loaded data in the cache.

            @throws com::sun::star::uno::Exception
                if loading the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual CacheLocation loadComponent(ComponentRequest const & _aRequest)
            CFG_UNO_THROW_ALL() = 0;

        /** releases data of a component from the cache.

            <p> Should be called when a client is done with a component.
                Each calls to <method>loadComponent</method> should
                be balanced by exactly one call to <method>freeComponent</method>.
            </p>

            @param _aRequest
                identifies a component previously loaded via <method>loadComponent</method>.

            @returns
                data that can be used to locate the loaded data in the cache.
        */
        virtual void freeComponent(ComponentRequest const & _aRequest)
            CFG_NOTHROW() = 0;

        /** refreshes data of an existing component from the backend

            <p> If the data is in the cache already, it is refreshed from the
                backend and the change are notified to all registered listeners.
            </p>
            <p> If the data isn't in the cache nothing is done and
                a NULL location is returned.
            </p>

            <p>Note: the caller <strong>must not</strong> hold any lock on the cache line affected.</p>

            @param _aRequest
                identifies the component to be refreshed.

            @returns
                data that can be used to locate the refreshed data in the cache.

                <p>If there is no data to refresh a NULL location is returned.</p>

            @throws com::sun::star::uno::Exception
                if loading the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual CacheLocation refreshComponent(ComponentRequest const & _aRequest)
            CFG_UNO_THROW_ALL() = 0;

         /** refreshes data of all existing components from the backend

            <p> If the data is in the cache already, it is refreshed from the
                backend and the changes are notified to all registered listeners.
            </p>
            <p> If the data isn't in the cache nothing is done and
                a NULL location is returned.
            </p>

            <p>Note: the caller <strong>must not</strong> hold any lock on the cache line affected.</p>

             @throws com::sun::star::uno::Exception
                if loading the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual void refreshAllComponents()
            CFG_UNO_THROW_ALL() = 0;
        /** flushes data of all pending updates from cache to the backend(s)
                @throws com::sun::star::uno::Exception
                if flushing the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual void flushPendingUpdates()
            CFG_NOTHROW() = 0;

        /** locates a template in the cache.

            <p> If the data isn't in the cache it is loaded from the backend. </p>

            <p>Note: the caller <strong>must not</strong> hold any lock on the cache line affected.</p>

            @param _aRequest
                identifies the template to be loaded.

            @returns
                data that can be used to locate the template data in the cache.

            @throws com::sun::star::uno::Exception
                if loading the template data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual CacheLocation loadTemplate(TemplateRequest const & _aRequest)
            CFG_UNO_THROW_ALL() = 0;

        /** saves changes to the backend and notifies them to registered listeners.

            <p> Must be called after the changes have been applied to the cache
                and before any subsequent changes to the same component.
            </p>

            <p> Notifications are guaranteed to be delivered
                before any subsequent changes to the same component are possible.
            </p>

            <p> Note: the caller <strong>must</strong> hold a read lock (but no write lock)
                on the cache line affected during the call.</p>

            @param _anUpdate
                identifies the node that changed and describes the changes.

            @throws com::sun::star::uno::Exception
                if saving the changes to the backend fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual void saveAndNotify(UpdateRequest const & _anUpdate)
            CFG_UNO_THROW_ALL() = 0;

        /** dispose this object and its cache and close the backend

            <p> discards the cache and flushes the backend.
            </p>
        */
        virtual void dispose()
            CFG_UNO_THROW_RTE() = 0;

       /** @returns
                an object that can used to broadcast changes done through this object.
                <p> The object returned is guaranteed to live as long
                    as this ICachedDataProvider lives.
                </p>
        */
        virtual ICachedDataNotifier & getNotifier() CFG_NOTHROW() = 0;

       /**  @returns
                an object that can be used to retrieve owned copies of the data,
                defaults and templates.
                <p> The object returned is guaranteed to live as long
                    as this ICachedDataProvider lives.
                </p>
        */
        virtual IDirectDataProvider & getDirectDataProvider() CFG_NOTHROW() = 0;
    };
// ---------------------------------------------------------------------------

    /** Listener interface for observing changes in the cache
        managed by a <type>ICachedDataProvider</type>
    */
    struct SAL_NO_VTABLE ICachedDataListener : Refcounted
    {
        /// is called when the provider is closing down
        virtual void disposing(ICachedDataProvider & _rProvider) CFG_NOTHROW() = 0;
        /// is called when a new component was loaded into the cache.
        virtual void componentCreated(ComponentRequest const & _aComponent)      CFG_NOTHROW() = 0;
        /// is called when data of an already loaded component changed in the cache.
        virtual void componentChanged(UpdateRequest  const & _anUpdate) CFG_NOTHROW() = 0;
    };
// ---------------------------------------------------------------------------

    /** Interface providing a multicasting service for changes to the cache
        managed by a <type>ICachedDataProvider</type>
    */
    struct SAL_NO_VTABLE ICachedDataNotifier
    {
    // firing notifications.
        /// notify all registered listeners and close down this notifier
        virtual void dispose(ICachedDataProvider & _rProvider) CFG_NOTHROW() = 0;

        /** notify a new component to all registered listeners.
            <p> Must be called after the component has been created in the cache.</p>
        */
        virtual void notifyCreated(ComponentRequest const & _aComponent) CFG_NOTHROW() = 0;

        /** notify changed data to all registered listeners.
            <p> Must be called after the change has been applied to the cache
                and before any subsequent changes to the same component.</p>
        */
        virtual void notifyChanged(UpdateRequest const & _anUpdate) CFG_NOTHROW() = 0;

    // listener registration.
        typedef rtl::Reference<ICachedDataListener> ListenerRef;

       /// register a listener for observing changes to the cached data
        virtual void addListener(ListenerRef _xListener) CFG_NOTHROW() = 0;
        /// unregister a listener previously registered
        virtual void removeListener(ListenerRef _xListener) CFG_NOTHROW() = 0;
    };
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

