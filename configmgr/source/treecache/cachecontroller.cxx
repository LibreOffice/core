/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachecontroller.cxx,v $
 * $Revision: 1.21 $
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

#include "cachecontroller.hxx"
#include "disposetimer.hxx"
#include "cachewritescheduler.hxx"
#include "builddata.hxx"
#include "localizedtreeactions.hxx"
#include "configexcept.hxx"
#include "tracer.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <osl/diagnose.h>
#include <rtl/logfile.hxx>

#ifndef _CONFIGMGR_BOOTSTRAP_HXX
#include "bootstrap.hxx"
#endif


#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
// -------------------------------------------------------------------------
    namespace backend
    {

static const rtl::OUString kCacheDisposeDelay(
    RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "CacheDisposeDelay"));
static const rtl::OUString kCacheDisposeInterval(
    RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "CacheDisposeInterval"));
static const rtl::OUString kCacheWriteInterval(
    RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "CacheWriteInterval"));
// -------------------------------------------------------------------------

OTreeDisposeScheduler* CacheController::createDisposer(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext)
{
    ContextReader aReader(_xContext);
    sal_uInt32 c_nDefaultDelay = 0;
    rtl::OUString sDefaultDelay;
    aReader.getBestContext()->getValueByName(kCacheDisposeDelay) >>= sDefaultDelay;
    c_nDefaultDelay = sDefaultDelay.toInt32()==0?900:sDefaultDelay.toInt32() ;

    sal_uInt32 c_nDefaultInterval = 0;
    rtl::OUString sDefaultInterval;
    aReader.getBestContext()->getValueByName(kCacheDisposeInterval) >>= sDefaultInterval;
    c_nDefaultInterval = sDefaultInterval.toInt32()==0?60:sDefaultInterval.toInt32();

    TimeInterval aDelay(c_nDefaultDelay);
    TimeInterval aInterval(c_nDefaultInterval);

    return new OTreeDisposeScheduler(*this, aDelay, aInterval);
}

// -----------------------------------------------------------------------------

OCacheWriteScheduler* CacheController::createCacheWriter(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext)
{
    ContextReader aReader(_xContext);
    sal_uInt32 c_nDefaultInterval=0;
    rtl::OUString sDefaultInterval;
    aReader.getBestContext()->getValueByName(kCacheWriteInterval) >>= sDefaultInterval;
    c_nDefaultInterval = sDefaultInterval.toInt32()==0?2:sDefaultInterval.toInt32();

    TimeInterval aInterval(c_nDefaultInterval);
    return new OCacheWriteScheduler(*this, aInterval);
}
// ----------------------------------------------------------------------------

rtl::Reference<CacheLoadingAccess> CacheController::getCacheAlways(RequestOptions const & _aOptions)
{
    rtl::Reference<CacheLoadingAccess> aResult = m_aCacheMap.get(_aOptions);
    if (!aResult.is())
    {
        rtl::Reference<CacheLoadingAccess> aNewCache( new CacheLoadingAccess() );
        aResult = m_aCacheMap.insert(_aOptions,aNewCache);
    }
    return aResult;
}

// -------------------------------------------------------------------------

// disposing
// -------------------------------------------------------------------------
void CacheController::disposeAll(bool _bFlushRemainingUpdates)
{
    CFG_TRACE_INFO("CacheController: Disposing all data" );
    CacheMap::Map aReleaseList;

    if (m_pDisposer)
    {
        m_pDisposer->stopAndClearTasks();
        m_aCacheMap.swap(aReleaseList); // move data out of m_aCacheMap and empty m_aCacheMap
    }

    if (_bFlushRemainingUpdates)
    {
        for (CacheMap::Map::iterator it = aReleaseList.begin(); it != aReleaseList.end(); ++it)
            saveAllPendingChanges(it->second,it->first);
    }
    // free all the trees
    aReleaseList.clear();
}

