/*************************************************************************
 *
 *  $RCSfile: nodeimplobj.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:41:10 $
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


#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#include "nodechangeinfo.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif

#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef CONFIGMGR_VIEWACCESS_HXX_
#include "viewaccess.hxx"
#endif
#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif

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

DeferredGroupNodeImpl::DeferredGroupNodeImpl(data::GroupNodeAddress const& _aNodeRef)
: GroupNodeImpl(_aNodeRef)
, m_aChanges()
{
}
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::~DeferredGroupNodeImpl()
{
}
//-----------------------------------------------------------------------------

ValueMemberNode DeferredGroupNodeImpl::makeValueMember(data::Accessor const& _aAccessor, Name const& _aName, bool _bForUpdate)
{
    MemberChanges::iterator it = m_aChanges.find(_aName);

    if (it != m_aChanges.end())
    {
        if (!it->second.is())
            OSL_ENSURE(_aName.isEmpty(), "ERROR: Found empty change reference");

        else if (_bForUpdate || it->second->isChange()) // found one
            return ValueMemberNode(_aAccessor, it->second);

        else // leftover non-change
            m_aChanges.erase(it);

        // if not found continue with default
    }

    data::ValueNodeAccess aOriginal = getOriginalValueNode(_aAccessor,_aName);

    if (_bForUpdate) // create a new change
    {
        if (aOriginal.isValid())
        {
            MemberChange aNewChange(new ValueMemberNode::DeferredImpl(aOriginal));
            m_aChanges[_aName] = aNewChange;

            return ValueMemberNode(_aAccessor, aNewChange);
       }
    }

    return GroupNodeImpl::makeValueMember(aOriginal);
}
//-----------------------------------------------------------------------------

bool DeferredGroupNodeImpl::hasChanges() const
{
    for (MemberChanges::const_iterator it = m_aChanges.begin(); it != m_aChanges.end(); it)
    {
        if (!it->second.is())
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

void DeferredGroupNodeImpl::collectValueChanges(data::Accessor const& _aAccessor, NodeChanges& rChanges, TreeImpl* pParentTree, NodeOffset nNode) const
{
    for (MemberChanges::const_iterator it = m_aChanges.begin(); it != m_aChanges.end(); ++it)
    {
        if (it->second.is())
        {
            OSL_ASSERT(!it->first.isEmpty());
            if (ValueChangeImpl* pValueChange = it->second->collectChange(_aAccessor))
            {
                pValueChange->setTarget(_aAccessor,pParentTree,nNode,it->first);

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

DeferredGroupNodeImpl::MemberChange DeferredGroupNodeImpl::findValueChange(Name const& aName)
{
    MemberChange aResult;

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
            OSL_ENSURE(aName.isEmpty(), "ERROR: Found empty change reference");
    }

    return aResult;
}

//-----------------------------------------------------------------------------
/*
void DeferredGroupNodeImpl::doCommitChanges(data::Accessor const& _aAccessor)
{
    for (ValueChanges::iterator pos = m_aChanges.begin(); pos != m_aChanges.end(); )
    {
        ValueChanges::iterator it = pos++; // this is used to allow erasing below
        if (it->second.is())
        {
            it->second->commitDirect(_aAccessor);
            m_aChanges.erase(it); // this goes here to ensure exception safety
        }
        else
            OSL_ASSERT(it->first.isEmpty());
    }
    m_aChanges.clear();
}
*/
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredGroupNodeImpl::preCommitValueChanges(data::Accessor const& _aAccessor)
{
    std::auto_ptr<SubtreeChange> aRet;

    if (!m_aChanges.empty())
    {
        data::NodeAccessRef aOriginalData = this->getOriginalNodeAccessRef(&_aAccessor);
        aRet.reset( new SubtreeChange(  aOriginalData.getName().toString(),
                                        aOriginalData.getAttributes() ) );

        for (MemberChanges::iterator pos = m_aChanges.begin(); pos != m_aChanges.end(); )
        {
            MemberChanges::iterator it = pos++; // this is used to allow erasing below

            if (!it->second.is())
            {
                OSL_ASSERT(it->first.isEmpty());
            }
            else if (it->second->isChange())
            {
                std::auto_ptr<ValueChange> aValueChange = it->second->preCommitChange(_aAccessor);
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

void DeferredGroupNodeImpl::finishCommit(data::Accessor const& _aAccessor, SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aValueName = makeNodeName(it->getNodeName(), Name::NoValidate());

        MemberChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            OSL_ENSURE( it->ISA(ValueChange) , "Unexpected type of element change");
            if (!it->ISA(ValueChange)) throw Exception("Unexpected type of element change");

            ValueChange & rValueChange = static_cast<ValueChange&>(*it);

            MemberChange aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.is(), "Found empty change object for Member value change");

            if (aStoredChange.is())
            {
                aStoredChange->finishCommit(rValueChange,_aAccessor);
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

void DeferredGroupNodeImpl::revertCommit(data::Accessor const& _aAccessor, SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aValueName = makeNodeName(it->getNodeName(), Name::NoValidate());

        MemberChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            OSL_ENSURE( it->ISA(ValueChange) , "Unexpected type of element change");
            if (!it->ISA(ValueChange)) continue;

            ValueChange & rValueChange = static_cast<ValueChange&>(*it);

            MemberChange aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.is(), "Cannot restore change: found empty change object for Member value change");

            if (aStoredChange.is())
            {
                aStoredChange->revertCommit(rValueChange,_aAccessor);
                OSL_ENSURE(!aStoredChange->isChange(),"ValueChange is not moot after reverting - will be discarded nevertheless");
            }
            m_aChanges.erase( itStoredChange ); // remove change if it is moot
        }
        else
            OSL_ENSURE( !it->ISA(ValueChange) , "Value member change has no change data representation");
    }
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::failedCommit(data::Accessor const& _aAccessor, SubtreeChange& rChanges)
{
    OSL_ENSURE(!rChanges.isSetNodeChange(),"ERROR: Change type SET does not match group");

    for(SubtreeChange::MutatingChildIterator it = rChanges.begin_changes(), stop = rChanges.end_changes();
        it != stop;
        ++it)
    {
        Name aValueName = makeNodeName(it->getNodeName(), Name::NoValidate());

        MemberChanges::iterator itStoredChange = m_aChanges.find(aValueName);

        if (itStoredChange != m_aChanges.end())
        {
            OSL_ENSURE( it->ISA(ValueChange) , "Unexpected type of element change");
            if (!it->ISA(ValueChange)) continue;

            ValueChange & rValueChange = static_cast<ValueChange&>(*it);

            MemberChange aStoredChange = itStoredChange->second;
            OSL_ENSURE( aStoredChange.is(), "Cannot recover from failed change: found empty change object for Member value change");

            if (aStoredChange.is())
                 aStoredChange->failedCommit(rValueChange,_aAccessor);
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


void DeferredGroupNodeImpl::markChanged()
{
    // special mark: a NULL DeferredImplRef at empty Name
    m_aChanges.insert( MemberChanges::value_type() );
}
//-----------------------------------------------------------------------------

// Set nodes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class DeferredTreeSetNodeImpl
//-----------------------------------------------------------------------------

DeferredSetNodeImpl::DeferredSetNodeImpl(data::SetNodeAddress const& _aNodeRef, Template* pTemplate)
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
    {for(NativeIterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (!m_aChangedData.hasElement(it->first)) return false;
    }}

    return true;
}
//-----------------------------------------------------------------------------

ElementTreeImpl* DeferredSetNodeImpl::doFindElement(Name const& aName)
{
    Element* pElement = m_aChangedData.getElement(aName);
    if (!pElement)
        pElement = SetNodeImpl::getStoredElement(aName);

    return pElement ? pElement->get() : 0;
}
//-----------------------------------------------------------------------------

SetNodeVisitor::Result DeferredSetNodeImpl::doDispatchToElements(data::Accessor const& _aAccessor, SetNodeVisitor& aVisitor)
{
    SetNodeVisitor::Result eRet = SetNodeVisitor::CONTINUE;
    // look for elements in the base set that are not hidden by changes
    {for(NativeIterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            eRet = aVisitor.visit(SetEntry(_aAccessor,it->second.get()));
        }
    }}

    // look for added elements
    {for(ElementSet::ConstIterator it = m_aChangedData.begin(), stop = m_aChangedData.end();
        it != stop && eRet != SetNodeVisitor::DONE;
        ++it)
    {
        if (it->isValid())
        {
            eRet = aVisitor.visit(SetEntry(_aAccessor,it->get()));
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

void DeferredSetNodeImpl::collectElementChanges(data::Accessor const& _aAccessor, NodeChanges& rChanges) const
{
    // collect added and deleted nodes
    {for(NativeIterator it = m_aChangedData.beginNative(), stop = m_aChangedData.endNative();
        it != stop;
        ++it)
    {
        Element const* pOriginal = SetNodeImpl::getStoredElement(it->first);

        if (it->second.isValid()) // added one
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(implCreateReplace(_aAccessor, it->first,it->second,*pOriginal)));
            }
            else
            {
                rChanges.add(NodeChange(implCreateInsert(_aAccessor, it->first,it->second)));
            }
        }
        else
        {
            if (pOriginal)
            {
                rChanges.add(NodeChange(implCreateRemove(_aAccessor, it->first,*pOriginal)));
            }

            //else nothing to do
        }
    }}

    // collect preexisting nodes
    // if (!containsValues()) // value elements ar immutable !
    {for(NativeIterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            view::ViewTreeAccess aElementView(_aAccessor, *it->second);

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
    {for(NativeIterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
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
        NativeIterator it = m_aChangedData.beginNative();
        NativeIterator const stop = m_aChangedData.endNative();

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

void DeferredSetNodeImpl::rebuildElement(data::Accessor const& _aAccessor, Name const& _aName, Element const& _aElement)
{
    TreeImpl* pContext = this->getParentTree();
    OSL_ENSURE(pContext, "Context tree must be set before rebuilding");

    rtl::Reference<view::ViewStrategy> xContextBehavior = pContext->getViewBehavior();

    data::TreeAccessor aElementAccessor = this->getDataAccess(_aAccessor).getElementTree(_aName);
    OSL_ENSURE(aElementAccessor.isValid(), "Element Tree not found in data");

    OSL_ENSURE(_aElement.isValid(), "Element not found in view");
    data::Accessor aOldAccessor( _aElement->getViewBehavior()->getDataSegment() );
    _aElement->rebuild(xContextBehavior,aElementAccessor,aOldAccessor);
}

//-----------------------------------------------------------------------------
std::auto_ptr<SubtreeChange> DeferredSetNodeImpl::preCommitChanges(data::Accessor const& _aAccessor, ElementList& _rRemovedElements)
{
    data::NodeAccessRef aOriginalData = this->getOriginalNodeAccessRef(&_aAccessor);
    // now first get the name of this node
    Name sSetName = aOriginalData.getName();

    // and make a SubtreeChange
    std::auto_ptr<SubtreeChange> pSetChange( new SubtreeChange(sSetName.toString(),
                                                               getElementTemplate()->getName().toString(),
                                                               getElementTemplate()->getModule().toString(),
                                                               aOriginalData.getAttributes() ) );

    // commit preexisting nodes
    {for(NativeIterator it = SetNodeImpl::beginElementSet(), stop = SetNodeImpl::endElementSet();
        it != stop;
        ++it)
    {
        if (m_aChangedData.getElement(it->first) == 0)
        {
            OSL_ASSERT(it->second.isValid());
            OSL_ENSURE( !m_bDefault || it->second.inDefault, "m_bDefault is inconsistent");

            view::ViewTreeAccess aElementView(_aAccessor,*it->second);
            std::auto_ptr<SubtreeChange> pNewChange = aElementView.preCommitChanges(_rRemovedElements);
            if (pNewChange.get() != 0)
            {
                //OSL_ENSURE( !containsValues(), "Unexpected change generated by value set element");
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

            Element* pOriginal = SetNodeImpl::getStoredElement(aName);

            if (aNewElement.isValid())
            {
                data::TreeSegment aAddedTree = aNewElement->releaseOwnedTree();

                OSL_ENSURE( aAddedTree.is(), "Could not take the new tree from the ElementTree");
                OSL_ENSURE( !m_bDefault || aNewElement.inDefault, "m_bDefault is inconsistent");

                AddNode* pAddNode = new AddNode(aAddedTree, aName.toString(), aNewElement.inDefault );

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

                    std::auto_ptr<Change> pNewChange( new RemoveNode(aName.toString(),aNewElement.inDefault) );

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

void DeferredSetNodeImpl::finishCommit(data::Accessor const& _aAccessor, SubtreeChange& rChanges)
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
            data::TreeSegment aRemovedTree;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                aRemovedTree = rAddNode.getReplacedTree();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == aRemovedTree.is()  );

                if (aOriginal.isValid())
                    SetNodeImpl::replaceElement(aElementName,*pNewElement);

                else
                    SetNodeImpl::insertElement(aElementName,*pNewElement);

                this->rebuildElement(_aAccessor,aElementName,*pNewElement);
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                aRemovedTree = rRemoveNode.getRemovedTree();

                OSL_ASSERT(aOriginal.isValid());
                if (aOriginal.isValid())
                    SetNodeImpl::removeElement(aElementName);
            }
            // handle a added or deleted node
            if (aOriginal.isValid())
            {
                OSL_ENSURE(aRemovedTree.is(), "Cannot take over the removed node");

                aOriginal->takeTreeAndRebuild(aRemovedTree, _aAccessor);
            }
            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            //OSL_ENSURE(!containsValues(), "Unexpected change to value set element");
            OSL_ENSURE(pOriginal && pOriginal->isValid(), "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected type of element change");

            if (!it->ISA(SubtreeChange)) throw Exception("Unexpected set element change");

            if (pOriginal && pOriginal->isValid())
                view::ViewTreeAccess(_aAccessor,**pOriginal).finishCommit(static_cast<SubtreeChange&>(*it));
        }
    }
    m_bChanged = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::revertCommit(data::Accessor const& _aAccessor, SubtreeChange& rChanges)
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
            data::TreeSegment pRemovedTree;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                pRemovedTree = rAddNode.getReplacedTree();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pRemovedTree.is())  );

                OSL_ENSURE(!rAddNode.wasInserted(), "Cannot retract new node: Change was integrated");

                data::TreeSegment aAddedTree = rAddNode.getNewTree();
                OSL_ENSURE(aAddedTree.is(), "Cannot restore new node: Change lost ownership");

                // restore the tree
                (*pNewElement)->takeTreeBack(aAddedTree);
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                pRemovedTree = rRemoveNode.getRemovedTree();

                OSL_ASSERT(pOriginal);
                OSL_ASSERT((0 != pOriginal) == (0!=pRemovedTree.is())  );
            }
            OSL_ENSURE(pRemovedTree.is(), "Possible problems reverting removed node: Change took ownership");
            // try handle a added or deleted node
            if (pOriginal && pRemovedTree.is())
            {
                OSL_ASSERT(pOriginal->isValid());
                (*pOriginal)->takeTreeAndRebuild(pRemovedTree,_aAccessor);
                OSL_DEBUG_ONLY(pRemovedTree.clear());
            }
            OSL_ENSURE(!pRemovedTree.is(), "Could not revert removed node: Nowhere to put ownership");
        }
        else
        {
            // handle preexisting nodes
            //OSL_ENSURE(!containsValues(), "Unexpected change to value set element");
            OSL_ENSURE(pOriginal && pOriginal->isValid(), "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected set element change");

            if (!it->ISA(SubtreeChange)) throw Exception("Unexpected set element change");

            if (pOriginal && pOriginal->isValid())
                view::ViewTreeAccess(_aAccessor,**pOriginal).revertCommit(static_cast<SubtreeChange&>(*it));
        }
    }
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::failedCommit(data::Accessor const& _aAccessor, SubtreeChange& rChanges)
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
            data::TreeSegment aRemovedTree;

            if (pNewElement->isValid())
            {
                OSL_ENSURE( it->ISA(AddNode) , "Unexpected type of element change");
                if (!it->ISA(AddNode)) throw Exception("Unexpected type of element change");

                AddNode& rAddNode =  static_cast<AddNode&>(*it);

                aRemovedTree = rAddNode.getReplacedTree();
                OSL_ASSERT( rAddNode.isReplacing() == (0!=pOriginal)  );
                OSL_ASSERT( rAddNode.isReplacing() == aRemovedTree.is()  );

                if (rAddNode.wasInserted())
                { // it has been integrated into the master tree
                    OSL_ENSURE(getDataAccess(_aAccessor).getElementTree(aElementName).address() == rAddNode.getInsertedTree(),
                                "Internal Error: Inserted tree address does not match actual data");

                    // so add it
                    if (aOriginal.isValid())
                        SetNodeImpl::replaceElement(aElementName,*pNewElement);

                    else
                        SetNodeImpl::insertElement(aElementName,*pNewElement);

                    this->rebuildElement(_aAccessor,aElementName,*pNewElement);
                }
                else // Change not done; need to restore new node (element will be released into the wild then)
                {
                    data::TreeSegment aAddedTree = rAddNode.getNewTree();

                    OSL_ENSURE(aAddedTree.is(), "Unexpected: added node is gone, but where ? May cause invalid references");
                    if (aAddedTree.is())
                        (*pNewElement)->takeTreeBack(aAddedTree);
                    detach(*pNewElement);
                }
            }
            else
            {
                OSL_ENSURE( it->ISA(RemoveNode) , "Unexpected type of element change");
                if (!it->ISA(RemoveNode)) throw Exception("Unexpected type of element change");

                RemoveNode& rRemoveNode =  static_cast<RemoveNode&>(*it);
                aRemovedTree = rRemoveNode.getRemovedTree();

                OSL_ASSERT(aOriginal.isValid());
                if (aRemovedTree.is() && !getDataAccess(_aAccessor).hasElement(aElementName))
                {
                    // really removed - then remove the originel
                    if (aOriginal.isValid())
                        SetNodeImpl::removeElement(aElementName);
                }
            }

            // handle a added or deleted node
            if (aOriginal.isValid() && aRemovedTree.is())
            {
                aOriginal->takeTreeAndRebuild(aRemovedTree,_aAccessor);
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
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected set element change");

            if (!it->ISA(SubtreeChange)) throw Exception("Unexpected set element change");

            if (pOriginal && pOriginal->isValid())
                view::ViewTreeAccess(_aAccessor,**pOriginal).recoverFailedCommit(static_cast<SubtreeChange&>(*it));
        }
    }
    m_bChanged = false;
    m_bDefault = false;

    OSL_ENSURE(m_aChangedData.isEmpty(), "ERROR: Uncommitted changes left in set node");
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::insertNewElement(Name const& aName, Element const& aNewElement)
{
    attach(aNewElement,aName);
    try
    {
        // put the new element into the changed set
        Element* pAddedElement = m_aChangedData.getElement(aName);
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

void DeferredSetNodeImpl::removeOldElement(Name const& aName)
{
    // put an empty (dummy) element into the changed set
    Element* pAddedElement = m_aChangedData.getElement(aName);
    if (pAddedElement)
    {
        OSL_ENSURE(pAddedElement->isValid(),"WARNING: Element being removed was already removed");
        detach(m_aChangedData.replaceElement(aName, Element()));
        m_bChanged = true;
        m_bDefault = false;
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

SetElementChangeImpl* DeferredSetNodeImpl::doAdjustChangedElement(data::Accessor const & _aAccessor, NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (Element* pElement = getStoredElement(aName))
        {
            OSL_ASSERT(pElement->isValid());

            if (aChange.ISA(SubtreeChange))
            {
                //OSL_ENSURE( !containsValues(), "Unexpected kind of change: Tree change applied to value set element" );
                SubtreeChange const& aSubtreeChange = static_cast<SubtreeChange const&>(aChange);

                // recurse to element tree - but do not notify those changes (?)
                view::Tree aElementTree(_aAccessor, **pElement);

                NodeChangesInformation aIgnoredChanges;
                view::getViewBehavior(aElementTree)->adjustToChanges(aIgnoredChanges,view::getRootNode(aElementTree),aSubtreeChange);
            }
            else
            {
                OSL_ENSURE( aChange.ISA(ValueChange), "Unexpected kind of change to value set element" );
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
            Element aLocalElement = *pLocalElement;

            // also signal something happened
            return implCreateReplace(_aAccessor,aName,aLocalElement,aLocalElement);
        }
        else
        {
            // already removed locally - should be notified by different route (if applicable)
            return NULL;
        }
    }
    else
    {
        return SetNodeImpl::doAdjustChangedElement(_aAccessor, rLocalChanges,aName,aChange);
    }
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* DeferredSetNodeImpl::doAdjustToAddedElement(data::Accessor const& _aAccessor, Name const& aName, AddNode const& aAddNodeChange, Element const& aNewElement)
{
    m_bDefault = false;
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        // We have another element replacing ours - what do we do ?
        if (Element* pOriginal = getStoredElement(aName))
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
            Element aLocalElement = *pLocalElement;

            // just signal something happened
            return implCreateReplace(_aAccessor,aName,aLocalElement,aLocalElement);
        }
        else // had been removed locally
        {
            // signal what happened
            return implCreateInsert(_aAccessor,aName,aNewElement);
        }
    }
    else
    {
        return SetNodeImpl::implAdjustToAddedElement(_aAccessor,aName,aNewElement,aAddNodeChange.isReplacing());
    }
}
//-----------------------------------------------------------------------------

SetElementChangeImpl* DeferredSetNodeImpl::doAdjustToRemovedElement(data::Accessor const& _aAccessor, Name const& aName, RemoveNode const& aRemoveNodeChange)
{
    m_bDefault = false;
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
        if (Element* pOriginal = getStoredElement(aName))
        {
            // take away the original
            this->removeElement(aName);
        }

        if (pLocalElement->isValid()) // remains a valid replacement
        {
            Element aLocalElement = *pLocalElement;

            // signal something happened
            return implCreateReplace(_aAccessor,aName,aLocalElement,aLocalElement);
        }
        else // already was removed locally
        {
            return 0;
        }
    }
    else
    {
        return SetNodeImpl::implAdjustToRemovedElement(_aAccessor,aName);
    }
}
//-----------------------------------------------------------------------------

void DeferredSetNodeImpl::doDifferenceToDefaultState(data::Accessor const& _aAccessor, SubtreeChange& _rChangeToDefault, ISubtree& _rDefaultTree)
{
    if (!m_bDefault)
    {
        implDifferenceToDefaultState(_aAccessor,_rChangeToDefault,_rDefaultTree);

        NativeIterator it = m_aChangedData.beginNative();
        NativeIterator const stop = m_aChangedData.endNative();

        while(it != stop)
        {
            Name    aName       = it->first;
            Element aElement    = it->second;

            Change* pChange = _rChangeToDefault.getChange( aName.toString() );
            OSL_ENSURE(pChange == NULL || pChange->ISA(AddNode) || pChange->ISA(RemoveNode),
                        "Unexpected change type found in difference to default tree");

            if (pChange == NULL)
            {
                std::auto_ptr<INode> aDefaultNode = _rDefaultTree.removeChild(aName.toString());
                OSL_ENSURE( aDefaultNode.get(), "Error: unused Default tree not found after SetNodeImpl::implDifferenceToDefaultState");

                OUString aElementTypeName = _rDefaultTree.getElementTemplateName();
                OSL_ENSURE( _rDefaultTree.isSetNode(), "Error: missing set template information in default data");

                data::TreeSegment aDefaultTree = data::TreeSegment::createNew(aDefaultNode,aElementTypeName);
                OSL_ENSURE( aDefaultTree.is(), "Error: unused Default tree not accessible after SetNodeImpl::implDifferenceToDefaultState");

                AddNode* pAddIt = new AddNode(aDefaultTree, aName.toString(), true );

                std::auto_ptr<Change> pNewChange( pAddIt );

                if (aElement.isValid())
                {
                    OSL_ENSURE(!aElement.inDefault, "Default element replaced by default");
                    pAddIt->setReplacing();
                }

                _rChangeToDefault.addChange(pNewChange);

            }
            else if ( pChange->ISA(AddNode) )
            {
                // adjust the AddNode - remove the original expected node
                AddNode* pAddIt = static_cast<AddNode*>(pChange);
                pAddIt->clearReplacedTree();

                if (aElement.isValid())
                {
                    if (aElement.inDefault)
                    {
                        // change already done locally
                        _rChangeToDefault.removeChange(aName.toString());
                    }
                    else // adjust here
                        pAddIt->setReplacing();
                }

                else
                    OSL_ENSURE(!pAddIt->isReplacing(),"Could not unmark the 'replacing' state of an AddNode");
            }
            else if ( pChange->ISA(RemoveNode) )
            {
                if (aElement.isValid())
                {
                    OSL_ENSURE(!aElement.inDefault, "Default element replaced by default");
                    // adjust the RemoveNode - remove the original expected node
                    RemoveNode* pRemoveIt = static_cast<RemoveNode*>(pChange);
                    pRemoveIt->clearRemovedTree();
                }
                else
                {
                    // change already done locally
                    _rChangeToDefault.removeChange(aName.toString());
                }
                // TODO: mark local removal as to-default
            }
        }
    }
}
//-----------------------------------------------------------------------------
    }
}
