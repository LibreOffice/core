/*************************************************************************
 *
 *  $RCSfile: nodeimplobj.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-12 15:04:12 $
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

#include "nodefactory.hxx"

#include "nodechange.hxx"
#include "nodechangeimpl.hxx"

#include "cmtreemodel.hxx"

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
// Specific types of nodes for direct or read only access
//-----------------------------------------------------------------------------

// Value Nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class ReadOnlyValueNodeImpl
//-----------------------------------------------------------------------------

bool ReadOnlyValueNodeImpl::isDefault() const
{
    return ValueNodeImpl::isDefault();
}
//-----------------------------------------------------------------------------

bool ReadOnlyValueNodeImpl::canGetDefaultValue() const
{
    return ValueNodeImpl::canGetDefaultValue();
}
//-----------------------------------------------------------------------------

UnoAny ReadOnlyValueNodeImpl::getValue() const
{
    return ValueNodeImpl::getValue();
}
//-----------------------------------------------------------------------------

UnoAny ReadOnlyValueNodeImpl::getDefaultValue() const
{
    return ValueNodeImpl::getDefaultValue();
}
//-----------------------------------------------------------------------------


UnoType ReadOnlyValueNodeImpl::getValueType() const
{
    return ValueNodeImpl::getValueType();
}
//-----------------------------------------------------------------------------


void ReadOnlyValueNodeImpl::setValue(UnoAny const& )
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyValueNodeImpl::setDefault()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

Attributes ReadOnlyValueNodeImpl::doGetAttributes() const
{
    return forceReadOnly( ValueNodeImpl::doGetAttributes() );
}
//-----------------------------------------------------------------------------

bool ReadOnlyValueNodeImpl::doHasChanges()  const
{
    return false;
}
//-----------------------------------------------------------------------------

void ReadOnlyValueNodeImpl::doCommitChanges()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

void ReadOnlyValueNodeImpl::doMarkChanged()
{
    failReadOnly();
}
//-----------------------------------------------------------------------------

NodeImplHolder ReadOnlyValueNodeImpl::doCloneIndirect(bool)
{
    return this;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class DirectValueNodeImpl
//-----------------------------------------------------------------------------

DirectValueNodeImpl::DirectValueNodeImpl(ValueNode& rOriginal)
: ValueNodeImpl(rOriginal)
{}
//-----------------------------------------------------------------------------

DirectValueNodeImpl::DirectValueNodeImpl(DeferredValueNodeImpl& rOriginal)
: ValueNodeImpl(rOriginal)
{}
//-----------------------------------------------------------------------------

bool DirectValueNodeImpl::isDefault() const
{
    return ValueNodeImpl::isDefault();
}
//-----------------------------------------------------------------------------

bool DirectValueNodeImpl::canGetDefaultValue() const
{
    return ValueNodeImpl::canGetDefaultValue();
}
//-----------------------------------------------------------------------------

UnoAny DirectValueNodeImpl::getValue() const
{
    return ValueNodeImpl::getValue();
}
//-----------------------------------------------------------------------------

UnoAny DirectValueNodeImpl::getDefaultValue() const
{
    return ValueNodeImpl::getDefaultValue();
}
//-----------------------------------------------------------------------------


UnoType DirectValueNodeImpl::getValueType() const
{
    return ValueNodeImpl::getValueType();
}
//-----------------------------------------------------------------------------


void DirectValueNodeImpl::setValue(UnoAny const& aNewValue)
{
    ValueNodeImpl::setValue(aNewValue);
}
//-----------------------------------------------------------------------------

void DirectValueNodeImpl::setDefault()
{
    ValueNodeImpl::setDefault();
}
//-----------------------------------------------------------------------------

bool DirectValueNodeImpl::doHasChanges()    const
{
    return false;
}
//-----------------------------------------------------------------------------

void DirectValueNodeImpl::doCommitChanges()
{
    OSL_ENSURE(false,"WARNING: Should not commit changes on object that hasn't any");
}
//-----------------------------------------------------------------------------

void DirectValueNodeImpl::doMarkChanged()
{
    // Ignore
}
//-----------------------------------------------------------------------------

NodeImplHolder DirectValueNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return new DeferredValueNodeImpl(*this);
    else
        return this;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// class DeferredValueNodeImpl
//-----------------------------------------------------------------------------

DeferredValueNodeImpl::DeferredValueNodeImpl(ValueNode& rOriginal)
: ValueNodeImpl(rOriginal)
, m_pNewValue(0)
, m_bDefault(false)
{
}
//-----------------------------------------------------------------------------

DeferredValueNodeImpl::DeferredValueNodeImpl(DirectValueNodeImpl& rOriginal)
: ValueNodeImpl(rOriginal)
, m_pNewValue(0)
, m_bDefault(false)
{
}
//-----------------------------------------------------------------------------

DeferredValueNodeImpl::~DeferredValueNodeImpl()
{
    delete m_pNewValue;
}
//-----------------------------------------------------------------------------

bool DeferredValueNodeImpl::isDefault() const
{
    if (m_pNewValue)
        return m_bDefault;
    else
        return ValueNodeImpl::isDefault();
}
//-----------------------------------------------------------------------------

bool DeferredValueNodeImpl::canGetDefaultValue() const
{
    return ValueNodeImpl::canGetDefaultValue();
}
//-----------------------------------------------------------------------------

UnoAny DeferredValueNodeImpl::getValue() const
{
    if (m_pNewValue)
        return *m_pNewValue;
    else if (m_bDefault)
        return ValueNodeImpl::getDefaultValue();
    else
        return ValueNodeImpl::getValue();
}
//-----------------------------------------------------------------------------

UnoAny DeferredValueNodeImpl::getDefaultValue() const
{
    return ValueNodeImpl::getDefaultValue();
}
//-----------------------------------------------------------------------------

UnoType DeferredValueNodeImpl::getValueType() const
{
    if (m_pNewValue && m_pNewValue->hasValue())
        return m_pNewValue->getValueType();
    else
        return ValueNodeImpl::getValueType();
}
//-----------------------------------------------------------------------------


void DeferredValueNodeImpl::setValue(UnoAny const& aNewValue)
{
    if (m_pNewValue)
        *m_pNewValue = aNewValue;
    else
        m_pNewValue = new UnoAny(aNewValue);

    m_bDefault = false;
}
//-----------------------------------------------------------------------------

void DeferredValueNodeImpl::setDefault()
{
    if (m_bDefault)
    {
        OSL_ASSERT(!m_pNewValue);
        OSL_ASSERT(!ValueNodeImpl::isDefault());
    }
    else if (ValueNodeImpl::isDefault())
    {
        delete m_pNewValue, m_pNewValue = 0;
        m_bDefault = false;
    }
    else if (!ValueNodeImpl::canGetDefaultValue())
    {
        OSL_ENSURE(false, "ERROR: Cannot retrieve the necessary default value");
        throw Exception("INTERNAL ERROR: Cannot retrieve the necessary default value");
    }
    else
    {
        delete m_pNewValue, m_pNewValue = 0;
        m_bDefault = true;
    }
}
//-----------------------------------------------------------------------------

bool DeferredValueNodeImpl::doHasChanges() const
{
    return m_pNewValue || m_bDefault;
}
//-----------------------------------------------------------------------------

NodeChangeImpl* DeferredValueNodeImpl::doCollectChange() const
{
    // TODO
    if (m_bDefault)
    {
        OSL_ASSERT(!m_pNewValue);
        return new ValueResetImpl(ValueNodeImpl::getDefaultValue(), ValueNodeImpl::getValue());
    }

    else if (m_pNewValue)
    {
        return new ValueReplaceImpl(*m_pNewValue, ValueNodeImpl::getValue());
    }
    else
    {
        return 0;
    }
}
//-----------------------------------------------------------------------------

NodeChangeImpl* DeferredValueNodeImpl::doAdjustToChange(ValueChange const& rExternalChange)
{
    if (m_bDefault && rExternalChange.getMode() == ValueChange::changeDefault)
    {
        OSL_ASSERT(!m_pNewValue);

        return new ValueReplaceImpl(rExternalChange.getNewValue(), rExternalChange.getOldValue());
    }
    else if (m_pNewValue) // return Surrogate
    {
        return new ValueReplaceImpl(*m_pNewValue, *m_pNewValue);
    }
    else
    {
        return ValueNodeImpl::doAdjustToChange(rExternalChange);
    }
}
//-----------------------------------------------------------------------------

void DeferredValueNodeImpl::doCommitChanges()
{
    if (m_bDefault)
    {
        OSL_ASSERT(!m_pNewValue);
        ValueNodeImpl::setDefault();
    }

    else if (m_pNewValue)
    {
        ValueNodeImpl::setValue(*m_pNewValue);

        OSL_ENSURE(ValueNodeImpl::getValue() == *m_pNewValue, "ERROR: Inconsistent committed value");

    }

    delete m_pNewValue, m_pNewValue = 0;
    m_bDefault = false;
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueChange> DeferredValueNodeImpl::doPreCommitChange()
{
    // first find the mode of the change
    // initial value is harmless (done locally) or produces an error elsewhere
    ValueChange::Mode eMode = ValueChange::changeDefault;

    if (m_bDefault)
    {
        OSL_ASSERT(!m_pNewValue);
        eMode = ValueChange::setToDefault;
    }
    else if (m_pNewValue == 0)
        OSL_ENSURE( false, "ERROR: Cannot make a change from no change !");

    else if (!ValueNodeImpl::isDefault())
        eMode = ValueChange::changeValue;

    else if (ValueNodeImpl::getValueType().getTypeClass() == uno::TypeClass_ANY)
        eMode = ValueChange::typeIsAny;

    else
        eMode = ValueChange::wasDefault;

    // now make a ValueChange
    ValueChange* pChange = new ValueChange( this->getOriginalNodeName(), getValue(),
                                            this->getAttributes(), eMode, ValueNodeImpl::getValue());

    return std::auto_ptr<ValueChange>( pChange);
}
//-----------------------------------------------------------------------------

void DeferredValueNodeImpl::doFinishCommit(ValueChange& rChange)
{
    OSL_ENSURE(rChange.getNewValue() == this->getValue(),"Committed change does not match the intended value");

    delete m_pNewValue, m_pNewValue = 0;
    m_bDefault = false;

    OSL_ENSURE(rChange.getNewValue() == this->getValue(),"Committed change does not match the actual value");
}
//-----------------------------------------------------------------------------

void DeferredValueNodeImpl::doRevertCommit(ValueChange& rChange)
{
    OSL_ENSURE(rChange.getNewValue() == this->getValue(),"Reverted change does not match the intended value");
    OSL_ENSURE(doHasChanges(), "DeferredValueNodeImpl: No Changes to restore");
}
//-----------------------------------------------------------------------------

void DeferredValueNodeImpl::doFailedCommit(ValueChange& rChange)
{
    // discard the change
    delete m_pNewValue, m_pNewValue = 0;
    m_bDefault = false;
}
//-----------------------------------------------------------------------------

void DeferredValueNodeImpl::doMarkChanged()
{
    // ignore
}
//-----------------------------------------------------------------------------

NodeImplHolder DeferredValueNodeImpl::doCloneIndirect(bool bIndirect)
{
    if (bIndirect)
        return this;
    else
        return new DirectValueNodeImpl(*this);
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
, m_bChanged(false)
{
}
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::DeferredGroupNodeImpl(DirectGroupNodeImpl& rOriginal)
: GroupNodeImpl(rOriginal)
, m_bChanged(false)
{
}
//-----------------------------------------------------------------------------

DeferredGroupNodeImpl::~DeferredGroupNodeImpl()
{
}
//-----------------------------------------------------------------------------

bool DeferredGroupNodeImpl::doHasChanges() const
{
    return m_bChanged;
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doCollectChangesWithTarget(NodeChanges& , TreeImpl* , NodeOffset ) const
{
    // TODO
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doCommitChanges()
{
    m_bChanged = false;
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredGroupNodeImpl::doPreCommitChanges()
{
    std::auto_ptr<SubtreeChange> aRet;

    if (m_bChanged)
    {
        // get the name of this node
        aRet.reset( new SubtreeChange(this->getOriginalNodeName(),
                                      this->getAttributes()) );
    }
    return aRet;
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doFinishCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    m_bChanged = false;
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doRevertCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    OSL_ENSURE(m_bChanged, "DeferredGroupNodeImpl: No Changes to restore");
}
//-----------------------------------------------------------------------------

void DeferredGroupNodeImpl::doFailedCommit(SubtreeChange& rChange)
{
    OSL_ENSURE(!rChange.isSetNodeChange(),"ERROR: Change type SET does not match group");
    // discard the change
    m_bChanged = false;
}
//-----------------------------------------------------------------------------


void DeferredGroupNodeImpl::doMarkChanged()
{
    m_bChanged = true;
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

void ReadOnlyTreeSetNodeImpl::doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth)
{
    TreeSetNodeImpl::initHelper(aTemplateProvider, NodeType::getReadAccessFactory(), rTree, nDepth);
}
//-----------------------------------------------------------------------------

ReadOnlyTreeSetNodeImpl::Element ReadOnlyTreeSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth)
{
    return TreeSetNodeImpl::makeAdditionalElement(aTemplateProvider, NodeType::getReadAccessFactory(), aAddNodeChange, nDepth);
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

void ReadOnlyValueSetNodeImpl::doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth )
{
    ValueSetNodeImpl::initHelper(aTemplateProvider, NodeType::getReadAccessFactory(), rTree);
}
//-----------------------------------------------------------------------------

ReadOnlyValueSetNodeImpl::Element ReadOnlyValueSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth )
{
    return ValueSetNodeImpl::makeAdditionalElement(aTemplateProvider, NodeType::getReadAccessFactory(), aAddNodeChange);
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

void DirectTreeSetNodeImpl::doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth)
{
    TreeSetNodeImpl::initHelper(aTemplateProvider,NodeType::getDirectAccessFactory(), rTree, nDepth);
}
//-----------------------------------------------------------------------------

DirectTreeSetNodeImpl::Element DirectTreeSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth)
{
    return TreeSetNodeImpl::makeAdditionalElement(aTemplateProvider, NodeType::getDirectAccessFactory(), aAddNodeChange, nDepth);
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

void DirectValueSetNodeImpl::doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth )
{
    ValueSetNodeImpl::initHelper(aTemplateProvider, NodeType::getDirectAccessFactory(), rTree);
}
//-----------------------------------------------------------------------------

DirectValueSetNodeImpl::Element DirectValueSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth )
{
    return ValueSetNodeImpl::makeAdditionalElement(aTemplateProvider, NodeType::getDirectAccessFactory(), aAddNodeChange);
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
            aVisitor.visit(SetEntry(it->second.getBodyPtr()));
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

void DeferredTreeSetNodeImpl::doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth nDepth)
{
    TreeSetNodeImpl::initHelper(aTemplateProvider, NodeType::getDeferredChangeFactory(), rTree, nDepth);
}
//-----------------------------------------------------------------------------

DeferredTreeSetNodeImpl::Element DeferredTreeSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth nDepth)
{
    return TreeSetNodeImpl::makeAdditionalElement(aTemplateProvider, NodeType::getDeferredChangeFactory(), aAddNodeChange, nDepth);
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
            it->second->commitChanges();
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
                (*pOriginal)->commitChanges();
                (*pOriginal)->makeIndirect(false);
            }

            ++it;
            m_aChangedData.removeElement(aName);
        }
    }

    m_bChanged = false;
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredTreeSetNodeImpl::doPreCommitChanges()
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

            std::auto_ptr<Change> pNewChange( it->second->legacyCommitChanges() );
            if (pNewChange.get() != 0)
                pSetChange->addChange(pNewChange);
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
        Name aElementName = Name(it->getNodeName(), Name::NoValidate());

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
                aOriginal->commitChanges(); // tree is detached => commit directly
                aOriginal->makeIndirect(false);
            }
            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            OSL_ENSURE(pOriginal, "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected type of element change");

            if (pOriginal)
                (*pOriginal)->legacyFinishCommit(*it);
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
        Name aElementName = Name(it->getNodeName(), Name::NoValidate());

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

            if (pOriginal)
                (*pOriginal)->legacyRevertCommit(*it);
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
        Name aElementName = Name(it->getNodeName(), Name::NoValidate());

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
                aOriginal->commitChanges(); // tree is detached => commit directly
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

            if (pOriginal)
                (*pOriginal)->legacyFailedCommit(*it);
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

void DeferredTreeSetNodeImpl::doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange, TemplateProvider const& aTemplateProvider)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
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

        if (Element* pElement = getStoredElement(aName))
        {
            // recurse to element tree - but do not notify those changes (?)
            //OSL_ENSURE(false, "Cannot properly notify this case - what can we do ?");

            OSL_ASSERT(pElement->isValid());
            //NodeChanges aIgnoredChanges;
            (*pElement)->adjustToChanges(rLocalChanges/*aIgnoredChanges*/,aChange,aTemplateProvider);
        }
        else
        {
            // could be changed to do an insert instead (?)
            OSL_ENSURE( false, "Changed Element didn't exist before it was removed/replaced" );
        }
    }
    else
    {
        TreeSetNodeImpl::doAdjustChangedElement(rLocalChanges,aName,aChange,aTemplateProvider);
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

void DeferredValueSetNodeImpl::doInitElements(TemplateProvider const& aTemplateProvider, ISubtree& rTree, TreeDepth )
{
    ValueSetNodeImpl::initHelper(aTemplateProvider, NodeType::getDeferredChangeFactory(), rTree);
}
//-----------------------------------------------------------------------------

DeferredValueSetNodeImpl::Element DeferredValueSetNodeImpl::doMakeAdditionalElement(AddNode const& aAddNodeChange, TemplateProvider const& aTemplateProvider, TreeDepth )
{
    return ValueSetNodeImpl::makeAdditionalElement(aTemplateProvider, NodeType::getDeferredChangeFactory(), aAddNodeChange);
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
            it->second->commitChanges();
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
                (*pOriginal)->commitChanges();
                (*pOriginal)->makeIndirect(false);
            }

            ++it;
            m_aChangedData.removeElement(aName);
        }
    }

    m_bChanged = false;
}
//-----------------------------------------------------------------------------

