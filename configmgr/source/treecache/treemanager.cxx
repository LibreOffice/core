/*************************************************************************
 *
 *  $RCSfile: treemanager.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:43 $
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

#include "treemanager.hxx"

#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#include "mergeddataprovider.hxx"
#endif
#ifndef CONFIGMGR_CACHEACCESS_HXX
#include "cacheaccess.hxx"
#endif

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

namespace configmgr
{

    namespace uno = ::com::sun::star::uno;
    namespace lang= ::com::sun::star::lang;

    namespace Path = configuration::Path;
    using configuration::Name;
    using configuration::AbsolutePath;
// =========================================================================
static void test_complete(memory::HeapManager & _rDummy)
{ new TreeManager(NULL,_rDummy); }
// =========================================================================

#define MAKEUSTRING( char_array ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( char_array ) )
// =========================================================================

static inline
AbsolutePath extractModulePath(AbsolutePath const & _aPath)
{
    if (_aPath.getDepth() <= 1) return _aPath;

    Name aModule = _aPath.getModuleName();

    return AbsolutePath::makeModulePath( aModule, AbsolutePath::NoValidate() );
}
// =========================================================================

// disposing
// -------------------------------------------------------------------------
void TreeManager::disposeAll()
{
    CFG_TRACE_INFO("TreeManager: Disposing all data" );
    CacheList::Map aReleaseList;

    {
        osl::MutexGuard aGuard(m_aCacheList.mutex());
        m_aCacheList.swap(aReleaseList);             // move data out of m_aCacheList
    }

    // free all the trees - not exception safe !! (i.e. disposeBroadcastHelper() must not throw)
    for (CacheList::Map::iterator i = aReleaseList.begin(); i != aReleaseList.end(); ++i)
    {
        if (ConfigChangeBroadcastHelper * pHelper = i->second->releaseBroadcaster())
            ConfigChangeBroadcaster::disposeBroadcastHelper(pHelper);
        i->second.clear();
    }
}

// -------------------------------------------------------------------------
void TreeManager::dispose()
{
    CFG_TRACE_INFO("TreeManager: dispoing the treemanager" );

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::TreeManager", "jb99855", "configmgr: TreeManager::dispose().");

    BackendCacheRef xBackendCache = maybeGetBackendCache();

    if (xBackendCache.is()) xBackendCache->getNotifier().removeListener(this);

    // cleaning the cache
    disposeAll();

    disposeBackendCache();
}

// -------------------------------------------------------------------------
void TreeManager::disposeOne(RequestOptions const& _aOptions)
{
    CFG_TRACE_INFO("TreeManager: Disposing data and TreeInfo for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

    CacheRef aRemoved = m_aCacheList.remove(_aOptions);

    if (aRemoved.is())
    {
        // got it out of reachability - now dispose/notify without lock
        implDisposeOne(aRemoved, _aOptions);
    }
    else
        CFG_TRACE_INFO_NI("- No affected TreeInfo found" );
}

// -------------------------------------------------------------------------
void TreeManager::disposeUser(RequestOptions const& _aUserOptions)
{
    osl::ClearableMutexGuard aGuard(m_aCacheList.mutex());

    CFG_TRACE_INFO("TreeManager: Disposing data and TreeInfo(s) for user '%s'",
                    OUSTRING2ASCII(_aUserOptions.getEntity()) );

    typedef std::vector< std::pair< RequestOptions, CacheRef > > DisposeList;

    DisposeList aDisposeList;
    // collect the ones to dispose
    {
        OUString sUser = _aUserOptions.getEntity();
        OSL_ASSERT(sUser.getLength());

        // This depends on the fact that Options are sorted (by struct ltOptions)
        // so that all options belonging to one user are together
        // (and that options with only a user set, sort first)

        CacheList::Map aCacheData;
        m_aCacheList.swap(aCacheData);

        // find the lower_bound of all options for the user
        CacheList::Map::iterator const aFirst = aCacheData.lower_bound(_aUserOptions);

        // find the upper_bound of all options for the user (using the lower one)
        CacheList::Map::iterator aLast = aFirst;
        while (aLast != aCacheData.end() && aLast->first.getEntity() == sUser)
        {
            CFG_TRACE_INFO_NI("- Found TreeInfo for locale '%s'", OUSTRING2ASCII(aLast->first.getLocale()) );
            OSL_ASSERT(aLast->second.is());

            if (aLast->second.is())
                aDisposeList.push_back( DisposeList::value_type(aLast->first,aLast->second) );

            ++aLast;
        }

        if (aFirst == aLast)
            CFG_TRACE_INFO_NI("- No affected TreeInfo found" );

        aCacheData.erase(aFirst, aLast);

        // replace the data into the map
        m_aCacheList.swap(aCacheData);
    }

    // got all out of external reach - now dispose/notify without lock
    aGuard.clear();

    for (DisposeList::iterator i = aDisposeList.begin(); i != aDisposeList.end(); ++i)
    {
        if (i->second.is())
            implDisposeOne(i->second, i->first);
    }
}

// -------------------------------------------------------------------------
void TreeManager::implDisposeOne(CacheRef const & _aDisposedCache, RequestOptions const & _aOptions)
{
    OSL_ASSERT(_aDisposedCache.is());
    CFG_TRACE_INFO("Now removing TreeInfo (user '%s' with locale '%s') and broadcaster",
                    OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

    // to do: release at the backend (?)
    if (ConfigChangeBroadcastHelper * pHelper = _aDisposedCache->releaseBroadcaster())
        ConfigChangeBroadcaster::disposeBroadcastHelper(pHelper);
}

// ConfigChangeBroadcaster
// -------------------------------------------------------------------------
ConfigChangeBroadcastHelper* TreeManager::getBroadcastHelper(RequestOptions const& _aOptions, bool bCreate)
{
    CacheRef aCache =  bCreate  ? this->getCacheAlways(_aOptions)
                                : m_aCacheList.get(_aOptions);

    return aCache.is() ? aCache->getBroadcaster() : NULL;
}


// -------------------------------------------------------------------------
TreeManager::TreeManager(BackendCacheRef const & _xBackend, memory::HeapManager & _rCacheHeapManager)
: m_xCacheController(_xBackend)
, m_aCacheList()
, m_aTemplates(new CacheData(_rCacheHeapManager))
{
    OSL_PRECOND(_xBackend.is(),"Trying to create a TreeManager without a backend");

    if (m_xCacheController.is()) m_xCacheController->getNotifier().addListener(this);
}

// -------------------------------------------------------------------------
TreeManager::~TreeManager()
{
}

// -------------------------------------------------------------------------
TreeManager::BackendCacheRef TreeManager::maybeGetBackendCache() CFG_NOTHROW( )
{
    osl::MutexGuard aGuard(m_aCacheControllerMutex);
    BackendCacheRef xResult(m_xCacheController);
    return xResult;
}

// -------------------------------------------------------------------------
TreeManager::BackendCacheRef TreeManager::getCacheLoader() CFG_UNO_THROW_RTE( )
{
    osl::MutexGuard aGuard(m_aCacheControllerMutex);
    if (!m_xCacheController.is())
    {
        OUString sMsg = OUString::createFromAscii("TreeManager: No backend available - tree manager was already disposed.");
        throw com::sun::star::lang::DisposedException(sMsg,NULL);
    }
    BackendCacheRef xResult(m_xCacheController);
    return xResult;
}

// -------------------------------------------------------------------------
void TreeManager::disposeBackendCache() CFG_NOTHROW( )
{
    osl::ClearableMutexGuard aGuard(m_aCacheControllerMutex);
    if (m_xCacheController.is())
    {
        BackendCacheRef xBackendCache(m_xCacheController);
        m_xCacheController.clear();
        aGuard.clear();
        xBackendCache->dispose();
    }
}

// -------------------------------------------------------------------------

memory::HeapManager & TreeManager::getCacheHeapManager() const
{
    return m_aTemplates.get()->getHeapManager();
}
// -------------------------------------------------------------------------

TreeManager::CacheRef TreeManager::getCacheAlways(RequestOptions const & _aOptions)
{
    osl::MutexGuard aGuard( m_aCacheList.mutex() );

    CacheRef aResult = m_aCacheList.get(_aOptions);
    if (!aResult.is())
    {
        CacheRef aNewCache( new Cache(getCacheHeapManager(), ConfigChangeBroadcaster::newBroadcastHelper()) );
        aResult = m_aCacheList.insert(_aOptions,aNewCache);
    }
    return aResult;
}

// -------------------------------------------------------------------------
memory::Segment* TreeManager::getDataSegment(AbsolutePath const& _rAccessor,
                                             const RequestOptions& _aOptions)
{
    CacheRef aCache = m_aCacheList.get(_aOptions);

    OSL_ENSURE(aCache.is(), "No cache data to get segment for");

    if (aCache.is())
        return aCache->getDataSegment(_rAccessor);

    else
        return NULL;
}
// -------------------------------------------------------------------------

data::NodeAccess TreeManager::requestSubtree(AbsolutePath const& aSubtreePath,
                                                const RequestOptions& _aOptions)
                                             CFG_UNO_THROW_ALL(  )
{
    CFG_TRACE_INFO("TreeManager: request for subtree '%s'", OUSTRING2ASCII(aSubtreePath.toString()));

    CacheRef aCache = getCacheAlways(_aOptions);
    OSL_ENSURE(aCache.is(),"TreeManager: Cannot create cache access for loading node");

    data::Accessor aAccessor(NULL);

    if (!aCache->hasModule(aSubtreePath))
    {
        CFG_TRACE_INFO_NI("TreeManager: cache miss. going to load the node");
        backend::ComponentRequest aQuery( aSubtreePath.getModuleName(), _aOptions );

        backend::CacheLocation aLoadedLocation = getCacheLoader()->loadComponent(aQuery);
        if (aLoadedLocation.isNull())
        {
            CFG_TRACE_WARNING_NI("TreeManager: requested component not found");
            throw com::sun::star::container::
                    NoSuchElementException( MAKEUSTRING("Requested component not found"), NULL);
        }

        CFG_TRACE_INFO_NI("TreeManager: attaching loaded cache segment ");

        aAccessor = data::Accessor(aCache->attachDataSegment(aLoadedLocation.segment,aSubtreePath));
        OSL_ENSURE(aAccessor.is(),"Cannot attach to loaded component");
    }
    else
    {
        CFG_TRACE_INFO_NI("TreeManager: found node in cache");

        aAccessor = data::Accessor(aCache->getDataSegment(aSubtreePath));
        OSL_ENSURE(aAccessor.is(),"Cannot get accessor for existing component");
    }

    data::NodeAddress aResultAddress = aCache->acquireNode(aAccessor,aSubtreePath);

    return data::NodeAccess(aAccessor,aResultAddress);
}

// -------------------------------------------------------------------------
void TreeManager::fetchSubtree(AbsolutePath const& aSubtreePath, const RequestOptions&  ) CFG_NOTHROW()
{
    CFG_TRACE_WARNING("TreeManager: Prefetching not implemented. (Request to prefetch component %s.", OUSTRING2ASCII(aSubtreePath.toString()));
}

// -------------------------------------------------------------------------
sal_Bool TreeManager::fetchDefaultData( memory::UpdateAccessor& _aAccessToken,
                                        AbsolutePath const& aSubtreePath,
                                        const RequestOptions& _aOptions
                                      ) CFG_UNO_THROW_ALL(  )
{
    CFG_TRACE_INFO("tree manager: checking the cache for defaults");

    CacheRef aCache = m_aCacheList.get(_aOptions);

    if (!aCache.is())
    {
        OSL_ENSURE(aCache.is(),"TreeManager: Cache access to fetch defaults for does not exist ! Where does the node access come from ?");
        return false;
    }

    if (aCache->hasModuleDefaults(_aAccessToken.accessor(),aSubtreePath))
    {
        CFG_TRACE_INFO_NI("TreeManager: found default data in cache");
        return true;
    }

    AbsolutePath aRequestPath = extractModulePath(aSubtreePath);

    backend::NodeRequest aRequest(aRequestPath,_aOptions);

    backend::NodeResult aDefaults = getCacheLoader()->getDirectDataProvider().getDefaultData( aRequest );

    if (!aDefaults.is())
    {
        CFG_TRACE_INFO_NI("TreeManager: merging loaded defaults into cache");
        return aCache->insertDefaults(_aAccessToken,aDefaults.instance());
    }
    else
    {
        CFG_TRACE_WARNING_NI("TreeManager: cannot load defaults: no data available or not supported");
        return false;
    }
}

// -------------------------------------------------------------------------
std::auto_ptr<ISubtree> TreeManager::requestDefaultData(AbsolutePath const& aSubtreePath,
                                                        const RequestOptions& _aOptions
                                                       ) CFG_UNO_THROW_ALL(  )
{
    // to do: check cache for existing default data (?!)
    CFG_TRACE_INFO_NI("TreeManager: loading default data directly");

    backend::NodeRequest aRequest(aSubtreePath,_aOptions);

    backend::NodeResult aDefaults = getCacheLoader()->getDirectDataProvider().getDefaultData( aRequest );

    return aDefaults.extractDataAndClear();
}

// -------------------------------------------------------------------------
AbsolutePath TreeManager::encodeTemplateLocation(const Name& _rLogicalTemplateName, const Name &_rModule)
{
    using namespace configuration::Path;

//  static const
//  Component aTemplateRoot = wrapSimpleName(OUString::createFromAscii("org.openoffice.Templates"));

    Component aTemplateModule = wrapSimpleName(_rModule);
    Component aTemplateName   = wrapSimpleName(_rLogicalTemplateName);

    Path::Rep aResult(aTemplateName);
    aResult.prepend(aTemplateModule);
//    aResult.prepend(aTemplateRoot);

    return AbsolutePath(aResult);
}

// -------------------------------------------------------------------------
data::TreeAccessor TreeManager::requestTemplate(data::Accessor const& /*_aSourceAccessor*/,
                                                Name const& _rName, Name const& _rModule
                                                ) CFG_UNO_THROW_ALL(  )
{
    OSL_ENSURE(!_rName.isEmpty(), "TreeManager::requestTemplate : invalid template name !");

    CFG_TRACE_INFO("TreeManager: going to get a template named %s", OUSTRING2ASCII(_rName.toString()));

    AbsolutePath aTemplateLocation = encodeTemplateLocation(_rName, _rModule);
    Name aCacheModule = aTemplateLocation.getModuleName();

    osl::MutexGuard aGuard(m_aTemplates.mutex());

    data::Accessor aTemplatesAccessor( getTemplates().getDataSegment(aCacheModule) );

    if (!getTemplates().hasNode(aTemplatesAccessor,aTemplateLocation))
    {
        aTemplatesAccessor.clear();

        CFG_TRACE_INFO_NI("TreeManager: cache miss. going to load the template");
        backend::TemplateRequest aQuery( _rName, _rModule );

        backend::CacheLocation aLoadedLocation = getCacheLoader()->loadTemplate(aQuery);
        if (aLoadedLocation.isNull())
        {
            CFG_TRACE_ERROR_NI("TreeManager: requested template module not found");
            throw com::sun::star::container::
                    NoSuchElementException( MAKEUSTRING("Requested template module not found"), NULL);
        }

        CFG_TRACE_INFO_NI("TreeManager: attaching to loaded template module");

        aTemplatesAccessor = data::Accessor(getTemplates().attachDataSegment(aLoadedLocation.segment,aCacheModule));
        OSL_ENSURE(aTemplatesAccessor.is(),"Cannot attach to loaded component");

        // create a client ref count on the template module
        getTemplates().acquireNode(aTemplatesAccessor,aTemplateLocation);
    }
    else
    {
        CFG_TRACE_INFO_NI("TreeManager: template module found in cache");
        OSL_ENSURE(aTemplatesAccessor.is(),"No accessor for existing data ?");
    }

    data::TreeAddress aTemplateAddr = getTemplates().getTemplateTree(aTemplatesAccessor,aTemplateLocation);
    if (aTemplateAddr.isNull())
    {
        CFG_TRACE_ERROR_NI("TreeManager: template not found in module");
        throw com::sun::star::container::
                    NoSuchElementException( MAKEUSTRING("Unknown template. Type description could not be found in the given module."), NULL);
    }
    return data::TreeAccessor(aTemplatesAccessor,aTemplateAddr);
}

