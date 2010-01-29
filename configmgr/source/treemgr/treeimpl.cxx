/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treeimpl.cxx,v $
 * $Revision: 1.33 $
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
#include <stdio.h>
#include "anynoderef.hxx"
#include "builddata.hxx"
#include "configset.hxx"
#include "tracer.hxx"
#include "tree.hxx"
#include "roottreeimpl.hxx"
#include "nodeimpl.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "noderef.hxx"
#include "template.hxx"
#include "nodevisitor.hxx"
#include "valueref.hxx"
#include "valuenode.hxx"
#include "change.hxx"
#include "valuenodeimpl.hxx"
#include "setnodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "viewaccess.hxx"
#include "viewfactory.hxx"
#include "nodefactory.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------
// class TreeImplBuilder - friend of Tree
//-----------------------------------------------------------------------------

/** is a visitor-style algorithm to construct a <type>Tree::NodeList</type>
    representing a configuration hierarchy
*/
class TreeImplBuilder : public data::NodeVisitor
{
public:
    /** constructs a TreeImplBuilder to append onto <var>rList</var>
        the products of <var>rFactory</var> up to depth <var>nDepth</var>
    */
    TreeImplBuilder(
            TemplateProvider const& aTemplateProvider,
            rtl::Reference<view::ViewStrategy> const& _xStrategy,
            Tree& rTree
         )
        : m_xStrategy(_xStrategy)
        , m_aTemplateProvider(aTemplateProvider)
        , m_rFactory(_xStrategy->getNodeFactory())
        , m_rTree(rTree)
        , m_nParent(0)
        , m_nDepthLeft(rTree.m_nDepth)
    {
        OSL_ASSERT(m_rTree.m_aNodes.empty());
        OSL_DEBUG_ONLY(m_bMemberCheck = false);
        m_rTree.m_xStrategy = _xStrategy;
    }

private:
    using NodeVisitor::handle;

    virtual bool handle(sharable::ValueNode * node);

    virtual bool handle(sharable::GroupNode * node);

    virtual bool handle(sharable::SetNode * node);

    /// add a Node for group node <var>_aGroup</var> to the list
    void addGroup(sharable::GroupNode * group);
    /// add a Node for set node <var>_aSet</var> to the list
    void addSet(sharable::SetNode * set);
    /// add a Node for value node <var>rValue</var> to the list
    void addValueElement(sharable::ValueNode * value);
    /// add a Member for value node <var>rValue</var> to the list
    void addValueMember(sharable::ValueNode * value);

    rtl::Reference<view::ViewStrategy>  m_xStrategy;
    TemplateProvider m_aTemplateProvider;
    view::NodeFactory&  m_rFactory;
    Tree&       m_rTree;
    unsigned int        m_nParent;
    unsigned int        m_nDepthLeft;
#if OSL_DEBUG_LEVEL > 0
    bool m_bMemberCheck;
#endif
};
//-----------------------------------------------------------------------------

bool TreeImplBuilder::handle(sharable::ValueNode * node)
{
    if (m_nParent == 0)
        addValueElement(node); // if it is the root it is a value set element
    else
        addValueMember(node); // if it is not the root it is a group member

    return false;
}
//-----------------------------------------------------------------------------

bool TreeImplBuilder::handle(sharable::GroupNode * node)
{
    addGroup(node);
    return false;
}
//-----------------------------------------------------------------------------

