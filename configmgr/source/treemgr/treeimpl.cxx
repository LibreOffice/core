/*************************************************************************
 *
 *  $RCSfile: treeimpl.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:42:19 $
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

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#include "template.hxx"
#endif

#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif
#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_VIEWACCESS_HXX_
#include "viewaccess.hxx"
#endif

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEFACTORY_HXX_
#include "nodefactory.hxx"
#endif
#ifndef CONFIGMGR_SEGMENT_HXX
#include "segment.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
        using view::NodeFactory;
//-----------------------------------------------------------------------------
// class TreeImplBuilder - friend of TreeImpl
//-----------------------------------------------------------------------------

/** is a visitor-style algorithm to construct a <type>TreeImpl::NodeList</type>
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
            TreeImpl& rTree
         )
        : m_aTemplateProvider(aTemplateProvider)
        , m_xStrategy(_xStrategy)
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
    /// implements the NodeVisitor handler for Value Nodes
    virtual Result handle(data::ValueNodeAccess const& _aValue);
    /// implements the NodeVisitor member for Group Nodes
    virtual Result handle(data::GroupNodeAccess const& _aGroup);
    /// implements the NodeVisitor member for Set Nodes
    virtual Result handle(data::SetNodeAccess const& _aSet);

    /// add a Node for group node <var>_aGroup</var> to the list
    void addGroup(data::GroupNodeAccess const& _aGroup);
    /// add a Node for set node <var>_aSet</var> to the list
    void addSet(data::SetNodeAccess const& _aSet);
    /// add a Node for value node <var>rValue</var> to the list
    void addValueElement(data::ValueNodeAccess const& _aValue);
    /// add a Member for value node <var>rValue</var> to the list
    void addValueMember(data::ValueNodeAccess const& _aValue);

    rtl::Reference<view::ViewStrategy>  m_xStrategy;
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
typedef TreeImplBuilder::Result VisitorStatus;
//-----------------------------------------------------------------------------

VisitorStatus TreeImplBuilder::handle(data::ValueNodeAccess const& _aValue)
{
    if (m_nParent == 0)
        addValueElement(_aValue); // if it is the root it is a value set element
    else
        addValueMember(_aValue); // if it is not the root it is a group member

    return CONTINUE;
}
//-----------------------------------------------------------------------------

VisitorStatus TreeImplBuilder::handle(data::GroupNodeAccess const& _aGroup)
{
    addGroup(_aGroup);
    return CONTINUE;
}
//-----------------------------------------------------------------------------

VisitorStatus TreeImplBuilder::handle(data::SetNodeAccess const& _aSet)
{
    addSet(_aSet);
    return CONTINUE;
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addValueElement(data::ValueNodeAccess const& _aValue)
{
    NodeImplHolder aValueNode( m_rFactory.makeValueNode(_aValue) );
    OSL_ENSURE( aValueNode.is(), "could not make value node wrapper" );

    OSL_ENSURE( m_nParent == 0, "Adding value element that is not root of its fragment" );
    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aValueNode.is() )
    {
        m_rTree.m_aNodes.push_back( NodeData(aValueNode,_aValue.getName(),m_nParent) );
    }
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addValueMember(data::ValueNodeAccess const& )
{
    // nothing to do
    OSL_DEBUG_ONLY(m_bMemberCheck = true);
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addGroup(data::GroupNodeAccess const& _aTree)
{
    NodeImplHolder aGroupNode( m_rFactory.makeGroupNode(_aTree) );
    OSL_ENSURE( aGroupNode.is(), "could not make group node wrapper" );

    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aGroupNode.is() )
    {
        m_rTree.m_aNodes.push_back( NodeData(aGroupNode,_aTree.getName(),m_nParent) );

        // now fill in group members
        if (m_nDepthLeft > 0)
        {
            NodeOffset nSaveParent = m_nParent;
            decDepth(m_nDepthLeft);

            m_nParent = m_rTree.m_aNodes.size() + m_rTree.root_() - 1;

        #ifdef _DEBUG
            bool bSaveMemberCheck = m_bMemberCheck;
            m_bMemberCheck = false;
        #endif

            // now recurse:
            this->visitChildren(_aTree);

            OSL_ENSURE(m_nParent < m_rTree.m_aNodes.size() || m_bMemberCheck,
                        "WARNING: Configuration: Group within requested depth has no members");

            OSL_DEBUG_ONLY(m_bMemberCheck = bSaveMemberCheck);

            incDepth(m_nDepthLeft);
            m_nParent = nSaveParent;
        }
    }
}
//-----------------------------------------------------------------------------

void TreeImplBuilder::addSet(data::SetNodeAccess const& _aSet)
{
    TemplateHolder aTemplate = makeSetElementTemplate(_aSet, m_aTemplateProvider);
    OSL_ASSERT(aTemplate.is());
    OSL_ENSURE(aTemplate->isInstanceTypeKnown(),"ERROR: Cannor create set instance without knowing the instance type");

    NodeImplHolder aSetNode( m_rFactory.makeSetNode(_aSet,aTemplate.get()) );
    OSL_ENSURE( aSetNode.is(), "could not make set node wrapper" );

    // TODO:!isValid() => maybe substitute a SimpleValueNodeImpl if possible
    if( aSetNode.is() )
    {
        m_rTree.m_aNodes.push_back( NodeData(aSetNode,_aSet.getName(),m_nParent) );

        // this also relies on one based offsets
        NodeOffset nNodeAdded = m_rTree.m_aNodes.size() + m_rTree.root_() - 1;

        OSL_ASSERT(&m_rTree.m_aNodes.back().nodeImpl() == aSetNode.get());
        static_cast<SetNodeImpl&>(*aSetNode).initElements(m_aTemplateProvider, m_rTree, nNodeAdded, m_nDepthLeft);
    }
}

//-----------------------------------------------------------------------------
// class NodeData
//-----------------------------------------------------------------------------

NodeData::NodeData(NodeImplHolder const& aSpecificNode, Name const& aName, NodeOffset nParent)
: m_pSpecificNode(aSpecificNode)
, m_nParent(nParent)
, m_aName_(aName)
{
}
//-----------------------------------------------------------------------------

data::NodeAccessRef NodeData::getOriginalNodeAccessRef(data::Accessor const * _pAccessor) const
{
    return data::NodeAccessRef(_pAccessor, m_pSpecificNode->getOriginalNodeAddress());
}
//-----------------------------------------------------------------------------

void NodeData::rebuild(rtl::Reference<view::ViewStrategy> const & _xNewStrategy, data::NodeAccessRef const & _aNewData, data::Accessor const& _aOldAccessor)
{
    using namespace data;

    NodeImplHolder aNewImpl;
    if (this->isSetNode(_aOldAccessor))
    {
        SetNodeAccess aNewSet(_aNewData);
        aNewImpl = _xNewStrategy->getNodeFactory().makeSetNode(aNewSet,NULL);

        SetNodeImpl & rOldSetData = this->setImpl(_aOldAccessor);
        SetNodeImpl & rNewSetData = static_cast<SetNodeImpl &>(*aNewImpl);

        SetNodeAccess aOldSet = rOldSetData.getDataAccess(_aOldAccessor);

        rNewSetData.rebuildFrom(rOldSetData,aNewSet,aOldSet);
    }
    else if (this->isGroupNode(_aOldAccessor))
        aNewImpl = _xNewStrategy->getNodeFactory().makeGroupNode(GroupNodeAccess(_aNewData));

    else if (this->isValueElementNode(_aOldAccessor))
        aNewImpl = _xNewStrategy->getNodeFactory().makeValueNode(ValueNodeAccess(_aNewData));

    m_pSpecificNode = aNewImpl;
}

//-----------------------------------------------------------------------------

bool NodeData::isSetNode(data::Accessor const& _aAccessor) const
{
    return data::SetNodeAccess::isInstance(getOriginalNodeAccessRef(&_aAccessor));
}
//-----------------------------------------------------------------------------

bool NodeData::isValueElementNode(data::Accessor const& _aAccessor) const
{
    return data::ValueNodeAccess::isInstance(getOriginalNodeAccessRef(&_aAccessor));
}
//-----------------------------------------------------------------------------

bool NodeData::isGroupNode(data::Accessor const& _aAccessor) const
{
    return data::GroupNodeAccess::isInstance(getOriginalNodeAccessRef(&_aAccessor));
}
//-----------------------------------------------------------------------------

SetNodeImpl&   NodeData::implGetSetImpl(data::Accessor const& _aAccessor)   const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    OSL_ASSERT(isSetNode(_aAccessor));

    if (!isSetNode(_aAccessor))
            throw Exception( "INTERNAL ERROR: Node is not a set node. Cast failing." );

    return static_cast<SetNodeImpl&>(*m_pSpecificNode);
}
//---------------------------------------------------------------------

GroupNodeImpl& NodeData::implGetGroupImpl(data::Accessor const& _aAccessor) const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    OSL_ASSERT(isGroupNode(_aAccessor));

    if (!isGroupNode(_aAccessor))
            throw Exception( "INTERNAL ERROR: Node is not a group node. Cast failing." );

    return static_cast<GroupNodeImpl&>(*m_pSpecificNode);
}
//---------------------------------------------------------------------

ValueElementNodeImpl& NodeData::implGetValueImpl(data::Accessor const& _aAccessor) const
{
    OSL_ASSERT(m_pSpecificNode != 0);
    OSL_ASSERT(isValueElementNode(_aAccessor));

    if (!isValueElementNode(_aAccessor))
            throw Exception( "INTERNAL ERROR: Node is not a value node. Cast failing." );

    return static_cast<ValueElementNodeImpl&>(*m_pSpecificNode);
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

data::Accessor TreeImpl::getDataAccessor(data::Accessor const& _aExternalAccessor) const
{
    if (memory::Segment const* pDataSegment = getDataSegment())
        return data::Accessor(pDataSegment);

    else
        return _aExternalAccessor;
}
//-----------------------------------------------------------------------------

data::TreeAccessor ElementTreeImpl::getOriginalTreeAccess(data::Accessor const& _aAccessor) const
{
    data::Accessor const& aRealAccessor = this->getDataAccessor(_aAccessor);
//    data::NodeAccess aRoot = nodeData(root_())->getOriginalNodeAccess(aRealAccessor);
    return data::TreeAccessor(aRealAccessor, m_aDataAddress/*aRoot.getDataPtr()*/);
}