// -------------------------------------------------------------------------
void TreeManager::saveAndNotifyUpdate(data::Accessor const& _aChangedDataAccessor, TreeChangeList const& aChangeTree) CFG_UNO_THROW_ALL(  )
{
    {
        CFG_TRACE_INFO("TreeManager: committing an Update to the cache controller");
        backend::UpdateRequest anUpdate(
                                & aChangeTree.root,
                                aChangeTree.getRootNodePath(),
                                aChangeTree.getOptions());

        getCacheLoader()->saveAndNotify(anUpdate);
        CFG_TRACE_INFO_NI("TreeManager: committing done");
    }

    // notification should be done via the cache loader
    // this->ConfigChangeBroadcaster::fireChanges(_aChangedDataAccessor, aChangeTree, false);
}

// -------------------------------------------------------------------------
void TreeManager::fireChanges(data::Accessor const& _aChangedDataAccessor, TreeChangeList const& aChangeTree, sal_Bool _bError)
{
    CFG_TRACE_INFO("TreeManager: broadcasting changes");
    ConfigChangeBroadcaster::fireChanges(_aChangedDataAccessor, aChangeTree, _bError);
}

// -----------------------------------------------------------------------------
void TreeManager::updateTree(memory::UpdateAccessor& _aAccessToken, TreeChangeList& _aChanges) CFG_UNO_THROW_ALL(  )
{
    CFG_TRACE_INFO("TreeManager: updating the cache from a changes list");

    backend::UpdateInstance anUpdate(&_aChanges.root,_aChanges.getRootNodePath());

    CacheRef aCache = m_aCacheList.get(_aChanges.getOptions());

    if (!aCache.is())
    {
        CFG_TRACE_ERROR_NI("TreeManager: Cache access to update into does not exist !");
        OSL_ENSURE(aCache.is(),"TreeManager: Cache access to update into does not exist ! Where does the update access come from ?");
        throw lang::DisposedException(OUString::createFromAscii("Tree to be updated was already disposed"), NULL);
    }

    // merge the changes into the tree
    aCache->applyUpdate(_aAccessToken, anUpdate);

    CFG_TRACE_INFO_NI("TreeManager: cache update done");
}

