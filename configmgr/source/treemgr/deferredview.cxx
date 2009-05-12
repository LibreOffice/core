/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: deferredview.cxx,v $
 * $Revision: 1.6 $
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
#include <stdio.h>
#include "deferredview.hxx"
#include "viewfactory.hxx"
#include "nodeimplobj.hxx"

namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------
static inline configuration::DeferredGroupNodeImpl* deferredGroupNode(Node const& _aNode)
{
    return static_cast<configuration::DeferredGroupNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline configuration::DeferredGroupNodeImpl* deferredGroupNode(GroupNode const& _aNode)
{
    return static_cast<configuration::DeferredGroupNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline configuration::DeferredSetNodeImpl* deferredSetNode(Node const& _aNode)
{
    return static_cast<configuration::DeferredSetNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline configuration::DeferredSetNodeImpl* deferredSetNode(SetNode const& _aNode)
{
    return static_cast<configuration::DeferredSetNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline void deferredValueNode(Node const& _aNode)
{
    { (void)_aNode; }
    OSL_ENSURE( _aNode.isValueNode(),"Unknown Node type in deferred view");
    OSL_ENSURE(_aNode.get_offset() == configuration::Tree::ROOT, "Tree: Unexpected node type - non-root value element");
}
//-----------------------------------------------------------------------------

static void deferredValueNodeChanged(Node const& _aNode)
{
    { (void)_aNode; }
    OSL_ENSURE( _aNode.isValueNode(),"Unknown Node type in deferred view");
    OSL_ENSURE(_aNode.get_offset() == configuration::Tree::ROOT, "Tree: Unexpected node type - non-root value element");
    OSL_ENSURE(!_aNode.isValueNode(),"Value node changes not supported in deferred view");
    throw configuration::Exception("Internal Error: Invalid operation applied to element");
}
//-----------------------------------------------------------------------------

bool DeferredViewStrategy::doHasChanges(Node const& _aNode) const
{
    if (_aNode.isGroupNode())
        return deferredGroupNode(_aNode)->hasChanges();

    else if (_aNode.isSetNode())
        return deferredSetNode(_aNode)->hasChanges();

    else
       deferredValueNode(_aNode);

    return false;
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doMarkChanged(Node const& _aNode)
{
    if (_aNode.isGroupNode())
        deferredGroupNode(_aNode)->markChanged();

    else if (_aNode.isSetNode())
        deferredSetNode(_aNode)->markChanged();

    else
       deferredValueNodeChanged(_aNode);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doCollectChanges(Node const& _aNode, configuration::NodeChanges& _rChanges) const
{
    implCollectChangesIn(_aNode,_rChanges);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::implCollectChangesIn(Node const& _aNode, configuration::NodeChanges& _rChanges)  const
{
    if ( hasChanges(_aNode) )
    {
        if (_aNode.isSetNode())
        {
            deferredSetNode(_aNode)->collectElementChanges( _rChanges);
        }
        else if (_aNode.isGroupNode())
        {
            GroupNode aGroup(_aNode);

            deferredGroupNode(aGroup)->collectValueChanges( _rChanges, _aNode.tree(), _aNode.get_offset());

            for( Node aChild = aGroup.getFirstChild();
                 aChild.is();
                 aChild = aGroup.getNextChild(aChild)
               )
            {
                this->implCollectChangesIn( aChild, _rChanges );
            }
        }
        else
            OSL_ASSERT(!"Unreachable code");
    }
}

//-----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> DeferredViewStrategy::doPreCommitChanges(configuration::Tree * tree, std::vector< rtl::Reference<configuration::ElementTree> >& _rRemovedElements)
{
    std::auto_ptr<SubtreeChange> pRet;
    if (hasChanges(tree))
    {
        pRet = implPreCommitChanges(getRootNode(tree),_rRemovedElements);
        if (pRet.get() != NULL)
        {
            pRet->setNodeName(getSimpleRootName(tree));
        }
    }
    return pRet;
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doFailedCommit(configuration::Tree * tree, SubtreeChange& rChanges)
{
    implFailedCommit(getRootNode(tree),rChanges);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doFinishCommit(configuration::Tree * tree, SubtreeChange& rChanges)
{
    implFinishCommit(getRootNode(tree),rChanges);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doRevertCommit(configuration::Tree * tree, SubtreeChange& rChanges)
{
    implRevertCommit(getRootNode(tree),rChanges);
}

//-----------------------------------------------------------------------------
configuration::ValueChangeImpl* DeferredViewStrategy::doAdjustToValueChange(GroupNode const& _aGroupNode, rtl::OUString const& _aName, ValueChange const& rExternalChange)
{
    rtl::Reference<configuration::ValueMemberNode::DeferredImpl> aChange = deferredGroupNode(_aGroupNode)->findValueChange(_aName);

    if (aChange.is())
    {
        if (configuration::ValueChangeImpl* pValueChange = aChange->adjustToChange(rExternalChange))
        {
            OSL_ENSURE(aChange->isChange(), "Got an adjusted change from a non-changing value");

            return pValueChange;
        }
        else // a non-change
        {
            OSL_ENSURE(!aChange->isChange(), "Got no adjusted change from a changing value") ;
            OSL_ENSURE(false, "Got a non-change from a (deferred) group") ;
            // then do as without deferred change
        }
    }

    return ViewStrategy::doAdjustToValueChange(_aGroupNode, _aName, rExternalChange);

}

//-----------------------------------------------------------------------------
node::Attributes DeferredViewStrategy::doAdjustAttributes(node::Attributes const& _aAttributes) const
{
    return _aAttributes;
}

//-----------------------------------------------------------------------------
configuration::ValueMemberNode DeferredViewStrategy::doGetValueMember(GroupNode const& _aNode, rtl::OUString const& _aName, bool _bForUpdate) const
{
    return deferredGroupNode(_aNode)->makeValueMember(_aName,_bForUpdate);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doInsertElement(SetNode const& _aNode, rtl::OUString const& aName, configuration::SetEntry const& _aNewEntry)
{
    // move to this memory segment
    // should be direct (as any free-floating one)

    //implMakeElement(aNewEntry)
    configuration::ElementTreeData aNewElement = implMakeElement(_aNode, _aNewEntry );

    deferredSetNode(_aNode)->insertNewElement(aName, aNewElement);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doRemoveElement(SetNode const& _aNode, rtl::OUString const& aName)
{
    deferredSetNode(_aNode)->removeOldElement(aName);
}

//-----------------------------------------------------------------------------
extern NodeFactory& getDeferredChangeFactory();

NodeFactory& DeferredViewStrategy::doGetNodeFactory()
{
    return getDeferredChangeFactory();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

rtl::Reference<ViewStrategy> createDeferredChangeStrategy()
{
    return new DeferredViewStrategy();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredViewStrategy::implPreCommitChanges(Node const& _aNode, std::vector< rtl::Reference<configuration::ElementTree> >& _rRemovedElements)
{
    std::auto_ptr<SubtreeChange> aRet;

    OSL_ASSERT (this->hasChanges( _aNode) );

    if (_aNode.isSetNode())
    {
        aRet = deferredSetNode(_aNode)->preCommitChanges(_rRemovedElements);
    }
    else if (_aNode.isGroupNode())
    {
        std::auto_ptr<SubtreeChange> aGroupChange(deferredGroupNode(_aNode)->preCommitValueChanges());

        OSL_ASSERT(aGroupChange.get());
        if (aGroupChange.get())
            implPreCommitSubChanges( GroupNode(_aNode), _rRemovedElements, *aGroupChange);

        aRet = aGroupChange;
    }
    else
       deferredValueNode(_aNode);

    return aRet;
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implFinishCommit(Node const& _aNode, SubtreeChange& rSubtreeChange)
{
    OSL_PRECOND( getSimpleNodeName(_aNode) == rSubtreeChange.getNodeName(), "ERROR: Change name does not match node");

    if (_aNode.isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        deferredSetNode(_aNode)->finishCommit(rSubtreeChange);
    }
    else if (_aNode.isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        deferredGroupNode(_aNode)->finishCommit(rSubtreeChange);
        implFinishSubCommitted( GroupNode(_aNode), rSubtreeChange );
    }
    else
    {
        deferredValueNode(_aNode);
        OSL_ENSURE(false, "Tree: Cannot finish commit: Unexpected node type");
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implRevertCommit(Node const& _aNode, SubtreeChange& rSubtreeChange)
{
    OSL_PRECOND( getSimpleNodeName(_aNode) == rSubtreeChange.getNodeName(), "ERROR: Change name does not match node");

    if (_aNode.isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        deferredSetNode(_aNode)->revertCommit(rSubtreeChange);
    }
    else if (_aNode.isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        deferredGroupNode(_aNode)->revertCommit(rSubtreeChange);
        implRevertSubCommitted( GroupNode(_aNode), rSubtreeChange );
    }
    else
    {
        deferredValueNode(_aNode);
        OSL_ENSURE(false, "Tree: Cannot revert commit: Unexpected node type");
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implFailedCommit(Node const& _aNode, SubtreeChange& rSubtreeChange)
{
    OSL_PRECOND( getSimpleNodeName(_aNode) == rSubtreeChange.getNodeName(), "ERROR: Change name does not match node");

    if (_aNode.isSetNode())
    {
        OSL_ENSURE(rSubtreeChange.isSetNodeChange(),"ERROR: Change type GROUP does not match set");

        deferredSetNode(_aNode)->failedCommit(rSubtreeChange);
    }
    else if (_aNode.isGroupNode())
    {
        OSL_ENSURE(!rSubtreeChange.isSetNodeChange(),"ERROR: Change type SET does not match group");

        deferredGroupNode(_aNode)->failedCommit(rSubtreeChange);
        implFailedSubCommitted( GroupNode(_aNode), rSubtreeChange );
    }
    else
    {
        deferredValueNode(_aNode);
        OSL_ENSURE(false, "Tree: Cannot handle commit failure: Unexpected node type");
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implPreCommitSubChanges(GroupNode const & _aGroup, std::vector< rtl::Reference<configuration::ElementTree> >& _rRemovedElements, SubtreeChange& _rParentChange)
{
    for( Node aChild = _aGroup.getFirstChild(); aChild.is(); aChild = _aGroup.getNextChild(aChild) )
    {
        if (this->hasChanges(aChild))
        {
            std::auto_ptr<SubtreeChange> aSubChanges( this->implPreCommitChanges(aChild,_rRemovedElements) );
            std::auto_ptr<Change> aSubChangesBase( aSubChanges.release() );
            _rParentChange.addChange( aSubChangesBase );
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implFinishSubCommitted(GroupNode const & _aGroup, SubtreeChange& aChangesParent)
{
    for(SubtreeChange::MutatingChildIterator
            it = aChangesParent.begin_changes(),
            stop = aChangesParent.end_changes();
        it != stop;
        ++it)
    {
        if (dynamic_cast< SubtreeChange * >(&*it) != 0)
        {
            Node aChild = _aGroup.findChild( it->getNodeName() );
            OSL_ENSURE( aChild.is(), "Changed sub-node not found in tree");

            this->implFinishCommit(aChild, static_cast<SubtreeChange&>(*it));
        }
        else
        {
            OSL_ENSURE(dynamic_cast< ValueChange * >(&*it) != 0, "Unexpected change type for inner node; change is ignored");
            OSL_ENSURE(! _aGroup.findChild(it->getNodeName()).is() ,
                        "Found sub(tree) node where a value was changed");
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implRevertSubCommitted(GroupNode const & _aGroup, SubtreeChange& aChangesParent)
{
    for(SubtreeChange::MutatingChildIterator
            it = aChangesParent.begin_changes(),
            stop = aChangesParent.end_changes();
        it != stop;
        ++it)
    {
        if (dynamic_cast< SubtreeChange * >(&*it) != 0)
        {
            Node aChild = _aGroup.findChild( it->getNodeName() );
            OSL_ENSURE( aChild.is(), "Changed sub-node not found in tree");

            this->implRevertCommit(aChild, static_cast<SubtreeChange&>(*it));
        }
        else
        {
            OSL_ENSURE(dynamic_cast< ValueChange * >(&*it) != 0, "Unexpected change type for inner node; change is ignored");
            OSL_ENSURE(! _aGroup.findChild(it->getNodeName()).is() ,
                        "Found sub(tree) node where a value was changed");
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implFailedSubCommitted(GroupNode const & _aGroup, SubtreeChange& aChangesParent)
{
    for(SubtreeChange::MutatingChildIterator
            it = aChangesParent.begin_changes(),
            stop = aChangesParent.end_changes();
        it != stop;
        ++it)
    {
        if (dynamic_cast< SubtreeChange * >(&*it) != 0)
        {
            Node aChild = _aGroup.findChild( it->getNodeName() );
            OSL_ENSURE( aChild.is(), "Changed sub-node not found in tree");

            this->implFailedCommit(aChild, static_cast<SubtreeChange&>(*it));
        }
        else
        {
            OSL_ENSURE(dynamic_cast< ValueChange * >(&*it) != 0, "Unexpected change type for inner node; change is ignored");
            OSL_ENSURE(! _aGroup.findChild(it->getNodeName()).is() ,
                        "Found sub(tree) node where a value was changed");
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}
