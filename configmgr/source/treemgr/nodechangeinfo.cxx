/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodechangeinfo.cxx,v $
 * $Revision: 1.12 $
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

#include "nodechangeinfo.hxx"

#include "noderef.hxx"
#include "tree.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

NodeChangeData::NodeChangeData()
: type(eNoChange)
, unoData()
, element()
{
}
//-----------------------------------------------------------------------------

NodeChangeData::NodeChangeData(NodeChangeData const& aOther)
: type(aOther.type)
, unoData(aOther.unoData)
, element(aOther.element)
{
}
//-----------------------------------------------------------------------------

NodeChangeData& NodeChangeData::operator=(NodeChangeData const& aOther)
{
    type        = aOther.type;
    unoData     = aOther.unoData;
    element     = aOther.element;
    return *this;
}
//-----------------------------------------------------------------------------

NodeChangeData::~NodeChangeData()
{
}
//-----------------------------------------------------------------------------

bool NodeChangeData::isDataChange() const
{
    if (isSetChange() && element.isDataChange())
        return true;

    return unoData.isDataChange();
}
//-----------------------------------------------------------------------------

rtl::Reference< Tree > NodeChangeData::getNewElementTree() const
{
    return element.newValue.get();
}
//-----------------------------------------------------------------------------

rtl::Reference< Tree > NodeChangeData::getOldElementTree() const
{
    return element.oldValue.get();
}
//-----------------------------------------------------------------------------

NodeID NodeChangeData::getNewElementNodeID() const
{
    rtl::Reference<ElementTree> newElement = this->element.newValue;
    if ( newElement.is() &&  newElement->nodeCount() > 0)
    {
        return NodeID( newElement.get(), Tree::ROOT );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeID NodeChangeData::getOldElementNodeID() const
{
    rtl::Reference<ElementTree> oldElement = this->element.oldValue;
    if ( oldElement.is() &&  oldElement->nodeCount() > 0)
    {
        return NodeID( oldElement.get(), Tree::ROOT );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeChangeLocation::NodeChangeLocation()
: m_path()
, m_base(0,0)
, m_affected(0,0)
, m_bSubNodeChanging(false)
{
}
//-----------------------------------------------------------------------------
bool NodeChangeLocation::isValidLocation() const
{
    return   m_base.isValidNode() &&
            (m_affected.isEmpty()
                ?   ! m_bSubNodeChanging
                :   ( m_affected.isValidNode() &&
                      (! m_bSubNodeChanging ||
                         (!m_path.isEmpty() &&
                            SubNodeID(m_affected,m_path.getLocalName().getName()).isValidNode()
                    ) )  ) );
}
#if OSL_DEBUG_LEVEL > 0
//-----------------------------------------------------------------------------
bool NodeChangeLocation::isValidData() const
{
    return   m_base.isValidNode() &&
            (m_affected.isEmpty()
                ?   ! m_bSubNodeChanging
                :   ( m_affected.isValidNode() &&
                      (! m_bSubNodeChanging || !m_path.isEmpty() )
            )       );
}
#endif
//-----------------------------------------------------------------------------

void NodeChangeLocation::setAccessor(RelativePath const& aAccessor)
{
    m_path = aAccessor;
}
//-----------------------------------------------------------------------------

void NodeChangeLocation::setBase(NodeID const& aBaseNode)
{
    m_base = aBaseNode;
}
//-----------------------------------------------------------------------------

void NodeChangeLocation::setAffected(NodeID const& aTargetNode)
{
    m_affected = aTargetNode;

    if (m_base.isEmpty())
        setBase(aTargetNode);
}
//-----------------------------------------------------------------------------

void NodeChangeLocation::setChangingSubnode( bool bSubnode )
{
    OSL_ENSURE(!m_affected.isEmpty() || !bSubnode, "Change without target cannot affect subnode");

    m_bSubNodeChanging = bSubnode;
}
//-----------------------------------------------------------------------------

rtl::Reference< Tree > NodeChangeLocation::getBaseTree() const
{
    OSL_ENSURE(m_base.isValidNode(), "Invalid base location set in NodeChangeLocation");
    return m_base.getTree();
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeLocation::getBaseNode() const
{
    OSL_ENSURE(m_base.isValidNode(), "Invalid base location set in NodeChangeLocation");
    return m_base.getNode();
}
//-----------------------------------------------------------------------------

rtl::Reference< Tree > NodeChangeLocation::getAffectedTreeRef() const
{
    NodeID aAffected = this->getAffectedNodeID();
    return aAffected.getTree();
}
//-----------------------------------------------------------------------------

NodeID NodeChangeLocation::getAffectedNodeID() const
{
    OSL_ENSURE(m_affected.isEmpty() || m_affected.isValidNode(), "Invalid target location set in NodeChangeLocation");
    return m_affected;
}
//-----------------------------------------------------------------------------

SubNodeID NodeChangeLocation::getChangingValueID() const
{
    if (!m_bSubNodeChanging) return SubNodeID::createEmpty();

    OSL_ENSURE(!m_affected.isEmpty() && m_affected.isValidNode(), "Invalid target location set in NodeChangeLocation with subnode");
    OSL_ENSURE(!m_path.isEmpty(), "No target accessor set in NodeChangeLocation with subnode");

    SubNodeID aResult( m_affected, m_path.getLocalName().getName() );

    return aResult;
}
//-----------------------------------------------------------------------------
    }
}

