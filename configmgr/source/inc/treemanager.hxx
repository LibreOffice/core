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

#include "treeprovider.hxx"
#include "defaultprovider.hxx"
#include "cacheddataprovider.hxx"
#include "confevents.hxx"
#include "options.hxx"
#include "autoreferencemap.hxx"
#include "autoobject.hxx"

namespace configmgr
{
    namespace uno       = ::com::sun::star::uno;

    //==========================================================================
    //= TreeManager
    //==========================================================================
    class CacheClientAccess;
    class CacheData;
    //==========================================================================
    class TreeManager   : public IConfigTreeManager
                        , public IConfigDefaultProvider
                        , public IDefaultableTreeManager
                        , private backend::ICachedDataListener
                        , private ConfigChangeBroadcaster
    {
        typedef CacheClientAccess       Cache;
        typedef rtl::Reference<Cache>   CacheRef;

        typedef AutoReferenceMap< RequestOptions, Cache, lessRequestOptions>    CacheList;
        typedef AutoObject< CacheData >                                         TemplateCache;

        typedef backend::ICachedDataProvider BackendCache;
        typedef rtl::Reference< BackendCache > BackendCacheRef;

        osl::Mutex          m_aCacheControllerMutex;
        BackendCacheRef     m_xCacheController;

        BackendCacheRef maybeGetBackendCache() CFG_NOTHROW( );
        BackendCacheRef getCacheLoader() CFG_UNO_THROW_RTE( );
        void disposeBackendCache() CFG_NOTHROW( );

        CacheList           m_aCacheList; // Map
        TemplateCache       m_aTemplates;
        sal_Bool            m_bEnableAsync;

    protected:
        // ref counted, that's why no public dtor
        ~TreeManager();

    public:
        /** ctor
        */
        explicit
        TreeManager(BackendCacheRef const & _xBackend);

        // disposing the cache before destroying
        void dispose();

        // ITreeManager
        /** requests a node given by it's path. Basicly, this means
            that the node is fetch from the cache when it contains it else it ask the server
            system into it's cache.
            @param          _rSubtreePath   the path to the node in URI notation.
            @param          _nMinLevels     indicates how many tree levels under the node determined by <arg>_rSubtreePath</arg>
                                            should be loaded
        */
        virtual data::NodeAccess requestSubtree(AbsolutePath const& _rSubtreePath,
                                                RequestOptions const& _aOptions
                                               ) CFG_UNO_THROW_ALL(  );

        virtual void updateTree(TreeChangeList& aChanges) CFG_UNO_THROW_ALL(  );

        virtual void saveAndNotifyUpdate(TreeChangeList const& aChanges) CFG_UNO_THROW_ALL(  );

        virtual void releaseSubtree(AbsolutePath const& aSubtreePath,
                                    RequestOptions const& _aOptions ) CFG_NOTHROW();

        virtual void disposeData(const RequestOptions& _aOptions) CFG_NOTHROW();

        virtual void fetchSubtree(  AbsolutePath const& aSubtreePath,
                                    RequestOptions const& _xOptions
                                 ) CFG_NOTHROW();

        virtual void refreshAll() CFG_UNO_THROW_ALL(  );
        virtual void flushAll() CFG_NOTHROW();
        virtual void enableAsync(const sal_Bool& bEnableAsync) CFG_NOTHROW() ;

        // IDefaultableTreeManager
        virtual sal_Bool fetchDefaultData(  AbsolutePath const& aSubtreePath,
                                            RequestOptions const& _aOptions
                                         ) CFG_UNO_THROW_ALL(  );

        // IDefaultProvider
        virtual std::auto_ptr<ISubtree> requestDefaultData(AbsolutePath const& aSubtreePath,
                                                            const RequestOptions& _aOptions
                                                          ) CFG_UNO_THROW_ALL(  );
        // ITemplateManager
        virtual data::TreeAccessor requestTemplate( Name const& aName, Name const& aModule
                                                  ) CFG_UNO_THROW_ALL(  );

        IConfigBroadcaster* getBroadcaster() { return this; }


    // implementation interfaces
        void refreshSubtree(const AbsolutePath &_aAbsoluteSubtreePath,
                            const RequestOptions& _aOptions) CFG_UNO_THROW_ALL(  );



    private:
        CacheData & getTemplates() { return * m_aTemplates.get(); }

        AbsolutePath encodeTemplateLocation(const Name& _rLogicalTemplateName, const Name &_rModule);

        void fireChanges(TreeChangeList const& aChangeTree, sal_Bool _bError);

    private:
        CacheRef getCacheAlways(RequestOptions const & _aOptions);

        // disposing
        void disposeAll();
        void disposeOne(RequestOptions const & _aOptions);
        void disposeUser(RequestOptions const & _aUserOptions);
        void implDisposeOne(CacheRef const & _aCache, RequestOptions const & _aOptions);

        // ConfigChangeBroadcaster
        virtual ConfigChangeBroadcastHelper* getBroadcastHelper(RequestOptions const& _aOptions, bool bCreate);

        // former INotifyListener
        void nodeUpdated(TreeChangeList& _rChanges);

        // ICachedDataListener
        virtual void disposing(backend::ICachedDataProvider & _rProvider)   CFG_NOTHROW();
        virtual void componentCreated(backend::ComponentRequest const & _aComponent) CFG_NOTHROW();
        virtual void componentChanged(backend::UpdateRequest  const & _anUpdate)     CFG_NOTHROW();
    };

}


#endif

