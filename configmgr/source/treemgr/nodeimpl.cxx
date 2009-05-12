/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeimpl.cxx,v $
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
#include "nodeimpl.hxx"
#include "valuenodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "nodevisitor.hxx"
#include "tree.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodechangeinfo.hxx"
#include "change.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

// Specific types of nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupNodeImpl
//-----------------------------------------------------------------------------

sharable::GroupNode * GroupNodeImpl::getDataAccess() const
{
    sharable::Node * node = getOriginalNodeAccess();
    OSL_ASSERT(node != 0 && node->isGroup());
    return &node->group;
}
//-----------------------------------------------------------------------------

GroupNodeImpl::GroupNodeImpl(sharable::GroupNode * _pNodeRef)
    : NodeImpl(reinterpret_cast<sharable::Node *>(_pNodeRef))
    , m_pCache( NULL )
{
}
//-----------------------------------------------------------------------------

bool GroupNodeImpl::areValueDefaultsAvailable() const
{
    return getDataAccess()->hasDefaultsAvailable();
}
//-----------------------------------------------------------------------------

ValueMemberNode GroupNodeImpl::makeValueMember(sharable::ValueNode * node)
{
    return ValueMemberNode(node);
}
//-----------------------------------------------------------------------------

sharable::ValueNode * GroupNodeImpl::getOriginalValueNode(rtl::OUString const& _aName) const
{
    OSL_ENSURE( _aName.getLength() != 0, "Cannot get nameless child value");

    sharable::GroupNode * group = getDataAccess();

    if (m_pCache)
    {
        if (m_pCache->isNamed(_aName))
            return m_pCache->valueData();

        m_pCache = group->getNextChild(m_pCache);

        if (m_pCache && m_pCache->isNamed(_aName))
            return m_pCache->valueData();
        m_pCache = NULL;
    }

    sharable::Node * child = group->getChild(_aName);
    m_pCache = child;

    // to do: investigate cache lifecycle more deeply.

    return child == 0 ? 0 : child->valueData();
}

//-----------------------------------------------------------------------------
// class ValueElementNodeImpl
//-----------------------------------------------------------------------------

sharable::ValueNode * ValueElementNodeImpl::getDataAccess() const
{
    sharable::Node * node = getOriginalNodeAccess();
    OSL_ASSERT(node != 0 && node->isValue());
    return &node->value;
}
//-----------------------------------------------------------------------------

ValueElementNodeImpl::ValueElementNodeImpl(sharable::ValueNode * const& _aNodeRef)
    : NodeImpl(reinterpret_cast<sharable::Node *>(_aNodeRef))
{
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Any    ValueElementNodeImpl::getValue() const
{
    return getDataAccess()->getValue();
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Type   ValueElementNodeImpl::getValueType() const
{
    return getDataAccess()->getValueType();
}
//-----------------------------------------------------------------------------
    }
}