bool TreeImplBuilder::handle(sharable::SetNode * node)
{
    addSet(node);
    return false;
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addValueElement(sharable::ValueNode * value)
{
    rtl::Reference<NodeImpl> aValueNode( m_rFactory.makeValueNode(value) );
    OSL_ENSURE( aValueNode.is(), "could not make value node wrapper" );

    OSL_ENSURE( m_nParent == 0, "Adding value element that is not root of its fragment" );
    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aValueNode.is() )
    {
        m_rTree.m_aNodes.push_back( NodeData(aValueNode, value->info.getName(), m_nParent) );
    }
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addValueMember(sharable::ValueNode *)
{
    // nothing to do
    OSL_DEBUG_ONLY(m_bMemberCheck = true);
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addGroup(sharable::GroupNode * group)
{
    rtl::Reference<NodeImpl> aGroupNode( m_rFactory.makeGroupNode(group) );
    OSL_ENSURE( aGroupNode.is(), "could not make group node wrapper" );

    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aGroupNode.is() )
    {
        m_rTree.m_aNodes.push_back( NodeData(aGroupNode,group->info.getName(),m_nParent) );

        // now fill in group members
        if (m_nDepthLeft > 0)
        {
            unsigned int nSaveParent = m_nParent;
            decDepth(m_nDepthLeft);

            m_nParent = m_rTree.m_aNodes.size() + Tree::ROOT - 1;

        #if OSL_DEBUG_LEVEL > 0
            bool bSaveMemberCheck = m_bMemberCheck;
            m_bMemberCheck = false;
        #endif

            // now recurse:
            this->visitChildren(group);

            OSL_ENSURE(m_nParent < m_rTree.m_aNodes.size() || m_bMemberCheck,
                        "WARNING: Configuration: Group within requested depth has no members");

            OSL_DEBUG_ONLY(m_bMemberCheck = bSaveMemberCheck);

            incDepth(m_nDepthLeft);
            m_nParent = nSaveParent;
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addSet(sharable::SetNode * set)
{
    rtl::Reference<Template> aTemplate = makeSetElementTemplate(set, m_aTemplateProvider);
    OSL_ASSERT(aTemplate.is());
    OSL_ENSURE(aTemplate->isInstanceTypeKnown(),"ERROR: Cannor create set instance without knowing the instance type");

    rtl::Reference<NodeImpl> aSetNode( m_rFactory.makeSetNode(set, aTemplate.get()) );
    OSL_ENSURE( aSetNode.is(), "could not make set node wrapper" );

    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aSetNode.is() )
    {
        m_rTree.m_aNodes.push_back( NodeData(aSetNode, set->info.getName(), m_nParent) );

        // this also relies on one based offsets
        unsigned int nNodeAdded = m_rTree.m_aNodes.size() + Tree::ROOT - 1;

        OSL_ASSERT(&m_rTree.m_aNodes.back().nodeImpl() == aSetNode.get());
        static_cast<SetNodeImpl&>(*aSetNode).initElements(m_aTemplateProvider, m_rTree, nNodeAdded, m_nDepthLeft);
    }
}

namespace {
    class FindNonDefaultElement: public SetNodeVisitor {
    public:
        static bool hasNonDefaultElement(
            view::ViewTreeAccess const & view, view::SetNode const & set)
        {
            FindNonDefaultElement visitor;
            return view.dispatchToElements(set, visitor) == DONE;
        }

    protected:
        virtual Result visit(SetEntry const & entry) {
            OSL_ASSERT(entry.isValid());
            rtl::Reference< Tree > tree(entry.tree());
            OSL_ASSERT(tree.is());
            node::Attributes atts(tree->getAttributes(tree->getRootNode()));
            // A set element is considered default iff it is not replaced/added:
            bool dflt = !atts.isReplacedForUser();
            return dflt ? CONTINUE : DONE;
        }
    };

    class SetVisitorAdapter: public SetNodeVisitor {
    public:
        explicit SetVisitorAdapter(NodeVisitor & visitor): m_visitor(visitor) {}

    protected:
        virtual Result visit(SetEntry const & entry) {
            OSL_ASSERT(entry.isValid());
            rtl::Reference< Tree > tree(entry.tree());
            if (tree.is()) {
                OSL_ASSERT(
                    Result(NodeVisitor::DONE) == SetNodeVisitor::DONE &&
                    Result(NodeVisitor::CONTINUE) == SetNodeVisitor::CONTINUE);
                return Result(tree->visit(tree->getRootNode(), m_visitor));
            } else {
                return CONTINUE;
            }
        }

    private:
        NodeVisitor & m_visitor;
    };

    class GroupVisitorAdapter: public GroupMemberVisitor {
    public:
        GroupVisitorAdapter(
            rtl::Reference< Tree > const & parentTree,
            NodeRef const & parentNode, NodeVisitor & visitor):
            m_parentTree(parentTree), m_parentPos(parentNode.getOffset()),
            m_visitor(visitor)
        {
            OSL_ASSERT(
                !isEmpty(parentTree.get()) && parentNode.isValid() &&
                parentTree->isValidNode(parentNode.getOffset()));
        }

    protected:
        virtual Result visit(ValueMemberNode const & value) {
            OSL_ASSERT(
                value.isValid() &&
                Result(NodeVisitor::DONE) == GroupMemberVisitor::DONE &&
                Result(NodeVisitor::CONTINUE) == GroupMemberVisitor::CONTINUE);
            return Result(
                m_parentTree->visit(
                    ValueRef(value.getNodeName(), m_parentPos), m_visitor));
        }

    private:
        rtl::Reference< Tree > m_parentTree;
        unsigned int m_parentPos;
        NodeVisitor & m_visitor;
    };
}

//-----------------------------------------------------------------------------
// class NodeData
//-----------------------------------------------------------------------------

NodeData::NodeData(rtl::Reference<NodeImpl> const& aSpecificNode, rtl::OUString const& aName, unsigned int nParent)
: m_pSpecificNode(aSpecificNode)
, m_aName_(aName)
, m_nParent(nParent)
{
}

//-----------------------------------------------------------------------------

void NodeData::rebuild(rtl::Reference<view::ViewStrategy> const & _xNewStrategy, sharable::Node * newData)
{
    rtl::Reference<NodeImpl> aNewImpl;
    if (this->isSetNode())
    {
        sharable::SetNode * newSet = newData == 0 ? 0 : newData->setData();
        aNewImpl = _xNewStrategy->getNodeFactory().makeSetNode(newSet, 0);

        SetNodeImpl & rOldSetData = this->setImpl();
        SetNodeImpl & rNewSetData = static_cast<SetNodeImpl &>(*aNewImpl);

        rNewSetData.rebuildFrom(rOldSetData, newSet);
    }
    else if (this->isGroupNode())
        aNewImpl = _xNewStrategy->getNodeFactory().makeGroupNode(newData == 0 ? 0 : newData->groupData());

    else if (this->isValueElementNode())
        aNewImpl = _xNewStrategy->getNodeFactory().makeValueNode(newData == 0 ? 0 : newData->valueData());

    m_pSpecificNode = aNewImpl;
}

//-----------------------------------------------------------------------------

bool NodeData::isSetNode() const
{
    sharable::Node * node = getOriginalNodeAccess();
    return node != 0 && node->isSet();
}
//-----------------------------------------------------------------------------

bool NodeData::isValueElementNode() const
{
    sharable::Node * node = getOriginalNodeAccess();
    return node != 0 && node->isValue();
}
//-----------------------------------------------------------------------------

bool NodeData::isGroupNode() const
{
    sharable::Node * node = getOriginalNodeAccess();
    return node != 0 && node->isGroup();
}
//-----------------------------------------------------------------------------

SetNodeImpl&   NodeData::implGetSetImpl()   const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    OSL_ASSERT(isSetNode());

    if (!isSetNode())
            throw Exception( "INTERNAL ERROR: Node is not a set node. Cast failing." );

    return static_cast<SetNodeImpl&>(*m_pSpecificNode);
}
//---------------------------------------------------------------------

GroupNodeImpl& NodeData::implGetGroupImpl() const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    OSL_ASSERT(isGroupNode());

    if (!isGroupNode())
            throw Exception( "INTERNAL ERROR: Node is not a group node. Cast failing." );

    return static_cast<GroupNodeImpl&>(*m_pSpecificNode);
}
//---------------------------------------------------------------------

ValueElementNodeImpl& NodeData::implGetValueImpl() const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    OSL_ASSERT(isValueElementNode());

    if (!isValueElementNode())
            throw Exception( "INTERNAL ERROR: Node is not a value node. Cast failing." );

    return static_cast<ValueElementNodeImpl&>(*m_pSpecificNode);
}

