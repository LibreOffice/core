/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodechangeimpl.cxx,v $
 * $Revision: 1.22 $
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

#include "nodechangeimpl.hxx"
#include "nodechangeinfo.hxx"
#include "nodeimpl.hxx"
#include "tree.hxx"
#include "configset.hxx"
#include "setnodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "valuenode.hxx"
#include "change.hxx"

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

view::ViewTreeAccess NodeChangeImpl::getTargetView()
{
    OSL_ENSURE( m_aAffectedTree.is(), "ERROR: Configuration Change: Target Tree Access has not been set up" );

    return view::ViewTreeAccess(m_aAffectedTree.get());
}
//-----------------------------------------------------------------------------

rtl::Reference<Tree> NodeChangeImpl::getTargetTree() const
{
    rtl::Reference<Tree> aRet = m_aAffectedTree;
    OSL_ENSURE( aRet.is(), "ERROR: Configuration Change: Target Tree has not been set up" );

    return aRet;
}
//-----------------------------------------------------------------------------

unsigned int NodeChangeImpl::getTargetNode() const
{
    unsigned int nRet = m_nAffectedNode;
    OSL_ENSURE( nRet != 0, "ERROR: Configuration Change: Target Node has not been set up" );
    OSL_ENSURE( m_aAffectedTree.is() && m_aAffectedTree->isValidNode(nRet),
                "ERROR: Configuration Change: Changing Node does not match tree" );

    return nRet;
}
//-----------------------------------------------------------------------------

