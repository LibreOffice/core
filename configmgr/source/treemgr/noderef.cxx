/*************************************************************************
 *
 *  $RCSfile: noderef.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-13 17:57:50 $
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

#include "noderef.hxx"
#include "treeimpl.hxx"

#include "configpath.hxx"
#include "nodechange.hxx"
#include "configexcept.hxx"

#include <algorithm> // for swap

namespace configmgr
{
    namespace configuration
    {

// helpers first

//-----------------------------------------------------------------------------
// class TreeImplHelper (declared in treeimpl.hxx)
//-----------------------------------------------------------------------------

NodeRef TreeImplHelper::makeNode(Node* pNode, NodeOffset nOffset, TreeDepth nDepth)
{
    return NodeRef(pNode,nOffset,nDepth);
}

//-----------------------------------------------------------------------------

bool TreeImplHelper::isSet(NodeRef const& aNode)
{
    OSL_ASSERT(aNode.m_pImpl); return aNode.m_pImpl && aNode.m_pImpl->isSetNode();
}
//-----------------------------------------------------------------------------

bool TreeImplHelper::isGroup(NodeRef const& aNode)
{
    OSL_ASSERT(aNode.m_pImpl); return aNode.m_pImpl && aNode.m_pImpl->isGroupNode();
}
//-----------------------------------------------------------------------------

bool TreeImplHelper::isValue(NodeRef const& aNode)
{
    OSL_ASSERT(aNode.m_pImpl); return aNode.m_pImpl && aNode.m_pImpl->isValueNode();
}
//-----------------------------------------------------------------------------

TreeImpl* TreeImplHelper::impl(Tree const& aTree)
{
    return aTree.m_pImpl;
}
//-----------------------------------------------------------------------------

Node* TreeImplHelper::node(NodeRef const& aNode)
{
    return aNode.m_pImpl;
}
//-----------------------------------------------------------------------------

NodeOffset TreeImplHelper::offset(NodeRef const& aNode)
{
    return aNode.m_nPos;
}
//-----------------------------------------------------------------------------

TreeImpl* TreeImplHelper::tree(NodeID const& aNodeID)
{
    return aNodeID.m_pTree;
}
//-----------------------------------------------------------------------------

NodeOffset TreeImplHelper::offset(NodeID const& aNodeID)
{
    return aNodeID.m_nNode;
}

//-----------------------------------------------------------------------------
// local  helpers
//-----------------------------------------------------------------------------

namespace
{
    struct SetVisitorAdapter : SetNodeVisitor
    {
        SetVisitorAdapter(NodeVisitor& rVisitor, TreeDepth nChildDepth = 0)
        : m_rVisitor(rVisitor)
        , m_nOffset(0)
        , m_nDepth(nChildDepth)
        {}
        Result visit(SetEntry const& anEntry);

        NodeVisitor&    m_rVisitor;
        NodeOffset      m_nOffset;
        TreeDepth       m_nDepth;
    };

    SetNodeVisitor::Result SetVisitorAdapter::visit(SetEntry const& anEntry)
    {
        OSL_ASSERT(anEntry.isValid());

        Node* pNode = 0;
        if (TreeImpl* pTree = anEntry.tree())
        {
            m_nOffset = pTree->root();
            m_nDepth = pTree->getAvailableDepth();
            pNode = pTree->node(m_nOffset);
        }

        OSL_ASSERT( NodeVisitor::DONE == SetNodeVisitor::DONE );
        OSL_ASSERT( NodeVisitor::CONTINUE == SetNodeVisitor::CONTINUE );

        return Result( TreeImplHelper::makeNode(pNode, m_nOffset, m_nDepth).accept(m_rVisitor) );
    }
}

//-----------------------------------------------------------------------------
// class NodeRef
//-----------------------------------------------------------------------------

NodeRef::NodeRef()
: m_pImpl(0)
, m_nPos(0)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

NodeRef::NodeRef(Node*  pImpl, NodeOffset nPos, TreeDepth nDepth)
: m_pImpl(pImpl)
, m_nPos(nPos)
, m_nDepth(nDepth)
{}
//-----------------------------------------------------------------------------

NodeRef::NodeRef(NodeRef const& rOther)
: m_pImpl(rOther.m_pImpl)
, m_nPos(rOther.m_nPos)
, m_nDepth(rOther.m_nDepth)
{
}
//-----------------------------------------------------------------------------

NodeRef& NodeRef::operator=(NodeRef const& rOther)
{
    NodeRef(rOther).swap(*this);
    return *this;
}
//-----------------------------------------------------------------------------

void NodeRef::swap(NodeRef& rOther)
{
    std::swap(m_pImpl,  rOther.m_pImpl);
    std::swap(m_nPos,   rOther.m_nPos);
    std::swap(m_nDepth, rOther.m_nDepth);
}
//-----------------------------------------------------------------------------

NodeRef::~NodeRef()
{
}
//-----------------------------------------------------------------------------

bool NodeRef::isValid() const
{
    OSL_ASSERT( m_pImpl != 0 || m_nPos == 0 );

    return m_pImpl != 0;
}
//-----------------------------------------------------------------------------

bool NodeRef::hasChildren() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_ENSURE( m_nDepth > 0, "WARNING: Configuration: Querying node beyond available depth" );

    return m_pImpl && m_pImpl->isSetNode() &&
            !m_pImpl->setImpl().isEmpty();
}
//-----------------------------------------------------------------------------

bool NodeRef::hasChild(Name const& aName) const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_ENSURE( m_nDepth > 0, "WARNING: Configuration: Querying node beyond available depth" );

    return  m_pImpl && m_pImpl->isSetNode() &&
            m_pImpl->setImpl().findElement(aName).isValid();
}
//-----------------------------------------------------------------------------

NodeRef NodeRef::getChild(Name const& aName, Tree& rTree) const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_ENSURE( m_nDepth > 0, "WARNING: Configuration: Requesting node beyond available depth" );

    if (m_pImpl && m_pImpl->isSetNode())
    {
        SetEntry aChildEntry = m_pImpl->setImpl().findElement(aName);
        if (TreeImpl* pTree = aChildEntry.tree())
        {
            NodeOffset  nFoundOffset = pTree->root();
            TreeDepth   nChildDepth  = pTree->getAvailableDepth();

            Node* pNode = pTree->node(nFoundOffset);
            // all fine ? now adjust the tree
            rTree = Tree(pTree);

            return NodeRef(pNode,nFoundOffset,nChildDepth);
        }
    }
    return NodeRef(); // nothing found
}
//-----------------------------------------------------------------------------

NodeInfo NodeRef::getInfo() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    if (!isValid()) return NodeInfo();

    return m_pImpl->info();
}
//-----------------------------------------------------------------------------

Attributes NodeRef::getAttributes() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    if (!isValid()) return Attributes();

    return m_pImpl->attributes();
}
//-----------------------------------------------------------------------------

Name NodeRef::getName() const
{
    if (!isValid()) return Name();

    return m_pImpl->name();
}
//-----------------------------------------------------------------------------

UnoType NodeRef::getUnoType() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    if (!isValid()) return getVoidCppuType();

    if (m_pImpl->isValueNode())
    {
        return m_pImpl->valueImpl().getValueType();
    }
    else
    {
        uno::Reference< uno::XInterface > const * const selectInterface=0;
        return ::getCppuType(selectInterface);
    }
}

//-----------------------------------------------------------------------------
// class Tree
//-----------------------------------------------------------------------------

Tree::Tree(TreeImpl* pImpl)
: m_pImpl(pImpl)
{
    if (m_pImpl) m_pImpl->acquire();
}
//-----------------------------------------------------------------------------

// just DTRT
Tree::Tree(Tree const& aTree)
: m_pImpl(aTree.m_pImpl)
{
    if (m_pImpl) m_pImpl->acquire();
}
//-----------------------------------------------------------------------------

// the usual exception-safe swap-based one
Tree& Tree::operator=(Tree const& rOther)
{
    Tree(rOther).swap(*this);
    return *this;
}
//-----------------------------------------------------------------------------

void Tree::swap(Tree& rOther)
{
    std::swap(m_pImpl,  rOther.m_pImpl);
}
//-----------------------------------------------------------------------------

Tree::~Tree()
{
    if (m_pImpl) m_pImpl->release();
}
//-----------------------------------------------------------------------------

bool Tree::isEmpty() const
{
    OSL_ASSERT( m_pImpl == 0 || m_pImpl->nodeCount() > 0 );
    return m_pImpl == 0 /*|| m_pImpl->nodeCount() == 0*/ ;
}
//-----------------------------------------------------------------------------