std::auto_ptr<SubtreeChange> DeferredValueSetNodeImpl::doPreCommitChanges()
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

            std::auto_ptr<Change> pNewChange( it->second->legacyCommitChanges() );
            if (pNewChange.get() != 0)
                pSetChange->addChange(pNewChange);
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
        Name aElementName = Name(it->getNodeName(), Name::NoValidate());

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
                aOriginal->commitChanges(); // tree is detached => commit directly
                aOriginal->makeIndirect(false);
            }
            m_aChangedData.removeElement(aElementName);
        }
        else
        {
            // handle preexisting nodes
            OSL_ENSURE(pOriginal, "Changed Element is missing");
            OSL_ENSURE(it->ISA(SubtreeChange), "Unexpected type of element change");

            if (pOriginal)
                (*pOriginal)->legacyFinishCommit(*it);
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
        Name aElementName = Name(it->getNodeName(), Name::NoValidate());

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

            if (pOriginal)
                (*pOriginal)->legacyRevertCommit(*it);
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
        Name aElementName = Name(it->getNodeName(), Name::NoValidate());

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
                aOriginal->commitChanges(); // tree is detached => commit directly
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

            if (pOriginal)
                (*pOriginal)->legacyFailedCommit(*it);
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

void DeferredValueSetNodeImpl::doAdjustChangedElement(NodeChangesInformation& rLocalChanges, Name const& aName, Change const& aChange, TemplateProvider const& aTemplateProvider)
{
    if (Element* pLocalElement = m_aChangedData.getElement(aName))
    {
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

        if (Element* pElement = getStoredElement(aName))
        {
            // recurse to element tree - but do not notify those changes (?)
            //OSL_ENSURE(false, "Cannot properly notify this case - what can we do ?");

            OSL_ASSERT(pElement->isValid());
            //NodeChanges aIgnoredChanges;
            (*pElement)->adjustToChanges(rLocalChanges/*aIgnoredChanges*/,aChange,aTemplateProvider);
        }
        else
        {
            // could be changed to do an insert instead (?)
            OSL_ENSURE( false, "Changed Element didn't exist before it was removed/replaced" );
        }
    }
    else
    {
        ValueSetNodeImpl::doAdjustChangedElement(rLocalChanges,aName,aChange,aTemplateProvider);
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
