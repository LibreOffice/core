/*************************************************************************
 *
 *  $RCSfile: cachecontroller.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:18:39 $
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

#include "cachecontroller.hxx"

#ifndef CONFIGMGR_DISPOSETIMER_HXX
#include "disposetimer.hxx"
#endif
#ifndef CONFIGMGR_CACHEWRITESCHEDULER_HXX
#include "cachewritescheduler.hxx"
#endif

#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif
#ifndef CONFIGMGR_TREEADDRESS_HXX
#include "treeaddress.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_BUILDDATA_HXX
#include "builddata.hxx"
#endif
#ifndef CONFIGMGR_LOCALIZEDTREEACTIONS_HXX
#include "localizedtreeactions.hxx"
#endif
#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
// -------------------------------------------------------------------------
    namespace backend
    {
// -------------------------------------------------------------------------

OTreeDisposeScheduler* CacheController::createDisposer()
{
    // TODO: parameterize the delays
#if OSL_DEBUG_LEVEL == 0
    const sal_uInt32 minute         = 60; // units are seconds
    // for initial debugging: use seconds instead of minutes
#else
    const sal_uInt32 minute         = 1;
#endif

    const sal_uInt32 c_nDefaultDelay    = 15 * minute;
    const sal_uInt32 c_nDefaultInterval =  1 * minute;

    TimeInterval aDelay(c_nDefaultDelay);
    TimeInterval aInterval(c_nDefaultInterval);

    return new OTreeDisposeScheduler(*this, aDelay, aInterval);
}

// -----------------------------------------------------------------------------

OCacheWriteScheduler* CacheController::createCacheWriter()
{
    const sal_uInt32 seconds = 1;
    const sal_uInt32 c_nDefaultInterval =  2 * seconds;

    TimeInterval aInterval(c_nDefaultInterval);
    return new OCacheWriteScheduler(*this, aInterval);
}
// ----------------------------------------------------------------------------

CacheController::CacheRef CacheController::getCacheAlways(RequestOptions const & _aOptions)
{
    osl::MutexGuard aGuard( m_aCacheList.mutex() );

    CacheRef aResult = m_aCacheList.get(_aOptions);
    if (!aResult.is())
    {
        CacheRef aNewCache( new Cache(getCacheHeapManager()) );
        aResult = m_aCacheList.insert(_aOptions,aNewCache);
    }
    return aResult;
}

// -------------------------------------------------------------------------

memory::HeapManager & CacheController::getCacheHeapManager() const
{
    return m_aTemplates.getHeapManager();
}
// -------------------------------------------------------------------------

// disposing
// -------------------------------------------------------------------------
void CacheController::disposeAll(bool _bFlushRemainingUpdates)
{
    CFG_TRACE_INFO("CacheController: Disposing all data" );
    CacheList::Map aReleaseList;

    if (m_pDisposer)
    {
        osl::MutexGuard aShotGuard(m_pDisposer->getShotMutex());
        osl::MutexGuard aGuard(m_aCacheList.mutex());
        m_pDisposer->stopAndClearTasks();
        m_aCacheList.swap(aReleaseList); // move data out of m_aCacheList and empty m_aCacheList
    }

    if (_bFlushRemainingUpdates)
    {
        for (CacheList::Map::iterator it = aReleaseList.begin(); it != aReleaseList.end(); ++it)
            saveAllPendingChanges(it->second,it->first);
    }
    // free all the trees
    aReleaseList.clear();
}

// -------------------------------------------------------------------------
void CacheController::dispose() CFG_UNO_THROW_RTE()
{
    CFG_TRACE_INFO("CacheController: dispose()" );

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::dispose(), disable lazy write cache.");
    m_bDisposing = true;                         // we are in dispose, handling of errors must be something different.

    // writing of pending updates
    this->flushPendingUpdates();

    // cleaning the cache
    this->disposeAll(true);
}

// -------------------------------------------------------------------------
void CacheController::disposeOne(RequestOptions const & _aOptions, bool _bFlushUpdates)
{
    osl::ClearableMutexGuard aGuard(m_aCacheList.mutex());

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

    CacheRef aRemoved = m_aCacheList.remove(_aOptions);

    if (aRemoved.is())
    {
        // got it out of reachability - now dispose/notify without lock
        aGuard.clear();
        implDisposeOne(aRemoved, _aOptions, _bFlushUpdates);
    }
    else
        CFG_TRACE_INFO_NI("- No affected TreeInfo found" );
}

// -------------------------------------------------------------------------
void CacheController::disposeUser(RequestOptions const & _aUserOptions, bool _bFlushUpdates)
{
    osl::ClearableMutexGuard aGuard(m_aCacheList.mutex());

    CFG_TRACE_INFO("CacheController: Disposing data and TreeInfo(s) for user '%s'",
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
            ++aLast;

        if (aFirst != aLast)
        {
            aDisposeList.reserve( std::distance(aFirst, aLast) );

            bool bHasPendingChanges = false;

            for (CacheList::Map::iterator it = aFirst; it != aLast; ++it)
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
        m_aCacheList.swap(aCacheData);
    }

    // got all out of external reach - now dispose/notify without lock
    aGuard.clear();

    for (DisposeList::iterator i = aDisposeList.begin(); i != aDisposeList.end(); ++i)
    {
        if (i->second.is())
            implDisposeOne(i->second, i->first, _bFlushUpdates);
    }
}

// -------------------------------------------------------------------------
void CacheController::implDisposeOne(CacheRef const & _aDisposedCache, RequestOptions const & _aOptions, bool _bFlushUpdates)
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
        CFG_TRACE_ERROR_NI("- Failed with exception %s (ignoring here)", OUSTRING2ASCII(e.Message) );
    }

    Cache::DisposeList aDisposedList;
    _aDisposedCache->clearData(aDisposedList);

    if (aDisposedList.size() > 0)
    {
        CFG_TRACE_INFO_NI("- Closing %d modules at the session",int(aDisposedList.size()));
        this->closeModules(aDisposedList,_aOptions);
    }
}

// -------------------------------------------------------------------------
CacheController::CacheController(BackendRef const & _xBackend, memory::HeapManager & _rCacheHeapManager)
: m_aNotifier()
, m_xBackend(_xBackend)
, m_aCacheList()
, m_aTemplates(_rCacheHeapManager)
, m_pDisposer()
, m_pCacheWriter()
, m_bDisposing(false)
{
    m_pDisposer = this->createDisposer();
    m_pCacheWriter = this->createCacheWriter();
}

// -------------------------------------------------------------------------
CacheController::~CacheController()
{
    OSL_ENSURE(m_bDisposing == true, "CacheController::dispose() wasn't called, something went wrong.");

    delete m_pDisposer;
    delete m_pCacheWriter;
}

// -------------------------------------------------------------------------
void CacheController::closeModules(Cache::DisposeList & _aList, RequestOptions const & _aOptions)
{
}

// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------

static
CacheLocation makeCacheLocation(memory::SegmentAddress const & _aSegment, memory::Heap::Address const & _anAddress)
{
    OSL_PRECOND(!_aSegment.isNull() || _anAddress == 0,"ERROR: Got Non-null address for NULL segment ?!");

    CacheLocation aResult;

    aResult.segment = _aSegment;
    aResult.address = _anAddress;

    OSL_ASSERT(!_aSegment.isNull() || aResult.isNull());

    return aResult;
}
// -------------------------------------------------------------------------
CacheLocation CacheController::loadComponent(ComponentRequest const & _aRequest)
{
    CFG_TRACE_INFO("CacheController: loading component '%s'", OUSTRING2ASCII(_aRequest.getComponentName().toString()));

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::loadComponent()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "component: %s", RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );

    CacheRef aCache = this->getCacheAlways(_aRequest.getOptions());

    OSL_ENSURE(aCache.is(), "Could not create CacheAccess");

    osl::MutexGuard aCacheLineGuard(aCache->mutex());

    data::TreeAddress aResultAddress;
    data::TreeAddress aTemplateResultAdddress;

    OSL_ENSURE(!_aRequest.isForcingReload(),"CacheController: No support for forced requests");
    if (aCache->hasModule(_aRequest.getComponentName()))
    {
        CFG_TRACE_INFO_NI("CacheController: found node in cache");

        aResultAddress = aCache->acquireModule(_aRequest.getComponentName());
    }
    else
    {
        ComponentResult aData = this->loadDirectly(_aRequest);

        bool bWithDefaults = ! m_xBackend->isStrippingDefaults();

        CFG_TRACE_INFO_NI("CacheController: adding loaded data to the cache");

        memory::UpdateAccessor aTargetSpace( aCache->createNewDataSegment(_aRequest.getComponentName()) );

        aResultAddress = aCache->addComponentData(aTargetSpace, aData.instance(), bWithDefaults);
        if (aData.instance().templateData().get()!=NULL)
        {
            aTemplateResultAdddress = addTemplates(aData.mutableInstance().componentTemplateData () );
        }
        // notify the new data to all clients
        m_aNotifier.notifyCreated(_aRequest);
    }

    return makeCacheLocation( aCache->getDataSegmentAddress(_aRequest.getComponentName()),
                              aResultAddress.addressValue());

}
// -------------------------------------------------------------------------

ComponentResult CacheController::getComponentData(ComponentRequest const & _aRequest) CFG_UNO_THROW_ALL()
{
    // TODO: Insert check here, if the data is in the cache already - and then clone
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::getComponentData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "component: %s", RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );

    ComponentResult aRet = this->loadDirectly(_aRequest);

    return aRet;
}
// -------------------------------------------------------------------------

NodeResult CacheController::getDefaultData(NodeRequest const & _aRequest) CFG_UNO_THROW_ALL(  )
{
    // TODO: Insert check here, if the data is in the cache already - and then clone
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::getDefaultData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "path: %s", RTL_LOGFILE_OU2A(_aRequest.getPath().toString()) );

    NodeResult aRet = this->loadDefaultsDirectly(_aRequest);

    return aRet;
}
// -------------------------------------------------------------------------

AbsolutePath CacheController::encodeTemplateLocation(const Name& _rName, const Name &_rModule) const
{
    namespace Path = configuration::Path;

//  static const
//  Component aTemplateRoot = wrapSimpleName(OUString::createFromAscii("org.openoffice.Templates"));

    Path::Component aTemplateModule = Path::wrapSimpleName(_rModule);
    Path::Component aTemplateName   = Path::wrapSimpleName(_rName);

    Path::Rep aResult(aTemplateName);
    aResult.prepend(aTemplateModule);
//    aResult.prepend(aTemplateRoot);

    return AbsolutePath(aResult);
}
// -------------------------------------------------------------------------

static
AbsolutePath templateLoadLocation(const AbsolutePath &_rTemplateLocation)
{
    namespace Path = configuration::Path;

    static const
    Path::Component aTemplateRoot = Path::wrapSimpleName(OUString::createFromAscii("org.openoffice.Templates"));

    Path::Rep aResult(_rTemplateLocation.rep());
    aResult.prepend(aTemplateRoot);

    return AbsolutePath(aResult);
}
// -------------------------------------------------------------------------
std::auto_ptr<ISubtree> CacheController::loadTemplateData(TemplateRequest const & _aTemplateRequest) CFG_UNO_THROW_ALL(  )
{
    std::auto_ptr<ISubtree> aMultiTemplates;
    TemplateResult aTemplateInstance = m_xBackend->getTemplateData(_aTemplateRequest);
    if (aTemplateInstance.is())
    {
        OSL_ASSERT(aTemplateInstance->name().isEmpty());
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
data::TreeAddress CacheController::addTemplates ( backend::ComponentData const & _aComponentInstance )
{
    OSL_PRECOND(_aComponentInstance.data.get(), "addTemplates: Data must not be NULL");
    osl::MutexGuard aGuard(m_aTemplatesMutex);
    TemplateCacheData::ModuleName aModuleName = _aComponentInstance.name;
    memory::UpdateAccessor aTemplatesUpdater( m_aTemplates.createDataSegment(aModuleName) );
    AbsolutePath aTemplateLocation = AbsolutePath::makeModulePath(_aComponentInstance.name , AbsolutePath::NoValidate());
    data::TreeAddress aTemplateAddr;
    if (!m_aTemplates.hasNode(aTemplatesUpdater.accessor(),aTemplateLocation ))
    {
        CFG_TRACE_INFO_NI("CacheController: cache miss for that template - loading from backend");
        aTemplateAddr = m_aTemplates.addTemplates(aTemplatesUpdater, _aComponentInstance );
    }
    OSL_ASSERT (aTemplateAddr.is());
    return aTemplateAddr;
 }
// -------------------------------------------------------------------------

CacheLocation CacheController::loadTemplate(TemplateRequest const & _aRequest) CFG_UNO_THROW_ALL(  )
{

    OSL_ENSURE(!_aRequest.getTemplateName().isEmpty(), "CacheController::loadTemplate : invalid template name !");
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::loadTemplate()");
    RTL_LOGFILE_CONTEXT_TRACE2(aLog, "requested template: %s/%s",
                                    RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) ,
                                    _aRequest.isComponentRequest() ?
                                        "*" : RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );


    AbsolutePath aTemplateLocation = encodeTemplateLocation(_aRequest.getTemplateName(), _aRequest.getComponentName());

    TemplateCacheData::ModuleName aModuleName =  aTemplateLocation.getModuleName();
    osl::MutexGuard aGuard(m_aTemplatesMutex);
    memory::Accessor aTemplatesAccessor( m_aTemplates.getDataSegment(aModuleName) );
    AbsolutePath aTemplateParent (aTemplateLocation.getParentPath());

    //Load-if-not-there (componentwise)
    if (!m_aTemplates.hasNode(aTemplatesAccessor, aTemplateParent))
    {
        aTemplatesAccessor.clear();
        OSL_ENSURE(aTemplateLocation.getDepth() > 1, "CacheController::ensureTemplate : invalid template location !");
        TemplateRequest aTemplateRequest = TemplateRequest::forComponent(_aRequest.getComponentName());

        std::auto_ptr<ISubtree> aMultiTemplates = loadTemplateData(aTemplateRequest);
        //add-if-not-loaded
        addTemplates(backend::ComponentData(aMultiTemplates, aModuleName));

    }
    memory::Accessor aTemplateAccessor( m_aTemplates.getDataSegment(aModuleName) );
    data::TreeAddress aTemplateAddr = m_aTemplates.getTemplateTree(aTemplateAccessor,aTemplateLocation);
    if (aTemplateAddr.isNull())
        throw uno::Exception(::rtl::OUString::createFromAscii("Unknown template. Type description could not be found in the given module."), NULL);

    return makeCacheLocation( m_aTemplates.getDataSegmentAddress(aTemplateLocation.getModuleName()) , aTemplateAddr.addressValue());
}
// -----------------------------------------------------------------------------

TemplateResult CacheController::getTemplateData(TemplateRequest const & _aRequest)
    CFG_UNO_THROW_ALL()
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::getTemplateData()");
    RTL_LOGFILE_CONTEXT_TRACE2(aLog, "requested template: %s/%s",
                                    RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) ,
                                    _aRequest.isComponentRequest() ?
                                        "*" : RTL_LOGFILE_OU2A(_aRequest.getComponentName().toString()) );



    AbsolutePath aTemplateLocation = encodeTemplateLocation(_aRequest.getTemplateName(), _aRequest.getComponentName());

    loadTemplate(_aRequest);
    //AbsolutePath aTemplateLocation = ensureTemplate(_aRequest.getTemplateName(), _aRequest.getComponentName());

    memory::Segment * pTemplatesSegment = m_aTemplates.getDataSegment(aTemplateLocation.getModuleName());

    memory::Accessor aTemplatesAccessor( pTemplatesSegment );

    data::TreeAddress aTemplateAddr = m_aTemplates.getTemplateTree(aTemplatesAccessor,aTemplateLocation);
    if (aTemplateAddr.isNull())
        throw uno::Exception(::rtl::OUString::createFromAscii("Unknown template. Type description could not be found in the given module."), NULL);

    data::TreeAccessor aTemplateTree(aTemplatesAccessor,aTemplateAddr);

    std::auto_ptr<INode> aResultTree = data::convertTree(aTemplateTree, true);

    TemplateInstance aResult(aResultTree,_aRequest.getTemplateName(), _aRequest.getComponentName());

    return TemplateResult(aResult);
}
// -----------------------------------------------------------------------------

void CacheController::saveAndNotify(UpdateRequest const & _anUpdate) CFG_UNO_THROW_ALL(  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::CacheController", "jb99855", "configmgr: CacheController::saveAndNotify()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "location: %s", RTL_LOGFILE_OU2A(_anUpdate.getUpdateRoot().toString()) );
    try
    {
        // ---------- preworking on the changes ----------
        // caller must own a read lock on this cache line
        CFG_TRACE_INFO("CacheController: saving an update for '%s'",OUSTRING2ASCII(_anUpdate.getUpdateRoot().toString()));

        CacheRef aCache = m_aCacheList.get(_anUpdate.getOptions());

        OSL_ENSURE(aCache.is(), "No cache data to update in saveAndNotify");

        if (!aCache.is()) throw lang::DisposedException(OUString::createFromAscii("Tree to be updated was already disposed"), NULL);

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

void CacheController::flushPendingUpdates()
{
    OSL_ASSERT(m_bDisposing);

    if (m_pCacheWriter)
    {
        osl::MutexGuard aShotGuard(m_pCacheWriter->getShotMutex());

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

ComponentResult CacheController::loadDirectly(ComponentRequest const & _aRequest) CFG_UNO_THROW_ALL(  )
{
    CFG_TRACE_INFO("CacheController: loading data for component '%s' from the backend", OUSTRING2ASCII(_aRequest.getComponentName().toString()));

    AbsolutePath aRequestPath = AbsolutePath::makeModulePath(_aRequest.getComponentName(), AbsolutePath::NoValidate());

    NodeRequest aNodeRequest(aRequestPath, _aRequest.getOptions());

    ComponentResult aResult = m_xBackend->getNodeData(_aRequest, this);

    OSL_PRECOND(aResult.mutableInstance().mutableData().get(), "loadDirectly: Data must not be NULL");

    CFG_TRACE_INFO_NI("- loading data completed - normalizing result");

    if (!normalizeResult( aResult.mutableInstance().mutableData(),_aRequest.getOptions()))
    {
        CFG_TRACE_ERROR_NI(" - cannot normalized result: failing");

        OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("Requested data at '"));
        sMsg += aRequestPath.toString();
        sMsg += OUString(RTL_CONSTASCII_USTRINGPARAM("'is not available: "));

        throw com::sun::star::container::NoSuchElementException(sMsg,NULL);
    }

    CFG_TRACE_INFO_NI(" - returning normalized defaults");

    return aResult;
}
// -----------------------------------------------------------------------------

NodeResult CacheController::loadDefaultsDirectly(NodeRequest const & _aRequest) CFG_UNO_THROW_ALL(  )
{
    CFG_TRACE_INFO("CacheController: loading defaults for '%s' from the backend", OUSTRING2ASCII(_aRequest.getPath().toString()));

    NodeResult aResult = m_xBackend->getDefaultData(_aRequest);

    CFG_TRACE_INFO_NI("- loading defaultscompleted - normalizing result");

    normalizeResult(aResult.mutableInstance().mutableData(),_aRequest.getOptions());

    CFG_TRACE_INFO_NI(" - returning normalized defaults");

    return aResult;
}
// -----------------------------------------------------------------------------

void CacheController::saveDirectly(UpdateRequest const & _anUpdate) CFG_UNO_THROW_ALL(  )
{
        m_xBackend->updateNodeData(_anUpdate);
}
// -----------------------------------------------------------------------------

void CacheController::savePendingChanges(CacheRef const & _aCache, ComponentRequest const & _aComponent) CFG_UNO_THROW_ALL(  )
{
    CFG_TRACE_INFO("CacheController: saving updates for tree: '%s'", OUSTRING2ASCII(_aComponent.getComponentName().toString()));

    try
    {
        CFG_TRACE_INFO("CacheController: saving updates for tree: '%s'", OUSTRING2ASCII(_aComponent.getComponentName().toString()));

          std::auto_ptr<SubtreeChange> aChangeData = _aCache->releasePendingChanges(_aComponent.getComponentName());

        if (aChangeData.get())
        {
            CFG_TRACE_INFO_NI("- found changes - sending to backend");

            AbsolutePath aRootPath = AbsolutePath::makeModulePath(_aComponent.getComponentName(), AbsolutePath::NoValidate());

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
        CFG_TRACE_ERROR_NI("CacheController: saving tree '%s' failed: %s",
                                OUSTRING2ASCII(_aComponent.getComponentName().toString()),
                                OUSTRING2ASCII(e.Message) );

        this->invalidateComponent(_aComponent);
        CFG_TRACE_INFO_NI("- component data invalidated");

        throw;
    }
}
// -----------------------------------------------------------------------------

bool CacheController::saveAllPendingChanges(CacheRef const & _aCache, RequestOptions const & _aOptions)
    CFG_UNO_THROW_RTE(  )
{
    CFG_TRACE_INFO("CacheController: Saving all pending changes for cache line");
    OSL_ASSERT(_aCache.is());

    typedef Cache::Data::PendingModuleList PMList;

    PMList aPendingModules;
    _aCache->findPendingChangedModules(aPendingModules);

    CFG_TRACE_INFO_NI("Found %d changed modules",int(aPendingModules.size()));

    bool bSuccess = true;
    for (PMList::iterator it = aPendingModules.begin();
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
// -----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CacheController::freeComponent(ComponentRequest const & _aRequest) CFG_NOTHROW()
{
    CFG_TRACE_INFO("CacheController: releasing module '%s' for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aRequest.getComponentName().toString()),
                    OUSTRING2ASCII(_aRequest.getOptions().getEntity()),
                    OUSTRING2ASCII(_aRequest.getOptions().getLocale()) );

    CacheRef aCache = m_aCacheList.get(_aRequest.getOptions());

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

// INotifyListener
// ----------------------------------------------------------------------------
/*
void CacheController::nodeUpdated(TreeChangeList& _rChanges)
{
    CFG_TRACE_INFO("cache manager: updating the tree from a notification");
    try
    {
        if (TreeInfo* pInfo = requestTreeInfo(_rChanges.getOptions(),false))
        {
            // first approve the changes and merge them with the current tree
            AbsolutePath aSubtreeName = _rChanges.getRootNodePath();

            memory::UpdateAccessor aUpdateAccess( pInfo->getDataSegment(aSubtreeName) );
            OSL_ENSURE(aUpdateAccess.is(), "CacheController::nodeUpdated : missing cache line!");

            data::NodeAddress aCacheTree = pInfo->getSubtree(aUpdateAccess.accessor(),aSubtreeName);
            OSL_ENSURE(aCacheTree.is(), "CacheController::nodeUpdated : node not found in cache!");

            if (aCacheTree.is())
            {
                if (adjustUpdate(_rChanges,aUpdateAccess,aCacheTree))
                {
                    pInfo->updateTree(aUpdateAccess,_rChanges);

                    data::Accessor aNotifyLock = aUpdateAccess.downgrade(); // keep a read lock during notification

                    notifyUpdate(aNotifyLock,_rChanges);
                }
            }
        }
    }
    catch (uno::RuntimeException&)
    {
        CFG_TRACE_ERROR("CacheController::nodeUpdated : could not insert notifications, data may be inconsistent !");
    }
}
*/


// -------------------------------------------------------------------------
    } // namespace

// -------------------------------------------------------------------------
} // namespace
