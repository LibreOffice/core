/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: invalidatetree.cxx,v $
 * $Revision: 1.24 $
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
#include "tracer.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

namespace configmgr
{

    namespace container = com::sun::star::container;
// -----------------------------------------------------------------------------
// ------------------------------- invalidateTree -------------------------------
// -----------------------------------------------------------------------------

namespace backend
{
// -----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> createDiffs(sharable::Node * cachedNode,
                                            ISubtree const * _pLoadedSubtree,
                                            configuration::AbsolutePath const& _aAbsoluteSubtreePath)
{
    OSL_PRECOND(cachedNode != 0, "Need an existing node to create a diff");
    OSL_PRECOND(_pLoadedSubtree != 0, "Need a result node to create a diff");
    // Create a TreeChangeList with the right name, parentname and ConfigurationProperties
    std::auto_ptr<SubtreeChange> aNewChange(new SubtreeChange(_aAbsoluteSubtreePath.getLocalName().getName(),
                                                                node::Attributes()) );

    if (!createUpdateFromDifference(*aNewChange, cachedNode, *_pLoadedSubtree))
        aNewChange.reset();

    return aNewChange;
}
// -----------------------------------------------------------------------------
#if 0
std::auto_ptr<ISubtree> TreeManager::loadNodeFromSession( configuration::AbsolutePath const& _aAbsoluteSubtreePath,
                                                     const vos::ORef < OOptions >& _xOptions,
                                                     sal_Int16 _nMinLevels)  SAL_THROW((com::sun::star::uno::Exception))
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

sharable::TreeFragment * CacheController::refreshComponent(ComponentRequest const & _aRequest) SAL_THROW((com::sun::star::uno::Exception))
{
    if (m_bDisposing) return NULL;

    rtl::Reference<CacheLoadingAccess> aCache = this->getCacheAlways(_aRequest.getOptions());

    if (!aCache.is()) return NULL;

    // load the Node direct from the session, without using the cache
    ComponentRequest aForcedRequest(_aRequest);
    aForcedRequest.forceReload();

    ResultHolder< ComponentInstance > aLoadedInstance = this->getComponentData(aForcedRequest,false);
    configuration::AbsolutePath aRequestPath = configuration::AbsolutePath::makeModulePath(_aRequest.getComponentName());
    NodeInstance aNodeInstance(aLoadedInstance.mutableInstance().mutableData(),aRequestPath) ;
    ResultHolder< NodeInstance > aLoadedNodeInstance(aNodeInstance) ;

    sharable::TreeFragment * aResult = NULL;
    if (aLoadedNodeInstance.is())
    {
        rtl::OUString aModuleName = aLoadedNodeInstance->root().getModuleName();

        bool bAcquired = aCache->acquireModule(aModuleName);
        aResult = (sharable::TreeFragment *)( aCache->getTreeAddress(aModuleName) );

        if (bAcquired)
        try
    {
            std::auto_ptr<SubtreeChange> aTreeChanges;
            sharable::Node * aRootAddress;

            {
                sharable::Node * rootNode = aResult == 0 ? 0 : aResult->getRootNode();

                aTreeChanges = createDiffs(rootNode, aLoadedNodeInstance->data().get(), aLoadedNodeInstance->root());
                aRootAddress = rootNode;
            }

            if (aTreeChanges.get() != NULL)
            {
        // change all Values... found in the Subtree in the CacheTree
        applyUpdateWithAdjustmentToTree(*aTreeChanges, aRootAddress);

                UpdateRequest anUpdateReq(  aTreeChanges.get(),
                                            aLoadedNodeInstance->root(),
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
    } // namespace backend

// -----------------------------------------------------------------------------
} // namespace configmgr