sharable::Node * NodeData::getOriginalNodeAccess() const
{
    return m_pSpecificNode->getOriginalNodeAccess();
}

NodeImpl & NodeData::implGetNodeImpl() const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    return *m_pSpecificNode;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class Tree
//-----------------------------------------------------------------------------

/// creates a Tree for a detached, virgin instance of <var>aTemplate</var>
Tree::Tree( )
: m_aNodes()
, m_pParentTree(0)
, m_nParentNode(0)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

Tree::Tree( Tree& rParentTree, unsigned int nParentNode )
: m_aNodes()
, m_pParentTree(&rParentTree)
, m_nParentNode(nParentNode)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

Tree::~Tree()
{
}
//-----------------------------------------------------------------------------

void Tree::disposeData()
{
    m_aNodes.clear();
}

bool Tree::isRootNode(NodeRef const & node) const {
        OSL_ASSERT(
        nodeCount() != 0 && (!node.isValid() || isValidNode(node.getOffset())));
        return node.isValid() && node.getOffset() == ROOT;
}

NodeRef Tree::getRootNode() const {
    OSL_ASSERT(nodeCount() != 0);
    return NodeRef(ROOT, m_nDepth);
}

NodeRef Tree::getContextNodeRef() const {
    OSL_ASSERT(nodeCount() != 0 && (m_pParentTree == 0 || m_nParentNode != 0));
    return m_pParentTree == 0
        ? NodeRef() : m_pParentTree->getNode(m_nParentNode);
}

bool Tree::isValidValueNode(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0);
    if (!(value.isValid() &&
          value.checkValidState() &&
          isValidNode(value.m_nParentPos) &&
          view::ViewTreeAccess(this).isGroupNodeAt(value.m_nParentPos) &&
          getMemberNode(value).isValid()))
    {
        return false;
    }
    OSL_ASSERT(value.m_sNodeName.getLength() != 0); // old value handling?
    return true;
}

#if OSL_DEBUG_LEVEL > 0
bool Tree::isValidAnyNode(AnyNodeRef const & node) {
    OSL_ASSERT(nodeCount() != 0);
    return node.isValid() && isValidNode(node.m_nUsedPos) &&
        (node.isNode() ||
         (view::ViewTreeAccess(this).isGroupNodeAt(node.m_nUsedPos) &&
          getMemberNode(node.toValue()).isValid()));
}
#endif

bool Tree::hasElements(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth" );
    }
    view::ViewTreeAccess v(this);
    return v.isSetNode(node) && !v.isEmpty(v.toSetNode(node));
}

bool Tree::hasElement(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    return v.isSetNode(node) &&
        v.findElement(v.toSetNode(node), name).isValid();
}

bool Tree::hasElement(NodeRef const & node, Path::Component const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    if (v.isSetNode(node)) {
        SetEntry e(v.findElement(v.toSetNode(node), name.getName()));
        // Check if types match:
        return e.isValid() &&
            Path::matches(e.tree()->getExtendedRootName(), name);
    } else {
        return false;
    }
}

rtl::Reference< ElementTree > Tree::getElement(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    if (v.isSetNode(node)) {
        SetEntry e(v.findElement(v.toSetNode(node), name));
        return rtl::Reference< ElementTree >(e.tree());
    } else {
        return rtl::Reference< ElementTree >();
    }
}

