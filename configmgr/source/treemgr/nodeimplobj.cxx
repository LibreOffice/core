/*************************************************************************
 *
 *  $RCSfile: nodeimplobj.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-20 11:01:51 $
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
 *  Source License Version 1.1 (the "License") You may not use this file
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
#include <stdio.h>
#include "nodeimplobj.hxx"

#ifndef CONFIGMGR_CONFIGNODEFACTORY_HXX_
#include "nodefactory.hxx"
#endif

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#include "nodechangeinfo.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

static void failReadOnly()
{
    throw ConstraintViolation("INTERNAL ERROR: Trying to update a read-only node");
}

static
inline
Attributes forceReadOnly(Attributes aAttributes)
{
    aAttributes.bWritable = false;
    return aAttributes;
}

static
inline
Attributes adjustForDirectAccess(Attributes aAttributes)
{
    aAttributes.bWritable = true;
    return aAttributes;
}
// Specific types of nodes for direct or read only access
//-----------------------------------------------------------------------------

// Value Nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class ReadOnlyValueElementNodeImpl
//-----------------------------------------------------------------------------
Attributes ReadOnlyValueElementNodeImpl::doGetAttributes() const
{
    return forceReadOnly( ValueElementNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

NodeImplHolder ReadOnlyValueElementNodeImpl::doCloneIndirect(bool)
{
    return this;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DirectValueElementNodeImpl
//-----------------------------------------------------------------------------

DirectValueElementNodeImpl::DirectValueElementNodeImpl(ValueNode& rOriginal)
: ValueElementNodeImpl(rOriginal)
{}
//-----------------------------------------------------------------------------

DirectValueElementNodeImpl::DirectValueElementNodeImpl(DeferredValueElementNodeImpl& rOriginal)
: ValueElementNodeImpl(rOriginal)
{}
//-----------------------------------------------------------------------------

Attributes DirectValueElementNodeImpl::doGetAttributes() const
{
    return adjustForDirectAccess( ValueElementNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

NodeImplHolder DirectValueElementNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return new DeferredValueElementNodeImpl(*this);
    else
        return this;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class DeferredValueElementNodeImpl
//-----------------------------------------------------------------------------

DeferredValueElementNodeImpl::DeferredValueElementNodeImpl(ValueNode& rOriginal)
: ValueElementNodeImpl(rOriginal)
{
}
//-----------------------------------------------------------------------------

DeferredValueElementNodeImpl::DeferredValueElementNodeImpl(DirectValueElementNodeImpl& rOriginal)
: ValueElementNodeImpl(rOriginal)
{
}
//-----------------------------------------------------------------------------

NodeImplHolder DeferredValueElementNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return this;
    else
        return new DirectValueElementNodeImpl(*this);
}
//-----------------------------------------------------------------------------

// Group Nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class ReadOnlyGroupNodeImpl
//-----------------------------------------------------------------------------

Attributes ReadOnlyGroupNodeImpl::doGetAttributes() const
{
    return forceReadOnly( GroupNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

ValueMemberNode ReadOnlyGroupNodeImpl::doGetValueMember(Name const& aName, bool bForUpdate)
{
    if (bForUpdate) failReadOnly();

    return GroupNodeImpl::doGetValueMember(aName, bForUpdate);
}
//-----------------------------------------------------------------------------

bool ReadOnlyGroupNodeImpl::doHasChanges() const
{
    return false;
}
//-----------------------------------------------------------------------------

void ReadOnlyGroupNodeImpl::doCommitChanges()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyGroupNodeImpl::doMarkChanged()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

NodeImplHolder ReadOnlyGroupNodeImpl::doCloneIndirect(bool)
{
    return this;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class DirectGroupNodeImpl
//-----------------------------------------------------------------------------

DirectGroupNodeImpl::DirectGroupNodeImpl(ISubtree& rOriginal)
: GroupNodeImpl(rOriginal)
{}
//-----------------------------------------------------------------------------

DirectGroupNodeImpl::DirectGroupNodeImpl(DeferredGroupNodeImpl& rOriginal)
: GroupNodeImpl(rOriginal)
{}
//-----------------------------------------------------------------------------

ValueMemberNode DirectGroupNodeImpl::doGetValueMember(Name const& aName, bool bForUpdate)
{
    return GroupNodeImpl::doGetValueMember(aName, bForUpdate);
}
//-----------------------------------------------------------------------------

bool DirectGroupNodeImpl::doHasChanges() const
{
    return false;
}
//-----------------------------------------------------------------------------

void DirectGroupNodeImpl::doCommitChanges()
{
    OSL_ENSURE(false,"WARNING: Should not commit changes on object that hasn't any");
}
//-----------------------------------------------------------------------------

void DirectGroupNodeImpl::doMarkChanged()
{
    // ignore
}
//-----------------------------------------------------------------------------

Attributes DirectGroupNodeImpl::doGetAttributes() const
{
    return adjustForDirectAccess( GroupNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

NodeImplHolder DirectGroupNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return new DeferredGroupNodeImpl(*this);
    else
        return this;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DeferredGroupNodeImpl
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::DeferredGroupNodeImpl(ISubtree& rOriginal)
: GroupNodeImpl(rOriginal)
, m_aChanges()
{
}
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::DeferredGroupNodeImpl(DirectGroupNodeImpl& rOriginal)
: GroupNodeImpl(rOriginal)
, m_aChanges()
{
}
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::~DeferredGroupNodeImpl()
{
}
//-----------------------------------------------------------------------------

ValueMemberNode DeferredGroupNodeImpl::doGetValueMember(Name const& aName, bool bForUpdate)
{
    ValueChanges::iterator it = m_aChanges.find(aName);

    if (it != m_aChanges.end())
    {
        if (it->second.isEmpty())
            OSL_ENSURE(aName.isEmpty(), "ERROR: Found empty change reference");

        else if (bForUpdate || it->second->isChange()) // found one
            return ValueMemberNode(it->second);

        else // leftover non-change
            m_aChanges.erase(it);

        // if not found continue with default
    }

    if (bForUpdate) // create a new change
    {
        if (ValueNode* pOriginal = getOriginalValueNode(aName))
        {
            DeferredValueImplRef aNewChange(new ValueMemberNode::DeferredImpl(*pOriginal));
            m_aChanges[aName] = aNewChange;

            return ValueMemberNode(aNewChange);
       }
    }

    return GroupNodeImpl::doGetValueMember(aName, bForUpdate);
}
//-----------------------------------------------------------------------------

bool DeferredGroupNodeImpl::doHasChanges() const
{
    for (ValueChanges::const_iterator it = m_aChanges.begin(); it != m_aChanges.end(); it)
    {
        if (!it->second.isValid())
        {
            // empty element is present -> marked as changed
            OSL_ASSERT(it->first.isEmpty());
            return true;
        }

        if (it->second->isChange())
            return true;
    }

    return false;
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doCollectChangesWithTarget(NodeChanges& rChanges, TreeImpl* pParentTree, NodeOffset nNode) const
{
    for (ValueChanges::const_iterator it = m_aChanges.begin(); it != m_aChanges.end(); ++it)
    {
        if (it->second.isValid())
        {
            OSL_ASSERT(!it->first.isEmpty());
            if (ValueChangeImpl* pValueChange = it->second->collectChange())
            {
                pValueChange->setTarget(pParentTree,nNode,it->first);

                rChanges.add( NodeChange(pValueChange) );
            }
            else // leftover non-change
                OSL_ENSURE(!it->second->isChange(), "Got no change from a changing value") ;
        }
        else
            OSL_ASSERT(it->first.isEmpty());
    }
}
//-----------------------------------------------------------------------------

ValueChangeImpl* DeferredGroupNodeImpl::doAdjustToValueChange(Name const& aName, ValueChange const& rExternalChange)
{
    ValueChanges::iterator it = m_aChanges.find(aName);

    if (it != m_aChanges.end())
    {
        if (it->second.isValid())
        {
            if (ValueChangeImpl* pValueChange = it->second->adjustToChange(rExternalChange))
            {
                OSL_ENSURE(it->second->isChange(), "Got an adjusted change from a non-changing value");
            }

            else // leftover non-change
            {
                OSL_ENSURE(!it->second->isChange(), "Got no adjusted change from a changing value") ;
                m_aChanges.erase(it);
                // then do as without deferred change
            }
        }
        else
            OSL_ENSURE(aName.isEmpty(), "ERROR: Found empty change reference");
    }

    return GroupNodeImpl::doAdjustToValueChange(aName, rExternalChange);
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doCommitChanges()
{
    for (ValueChanges::iterator pos = m_aChanges.begin(); pos != m_aChanges.end(); )
    {
        ValueChanges::iterator it = pos++; // this is used to allow erasing below
        if (it->second.isValid())
        {
            it->second->commitDirect();
            m_aChanges.erase(it); // this goes here to ensure exception safety
        }
        else
            OSL_ASSERT(it->first.isEmpty());
    }
    m_aChanges.clear();
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredGroupNodeImpl::doPreCommitChanges()
{
    std::auto_ptr<SubtreeChange> aRet;

    if (!m_aChanges.empty())
    {
        aRet.reset( new SubtreeChange(this->getOriginalNodeName(),
                                      this->getAttributes()) );

        for (ValueChanges::iterator pos = m_aChanges.begin(); pos != m_aChanges.end(); )
        {
            ValueChanges::iterator it = pos++; // this is used to allow erasing below

            if (!it->second.isValid())
            {
                OSL_ASSERT(it->first.isEmpty());
            }
            else if (it->second->isChange())
            {
                std::auto_ptr<ValueChange> aValueChange = it->second->preCommitChange();
                if (aValueChange.get())
                {
                    std::auto_ptr<Change> aBaseChange(aValueChange.release());
                    aRet->addChange( aBaseChange );
                }
                else
                    OSL_ENSURE(false, "Got no change from a changed member");
            }
            else // found left-over non-change
                m_aChanges.erase(it);
        }
        if (m_aChanges.empty()) aRet.reset();
    }

    return aRet;
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doFinishCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aValueName = makeNodeName(it->getNodeName(), Name::NoValidate());

        ValueChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            OSL_ENSURE( it->ISA(ValueChange) , "Unexpected type of element change");
            if (!it->ISA(ValueChange)) throw Exception("Unexpected type of element change");

            ValueChange & rValueChange = static_cast<ValueChange&>(*it);

            DeferredValueImplRef aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.isValid(), "Found empty change object for Member value change");

            if (aStoredChange.isValid())
            {
                aStoredChange->finishCommit(rValueChange);
                OSL_ENSURE(!aStoredChange->isChange(),"ValueChange is not moot after finishCommit");
            }

            m_aChanges.erase( itStoredChange ); // remove finished change
        }
        else
            OSL_ENSURE( !it->ISA(ValueChange) , "Value member change has no change data representation");

    }

    OSL_DEBUG_ONLY( m_aChanges.erase( Name() ) ); // remove change marker (if present)
    OSL_ENSURE(m_aChanges.empty(), "Found unprocessed changes to values in group");

    m_aChanges.clear(); // remove all pending stuff and marker
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doRevertCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aValueName = makeNodeName(it->getNodeName(), Name::NoValidate());

        ValueChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            OSL_ENSURE( it->ISA(ValueChange) , "Unexpected type of element change");
            if (!it->ISA(ValueChange)) continue;

            ValueChange & rValueChange = static_cast<ValueChange&>(*it);

            DeferredValueImplRef aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.isValid(), "Cannot restore change: found empty change object for Member value change");

            if (aStoredChange.isValid())
            {
                aStoredChange->revertCommit(rValueChange);
                OSL_ENSURE(!aStoredChange->isChange(),"ValueChange is not moot after reverting - will be discarded nevertheless");
            }
            m_aChanges.erase( itStoredChange ); // remove change if it is moot
        }
        else
            OSL_ENSURE( !it->ISA(ValueChange) , "Value member change has no change data representation");
    }
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doFailedCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aValueName = makeNodeName(it->getNodeName(), Name::NoValidate());

        ValueChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            OSL_ENSURE( it->ISA(ValueChange) , "Unexpected type of element change");
            if (!it->ISA(ValueChange)) continue;

            ValueChange & rValueChange = static_cast<ValueChange&>(*it);

            DeferredValueImplRef aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.isValid(), "Cannot recover from failed change: found empty change object for Member value change");

            if (aStoredChange.isValid())
                 aStoredChange->failedCommit(rValueChange);
           {
                if (!aStoredChange->isChange())
                    m_aChanges.erase( itStoredChange ); // remove change if it is moot
            }
        }
        else
            OSL_ENSURE( !it->ISA(ValueChange) , "Value member change has no change data representation");
    }

    OSL_DEBUG_ONLY( m_aChanges.erase( Name() ) ); // remove change marker (if present)
    OSL_ENSURE(m_aChanges.empty(), "RevertCommit: Found unprocessed changes to values in group");

    m_aChanges.clear(); // discard all pending stuff and marker
}
//-----------------------------------------------------------------------------


void DeferredGroupNodeImpl::doMarkChanged()
{
    // special mark: a NULL DeferredImplRef at empty Name
    m_aChanges.insert( ValueChanges::value_type() );
}
//-----------------------------------------------------------------------------

NodeImplHolder DeferredGroupNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return this;
    else
        return new DirectGroupNodeImpl(*this);
}
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class ReadOnlyTreeSetNodeImpl
//-----------------------------------------------------------------------------

void ReadOnlyTreeSetNodeImpl::doInsertElement(Name const& , SetEntry const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyTreeSetNodeImpl::doRemoveElement(Name const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyTreeSetNodeImpl::doInitElements( ISubtree& rTree, TreeDepth nDepth)
{
    TreeSetNodeImpl::initHelper( NodeType::getReadAccessFactory(), rTree, nDepth);
}
//-----------------------------------------------------------------------------

ReadOnlyTreeSetNodeImpl::Element ReadOnlyTreeSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth)
{
    return TreeSetNodeImpl::makeAdditionalElement( NodeType::getReadAccessFactory(), aAddNodeChange, nDepth);
}
//-----------------------------------------------------------------------------

Attributes ReadOnlyTreeSetNodeImpl::doGetAttributes() const
{
    return forceReadOnly( TreeSetNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

bool ReadOnlyTreeSetNodeImpl::doHasChanges()    const
{
    return false;
}
//-----------------------------------------------------------------------------

void ReadOnlyTreeSetNodeImpl::doCollectChanges(NodeChanges& ) const
{
}
//-----------------------------------------------------------------------------

void ReadOnlyTreeSetNodeImpl::doCommitChanges()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyTreeSetNodeImpl::doMarkChanged()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

NodeImplHolder ReadOnlyTreeSetNodeImpl::doCloneIndirect(bool)
{
    return this;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class ReadOnlyValueSetNodeImpl
//-----------------------------------------------------------------------------

void ReadOnlyValueSetNodeImpl::doInsertElement(Name const& , SetEntry const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyValueSetNodeImpl::doRemoveElement(Name const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyValueSetNodeImpl::doInitElements( ISubtree& rTree, TreeDepth )
{
    ValueSetNodeImpl::initHelper( NodeType::getReadAccessFactory(), rTree);
}
//-----------------------------------------------------------------------------

ReadOnlyValueSetNodeImpl::Element ReadOnlyValueSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth )
{
    return ValueSetNodeImpl::makeAdditionalElement( NodeType::getReadAccessFactory(), aAddNodeChange);
}
//-----------------------------------------------------------------------------

Attributes ReadOnlyValueSetNodeImpl::doGetAttributes() const
{
    return forceReadOnly( ValueSetNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

bool ReadOnlyValueSetNodeImpl::doHasChanges()   const
{
    return false;
}
//-----------------------------------------------------------------------------

void ReadOnlyValueSetNodeImpl::doCollectChanges(NodeChanges& ) const
{
}
//-----------------------------------------------------------------------------

void ReadOnlyValueSetNodeImpl::doCommitChanges()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyValueSetNodeImpl::doMarkChanged()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

NodeImplHolder ReadOnlyValueSetNodeImpl::doCloneIndirect(bool)
{
    return this;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class DirectTreeSetNodeImpl
//-----------------------------------------------------------------------------

DirectTreeSetNodeImpl::DirectTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
: TreeSetNodeImpl(rOriginal,pTemplate)
{
}
//-----------------------------------------------------------------------------

DirectTreeSetNodeImpl::DirectTreeSetNodeImpl(DeferredTreeSetNodeImpl& rOriginal)
: TreeSetNodeImpl(rOriginal)
{
    implMakeIndirect(false);
}
//-----------------------------------------------------------------------------

void DirectTreeSetNodeImpl::doInsertElement(Name const& aName, SetEntry const& aNewEntry)
{
    TreeSetNodeImpl::doInsertElement(aName,aNewEntry);
    aNewEntry.tree()->makeIndirect(false);
}
//-----------------------------------------------------------------------------

void DirectTreeSetNodeImpl::doRemoveElement(Name const& aName)
{
    TreeSetNodeImpl::doRemoveElement(aName);
}
//-----------------------------------------------------------------------------

void DirectTreeSetNodeImpl::doInitElements(ISubtree& rTree, TreeDepth nDepth)
{
    TreeSetNodeImpl::initHelper(NodeType::getDirectAccessFactory(), rTree, nDepth);
}
//-----------------------------------------------------------------------------

DirectTreeSetNodeImpl::Element DirectTreeSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth)
{
    return TreeSetNodeImpl::makeAdditionalElement( NodeType::getDirectAccessFactory(), aAddNodeChange, nDepth);
}
//-----------------------------------------------------------------------------

bool DirectTreeSetNodeImpl::doHasChanges() const
{
    return false;
}
//-----------------------------------------------------------------------------

void DirectTreeSetNodeImpl::doCollectChanges(NodeChanges& ) const
{
}
//-----------------------------------------------------------------------------

void DirectTreeSetNodeImpl::doCommitChanges()
{
    OSL_ENSURE(false,"WARNING: Should not commit changes on object that hasn't any");
}
//-----------------------------------------------------------------------------

void DirectTreeSetNodeImpl::doMarkChanged()
{
    // ignore
}
//-----------------------------------------------------------------------------

Attributes DirectTreeSetNodeImpl::doGetAttributes() const
{
    return adjustForDirectAccess( TreeSetNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

NodeImplHolder DirectTreeSetNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return new DeferredTreeSetNodeImpl(*this);
    else
        return this;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DirectValueSetNodeImpl
//-----------------------------------------------------------------------------

DirectValueSetNodeImpl::DirectValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
: ValueSetNodeImpl(rOriginal,pTemplate)
{
}
//-----------------------------------------------------------------------------

DirectValueSetNodeImpl::DirectValueSetNodeImpl(DeferredValueSetNodeImpl& rOriginal)
: ValueSetNodeImpl(rOriginal)
{
    implMakeIndirect(false);
}
//-----------------------------------------------------------------------------

void DirectValueSetNodeImpl::doInsertElement(Name const& aName, SetEntry const& aNewElement)
{
    ValueSetNodeImpl::doInsertElement(aName,aNewElement);
    aNewElement.tree()->makeIndirect(false);
}
//-----------------------------------------------------------------------------

void DirectValueSetNodeImpl::doRemoveElement(Name const& aName)
{
    ValueSetNodeImpl::doRemoveElement(aName);
}
//-----------------------------------------------------------------------------

void DirectValueSetNodeImpl::doInitElements(ISubtree& rTree, TreeDepth )
{
    ValueSetNodeImpl::initHelper( NodeType::getDirectAccessFactory(), rTree);
}
//-----------------------------------------------------------------------------

DirectValueSetNodeImpl::Element DirectValueSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth )
{
    return ValueSetNodeImpl::makeAdditionalElement( NodeType::getDirectAccessFactory(), aAddNodeChange);
}
//-----------------------------------------------------------------------------

bool DirectValueSetNodeImpl::doHasChanges() const
{
    return false;
}
//-----------------------------------------------------------------------------

void DirectValueSetNodeImpl::doCollectChanges(NodeChanges& ) const
{
}
//-----------------------------------------------------------------------------

void DirectValueSetNodeImpl::doCommitChanges()
{
    OSL_ENSURE(false,"WARNING: Should not commit changes on object that hasn't any");
}
//-----------------------------------------------------------------------------

void DirectValueSetNodeImpl::doMarkChanged()
{
    // Ignore
}
//-----------------------------------------------------------------------------

Attributes DirectValueSetNodeImpl::doGetAttributes() const
{
    return adjustForDirectAccess( ValueSetNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

NodeImplHolder DirectValueSetNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return new DeferredValueSetNodeImpl(*this);
    else
        return this;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DeferredTreeSetNodeImpl
//-----------------------------------------------------------------------------

DeferredTreeSetNodeImpl::DeferredTreeSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
: TreeSetNodeImpl(rOriginal,pTemplate)
, m_aChangedData()
, m_bChanged(false)
{
}
//-----------------------------------------------------------------------------

DeferredTreeSetNodeImpl::DeferredTreeSetNodeImpl(DirectTreeSetNodeImpl& rOriginal)
: TreeSetNodeImpl(rOriginal)
, m_aChangedData()
, m_bChanged(false)
{
    implMakeIndirect(true);
}
//-----------------------------------------------------------------------------

bool DeferredTreeSetNodeImpl::doIsEmpty() const
{
    if (m_aChangedData.isEmpty())
        return TreeSetNodeImpl::doIsEmpty();

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop;
        ++it)
    {
        if (it->isValid()) return false;
    }}


    // look for elements in the base set that are not 'deleted' (the changes are all deletions here)
    {for(NativeIterator it = TreeSetNodeImpl::beginElementSet(), stop = TreeSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.hasElement(it->first)) return false;
    }}

    return true;
}
//-----------------------------------------------------------------------------

SetEntry DeferredTreeSetNodeImpl::doFindElement(Name const& aName)
{
    Element* pElement = m_aChangedData.getElement(aName);
    if (!pElement)
        pElement = TreeSetNodeImpl::getStoredElement(aName);

    return SetEntry(pElement ? pElement->getBodyPtr() : 0);
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doClearElements()
{
    TreeSetNodeImpl::doClearElements();
    m_aChangedData.clearElements();
}
//-----------------------------------------------------------------------------

SetNodeVisitor::Result DeferredTreeSetNodeImpl::doDispatchToElements(SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    // look for elements in the base set that are not hidden by changes
    {for(NativeIterator it = TreeSetNodeImpl::beginElementSet(), stop = TreeSetNodeImpl::endElementSet();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            eRet = aVisitor.visit(SetEntry(it->second.getBodyPtr()));
        }
    }}

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (it->isValid())
        {
            eRet = aVisitor.visit(SetEntry(it->getBodyPtr()));
        }
    }}
    return eRet;
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doInsertElement(Name const& aName, SetEntry const& aNewEntry)
{
    implInsertNewElement(aName, TreeSetNodeImpl::implMakeElement(aNewEntry.tree()));
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doRemoveElement(Name const& aName)
{
    implRemoveOldElement(aName);
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doInitElements(ISubtree& rTree, TreeDepth nDepth)
{
    TreeSetNodeImpl::initHelper(NodeType::getDeferredChangeFactory(), rTree, nDepth);
}
//-----------------------------------------------------------------------------

DeferredTreeSetNodeImpl::Element DeferredTreeSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth nDepth)
{
    return TreeSetNodeImpl::makeAdditionalElement( NodeType::getDeferredChangeFactory(), aAddNodeChange, nDepth);
}
//-----------------------------------------------------------------------------


bool DeferredTreeSetNodeImpl::doHasChanges() const
{
    return m_bChanged || !m_aChangedData.isEmpty();
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doCollectChanges(NodeChanges& rChanges) const
{
    // collect added and deleted nodes
    {for(NativeIterator it = m_aChangedData.beginNative(), stop = m_aChangedData.endNative();
        it != stop;
        ++it)
    {
        Element const* pOriginal = TreeSetNodeImpl::getStoredElement(it->first);

        if (it->second.isValid()) // added one
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(doCreateReplace(it->first,it->second,*pOriginal)));
            }
            else
            {
                rChanges.add(NodeChange(doCreateInsert(it->first,it->second)));
            }
        }
        else
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(doCreateRemove(it->first,*pOriginal)));
            }

            //else nothing to do
        }
    }}

    // collect preexisting nodes
    {for(NativeIterator it = TreeSetNodeImpl::beginElementSet(), stop = TreeSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            if (it->second->hasChanges())
                it->second->collectChanges(rChanges);
        }
    }}

}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doMarkChanged()
{
    m_bChanged = true;
}
//-----------------------------------------------------------------------------

NodeImplHolder DeferredTreeSetNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return this;
    else
        return new DirectTreeSetNodeImpl(*this);
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doCommitChanges()
{
    // commit preexisting nodes
    {for(NativeIterator it = TreeSetNodeImpl::beginElementSet(), stop = TreeSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            it->second->commitDirect();
        }
    }}

    // commit added and deleted nodes
    {
        NativeIterator it = m_aChangedData.beginNative();
        NativeIterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            Name    aName       = it->first;
            Element aNewElement = it->second;

            Element* pOriginal = TreeSetNodeImpl::getStoredElement(aName);

            if (aNewElement.isValid())
            {
                if (pOriginal)
                    TreeSetNodeImpl::implReplaceElement(aName,aNewElement,true);

                else
                    TreeSetNodeImpl::implInsertElement(aName,aNewElement,true);

                aNewElement->makeIndirect(true);
            }
            else
            {
                if (pOriginal)
                {
                    TreeSetNodeImpl::implRemoveElement(aName,true);
                }

                //else nothing to do
            }
            if (pOriginal)
            {
                OSL_ASSERT(pOriginal->isValid());
                (*pOriginal)->commitDirect();
                (*pOriginal)->makeIndirect(false);
            }

            ++it;
            m_aChangedData.removeElement(aName);
        }
    }

    m_bChanged = false;
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredTreeSetNodeImpl::doPreCommitChanges(ElementList& _rRemovedElements)
{
    // nowfirst get the name of this node
    OUString aName = this->getOriginalNodeName();

    // and make a SubtreeChange
    std::auto_ptr<SubtreeChange> pSetChange( new SubtreeChange(aName,
                                                               getElementTemplate()->getName().toString(),
                                                               getElementTemplate()->getModule().toString(),
                                                               this->getAttributes()) );

    // commit preexisting nodes
    {for(NativeIterator it = TreeSetNodeImpl::beginElementSet(), stop = TreeSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());

            std::auto_ptr<SubtreeChange> pNewChange( it->second->preCommitChanges(_rRemovedElements) );
            if (pNewChange.get() != 0)
            {
                std::auto_ptr<Change> pNewChangeBase( pNewChange.release() );
                pSetChange->addChange(pNewChangeBase);
            }
        }
    }}

    // commit added and deleted nodes
    {
        NativeIterator it = m_aChangedData.beginNative();
        NativeIterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            Name    aName       = it->first;
            Element aNewElement = it->second;

            Element* pOriginal = TreeSetNodeImpl::getStoredElement(aName);

            if (aNewElement.isValid())
            {
                std::auto_ptr<INode> aAddedTree;
                aNewElement->releaseTo( aAddedTree );

                OSL_ENSURE( aAddedTree.get(), "Could not take the new tree from the ElementTree");

                AddNode* pAddNode = new AddNode(aAddedTree, aName.toString() );

                std::auto_ptr<Change> pNewChange( pAddNode );

                if (pOriginal)
                    pAddNode->setReplacing();

                pSetChange->addChange(pNewChange);
            }
            else
            {
                if (pOriginal)
                {
                    std::auto_ptr<Change> pNewChange( new RemoveNode(aName.toString()) );

                    pSetChange->addChange(pNewChange);
                }
                //else nothing to do
            }

            // collect removed or replaced element
            if (pOriginal)
                _rRemovedElements.push_back( *pOriginal );

            ++it;
        }
    }
    return pSetChange;
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doFinishCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aElementName = makeElementName(it->getNodeName(), Name::NoValidate());

        Element* pOriginal = getStoredElement(aElementName);

        if (Element* pNewElement = m_aChangedData.getElement(aElementName))
        {
            Element aOriginal;
            if (pOriginal)
            {
                aOriginal = *pOriginal;
                OSL_ASSERT(aOriginal.isValid());
            }
            else
                OSL_ASSERT(!aOriginal.isValid());

            // handle a added, replaced or deleted node
            std::auto_ptr<INode> aRemovedNode;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                aRemovedNode = rAddNode.releaseReplacedNode();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=aRemovedNode.get())  );

                if (aOriginal.isValid())
                    TreeSetNodeImpl::implReplaceElement(aElementName,*pNewElement,false);

                else
                    TreeSetNodeImpl::implInsertElement(aElementName,*pNewElement,false);

                (*pNewElement)->makeIndirect(true);
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                aRemovedNode = rRemoveNode.releaseRemovedNode();

                OSL_ASSERT(aOriginal.isValid());
                if (aOriginal.isValid())
                    TreeSetNodeImpl::implRemoveElement(aElementName,false);
            }
            // handle a added or deleted node
            if (aOriginal.isValid())
            {
                OSL_ENSURE(aRemovedNode.get(), "Cannot take over the removed node");

                aOriginal->takeNodeFrom(aRemovedNode);
                aOriginal->commitDirect(); // tree is detached => commit directly
                aOriginal->makeIndirect(false);
            }
            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            OSL_ENSURE(pOriginal, "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected type of element change");

            if (!it->ISA(SubtreeChange)) throw Exception("Unexpected type of element change");

            if (pOriginal)
                (*pOriginal)->finishCommit(static_cast<SubtreeChange&>(*it));
        }
    }
    m_bChanged = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doRevertCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aElementName = makeElementName(it->getNodeName(), Name::NoValidate());

        Element* pOriginal = getStoredElement(aElementName);

        if (Element* pNewElement = m_aChangedData.getElement(aElementName))
        {
            // handle a added, replaced or deleted node
            std::auto_ptr<INode> pRemovedNode;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                pRemovedNode = rAddNode.releaseReplacedNode();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=rAddNode.getReplacedNode_Unsafe())  );

                std::auto_ptr<INode> aAddedNode = rAddNode.releaseAddedNode();
                OSL_ENSURE(aAddedNode.get(), "Cannot restore new node: Change lost ownership");

                // restore the tree
                (*pNewElement)->takeNodeFrom(aAddedNode);
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                pRemovedNode = rRemoveNode.releaseRemovedNode();

                OSL_ASSERT(pOriginal);
                OSL_ASSERT((0 != pOriginal) == (0!=rRemoveNode.getRemovedNode_Unsafe())  );
            }
            OSL_ENSURE(pRemovedNode.get() == 0, "Possible problems reverting removed node: Change took ownership");
            // handle a added or deleted node
            if (pOriginal && pRemovedNode.get())
            {
                OSL_ASSERT(pOriginal->isValid());
                (*pOriginal)->takeNodeFrom(pRemovedNode);
            }
            OSL_ENSURE(pRemovedNode.get() == 0, "Could not revert removed node: Nowhere to put ownership");
        }
        else
        {
            // handle preexisting nodes
            OSL_ENSURE(pOriginal, "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected type of element change");

            if (!it->ISA(SubtreeChange)) throw Exception("Unexpected type of element change");

            if (pOriginal)
                (*pOriginal)->revertCommit(static_cast<SubtreeChange&>(*it));
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doFailedCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aElementName = makeElementName(it->getNodeName(), Name::NoValidate());

        Element* pOriginal = getStoredElement(aElementName);

        if (Element* pNewElement = m_aChangedData.getElement(aElementName))
        {
            Element aOriginal;
            if (pOriginal)
            {
                aOriginal = *pOriginal;
                OSL_ASSERT(aOriginal.isValid());
            }
            else
                OSL_ASSERT(!aOriginal.isValid());

            // handle a added, replaced or deleted node
            std::auto_ptr<INode> aRemovedNode;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                aRemovedNode = rAddNode.releaseReplacedNode();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=rAddNode.getReplacedNode_Unsafe())  );

                std::auto_ptr<INode> aAddedNode = rAddNode.releaseAddedNode();

                if (aAddedNode.get()) // Change not done; need to restore new node (element will be released into the wild then)
                {
                    (*pNewElement)->takeNodeFrom(aAddedNode);
                    detach(*pNewElement, false);
                }

                else if (getOriginalSetNode().getChild(aElementName.toString()) == rAddNode.getAddedNode_unsafe())
                { // it has been integrated into the master tree

                    // so add it
                    if (aOriginal.isValid())
                        TreeSetNodeImpl::implReplaceElement(aElementName,*pNewElement,false);

                    else
                        TreeSetNodeImpl::implInsertElement(aElementName,*pNewElement,false);

                    (*pNewElement)->makeIndirect(true);
                }
                else
                {
                    OSL_ENSURE(false, "Unexpected: added node is gone, but where ? May cause invalid references");
                    detach(*pNewElement, false);
                    // pNewElement->disposeData()
                }

            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                aRemovedNode = rRemoveNode.releaseRemovedNode();

                OSL_ASSERT(aOriginal.isValid());
                if (aRemovedNode.get() || rRemoveNode.getRemovedNode_Unsafe() != getOriginalSetNode().getChild(aElementName.toString()))
                {
                    // really removed - then remove the originel
                    if (aOriginal.isValid())
                        TreeSetNodeImpl::implRemoveElement(aElementName,false);
                    OSL_ENSURE(NULL == getOriginalSetNode().getChild(aElementName.toString()),"ERROR: Removed Node still there or replaced");
                }
            }

            // handle a added or deleted node
            if (aOriginal.isValid() && aRemovedNode.get())
            {
                aOriginal->takeNodeFrom(aRemovedNode);
                aOriginal->commitDirect(); // tree is detached => commit directly
                aOriginal->makeIndirect(false);
            }
            OSL_ENSURE(aRemovedNode.get() == 0, "Could not revert removed node: Nowhere to put ownership");

            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            OSL_ENSURE(pOriginal, "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected type of element change");

            if (!it->ISA(SubtreeChange)) throw Exception("Unexpected type of element change");

            if (pOriginal)
                (*pOriginal)->recoverFailedCommit(static_cast<SubtreeChange&>(*it));
        }
    }
    m_bChanged = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::implInsertNewElement(Name const& aName, Element const& aNewElement)
{
    attach(aNewElement,aName,false);
    try
    {
        // put the new element into the changed set
        Element* pAddedElement = m_aChangedData.getElement(aName);
        if (pAddedElement)
        {
            OSL_ENSURE(!pAddedElement->isValid(),"WARNING: Element being inserted was already there - replacing");
            detach(m_aChangedData.replaceElement(aName,aNewElement),false);
        }
        else
        {
            m_aChangedData.insertElement(aName, aNewElement);
        }
        m_bChanged = true;
    }
    catch (std::exception&)
    {
        detach(aNewElement,false);
        throw;
    }
}
//-------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::implRemoveOldElement(Name const& aName)
{
    // put an empty (dummy) element into the changed set
    Element* pAddedElement = m_aChangedData.getElement(aName);
    if (pAddedElement)
    {
        OSL_ENSURE(pAddedElement->isValid(),"WARNING: Element being removed was already removed");
        detach(m_aChangedData.replaceElement(aName, Element()),false);
        m_bChanged = true;
    }
    else
    {
        m_aChangedData.insertElement(aName, Element());
    }

    // now check the original one
    Element* pOldElement = getStoredElement(aName);
    if (pOldElement)
    {
        OSL_ASSERT(pOldElement->isValid());
        detach(*pOldElement,false);
        m_bChanged = true;
    }
    else // just clear things out
    {
        m_aChangedData.removeElement(aName);
    }

    OSL_ENSURE(pOldElement || pAddedElement,"WARNING: Element being removed was not found in set");
}
//-----------------------------------------------------------------------------

void DeferredTreeSetNodeImpl::doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (Element* pElement = getStoredElement(aName))
        {
            OSL_ASSERT(pElement->isValid());

            if (aChange.ISA(SubtreeChange))
            {
                SubtreeChange const& aSubtreeChange = static_cast<SubtreeChange const&>(aChange);

                // recurse to element tree - but do not notify those changes (?)

                NodeChangesInformation aIgnoredChanges;
                (*pElement)->adjustToChanges(aIgnoredChanges,aSubtreeChange);
            }
            else
            OSL_ENSURE( false, "Unexpected kind of change to set element" );

        }
        else
        {
            // could be changed to do an insert instead (?)
            OSL_ENSURE( false, "Changed Element didn't exist before it was removed/replaced" );
        }

        if (pLocalElement->isValid())
        {
            // we have a complete replacement for the changed node
            Element aLocalElement = *pLocalElement;

            // also signal something happened
            addLocalChangeHelper(rLocalChanges, NodeChange( doCreateReplace(aName,aLocalElement,aLocalElement) ) );
        }
        else
        {
            // already removed locally - should be notified by different route (if applicable)
        }
    }
    else
    {
        TreeSetNodeImpl::doAdjustChangedElement(rLocalChanges,aName,aChange);
    }
}
//-----------------------------------------------------------------------------

NodeChangeImpl* DeferredTreeSetNodeImpl::doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        // We have another element replacing ours - what do we do ?
        if (Element* pOriginal = getStoredElement(aName))
        {
            OSL_ENSURE( aAddNodeChange.isReplacing(), "Added Element already exists - replacing" );

            TreeSetNodeImpl::implReplaceElement(aName,aNewElement, false);
        }
        else
        {
            OSL_ENSURE( !aAddNodeChange.isReplacing(), "Replaced Element doesn't exist - simply adding" );
            TreeSetNodeImpl::implInsertElement(aName,aNewElement, false);
        }


        if (pLocalElement->isValid()) // ours remains a valid replacement
        {
            Element aLocalElement = *pLocalElement;

            // just signal something happened
            return doCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else // had been removed locally
        {
            // signal what happened
            return doCreateInsert(aName,aNewElement);
        }
    }
    else
    {
        return TreeSetNodeImpl::doAdjustToAddedElement(aName,aAddNodeChange,aNewElement);
    }
}
//-----------------------------------------------------------------------------

NodeChangeImpl* DeferredTreeSetNodeImpl::doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (Element* pOriginal = getStoredElement(aName))
        {
            // take away the original
                TreeSetNodeImpl::implRemoveElement(aName, false);
        }

        if (pLocalElement->isValid()) // remains a valid replacement
        {
            Element aLocalElement = *pLocalElement;

            // signal something happened
            return doCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else // already was removed locally
        {
            return 0;
        }
    }
    else
    {
        return TreeSetNodeImpl::doAdjustToRemovedElement(aName,aRemoveNodeChange);
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DeferredValueSetNodeImpl
//-----------------------------------------------------------------------------

DeferredValueSetNodeImpl::DeferredValueSetNodeImpl(ISubtree& rOriginal, Template* pTemplate)
: ValueSetNodeImpl(rOriginal,pTemplate)
, m_aChangedData()
, m_bChanged(false)
{
}
//-----------------------------------------------------------------------------

DeferredValueSetNodeImpl::DeferredValueSetNodeImpl(DirectValueSetNodeImpl& rOriginal)
: ValueSetNodeImpl(rOriginal)
, m_aChangedData()
, m_bChanged(false)
{
    implMakeIndirect(true);
}
//-----------------------------------------------------------------------------

bool DeferredValueSetNodeImpl::doIsEmpty() const
{
    if (m_aChangedData.isEmpty())
        return ValueSetNodeImpl::doIsEmpty();

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop;
        ++it)
    {
        if (it->isValid()) return false;
    }}


    // look for elements in the base set that are not 'deleted'
    {for(NativeIterator it = ValueSetNodeImpl::beginElementSet(), stop = ValueSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.hasElement(it->first)) return false;
    }}

    return true;
}
//-----------------------------------------------------------------------------

SetEntry DeferredValueSetNodeImpl::doFindElement(Name const& aName)
{
    Element* pElement = m_aChangedData.getElement(aName);
    if (!pElement)
        pElement = ValueSetNodeImpl::getStoredElement(aName);

    return SetEntry(pElement ? pElement->getBodyPtr() : 0);
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doClearElements()
{
    ValueSetNodeImpl::doClearElements();
    m_aChangedData.clearElements();
}
//-----------------------------------------------------------------------------

SetNodeVisitor::Result DeferredValueSetNodeImpl::doDispatchToElements(SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    // look for elements in the base set that are not hidden by changes
    {for(NativeIterator it = ValueSetNodeImpl::beginElementSet(), stop = ValueSetNodeImpl::endElementSet();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            eRet = aVisitor.visit(SetEntry(it->second.getBodyPtr()));
        }
    }}

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (it->isValid())
        {
            eRet = aVisitor.visit(SetEntry(it->getBodyPtr()));
        }
    }}
    return eRet;
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doInsertElement(Name const& aName, SetEntry const& aNewEntry)
{
    implInsertNewElement(aName, ValueSetNodeImpl::implMakeElement(aNewEntry.tree()));
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doRemoveElement(Name const& aName)
{
    implRemoveOldElement(aName);
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doInitElements(ISubtree& rTree, TreeDepth )
{
    ValueSetNodeImpl::initHelper( NodeType::getDeferredChangeFactory(), rTree);
}
//-----------------------------------------------------------------------------

DeferredValueSetNodeImpl::Element DeferredValueSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TreeDepth )
{
    return ValueSetNodeImpl::makeAdditionalElement( NodeType::getDeferredChangeFactory(), aAddNodeChange);
}
//-----------------------------------------------------------------------------

bool DeferredValueSetNodeImpl::doHasChanges() const
{
    return m_bChanged || !m_aChangedData.isEmpty();
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doCollectChanges(NodeChanges& rChanges) const
{
    // collect added and deleted nodes
    {for(NativeIterator it = m_aChangedData.beginNative(), stop = m_aChangedData.endNative();
        it != stop;
        ++it)
    {
        Element const* pOriginal = ValueSetNodeImpl::getStoredElement(it->first);

        if (it->second.isValid()) // added one
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(doCreateReplace(it->first,it->second,*pOriginal)));
            }
            else
            {
                rChanges.add(NodeChange(doCreateInsert(it->first,it->second)));
            }
        }
        else
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(doCreateRemove(it->first,*pOriginal)));
            }

            //else nothing to do
        }
    }}

    // collect preexisting nodes
    {for(NativeIterator it = ValueSetNodeImpl::beginElementSet(), stop = ValueSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            if (it->second->hasChanges())
                it->second->collectChanges(rChanges);
        }
    }}

}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doMarkChanged()
{
    m_bChanged = true;
}
//-----------------------------------------------------------------------------

NodeImplHolder DeferredValueSetNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return this;
    else
        return new DirectValueSetNodeImpl(*this);
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doCommitChanges()
{
    // commit preexisting nodes
    {for(NativeIterator it = ValueSetNodeImpl::beginElementSet(), stop = ValueSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            it->second->commitDirect();
        }
    }}


    // commit added and deleted nodes
    {
        NativeIterator it = m_aChangedData.beginNative();
        NativeIterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            Name    aName       = it->first;
            Element aNewElement = it->second;

            Element* pOriginal = ValueSetNodeImpl::getStoredElement(aName);

            if (aNewElement.isValid())
            {
                if (pOriginal)
                    ValueSetNodeImpl::implReplaceElement(aName,aNewElement,true);

                else
                    ValueSetNodeImpl::implInsertElement(aName,aNewElement,true);

                aNewElement->makeIndirect(true);
            }
            else
            {
                if (pOriginal)
                    ValueSetNodeImpl::implRemoveElement(aName,true);

                //else nothing to do
            }
            if (pOriginal)
            {
                OSL_ASSERT(pOriginal->isValid());
                (*pOriginal)->commitDirect();
                (*pOriginal)->makeIndirect(false);
            }

            ++it;
            m_aChangedData.removeElement(aName);
        }
    }

    m_bChanged = false;
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredValueSetNodeImpl::doPreCommitChanges(ElementList& _rRemovedElements)
{
    // and make a SubtreeChange
    std::auto_ptr<SubtreeChange> pSetChange( new SubtreeChange(this->getOriginalNodeName(),
                                                                getElementTemplate()->getName().toString(),
                                                                getElementTemplate()->getModule().toString(),
                                                                this->getAttributes()) );

    // commit preexisting nodes
    {for(NativeIterator it = ValueSetNodeImpl::beginElementSet(), stop = ValueSetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());

            std::auto_ptr<SubtreeChange> pNewChange( it->second->preCommitChanges(_rRemovedElements) );

            OSL_ENSURE(pNewChange.get() == NULL, "Unexpected change generated by value set element - ignoring that change");
        }
    }}

    // commit added and deleted nodes
    {
        NativeIterator it = m_aChangedData.beginNative();
        NativeIterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            Name    aName       = it->first;
            Element aNewElement = it->second;

            Element* pOriginal = ValueSetNodeImpl::getStoredElement(aName);

            if (aNewElement.isValid())
            {
                std::auto_ptr<INode> aAddedTree;
                aNewElement->releaseTo( aAddedTree );

                OSL_ENSURE( aAddedTree.get(), "Could not take the new tree from the ElementTree");

                AddNode* pAddNode = new AddNode(aAddedTree, aName.toString() );

                std::auto_ptr<Change> pNewChange( pAddNode );

                if (pOriginal)
                    pAddNode->setReplacing();

                pSetChange->addChange(pNewChange);
            }
            else
            {
                if (pOriginal)
                {
                    std::auto_ptr<Change> pNewChange( new RemoveNode(aName.toString()) );

                    pSetChange->addChange(pNewChange);
                }
                //else nothing to do
            }

            // collect removed or replaced element
            if (pOriginal)
                _rRemovedElements.push_back( *pOriginal );

            ++it;
        }
    }
    return pSetChange;
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doFinishCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aElementName = makeElementName(it->getNodeName(), Name::NoValidate());

        Element* pOriginal = getStoredElement(aElementName);

        if (Element* pNewElement = m_aChangedData.getElement(aElementName))
        {
            Element aOriginal;
            if (pOriginal)
            {
                aOriginal = *pOriginal;
                OSL_ASSERT(aOriginal.isValid());
            }
            else
                OSL_ASSERT(!aOriginal.isValid());

            // handle a added, replaced or deleted node
            std::auto_ptr<INode> aRemovedNode;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                aRemovedNode = rAddNode.releaseReplacedNode();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=aRemovedNode.get())  );

                if (aOriginal.isValid())
                    ValueSetNodeImpl::implReplaceElement(aElementName,*pNewElement,false);

                else
                    ValueSetNodeImpl::implInsertElement(aElementName,*pNewElement,false);

                (*pNewElement)->makeIndirect(true);
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                aRemovedNode = rRemoveNode.releaseRemovedNode();

                OSL_ASSERT(aOriginal.isValid());
                if (aOriginal.isValid())
                    ValueSetNodeImpl::implRemoveElement(aElementName,false);
            }
            // handle a added or deleted node
            if (aOriginal.isValid())
            {
                OSL_ENSURE(aRemovedNode.get(), "Cannot take over the removed node");

                aOriginal->takeNodeFrom(aRemovedNode);
                aOriginal->commitDirect(); // tree is detached => commit directly
                aOriginal->makeIndirect(false);
            }
            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // cannot apply changes to preexisting nodes
            OSL_ENSURE(false, "Unexpected: Cannot finish commit - unexpected change for value set element");

            throw Exception("Unexpected value set element change");
        }
    }
    m_bChanged = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doRevertCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aElementName = makeElementName(it->getNodeName(), Name::NoValidate());

        Element* pOriginal = getStoredElement(aElementName);

        if (Element* pNewElement = m_aChangedData.getElement(aElementName))
        {
            // handle a added, replaced or deleted node
            std::auto_ptr<INode> pRemovedNode;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                pRemovedNode = rAddNode.releaseReplacedNode();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=rAddNode.getReplacedNode_Unsafe())  );

                std::auto_ptr<INode> aAddedNode = rAddNode.releaseAddedNode();
                OSL_ENSURE(aAddedNode.get(), "Cannot restore new node: Change lost ownership");

                // restore the tree
                (*pNewElement)->takeNodeFrom(aAddedNode);
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                pRemovedNode = rRemoveNode.releaseRemovedNode();

                OSL_ASSERT(pOriginal);
                OSL_ASSERT((0 != pOriginal) == (0!=rRemoveNode.getRemovedNode_Unsafe())  );
            }
            OSL_ENSURE(pRemovedNode.get() == 0, "Possible problems reverting removed node: Change took ownership");
            // handle a added or deleted node
            if (pOriginal && pRemovedNode.get())
            {
                OSL_ASSERT(pOriginal->isValid());
                (*pOriginal)->takeNodeFrom(pRemovedNode);
            }
            OSL_ENSURE(pRemovedNode.get() == 0, "Could not revert removed node: Nowhere to put ownership");
        }
        else
        {
            // cannot apply changes to preexisting nodes
            OSL_ENSURE(false, "Unexpected: Cannot revert commit - unexpected change for value set element");

            throw Exception("Unexpected value set element change");
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doFailedCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName().toString(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule().toString(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aElementName = makeElementName(it->getNodeName(), Name::NoValidate());

        Element* pOriginal = getStoredElement(aElementName);

        if (Element* pNewElement = m_aChangedData.getElement(aElementName))
        {
            Element aOriginal;
            if (pOriginal)
            {
                aOriginal = *pOriginal;
                OSL_ASSERT(aOriginal.isValid());
            }
            else
                OSL_ASSERT(!aOriginal.isValid());

            // handle a added, replaced or deleted node
            std::auto_ptr<INode> aRemovedNode;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                aRemovedNode = rAddNode.releaseReplacedNode();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=rAddNode.getReplacedNode_Unsafe())  );

                std::auto_ptr<INode> aAddedNode = rAddNode.releaseAddedNode();

                if (aAddedNode.get()) // Change not done; need to restore new node (element will be released into the wild then)
                {
                    (*pNewElement)->takeNodeFrom(aAddedNode);
                    detach(*pNewElement, false);
                }

                else if (getOriginalSetNode().getChild(aElementName.toString()) == rAddNode.getAddedNode_unsafe())
                { // it has been integrated into the master tree

                    // so add it
                    if (aOriginal.isValid())
                        ValueSetNodeImpl::implReplaceElement(aElementName,*pNewElement,false);

                    else
                        ValueSetNodeImpl::implInsertElement(aElementName,*pNewElement,false);

                    (*pNewElement)->makeIndirect(true);
                }
                else
                {
                    OSL_ENSURE(false, "Unexpected: added node is gone, but where ? May cause invalid references");
                    detach(*pNewElement, false);
                    // pNewElement->disposeData()
                }

            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                aRemovedNode = rRemoveNode.releaseRemovedNode();

                OSL_ASSERT(aOriginal.isValid());
                if (aRemovedNode.get() || rRemoveNode.getRemovedNode_Unsafe() != getOriginalSetNode().getChild(aElementName.toString()))
                {
                    // really removed - then remove the originel
                    if (aOriginal.isValid())
                        ValueSetNodeImpl::implRemoveElement(aElementName,false);
                    OSL_ENSURE(NULL == getOriginalSetNode().getChild(aElementName.toString()),"ERROR: Removed Node still there or replaced");
                }
            }

            // handle a added or deleted node
            if (aOriginal.isValid() && aRemovedNode.get())
            {
                aOriginal->takeNodeFrom(aRemovedNode);
                aOriginal->commitDirect(); // tree is detached => commit directly
                aOriginal->makeIndirect(false);
            }
            OSL_ENSURE(aRemovedNode.get() == 0, "Could not revert removed node: Nowhere to put ownership");

            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // cannot handle changes to preexisting nodes
            OSL_ENSURE(false, "Unexpected: Cannot finish commit - unexpected change for value set element");

            throw Exception("Unexpected value set element change");
        }
    }
    m_bChanged = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::implInsertNewElement(Name const& aName, Element const& aNewElement)
{
    attach(aNewElement,aName,false);
    try
    {
        // put the new element into the changed set
        Element* pAddedElement = m_aChangedData.getElement(aName);
        if (pAddedElement)
        {
            OSL_ENSURE(!pAddedElement->isValid(),"WARNING: Element being inserted was already there - replacing");
            detach(m_aChangedData.replaceElement(aName,aNewElement),false);
        }
        else
        {
            m_aChangedData.insertElement(aName, aNewElement);
        }
        m_bChanged = true;
    }
    catch (std::exception&)
    {
        detach(aNewElement,false);
        throw;
    }
}
//-------------------------------------------------------------------------

void DeferredValueSetNodeImpl::implRemoveOldElement(Name const& aName)
{
    // put an empty (dummy) element into the changed set
    Element* pAddedElement = m_aChangedData.getElement(aName);
    if (pAddedElement)
    {
        OSL_ENSURE(pAddedElement->isValid(),"WARNING: Element being removed was already removed");
        detach(m_aChangedData.replaceElement(aName, Element()),false);
        m_bChanged = true;
    }
    else
    {
        m_aChangedData.insertElement(aName, Element());
    }

    // now check the original one
    Element* pOldElement = getStoredElement(aName);
    if (pOldElement)
    {
        OSL_ASSERT(pOldElement->isValid());
        detach(*pOldElement,false);
        m_bChanged = true;
    }
    else // just clear things out
    {
        m_aChangedData.removeElement(aName);
    }

    OSL_ENSURE(pOldElement || pAddedElement,"WARNING: Element being removed was not found in set");
}
//-----------------------------------------------------------------------------

void DeferredValueSetNodeImpl::doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (Element* pElement = getStoredElement(aName))
        {
            OSL_ASSERT(pElement->isValid());
            OSL_ENSURE( aChange.ISA(ValueChange), "Unexpected kind of change to value set element" );

        }
        else
        {
            // could be changed to do an insert instead (?)
            OSL_ENSURE( false, "Changed Element didn't exist before it was removed/replaced" );
        }

        if (pLocalElement->isValid())
        {
            // we have a complete replacement for the changed node
            Element aLocalElement = *pLocalElement;

            // also signal something happened
            addLocalChangeHelper( rLocalChanges, NodeChange( doCreateReplace(aName,aLocalElement,aLocalElement) ) );
        }
        else
        {
            // already removed locally - should be notified by different route (if applicable)
        }
    }
    else
    {
        ValueSetNodeImpl::doAdjustChangedElement(rLocalChanges,aName,aChange);
    }
}
//-----------------------------------------------------------------------------

NodeChangeImpl* DeferredValueSetNodeImpl::doAdjustToAddedElement(Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        // We have another element replacing ours - what do we do ?
        if (Element* pOriginal = getStoredElement(aName))
        {
            OSL_ENSURE( aAddNodeChange.isReplacing(), "Added Element already exists - replacing" );

            ValueSetNodeImpl::implReplaceElement(aName,aNewElement, false);
        }
        else
        {
            OSL_ENSURE( !aAddNodeChange.isReplacing(), "Replaced Element doesn't exist - simply adding" );
            ValueSetNodeImpl::implInsertElement(aName,aNewElement, false);
        }


        if (pLocalElement->isValid()) // ours remains a valid replacement
        {
            Element aLocalElement = *pLocalElement;

            // just signal something happened
            return doCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else // had been removed locally
        {
            // signal what happened
            return doCreateInsert(aName,aNewElement);
        }
    }
    else
    {
        return ValueSetNodeImpl::doAdjustToAddedElement(aName,aAddNodeChange,aNewElement);
    }
}
//-----------------------------------------------------------------------------

NodeChangeImpl* DeferredValueSetNodeImpl::doAdjustToRemovedElement(Name const& aName, RemoveNode const& aRemoveNodeChange)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (Element* pOriginal = getStoredElement(aName))
        {
            // take away the original
                ValueSetNodeImpl::implRemoveElement(aName, false);
        }

        if (pLocalElement->isValid()) // remains a valid replacement
        {
            Element aLocalElement = *pLocalElement;

            // signal something happened
            return doCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else // already was removed locally
        {
            return 0;
        }
    }
    else
    {
        return ValueSetNodeImpl::doAdjustToRemovedElement(aName,aRemoveNodeChange);
    }
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
    }
}
