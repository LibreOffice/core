/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treemanager.cxx,v $
 * $Revision: 1.14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "treemanager.hxx"
#include "mergeddataprovider.hxx"
#include "cacheaccess.hxx"
#include "cachecontroller.hxx"
#include "cachemulticaster.hxx"
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include "tracer.hxx"
#include <osl/diagnose.h>
#include <rtl/logfile.hxx>

namespace configmgr
{

    namespace uno = ::com::sun::star::uno;
    namespace lang= ::com::sun::star::lang;

    namespace Path = configuration::Path;
// =========================================================================
//#if OSL_DEBUG_LEVEL > 0
#if 0 // currently not used in debug build!
static void test_complete(memory::HeapManager & _rDummy)
{ new TreeManager(NULL,_rDummy); }
#endif
// =========================================================================

#define MAKEUSTRING( char_array ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( char_array ) )
// =========================================================================

static inline
configuration::AbsolutePath extractModulePath(configuration::AbsolutePath const & _aPath)
{
    if (_aPath.getDepth() <= 1) return _aPath;

    rtl::OUString aModule = _aPath.getModuleName();

    return configuration::AbsolutePath::makeModulePath(aModule);
}
// =========================================================================

// disposing
// -------------------------------------------------------------------------
void TreeManager::disposeAll()
{
    CFG_TRACE_INFO("TreeManager: Disposing all data" );
    CacheList::Map aReleaseList;

    m_aCacheList.swap(aReleaseList);             // move data out of m_aCacheList

    // free all the trees - not exception safe !! (i.e. disposeBroadcastHelper() must not throw)
    for (CacheList::Map::iterator i = aReleaseList.begin(); i != aReleaseList.end(); ++i)
    {
        if (ConfigChangeBroadcastHelper * pHelper = i->second->releaseBroadcaster())
            disposeBroadcastHelper(pHelper);
        i->second.clear();
    }
}

// -------------------------------------------------------------------------
void TreeManager::dispose()
{
    CFG_TRACE_INFO("TreeManager: dispoing the treemanager" );

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::TreeManager", "jb99855", "configmgr: TreeManager::dispose().");

    rtl::Reference< backend::CacheController > xBackendCache = maybeGetBackendCache();

    if (xBackendCache.is()) xBackendCache->getNotifier().removeListener(this);

    // cleaning the cache
    disposeAll();

    disposeBackendCache();
}

// -------------------------------------------------------------------------
ConfigChangeBroadcastHelper* TreeManager::getBroadcastHelper(RequestOptions const& _aOptions, bool bCreate)
{
    rtl::Reference<CacheClientAccess> aCache =  bCreate  ? this->getCacheAlways(_aOptions)
                                : m_aCacheList.get(_aOptions);

    return aCache.is() ? aCache->getBroadcaster() : NULL;
}


// -------------------------------------------------------------------------
TreeManager::TreeManager(rtl::Reference< backend::CacheController > const & _xBackend)
: m_xCacheController(_xBackend)
, m_aCacheList()
, m_aTemplates(new CacheData())
, m_bEnableAsync(true)
{
    OSL_PRECOND(_xBackend.is(),"Trying to create a TreeManager without a backend");

    if (m_xCacheController.is()) m_xCacheController->getNotifier().addListener(this);
}

// -------------------------------------------------------------------------
TreeManager::~TreeManager()
{
}

// -------------------------------------------------------------------------
rtl::Reference< backend::CacheController > TreeManager::maybeGetBackendCache() SAL_THROW(())
{
    osl::MutexGuard aGuard(m_aCacheControllerMutex);
    rtl::Reference< backend::CacheController > xResult(m_xCacheController);
    return xResult;
}

// -------------------------------------------------------------------------
rtl::Reference< backend::CacheController > TreeManager::getCacheLoader() SAL_THROW((com::sun::star::uno::RuntimeException))
{
    osl::MutexGuard aGuard(m_aCacheControllerMutex);
    if (!m_xCacheController.is())
    {
        rtl::OUString sMsg = rtl::OUString::createFromAscii("TreeManager: No backend available - tree manager was already disposed.");
        throw com::sun::star::lang::DisposedException(sMsg,NULL);
    }
    rtl::Reference< backend::CacheController > xResult(m_xCacheController);
    return xResult;
}

// -------------------------------------------------------------------------
void TreeManager::disposeBackendCache() SAL_THROW(())
{
    osl::ClearableMutexGuard aGuard(m_aCacheControllerMutex);
    if (m_xCacheController.is())
    {
        rtl::Reference< backend::CacheController > xBackendCache(m_xCacheController);
        m_xCacheController.clear();
        aGuard.clear();
        xBackendCache->dispose();
    }
}

// -------------------------------------------------------------------------

rtl::Reference<CacheClientAccess> TreeManager::getCacheAlways(RequestOptions const & _aOptions)
{
    rtl::Reference<CacheClientAccess> aResult = m_aCacheList.get(_aOptions);
    if (!aResult.is())
    {
        rtl::Reference<CacheClientAccess> aNewCache( new CacheClientAccess(new ConfigChangeBroadcastHelper()) );
        aResult = m_aCacheList.insert(_aOptions,aNewCache);
    }
    return aResult;
}

// -------------------------------------------------------------------------

sharable::Node * TreeManager::requestSubtree(configuration::AbsolutePath const& aSubtreePath,
                         const RequestOptions& _aOptions)
                        SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("TreeManager: request for subtree '%s'", OUSTRING2ASCII(aSubtreePath.toString()));

