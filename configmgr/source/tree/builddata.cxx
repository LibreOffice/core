/*************************************************************************
 *
 *  $RCSfile: builddata.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:25:05 $
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

#include "builddata.hxx"

#ifndef CONFIGMGR_TREEADDRESS_HXX
#include "treeaddress.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif

#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif

#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    typedef ValueNode OValueNode; // to avoid ambiguity with sharable::ValueNode
//-----------------------------------------------------------------------------
    namespace data
    {
    //-------------------------------------------------------------------------
        using namespace sharable;
        using memory::Pointer;
        using memory::Accessor;
        using memory::Allocator;
        using memory::UpdateAccessor;
//-----------------------------------------------------------------------------

    static
    inline
    NodeAddress offsetNodeBy(NodeAddress _aNode, Offset _nOffset)
    {
        Address aRawAddr = _aNode.addressValue() + _nOffset * sizeof(Node);

        return NodeAddress(Pointer(aRawAddr));
    }

    static
    inline
    NodeAddress addressOfNodeAt(TreeAddress _aTree, Offset _nOffset)
    {
        Address aRawAddr = _aTree.addressValue() +
                            offsetof(TreeFragment,nodes) +
                            _nOffset * sizeof(Node);

        return NodeAddress(Pointer(aRawAddr));
    }

//-----------------------------------------------------------------------------

    class TreeNodeBuilder
    {
        TreeFragmentHeader  m_header;
        std::vector< Node > m_nodes;
        Offset              m_parent;
    public:
        TreeNodeBuilder() : m_header(), m_nodes(), m_parent() {}

        TreeFragmentHeader & header() { return m_header; }

        Node &      nodeAt(Offset _pos)     { checkOffset(_pos); return m_nodes[_pos]; }
        NodeInfo &  nodeInfoAt(Offset _pos) { checkOffset(_pos); return m_nodes[_pos].node.info; }

        Node &      lastNode()     { checkOffset(0); return m_nodes.back(); }
        NodeInfo &  lastNodeInfo() { checkOffset(0); return m_nodes.back().node.info; }

        void resetTreeFragment();
        void resetTreeFragment(sharable::String _treeName, State::Field _state);

        TreeAddress createTreeFragment(UpdateAccessor & _anUpdater);

        Offset  startGroup( Name _aName, Flags::Field _aFlags );
        void    endGroup( Offset _nPos );

        void    addSet( Name _aName, Flags::Field _aFlags, Address _aElementType );

        void    addValue( Name _aName, Flags::Field _aFlags,
                            AnyData::TypeCode _aValueType,
                            AnyData _aUserValue,
                            AnyData _aDefaultName );
    public:
        class CollectSetElements;
        class LinkSetNodes;

    private:
        TreeAddress allocTreeFragment(UpdateAccessor & _anUpdater);
        void linkTreeFragment(UpdateAccessor & _anUpdater, TreeAddress _aTreeAddr);

        Offset addNode(Name _aName, Flags::Field _aFlags, Type::Field _aType);
        void checkOffset(Offset _pos);
    };
//-----------------------------------------------------------------------------

    class TreeNodeBuilder::CollectSetElements
    {
        UpdateAccessor &    m_updater;
        TreeAddress         m_head;
    public:
        explicit
        CollectSetElements(UpdateAccessor & _anUpdater)
        : m_updater(_anUpdater)
        {
        }

        void resetElementList();
        void addElement(TreeAddress _aNewElement);
        List getElementListAndClear();

        UpdateAccessor &    updater()   const { return m_updater; }
    };
//-----------------------------------------------------------------------------

    class TreeNodeBuilder::LinkSetNodes : private SetVisitor
    {
        UpdateAccessor &    m_updater;
        NodeAddress         m_aParentAddr;
    public:
        explicit
        LinkSetNodes(UpdateAccessor & _anUpdater)
        : m_updater(_anUpdater)
        , m_aParentAddr()
        {
        }

        Result  linkTree(TreeAddress const & _aFragment);
        Result  linkSet(SetNodeAccess const & _aSet);
    private:
        Result  handle(TreeAccessor const & _aElement);
        Result  handle(SetNodeAccess const & _aSet);
    };
//-----------------------------------------------------------------------------

    class BasicDataTreeBuilder
    {
    public:
        explicit
        BasicDataTreeBuilder(UpdateAccessor & _anUpdater)
        : m_updater(_anUpdater)
        {}

        TreeAddress createTree() { return m_builder.createTreeFragment(m_updater); }

    protected:
        TreeNodeBuilder&    builder()         { return m_builder; }
        UpdateAccessor &    updater()   const { return m_updater; }
        Accessor            accessor()  const { return m_updater.accessor(); }
        Allocator           allocator() const { return m_updater.allocator(); }
    private:
        UpdateAccessor &    m_updater;
        TreeNodeBuilder     m_builder;
    };
//-----------------------------------------------------------------------------

    class ConvertingDataTreeBuilder : private NodeAction, public BasicDataTreeBuilder
    {
        rtl::OUString m_sRootName;
        bool m_bWithDefaults;
    public:
        explicit
        ConvertingDataTreeBuilder(UpdateAccessor & _anUpdater)
        : BasicDataTreeBuilder(_anUpdater)
        {}

        TreeAddress buildTree(OUString const & _aTreeName, INode const& _aNode, bool _bWithDefault);
        TreeAddress buildElement(INode const& _aNode, OUString const & _aTypeName, bool _bWithDefault);
    private:
        class ElementListBuilder;

        virtual void handle(ISubtree  const & _aNode);
        virtual void handle(OValueNode const & _aNode);

        Address makeTemplateData(rtl::OUString const & _aTemplateName, rtl::OUString const & _aTemplateModule);

        Name allocName(INode const & _aNode);
        State::Field makeState(node::Attributes const & _aAttributes);
        Flags::Field makeFlags(node::Attributes const & _aAttributes);
    };
//-----------------------------------------------------------------------------

    class ConvertingDataTreeBuilder::ElementListBuilder : private NodeAction
    {
        TreeNodeBuilder::CollectSetElements m_aCollector;

        rtl::OUString   m_sTypeName;
        bool            m_bWithDefaults;
    public:
        explicit
        ElementListBuilder(UpdateAccessor & _anUpdater)
        : m_aCollector(_anUpdater)
        , m_sTypeName()
        , m_bWithDefaults()
        {}

        List buildElementList(ISubtree const & _aSet, bool _bWithDefaults);
    private:
        void handleNode(INode const & _aSourceNode);

        void handle(OValueNode const & _aSourceNode);
        void handle(ISubtree  const & _aSourceNode);
    };
//-----------------------------------------------------------------------------

    class CopyingDataTreeBuilder : private NodeVisitor, public BasicDataTreeBuilder
    {
    public:
        explicit
        CopyingDataTreeBuilder(UpdateAccessor & _anUpdater)
        : BasicDataTreeBuilder(_anUpdater)
        {}

        TreeAddress buildTree(TreeAccessor const & _aSourceTree);
    private:
        class ElementListBuilder;

        Result handle(ValueNodeAccess const & _aNode);
        Result handle(GroupNodeAccess const & _aNode);
        Result handle(SetNodeAccess const & _aNode);

        Address makeTemplateData(Accessor const & _aSourceAccessor, Address _aSourceTemplate);
    };
//-----------------------------------------------------------------------------

    class CopyingDataTreeBuilder::ElementListBuilder : private SetVisitor
    {
        TreeNodeBuilder::CollectSetElements m_aCollector;
    public:
        explicit
        ElementListBuilder(UpdateAccessor & _anUpdater)
        : m_aCollector(_anUpdater)
        {}

        List buildElementList(SetNodeAccess const & _aSet);
    private:
        Result handle(TreeAccessor const & _aSourceTree);
    };
//-----------------------------------------------------------------------------

    class ConvertingNodeBuilder : private NodeVisitor
    {
        OTreeNodeFactory &  m_rNodeFactory;

        std::auto_ptr<INode> m_pNode;
    public:
        ConvertingNodeBuilder(OTreeNodeFactory & _rNodeFactory)
        : m_rNodeFactory(_rNodeFactory)
        , m_pNode()
        {
        }

        std::auto_ptr<INode>        buildNode(TreeAccessor  const& _aTree, bool _bUseTreeName);
        std::auto_ptr<INode>        buildNode(NodeAccessRef  const& _aTree);

        std::auto_ptr<ISubtree>     buildNodeTree(GroupNodeAccess const& _aGroupNode) const;
        std::auto_ptr<ISubtree>     buildNodeTree(SetNodeAccess const& _aSetNode) const;
        std::auto_ptr<OValueNode>   buildNodeTree(ValueNodeAccess const& _aValueNode) const
        { return this->convertNode(_aValueNode); }

        static node::Attributes convertAttributes(NodeAccessRef const& _aNode)
        { return _aNode.getAttributes(); }
    private:
        std::auto_ptr<ISubtree>     convertNode(GroupNodeAccess const& _aGroupNode) const;
        std::auto_ptr<ISubtree>     convertNode(SetNodeAccess const& _aSetNode) const;
        std::auto_ptr<OValueNode>   convertNode(ValueNodeAccess const& _aValueNode) const;

        Result handle(ValueNodeAccess const & _aNode);
        Result handle(GroupNodeAccess const & _aNode);
        Result handle(SetNodeAccess const & _aNode);
    };
//-----------------------------------------------------------------------------

    class ConvertingSubnodeBuilder : private SetVisitor
    {
        ConvertingNodeBuilder   m_aSubnodeBuilder;
        ISubtree &              m_rParentNode;
    public:
        ConvertingSubnodeBuilder(OTreeNodeFactory & _rNodeFactory, ISubtree & _rParentNode)
        : m_aSubnodeBuilder(_rNodeFactory)
        , m_rParentNode(_rParentNode)
        {
        }

        void addElements(SetNodeAccess const & _aSet)       { this->visitElements(_aSet); }
        void addChildren(GroupNodeAccess const & _aGroup)   { this->visitChildren(_aGroup); }
    private:
        Result handle(TreeAccessor const & _aElement);
        Result handle(NodeAccessRef const & _aMember);
    };
//-----------------------------------------------------------------------------

    class DataTreeDefaultMerger : private NodeAction
    {
    public:
        explicit
        DataTreeDefaultMerger(UpdateAccessor & _anUpdater)
        : m_updater(_anUpdater)
        {}

        void mergeDefaults(TreeAddress _aBaseAddress, INode const& _aDefaultNode);
    private:
        void handle(OValueNode const & _aNode);
        void handle(ISubtree  const & _aNode);
    protected:
        UpdateAccessor &    updater()   const { return m_updater; }
        Accessor            accessor()  const { return m_updater.accessor(); }
        Allocator           allocator() const { return m_updater.allocator(); }
    private:
        UpdateAccessor &    m_updater;
    };

//-----------------------------------------------------------------------------

    class DataTreeCleanup
    {
    public:
        explicit
        DataTreeCleanup(UpdateAccessor & _anUpdater)
        : m_updater(_anUpdater)
        {}

        TreeAddress destroyTree(TreeAddress _aBaseAddress);
    private:
        void destroyNode(NodeAddress _aNodeAddress);

        void destroyData(TreeFragmentHeader * _pHeader);
        void destroyData(NodeInfo  * _pNodeInfo);

        void destroyData(sharable::GroupNode * _pNode);
        void destroyData(sharable::ValueNode * _pNode);
        void destroyData(sharable::SetNode   * _pNode);
    private:
        UpdateAccessor &    updater()   const { return m_updater; }
        Accessor            accessor()  const { return m_updater.accessor(); }
        Allocator           allocator() const { return m_updater.allocator(); }
    private:
        UpdateAccessor &    m_updater;
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TreeAddress buildTree(memory::UpdateAccessor& _aTargetMemory, TreeAccessor const& _aTree)
{
    CopyingDataTreeBuilder aBuilder(_aTargetMemory);

    TreeAddress aResult = aBuilder.buildTree(_aTree);

    return aResult;
}
//-----------------------------------------------------------------------------

TreeAddress buildTree(memory::UpdateAccessor& _aTargetMemory, rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults)
{
    ConvertingDataTreeBuilder aBuilder(_aTargetMemory);

    TreeAddress aResult = aBuilder.buildTree(_aTreeName, _aNode,_bWithDefaults);

    return aResult;
}
//-----------------------------------------------------------------------------

TreeAddress buildElementTree(memory::UpdateAccessor& _aTargetMemory, INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefaults)
{
    ConvertingDataTreeBuilder aBuilder(_aTargetMemory);

    TreeAddress aResult = aBuilder.buildElement(_aNode, _aTypeName, _bWithDefaults);

    return aResult;
}
//-----------------------------------------------------------------------------

void mergeDefaults(memory::UpdateAccessor& _aTargetMemory, TreeAddress _aBaseAddress, INode const& _aDefaultNode)
{
    DataTreeDefaultMerger aMergeHelper(_aTargetMemory);

    aMergeHelper.mergeDefaults(_aBaseAddress, _aDefaultNode);
}
//-----------------------------------------------------------------------------

void destroyTree(memory::UpdateAccessor& _aTreeMemory, TreeAddress _aBaseAddress)
{
    DataTreeCleanup aCleaner(_aTreeMemory);

    aCleaner.destroyTree(_aBaseAddress);
}
//-----------------------------------------------------------------------------

std::auto_ptr<INode> convertTree(TreeAccessor const & _aTree, bool _bUseTreeName)
{
    ConvertingNodeBuilder aBuilder( configmgr::getDefaultTreeNodeFactory() );

    return aBuilder.buildNode(_aTree,_bUseTreeName);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline
void TreeNodeBuilder::CollectSetElements::resetElementList()
{
    OSL_ENSURE(m_head.isNull(), "Joining to a element list that was forgotten");
}
//-----------------------------------------------------------------------------

inline
List TreeNodeBuilder::CollectSetElements::getElementListAndClear()
{
    List aResult = m_head.addressValue();
    m_head = TreeAddress();
    return aResult;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::CollectSetElements::addElement(TreeAddress _aNewElement)
{
    if (TreeFragment * pNewFragment = TreeAccessor::access(_aNewElement,m_updater))
    {
        pNewFragment->header.parent = 0; // data not available here
        pNewFragment->header.next   = m_head.addressValue();

        m_head = _aNewElement;
    }
    else
        OSL_ENSURE(false, "Cannot add NULL element");
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

NodeVisitor::Result TreeNodeBuilder::LinkSetNodes::linkTree(TreeAddress const & _aTree)
{
    TreeAccessor aTreeAccess(m_updater.accessor(), _aTree);

    TreeFragment const & rTreeData = aTreeAccess.data();

    NodeAddress aOldParent = m_aParentAddr;
    m_aParentAddr = NodeAddress();

    Result eResult = CONTINUE;

    Offset nCount = rTreeData.header.count;
    for(Offset i=0; i < nCount; ++i)
    {
        NodeAccessRef aNode(&aTreeAccess.accessor(),&rTreeData.nodes[i]);
        eResult =this->visitNode( aNode );

        if (eResult == DONE) break;
    }

    m_aParentAddr = aOldParent;

    return eResult;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result TreeNodeBuilder::LinkSetNodes::linkSet(SetNodeAccess const & _aSet)
{
    OSL_ENSURE(m_aParentAddr.isNull(),"Linking set data already in progress");
    m_aParentAddr = _aSet.address();

    Result aResult = this->visitElements(_aSet);

    m_aParentAddr = NodeAddress();

    return aResult;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result TreeNodeBuilder::LinkSetNodes::handle(TreeAccessor const & _aSourceTree)
{
    OSL_ENSURE(m_aParentAddr.is(),"Cannot link set element without parent address");

    TreeFragment * pFragment = _aSourceTree.access(_aSourceTree.address(), m_updater);

    pFragment->header.parent = m_aParentAddr.addressValue();

    return CONTINUE;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result TreeNodeBuilder::LinkSetNodes::handle(SetNodeAccess const & _aNode)
{
    return this->linkSet(_aNode);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline void TreeNodeBuilder::checkOffset(Offset _pos)
{
    OSL_ENSURE(_pos < m_nodes.size(), "TreeNodeBuilder: Node access past end.");
}
//-----------------------------------------------------------------------------

Offset TreeNodeBuilder::addNode(Name _aName, Flags::Field _aFlags, Type::Field _aType)
{
    OSL_PRECOND(_aName, "TreeNodeBuilder: Unexpected NULL name");

    // TODO: consistencý checks for flags
    OSL_ENSURE(m_nodes.size() == m_header.count, "TreeNodeBuilder: node count mismatch");

    Offset nNewOffset = m_header.count++;

    m_nodes.push_back( Node() );

    OSL_ASSERT( &lastNode() == &nodeAt(nNewOffset) );

    NodeInfo & rInfo = lastNode().node.info;

    rInfo.name  = _aName;
    rInfo.flags = _aFlags;
    rInfo.type  = _aType;

    OSL_ENSURE(0 <= m_parent && m_parent <= nNewOffset, "ERROR - TreeNodeBuilder: invalid parent");
    OSL_ENSURE( (nNewOffset == 0) == (nNewOffset == m_parent), "ERROR - TreeNodeBuilder: node is own parent");

    rInfo.parent = nNewOffset - m_parent;

    return nNewOffset;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::resetTreeFragment()
{
    State::Field nNullState = State::merged | State::flag_mandatory | State::flag_readonly;
    this->resetTreeFragment(0,nNullState);
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::resetTreeFragment(sharable::String _name, State::Field _state)
{
    m_header.next = 0;
    m_header.name  = _name;

    m_header.parent = 0;

    m_header.count = 0;
    m_header.state = _state;

    m_nodes.clear();
    m_parent = 0;
}
//-----------------------------------------------------------------------------

TreeAddress TreeNodeBuilder::allocTreeFragment(UpdateAccessor & _anUpdater)
{
    OSL_ENSURE(m_nodes.size() == m_header.count, "TreeNodeBuilder: node count mismatch");

    sal_uInt32 const nFragmentSize = sizeof(TreeFragment) + (m_header.count-1)*sizeof(Node);

    OSL_ASSERT(nFragmentSize >= sizeof m_header + m_header.count*sizeof(Node));

    Address aBaseAddress = _anUpdater.allocator().allocate(nFragmentSize);

    TreeAddress aResult = TreeAddress( memory::Pointer(aBaseAddress) );

    if (TreeFragment * pFragment = TreeAccessor::access(aResult,_anUpdater))
    {
        pFragment->header = m_header;
        std::copy(m_nodes.begin(),m_nodes.end(),pFragment->nodes);
    }

    return aResult;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::linkTreeFragment(UpdateAccessor & _anUpdater, TreeAddress _aTreeFragment)
{
    LinkSetNodes(_anUpdater).linkTree(_aTreeFragment);
}
//-----------------------------------------------------------------------------

TreeAddress TreeNodeBuilder::createTreeFragment(UpdateAccessor & _anUpdater)
{
    TreeAddress aResult = allocTreeFragment(_anUpdater);

    if (aResult.is())
    {
        linkTreeFragment(_anUpdater,aResult);

        m_nodes.clear(); // ownership of indirect data has gone ...
    }
    return aResult;
}
//-----------------------------------------------------------------------------

Offset  TreeNodeBuilder::startGroup( Name _aName, Flags::Field _aFlags )
{
    Offset nNewIndex = addNode(_aName,_aFlags,Type::nodetype_group);

    lastNode().group.numDescendants = 0;

    m_parent = nNewIndex;

    return nNewIndex;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::endGroup( Offset _nPos )
{
    // while (_nPos < m_parent) endGroup(m_parent);
    OSL_PRECOND(_nPos == m_parent, "TreeNodeBuilder: Group being closed is not the current parent");

    OSL_ENSURE(nodeAt(_nPos).isGroup(), "TreeNodeBuilder: Group being closed is not a group");

    OSL_ENSURE(m_nodes.size() == m_header.count, "TreeNodeBuilder: node count mismatch");

    GroupNode & rGroup = nodeAt(_nPos).group;

    rGroup.numDescendants = m_nodes.size() - _nPos - 1;
    m_parent -= rGroup.info.parent;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::addSet( Name _aName, Flags::Field _aFlags, Address _aElementType )
{
    Offset nNewIndex = addNode(_aName,_aFlags,Type::nodetype_set);

    lastNode().set.elementType  = _aElementType;
    lastNode().set.elements     = 0;
}

//-----------------------------------------------------------------------------

void TreeNodeBuilder::addValue( Name _aName, Flags::Field _aFlags,
                                AnyData::TypeCode _aValueType,
                                AnyData _aUserValue,
                                AnyData _aDefaultValue )
{
    OSL_PRECOND(_aValueType == (_aValueType & Type::mask_valuetype), "TreeNodeBuilder: invalid value type");

    Offset nNewIndex = addNode(_aName,_aFlags,Type::nodetype_value | _aValueType);

    lastNode().value.value          = _aUserValue;
    lastNode().value.defaultValue   = _aDefaultValue;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TreeAddress CopyingDataTreeBuilder::buildTree(TreeAccessor const & _aSourceTree)
{
    OSL_ENSURE(_aSourceTree.isValid(), "Trying to build a tree from  NULL data");
    if (!_aSourceTree.isValid()) return TreeAddress();

    TreeFragment const & aSrc = _aSourceTree.data();

    sharable::String aTreeName = allocString( allocator(), aSrc.getName(_aSourceTree.accessor()));
    this->builder().resetTreeFragment(aTreeName, aSrc.header.state);

    this->visitNode(_aSourceTree.getRootNode());

    return this->createTree();
}
//-----------------------------------------------------------------------------

NodeVisitor::Result CopyingDataTreeBuilder::handle(ValueNodeAccess const & _aNode)
{
    sharable::ValueNode const & aSrc = _aNode.data();

    sharable::Name aNodeName = allocName( allocator(), aSrc.info.getName(_aNode.accessor()));
    Flags::Field aFlags = aSrc.info.flags;

    AnyData::TypeCode aType = aSrc.info.type & Type::mask_valuetype;

    AnyData aNewValue, aNewDefault;
    if (aFlags & Flags::valueAvailable)
        aNewValue = allocData(allocator(), aType, aSrc.getUserValue(_aNode.accessor()));
    else
        aNewValue.data = 0;

    if (aFlags & Flags::defaultAvailable)
        aNewDefault = allocData(allocator(), aType, aSrc.getDefaultValue(_aNode.accessor()));
    else
        aNewDefault.data = 0;

    this->builder().addValue(aNodeName,aFlags,aType,aNewValue,aNewDefault);

    return CONTINUE;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result CopyingDataTreeBuilder::handle(GroupNodeAccess const & _aNode)
{
    sharable::GroupNode const & aSrc = _aNode.data();

    sharable::Name aNodeName = allocName( allocator(), aSrc.info.getName(_aNode.accessor()));
    Flags::Field aFlags = aSrc.info.flags;

    Offset nGroupOffset = this->builder().startGroup(aNodeName,aFlags);
    this->visitChildren(_aNode);
    this->builder().endGroup(nGroupOffset);

    return CONTINUE;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result CopyingDataTreeBuilder::handle(SetNodeAccess const & _aNode)
{
    sharable::SetNode const & aSrc = _aNode.data();

    sharable::Name aNodeName = allocName( allocator(), aSrc.info.getName(_aNode.accessor()));
    Flags::Field aFlags = aSrc.info.flags;
    Address aTemplate = this->makeTemplateData(_aNode.accessor(), aSrc.elementType);

    this->builder().addSet(aNodeName,aFlags,aTemplate);

    OSL_ASSERT( this->builder().lastNode().isSet() );
    SetNode& _aNewSet = this->builder().lastNode().set;

    _aNewSet.elements = ElementListBuilder( this->updater() ).buildElementList(_aNode);

    return CONTINUE;
}
//-----------------------------------------------------------------------------

Address CopyingDataTreeBuilder::makeTemplateData(Accessor const & _aSourceAccessor, Address _aSourceTemplate)
{
    NameChar const * pTemplateName      = SetNode::getTemplateDataName(_aSourceAccessor,_aSourceTemplate);
    NameChar const * pTemplateModule    = SetNode::getTemplateDataModule(_aSourceAccessor,_aSourceTemplate);

    return SetNode::allocTemplateData(allocator(), pTemplateName, pTemplateModule );
}
//-----------------------------------------------------------------------------

List CopyingDataTreeBuilder::ElementListBuilder::buildElementList(SetNodeAccess const & _aSet)
{
    OSL_PRECOND(_aSet.isValid(), "Node must not be NULL");

    m_aCollector.resetElementList();

    this->visitElements(_aSet);

    return m_aCollector.getElementListAndClear();
}
//-----------------------------------------------------------------------------

NodeVisitor::Result CopyingDataTreeBuilder::ElementListBuilder::handle(TreeAccessor const & _aSourceTree)
{
    TreeAddress aNewElement = CopyingDataTreeBuilder(m_aCollector.updater()).buildTree(_aSourceTree);

    m_aCollector.addElement(aNewElement);

    return CONTINUE;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Name ConvertingDataTreeBuilder::allocName(INode const & _aNode)
{
    rtl::OUString sNextName = _aNode.getName();

    if (m_sRootName.getLength())
    {
        sNextName = m_sRootName;
        m_sRootName = rtl::OUString();
    }

    return sharable::allocName( allocator(), sNextName);
}
//-----------------------------------------------------------------------------

TreeAddress ConvertingDataTreeBuilder::buildElement(INode const& _aNode, OUString const & _aTypeName, bool _bWithDefaults)
{
    m_sRootName = _aTypeName;
    m_bWithDefaults = _bWithDefaults;

    sharable::String aTreeName = allocString( allocator(), _aNode.getName());
    this->builder().resetTreeFragment(aTreeName, makeState(_aNode.getAttributes()));


    this->applyToNode(_aNode);

    return this->createTree();
}
//-----------------------------------------------------------------------------

TreeAddress ConvertingDataTreeBuilder::buildTree(OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults)
{
    m_sRootName = OUString();
    m_bWithDefaults = _bWithDefaults;

    sharable::String aTreeName = allocString( allocator(), _aTreeName );
    this->builder().resetTreeFragment(aTreeName, makeState(_aNode.getAttributes()));


    this->applyToNode(_aNode);

    return this->createTree();
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::handle(ISubtree const & _aNode)
{
    sharable::Name aNodeName = allocName( _aNode );
    Flags::Field aFlags = makeFlags(_aNode.getAttributes());

    if (_aNode.isSetNode())
    {
        Address aTemplate = this->makeTemplateData(_aNode.getElementTemplateName(),_aNode.getElementTemplateModule());

        this->builder().addSet(aNodeName,aFlags,aTemplate);

        OSL_ASSERT( this->builder().lastNode().isSet() );
        SetNode& _aNewSet = this->builder().lastNode().set;

        _aNewSet.elements = ElementListBuilder(this->updater()).buildElementList(_aNode, m_bWithDefaults);
    }
    else
    {
        Offset nGroupOffset = this->builder().startGroup(aNodeName,aFlags);
        this->applyToChildren(_aNode);
        this->builder().endGroup(nGroupOffset);
    }
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::handle(OValueNode const & _aNode)
{
    sharable::Name aNodeName = allocName( _aNode );
    Flags::Field aFlags = makeFlags(_aNode.getAttributes());

    AnyData::TypeCode aType = getTypeCode(_aNode.getValueType());

    AnyData aNewValue;   aNewValue.data = 0;
    AnyData aNewDefault; aNewDefault.data = 0;

    OSL_ASSERT( !(aFlags & (Flags::valueAvailable | Flags::defaultAvailable)) );

    if (!_aNode.isDefault())
    {
        uno::Any aValue = _aNode.getValue();
        if (aValue.hasValue())
        {
            aNewValue = allocData(allocator(), aType, aValue);
            aFlags |= Flags::valueAvailable;
        }
    }

    if (_aNode.hasUsableDefault())
    {
        uno::Any aDefault = _aNode.getDefault();
        if (aDefault.hasValue())
        {
            aNewDefault = allocData(allocator(), aType, aDefault);
            aFlags |= Flags::defaultAvailable;
        }
    }

    this->builder().addValue(aNodeName,aFlags,aType,aNewValue,aNewDefault);
}
//-----------------------------------------------------------------------------

State::Field ConvertingDataTreeBuilder::makeState(node::Attributes const & _aAttributes)
{
    State::Field state;

    switch (_aAttributes.state())
    {
    case node::isDefault:   state = State::defaulted; m_bWithDefaults = true;  break;
    case node::isMerged:    state = State::merged;     break;
    case node::isReplaced:  state = State::replaced;  m_bWithDefaults = false; break;
    case node::isAdded:     state = State::added;     m_bWithDefaults = false; break;

    default: OSL_ASSERT(false); state = 0; break;
    }

    if (_aAttributes.isReadonly())
        state |= State::flag_readonly;
    //Map mandatory and Removable
    if (_aAttributes.isMandatory())
        state |= State::flag_mandatory;

    if (_aAttributes.isRemovable())
        state |= State::flag_removable;

    if ( m_bWithDefaults )
        state |= State::flag_default_avail;

    return state;
}
//-----------------------------------------------------------------------------

Flags::Field ConvertingDataTreeBuilder::makeFlags(node::Attributes const & _aAttributes)
{
    Flags::Field flags = 0;

    if ( _aAttributes.isReadonly())
        flags |= Flags::readonly;

    if ( _aAttributes.isFinalized())
        flags |= Flags::finalized;

    if ( _aAttributes.isNullable())
        flags |= Flags::nullable;

    if ( _aAttributes.isLocalized())
        flags |= Flags::localized;

    if (_aAttributes.isDefault())
        flags |= Flags::defaulted; // somewhat redundant with State

    if (!_aAttributes.isReplacedForUser())
        flags |= Flags::defaultable;  // redundant with State (merged || defaulted)

    return flags;
}
//-----------------------------------------------------------------------------

Address ConvertingDataTreeBuilder::makeTemplateData(rtl::OUString const & _aTemplateName, rtl::OUString const & _aTemplateModule)
{
    NameChar const * pTemplateName      = _aTemplateName.getStr();
    NameChar const * pTemplateModule    = _aTemplateModule.getStr();

    return SetNode::allocTemplateData(allocator(), pTemplateName, pTemplateModule );
}
//-----------------------------------------------------------------------------

List ConvertingDataTreeBuilder::ElementListBuilder::buildElementList(ISubtree const & _aSet, bool _bWithDefaults)
{
    OSL_PRECOND(_aSet.isSetNode(), "Node must be a set");

    m_aCollector.resetElementList();

    m_sTypeName = _aSet.getElementTemplateName();
    m_bWithDefaults = _bWithDefaults;

    this->applyToChildren(_aSet);

    return m_aCollector.getElementListAndClear();
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::ElementListBuilder::handleNode(INode const & _aSourceNode)
{
    TreeAddress aNewElement = ConvertingDataTreeBuilder(m_aCollector.updater())
                                 .buildElement(_aSourceNode,m_sTypeName,m_bWithDefaults);

    m_aCollector.addElement(aNewElement);
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::ElementListBuilder::handle(OValueNode const & _aSourceNode)
{
    handleNode(_aSourceNode);
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::ElementListBuilder::handle(ISubtree  const & _aSourceNode)
{
    handleNode(_aSourceNode);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

std::auto_ptr<INode> ConvertingNodeBuilder::buildNode(TreeAccessor const & _aSourceTree, bool _bUseTreeName)
{
    std::auto_ptr<INode> pResult = this->buildNode(_aSourceTree.getRootNode());
    if (pResult.get() != NULL)
    {
        // use the element name !
        if (_bUseTreeName) pResult->setName( _aSourceTree.getName().toString() );

        // do something about attributes here ?
    }
    return pResult;
}
//-----------------------------------------------------------------------------

std::auto_ptr<INode> ConvertingNodeBuilder::buildNode(NodeAccessRef const & _aSourceNode)
{
    OSL_ENSURE( !m_pNode.get(), "Old node tree will be dropped");
    this->visitNode(_aSourceNode);
    return m_pNode;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::buildNodeTree(GroupNodeAccess const& _aGroupNode) const
{
    std::auto_ptr<ISubtree> pResult = convertNode(_aGroupNode);

    if (pResult.get() != NULL)
    {
        ConvertingSubnodeBuilder aCollector(m_rNodeFactory, *pResult);
        aCollector.addChildren(_aGroupNode);
    }

    return pResult;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::buildNodeTree(SetNodeAccess const& _aSetNode) const
{
    std::auto_ptr<ISubtree> pResult = convertNode(_aSetNode);

    if (pResult.get() != NULL)
    {
        ConvertingSubnodeBuilder aCollector(m_rNodeFactory, *pResult);
        aCollector.addElements(_aSetNode);
    }

    return pResult;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::convertNode(GroupNodeAccess const& _aGroupNode) const
{
    return m_rNodeFactory.createGroupNode( _aGroupNode.getName().toString(),
                                            convertAttributes(_aGroupNode));
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::convertNode(SetNodeAccess const& _aSetNode) const
{
    return m_rNodeFactory.createSetNode(_aSetNode.getName().toString(),
                                        _aSetNode.getElementTemplateName().toString(),
                                        _aSetNode.getElementTemplateModule().toString(),
                                        convertAttributes(_aSetNode));
}
//-----------------------------------------------------------------------------

std::auto_ptr<OValueNode> ConvertingNodeBuilder::convertNode(ValueNodeAccess const& _aValueNode) const
{
    uno::Any aUserValue = _aValueNode.getUserValue();
    uno::Any aDefValue  = _aValueNode.getDefaultValue();

    if (aUserValue.hasValue() || aDefValue.hasValue())
    {
        return m_rNodeFactory.createValueNode(_aValueNode.getName().toString(),
                                                aUserValue, aDefValue,
                                                convertAttributes(_aValueNode));
    }
    else
    {
        return m_rNodeFactory.createNullValueNode(_aValueNode.getName().toString(),
                                                    _aValueNode.getValueType(),
                                                    convertAttributes(_aValueNode));
    }
}
//-----------------------------------------------------------------------------

NodeVisitor::Result ConvertingNodeBuilder::handle(ValueNodeAccess const & _aNode)
{
    m_pNode = base_ptr(buildNodeTree(_aNode));
    return DONE;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result ConvertingNodeBuilder::handle(GroupNodeAccess const & _aNode)
{
    m_pNode = base_ptr(buildNodeTree(_aNode));
    return DONE;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result ConvertingNodeBuilder::handle(SetNodeAccess const & _aNode)
{
    m_pNode = base_ptr(buildNodeTree(_aNode));
    return DONE;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

NodeVisitor::Result ConvertingSubnodeBuilder::handle(TreeAccessor const & _aElement)
{
    OSL_ASSERT(m_rParentNode.isSetNode());
    m_rParentNode.addChild( m_aSubnodeBuilder.buildNode(_aElement,true) );
    return CONTINUE;
}
//-----------------------------------------------------------------------------

NodeVisitor::Result ConvertingSubnodeBuilder::handle(NodeAccessRef const & _aMember)
{
    OSL_ASSERT(!m_rParentNode.isSetNode());
    m_rParentNode.addChild( m_aSubnodeBuilder.buildNode(_aMember) );
    return CONTINUE;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void DataTreeDefaultMerger::mergeDefaults(TreeAddress _aBaseAddress, INode const& _aDefaultNode)
{
}
//-----------------------------------------------------------------------------

void DataTreeDefaultMerger::handle(ISubtree const & _aNode)
{
}
//-----------------------------------------------------------------------------

void DataTreeDefaultMerger::handle(OValueNode const & _aNode)
{
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TreeAddress DataTreeCleanup::destroyTree(TreeAddress _aBaseAddress)
{
    TreeFragment * pData = TreeAccessor::access(_aBaseAddress,updater());

    List aNext = pData->header.next;

    Offset const nCount = pData->header.count;

    destroyData( & pData->header );

    for (Offset i = 0; i< nCount; ++i)
    {
        destroyNode( addressOfNodeAt(_aBaseAddress,i) );
    }

    allocator().deallocate( _aBaseAddress.addressValue() );

    return TreeAddress( Pointer(aNext) );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyNode(NodeAddress _aNodeAddress)
{
    Node * pNode = NodeAccess::access(_aNodeAddress,updater());

    Type::Field aTypeTag = pNode->node.info.type;
    switch ( aTypeTag & Type::mask_nodetype )
    {
    case Type::nodetype_group:
        destroyData( &pNode->group );
        break;
    case Type::nodetype_value:
        destroyData( &pNode->value );
        break;
    case Type::nodetype_set:
        destroyData( &pNode->set );
        break;
    default:
        OSL_ENSURE(false, "Cannot destroy node: Invalid type tag in node");
        break;
    }
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(TreeFragmentHeader * _pHeader)
{
    // 'component' is owned elsewhere -> leave alone

    sharable::String aName = _pHeader->name;

    freeString( allocator(), aName );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(NodeInfo * _pNodeInfo)
{
    Name aName = _pNodeInfo->name;

    if (aName) freeName( allocator(), aName );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(sharable::SetNode * _pNode)
{
    TreeAddress aElement( Pointer( _pNode->elements ) );

    Address aTemplate = _pNode->elementType;;

    destroyData(&_pNode->info);

    while (aElement.is())
        aElement = destroyTree(aElement);

    SetNode::releaseTemplateData( allocator(), aTemplate );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(sharable::GroupNode * _pNode)
{
    destroyData(&_pNode->info);
    // nothing more to do
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(sharable::ValueNode * _pNode)
{
    AnyData::TypeCode aValueType = _pNode->info.type & Type::mask_valuetype;
    Flags::Field aFlags          = _pNode->info.flags;

    destroyData(&_pNode->info);

    if (aFlags & Flags::valueAvailable)
        freeData( allocator(), aValueType, _pNode->value );

    if (aFlags & Flags::defaultAvailable)
        freeData( allocator(), aValueType, _pNode->defaultValue );

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
} // namespace configmgr


