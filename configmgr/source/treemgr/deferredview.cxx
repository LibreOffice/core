/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: deferredview.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:41:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h>
#include "deferredview.hxx"

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif
#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#include "nodeimplobj.hxx"
#endif

namespace configmgr
{
    namespace view
    {
//-----------------------------------------------------------------------------
using configuration::DeferredGroupNodeImpl;
using configuration::DeferredSetNodeImpl;

//-----------------------------------------------------------------------------
static inline DeferredGroupNodeImpl* deferredGroupNode(Node const& _aNode)
{
    return static_cast<DeferredGroupNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline DeferredGroupNodeImpl* deferredGroupNode(GroupNode const& _aNode)
{
    return static_cast<DeferredGroupNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline DeferredSetNodeImpl* deferredSetNode(Node const& _aNode)
{
    return static_cast<DeferredSetNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline DeferredSetNodeImpl* deferredSetNode(SetNode const& _aNode)
{
    return static_cast<DeferredSetNodeImpl*>(_aNode.get_impl());
}
//-----------------------------------------------------------------------------

static inline void deferredValueNode(Node const& _aNode)
{
    { (void)_aNode; }
    OSL_ENSURE( _aNode.isValueNode(),"Unknown Node type in deferred view");
    OSL_ENSURE(_aNode.get_offset() == _aNode.tree().get_impl()->root_(), "TreeImpl: Unexpected node type - non-root value element");
}
//-----------------------------------------------------------------------------

static void deferredValueNodeChanged(Node const& _aNode)
{
    { (void)_aNode; }
    OSL_ENSURE( _aNode.isValueNode(),"Unknown Node type in deferred view");
    OSL_ENSURE(_aNode.get_offset() == _aNode.tree().get_impl()->root_(), "TreeImpl: Unexpected node type - non-root value element");
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

            deferredGroupNode(aGroup)->collectValueChanges( _rChanges, _aNode.tree().get_impl(), _aNode.get_offset());

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
std::auto_ptr<SubtreeChange> DeferredViewStrategy::doPreCommitChanges(Tree const& _aTree, configuration::ElementList& _rRemovedElements)
{
    std::auto_ptr<SubtreeChange> pRet;
    if (hasChanges(_aTree))
    {
        pRet = implPreCommitChanges(getRootNode(_aTree),_rRemovedElements);
        if (pRet.get() != NULL)
        {
            pRet->setNodeName(getSimpleRootName(_aTree).toString());
        }
    }
    return pRet;
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doFailedCommit(Tree const& _aTree, SubtreeChange& rChanges)
{
    implFailedCommit(getRootNode(_aTree),rChanges);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doFinishCommit(Tree const& _aTree, SubtreeChange& rChanges)
{
    implFinishCommit(getRootNode(_aTree),rChanges);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doRevertCommit(Tree const& _aTree, SubtreeChange& rChanges)
{
    implRevertCommit(getRootNode(_aTree),rChanges);
}

//-----------------------------------------------------------------------------
configuration::ValueChangeImpl* DeferredViewStrategy::doAdjustToValueChange(GroupNode const& _aGroupNode, Name const& _aName, ValueChange const& rExternalChange)
{
    using configuration::ValueChangeImpl;
    DeferredGroupNodeImpl::MemberChange aChange = deferredGroupNode(_aGroupNode)->findValueChange(_aName);

    if (aChange.is())
    {
        if (ValueChangeImpl* pValueChange = aChange->adjustToChange(rExternalChange))
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
configuration::ValueMemberNode DeferredViewStrategy::doGetValueMember(GroupNode const& _aNode, Name const& _aName, bool _bForUpdate) const
{
    return deferredGroupNode(_aNode)->makeValueMember(_aName,_bForUpdate);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doInsertElement(SetNode const& _aNode, Name const& aName, configuration::SetEntry const& _aNewEntry)
{
    // move to this memory segment
    // should be direct (as any free-floating one)

    //implMakeElement(aNewEntry)
    SetNodeElement aNewElement = implMakeElement(_aNode, _aNewEntry );

    deferredSetNode(_aNode)->insertNewElement(aName, aNewElement);
}

//-----------------------------------------------------------------------------
void DeferredViewStrategy::doRemoveElement(SetNode const& _aNode, Name const& aName)
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

ViewStrategyRef createDeferredChangeStrategy()
{
    return new DeferredViewStrategy();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredViewStrategy::implPreCommitChanges(Node const& _aNode, configuration::ElementList& _rRemovedElements)
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
    OSL_PRECOND( getSimpleNodeName(_aNode).toString() == rSubtreeChange.getNodeName(), "ERROR: Change name does not match node");

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
        OSL_ENSURE(false, "TreeImpl: Cannot finish commit: Unexpected node type");
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implRevertCommit(Node const& _aNode, SubtreeChange& rSubtreeChange)
{
    OSL_PRECOND( getSimpleNodeName(_aNode).toString() == rSubtreeChange.getNodeName(), "ERROR: Change name does not match node");

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
        OSL_ENSURE(false, "TreeImpl: Cannot revert commit: Unexpected node type");
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implFailedCommit(Node const& _aNode, SubtreeChange& rSubtreeChange)
{
    OSL_PRECOND( getSimpleNodeName(_aNode).toString() == rSubtreeChange.getNodeName(), "ERROR: Change name does not match node");

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
        OSL_ENSURE(false, "TreeImpl: Cannot handle commit failure: Unexpected node type");
    }
}
//-----------------------------------------------------------------------------

void DeferredViewStrategy::implPreCommitSubChanges(GroupNode const & _aGroup, configuration::ElementList& _rRemovedElements, SubtreeChange& _rParentChange)
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
        if ( it->ISA(SubtreeChange) )
        {
            Node aChild = _aGroup.findChild( configuration::makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( aChild.is(), "Changed sub-node not found in tree");

            this->implFinishCommit(aChild, static_cast<SubtreeChange&>(*it));
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for ínner node; change is ignored");
            OSL_ENSURE(! _aGroup.findChild( configuration::makeNodeName(it->getNodeName(), Name::NoValidate())).is() ,
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
        if ( it->ISA(SubtreeChange) )
        {
            Node aChild = _aGroup.findChild( configuration::makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( aChild.is(), "Changed sub-node not found in tree");

            this->implRevertCommit(aChild, static_cast<SubtreeChange&>(*it));
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for ínner node; change is ignored");
            OSL_ENSURE(! _aGroup.findChild( configuration::makeNodeName(it->getNodeName(), Name::NoValidate())).is() ,
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
        if ( it->ISA(SubtreeChange) )
        {
            Node aChild = _aGroup.findChild( configuration::makeNodeName(it->getNodeName(), Name::NoValidate()) );
            OSL_ENSURE( aChild.is(), "Changed sub-node not found in tree");

            this->implFailedCommit(aChild, static_cast<SubtreeChange&>(*it));
        }
        else
        {
            OSL_ENSURE(it->ISA(ValueChange), "Unexpected change type for ínner node; change is ignored");
            OSL_ENSURE(! _aGroup.findChild( configuration::makeNodeName(it->getNodeName(), Name::NoValidate())).is() ,
                        "Found sub(tree) node where a value was changed");
        }
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}
