/*************************************************************************
 *
 *  $RCSfile: invalidatetree.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jb $ $Date: 2002-02-11 13:47:55 $
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


#include "treecache.hxx"

#ifndef _CONFIGMGR_SESSION_CONFIGSESSION_HXX_
#include "configsession.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#ifndef CONFIGMGR_UPDATEHELPER_HXX
#include "updatehelper.hxx"
#endif
#ifndef INCLUDED_CONFIGMGR_MERGECHANGE_HXX
#include "mergechange.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif

#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif
#ifndef CONFIGMGR_TREEDATA_HXX
#include "treedata.hxx"
#endif
#ifndef CONFIGMGR_LOADER_HXX
#include "loader.hxx"
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

// -----------------------------------------------------------------------------
auto_ptr<TreeChangeList> createDiffs(data::NodeAccess const& _aCachedNode, ISubtree const * _pLoadedSubtree,
                                    const vos::ORef<OOptions>& _rOptions,
                                    AbsolutePath const& _aAbsoluteSubtreePath)
{
    OSL_PRECOND(_aCachedNode.isValid(), "Need an existing node to create a diff");
    OSL_PRECOND(_pLoadedSubtree != 0, "Need a result node to create a diff");
    // Create a TreeChangeList with the right name, parentname and ConfigurationProperties
    std::auto_ptr<TreeChangeList> aNewChangeList(
                                        new TreeChangeList(_rOptions, _aAbsoluteSubtreePath) );

    if (!createUpdateFromDifference(aNewChangeList->root, _aCachedNode, *_pLoadedSubtree))
        aNewChangeList.reset();

    return aNewChangeList;
}
// -----------------------------------------------------------------------------

auto_ptr<ISubtree> TreeManager::loadNodeFromSession( IConfigSession *_pSession, AbsolutePath const& _aAbsoluteSubtreePath,
                                                     const vos::ORef < OOptions >& _xOptions,
                                                     sal_Int16 _nMinLevels)  CFG_UNO_THROW_ALL()
{
    TreeInfo* pInfo = this->requestTreeInfo(_xOptions,true /*create TreeInfo*/);

    CFG_TRACE_INFO_NI("cache manager: cache miss. going to load the node");
    vos::ORef< OTreeLoader > xLoader = pInfo->getNewLoaderWithoutPending(_aAbsoluteSubtreePath, _nMinLevels, _xOptions, _pSession);

    OSL_ENSURE(xLoader.getBodyPtr(), "Did not receive a loader for retrieving the node");
    CFG_TRACE_INFO_NI("cache manager: cache miss. going to load the node");
    if (!xLoader.getBodyPtr())
        throw container::NoSuchElementException((::rtl::OUString::createFromAscii("Error while retrieving the node")), NULL);

    // start loading
    xLoader->start(this);

    // now block for reading
    std::auto_ptr<ISubtree> pResponse;
    try
    {
        pResponse = xLoader->waitToResponse();
    }
    catch (uno::Exception& e)
    {
        pInfo->releaseLoader(xLoader);
        throw e;
    }

    pInfo->releaseLoader(xLoader);

    return pResponse;
}

// -----------------------------------------------------------------------------

class OInvalidateTreeThread: public vos::OThread
{
    vos::ORef<OOptions> m_aOptions;
    TreeManager&        m_rTreeManager;
    AbsolutePath        m_aAbsoluteSubtreePath;

    virtual void SAL_CALL run();
public:
    OInvalidateTreeThread(TreeManager& _pTreeManager, const AbsolutePath &_aAbsoluteSubtreePath,
                          const vos::ORef<OOptions>& _rOptions)
            :m_rTreeManager(_pTreeManager),
             m_aAbsoluteSubtreePath(_aAbsoluteSubtreePath),
             m_aOptions(_rOptions)
        {}
    ~OInvalidateTreeThread(){}

    virtual void SAL_CALL onTerminated()
        {
            delete this;
        }
};

// -----------------------------------------------------------------------------
void TreeManager::invalidateTreeAsync(const AbsolutePath &_aAbsoluteSubtreePath, const vos::ORef<OOptions>& _rOptions) CFG_UNO_THROW_ALL()
{
    if (m_bDisposeMode == false)
    {
        // start the InvalidateTreeThread only, if we are not at disposemode
        // for correct handling

        OInvalidateTreeThread *pThread = new OInvalidateTreeThread(*this, _aAbsoluteSubtreePath, _rOptions);
        if (pThread)
        {
            pThread->create();
        }
    }
}


// -----------------------------------------------------------------------------

void TreeManager::refreshSubtree(const AbsolutePath &_aAbsoluteSubtreePath, const vos::ORef<OOptions>& _aOptions) CFG_UNO_THROW_ALL()
{
    // load the Node direct from the session, without using the cache
    auto_ptr<ISubtree> aLoadedSubtree( this->loadNodeFromSession(m_pSession, _aAbsoluteSubtreePath, _aOptions, -1) );

    if (aLoadedSubtree.get())
    {
        if (TreeInfo* pTreeInfo = this->requestTreeInfo(_aOptions, false))
        {
            memory::UpdateAccessor aChangingAccessor(pTreeInfo->getDataSegment(_aAbsoluteSubtreePath)); // todo: get from lock

            OSL_ENSURE(aChangingAccessor.is(), "No existing cache line for tree being refreshed");

            data::NodeAddress aCachedTreeAddress =
                pTreeInfo->acquireSubtreeWithDepth(aChangingAccessor.accessor(), _aAbsoluteSubtreePath, 0, 0);

            if (aCachedTreeAddress.is())
            try
            {
                auto_ptr<TreeChangeList> aTreeChanges =
                    createDiffs(data::NodeAccess(aChangingAccessor.accessor(),aCachedTreeAddress), aLoadedSubtree.get(), _aOptions, _aAbsoluteSubtreePath);

                if (aTreeChanges.get() != NULL)
                {
                    // change all Values... found in the Subtree in the CacheTree
                    applyUpdateWithAdjustment(*aTreeChanges, aChangingAccessor, aCachedTreeAddress);

                    data::Accessor aNotifyLock = aChangingAccessor.downgrade(); // keep a read lock during notification

                    this->notifyUpdate(aNotifyLock,*aTreeChanges);
                }

                this->releaseSubtree(_aAbsoluteSubtreePath, _aOptions);
            }
            catch (...)
            {
                this->releaseSubtree(_aAbsoluteSubtreePath, _aOptions);
                throw;
            }
        }
    }
}

// -----------------------------------------------------------------------------
void OInvalidateTreeThread::run()
{
    try
    {
        m_rTreeManager.refreshSubtree(m_aAbsoluteSubtreePath, m_aOptions);
    }
    catch(uno::Exception&)
    {
        // do nothing, only thread safe exception absorb
        CFG_TRACE_ERROR_NI("OInvalidateTreeThread::run: absorb exception");
    }
}
} // namespace configmgr


