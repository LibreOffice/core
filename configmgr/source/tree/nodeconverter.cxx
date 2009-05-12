/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeconverter.cxx,v $
 * $Revision: 1.7 $
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

#include "builddata.hxx"
#include "nodeconverter.hxx"
#include "treenodefactory.hxx"
#include <osl/diagnose.h>


//..........................................................................
namespace configmgr
{

//==========================================================================
//= OTreeNodeConverter
//==========================================================================
OTreeNodeConverter::OTreeNodeConverter()
: m_rFactory( getDefaultTreeNodeFactory() )
{
}

std::auto_ptr<ISubtree>  OTreeNodeConverter::createCorrespondingNode(SubtreeChange const& _rChange)
{
    std::auto_ptr<ISubtree> aRet;

    //if ( isLocalizedValueSet(aSubtree) ) { ... } else - no special case yet
    if (_rChange.isSetNodeChange())
    {
        aRet = nodeFactory().createSetNode(_rChange.getNodeName(),
                                       _rChange.getElementTemplateName(),
                                       _rChange.getElementTemplateModule(),
                                       _rChange.getAttributes());
    }
    else
    {
        aRet = nodeFactory().createGroupNode(_rChange.getNodeName(),
                                           _rChange.getAttributes());
    }
    return aRet;
}

//--------------------------------------------------------------------------
std::auto_ptr<ValueNode> OTreeNodeConverter::createCorrespondingNode(ValueChange const&  _rChange)
{
   // DEFAULT-TODO
    OSL_ENSURE(_rChange.getValueType().getTypeClass() != uno::TypeClass_VOID, "Losing type information converting change to value");

    std::auto_ptr<ValueNode> aRet;
    if (_rChange.getNewValue().hasValue())
       aRet = nodeFactory().createValueNode(_rChange.getNodeName(), _rChange.getNewValue(), _rChange.getAttributes());

    else
       aRet = nodeFactory().createNullValueNode(_rChange.getNodeName(), _rChange.getValueType(), _rChange.getAttributes());

    OSL_ENSURE(aRet.get() && aRet->isValid(), "Could not create corresponding value node");

    return aRet;
}

//==========================================================================
//= ONodeConverter
//==========================================================================
class ONodeConverter : public ChangeTreeModification
{
    OTreeNodeConverter&     m_rFactory;
    std::auto_ptr<INode>    m_pNode;

public:
    explicit
    ONodeConverter(OTreeNodeConverter& rFactory)
        : m_rFactory(rFactory)
    {
    }

    virtual void handle(ValueChange& aValueNode);
    virtual void handle(AddNode& aAddNode);
    virtual void handle(RemoveNode& aRemoveNode);
    virtual void handle(SubtreeChange& aSubtree);

    std::auto_ptr<INode> result() { return m_pNode; }

};
//==========================================================================
//= OCreateSubtreeAction
//==========================================================================
//= creates a subtree out of a changes list
//==========================================================================
struct OCreateSubtreeAction : public ChangeTreeModification
{
    ISubtree&           m_rTree;
    OTreeNodeConverter& m_rNodeFactory;

public:
    OCreateSubtreeAction(ISubtree& _rTree, OTreeNodeConverter& rFactory)
        :m_rTree(_rTree)
        ,m_rNodeFactory(rFactory) {}

    void handle(ValueChange& aValueNode);
    void handle(AddNode& aAddNode);
    void handle(RemoveNode& aRemoveNode);
    void handle(SubtreeChange& aSubtree);
};


//--------------------------------------------------------------------------
std::auto_ptr<ISubtree> OTreeNodeConverter::createCorrespondingTree(SubtreeChange& _rChange)
{
    std::auto_ptr<ISubtree> pBaseTree = this->createCorrespondingNode(_rChange);

    OCreateSubtreeAction aNextLevel(*pBaseTree,*this);
    _rChange.forEachChange(aNextLevel);

    return pBaseTree;
}

//--------------------------------------------------------------------------
void ONodeConverter::handle(ValueChange& aValueNode)
{
    m_pNode = base_ptr(m_rFactory.createCorrespondingNode(aValueNode));
}

//--------------------------------------------------------------------------
void ONodeConverter::handle(AddNode& aAddNode)
{
    rtl::Reference< data::TreeSegment > seg(aAddNode.getNewTree());
    m_pNode = data::convertTree(seg.is() ? seg->fragment : 0, true);
}

//--------------------------------------------------------------------------
void ONodeConverter::handle(RemoveNode& /*aRemoveNode*/)
{
    m_pNode.reset();
}

//--------------------------------------------------------------------------
void ONodeConverter::handle(SubtreeChange& aSubtree)
{
    m_pNode = base_ptr(m_rFactory.createCorrespondingNode(aSubtree));
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(ValueChange& _rChange)
{
    // create a node by a ValueChange
    std::auto_ptr<ValueNode> pNode = m_rNodeFactory.createCorrespondingNode(_rChange);

    m_rTree.addChild(base_ptr(pNode));
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(SubtreeChange& _rChange)
{
    // create a node from a SubtreeChange (recursively)
    std::auto_ptr<ISubtree> pNode = m_rNodeFactory.createCorrespondingTree(_rChange);

    // add it to the tree
    m_rTree.addChild(base_ptr(pNode));
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(RemoveNode& _rChange)
{
    { (void)_rChange; }
    // we have nothing to do
    OSL_ENSURE(!m_rTree.getChild(_rChange.getNodeName()), "Removed node found in tree being built");
}

//--------------------------------------------------------------------------
void OCreateSubtreeAction::handle(AddNode& _rChange)
{
    // free the node and add it to the subtree
    rtl::Reference< data::TreeSegment > aNewNode = _rChange.getNewTree();
    m_rTree.addChild(data::convertTree(aNewNode.is() ? aNewNode->fragment : 0, true));
}


//--------------------------------------------------------------------------

//..........................................................................
}   // namespace configmgr
//..........................................................................


