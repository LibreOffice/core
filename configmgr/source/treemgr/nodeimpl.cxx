/*************************************************************************
 *
 *  $RCSfile: nodeimpl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:20:54 $
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
    inline void fillInfo(NodeInfo& rInfo,OUString const& sName, Attributes const& aAttributes)
    {
        rInfo.aName = Name(sName,Name::NoValidate());
        rInfo.aAttributes = aAttributes;
    }
    inline void fetchInfo(NodeInfo& rInfo,INode const& rNode)
    {
        fillInfo(rInfo,rNode.getName(),rNode.getAttributes());
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

NodeType::Enum  GroupNodeImpl::doGetType() const
{
    return NodeType::eGROUP;
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doGetNodeInfo(NodeInfo& rInfo) const
{
    fetchInfo(rInfo,m_rOriginal);
}
//-----------------------------------------------------------------------------

void GroupNodeImpl::doSetNodeName(Name const& aName)
{
    m_rOriginal.setName(aName.toString());
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
// struct SetNodeImpl::InitHelper
//-----------------------------------------------------------------------------

struct SetNodeImpl::InitHelper
{
    TemplateProvider    aTemplateProvider;
    TreeDepth           nLoadDepth;

    InitHelper()
        : aTemplateProvider()
        , nLoadDepth(0)
    {
    }

    InitHelper(TemplateProvider const& aTP_, TreeDepth nDepth_)
        : aTemplateProvider(aTP_)
        , nLoadDepth(nDepth_)
    {
        OSL_ASSERT(nDepth_ > 0 || !aTP_.isValid());
    }
};

//-----------------------------------------------------------------------------
// class SetNodeImpl
//-----------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(ISubtree& rOriginal,Template* pTemplate)
: m_rOriginal(rOriginal)
,m_aTemplate(pTemplate)
,m_pParentTree(0)
,m_nContextPos(0)
,m_pInit(new InitHelper())
{
}
//-----------------------------------------------------------------------------

SetNodeImpl::SetNodeImpl(SetNodeImpl& rOriginal)
: m_rOriginal(rOriginal.m_rOriginal)
,m_aTemplate(rOriginal.m_aTemplate)
,m_pParentTree(rOriginal.m_pParentTree)
,m_nContextPos(rOriginal.m_nContextPos)
,m_pInit(rOriginal.m_pInit)
{
    // unbind the original
    rOriginal.m_aTemplate.unbind();
    rOriginal.m_pParentTree = 0;
    rOriginal.m_nContextPos = 0;

}
//-----------------------------------------------------------------------------

SetNodeImpl::~SetNodeImpl()
{
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

void SetNodeImpl::doGetNodeInfo(NodeInfo& rInfo) const
{
    fetchInfo(rInfo,m_rOriginal);
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doSetNodeName(Name const& aName)
{
    m_rOriginal.setName(aName.toString());
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
    return m_pInit.get() == 0; // cannot check whether init was called though ...
}

//-----------------------------------------------------------------------------
bool SetNodeImpl::implLoadElements()
{
    if (m_pInit.get() != 0 && m_pInit->nLoadDepth > 0)
    {
        implInitElements(*m_pInit);
        m_pInit.reset();

    }
    OSL_ASSERT(implHasLoadedElements() || m_pInit->nLoadDepth == 0);

    return m_pInit.get() == 0;
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
    TreeDepth nDepth = aInit.nLoadDepth;
    if (nDepth > 0)
    {
        OSL_ENSURE(m_aTemplate.isEmpty() || m_aTemplate->isInstanceTypeKnown(),"ERROR: Need a type-validated template to fill a set");
        OSL_ENSURE(aInit.aTemplateProvider.isValid() || m_aTemplate->isInstanceValue(), "ERROR: Need a template provider to fill a non-primitive set");

        doInitElements(aInit.aTemplateProvider,m_rOriginal,childDepth(nDepth));
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

    OSL_ENSURE(!implHasLoadedElements(),"ERROR: Reinitializing set"); //doClearElements();
    OSL_ASSERT(doIsEmpty()); //doClearElements();

    if (nDepth > 0)
        m_pInit.reset( new InitHelper(aTemplateProvider,nDepth) );
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

void ValueNodeImpl::doGetNodeInfo(NodeInfo& rInfo) const
{
    fetchInfo(rInfo,m_rOriginal);
    rInfo.aAttributes.bDefaultable = m_rOriginal.hasDefault();
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::doSetNodeName(Name const& aName)
{
    m_rOriginal.setName(aName.toString());
}
//-----------------------------------------------------------------------------

NodeType::Enum ValueNodeImpl::doGetType() const
{
    return NodeType::eVALUE;
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::doDispatch(INodeHandler& rHandler)
{
    rHandler.handle(*this);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// legacy commit
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> SetNodeImpl::preCommitChanges()
{
    // cannot have changes if elements not yet loaded
    if (implHasLoadedElements())
    {
        return doPreCommitChanges();
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

std::auto_ptr<SubtreeChange> SetNodeImpl::doPreCommitChanges()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<SubtreeChange>();
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doFinishCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getChildTemplateName() ==  getElementTemplate()->getPath().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doRevertCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getChildTemplateName() ==  getElementTemplate()->getPath().toString(),
                "ERROR: Element template of change does not match the template of the set");

    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

void SetNodeImpl::doFailedCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(rChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChange.getChildTemplateName() ==  getElementTemplate()->getPath().toString(),
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

void SetNodeImpl::adjustToChanges(NodeChangesInformation& rLocalChanges, SubtreeChange const& rExternalChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth)
{
    if (nDepth > 0)
    {
        OSL_ASSERT( aTemplateProvider.isValid() );

        if (implHasLoadedElements())
        {
            doAdjustToChanges(rLocalChanges, rExternalChange, aTemplateProvider, childDepth(nDepth));
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

        CollectChanges aCollector(rLocalChanges, *pParentTree, nNode, nDepth);

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
    OSL_ENSURE(!hasChanges(),"ERROR: Some Pending changes may be missed by collection");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::auto_ptr<ValueChange> ValueNodeImpl::preCommitChange()
{
    return doPreCommitChange();
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueChange> ValueNodeImpl::doPreCommitChange()
{
    OSL_ENSURE(!hasChanges(),"ERROR: Committing to an old changes tree is not supported on this node");
    return std::auto_ptr<ValueChange>();
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::doFinishCommit(ValueChange& )
{
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes are lost");
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::doRevertCommit(ValueChange& )
{
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not restored");
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::doFailedCommit(ValueChange& )
{
    OSL_ENSURE(!hasChanges(),"ERROR: Old-style commit not supported: changes not recovered");
}
//-----------------------------------------------------------------------------

void ValueNodeImpl::doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParent, NodeOffset nNode) const
{
    if (NodeChangeImpl* pThisChange = doCollectChange())
    {
        pThisChange->setTarget(pParent,nNode);

        rChanges.add( NodeChange(pThisChange) );
    }
}
//-----------------------------------------------------------------------------

NodeChangeImpl* ValueNodeImpl::doCollectChange() const
{
    OSL_ENSURE(!hasChanges(),"ERROR: Some Pending changes missed by collection");
    return 0;
}
//-----------------------------------------------------------------------------


void ValueNodeImpl::adjustToChange(NodeChangesInformation& rLocalChanges, ValueChange const& rExternalChange, TreeImpl& rParentTree, NodeOffset nPos)
{
    if (NodeChangeImpl* pThisChange = doAdjustToChange(rExternalChange))
    {
        pThisChange->setTarget(&rParentTree,nPos);
        addLocalChangeHelper(rLocalChanges, NodeChange(pThisChange));
    }
    else
        OSL_TRACE("WARNING: Configuration: derived class hides an external value change from listeners");
}
//-----------------------------------------------------------------------------
NodeChangeImpl* ValueNodeImpl::doAdjustToChange(ValueChange const& rExternalChange)
{
    // convert rExternalChange exactly to a NodeChange
    ValueChangeImpl* pChangeImpl = 0;

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
        pChangeImpl = new ValueReplaceImpl( getValue(), getValue() );
        break;
    }
    OSL_ASSERT( pChangeImpl );

    return pChangeImpl;
}
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
