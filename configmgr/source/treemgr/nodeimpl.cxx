/*************************************************************************
 *
 *  $RCSfile: nodeimpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 12:17:22 $
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

#include "nodeimpl.hxx"

#include "treeimpl.hxx"
#include "nodechangeimpl.hxx"

#include "cmtreemodel.hxx"

#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

// helpers
//-----------------------------------------------------------------------------

namespace
{
    inline void fillInfo(NodeInfo& rInfo, OUString const& sName, NodeAttributes const& aAttributes)
    {
        rInfo.name = Name(sName, Name::NoValidate());

        rInfo.is.writable    = aAttributes.writable;
        rInfo.is.nullable    = aAttributes.optional;
        rInfo.is.notified    = aAttributes.notified;
        rInfo.is.constrained = aAttributes.constrained;

        rInfo.is.localized   = false;
        rInfo.is.defaultable = false;
    }
    inline void fetchInfo(NodeInfo& rInfo, INode const& rNode)
    {
        fillInfo(rInfo, rNode.getName(), rNode.getAttributes());
    }
}

//-----------------------------------------------------------------------------
// class NodeImpl
//-----------------------------------------------------------------------------

void NodeImpl::makeIndirect(NodeImplHolder& aThis, bool bIndirect)
{
    OSL_PRECOND(aThis.isValid() ,"ERROR: Unexpected NULL node");

    if (aThis.isValid())
    {
        OSL_ENSURE(!aThis->doHasChanges() ,"WARNING: Uncommitted changes while (possibly) changing node type - changes may be lost");
        NodeImplHolder aChanged = aThis->doCloneIndirect(bIndirect);
        aThis = aChanged;
    }
}

// Specific types of nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupNodeImpl
//-----------------------------------------------------------------------------

GroupNodeImpl::GroupNodeImpl(ISubtree& rOriginal)
: m_rOriginal(rOriginal)
{
}
//-----------------------------------------------------------------------------

GroupNodeImpl::GroupNodeImpl(GroupNodeImpl& rOriginal)
: m_rOriginal(rOriginal.m_rOriginal)
{
}
//-----------------------------------------------------------------------------

NodeType::Enum  GroupNodeImpl::getType() const
{
    return NodeType::eGROUP;
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::getNodeInfo(NodeInfo& rInfo) const
{
    fetchInfo(rInfo, m_rOriginal);
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::setNodeName(Name const& aName)
{
    m_rOriginal.setName(aName.toString());
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::dispatch(INodeHandler& rHandler)
{
    rHandler.handle(*this);
}

//-----------------------------------------------------------------------------
// class SetEntry
//-----------------------------------------------------------------------------

SetEntry::SetEntry(ElementTreeImpl* pTree_)
: m_pTree(pTree_)
{
    OSL_ENSURE(pTree_ == 0 || pTree_->isValidNode(pTree_->root()),
                "INTERNAL ERROR: Invalid empty tree used for SetEntry ");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class SetNodeImpl
//-----------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
: m_rOriginal(rOriginal)
, m_aTemplate(pTemplate)
, m_pParentTree(0)
, m_nContextPos(0)
{
}
//-----------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(SetNodeImpl& rOriginal)
: m_rOriginal(rOriginal.m_rOriginal)
, m_aTemplate(rOriginal.m_aTemplate)
, m_pParentTree(rOriginal.m_pParentTree)
, m_nContextPos(rOriginal.m_nContextPos)
{
    // unbind the original
    rOriginal.m_aTemplate.unbind();
    rOriginal.m_pParentTree = 0;
    rOriginal.m_nContextPos = 0;
}
//-----------------------------------------------------------------------------

TreeImpl*   SetNodeImpl::getParentTree() const
{
    OSL_ENSURE(m_pParentTree,"Set Node: Parent tree not set !");
    return m_pParentTree;
}
//-----------------------------------------------------------------------------

NodeOffset  SetNodeImpl::getContextOffset() const
{
    OSL_ENSURE(m_nContextPos,"Set Node: Position within parent tree not set !");
    return m_nContextPos;
}
//-----------------------------------------------------------------------------

void SetNodeImpl::getNodeInfo(NodeInfo& rInfo) const
{
    fetchInfo(rInfo, m_rOriginal);
}
//-----------------------------------------------------------------------------

void SetNodeImpl::setNodeName(Name const& aName)
{
    m_rOriginal.setName(aName.toString());
}
//-----------------------------------------------------------------------------

NodeType::Enum  SetNodeImpl::getType() const
{
    return NodeType::eSET;
}

//-----------------------------------------------------------------------------

void SetNodeImpl::dispatch(INodeHandler& rHandler)
{
    rHandler.handle(*this);
}

//-----------------------------------------------------------------------------

void SetNodeImpl::initElements(TreeImpl& rParentTree, NodeOffset nPos, TreeDepth nDepth)
{
    OSL_ENSURE(m_pParentTree == 0 || m_pParentTree == &rParentTree, "WARNING: Set Node: Changing parent");
    OSL_ENSURE(m_nContextPos == 0 || m_nContextPos == nPos,         "WARNING: Set Node: Changing location within parent");
    m_pParentTree = &rParentTree;
    m_nContextPos = nPos;

    OSL_ASSERT(doIsEmpty()); //doClearElements();

    if (nDepth > 0) doInitElements(m_rOriginal, nDepth-1);
}

//-----------------------------------------------------------------------------
// class ValueNodeImpl
//-----------------------------------------------------------------------------

ValueNodeImpl::ValueNodeImpl(ValueNode& rOriginal)
: m_rOriginal(rOriginal)
{
}
//-----------------------------------------------------------------------------

ValueNodeImpl::ValueNodeImpl(ValueNodeImpl& rOriginal)
: m_rOriginal(rOriginal.m_rOriginal)
{
}
//-----------------------------------------------------------------------------

bool ValueNodeImpl::isDefault() const
{
    return m_rOriginal.isDefault();
}
//-----------------------------------------------------------------------------

bool ValueNodeImpl::canGetDefaultValue() const
{
    return m_rOriginal.hasDefault();
}
//-----------------------------------------------------------------------------

UnoAny  ValueNodeImpl::getValue() const
{
    return m_rOriginal.getValue();
}
//-----------------------------------------------------------------------------

UnoAny ValueNodeImpl::getDefaultValue() const
{
    return m_rOriginal.getDefault();
}
//-----------------------------------------------------------------------------

UnoType ValueNodeImpl::getValueType() const
{
    return m_rOriginal.getValueType();
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::setValue(UnoAny const& aNewValue)
{
    m_rOriginal.setValue(aNewValue);
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::setDefault()
{
    m_rOriginal.setDefault();
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::getNodeInfo(NodeInfo& rInfo) const
{
    fetchInfo(rInfo, m_rOriginal);
    rInfo.is.defaultable = m_rOriginal.hasDefault();
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::setNodeName(Name const& aName)
{
    m_rOriginal.setName(aName.toString());
}
//-----------------------------------------------------------------------------

NodeType::Enum ValueNodeImpl::getType() const
{
    return NodeType::eVALUE;
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::dispatch(INodeHandler& rHandler)
{
    rHandler.handle(*this);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// legacy commit
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> SetNodeImpl::preCommitChanges()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<SubtreeChange>();
}
//-----------------------------------------------------------------------------

void SetNodeImpl::finishCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getChildTemplateName() ==  getElementTemplate()->getPath().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::revertCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getChildTemplateName() ==  getElementTemplate()->getPath().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> GroupNodeImpl::preCommitChanges()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<SubtreeChange>();
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::finishCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::revertCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::auto_ptr<ValueChange> ValueNodeImpl::preCommitChange()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<ValueChange>();
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::finishCommit(ValueChange& )
{
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::revertCommit(ValueChange& )
{
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
namespace
{
    struct AbstractNodeCast : INodeHandler
    {
        virtual void handle( ValueNodeImpl& rNode)
        {
            throw Exception( "INTERNAL ERROR: Node is not a value node. Cast failing." );
        }
        virtual void handle( GroupNodeImpl& rNode)
        {
            throw Exception( "INTERNAL ERROR: Node is not a group node. Cast failing." );
        }
        virtual void handle( SetNodeImpl& rNode)
        {
            throw Exception( "INTERNAL ERROR: Node is not a set node. Cast failing." );
        }
    };

    template <class NodeType>
    class NodeCast : AbstractNodeCast
    {
    public:
        NodeCast(NodeImpl& rOriginalNode)
        : m_pNode(0)
        {
            rOriginalNode.dispatch(*this);
        }

        NodeType& get() const
        {
            OSL_ENSURE(m_pNode, "INTERNAL ERROR: Node not set after Cast." );
            return *m_pNode;
        }

        operator NodeType& () const { return get(); }
    private:
        virtual void handle( NodeType& rNode) { m_pNode = &rNode; }
        NodeType* m_pNode;
    };
}
//-----------------------------------------------------------------------------
// domain-specific 'dynamic_cast' replacements
ValueNodeImpl&  AsValueNode(NodeImpl& rNode)
{
    return NodeCast<ValueNodeImpl>(rNode).get();
}
GroupNodeImpl&  AsGroupNode(NodeImpl& rNode)
{
    return NodeCast<GroupNodeImpl>(rNode).get();
}
SetNodeImpl&    AsSetNode  (NodeImpl& rNode)
{
    return NodeCast<SetNodeImpl>(rNode).get();
}

//-----------------------------------------------------------------------------
    }
}
