/*************************************************************************
 *
 *  $RCSfile: nodechangeinfo.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:21:19 $
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
    return Tree( element.newValue.getBodyPtr() );
}
//-----------------------------------------------------------------------------

Tree NodeChangeData::getOldElementTree() const
{
    return Tree( element.oldValue.getBodyPtr() );
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeData::getNewElementNodeRef() const
{
    ElementTreeHolder newElement = this->element.newValue;
    if ( newElement.isValid() &&  newElement->nodeCount() > 0)
    {
        NodeOffset n = newElement->root();
        return TreeImplHelper::makeNode( newElement.getBody(), n);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeData::getOldElementNodeRef() const
{
    ElementTreeHolder oldElement = this->element.oldValue;
    if ( oldElement.isValid() &&  oldElement->nodeCount() > 0)
    {
        NodeOffset n = oldElement->root();
        return TreeImplHelper::makeNode( oldElement.getBody(), n);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeID NodeChangeData::getNewElementNodeID() const
{
    ElementTreeHolder newElement = this->element.newValue;
    if ( newElement.isValid() &&  newElement->nodeCount() > 0)
    {
        return NodeID( newElement.getBodyPtr(), newElement->root() );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeID NodeChangeData::getOldElementNodeID() const
{
    ElementTreeHolder oldElement = this->element.oldValue;
    if ( oldElement.isValid() &&  oldElement->nodeCount() > 0)
    {
        return NodeID( oldElement.getBodyPtr(), oldElement->root() );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeChangeLocation::NodeChangeLocation()
: m_path()
, m_base(0,0)
, m_target(0,0)
, m_changed(0,0)
{
}
//-----------------------------------------------------------------------------

bool NodeChangeLocation::isValidLocation() const
{
    // TODO: Validate that base,target and accessor relate correctly (?)
    return   m_base.isValidNode() &&
            (m_target.isEmpty()
                ?   m_changed.isEmpty()
                :   ( m_target.isValidNode() &&
                      (m_changed.isEmpty() || m_changed.isValidNode()) ) );
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

void NodeChangeLocation::setTarget(NodeID const& aTargetNode)
{
    m_target = aTargetNode;

    if (m_base.isEmpty())
        setBase(aTargetNode);
}
//-----------------------------------------------------------------------------

void NodeChangeLocation::setChanging(NodeID const& aChangedNode)
{
    m_changed = aChangedNode;

    if (m_target.isEmpty())
        setTarget(aChangedNode);
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
    return TreeImplHelper::makeNode(m_base);
}
//-----------------------------------------------------------------------------

Tree NodeChangeLocation::getAffectedTree() const
{
    NodeID aAffected = this->getAffectedNodeID();
    return Tree( TreeImplHelper::tree(aAffected) );
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
    OSL_ENSURE(m_target.isEmpty() || m_target.isValidNode(), "Invalid target location set in NodeChangeLocation");
    return m_target;
}
//-----------------------------------------------------------------------------

NodeID NodeChangeLocation::getChangedNodeID() const
{
    OSL_ENSURE(m_changed.isEmpty() || m_changed.isValidNode(), "Invalid change location set in NodeChangeLocation");
    return m_changed;
}
//-----------------------------------------------------------------------------
    }
}