NodeOffset Tree::getContainedNodeCount() const
{
    OSL_PRECOND(m_pImpl, "ERROR: Configuration: Counting nodes requires a valid Tree");

    return m_pImpl ? m_pImpl->nodeCount() : 0;
}
//-----------------------------------------------------------------------------

bool Tree::isValidNode(NodeRef const& aNode) const
{
    OSL_PRECOND(m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!aNode.isValid()) return false;
    if (this->isEmpty()) return false;

    if (!m_pImpl->isValidNode(aNode.m_nPos)) return false;
    if (m_pImpl->node(aNode.m_nPos) != aNode.m_pImpl) return false;

    return true;
}
//-----------------------------------------------------------------------------

bool Tree::hasChildren(NodeRef const& aNode) const
{
    OSL_PRECOND( m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ENSURE(aNode.m_nDepth > 0, "WARNING: Configuration: Querying node beyond available depth" );

    if (this->isEmpty()) return false;
    if (!aNode.isValid()) return false;

    return (m_pImpl->firstChild(aNode.m_nPos) != 0);
}
//-----------------------------------------------------------------------------

bool Tree::hasChild(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ENSURE(aNode.m_nDepth > 0, "WARNING: Configuration: Querying node beyond available depth" );

    if (this->isEmpty()) return false;
    if (!aNode.isValid()) return false;

    return (m_pImpl->findChild(aNode.m_nPos, aName) != 0);
}
//-----------------------------------------------------------------------------

NodeRef Tree::getChild(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    // OSL_PRECOND(this->hasChild(aNode,aName),"ERROR: Configuration: Invalid node request.");
    OSL_ENSURE(aNode.m_nDepth > 0, "WARNING: Configuration: Requesting node beyond available depth" );

    NodeOffset nOffset  = m_pImpl ? m_pImpl->findChild(aNode.m_nPos, aName) : 0;

    Node* pFound    = nOffset ? m_pImpl->node(nOffset) : 0;

    return NodeRef(pFound, nOffset, aNode.m_nDepth-1);
}
//-----------------------------------------------------------------------------

NodeRef Tree::getParent(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( m_pImpl->parent(m_pImpl->root()) == 0 );

    NodeOffset nParent = aNode.isValid() ? aNode.m_pImpl->parent() : 0;

    OSL_ENSURE(  m_pImpl->isValidNode(nParent), "ERROR: Configuration: NodeRef has invalid parent");

    Node*  pParent = nParent ? m_pImpl->node(nParent) : 0;

    return NodeRef(pParent, nParent, aNode.m_nDepth+1);
}
//-----------------------------------------------------------------------------

RelativePath Tree::getLocalPath(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    Path::Components aNames;

    // Case of invalid node is handled properly (as they (should) have m_nPos == 0)
    m_pImpl->appendPathTo( aNode.m_nPos, aNames );

    return RelativePath(aNames);
}
//-----------------------------------------------------------------------------

NodeRef Tree::getRootNode() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    if (isEmpty()) return NodeRef();

    NodeOffset nRoot = m_pImpl->root();
    return NodeRef( m_pImpl->node(nRoot), nRoot, m_pImpl->getAvailableDepth() );
}
//-----------------------------------------------------------------------------