    rtl::Reference<CacheClientAccess> aCache = getCacheAlways(_aOptions);
    OSL_ENSURE(aCache.is(),"TreeManager: Cannot create cache access for loading node");

    if (!aCache->hasModule(aSubtreePath))
    {
    CFG_TRACE_INFO_NI("TreeManager: cache miss. going to load the node");
        backend::ComponentRequest aQuery( aSubtreePath.getModuleName(), _aOptions );

        sharable::TreeFragment * aLoadedLocation = getCacheLoader()->loadComponent(aQuery);
        if (aLoadedLocation == NULL)
        {
            CFG_TRACE_WARNING_NI("TreeManager: requested component not found");
            throw com::sun::star::container::
                    NoSuchElementException( MAKEUSTRING("Requested component not found"), NULL);
        }

    CFG_TRACE_INFO_NI("TreeManager: attaching loaded cache segment ");
        aCache->attachModule(aLoadedLocation,aSubtreePath.getModuleName());
    }
    else
    {
    CFG_TRACE_INFO_NI("TreeManager: found node in cache");
        if (_aOptions.isRefreshEnabled())
        {
             backend::ComponentRequest aRequest( aSubtreePath.getModuleName(), _aOptions );
             getCacheLoader()->refreshComponent(aRequest);
        }
    }

    return aCache->acquireNode(aSubtreePath);
}

// -------------------------------------------------------------------------
void TreeManager::fetchSubtree(configuration::AbsolutePath const& aSubtreePath, const RequestOptions&  ) SAL_THROW(())
{
    (void) aSubtreePath; // avoid warning about unused parameter
    CFG_TRACE_WARNING("TreeManager: Prefetching not implemented. (Request to prefetch component %s.", OUSTRING2ASCII(aSubtreePath.toString()));
}

// -------------------------------------------------------------------------
sal_Bool TreeManager::fetchDefaultData( configuration::AbsolutePath const& aSubtreePath,
                                        const RequestOptions& _aOptions
    ) SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("tree manager: checking the cache for defaults");

    rtl::Reference<CacheClientAccess> aCache = m_aCacheList.get(_aOptions);

    if (!aCache.is())
    {
    OSL_ENSURE(aCache.is(),"TreeManager: Cache access to fetch defaults for does not exist ! Where does the node access come from ?");
        return false;
    }

    if (aCache->hasModuleDefaults(aSubtreePath))
    {
        CFG_TRACE_INFO_NI("TreeManager: found default data in cache");
        return true;
    }

    configuration::AbsolutePath aRequestPath = extractModulePath(aSubtreePath);

    backend::NodeRequest aRequest(aRequestPath,_aOptions);

    backend::ResultHolder< backend::NodeInstance > aDefaults = getCacheLoader()->getDefaultData( aRequest );

    if (!aDefaults.is())
    {
        CFG_TRACE_INFO_NI("TreeManager: merging loaded defaults into cache");
        return aCache->insertDefaults(aDefaults.instance());
    }
    else
    {
        CFG_TRACE_WARNING_NI("TreeManager: cannot load defaults: no data available or not supported");
        return false;
    }
}

// -------------------------------------------------------------------------
std::auto_ptr<ISubtree> TreeManager::requestDefaultData(configuration::AbsolutePath const& aSubtreePath,
                                                        const RequestOptions& _aOptions
                                                       ) SAL_THROW((com::sun::star::uno::Exception))
{
    // to do: check cache for existing default data (?!)
    CFG_TRACE_INFO_NI("TreeManager: loading default data directly");

    backend::NodeRequest aRequest(aSubtreePath,_aOptions);

    backend::ResultHolder< backend::NodeInstance > aDefaults = getCacheLoader()->getDefaultData( aRequest );

    return aDefaults.extractDataAndClear();
}

