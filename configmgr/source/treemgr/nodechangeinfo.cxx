/*************************************************************************
 *
 *  $RCSfile: nodechangeinfo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-14 10:53:36 $
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

NodeChangeInfo::NodeChangeInfo()
: type(eNoChange)
, oldValue()
, newValue()
, oldElement()
, newElement()
{
}
//-----------------------------------------------------------------------------

NodeChangeInfo::NodeChangeInfo(NodeChangeInfo const& aOther)
: type(aOther.type)
, oldValue(aOther.oldValue)
, newValue(aOther.newValue)
, oldElement(aOther.oldElement)
, newElement(aOther.newElement)
{
}
//-----------------------------------------------------------------------------

NodeChangeInfo& NodeChangeInfo::operator=(NodeChangeInfo const& aOther)
{
    type        = aOther.type;
    oldValue    = aOther.oldValue;
    newValue    = aOther.newValue;
    oldElement  = aOther.oldElement;
    newElement  = aOther.newElement;
    return *this;
}
//-----------------------------------------------------------------------------

NodeChangeInfo::~NodeChangeInfo()
{
}
//-----------------------------------------------------------------------------

bool NodeChangeInfo::isChange() const
{
    return !isEmpty() && (isSetChange() ? oldElement != newElement : oldValue != newValue);
}
//-----------------------------------------------------------------------------

Tree NodeChangeInfo::getNewElementTree() const
{
    return Tree( newElement.getBodyPtr() );
}
//-----------------------------------------------------------------------------

Tree NodeChangeInfo::getOldElementTree() const
{
    return Tree( oldElement.getBodyPtr() );
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeInfo::getNewElementNodeRef() const
{
    if ( newElement.isValid() &&  newElement->nodeCount() > 0)
    {
        NodeOffset n = newElement->root();
        return TreeImplHelper::makeNode( newElement.getBody(), n);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeRef NodeChangeInfo::getOldElementNodeRef() const
{
    if ( oldElement.isValid() &&  oldElement->nodeCount() > 0)
    {
        NodeOffset n = oldElement->root();
        return TreeImplHelper::makeNode( oldElement.getBody(), n);
    }
    else
        return NodeRef();
}
//-----------------------------------------------------------------------------

NodeID NodeChangeInfo::getNewElementNodeID() const
{
    if ( newElement.isValid() &&  newElement->nodeCount() > 0)
    {
        return NodeID( newElement.getBodyPtr(), newElement->root() );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

NodeID NodeChangeInfo::getOldElementNodeID() const
{
    if ( oldElement.isValid() &&  oldElement->nodeCount() > 0)
    {
        return NodeID( oldElement.getBodyPtr(), oldElement->root() );
    }
    else
        return NodeID(0,0);
}
//-----------------------------------------------------------------------------

ExtendedNodeChangeInfo::ExtendedNodeChangeInfo()
: change()
, baseTree(0)
, baseNode()
, accessor()
{
}

//-----------------------------------------------------------------------------
    }
}

