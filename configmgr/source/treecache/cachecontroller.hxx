/*************************************************************************
 *
 *  $RCSfile: cachecontroller.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:41 $
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


#ifndef CONFIGMGR_BACKEND_CACHECONTROLLER_HXX
#define CONFIGMGR_BACKEND_CACHECONTROLLER_HXX

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_CACHEDDATAPROVIDER_HXX
#include "cacheddataprovider.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#include "mergeddataprovider.hxx"
#endif

#ifndef CONFIGMGR_CACHEACCESS_HXX
#include "cacheaccess.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX
#include "cachemulticaster.hxx"
#endif
#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif

#ifndef CONFIGMGR_AUTOREFERENCEMAP_HXX
#include "autoreferencemap.hxx"
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
    class OTreeDisposeScheduler;
    class OCacheWriteScheduler;
// ---------------------------------------------------------------------------
    namespace backend
    {
// ---------------------------------------------------------------------------

    /** manages a shared data cache for configuration data
        trying to ensure consistency with a backend
        and provides access to the data for clients
    */
    class CacheController

    : public ICachedDataProvider
    , public IDirectDataProvider // Refcounted
    {
        typedef backend::IMergedDataProvider Backend;
        typedef rtl::Reference< Backend > BackendRef;
    public:
        /** ctor
        */
        explicit
        CacheController(BackendRef const & _xBackend, memory::HeapManager & _rCacheHeapManager);

    // ICachedDataProvider implementation
    public:
        // disposing the cache before destroying
        virtual void dispose() CFG_UNO_THROW_RTE();

        /** locates data of a component in the cache.

            <p> If the data isn't in the cache it is loaded from the backend. </p>

            @param _aRequest
                identifies the component to be loaded.

            @returns
                data that can be used to locate the loaded data in the cache.

            @throws com::sun::star::uno::Exception
                if loading the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual CacheLocation loadComponent(ComponentRequest const & _aRequest)
            CFG_UNO_THROW_ALL();

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
            CFG_NOTHROW();

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
            CFG_UNO_THROW_ALL();

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
            CFG_UNO_THROW_ALL();

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
            CFG_UNO_THROW_ALL();

       /** @returns
                an object that can used to broadcast changes done through this object.
                <p> The object returned is guaranteed to live as long
                    as this ICachedDataProvider lives.
                </p>
        */
        virtual ICachedDataNotifier & getNotifier() CFG_NOTHROW()
        { return m_aNotifier; }

       /**  @returns
                an object that can be used to retrieve owned copies of the data,
                defaults and templates.
                <p> The object returned is guaranteed to live as long
                    as this ICachedDataProvider lives.
                </p>
        */
        virtual IDirectDataProvider & getDirectDataProvider() CFG_NOTHROW()
        { return *this; }

    // IDirectDataProvider implementation
    public:
        /** loads merged data for a (complete) tree and returns it as return value.

            @param _aRequest
                identifies the component to be loaded

            @returns
                A valid component instance for the given component.

            @throws com::sun::star::uno::Exception
                if the node cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.

        */
        virtual ComponentResult getComponentData(ComponentRequest const & _aRequest)
            CFG_UNO_THROW_ALL();

        /** loads default data for a (partial) tree and returns it as return value

            @param _aRequest
                identifies the node to be loaded

            @returns
                A valid node instance for the default state of the given node.

                <p>May be NULL, if the node exists but has no default equivalent.</p>

            @throws com::sun::star::uno::Exception
                if the default cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual NodeResult getDefaultData(NodeRequest const & _aRequest)
            CFG_UNO_THROW_ALL();

        /** loads a given template and returns it as return value

            @param _aRequest
                identifies the template to be loaded

            @returns
                A valid instance of the given template.

                <p> Currently a request with empty template name
                    will retrieve a group node holding all templates
                    of a component.
                </p>

            @throws com::sun::star::uno::Exception
                if the template cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.
        */
        virtual TemplateResult getTemplateData(TemplateRequest const & _aRequest)
            CFG_UNO_THROW_ALL();

    protected:
        // ref counted, that's why no public dtor
        ~CacheController();

        memory::HeapManager & getCacheHeapManager() const;
    // implementation
    private:
        typedef CacheLoadingAccess      Cache;
        typedef rtl::Reference<Cache>   CacheRef;

    private:
        AbsolutePath encodeTemplateLocation(const Name& _rName, const Name &_rModule) const;

        AbsolutePath ensureTemplate(Name const& _rName, Name const& _rModule) CFG_UNO_THROW_ALL(  );

        // adjust a node result for locale, ...
        bool normalizeResult(std::auto_ptr<ISubtree> &  _aResult, RequestOptions const & _aOptions);

        // reads data from the backend directly
        ComponentResult loadDirectly(ComponentRequest const & _aRequest) CFG_UNO_THROW_ALL(  );
        // reads default data from the backend directly
        NodeResult loadDefaultsDirectly(NodeRequest const & _aRequest) CFG_UNO_THROW_ALL(  );
        // writes an update  to the backend directly
        void saveDirectly(UpdateRequest const & _anUpdate) CFG_UNO_THROW_ALL(  );

        // marks a component as invalid and initiates a (background ?) refresh on it
        void invalidateComponent(ComponentRequest const & _aComponent) CFG_UNO_THROW_ALL(  );
        // writes updates for a component to the backend directly
        void savePendingChanges(CacheRef const & _aCache, ComponentRequest const & _aComponent)
            CFG_UNO_THROW_ALL(  );
        // saves all pending changes from a cache access to the backend
        bool saveAllPendingChanges(CacheRef const & _aCache, RequestOptions const & _aOptions)
            CFG_UNO_THROW_RTE(  );
        // load templates componentwise from backend
        std::auto_ptr<ISubtree> loadTemplateData(TemplateRequest const & _aRequest)
            CFG_UNO_THROW_ALL(  );
        // add templates componentwise to cache
        data::TreeAddress addTemplates ( backend::ComponentData const & _aComponentInstance );
        CacheRef getCacheAlways(RequestOptions const & _aOptions);

        OTreeDisposeScheduler   * createDisposer();
        OCacheWriteScheduler    * createCacheWriter();

        void flushPendingUpdates();

        // disposing
        void disposeAll(bool _bFlushRemainingUpdates);
        void disposeOne(RequestOptions const & _aOptions, bool _bFlushUpdates = true);
        void disposeUser(RequestOptions const & _aUserOptions, bool _bFlushUpdates = true);
        void implDisposeOne(CacheRef const & _aCache, RequestOptions const & _aOptions, bool _bFlushUpdates);

        void closeModules(Cache::DisposeList & _aList, RequestOptions const & _aOptions);
    private:
        typedef AutoReferenceMap<RequestOptions,Cache,lessRequestOptions>   CacheList;
        typedef TemplateCacheData TemplateCache;


        CacheChangeMulticaster  m_aNotifier;
        BackendRef              m_xBackend;

        CacheList               m_aCacheList; // Map
        TemplateCache           m_aTemplates;

        osl::Mutex              m_aTemplatesMutex;

        OTreeDisposeScheduler*  m_pDisposer;
        OCacheWriteScheduler *  m_pCacheWriter;

        bool m_bDisposing; // disables async writing and automatic refresh

        friend class configmgr::OTreeDisposeScheduler;
        friend class configmgr::OCacheWriteScheduler;
        friend class OInvalidateTreeThread;

    };
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

