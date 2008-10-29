/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: collectchanges.cxx,v $
 * $Revision: 1.11 $
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

#include <string.h>
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

bool convertNodeChange(NodeChangeData& aData_, RemoveNode const& /*aChange_*/)
{
    aData_.type = NodeChangeData::eRemoveElement;

    return true;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CollectChanges visitor class
//-----------------------------------------------------------------------------

CollectChanges::CollectChanges( NodeChangesInformation& rTargetList_,
                Tree& rStartTree_, unsigned int nStartNode_,
                rtl::Reference<Template> aElementTemplate_,
                unsigned int nMaxDepth)
: m_rTargetList(rTargetList_)
, m_aAccessor()
, m_aContextTypeName()
, m_pBaseTree(&rStartTree_)
, m_nBaseNode(nStartNode_)
, m_nDepthLeft( nMaxDepth )
{
    if (aElementTemplate_.is())
        m_aContextTypeName = aElementTemplate_->getName();
}

//-----------------------------------------------------------------------------
CollectChanges::CollectChanges( CollectChanges const& rBase, Path::Component const& rChildName, rtl::OUString const& aSubTypeName_)
: m_rTargetList(rBase.m_rTargetList)
, m_aAccessor(rBase.m_aAccessor.compose(rChildName))
, m_aContextTypeName(aSubTypeName_)
, m_pBaseTree(rBase.m_pBaseTree)
, m_nBaseNode(rBase.m_nBaseNode)
, m_nDepthLeft(childDepth(rBase.m_nDepthLeft))
{
    OSL_ASSERT(rBase.m_nDepthLeft > 0);
}

//-----------------------------------------------------------------------------
inline
Path::Component CollectChanges::implGetNodeName(Change const& aChange_) const
{
    rtl::OUString aSimpleNodeName( aChange_.getNodeName() );

    if (m_aContextTypeName.getLength() == 0)
    {
        OSL_ENSURE(isSimpleName(aSimpleNodeName),"Unexpected: Found non-simple name without a type");
        return Path::wrapSafeName(aSimpleNodeName);
    }
    else
        return Path::makeCompositeName(aSimpleNodeName, m_aContextTypeName);
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
        rtl::OUString aSubTypeName( aChanges_.getElementTemplateName() );

        CollectChanges aSubcollector( *this, implGetNodeName(aChanges_), aSubTypeName );

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
        rLocation_.setAffected( aBaseID );

    Path::Component aChangeName = implGetNodeName( aOriginal_ );
    rLocation_.setAccessor( m_aAccessor.compose( aChangeName ) );

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

