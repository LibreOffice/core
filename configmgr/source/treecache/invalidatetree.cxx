/*************************************************************************
 *
 *  $RCSfile: invalidatetree.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-05 17:05:50 $
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

    struct OBuildChangeTree : NodeModification
    {
    protected:
        SubtreeChange&  m_rChangeList;
        INode*          m_pCacheNode;
        sal_Int32       m_nFlag;
    public:
        OBuildChangeTree(SubtreeChange& rList, INode* pNode, sal_Int32 _nFlag)
                :m_rChangeList(rList),
                 m_pCacheNode(pNode),
                 m_nFlag(_nFlag)
            {
            }

        virtual void handle(ValueNode& _nNode)
            {
                if (m_nFlag == 2) return;
                OUString aNodeName = _nNode.getName();
                ISubtree* pTree = m_pCacheNode->asISubtree();
                OSL_ENSURE(pTree, "OBuildChangeTree::handle : node must be a inner node!");
                if (pTree)
                {
                    INode* pChild = pTree->getChild(aNodeName);
                    ValueNode* pValueNode = pChild ? pChild->asValueNode() : NULL;
                    OSL_ENSURE(pValueNode, "OBuildChangeTree::handle : node must be a value node!");

                    // if the values differ add a new change
                    if (pValueNode && _nNode.getValue() != pValueNode->getValue())
                    {
                        ValueChange* pChange = new ValueChange(_nNode.getValue(), *pValueNode);
                        m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
                    }
                }
            }
        virtual void handle(ISubtree& _rSubtree)
            {
                OUString aNodeName = _rSubtree.getName();
                ISubtree* pSubtreeInCache = m_pCacheNode->asISubtree();
                OSL_ENSURE(pSubtreeInCache, "OBuildChangeTree::handle : node must be a inner node!");
                if (pSubtreeInCache)
                {
                    INode* pChild = pSubtreeInCache->getChild(aNodeName);
                    // node not in cache, so ignore it
                    // later, when we get additions and removements within on transaction, then we have to care about
                    if (pChild)
                    {
                        if (m_nFlag == 1)
                        {
                            ISubtree* pSubTree = pChild->asISubtree();
                            OSL_ENSURE(pSubTree, "OBuildChangeTree::handle : node must be a inner node!");
                            // generate a new change

                            SubtreeChange* pChange = new SubtreeChange(_rSubtree);
                            OBuildChangeTree aNextLevel(*pChange, pSubTree, m_nFlag);
                            aNextLevel.applyToChildren(_rSubtree);

                            // now count if there are any changes
                            OChangeCounter aCounter;
                            pChange->dispatch(aCounter);

                            if (aCounter.nCount != 0)
                                m_rChangeList.addChange(::std::auto_ptr<Change>(pChange));
                            else
                                delete pChange;
                        }
                        else
                        {
                            // Do nothing, traverse down to next change
                            ISubtree* pSubTree = pChild->asISubtree();
                            Change* pChange = m_rChangeList.getChange(aNodeName);
                            if (pChange && pChange->ISA(SubtreeChange))
                            {
                                SubtreeChange* pSubtreeChange = static_cast<SubtreeChange*>(pChange);
                                OBuildChangeTree aNextLevel(*pSubtreeChange, pSubTree, m_nFlag);
                                aNextLevel.applyToChildren(_rSubtree);
                            }
                        }
                    }
                    else
                    {
                        if(m_nFlag == 1)
                        {
                            // Subtree not in Cache, add in TreeChangeList
                            // SubtreeChange* pChange = new SubtreeChange(_rSubtree);
                            INode *pSubtree = _rSubtree.clone();
                            auto_ptr<Change> pAdd(new AddNode(auto_ptr<INode>(pSubtree), aNodeName));

                            m_rChangeList.addChange(::std::auto_ptr<Change>(pAdd));
                        }
                        else
                        {
                            // Remove Node
                            auto_ptr<Change> pRemove(new RemoveNode(aNodeName));
                            m_rChangeList.addChange(::std::auto_ptr<Change>(pRemove));
                        }
                    }
                }
            }
    };

// -----------------------------------------------------------------------------
auto_ptr<TreeChangeList> createDiffs(ISubtree* _pCachedTree, ISubtree * _pLoadedSubtree,
                                    const vos::ORef<OOptions>& _rOptions,
                                    AbsolutePath const& _aAbsoluteSubtreePath)
{
    // a new TreeChangeList, will filled with the changes between the cached tree and the new loaded tree

    // Create a TreeChangeList with the right name, parentname and ConfigurationProperties
    std::auto_ptr<TreeChangeList> aNewChangeList(
                                        new TreeChangeList(_rOptions, _aAbsoluteSubtreePath,Chg()) );

    // create the differences
    OBuildChangeTree aNewChangeTree(aNewChangeList.get()->root, _pCachedTree, 1);
    _pLoadedSubtree->forEachChild(aNewChangeTree);

    OBuildChangeTree aNewChangeTree2(aNewChangeList.get()->root, _pLoadedSubtree, 2);
    _pCachedTree->forEachChild(aNewChangeTree2);

    // MyAction aAction;
    // aAction.handle(*_pLoadedSubtree);

    return aNewChangeList;
}

// -----------------------------------------------------------------------------
void concatSubtreeWithChanges(ISubtree* _pSubtree, TreeChangeList &_aChangeList)
{
    // POST: pSubtree = pSubtree + aChangeList

    TreeUpdate aTreeUpdate(_pSubtree);
    // ISubtree *pAll =

    TreeChangeList aMergeChangeList(_aChangeList, SubtreeChange::NoChildCopy());

    OMergeTreeAction aChangeHandler(aMergeChangeList.root, _pSubtree);
    _aChangeList.root.forEachChange(aChangeHandler);

    // now check the real modifications
    OChangeActionCounter aChangeCounter;
    aChangeCounter.handle(aMergeChangeList.root);
    CFG_TRACE_INFO_NI("cache manager: counted changes from notification : additions: %i , removes: %i, value changes: %i", aChangeCounter.nAdds, aChangeCounter.nRemoves, aChangeCounter.nValues);
    if (aChangeCounter.hasChanges())
    {
        // aTree.updateTree(aMergeChangeList);
        aMergeChangeList.root.forEachChange(aTreeUpdate);
    }
}
// -----------------------------------------------------------------------------

auto_ptr<ISubtree> TreeManager::loadNodeFromSession( IConfigSession *_pSession, AbsolutePath const& _aAbsoluteSubtreePath,
                                                     const vos::ORef < OOptions >& _xOptions,
                                                     sal_Int16 _nMinLevels)  throw (uno::Exception)
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
void TreeManager::invalidateTreeAsync(const AbsolutePath &_aAbsoluteSubtreePath, const vos::ORef<OOptions>& _rOptions) throw (uno::Exception)
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

void TreeManager::refreshSubtree(const AbsolutePath &_aAbsoluteSubtreePath, const vos::ORef<OOptions>& _aOptions) throw (uno::Exception)
{
    // load the Node direct from the session, without using the cache
    auto_ptr<ISubtree> aLoadedSubtree( this->loadNodeFromSession(m_pSession, _aAbsoluteSubtreePath, _aOptions, -1) );

    if (aLoadedSubtree.get())
    {
        OClearableWriteSynchronized aWriteGuard(this);
        TreeInfo* pTreeInfo = this->requestTreeInfo(_aOptions, false);
        if (pTreeInfo != NULL)
        {
            ISubtree* pCachedTree = pTreeInfo->acquireSubtreeWithDepth(_aAbsoluteSubtreePath, 0);
            if (pCachedTree != NULL)
            {
                auto_ptr<TreeChangeList> aTreeChanges( createDiffs(pCachedTree, aLoadedSubtree.get(), _aOptions, _aAbsoluteSubtreePath) );

                // change all Values... found in the Subtree in the CacheTree
                concatSubtreeWithChanges(pCachedTree, *(aTreeChanges.get()));

                // blow away in the hole world
                // notify(pTreeChanges);

                aWriteGuard.downgrade(); // keep a read lock during notification
                this->notifyUpdate(*(aTreeChanges.get()));

                this->releaseSubtree(_aAbsoluteSubtreePath, _aOptions);
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


