/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeimplobj.cxx,v $
 * $Revision: 1.26 $
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
#include "nodeimplobj.hxx"


#include "nodechange.hxx"
#include "nodechangeinfo.hxx"
#include "nodechangeimpl.hxx"
#include "valuenode.hxx"
#include "change.hxx"
#include "viewaccess.hxx"
#include "viewfactory.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// Value Nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DeferredValueElementNodeImpl
//-----------------------------------------------------------------------------

// Group Nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DeferredGroupNodeImpl
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::DeferredGroupNodeImpl(sharable::GroupNode * const& _aNodeRef)
: GroupNodeImpl(_aNodeRef)
, m_aChanges()
{
}
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::~DeferredGroupNodeImpl()
{
}
//-----------------------------------------------------------------------------

ValueMemberNode DeferredGroupNodeImpl::makeValueMember(rtl::OUString const& _aName, bool _bForUpdate)
{
    MemberChanges::iterator it = m_aChanges.find(_aName);

    if (it != m_aChanges.end())
    {
        if (!it->second.is())
            OSL_ENSURE(_aName.getLength() == 0, "ERROR: Found empty change reference");

        else if (_bForUpdate || it->second->isChange()) // found one
            return ValueMemberNode(it->second);

        else // leftover non-change
            m_aChanges.erase(it);

        // if not found continue with default
    }

    sharable::ValueNode * original = getOriginalValueNode(_aName);

    if (_bForUpdate) // create a new change
    {
        if (original != 0)
        {
            rtl::Reference<ValueMemberNode::DeferredImpl> aNewChange(new ValueMemberNode::DeferredImpl(original));
            m_aChanges[_aName] = aNewChange;

            return ValueMemberNode(aNewChange);
       }
    }

    return GroupNodeImpl::makeValueMember(original);
}
//-----------------------------------------------------------------------------

