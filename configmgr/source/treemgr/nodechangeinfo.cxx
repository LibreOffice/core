/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodechangeinfo.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:43:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "nodechangeinfo.hxx"

#include "noderef.hxx"
#include "treeimpl.hxx"

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

Tree NodeChangeData::getNewElementTree() const
{
    return Tree( element.newValue.get() );
}
//-----------------------------------------------------------------------------

Tree NodeChangeData::getOldElementTree() const
{
    return Tree( element.oldValue.get() );
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeData::getNewElementNodeRef() const
{
    ElementTreeHolder newElement = this->element.newValue;
    if ( newElement.is() &&  newElement->nodeCount() > 0)
    {
        NodeOffset n = newElement->root_();
        return TreeImplHelper::makeNode( *newElement, n);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeData::getOldElementNodeRef() const
{
    ElementTreeHolder oldElement = this->element.oldValue;
    if ( oldElement.is() &&  oldElement->nodeCount() > 0)
    {
        NodeOffset n = oldElement->root_();
        return TreeImplHelper::makeNode( *oldElement, n);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeID NodeChangeData::getNewElementNodeID() const
{
    ElementTreeHolder newElement = this->element.newValue;
    if ( newElement.is() &&  newElement->nodeCount() > 0)
    {
        return NodeID( newElement.get(), newElement->root_() );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeID NodeChangeData::getOldElementNodeID() const
{
    ElementTreeHolder oldElement = this->element.oldValue;
    if ( oldElement.is() &&  oldElement->nodeCount() > 0)
    {
        return NodeID( oldElement.get(), oldElement->root_() );
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

Tree NodeChangeLocation::getBaseTree() const
{
    OSL_ENSURE(m_base.isValidNode(), "Invalid base location set in NodeChangeLocation");
    return Tree( TreeImplHelper::tree(m_base) );
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeLocation::getBaseNode() const
{
    OSL_ENSURE(m_base.isValidNode(), "Invalid base location set in NodeChangeLocation");
    return TreeImplHelper::makeNode(m_base);
}
//-----------------------------------------------------------------------------

TreeRef NodeChangeLocation::getAffectedTreeRef() const
{
    NodeID aAffected = this->getAffectedNodeID();
    return TreeRef( TreeImplHelper::tree(aAffected) );
}
//-----------------------------------------------------------------------------

Tree NodeChangeLocation::getAffectedTree() const
{
    return Tree( getAffectedTreeRef() );
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeLocation::getAffectedNode() const
{
    NodeID aAffected = this->getAffectedNodeID();
    return TreeImplHelper::makeNode(aAffected);
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