bool Tree::isRootNode(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    return m_pImpl && aNode.isValid() && m_pImpl->root() == aNode.m_nPos;
}
//-----------------------------------------------------------------------------

void Tree::ensureDefaults() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_ENSURE(false,"Configuration: WARNING: Default handling not really implemented yet");
}
//-----------------------------------------------------------------------------

bool Tree::isNodeDefault(NodeRef const& aNode) const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    return  aNode.m_pImpl->isValueNode() &&
            aNode.m_pImpl->valueImpl().isDefault();
}
//-----------------------------------------------------------------------------

UnoAny Tree::getNodeDefault(NodeRef const& aNode)       const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    if ( aNode.m_pImpl->isValueNode() )
    {
        ValueNodeImpl& rNodeImpl = aNode.m_pImpl->valueImpl();

        if (rNodeImpl.canGetDefaultValue())
            return rNodeImpl.getDefaultValue();
    }

    return UnoAny();
}
//-----------------------------------------------------------------------------

Tree Tree::getContextTree() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    TreeImpl* pContext = isEmpty() ? 0 : m_pImpl->getContextTree();

    return Tree(pContext);
}
//-----------------------------------------------------------------------------

NodeRef Tree::getContextNode() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (isEmpty()) return NodeRef();

    NodeOffset nOffset = m_pImpl->getContextNode();
    TreeImpl* pContext = m_pImpl->getContextTree();

    // if we have a context, we must have a (parent's) position in it
    OSL_ASSERT( pContext == 0 || nOffset != 0);
    if (pContext == 0) return NodeRef();

    return NodeRef( pContext->node(nOffset), nOffset, pContext->getAvailableDepth() - pContext->depthTo(nOffset));
}
//-----------------------------------------------------------------------------