bool DeferredGroupNodeImpl::hasChanges() const
{
    for (MemberChanges::const_iterator it = m_aChanges.begin(); it != m_aChanges.end(); ++it)
    {
        if (!it->second.is())
        {
            // empty element is present -> marked as changed
            OSL_ASSERT(it->first.getLength() == 0);
            return true;
        }

        if (it->second->isChange())
            return true;
    }

    return false;
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::collectValueChanges(NodeChanges& rChanges, Tree* pParentTree, unsigned int nNode) const
{
    for (MemberChanges::const_iterator it = m_aChanges.begin(); it != m_aChanges.end(); ++it)
    {
        if (it->second.is())
        {
            OSL_ASSERT(it->first.getLength() != 0);
            if (ValueChangeImpl* pValueChange = it->second->collectChange())
            {
                pValueChange->setTarget(pParentTree,nNode,it->first);

                rChanges.add( NodeChange(pValueChange) );
            }
            else // leftover non-change
                OSL_ENSURE(!it->second->isChange(), "Got no change from a changing value") ;
        }
        else
            OSL_ASSERT(it->first.getLength() == 0);
    }
}
//-----------------------------------------------------------------------------

rtl::Reference<ValueMemberNode::DeferredImpl> DeferredGroupNodeImpl::findValueChange(rtl::OUString const& aName)
{
    rtl::Reference<ValueMemberNode::DeferredImpl> aResult;

    MemberChanges::iterator it = m_aChanges.find(aName);

    if (it != m_aChanges.end())
    {
        if (it->second.is() )
        {
            if (it->second->isChange())
            {
                aResult = it->second;
            }

            else // leftover non-change -> drop
            {
                m_aChanges.erase(it);
            }
        }
        else
            OSL_ENSURE(aName.getLength() == 0, "ERROR: Found empty change reference");
    }

    return aResult;
}

//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredGroupNodeImpl::preCommitValueChanges()
{
    std::auto_ptr<SubtreeChange> aRet;

    if (!m_aChanges.empty())
    {
        sharable::Node * originalData = this->getOriginalNodeAccess();
        aRet.reset( new SubtreeChange(  originalData->getName(),
                        originalData->getAttributes() ) );

        for (MemberChanges::iterator pos = m_aChanges.begin(); pos != m_aChanges.end(); )
        {
            MemberChanges::iterator it = pos++; // this is used to allow erasing below

            if (!it->second.is())
            {
                OSL_ASSERT(it->first.getLength() == 0);
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

void DeferredGroupNodeImpl::finishCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        rtl::OUString aValueName(it->getNodeName());

        MemberChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            ValueChange * valueChange = dynamic_cast< ValueChange * >(&*it);
            OSL_ENSURE(valueChange != 0, "Unexpected type of element change");
            if (valueChange == 0) throw Exception("Unexpected type of element change");

            rtl::Reference<ValueMemberNode::DeferredImpl> aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.is(), "Found empty change object for Member value change");

            if (aStoredChange.is())
            {
                aStoredChange->finishCommit(*valueChange);
                OSL_ENSURE(!aStoredChange->isChange(),"ValueChange is not moot after finishCommit");
            }

            m_aChanges.erase( itStoredChange ); // remove finished change
        }
        else
            OSL_ENSURE(dynamic_cast< ValueChange * >(&*it) == 0, "Value member change has no change data representation");

    }

    OSL_DEBUG_ONLY( m_aChanges.erase( rtl::OUString() ) ); // remove change marker (if present)
    OSL_ENSURE(m_aChanges.empty(), "Found unprocessed changes to values in group");

    m_aChanges.clear(); // remove all pending stuff and marker
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::revertCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        rtl::OUString aValueName(it->getNodeName());

        MemberChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            ValueChange * valueChange = dynamic_cast< ValueChange * >(&*it);
            OSL_ENSURE(valueChange != 0, "Unexpected type of element change");
            if (valueChange == 0) continue;

            rtl::Reference<ValueMemberNode::DeferredImpl> aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.is(), "Cannot restore change: found empty change object for Member value change");

            if (aStoredChange.is())
            {
                aStoredChange->revertCommit(*valueChange);
                OSL_ENSURE(!aStoredChange->isChange(),"ValueChange is not moot after reverting - will be discarded nevertheless");
            }
            m_aChanges.erase( itStoredChange ); // remove change if it is moot
        }
        else
            OSL_ENSURE(dynamic_cast< ValueChange * >(&*it) == 0, "Value member change has no change data representation");
    }
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::failedCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        rtl::OUString aValueName(it->getNodeName());

        MemberChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            ValueChange * valueChange = dynamic_cast< ValueChange * >(&*it);
            OSL_ENSURE(valueChange != 0, "Unexpected type of element change");
            if (valueChange == 0) continue;

            rtl::Reference<ValueMemberNode::DeferredImpl> aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.is(), "Cannot recover from failed change: found empty change object for Member value change");

            if (aStoredChange.is())
                 aStoredChange->failedCommit(*valueChange);
           {
                if (!aStoredChange->isChange())
                    m_aChanges.erase( itStoredChange ); // remove change if it is moot
            }
        }
        else
            OSL_ENSURE(dynamic_cast< ValueChange * >(&*it) == 0, "Value member change has no change data representation");
    }

    OSL_DEBUG_ONLY( m_aChanges.erase( rtl::OUString() ) ); // remove change marker (if present)
    OSL_ENSURE(m_aChanges.empty(), "RevertCommit: Found unprocessed changes to values in group");

    m_aChanges.clear(); // discard all pending stuff and marker
}
//-----------------------------------------------------------------------------


void DeferredGroupNodeImpl::markChanged()
{
    // special mark: a NULL rtl::Reference<ValueMemberNode::DeferredImpl> at empty name
    m_aChanges.insert( MemberChanges::value_type() );
}
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class DeferredTreeSetNodeImpl
//-----------------------------------------------------------------------------

DeferredSetNodeImpl::DeferredSetNodeImpl(sharable::SetNode * const& _aNodeRef, Template* pTemplate)
: SetNodeImpl(_aNodeRef,pTemplate)
, m_aChangedData()
, m_bChanged(false)
, m_bDefault(false)
{
}
//-----------------------------------------------------------------------------

