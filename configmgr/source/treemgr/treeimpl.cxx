/*************************************************************************
 *
 *  $RCSfile: treeimpl.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-20 11:01:51 $
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
#include <stdio.h>
#include "treeimpl.hxx"
#include "roottreeimpl.hxx"

#include "nodeimpl.hxx"
#include "nodechangeimpl.hxx"
#include "template.hxx"

#include "valuenode.hxx"
#include "change.hxx"

#include "valuenodeimpl.hxx"
#include "setnodeimplbase.hxx"
#include "groupnodeimpl.hxx"

#include <algorithm> // for reverse
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
inline
static
Name nodeName(INode const& aNode)
{
    return makeName(aNode.getName(),Name::NoValidate());
}
//-----------------------------------------------------------------------------
// class TreeImplBuilder - friend of TreeImpl
//-----------------------------------------------------------------------------

/** is a visitor-style algorithm to construct a <type>TreeImpl::NodeList</type>
    representing a configuration hierarchy
*/
class TreeImplBuilder : public NodeModification
{
public:
    /** constructs a TreeImplBuilder to append onto <var>rList</var>
        the products of <var>rFactory</var> up to depth <var>nDepth</var>
    */
    TreeImplBuilder(TemplateProvider const& aTemplateProvider, NodeFactory& rFactory, TreeImpl& rTree)
        : m_aTemplateProvider(aTemplateProvider)
        , m_rFactory(rFactory)
        , m_rTree(rTree)
        , m_nParent(0)
        , m_nDepthLeft(rTree.m_nDepth)
    {
        OSL_ASSERT(m_rTree.m_aNodes.empty());
        OSL_DEBUG_ONLY(m_bMemberCheck = false);
    }

private:
    /// implements the NodeModification handler for ValueNodes
    virtual void handle(ValueNode& rValue);
    /// implements the NodeModification member for ValueNodes
    virtual void handle(ISubtree& rTree);

    /// add a Node for group node <var>rGroup</var> to the list
    void addGroup(ISubtree& rGroup);
    /// add a Node for set node <var>rSet</var> to the list
    void addSet(ISubtree& rSet);
    /// add a Node for value node <var>rValue</var> to the list
    void addValueElement(ValueNode& rValue);
    /// add a Member for value node <var>rValue</var> to the list
    void addValueMember(ValueNode& rValue);

    TemplateProvider m_aTemplateProvider;
    NodeFactory&    m_rFactory;
    TreeImpl&       m_rTree;
    NodeOffset      m_nParent;
    TreeDepth       m_nDepthLeft;
#ifdef _DEBUG
    bool m_bMemberCheck;
#endif
};
//-----------------------------------------------------------------------------

