/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treemanager.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_TREEMANAGER_HXX_
#define CONFIGMGR_TREEMANAGER_HXX_

#include <memory>

#include "salhelper/simplereferenceobject.hxx"

#include "defaultprovider.hxx"
#include "confevents.hxx"
#include "options.hxx"
#include "autoreferencemap.hxx"
#include "autoobject.hxx"

namespace configmgr
{
    namespace uno       = ::com::sun::star::uno;

    namespace backend {
        class CacheController;
        class ComponentRequest;
        class UpdateRequest;
    }
    namespace sharable {
        struct TreeFragment;
        union Node;
    }

    //==========================================================================
    //= TreeManager
    //==========================================================================
    class CacheClientAccess;
    class CacheData;
    class ConfigChangeBroadcastHelper;
    class ISubtree;
    //==========================================================================
    class TreeManager: public salhelper::SimpleReferenceObject {
        typedef AutoReferenceMap< RequestOptions, CacheClientAccess, lessRequestOptions>    CacheList;

        osl::Mutex          m_aCacheControllerMutex;
        rtl::Reference< backend::CacheController >     m_xCacheController;

        rtl::Reference< backend::CacheController > maybeGetBackendCache() SAL_THROW(());
        rtl::Reference< backend::CacheController > getCacheLoader() SAL_THROW((com::sun::star::uno::RuntimeException));
        void disposeBackendCache() SAL_THROW(());

        CacheList           m_aCacheList; // Map
        AutoObject< CacheData >     m_aTemplates;
        sal_Bool            m_bEnableAsync;

        virtual ~TreeManager();

    public:
        /** ctor
        */
        explicit
        TreeManager(rtl::Reference< backend::CacheController > const & _xBackend);

        // disposing the cache before destroying
        void dispose();

        /** requests a node given by it's path. Basicly, this means
            that the node is fetch from the cache when it contains it else it ask the server
            system into it's cache.
            @param          _rSubtreePath   the path to the node in URI notation.
            @param          _nMinLevels     indicates how many tree levels under the node determined by <arg>_rSubtreePath</arg>
                                            should be loaded
        */
        sharable::Node * requestSubtree(configuration::AbsolutePath const& _rSubtreePath,
                                                RequestOptions const& _aOptions
                                               ) SAL_THROW((com::sun::star::uno::Exception));

        void updateTree(TreeChangeList& aChanges) SAL_THROW((com::sun::star::uno::Exception));

        void saveAndNotifyUpdate(TreeChangeList const& aChanges) SAL_THROW((com::sun::star::uno::Exception));

        void releaseSubtree(configuration::AbsolutePath const& aSubtreePath,
                                    RequestOptions const& _aOptions ) SAL_THROW(());

        void fetchSubtree(  configuration::AbsolutePath const& aSubtreePath,
                                    RequestOptions const& _xOptions
                                 ) SAL_THROW(());

        void refreshAll() SAL_THROW((com::sun::star::uno::Exception));
        void flushAll() SAL_THROW(());
        void enableAsync(const sal_Bool& bEnableAsync) SAL_THROW(()) ;

        sal_Bool fetchDefaultData(  configuration::AbsolutePath const& aSubtreePath,
                                            RequestOptions const& _aOptions
                                         ) SAL_THROW((com::sun::star::uno::Exception));

        std::auto_ptr<ISubtree> requestDefaultData(configuration::AbsolutePath const& aSubtreePath,
                                                            const RequestOptions& _aOptions
                                                          ) SAL_THROW((com::sun::star::uno::Exception));

        sharable::TreeFragment * requestTemplate( rtl::OUString const& aName, rtl::OUString const& aModule
                                                  ) SAL_THROW((com::sun::star::uno::Exception));

    // implementation interfaces
        void refreshSubtree(const configuration::AbsolutePath &_aAbsoluteSubtreePath,
                            const RequestOptions& _aOptions) SAL_THROW((com::sun::star::uno::Exception));

        void addListener(configuration::AbsolutePath const& aName, const RequestOptions& _aOptions, rtl::Reference<INodeListener> const& pListener);
        void removeListener(const RequestOptions& _aOptions, rtl::Reference<INodeListener> const& pListener);

        void componentCreated(backend::ComponentRequest const & _aComponent) SAL_THROW(());
        void componentChanged(backend::UpdateRequest  const & _anUpdate)     SAL_THROW(());

    private:
        CacheData & getTemplates() { return * m_aTemplates.get(); }

        configuration::AbsolutePath encodeTemplateLocation(rtl::OUString const & _rLogicalTemplateName, rtl::OUString const &_rModule);

    private:
        rtl::Reference<CacheClientAccess> getCacheAlways(RequestOptions const & _aOptions);

        // disposing
        void disposeAll();

        void fireChanges(TreeChangeList const& _aChanges, sal_Bool _bError);
        ConfigChangeBroadcastHelper* getBroadcastHelper(RequestOptions const& _aOptions, bool bCreate);
        void disposeBroadcastHelper(ConfigChangeBroadcastHelper* pHelper);

        void nodeUpdated(TreeChangeList& _rChanges);
    };

}


#endif

