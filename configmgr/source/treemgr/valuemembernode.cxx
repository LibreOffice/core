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
void setOriginalValue(sharable::ValueNode * const& rOriginalAddress, com::sun::star::uno::Any const& aNewValue)
{
    rOriginalAddress->setValue(aNewValue);
}
//-----------------------------------------------------------------------------

inline
void setOriginalToDefault(sharable::ValueNode * const& rOriginalAddress)
{
    rOriginalAddress->setToDefault();
}
} // anonymous namespace


//-----------------------------------------------------------------------------
// class ValueMemberNode
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(sharable::ValueNode * node)
    : m_node(node)
    , m_xDeferredOperation()
{}
//-----------------------------------------------------------------------------
ValueMemberNode::ValueMemberNode(rtl::Reference<DeferredImpl> const& _xDeferred) // must be valid
    : m_node( _xDeferred->getOriginalNode() )
    , m_xDeferredOperation(_xDeferred)
{}
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(ValueMemberNode const& rOriginal)
    : m_node(rOriginal.m_node)
    , m_xDeferredOperation(rOriginal.m_xDeferredOperation)
{}
//-----------------------------------------------------------------------------
ValueMemberNode& ValueMemberNode::operator=(ValueMemberNode const& rOriginal)
{
    m_node = rOriginal.m_node;
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
                 m_node == m_xDeferredOperation->getOriginalNode());

    return m_node != 0;
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

rtl::OUString ValueMemberNode::getNodeName() const
{
    return m_node->info.getName();
}
//-----------------------------------------------------------------------------

node::Attributes ValueMemberNode::getAttributes()   const
{
    return sharable::node(m_node)->getAttributes();
}
//-----------------------------------------------------------------------------


bool ValueMemberNode::isDefault() const
{
    if (hasChange())
        return m_xDeferredOperation->isToDefault();

    return m_node->info.isDefault();
}
//-----------------------------------------------------------------------------

bool ValueMemberNode::canGetDefaultValue() const
{
    return m_node->hasUsableDefault();
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any    ValueMemberNode::getValue() const
{
    if (hasChange())
        return m_xDeferredOperation->getNewValue();

    return m_node->getValue();
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any ValueMemberNode::getDefaultValue() const
{
    return m_node->getDefaultValue();
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Type   ValueMemberNode::getValueType() const
{
    return m_node->getValueType();
}
//-----------------------------------------------------------------------------


void ValueMemberUpdate::setValue(com::sun::star::uno::Any const& aNewValue)
{
    if (m_aMemberNode.m_xDeferredOperation.is())
        m_aMemberNode.m_xDeferredOperation->setValue(aNewValue, m_aMemberNode.m_node);
    else
        setOriginalValue(m_aMemberNode.m_node, aNewValue );
}
//-----------------------------------------------------------------------------

void ValueMemberUpdate::setDefault()
{
    if (m_aMemberNode.m_xDeferredOperation.is())
        m_aMemberNode.m_xDeferredOperation->setValueToDefault(m_aMemberNode.m_node);
    else
        setOriginalToDefault(m_aMemberNode.m_node);
}

//-----------------------------------------------------------------------------
// class ValueMemberNode::DeferredImpl
//-----------------------------------------------------------------------------

ValueMemberNode::DeferredImpl::DeferredImpl(sharable::ValueNode * valueNode)
: m_valueNode(valueNode)
, m_aNewValue(valueNode->getValue())
, m_bToDefault(false)
, m_bChange(false)
{}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValue(com::sun::star::uno::Any const& aNewValue, sharable::ValueNode * originalNode)
{
    OSL_ENSURE(originalNode == m_valueNode, "Incorrect original node passed");

    m_aNewValue = aNewValue;
    m_bToDefault = false;

    m_bChange = originalNode->info.isDefault() || aNewValue != originalNode->getValue();
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValueToDefault(sharable::ValueNode * originalNode)
{
    OSL_ENSURE(originalNode == m_valueNode, "Incorrect original node passed");

    m_aNewValue = originalNode->getDefaultValue();
    m_bToDefault = true;

    m_bChange = !originalNode->info.isDefault();
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueChange> ValueMemberNode::DeferredImpl::preCommitChange()
{
    OSL_ENSURE(isChange(), "Trying to commit a non-change");

    sharable::ValueNode * originalNode = getOriginalNode();

    // first find the mode of the change
    ValueChange::Mode eMode;

    if (m_bToDefault)
        eMode = ValueChange::setToDefault;

    else if (! originalNode->info.isDefault())
        eMode = ValueChange::changeValue;

    else
        eMode = ValueChange::wasDefault;

    // now make a ValueChange
    std::auto_ptr<ValueChange>pChange( new ValueChange( originalNode->info.getName(),
                                                        sharable::node(originalNode)->getAttributes(),
                                                        eMode,
                                                        this->getNewValue(),
                                                        originalNode->getValue()
                                                      ) );

    return  pChange;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::finishCommit(ValueChange& rChange)
{
    { (void)rChange; }
    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Committed change does not match the intended value");

    sharable::ValueNode * originalNode = getOriginalNode();

    m_aNewValue = originalNode->getValue();
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
    sharable::ValueNode * originalNode = getOriginalNode();

    // discard the change
    m_aNewValue = originalNode->getValue();
    m_bToDefault = false;

    m_bChange= false;
}
//-----------------------------------------------------------------------------
ValueChangeImpl* ValueMemberNode::DeferredImpl::collectChange()
{
    sharable::ValueNode * originalNode = getOriginalNode();

    com::sun::star::uno::Any aOldValue = originalNode->getValue();
    if (!m_bChange)
    {
        return NULL;
    }
    else if (m_bToDefault)
    {
        OSL_ASSERT( m_aNewValue == originalNode->getDefaultValue() );
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