rtl::Reference< ElementTree > Tree::getAvailableElement(
    NodeRef const & node, rtl::OUString const & name)
{
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    view::ViewTreeAccess v(this);
    if (v.isSetNode(node)) {
        SetEntry e(v.findAvailableElement(v.toSetNode(node), name));
        return rtl::Reference< ElementTree >(e.tree());
    } else {
        return rtl::Reference< ElementTree >();
    }
}

bool Tree::hasChildren(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    if (v.isGroupNode(node)) {
        view::GroupNode g(v.toGroupNode(node));
        return v.hasValue(g) || g.getFirstChild().is();
    } else {
        return false;
    }
}

bool Tree::hasChildValue(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    return v.isGroupNode(node) && v.hasValue(v.toGroupNode(node), name);
}

bool Tree::hasChildNode(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    return v.isGroupNode(node) && v.toGroupNode(node).findChild(name).is();
}

bool Tree::hasChild(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    if (v.isGroupNode(node)) {
        view::GroupNode g(v.toGroupNode(node));
        return v.hasValue(g, name) || g.findChild(name).is();
    } else {
        return false;
    }
}

ValueRef Tree::getChildValue(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    return v.isGroupNode(node) && v.hasValue(v.toGroupNode(node), name)
        ? ValueRef(name, node.getOffset()) : ValueRef();
}

NodeRef Tree::getChildNode(NodeRef const & node, rtl::OUString const & name)
{
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    return NodeRef(
        (v.isGroupNode(node)
         ? v.toGroupNode(node).findChild(name).get_offset() : 0),
        childDepth(node.getDepth()));
}

AnyNodeRef Tree::getAnyChild(NodeRef const & node, rtl::OUString const & name) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    unsigned int n = 0;
    if (v.isGroupNode(node)) {
        if (v.hasValue(v.toGroupNode(node), name)) {
            return AnyNodeRef(name, node.getOffset());
        }
        n = v.toGroupNode(node).findChild(name).get_offset();
    }
    return AnyNodeRef(n, childDepth(node.getDepth()));
}

node::Attributes Tree::getAttributes(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    return view::ViewTreeAccess(this).getAttributes(node);
}

node::Attributes Tree::getAttributes(AnyNodeRef const & node) {
    OSL_ASSERT(nodeCount() != 0 && isValidAnyNode(node));
    if (node.isNode()) {
        return view::ViewTreeAccess(this).getAttributes(node.toNode());
    } else {
        return getMemberNode(node.toValue()).getAttributes();
    }
}

node::Attributes Tree::getAttributes(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0 && isValidValueNode(value));
    return getMemberNode(value).getAttributes();
}

com::sun::star::uno::Type Tree::getUnoType(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0 && isValidValueNode(value));
    return getMemberNode(value).getValueType();
}

NodeRef Tree::getParent(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    view::ViewTreeAccess v(this);
    OSL_ASSERT(!v.makeNode(getRootNode()).getParent().is());
    view::Node p(v.makeNode(node).getParent());
    OSL_ASSERT(isValidNode(p.get_offset()));
    return NodeRef(p.get_offset(), parentDepth(node.getDepth()));
}

NodeRef Tree::getParent(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0 && isValidValueNode(value));
    unsigned int n = value.m_nParentPos;
    OSL_ASSERT(n == 0 || value.m_sNodeName.getLength() != 0);
    return getNode(n);
}

AbsolutePath Tree::getAbsolutePath(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    Path::Rep r;
    prependLocalPathTo(node.getOffset(), r);
    r.prepend(getRootPath().rep());
    return AbsolutePath(r);
}

com::sun::star::uno::Any Tree::getNodeValue(ValueRef const & value) {
    OSL_ASSERT(isValidValueNode(value));
    return getMemberNode(value).getValue();
}

bool Tree::hasNodeDefault(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0 && isValidValueNode(value));
    return getMemberNode(value).canGetDefaultValue();
}

bool Tree::isNodeDefault(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0 && isValidValueNode(value));
    return hasNodeDefault(value) && getMemberNode(value).isDefault();
}

bool Tree::hasNodeDefault(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    // Not a set, then it has no default:
    return view::ViewTreeAccess(this).isSetNode(node);
}

bool Tree::isNodeDefault(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (!hasNodeDefault(node)) {
        return false;
    }
    view::ViewTreeAccess v(this);
    OSL_ASSERT(v.isSetNode(node)); // not a set, then it has no default
    // A set is default if all its elements are default:
    return !FindNonDefaultElement::hasNonDefaultElement(v, v.toSetNode(node));
}

bool Tree::hasNodeDefault(AnyNodeRef const & node) {
    OSL_ASSERT(nodeCount() != 0 && isValidAnyNode(node));
    return node.isNode()
        ? hasNodeDefault(node.toNode()) : hasNodeDefault(node.toValue());
}

bool Tree::isNodeDefault(AnyNodeRef const & node) {
    OSL_ASSERT(nodeCount() != 0 && isValidAnyNode(node));
    return node.isNode()
        ? isNodeDefault(node.toNode()) : isNodeDefault(node.toValue());
}