// -------------------------------------------------------------------------
void CacheController::dispose() SAL_THROW((com::sun::star::uno::RuntimeException))
{
    UnoApiLock aLock;

    CFG_TRACE_INFO("CacheController: dispose()" );

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::dispose(), disable lazy write cache.");
    m_bDisposing = true;                         // we are in dispose, handling of errors must be something different.

    // writing of pending updates
    this->flushCacheWriter();

    // cleaning the cache
    this->disposeAll(true);
}

// -------------------------------------------------------------------------
void CacheController::disposeOne(RequestOptions const & _aOptions, bool _bFlushUpdates)
{
    CFG_TRACE_INFO("CacheController: Disposing data and TreeInfo for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

    m_pDisposer->clearTasks(_aOptions);
    if (!m_pCacheWriter->clearTasks(_aOptions)) // had no pending updates
    {
        _bFlushUpdates = false;
    }
    else if (!_bFlushUpdates)
    {
        CFG_TRACE_WARNING_NI("Found orphaned Changes in the cache - Discarding.");
    }

    rtl::Reference<CacheLoadingAccess> aRemoved = m_aCacheMap.remove(_aOptions);

    if (aRemoved.is())
    {
        // got it out of reachability - now dispose/notify without lock
        implDisposeOne(aRemoved, _aOptions, _bFlushUpdates);
    }
    else
        CFG_TRACE_INFO_NI("- No affected TreeInfo found" );
}

// -------------------------------------------------------------------------
void CacheController::disposeUser(RequestOptions const & _aUserOptions, bool _bFlushUpdates)
{
    CFG_TRACE_INFO("CacheController: Disposing data and TreeInfo(s) for user '%s'",
                    OUSTRING2ASCII(_aUserOptions.getEntity()) );

    std::vector< std::pair< RequestOptions, rtl::Reference<CacheLoadingAccess> > > aDisposeList;
    // collect the ones to dispose
    {
        rtl::OUString sUser = _aUserOptions.getEntity();
        OSL_ASSERT(sUser.getLength());

        // This depends on the fact that Options are sorted (by struct ltOptions)
        // so that all options belonging to one user are together
        // (and that options with only a user set, sort first)

        CacheMap::Map aCacheData;
        m_aCacheMap.swap(aCacheData);

        // find the lower_bound of all options for the user
        CacheMap::Map::iterator const aFirst = aCacheData.lower_bound(_aUserOptions);

        // find the upper_bound of all options for the user (using the lower one)
        CacheMap::Map::iterator aLast = aFirst;
        while (aLast != aCacheData.end() && aLast->first.getEntity() == sUser)
            ++aLast;

        if (aFirst != aLast)
        {
            aDisposeList.reserve( std::distance(aFirst, aLast) );

            bool bHasPendingChanges = false;

            for (CacheMap::Map::iterator it = aFirst; it != aLast; ++it)
            {
                CFG_TRACE_INFO_NI("- Found TreeInfo for locale '%s'", OUSTRING2ASCII(it->first.getLocale()) );
                m_pDisposer->clearTasks(it->first);

                if (m_pCacheWriter->clearTasks(it->first))
                    bHasPendingChanges = true;

                OSL_ASSERT(it->second.is());
                if (it->second.is())
                {
                    //aDisposeList.push_back( *it );
                    aDisposeList.push_back( std::make_pair(it->first,it->second) );
                }
            }

            if (!bHasPendingChanges)
                _bFlushUpdates = false;

            else if (!_bFlushUpdates)
                CFG_TRACE_WARNING_NI("Found orphaned Changes in the cache - Discarding.");

            aCacheData.erase(aFirst, aLast);
        }
        else
            CFG_TRACE_INFO_NI("- No affected TreeInfo found" );

        // replace the data into the map
        m_aCacheMap.swap(aCacheData);
    }

    // got all out of external reach - now dispose/notify without lock
    for (std::vector< std::pair< RequestOptions, rtl::Reference<CacheLoadingAccess> > >::iterator i = aDisposeList.begin(); i != aDisposeList.end(); ++i)
    {
        if (i->second.is())
            implDisposeOne(i->second, i->first, _bFlushUpdates);
    }
}

// -------------------------------------------------------------------------
void CacheController::implDisposeOne(rtl::Reference<CacheLoadingAccess> const & _aDisposedCache, RequestOptions const & _aOptions, bool _bFlushUpdates)
{
    OSL_ASSERT(_aDisposedCache.is());
    CFG_TRACE_INFO("Now removing Cache section (user '%s' with locale '%s')",
                    OUSTRING2ASCII(_aOptions.getEntity()), OUSTRING2ASCII(_aOptions.getLocale()) );

    if (_bFlushUpdates) try
    {
        CFG_TRACE_INFO_NI("- Flushing pending changes" );

        if ( !this->saveAllPendingChanges(_aDisposedCache,_aOptions) )
        {
            CFG_TRACE_ERROR_NI("- Error while flushing - changes will be lost" );
            OSL_ENSURE(false,"Error while flushing changes from discarded Cache section - changes will be lost" );
        }
    }
    catch (uno::Exception& e)
    {
        (void)e;
        CFG_TRACE_ERROR_NI("- Failed with exception %s (ignoring here)", OUSTRING2ASCII(e.Message) );
    }

    std::vector< rtl::Reference<CacheLine> > aDisposedList;
    _aDisposedCache->clearData(aDisposedList);

    if (aDisposedList.size() > 0)
    {
        CFG_TRACE_INFO_NI("- Closing %d modules at the session",int(aDisposedList.size()));
        this->closeModules(aDisposedList,_aOptions);
    }
}

// -------------------------------------------------------------------------
CacheController::CacheController(rtl::Reference< backend::IMergedDataProvider > const & _xBackend,
                                 const uno::Reference<uno::XComponentContext>& xContext)
: m_aNotifier()
, m_xBackend(_xBackend)
, m_aCacheMap()
, m_aTemplates()
, m_pDisposer()
, m_pCacheWriter()
, m_bDisposing(false)
{
    m_pDisposer = this->createDisposer(xContext);
    m_pCacheWriter = this->createCacheWriter(xContext);
}

// -------------------------------------------------------------------------
CacheController::~CacheController()
{
    OSL_ENSURE(m_bDisposing == true, "CacheController::dispose() wasn't called, something went wrong.");

    delete m_pDisposer;
    delete m_pCacheWriter;
}

// -------------------------------------------------------------------------
void CacheController::closeModules(std::vector< rtl::Reference<CacheLine> > & _aList, RequestOptions const & _aOptions)
{
    //Remove listeners from Backend as module no longer in cache
    for (sal_uInt32 i =0; i < _aList.size(); ++i)
    {
        rtl::OUString aModuleName = _aList[i]->getModuleName();
        ComponentRequest aRequest(aModuleName, _aOptions);
        m_xBackend->removeRequestListener(this, aRequest);
    }
}
// -------------------------------------------------------------------------
#if 0
static
std::auto_ptr<ISubtree> reduceSubtreeForLocale(std::auto_ptr<ISubtree> _pSubtree, RequestOptions const & _aOptions)
{
    OSL_ENSURE(!_pSubtree.get() || !isLocalizedValueSet(*_pSubtree), "Unexpected node. Expecting a subtree, Found a single localized value.");

    std::auto_ptr<ISubtree> aRet;

    std::auto_ptr<INode> aReduced = reduceExpandedForLocale(_pSubtree, _aOptions.getLocale());

    if (aReduced.get())
    {
        if (ISubtree* pReduced =aReduced->asISubtree())
        {
            aRet.reset(pReduced);
            aReduced.release();
        }
        else
        {
            OSL_ENSURE(false, "Tree unexpectedly reduced to non-tree");
        }
    }
    else
        OSL_ENSURE(!_pSubtree.get(), "Tree unexpectedly reduced to nothing");

    return aRet;
}
#endif

// -------------------------------------------------------------------------
sharable::TreeFragment * CacheController::loadComponent(ComponentRequest const & _aRequest)
{
    CFG_TRACE_INFO("CacheController: loading component '%s'", OUSTRING2ASCII(_aRequest.getComponentName()));

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::loadComponent()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "component: %s", RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );

    rtl::Reference<CacheLoadingAccess> aCache = this->getCacheAlways(_aRequest.getOptions());

    OSL_ENSURE(aCache.is(), "Could not create CacheAccess");

    sharable::TreeFragment * aTemplateResultAdddress;

    OSL_ENSURE(!_aRequest.isForcingReload(),"CacheController: No support for forced requests");
    if (aCache->hasModule(_aRequest.getComponentName()))
    {
        CFG_TRACE_INFO_NI("CacheController: found node in cache");
        if (_aRequest.getOptions().isRefreshEnabled())
        {
            refreshComponent(_aRequest);
        }
        aCache->acquireModule(_aRequest.getComponentName());
    }
    else
    {
        ResultHolder< ComponentInstance > aData = this->loadDirectly(_aRequest,true);

        CFG_TRACE_INFO_NI("CacheController: adding loaded data to the cache");

        aCache->createModule(_aRequest.getComponentName());

        aCache->addComponentData(aData.instance(), true);
        if (aData.instance().templateData().get()!=NULL)
            aTemplateResultAdddress = addTemplates(aData.mutableInstance().componentTemplateData() );

        // notify the new data to all clients
        m_aNotifier.notifyCreated(_aRequest);
    }

    return aCache->getTreeAddress(_aRequest.getComponentName());
}
// -------------------------------------------------------------------------

