/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: invalidatetree.cxx,v $
 * $Revision: 1.23 $
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
#include "change.hxx"
#include "valuenode.hxx"
#include "updatehelper.hxx"
#include "treeactions.hxx"
#include "treeaccessor.hxx"
#include "tracer.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <vos/thread.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

namespace configmgr
{

    using namespace com::sun::star::uno;
    using namespace configuration;
    namespace container = com::sun::star::container;
// -----------------------------------------------------------------------------
// ------------------------------- invalidateTree -------------------------------
// -----------------------------------------------------------------------------

namespace backend
{
// -----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> createDiffs(data::NodeAccess const& _aCachedNode,
                                            ISubtree const * _pLoadedSubtree,
                                            AbsolutePath const& _aAbsoluteSubtreePath)
{
    OSL_PRECOND(_aCachedNode.isValid(), "Need an existing node to create a diff");
    OSL_PRECOND(_pLoadedSubtree != 0, "Need a result node to create a diff");
    // Create a TreeChangeList with the right name, parentname and ConfigurationProperties
    std::auto_ptr<SubtreeChange> aNewChange(new SubtreeChange(_aAbsoluteSubtreePath.getLocalName().getName().toString(),
                                                                node::Attributes()) );

    if (!createUpdateFromDifference(*aNewChange, _aCachedNode, *_pLoadedSubtree))
        aNewChange.reset();

    return aNewChange;
}
// -----------------------------------------------------------------------------
#if 0
std::auto_ptr<ISubtree> TreeManager::loadNodeFromSession( AbsolutePath const& _aAbsoluteSubtreePath,
                                                     const vos::ORef < OOptions >& _xOptions,
                                                     sal_Int16 _nMinLevels)  CFG_UNO_THROW_ALL()
{
    TreeInfo* pInfo = this->requestTreeInfo(_xOptions,true /*create TreeInfo*/);

    CFG_TRACE_INFO_NI("cache manager: cache miss. going to load the node");
    rtl::Reference< OTreeLoader > xLoader = pInfo->getNewLoaderWithoutPending(_aAbsoluteSubtreePath, _nMinLevels, _xOptions, m_xBackend.get());

    OSL_ENSURE(xLoader.is(), "Did not receive a loader for retrieving the node");
    CFG_TRACE_INFO_NI("cache manager: cache miss. going to load the node");
    if (!xLoader.is())
        throw container::NoSuchElementException((::rtl::OUString::createFromAscii("Error while retrieving the node")), NULL);

    // now block for reading
    std::auto_ptr<ISubtree> pResponse;
    try
    {
        pResponse = xLoader->waitToResponse();
    }
    catch (uno::Exception& e)
    {
        pInfo->releaseLoader(xLoader);
        throw;
    }

    pInfo->releaseLoader(xLoader);

    return pResponse;
}
#endif
// -----------------------------------------------------------------------------

class OInvalidateTreeThread: public vos::OThread
{
    typedef backend::ICachedDataProvider CacheManager;
    typedef rtl::Reference< CacheManager > CacheManagerRef;
    CacheManagerRef     m_rTreeManager;
    Name                m_aComponentName;
    RequestOptions      m_aOptions;

public:
    OInvalidateTreeThread(CacheManager* _rTreeManager,
                          Name const & _aComponentName,
                          const RequestOptions& _aOptions)
    : m_rTreeManager(_rTreeManager)
    , m_aComponentName(_aComponentName)
    , m_aOptions(_aOptions)
    {}

    ~OInvalidateTreeThread()
    {}

private:
    virtual void SAL_CALL onTerminated()
    {
        delete this;
    }

    virtual void SAL_CALL run();
};

// -----------------------------------------------------------------------------

void CacheController::invalidateComponent(ComponentRequest const & _aComponent) CFG_UNO_THROW_ALL(  )
{
    if (!this->m_bDisposing)
    {
        // start the InvalidateTreeThread only, if we are not at disposemode
        if (OInvalidateTreeThread *pThread =
            new OInvalidateTreeThread(this, _aComponent.getComponentName(), _aComponent.getOptions()))
        {
            pThread->create();
        }
        else
            OSL_ENSURE(false, "Could not create refresher thread");
    }

}

// -----------------------------------------------------------------------------

CacheLocation CacheController::refreshComponent(ComponentRequest const & _aRequest) CFG_UNO_THROW_ALL()
{
    if (m_bDisposing) return NULL;

    CacheRef aCache = this->getCacheAlways(_aRequest.getOptions());

    if (!aCache.is()) return NULL;

    // load the Node direct from the session, without using the cache
    ComponentRequest aForcedRequest(_aRequest);
    aForcedRequest.forceReload();

    ComponentResult aLoadedInstance = this->getComponentData(aForcedRequest,false);
    AbsolutePath aRequestPath = AbsolutePath::makeModulePath(_aRequest.getComponentName(), AbsolutePath::NoValidate());
    NodeInstance aNodeInstance(aLoadedInstance.mutableInstance().mutableData(),aRequestPath) ;
    NodeResult aLoadedNodeInstance(aNodeInstance) ;

    data::TreeAddress aResult = NULL;
    if (aLoadedNodeInstance.is())
    {
        Name aModuleName = aLoadedNodeInstance->root().getModuleName();

        bool bAcquired = aCache->acquireModule(aModuleName);
        aResult = CacheLocation( aCache->getTreeAddress(aModuleName) );

        if (bAcquired)
        try
    {
            std::auto_ptr<SubtreeChange> aTreeChanges;
            data::NodeAddress aRootAddress;

            {
                data::TreeAccessor aTreeAccess(aResult);
                data::NodeAccess aRootNode = aTreeAccess.getRootNode();

                aTreeChanges = createDiffs(aRootNode, aLoadedNodeInstance->data().get(), aLoadedNodeInstance->root().location());
                aRootAddress = aRootNode;
            }

            if (aTreeChanges.get() != NULL)
            {
        // change all Values... found in the Subtree in the CacheTree
        applyUpdateWithAdjustmentToTree(*aTreeChanges, aRootAddress);

                UpdateRequest anUpdateReq(  aTreeChanges.get(),
                                            aLoadedNodeInstance->root().location(),
                                            _aRequest.getOptions()
                                          );

                m_aNotifier.notifyChanged(anUpdateReq);
            }
        aCache->releaseModule(aModuleName);
        }
        catch (...)
        {
        aCache->releaseModule(aModuleName);
            throw;
        }
    }
    return aResult;
}

// -----------------------------------------------------------------------------
void OInvalidateTreeThread::run()
{
    try
    {
        UnoApiLock aLock;
        ComponentRequest aRequest(m_aComponentName, m_aOptions);
        m_rTreeManager->refreshComponent(aRequest);
    }
    catch(uno::Exception&)
    {
        // do nothing, only thread safe exception absorb
        CFG_TRACE_ERROR_NI("OInvalidateTreeThread::run: refreshing failed - ignoring the exception");
    }
}
// -----------------------------------------------------------------------------
    } // namespace backend

// -----------------------------------------------------------------------------
} // namespace configmgr