bool Tree::areValueDefaultsAvailable(NodeRef const & node) {
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    view::ViewTreeAccess v(this);
    OSL_ASSERT(v.isGroupNode(node));
    return v.isGroupNode(node) &&
        v.areValueDefaultsAvailable(v.toGroupNode(node));
}

com::sun::star::uno::Any Tree::getNodeDefaultValue(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0 && isValidValueNode(value));
    ValueMemberNode m(getMemberNode(value));
    return m.canGetDefaultValue() ?
        m.getDefaultValue() : com::sun::star::uno::Any();
}

bool Tree::hasChanges() {
    return view::ViewTreeAccess(this).hasChanges();
}

bool Tree::collectChanges(NodeChanges & changes) {
    OSL_ASSERT(nodeCount() != 0);
    view::ViewTreeAccess v(this);
    if (v.hasChanges()) {
        v.collectChanges(changes);
        return true;
    } else {
        return false;
    }
}

void Tree::integrate(NodeChange & change, NodeRef const & node, bool local)
{
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (change.test().isChange()) {
        change.apply();
        if (local) {
            view::ViewTreeAccess(this).markChanged(node);
        } else {
            rtl::Reference< Tree > at(change.getAffectedTree());
            NodeRef an(change.getAffectedNode());
            OSL_ASSERT(
                !isEmpty(at.get()) && an.isValid() &&
                at->isValidNode(an.getOffset()));
            view::ViewTreeAccess(at.get()).markChanged(an);
            OSL_ASSERT(view::ViewTreeAccess(this).hasChanges());
        }
    }
}

void Tree::integrate(NodeChanges& changes, NodeRef const & node, bool local)
{
    for (std::vector< NodeChange >::iterator it(changes.begin());
         it != changes.end(); ++it)
    {
        integrate(*it, node, local);
    }
}

NodeVisitor::Result Tree::dispatchToChildren(
    NodeRef const & node, NodeVisitor & visitor)
{
    OSL_ASSERT(
        nodeCount() != 0 && node.isValid() && isValidNode(node.getOffset()));
    if (node.getDepth() == 0) {
        CFG_TRACE_WARNING(
            "configuration: Querying node beyond available depth");
    }
    view::ViewTreeAccess v(this);
    if (v.isGroupNode(node)) {
        view::GroupNode const p(v.toGroupNode(node));
        OSL_ASSERT(
            (NodeVisitor::Result(GroupMemberVisitor::DONE) ==
             NodeVisitor::DONE) &&
            (NodeVisitor::Result(GroupMemberVisitor::CONTINUE) ==
             NodeVisitor::CONTINUE));
        GroupVisitorAdapter adapter(this, node, visitor);
        NodeVisitor::Result ret = NodeVisitor::Result(
            v.dispatchToValues(p, adapter));
        unsigned int n = childDepth(node.getDepth());
        for (view::Node c(p.getFirstChild());
             c.is() && ret != NodeVisitor::DONE; c = p.getNextChild(c))
        {
            ret = visit(NodeRef(c.get_offset(), n), visitor);
        }
        return ret;
    } else if (v.isSetNode(node)) {
        OSL_ASSERT(
            NodeVisitor::Result(SetNodeVisitor::DONE) == NodeVisitor::DONE &&
            (NodeVisitor::Result(SetNodeVisitor::CONTINUE) ==
             NodeVisitor::CONTINUE));
        SetVisitorAdapter adapter(visitor);
        return NodeVisitor::Result(
            v.dispatchToElements(v.toSetNode(node), adapter));
    } else {
        OSL_TRACE("configuration: Trying to iterate a value node");
        return NodeVisitor::CONTINUE;
    }
}

NodeRef Tree::getNode(unsigned int offset) const {
    if (offset == 0) {
        return NodeRef();
    } else {
        OSL_ASSERT(isValidNode(offset));
        return NodeRef(
            offset, remainingDepth(getAvailableDepth(), depthTo(offset)));
    }
}

rtl::Reference< Template > Tree::extractElementInfo(NodeRef const & node) {
    OSL_ASSERT(node.isValid() && isValidNode(node.getOffset()));
    view::ViewTreeAccess v(this);
    OSL_ASSERT(v.isSetNode(node));
    return v.getElementTemplate(v.toSetNode(node));
}

//-----------------------------------------------------------------------------
void Tree::rebuild(rtl::Reference<view::ViewStrategy> const & _xNewStrategy, sharable::Node * newData)
{
    m_xStrategy = _xNewStrategy;
    this->implRebuild(ROOT, newData);
}

//-----------------------------------------------------------------------------
void Tree::implRebuild(unsigned int nNode, sharable::Node * newData)
{
    NodeData * pNode = nodeData(nNode);
    if (pNode->isGroupNode())
    {
        // first rebuild the children
        OSL_ASSERT(newData != 0 && newData->isGroup());
        sharable::GroupNode * newGroup = &newData->group;

        for (unsigned int nChild = firstChild_(nNode); isValidNode(nChild); nChild = findNextChild_(nNode,nChild))
        {
            sharable::Node * childAccess = newGroup->getChild(implGetOriginalName(nChild));
            OSL_ASSERT(childAccess != 0);
            implRebuild(nChild, childAccess);
        }
    }

    pNode->rebuild(m_xStrategy, newData);
}