ResultHolder< ComponentInstance > CacheController::getComponentData(ComponentRequest const & _aRequest,
                                                  bool _bAddListenter ) SAL_THROW((com::sun::star::uno::Exception))
{
    // TODO: Insert check here, if the data is in the cache already - and then clone
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::getComponentData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "component: %s", RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );

    ResultHolder< ComponentInstance > aRet = this->loadDirectly(_aRequest, _bAddListenter);

    return aRet;
}
// -------------------------------------------------------------------------

ResultHolder< NodeInstance > CacheController::getDefaultData(NodeRequest const & _aRequest) SAL_THROW((com::sun::star::uno::Exception))
{
    // TODO: Insert check here, if the data is in the cache already - and then clone
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::getDefaultData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "path: %s", RTL_LOGFILE_OU2A(_aRequest.getPath().toString()) );

    ResultHolder< NodeInstance > aRet = this->loadDefaultsDirectly(_aRequest);

    return aRet;
}
// -------------------------------------------------------------------------

configuration::AbsolutePath CacheController::encodeTemplateLocation(const rtl::OUString& _rName, const rtl::OUString &_rModule) const
{
    namespace Path = configuration::Path;

//  static const
//  Component aTemplateRoot = wrapSimpleName(rtl::OUString::createFromAscii("org.openoffice.Templates"));

    Path::Component aTemplateModule = Path::wrapSimpleName(_rModule);
    Path::Component aTemplateName   = Path::wrapSimpleName(_rName);

    Path::Rep aResult(aTemplateName);
    aResult.prepend(aTemplateModule);
//    aResult.prepend(aTemplateRoot);

    return configuration::AbsolutePath(aResult);
}
// -------------------------------------------------------------------------
#if 0
static
configuration::AbsolutePath templateLoadLocation(const configuration::AbsolutePath &_rTemplateLocation)
{
    namespace Path = configuration::Path;

    static const
    Path::Component aTemplateRoot = Path::wrapSimpleName(rtl::OUString::createFromAscii("org.openoffice.Templates"));

    Path::Rep aResult(_rTemplateLocation.rep());
    aResult.prepend(aTemplateRoot);

    return configuration::AbsolutePath(aResult);
}
#endif
// -------------------------------------------------------------------------
std::auto_ptr<ISubtree> CacheController::loadTemplateData(TemplateRequest const & _aTemplateRequest) SAL_THROW((com::sun::star::uno::Exception))
{
    std::auto_ptr<ISubtree> aMultiTemplates;
    ResultHolder< TemplateInstance > aTemplateInstance = m_xBackend->getTemplateData(_aTemplateRequest);
    if (aTemplateInstance.is())
    {
        OSL_ASSERT(aTemplateInstance->name().getLength() == 0);
        if (ISubtree * pMulti = aTemplateInstance->data()->asISubtree())
        {
            aTemplateInstance.releaseAndClear();
            aMultiTemplates.reset(pMulti);
        }
        else
            OSL_ENSURE(false,"Requested multiple templates, got non-subtree node");
    }
    else
        OSL_ENSURE(false,"Requested configuration template does not exist");

    if (aMultiTemplates.get() == NULL)
    {
        CFG_TRACE_ERROR_NI("CacheController: could not load the templates");
        throw uno::Exception(::rtl::OUString::createFromAscii("The template description could not be loaded. The template does not exist."), NULL);
    }

    return  aMultiTemplates;
}
// -------------------------------------------------------------------------
sharable::TreeFragment * CacheController::addTemplates ( backend::ComponentDataStruct const & _aComponentInstance )
{
    OSL_PRECOND(_aComponentInstance.data.get(), "addTemplates: Data must not be NULL");
    rtl::OUString aModuleName = _aComponentInstance.name;
    m_aTemplates.createModule(aModuleName);
    configuration::AbsolutePath aTemplateLocation = configuration::AbsolutePath::makeModulePath(_aComponentInstance.name);
    sharable::TreeFragment * aTemplateAddr = NULL;

    if (!m_aTemplates.hasNode(aTemplateLocation ))
    {
        CFG_TRACE_INFO_NI("CacheController: cache miss for that template - loading from backend");
        aTemplateAddr = m_aTemplates.addTemplates(_aComponentInstance );
    }
    OSL_ASSERT (aTemplateAddr != NULL);
    return aTemplateAddr;
 }
