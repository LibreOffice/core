/*************************************************************************
 *
 *  $RCSfile: changes.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: jb $ $Date: 2002-02-15 14:34:34 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
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

#include "change.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{

//==========================================================================
//= ValueChange
//==========================================================================
// -------------------------------------------------------------------------

// works reliably only if old value is set and the value really changes
uno::Type implGetValueType(uno::Any const & _aValue, uno::Any const & _aOldValue)
{
    if (_aValue.hasValue())
    {
        OSL_ENSURE(!_aOldValue.hasValue() || _aOldValue.getValueType() == _aValue.getValueType(),
                    "ERROR: Type mismatch in value change");

        return _aValue.getValueType();
    }
    else
    {
        OSL_ENSURE(_aOldValue.hasValue(),"WARNING: Cannot determine value type of change");
        return _aOldValue.getValueType();
    }
}
// -------------------------------------------------------------------------
static inline bool isDefaultMode(ValueChange::Mode _eMode)
{ return (_eMode == ValueChange::setToDefault) || (_eMode == ValueChange::changeDefault); }
// -------------------------------------------------------------------------
static inline bool isLayerChangeMode(ValueChange::Mode _eMode)
{ return (_eMode == ValueChange::setToDefault) || (_eMode == ValueChange::wasDefault); }
// -----------------------------------------------------------------------------
ValueChange::ValueChange(OUString const& _rName,
                         const node::Attributes& _rAttributes,
                         Mode _eMode,
                         Any const & aNewValue, Any const & aOldValue)
    : Change(_rName, isDefaultMode(_eMode))
     ,m_aValueType( implGetValueType(aNewValue,aOldValue) )
     ,m_aValue(aNewValue)
     ,m_aOldValue(aOldValue)
     ,m_eMode(_eMode)
     ,m_aAttributes(_rAttributes)
{
    m_aAttributes.markAsDefault(Change::isToDefault());
}
// -----------------------------------------------------------------------------
ValueChange::ValueChange(OUString const& _rName,
                         const node::Attributes& _rAttributes,
                         Mode _eMode,
                         uno::Type const & aValueType)
    : Change(_rName, isDefaultMode(_eMode))
     ,m_aValueType( aValueType )
     ,m_aValue()
     ,m_aOldValue()
     ,m_eMode(_eMode)
     ,m_aAttributes(_rAttributes)
{
    m_aAttributes.markAsDefault(Change::isToDefault());
}
// -------------------------------------------------------------------------
ValueChange::ValueChange(Any const & aNewValue, ValueNode const& aOldValue)
    : Change(aOldValue.getName(),false)
     ,m_aValueType( aOldValue.getValueType() )
     ,m_aValue(aNewValue)
     ,m_aOldValue(aOldValue.getValue())
     ,m_aAttributes(aOldValue.getAttributes())
{
    OSL_ENSURE(aNewValue.getValueType() == m_aValueType || !aNewValue.hasValue(),
                "ValueChange: Type mismatch in new value" );

    m_eMode = aOldValue.isDefault() ? wasDefault : changeValue;
    m_aAttributes.markAsDefault(false);
}
// -------------------------------------------------------------------------
ValueChange::ValueChange(SetToDefault, ValueNode const& aOldValue)
    : Change(aOldValue.getName(),true)
     ,m_aValueType( aOldValue.getValueType() )
     ,m_aValue(aOldValue.getDefault())
     ,m_aOldValue(aOldValue.getValue())
     ,m_eMode(setToDefault)
     ,m_aAttributes(aOldValue.getAttributes())
{
    m_aAttributes.markAsDefault(true);
}

// -----------------------------------------------------------------------------
void ValueChange::setNewValue(const uno::Any& _rNewVal)
{
    OSL_ENSURE(_rNewVal.getValueType() == m_aValueType || !_rNewVal.hasValue(),
                "ValueChange: Type mismatch in setNewValue" );

    m_aValue = _rNewVal;
}

// -----------------------------------------------------------------------------
std::auto_ptr<Change> ValueChange::clone() const
{
    return std::auto_ptr<Change>(new ValueChange(*this));
}

// -----------------------------------------------------------------------------
bool ValueChange::isChange() const // makes sense only if old value is set
{
    return isLayerChangeMode(m_eMode) || (m_aOldValue != m_aValue);
}
// -------------------------------------------------------------------------
namespace tree_changes_internal {
    inline void doAdjust(uno::Any& aActual, uno::Any const& aTarget)
    {
        // If set - it should already match
        OSL_ASSERT(!aActual.hasValue() || aTarget == aActual);
        aActual = aTarget;
    }
}
using namespace tree_changes_internal;

// -------------------------------------------------------------------------
void ValueChange::applyTo(ValueNode& aValue)
{
    switch (getMode())
    {
    case wasDefault:
        OSL_ASSERT(aValue.isDefault());
    case changeValue:
        doAdjust( m_aOldValue, aValue.getValue());
        aValue.setValue(getNewValue());
        break;

    case setToDefault:
        doAdjust( m_aOldValue,  aValue.getValue());
        doAdjust( m_aValue,     aValue.getDefault());
        aValue.setDefault();
        break;

    case changeDefault:
        doAdjust( m_aOldValue,  aValue.getDefault());
        aValue.changeDefault(getNewValue());
        break;

    default:
        OSL_ENSURE(0, "Unknown mode found for ValueChange");
        break;
    }
}

// -------------------------------------------------------------------------
void ValueChange::applyChangeNoRecover(ValueNode& aValue) const
{
    switch (getMode())
    {
    case wasDefault:
        OSL_ASSERT(aValue.isDefault());
    case changeValue:
        aValue.setValue(getNewValue());
        break;

    case setToDefault:
        aValue.setDefault();
        break;

    case changeDefault:
        aValue.changeDefault(getNewValue());
        break;

    default:
        OSL_ENSURE(0, "Unknown mode found for ValueChange");
        break;
    }
}

// -------------------------------------------------------------------------
::rtl::OUString ValueChange::getModeAsString() const
{
    ::rtl::OUString aRet;
    switch(m_eMode)
    {
    case wasDefault:
        aRet = ::rtl::OUString::createFromAscii("wasDefault");
        break;
    case changeValue:
        aRet = ::rtl::OUString::createFromAscii("changeValue");
        break;
    case setToDefault:
        aRet = ::rtl::OUString::createFromAscii("setToDefault");
        break;
    case changeDefault:
        aRet = ::rtl::OUString::createFromAscii("changeDefault");
        break;
    default:
        OSL_ENSURE(0,"getModeAsString: Wrong mode!");
    }

    return aRet;
}
// -------------------------------------------------------------------------
void ValueChange::setModeAsString(const ::rtl::OUString& _rMode)
{
    if(_rMode == ::rtl::OUString::createFromAscii("wasDefault"))        m_eMode = wasDefault;
    else if(_rMode == ::rtl::OUString::createFromAscii("changeValue"))  m_eMode = changeValue;
    else if(_rMode == ::rtl::OUString::createFromAscii("setToDefault")) m_eMode = setToDefault;
    else if(_rMode == ::rtl::OUString::createFromAscii("changeDefault"))m_eMode = changeDefault;
    else
    {
        OSL_ENSURE(0,"setModeAsString: Wrong mode!");
    }
}

//==========================================================================
//= AddNode
//==========================================================================
using data::TreeSegment;
//------------------------------------------0--------------------------------
AddNode::AddNode(TreeSegment const & _aAddedTree, OUString const& _rName, bool _bToDefault)
    :Change(_rName,_bToDefault)
    ,m_aOwnNewNode(_aAddedTree)
    ,m_aOwnOldNode()
    ,m_aInsertedTree()
    ,m_bReplacing(false)
{
}

//--------------------------------------------------------------------------
AddNode::~AddNode()
{
}

// -----------------------------------------------------------------------------
AddNode::AddNode(const AddNode& _aObj)
: Change(_aObj)
, m_bReplacing(_aObj.m_bReplacing)
, m_aOwnNewNode(_aObj.m_aOwnNewNode.cloneSegment())
, m_aOwnOldNode(_aObj.m_aOwnOldNode.cloneSegment())
, m_aInsertedTree()
{
}

// -----------------------------------------------------------------------------
std::auto_ptr<Change> AddNode::clone() const
{
    return std::auto_ptr<Change>(new AddNode(*this));
}

//--------------------------------------------------------------------------
void AddNode::setInsertedAddress(data::TreeAddress const & _aInsertedTree)
{
    OSL_ENSURE( !m_aInsertedTree.is(), "AddNode already was applied - inserted a second time ?");
    m_aInsertedTree = _aInsertedTree;
}
//--------------------------------------------------------------------------

#if 0
void AddNode::expectReplacedNode(INode const* pOldNode)
{
    if (pOldNode != m_aOwnOldNode.getRoot())
    {
        OSL_ENSURE(!m_aOwnOldNode.is(), "This AddNode already owns a replaced Node - throwing that away");
        m_aOwnOldNode.clear();
    }
    m_pOldNode = pOldNode;
}
#endif
//--------------------------------------------------------------------------

void AddNode::takeReplacedTree(TreeSegment const & _aReplacedTree)
{
    m_aOwnOldNode   = _aReplacedTree;

    if (m_aOwnOldNode.is()) m_bReplacing = true;
}


//==========================================================================
//= RemoveNode
//==========================================================================
RemoveNode::RemoveNode(OUString const& _rName, bool _bToDefault)
    :Change(_rName,_bToDefault)
    ,m_aOwnOldNode()
{
}

//--------------------------------------------------------------------------
RemoveNode::~RemoveNode()
{
}
// -----------------------------------------------------------------------------
RemoveNode::RemoveNode(const RemoveNode& _aObj)
: Change(_aObj)
, m_aOwnOldNode(_aObj.m_aOwnOldNode.cloneSegment())
{
}

// -----------------------------------------------------------------------------
std::auto_ptr<Change> RemoveNode::clone() const
{
    return std::auto_ptr<Change>(new RemoveNode(*this));
}
//--------------------------------------------------------------------------
#if 0
void RemoveNode::expectRemovedNode(INode const* pOldNode)
{
    if (pOldNode != m_aOwnOldNode.getRoot())
    {
        OSL_ENSURE(!m_aOwnOldNode.is(), "This RemoveNode already owns a Node - throwing that away");
        m_aOwnOldNode.clear();
    }
    m_pOldNode = pOldNode;
}
#endif
//--------------------------------------------------------------------------

void RemoveNode::takeRemovedTree(data::TreeSegment const & _aRemovedTree)
{
    m_aOwnOldNode   = _aRemovedTree;
}

//--------------------------------------------------------------------------
}  // namespace configmgr