//-----------------------------------------------------------------------------

void ElementTree::doFinishRootPath(Path::Rep& rPath) const
{
    rPath.prepend( doGetRootName() );
    rPath.prepend( AbsolutePath::detachedRoot().rep() );
}
//-----------------------------------------------------------------------------

void RootTree::doFinishRootPath(Path::Rep& rPath) const
{
    rPath.prepend( m_aRootPath.rep() );
}

//-----------------------------------------------------------------------------

void Tree::implPrependRootPath(Path::Rep& rPath) const
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

AbsolutePath Tree::getRootPath() const
{
    Path::Rep aPath;
    implPrependRootPath(aPath);
    return AbsolutePath(aPath);
}
//-----------------------------------------------------------------------------
void Tree::build(rtl::Reference<view::ViewStrategy> const& _xStrategy, sharable::Node * rootNode, unsigned int nDepth, TemplateProvider const& aTemplateProvider)
{
    OSL_ASSERT(m_aNodes.empty());
    m_nDepth = nDepth;
    TreeImplBuilder a(aTemplateProvider, _xStrategy,*this);
    a.visitNode(rootNode);
}
//-----------------------------------------------------------------------------

rtl::Reference< view::ViewStrategy > Tree::getViewBehavior() const
{
    return m_xStrategy;
}

// context handling
//-----------------------------------------------------------------------------

