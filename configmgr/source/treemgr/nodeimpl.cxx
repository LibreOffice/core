/*************************************************************************
 *
 *  $RCSfile: nodeimpl.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-20 11:01:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc.,October,2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems,Inc.
 *  901 San Antonio Road,Palo Alto,CA 94303,USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1,as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not,write to the Free Software
 *  Foundation,Inc.,59 Temple Place,Suite 330,Boston,
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
 *  WITHOUT WARRANTY OF ANY KIND,EITHER EXPRESSED OR IMPLIED,INCLUDING,
 *  WITHOUT LIMITATION,WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE,FIT FOR A PARTICULAR PURPOSE,OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems,Inc.
 *
 *  Copyright: 2000 by Sun Microsystems,Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include "nodeimpl.hxx"

#include "valuenodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "setnodeimpl.hxx"

#include "treeimpl.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodechangeinfo.hxx"
#include "change.hxx"
#include "collectchanges.hxx"

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
    inline Attributes fetchAttributes(INode const& rNode)
    {
        return rNode.getAttributes();
    }

    struct GroupMemberDispatch : NodeAction
    {
        GroupMemberDispatch(GroupNodeImpl& rGroup, GroupMemberVisitor& rVisitor)
        : m_rGroup(rGroup)
        , m_rVisitor(rVisitor)
        , m_aResult(GroupMemberVisitor::CONTINUE)
        {}

        bool done() const { return m_aResult == GroupMemberVisitor::DONE; }

        bool test_value(INode const & rNode) const;

        GroupMemberVisitor::Result result() const { return m_aResult; }

        virtual void handle(ValueNode const& _rValue);
        virtual void handle(ISubtree const& _rSubtree);

        GroupNodeImpl&      m_rGroup;
        GroupMemberVisitor& m_rVisitor;

        GroupMemberVisitor::Result m_aResult;
    };

    bool GroupMemberDispatch::test_value(INode const& _rNode) const
    {
        Name aName = makeName( _rNode.getName(), Name::NoValidate() );

        return m_rGroup.hasValue( aName );
    }

    void GroupMemberDispatch::handle(ValueNode const& _rValue)
    {
        OSL_ENSURE( test_value(_rValue), "ERROR: GroupMemberDispatch:Did not find a ValueMember for a value child.");
        if ( !done() )
        {
            Name aValueName( makeNodeName(_rValue.getName(), Name::NoValidate()) );

            m_aResult = m_rVisitor.visit( m_rGroup.getValue(aValueName) );
        }
    }

    void GroupMemberDispatch::handle(ISubtree const& _rTree)
    {
        OSL_ENSURE( !test_value(_rTree), "ERROR: GroupMemberDispatch:Found a ValueMember for a subtree child.");
    }
}

//-----------------------------------------------------------------------------
// class NodeImpl
//-----------------------------------------------------------------------------

void NodeImpl::makeIndirect(NodeImplHolder& aThis,bool bIndirect)
{
    OSL_PRECOND(aThis.isValid(), "ERROR: Unexpected NULL node");

    if (aThis.isValid())
    {
        OSL_ENSURE(!aThis->doHasChanges(), "WARNING: Uncommitted changes while (possibly) changing node type - changes may be lost");
        NodeImplHolder aChanged = aThis->doCloneIndirect(bIndirect);
        aThis = aChanged;
    }
}

// helper for derived classes
//-----------------------------------------------------------------------------

void NodeImpl::addLocalChangeHelper( NodeChangesInformation& rLocalChanges_, NodeChange const& aChange_)
{
    NodeChangeInformation aThisInfo;
    if (aChange_.getChangeInfo(aThisInfo))
        rLocalChanges_.push_back( aThisInfo );
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

OUString GroupNodeImpl::getOriginalNodeName() const
{
    return m_rOriginal.getName();
}
//-----------------------------------------------------------------------------

bool GroupNodeImpl::hasValue(Name const& aName) const
{
    return this->getOriginalValueNode(aName) != NULL;
}
//-----------------------------------------------------------------------------

ValueNode* GroupNodeImpl::getOriginalValueNode(Name const& aName) const
{
    OSL_ENSURE( !aName.isEmpty(), "Cannot get nameless child value");
    INode* pChildNode = m_rOriginal.getChild(aName.toString());

    return pChildNode ? pChildNode->asValueNode() : NULL;
}
//-----------------------------------------------------------------------------

GroupMemberVisitor::Result GroupNodeImpl::dispatchToValues(GroupMemberVisitor& aVisitor)
{
    GroupMemberDispatch aDispatch(*this,aVisitor);

    aDispatch.applyToChildren( m_rOriginal );

    return aDispatch.result();
}
//-----------------------------------------------------------------------------

ValueMemberNode GroupNodeImpl::doGetValueMember(Name const& aName, bool )
{
    return ValueMemberNode( getOriginalValueNode(aName) );
}
//-----------------------------------------------------------------------------

NodeType::Enum  GroupNodeImpl::doGetType() const
{
    return NodeType::eGROUP;
}
//-----------------------------------------------------------------------------

Attributes GroupNodeImpl::doGetAttributes() const
{
    return fetchAttributes(m_rOriginal);
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doDispatch(INodeHandler& rHandler)
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
// class SetNodeImpl
//-----------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(ISubtree& rOriginal,Template* pTemplate)
: m_rOriginal(rOriginal)
,m_aTemplate(pTemplate)
,m_aTemplateProvider()
,m_pParentTree(0)
,m_nContextPos(0)
,m_aInit(0)
{
}
//-----------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(SetNodeImpl& rOriginal)
: m_rOriginal(rOriginal.m_rOriginal)
,m_aTemplate(rOriginal.m_aTemplate)
,m_aTemplateProvider(rOriginal.m_aTemplateProvider)
,m_pParentTree(rOriginal.m_pParentTree)
,m_nContextPos(rOriginal.m_nContextPos)
,m_aInit(rOriginal.m_aInit)
{
    // unbind the original
    rOriginal.m_aTemplate.unbind();
    rOriginal.m_aTemplateProvider = TemplateProvider();
    rOriginal.m_pParentTree = 0;
    rOriginal.m_nContextPos = 0;

}
//-----------------------------------------------------------------------------

SetNodeImpl::~SetNodeImpl()
{
}
//-----------------------------------------------------------------------------

OUString SetNodeImpl::getOriginalNodeName() const
{
    return m_rOriginal.getName();
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

bool SetNodeImpl::isEmpty()
{
    return !implLoadElements() || doIsEmpty();
}
//-----------------------------------------------------------------------------

SetEntry SetNodeImpl::findElement(Name const& aName)
{
    implEnsureElementsLoaded();
    return doFindElement(aName);
};
//-----------------------------------------------------------------------------

SetEntry SetNodeImpl::findAvailableElement(Name const& aName)
{
    if (implLoadElements())
        return doFindElement(aName);
    else
        return SetEntry(0);
};
//-----------------------------------------------------------------------------

void SetNodeImpl::insertElement(Name const& aName, SetEntry const& aNewEntry)
{
    // cannot insert, if we cannot check for collisions
    implEnsureElementsLoaded();
    doInsertElement(aName,aNewEntry);
}
//-----------------------------------------------------------------------------

void SetNodeImpl::removeElement(Name const& aName)
{
    // cannot remove, if we cannot check for existance
    implEnsureElementsLoaded();
    doRemoveElement(aName);
}
//-----------------------------------------------------------------------------

SetNodeVisitor::Result SetNodeImpl::dispatchToElements(SetNodeVisitor& aVisitor)
{
    if (implLoadElements())
        return doDispatchToElements(aVisitor);

    else
        return SetNodeVisitor::CONTINUE;
}
//-----------------------------------------------------------------------------

Attributes SetNodeImpl::doGetAttributes() const
{
    return fetchAttributes(m_rOriginal);
}
//-----------------------------------------------------------------------------

NodeType::Enum  SetNodeImpl::doGetType() const
{
    return NodeType::eSET;
}

//-----------------------------------------------------------------------------
void SetNodeImpl::doDispatch(INodeHandler& rHandler)
{
    rHandler.handle(*this);
}

//-----------------------------------------------------------------------------
bool SetNodeImpl::implHasLoadedElements() const
{
    return m_aInit == 0; // cannot check whether init was called though ...
}

//-----------------------------------------------------------------------------
bool SetNodeImpl::implLoadElements()
{
    if (m_aInit > 0)
    {
        implInitElements(m_aInit);
        m_aInit = 0;

    }
    OSL_ASSERT(implHasLoadedElements());

    return m_aInit == 0;
}

//-----------------------------------------------------------------------------
void SetNodeImpl::implEnsureElementsLoaded()
{
    if (!implLoadElements())
        throw ConstraintViolation("Trying to access set elements beyond the loaded nestíng level");
}

//-----------------------------------------------------------------------------
bool SetNodeImpl::implInitElements(InitHelper const& aInit)
{
    TreeDepth nDepth = aInit;
    if (nDepth > 0)
    {
        OSL_ENSURE(m_aTemplate.isEmpty() || m_aTemplate->isInstanceTypeKnown(),"ERROR: Need a type-validated template to fill a set");
        OSL_ENSURE(m_aTemplateProvider.isValid() || m_aTemplate->isInstanceValue(), "ERROR: Need a template provider to fill a non-primitive set");

        doInitElements(m_rOriginal,childDepth(nDepth));
        return true;
    }
    else
        return false;
}

//-----------------------------------------------------------------------------
void SetNodeImpl::initElements(TemplateProvider const& aTemplateProvider,TreeImpl& rParentTree,NodeOffset nPos,TreeDepth nDepth)
{
    OSL_ENSURE(m_pParentTree == 0 || m_pParentTree == &rParentTree, "WARNING: Set Node: Changing parent");
    OSL_ENSURE(m_nContextPos == 0 || m_nContextPos == nPos,         "WARNING: Set Node: Changing location within parent");
    m_pParentTree = &rParentTree;
    m_nContextPos = nPos;

    OSL_ENSURE(!m_aTemplateProvider.isValid() || !implHasLoadedElements(),"ERROR: Reinitializing set"); //doClearElements();
    OSL_ASSERT(doIsEmpty()); //doClearElements();

    OSL_ENSURE(m_aTemplate.isEmpty() || m_aTemplate->isInstanceTypeKnown(),"ERROR: Need a type-validated template to fill a set");
    OSL_ENSURE(aTemplateProvider.isValid() || nDepth == 0 || m_aTemplate->isInstanceValue(), "ERROR: Need a template provider to fill a non-primitive set");

    if (nDepth > 0) // dont set a template provider for zero-depth sets
    {
        m_aInit = nDepth;
        m_aTemplateProvider = aTemplateProvider;
    }
}

//-----------------------------------------------------------------------------
// class ValueElementNodeImpl
//-----------------------------------------------------------------------------

ValueElementNodeImpl::ValueElementNodeImpl(ValueNode& rOriginal)
: m_rOriginal(rOriginal)
{
}
//-----------------------------------------------------------------------------

ValueElementNodeImpl::ValueElementNodeImpl(ValueElementNodeImpl& rOriginal)
: m_rOriginal(rOriginal.m_rOriginal)
{
}
//-----------------------------------------------------------------------------

OUString ValueElementNodeImpl::getOriginalNodeName() const
{
    return m_rOriginal.getName();
}
//-----------------------------------------------------------------------------


UnoAny  ValueElementNodeImpl::getValue() const
{
    return m_rOriginal.getValue();
}
//-----------------------------------------------------------------------------

UnoType ValueElementNodeImpl::getValueType() const
{
    return m_rOriginal.getValueType();
}
//-----------------------------------------------------------------------------

Attributes ValueElementNodeImpl::doGetAttributes() const
{
    Attributes  aResult = fetchAttributes(m_rOriginal);
    aResult.bDefaultable = false;
    return aResult;
}
//-----------------------------------------------------------------------------

NodeType::Enum ValueElementNodeImpl::doGetType() const
{
    return NodeType::eVALUE;
}
//-----------------------------------------------------------------------------

void ValueElementNodeImpl::doDispatch(INodeHandler& rHandler)
{
    rHandler.handle(*this);
}
//-----------------------------------------------------------------------------

bool ValueElementNodeImpl::doHasChanges()   const
{
    return false; // this is an immutable object
}
//-----------------------------------------------------------------------------

void ValueElementNodeImpl::doMarkChanged()
{
    OSL_ENSURE(false,"WARNING: Cannot mark value element as changed");
}
//-----------------------------------------------------------------------------

void ValueElementNodeImpl::doCommitChanges()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Commit missing unexpected change of value element node");
}
//-----------------------------------------------------------------------------

void ValueElementNodeImpl::doCollectChangesWithTarget(NodeChanges& , TreeImpl* , NodeOffset ) const
{
    OSL_ENSURE(!hasChanges(),"ERROR: Collection of changes missing unexpected change of value element node");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// legacy commit
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> SetNodeImpl::preCommitChanges(ElementList& _rRemovedElements)
{
    // cannot have changes if elements not yet loaded
    if (implHasLoadedElements())
    {
        return doPreCommitChanges(_rRemovedElements);
    }
    else
    {
        OSL_ENSURE(!hasChanges(),"ERROR: Cannot have changes if elements haven't been loaded yet");
        return std::auto_ptr<SubtreeChange>();
    }
}
//-----------------------------------------------------------------------------

void SetNodeImpl::finishCommit(SubtreeChange& rChanges)
{
    // cannot have changes if elements not yet loaded
    OSL_ENSURE(implHasLoadedElements(),"ERROR: Cannot have provided changes to be finished - set not yet loaded");
    doFinishCommit(rChanges);
}
//-----------------------------------------------------------------------------

void SetNodeImpl::revertCommit(SubtreeChange& rChanges)
{
    // cannot have changes if elements not yet loaded
    OSL_ENSURE(implHasLoadedElements(),"ERROR: Cannot have provided changes to be reverted - set not yet loaded");
    doRevertCommit(rChanges);
}
//-----------------------------------------------------------------------------

void SetNodeImpl::failedCommit(SubtreeChange& rChanges)
{
    // cannot have changes if elements not yet loaded
    OSL_ENSURE(implHasLoadedElements(),"ERROR: Cannot have provided changes that failed - set not yet loaded");
    doFailedCommit(rChanges);
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> SetNodeImpl::doPreCommitChanges(ElementList& )
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<SubtreeChange>();
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doFinishCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChange.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doRevertCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChange.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doFailedCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChange.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not recovered");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const
{
    OSL_ENSURE(getParentTree()  == pParent, "Unexpected value for context tree parameter");
    OSL_ENSURE(getContextOffset() == nNode, "Unexpected value for context node parameter");

    doCollectChanges( rChanges );
}
//-----------------------------------------------------------------------------

void SetNodeImpl::adjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChange, TreeDepth nDepth)
{
    if (nDepth > 0)
    {
        OSL_ENSURE( m_aTemplateProvider.isValid(), "SetNodeImpl: Cannot adjust to changes - node was never initialized" );

        if (implHasLoadedElements())
        {
            doAdjustToChanges(rLocalChanges, rExternalChange, childDepth(nDepth));
        }
        else
        {
            OSL_ENSURE( !hasChanges(),"Cannot have changes to consider when no elements are loaded");

            implCollectChanges( rLocalChanges, rExternalChange, nDepth);
        }
    }
}
//-----------------------------------------------------------------------------

void SetNodeImpl::implCollectChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChange,
                                     TreeDepth nDepth)
{
    OSL_ASSERT(nDepth > 0);

    if (TreeImpl* pParentTree = this->getParentTree())
    {
        NodeOffset nNode = getContextOffset();

        OSL_ENSURE(pParentTree->isValidNode(nNode), "Invalid context node in Set");
        OSL_ENSURE(&pParentTree->node(nNode)->setImpl() == this, "Wrong context node in Set");

        CollectChanges aCollector(rLocalChanges, *pParentTree, nNode, getElementTemplate(), nDepth);

        aCollector.collectFrom(rExternalChange);
    }
    else
        OSL_ENSURE(false, "Missing context tree in Set");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> GroupNodeImpl::preCommitChanges()
{
    return doPreCommitChanges();
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> GroupNodeImpl::doPreCommitChanges()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<SubtreeChange>();
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doFinishCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doRevertCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doFailedCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not recovered");
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doCollectChangesWithTarget(NodeChanges& , TreeImpl* , NodeOffset ) const
{
//  OSL_ENSURE(!hasChanges(),"ERROR: Some Pending changes may be missed by collection");
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::adjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChanges, TreeImpl& rParentTree, NodeOffset nPos)
{
    for (SubtreeChange::ChildIterator it = rExternalChanges.begin(); it != rExternalChanges.end(); ++it)
    {
        if (it->ISA(ValueChange))
        {
            ValueChange const& rValueChange = static_cast<ValueChange const&>(*it);

            Name aValueName = makeNodeName( rValueChange.getNodeName(), Name::NoValidate() );

            if (ValueChangeImpl* pThisChange = doAdjustToValueChange(aValueName, rValueChange))
            {
                pThisChange->setTarget(&rParentTree,nPos,aValueName);
                addLocalChangeHelper(rLocalChanges, NodeChange(pThisChange));
            }
            else
                OSL_TRACE("WARNING: Configuration: derived class hides an external value member change from listeners");
        }
        else
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected change type within group");
    }
}
//-----------------------------------------------------------------------------

ValueChangeImpl* GroupNodeImpl::doAdjustToValueChange(Name const& aName, ValueChange const& rExternalChange)
{
    ValueChangeImpl* pChangeImpl = NULL;

    if (ValueNode* pLocalNode = getOriginalValueNode(aName))
    {
        switch( rExternalChange. getMode() )
        {
        case ValueChange::wasDefault:
        case ValueChange::changeValue:
        case ValueChange::typeIsAny:
            pChangeImpl = new ValueReplaceImpl( rExternalChange.getNewValue(), rExternalChange.getOldValue() );
            break;

            break;

        case ValueChange::setToDefault:
            pChangeImpl = new ValueResetImpl( rExternalChange.getNewValue(), rExternalChange.getOldValue() );
            break;

        default: OSL_ENSURE(false, "Unknown change mode");
            // fall thru to next case for somewhat meaningful return value
        case ValueChange::changeDefault:
            {
                UnoAny aLocalValue = pLocalNode->getValue();

                pChangeImpl = new ValueReplaceImpl( aLocalValue, aLocalValue );
            }
            break;
        }
        OSL_ASSERT( pChangeImpl );
    }
    else
    {
        OSL_ENSURE(false, "ERROR: Notification tries to change nonexistent value within group");
    }

    return pChangeImpl;
}
//-----------------------------------------------------------------------------

namespace
{
    struct AbstractNodeCast : INodeHandler
    {
        virtual void handle( ValueElementNodeImpl& rNode)
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
            OSL_ENSURE(m_pNode,"INTERNAL ERROR: Node not set after Cast." );
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
ValueElementNodeImpl&   AsValueNode(NodeImpl& rNode)
{
    return NodeCast<ValueElementNodeImpl>(rNode).get();
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
