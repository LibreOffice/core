/*************************************************************************
 *
 *  $RCSfile: collectchanges.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:22:35 $
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

#include "collectchanges.hxx"

#include "nodechangeinfo.hxx"

#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
// conversion helper function
//-----------------------------------------------------------------------------
bool convertNodeChange(NodeChangeData& aData_, ValueChange const& aChange_)
{
    switch(aChange_.getMode())
    {
    case ValueChange::wasDefault:
    case ValueChange::typeIsAny:
    case ValueChange::changeValue:
        aData_.type = NodeChangeData::eSetValue;
        break;

    case ValueChange::setToDefault:
        aData_.type = NodeChangeData::eSetDefault;
        break;

    case ValueChange::changeDefault:
        aData_.type = NodeChangeData::eNoChange; // ??
        break;

    default:
        OSL_ENSURE(false,"Unknown change type found");
        return false;
    }

    aData_.unoData.newValue = aChange_.getNewValue();
    aData_.unoData.oldValue = aChange_.getOldValue();
    return true;
}
//-----------------------------------------------------------------------------

bool convertNodeChange(NodeChangeData& aData_, AddNode const& aChange_)
{
    aData_.type = aChange_.isReplacing()
                    ? NodeChangeData::eReplaceElement
                    : NodeChangeData::eInsertElement;

    return true;
}
//-----------------------------------------------------------------------------

bool convertNodeChange(NodeChangeData& aData_, RemoveNode const& aChange_)
{
    aData_.type = NodeChangeData::eRemoveElement;

    return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CollectChanges visitor class
//-----------------------------------------------------------------------------

CollectChanges::CollectChanges( NodeChangesInformation& rTargetList_,
                TreeImpl& rStartTree_, NodeOffset nStartNode_,
                TreeDepth nMaxDepth)
: m_rTargetList(rTargetList_)
, m_aAccessor()
, m_pBaseTree(&rStartTree_)
, m_nBaseNode(nStartNode_)
, m_nDepthLeft( nMaxDepth )
{
}

//-----------------------------------------------------------------------------
CollectChanges::CollectChanges( CollectChanges const& rBase, Name const& rChildName)
: m_rTargetList(rBase.m_rTargetList)
, m_aAccessor(rBase.m_aAccessor.child(rChildName))
, m_pBaseTree(rBase.m_pBaseTree)
, m_nBaseNode(rBase.m_nBaseNode)
, m_nDepthLeft(childDepth(rBase.m_nDepthLeft))
{
    OSL_ASSERT(rBase.m_nDepthLeft > 0);
}

//-----------------------------------------------------------------------------
void CollectChanges::collectFrom(ValueChange const& aChange_)
{
    NodeChangeInformation aInfo;

    if ( convertNodeChange( aInfo.change, aChange_ ) &&
         implSetLocation( aInfo.location, aChange_, false ) )
    {
        implAdd( aInfo );
    }
}

//-----------------------------------------------------------------------------
void CollectChanges::collectFrom(AddNode const& aChange_)
{
    NodeChangeInformation aInfo;

    if ( convertNodeChange( aInfo.change, aChange_ ) &&
         implSetLocation( aInfo.location, aChange_, true ) )
    {
        implAdd( aInfo );
    }
}

//-----------------------------------------------------------------------------
void CollectChanges::collectFrom(RemoveNode const& aChange_)
{
    NodeChangeInformation aInfo;

    if ( convertNodeChange( aInfo.change, aChange_ ) &&
         implSetLocation( aInfo.location, aChange_, true ) )
    {
        implAdd( aInfo );
    }
}

//-----------------------------------------------------------------------------
void CollectChanges::collectFrom(SubtreeChange const& aChanges_)
{
    if (m_nDepthLeft > 0)
    {
        Name aNodeName( aChanges_.getNodeName(), Name::NoValidate() );

        CollectChanges aSubcollector( *this, aNodeName );

        aSubcollector.applyToChildren(aChanges_);
    }
}

//-----------------------------------------------------------------------------
void CollectChanges::implAdd(NodeChangeInformation const& aChangeInfo_)
{
    m_rTargetList.push_back(aChangeInfo_);
}

//-----------------------------------------------------------------------------
bool CollectChanges::implSetLocation(NodeChangeLocation& rLocation_, Change const& aOriginal_, bool bSet_) const
{
    NodeID aBaseID(m_pBaseTree,m_nBaseNode);
    if (aBaseID.isEmpty())
        return false;

    rLocation_.setBase( aBaseID );

    if (bSet_ && m_aAccessor.isEmpty()) // It is  a set change affecting the base ...
        rLocation_.setTarget( aBaseID );

    Name aChangeName( aOriginal_.getNodeName(), Name::NoValidate() );
    rLocation_.setAccessor( m_aAccessor.child( aChangeName ) );

    return true;
}

// ChangeTreeAction implementations
//-----------------------------------------------------------------------------
void CollectChanges::handle(ValueChange const& aValueNode_)
{
    collectFrom(aValueNode_);
}

//-----------------------------------------------------------------------------
void CollectChanges::handle(AddNode const& aAddNode_)
{
    collectFrom(aAddNode_);
}

//-----------------------------------------------------------------------------
void CollectChanges::handle(RemoveNode const& aRemoveNode_)
{
    collectFrom(aRemoveNode_);
}

//-----------------------------------------------------------------------------
void CollectChanges::handle(SubtreeChange const& aSubtree_)
{
    collectFrom( aSubtree_ );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

    }
}

