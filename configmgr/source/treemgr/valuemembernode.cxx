/*************************************************************************
 *
 *  $RCSfile: valuemembernode.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:49 $
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

#include "valuemembernode.hxx"

#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#include "nodeimplobj.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
void setOriginalValue(memory::UpdateAccessor& _aAccessor, data::ValueNodeAddress const& rOriginalAddress, UnoAny const& aNewValue)
{
    data::ValueNodeAccess::setValue(_aAccessor,rOriginalAddress,aNewValue);
}
//-----------------------------------------------------------------------------

inline
void setOriginalToDefault(memory::UpdateAccessor& _aAccessor, data::ValueNodeAddress const& rOriginalAddress)
{
    data::ValueNodeAccess::setToDefault(_aAccessor,rOriginalAddress);
}
//-----------------------------------------------------------------------------
/*
void ValueMemberNode::DeferredImpl::commitDirect(data::Accessor const& _aAccessor)
{
    data::ValueNodeAccess aOriginalNode = getOriginalNode(_aAccessor);

    if (isChange())
    {
        if (m_bToDefault)
            setOriginalToDefault(directValueAccess(aOriginalNode));

        else
            setOriginalValue(directValueAccess(aOriginalNode),m_aNewValue);
    }

    OSL_ENSURE( m_aNewValue == aOriginalNode.getValue(), "Direct Commit: Inconsistent committed value");
    m_bChange = false;
}
*/
//-----------------------------------------------------------------------------
} // anonymous namespace


//-----------------------------------------------------------------------------
// class ValueMemberNode
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(data::ValueNodeAccess const& _aNodeAccess)
: m_aNodeRef(_aNodeAccess)
, m_xDeferredOperation()
{}
//-----------------------------------------------------------------------------
ValueMemberNode::ValueMemberNode(data::Accessor const& _aAccessor, DeferredImplRef const& _xDeferred) // must be valid
: m_aNodeRef( _xDeferred->getOriginalNode(_aAccessor) )
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
                 m_xDeferredOperation->getOriginalNodeAddress() == m_aNodeRef.address() );

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

    else if (memory::Segment * pUpdatableSegment = m_pStrategy->getDataSegmentForUpdate())
    {
        memory::UpdateAccessor aUpdater(pUpdatableSegment);
        setOriginalValue( aUpdater, m_aMemberNode.m_aNodeRef.address(), aNewValue );
    }
}
//-----------------------------------------------------------------------------

void ValueMemberUpdate::setDefault()
{
    if (m_aMemberNode.m_xDeferredOperation.is())
        m_aMemberNode.m_xDeferredOperation->setValueToDefault(m_aMemberNode.m_aNodeRef);

    else if (memory::Segment * pUpdatableSegment = m_pStrategy->getDataSegmentForUpdate())
    {
        memory::UpdateAccessor aUpdater(pUpdatableSegment);
        setOriginalToDefault( aUpdater, m_aMemberNode.m_aNodeRef.address() );
    }
}

//-----------------------------------------------------------------------------
// class ValueMemberNode::DeferredImpl
//-----------------------------------------------------------------------------

ValueMemberNode::DeferredImpl::DeferredImpl(data::ValueNodeAccess const& _aValueNode)
: m_aValueRef(_aValueNode.address())
, m_aNewValue(_aValueNode.getValue())
, m_bToDefault(false)
, m_bChange(false)
{}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValue(UnoAny const& aNewValue, data::ValueNodeAccess const& _aOriginalNode)
{
    OSL_ENSURE(_aOriginalNode.address() == m_aValueRef, "Incorrect original node passed");

    m_aNewValue = aNewValue;
    m_bToDefault = false;

    m_bChange = _aOriginalNode.isDefault() || aNewValue != _aOriginalNode.getValue();
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValueToDefault(data::ValueNodeAccess const& _aOriginalNode)
{
    OSL_ENSURE(_aOriginalNode.address() == m_aValueRef, "Incorrect original node passed");

    m_aNewValue = _aOriginalNode.getDefaultValue();
    m_bToDefault = true;

    m_bChange = !_aOriginalNode.isDefault();
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueChange> ValueMemberNode::DeferredImpl::preCommitChange(data::Accessor const& _aAccessor)
{
    OSL_ENSURE(isChange(), "Trying to commit a non-change");

    data::ValueNodeAccess aOriginalNode = getOriginalNode(_aAccessor);

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

void ValueMemberNode::DeferredImpl::finishCommit(ValueChange& rChange, data::Accessor const& _aAccessor)
{
    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Committed change does not match the intended value");

    data::ValueNodeAccess aOriginalNode = getOriginalNode(_aAccessor);

    m_aNewValue = aOriginalNode.getValue();
    m_bToDefault = false;

    OSL_ENSURE(rChange.getNewValue() == m_aNewValue,"Committed change does not match the actual value");
    m_bChange= false;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::revertCommit(ValueChange& rChange, data::Accessor const& )
{
    //data::ValueNodeAccess aOriginalNode = getOriginalNode(_aAccessor):

    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Reverted change does not match the intended value");
    OSL_ENSURE(isChange(), "ValueMemeberNode::DeferredImpl: No Changes to revert");
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::failedCommit(ValueChange& , data::Accessor const& _aAccessor)
{
    data::ValueNodeAccess aOriginalNode = getOriginalNode(_aAccessor);

    // discard the change
    m_aNewValue = aOriginalNode.getValue();
    m_bToDefault = false;

    m_bChange= false;
}
//-----------------------------------------------------------------------------
ValueChangeImpl* ValueMemberNode::DeferredImpl::collectChange(data::Accessor const& _aAccessor)
{
    data::ValueNodeAccess aOriginalNode = getOriginalNode(_aAccessor);

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
