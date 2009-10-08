/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: builddata.cxx,v $
 * $Revision: 1.14 $
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

#include "sal/types.h"

#include "builddata.hxx"
#include "nodevisitor.hxx"
#include "node.hxx"
#include "treefragment.hxx"
#include "valuenode.hxx"
#include "treenodefactory.hxx"
#include "utility.hxx"

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#include <vector>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data
    {
//-----------------------------------------------------------------------------

    static
    inline
    sharable::Node * offsetNodeBy(sharable::Node * _aNode, sal_uInt16 _nOffset)
    {
        sharable::Node *pNode = _aNode;
        pNode += _nOffset;
        return (sharable::Node *)(pNode);
    }

    static
    inline
    sharable::Node * addressOfNodeAt(sharable::TreeFragment * _aTree, sal_uInt16 _nOffset)
    {
        sharable::TreeFragment *pRaw = _aTree;
        return &pRaw->nodes[_nOffset];
    }

//-----------------------------------------------------------------------------

    class TreeNodeBuilder
    {
        sharable::TreeFragmentHeader  m_header;
        std::vector< sharable::Node > m_nodes;
        sal_uInt16              m_parent;
    public:
        TreeNodeBuilder() : m_header(), m_nodes(), m_parent() {}

        sharable::TreeFragmentHeader & header() { return m_header; }

        sharable::Node &      nodeAt(sal_uInt16 _pos)     { checkOffset(_pos); return m_nodes[_pos]; }
        sharable::NodeInfo &  nodeInfoAt(sal_uInt16 _pos) { checkOffset(_pos); return m_nodes[_pos].info; }

        sharable::Node &      lastNode()     { checkOffset(0); return m_nodes.back(); }
        sharable::NodeInfo &  lastNodeInfo() { checkOffset(0); return m_nodes.back().info; }

        void resetTreeFragment(rtl_uString * _treeName, sal_uInt8 _state);

        sharable::TreeFragment * createTreeFragment();

        sal_uInt16  startGroup( rtl_uString * _aName, sal_uInt8 _aFlags );
        void    endGroup( sal_uInt16 _nPos );

        void    addSet( rtl_uString * _aName, sal_uInt8 _aFlags, sal_uInt8 * _aElementType );

        void    addValue( rtl_uString * _aName, sal_uInt8 _aFlags,
                            sal_uInt8 _aValueType,
                            sharable::AnyData _aUserValue,
                            sharable::AnyData _aDefaultName );
    public:
        class CollectSetElements;
        class LinkSetNodes;

    private:
    sharable::TreeFragment * allocTreeFragment();
        void linkTreeFragment(sharable::TreeFragment * _aTreeAddr);

        sal_uInt16 addNode(rtl_uString * _aName, sal_uInt8 _aFlags, sal_uInt8 _aType);
        void checkOffset(sal_uInt16 _pos);
    };
//-----------------------------------------------------------------------------

    class TreeNodeBuilder::CollectSetElements
    {
        sharable::TreeFragment *         m_head;
    public:
        explicit
        CollectSetElements() : m_head(NULL) {}

        void resetElementList();
        void addElement(sharable::TreeFragment * _aNewElement);
        sharable::TreeFragment * getElementListAndClear();
    };
//-----------------------------------------------------------------------------

    class TreeNodeBuilder::LinkSetNodes: private SetVisitor {
        sharable::Node * m_parent;
    public:
        LinkSetNodes(): m_parent(0) {}

        void linkTree(sharable::TreeFragment * tree);

    private:
        using SetVisitor::handle;

        virtual bool handle(sharable::SetNode * node);

        virtual bool handle(sharable::TreeFragment * tree);
    };
//-----------------------------------------------------------------------------

    class BasicDataTreeBuilder
    {
    public:
        explicit
        BasicDataTreeBuilder() {}

        sharable::TreeFragment * createTree() { return m_builder.createTreeFragment(); }

    protected:
        TreeNodeBuilder&    builder()         { return m_builder; }
    private:
        TreeNodeBuilder     m_builder;
    };
//-----------------------------------------------------------------------------

    class ConvertingDataTreeBuilder : private NodeAction, public BasicDataTreeBuilder
    {
        rtl::OUString m_sRootName;
        bool m_bWithDefaults;
    public:
        explicit
        ConvertingDataTreeBuilder() : BasicDataTreeBuilder() {}

        sharable::TreeFragment * buildTree(rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefault);
        sharable::TreeFragment * buildElement(INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefault);
    private:
        class ElementListBuilder;

        virtual void handle(ISubtree  const & _aNode);
        virtual void handle(ValueNode const & _aNode);

        sal_uInt8 * makeTemplateData(rtl::OUString const & _aTemplateName, rtl::OUString const & _aTemplateModule);

        rtl_uString * allocName(INode const & _aNode);
        sal_uInt8 makeState(node::Attributes const & _aAttributes);
        sal_uInt8 makeFlags(node::Attributes const & _aAttributes);
    };
//-----------------------------------------------------------------------------

    class ConvertingDataTreeBuilder::ElementListBuilder : private NodeAction
    {
        TreeNodeBuilder::CollectSetElements m_aCollector;

        rtl::OUString   m_sTypeName;
        bool            m_bWithDefaults;
    public:
        explicit
        ElementListBuilder()
        : m_aCollector()
        , m_sTypeName()
        , m_bWithDefaults()
        {}

        sharable::TreeFragment *buildElementList(ISubtree const & _aSet, bool _bWithDefaults);
    private:
        void handleNode(INode const & _aSourceNode);

        void handle(ValueNode const & _aSourceNode);
        void handle(ISubtree  const & _aSourceNode);
    };
//-----------------------------------------------------------------------------

    class CopyingDataTreeBuilder : private NodeVisitor, public BasicDataTreeBuilder
    {
    public:
        explicit
        CopyingDataTreeBuilder() : BasicDataTreeBuilder() {}

        sharable::TreeFragment * buildTree(sharable::TreeFragment * sourceTree);

    private:
        class ElementListBuilder;

        using NodeVisitor::handle;
        virtual bool handle(sharable::ValueNode * node);
        virtual bool handle(sharable::GroupNode * node);
        virtual bool handle(sharable::SetNode * node);

        sal_uInt8 * makeTemplateData(sal_uInt8 * _aSourceTemplate);
    };
//-----------------------------------------------------------------------------

    class CopyingDataTreeBuilder::ElementListBuilder : private SetVisitor
    {
        TreeNodeBuilder::CollectSetElements m_aCollector;
    public:
        explicit
        ElementListBuilder() : m_aCollector() {}

        sharable::TreeFragment * buildElementList(sharable::SetNode * set);

    private:
        using SetVisitor::handle;
        virtual bool handle(sharable::TreeFragment * tree);
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

        std::auto_ptr<INode>        buildNode(sharable::TreeFragment * tree, bool _bUseTreeName);
        std::auto_ptr<INode>        buildNode(sharable::Node * tree);

        std::auto_ptr<ISubtree>     buildNodeTree(sharable::GroupNode * groupNode) const;
        std::auto_ptr<ISubtree>     buildNodeTree(sharable::SetNode * setNode) const;
        std::auto_ptr<ValueNode>   buildNodeTree(sharable::ValueNode * valueNode) const
        { return convertNode(valueNode); }

        static node::Attributes convertAttributes(sharable::Node * node)
        { return node->getAttributes(); }

    private:
        std::auto_ptr<ISubtree>     convertNode(sharable::GroupNode * groupNode) const;
        std::auto_ptr<ISubtree>     convertNode(sharable::SetNode * setNode) const;
        std::auto_ptr<ValueNode>   convertNode(sharable::ValueNode * valueNode) const;

        using NodeVisitor::handle;
        virtual bool handle(sharable::ValueNode * node);
        virtual bool handle(sharable::GroupNode * node);
        virtual bool handle(sharable::SetNode * node);
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

        void addElements(sharable::SetNode * set) { visitElements(set); }
        void addChildren(sharable::GroupNode * group) { visitChildren(group); }

    private:
        using SetVisitor::handle;
        virtual bool handle(sharable::Node * node);
        virtual bool handle(sharable::TreeFragment * tree);
    };
//-----------------------------------------------------------------------------

    class DataTreeDefaultMerger : private NodeAction
    {
    public:
        explicit
        DataTreeDefaultMerger() {}

        void mergeDefaults(sharable::TreeFragment * _aBaseAddress, INode const& _aDefaultNode);
    private:
        void handle(ValueNode const & _aNode);
        void handle(ISubtree  const & _aNode);
    };

//-----------------------------------------------------------------------------

    class DataTreeCleanup
    {
    public:
        explicit
        DataTreeCleanup() {}

        sharable::TreeFragment * destroyTree(sharable::TreeFragment * _aBaseAddress);
    private:
        void destroyNode(sharable::Node * _aNodeAddress);

        void destroyData(sharable::TreeFragmentHeader * _pHeader);
        void destroyData(sharable::NodeInfo  * _pNodeInfo);

        void destroyData(sharable::GroupNode * _pNode);
        void destroyData(sharable::ValueNode * _pNode);
        void destroyData(sharable::SetNode   * _pNode);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

sharable::TreeFragment * buildTree(sharable::TreeFragment * tree)
{
    return CopyingDataTreeBuilder().buildTree(tree);
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * buildTree(rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults)
{
    ConvertingDataTreeBuilder aBuilder;

    sharable::TreeFragment * aResult = aBuilder.buildTree(_aTreeName, _aNode,_bWithDefaults);

    return aResult;
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * buildElementTree(INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefaults)
{
    ConvertingDataTreeBuilder aBuilder;

    sharable::TreeFragment * aResult = aBuilder.buildElement(_aNode, _aTypeName, _bWithDefaults);

    return aResult;
}
//-----------------------------------------------------------------------------

void mergeDefaults(sharable::TreeFragment * _aBaseAddress, INode const& _aDefaultNode)
{
    DataTreeDefaultMerger aMergeHelper;

    aMergeHelper.mergeDefaults(_aBaseAddress, _aDefaultNode);
}
//-----------------------------------------------------------------------------

void destroyTree(sharable::TreeFragment * _aBaseAddress)
{
    DataTreeCleanup aCleaner;

    aCleaner.destroyTree(_aBaseAddress);
}
//-----------------------------------------------------------------------------

std::auto_ptr<INode> convertTree(sharable::TreeFragment * tree, bool _bUseTreeName)
{
    ConvertingNodeBuilder aBuilder( configmgr::getDefaultTreeNodeFactory() );

    return aBuilder.buildNode(tree, _bUseTreeName);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline
void TreeNodeBuilder::CollectSetElements::resetElementList()
{
    OSL_ENSURE(m_head == NULL, "Joining to a element list that was forgotten");
}
//-----------------------------------------------------------------------------

inline
sharable::TreeFragment * TreeNodeBuilder::CollectSetElements::getElementListAndClear()
{
    sharable::TreeFragment * aResult = m_head;
    m_head = NULL;
    return aResult;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::CollectSetElements::addElement(sharable::TreeFragment * _aNewElement)
{
    if (sharable::TreeFragment * pNewFragment = _aNewElement)
    {
        pNewFragment->header.parent = 0; // data not available here
        pNewFragment->header.next   = m_head;

        m_head = _aNewElement;
    }
    else
        OSL_ENSURE(false, "Cannot add NULL element");
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void TreeNodeBuilder::LinkSetNodes::linkTree(sharable::TreeFragment * tree) {
    sharable::Node * old = m_parent;
    m_parent = 0;
    sal_uInt16 n = tree->header.count;
    for (sal_uInt16 i = 0; i < n; ++i) {
        if (visitNode(tree->nodes + i)) {
            break;
        }
    }
    m_parent = old;
}

bool TreeNodeBuilder::LinkSetNodes::handle(sharable::SetNode * node)
{
    OSL_ASSERT(m_parent == 0);
    m_parent = sharable::node(node);
    bool done = visitElements(node);
    m_parent = 0;
    return done;
}

bool TreeNodeBuilder::LinkSetNodes::handle(sharable::TreeFragment * tree)
{
    OSL_ASSERT(m_parent != 0);
    tree->header.parent = m_parent;
    return false;
}

//-----------------------------------------------------------------------------

inline void TreeNodeBuilder::checkOffset(sal_uInt16 _pos)
{
    { (void)_pos; }
    OSL_ENSURE(_pos < m_nodes.size(), "TreeNodeBuilder: Node access past end.");
}
//-----------------------------------------------------------------------------

sal_uInt16 TreeNodeBuilder::addNode(rtl_uString * _aName, sal_uInt8 _aFlags, sal_uInt8 _aType)
{
    OSL_PRECOND(_aName, "TreeNodeBuilder: Unexpected NULL name");

    // TODO: consistencý checks for flags
    OSL_ENSURE(m_nodes.size() == m_header.count, "TreeNodeBuilder: node count mismatch");

    sal_uInt16 nNewOffset = m_header.count++;

    m_nodes.push_back( sharable::Node() );

    OSL_ASSERT( &lastNode() == &nodeAt(nNewOffset) );

    sharable::NodeInfo & rInfo = lastNode().info;

    rInfo.name  = _aName;
    rInfo.flags = _aFlags;
    rInfo.type  = _aType;

    OSL_ENSURE( m_parent <= nNewOffset, "ERROR - TreeNodeBuilder: invalid parent");
    OSL_ENSURE( (nNewOffset == 0) == (nNewOffset == m_parent), "ERROR - TreeNodeBuilder: node is own parent");

    rInfo.parent = nNewOffset - m_parent;

    return nNewOffset;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::resetTreeFragment(rtl_uString * _name, sal_uInt8 _state)
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

sharable::TreeFragment * TreeNodeBuilder::allocTreeFragment()
{
    OSL_ENSURE(m_nodes.size() == m_header.count, "TreeNodeBuilder: node count mismatch");

    sharable::TreeFragment *pFragment = sharable::TreeFragment::allocate(m_header.count);
    pFragment->header = m_header;
    std::copy(m_nodes.begin(),m_nodes.end(),pFragment->nodes);

    return (sharable::TreeFragment *)( pFragment );
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::linkTreeFragment(sharable::TreeFragment * _aTreeFragment)
{
    LinkSetNodes().linkTree(_aTreeFragment);
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * TreeNodeBuilder::createTreeFragment()
{
    sharable::TreeFragment * aResult = allocTreeFragment();

    if (aResult != NULL)
    {
        linkTreeFragment(aResult);

        m_nodes.clear(); // ownership of indirect data has gone ...
    }
    return aResult;
}
//-----------------------------------------------------------------------------

sal_uInt16  TreeNodeBuilder::startGroup( rtl_uString * _aName, sal_uInt8 _aFlags )
{
    sal_uInt16 nNewIndex = addNode(_aName,_aFlags,Type::nodetype_group);

    lastNode().group.numDescendants = 0;

    m_parent = nNewIndex;

    return nNewIndex;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::endGroup( sal_uInt16 _nPos )
{
    // while (_nPos < m_parent) endGroup(m_parent);
    OSL_PRECOND(_nPos == m_parent, "TreeNodeBuilder: Group being closed is not the current parent");

    OSL_ENSURE(nodeAt(_nPos).isGroup(), "TreeNodeBuilder: Group being closed is not a group");

    OSL_ENSURE(m_nodes.size() == m_header.count, "TreeNodeBuilder: node count mismatch");

    sharable::GroupNode & rGroup = nodeAt(_nPos).group;

    rGroup.numDescendants = sal_uInt16( m_nodes.size() - static_cast< ::std::size_t >(_nPos) - 1 );
    m_parent = m_parent - rGroup.info.parent;
}
//-----------------------------------------------------------------------------

void TreeNodeBuilder::addSet( rtl_uString * _aName, sal_uInt8 _aFlags, sal_uInt8 * _aElementType )
{
    addNode(_aName,_aFlags,Type::nodetype_set);

    lastNode().set.elementType  = _aElementType;
    lastNode().set.elements     = 0;
}

//-----------------------------------------------------------------------------

void TreeNodeBuilder::addValue( rtl_uString * _aName, sal_uInt8 _aFlags,
                                sal_uInt8 _aValueType,
                                sharable::AnyData _aUserValue,
                                sharable::AnyData _aDefaultValue )
{
    OSL_PRECOND(_aValueType == (_aValueType & Type::mask_valuetype), "TreeNodeBuilder: invalid value type");

    addNode(_aName,_aFlags,sal_uInt8(Type::nodetype_value | _aValueType));

    lastNode().value.value          = _aUserValue;
    lastNode().value.defaultValue   = _aDefaultValue;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

sharable::TreeFragment * CopyingDataTreeBuilder::buildTree(sharable::TreeFragment * sourceTree)
{
    OSL_ENSURE(sourceTree != 0, "Trying to build a tree from  NULL data");

    rtl_uString * aTreeName = acquireString( sourceTree->getName());
    this->builder().resetTreeFragment(aTreeName, sourceTree->header.state);

    this->visitNode(sourceTree->getRootNode());

    return this->createTree();
}
//-----------------------------------------------------------------------------

bool CopyingDataTreeBuilder::handle(sharable::ValueNode * node)
{
    rtl_uString * aNodeName = acquireString( node->info.getName());
    sal_uInt8 aFlags = node->info.flags;

    sal_uInt8 aType = sal_uInt8( node->info.type & Type::mask_valuetype );

    sharable::AnyData aNewValue, aNewDefault;
    if (aFlags & Flags::valueAvailable)
        aNewValue = sharable::allocData(aType, node->getUserValue());
    else
        aNewValue.data = 0;

    if (aFlags & Flags::defaultAvailable)
        aNewDefault = sharable::allocData(aType, node->getDefaultValue());
    else
        aNewDefault.data = 0;

    this->builder().addValue(aNodeName,aFlags,aType,aNewValue,aNewDefault);

    return false;
}
//-----------------------------------------------------------------------------

bool CopyingDataTreeBuilder::handle(sharable::GroupNode * node)
{
    rtl_uString * aNodeName = acquireString( node->info.getName());
    sal_uInt8 aFlags = node->info.flags;

    sal_uInt16 nGroupOffset = this->builder().startGroup(aNodeName,aFlags);
    this->visitChildren(node);
    this->builder().endGroup(nGroupOffset);

    return false;
}
//-----------------------------------------------------------------------------

bool CopyingDataTreeBuilder::handle(sharable::SetNode * node)
{
    rtl_uString * aNodeName = acquireString( node->info.getName());
    sal_uInt8 aFlags = node->info.flags;
    sal_uInt8 * aTemplate = this->makeTemplateData(node->elementType);

    this->builder().addSet(aNodeName,aFlags,aTemplate);

    OSL_ASSERT( this->builder().lastNode().isSet() );
    sharable::SetNode& _aNewSet = this->builder().lastNode().set;

    _aNewSet.elements = ElementListBuilder().buildElementList(node);

    return false;
}
//-----------------------------------------------------------------------------

sal_uInt8 * CopyingDataTreeBuilder::makeTemplateData(sal_uInt8 * _aSourceTemplate)
{
    return sharable::SetNode::copyTemplateData(_aSourceTemplate);
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * CopyingDataTreeBuilder::ElementListBuilder::buildElementList(sharable::SetNode * set)
{
    OSL_ASSERT(set != 0);

    m_aCollector.resetElementList();

    this->visitElements(set);

    return m_aCollector.getElementListAndClear();
}
//-----------------------------------------------------------------------------

bool CopyingDataTreeBuilder::ElementListBuilder::handle(sharable::TreeFragment * tree)
{
    sharable::TreeFragment * aNewElement = CopyingDataTreeBuilder().buildTree(tree);

    m_aCollector.addElement(aNewElement);

    return false;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

rtl_uString * ConvertingDataTreeBuilder::allocName(INode const & _aNode)
{
    rtl::OUString sNextName = _aNode.getName();

    if (m_sRootName.getLength())
    {
        sNextName = m_sRootName;
        m_sRootName = rtl::OUString();
    }

    return acquireString( sNextName);
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * ConvertingDataTreeBuilder::buildElement(INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefaults)
{
    m_sRootName = _aTypeName;
    m_bWithDefaults = _bWithDefaults;

    rtl_uString * aTreeName = acquireString( _aNode.getName());
    this->builder().resetTreeFragment(aTreeName, makeState(_aNode.getAttributes()));


    this->applyToNode(_aNode);

    return this->createTree();
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * ConvertingDataTreeBuilder::buildTree(rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults)
{
    m_sRootName = rtl::OUString();
    m_bWithDefaults = _bWithDefaults;

    rtl_uString * aTreeName = acquireString( _aTreeName );
    this->builder().resetTreeFragment(aTreeName, makeState(_aNode.getAttributes()));


    this->applyToNode(_aNode);

    return this->createTree();
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::handle(ISubtree const & _aNode)
{
    rtl_uString * aNodeName = allocName( _aNode );
    sal_uInt8 aFlags = makeFlags(_aNode.getAttributes());

    if (_aNode.isSetNode())
    {
        sal_uInt8 * aTemplate = this->makeTemplateData(_aNode.getElementTemplateName(),
                                 _aNode.getElementTemplateModule());

        this->builder().addSet(aNodeName,aFlags,aTemplate);

        OSL_ASSERT( this->builder().lastNode().isSet() );
        sharable::SetNode& _aNewSet = this->builder().lastNode().set;

        _aNewSet.elements = ElementListBuilder().buildElementList(_aNode, m_bWithDefaults);
    }
    else
    {
        sal_uInt16 nGroupOffset = this->builder().startGroup(aNodeName,aFlags);
        this->applyToChildren(_aNode);
        this->builder().endGroup(nGroupOffset);
    }
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::handle(ValueNode const & _aNode)
{
    rtl_uString * aNodeName = allocName( _aNode );
    sal_uInt8 aFlags = makeFlags(_aNode.getAttributes());

    sal_uInt8 aType = sharable::getTypeCode(_aNode.getValueType());

    sharable::AnyData aNewValue;   aNewValue.data = 0;
    sharable::AnyData aNewDefault; aNewDefault.data = 0;

    OSL_ASSERT( !(aFlags & (Flags::valueAvailable | Flags::defaultAvailable)) );

    if (!_aNode.isDefault())
    {
        uno::Any aValue = _aNode.getValue();
        if (aValue.hasValue())
        {
            aNewValue = sharable::allocData(aType, aValue);
            aFlags |= Flags::valueAvailable;
        }
    }

    if (_aNode.hasUsableDefault())
    {
        uno::Any aDefault = _aNode.getDefault();
        if (aDefault.hasValue())
        {
            aNewDefault = sharable::allocData(aType, aDefault);
            aFlags |= Flags::defaultAvailable;
        }
    }

    this->builder().addValue(aNodeName,aFlags,aType,aNewValue,aNewDefault);
}
//-----------------------------------------------------------------------------

sal_uInt8 ConvertingDataTreeBuilder::makeState(node::Attributes const & _aAttributes)
{
    sal_uInt8 state;

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

sal_uInt8 ConvertingDataTreeBuilder::makeFlags(node::Attributes const & _aAttributes)
{
    sal_uInt8 flags = 0;

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

sal_uInt8 * ConvertingDataTreeBuilder::makeTemplateData(rtl::OUString const & _aTemplateName, rtl::OUString const & _aTemplateModule)
{
    return sharable::SetNode::allocTemplateData(_aTemplateName, _aTemplateModule );
}
//-----------------------------------------------------------------------------

sharable::TreeFragment * ConvertingDataTreeBuilder::ElementListBuilder::buildElementList(ISubtree const & _aSet, bool _bWithDefaults)
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
    sharable::TreeFragment * aNewElement = ConvertingDataTreeBuilder()
                                 .buildElement(_aSourceNode,m_sTypeName,m_bWithDefaults);

    m_aCollector.addElement(aNewElement);
}
//-----------------------------------------------------------------------------

void ConvertingDataTreeBuilder::ElementListBuilder::handle(ValueNode const & _aSourceNode)
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

std::auto_ptr<INode> ConvertingNodeBuilder::buildNode(sharable::TreeFragment * sourceTree, bool _bUseTreeName)
{
    std::auto_ptr<INode> pResult = this->buildNode(sourceTree == 0 ? 0 : sourceTree->getRootNode());
    if (pResult.get() != NULL)
    {
        // use the element name !
        if (_bUseTreeName) pResult->setName( sourceTree->getName() );

        // do something about attributes here ?
    }
    return pResult;
}
//-----------------------------------------------------------------------------

std::auto_ptr<INode> ConvertingNodeBuilder::buildNode(sharable::Node * sourceNode)
{
    OSL_ENSURE( !m_pNode.get(), "Old node tree will be dropped");
    visitNode(sourceNode);
    return m_pNode;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::buildNodeTree(sharable::GroupNode * groupNode) const
{
    std::auto_ptr<ISubtree> pResult = convertNode(groupNode);

    if (pResult.get() != NULL)
    {
        ConvertingSubnodeBuilder aCollector(m_rNodeFactory, *pResult);
        aCollector.addChildren(groupNode);
    }

    return pResult;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::buildNodeTree(sharable::SetNode * setNode) const
{
    std::auto_ptr<ISubtree> pResult = convertNode(setNode);

    if (pResult.get() != NULL)
    {
        ConvertingSubnodeBuilder aCollector(m_rNodeFactory, *pResult);
        aCollector.addElements(setNode);
    }

    return pResult;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::convertNode(sharable::GroupNode * groupNode) const
{
    return m_rNodeFactory.createGroupNode( groupNode->info.getName(),
                                           convertAttributes(sharable::node(groupNode)));
}
//-----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ConvertingNodeBuilder::convertNode(sharable::SetNode * setNode) const
{
    return m_rNodeFactory.createSetNode(setNode->info.getName(),
                                        setNode->getElementTemplateName(),
                                        setNode->getElementTemplateModule(),
                                        convertAttributes(sharable::node(setNode)));
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueNode> ConvertingNodeBuilder::convertNode(sharable::ValueNode * valueNode) const
{
    uno::Any aUserValue = valueNode->getUserValue();
    uno::Any aDefValue  = valueNode->getDefaultValue();

    if (aUserValue.hasValue() || aDefValue.hasValue())
    {
        return m_rNodeFactory.createValueNode(valueNode->info.getName(),
                                              aUserValue, aDefValue,
                                              convertAttributes(sharable::node(valueNode)));
    }
    else
    {
        return m_rNodeFactory.createNullValueNode(valueNode->info.getName(),
                                                  valueNode->getValueType(),
                                                  convertAttributes(sharable::node(valueNode)));
    }
}
//-----------------------------------------------------------------------------

bool ConvertingNodeBuilder::handle(sharable::ValueNode * node)
{
    m_pNode = base_ptr(buildNodeTree(node));
    return true;
}
//-----------------------------------------------------------------------------

bool ConvertingNodeBuilder::handle(sharable::GroupNode * node)
{
    m_pNode = base_ptr(buildNodeTree(node));
    return true;
}
//-----------------------------------------------------------------------------

bool ConvertingNodeBuilder::handle(sharable::SetNode * node)
{
    m_pNode = base_ptr(buildNodeTree(node));
    return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool ConvertingSubnodeBuilder::handle(sharable::TreeFragment * tree)
{
    OSL_ASSERT(m_rParentNode.isSetNode());
    m_rParentNode.addChild(m_aSubnodeBuilder.buildNode(tree, true));
    return false;
}
//-----------------------------------------------------------------------------

bool ConvertingSubnodeBuilder::handle(sharable::Node * node)
{
    OSL_ASSERT(!m_rParentNode.isSetNode());
    m_rParentNode.addChild(m_aSubnodeBuilder.buildNode(node));
    return false;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void DataTreeDefaultMerger::mergeDefaults(sharable::TreeFragment * /*_aBaseAddress*/, INode const& /*_aDefaultNode*/)
{
}
//-----------------------------------------------------------------------------

void DataTreeDefaultMerger::handle(ISubtree const & /*_aNode*/)
{
}
//-----------------------------------------------------------------------------

void DataTreeDefaultMerger::handle(ValueNode const & /*_aNode*/)
{
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

sharable::TreeFragment * DataTreeCleanup::destroyTree(sharable::TreeFragment * _aBaseAddress)
{
    sharable::TreeFragment *pData = _aBaseAddress;

    sharable::TreeFragment *pNext = pData->header.next;

    sal_uInt16 const nCount = pData->header.count;

    destroyData( & pData->header );

    for (sal_uInt16 i = 0; i< nCount; ++i)
    {
        destroyNode( addressOfNodeAt(_aBaseAddress,i) );
    }

    sharable::TreeFragment::free_shallow( pData );

    return (sharable::TreeFragment *)( pNext );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyNode(sharable::Node * _aNodeAddress)
{
    sharable::Node * pNode = _aNodeAddress;

    sal_uInt8 aTypeTag = pNode->info.type;
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

void DataTreeCleanup::destroyData(sharable::TreeFragmentHeader * _pHeader)
{
    // 'component' is owned elsewhere -> leave alone

    rtl_uString * aName = _pHeader->name;

    rtl_uString_release( aName );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(sharable::NodeInfo * _pNodeInfo)
{
    rtl_uString * aName = _pNodeInfo->name;

    if (aName) rtl_uString_release( aName );
}
//-----------------------------------------------------------------------------

void DataTreeCleanup::destroyData(sharable::SetNode * _pNode)
{
    sharable::TreeFragment * aElement( _pNode->elements );

    sal_uInt8 * aTemplate = _pNode->elementType;;

    destroyData(&_pNode->info);

    while (aElement != NULL)
        aElement = destroyTree(aElement);

    sharable::SetNode::releaseTemplateData( aTemplate );
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
    sal_uInt8 aValueType = sal_uInt8( _pNode->info.type & Type::mask_valuetype );
    sal_uInt8 aFlags          = _pNode->info.flags;

    destroyData(&_pNode->info);

    if (aFlags & Flags::valueAvailable)
        freeData( aValueType, _pNode->value );

    if (aFlags & Flags::defaultAvailable)
        freeData( aValueType, _pNode->defaultValue );

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
} // namespace configmgr