// -------------------------------------------------------------------------
configuration::AbsolutePath TreeManager::encodeTemplateLocation(const rtl::OUString& _rLogicalTemplateName, const rtl::OUString &_rModule)
{
//  static const
//  configuration::Path::Component aTemplateRoot = configuration::Path::wrapSimpleName(rtl::OUString::createFromAscii("org.openoffice.Templates"));

    configuration::Path::Component aTemplateModule = configuration::Path::wrapSimpleName(_rModule);
    configuration::Path::Component aTemplateName   = configuration::Path::wrapSimpleName(_rLogicalTemplateName);

    Path::Rep aResult(aTemplateName);
    aResult.prepend(aTemplateModule);
//    aResult.prepend(aTemplateRoot);

    return configuration::AbsolutePath(aResult);
}

// -------------------------------------------------------------------------
sharable::TreeFragment * TreeManager::requestTemplate(rtl::OUString const& _rName,
                                                rtl::OUString const& _rModule) SAL_THROW((com::sun::star::uno::Exception))
{
    OSL_ENSURE(_rName.getLength() != 0, "TreeManager::requestTemplate : invalid template name !");

    CFG_TRACE_INFO("TreeManager: going to get a template named %s", OUSTRING2ASCII(_rName));

    configuration::AbsolutePath aTemplateLocation = encodeTemplateLocation(_rName, _rModule);
    rtl::OUString aCacheModule = aTemplateLocation.getModuleName();

    if (!getTemplates().hasNode(aTemplateLocation))
    {
        CFG_TRACE_INFO_NI("TreeManager: cache miss. going to load the template");
        backend::TemplateRequest aQuery( _rName, _rModule );

        sharable::TreeFragment * aLoadedLocation = getCacheLoader()->loadTemplate(aQuery);
        if (aLoadedLocation == NULL)
        {
            CFG_TRACE_ERROR_NI("TreeManager: requested template module not found");
            throw com::sun::star::container::
                    NoSuchElementException( MAKEUSTRING("Requested template module not found"), NULL);
        }

        CFG_TRACE_INFO_NI("TreeManager: attaching to loaded template module");

        getTemplates().attachModule(aLoadedLocation,aCacheModule);

        // create a client ref count on the template module
        getTemplates().acquireNode(aTemplateLocation);
    }
    else
    {
        CFG_TRACE_INFO_NI("TreeManager: template module found in cache");
    }

    sharable::TreeFragment * aTemplateAddr = getTemplates().getTemplateTree(aTemplateLocation);
    if (aTemplateAddr == NULL)
    {
        CFG_TRACE_ERROR_NI("TreeManager: template not found in module");
        throw com::sun::star::container::
                    NoSuchElementException( MAKEUSTRING("Unknown template. Type description could not be found in the given module."), NULL);
    }
    return aTemplateAddr;
}

// -------------------------------------------------------------------------
void TreeManager::saveAndNotifyUpdate(TreeChangeList const& aChangeTree) SAL_THROW((com::sun::star::uno::Exception))
{
    {
        CFG_TRACE_INFO("TreeManager: committing an Update to the cache controller");
        RequestOptions aOptions = aChangeTree.getOptions();;
        //Modify RequestOptions - suppress async commit, if disabled
        if(!m_bEnableAsync)
            aOptions.enableAsync(false);

        backend::UpdateRequest anUpdate(
                                & aChangeTree.root,
                                aChangeTree.getRootNodePath(),
                                aOptions);

        getCacheLoader()->saveAndNotify(anUpdate);
        CFG_TRACE_INFO_NI("TreeManager: committing done");
    }
}

// -----------------------------------------------------------------------------
void TreeManager::updateTree(TreeChangeList& _aChanges) SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("TreeManager: updating the cache from a changes list");

    backend::UpdateInstance anUpdate(&_aChanges.root,_aChanges.getRootNodePath());

    rtl::Reference<CacheClientAccess> aCache = m_aCacheList.get(_aChanges.getOptions());

    if (!aCache.is())
    {
        CFG_TRACE_ERROR_NI("TreeManager: Cache access to update into does not exist !");
        OSL_ENSURE(aCache.is(),"TreeManager: Cache access to update into does not exist ! Where does the update access come from ?");
        throw lang::DisposedException(rtl::OUString::createFromAscii("Tree to be updated was already disposed"), NULL);
    }

    // merge the changes into the tree
    aCache->applyUpdate(anUpdate);

    CFG_TRACE_INFO_NI("TreeManager: cache update done");
}