// -------------------------------------------------------------------------

sharable::TreeFragment * CacheController::loadTemplate(TemplateRequest const & _aRequest) SAL_THROW((com::sun::star::uno::Exception))
{

    OSL_ENSURE(_aRequest.getTemplateName().getLength() != 0, "CacheController::loadTemplate : invalid template name !");
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::loadTemplate()");
    RTL_LOGFILE_CONTEXT_TRACE2(aLog, "requested template: %s/%s",
                                    RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) ,
                                    _aRequest.isComponentRequest() ?
                                        "*" : RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );


    configuration::AbsolutePath aTemplateLocation = encodeTemplateLocation(_aRequest.getTemplateName(), _aRequest.getComponentName());

    rtl::OUString aModuleName =  aTemplateLocation.getModuleName();

    configuration::AbsolutePath aTemplateParent (aTemplateLocation.getParentPath());

    //Load-if-not-there (componentwise)
    if (!m_aTemplates.hasNode(aTemplateParent))
    {
        OSL_ENSURE(aTemplateLocation.getDepth() > 1, "CacheController::ensureTemplate : invalid template location !");
        TemplateRequest aTemplateRequest = TemplateRequest::forComponent(_aRequest.getComponentName());

        std::auto_ptr<ISubtree> aMultiTemplates = loadTemplateData(aTemplateRequest);
        //add-if-not-loaded
        addTemplates(backend::ComponentDataStruct(aMultiTemplates, aModuleName));

    }
    sharable::TreeFragment * aTemplateAddr = m_aTemplates.getTemplateTree(aTemplateLocation);
    if (aTemplateAddr == NULL)
        throw uno::Exception(::rtl::OUString::createFromAscii("Unknown template. Type description could not be found in the given module."), NULL);

    return m_aTemplates.getTreeAddress(aTemplateLocation.getModuleName());
}
// -----------------------------------------------------------------------------