void Tree::setContext(Tree* pParentTree, unsigned int nParentNode)
{
    OSL_ENSURE(pParentTree,"ERROR: Moving tree to nowhere");

    if (pParentTree)
    {
        OSL_ENSURE( pParentTree->isValidNode(nParentNode),"ERROR: Moving tree to invalid node");
        if (!pParentTree->isValidNode(nParentNode))
            throw Exception("INTERNAL ERROR: Moving tree to invalid parent node");

    //  OSL_ENSURE( pParentTree->isSetNodeAt(nParentNode),"WARNING: Moving tree to node that is not a set");
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

void Tree::clearContext()
{
    m_pParentTree = 0;
    m_nParentNode = 0;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Node Collection navigation
//-----------------------------------------------------------------------------

unsigned int Tree::parent_(unsigned int nNode) const
{
    OSL_ASSERT(isValidNode(nNode));
    return nodeData(nNode)->getParent();
}
//-----------------------------------------------------------------------------
inline // is protected and should be used only here
rtl::OUString Tree::implGetOriginalName(unsigned int nNode) const
{
    OSL_ASSERT(isValidNode(nNode));

    return nodeData(nNode)->getName();
}
//-----------------------------------------------------------------------------

Path::Component ElementTree::doGetRootName() const
{
    return makeExtendedName( m_aElementName );
}

//-----------------------------------------------------------------------------

rtl::OUString ElementTree::getSimpleRootName() const
{
    // Tree::getSimpleRootName tends to parse &
    // then split the same name, burning CPU L&R
    return m_aElementName;
}

//-----------------------------------------------------------------------------

Path::Component RootTree::doGetRootName() const
{
    return m_aRootPath.getLocalName();
}
//-----------------------------------------------------------------------------


rtl::OUString Tree::getSimpleNodeName(unsigned int nNode) const
{
    if (nNode == ROOT) return getSimpleRootName();

    return implGetOriginalName(nNode);
}
//-----------------------------------------------------------------------------

rtl::OUString Tree::getSimpleRootName() const
{
    return doGetRootName().getName();
}

//-----------------------------------------------------------------------------

Path::Component Tree::getExtendedRootName() const
{
    return doGetRootName();
}
//-----------------------------------------------------------------------------

unsigned int Tree::depthTo(unsigned int nNode) const
{
    OSL_ASSERT(isValidNode(nNode));

    unsigned int nDepth = 0;
    while( 0 != (nNode=parent_(nNode)) )
    {
        ++nDepth;
    }

    return nDepth;
}
//-----------------------------------------------------------------------------

void Tree::prependLocalPathTo(unsigned int nNode, Path::Rep& rNames)
{
    OSL_ASSERT(isValidNode(nNode));

    for (; nNode != ROOT; nNode = parent_(nNode) )
    {
        OSL_ENSURE( isValidNode(nNode), "ERROR: Configuration: node has invalid parent");
        rNames.prepend( Path::wrapSimpleName( implGetOriginalName(nNode) ) );
    }

    OSL_ASSERT(nNode == ROOT);
}
//-----------------------------------------------------------------------------

// Node iteration and access
unsigned int Tree::firstChild_ (unsigned int nParent) const
{
    return findNextChild_(nParent,nParent);
}
//-----------------------------------------------------------------------------

unsigned int Tree::findNextChild_(unsigned int nParent, unsigned int nStartAfter) const
{
    OSL_ASSERT(isValidNode(nParent));
    OSL_ASSERT(nStartAfter == 0 || isValidNode(nStartAfter));

    unsigned int nPos = nStartAfter ? nStartAfter : ROOT - 1;
    unsigned int const nAfterLast = nodeCount() + ROOT;
    while (++nPos < nAfterLast)
    {
        if(parent_(nPos) == nParent) return nPos;
    }
    return 0;
}
//-----------------------------------------------------------------------------

unsigned int Tree::findChild_(unsigned int nParent, rtl::OUString const& aName) const
{
    OSL_ASSERT(isValidNode(nParent));

    unsigned int nPos = nParent;
    unsigned int const nAfterLast = nodeCount() + ROOT;
    while (++nPos < nAfterLast)
    {
        if(parent_(nPos) == nParent && implGetOriginalName(nPos) == aName)
            return nPos;
    }
    return 0;
}

ValueMemberNode Tree::getMemberNode(ValueRef const & value) {
    OSL_ASSERT(nodeCount() != 0);
    view::ViewTreeAccess v(this);
    return v.getValue(v.getGroupNodeAt(value.m_nParentPos), value.m_sNodeName);
}

//-----------------------------------------------------------------------------
// class RootTree
//-----------------------------------------------------------------------------

RootTree::RootTree( rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            AbsolutePath const& aRootPath,
                            sharable::Node * cacheNode, unsigned int nDepth,
                            TemplateProvider const& aTemplateProvider)
: Tree()
, m_aRootPath(aRootPath)
{
//    OSL_ENSURE( aRootPath.getLocalName().getName() == cacheNode->getName(),
//                "Constructing root node: Path does not match node name");

    Tree::build(_xStrategy,cacheNode,nDepth,aTemplateProvider);
}

bool isEmpty(Tree * tree)
{
    return tree == 0 || tree->nodeCount() == 0;
}

//-----------------------------------------------------------------------------
// class ElementTree
//-----------------------------------------------------------------------------

ElementTree::ElementTree(   rtl::Reference<view::ViewStrategy> const& _xStrategy,
                                    sharable::TreeFragment * cacheTree, unsigned int nDepth,
                                    rtl::Reference<Template> aTemplateInfo,
                                    TemplateProvider const& aTemplateProvider )
: Tree()
, m_aInstanceInfo(aTemplateInfo)
, m_aElementName(cacheTree->getName())
, m_aDataAddress(cacheTree)
, m_aOwnData()
{
    Tree::build( _xStrategy, cacheTree->getRootNode(), nDepth, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTree::ElementTree(   rtl::Reference<view::ViewStrategy> const& _xStrategy,
                                    Tree& rParentTree, unsigned int nParentNode,
                                    sharable::TreeFragment * cacheTree, unsigned int nDepth,
                                    rtl::Reference<Template> aTemplateInfo,
                                    TemplateProvider const& aTemplateProvider )
: Tree( rParentTree, nParentNode )
, m_aInstanceInfo(aTemplateInfo)
, m_aElementName(cacheTree->getName())
, m_aDataAddress(cacheTree)
, m_aOwnData()
{
    Tree::build( _xStrategy, cacheTree->getRootNode(), nDepth, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTree::ElementTree( rtl::Reference< data::TreeSegment > const& pNewTree,
                                    rtl::Reference<Template> aTemplate,
                                    TemplateProvider const& aTemplateProvider )
: Tree()
, m_aInstanceInfo(aTemplate)
, m_aOwnData(pNewTree)
{
    if (!pNewTree.is())
    throw Exception("ERROR: Provider can't create Element Instance From Template");
    m_aElementName = pNewTree->fragment->getName();
    m_aDataAddress = pNewTree->fragment;

    Tree::build( view::createDirectAccessStrategy(m_aOwnData), m_aOwnData->fragment->nodes, c_TreeDepthAll, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTree::~ElementTree()
{
}
//-----------------------------------------------------------------------------

bool ElementTree::isUpdatableSegment(Tree& _rTree)
{
    Tree * pTree = &_rTree;

    while (ElementTree * pElement = dynamic_cast< ElementTree * >(pTree))
    {
        if (pElement->m_aOwnData.is())
        {
            OSL_ENSURE( pElement->getContextTree()==NULL ||
                        pElement->getContextTree()->getViewBehavior() != pElement->getViewBehavior(),
                        "ElementTree with parent in same fragment should not own its data");
            return true;
        }

        pTree = pElement->getContextTree();

        if (!pTree)
        {
            OSL_ENSURE( false, "ElementTree without own data should have a parent");
            return false;
        }

    }
    OSL_ENSURE( false, "Tree is not part of free-floating segment - cannot support direct update");

    return false;
}
//-----------------------------------------------------------------------------

void ElementTree::disposeData()
{
    Tree::disposeData();
    m_aOwnData.clear();
}
//-----------------------------------------------------------------------------

Path::Component ElementTree::makeExtendedName(rtl::OUString const& _aSimpleName) const
{
    rtl::OUString aTypeName = implGetOriginalName(ROOT);

    OSL_ENSURE(this->isTemplateInstance(), "ElementTree: Cannot discover the type this instantiatiates");

    OSL_ENSURE(! this->isTemplateInstance() || this->getTemplate()->getName() == aTypeName,
                "ElementTree: Type name does not match template");

    return Path::makeCompositeName(_aSimpleName, aTypeName);
}
//-----------------------------------------------------------------------------

// ownership handling
//-----------------------------------------------------------------------------
void ElementTree::rebuild(rtl::Reference<view::ViewStrategy> const & _aStrategy, sharable::TreeFragment * newTree)
{
    Tree::rebuild(_aStrategy, newTree->getRootNode());
    m_aDataAddress = newTree;
    m_aElementName = newTree->getName();
}

//-----------------------------------------------------------------------------
/// transfer ownership to the given set
// -----------------------------------------------------------------------------
void ElementTree::attachTo(sharable::SetNode * owningSet, rtl::OUString const& aElementName)
{
    OSL_ENSURE(m_aOwnData.is(), "ERROR: Cannot add a non-owned node to a subtree");

    if (m_aOwnData.is())
    {
        OSL_ENSURE(this->getSimpleRootName() == aElementName,"ElementTree: Attaching with unexpected element name");
        m_aOwnData->fragment->setName(aElementName);

        Tree* pOwningTree = this->getContextTree();
        OSL_ENSURE(pOwningTree, "Element Tree Context must be set before attaching data");

        if (isUpdatableSegment(*pOwningTree))
        {
            // copy over to the new segment
            sharable::TreeFragment * aNewElement = data::buildTree(m_aOwnData->fragment);

            owningSet->addElement(aNewElement);

            rtl::Reference<view::ViewStrategy> xNewBehavior = pOwningTree->getViewBehavior();

            this->rebuild(xNewBehavior,aNewElement);
        }
        else
            OSL_ENSURE( false, "Cannot attach directly to new tree - no update access available");

        m_aOwnData.clear();
        OSL_ASSERT(!m_aOwnData.is());
    }
}
//-----------------------------------------------------------------------------

/// tranfer ownership from the given set
void ElementTree::detachFrom(sharable::SetNode * owningSet, rtl::OUString const& aElementName)
{
    OSL_ENSURE(!m_aOwnData.is(),"ERROR: Cannot detach a already owned node from a subtree");
    OSL_ENSURE(this->getSimpleRootName() == aElementName,"ElementTree: Detaching with unexpected element name");

    rtl::Reference< view::ViewStrategy > xOldStrategy = this->getViewBehavior();
    OSL_ENSURE(xOldStrategy.is(), "Element Tree Context must still have the old strategy when detaching data");

    {
        // make a new segment with a copy of the data
        rtl::Reference< data::TreeSegment > aNewSegment = data::TreeSegment::create( this->getOriginalTreeAccess() );

        OSL_ENSURE(aNewSegment.is(),"ERROR: Could not create detached copy of elment data");

        this->takeTreeAndRebuild( aNewSegment );

        sharable::TreeFragment * aOldElement = owningSet->removeElement(aElementName );
        OSL_ENSURE(aOldElement != NULL,"ERROR: Detached node not found in the given subtree");

        data::destroyTree(aOldElement);
    }

    OSL_ENSURE(m_aOwnData.is(),"ERROR: Could not create own data segment for detached node");
}
//-----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/// transfer ownership from the given owner
void ElementTree::takeTreeBack(rtl::Reference< data::TreeSegment > const & _aDataSegment)
{
    OSL_ENSURE(!m_aOwnData.is(), "ERROR: Cannot take over a node - already owning");
    OSL_ENSURE(_aDataSegment.is(), "ERROR: Cannot take over NULL tree segment");

    m_aOwnData = _aDataSegment;
    OSL_ENSURE(m_aOwnData.is(), "ERROR: Could not take over data segment");

    m_aDataAddress = m_aOwnData->fragment;
    m_aElementName = m_aOwnData->fragment->getName();
}
//-----------------------------------------------------------------------------

/// transfer ownership from the given owner
void ElementTree::takeTreeAndRebuild(rtl::Reference< data::TreeSegment > const & _aDataSegment)
{
    OSL_ENSURE(!m_aOwnData.is(), "ERROR: Cannot take over a node - already owning");
    OSL_ENSURE(_aDataSegment.is(), "ERROR: Cannot take over NULL tree segment");
    this->rebuild(view::createDirectAccessStrategy(_aDataSegment), _aDataSegment->fragment);
    m_aOwnData = _aDataSegment;
}
//-----------------------------------------------------------------------------

/// release ownership
rtl::Reference< data::TreeSegment > ElementTree::releaseOwnedTree()
{
    OSL_ENSURE(m_aOwnData.is(), "ERROR: Cannot release and rename a non-owned node");
    rtl::Reference< data::TreeSegment > aTree(m_aOwnData);
    m_aOwnData.clear();
    aTree->fragment->setName(m_aElementName);
    return aTree;
}
//-----------------------------------------------------------------------------

// context handling
//-----------------------------------------------------------------------------

/// renames the tree's root without concern for context consistency !
void ElementTree::renameTree(rtl::OUString const& aNewName)
{
    m_aElementName = aNewName;
}
//-----------------------------------------------------------------------------

void ElementTree::moveTree(Tree* pParentTree, unsigned int nParentNode)
{
    Tree::setContext(pParentTree,nParentNode);
}
//-----------------------------------------------------------------------------

void ElementTree::detachTree()
{
    Tree::clearContext();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }   // namespace configuration
}       // namespace configmgr