void NodeChangeImpl::setTarget(view::Node _aAffectedNode)
{
    this->setTarget(_aAffectedNode.tree(), _aAffectedNode.get_offset());

}
void NodeChangeImpl::setTarget(rtl::Reference<Tree> const& _aAffectedTree, unsigned int _nAffectedNode)
{
    OSL_ENSURE(m_nState == 0 || (!m_aAffectedTree.is() && m_nState == eNoCheck), "WARNING: Configuration: Retargeting change that already was tested or applied");

    OSL_ENSURE( _aAffectedTree.is(), "ERROR: Configuration Change: NULL Target Tree is not allowed" );
    OSL_ENSURE( _nAffectedNode, "ERROR: Configuration Change: NULL Target Node is not allowed" );
    OSL_ENSURE( _aAffectedTree->isValidNode(_nAffectedNode), "ERROR: Configuration Change: Target Node does not match Tree" );

    if (m_nState != eNoCheck) m_nState = 0; // previous checks are invalidated

    m_aAffectedTree = _aAffectedTree;
    m_nAffectedNode = _nAffectedNode;
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::isChange(bool bAllowUntested) const
{
    OSL_ENSURE(bAllowUntested || (m_nState & eTestedChange), "WARNING: Configuration: Change was not tested  - isChange is meaningless");

    if (m_nState == eNoCheck)
        return true;

    if (!(m_nState & eTestedChange))
        return bAllowUntested;

    return doIsChange();
}
//-----------------------------------------------------------------------------

sal_uInt32 NodeChangeImpl::getChangeDataCount() const
{
    OSL_PRECOND(m_nState & eTestedChange, "WARNING: Configuration: Change was not tested  - change data count may be incorrect");

    return  doGetChangeCount();
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::fillChangeData(NodeChangeData& rChange, sal_uInt32 _ix) const
{
    OSL_PRECOND(_ix < doGetChangeCount(), "ERROR: Configuration: Change index out of range");
    OSL_PRECOND(m_nState & eTestedChange, "WARNING: Configuration: Change was not tested  - fillChange is partially meaningless");

    return doFillChange(rChange, _ix) || rChange.isDataChange(); // force true if the data is signaling change
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::fillChangeLocation(NodeChangeLocation& rChange, sal_uInt32 _ix) const
{
    if (!m_aAffectedTree.is()) return false;

    rChange.setBase( NodeID(this->getTargetTree().get(), this->getTargetNode()) );

    rChange.setAccessor( this->doGetChangingNodePath(_ix) );

    rChange.setAffected( NodeID(this->getTargetTree().get(), this->getTargetNode()) );

    rChange.setChangingSubnode( this->doIsChangingSubnode() );

    return true;
}
//-----------------------------------------------------------------------------

bool NodeChangeImpl::fillChangeInfo(NodeChangeInformation& rChange, sal_uInt32 _ix) const
{
    return fillChangeLocation(rChange.location, _ix) & fillChangeData(rChange.change, _ix);
}
//-----------------------------------------------------------------------------

void NodeChangeImpl::test()
{
    if (!(m_nState & eTestedChange))
    {
        doTest(implGetTarget());
        m_nState |= eTestedChange;
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

// default count is 1
sal_uInt32 NodeChangeImpl::doGetChangeCount() const
{
    return  1;
}
//-----------------------------------------------------------------------------

/// apply this change to the given node - start state is nState (which is then updated)
void NodeChangeImpl::implApply()
{
    OSL_ASSERT( !(m_nState & eAppliedChange)); // Caller must check

    view::Node aTarget = implGetTarget();

    if (!(m_nState & eTestedChange))  // Test checks the old value if there is realy a change
    {                               // for eventlisteners to say "the old value is kept"
        doTest(aTarget);
        m_nState |= eTestedChange;
    }

    doApply(aTarget);
    m_nState |= eAppliedChange;
}
//-----------------------------------------------------------------------------

view::Node NodeChangeImpl::implGetTarget()
{
    OSL_ENSURE(m_aAffectedTree.is(), "ERROR: Configuration Change: no target tree set");

    OSL_ENSURE(m_aAffectedTree->isValidNode(m_nAffectedNode), "ERROR: Configuration Change: target node not in target tree");

    view::Node aTarget = getTargetView().makeNode(m_nAffectedNode);
    OSL_ENSURE(aTarget.is(), "ERROR: Configuration: No target for change");
    return aTarget;
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

ValueChangeImpl::ValueChangeImpl(com::sun::star::uno::Any const& aNewValue)
: m_aNewValue(aNewValue)
, m_aOldValue()
{
}
//-----------------------------------------------------------------------------

ValueChangeImpl::ValueChangeImpl(com::sun::star::uno::Any const& aNewValue, com::sun::star::uno::Any const& aOldValue)
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

void ValueChangeImpl::setTarget(view::GroupNode const& _aParentNode, rtl::OUString const& sNodeName)
{
    OSL_ENSURE(sNodeName.getLength() != 0, "ValueChangeTarget is being set without a name");

    NodeChangeImpl::setTarget(_aParentNode.node());
    m_aName = sNodeName;
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::setTarget(rtl::Reference<Tree> const& aAffectedTree, unsigned int nParentNode, rtl::OUString const& sNodeName)
{
    OSL_ENSURE(sNodeName.getLength() != 0, "ValueChangeTarget is being set without a name");

    NodeChangeImpl::setTarget(aAffectedTree,nParentNode);
    m_aName = sNodeName;
}
//-----------------------------------------------------------------------------

RelativePath ValueChangeImpl::doGetChangingNodePath(sal_uInt32 ) const
{
    return RelativePath( Path::wrapSimpleName(m_aName) );
}
//-----------------------------------------------------------------------------

bool ValueChangeImpl::doIsChangingSubnode() const
{
    return m_aName.getLength() != 0;
}
//-----------------------------------------------------------------------------

bool ValueChangeImpl::doIsChange() const
{
    return !!(getNewValue() != getOldValue());
}
//-----------------------------------------------------------------------------

bool ValueChangeImpl::doFillChange(NodeChangeData& rChange, sal_uInt32) const
{
    rChange.unoData.newValue = getNewValue();
    rChange.unoData.oldValue = getOldValue();
    return rChange.unoData.isDataChange();
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::doTest( view::Node const& rTarget)
{
    view::ViewTreeAccess aTargetView = getTargetView();

    OSL_ENSURE(rTarget.isGroupNode(), "ERROR: Configuration: Target type mismatch: expected a group node holding the value");

    ValueMemberNode aValueTarget = aTargetView.getValue( view::GroupNode(rTarget), m_aName );

    OSL_ENSURE(aValueTarget.isValid(), "ERROR: Configuration: Target missing: could not find the changing value");

    preCheckValue(aValueTarget, m_aOldValue, m_aNewValue);
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::doApply( view::Node const& rTarget)
{
    view::ViewTreeAccess aTargetView = getTargetView();

    OSL_ENSURE(rTarget.isGroupNode(), "ERROR: Configuration: Target type mismatch: expected a group node holding the value");

    ValueMemberUpdate aValueTarget = aTargetView.getValueForUpdate( view::GroupNode(rTarget), m_aName );

    OSL_ENSURE(aValueTarget.isValid(), "ERROR: Configuration: Target missing: could not find the changing value");

    doApplyChange(aValueTarget);
    configmgr::configuration::ValueMemberNode aNode(aValueTarget.getNode());
    postCheckValue(aNode, m_aNewValue); // Sideeffect: m_aNewValue will be changed
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::preCheckValue(ValueMemberNode& rNode, com::sun::star::uno::Any& rOld, com::sun::star::uno::Any& )
{
    com::sun::star::uno::Any aPrevValue = rNode.getValue();
    OSL_ENSURE(!rOld.hasValue() || rOld == aPrevValue, "ERROR: Configuration: Stored old value of target does not match the actual value");
    rOld = aPrevValue;
}
//-----------------------------------------------------------------------------

void ValueChangeImpl::postCheckValue(ValueMemberNode& rNode, com::sun::star::uno::Any& rNew)
{
    com::sun::star::uno::Any aResultValue = rNode.getValue();
    OSL_ENSURE(!rNew.hasValue() || rNew == aResultValue, "ERROR: Configuration: New value of target does not match the predicted result");
    rNew = aResultValue;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Value operations: ValueReplaceImpl = set local value
//-----------------------------------------------------------------------------

ValueReplaceImpl::ValueReplaceImpl(com::sun::star::uno::Any const& aNewValue)
:ValueChangeImpl(aNewValue)
{
}
//-----------------------------------------------------------------------------

ValueReplaceImpl::ValueReplaceImpl(com::sun::star::uno::Any const& aNewValue, com::sun::star::uno::Any const& aOldValue)
:ValueChangeImpl(aNewValue, aOldValue)
{
}
//-----------------------------------------------------------------------------

void ValueReplaceImpl::doApplyChange( ValueMemberUpdate& rNode)
{
    rNode.setValue(getNewValue());
}
//-----------------------------------------------------------------------------

bool ValueReplaceImpl::doFillChange( NodeChangeData& rChange, sal_uInt32 _ix) const
{
    rChange.type = NodeChangeData::eSetValue;
    return ValueChangeImpl::doFillChange(rChange, _ix);
}

//-----------------------------------------------------------------------------
// Value operations: ValueResetImpl = set to default
//-----------------------------------------------------------------------------

ValueResetImpl::ValueResetImpl()
:ValueChangeImpl()
, m_bTargetIsDefault(false)
{
}
//-----------------------------------------------------------------------------

ValueResetImpl::ValueResetImpl(com::sun::star::uno::Any const& aNewValue, com::sun::star::uno::Any const& aOldValue)
:ValueChangeImpl(aNewValue, aOldValue)
, m_bTargetIsDefault(false)
{
}
//-----------------------------------------------------------------------------

void ValueResetImpl::doApplyChange( ValueMemberUpdate& rNode)
{
    rNode.setDefault();
}
//-----------------------------------------------------------------------------

bool ValueResetImpl::doIsChange() const
{
    return !m_bTargetIsDefault;
}
//-----------------------------------------------------------------------------

bool ValueResetImpl::doFillChange( NodeChangeData& rChange, sal_uInt32 _ix) const
{
    rChange.type = NodeChangeData::eSetDefault;
    ValueChangeImpl::doFillChange(rChange,_ix);
    return !m_bTargetIsDefault && !rChange.isEmptyChange(); // do it defensively here - default (= 'new') may be unknown still
}
//-----------------------------------------------------------------------------

void ValueResetImpl::preCheckValue(ValueMemberNode& rNode, com::sun::star::uno::Any& rOld, com::sun::star::uno::Any& rNew)
{
    ValueChangeImpl::preCheckValue(rNode,rOld,rNew);

    com::sun::star::uno::Any aDefaultValue = rNode.getDefaultValue();
    OSL_ENSURE(!rNew.hasValue() || rNew == aDefaultValue, "ERROR: Configuration: Stored new value of target does not match the actual default value");
    rNew = aDefaultValue;
    m_bTargetIsDefault = rNode.isDefault();
}

//-----------------------------------------------------------------------------
// All Set Changes: SetChangeImpl - common base
//-----------------------------------------------------------------------------

SetChangeImpl::SetChangeImpl(bool bNoCheck)
: NodeChangeImpl(bNoCheck)
{
}
//-----------------------------------------------------------------------------

bool SetChangeImpl::doIsChangingSubnode() const
{
    return false;
}

//-----------------------------------------------------------------------------
// Resetting a set to its default state
//-----------------------------------------------------------------------------

SetResetImpl::SetResetImpl(
    SetElementFactory& _rElementFactory,
    std::auto_ptr<ISubtree> _pDefaultData,
    bool _bNoCheck
)
: SetChangeImpl(_bNoCheck)
, m_aDefaultData(_pDefaultData)
, m_rElementFactory(_rElementFactory)
, m_aTreeChanges()
{
}
//-----------------------------------------------------------------------------

SetResetImpl::~SetResetImpl()
{
}
//-----------------------------------------------------------------------------

RelativePath SetResetImpl::doGetChangingNodePath(sal_uInt32 _ix) const
{
    OSL_ENSURE( _ix < m_aTreeChanges.size() || _ix == scCommonBase, "Illegal Change index" );
    OSL_ASSERT( static_cast<size_t>(scCommonBase) > m_aTreeChanges.size() );

    if ( _ix < m_aTreeChanges.size() )
        return RelativePath( m_aTreeChanges[_ix].m_aElementName);

    else
        return RelativePath();
}
//-----------------------------------------------------------------------------

static NodeChangeData::Type getChangeType(ElementTreeChange const& aChange)
{
    sal_Bool bHasNew = aChange.m_aAddedElement.is();
    sal_Bool bHasOld = aChange.m_aRemovedElement.is();

    NodeChangeData::Type aResult;
     if (bHasNew)
        aResult = bHasOld ? NodeChangeData::eReplaceElement : NodeChangeData::eInsertElement;
    else
        aResult = bHasOld ? NodeChangeData::eRemoveElement : NodeChangeData::eSetDefault;

    return aResult;
}
//-----------------------------------------------------------------------------

bool SetResetImpl::doIsChange() const
{
    return !m_aTreeChanges.empty() || m_aDefaultData.get();
}
//-----------------------------------------------------------------------------

bool SetResetImpl::doFillChange(NodeChangeData& rChange, sal_uInt32 _ix) const
{
    OSL_ENSURE( _ix < m_aTreeChanges.size() || _ix == scCommonBase, "Illegal Change index" );
    if (_ix >= m_aTreeChanges.size())
    {
        rChange.type = NodeChangeData::eResetSetDefault;
        return m_aDefaultData.get() != NULL;
    }
    ElementTreeChange const& aChange = m_aTreeChanges[_ix];

    rChange.type = getChangeType(aChange);

    rChange.element.newValue =  aChange.m_aAddedElement;
    rChange.element.oldValue =  aChange.m_aRemovedElement;

    return true;
}
//-----------------------------------------------------------------------------

void SetResetImpl::doTest( view::Node const& rTarget)
{
    if ( m_aDefaultData.get() )
    {
        view::ViewTreeAccess accessor = this->getTargetView();

        view::SetNode aTargetSet(rTarget);

        std::auto_ptr<SubtreeChange> pChanges = accessor.differenceToDefaultState(aTargetSet, *m_aDefaultData);

        if (pChanges.get())
        {
            for (SubtreeChange::MutatingChildIterator   it = pChanges->begin_changes(),
                                                        stop = pChanges->end_changes();
                 it != stop;
                 ++it)
            {
                rtl::OUString aName(it->getNodeName());

                SetEntry anExistingEntry = accessor.findElement(aTargetSet,aName);

                rtl::Reference<ElementTree> aOldTree = anExistingEntry.tree();
                rtl::Reference<ElementTree> aNewTree;

                if (AddNode * addNode = dynamic_cast< AddNode * >(&*it))
                {
                    rtl::Reference< data::TreeSegment > pAddedNode = addNode->getNewTree();

                    OSL_ENSURE(pAddedNode.is(), "Processing an addNode to default - no node to add");

                    aNewTree = m_rElementFactory.instantiateOnDefault(pAddedNode,accessor.getElementTemplate(aTargetSet)).get();
                }


                Path::Component aFullName =
                                    aNewTree.is() ? aNewTree->getExtendedRootName() :
                                    aOldTree.is() ? aOldTree->getExtendedRootName() :
                                    Path::makeCompositeName(aName,accessor.getElementTemplate(aTargetSet)->getName());

                OSL_ENSURE(aOldTree.is() || aNewTree.is(), "No data for change to default");

                m_aTreeChanges.push_back(ElementTreeChange(aFullName,aNewTree,aOldTree));
            }
        }

        m_aDefaultData.reset();
    }
}
//-----------------------------------------------------------------------------

void SetResetImpl::doApply( view::Node const& rTarget)
{
    view::ViewTreeAccess accessor = this->getTargetView();

    view::SetNode aTargetSet(rTarget);

    for (std::vector< ElementTreeChange >::iterator it = m_aTreeChanges.begin(); it != m_aTreeChanges.end(); ++it)
    {
        rtl::OUString aElementName = it->m_aElementName.getName();

        if (it->m_aRemovedElement.is())
            accessor.removeElement(aTargetSet, aElementName);

        if (it->m_aAddedElement.is())
        {
            SetEntry aNewEntry( it->m_aAddedElement.get() );
            accessor.insertElement(aTargetSet, aElementName, aNewEntry);
        }

        OSL_ENSURE(getChangeType(*it) != NodeChangeData::eSetDefault,
                    "Cannot apply change without data");
    }
}

//-----------------------------------------------------------------------------
// All Set Changes affecting a single element: SetElementChangeImpl - common base
//-----------------------------------------------------------------------------

SetElementChangeImpl::SetElementChangeImpl(Path::Component const& aName, bool bNoCheck)
: SetChangeImpl(bNoCheck)
, m_aName(aName)
{
}
//-----------------------------------------------------------------------------

RelativePath SetElementChangeImpl::doGetChangingNodePath(sal_uInt32 ) const
{
    return RelativePath(getFullElementName());
}
//-----------------------------------------------------------------------------

void SetElementChangeImpl::doTest( view::Node const& rTarget)
{
    doTestElement(view::SetNode(rTarget), getElementName() );
}
//-----------------------------------------------------------------------------

void SetElementChangeImpl::doApply( view::Node const& rTarget)
{
    doApplyToElement(view::SetNode(rTarget), getElementName() );
}

//-----------------------------------------------------------------------------
// Full Sets: SetInsertTreeImpl
//-----------------------------------------------------------------------------

SetInsertImpl::SetInsertImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aNewTree, bool bNoCheck)
: SetElementChangeImpl(aName,bNoCheck)
, m_aNewTree(aNewTree)
{
}
//-----------------------------------------------------------------------------

bool SetInsertImpl::doIsChange() const
{
    return !!m_aNewTree.is();
}
//-----------------------------------------------------------------------------

bool SetInsertImpl::doFillChange(NodeChangeData& rChange, sal_uInt32) const
{
    rChange.type = NodeChangeData::eInsertElement;
    if (m_aNewTree.is())
        rChange.element.newValue = m_aNewTree;

    return isChange(true);
}
//-----------------------------------------------------------------------------

void SetInsertImpl::doTestElement( view::SetNode const& _aNode, rtl::OUString const& aName)
{
    SetEntry anEntry = getTargetView().findElement(_aNode,aName); // require loaded children
    OSL_ENSURE(!anEntry.isValid(), "ERROR: Configuration: Adding a node that already exists");
}
//-----------------------------------------------------------------------------

void SetInsertImpl::doApplyToElement( view::SetNode const& _aNode, rtl::OUString const& aName)
{
    if (m_aNewTree.is())
    {
        SetEntry aNewEntry( m_aNewTree.get() );
        getTargetView().insertElement( _aNode, aName, aNewEntry);
    }
}

//-----------------------------------------------------------------------------
// Full Sets: SetReplaceTreeImpl
//-----------------------------------------------------------------------------

SetReplaceImpl::SetReplaceImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aNewTree)
: SetElementChangeImpl(aName)
, m_aNewTree(aNewTree)
, m_aOldTree()
{
}
//-----------------------------------------------------------------------------

SetReplaceImpl::SetReplaceImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aNewTree, rtl::Reference<ElementTree> const& aOldTree)
: SetElementChangeImpl(aName,true)
, m_aNewTree(aNewTree)
, m_aOldTree(aOldTree)
{
}
//-----------------------------------------------------------------------------

/// checks, if this represents an actual change
bool SetReplaceImpl::doIsChange() const
{
    return !(m_aOldTree == m_aNewTree);
}
//-----------------------------------------------------------------------------

/// fills in pre- and post-change values, returns wether they differ
bool SetReplaceImpl::doFillChange(NodeChangeData& rChange, sal_uInt32) const
{
    rChange.type = NodeChangeData::eReplaceElement;
    if (m_aNewTree.is())
        rChange.element.newValue =  m_aNewTree;

    if (m_aOldTree.is())
        rChange.element.oldValue = m_aOldTree;

    return isChange(true);
}
//-----------------------------------------------------------------------------

void SetReplaceImpl::doTestElement( view::SetNode const& _aNode, rtl::OUString const& aName)
{
    OSL_ASSERT(!m_aOldTree.is()); // already tested ?

    // remove the old node
    SetEntry anEntry = getTargetView().findElement(_aNode,aName); // require loaded children
    OSL_ENSURE(anEntry.isValid(), "ERROR: Configuration: Replacing a node that doesn't exist");

    m_aOldTree = anEntry.tree();
}
//-----------------------------------------------------------------------------

void SetReplaceImpl::doApplyToElement( view::SetNode const& _aNode, rtl::OUString const& aName)
{
    if (m_aOldTree != m_aNewTree)
    {
        view::ViewTreeAccess aTargetView = this->getTargetView();

        OSL_ENSURE(m_aOldTree.is(), "ERROR: Configuration: Replacing a node that doesn't exist");
        aTargetView.removeElement(_aNode, aName);

    // add the new one
        OSL_ENSURE(m_aNewTree.is(), "ERROR: Configuration: Replacing a node with nothing");
        if (m_aNewTree.is())
        {
            SetEntry aNewEntry( m_aNewTree.get() );
            aTargetView.insertElement( _aNode, aName, aNewEntry);
        }
    }
}

//-----------------------------------------------------------------------------
// Full Sets: SetRemoveTreeImpl
//-----------------------------------------------------------------------------

SetRemoveImpl::SetRemoveImpl(Path::Component const& aName)
: SetElementChangeImpl(aName)
, m_aOldTree()
{
}
//-----------------------------------------------------------------------------

SetRemoveImpl::SetRemoveImpl(Path::Component const& aName, rtl::Reference<ElementTree> const& aOldTree)
: SetElementChangeImpl(aName,true)
, m_aOldTree(aOldTree)
{
}
//-----------------------------------------------------------------------------

/// checks, if this represents an actual change
bool SetRemoveImpl::doIsChange() const
{
    return !!m_aOldTree.is();
}
//-----------------------------------------------------------------------------

/// fills in pre- and post-change values, returns wether they differ
bool SetRemoveImpl::doFillChange(NodeChangeData& rChange, sal_uInt32) const
{
    rChange.type = NodeChangeData::eRemoveElement;
    if (m_aOldTree.is())
        rChange.element.oldValue = m_aOldTree;

    return isChange(true);
}
//-----------------------------------------------------------------------------

void SetRemoveImpl::doTestElement( view::SetNode const& _aNode, rtl::OUString const& aName)
{
    OSL_ASSERT(!m_aOldTree.is()); // already tested ?

    // remove the old node
    SetEntry anEntry = getTargetView().findElement(_aNode,aName); // require loaded children
    OSL_ENSURE(anEntry.isValid(), "ERROR: Configuration: Removing a node that doesn't exist");

    m_aOldTree = anEntry.tree();
}
//-----------------------------------------------------------------------------

void SetRemoveImpl::doApplyToElement( view::SetNode const& _aNode, rtl::OUString const& aName)
{
    getTargetView().removeElement(_aNode, aName);
}

//-----------------------------------------------------------------------------
    }
}
