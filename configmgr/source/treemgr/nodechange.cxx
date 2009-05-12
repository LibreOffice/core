/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodechange.cxx,v $
 * $Revision: 1.13 $
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

#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodechangeinfo.hxx"

#include "noderef.hxx"
#include "tree.hxx"

#include <algorithm>
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
// NodeChange handle class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
inline void NodeChange::init()
{
    if (m_pImpl) m_pImpl->acquire();
}
inline void NodeChange::deinit()
{
    if (m_pImpl) m_pImpl->release();
}

//-----------------------------------------------------------------------------
NodeChange::NodeChange()
: m_pImpl(0)
{
}
//-----------------------------------------------------------------------------

NodeChange::NodeChange(NodeChangeImpl* pImpl)
: m_pImpl(pImpl)
{
    init();
}
//-----------------------------------------------------------------------------

NodeChange::NodeChange(NodeChange const& rOther)
: m_pImpl(rOther.m_pImpl)
{
    init();
}
//-----------------------------------------------------------------------------

NodeChange& NodeChange::operator=(NodeChange const& rOther)
{
    NodeChange(rOther).swap(*this);
    return *this;
}
//-----------------------------------------------------------------------------

void NodeChange::swap(NodeChange& rOther)
{
    std::swap(m_pImpl,rOther.m_pImpl);
}
//-----------------------------------------------------------------------------

NodeChange::~NodeChange()
{
    deinit();
}
//-----------------------------------------------------------------------------

bool NodeChange::maybeChange() const
{
    return m_pImpl && m_pImpl->isChange(true);
}
//-----------------------------------------------------------------------------

bool NodeChange::isChange() const
{
    return m_pImpl && m_pImpl->isChange(false);
}
//-----------------------------------------------------------------------------

sal_uInt32 NodeChange::getChangeInfos(NodeChangesInformation& _rInfos) const
{
    sal_uInt32 nCount = 0;
    if (m_pImpl)
    {
        sal_uInt32 nChanges = m_pImpl->getChangeDataCount();

        for (sal_uInt32 ix = 0; ix < nChanges; ++ix)
        {
        NodeChangeInformation aSingleInfo;
            aSingleInfo.change.type = NodeChangeData::eNoChange;

            m_pImpl->fillChangeInfo(aSingleInfo,ix);

            if ( !aSingleInfo.isEmptyChange() )
            {
                _rInfos.push_back(aSingleInfo);
                ++nCount;
            }
        }
    }

    return nCount;
}
//-----------------------------------------------------------------------------

bool NodeChange::getChangeLocation(NodeChangeLocation& rLoc) const
{
    return m_pImpl && m_pImpl->fillChangeLocation(rLoc);
}

//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
rtl::Reference< Tree > NodeChange::getAffectedTree() const
{
    if (this->maybeChange())
        return m_pImpl->getTargetTree().get();
    else
        return NULL;
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
NodeRef NodeChange::getAffectedNode() const
{
    if (this->maybeChange())
    {
        rtl::Reference<Tree> aTree = m_pImpl->getTargetTree();
        unsigned int nOffset = m_pImpl->getTargetNode();

        OSL_ASSERT(aTree.is() && aTree->isValidNode(nOffset));

        if (aTree.is() && nOffset)
            return aTree->getNode(nOffset);
    }
    return NodeRef();
}
//-----------------------------------------------------------------------------

NodeChange& NodeChange::test()
{
    if (m_pImpl) m_pImpl->test();
    return *this;
}

//-----------------------------------------------------------------------------
NodeChange const& NodeChange::test() const
{
    if (m_pImpl) m_pImpl->test();
    return *this;
}

//-----------------------------------------------------------------------------
NodeChange& NodeChange::apply()
{
    if (m_pImpl) m_pImpl->apply();
    return *this;
}

//-----------------------------------------------------------------------------
NodeChange const& NodeChange::apply() const
{
    if (m_pImpl) m_pImpl->apply();
    return *this;
}

//-----------------------------------------------------------------------------
// NodeChanges collection
//-----------------------------------------------------------------------------

NodeChanges::NodeChanges()
: m_aChanges()
{
}
//-----------------------------------------------------------------------------

bool NodeChanges::isEmpty() const
{
    return m_aChanges.empty();
}
//-----------------------------------------------------------------------------

/// predicate for compact

static bool isEmptyChange(NodeChange const& aChange)
{
    return !aChange.maybeChange();
}
//-----------------------------------------------------------------------------


NodeChanges& NodeChanges::compact()
{
    m_aChanges.erase( std::remove_if(begin(),end(),isEmptyChange), end() );
    return *this;
}
//-----------------------------------------------------------------------------

void NodeChanges::implTest() const
{
    for(std::vector<NodeChange>::const_iterator it = begin(), stop = end(); it != stop; ++it)
    {
        it ->test();
    }
}
//-----------------------------------------------------------------------------
/** insert a change into this collection
*/
void NodeChanges::add(NodeChange const& aChange)
{
    m_aChanges.push_back(aChange);
}

//-----------------------------------------------------------------------------

/** removes a change to <var>aNode</var> from this collection (if there is one)
void NodeChanges::reset(Node const& aNode)
{
}
*/

sal_uInt32 NodeChanges::getChangesInfos(NodeChangesInformation& _rInfos) const
{
    if (isEmpty()) return 0;

    _rInfos.reserve(_rInfos.size() + this->getCount());

    sal_Int32 nResult = 0;
    for (std::vector<NodeChange>::const_iterator it = begin(); it != end(); ++it)
    {
        nResult += it->getChangeInfos(_rInfos);
    }

    return nResult;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

    }
}

