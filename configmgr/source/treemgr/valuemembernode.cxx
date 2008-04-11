/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valuemembernode.cxx,v $
 * $Revision: 1.11 $
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

#include "valuemembernode.hxx"
#include "nodeimplobj.hxx"
#include "nodechangeimpl.hxx"
#include "change.hxx"
#include "valuenode.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

// helpers
//-----------------------------------------------------------------------------

namespace
{

//-----------------------------------------------------------------------------
// internal accessors for direct updates to data
//-----------------------------------------------------------------------------

inline
void setOriginalValue(data::ValueNodeAddress const& rOriginalAddress, UnoAny const& aNewValue)
{
    data::ValueNodeAccess::setValue(rOriginalAddress,aNewValue);
}
//-----------------------------------------------------------------------------

inline
void setOriginalToDefault(data::ValueNodeAddress const& rOriginalAddress)
{
    data::ValueNodeAccess::setToDefault(rOriginalAddress);
}
} // anonymous namespace


//-----------------------------------------------------------------------------
// class ValueMemberNode
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(data::ValueNodeAccess const& _aNodeAccess)
    : m_aNodeRef(_aNodeAccess)
    , m_xDeferredOperation()
{}
//-----------------------------------------------------------------------------
ValueMemberNode::ValueMemberNode(DeferredImplRef const& _xDeferred) // must be valid
    : m_aNodeRef( _xDeferred->getOriginalNode() )
    , m_xDeferredOperation(_xDeferred)
{}
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(ValueMemberNode const& rOriginal)
    : m_aNodeRef(rOriginal.m_aNodeRef)
    , m_xDeferredOperation(rOriginal.m_xDeferredOperation)
{}
//-----------------------------------------------------------------------------
ValueMemberNode& ValueMemberNode::operator=(ValueMemberNode const& rOriginal)
{
    m_aNodeRef              = rOriginal.m_aNodeRef;
    m_xDeferredOperation    = rOriginal.m_xDeferredOperation;
    return *this;
}
//-----------------------------------------------------------------------------
ValueMemberNode::~ValueMemberNode()
{
}
//-----------------------------------------------------------------------------

bool ValueMemberNode::isValid() const
{
    OSL_ASSERT( !m_xDeferredOperation.is() ||
                 m_aNodeRef == m_xDeferredOperation->getOriginalNodeAddress());

    return m_aNodeRef.isValid();
}
//-----------------------------------------------------------------------------