// -----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void TreeManager::releaseSubtree( AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions ) CFG_NOTHROW()
{
    CFG_TRACE_INFO("TreeManager: releasing subtree '%s' for entity '%s' with locale '%s'", OUSTRING2ASCII(aSubtreePath.toString()), OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

    CacheRef aCache = m_aCacheList.get(_aOptions);

    OSL_ENSURE(aCache.is(),"TreeManager: No local data to release");

    if (aCache.is())
    {
        CFG_TRACE_INFO_NI("TreeManager: decrementing refcount for subtree '%s'", OUSTRING2ASCII(aSubtreePath.toString()) );
        if (aCache->releaseNode(aSubtreePath) == 0)
        {
            backend::ComponentRequest aComponentDesc(aSubtreePath.getModuleName(),_aOptions);
            BackendCacheRef xBackendCache = maybeGetBackendCache();
            if (xBackendCache.is()) xBackendCache->freeComponent(aComponentDesc);
        }
    }
}

//-----------------------------------------------------------------------------
void TreeManager::disposeData(const RequestOptions& _aOptions) CFG_NOTHROW()
{
    CFG_TRACE_INFO("TreeManager: disposing data by options");

    if (_aOptions.getLocale().getLength() != 0)
    {
        OSL_ENSURE(_aOptions.getEntity().getLength() != 0, "TreeManager: Cannot dispose locale without user");
        CFG_TRACE_INFO_NI( "TreeManager: Disposing data for options: ENTITY='%s' and LOCALE = '%s'",
                                OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

        this->disposeOne( _aOptions );
    }
    else if (_aOptions.getEntity().getLength() != 0)
    {
        CFG_TRACE_INFO_NI( "TreeManager: Disposing data for user: '%s'", OUSTRING2ASCII(_aOptions.getEntity()) );

        this->disposeUser(_aOptions );
    }
    else
    {
        OSL_ENSURE(false, "TreeManager: Cannot dispose: neither user nor locale specified in options");
    }
    CFG_TRACE_INFO_NI("TreeManager: disposing data done");
}

// INotifyListener
// ----------------------------------------------------------------------------
void TreeManager::nodeUpdated(TreeChangeList& _rChanges)
{
    CFG_TRACE_INFO("TreeManager: nodeUpdated");
    try
    {
        CacheRef aCache = m_aCacheList.get(_rChanges.getOptions());

        if (aCache.is())
        {
            // first approve the changes and merge them with the current tree
            AbsolutePath aSubtreeName = _rChanges.getRootNodePath();

            memory::Accessor aAccessor( aCache->getDataSegment(aSubtreeName) );
            OSL_ENSURE(aAccessor.is(), "TreeManager::nodeUpdated : cannot access cache !");

            data::NodeAddress aCacheTree = aCache->findInnerNode(aAccessor,aSubtreeName);
            OSL_ENSURE(aCacheTree.is(), "TreeManager::nodeUpdated : node not found in cache!");

            if (aCacheTree.is())
                this->fireChanges(aAccessor,_rChanges,false);
        }
    }
    catch (uno::RuntimeException&)
    {
        CFG_TRACE_ERROR_NI("TreeManager::nodeUpdated : could not notify !");
    }
    CFG_TRACE_INFO_NI("TreeManager: nodeUpdated done");
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ICachedDataListener
// ----------------------------------------------------------------------------

void TreeManager::disposing(backend::ICachedDataProvider & _rProvider)   CFG_NOTHROW()
{
    CFG_TRACE_INFO("TreeManager: provider is being disposed");
    this->dispose();
}
// ----------------------------------------------------------------------------

void TreeManager::componentCreated(backend::ComponentRequest const & ) CFG_NOTHROW()
{
    CFG_TRACE_INFO("TreeManager: component was created");
}
// ----------------------------------------------------------------------------

void TreeManager::componentChanged(backend::UpdateRequest  const & _anUpdate)     CFG_NOTHROW()
{
    TreeChangeList aChanges(_anUpdate.getOptions(),
                            _anUpdate.getUpdateRoot().location(),
                            *_anUpdate.getUpdateData(),
                            SubtreeChange::DeepChildCopy() );

    this->nodeUpdated(aChanges);
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
} // namespace