// -----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void TreeManager::releaseSubtree( configuration::AbsolutePath const& aSubtreePath, const RequestOptions& _aOptions ) SAL_THROW(())
{
    CFG_TRACE_INFO("TreeManager: releasing subtree '%s' for entity '%s' with locale '%s'", OUSTRING2ASCII(aSubtreePath.toString()), OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

    rtl::Reference<CacheClientAccess> aCache = m_aCacheList.get(_aOptions);

    OSL_ENSURE(aCache.is(),"TreeManager: No local data to release");

    if (aCache.is())
    {
        CFG_TRACE_INFO_NI("TreeManager: decrementing refcount for subtree '%s'", OUSTRING2ASCII(aSubtreePath.toString()) );
        if (aCache->releaseNode(aSubtreePath) == 0)
        {
            backend::ComponentRequest aComponentDesc(aSubtreePath.getModuleName(),_aOptions);
            rtl::Reference< backend::CacheController > xBackendCache = maybeGetBackendCache();
            if (xBackendCache.is()) xBackendCache->freeComponent(aComponentDesc);
        }
    }
}
// ----------------------------------------------------------------------------
void TreeManager::refreshAll() SAL_THROW((com::sun::star::uno::Exception))
{
    //Find what components are in cache and that have client references and reload
    //such components.
     rtl::Reference< backend::CacheController > aCacheRef = maybeGetBackendCache();
     if (aCacheRef.is()) aCacheRef->refreshAllComponents();
}
// ----------------------------------------------------------------------------
void TreeManager::flushAll()SAL_THROW(())
{
     rtl::Reference< backend::CacheController > aCacheRef = maybeGetBackendCache();
     if (aCacheRef.is()) aCacheRef->flushPendingUpdates();
}
//-----------------------------------------------------------------------------
void TreeManager::enableAsync(const sal_Bool& bEnableAsync) SAL_THROW(())
{
    m_bEnableAsync = bEnableAsync;
}

    /////////////////////////////////////////////////////////////////////////
    void TreeManager::addListener(configuration::AbsolutePath const& aName, RequestOptions const & _aOptions, rtl::Reference<INodeListener> const& pHandler)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(_aOptions,true))
        {
            pHelper->addListener(aName, pHandler);
        }
        else
            OSL_ASSERT(false);
    }

    void TreeManager::removeListener(RequestOptions const & _aOptions, rtl::Reference<INodeListener> const& pHandler)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(_aOptions,false))
        {
            pHelper->removeListener( pHandler);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    void TreeManager::fireChanges(TreeChangeList const& rList_, sal_Bool bError_)
    {
        if (ConfigChangeBroadcastHelper* pHelper = getBroadcastHelper(rList_.getOptions(),false))
        {
            pHelper->broadcast(rList_, bError_, this);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    void TreeManager::disposeBroadcastHelper(ConfigChangeBroadcastHelper* pHelper)
    {
        if (pHelper)
        {
            pHelper->dispose(this);
            delete pHelper;
        }
    }

// ----------------------------------------------------------------------------
void TreeManager::nodeUpdated(TreeChangeList& _rChanges)
{
    CFG_TRACE_INFO("TreeManager: nodeUpdated");
    try
    {
    rtl::Reference<CacheClientAccess> aCache = m_aCacheList.get(_rChanges.getOptions());

    if (aCache.is())
    {
        // first approve the changes and merge them with the current tree
        configuration::AbsolutePath aSubtreeName = _rChanges.getRootNodePath();

        sharable::Node * aCacheTree = aCache->findInnerNode(aSubtreeName);
        //OSL_ENSURE(aCacheTree != NULL, "TreeManager::nodeUpdated : node not found in cache!");

        if (aCacheTree != NULL)
            this->fireChanges(_rChanges,false);
    }
    }
    catch (uno::RuntimeException&)
    {
    CFG_TRACE_ERROR_NI("TreeManager::nodeUpdated : could not notify !");
    }
    CFG_TRACE_INFO_NI("TreeManager: nodeUpdated done");
}

// ----------------------------------------------------------------------------

void TreeManager::componentCreated(backend::ComponentRequest const & ) SAL_THROW(())
{
    CFG_TRACE_INFO("TreeManager: component was created");
}
// ----------------------------------------------------------------------------

void TreeManager::componentChanged(backend::UpdateRequest  const & _anUpdate)     SAL_THROW(())
{
    TreeChangeList aChanges(_anUpdate.getOptions(),
                            _anUpdate.getUpdateRoot(),
                            *_anUpdate.getUpdateData(),
                            treeop::DeepChildCopy() );

    this->nodeUpdated(aChanges);
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
} // namespace