ResultHolder< TemplateInstance > CacheController::getTemplateData(TemplateRequest const & _aRequest)
    SAL_THROW((com::sun::star::uno::Exception))
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::getTemplateData()");
    RTL_LOGFILE_CONTEXT_TRACE2(aLog, "requested template: %s/%s",
                   RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) ,
                   _aRequest.isComponentRequest() ?
                   "*" : RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );

    configuration::AbsolutePath aTemplateLocation = encodeTemplateLocation(_aRequest.getTemplateName(), _aRequest.getComponentName());

    loadTemplate(_aRequest);
    //configuration::AbsolutePath aTemplateLocation = ensureTemplate(_aRequest.getTemplateName(), _aRequest.getComponentName());

    sharable::TreeFragment * aTemplateAddr = m_aTemplates.getTemplateTree(aTemplateLocation);
    if (aTemplateAddr == NULL)
    throw uno::Exception(::rtl::OUString::createFromAscii("Unknown template. Type description could not be found in the given module."), NULL);

    std::auto_ptr<INode> aResultTree = data::convertTree(aTemplateAddr, true);

    TemplateInstance aResult(aResultTree,_aRequest.getTemplateName(), _aRequest.getComponentName());

    return ResultHolder< TemplateInstance >(aResult);
}
// -----------------------------------------------------------------------------

