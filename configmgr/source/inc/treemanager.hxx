/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treemanager.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:25:53 $
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

#ifndef CONFIGMGR_TREEMANAGER_HXX_
#define CONFIGMGR_TREEMANAGER_HXX_

#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif
#ifndef CONFIGMGR_DEFAULTPROVIDER_HXX
#include "defaultprovider.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_CACHEDDATAPROVIDER_HXX
#include "cacheddataprovider.hxx"
#endif

#ifndef CONFIGMGR_API_EVENTS_HXX_
#include "confevents.hxx"
#endif
#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif

#ifndef CONFIGMGR_AUTOREFERENCEMAP_HXX
#include "autoreferencemap.hxx"
#endif
#ifndef CONFIGMGR_AUTOOBJECT_HXX
#include "autoobject.hxx"
#endif

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

