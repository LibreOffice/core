/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: changes.cxx,v $
 * $Revision: 1.22 $
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

#include "change.hxx"
#include <osl/diagnose.h>

namespace configmgr
{

//==========================================================================
//= ValueChange
//==========================================================================

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
ValueChange::ValueChange(rtl::OUString const& _rName,
                         const node::Attributes& _rAttributes,
                         Mode _eMode,
                         uno::Any const & aNewValue, uno::Any const & aOldValue)
    : Change(_rName, isDefaultMode(_eMode))
     ,m_aValueType( implGetValueType(aNewValue,aOldValue) )
     ,m_aValue(aNewValue)
     ,m_aOldValue(aOldValue)
     ,m_aAttributes(_rAttributes)
     ,m_eMode(_eMode)
{
    m_aAttributes.markAsDefault(Change::isToDefault());
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

//==========================================================================
//= AddNode
//==========================================================================
AddNode::AddNode(rtl::Reference< data::TreeSegment > const & _aAddedTree, rtl::OUString const& _rName, bool _bToDefault)
    :Change(_rName,_bToDefault)
    ,m_aOwnNewNode(_aAddedTree)
    ,m_aOwnOldNode()
    ,m_aInsertedTree(NULL)
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
, m_aOwnNewNode(data::TreeSegment::create(_aObj.m_aOwnNewNode))
, m_aOwnOldNode(data::TreeSegment::create(_aObj.m_aOwnOldNode))
, m_aInsertedTree(_aObj.m_aInsertedTree)
, m_bReplacing(_aObj.m_bReplacing)
{
}

// -----------------------------------------------------------------------------
std::auto_ptr<Change> AddNode::clone() const
{
    return std::auto_ptr<Change>(new AddNode(*this));
}

//--------------------------------------------------------------------------
void AddNode::setInsertedAddress(sharable::TreeFragment * const & _aInsertedTree)
{
    OSL_ENSURE( m_aInsertedTree == NULL, "AddNode already was applied - inserted a second time ?");
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

void AddNode::takeReplacedTree(rtl::Reference< data::TreeSegment > const & _aReplacedTree)
{
    m_aOwnOldNode   = _aReplacedTree;

    if (m_aOwnOldNode.is()) m_bReplacing = true;
}


//==========================================================================
//= RemoveNode
//==========================================================================
RemoveNode::RemoveNode(rtl::OUString const& _rName, bool _bToDefault)
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
, m_aOwnOldNode(data::TreeSegment::create(_aObj.m_aOwnOldNode))
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

void RemoveNode::takeRemovedTree(rtl::Reference< data::TreeSegment > const & _aRemovedTree)
{
    m_aOwnOldNode   = _aRemovedTree;
}

//--------------------------------------------------------------------------
}  // namespace configmgr