void CacheController::saveAndNotify(UpdateRequest const & _anUpdate) SAL_THROW((com::sun::star::uno::Exception))
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::saveAndNotify()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "location: %s", RTL_LOGFILE_OU2A(_anUpdate.getUpdateRoot().toString()) );
    try
    {
        // ---------- preworking on the changes ----------
        // caller must own a read lock on this cache line
        CFG_TRACE_INFO("CacheController: saving an update for '%s'",OUSTRING2ASCII(_anUpdate.getUpdateRoot().toString()));

        rtl::Reference<CacheLoadingAccess> aCache = m_aCacheMap.get(_anUpdate.getOptions());

        OSL_ENSURE(aCache.is(), "No cache data to update in saveAndNotify");

        if (!aCache.is()) throw lang::DisposedException(rtl::OUString::createFromAscii("Tree to be updated was already disposed"), NULL);

        aCache->addChangesToPending(_anUpdate.getUpdate());

        if ( _anUpdate.isSyncRequired()||  m_bDisposing ) // cannot do it asynchronously
        {
            CFG_TRACE_INFO_NI("Running synchronous write");
            savePendingChanges( aCache, getComponentRequest(_anUpdate) );
        }

        else
        {
            CFG_TRACE_INFO_NI("Posting asynchronous write");
            m_pCacheWriter->scheduleWrite( getComponentRequest(_anUpdate) );
        }

        CFG_TRACE_INFO_NI("Notifying the changes");
        // notify the changes to all clients
        m_aNotifier.notifyChanged(_anUpdate);
    }
    catch(configuration::Exception& ex)
    {
        CFG_TRACE_ERROR_NI("Got unexpected exception: %s", ex.what());
        configapi::ExceptionMapper e(ex);
        e.unhandled();
    }

}
// -----------------------------------------------------------------------------

void CacheController::flushPendingUpdates()SAL_THROW((com::sun::star::uno::Exception))
{
    CacheMap::Map aFlushList = m_aCacheMap.copy();

    for (CacheMap::Map::iterator it = aFlushList.begin(); it != aFlushList.end(); ++it)
        saveAllPendingChanges(it->second,it->first);
}

void CacheController::flushCacheWriter()SAL_THROW(())
{
    //OSL_ASSERT(m_bDisposing);

    if (m_pCacheWriter)
    {
        CFG_TRACE_INFO("CacheController: flushing all pending updates");

        m_pCacheWriter->stopAndWriteCache();
    }
}
// -----------------------------------------------------------------------------

