/*************************************************************************
 *
 *  $RCSfile: noderef.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: jb $ $Date: 2001-08-06 15:25:20 $
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

#include "anynoderef.hxx"
#include "valueref.hxx"
#include "noderef.hxx"

#include "treeimpl.hxx"

#include "noderef.hxx"

#include "setnodeimplbase.hxx"
#include "valuenodeimpl.hxx"
#include "groupnodeimpl.hxx"

#include "configpath.hxx"
#include "nodechange.hxx"
#include "configexcept.hxx"
#include "configset.hxx"

#include "tracer.hxx"

#include <algorithm> // for swap
#include <functional> // for less

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

NodeRef TreeImplHelper::makeNode(TreeImpl& rTree, NodeOffset nOffset)
{
    if (nOffset)
    {
        OSL_ASSERT(rTree.isValidNode(nOffset));

        TreeDepth nDepth = remainingDepth(rTree.getAvailableDepth(), rTree.depthTo(nOffset));
        return NodeRef(rTree.node(nOffset),nOffset,nDepth);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeRef TreeImplHelper::makeNode(NodeID const& aNodeID)
{
    if (TreeImpl* pTree = aNodeID.m_pTree)
    {
        return makeNode(*pTree,aNodeID.m_nNode);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

ValueRef TreeImplHelper::makeValue(Name const& aName, Node* pParentNode, NodeOffset nParentOffset)
{
    return ValueRef(aName,pParentNode,nParentOffset);
}
//-----------------------------------------------------------------------------

AnyNodeRef TreeImplHelper::makeAnyNode(Node* pNode, NodeOffset nOffset, TreeDepth nDepth)
{
    return AnyNodeRef(pNode,nOffset, nDepth);
}
//-----------------------------------------------------------------------------

AnyNodeRef TreeImplHelper::makeAnyNode(Name const& aName, Node* pParentNode, NodeOffset nParentOffset)
{
    return AnyNodeRef(aName,pParentNode,nParentOffset);
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

bool TreeImplHelper::isValueElement(NodeRef const& aNode)
{
    OSL_ASSERT(aNode.m_pImpl); return aNode.m_pImpl && aNode.m_pImpl->isValueElementNode();
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

Node* TreeImplHelper::parent_node(ValueRef const& aNode)
{
    return aNode.m_pParentImpl;
}
//-----------------------------------------------------------------------------

NodeOffset TreeImplHelper::offset(NodeRef const& aNode)
{
    return aNode.m_nPos;
}
//-----------------------------------------------------------------------------

NodeOffset TreeImplHelper::parent_offset(ValueRef const& aNode)
{
    return aNode.m_nParentPos;
}
//-----------------------------------------------------------------------------

Name TreeImplHelper::value_name(ValueRef const& aValueNode)
{
    return aValueNode.m_sNodeName;
}
//-----------------------------------------------------------------------------

static inline
ValueMemberNode impl_member_node(Node* pParentImpl, Name const& aValueName)
{
    OSL_ENSURE(pParentImpl,"INTERNAL ERROR: impl_member_node: NULL parent passed");
    OSL_ENSURE(pParentImpl->isGroupNode(),"INTERNAL ERROR: impl_member_node: non-group parent passed");
    OSL_ENSURE(!aValueName.isEmpty(),"INTERNAL ERROR: impl_member_node: Missing value name");
    return pParentImpl->groupImpl().getValue(aValueName);
}
//-----------------------------------------------------------------------------

ValueMemberNode TreeImplHelper::member_node(ValueRef const& aValueNode)
{
    return impl_member_node(aValueNode.m_pParentImpl,aValueNode.m_sNodeName);
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
//-----------------------------------------------------------------------------
    struct SetVisitorAdapter : SetNodeVisitor
    {
        SetVisitorAdapter(NodeVisitor& rVisitor)
        : m_rVisitor(rVisitor)
        {}
        Result visit(SetEntry const& anEntry);

        NodeVisitor&    m_rVisitor;
    };

    SetNodeVisitor::Result SetVisitorAdapter::visit(SetEntry const& anEntry)
    {
        OSL_ASSERT(anEntry.isValid());

        Result aResult = CONTINUE;

        if (TreeImpl* pTree = anEntry.tree())
        {
            Tree aTree( pTree );
            NodeRef aTreeRoot = aTree.getRootNode();

            OSL_ASSERT( NodeVisitor::DONE == SetNodeVisitor::DONE );
            OSL_ASSERT( NodeVisitor::CONTINUE == SetNodeVisitor::CONTINUE );

            aResult = Result( aTree.visit(aTreeRoot,m_rVisitor) );
        }

        return aResult;
    }
//-----------------------------------------------------------------------------
    struct GroupVisitorAdapter : GroupMemberVisitor
    {
        GroupVisitorAdapter(Tree const& aParentTree, NodeRef const& aParentNode, NodeVisitor& rVisitor)
        : m_rVisitor(rVisitor)
        , m_aParentTree(aParentTree)
        , m_pParentNode( TreeImplHelper::node(aParentNode) )
        , m_nParentPos( TreeImplHelper::offset(aParentNode) )
        {
            OSL_ASSERT(!aParentTree.isEmpty());
            OSL_ASSERT(aParentNode.isValid());
             OSL_ASSERT(aParentTree.isValidNode(aParentNode));

        }

        Result visit(ValueMemberNode const& anEntry);

        NodeVisitor&    m_rVisitor;
        Tree            m_aParentTree;
        Node*           m_pParentNode;
        NodeOffset      m_nParentPos;
    };

    GroupMemberVisitor::Result GroupVisitorAdapter::visit(ValueMemberNode const& aValue)
    {
        OSL_ASSERT(aValue.isValid());

        Result aResult = CONTINUE;

        Name aValueName = aValue.getNodeName();

        ValueRef const aValueRef = TreeImplHelper::makeValue(aValueName,m_pParentNode,m_nParentPos);

        OSL_ASSERT( NodeVisitor::DONE       == GroupMemberVisitor::DONE );
        OSL_ASSERT( NodeVisitor::CONTINUE   == GroupMemberVisitor::CONTINUE );

        aResult = Result( m_aParentTree.visit(aValueRef,m_rVisitor) );

        return aResult;
    }
//-----------------------------------------------------------------------------
    struct CollectValueIDs : GroupMemberVisitor
    {
        CollectValueIDs(NodeID const& aParentID, SubNodeIDList& rValueList)
        : m_aParentID(aParentID)
        , m_rValueList(rValueList)
        {
        }

        Result visit(ValueMemberNode const& anEntry);

        NodeID  m_aParentID;
        SubNodeIDList& m_rValueList;
    };

    GroupMemberVisitor::Result CollectValueIDs::visit(ValueMemberNode const& aValue)
    {
        OSL_ASSERT(aValue.isValid());

        Name aValueName = aValue.getNodeName();

        m_rValueList.push_back(SubNodeID( m_aParentID, aValueName));

        return CONTINUE;
    }
//-----------------------------------------------------------------------------
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

bool Tree::hasElements(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    return aNode.m_pImpl && aNode.m_pImpl->isSetNode() &&
            !aNode.m_pImpl->setImpl().isEmpty();
}
//-----------------------------------------------------------------------------

bool Tree::hasElement(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    return  aNode.m_pImpl && aNode.m_pImpl->isSetNode() &&
            aNode.m_pImpl->setImpl().findElement(aName).isValid();
}
//-----------------------------------------------------------------------------

bool Tree::hasElement(NodeRef const& aNode, Path::Component const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    bool bFound = false;

    if (aNode.m_pImpl && aNode.m_pImpl->isSetNode())
    {
        SetEntry aChildEntry = aNode.m_pImpl->setImpl().findElement(aName.getName());

        // do check if types do match as well
        bFound = aChildEntry.isValid() &&
                 Path::matches(aChildEntry.tree()->getExtendedRootName(),aName);
    }

    return bFound; // even if nothing found
}
//-----------------------------------------------------------------------------

ElementRef Tree::getElement(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Requesting node beyond available depth" );
    }

    ElementTreeImpl* pElementTree = NULL;

    if (aNode.m_pImpl && aNode.m_pImpl->isSetNode())
    {
        SetEntry aChildEntry = aNode.m_pImpl->setImpl().findElement(aName);
        pElementTree = aChildEntry.tree();
    }

    return ElementRef(pElementTree); // even if nothing found
}
//-----------------------------------------------------------------------------

// a version of Tree::getElement that retrieves only loaded nodes
ElementRef Tree::getAvailableElement(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    ElementTreeImpl* pElementTree = NULL;

    if (aNode.m_pImpl && aNode.m_pImpl->isSetNode())
    {
        SetEntry aChildEntry = aNode.m_pImpl->setImpl().findAvailableElement(aName);
        pElementTree = aChildEntry.tree();
    }

    return ElementRef(pElementTree); // even if nothing found
}
//-----------------------------------------------------------------------------

Attributes Tree::getAttributes(NodeRef const& aNode)    const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (!aNode.isValid()) return Attributes();

    return aNode.m_pImpl->attributes();
}
//-----------------------------------------------------------------------------

Name Tree::getName(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (isEmpty()) return Name();

    return m_pImpl->getSimpleNodeName(aNode.m_nPos);
}
//-----------------------------------------------------------------------------
// class ValueRef
//-----------------------------------------------------------------------------

bool ValueRef::checkValidState() const
{
    if (m_pParentImpl == NULL) return false;
    if (m_nParentPos  == 0)    return false;

    // old node semantics for now
    if ( m_sNodeName.isEmpty() ) return false;

    if (!m_pParentImpl->isGroupNode()) return false;

    return TreeImplHelper::member_node(*this).isValid();
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef()
: m_sNodeName()
, m_pParentImpl(0)
, m_nParentPos(0)
{
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef(Name const& aName, Node* pParentImpl, NodeOffset nParentPos)
: m_sNodeName(aName)
, m_pParentImpl(pParentImpl)
, m_nParentPos(nParentPos)
{
    OSL_ENSURE( pParentImpl == NULL || checkValidState(), "Constructing invalid ValueRef");
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef(ValueRef const& rOther)
: m_sNodeName(rOther.m_sNodeName)
, m_pParentImpl(rOther.m_pParentImpl)
, m_nParentPos(rOther.m_nParentPos)
{
}
//-----------------------------------------------------------------------------

ValueRef& ValueRef::operator=(ValueRef const& rOther)
{
    ValueRef(rOther).swap(*this);
    return *this;
}
//-----------------------------------------------------------------------------

void ValueRef::swap(ValueRef& rOther)
{
    std::swap(m_sNodeName,  rOther.m_sNodeName);
    std::swap(m_pParentImpl,rOther.m_pParentImpl);
    std::swap(m_nParentPos, rOther.m_nParentPos);
}
//-----------------------------------------------------------------------------

ValueRef::~ValueRef()
{
}

//-----------------------------------------------------------------------------

Attributes Tree::getAttributes(ValueRef const& aValue)  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aValue.isValid(), "ERROR: Configuration: ValueRef operation requires valid reference" );
    OSL_PRECOND( isValidNode(aValue), "ERROR: Configuration: ValueRef does not match tree" );

    if (!aValue.isValid()) return Attributes();

    return TreeImplHelper::member_node(aValue).getAttributes();
}
//-----------------------------------------------------------------------------

Name Tree::getName(ValueRef const& aValue) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( !aValue.isValid() || isValidNode(aValue), "ERROR: Configuration: ValueRef does not match tree" );

    OSL_ENSURE( aValue.isValid() || aValue.m_sNodeName.isEmpty(), "Invalid value has a non-empty name ?!");

    return aValue.m_sNodeName;
}
//-----------------------------------------------------------------------------

UnoType Tree::getUnoType(ValueRef const& aValue) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aValue.isValid(), "ERROR: Configuration: ValueRef operation requires valid reference" );
    OSL_PRECOND( isValidNode(aValue), "ERROR: Configuration: ValueRef does not match tree" );

    if (!aValue.isValid()) return getVoidCppuType();

    return TreeImplHelper::member_node(aValue).getValueType();
}

//-----------------------------------------------------------------------------
// class AnyNodeRef
//-----------------------------------------------------------------------------

bool AnyNodeRef::checkValidState() const
{
    if (m_pUsedImpl == NULL) return false;
    if (m_nUsedPos  == 0)    return false;

    if ( !m_sNodeName.isEmpty() ) // it's a local value
    {
        // not used as runtime check as it should not be dangerous
        OSL_ENSURE(m_nDepth ==0, "AnyNodeRef that wraps a ValueRef should have no depth"); // value has no depth

        if (!m_pUsedImpl->isGroupNode()) return false;

        // does the named value exist
        if (!impl_member_node(m_pUsedImpl,m_sNodeName).isValid()) return false;
    }

    return true;
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef()
: m_sNodeName()
, m_pUsedImpl(0)
, m_nUsedPos(0)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(Node* pImpl, NodeOffset nPos, TreeDepth nDepth)
: m_sNodeName()
, m_pUsedImpl(pImpl)
, m_nUsedPos(nPos)
, m_nDepth(nDepth)
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(Name const& aName, Node* pParentImpl, NodeOffset nParentPos)
: m_sNodeName(aName)
, m_pUsedImpl(pParentImpl)
, m_nUsedPos(nParentPos)
, m_nDepth(0)
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(AnyNodeRef const& rOther)
: m_sNodeName(rOther.m_sNodeName)
, m_pUsedImpl(rOther.m_pUsedImpl)
, m_nUsedPos(rOther.m_nUsedPos)
, m_nDepth(rOther.m_nDepth)
{
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(NodeRef const& aNodeRef)
: m_sNodeName()
, m_pUsedImpl( aNodeRef.m_pImpl )
, m_nUsedPos(  aNodeRef.m_nPos )
, m_nDepth( aNodeRef.m_nDepth )
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(ValueRef const& aValueRef)
: m_sNodeName( TreeImplHelper::value_name(aValueRef) )
, m_pUsedImpl( TreeImplHelper::parent_node(aValueRef) )
, m_nUsedPos(  TreeImplHelper::parent_offset(aValueRef) )
, m_nDepth( 0 )
{}
//-----------------------------------------------------------------------------

AnyNodeRef& AnyNodeRef::operator=(AnyNodeRef const& rOther)
{
    AnyNodeRef(rOther).swap(*this);
    return *this;
}
//-----------------------------------------------------------------------------

void AnyNodeRef::swap(AnyNodeRef& rOther)
{
    std::swap(m_sNodeName,  rOther.m_sNodeName);
    std::swap(m_pUsedImpl,  rOther.m_pUsedImpl);
    std::swap(m_nUsedPos,   rOther.m_nUsedPos);
    std::swap(m_nDepth,     rOther.m_nDepth);
}
//-----------------------------------------------------------------------------

AnyNodeRef::~AnyNodeRef()
{
}

//-----------------------------------------------------------------------------

bool AnyNodeRef::isNode() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: AnyNodeRef operation requires valid node" );
    if (!isValid()) return false;

    return m_sNodeName.isEmpty();
}
//-----------------------------------------------------------------------------

NodeRef AnyNodeRef::toNode() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: AnyNodeRef operation requires valid node" );
    if (!isValid() || !isNode()) return NodeRef();

    return TreeImplHelper::makeNode(m_pUsedImpl,m_nUsedPos,m_nDepth);
}
//-----------------------------------------------------------------------------

ValueRef AnyNodeRef::toValue() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: AnyNodeRef operation requires valid node" );
    if (!isValid() || isNode()) return ValueRef();

    return TreeImplHelper::makeValue(m_sNodeName, m_pUsedImpl,m_nUsedPos);
}
//-----------------------------------------------------------------------------

Attributes Tree::getAttributes(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (!aNode.isValid()) return Attributes();

    if (aNode.isNode())
        return aNode.m_pUsedImpl->attributes();

    else
        return impl_member_node(aNode.m_pUsedImpl,aNode.m_sNodeName).getAttributes();
}
//-----------------------------------------------------------------------------

Name Tree::getName(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (isEmpty() || !aNode.isValid()) return Name();

    if (aNode.isNode())
        return m_pImpl->getSimpleNodeName(aNode.m_nUsedPos);

    else
        return aNode.m_sNodeName;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result Tree::visit(AnyNodeRef const& aNode, NodeVisitor& aVisitor) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    return aNode.isNode() ? visit(aNode.toNode(),aVisitor) : visit(aNode.toValue(),aVisitor);
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

void Tree::disposeData()
{
    TreeImpl* pImpl = m_pImpl;
//  m_pImpl = 0;
    if (pImpl)
    {
        pImpl->disposeData();
//      pImpl->release();
    }
}
//-----------------------------------------------------------------------------

bool Tree::isEmpty() const
{
    return m_pImpl == 0 || m_pImpl->nodeCount() == 0;
}
//-----------------------------------------------------------------------------

NodeOffset Tree::getContainedInnerNodeCount() const
{
    OSL_PRECOND(m_pImpl, "ERROR: Configuration: Counting nodes requires a valid Tree");

    return m_pImpl ? m_pImpl->nodeCount() : 0;
}
//-----------------------------------------------------------------------------

bool Tree::isValidNode(ValueRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!aNode.isValid()) return false;
    if (!aNode.checkValidState()) return false;

    if (this->isEmpty()) return false;

    if (!m_pImpl->isValidNode(aNode.m_nParentPos)) return false;
    if (m_pImpl->node(aNode.m_nParentPos) != aNode.m_pParentImpl) return false;

    OSL_ASSERT(!aNode.m_sNodeName.isEmpty()); // old value handling ?

    return true;
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

bool Tree::isValidNode(AnyNodeRef const& aNode) const
{
    OSL_PRECOND(m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!aNode.isValid()) return false;
    if (!aNode.checkValidState()) return false;

    if (this->isEmpty()) return false;

    if (!m_pImpl->isValidNode(aNode.m_nUsedPos)) return false;
    if (m_pImpl->node(aNode.m_nUsedPos) != aNode.m_pUsedImpl) return false;

    return true;
}
//-----------------------------------------------------------------------------

bool Tree::hasChildren(NodeRef const& aNode) const
{
    OSL_PRECOND( m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;
    if (!aNode.isValid()) return false;

    return (m_pImpl->firstChild(aNode.m_nPos) != 0);
}
//-----------------------------------------------------------------------------

bool Tree::hasChildNode(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;
    if (!aNode.isValid()) return false;

    NodeOffset nOffset  = m_pImpl->findChild(aNode.m_nPos, aName);

    return nOffset != 0;
}
//-----------------------------------------------------------------------------

bool Tree::hasChildValue(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;

    if (!TreeImplHelper::isGroup(aNode)) return false;

    return aNode.m_pImpl->groupImpl().hasValue(aName);
}
//-----------------------------------------------------------------------------

bool Tree::hasChild(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( m_pImpl, "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;
    if (!aNode.isValid()) return false;

    if (TreeImplHelper::isGroup(aNode))
    {
        if (aNode.m_pImpl->groupImpl().hasValue(aName)) return true;
    }

    NodeOffset nOffset  = m_pImpl->findChild(aNode.m_nPos, aName);

    return nOffset != 0;
}
//-----------------------------------------------------------------------------

NodeRef Tree::getChildNode(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    // OSL_PRECOND(this->hasChild(aNode,aName),"ERROR: Configuration: Invalid node request.");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Requesting node beyond available depth" );
    }

    NodeOffset nOffset  = m_pImpl ? m_pImpl->findChild(aNode.m_nPos, aName) : 0;

    Node* pFound    = nOffset ? m_pImpl->node(nOffset) : 0;

    return NodeRef(pFound, nOffset, childDepth(aNode.m_nDepth));
}
//-----------------------------------------------------------------------------

ValueRef Tree::getChildValue(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    // OSL_PRECOND(this->hasChild(aNode,aName),"ERROR: Configuration: Invalid node request.");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Requesting node beyond available depth" );
    }

    if (!TreeImplHelper::isGroup(aNode)) return ValueRef();

    if (!aNode.m_pImpl->groupImpl().hasValue(aName)) return ValueRef();

    return ValueRef(aName, aNode.m_pImpl, aNode.m_nPos);
}
//-----------------------------------------------------------------------------

AnyNodeRef Tree::getAnyChild(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    // OSL_PRECOND(this->hasChild(aNode,aName),"ERROR: Configuration: Invalid node request.");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Requesting node beyond available depth" );
    }

    if (TreeImplHelper::isGroup(aNode))
    {
        if (aNode.m_pImpl->groupImpl().hasValue(aName))
        {
            return AnyNodeRef(aName, aNode.m_pImpl, aNode.m_nPos);

        }
    }

    NodeOffset nOffset  = m_pImpl ? m_pImpl->findChild(aNode.m_nPos, aName) : 0;

    Node* pFound    = nOffset ? m_pImpl->node(nOffset) : 0;

    return AnyNodeRef(pFound, nOffset, childDepth(aNode.m_nDepth));
}
//-----------------------------------------------------------------------------

NodeRef Tree::getParent(ValueRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    NodeOffset nParent = aNode.m_nParentPos;

    OSL_ASSERT(nParent == 0 || !aNode.m_sNodeName.isEmpty());

    return TreeImplHelper::makeNode(*m_pImpl, nParent);
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

    return NodeRef(pParent, nParent, parentDepth(aNode.m_nDepth));
}
//-----------------------------------------------------------------------------

NodeRef Tree::getParent(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( m_pImpl->parent(m_pImpl->root()) == 0 );

    NodeOffset nParent = aNode.m_nUsedPos;

    if (aNode.m_sNodeName.isEmpty() && aNode.isValid())
        nParent = aNode.m_pUsedImpl->parent();

    return TreeImplHelper::makeNode(*m_pImpl, nParent);
}
//-----------------------------------------------------------------------------

UnoAny Tree::getNodeValue(ValueRef const& aNode) const
{
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid Value Ref");
    if (!aNode.isValid()) return UnoAny();

    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: Value Ref does not point to valid value");

    return TreeImplHelper::member_node(aNode).getValue();
}
//-----------------------------------------------------------------------------

AbsolutePath Tree::getAbsolutePath(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    Path::Rep aNames;

    if (!this->isEmpty())
    {
        if ( aNode.isValid() )
            m_pImpl->prependLocalPathTo( aNode.m_nPos, aNames );

        aNames.prepend( m_pImpl->getRootPath().rep() );
    }
    return AbsolutePath(aNames);
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

Path::Component Tree::getRootName() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    if (isEmpty()) return Path::makeEmptyComponent();

    return m_pImpl->getExtendedRootName();
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

bool Tree::isNodeDefault(ValueRef const& aNode) const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid ValueRef");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: ValueRef does not point to valid value");

    if (!aNode.isValid()) return false;

    return  TreeImplHelper::member_node(aNode).isDefault();
}
//-----------------------------------------------------------------------------

bool Tree::isNodeDefault(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: Node does not match Tree");

    // not a value - then it isn't default
    if (aNode.isNode()) return false;

    return this->isNodeDefault( aNode.toValue() );
}
//-----------------------------------------------------------------------------

UnoAny Tree::getNodeDefault(ValueRef const& aNode)      const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid ValueRef");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: ValueRef does not point to valid value");

    if (aNode.isValid())
    {
        ValueMemberNode  aValueMember = TreeImplHelper::member_node(aNode);

        if (aValueMember.canGetDefaultValue())
            return aValueMember.getDefaultValue();
    }

    return UnoAny();
}
//-----------------------------------------------------------------------------

UnoAny Tree::getNodeDefault(AnyNodeRef const& aNode)        const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: Node does not match Tree");


    // not a value - then there isn't a default
    if (aNode.isNode()) return UnoAny();

    return this->getNodeDefault( aNode.toValue() );
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

    return TreeImplHelper::makeNode(*pContext, nOffset);
}
//-----------------------------------------------------------------------------

AbsolutePath Tree::getRootPath() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    return isEmpty() ? AbsolutePath::root() : m_pImpl->getRootPath();
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
    if (aChange.test().isChange())
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
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING("configuration: Dispatching Visitor to node beyond available depth" );
    }

    typedef NodeVisitor::Result Result;
    Result aRet = NodeVisitor::CONTINUE;

    if (!aNode.isValid())
        OSL_TRACE("WARNING: Configuration: trying to iterate an invalid node !");

    else if (aNode.m_pImpl->isGroupNode())
    {
        GroupVisitorAdapter aAdapter(*this,aNode,aVisitor);

        OSL_ASSERT( NodeVisitor::DONE       == GroupMemberVisitor::DONE );
        OSL_ASSERT( NodeVisitor::CONTINUE   == GroupMemberVisitor::CONTINUE );

        aRet = Result( aNode.m_pImpl->groupImpl().dispatchToValues(aAdapter) );

        NodeOffset const nParent = aNode.m_nPos;
        TreeDepth const nDepth = childDepth(aNode.m_nDepth);

        for( NodeOffset nPos = m_pImpl->firstChild(nParent);
             nPos != 0 && aRet != NodeVisitor::DONE;
             nPos = m_pImpl->findNextChild(nParent,nPos))
        {
            aRet = visit( NodeRef( m_pImpl->node(nPos), nPos, nDepth ), aVisitor);
        }
    }

    else if (aNode.m_pImpl->isSetNode())
    {
        SetVisitorAdapter aAdapter(aVisitor);

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
        return TreeImplHelper::makeNode(*m_pImpl, nNode);
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

//-----------------------------------------------------------------------------
// hashing any pointer
//-----------------------------------------------------------------------------
static // for now
// should move this to a more public place sometime
// need  this, as STLPORT does not hash sal_(u)Int64 (at least on MSVC)
inline
size_t hash64(sal_uInt64 n)
{
    // simple solution (but the same that STLPORT uses for unsigned long long (if enabled))
    return static_cast<size_t>(n);
}
//-----------------------------------------------------------------------------

static // for now
// should move this to a more public place sometime
inline
size_t hashAnyPointer(void* p)
{
    // most portable quick solution IMHO (we need this cast for UNO tunnels anyway)
    sal_uInt64 n = reinterpret_cast<sal_uInt64>(p);

    return hash64(n);
}
//-----------------------------------------------------------------------------

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

bool NodeID::isEmpty() const
{
    OSL_ENSURE( m_pTree == NULL || m_pTree->isValidNode(m_nNode), "Node does not match tree in NodeID");
    return m_pTree == NULL;
}
//-----------------------------------------------------------------------------

bool NodeID::isValidNode() const
{
    return m_pTree != NULL && m_pTree->isValidNode(m_nNode);
}
//-----------------------------------------------------------------------------

// hashing
size_t NodeID::hashCode() const
{
    return hashAnyPointer(m_pTree) + 5*m_nNode;
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
bool operator < (NodeID const& lhs, NodeID const& rhs)
{
    using std::less;
    if (lhs.m_pTree == rhs.m_pTree)
        return lhs.m_nNode < rhs.m_nNode;
    else
        return less<TreeImpl*>()(lhs.m_pTree,rhs.m_pTree);
}

//-----------------------------------------------------------------------------
// class SubNodeID
//-----------------------------------------------------------------------------

SubNodeID::SubNodeID()
: m_sNodeName()
, m_aParentID(0,0)
{
}
//-----------------------------------------------------------------------------

SubNodeID::SubNodeID(Tree const& rTree, ValueRef const& rNode)
: m_sNodeName(rTree.getName(rNode))
, m_aParentID(rTree,rTree.getParent(rNode))
{
}
//-----------------------------------------------------------------------------

SubNodeID::SubNodeID(Tree const& rTree, NodeRef const& rParentNode, Name const& aName)
: m_sNodeName(aName)
, m_aParentID(rTree,rParentNode)
{
}
//-----------------------------------------------------------------------------

SubNodeID::SubNodeID(NodeID const& rParentNodeID, Name const& aName)
: m_sNodeName(aName)
, m_aParentID(rParentNodeID)
{
}
//-----------------------------------------------------------------------------

bool SubNodeID::isEmpty() const
{
    OSL_ENSURE(m_aParentID.isEmpty() || isValidNode(),"Invalid subnode ID");
    return m_aParentID.isEmpty();
}
//-----------------------------------------------------------------------------

bool SubNodeID::isValidNode() const
{
    if (!m_aParentID.isValidNode()) return false;

    OSL_ENSURE(!m_sNodeName.isEmpty(),"Invalid subnode ID: Missing Name");

    Tree aCheck( TreeImplHelper::tree(m_aParentID) );
    return aCheck.hasChild( TreeImplHelper::makeNode(m_aParentID),m_sNodeName );
}
//-----------------------------------------------------------------------------

// hashing
size_t SubNodeID::hashCode() const
{
    return m_aParentID.hashCode() + 5*m_sNodeName.hashCode();
}
//-----------------------------------------------------------------------------

bool operator < (SubNodeID const& lhs, SubNodeID const& rhs)
{
    if (lhs.m_aParentID == rhs.m_aParentID)
        return !!(lhs.m_sNodeName < rhs.m_sNodeName);
    else
        return lhs.m_aParentID < rhs.m_aParentID;
}

//-----------------------------------------------------------------------------
// Free functions
//-----------------------------------------------------------------------------

Name validateElementName(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  TreeImplHelper::isSet(aNode), "ERROR: Configuration: Set node expected.");

    return validateElementName(sName);
}
//-----------------------------------------------------------------------------

Name validateChildName(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  TreeImplHelper::isGroup(aNode), "ERROR: Configuration: Group node expected.");

    return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

Name validateChildOrElementName(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  isStructuralNode(aTree,aNode), "ERROR: Configuration: Inner node expected.");

    if (TreeImplHelper::isSet(aNode))
        return validateElementName(sName);

    else
        return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

Path::Component validateElementPathComponent(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    Name aElementName = validateElementName(sName,aTree,aNode);

    TemplateHolder aTemplate = SetElementInfo::extractElementInfo(aTree,aNode);
    if (aTemplate.isValid())
    {
        return Path::makeCompositeName( aElementName, aTemplate->getName() );
    }
    else
    {
        OSL_ENSURE(false, "WARNING: Cannot find element type information for building an element name");
        return Path::wrapElementName(aElementName);
    }
}
//-----------------------------------------------------------------------------

static void implValidateLocalPath(RelativePath& _rPath, Tree const& , NodeRef const& aNode)
{
    if (_rPath.isEmpty())
        throw InvalidName(_rPath.toString(), "is an empty path.");

    // FOR NOW: validate only the first component
    if (aNode.isValid() && !TreeImplHelper::isSet(aNode))
        if (!_rPath.getFirstName().isSimpleName())
            throw InvalidName(_rPath.toString(), "is not valid in this context. Predicate expression used to select group member.");
}
//-----------------------------------------------------------------------------

RelativePath validateRelativePath(OUString const& _sPath, Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  isStructuralNode(aTree,aNode), "ERROR: Configuration: Inner node expected.");

    if (  Path::isAbsolutePath(_sPath) )
    {
        OSL_ENSURE(false, "Absolute pathes are not allowed here (compatibility support enabled");
        return validateAndReducePath(_sPath,aTree,aNode);
    }

    RelativePath aResult = RelativePath::parse(_sPath);

    implValidateLocalPath(aResult,aTree,aNode);

    return aResult;
}
//-----------------------------------------------------------------------------

RelativePath validateAndReducePath(OUString const& _sPath, Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  isStructuralNode(aTree,aNode), "ERROR: Configuration: Inner node expected.");

    if ( !Path::isAbsolutePath(_sPath) )
        return validateRelativePath(_sPath,aTree,aNode);

    AbsolutePath aInputPath = AbsolutePath::parse(_sPath);

    RelativePath aStrippedPath = Path::stripPrefix( aInputPath, aTree.getAbsolutePath(aNode) );

    implValidateLocalPath(aStrippedPath,aTree,aNode);

    return aStrippedPath;
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode)
{
    return TreeImplHelper::isSet(aNode) ? aTree.hasElements(aNode) : aTree.hasChildren(aNode);
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode, Name const& aName)
{
    return TreeImplHelper::isSet(aNode) ? aTree.hasElement(aNode,aName) : aTree.hasChild(aNode,aName);
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode, Path::Component const& aName)
{
    return TreeImplHelper::isSet(aNode) ? aTree.hasElement(aNode,aName) : aTree.hasChild(aNode,aName.getName());
}
//-----------------------------------------------------------------------------

bool findInnerChildOrAvailableElement(Tree& aTree, NodeRef& aNode, Name const& aName)
{
    if ( TreeImplHelper::isSet(aNode) )
    {
        ElementRef aElement = aTree.getAvailableElement(aNode,aName);
        if (aElement.isValid())
        {
            aTree = aElement.getElementTree().getTree();
            aNode = aTree.getRootNode();
            return true;
        }
    }
    else
    {
        NodeRef aChild = aTree.getChildNode(aNode,aName);

        if ( aChild.isValid() )
        {
            aNode = aChild;
            return true;
        }
    }

    return false;
}
//-----------------------------------------------------------------------------

AnyNodeRef getChildOrElement(Tree& aTree, NodeRef const& aParentNode, Name const& aName)
{
    if (aTree.hasChildValue(aParentNode,aName))
    {
        return AnyNodeRef(aTree.getChildValue(aParentNode,aName));
    }

    else if ( TreeImplHelper::isSet(aParentNode) )
    {
        ElementRef aElement = aTree.getElement(aParentNode,aName);
        if (aElement.isValid())
        {
            aTree = aElement.getElementTree().getTree();
            return AnyNodeRef(aTree.getRootNode());
        }
    }

    else
    {
        NodeRef aChild = aTree.getChildNode(aParentNode,aName);

        if ( aChild.isValid() )
        {
            return AnyNodeRef(aChild);
        }
    }

    return AnyNodeRef();
}
//-----------------------------------------------------------------------------

static
inline
bool findLocalInnerChild(Tree const& aTree, NodeRef& aNode, Path::Component const& aName)
{
    NodeRef aChild = aTree.getChildNode(aNode,aName.getName());

    if ( !aChild.isValid() ) return false;

    OSL_ENSURE( aName.isSimpleName(), "Child of group was found by request using element name format -failing");
    if ( !aName.isSimpleName()) return false;

    aNode = aChild;

    return true;
}
//-----------------------------------------------------------------------------

static
inline
bool findElement(Tree& aTree, NodeRef& aNode, Path::Component const& aName)
{
    ElementRef aElement = aTree.getElement(aNode,aName.getName());

    if (!aElement.isValid()) return false;

    Tree aFoundTree = aElement.getElementTree().getTree();

    OSL_ENSURE(matches(aFoundTree.getRootName(),aName), "Element found, but type prefix does not match - failing");
    if ( !matches(aFoundTree.getRootName(),aName) ) return false;

    aTree = aFoundTree;
    aNode = aTree.getRootNode();

    return true;
}
//-----------------------------------------------------------------------------

static
bool findLocalInnerDescendant(Tree const& aTree, NodeRef& aNode, RelativePath& rPath)
{
    while ( !rPath.isEmpty() )
    {
        if (  TreeImplHelper::isSet(aNode) ) return false;

        if ( ! findLocalInnerChild(aTree,aNode,rPath.getFirstName()) ) return false;

        rPath.dropFirstName();
    }

    return true;
}
//-----------------------------------------------------------------------------

static
bool findDeepInnerDescendant(Tree& aTree, NodeRef& aNode, RelativePath& rPath)
{
    while ( !rPath.isEmpty() )
    {
        if (  TreeImplHelper::isSet(aNode) )
        {
            if ( ! findElement(aTree,aNode,rPath.getFirstName()) ) return false;
        }
        else
        {
            if ( ! findLocalInnerChild(aTree,aNode,rPath.getFirstName()) ) return false;
        }

        rPath.dropFirstName();
    }

    return true;
}
//-----------------------------------------------------------------------------

static
inline
bool identifiesLocalValue(Tree const& aTree, NodeRef const& aNode, RelativePath const& aPath)
{
    if ( aPath.getDepth() == 1 )
    {
        Path::Component const & aLocalName = aPath.getLocalName();
        Name aName = aLocalName.getName();

        if (aTree.hasChildValue(aNode,aName))
        {
            OSL_ENSURE( aLocalName.isSimpleName(), "Value in group was found by request using element name format");
            if ( aLocalName.isSimpleName())
                return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------

AnyNodeRef getLocalDescendant(Tree /*const*/& aTree, NodeRef& aNode, RelativePath& rPath)
{
    if ( findLocalInnerDescendant(aTree,aNode,rPath) )
    {
        OSL_ASSERT(aTree.isValidNode(aNode));
        return AnyNodeRef(aNode);
    }

    if ( identifiesLocalValue(aTree,aNode,rPath) )
    {
        ValueRef aValue = aTree.getChildValue(aNode,rPath.getLocalName().getName());
        OSL_ASSERT(aTree.isValidNode(aValue));
        return AnyNodeRef(aValue);
    }

    // compatibility hack
    if (aTree.hasElement(aNode,rPath.getFirstName()))
    {
        OSL_ENSURE(false, "WARNING: Hierarchical Access to set elements is not specified for this interface. This usage is deprecated");
        // compatibility access only
        return getDeepDescendant(aTree,aNode,rPath);
    }

    return AnyNodeRef();
}
//-----------------------------------------------------------------------------

