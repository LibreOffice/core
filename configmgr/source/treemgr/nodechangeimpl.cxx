/*************************************************************************
 *
 *  $RCSfile: nodechangeimpl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-21 12:02:38 $
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

#include "nodechangeimpl.hxx"

#include "nodechangeinfo.hxx"
#include "nodeimpl.hxx"
#include "treeimpl.hxx"

#include "setnodeimplbase.hxx"
#include "groupnodeimpl.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// All Changes:NodeChangeImpl  - common base
//-----------------------------------------------------------------------------

// life cycle states for a NodeChangeImpl
enum { eTestedChange = 0x01, eAppliedChange = 0x02, eNoCheck = 0x07 };
//-----------------------------------------------------------------------------

NodeChangeImpl::NodeChangeImpl(bool bNoCheck)
: m_aAffectedTree()
, m_nAffectedNode(0)
, m_nState(0)
{
    if (bNoCheck) m_nState = eNoCheck;
}
//-----------------------------------------------------------------------------

TreeHolder NodeChangeImpl::getBaseTree() const
{
    TreeHolder aRet = doGetBaseTree();
    OSL_ENSURE( aRet.isValid(), "ERROR: Configuration Change: Base Tree has not been set up" );

    return aRet;
}
//-----------------------------------------------------------------------------

NodeOffset NodeChangeImpl::getBaseNode() const
{
    NodeOffset nRet = doGetBaseNode();
    OSL_ENSURE( nRet != 0, "ERROR: Configuration Change: Base Node has not been set up" );
    OSL_ENSURE( doGetBaseTree().isValid() && doGetBaseTree()->isValidNode(nRet),
                "ERROR: Configuration Change: Base Node does not match tree" );

    return nRet;
}
//-----------------------------------------------------------------------------

TreeHolder NodeChangeImpl::getAffectedTree() const
{
    TreeHolder aRet = m_aAffectedTree;
    OSL_ENSURE( aRet.isValid(), "ERROR: Configuration Change: Target Tree has not been set up" );

    return aRet;
}
//-----------------------------------------------------------------------------

NodeOffset NodeChangeImpl::getAffectedNode() const
{
    NodeOffset nRet = m_nAffectedNode;
    OSL_ENSURE( nRet != 0, "ERROR: Configuration Change: Target Node has not been set up" );
    OSL_ENSURE( m_aAffectedTree.isValid() && m_aAffectedTree->isValidNode(nRet),
                "ERROR: Configuration Change: Changing Node does not match tree" );

    return nRet;
}
//-----------------------------------------------------------------------------

RelativePath NodeChangeImpl::getPathToChangingNode() const
{
    // TODO: Add DEBUG-verification of the result
    return doGetChangingNodePath();
}
//-----------------------------------------------------------------------------


void NodeChangeImpl::setAffected(TreeHolder const& aAffectedTree, NodeOffset nAffectedNode)
{
    OSL_ENSURE(m_nState == 0 || (m_aAffectedTree.isEmpty() && m_nState == eNoCheck), "WARNING: Configuration: Retargeting change that already was tested or applied");

    OSL_ENSURE( aAffectedTree.isValid(), "ERROR: Configuration Change: NULL Target Tree is not allowed" );
    OSL_ENSURE( nAffectedNode, "ERROR: Configuration Change: NULL Target Node is not allowed" );
    OSL_ENSURE( aAffectedTree->isValidNode(nAffectedNode), "ERROR: Configuration Change: Target Node does not match Tree" );

    if (m_nState != eNoCheck) m_nState = 0; // previous checks are invalidated

    m_aAffectedTree = aAffectedTree;
    m_nAffectedNode = nAffectedNode;
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::isChange(bool bAllowUntested) const
{
    OSL_ENSURE(bAllowUntested || (m_nState & eTestedChange), "WARNING: Configuration: Change was not tested  - isChange is meaningless");

    if (m_nState == eNoCheck)
        return true;

    if (!(m_nState & eTestedChange))
        return bAllowUntested;

    return doIsChange(!(m_nState & eAppliedChange));
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::fillChangeData(NodeChangeData& rChange) const
{
    OSL_ENSURE(m_nState & eTestedChange, "WARNING: Configuration: Change was not tested  - fillChange is partially meaningless");
    return doFillChange(rChange) || rChange.isDataChange(); // force true if the data is signaling change
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::fillChangeLocation(NodeChangeLocation& rChange) const
{
    if (!m_aAffectedTree.isValid()) return false;

    rChange.setBase( NodeID(this->getBaseTree().getBodyPtr(), this->getBaseNode()) );

    rChange.setAccessor( this->getPathToChangingNode() );

    rChange.setAffected( NodeID(this->getAffectedTree().getBodyPtr(), this->getAffectedNode()) );

    rChange.setChangingSubnode( this->doIsChangingSubnode() );

    return true;
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::fillChangeInfo(NodeChangeInformation& rChange) const
{
    return fillChangeLocation(rChange.location) & fillChangeData(rChange.change);
}
//-----------------------------------------------------------------------------

void NodeChangeImpl::test()
{
    if (!(m_nState & eTestedChange))
    {
        Node* pTarget = implGetTarget();
        OSL_ENSURE(pTarget, "ERROR: Configuration: No target - cannot test change");
        if (pTarget)
        {
            doTest(*pTarget);
            m_nState |= eTestedChange;
        }
    }
}
//-----------------------------------------------------------------------------

void NodeChangeImpl::apply()
{
    if (!(m_nState & eAppliedChange))
    {
        implApply();

        OSL_ENSURE(m_nState & eAppliedChange, "ERROR: Configuration: Change could not be applied");
        OSL_ENSURE(m_nState & eTestedChange, "ERROR: Configuration: Change was not tested while applied");
    }
    else
        OSL_ENSURE(m_nState & eTestedChange, "ERROR: Configuration: Change marked applied but not tested");
}
//-----------------------------------------------------------------------------

/// apply this change to the given node - start state is nState (which is then updated)
void NodeChangeImpl::implApply()
{
    OSL_ASSERT( !(m_nState & eAppliedChange)); // Caller must check

    Node* pTarget = implGetTarget();
    if (pTarget)
    {
        if (!(m_nState & eTestedChange))  // Test checks the old value if there is realy a change
        {                               // for eventlisteners to say "the old value is kept"
            doTest(*pTarget);
            m_nState |= eTestedChange;
        }
        doApply(*pTarget);
        m_nState |= eAppliedChange;
    }
}
//-----------------------------------------------------------------------------

Node* NodeChangeImpl::implGetTarget() const
{
    OSL_ENSURE(m_aAffectedTree.isValid(), "ERROR: Configuration Change: no target tree set");
    if (!m_aAffectedTree.isValid()) return NULL;

    OSL_ENSURE(m_aAffectedTree->isValidNode(m_nAffectedNode), "ERROR: Configuration Change: target node not in target tree");
    if (!m_aAffectedTree->isValidNode(m_nAffectedNode)) return NULL;

    Node* pTarget = m_aAffectedTree->node(m_nAffectedNode);
    OSL_ENSURE(pTarget, "ERROR: Configuration: No target for change");
    return pTarget;
}
//-----------------------------------------------------------------------------

// hook method default implementations
TreeHolder NodeChangeImpl::doGetBaseTree() const
{
    return m_aAffectedTree;
}
//-----------------------------------------------------------------------------

NodeOffset NodeChangeImpl::doGetBaseNode() const
{
    return m_nAffectedNode;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Value operations: ValueChangeImpl = common base
//-----------------------------------------------------------------------------

ValueChangeImpl::ValueChangeImpl()
: m_aNewValue()
, m_aOldValue()
{
}
//-----------------------------------------------------------------------------

ValueChangeImpl::ValueChangeImpl(UnoAny const& aNewValue)
: m_aNewValue(aNewValue)
, m_aOldValue()
{
}
//-----------------------------------------------------------------------------

ValueChangeImpl::ValueChangeImpl(UnoAny const& aNewValue, UnoAny const& aOldValue)
: NodeChangeImpl(true)
, m_aNewValue(aNewValue)
, m_aOldValue(aOldValue)
{
}
//-----------------------------------------------------------------------------

ValueChangeImpl::~ValueChangeImpl()
{
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::setTarget(TreeHolder const& aAffectedTree, NodeOffset nParentNode, Name const& sNodeName)
{
    if ( !sNodeName.isEmpty() || !aAffectedTree.isValid() )
    {
        NodeChangeImpl::setAffected(aAffectedTree,nParentNode);
        m_aName = sNodeName;
    }
    else
    {
        OSL_ENSURE(false, "ValueChangeTarget is being set without a name");
        NodeChangeImpl::setAffected(aAffectedTree,aAffectedTree->parent(nParentNode));
        m_aName = aAffectedTree->getNodeName(nParentNode);
    }
}
//-----------------------------------------------------------------------------

RelativePath ValueChangeImpl::doGetChangingNodePath() const
{
    return RelativePath( m_aName );
}
//-----------------------------------------------------------------------------

bool ValueChangeImpl::doIsChangingSubnode() const
{
    return ! m_aName.isEmpty();
}
//-----------------------------------------------------------------------------

bool ValueChangeImpl::doIsChange(bool ) const
{
    return !!(getNewValue() != getOldValue());
}
//-----------------------------------------------------------------------------

bool ValueChangeImpl::doFillChange(NodeChangeData& rChange) const
{
    rChange.unoData.newValue = getNewValue();
    rChange.unoData.oldValue = getOldValue();
    return rChange.unoData.isDataChange();
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::doTest( Node& rTarget)
{
    OSL_ENSURE(rTarget.isGroupNode(), "ERROR: Configuration: Target type mismatch: expected a group node holding the value");
    ValueMemberNode aValueTarget = rTarget.groupImpl().getValue( m_aName );

    OSL_ENSURE(aValueTarget.isValid(), "ERROR: Configuration: Target missing: could not find the changing value");

    preCheckValue(aValueTarget, m_aOldValue, m_aNewValue);
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::doApply( Node& rTarget)
{
    OSL_ENSURE(rTarget.isGroupNode(), "ERROR: Configuration: Target type mismatch: expected a group node holding the value");
    ValueMemberUpdate aValueTarget = rTarget.groupImpl().getValueForUpdate( m_aName );

    OSL_ENSURE(aValueTarget.isValid(), "ERROR: Configuration: Target missing: could not find the changing value");

    doApplyChange(aValueTarget);
    postCheckValue(aValueTarget.getNode(), m_aNewValue); // Sideeffect: m_aNewValue will be changed
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::preCheckValue(ValueMemberNode& rNode, UnoAny& rOld, UnoAny& )
{
    UnoAny aPrevValue = rNode.getValue();
    OSL_ENSURE(!rOld.hasValue() || rOld == aPrevValue, "ERROR: Configuration: Stored old value of target does not match the actual value");
    rOld = aPrevValue;
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::postCheckValue(ValueMemberNode& rNode, UnoAny& rNew)
{
    UnoAny aResultValue = rNode.getValue();
    OSL_ENSURE(!rNew.hasValue() || rNew == aResultValue, "ERROR: Configuration: New value of target does not match the predicted result");
    rNew = aResultValue;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Value operations: ValueReplaceImpl = set local value
//-----------------------------------------------------------------------------

ValueReplaceImpl::ValueReplaceImpl(UnoAny const& aNewValue)
:ValueChangeImpl(aNewValue)
{
}
//-----------------------------------------------------------------------------

ValueReplaceImpl::ValueReplaceImpl(UnoAny const& aNewValue, UnoAny const& aOldValue)
:ValueChangeImpl(aNewValue, aOldValue)
{
}
//-----------------------------------------------------------------------------

void ValueReplaceImpl::doApplyChange( ValueMemberUpdate& rNode)
{
    rNode.setValue(getNewValue());
}
//-----------------------------------------------------------------------------

bool ValueReplaceImpl::doFillChange( NodeChangeData& rChange) const
{
    rChange.type = NodeChangeData::eSetValue;
    return ValueChangeImpl::doFillChange(rChange);
}

//-----------------------------------------------------------------------------
// Value operations: ValueResetImpl = set to default
//-----------------------------------------------------------------------------

ValueResetImpl::ValueResetImpl()
:ValueChangeImpl()
{
}
//-----------------------------------------------------------------------------

ValueResetImpl::ValueResetImpl(UnoAny const& aNewValue, UnoAny const& aOldValue)
:ValueChangeImpl(aNewValue, aOldValue)
{
}
//-----------------------------------------------------------------------------

void ValueResetImpl::doApplyChange( ValueMemberUpdate& rNode)
{
    rNode.setDefault();
}
//-----------------------------------------------------------------------------

bool ValueResetImpl::doFillChange( NodeChangeData& rChange) const
{
    rChange.type = NodeChangeData::eSetDefault;
    ValueChangeImpl::doFillChange(rChange);
    return !rChange.isEmptyChange(); // do it defensively here - default (= 'new') may be unknown still
}
//-----------------------------------------------------------------------------

void ValueResetImpl::preCheckValue(ValueMemberNode& rNode, UnoAny& rOld, UnoAny& rNew)
{
    ValueChangeImpl::preCheckValue(rNode,rOld,rNew);

    UnoAny aDefaultValue = rNode.getDefaultValue();
    OSL_ENSURE(!rNew.hasValue() || rNew == aDefaultValue, "ERROR: Configuration: Stored new value of target does not match the actual default value");
    rNew = aDefaultValue;
}

//-----------------------------------------------------------------------------
// Value operations: DeepValueReplaceImpl = set nested value
//-----------------------------------------------------------------------------

DeepValueReplaceImpl::DeepValueReplaceImpl(RelativePath const& aRelPath, UnoAny const& aNewValue)
: ValueReplaceImpl(aNewValue)
, m_aBaseTree()
, m_nBaseNode(0)
, m_aNestedPath(aRelPath)
{
}
//-----------------------------------------------------------------------------

DeepValueReplaceImpl::DeepValueReplaceImpl(RelativePath const& aRelPath, UnoAny const& aNewValue, UnoAny const& aOldValue)
: ValueReplaceImpl(aNewValue, aOldValue)
, m_aBaseTree()
, m_nBaseNode(0)
, m_aNestedPath(aRelPath)
{
}
//-----------------------------------------------------------------------------

void DeepValueReplaceImpl::setBaseContext(TreeHolder const& aBaseTree, NodeOffset nBaseNode)
{
    OSL_ENSURE(aBaseTree.isValid() && aBaseTree->isValidNode(nBaseNode), "ERROR: Setting invalid base tree / node for deep change");

    m_aBaseTree = aBaseTree;
    m_nBaseNode = nBaseNode;
}
//-----------------------------------------------------------------------------

RelativePath DeepValueReplaceImpl::doGetChangingNodePath() const
{
    return m_aNestedPath;
}
//-----------------------------------------------------------------------------

TreeHolder DeepValueReplaceImpl::doGetBaseTree() const
{
    return m_aBaseTree;
}
//-----------------------------------------------------------------------------

NodeOffset DeepValueReplaceImpl::doGetBaseNode() const // use your own
{
    return m_nBaseNode;
}

//-----------------------------------------------------------------------------
// All Sets: SetChangeImpl - common base
//-----------------------------------------------------------------------------

SetChangeImpl::SetChangeImpl(Name const& aName, bool bNoCheck)
: NodeChangeImpl(bNoCheck)
, m_aName(aName)
{
}
//-----------------------------------------------------------------------------

void SetChangeImpl::setTarget(TreeHolder const& aAffectedTree, NodeOffset nAffectedNode)
{
    NodeChangeImpl::setAffected(aAffectedTree,nAffectedNode);
}
//-----------------------------------------------------------------------------

RelativePath SetChangeImpl::doGetChangingNodePath() const
{
    return RelativePath(getElementName());
}
//-----------------------------------------------------------------------------

bool SetChangeImpl::doIsChangingSubnode() const
{
    return false;
}
//-----------------------------------------------------------------------------

void SetChangeImpl::doTest( Node& rTarget)
{
    doTestElement(rTarget.setImpl(), getElementName() );
}
//-----------------------------------------------------------------------------

void SetChangeImpl::doApply( Node& rTarget)
{
    doApplyToElement(rTarget.setImpl(), getElementName() );
}

//-----------------------------------------------------------------------------
// Full Sets: SetInsertTreeImpl
//-----------------------------------------------------------------------------

SetInsertTreeImpl::SetInsertTreeImpl(Name const& aName, ElementTreeHolder const& aNewTree, bool bNoCheck)
: SetChangeImpl(aName,bNoCheck)
, m_aNewTree(aNewTree)
{
}
//-----------------------------------------------------------------------------

bool SetInsertTreeImpl::doIsChange(bool ) const
{
    return !!m_aNewTree.isValid();
}
//-----------------------------------------------------------------------------

bool SetInsertTreeImpl::doFillChange(NodeChangeData& rChange) const
{
    rChange.type = NodeChangeData::eInsertElement;
    if (m_aNewTree.isValid())
        rChange.element.newValue = m_aNewTree;

    return isChange(true);
}
//-----------------------------------------------------------------------------

void SetInsertTreeImpl::doTestElement( SetNodeImpl& rNode, Name const& aName)
{
    SetEntry anEntry = rNode.findElement(aName); // require loaded children
    OSL_ENSURE(!anEntry.isValid(), "ERROR: Configuration: Adding a node that already exists");
}
//-----------------------------------------------------------------------------

void SetInsertTreeImpl::doApplyToElement( SetNodeImpl& rNode, Name const& aName)
{
    if (m_aNewTree.isValid())
    {
        SetEntry aNewEntry( m_aNewTree.getBodyPtr() );
        rNode.insertElement( aName, aNewEntry);
    }
}

//-----------------------------------------------------------------------------
// Full Sets: SetReplaceTreeImpl
//-----------------------------------------------------------------------------

SetReplaceTreeImpl::SetReplaceTreeImpl(Name const& aName, ElementTreeHolder const& aNewTree)
: SetChangeImpl(aName)
, m_aNewTree(aNewTree)
, m_aOldTree()
{
}
//-----------------------------------------------------------------------------

SetReplaceTreeImpl::SetReplaceTreeImpl(Name const& aName, ElementTreeHolder const& aNewTree, ElementTreeHolder const& aOldTree)
: SetChangeImpl(aName,true)
, m_aNewTree(aNewTree)
, m_aOldTree(aOldTree)
{
}
//-----------------------------------------------------------------------------

/// checks, if this represents an actual change
bool SetReplaceTreeImpl::doIsChange(bool) const
{
    return !!(m_aOldTree != m_aNewTree);
}
//-----------------------------------------------------------------------------

/// fills in pre- and post-change values, returns wether they differ
bool SetReplaceTreeImpl::doFillChange(NodeChangeData& rChange) const
{
    rChange.type = NodeChangeData::eReplaceElement;
    if (m_aNewTree.isValid())
        rChange.element.newValue =  m_aNewTree;

    if (m_aOldTree.isValid())
        rChange.element.oldValue = m_aOldTree;

    return isChange(true);
}
//-----------------------------------------------------------------------------

void SetReplaceTreeImpl::doTestElement( SetNodeImpl& rNode, Name const& aName)
{
    OSL_ASSERT(!m_aOldTree.isValid()); // already tested ?

    // remove the old node
    SetEntry anEntry = rNode.findElement(aName); // require loaded children
    OSL_ENSURE(anEntry.isValid(), "ERROR: Configuration: Replacing a node that doesn't exist");

    m_aOldTree = anEntry.tree();
}
//-----------------------------------------------------------------------------

void SetReplaceTreeImpl::doApplyToElement( SetNodeImpl& rNode, Name const& aName)
{
    if (m_aOldTree != m_aNewTree)
    {
        OSL_ENSURE(m_aOldTree.isValid(), "ERROR: Configuration: Replacing a node that doesn't exist");
        rNode.removeElement(aName);

    // add the new one
        OSL_ENSURE(m_aNewTree.isValid(), "ERROR: Configuration: Replacing a node with nothing");
        if (m_aNewTree.isValid())
        {
            SetEntry aNewEntry( m_aNewTree.getBodyPtr() );
            rNode.insertElement( aName, aNewEntry);
        }
    }
}

//-----------------------------------------------------------------------------
// Full Sets: SetRemoveTreeImpl
//-----------------------------------------------------------------------------

SetRemoveTreeImpl::SetRemoveTreeImpl(Name const& aName)
: SetChangeImpl(aName)
, m_aOldTree()
{
}
//-----------------------------------------------------------------------------

SetRemoveTreeImpl::SetRemoveTreeImpl(Name const& aName, ElementTreeHolder const& aOldTree)
: SetChangeImpl(aName,true)
, m_aOldTree(aOldTree)
{
}
//-----------------------------------------------------------------------------

/// checks, if this represents an actual change
bool SetRemoveTreeImpl::doIsChange(bool) const
{
    return !!m_aOldTree.isValid();
}
//-----------------------------------------------------------------------------

/// fills in pre- and post-change values, returns wether they differ
bool SetRemoveTreeImpl::doFillChange(NodeChangeData& rChange) const
{
    rChange.type = NodeChangeData::eRemoveElement;
    if (m_aOldTree.isValid())
        rChange.element.oldValue = m_aOldTree;

    return isChange(true);
}
//-----------------------------------------------------------------------------

void SetRemoveTreeImpl::doTestElement( SetNodeImpl& rNode, Name const& aName)
{
    OSL_ASSERT(!m_aOldTree.isValid()); // already tested ?

    // remove the old node
    SetEntry anEntry = rNode.findElement(aName); // require loaded children
    OSL_ENSURE(anEntry.isValid(), "ERROR: Configuration: Removing a node that doesn't exist");

    m_aOldTree = anEntry.tree();
}
//-----------------------------------------------------------------------------

void SetRemoveTreeImpl::doApplyToElement( SetNodeImpl& rNode, Name const& aName)
{
    rNode.removeElement(aName);
}

/*
//-----------------------------------------------------------------------------
// Value Sets: Helpers
//-----------------------------------------------------------------------------

/// provides a wrapper for completely new leaf values (e.g. within value sets)
ElementTreeHolder makeNewValueTree(Name const& rName, UnoAny const& rValue, UnoType& rType)
{
    std::auto_ptr<ValueNode> pNode;
    if (rValue.hasValue())
        pNode.reset( new ValueNode(rName.toString(),rValue) );
    else
        pNode.reset( new ValueNode(rName.toString(),rType) );

    return new ElementTreeImpl(pNode);
}

//-----------------------------------------------------------------------------
// Value Sets: SetInsertValueImpl
//-----------------------------------------------------------------------------

SetInsertValueImpl::SetInsertValueImpl(Name const& aName, UnoAny const& aNewValue)
: SetChangeImpl(aName)
, m_aNewValue(aNewValue)
{
}
//-----------------------------------------------------------------------------

bool SetInsertValueImpl::doIsChange(bool) const
{
    return true;
}
//-----------------------------------------------------------------------------

bool SetInsertValueImpl::doFillChange(DataChange& rChange) const
{
    rChange.newValue = getNewValue();

    return rChange.isChange();
}
//-----------------------------------------------------------------------------

void SetInsertValueImpl::doTestElement( SetNodeImpl& rNode, Name const& aName)
{
    OSL_ENSURE(!rNode.findElement(aName).isValid(), "ERROR: Configuration: Adding a node that already exists");
}

//-----------------------------------------------------------------------------
void SetInsertValueImpl::doApplyToElement( SetNodeImpl& rNode, Name const& aName)
{
//  SetEntry aNewEntry ;//= { m_aNewValue.getBodyPtr(), m_aNewValue->rootNode() };
    rNode.insertValue(aName, getNewValue());
}

//-----------------------------------------------------------------------------
// Value Sets: SetReplaceValueImpl
//-----------------------------------------------------------------------------

SetReplaceValueImpl::SetReplaceValueImpl(Name const& aName, UnoAny const& aNewValue)
: SetChangeImpl(aName)
, m_aReplacer(aNewValue)
{
}
//-----------------------------------------------------------------------------

bool SetReplaceValueImpl::doIsChange(bool) const
{
    return m_aReplacer.isChange();
}
//-----------------------------------------------------------------------------

bool SetReplaceValueImpl::doFillChange(DataChange& rChange) const
{
    return m_aReplacer.fillChange(rChange);
}
//-----------------------------------------------------------------------------

void SetReplaceValueImpl::doTestElement( SetNodeImpl& rNode, Name const& aName)
{
    Node* pNode = rNode.findElement(aName).pNode;
    OSL_ENSURE( pNode, "ERROR: Configuration: Replacing a node that doesn't exist" );

    if (pNode)
    {
        OSL_ENSURE( pNode->isValueNode(), "ERROR: Configuration: Replacing a non value node as value node" );
        m_aReplacer.doTest(*pNode);
    }

}
//-----------------------------------------------------------------------------

void SetReplaceValueImpl::doApplyToElement( SetNodeImpl& rNode, Name const& aName)
{
    Node* pNode = rNode.findElement(aName).pNode;
    OSL_ENSURE( pNode, "ERROR: Configuration: Replacing a node that doesn't exist" );

    if (pNode)
    {
        OSL_ENSURE( pNode->isValueNode(), "ERROR: Configuration: Replacing a non value node as value node" );
        m_aReplacer.doApply(*pNode);
    }
}

//-----------------------------------------------------------------------------
// Value Sets: SetRemoveValueImpl
//-----------------------------------------------------------------------------

SetRemoveValueImpl::SetRemoveValueImpl(Name const& aName)
: SetChangeImpl(aName)
, m_aOldValue()
, m_bRemove(true)
{
}
//-----------------------------------------------------------------------------

/// checks, if this represents an actual change
bool SetRemoveValueImpl::doIsChange(bool) const
{
    return m_bRemove;
}
//-----------------------------------------------------------------------------

bool SetRemoveValueImpl::doFillChange(DataChange& rChange) const
{
    rChange.oldValue = m_aOldValue;

    return rChange.isChange();
}
//-----------------------------------------------------------------------------

void SetRemoveValueImpl::doTestElement( SetNodeImpl& rNode, Name const& aName)
{
    OSL_ASSERT(m_bRemove); // test failed before ???

    Node* pNode = rNode.findElement(aName).pNode;
    OSL_ENSURE( pNode, "ERROR: Configuration: Removing a node that doesn't exist" );

    if (pNode)
    {
        OSL_ENSURE( pNode->isValueNode(), "ERROR: Configuration: Removing a non-value node as value node" );

        m_aOldValue = pNode->valueImpl().getValue();
        m_bRemove = true;
    }
    else
        m_bRemove = false;
}
//-----------------------------------------------------------------------------

void SetRemoveValueImpl::doApplyToElement( SetNodeImpl& rNode, Name const& aName)
{
    OSL_ASSERT(m_bRemove); // test failed before ???

    rNode.removeElement(aName);
}
*/
//-----------------------------------------------------------------------------
    }
}