bool CacheController::normalizeResult(std::auto_ptr<ISubtree> &  _aResult, RequestOptions const & _aOptions)
{

    if (_aResult.get()==NULL) return false;

    if (_aOptions.isForAllLocales()) return true;

    std::auto_ptr<INode> aReduced = reduceExpandedForLocale(_aResult, _aOptions.getLocale());

    std::auto_ptr<ISubtree> aReducedTree;
    if (aReduced.get())
    {
        if (ISubtree* pReducedTree =aReduced->asISubtree())
        {
            aReduced.release();
            aReducedTree.reset(pReducedTree);
        }
        else
        {
            OSL_ENSURE(false, "Tree unexpectedly reduced to non-tree");
        }
    }
    else
        OSL_ENSURE(false, "Tree unexpectedly reduced to nothing");


    _aResult = aReducedTree;
    bool retCode = _aResult.get()!=NULL ? true : false;
    return retCode;
}
// -----------------------------------------------------------------------------

ResultHolder< ComponentInstance > CacheController::loadDirectly(ComponentRequest const & _aRequest, bool _bAddListenter) SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("CacheController: loading data for component '%s' from the backend", OUSTRING2ASCII(_aRequest.getComponentName()));

    configuration::AbsolutePath aRequestPath = configuration::AbsolutePath::makeModulePath(_aRequest.getComponentName());

    NodeRequest aNodeRequest(aRequestPath, _aRequest.getOptions());

    ResultHolder< ComponentInstance > aResult = m_xBackend->getNodeData(_aRequest, this, _bAddListenter?this:NULL);

    OSL_PRECOND(aResult.mutableInstance().mutableData().get(), "loadDirectly: Data must not be NULL");

    CFG_TRACE_INFO_NI("- loading data completed - normalizing result");

    if (!normalizeResult( aResult.mutableInstance().mutableData(),_aRequest.getOptions()))
    {
    CFG_TRACE_ERROR_NI(" - cannot normalized result: failing");

        rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("Requested data at '"));
        sMsg += aRequestPath.toString();
        sMsg += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'is not available: "));

        throw com::sun::star::container::NoSuchElementException(sMsg,NULL);
    }

    CFG_TRACE_INFO_NI(" - returning normalized defaults");

    return aResult;
}
// -----------------------------------------------------------------------------

ResultHolder< NodeInstance > CacheController::loadDefaultsDirectly(NodeRequest const & _aRequest) SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("CacheController: loading defaults for '%s' from the backend", OUSTRING2ASCII(_aRequest.getPath().toString()));

    ResultHolder< NodeInstance > aResult = m_xBackend->getDefaultData(_aRequest);

    CFG_TRACE_INFO_NI("- loading defaultscompleted - normalizing result");

    normalizeResult(aResult.mutableInstance().mutableData(),_aRequest.getOptions());

    CFG_TRACE_INFO_NI(" - returning normalized defaults");

    return aResult;
}
// -----------------------------------------------------------------------------

void CacheController::saveDirectly(UpdateRequest const & _anUpdate) SAL_THROW((com::sun::star::uno::Exception))
{
    m_xBackend->updateNodeData(_anUpdate);
}
// -----------------------------------------------------------------------------

void CacheController::savePendingChanges(rtl::Reference<CacheLoadingAccess> const & _aCache, ComponentRequest const & _aComponent) SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("CacheController: saving updates for tree: '%s'", OUSTRING2ASCII(_aComponent.getComponentName()));

    try
    {
    CFG_TRACE_INFO2("CacheController: saving updates for tree: '%s'", OUSTRING2ASCII(_aComponent.getComponentName()));

    std::auto_ptr<SubtreeChange> aChangeData = _aCache->releasePendingChanges(_aComponent.getComponentName());

        if (aChangeData.get())
        {
        CFG_TRACE_INFO_NI("- found changes - sending to backend");

        configuration::AbsolutePath aRootPath = configuration::AbsolutePath::makeModulePath(_aComponent.getComponentName());

            backend::UpdateRequest anUpdateSpec(aChangeData.get(),aRootPath,_aComponent.getOptions());

            // anUpdateSpec.setRequestId(pInfo->getRequestId(_aRootPath));

            this->saveDirectly(anUpdateSpec);

        CFG_TRACE_INFO_NI("- saving changes completed successfully");
        }
        else
        CFG_TRACE_WARNING_NI("- no changes found - cannot save");
    }
    catch(uno::Exception& e)
    {
    (void)e;
    CFG_TRACE_ERROR_NI("CacheController: saving tree '%s' failed: %s",
               OUSTRING2ASCII(_aComponent.getComponentName()),
               OUSTRING2ASCII(e.Message) );

    refreshComponent(_aComponent);
    CFG_TRACE_INFO_NI("- component data invalidated");

    throw;
    }
}
// -----------------------------------------------------------------------------