bool DeferredSetNodeImpl::doIsEmpty() const
{
    if (m_aChangedData.isEmpty())
        return SetNodeImpl::doIsEmpty();

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop;
        ++it)
    {
        if (it->isValid()) return false;
    }}


    // look for elements in the base set that are not 'deleted' (the changes are all deletions here)
    {for(ElementSet::Data::const_iterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (!m_aChangedData.hasElement(it->first)) return false;
    }}

    return true;
}
//-----------------------------------------------------------------------------

ElementTree* DeferredSetNodeImpl::doFindElement(rtl::OUString const& aName)
{
    ElementTreeData* pElement = m_aChangedData.getElement(aName);
    if (!pElement)
        pElement = SetNodeImpl::getStoredElement(aName);

    return pElement ? pElement->get() : 0;
}
//-----------------------------------------------------------------------------

SetNodeVisitor::Result DeferredSetNodeImpl::doDispatchToElements(SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    // look for elements in the base set that are not hidden by changes
    {for(ElementSet::Data::const_iterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            eRet = aVisitor.visit(SetEntry(it->second.get()));
        }
    }}

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (it->isValid())
        {
            eRet = aVisitor.visit(SetEntry(it->get()));
        }
    }}
    return eRet;
}
//-----------------------------------------------------------------------------

bool DeferredSetNodeImpl::hasChanges() const
{
    return m_bChanged || !m_aChangedData.isEmpty();
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::collectElementChanges(NodeChanges& rChanges) const
{
    // collect added and deleted nodes
    {for(ElementSet::Data::const_iterator it = m_aChangedData.beginNative(), stop = m_aChangedData.endNative();
        it != stop;
        ++it)
    {
        ElementTreeData const* pOriginal = SetNodeImpl::getStoredElement(it->first);

        if (it->second.isValid()) // added one
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(implCreateReplace(it->first,it->second,*pOriginal)));
            }
            else
            {
                rChanges.add(NodeChange(implCreateInsert(it->first,it->second)));
            }
        }
        else
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(implCreateRemove(it->first,*pOriginal)));
            }

            //else nothing to do
        }
    }}

    // collect preexisting nodes
    // if (!containsValues()) // value elements ar immutable !
    {for(ElementSet::Data::const_iterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            view::ViewTreeAccess aElementView(it->second.get());

            if (aElementView.hasChanges())
                aElementView.collectChanges(rChanges);
        }
    }}

}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::markChanged()
{
    m_bChanged = true;
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::doTransferElements(ElementSet& rReplacement)
{
    // transfer preexisting nodes (unless replaced/deleted)
    {for(ElementSet::Data::const_iterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());

            rReplacement.insertElement(it->first,it->second);
        }
    }}

    // commit added and deleted nodes
    {
        ElementSet::Data::const_iterator it = m_aChangedData.beginNative();
        ElementSet::Data::const_iterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            if (it->second.isValid())
                rReplacement.insertElement(it->first,it->second);

            ++it;
            m_aChangedData.removeElement(it->first);
        }
    }

    m_bChanged = false;
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::rebuildElement(rtl::OUString const& _aName, ElementTreeData const& _aElement)
{
    Tree* pContext = this->getParentTree();
    OSL_ENSURE(pContext, "Context tree must be set before rebuilding");

    rtl::Reference<view::ViewStrategy> xContextBehavior = pContext->getViewBehavior();

    sharable::TreeFragment * elementAccessor = this->getDataAccess()->getElement(_aName);
    OSL_ENSURE(elementAccessor != 0, "Element Tree not found in data");

    OSL_ENSURE(_aElement.isValid(), "Element not found in view");
    _aElement->rebuild(xContextBehavior, elementAccessor);
}

