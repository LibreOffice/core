/*************************************************************************
 *
 *  $RCSfile: valuemembernode.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:40:28 $
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
#include "nodeimplobj.hxx"

#include "nodechangeimpl.hxx"

#include "valuenode.hxx"
#include "change.hxx"

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
// internal accessors for original data
//-----------------------------------------------------------------------------

inline
OUString getOriginalNodeName(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    return pOriginal->getName();
}
//-----------------------------------------------------------------------------

inline
Attributes getOriginalNodeAttributes(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    Attributes aAttributes = pOriginal->getAttributes();
    return aAttributes;
}
//-----------------------------------------------------------------------------


inline
bool isOriginalNodeDefault(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    return pOriginal->isDefault();
}
//-----------------------------------------------------------------------------

inline
bool hasOriginalNodeDefault(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    return pOriginal->hasDefault();
}
//-----------------------------------------------------------------------------

inline
UnoAny  getOriginalNodeValue(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    return pOriginal->getValue();
}
//-----------------------------------------------------------------------------

inline
UnoAny getOriginalDefaultValue(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    return pOriginal->getDefault();
}
//-----------------------------------------------------------------------------

inline
UnoType getOriginalValueType(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    return pOriginal->getValueType();
}
//-----------------------------------------------------------------------------

inline
void setOriginalValue(ValueNode* pOriginal, UnoAny const& aNewValue)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    pOriginal->setValue(aNewValue);
}
//-----------------------------------------------------------------------------

inline
void setOriginalToDefault(ValueNode* pOriginal)
{
    OSL_ENSURE( pOriginal, "Value Member Operation requires valid node");
    pOriginal->setDefault();
}
//-----------------------------------------------------------------------------
} // anonymous namespace

//-----------------------------------------------------------------------------
// class ValueMemberNode
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(ValueNode* pOriginal)
: m_pOriginal(pOriginal)
, m_xDeferredOperation()
{}
//-----------------------------------------------------------------------------
ValueMemberNode::ValueMemberNode(DeferredImplRef const& xOriginal) // must be valid
: m_pOriginal( &xOriginal->getOriginalNode() )
, m_xDeferredOperation(xOriginal)
{}
//-----------------------------------------------------------------------------

ValueMemberNode::ValueMemberNode(ValueMemberNode const& rOriginal)
: m_pOriginal(rOriginal.m_pOriginal)
, m_xDeferredOperation(rOriginal.m_xDeferredOperation)
{}
//-----------------------------------------------------------------------------
ValueMemberNode& ValueMemberNode::operator=(ValueMemberNode const& rOriginal)
{
    m_pOriginal             = rOriginal.m_pOriginal;
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
    OSL_ASSERT( m_xDeferredOperation.isEmpty() ||
                & m_xDeferredOperation->getOriginalNode() == m_pOriginal );

    return m_pOriginal != NULL;
}
//-----------------------------------------------------------------------------

bool ValueMemberNode::hasChange() const
{
    return m_xDeferredOperation.isValid()
           && m_xDeferredOperation->isChange();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// external accessors
//-----------------------------------------------------------------------------

Name ValueMemberNode::getNodeName() const
{
    return Name( getOriginalNodeName(m_pOriginal), Name::NoValidate() );
}
//-----------------------------------------------------------------------------

Attributes ValueMemberNode::getAttributes() const
{
    return getOriginalNodeAttributes(m_pOriginal);
}
//-----------------------------------------------------------------------------


bool ValueMemberNode::isDefault() const
{
    if (hasChange())
        return m_xDeferredOperation->isToDefault();

    return isOriginalNodeDefault(m_pOriginal);
}
//-----------------------------------------------------------------------------

bool ValueMemberNode::canGetDefaultValue() const
{
    if (this->getAttributes().bDefaultable)
        return true;

    return hasOriginalNodeDefault(m_pOriginal);
}
//-----------------------------------------------------------------------------

UnoAny  ValueMemberNode::getValue() const
{
    if (hasChange())
        return m_xDeferredOperation->getNewValue();

    return getOriginalNodeValue(m_pOriginal);
}
//-----------------------------------------------------------------------------

UnoAny ValueMemberNode::getDefaultValue() const
{
    return getOriginalDefaultValue(m_pOriginal);
}
//-----------------------------------------------------------------------------

UnoType ValueMemberNode::getValueType() const
{
    return getOriginalValueType(m_pOriginal);
}
//-----------------------------------------------------------------------------


void ValueMemberUpdate::setValue(UnoAny const& aNewValue)
{
    if (m_aMemberNode.m_xDeferredOperation.isValid())
        m_aMemberNode.m_xDeferredOperation->setValue(aNewValue);

    else
        setOriginalValue(m_aMemberNode.m_pOriginal,aNewValue);
}
//-----------------------------------------------------------------------------

void ValueMemberUpdate::setDefault()
{
    if (m_aMemberNode.m_xDeferredOperation.isValid())
        m_aMemberNode.m_xDeferredOperation->setValueToDefault();

    else
        setOriginalToDefault(m_aMemberNode.m_pOriginal);
}

//-----------------------------------------------------------------------------
// class ValueMemberNode::DeferredImpl : public vos::OReference
//-----------------------------------------------------------------------------

ValueMemberNode::DeferredImpl::DeferredImpl(ValueNode& rOriginal)
: m_rOriginal(rOriginal)
, m_aNewValue( getOriginalNodeValue(&m_rOriginal) )
, m_bToDefault(false)
, m_bChange(false)
{}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValue(UnoAny const& aNewValue)
{
    m_aNewValue = aNewValue;
    m_bToDefault = false;

    m_bChange = isOriginalNodeDefault(&m_rOriginal) || aNewValue != getOriginalNodeValue(&m_rOriginal);
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::setValueToDefault()
{
    m_aNewValue = getOriginalDefaultValue( &m_rOriginal );
    m_bToDefault = true;

    m_bChange = ! isOriginalNodeDefault(&m_rOriginal);
}
//-----------------------------------------------------------------------------

std::auto_ptr<ValueChange> ValueMemberNode::DeferredImpl::preCommitChange()
{
    OSL_ENSURE(isChange(), "Trying to commit a non-change");

    // first find the mode of the change
    ValueChange::Mode eMode;

    if (m_bToDefault)
        eMode = ValueChange::setToDefault;

    else if (! isOriginalNodeDefault(&m_rOriginal))
        eMode = ValueChange::changeValue;

    else if ( getOriginalValueType(&m_rOriginal).getTypeClass() == uno::TypeClass_ANY)
        eMode = ValueChange::typeIsAny;

    else
        eMode = ValueChange::wasDefault;

    // now make a ValueChange
    std::auto_ptr<ValueChange>pChange( new ValueChange( getOriginalNodeName(&m_rOriginal), this->getNewValue(),
                                                        getOriginalNodeAttributes(&m_rOriginal), eMode,
                                                        getOriginalNodeValue(&m_rOriginal)
                                                      ) );

    return  pChange;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::finishCommit(ValueChange& rChange)
{
    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Committed change does not match the intended value");

    m_aNewValue = getOriginalNodeValue(&m_rOriginal);
    m_bToDefault = false;

    OSL_ENSURE(rChange.getNewValue() == m_aNewValue,"Committed change does not match the actual value");
    m_bChange= false;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::revertCommit(ValueChange& rChange)
{
    OSL_ENSURE(rChange.getNewValue() == this->getNewValue(),"Reverted change does not match the intended value");
    OSL_ENSURE(isChange(), "ValueMemeberNode::DeferredImpl: No Changes to revert");
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::failedCommit(ValueChange& )
{
    // discard the change
    m_aNewValue = getOriginalNodeValue(&m_rOriginal);
    m_bToDefault = false;

    m_bChange= false;
}
//-----------------------------------------------------------------------------

void ValueMemberNode::DeferredImpl::commitDirect()
{
    if (isChange())
    {
        if (m_bToDefault)
            setOriginalToDefault(&m_rOriginal);

        else
            setOriginalValue(&m_rOriginal,m_aNewValue);
    }

    OSL_ENSURE( m_aNewValue == getOriginalNodeValue( &m_rOriginal ), "Direct Commit: Inconsistent committed value");
    m_bChange = false;
}
//-----------------------------------------------------------------------------

ValueChangeImpl* ValueMemberNode::DeferredImpl::collectChange()
{
    UnoAny aOldValue = getOriginalNodeValue(&m_rOriginal);
    if (!m_bChange)
    {
        return NULL;
    }
    else if (m_bToDefault)
    {
        OSL_ASSERT( m_aNewValue == getOriginalDefaultValue(&m_rOriginal) );
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