AbsolutePath Tree::getContextPath() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    return isEmpty() ? AbsolutePath::root() : m_pImpl->getContextPath();
}
//-----------------------------------------------------------------------------

bool Tree::hasChanges()  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    return !isEmpty() && m_pImpl->hasChanges();
}
//-----------------------------------------------------------------------------

bool Tree::collectChanges(NodeChanges& aChanges)  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!isEmpty() && m_pImpl->hasChanges())
    {
        m_pImpl->collectChanges(aChanges);
        return true;
    }
    else
        return false;
}
//-----------------------------------------------------------------------------

void Tree::integrate(NodeChange& aChange, NodeRef const& aNode, bool bLocal)  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aChange.isChange())
    {
        aChange.apply();
        if (bLocal)
        {
            m_pImpl->markChanged(aNode.m_nPos);
        }
        else
        {
            Tree aAffectedTree      = aChange.getAffectedTree();
            NodeRef aAffectedNode   = aChange.getAffectedNode();
            OSL_ASSERT(!aAffectedTree.isEmpty() && aAffectedTree.isValidNode(aAffectedNode));

            aAffectedTree.m_pImpl->markChanged(aAffectedNode.m_nPos);

            OSL_ASSERT(this->m_pImpl->hasChanges());
        }
    }
}
//-----------------------------------------------------------------------------

void Tree::integrate(NodeChanges& aChanges, NodeRef const& aNode, bool bLocal) const
{
    typedef NodeChanges::MutatingIterator Iter;
    for(Iter it = aChanges.begin(), end = aChanges.end(); it != end; ++it)
        this->integrate(*it, aNode, bLocal);
}
//-----------------------------------------------------------------------------

NodeVisitor::Result Tree::dispatchToChildren(NodeRef const& aNode, NodeVisitor& aVisitor) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ENSURE(aNode.m_nDepth > 0, "WARNING: Configuration: Requesting node beyond available depth" );

    typedef NodeVisitor::Result Result;
    Result aRet = NodeVisitor::CONTINUE;

    if (!aNode.isValid())
        OSL_TRACE("WARNING: Configuration: trying to iterate an invalid node !");

    else if (aNode.m_pImpl->isGroupNode())
    {
        NodeOffset const nParent = aNode.m_nPos;
        TreeDepth const nDepth = aNode.m_nDepth-1;

        for( NodeOffset nPos = m_pImpl->firstChild(nParent);
             nPos != 0 && aRet != NodeVisitor::DONE;
             nPos = m_pImpl->findNextChild(nParent,nPos))
        {
            aRet = NodeRef( m_pImpl->node(nPos), nPos, nDepth ).accept(aVisitor);
        }
    }

    else if (aNode.m_pImpl->isSetNode())
    {
        SetVisitorAdapter aAdapter(aVisitor, aNode.m_nDepth-1);

        OSL_ASSERT( NodeVisitor::DONE == SetNodeVisitor::DONE );
        OSL_ASSERT( NodeVisitor::CONTINUE == SetNodeVisitor::CONTINUE );

        aRet = Result(aNode.m_pImpl->setImpl().dispatchToElements(aAdapter));
    }

    else
        OSL_TRACE("WARNING: Configuration: trying to iterate a Value node !");

    return aRet;
}
//-----------------------------------------------------------------------------

NodeRef Tree::bind(NodeOffset nNode) const
{
    if (m_pImpl && m_pImpl->isValidNode(nNode))
    {
        TreeDepth nDepth = m_pImpl->getAvailableDepth() - m_pImpl->depthTo(nNode);
        return NodeRef(m_pImpl->node(nNode), nNode, nDepth);
    }
    else
    {
        return NodeRef();
    }
}
//-----------------------------------------------------------------------------

