/*************************************************************************
 *
 *  $RCSfile: noderef.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:47 $
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

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif

#ifndef CONFIGMGR_VIEWACCESS_HXX_
#include "viewaccess.hxx"
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_CONFIGSET_HXX_
#include "configset.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm> // for swap
#define INCLUDED_ALGORITHM
#endif
#ifndef INCLUDED_FUNCTIONAL
#include <functional> // for less
#define INCLUDED_FUNCTIONAL
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// helpers first

view::ViewTreeAccess Tree::getView() const
{
    OSL_ENSURE(!isEmpty(),"Accessing view for tree: Tree must not be nil");
    return view::ViewTreeAccess(this->getDataAccessor(),*m_ref);
}
//-----------------------------------------------------------------------------
// class TreeImplHelper (declared in treeimpl.hxx)
//-----------------------------------------------------------------------------

NodeRef TreeImplHelper::makeNode(NodeOffset nOffset, TreeDepth nDepth)
{
    return NodeRef(nOffset,nDepth);
}
//-----------------------------------------------------------------------------

NodeRef TreeImplHelper::makeNode(TreeImpl& rTree, NodeOffset nOffset)
{
    if (nOffset)
    {
        OSL_ASSERT(rTree.isValidNode(nOffset));

        TreeDepth nDepth = remainingDepth(rTree.getAvailableDepth(), rTree.depthTo(nOffset));
        return NodeRef(nOffset,nDepth);
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

ValueRef TreeImplHelper::makeValue(Name const& aName, NodeOffset nParentOffset)
{
    return ValueRef(aName,nParentOffset);
}
//-----------------------------------------------------------------------------

AnyNodeRef TreeImplHelper::makeAnyNode(NodeOffset nOffset, TreeDepth nDepth)
{
    return AnyNodeRef(nOffset, nDepth);
}
//-----------------------------------------------------------------------------

AnyNodeRef TreeImplHelper::makeAnyNode(Name const& aName, NodeOffset nParentOffset)
{
    return AnyNodeRef(aName,nParentOffset);
}
//-----------------------------------------------------------------------------

TreeImpl* TreeImplHelper::impl(TreeRef const& aRef)
{
    return aRef.get();
}
//-----------------------------------------------------------------------------

TreeImpl* TreeImplHelper::impl(Tree const& aTree)
{
    return aTree.m_ref.get();
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

ValueMemberNode TreeImplHelper::member_node(Tree const & _aTree, ValueRef const& aValueNode)
{
    OSL_ENSURE(!_aTree.isEmpty(),"INTERNAL ERROR: member_node: NULL tree passed");
    view::ViewTreeAccess aView = _aTree.getView();

    return aView.getValue(aView.getGroupNodeAt(aValueNode.m_nParentPos), aValueNode.m_sNodeName );
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
            Tree aTree( anEntry.accessor(), pTree );
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
        , m_nParentPos( TreeImplHelper::offset(aParentNode) )
        {
            OSL_ASSERT(!aParentTree.isEmpty());
            OSL_ASSERT(aParentNode.isValid());
             OSL_ASSERT(aParentTree.isValidNode(aParentNode));

        }

        Result visit(ValueMemberNode const& anEntry);

        NodeVisitor&    m_rVisitor;
        Tree            m_aParentTree;
        NodeOffset      m_nParentPos;
    };

    GroupMemberVisitor::Result GroupVisitorAdapter::visit(ValueMemberNode const& aValue)
    {
        OSL_ASSERT(aValue.isValid());

        Result aResult = CONTINUE;

        Name aValueName = aValue.getNodeName();

        ValueRef const aValueRef = TreeImplHelper::makeValue(aValueName,m_nParentPos);

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
    struct FindNonDefaultElement : SetNodeVisitor
    {
        Result visit(SetEntry const& anEntry);

        static bool hasNonDefaultElement(view::ViewTreeAccess const& _aView, view::SetNode const& _aSet);
    };

    SetNodeVisitor::Result FindNonDefaultElement::visit(SetEntry const& anEntry)
    {
        OSL_ASSERT(anEntry.isValid());

        Result aResult = DONE; // if we find a NULL element we consider this

        if (TreeImpl* pTree = anEntry.tree())
        {
            Tree aTree( anEntry.accessor(), pTree );

            node::Attributes aElementAttributes = aTree.getAttributes(aTree.getRootNode());

            // a set element is considered default iff it is not replaced/added
            bool bDefault = !aElementAttributes.isReplacedForUser();

            aResult = bDefault ? CONTINUE : DONE;
        }
        else
            OSL_ENSURE(false,"Unexpected NULL SetEntry considered as non-default.");

        return aResult;
    }

    bool FindNonDefaultElement::hasNonDefaultElement(view::ViewTreeAccess const& _aView, view::SetNode const& _aSet)
    {
        FindNonDefaultElement aCheck;
        Result aRes = _aView.dispatchToElements(_aSet,aCheck);
        return aRes == DONE;
    }
//-----------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------
// class NodeRef
//-----------------------------------------------------------------------------

NodeRef::NodeRef()
: m_nPos(0)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

NodeRef::NodeRef(NodeOffset nPos, TreeDepth nDepth)
: m_nPos(nPos)
, m_nDepth(nDepth)
{}
//-----------------------------------------------------------------------------

NodeRef::NodeRef(NodeRef const& rOther)
: m_nPos(rOther.m_nPos)
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

    view::ViewTreeAccess aView = this->getView();

    return    aView.isSetNode(aNode) &&
            ! aView.isEmpty(aView.toSetNode(aNode));
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

    view::ViewTreeAccess aView = this->getView();

    return  aView.isSetNode(aNode) &&
            aView.findElement(aView.toSetNode(aNode),aName).isValid();
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

    view::ViewTreeAccess aView = this->getView();

    bool bFound = false;

    if (aView.isSetNode(aNode))
    {
        SetEntry aChildEntry = aView.findElement(aView.toSetNode(aNode),aName.getName());

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

    view::ViewTreeAccess aView = this->getView();

    ElementTreeImpl* pElementTree = NULL;

    if (aView.isSetNode(aNode))
    {
        SetEntry aChildEntry = aView.findElement(aView.toSetNode(aNode),aName);
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

    view::ViewTreeAccess aView = this->getView();

    ElementTreeImpl* pElementTree = NULL;

    if (aView.isSetNode(aNode))
    {
        SetEntry aChildEntry = aView.findAvailableElement(aView.toSetNode(aNode),aName);
        pElementTree = aChildEntry.tree();
    }

    return ElementRef(pElementTree); // even if nothing found
}
//-----------------------------------------------------------------------------

node::Attributes Tree::getAttributes(NodeRef const& aNode)  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (!aNode.isValid()) return NodeAttributes();

    return this->getView().getAttributes(aNode);
}
//-----------------------------------------------------------------------------

Name Tree::getName(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (isEmpty()) return Name();

    return m_ref->getSimpleNodeName(aNode.m_nPos);
}
//-----------------------------------------------------------------------------
// class ValueRef
//-----------------------------------------------------------------------------

bool ValueRef::checkValidState() const
{
    if (m_nParentPos  == 0)    return false;

    // old node semantics for now
    if ( m_sNodeName.isEmpty() ) return false;

    return true;
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef()
: m_sNodeName()
, m_nParentPos(0)
{
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef(Name const& aName, NodeOffset nParentPos)
: m_sNodeName(aName)
, m_nParentPos(nParentPos)
{
    OSL_ENSURE( nParentPos == 0 || checkValidState(), "Constructing invalid ValueRef");
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef(ValueRef const& rOther)
: m_sNodeName(rOther.m_sNodeName)
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
    std::swap(m_nParentPos, rOther.m_nParentPos);
}
//-----------------------------------------------------------------------------

ValueRef::~ValueRef()
{
}

//-----------------------------------------------------------------------------

node::Attributes Tree::getAttributes(ValueRef const& aValue)    const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aValue.isValid(), "ERROR: Configuration: ValueRef operation requires valid reference" );
    OSL_PRECOND( isValidNode(aValue), "ERROR: Configuration: ValueRef does not match tree" );

    if (!aValue.isValid()) return NodeAttributes();

    return TreeImplHelper::member_node(*this,aValue).getAttributes();
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

    return TreeImplHelper::member_node(*this,aValue).getValueType();
}

//-----------------------------------------------------------------------------
// class AnyNodeRef
//-----------------------------------------------------------------------------

bool AnyNodeRef::checkValidState() const
{
    if (m_nUsedPos  == 0)    return false;

    if ( !m_sNodeName.isEmpty() ) // it's a local value
    {
        // not used as runtime check as it should not be dangerous
        OSL_ENSURE(m_nDepth ==0, "AnyNodeRef that wraps a ValueRef should have no depth"); // value has no depth
    }

    return true;
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef()
: m_sNodeName()
, m_nUsedPos(0)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(NodeOffset nPos, TreeDepth nDepth)
: m_sNodeName()
, m_nUsedPos(nPos)
, m_nDepth(nDepth)
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(Name const& aName, NodeOffset nParentPos)
: m_sNodeName(aName)
, m_nUsedPos(nParentPos)
, m_nDepth(0)
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(AnyNodeRef const& rOther)
: m_sNodeName(rOther.m_sNodeName)
, m_nUsedPos(rOther.m_nUsedPos)
, m_nDepth(rOther.m_nDepth)
{
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(NodeRef const& aNodeRef)
: m_sNodeName()
, m_nUsedPos(  aNodeRef.m_nPos )
, m_nDepth( aNodeRef.m_nDepth )
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(ValueRef const& aValueRef)
: m_sNodeName( TreeImplHelper::value_name(aValueRef) )
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

    return TreeImplHelper::makeNode(m_nUsedPos,m_nDepth);
}
//-----------------------------------------------------------------------------

ValueRef AnyNodeRef::toValue() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: AnyNodeRef operation requires valid node" );
    if (!isValid() || isNode()) return ValueRef();

    return TreeImplHelper::makeValue(m_sNodeName, m_nUsedPos);
}
//-----------------------------------------------------------------------------

node::Attributes Tree::getAttributes(AnyNodeRef const& aNode)   const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: NodeRef operation requires valid node" );
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (!aNode.isValid()) return NodeAttributes();

    if (aNode.isNode())
        return this->getView().getAttributes(aNode.toNode());

    else
        return TreeImplHelper::member_node(*this,aNode.toValue()).getAttributes();
}
//-----------------------------------------------------------------------------

Name Tree::getName(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

    if (isEmpty() || !aNode.isValid()) return Name();

    if (aNode.isNode())
        return m_ref->getSimpleNodeName(aNode.m_nUsedPos);

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
// class TreeRef
//-----------------------------------------------------------------------------

TreeRef::TreeRef(TreeImpl* pImpl)
: m_pImpl(pImpl)
{
    if (m_pImpl) m_pImpl->acquire();
}
//-----------------------------------------------------------------------------

// just DTRT
TreeRef::TreeRef(TreeRef const& aTree)
: m_pImpl(aTree.m_pImpl)
{
    if (m_pImpl) m_pImpl->acquire();
}
//-----------------------------------------------------------------------------

// the usual exception-safe swap-based one
TreeRef& TreeRef::operator=(TreeRef const& rOther)
{
    TreeRef(rOther).swap(*this);
    return *this;
}
//-----------------------------------------------------------------------------

void TreeRef::swap(TreeRef& rOther)
{
    std::swap(m_pImpl,      rOther.m_pImpl);
}
//-----------------------------------------------------------------------------

TreeRef::~TreeRef()
{
    if (m_pImpl) m_pImpl->release();
}
//-----------------------------------------------------------------------------

bool TreeRef::isEmpty() const
{
    return m_pImpl == 0 || m_pImpl->nodeCount() == 0;
}
//-----------------------------------------------------------------------------

NodeOffset TreeRef::getContainedInnerNodeCount() const
{
    OSL_PRECOND(isValid(), "ERROR: Configuration: Counting nodes requires a valid Tree");

    return m_pImpl ? m_pImpl->nodeCount() : 0;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class Tree
//-----------------------------------------------------------------------------

Tree::Tree(data::Accessor const& _accessor, TreeImpl* pImpl)
: m_accessor(_accessor)
, m_ref(pImpl)
{
}
//-----------------------------------------------------------------------------

// just DTRT
Tree::Tree(data::Accessor const& _accessor, TreeRef const& _ref)
: m_accessor(_accessor)
, m_ref(_ref)
{
}
//-----------------------------------------------------------------------------

void TreeRef::disposeData()
{
    if (TreeImpl* pImpl = get())
    {
        pImpl->disposeData();
//      pImpl->release();
    }
}
//-----------------------------------------------------------------------------

bool Tree::isValidNode(ValueRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!this->isValid()) return false;

    if (!aNode.isValid()) return false;
    if (!aNode.checkValidState()) return false;

    if (!m_ref->isValidNode(aNode.m_nParentPos)) return false;

    if (!this->getView().isGroupNodeAt(aNode.m_nParentPos)) return false;
    if (!TreeImplHelper::member_node(*this,aNode).isValid()) return false;

    OSL_ASSERT(!aNode.m_sNodeName.isEmpty()); // old value handling ?

    return true;
}
//-----------------------------------------------------------------------------

bool TreeRef::isValidNode(NodeRef const& aNode) const
{
    OSL_PRECOND(!isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!this->isValid()) return false;
    if (!aNode.isValid()) return false;

    if (!m_pImpl->isValidNode(aNode.m_nPos)) return false;

    return true;
}
//-----------------------------------------------------------------------------

bool Tree::isValidNode(NodeRef const& aNode) const
{
    return m_ref.isValidNode(aNode);
}
//-----------------------------------------------------------------------------

bool Tree::isValidNode(AnyNodeRef const& aNode) const
{
    OSL_PRECOND(!isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!this->isValid()) return false;

    if (!aNode.isValid()) return false;
    if (!aNode.checkValidState()) return false;

    if (!m_ref->isValidNode(aNode.m_nUsedPos)) return false;

    if (!aNode.isNode() && !this->getView().isGroupNodeAt(aNode.m_nUsedPos)) return false;
    if (!aNode.isNode() && !TreeImplHelper::member_node(*this,aNode.toValue()).isValid()) return false;

    return true;
}
//-----------------------------------------------------------------------------

bool Tree::hasChildren(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;

    view::ViewTreeAccess aView = this->getView();

    if (!aView.isGroupNode(aNode)) return false;

    return aView.toGroupNode(aNode).getFirstChild().is();
}
//-----------------------------------------------------------------------------

bool Tree::hasChildNode(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;

    view::ViewTreeAccess aView = this->getView();

    if (!aView.isGroupNode(aNode)) return false;

    return aView.toGroupNode(aNode).findChild(aName).is();
}
//-----------------------------------------------------------------------------

bool Tree::hasChildValue(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;

    view::ViewTreeAccess aView = this->getView();

    if (! aView.isGroupNode(aNode) ) return false;

    return aView.hasValue(aView.toGroupNode(aNode), aName);
}
//-----------------------------------------------------------------------------

bool Tree::hasChild(NodeRef const& aNode, Name const& aName) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    if (aNode.m_nDepth == 0)
    {
        CFG_TRACE_WARNING( "configuration: Querying node beyond available depth" );
    }

    if (this->isEmpty()) return false;
    if (!aNode.isValid()) return false;

    view::ViewTreeAccess aView = this->getView();

    if ( !aView.isGroupNode(aNode) ) return false;

    view::GroupNode aGroupNode = aView.toGroupNode(aNode);

    if (aView.hasValue(aGroupNode, aName)) return true;

    return aGroupNode.findChild(aName).is();
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

    view::ViewTreeAccess aView = this->getView();

    NodeOffset nOffset  = 0;

    if ( aView.isGroupNode(aNode) )
    {
        nOffset = aView.toGroupNode(aNode).findChild(aName).get_offset();
    }

    return NodeRef(nOffset, childDepth(aNode.m_nDepth));
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

    view::ViewTreeAccess aView = this->getView();

    if (! aView.isGroupNode(aNode) ) return ValueRef();

    if (!aView.hasValue(aView.toGroupNode(aNode), aName)) return ValueRef();

    return ValueRef(aName, aNode.m_nPos);
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

    view::ViewTreeAccess aView = this->getView();

    NodeOffset nOffset  = 0;

    if (aView.isGroupNode(aNode))
    {
        if (aView.hasValue(aView.toGroupNode(aNode), aName))
        {
            return AnyNodeRef(aName, aNode.m_nPos);

        }

        nOffset = aView.toGroupNode(aNode).findChild(aName).get_offset();
    }

    return AnyNodeRef(nOffset, childDepth(aNode.m_nDepth));
}
//-----------------------------------------------------------------------------

NodeRef Tree::getParent(ValueRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    NodeOffset nParent = aNode.m_nParentPos;

    OSL_ASSERT(nParent == 0 || !aNode.m_sNodeName.isEmpty());

    return TreeImplHelper::makeNode(*m_ref, nParent);
}
//-----------------------------------------------------------------------------

NodeRef Tree::getParent(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( getView().makeNode(getRootNode()).getParent().is() == false );

    view::Node aParent = getView().makeNode(aNode).getParent();

    OSL_ENSURE(  m_ref->isValidNode(aParent.get_offset()), "ERROR: Configuration: NodeRef has invalid parent");

    return NodeRef(aParent.get_offset(), parentDepth(aNode.m_nDepth));
}
//-----------------------------------------------------------------------------

NodeRef Tree::getParent(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    OSL_ASSERT( getView().makeNode(getRootNode()).getParent().is() == false );

    view::Node aParent = getView().makeNode(aNode.m_nUsedPos);

    if (aNode.m_sNodeName.isEmpty() && aNode.isValid())
        aParent = aParent.getParent();

    return TreeImplHelper::makeNode(*m_ref, aParent.get_offset());
}
//-----------------------------------------------------------------------------

UnoAny Tree::getNodeValue(ValueRef const& aNode) const
{
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid Value Ref");
    if (!aNode.isValid()) return UnoAny();

    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: Value Ref does not point to valid value");

    return TreeImplHelper::member_node(*this,aNode).getValue();
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
            m_ref->prependLocalPathTo( aNode.m_nPos, aNames );

        aNames.prepend( m_ref->getRootPath().rep() );
    }
    return AbsolutePath(aNames);
}
//-----------------------------------------------------------------------------

NodeRef TreeRef::getRootNode() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    if (isEmpty()) return NodeRef();

    return NodeRef( m_pImpl->root_(), m_pImpl->getAvailableDepth() );
}
//-----------------------------------------------------------------------------

Path::Component Tree::getRootName() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    if (isEmpty()) return Path::makeEmptyComponent();

    return m_ref->getExtendedRootName();
}
//-----------------------------------------------------------------------------

bool TreeRef::isRootNode(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");
    return !isEmpty() && aNode.isValid() && m_pImpl->root_() == aNode.m_nPos;
}
//-----------------------------------------------------------------------------

bool Tree::hasNodeDefault(ValueRef const& aNode)        const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid ValueRef");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: ValueRef does not point to valid value");

    bool bHasDefault = false;
    if (aNode.isValid())
    {
        ValueMemberNode  aValueMember = TreeImplHelper::member_node(*this,aNode);

        bHasDefault = aValueMember.canGetDefaultValue();
    }

    return bHasDefault;
}
//-----------------------------------------------------------------------------

bool Tree::isNodeDefault(ValueRef const& aNode) const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid ValueRef");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: ValueRef does not point to valid value");

    if (!hasNodeDefault(aNode)) return false;

    return  TreeImplHelper::member_node(*this,aNode).isDefault();
}
//-----------------------------------------------------------------------------

bool Tree::hasNodeDefault(NodeRef const& aNode)     const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid ValueRef");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: ValueRef does not point to valid value");

    // not a set - then it has no default
    return this->getView().isSetNode(aNode);
}
//-----------------------------------------------------------------------------

bool Tree::isNodeDefault(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: Node does not match Tree");

    if (!hasNodeDefault(aNode)) return false;

    view::ViewTreeAccess aView = this->getView();

    // not a set - then it isn't default
    OSL_ASSERT(aView.isSetNode(aNode));

    // a set is defaults, if all its elements are default
    return !FindNonDefaultElement::hasNonDefaultElement(aView,aView.toSetNode(aNode));
}
//-----------------------------------------------------------------------------

bool Tree::hasNodeDefault(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: Node does not match Tree");

    if (aNode.isNode())
        return this->hasNodeDefault( aNode.toNode() );
    else
        return this->hasNodeDefault( aNode.toValue() );
}
//-----------------------------------------------------------------------------

bool Tree::isNodeDefault(AnyNodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: Node does not match Tree");

    if (aNode.isNode())
        return this->isNodeDefault( aNode.toNode() );
    else
        return this->isNodeDefault( aNode.toValue() );
}
//-----------------------------------------------------------------------------

bool Tree::areValueDefaultsAvailable(NodeRef const& aNode) const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND(  isValidNode(aNode), "ERROR: Configuration: Node does not match Tree");

    view::ViewTreeAccess aView = this->getView();

    OSL_PRECOND( aView.isGroupNode(aNode),
                "WARNING: Configuration: Group Node expected. Result is not meaningful");

    return  aView.isGroupNode(aNode) &&
            aView.areValueDefaultsAvailable( aView.toGroupNode(aNode) );
}
//-----------------------------------------------------------------------------

UnoAny Tree::getNodeDefaultValue(ValueRef const& aNode)     const // only works for value nodes
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Value operation requires a valid ValueRef");
    OSL_PRECOND( isValidNode(aNode), "ERROR: Configuration: ValueRef does not point to valid value");

    if (aNode.isValid())
    {
        ValueMemberNode  aValueMember = TreeImplHelper::member_node(*this,aNode);

        if (aValueMember.canGetDefaultValue())
            return aValueMember.getDefaultValue();
    }

    return UnoAny();
}
//-----------------------------------------------------------------------------

TreeRef TreeRef::getContextTree() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    TreeImpl* pContext = isValid() ? m_pImpl->getContextTree() : 0;

    return TreeRef(pContext);
}
//-----------------------------------------------------------------------------

Tree Tree::getContextTree() const
{
    return Tree(m_accessor, m_ref.getContextTree());
}
//-----------------------------------------------------------------------------

NodeRef TreeRef::getContextNode() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!isValid()) return NodeRef();

    NodeOffset nOffset = m_pImpl->getContextNode();
    TreeImpl* pContext = m_pImpl->getContextTree();

    // if we have a context, we must have a (parent's) position in it
    OSL_ASSERT( pContext == 0 || nOffset != 0);
    if (pContext == 0) return NodeRef();

    return TreeImplHelper::makeNode(*pContext, nOffset);
}
//-----------------------------------------------------------------------------

NodeRef Tree::getContextNode() const
{
    return m_ref.getContextNode();
}
//-----------------------------------------------------------------------------

AbsolutePath Tree::getRootPath() const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    return isEmpty() ? AbsolutePath::root() : m_ref->getRootPath();
}
//-----------------------------------------------------------------------------

bool Tree::hasChanges()  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    return !isEmpty() && this->getView().hasChanges();
}
//-----------------------------------------------------------------------------

bool Tree::collectChanges(NodeChanges& aChanges)  const
{
    OSL_PRECOND( !isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");

    if (!isEmpty() && this->getView().hasChanges())
    {
        this->getView().collectChanges(aChanges);
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
            this->getView().markChanged(aNode);
        }
        else
        {
            Tree aAffectedTree      = aChange.getAffectedTree();
            NodeRef aAffectedNode   = aChange.getAffectedNode();
            OSL_ASSERT(!aAffectedTree.isEmpty() && aAffectedTree.isValidNode(aAffectedNode));

            aAffectedTree.getView().markChanged(aAffectedNode);

            OSL_ASSERT(this->getView().hasChanges());
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

    view::ViewTreeAccess aView = this->getView();

    typedef NodeVisitor::Result Result;
    Result aRet = NodeVisitor::CONTINUE;

    if (!aNode.isValid())
        OSL_TRACE("WARNING: Configuration: trying to iterate an invalid node !");

    else if (aView.isGroupNode(aNode))
    {
        GroupVisitorAdapter aAdapter(*this,aNode,aVisitor);

        OSL_ASSERT( NodeVisitor::DONE       == GroupMemberVisitor::DONE );
        OSL_ASSERT( NodeVisitor::CONTINUE   == GroupMemberVisitor::CONTINUE );

        view::GroupNode const aParent = aView.toGroupNode(aNode);

        aRet = Result( aView.dispatchToValues(aParent,aAdapter) );

        TreeDepth const nDepth = childDepth(aNode.m_nDepth);

        for( view::Node aChild = aParent.getFirstChild();
             aChild.is() && aRet != NodeVisitor::DONE;
             aChild = aParent.getNextChild(aChild))
        {
            aRet = visit( NodeRef( aChild.get_offset(), nDepth ), aVisitor);
        }
    }

    else if (aView.isSetNode(aNode))
    {
        SetVisitorAdapter aAdapter(aVisitor);

        OSL_ASSERT( NodeVisitor::DONE == SetNodeVisitor::DONE );
        OSL_ASSERT( NodeVisitor::CONTINUE == SetNodeVisitor::CONTINUE );

        aRet = Result(aView.dispatchToElements(aView.toSetNode(aNode),aAdapter));
    }

    else
        OSL_TRACE("WARNING: Configuration: trying to iterate a Value node !");

    return aRet;
}
//-----------------------------------------------------------------------------

void Tree::rebind(data::Accessor const& _aAccessor)
{
    m_accessor = _aAccessor;
}
//-----------------------------------------------------------------------------

void Tree::unbind()
{
    m_accessor.clear();
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

NodeID::NodeID(TreeRef const& rTree, NodeRef const& rNode)
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

        n -= m_pTree->root_();
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

SubNodeID::SubNodeID(TreeRef const& rTree, NodeRef const& rParentNode, Name const& aName)
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
    OSL_ENSURE( m_aParentID.isEmpty() ||
                (m_aParentID.isValidNode() && !m_sNodeName.isEmpty()),"Invalid subnode ID");
    return m_aParentID.isEmpty();
}
//-----------------------------------------------------------------------------

bool SubNodeID::isValidNode(data::Accessor const& _accessor) const
{
    if (!m_aParentID.isValidNode()) return false;

    OSL_ENSURE(!m_sNodeName.isEmpty(),"Invalid subnode ID: Missing Name");

    Tree aCheck( _accessor, TreeImplHelper::tree(m_aParentID) );
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

    OSL_PRECOND(  aTree.getView().isSetNode(aNode), "ERROR: Configuration: Set node expected.");

    return validateElementName(sName);
}
//-----------------------------------------------------------------------------

Name validateChildName(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  aTree.getView().isGroupNode(aNode), "ERROR: Configuration: Group node expected.");

    return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

Name validateChildOrElementName(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND(  aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  isStructuralNode(aTree,aNode), "ERROR: Configuration: Inner node expected.");

    if (aTree.getView().isSetNode(aNode))
        return validateElementName(sName);

    else
        return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

Path::Component validateElementPathComponent(OUString const& sName, Tree const& aTree, NodeRef const& aNode )
{
    Name aElementName = validateElementName(sName,aTree,aNode);

    TemplateHolder aTemplate = SetElementInfo::extractElementInfo(aTree,aNode);
    if (aTemplate.is())
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

static void implValidateLocalPath(RelativePath& _rPath, Tree const& aTree, NodeRef const& aNode)
{
    if (_rPath.isEmpty())
        throw InvalidName(_rPath.toString(), "is an empty path.");

    // FOR NOW: validate only the first component
    if (!aTree.getView().isSetNode(aNode))
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
    return aTree.getView().isSetNode(aNode) ? aTree.hasElements(aNode) : aTree.hasChildren(aNode);
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode, Name const& aName)
{
    return aTree.getView().isSetNode(aNode) ? aTree.hasElement(aNode,aName) : aTree.hasChild(aNode,aName);
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(Tree const& aTree, NodeRef const& aNode, Path::Component const& aName)
{
    return aTree.getView().isSetNode(aNode) ? aTree.hasElement(aNode,aName) : aTree.hasChild(aNode,aName.getName());
}
//-----------------------------------------------------------------------------

bool findInnerChildOrAvailableElement(Tree& aTree, NodeRef& aNode, Name const& aName)
{
    if ( aTree.getView().isSetNode(aNode) )
    {
        ElementRef aElement = aTree.getAvailableElement(aNode,aName);
        if (aElement.isValid())
        {
            aTree = aElement.getElementTree(aTree.getDataAccessor()).getTree();
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

    else if ( aTree.getView().isSetNode(aParentNode) )
    {
        ElementRef aElement = aTree.getElement(aParentNode,aName);
        if (aElement.isValid())
        {
            aTree = aElement.getElementTree(aTree.getDataAccessor()).getTree();
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

    Tree aFoundTree = aElement.getElementTree(aTree.getDataAccessor()).getTree();

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
        if (  aTree.getView().isSetNode(aNode) ) return false;

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
        if ( aTree.getView().isSetNode(aNode) )
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

AnyNodeRef getLocalDescendant(Tree const& aTree, NodeRef const& aNode, RelativePath const& rPath)
{
    NodeRef aNestedNode( aNode );
    RelativePath aRemainingPath(rPath);

    if ( findLocalInnerDescendant(aTree,aNestedNode,aRemainingPath) )
    {
        OSL_ASSERT(aTree.isValidNode(aNestedNode));
        return AnyNodeRef(aNestedNode);
    }

    if ( identifiesLocalValue(aTree,aNestedNode,aRemainingPath) )
    {
        ValueRef aValue = aTree.getChildValue(aNestedNode,rPath.getLocalName().getName());
        OSL_ASSERT(aTree.isValidNode(aValue));
        return AnyNodeRef(aValue);
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

        NodeOffset const nEnd = pImpl->root_() + nCount;

        for(NodeOffset nOffset = pImpl->root_();
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

void getAllChildrenHelper(data::Accessor const& _aAccessor, NodeID const& aNode, SubNodeIDList& aList)
{
    aList.clear();

    if (TreeImpl* pTreeImpl = TreeImplHelper::tree(aNode))
    {
        view::ViewTreeAccess aView(_aAccessor, *pTreeImpl);

        if (NodeOffset const nParent = TreeImplHelper::offset(aNode))
        {
            OSL_ASSERT( pTreeImpl->isValidNode(nParent) );

            if (aView.isGroupNodeAt(nParent))
            {
                view::GroupNode aParent = aView.getGroupNodeAt(nParent);

                {
                    CollectValueIDs aCollector(aNode, aList);
                    aView.dispatchToValues(aView.getGroupNodeAt(nParent),aCollector);
                }

                for(view::Node aChild = aParent.getFirstChild();
                    aChild.is();
                    aChild = aParent.getNextChild(aChild))
                {
                    OSL_ASSERT( pTreeImpl->isValidNode(aChild.get_offset()) );
                    aList.push_back( SubNodeID( aNode, pTreeImpl->getSimpleNodeName(aChild.get_offset())) );
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
NodeID findNodeFromIndex(TreeRef const& aTree, NodeOffset nIndex)
{
    if (TreeImpl* pImpl = TreeImplHelper::impl(aTree))
    {
        NodeOffset nNode = nIndex + pImpl->root_();
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
    return aNode.isValid() && (!aNode.isNode() || aTree.getView().isValueNode(aNode.toNode()));
}
//-----------------------------------------------------------------------------

static inline bool isRootNode(Tree const& aTree, NodeRef const& aNode)
{
    return TreeImplHelper::offset(aNode) == TreeImplHelper::impl(aTree)->root_();
}
//-----------------------------------------------------------------------------

bool isSimpleValueElement(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = aTree.getView();

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && isRootNode(aTree,aNode) && aView.isValueNode(aNode);
}
//-----------------------------------------------------------------------------

bool isStructuralNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = aTree.getView();

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && ! aView.isValueNode(aNode);
}
//-----------------------------------------------------------------------------

bool isGroupNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = aTree.getView();

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && aView.isGroupNode(aNode);
}
//-----------------------------------------------------------------------------

bool isSetNode(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = aTree.getView();

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aTree,aNode)) );

    return aNode.isValid() && aView.isSetNode(aNode);
}
//-----------------------------------------------------------------------------

UnoAny getSimpleElementValue(Tree const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND( aTree.isValidNode(aNode), "WARNING: Configuration: NodeRef does not match Tree");

    if (!aNode.isValid()) return UnoAny();

    OSL_PRECOND( isSimpleValueElement(aTree, aNode), "ERROR: Configuration: Getting value is supported only for value nodes");

    view::ViewTreeAccess aView = aTree.getView();

    return aView.getValue(aView.toValueNode(aNode));
}

//-----------------------------------------------------------------------------

osl::Mutex& getRootLock(TreeRef const& aTree)
{
    TreeImpl* pImpl = TreeImplHelper::impl(aTree);
    OSL_PRECOND( pImpl, "ERROR: Configuration: Tree locking requires a non-NULL Tree");

    return pImpl->getRootLock();
}
//-----------------------------------------------------------------------------

memory::Segment const * getRootSegment(TreeRef const& aTree)
{
    TreeImpl* pImpl = TreeImplHelper::impl(aTree);
    OSL_PRECOND( pImpl, "ERROR: Configuration: Tree locking requires a non-NULL Tree");

    return pImpl ? pImpl->getRootSegment() : NULL;
}

//-----------------------------------------------------------------------------
    }   // namespace configuration
}       // namespace configmgr