bool CacheController::saveAllPendingChanges(rtl::Reference<CacheLoadingAccess> const & _aCache, RequestOptions const & _aOptions)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    CFG_TRACE_INFO("CacheController: Saving all pending changes for cache line");
    OSL_ASSERT(_aCache.is());

    std::vector< rtl::OUString > aPendingModules;
    _aCache->findPendingChangedModules(aPendingModules);

    CFG_TRACE_INFO_NI("Found %d changed modules",int(aPendingModules.size()));

    bool bSuccess = true;
    for (std::vector< rtl::OUString >::iterator it = aPendingModules.begin();
            it != aPendingModules.end();
            ++it )
    {
        try
        {
            this->savePendingChanges(_aCache, ComponentRequest(*it,_aOptions) );
        }
        catch (uno::Exception & )
        {
            CFG_TRACE_ERROR_NI("CacheController: Exception while saving one module - ignoring");
            bSuccess = false;
        }
    }
    CFG_TRACE_INFO_NI("Done saving pending changes for cache line");

    return bSuccess;
}
// -----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void CacheController::freeComponent(ComponentRequest const & _aRequest) SAL_THROW(())
{
    CFG_TRACE_INFO("CacheController: releasing module '%s' for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aRequest.getComponentName()),
                    OUSTRING2ASCII(_aRequest.getOptions().getEntity()),
                    OUSTRING2ASCII(_aRequest.getOptions().getLocale()) );

    rtl::Reference<CacheLoadingAccess> aCache = m_aCacheMap.get(_aRequest.getOptions());

    OSL_ENSURE(aCache.is(), "Releasing a nonexisting module");

    if (aCache.is())
    {
        if (aCache->releaseModule(_aRequest.getComponentName()) == 0)
        {
            // start the cleanup
            m_pDisposer->scheduleCleanup(_aRequest.getOptions());
        }
    }
}
// -----------------------------------------------------------------------------
void CacheController::dataChanged(const ComponentRequest& _aRequest) SAL_THROW(())
{
    refreshComponent(_aRequest);
}
// -----------------------------------------------------------------------------
void CacheController::refreshAllComponents() SAL_THROW((com::sun::star::uno::Exception))
{
    CacheMap::Map aRefreshList = m_aCacheMap.copy();

    for (CacheMap::Map::iterator i = aRefreshList.begin();
        i != aRefreshList.end(); ++i)
    {
        if (!i->second->isEmpty())
        {
            ExtendedCacheData aCacheData = i->second->m_aData;
            RequestOptions aOption = i->first;
            CacheData::ModuleList aModuleList = aCacheData.accessModuleList();
            for (CacheData::ModuleList::iterator itr = aModuleList.begin();
                itr != aModuleList.end(); ++itr)
            {
                //Check the cacheline has atleast one client reference


                if (itr->second->clientReferences() > 0)
                {
                    ComponentRequest aRequest(itr->first,i->first);
                    refreshComponent(aRequest);
                } else
                {
                    // FIXME: otherwise dispose now
                    // XXX: (lo) refresh all, preventing cache corruption.
                    // An unused component should be purged from the cache
                    // instead of being refreshed
                    ComponentRequest aRequest(itr->first,i->first);
                    refreshComponent(aRequest);
                }
            }
        }
    }
}

// -------------------------------------------------------------------------
    } // namespace

// -------------------------------------------------------------------------
} // namespace