NodeRef Tree::rebind(NodeRef const& aNode) const
{
    return bind(aNode.m_nPos);
}

//-----------------------------------------------------------------------------
// class NodeID
//-----------------------------------------------------------------------------

NodeID::NodeID(Tree const& rTree, NodeRef const& rNode)
: m_pTree( TreeImplHelper::impl(rTree) )
, m_nNode( TreeImplHelper::offset(rNode) )
{
}
//-----------------------------------------------------------------------------

NodeID::NodeID(TreeImpl* pImpl, NodeOffset nNode)
: m_pTree( pImpl )
, m_nNode( nNode )
{
}
//-----------------------------------------------------------------------------

// hashing
size_t NodeID::hashCode() const
{
    size_t nBaseHash = m_pTree ? m_pTree->getContextPath().hashCode() : 0;
    return nBaseHash + 5*m_nNode;
}
//-----------------------------------------------------------------------------

NodeOffset NodeID::toIndex() const
{
    NodeOffset n = m_nNode;
    if (m_pTree)
    {
        OSL_ENSURE(m_pTree->isValidNode(n),"Cannot produce valid Index for NodeID");

        n -= m_pTree->root();
    }
    return n;
}

//-----------------------------------------------------------------------------
// Free functions
//-----------------------------------------------------------------------------

Name validateNodeName(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    if (!aNode.isValid()) // no node - any name
        return makeName(sName);

    else if (TreeImplHelper::isSet(aNode))
        return validateElementName(sName);

    else
        return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

RelativePath reduceRelativePath(OUString const& sPath, Tree const& aTree, NodeRef const& aBaseNode)
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  !aBaseNode.isValid() || aTree.isValidNode(aBaseNode), "WARNING: Configuration: NodeRef does not match Tree");

    // TODO: Evaluate aTree's ancestors as node context for path parsing (?)
    Path aPath( sPath, Path::NoValidate() );

    Path::Type eType = aPath.getType();

    if (eType != PathType::eABSOLUTE)
    {
        // TODO: Validate path components based on node context
        return RelativePath(aPath.rep());
    }

    // handle absolute path prefix removal now
    typedef Path::Iterator Iter;

    Iter        itPath      = aPath.begin();
    Iter const  itPathEnd   = aPath.end();

    if (itPath != itPathEnd && itPath->isEmpty())   // absolute pathes may use a dummy empty first component
        ++itPath;

    // Tree context resolution
    {
        AbsolutePath aContextPath( aTree.getContextPath() );
        Iter        itCtx       = aContextPath.begin();
        Iter const  itCtxEnd    = aContextPath.end();

        for ( ; itCtx != itCtxEnd; ++itPath, ++itCtx)
        {
            if (itPath == itPathEnd || *itPath != *itCtx )
                throw InvalidName(sPath, " does not point into the current node's tree context.");
        }
    }
    // now resolve the path within the tree
    {
        RelativePath aLocalPath( aTree.getLocalPath(aBaseNode) );
        // could be optimized by just building the reverse list

        Iter        itLocal     = aLocalPath.begin();
        Iter const  itLocalEnd  = aLocalPath.end();

        for ( ; itLocal != itLocalEnd; ++itPath, ++itLocal)
        {
            if (itPath == itPathEnd || *itPath != *itLocal )
                throw InvalidName(sPath, " does not point into the current context node.");
        }
    }
    // TODO: Validate subsequent path components based on node context

    // now return the valid remainder
    return RelativePath(Path::Components(itPath, itPathEnd));
}
//-----------------------------------------------------------------------------

bool hasChildNode(Tree const& aTree, NodeRef const& aNode)
{
    return TreeImplHelper::isSet(aNode) ? aNode.hasChildren() : aTree.hasChildren(aNode);
}
//-----------------------------------------------------------------------------

bool hasChildNode(Tree const& aTree, NodeRef const& aNode, Name const& aName)
{
    return TreeImplHelper::isSet(aNode) ? aNode.hasChild(aName) : aTree.hasChild(aNode,aName);
}
//-----------------------------------------------------------------------------

bool findChildNode(Tree& aTree, NodeRef& aNode, Name const& aName)
{
    NodeRef aChild = TreeImplHelper::isSet(aNode) ? aNode.getChild(aName,aTree) : aTree.getChild(aNode,aName);

    if ( aChild.isValid() )
        aNode = aChild;

    return aChild.isValid();
}
//-----------------------------------------------------------------------------