AnyNodeRef getDeepDescendant(Tree& aTree, NodeRef& aNode, RelativePath& rPath)
{
    if ( findDeepInnerDescendant(aTree,aNode,rPath) )
    {
        OSL_ASSERT(aTree.isValidNode(aNode));
        return AnyNodeRef(aNode);
    }

    if ( identifiesLocalValue(aTree,aNode,rPath) )
    {
        ValueRef aValue = aTree.getChildValue(aNode,rPath.getLocalName().getName());
        OSL_ASSERT(aTree.isValidNode(aValue));
        return AnyNodeRef(aValue);
    }

    return AnyNodeRef();
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

void getAllChildrenHelper(NodeID const& aNode, SubNodeIDList& aList)
{
    aList.clear();

    if (TreeImpl* pTreeImpl = TreeImplHelper::tree(aNode))
    {
        if (NodeOffset const nParent = TreeImplHelper::offset(aNode))
        {
            OSL_ASSERT( pTreeImpl->isValidNode(nParent) );
            if (Node* const pNode =pTreeImpl->node(nParent))
            {
                if (pNode->isGroupNode())
                {
                    CollectValueIDs aCollector(aNode, aList);

                    pNode->groupImpl().dispatchToValues(aCollector);
                }
            }
            else
                OSL_ENSURE(false, "Unexpected: could not get node for offset");

            for(NodeOffset nOffset = pTreeImpl->firstChild(nParent);
                nOffset != 0;
                nOffset = pTreeImpl->findNextChild(nParent,nOffset))
            {
                OSL_ASSERT( pTreeImpl->isValidNode(nOffset) );
                aList.push_back( SubNodeID( aNode, pTreeImpl->getSimpleNodeName(nOffset)) );
            }
        }
    }
}

//-----------------------------------------------------------------------------
NodeID findNodeFromIndex(Tree const& aTree, NodeOffset nIndex)
{
    if (TreeImpl* pImpl = TreeImplHelper::impl(aTree))
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
bool isSimpleValue(Tree const& aTree, AnyNodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    return aNode.isValid() && (!aNode.isNode() || TreeImplHelper::isValueElement(aNode.toNode()));
}
//-----------------------------------------------------------------------------

static inline bool isRootNode(Tree const& aTree, NodeRef const& aNode)
{
    return TreeImplHelper::offset(aNode) == TreeImplHelper::impl(aTree)->root();
}
//-----------------------------------------------------------------------------

bool isSimpleValueElement(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( !aNode.isValid() ||
                TreeImplHelper::isGroup(aNode) ||
                TreeImplHelper::isSet(aNode) ||
                (TreeImplHelper::isValueElement(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && isRootNode(aTree,aNode) && TreeImplHelper::isValueElement(aNode);
}
//-----------------------------------------------------------------------------

bool isStructuralNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( !aNode.isValid() ||
                TreeImplHelper::isGroup(aNode) ||
                TreeImplHelper::isSet(aNode) ||
                (TreeImplHelper::isValueElement(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && ! TreeImplHelper::isValueElement(aNode);

}
//-----------------------------------------------------------------------------

bool isGroupNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( !aNode.isValid() ||
                TreeImplHelper::isGroup(aNode) ||
                TreeImplHelper::isSet(aNode) ||
                (TreeImplHelper::isValueElement(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && TreeImplHelper::isGroup(aNode);
}
//-----------------------------------------------------------------------------

bool isSetNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( !aNode.isValid() ||
                TreeImplHelper::isGroup(aNode) ||
                TreeImplHelper::isSet(aNode) ||
                (TreeImplHelper::isValueElement(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && TreeImplHelper::isSet(aNode);
}
//-----------------------------------------------------------------------------

UnoAny getSimpleElementValue(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND( aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");

    if (!aNode.isValid()) return UnoAny();

    OSL_PRECOND( isSimpleValueElement(aTree, aNode), "ERROR: Configuration: Getting value is supported only for value nodes");

    return TreeImplHelper::node(aNode)->valueElementImpl().getValue();
}

//-----------------------------------------------------------------------------

ISynchronizedData* getRootLock(Tree const& aTree)
{
    TreeImpl* pImpl = TreeImplHelper::impl(aTree);
    OSL_PRECOND( pImpl, "ERROR: Configuration: Tree locking requires a non-NULL Tree");
    if (pImpl == NULL) return 0;

    return pImpl->getRootLock();
}

//-----------------------------------------------------------------------------
    }   // namespace configuration
}       // namespace configmgr