bool ValueMemberNode::hasChange() const
{
    return m_xDeferredOperation.is()
           && m_xDeferredOperation->isChange();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// external accessors
//-----------------------------------------------------------------------------

Name ValueMemberNode::getNodeName() const
{
    return m_aNodeRef.getName();
}
//-----------------------------------------------------------------------------

node::Attributes ValueMemberNode::getAttributes()   const
{
    return m_aNodeRef.getAttributes();
}
//-----------------------------------------------------------------------------


bool ValueMemberNode::isDefault() const
{
    if (hasChange())
        return m_xDeferredOperation->isToDefault();

    return m_aNodeRef.isDefault();
}
//-----------------------------------------------------------------------------

bool ValueMemberNode::canGetDefaultValue() const
{
    return m_aNodeRef.hasUsableDefault();
}
//-----------------------------------------------------------------------------

UnoAny  ValueMemberNode::getValue() const
{
    if (hasChange())
        return m_xDeferredOperation->getNewValue();

    return m_aNodeRef.getValue();
}
//-----------------------------------------------------------------------------

UnoAny ValueMemberNode::getDefaultValue() const
{
    return m_aNodeRef.getDefaultValue();
}
//-----------------------------------------------------------------------------

UnoType ValueMemberNode::getValueType() const
{
    return m_aNodeRef.getValueType();
}
//-----------------------------------------------------------------------------


void ValueMemberUpdate::setValue(UnoAny const& aNewValue)
{
    if (m_aMemberNode.m_xDeferredOperation.is())
        m_aMemberNode.m_xDeferredOperation->setValue(aNewValue, m_aMemberNode.m_aNodeRef);
    else
        setOriginalValue(m_aMemberNode.m_aNodeRef, aNewValue );
}
//-----------------------------------------------------------------------------

void ValueMemberUpdate::setDefault()
{
    if (m_aMemberNode.m_xDeferredOperation.is())
        m_aMemberNode.m_xDeferredOperation->setValueToDefault(m_aMemberNode.m_aNodeRef);
    else
        setOriginalToDefault( m_aMemberNode.m_aNodeRef );
}

//-----------------------------------------------------------------------------
// class ValueMemberNode::DeferredImpl
//-----------------------------------------------------------------------------

ValueMemberNode::DeferredImpl::DeferredImpl(data::ValueNodeAccess const& _aValueNode)
: m_aValueRef(_aValueNode)
, m_aNewValue(_aValueNode.getValue())
, m_bToDefault(false)
, m_bChange(false)
{}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValue(UnoAny const& aNewValue, data::ValueNodeAccess const& _aOriginalNode)
{
    OSL_ENSURE(_aOriginalNode == m_aValueRef, "Incorrect original node passed");

    m_aNewValue = aNewValue;
    m_bToDefault = false;

    m_bChange = _aOriginalNode.isDefault() || aNewValue != _aOriginalNode.getValue();
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValueToDefault(data::ValueNodeAccess const& _aOriginalNode)
{
    OSL_ENSURE(_aOriginalNode == m_aValueRef, "Incorrect original node passed");

    m_aNewValue = _aOriginalNode.getDefaultValue();
    m_bToDefault = true;

    m_bChange = !_aOriginalNode.isDefault();
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueChange> ValueMemberNode::DeferredImpl::preCommitChange()
{
    OSL_ENSURE(isChange(), "Trying to commit a non-change");

    data::ValueNodeAccess aOriginalNode = getOriginalNode();

    // first find the mode of the change
    ValueChange::Mode eMode;

    if (m_bToDefault)
        eMode = ValueChange::setToDefault;

    else if (! aOriginalNode.isDefault())
        eMode = ValueChange::changeValue;

    else
        eMode = ValueChange::wasDefault;

    // now make a ValueChange
    std::auto_ptr<ValueChange>pChange( new ValueChange( aOriginalNode.getName().toString(),
                                                        aOriginalNode.getAttributes(),
                                                        eMode,
                                                        this->getNewValue(),
                                                        aOriginalNode.getValue()
                                                      ) );

    return  pChange;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::finishCommit(ValueChange& rChange)
{
    { (void)rChange; }
    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Committed change does not match the intended value");

    data::ValueNodeAccess aOriginalNode = getOriginalNode();

    m_aNewValue = aOriginalNode.getValue();
    m_bToDefault = false;

    OSL_ENSURE(rChange.getNewValue() == m_aNewValue,"Committed change does not match the actual value");
    m_bChange= false;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::revertCommit(ValueChange& rChange)
{
    { (void)rChange; }

    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Reverted change does not match the intended value");
    OSL_ENSURE(isChange(), "ValueMemeberNode::DeferredImpl: No Changes to revert");
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::failedCommit(ValueChange&)
{
    data::ValueNodeAccess aOriginalNode = getOriginalNode();

    // discard the change
    m_aNewValue = aOriginalNode.getValue();
    m_bToDefault = false;

    m_bChange= false;
}
//-----------------------------------------------------------------------------
ValueChangeImpl* ValueMemberNode::DeferredImpl::collectChange()
{
    data::ValueNodeAccess aOriginalNode = getOriginalNode();

    UnoAny aOldValue = aOriginalNode.getValue();
    if (!m_bChange)
    {
        return NULL;
    }
    else if (m_bToDefault)
    {
        OSL_ASSERT( m_aNewValue == aOriginalNode.getDefaultValue() );
        return new ValueResetImpl( m_aNewValue, aOldValue );
    }

    else
    {
        return new ValueReplaceImpl( m_aNewValue, aOldValue );
    }
}
//-----------------------------------------------------------------------------

ValueChangeImpl* ValueMemberNode::DeferredImpl::adjustToChange(ValueChange const& rExternalChange)
{
    if (!m_bChange)
    {
        return NULL;
    }
    else if (m_bToDefault && rExternalChange.getMode() == ValueChange::changeDefault)
    {
        OSL_ASSERT( m_aNewValue == rExternalChange.getOldValue() );

        m_aNewValue = rExternalChange.getNewValue();

        return new ValueReplaceImpl(m_aNewValue, rExternalChange.getOldValue());
    }
    else // return Surrogate - does not honor m_bToDefault
    {
        return new ValueReplaceImpl(m_aNewValue, m_aNewValue);
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}