bool findDescendantNode(Tree& aTree, NodeRef& aNode, RelativePath& aPath)
{
    // requires: findChildNode leaves node and tree unchanged when returning false
    typedef Path::Iterator Iter;

    Iter        itPath      = aPath.begin();
    Iter const  itPathEnd   = aPath.end();

    for ( ; itPath != itPathEnd; ++itPath)
        if (!findChildNode(aTree,aNode,*itPath))
            return false;

    return true;
}
//-----------------------------------------------------------------------------

void getAllContainedNodes(Tree const& aTree, NodeIDList& aList)
{
    aList.clear();

    if (TreeImpl* pImpl = TreeImplHelper::impl(aTree))
    {
        NodeOffset nCount = pImpl->nodeCount();
        aList.reserve(nCount);

        NodeOffset const nEnd = pImpl->root() + nCount;

        for(NodeOffset nOffset = pImpl->root();
            nOffset < nEnd;
            ++nOffset)
        {
            OSL_ASSERT( pImpl->isValidNode(nOffset) );
            aList.push_back( NodeID(pImpl,nOffset) );
        }

        OSL_ASSERT( aList.size()==nCount );
    }
}
//-----------------------------------------------------------------------------

void getAllChildrenHelper(NodeID const& aNode, NodeIDList& aList)
{
    aList.clear();

    if (TreeImpl* pImpl = TreeImplHelper::tree(aNode))
    {
        if (NodeOffset const nParent = TreeImplHelper::offset(aNode))
        {
            for(NodeOffset nOffset = pImpl->firstChild(nParent);
                nOffset != 0;
                nOffset = pImpl->findNextChild(nParent,nOffset))
            {
                OSL_ASSERT( pImpl->isValidNode(nOffset) );
                aList.push_back( NodeID(pImpl,nOffset) );
            }
        }
    }
}
//-----------------------------------------------------------------------------

NodeID getParentHelper(NodeID const& aNode)
{
    if (TreeImpl* pImpl = TreeImplHelper::tree(aNode))
    {
        if (NodeOffset const nNode = TreeImplHelper::offset(aNode))
        {
            if (NodeOffset nParent = pImpl->parent(nNode))
            {
                return NodeID(pImpl,nParent);
            }
        }
    }
    return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeID findNeighbor(NodeID const& aNode, NodeOffset nNeighbor)
{
    if (TreeImpl* pImpl = TreeImplHelper::tree(aNode))
    {
        if (pImpl->isValidNode(nNeighbor))
        {
            return NodeID(pImpl,nNeighbor);
        }
    }
    return NodeID(0,0);
}

//-----------------------------------------------------------------------------
NodeID findNodeFromIndex(NodeID const& aNode, NodeOffset nIndex)
{
    if (TreeImpl* pImpl = TreeImplHelper::tree(aNode))
    {
        NodeOffset nNode = nIndex + pImpl->root();
        if (pImpl->isValidNode(nNode))
        {
            return NodeID(pImpl,nNode);
        }
    }
    return NodeID(0,0);
}

//-----------------------------------------------------------------------------
bool isSimpleValue(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    return aNode.isValid() && TreeImplHelper::isValue(aNode);
}
//-----------------------------------------------------------------------------

bool isGroupNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    return aNode.isValid() && TreeImplHelper::isGroup(aNode);
}
//-----------------------------------------------------------------------------

bool isSetNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    return aNode.isValid() && TreeImplHelper::isSet(aNode);
}
//-----------------------------------------------------------------------------

UnoAny getSimpleValue(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    if (!aNode.isValid()) return UnoAny();

    OSL_PRECOND( aTree.isEmpty() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    OSL_PRECOND( TreeImplHelper::isValue(aNode), "ERROR: Configuration: Getting value is supported only for value nodes");
    return TreeImplHelper::node(aNode)->valueImpl().getValue();
}

//-----------------------------------------------------------------------------

ISynchronizedData* getRootLock(Tree const& aTree)
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    if (aTree.isEmpty()) return 0;

    return TreeImplHelper::impl(aTree)->getRootLock();
}

//-----------------------------------------------------------------------------
    }   // namespace configuration
}       // namespace configmgr