//-----------------------------------------------------------------------------
void TreeImpl::rebuild(rtl::Reference<view::ViewStrategy> const & _xNewStrategy, data::NodeAccessRef const & _aNewData, data::Accessor const& _aOldAccessor)
{
    m_xStrategy = _xNewStrategy;
    this->implRebuild( this->root_(), _aNewData, _aOldAccessor);
}

//-----------------------------------------------------------------------------
void TreeImpl::implRebuild(NodeOffset nNode, data::NodeAccessRef const & _aNewData, data::Accessor const& _aOldAccessor)
{
    NodeData * pNode = nodeData(nNode);
    if (pNode->isGroupNode(_aOldAccessor))
    {
        // first rebuild the children
        data::GroupNodeAccess aNewGroupAccess(_aNewData);
        OSL_ASSERT(aNewGroupAccess.isValid());

        for (NodeOffset nChild = firstChild_(nNode); isValidNode(nChild); nChild = findNextChild_(nNode,nChild))
        {
            data::NodeAccessRef aChildAccess = aNewGroupAccess.getChildNode(implGetOriginalName(nChild));
            OSL_ASSERT(aChildAccess.isValid());
            implRebuild(nChild,aChildAccess,_aOldAccessor);
        }
    }

    pNode->rebuild(m_xStrategy,_aNewData,_aOldAccessor);
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
void TreeImpl::build(rtl::Reference<view::ViewStrategy> const& _xStrategy, data::NodeAccessRef const& _aRootNode, TreeDepth nDepth, TemplateProvider const& aTemplateProvider)
{
    OSL_ASSERT(m_aNodes.empty());
    m_nDepth = nDepth;
    TreeImplBuilder a(aTemplateProvider, _xStrategy,*this);
    a.visitNode(_aRootNode);
}
//-----------------------------------------------------------------------------

rtl::Reference< view::ViewStrategy > TreeImpl::getViewBehavior() const
{
    return m_xStrategy;
}

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

void TreeImpl::clearContext()
{
    m_pParentTree = 0;
    m_nParentNode = 0;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Node Collection navigation
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::parent_(NodeOffset nNode) const
{
    OSL_ASSERT(isValidNode(nNode));
    return nodeData(nNode)->getParent();
}
//-----------------------------------------------------------------------------
inline // is protected and should be used only here
Name TreeImpl::implGetOriginalName(NodeOffset nNode) const
{
    OSL_ASSERT(isValidNode(nNode));

    return nodeData(nNode)->getName();
}
//-----------------------------------------------------------------------------

Path::Component ElementTreeImpl::doGetRootName() const
{
    return makeExtendedName( m_aElementName );
}

//-----------------------------------------------------------------------------

Path::Component RootTreeImpl::doGetRootName() const
{
    return m_aRootPath.getLocalName();
}
//-----------------------------------------------------------------------------


Name TreeImpl::getSimpleNodeName(NodeOffset nNode) const
{
    if (nNode == root_()) return getSimpleRootName();

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
    while( 0 != (nNode=parent_(nNode)) )
    {
        ++nDepth;
    }

    return nDepth;
}
//-----------------------------------------------------------------------------

void TreeImpl::prependLocalPathTo(NodeOffset nNode, Path::Rep& rNames)
{
    OSL_ASSERT(isValidNode(nNode));

    for (; nNode != root_(); nNode = parent_(nNode) )
    {
        OSL_ENSURE( isValidNode(nNode), "ERROR: Configuration: node has invalid parent");
        rNames.prepend( Path::wrapSimpleName( implGetOriginalName(nNode) ) );
    }

    OSL_ASSERT(nNode == root_());
}
//-----------------------------------------------------------------------------

// Node iteration and access
NodeOffset TreeImpl::firstChild_ (NodeOffset nParent) const
{
    return findNextChild_(nParent,nParent);
}
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::nextSibling_(NodeOffset nNode) const
{
    return findNextChild_(parent_(nNode),nNode);
}
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::findNextChild_(NodeOffset nParent, NodeOffset nStartAfter) const
{
    OSL_ASSERT(isValidNode(nParent));
    OSL_ASSERT(nStartAfter == 0 || isValidNode(nStartAfter));

    NodeOffset nPos = nStartAfter ? nStartAfter : root_()-1;
    NodeOffset const nAfterLast = nodeCount() + root_();
    while (++nPos < nAfterLast)
    {
        if(parent_(nPos) == nParent) return nPos;
    }
    return 0;
}
//-----------------------------------------------------------------------------

NodeOffset TreeImpl::findChild_(NodeOffset nParent, Name const& aName) const
{
    OSL_ASSERT(isValidNode(nParent));

    NodeOffset nPos = nParent;
    NodeOffset const nAfterLast = nodeCount() + root_();
    while (++nPos < nAfterLast)
    {
        if(parent_(nPos) == nParent && implGetOriginalName(nPos) == aName)
            return nPos;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// locking
//-----------------------------------------------------------------------------
osl::Mutex& TreeImpl::getRootLock() const
{
    if ( m_pParentTree )
        return m_pParentTree->getRootLock();
    else
        return m_aOwnLock;
}
//-----------------------------------------------------------------------------
memory::Segment const * TreeImpl::getRootSegment() const
{
    if ( m_pParentTree )
        return m_pParentTree->getRootSegment();
    else
        return this->getDataSegment();
}
//-----------------------------------------------------------------------------
memory::Segment const * TreeImpl::getDataSegment() const
{
    return m_xStrategy->getDataSegment();
}
//-----------------------------------------------------------------------------

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

RootTreeImpl::RootTreeImpl( rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            AbsolutePath const& aRootPath,
                            data::NodeAccess const& _aCacheNode, TreeDepth nDepth,
                            TemplateProvider const& aTemplateProvider)
: TreeImpl()
, m_aRootPath(aRootPath)
{
//    OSL_ENSURE( aRootPath.getLocalName().getName() == _aCacheNode.getName(),
//                "Constructing root node: Path does not match node name");

    TreeImpl::build(_xStrategy,_aCacheNode,nDepth,aTemplateProvider);
}

//-----------------------------------------------------------------------------
// class ElementTreeImpl
//-----------------------------------------------------------------------------

ElementTreeImpl::ElementTreeImpl(   rtl::Reference<view::ViewStrategy> const& _xStrategy,
                                    data::TreeAccessor const& _aCacheTree, TreeDepth nDepth,
                                    TemplateHolder aTemplateInfo,
                                    TemplateProvider const& aTemplateProvider )
: TreeImpl()
, m_aInstanceInfo(aTemplateInfo)
, m_aDataAddress(_aCacheTree.address())
, m_aElementName(_aCacheTree.getName())
, m_aOwnData()
{
    TreeImpl::build( _xStrategy, _aCacheTree.getRootNode(), nDepth, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTreeImpl::ElementTreeImpl(   rtl::Reference<view::ViewStrategy> const& _xStrategy,
                                    TreeImpl& rParentTree, NodeOffset nParentNode,
                                    data::TreeAccessor const& _aCacheTree, TreeDepth nDepth,
                                    TemplateHolder aTemplateInfo,
                                    TemplateProvider const& aTemplateProvider )
: TreeImpl( rParentTree, nParentNode )
, m_aInstanceInfo(aTemplateInfo)
, m_aDataAddress(_aCacheTree.address())
, m_aElementName(_aCacheTree.getName())
, m_aOwnData()
{
    TreeImpl::build( _xStrategy, _aCacheTree.getRootNode(), nDepth, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTreeImpl::ElementTreeImpl(   data::TreeSegment const& pNewTree,
                                    TemplateHolder aTemplate,
                                    TemplateProvider const& aTemplateProvider )
: TreeImpl()
, m_aInstanceInfo(aTemplate)
, m_aDataAddress(pNewTree.getBaseAddress())
, m_aElementName(pNewTree.getName())
, m_aOwnData(pNewTree)
{
    if (!pNewTree.is())
    {
        throw Exception("ERROR: Provider can't create Element Instance From Template");
    }

    data::NodeAccess aNewNodeWrapper( m_aOwnData.getAccessor(), m_aOwnData.getSegmentRootNode() );

    TreeImpl::build( view::createDirectAccessStrategy(m_aOwnData), aNewNodeWrapper, c_TreeDepthAll, aTemplateProvider );
}
//-----------------------------------------------------------------------------

ElementTreeImpl::~ElementTreeImpl()
{
}
//-----------------------------------------------------------------------------

memory::Segment * ElementTreeImpl::getUpdatableSegment(TreeImpl& _rTree)
{
    TreeImpl * pTree = &_rTree;
    while (ElementTreeImpl * pElement = pTree->asElementTree())
    {
        if (pElement->m_aOwnData.is())
        {
            OSL_ENSURE( pElement->getContextTree()==NULL ||
                        pElement->getContextTree()->getViewBehavior() != pElement->getViewBehavior(),
                        "ElementTree with parent in same fragment should not own its data");

            memory::Segment * pSegment = pElement->m_aOwnData.getSegment();
            OSL_ASSERT(_rTree.getDataSegment() == pSegment);
            return pSegment;
        }

        pTree = pElement->getContextTree();

        if (!pTree)
        {
            OSL_ENSURE( false, "ElementTree without own data should have a parent");

            return NULL;
        }

    }
    OSL_ENSURE( false, "Tree is not part of free-floating segment - cannot support direct update");

    return NULL;

}
//-----------------------------------------------------------------------------

void ElementTreeImpl::disposeData()
{
    TreeImpl::disposeData();
    m_aOwnData.clear();
}
//-----------------------------------------------------------------------------

Path::Component ElementTreeImpl::makeExtendedName(Name const& _aSimpleName) const
{
    Name aTypeName = implGetOriginalName(root_());

    OSL_ENSURE(this->isTemplateInstance(), "ElementTree: Cannot discover the type this instantiatiates");

    OSL_ENSURE(! this->isTemplateInstance() || this->getTemplate()->getName() == aTypeName,
                "ElementTree: Type name does not match template");

    return Path::makeCompositeName(_aSimpleName, aTypeName);
}
//-----------------------------------------------------------------------------

// ownership handling
//-----------------------------------------------------------------------------
void ElementTreeImpl::rebuild(rtl::Reference<view::ViewStrategy> const & _aStrategy, data::TreeAccessor const & _aNewTree, data::Accessor const& _aOldAccessor)
{
    TreeImpl::rebuild(_aStrategy,_aNewTree.getRootNode(),_aOldAccessor);
    m_aDataAddress = _aNewTree.address();
    m_aElementName = _aNewTree.getName();
}

//-----------------------------------------------------------------------------
#if 0
void ElementTreeImpl::rebuild(rtl::Reference<view::ViewStrategy> const & _xStrategy, data::TreeAccessor const & _aNewTree)
{
    data::Accessor aOldAccessor( getViewBehavior()->getDataSegment() );
    this->rebuild(_xStrategy,_aNewTree,aOldAccessor);
}
#endif
//-----------------------------------------------------------------------------
/// transfer ownership to the given set

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void ElementTreeImpl::attachTo(data::SetNodeAccess const & aOwningSet, Name const& aElementName)
{
    OSL_ENSURE(m_aOwnData.is(),"ERROR: Cannot add a non-owned node to a subtree");

    if (m_aOwnData.is())
    {
        OSL_ENSURE(this->getSimpleRootName() == aElementName,"ElementTree: Attaching with unexpected element name");
        m_aOwnData.setName(aElementName);

        TreeImpl* pOwningTree = this->getContextTree();
        OSL_ENSURE(pOwningTree, "Element Tree Context must be set before attaching data");

        if (memory::Segment * pTargetSpace = getUpdatableSegment(*pOwningTree))
        {
            memory::UpdateAccessor aTargetAccessor(pTargetSpace);

            // copy over to the new segment
            data::TreeAddress aNewElement = m_aOwnData.getTreeAccess().copyTree(aTargetAccessor);

            data::SetNodeAccess::addElement(aTargetAccessor,aOwningSet.address(),  aNewElement);

            data::TreeAccessor aNewAccessor(aTargetAccessor.downgrade(),aNewElement);

            rtl::Reference<view::ViewStrategy> xNewBehavior = pOwningTree->getViewBehavior();

            this->rebuild(xNewBehavior,aNewAccessor,m_aOwnData.getAccessor());
        }
        else
            OSL_ENSURE( false, "Cannot attach directly to new tree - no update access available");

        m_aOwnData.clearData();
        OSL_ASSERT(!m_aOwnData.is());
    }
}
//-----------------------------------------------------------------------------

/// tranfer ownership from the given set
void ElementTreeImpl::detachFrom(data::SetNodeAccess const & aOwningSet, Name const& aElementName)
{
    OSL_ENSURE(!m_aOwnData.is(),"ERROR: Cannot detach a already owned node from a subtree");
    if (!m_aOwnData.is())
    {
        OSL_ENSURE(this->getSimpleRootName() == aElementName,"ElementTree: Detaching with unexpected element name");

        TreeImpl* pOwningTree = this->getContextTree();
        OSL_ENSURE(pOwningTree, "Element Tree Context must still be set when detaching data");

        if (memory::Segment * pTargetSpace = getUpdatableSegment(*pOwningTree))
        {
            using namespace data;

            // make a new segment with a copy of the data
            TreeSegment aNewSegment = TreeSegment::createNew( this->getOriginalTreeAccess(aOwningSet.accessor()) );

            OSL_ENSURE(aNewSegment.is(),"ERROR: Could not create detached copy of elment data");

            this->takeTreeAndRebuild( aNewSegment, aOwningSet.accessor() );

            memory::UpdateAccessor aTargetAccessor(pTargetSpace);

            TreeAddress aOldElement = data::SetNodeAccess::removeElement(aTargetAccessor,aOwningSet.address(), aElementName );
            OSL_ENSURE(aOldElement.is(),"ERROR: Detached node not found in the given subtree");

            TreeAccessor::freeTree(aTargetAccessor,aOldElement);
        }
        else
            OSL_ENSURE( false, "Cannot detach directly from old tree - no update access available");

        OSL_ENSURE(m_aOwnData.is(),"ERROR: Could not create own data segment for detached node");
    }
}
//-----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/// transfer ownership from the given owner
void ElementTreeImpl::takeTreeBack(data::TreeSegment const & _aDataSegment)
{
    OSL_ENSURE(!m_aOwnData.is(),"ERROR: Cannot take over a node - already owning");
    OSL_ENSURE(_aDataSegment.is(),"ERROR: Cannot take over NULL tree segment");

    m_aOwnData = _aDataSegment;
    OSL_ENSURE(m_aOwnData.is(),"ERROR: Could not take over data segment");

    m_aDataAddress = m_aOwnData.getBaseAddress();
    m_aElementName = m_aOwnData.getName();
}
//-----------------------------------------------------------------------------

/// transfer ownership from the given owner
void ElementTreeImpl::takeTreeAndRebuild(data::TreeSegment const & _aDataSegment, data::Accessor const & _aOldAccessor)
{
    OSL_ENSURE(!m_aOwnData.is(),"ERROR: Cannot take over a node - already owning");
    OSL_ENSURE(_aDataSegment.is(),"ERROR: Cannot take over NULL tree segment");
    if (!m_aOwnData.is())
    {
        this->rebuild(view::createDirectAccessStrategy(_aDataSegment), _aDataSegment.getTreeAccess(),_aOldAccessor);

        m_aOwnData = _aDataSegment;
        OSL_ENSURE(m_aOwnData.is(),"ERROR: Could not take over data segment");
    }
}
//-----------------------------------------------------------------------------

/// transfer ownership to the given owner
data::TreeSegment ElementTreeImpl::getOwnedTree() const
{
    OSL_ENSURE(m_aOwnData.is(),"ERROR: Cannot provide segment for a non-owned node");
    return m_aOwnData;
}
//-----------------------------------------------------------------------------

/// release ownership
data::TreeSegment ElementTreeImpl::releaseOwnedTree()
{
    OSL_ENSURE(m_aOwnData.is(),"ERROR: Cannot release and rename a non-owned node");

    data::TreeSegment aTree = m_aOwnData;
    m_aOwnData.clear();

    if (aTree.is())
        aTree.setName(m_aElementName);

    return aTree;
}
//-----------------------------------------------------------------------------

// context handling
//-----------------------------------------------------------------------------

/// renames the tree's root without concern for context consistency !
void ElementTreeImpl::renameTree(Name const& aNewName)
{
    m_aElementName = aNewName;
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

