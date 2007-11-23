/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodechange.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:42:43 $
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

#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodechangeinfo.hxx"

#include "noderef.hxx"
#include "treeimpl.hxx"

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
        NodeChangeImpl::ChangeCount nChanges = m_pImpl->getChangeDataCount();

        for (NodeChangeImpl::ChangeCount ix = 0; ix < nChanges; ++ix)
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

Tree NodeChange::getBaseTree() const
{
    return Tree(m_pImpl->getTargetTree().get());
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
NodeRef NodeChange::getBaseNode() const
{
    TreeHolder aTree = m_pImpl->getTargetTree();
    NodeOffset nOffset = m_pImpl->getTargetNode();

    OSL_ASSERT(aTree.is() && aTree->isValidNode(nOffset));

    if (aTree.is() && nOffset)
        return TreeImplHelper::makeNode(*aTree,nOffset);

    return NodeRef();
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
Tree NodeChange::getAffectedTree() const
{
    if (this->maybeChange())
        return Tree(m_pImpl->getTargetTree().get());
    else
        return Tree(NULL);
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
NodeRef NodeChange::getAffectedNode() const
{
    if (this->maybeChange())
    {
        TreeHolder aTree = m_pImpl->getTargetTree();
        NodeOffset nOffset = m_pImpl->getTargetNode();

        OSL_ASSERT(aTree.is() && aTree->isValidNode(nOffset));

        if (aTree.is() && nOffset)
            return TreeImplHelper::makeNode(*aTree,nOffset );
    }
    return NodeRef();
}
//-----------------------------------------------------------------------------

NodeID NodeChange::getAffectedNodeID() const
{
    TreeHolder aTree = m_pImpl->getTargetTree();
    NodeOffset nOffset = m_pImpl->getTargetNode();

    OSL_ASSERT(aTree.is() ? aTree->isValidNode(nOffset) : 0==nOffset);

    return NodeID(aTree.get(),nOffset);
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
    for(Iterator it = begin(), stop = end(); it != stop; ++it)
    {
        it ->test();
    }
}
//-----------------------------------------------------------------------------
void NodeChanges::implApply() const
{
    for(Iterator it = begin(), stop = end(); it != stop; ++it)
    {
        it ->apply();
    }
}
//-----------------------------------------------------------------------------
/// gets the nearest common ancestor node of all changes this tree
//Node getBaseNode() const;

/// gets the tree the base node belongs to, if available
//Tree getBaseTree() const;

/** insert a change into this collection
*/
void NodeChanges::add(NodeChange const& aChange)
{
    m_aChanges.push_back(aChange);
}
//-----------------------------------------------------------------------------

/** insert multiple changes into this collection
*/
void NodeChanges::add(NodeChanges const& aChanges)
{
    m_aChanges.insert(m_aChanges.end(),aChanges.begin(),aChanges.end());
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
    for (Iterator it = begin(); it != end(); ++it)
    {
        nResult += it->getChangeInfos(_rInfos);
    }

    return nResult;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

    }
}

