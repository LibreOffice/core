/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: noderef.cxx,v $
 * $Revision: 1.34 $
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

#include "anynoderef.hxx"
#include "valueref.hxx"
#include "noderef.hxx"
#include "tree.hxx"
#include "viewaccess.hxx"
#include "configpath.hxx"
#include "nodechange.hxx"
#include "configexcept.hxx"
#include "configset.hxx"
#include "tracer.hxx"

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
// local  helpers
//-----------------------------------------------------------------------------

namespace
{
//-----------------------------------------------------------------------------
    struct CollectValueIDs : GroupMemberVisitor
    {
        CollectValueIDs(NodeID const& aParentID, std::vector<SubNodeID>& rValueList)
        : m_aParentID(aParentID)
        , m_rValueList(rValueList)
        {
        }

        Result visit(ValueMemberNode const& anEntry);

        NodeID  m_aParentID;
        std::vector<SubNodeID>& m_rValueList;
    };

    GroupMemberVisitor::Result CollectValueIDs::visit(ValueMemberNode const& aValue)
    {
        OSL_ASSERT(aValue.isValid());

        rtl::OUString aValueName = aValue.getNodeName();

        m_rValueList.push_back(SubNodeID( m_aParentID, aValueName));

        return CONTINUE;
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

NodeRef::NodeRef(unsigned int nPos, unsigned int nDepth)
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
// class ValueRef
//-----------------------------------------------------------------------------

bool ValueRef::checkValidState() const
{
    if (m_nParentPos  == 0)    return false;

    // old node semantics for now
    if ( m_sNodeName.getLength() == 0 ) return false;

    return true;
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef()
: m_sNodeName()
, m_nParentPos(0)
{
}
//-----------------------------------------------------------------------------

ValueRef::ValueRef(rtl::OUString const& aName, unsigned int nParentPos)
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
// class AnyNodeRef
//-----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
bool AnyNodeRef::checkValidState() const
{
    if (m_nUsedPos  == 0)    return false;

    if ( m_sNodeName.getLength() != 0 ) // it's a local value
    {
        // not used as runtime check as it should not be dangerous
        OSL_ENSURE(m_nDepth ==0, "AnyNodeRef that wraps a ValueRef should have no depth"); // value has no depth
    }

    return true;
}
#endif
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef()
: m_sNodeName()
, m_nUsedPos(0)
, m_nDepth(0)
{
}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(unsigned int nPos, unsigned int nDepth)
: m_sNodeName()
, m_nUsedPos(nPos)
, m_nDepth(nDepth)
{}
//-----------------------------------------------------------------------------

AnyNodeRef::AnyNodeRef(rtl::OUString const& aName, unsigned int nParentPos)
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
: m_sNodeName( aValueRef.m_sNodeName )
, m_nUsedPos(  aValueRef.m_nParentPos )
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

    return m_sNodeName.getLength() == 0;
}
//-----------------------------------------------------------------------------

NodeRef AnyNodeRef::toNode() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: AnyNodeRef operation requires valid node" );
    if (!isValid() || !isNode()) return NodeRef();

    return NodeRef(m_nUsedPos,m_nDepth);
}
//-----------------------------------------------------------------------------

ValueRef AnyNodeRef::toValue() const
{
    OSL_PRECOND( isValid(), "ERROR: Configuration: AnyNodeRef operation requires valid node" );
    if (!isValid() || isNode()) return ValueRef();

    return ValueRef(m_sNodeName, m_nUsedPos);
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

NodeID::NodeID(rtl::Reference< Tree > const& rTree, NodeRef const& rNode)
: m_pTree( rTree.get() )
, m_nNode( rNode.getOffset() )
{
}
//-----------------------------------------------------------------------------

NodeID::NodeID(Tree* pImpl, unsigned int nNode)
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

unsigned int NodeID::toIndex() const
{
    unsigned int n = m_nNode;
    if (m_pTree)
    {
        OSL_ENSURE(m_pTree->isValidNode(n),"Cannot produce valid Index for NodeID");

        n -= Tree::ROOT;
    }
    return n;
}

NodeRef NodeID::getNode() const {
    return m_pTree == 0 ? NodeRef() : m_pTree->getNode(m_nNode);
}

//-----------------------------------------------------------------------------
bool operator < (NodeID const& lhs, NodeID const& rhs)
{
    if (lhs.m_pTree == rhs.m_pTree)
        return lhs.m_nNode < rhs.m_nNode;
    else
        return std::less<Tree*>()(lhs.m_pTree,rhs.m_pTree);
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

SubNodeID::SubNodeID(rtl::Reference< Tree > const& rTree, NodeRef const& rParentNode, rtl::OUString const& aName)
: m_sNodeName(aName)
, m_aParentID(rTree,rParentNode)
{
}
//-----------------------------------------------------------------------------

SubNodeID::SubNodeID(NodeID const& rParentNodeID, rtl::OUString const& aName)
: m_sNodeName(aName)
, m_aParentID(rParentNodeID)
{
}
//-----------------------------------------------------------------------------

bool SubNodeID::isValidNode() const
{
    if (!m_aParentID.isValidNode()) return false;

    OSL_ENSURE(m_sNodeName.getLength() != 0,"Invalid subnode ID: Missing name");

    rtl::Reference< Tree > aCheck( m_aParentID.getTree() );
    return aCheck->hasChild( m_aParentID.getNode(), m_sNodeName );
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

rtl::OUString validateElementName(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode )
{
    { (void)aTree; (void)aNode; }
    OSL_PRECOND( !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  view::ViewTreeAccess(aTree.get()).isSetNode(aNode), "ERROR: Configuration: Set node expected.");

    return validateElementName(sName);
}
//-----------------------------------------------------------------------------

rtl::OUString validateChildName(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode )
{
    { (void)aTree; (void)aNode; }
    OSL_PRECOND( !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  view::ViewTreeAccess(aTree.get()).isGroupNode(aNode), "ERROR: Configuration: Group node expected.");

    return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

rtl::OUString validateChildOrElementName(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode )
{
    OSL_PRECOND( !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  isStructuralNode(aTree,aNode), "ERROR: Configuration: Inner node expected.");

    if (view::ViewTreeAccess(aTree.get()).isSetNode(aNode))
        return validateElementName(sName);

    else
        return validateNodeName(sName);
}
//-----------------------------------------------------------------------------

Path::Component validateElementPathComponent(rtl::OUString const& sName, rtl::Reference< Tree > const& aTree, NodeRef const& aNode )
{
    rtl::OUString aElementName = validateElementName(sName,aTree,aNode);

    rtl::Reference<Template> aTemplate = aTree->extractElementInfo(aNode);
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

static void implValidateLocalPath(RelativePath& _rPath, rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    if (_rPath.isEmpty())
        throw InvalidName(_rPath.toString(), "is an empty path.");

    // FOR NOW: validate only the first component
    if (!view::ViewTreeAccess(aTree.get()).isSetNode(aNode))
        if (!_rPath.getFirstName().isSimpleName())
            throw InvalidName(_rPath.toString(), "is not valid in this context. Predicate expression used to select group member.");
}
//-----------------------------------------------------------------------------

RelativePath validateRelativePath(rtl::OUString const& _sPath, rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");

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

RelativePath validateAndReducePath(rtl::OUString const& _sPath, rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aNode.isValid(), "ERROR: Configuration: Node operation requires a valid NodeRef");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match Tree");

    OSL_PRECOND(  isStructuralNode(aTree,aNode), "ERROR: Configuration: Inner node expected.");

    if ( !Path::isAbsolutePath(_sPath) )
        return validateRelativePath(_sPath,aTree,aNode);

    AbsolutePath aInputPath = AbsolutePath::parse(_sPath);

    RelativePath aStrippedPath = Path::stripPrefix( aInputPath, aTree->getAbsolutePath(aNode) );

    implValidateLocalPath(aStrippedPath,aTree,aNode);

    return aStrippedPath;
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, rtl::OUString const& aName)
{
    return view::ViewTreeAccess(aTree.get()).isSetNode(aNode) ? aTree->hasElement(aNode,aName) : aTree->hasChild(aNode,aName);
}
//-----------------------------------------------------------------------------

bool hasChildOrElement(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, Path::Component const& aName)
{
    return view::ViewTreeAccess(aTree.get()).isSetNode(aNode) ? aTree->hasElement(aNode,aName) : aTree->hasChild(aNode,aName.getName());
}
//-----------------------------------------------------------------------------

bool findInnerChildOrAvailableElement(rtl::Reference< Tree > & aTree, NodeRef& aNode, rtl::OUString const& aName)
{
    if ( view::ViewTreeAccess(aTree.get()).isSetNode(aNode) )
    {
        rtl::Reference< ElementTree > aElement = aTree->getAvailableElement(aNode,aName);
        if (aElement.is())
        {
            aTree = aElement.get();
            aNode = aTree->getRootNode();
            return true;
        }
    }
    else
    {
        NodeRef aChild = aTree->getChildNode(aNode,aName);

        if ( aChild.isValid() )
        {
            aNode = aChild;
            return true;
        }
    }

    return false;
}
//-----------------------------------------------------------------------------

AnyNodeRef getChildOrElement(rtl::Reference< Tree > & aTree, NodeRef const& aParentNode, rtl::OUString const& aName)
{
    if (aTree->hasChildValue(aParentNode,aName))
    {
        return AnyNodeRef(aTree->getChildValue(aParentNode,aName));
    }

    else if ( view::ViewTreeAccess(aTree.get()).isSetNode(aParentNode) )
    {
        rtl::Reference< ElementTree > aElement = aTree->getElement(aParentNode,aName);
        if (aElement.is())
        {
            aTree = aElement.get();
            return AnyNodeRef(aTree->getRootNode());
        }
    }

    else
    {
        NodeRef aChild = aTree->getChildNode(aParentNode,aName);

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
bool findLocalInnerChild(rtl::Reference< Tree > const& aTree, NodeRef& aNode, Path::Component const& aName)
{
    NodeRef aChild = aTree->getChildNode(aNode,aName.getName());

    if ( !aChild.isValid() ) return false;

    OSL_ENSURE( aName.isSimpleName(), "Child of group was found by request using element name format -failing");
    if ( !aName.isSimpleName()) return false;

    aNode = aChild;

    return true;
}
//-----------------------------------------------------------------------------

static
inline
bool findElement(rtl::Reference< Tree > & aTree, NodeRef& aNode, Path::Component const& aName)
{
    rtl::Reference< ElementTree > aElement = aTree->getElement(aNode,aName.getName());

    if (!aElement.is()) return false;

    rtl::Reference< Tree > aFoundTree = aElement.get();

    OSL_ENSURE(matches(aFoundTree->getExtendedRootName(),aName), "Element found, but type prefix does not match - failing");
    if ( !matches(aFoundTree->getExtendedRootName(),aName) ) return false;

    aTree = aFoundTree;
    aNode = aTree->getRootNode();

    return true;
}
//-----------------------------------------------------------------------------

static
bool findLocalInnerDescendant(rtl::Reference< Tree > const& aTree, NodeRef& aNode, RelativePath& rPath)
{
    while ( !rPath.isEmpty() )
    {
        if (  view::ViewTreeAccess(aTree.get()).isSetNode(aNode) ) return false;

        if ( ! findLocalInnerChild(aTree,aNode,rPath.getFirstName()) ) return false;

        rPath.dropFirstName();
    }

    return true;
}
//-----------------------------------------------------------------------------

static
bool findDeepInnerDescendant(rtl::Reference< Tree > & aTree, NodeRef& aNode, RelativePath& rPath)
{
    while ( !rPath.isEmpty() )
    {
        if ( view::ViewTreeAccess(aTree.get()).isSetNode(aNode) )
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
bool identifiesLocalValue(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, RelativePath const& aPath)
{
    if ( aPath.getDepth() == 1 )
    {
        Path::Component const & aLocalName = aPath.getLocalName();
        rtl::OUString aName = aLocalName.getName();

        if (aTree->hasChildValue(aNode,aName))
        {
            OSL_ENSURE( aLocalName.isSimpleName(), "Value in group was found by request using element name format");
            if ( aLocalName.isSimpleName())
                return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------

AnyNodeRef getLocalDescendant(rtl::Reference< Tree > const& aTree, NodeRef const& aNode, RelativePath const& rPath)
{
    NodeRef aNestedNode( aNode );
    RelativePath aRemainingPath(rPath);

    if ( findLocalInnerDescendant(aTree,aNestedNode,aRemainingPath) )
    {
        OSL_ASSERT(
            aNestedNode.isValid() &&
            aTree->isValidNode(aNestedNode.getOffset()));
        return AnyNodeRef(aNestedNode);
    }

    if ( identifiesLocalValue(aTree,aNestedNode,aRemainingPath) )
    {
        ValueRef aValue = aTree->getChildValue(aNestedNode,rPath.getLocalName().getName());
        OSL_ASSERT(aTree->isValidValueNode(aValue));
        return AnyNodeRef(aValue);
    }

    return AnyNodeRef();
}
//-----------------------------------------------------------------------------

AnyNodeRef getDeepDescendant(rtl::Reference< Tree > & aTree, NodeRef& aNode, RelativePath& rPath)
{
    if ( findDeepInnerDescendant(aTree,aNode,rPath) )
    {
        OSL_ASSERT(
            aNode.isValid() && aTree->isValidNode(aNode.getOffset()));
        return AnyNodeRef(aNode);
    }

    if ( identifiesLocalValue(aTree,aNode,rPath) )
    {
        ValueRef aValue = aTree->getChildValue(aNode,rPath.getLocalName().getName());
        OSL_ASSERT(aTree->isValidValueNode(aValue));
        return AnyNodeRef(aValue);
    }

    return AnyNodeRef();
}
//-----------------------------------------------------------------------------

void getAllContainedNodes(rtl::Reference< Tree > const& aTree, std::vector<NodeID>& aList)
{
    aList.clear();

    if (Tree* pImpl = aTree.get())
    {
        unsigned int nCount = pImpl->nodeCount();
        aList.reserve(nCount);

        unsigned int const nEnd = Tree::ROOT + nCount;

        for(unsigned int nOffset = Tree::ROOT;
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

void getAllChildrenHelper(NodeID const& aNode, std::vector<SubNodeID>& aList)
{
    aList.clear();

    if (Tree* pTreeImpl = aNode.getTree())
    {
        view::ViewTreeAccess aView(pTreeImpl);

        if (unsigned int const nParent = aNode.getOffset())
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
NodeID findNodeFromIndex(rtl::Reference< Tree > const& aTree, unsigned int nIndex)
{
    if (Tree* pImpl = aTree.get())
    {
        unsigned int nNode = nIndex + Tree::ROOT;
        if (pImpl->isValidNode(nNode))
        {
            return NodeID(pImpl,nNode);
        }
    }
    return NodeID(0,0);
}

//-----------------------------------------------------------------------------

static inline bool isRootNode(NodeRef const& aNode)
{
    return aNode.getOffset() == Tree::ROOT;
}
//-----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
bool isSimpleValueElement(rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree->isValidNode(aNode.getOffset()), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = view::ViewTreeAccess(aTree.get());

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aNode)) );

    return aNode.isValid() && isRootNode(aNode) && aView.isValueNode(aNode);
}
#endif
//-----------------------------------------------------------------------------

bool isStructuralNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree->isValidNode(aNode.getOffset()), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = view::ViewTreeAccess(aTree.get());

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aNode)) );

    return aNode.isValid() && ! aView.isValueNode(aNode);
}
//-----------------------------------------------------------------------------

bool isGroupNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree->isValidNode(aNode.getOffset()), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = view::ViewTreeAccess(aTree.get());

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aNode)) );

    return aNode.isValid() && aView.isGroupNode(aNode);
}
//-----------------------------------------------------------------------------

bool isSetNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !aNode.isValid() || !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( !aNode.isValid() || aTree->isValidNode(aNode.getOffset()), "WARNING: Configuration: NodeRef does not match Tree");

    view::ViewTreeAccess aView = view::ViewTreeAccess(aTree.get());

    OSL_ASSERT( !aNode.isValid() ||
                aView.isGroupNode(aNode) ||
                aView.isSetNode(aNode) ||
                (aView.isValueNode(aNode) && isRootNode(aNode)) );

    return aNode.isValid() && aView.isSetNode(aNode);
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any getSimpleElementValue(rtl::Reference< Tree > const& aTree, NodeRef const& aNode)
{
    OSL_PRECOND( !isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND( aNode.isValid(), "ERROR: Configuration: Node operation requires a valid Node");
    OSL_PRECOND( aTree->isValidNode(aNode.getOffset()), "WARNING: Configuration: NodeRef does not match Tree");

    if (!aNode.isValid()) return com::sun::star::uno::Any();

    OSL_PRECOND( isSimpleValueElement(aTree, aNode), "ERROR: Configuration: Getting value is supported only for value nodes");

    view::ViewTreeAccess aView = view::ViewTreeAccess(aTree.get());

    return aView.getValue(aView.toValueNode(aNode));
}

//-----------------------------------------------------------------------------
    }   // namespace configuration
}       // namespace configmgr