//-----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> DeferredSetNodeImpl::preCommitChanges(std::vector< rtl::Reference<ElementTree> >& _rRemovedElements)
{
    sharable::Node * originalData = this->getOriginalNodeAccess();
    // now first get the name of this node
    rtl::OUString sSetName = originalData->getName();

    // and make a SubtreeChange
    std::auto_ptr<SubtreeChange> pSetChange( new SubtreeChange(sSetName,
                                   getElementTemplate()->getName(),
                                   getElementTemplate()->getModule(),
                                   originalData->getAttributes() ) );

    // commit preexisting nodes
    {
    for(ElementSet::Data::const_iterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
        OSL_ASSERT(it->second.isValid());
        OSL_ENSURE( !m_bDefault || it->second.inDefault, "m_bDefault is inconsistent");

        view::ViewTreeAccess aElementView(it->second.get());
        std::auto_ptr<SubtreeChange> pNewChange = aElementView.preCommitChanges(_rRemovedElements);
        if (pNewChange.get() != 0)
        {
            //OSL_ENSURE( !containsValues(), "Unexpected change generated by value set element");
          std::auto_ptr<Change> pNewChangeBase( pNewChange.release() );
          pSetChange->addChange(pNewChangeBase);
        }
        }
    }
    }

    // commit added and deleted nodes
    {
        ElementSet::Data::const_iterator it = m_aChangedData.beginNative();
        ElementSet::Data::const_iterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            rtl::OUString   aName       = it->first;
            ElementTreeData aNewElement = it->second;

            ElementTreeData* pOriginal = SetNodeImpl::getStoredElement(aName);

            if (aNewElement.isValid())
            {
                rtl::Reference< data::TreeSegment > aAddedTree = aNewElement->releaseOwnedTree();
                if (!aAddedTree.is())
                {
                    throw Exception("INTERNAL ERROR: Could not find data for the added ElementTree");
                }

                OSL_ENSURE( !m_bDefault || aNewElement.inDefault, "m_bDefault is inconsistent");

                AddNode* pAddNode = new AddNode(aAddedTree, aName, aNewElement.inDefault );

                std::auto_ptr<Change> pNewChange( pAddNode );

                if (pOriginal)
                    pAddNode->setReplacing();

                pSetChange->addChange(pNewChange);
            }
            else
            {
                if (pOriginal)
                {
                    OSL_ENSURE( !m_bDefault || aNewElement.inDefault, "m_bDefault is inconsistent");

                    std::auto_ptr<Change> pNewChange( new RemoveNode(aName,aNewElement.inDefault) );

                    pSetChange->addChange(pNewChange);
                }
                //else nothing to do
            }

            // collect removed or replaced element
            if (pOriginal)
                _rRemovedElements.push_back( pOriginal->tree );

            ++it;
        }
    }
    return pSetChange;
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::finishCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule(),
                "ERROR: Element template module of change does not match the template of the set");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        rtl::OUString aElementName(it->getNodeName());

        ElementTreeData* pOriginal = getStoredElement(aElementName);

        if (ElementTreeData* pNewElement = m_aChangedData.getElement(aElementName))
        {
            ElementTreeData aOriginal;
            if (pOriginal)
            {
                aOriginal = *pOriginal;
                OSL_ASSERT(aOriginal.isValid());
            }
            else
                OSL_ASSERT(!aOriginal.isValid());

            // handle a added, replaced or deleted node
            rtl::Reference< data::TreeSegment > aRemovedTree;

            if (pNewElement->isValid())
            {
                AddNode * addNode = dynamic_cast< AddNode * >(&*it);
                OSL_ENSURE(addNode != 0, "Unexpected type of element change");
                if (addNode == 0) throw Exception("Unexpected type of element change");

                aRemovedTree = addNode->getReplacedTree();
                OSL_ASSERT( addNode->isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( addNode->isReplacing() == (bool) aRemovedTree.is() );

                if (aOriginal.isValid())
                    SetNodeImpl::replaceElement(aElementName,*pNewElement);

                else
                    SetNodeImpl::insertElement(aElementName,*pNewElement);

                this->rebuildElement(aElementName,*pNewElement);
            }
            else
            {
                RemoveNode * removeNode = dynamic_cast< RemoveNode * >(&*it);
                OSL_ENSURE(removeNode != 0, "Unexpected type of element change");
                if (removeNode == 0) throw Exception("Unexpected type of element change");

                aRemovedTree = removeNode->getRemovedTree();

                OSL_ASSERT(aOriginal.isValid());
                if (aOriginal.isValid())
                    SetNodeImpl::removeElement(aElementName);
            }
            // handle a added or deleted node
            if (aOriginal.isValid())
            {
                OSL_ENSURE(aRemovedTree.is(), "Cannot take over the removed node");

                aOriginal->takeTreeAndRebuild(aRemovedTree);
            }
            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            //OSL_ENSURE(!containsValues(), "Unexpected change to value set element");
            OSL_ENSURE(pOriginal && pOriginal->isValid(), "Changed Element is missing");
            SubtreeChange * subtreeChange = dynamic_cast< SubtreeChange * >(&*it);
            OSL_ENSURE(subtreeChange != 0, "Unexpected type of element change");

            if (subtreeChange == 0) throw Exception("Unexpected set element change");

            if (pOriginal && pOriginal->isValid())
                view::ViewTreeAccess(pOriginal->get()).finishCommit(*subtreeChange);
        }
    }
    m_bChanged = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::revertCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule(),
                "ERROR: Element template module of change does not match the template of the set");


    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        rtl::OUString aElementName(it->getNodeName());

        ElementTreeData* pOriginal = getStoredElement(aElementName);

        if (ElementTreeData* pNewElement = m_aChangedData.getElement(aElementName))
        {
            // handle a added, replaced or deleted node
            rtl::Reference< data::TreeSegment > pRemovedTree;

            if (pNewElement->isValid())
            {
                AddNode * addNode = dynamic_cast< AddNode * >(&*it);
                OSL_ENSURE(addNode != 0, "Unexpected type of element change");
                if (addNode == 0) throw Exception("Unexpected type of element change");

                pRemovedTree = addNode->getReplacedTree();
                OSL_ASSERT( addNode->isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( addNode->isReplacing() == (0!=pRemovedTree.is())  );

                OSL_ENSURE(!addNode->wasInserted(), "Cannot retract new node: Change was integrated");

                rtl::Reference< data::TreeSegment > aAddedTree = addNode->getNewTree();
                OSL_ENSURE(aAddedTree.is(), "Cannot restore new node: Change lost ownership");

                // restore the tree
                (*pNewElement)->takeTreeBack(aAddedTree);
            }
            else
            {
                RemoveNode * removeNode = dynamic_cast< RemoveNode * >(&*it);
                OSL_ENSURE(removeNode != 0, "Unexpected type of element change");
                if (removeNode == 0) throw Exception("Unexpected type of element change");

                pRemovedTree = removeNode->getRemovedTree();

                OSL_ASSERT(pOriginal);
                OSL_ASSERT((0 != pOriginal) == (0!=pRemovedTree.is())  );
            }
            OSL_ENSURE(pRemovedTree.is(), "Possible problems reverting removed node: Change took ownership");
            // try handle a added or deleted node
            if (pOriginal)
            {
                OSL_ASSERT(pOriginal->isValid());
                (*pOriginal)->takeTreeAndRebuild(pRemovedTree);
                OSL_DEBUG_ONLY(pRemovedTree.clear());
            }
            OSL_ENSURE(!pRemovedTree.is(), "Could not revert removed node: Nowhere to put ownership");
        }
        else
        {
            // handle preexisting nodes
            //OSL_ENSURE(!containsValues(), "Unexpected change to value set element");
            OSL_ENSURE(pOriginal && pOriginal->isValid(), "Changed Element is missing");
            SubtreeChange * subtreeChange = dynamic_cast< SubtreeChange * >(&*it);
            OSL_ENSURE(subtreeChange != 0, "Unexpected set element change");

            if (subtreeChange == 0) throw Exception("Unexpected set element change");

            if (pOriginal && pOriginal->isValid())
                view::ViewTreeAccess(pOriginal->get()).revertCommit(*subtreeChange);
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::failedCommit(SubtreeChange& rChanges)
{
    OSL_ENSURE(rChanges.isSetNodeChange(),"ERROR: Change type GROUP does not match set");
    OSL_ENSURE( rChanges.getElementTemplateName() ==  getElementTemplate()->getName(),
                "ERROR: Element template of change does not match the template of the set");
    OSL_ENSURE( rChanges.getElementTemplateModule() ==  getElementTemplate()->getModule(),
                "ERROR: Element template module of change does not match the template of the set");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        rtl::OUString aElementName(it->getNodeName());

        ElementTreeData* pOriginal = getStoredElement(aElementName);

        if (ElementTreeData* pNewElement = m_aChangedData.getElement(aElementName))
        {
            ElementTreeData aOriginal;
            if (pOriginal)
            {
                aOriginal = *pOriginal;
                OSL_ASSERT(aOriginal.isValid());
            }
            else
                OSL_ASSERT(!aOriginal.isValid());

            // handle a added, replaced or deleted node
            rtl::Reference< data::TreeSegment > aRemovedTree;

            if (pNewElement->isValid())
            {
                AddNode * addNode = dynamic_cast< AddNode * >(&*it);
                OSL_ENSURE(addNode != 0, "Unexpected type of element change");
                if (addNode == 0) throw Exception("Unexpected type of element change");

                aRemovedTree = addNode->getReplacedTree();
                OSL_ASSERT( addNode->isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( addNode->isReplacing() == (bool) aRemovedTree.is() );

                if (addNode->wasInserted())
                { // it has been integrated into the master tree
                    OSL_ENSURE(getDataAccess()->getElement(aElementName) == addNode->getInsertedTree(),
                           "Internal Error: Inserted tree address does not match actual data");

                    // so add it
                    if (aOriginal.isValid())
                        SetNodeImpl::replaceElement(aElementName,*pNewElement);

                    else
                        SetNodeImpl::insertElement(aElementName,*pNewElement);

                    this->rebuildElement(aElementName,*pNewElement);
                }
                else // Change not done; need to restore new node (element will be released into the wild then)
                {
                    rtl::Reference< data::TreeSegment > aAddedTree = addNode->getNewTree();

                    OSL_ENSURE(aAddedTree.is(), "Unexpected: added node is gone, but where ? May cause invalid references");
                    (*pNewElement)->takeTreeBack(aAddedTree);
                    detach(*pNewElement);
                }
            }
            else
            {
                RemoveNode * removeNode = dynamic_cast< RemoveNode * >(&*it);
                OSL_ENSURE(removeNode != 0, "Unexpected type of element change");
                if (removeNode == 0) throw Exception("Unexpected type of element change");

                aRemovedTree = removeNode->getRemovedTree();

                OSL_ASSERT(aOriginal.isValid());
                if (aRemovedTree.is() && getDataAccess()->getElement(aElementName) == 0)
                {
                    // really removed - then remove the originel
                    if (aOriginal.isValid())
                        SetNodeImpl::removeElement(aElementName);
                }
            }

            // handle a added or deleted node
            if (aOriginal.isValid() && aRemovedTree.is())
            {
              aOriginal->takeTreeAndRebuild(aRemovedTree);
                //aOriginal->getAccess().makeDirect();
                OSL_DEBUG_ONLY(aRemovedTree.clear());
            }
            OSL_ENSURE(!aRemovedTree.is(), "Could not revert removed node: Nowhere to put ownership");

            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            //OSL_ENSURE(!containsValues(), "Unexpected change to value set element");
            OSL_ENSURE(pOriginal && pOriginal->isValid(), "Changed Element is missing");
            SubtreeChange * subtreeChange = dynamic_cast< SubtreeChange * >(&*it);
            OSL_ENSURE(subtreeChange != 0, "Unexpected set element change");

            if (subtreeChange == 0) throw Exception("Unexpected set element change");

            if (pOriginal && pOriginal->isValid())
                view::ViewTreeAccess(pOriginal->get()).recoverFailedCommit(*subtreeChange);
        }
    }
    m_bChanged = false;
    m_bDefault = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::insertNewElement(rtl::OUString const& aName, ElementTreeData const& aNewElement)
{
    attach(aNewElement,aName);
    try
    {
        // put the new element into the changed set
        ElementTreeData* pAddedElement = m_aChangedData.getElement(aName);
        if (pAddedElement)
        {
            OSL_ENSURE(!pAddedElement->isValid(),"WARNING: Element being inserted was already there - replacing");
            detach(m_aChangedData.replaceElement(aName,aNewElement));
        }
        else
        {
            m_aChangedData.insertElement(aName, aNewElement);
        }
        m_bChanged = true;
        m_bDefault = false;
    }
    catch (std::exception&)
    {
        detach(aNewElement);
        throw;
    }
}
//-------------------------------------------------------------------------

void DeferredSetNodeImpl::removeOldElement(rtl::OUString const& aName)
{
    // put an empty (dummy) element into the changed set
    ElementTreeData* pAddedElement = m_aChangedData.getElement(aName);
    if (pAddedElement)
    {
        OSL_ENSURE(pAddedElement->isValid(),"WARNING: Element being removed was already removed");
        detach(m_aChangedData.replaceElement(aName, ElementTreeData()));
        m_bChanged = true;
        m_bDefault = false;
    }
    else
    {
        m_aChangedData.insertElement(aName, ElementTreeData());
    }

    // now check the original one
    ElementTreeData* pOldElement = getStoredElement(aName);
    if (pOldElement)
    {
        OSL_ASSERT(pOldElement->isValid());
        detach(*pOldElement);
        m_bChanged = true;
        m_bDefault = false;
    }
    else // just clear things out
    {
        m_aChangedData.removeElement(aName);
    }

    OSL_ENSURE(pOldElement || pAddedElement,"WARNING: Element being removed was not found in set");
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* DeferredSetNodeImpl::doAdjustChangedElement(NodeChangesInformation& rLocalChanges, rtl::OUString const& aName, Change const& aChange)
{
    if (ElementTreeData* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (ElementTreeData* pElement = getStoredElement(aName))
        {
            OSL_ASSERT(pElement->isValid());

            if (SubtreeChange const * subtreeChange = dynamic_cast< SubtreeChange const * >(&aChange))
            {
                // recurse to element tree - but do not notify those changes (?)
                Tree * elementTree = pElement->get();

                NodeChangesInformation aIgnoredChanges;
                view::getViewBehavior(elementTree)->adjustToChanges(aIgnoredChanges, view::getRootNode(elementTree), *subtreeChange);
            }
            else
            {
                OSL_ENSURE(dynamic_cast<ValueChange const * >(&aChange) != 0, "Unexpected kind of change to value set element" );
                //OSL_ENSURE( containsValues(), "Unexpected kind of change: Value change applied to tree set element" );
            }

        }
        else
        {
            // could be changed to do an insert instead (?)
            OSL_ENSURE( false, "Changed Element didn't exist before it was removed/replaced" );
        }

        if (pLocalElement->isValid())
        {
            // we have a complete replacement for the changed node
            ElementTreeData aLocalElement = *pLocalElement;

            // also signal something happened
            return implCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else
        {
            // already removed locally - should be notified by different route (if applicable)
            return NULL;
        }
    }
    else
    {
        return SetNodeImpl::doAdjustChangedElement( rLocalChanges,aName,aChange);
    }
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* DeferredSetNodeImpl::doAdjustToAddedElement(rtl::OUString const& aName, AddNode const& aAddNodeChange, ElementTreeData const& aNewElement)
{
    m_bDefault = false;
    if (ElementTreeData* pLocalElement = m_aChangedData.getElement(aName))
    {
        // We have another element replacing ours - what do we do ?
        if (hasStoredElement(aName))
        {
            OSL_ENSURE( aAddNodeChange.isReplacing(), "Added Element already exists - replacing" );

            this->replaceElement(aName,aNewElement);
        }
        else
        {
            OSL_ENSURE( !aAddNodeChange.isReplacing(), "Replaced Element doesn't exist - simply adding" );
            this->insertElement(aName,aNewElement);
        }


        if (pLocalElement->isValid()) // ours remains a valid replacement
        {
            ElementTreeData aLocalElement = *pLocalElement;

            // just signal something happened
            return implCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else // had been removed locally
        {
            // signal what happened
            return implCreateInsert(aName,aNewElement);
        }
    }
    else
    {
        return SetNodeImpl::implAdjustToAddedElement(aName,aNewElement,aAddNodeChange.isReplacing());
    }
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* DeferredSetNodeImpl::doAdjustToRemovedElement(rtl::OUString const& aName, RemoveNode const& /*aRemoveNodeChange*/)
{
    m_bDefault = false;
    if (ElementTreeData* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (hasStoredElement(aName))
        {
            // take away the original
            this->removeElement(aName);
        }

        if (pLocalElement->isValid()) // remains a valid replacement
        {
            ElementTreeData aLocalElement = *pLocalElement;

            // signal something happened
            return implCreateReplace(aName,aLocalElement,aLocalElement);
        }
        else // already was removed locally
        {
            return 0;
        }
    }
    else
    {
        return SetNodeImpl::implAdjustToRemovedElement(aName);
    }
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::doDifferenceToDefaultState(SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree)
{
    if (!m_bDefault)
    {
        implDifferenceToDefaultState(_rChangeToDefault,_rDefaultTree);

        ElementSet::Data::const_iterator it = m_aChangedData.beginNative();
        ElementSet::Data::const_iterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            rtl::OUString   aName       = it->first;
            ElementTreeData aElement    = it->second;

            Change* pChange = _rChangeToDefault.getChange( aName );
            OSL_ENSURE(pChange == NULL || dynamic_cast< AddNode * >(pChange) != 0 || dynamic_cast< RemoveNode * >(pChange) != 0,
                        "Unexpected change type found in difference to default tree");

            if (pChange == NULL)
            {
                std::auto_ptr<INode> aDefaultNode = _rDefaultTree.removeChild(aName);
                OSL_ENSURE( aDefaultNode.get(), "Error: unused Default tree not found after SetNodeImpl::implDifferenceToDefaultState");

                rtl::OUString aElementTypeName = _rDefaultTree.getElementTemplateName();
                OSL_ENSURE( _rDefaultTree.isSetNode(), "Error: missing set template information in default data");

                rtl::Reference< data::TreeSegment > aDefaultTree = data::TreeSegment::create(aDefaultNode,aElementTypeName);
                OSL_ENSURE(aDefaultTree.is(), "Error: unused Default tree not accessible after SetNodeImpl::implDifferenceToDefaultState");

                AddNode* pAddIt = new AddNode(aDefaultTree, aName, true );

                std::auto_ptr<Change> pNewChange( pAddIt );

                if (aElement.isValid())
                {
                    OSL_ENSURE(!aElement.inDefault, "Default element replaced by default");
                    pAddIt->setReplacing();
                }

                _rChangeToDefault.addChange(pNewChange);

            }
            else if (AddNode * addNode = dynamic_cast< AddNode * >(pChange))
            {
                // adjust the AddNode - remove the original expected node
                addNode->clearReplacedTree();

                if (aElement.isValid())
                {
                    if (aElement.inDefault)
                    {
                        // change already done locally
                        _rChangeToDefault.removeChange(aName);
                    }
                    else // adjust here
                        addNode->setReplacing();
                }

                else
                    OSL_ENSURE(!addNode->isReplacing(),"Could not unmark the 'replacing' state of an AddNode");
            }
            else if (RemoveNode * removeNode = dynamic_cast< RemoveNode * >(pChange))
            {
                if (aElement.isValid())
                {
                    OSL_ENSURE(!aElement.inDefault, "Default element replaced by default");
                    // adjust the RemoveNode - remove the original expected node
                    removeNode->clearRemovedTree();
                }
                else
                {
                    // change already done locally
                    _rChangeToDefault.removeChange(aName);
                }
                // TODO: mark local removal as to-default
            }
        }
    }
}
//-----------------------------------------------------------------------------
    }
}
