/*************************************************************************
 *
 *  $RCSfile: invalidatetree.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:42 $
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

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#ifndef CONFIGMGR_UPDATEHELPER_HXX
#include "updatehelper.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif

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
/*
std::auto_ptr<ISubtree> TreeManager::loadNodeFromSession( AbsolutePath const& _aAbsoluteSubtreePath,
                                                     const vos::ORef < OOptions >& _xOptions,
                                                     sal_Int16 _nMinLevels)  CFG_UNO_THROW_ALL()
{
    TreeInfo* pInfo = this->requestTreeInfo(_xOptions,true /*create TreeInfo* /);

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
*/
// -----------------------------------------------------------------------------

class OInvalidateTreeThread: public vos::OThread
{
    typedef CacheController CacheManager;
    RequestOptions      m_aOptions;
    CacheManager&       m_rTreeManager;
    Name                m_aComponentName;

public:
    OInvalidateTreeThread(CacheManager& _rTreeManager,  Name const & _aComponentName,
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
            new OInvalidateTreeThread(*this, _aComponent.getComponentName(), _aComponent.getOptions()))
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
    if (m_bDisposing) return CacheLocation();

    CacheRef aCache = m_aCacheList.get(_aRequest.getOptions());
    if (!aCache.is()) return CacheLocation();

    // load the Node direct from the session, without using the cache
    ComponentRequest aForcedRequest(_aRequest);
    aForcedRequest.forceReload();

    ComponentResult aLoadedInstance = this->getComponentData(aForcedRequest);
    AbsolutePath aRequestPath = AbsolutePath::makeModulePath(_aRequest.getComponentName(), AbsolutePath::NoValidate());
    NodeInstance aNodeInstance(aLoadedInstance.mutableInstance().mutableData(),aRequestPath) ;
    NodeResult aLoadedNodeInstance(aNodeInstance) ;

    CacheLocation aResult;
    if (aLoadedInstance.is())
    {
        Name aModuleName = aLoadedNodeInstance->root().getModuleName();

        memory::UpdateAccessor aChangingAccessor( aCache->getDataSegment(aModuleName) );
        OSL_ENSURE(aChangingAccessor.is(), "No existing cache line for tree being refreshed");

        data::TreeAddress aCachedTreeAddress = aCache->acquireModule(aModuleName);

        aResult.segment = aCache->getDataSegmentAddress(aModuleName);
        aResult.address = aCachedTreeAddress.addressValue();

        if (aCachedTreeAddress.is())
        try
        {
            std::auto_ptr<SubtreeChange> aTreeChanges;
            data::NodeAddress aRootAddress;

            {
                data::TreeAccessor aTreeAccess(aChangingAccessor.accessor(),aCachedTreeAddress);
                data::NodeAccess aRootNode = aTreeAccess.getRootNode();

                aTreeChanges = createDiffs(aRootNode, aLoadedNodeInstance->data().get(), aLoadedNodeInstance->root().location());
                aRootAddress = aRootNode.address();
            }

            if (aTreeChanges.get() != NULL)
            {
                // change all Values... found in the Subtree in the CacheTree
                applyUpdateWithAdjustmentToTree(*aTreeChanges, aChangingAccessor, aRootAddress);

                data::Accessor aNotifyLock = aChangingAccessor.downgrade(); // keep a read lock during notification

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
        ComponentRequest aRequest(m_aComponentName, m_aOptions);
        m_rTreeManager.refreshComponent(aRequest);
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

