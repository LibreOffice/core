/*************************************************************************
 *
 *  $RCSfile: changes.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-21 12:26:09 $
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
#include "cmtreemodel.hxx"
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace configmgr;

//==========================================================================
//= ValueChange
//==========================================================================
// -------------------------------------------------------------------------
ValueChange::ValueChange(OUString const& _rName,Any aNewValue, const configuration::Attributes& _rAttributes, Mode aMode, Any aOldValue)
    : Change(_rName)
     ,m_aValue(aNewValue)
     ,m_aOldValue(aOldValue)
     ,m_eMode(aMode)
     ,m_aAttributes(_rAttributes)
{
}
// -------------------------------------------------------------------------
ValueChange::ValueChange(Any aNewValue, ValueNode const& aOldValue)
    : Change(aOldValue.getName())
     ,m_aValue(aNewValue)
     ,m_aOldValue(aOldValue.getValue())
     ,m_aAttributes(aOldValue.getAttributes())
{
    m_eMode = aOldValue.isDefault() ? wasDefault : changeValue;
}
// -------------------------------------------------------------------------
ValueChange::ValueChange(SetToDefault, ValueNode const& aOldValue)
    : Change(aOldValue.getName())
     ,m_aValue(aOldValue.getDefault())
     ,m_aOldValue(aOldValue.getValue())
     ,m_eMode(setToDefault)
     ,m_aAttributes(aOldValue.getAttributes())
{
}

// -------------------------------------------------------------------------
ValueChange::ValueChange(const ValueChange& _rChange)
            :Change(_rChange.getNodeName())
            ,m_aValue(_rChange.getNewValue())
            ,m_aOldValue(_rChange.getOldValue())
            ,m_eMode(_rChange.getMode())
            ,m_aAttributes(_rChange.getAttributes())
{}

// -----------------------------------------------------------------------------
Change* ValueChange::clone() const
{
    return new ValueChange(*this);
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
//--------------------------------------------------------------------------
AddNode::AddNode(std::auto_ptr<INode> aNewNode_, OUString const& _rName)
    :Change(_rName)
    ,m_aOwnNewNode(aNewNode_)
    ,m_aOwnOldNode()
    ,m_pOldNode(0)
    ,m_bReplacing(false)
{
    m_pNewNode = m_aOwnNewNode.get();
}

//--------------------------------------------------------------------------
AddNode::~AddNode()
{
}

// -----------------------------------------------------------------------------
AddNode::AddNode(const AddNode& _aObj)
        : Change(_aObj), m_bReplacing(_aObj.m_bReplacing)
{
    m_pNewNode = _aObj.m_pNewNode ? _aObj.m_pNewNode->clone() : NULL;
    m_pOldNode = _aObj.m_pOldNode ? _aObj.m_pOldNode->clone() : NULL;
    if (_aObj.m_aOwnNewNode.get())
        m_aOwnNewNode.reset(_aObj.m_aOwnNewNode.get()->clone());
    if (_aObj.m_aOwnOldNode.get())
        m_aOwnOldNode.reset(_aObj.m_aOwnOldNode.get()->clone());
}

// -----------------------------------------------------------------------------
Change* AddNode::clone() const
{
    return new AddNode(*this);
}

//--------------------------------------------------------------------------
void AddNode::expectReplacedNode(INode* pOldNode)
{
    if (pOldNode != m_aOwnOldNode.get())
    {
        OSL_ENSURE(!m_aOwnOldNode.get(), "This RemoveNode already owns a Node - throwing that away");
        m_aOwnOldNode.reset();
    }
    m_pOldNode = pOldNode;
}
//--------------------------------------------------------------------------

void AddNode::takeReplacedNode(std::auto_ptr<INode> aNode)
{
    OSL_ENSURE(m_pOldNode == 0 || m_pOldNode == aNode.get(), "Removing unexpected Node");
    m_aOwnOldNode = aNode;
    m_pOldNode = m_aOwnOldNode.get();
}


//==========================================================================
//= RemoveNode
//==========================================================================
RemoveNode::RemoveNode(OUString const& _rName)
    :Change(_rName)
    ,m_aOwnOldNode()
    ,m_pOldNode(0)
{
}

//--------------------------------------------------------------------------
RemoveNode::~RemoveNode()
{
}
// -----------------------------------------------------------------------------
RemoveNode::RemoveNode(const RemoveNode& _aObj)
        :Change(_aObj)
{
    m_pOldNode = _aObj.m_pOldNode ? _aObj.m_pOldNode->clone() : NULL;
    if (_aObj.m_aOwnOldNode.get())
        m_aOwnOldNode.reset(_aObj.m_aOwnOldNode.get()->clone());
}

// -----------------------------------------------------------------------------
Change* RemoveNode::clone() const
{
    return new RemoveNode(*this);
}
//--------------------------------------------------------------------------

void RemoveNode::expectRemovedNode(INode* pOldNode)
{
    if (pOldNode != m_aOwnOldNode.get())
    {
        OSL_ENSURE(!m_aOwnOldNode.get(), "This RemoveNode already owns a Node - throwing that away");
        m_aOwnOldNode.reset();
    }
    m_pOldNode = pOldNode;
}
//--------------------------------------------------------------------------

void RemoveNode::takeRemovedNode(std::auto_ptr<INode> aNode)
{
    OSL_ENSURE(m_pOldNode == 0 || m_pOldNode == aNode.get(), "Removing unexpected Node");
    m_aOwnOldNode = aNode;
    m_pOldNode = m_aOwnOldNode.get();
}