void TreeImplBuilder::handle(ValueNode& rValue)
{
    if (m_nParent == 0)
        addValueElement(rValue); // if it is the root it is a value set element
    else
        addValueMember(rValue); // if it is not the root it is a group member
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::handle(ISubtree& rTree)
{
    if (rTree.isSetNode())
        addSet(rTree);
    else
        addGroup(rTree);
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addValueElement(ValueNode& rValue)
{
    NodeImplHolder aValueNode( m_rFactory.makeValueNode(rValue) );
    OSL_ENSURE( aValueNode.isValid(), "could not make value node wrapper" );

    OSL_ENSURE( m_nParent == 0, "Adding value element that is not root of its fragment" );
    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aValueNode.isValid() )
    {
        m_rTree.m_aNodes.push_back( Node(aValueNode,nodeName(rValue),m_nParent) );
    }
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addValueMember(ValueNode& )
{
    // nothing to do
    OSL_DEBUG_ONLY(m_bMemberCheck = true);
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addGroup(ISubtree& rTree)
{
    NodeImplHolder aGroupNode( m_rFactory.makeGroupNode(rTree) );
    OSL_ENSURE( aGroupNode.isValid(), "could not make group node wrapper" );

    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aGroupNode.isValid() )
    {
        m_rTree.m_aNodes.push_back( Node(aGroupNode,nodeName(rTree),m_nParent) );

        // now fill in group members
        if (m_nDepthLeft > 0)
        {
            NodeOffset nSaveParent = m_nParent;
            decDepth(m_nDepthLeft);

            m_nParent = m_rTree.m_aNodes.size() + m_rTree.root() - 1;

        #ifdef _DEBUG
            bool bSaveMemberCheck = m_bMemberCheck;
            m_bMemberCheck = false;
        #endif

            // now recurse:
            this->applyToChildren(rTree);

            OSL_ENSURE(m_nParent < m_rTree.m_aNodes.size() || m_bMemberCheck,
                        "WARNING: Configuration: Group within requested depth has no members");

            OSL_DEBUG_ONLY(m_bMemberCheck = bSaveMemberCheck);

            incDepth(m_nDepthLeft);
            m_nParent = nSaveParent;
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addSet(ISubtree& rTree)
{
    TemplateHolder aTemplate = makeSetElementTemplate(rTree, m_aTemplateProvider);
    OSL_ASSERT(aTemplate.isValid());
    OSL_ENSURE(aTemplate->isInstanceTypeKnown(),"ERROR: Cannor create set instance without knowing the instance type");

    NodeImplHolder aSetNode( m_rFactory.makeSetNode(rTree,aTemplate.getBodyPtr()) );
    OSL_ENSURE( aSetNode.isValid(), "could not make set node wrapper" );

    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aSetNode.isValid() )
    {
        m_rTree.m_aNodes.push_back( Node(aSetNode,nodeName(rTree),m_nParent) );

        // this also relies on one based offsets
        NodeOffset nNodeAdded = m_rTree.m_aNodes.size() + m_rTree.root() - 1;

        m_rTree.m_aNodes.back().setImpl().initElements(m_aTemplateProvider, m_rTree, nNodeAdded, m_nDepthLeft);
    }
}

//-----------------------------------------------------------------------------
// class Node
//-----------------------------------------------------------------------------

Node::Node(NodeImplHolder const& aSpecificNode, Name const& aName, NodeOffset nParent)
: m_pSpecificNode(aSpecificNode)
, m_nParent(nParent)
, m_aName(aName)
{
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class TreeImpl
//-----------------------------------------------------------------------------

/// creates a TreeImpl for a detached, virgin instance of <var>aTemplate</var>
TreeImpl::TreeImpl( )
: m_aNodes()
, m_pParentTree(0)
, m_nParentNode(0)
, m_nDepth(0)
//, m_nRoot(nRoot)
{
}
//-----------------------------------------------------------------------------

TreeImpl::TreeImpl( TreeImpl& rParentTree, NodeOffset nParentNode )
: m_aNodes()
, m_pParentTree(&rParentTree)
, m_nParentNode(nParentNode)
, m_nDepth(0)
//, m_nRoot(1)
{
}
//-----------------------------------------------------------------------------

TreeImpl::~TreeImpl()
{
}
//-----------------------------------------------------------------------------

void TreeImpl::disposeData()
{
    m_aNodes.clear();
}
//-----------------------------------------------------------------------------

void ElementTreeImpl::doFinishRootPath(Path::Rep& rPath) const
{
    rPath.prepend( doGetRootName() );
    rPath.prepend( AbsolutePath::detachedRoot().rep() );
}
//-----------------------------------------------------------------------------

void RootTreeImpl::doFinishRootPath(Path::Rep& rPath) const
{
    rPath.prepend( m_aRootPath.rep() );
}

//-----------------------------------------------------------------------------

void TreeImpl::implPrependRootPath(Path::Rep& rPath) const
{
    if (m_pParentTree)
    {
        rPath.prepend( doGetRootName() );
        OSL_ASSERT(m_nParentNode);
        m_pParentTree->prependLocalPathTo(m_nParentNode,rPath);
        m_pParentTree->implPrependRootPath(rPath);
    }
    else
    {
        doFinishRootPath( rPath );
    }
}
//-----------------------------------------------------------------------------

AbsolutePath TreeImpl::getRootPath() const
{
    Path::Rep aPath;
    implPrependRootPath(aPath);
    return AbsolutePath(aPath);
}
//-----------------------------------------------------------------------------
void TreeImpl::build(NodeFactory& rFactory, INode& rCacheNode, TreeDepth nDepth, TemplateProvider const& aTemplateProvider)
{
    OSL_ASSERT(m_aNodes.empty());
    m_nDepth = nDepth;
    TreeImplBuilder a(aTemplateProvider, rFactory,*this);
    a.applyToNode(rCacheNode);
}
//-----------------------------------------------------------------------------

// context handling
//-----------------------------------------------------------------------------

void TreeImpl::setContext(TreeImpl* pParentTree, NodeOffset nParentNode)
{
    OSL_ENSURE(pParentTree,"ERROR: Moving tree to nowhere");

    if (pParentTree)
    {
        OSL_ENSURE( pParentTree->isValidNode(nParentNode),"ERROR: Moving tree to invalid node");
        if (!pParentTree->isValidNode(nParentNode))
            throw Exception("INTERNAL ERROR: Moving tree to invalid parent node");

        OSL_ENSURE( pParentTree->node(nParentNode)->isSetNode(),"WARNING: Moving tree to node that is not a set");
    }
    else
    {
        OSL_ENSURE( nParentNode == 0,"WARNING: Moving tree to node without a tree");
        nParentNode = 0;
    }

    m_pParentTree = pParentTree;
    m_nParentNode = nParentNode;
}
//-----------------------------------------------------------------------------

void TreeImpl::clearContext()
{
    m_pParentTree = 0;
    m_nParentNode = 0;
}
//-----------------------------------------------------------------------------

bool TreeImpl::hasChanges() const
{
    return node(root())->hasChanges();
}
//-----------------------------------------------------------------------------

void TreeImpl::collectChanges(NodeChanges& rChanges)
{
    implCollectChangesFrom(root(),rChanges);
}
//-----------------------------------------------------------------------------

void TreeImpl::implCollectChangesFrom(NodeOffset nNode, NodeChanges& rChanges)
{
    Node const* pNode = node(nNode);
    if (pNode->hasChanges())
    {
        pNode->collectChanges(rChanges,this,nNode);
        for (NodeOffset nChild = firstChild(nNode); isValidNode(nChild); nChild = findNextChild(nNode, nChild) )
        {
            implCollectChangesFrom(nChild,rChanges);
        }
    }
}
//-----------------------------------------------------------------------------
// mark the given node and all its ancestors (we can stop when we hit a node that already is marked)
void TreeImpl::markChanged(NodeOffset nNode)
{
    OSL_ASSERT(isValidNode(nNode));

    if (nNode)
    {
        do
        {
            Node* pNode = node(nNode);
            pNode->markChanged();

            nNode = pNode->parent();
        }
        while (nNode && !node(nNode)->hasChanges());
    }

    if (nNode == 0) // just marked parent
    {
        TreeImpl* pContext = getContextTree();
        NodeOffset nContext = getContextNode();
        if (pContext)
        {
            OSL_ASSERT(pContext->isValidNode(nContext));
            pContext->markChanged(nContext);
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::commitDirect()
{
    implCommitDirectFrom(root());
}
//-----------------------------------------------------------------------------

void TreeImpl::implCommitDirectFrom(NodeOffset nNode)
{
    Node* pNode = node(nNode);
    if (pNode->hasChanges())
    {
        pNode->commitDirect();
        for (NodeOffset nChild = firstChild(nNode); isValidNode(nChild); nChild = findNextChild(nNode, nChild) )
        {
            implCommitDirectFrom(nChild);
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::makeIndirect(bool bIndirect)
{
    // do it from outside in
    for(NodeList::reverse_iterator it = m_aNodes.rbegin(), stop = m_aNodes.rend();
        it != stop;
        ++it)
        it->makeIndirect(bIndirect);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// old-style commit handling
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> TreeImpl::preCommitChanges(ElementList& _rRemovedElements)
{
    return doCommitChanges( _rRemovedElements, root() );
}
//-----------------------------------------------------------------------------

void TreeImpl::finishCommit(SubtreeChange& rRootChange)
{
    doFinishCommit( rRootChange, root() );
}
//-----------------------------------------------------------------------------

void TreeImpl::revertCommit(SubtreeChange& rRootChange)
{
    doRevertCommit( rRootChange, root() );
}
//-----------------------------------------------------------------------------

void TreeImpl::recoverFailedCommit(SubtreeChange& rRootChange)
{
    doFailedCommit( rRootChange, root() );
}
//-----------------------------------------------------------------------------

void TreeImpl::adjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& aExternalChange)
{
    OSL_PRECOND( getSimpleRootName().toString() == aExternalChange.getNodeName(), "Name of change does not match actual node" );

    TreeDepth nDepth = getAvailableDepth();

    doAdjustToChanges(rLocalChanges,aExternalChange,root(), nDepth);
}
//-----------------------------------------------------------------------------

void TreeImpl::adjustToChanges(NodeChangesInformation& rLocalChanges, NodeOffset nNode, SubtreeChange const& aExternalChange)
{
    OSL_PRECOND( isValidNode(nNode), "ERROR: Valid node required for adjusting to changes" );
    OSL_PRECOND( getSimpleNodeName(nNode).toString() == aExternalChange.getNodeName(), "Name of change does not match actual node" );

    TreeDepth nDepth = remainingDepth(getAvailableDepth(),depthTo(nNode));

    doAdjustToChanges(rLocalChanges,aExternalChange,nNode, nDepth);
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> TreeImpl::doCommitChanges(ElementList& _rRemovedElements, NodeOffset nNode)
{
    OSL_ASSERT(isValidNode(nNode));
    Node* pNode = node(nNode);

    std::auto_ptr<SubtreeChange> aRet;

    if (!pNode->hasChanges())
    {
        // do nothing
        OSL_ASSERT(!aRet.get());
        OSL_ASSERT(nNode == root()); // only hit this for root (external request)
    }
    else if (pNode->isSetNode())
    {
        aRet = pNode->setImpl().preCommitChanges(_rRemovedElements);
    }
    else if (pNode->isGroupNode())
    {
        std::auto_ptr<SubtreeChange> aGroupChange(pNode->groupImpl().preCommitChanges());

        OSL_ASSERT(aGroupChange.get());
        if (aGroupChange.get())
            doCommitSubChanges(_rRemovedElements, *aGroupChange, nNode);

        aRet = aGroupChange;
    }
    else
        OSL_ENSURE(nNode == root() && pNode->isValueElementNode(), "TreeImpl: Cannot commit changes: Unexpected node type");

    return aRet;
}
//-----------------------------------------------------------------------------

void TreeImpl::doFinishCommit(SubtreeChange& rSubtreeChange, NodeOffset nNode)
{
    OSL_ASSERT(isValidNode(nNode));
    Node* pNode = node(nNode);

    OSL_ENSURE(rSubtreeChange.getNodeName() == getSimpleNodeName(nNode).toString(), "ERROR: Change name does not match node");
    if (pNode->isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        pNode->setImpl().finishCommit(rSubtreeChange);
    }
    else if (pNode->isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        pNode->groupImpl().finishCommit(rSubtreeChange);
        doFinishSubCommitted( rSubtreeChange, nNode);
    }
    else
        OSL_ENSURE(nNode == root() && pNode->isValueElementNode(), "TreeImpl: Cannot finish commit: Unexpected node type");
}
//-----------------------------------------------------------------------------

void TreeImpl::doRevertCommit(SubtreeChange& rSubtreeChange, NodeOffset nNode)
{
    OSL_ASSERT(isValidNode(nNode));
    Node* pNode = node(nNode);

    OSL_ENSURE(rSubtreeChange.getNodeName() == getSimpleNodeName(nNode).toString(), "ERROR: Change name does not match node");
    if (pNode->isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        pNode->setImpl().revertCommit(rSubtreeChange);
    }
    else if (pNode->isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        pNode->groupImpl().revertCommit(rSubtreeChange);
        doRevertSubCommitted( rSubtreeChange, nNode);
    }
    else
        OSL_ENSURE(nNode == root() && pNode->isValueElementNode(), "TreeImpl: Cannot revert commit: Unexpected node type");
}
//-----------------------------------------------------------------------------

void TreeImpl::doFailedCommit(SubtreeChange& rSubtreeChange, NodeOffset nNode)
{
    OSL_ASSERT(isValidNode(nNode));
    Node* pNode = node(nNode);

    OSL_ENSURE(rSubtreeChange.getNodeName() == getSimpleNodeName(nNode).toString(), "ERROR: Change name does not match node");
    if (pNode->isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        pNode->setImpl().failedCommit(rSubtreeChange);
    }
    else if (pNode->isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        pNode->groupImpl().failedCommit(rSubtreeChange);
        doFailedSubCommitted( rSubtreeChange, nNode);
    }
    else
        OSL_ENSURE(nNode == root() && pNode->isValueElementNode(), "TreeImpl: Cannot finish commit: Unexpected node type");
}
//-----------------------------------------------------------------------------

void TreeImpl::doAdjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rSubtreeChange, NodeOffset nNode, TreeDepth nDepth)
{
    OSL_ASSERT(isValidNode(nNode));
    Node* pNode = node(nNode);

    OSL_ENSURE(rSubtreeChange.getNodeName() == getSimpleNodeName(nNode).toString(), "ERROR: Change name does not match node");

    if (pNode->isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        pNode->setImpl().adjustToChanges(rLocalChanges, rSubtreeChange, nDepth);
    }
    else if (pNode->isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        pNode->groupImpl().adjustToChanges(rLocalChanges, rSubtreeChange, *this, nNode);
        doAdjustToSubChanges( rLocalChanges, rSubtreeChange, nNode, nDepth);
    }
    else // might occur on external change (?)
    {
        OSL_ENSURE(pNode->isValueElementNode(), "TreeImpl: Unknown node type to adjust to changes");

        OSL_ENSURE(nNode == root(), "TreeImpl: Unexpected node type - non-root value element");

        OSL_ENSURE(false,"ERROR: Change type does not match node: Trying to apply subtree change to value element.");
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::doCommitSubChanges(ElementList& _rRemovedElements, SubtreeChange& aChangesParent, NodeOffset nParentNode)
{
    for(NodeOffset nNode = firstChild(nParentNode); nNode != 0; nNode = findNextChild(nParentNode,nNode) )
    {
        if (node(nNode)->hasChanges())
        {
            std::auto_ptr<SubtreeChange> aSubChanges( doCommitChanges(_rRemovedElements, nNode) );
            std::auto_ptr<Change> aSubChangesBase( aSubChanges.release() );
            aChangesParent.addChange( aSubChangesBase );
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::doFinishSubCommitted(SubtreeChange& aChangesParent, NodeOffset nParentNode)
{
    for(SubtreeChange::MutatingChildIterator
            it = aChangesParent.begin_changes(),
            stop = aChangesParent.end_changes();
        it != stop;
        ++it)
    {
        if ( it->ISA(SubtreeChange) )
        {
            NodeOffset nNode = findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( nNode != 0, "Changed sub-node not found in tree");

            doFinishCommit(static_cast<SubtreeChange&>(*it),nNode);
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for child of group node; change is ignored");
            OSL_ENSURE(0 == findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate())),
                        "Found sub(tree) node where a value was expected");
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::doRevertSubCommitted(SubtreeChange& aChangesParent, NodeOffset nParentNode)
{
    for(SubtreeChange::MutatingChildIterator
            it = aChangesParent.begin_changes(),
            stop = aChangesParent.end_changes();
        it != stop;
        ++it)
    {
        if ( it->ISA(SubtreeChange) )
        {
            NodeOffset nNode = findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( nNode != 0, "Changed sub-node not found in tree");

            doRevertCommit(static_cast<SubtreeChange&>(*it),nNode);
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for child of group node; change is ignored");
            OSL_ENSURE(0 == findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate())),
                        "Found sub(tree) node where a value was expected");
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::doFailedSubCommitted(SubtreeChange& aChangesParent, NodeOffset nParentNode)
{
    for(SubtreeChange::MutatingChildIterator
            it = aChangesParent.begin_changes(),
            stop = aChangesParent.end_changes();
        it != stop;
        ++it)
    {
        if ( it->ISA(SubtreeChange) )
        {
            NodeOffset nNode = findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( nNode != 0, "Changed node not found in tree");

            doFailedCommit(static_cast<SubtreeChange&>(*it),nNode);
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for child of group node; change is ignored");
            OSL_ENSURE(0 == findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate())),
                        "Found sub(tree) node where a value was expected");
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImpl::doAdjustToSubChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& aChangesParent, NodeOffset nParentNode,
                                    TreeDepth nDepth)
{
    for(SubtreeChange::ChildIterator
            it = aChangesParent.begin(),
            stop = aChangesParent.end();
        it != stop;
        ++it)
    {
        if ( it->ISA(SubtreeChange) )
        {
            NodeOffset nNode = findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( nNode != 0 || depthTo(nParentNode) >= getAvailableDepth(), "Changed node not found in tree");

            if (nNode != 0)
            {
                OSL_ENSURE( nDepth > 0, "Depth is smaller than expected for tree");
                doAdjustToChanges(rLocalChanges, static_cast<SubtreeChange const&>(*it),nNode,childDepth(nDepth));
            }
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for child of group node; change is ignored");
            OSL_ENSURE(0 == findChild(nParentNode, makeNodeName(it->getNodeName(), Name::NoValidate())),
                        "Found sub(tree) node where a value was expected");
        }
    }
}
//-----------------------------------------------------------------------------

// Node Collection navigation
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::parent(NodeOffset nNode) const
{
    OSL_ASSERT(isValidNode(nNode));
    return node(nNode)->parent();
}
//-----------------------------------------------------------------------------
inline // is protected and should be used only here
Name TreeImpl::implGetOriginalName(NodeOffset nNode) const
{
    OSL_ASSERT(isValidNode(nNode));

    return node(nNode)->m_aName;
}
//-----------------------------------------------------------------------------

Path::Component ElementTreeImpl::doGetRootName() const
{
    return makeExtendedName( implGetOriginalName( root() ) );
}

//-----------------------------------------------------------------------------

Path::Component RootTreeImpl::doGetRootName() const
{
    return m_aRootPath.getLocalName();
}
//-----------------------------------------------------------------------------


Name TreeImpl::getSimpleNodeName(NodeOffset nNode) const
{
    if (nNode == root()) return getSimpleRootName();

    return implGetOriginalName(nNode);
}
//-----------------------------------------------------------------------------

Name TreeImpl::getSimpleRootName() const
{
    return doGetRootName().getName();
}
//-----------------------------------------------------------------------------

Path::Component TreeImpl::getExtendedRootName() const
{
    return doGetRootName();
}
//-----------------------------------------------------------------------------

TreeDepth TreeImpl::depthTo(NodeOffset nNode) const
{
    OSL_ASSERT(isValidNode(nNode));

    TreeDepth nDepth = 0;
    while( 0 != (nNode=parent(nNode)) )
    {
        ++nDepth;
    }

    return nDepth;
}
//-----------------------------------------------------------------------------

void TreeImpl::prependLocalPathTo(NodeOffset nNode, Path::Rep& rNames)
{
    OSL_ASSERT(isValidNode(nNode));

    for (; nNode != root(); nNode = parent(nNode) )
    {
        OSL_ENSURE( isValidNode(nNode), "ERROR: Configuration: node has invalid parent");
        rNames.prepend( Path::wrapSimpleName( implGetOriginalName(nNode) ) );
    }

    OSL_ASSERT(nNode == root());
}
//-----------------------------------------------------------------------------

// Node iteration and access
NodeOffset TreeImpl::firstChild (NodeOffset nParent) const
{
    return findNextChild(nParent,nParent);
}
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::nextSibling(NodeOffset nNode) const
{
    return findNextChild(parent(nNode),nNode);
}
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::findNextChild(NodeOffset nParent, NodeOffset nStartAfter) const
{
    OSL_ASSERT(isValidNode(nParent));
    OSL_ASSERT(nStartAfter == 0 || isValidNode(nStartAfter));

    NodeOffset nPos = nStartAfter ? nStartAfter : root()-1;
    NodeOffset const nAfterLast = nodeCount() + root();
    while (++nPos < nAfterLast)
    {
        if(parent(nPos) == nParent) return nPos;
    }
    return 0;
}
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::findChild(NodeOffset nParent, Name const& aName) const
{
    OSL_ASSERT(isValidNode(nParent));

    NodeOffset nPos = nParent;
    NodeOffset const nAfterLast = nodeCount() + root();
    while (++nPos < nAfterLast)
    {
        if(parent(nPos) == nParent && implGetOriginalName(nPos) == aName)
            return nPos;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// locking
//-----------------------------------------------------------------------------
ISynchronizedData* TreeImpl::getRootLock()
{
    if ( m_pParentTree )
        return m_pParentTree->getRootLock();
    else
        return this;
}

ISynchronizedData const* TreeImpl::getRootLock() const
{
    if ( m_pParentTree )
        return m_pParentTree->getRootLock();
    else
        return this;
}

//-----------------------------------------------------------------------------

void TreeImpl::acquireReadAccess() const
{
    m_aOwnLock.acquireReadAccess();
}
//-----------------------------------------------------------------------------
void TreeImpl::releaseReadAccess() const
{
    m_aOwnLock.releaseReadAccess();
}
//-----------------------------------------------------------------------------
void TreeImpl::acquireWriteAccess()
{
    m_aOwnLock.acquireWriteAccess();
}
//-----------------------------------------------------------------------------
void TreeImpl::releaseWriteAccess()
{
    m_aOwnLock.releaseWriteAccess();
}

//-----------------------------------------------------------------------------
// dynamic-casting
//-----------------------------------------------------------------------------

ElementTreeImpl * TreeImpl::asElementTree()
{
    ElementTreeImpl const* const pResult = doCastToElementTree();
    TreeImpl        const* const pTest = pResult;
    if (pTest== this)
        return const_cast<ElementTreeImpl *>(pResult);
    else
        return 0;
}
//-----------------------------------------------------------------------------

RootTreeImpl * TreeImpl::asRootTree()
{
    RootTreeImpl    const* const pResult = doCastToRootTree();
    TreeImpl        const* const pTest = pResult;
    if (pTest== this)
        return const_cast<RootTreeImpl *>(pResult);
    else
        return 0;
}
//-----------------------------------------------------------------------------

ElementTreeImpl const* TreeImpl::asElementTree() const
{
    return doCastToElementTree();
}
//-----------------------------------------------------------------------------

RootTreeImpl const* TreeImpl::asRootTree() const
{
    return doCastToRootTree();
}
//-----------------------------------------------------------------------------

RootTreeImpl const* ElementTreeImpl::doCastToRootTree() const
{
    return 0;
}
//-----------------------------------------------------------------------------

ElementTreeImpl const* ElementTreeImpl::doCastToElementTree() const
{
    return this;
}
//-----------------------------------------------------------------------------

RootTreeImpl const* RootTreeImpl::doCastToRootTree() const
{
    return this;
}
//-----------------------------------------------------------------------------

ElementTreeImpl const* RootTreeImpl::doCastToElementTree() const
{
    return 0;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class RootTreeImpl
//-----------------------------------------------------------------------------

RootTreeImpl::RootTreeImpl( NodeFactory& rNodeFactory,
                            AbsolutePath const& aRootPath,
                            ISubtree& rCacheNode, TreeDepth nDepth,
                            TemplateProvider const& aTemplateProvider)
: TreeImpl()
, m_aRootPath(aRootPath)
{
    OSL_ENSURE( aRootPath.getLocalName().getName().toString() == rCacheNode.getName(),
                "Constructing root node: Path does not match node name");

    TreeImpl::build(rNodeFactory,rCacheNode,nDepth,aTemplateProvider);
}
//-----------------------------------------------------------------------------
// class ElementTreeImpl
//-----------------------------------------------------------------------------

ElementTreeImpl::ElementTreeImpl(   NodeFactory& rFactory,
                                    INode& rCacheNode, TreeDepth nDepth,
                                    TemplateHolder aTemplateInfo,
                                    TemplateProvider const& aTemplateProvider )
: TreeImpl()
, m_aInstanceInfo(aTemplateInfo)
, m_pOwnedNode(0)
{
    TreeImpl::build( rFactory, rCacheNode, nDepth, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTreeImpl::ElementTreeImpl(   NodeFactory& rFactory,
                                    TreeImpl& rParentTree, NodeOffset nParentNode,
                                    INode& rCacheNode, TreeDepth nDepth,
                                    TemplateHolder aTemplateInfo,
                                    TemplateProvider const& aTemplateProvider )
: TreeImpl( rParentTree, nParentNode )
, m_aInstanceInfo(aTemplateInfo)
, m_pOwnedNode(0)
{
    TreeImpl::build( rFactory, rCacheNode, nDepth, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTreeImpl::ElementTreeImpl(   std::auto_ptr<INode>& pNewNode,
                                    TemplateHolder aTemplate,
                                    TemplateProvider const& aTemplateProvider )
: TreeImpl()
, m_aInstanceInfo(aTemplate)
, m_pOwnedNode(0)
{
    if (!pNewNode.get())
    {
        throw Exception("ERROR: Provider can't create Element Instance From Template");
    }

    TreeImpl::build( NodeType::getDirectAccessFactory(), *pNewNode, c_TreeDepthAll, aTemplateProvider );
    m_pOwnedNode = pNewNode.release();
}
//-----------------------------------------------------------------------------

ElementTreeImpl::~ElementTreeImpl()
{
    delete m_pOwnedNode;
}
//-----------------------------------------------------------------------------

void ElementTreeImpl::disposeData()
{
    TreeImpl::disposeData();
    delete m_pOwnedNode;
    m_pOwnedNode = 0;
}
//-----------------------------------------------------------------------------

Path::Component ElementTreeImpl::makeExtendedName(Name const& _aSimpleName) const
{
    OSL_ENSURE(this->isTemplateInstance(), "ElementTree: Cannot discover the type this instantiatiates");

    Name aTypeName = this->isTemplateInstance() ? this->getTemplate()->getName() : Name();

    return Path::makeCompositeName(_aSimpleName, aTypeName);
}
//-----------------------------------------------------------------------------

// ownership handling
//-----------------------------------------------------------------------------

/// transfer ownership to the given set
void ElementTreeImpl::attachTo(ISubtree& rOwningSet, Name const& aElementName)
{
    OSL_ENSURE(m_pOwnedNode,"ERROR: Cannot add a non-owned node to a subtree");

    if (m_pOwnedNode)
    {
        OSL_ENSURE(this->getSimpleRootName() == aElementName,"ElementTree: Attaching with unexpected element name");
        m_pOwnedNode->setName(aElementName.toString());

        std::auto_ptr<INode> aNode(m_pOwnedNode);
        m_pOwnedNode = 0;

        rOwningSet.addChild(aNode);
    }
}
//-----------------------------------------------------------------------------

/// tranfer ownership from the given set
void ElementTreeImpl::detachFrom(ISubtree& rOwningSet, Name const& aElementName)
{
    OSL_ENSURE(!m_pOwnedNode,"ERROR: Cannot detach a already owned node from a subtree");
    if (!m_pOwnedNode)
    {
        OSL_ENSURE(this->getSimpleRootName() == aElementName,"ElementTree: Detaching with unexpected element name");
        std::auto_ptr<INode> aNode( rOwningSet.removeChild(aElementName.toString()) );
        OSL_ENSURE(aNode.get(),"ERROR: Detached node not found in the given subtree");

        m_pOwnedNode = aNode.release();
    }
}
//-----------------------------------------------------------------------------

/// transfer ownership from the given owner
void ElementTreeImpl::takeNodeFrom(std::auto_ptr<INode>& rOldOwner)
{
    OSL_ENSURE(!m_pOwnedNode,"ERROR: Cannot take over a node - already owning");
    OSL_ENSURE(rOldOwner.get(),"ERROR: Cannot take over NULL node");
    if (!m_pOwnedNode)
    {
        m_pOwnedNode = rOldOwner.release();
    }
}
//-----------------------------------------------------------------------------

/// transfer ownership to the given owner
void ElementTreeImpl::releaseTo(std::auto_ptr<INode>& rNewOwner)
{
    OSL_ENSURE(m_pOwnedNode,"ERROR: Cannot release a non-owned node");
    if (m_pOwnedNode)
    {
        Name aNodeName = getSimpleRootName();
        m_pOwnedNode->setName( aNodeName.toString() );
    }

    rNewOwner.reset(m_pOwnedNode);
    m_pOwnedNode = 0;
}
//-----------------------------------------------------------------------------

/// transfer ownership to the given owner, also providing a new name
void ElementTreeImpl::releaseAs(std::auto_ptr<INode>& rNewOwner, Name const& aElementName)
{
    OSL_ENSURE(m_pOwnedNode,"ERROR: Cannot release and rename a non-owned node");

    if (m_pOwnedNode)
        renameTree(aElementName);

    this->releaseTo(rNewOwner);
}
//-----------------------------------------------------------------------------

// context handling
//-----------------------------------------------------------------------------

/// renames the tree's root without concern for context consistency !
void ElementTreeImpl::renameTree(Name const& aNewName)
{
    node(root())->renameNode(aNewName);
}
//-----------------------------------------------------------------------------

void ElementTreeImpl::moveTree(TreeImpl* pParentTree, NodeOffset nParentNode)
{
    TreeImpl::setContext(pParentTree,nParentNode);
}
//-----------------------------------------------------------------------------

void ElementTreeImpl::detachTree()
{
    TreeImpl::clearContext();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }   // namespace configuration
}       // namespace configmgr

