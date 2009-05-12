/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachecontroller.hxx,v $
 * $Revision: 1.13 $
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

#ifndef CONFIGMGR_BACKEND_CACHECONTROLLER_HXX
#define CONFIGMGR_BACKEND_CACHECONTROLLER_HXX

#include "sal/config.h"

#include "salhelper/simplereferenceobject.hxx"

#include "utility.hxx"
#include "mergeddataprovider.hxx"
#include "cacheaccess.hxx"
#include "cachemulticaster.hxx"
#include "requestoptions.hxx"
#include "autoreferencemap.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>

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
    class CacheController:
            public salhelper::SimpleReferenceObject,
            public ITemplateDataProvider, public INodeDataListener
    {
    public:
        /** ctor
        */
        explicit
        CacheController(rtl::Reference< backend::IMergedDataProvider > const & _xBackend,
                        const uno::Reference<uno::XComponentContext>& xContext);

        // disposing the cache before destroying
        void dispose() SAL_THROW((com::sun::star::uno::RuntimeException));

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
        sharable::TreeFragment * loadComponent(ComponentRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception));

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
        void freeComponent(ComponentRequest const & _aRequest)
            SAL_THROW(());

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
        sharable::TreeFragment * refreshComponent(ComponentRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception));
         /** refreshes data of all existing components from the backend

            <p> If the data is in the cache already, it is refreshed from the
                backend and the change are notified to all registered listeners.
            </p>
            <p> If the data isn't in the cache nothing is done and
                a NULL location is returned.
            </p>

            <p>Note: the caller <strong>must not</strong> hold any lock on the cache line affected.</p>

             @throws com::sun::star::uno::Exception
                if loading the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        void refreshAllComponents()
            SAL_THROW((com::sun::star::uno::Exception));

        /** flushes data of all pending updates from cache to the backend(s)
                @throws com::sun::star::uno::Exception
                if flushing the data fails.
                The exact exception being thrown may depend on the underlying backend.
        */
        void flushPendingUpdates() SAL_THROW((com::sun::star::uno::Exception));

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
        sharable::TreeFragment * loadTemplate(TemplateRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception));

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
        void saveAndNotify(UpdateRequest const & _anUpdate)
            SAL_THROW((com::sun::star::uno::Exception));

       /** @returns
                an object that can used to broadcast changes done through this object.
                <p> The object returned is guaranteed to live as long
                    as this object lives.
                </p>
        */
        CacheChangeMulticaster & getNotifier() SAL_THROW(())
        { return m_aNotifier; }

        /** loads merged data for a (complete) tree and returns it as return value.

            @param _aRequest
                identifies the component to be loaded

            @param __bAddListenter
                identifies is listener is to be registered to backend

            @returns
                A valid component instance for the given component.

            @throws com::sun::star::uno::Exception
                if the node cannot be retrieved.
                The exact exception being thrown may depend on the underlying backend.

        */
        ResultHolder< ComponentInstance > getComponentData(ComponentRequest const & _aRequest,
                                                 bool _bAddListenter)
            SAL_THROW((com::sun::star::uno::Exception));

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
        ResultHolder< NodeInstance > getDefaultData(NodeRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception));

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
        virtual ResultHolder< TemplateInstance > getTemplateData(TemplateRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception));
        //INodeDataListener Implementation
        /** Triggered when component data is changed

            @param _aRequest
                identifies the data that changed
        */
        virtual void dataChanged(const ComponentRequest& _aRequest) SAL_THROW(());
    protected:
        // ref counted, that's why no public dtor
        ~CacheController();
    // implementation
    private:
        configuration::AbsolutePath encodeTemplateLocation(rtl::OUString const & _rName, rtl::OUString const & _rModule) const;

        configuration::AbsolutePath ensureTemplate(rtl::OUString const& _rName, rtl::OUString const& _rModule) SAL_THROW((com::sun::star::uno::Exception));

        // adjust a node result for locale, ...
        bool normalizeResult(std::auto_ptr<ISubtree> &  _aResult, RequestOptions const & _aOptions);

        // reads data from the backend directly
        ResultHolder< ComponentInstance > loadDirectly(ComponentRequest const & _aRequest, bool _bAddListenter )
            SAL_THROW((com::sun::star::uno::Exception));
        // reads default data from the backend directly
        ResultHolder< NodeInstance > loadDefaultsDirectly(NodeRequest const & _aRequest) SAL_THROW((com::sun::star::uno::Exception));
        // writes an update  to the backend directly
        void saveDirectly(UpdateRequest const & _anUpdate) SAL_THROW((com::sun::star::uno::Exception));

        // writes updates for a component to the backend directly
        void savePendingChanges(rtl::Reference<CacheLoadingAccess> const & _aCache, ComponentRequest const & _aComponent)
            SAL_THROW((com::sun::star::uno::Exception));
        // saves all pending changes from a cache access to the backend
        bool saveAllPendingChanges(rtl::Reference<CacheLoadingAccess> const & _aCache, RequestOptions const & _aOptions)
            SAL_THROW((com::sun::star::uno::RuntimeException));
        // load templates componentwise from backend
        std::auto_ptr<ISubtree> loadTemplateData(TemplateRequest const & _aRequest)
            SAL_THROW((com::sun::star::uno::Exception));


        void flushCacheWriter() SAL_THROW(());
        // add templates componentwise to cache
        sharable::TreeFragment * addTemplates ( backend::ComponentDataStruct const & _aComponentInstance );
        rtl::Reference<CacheLoadingAccess> getCacheAlways(RequestOptions const & _aOptions);

        OTreeDisposeScheduler   * createDisposer(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext);
        OCacheWriteScheduler    * createCacheWriter(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext);



        // disposing
        void disposeAll(bool _bFlushRemainingUpdates);

        void closeModules(std::vector< rtl::Reference<CacheLine> > & _aList, RequestOptions const & _aOptions);
    private:
        typedef AutoReferenceMap<RequestOptions,CacheLoadingAccess,lessRequestOptions>   CacheMap;

        CacheChangeMulticaster  m_aNotifier;
        rtl::Reference< backend::IMergedDataProvider >      m_xBackend;
        CacheMap        m_aCacheMap;
        TemplateCacheData   m_aTemplates;

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

