/*************************************************************************
 *
 *  $RCSfile: nodechange.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-16 18:03:39 $
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

bool NodeChange::isChange() const
{
    return m_pImpl && m_pImpl->isChange();
}
//-----------------------------------------------------------------------------

bool NodeChange::getChangeInfo(NodeChangeInfo& rInfo) const
{
    rInfo.type = NodeChangeInfo::eNoChange;
    if (m_pImpl)
        m_pImpl->fillChange(rInfo);
    else
        OSL_ASSERT(rInfo.isEmpty());

    return !rInfo.isEmpty();
}
//-----------------------------------------------------------------------------

bool NodeChange::getChangeInfo(ExtendedNodeChangeInfo& rInfo) const
{
    if (this->getChangeInfo( rInfo.change))
    {
        rInfo.baseTree = getBaseTree();
        rInfo.baseNode = getBaseNode();
        rInfo.accessor = m_pImpl->getPathToChangingNode();
        return true;
    }
    else
    {
        return false;
    }
}
//-----------------------------------------------------------------------------

Tree NodeChange::getBaseTree() const
{
    return Tree(m_pImpl->getBaseTree().getBodyPtr());
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
NodeRef NodeChange::getBaseNode() const
{
    TreeHolder aTree = m_pImpl->getBaseTree();
    NodeOffset nOffset = m_pImpl->getBaseNode();

    OSL_ASSERT(aTree.isValid() && aTree->isValidNode(nOffset));

    if (aTree.isValid() && nOffset)
        return TreeImplHelper::makeNode(aTree.getBody(),nOffset);

    return NodeRef();
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
Tree NodeChange::getAffectedTree() const
{
    if (isChange())
        return Tree(m_pImpl->getAffectedTree().getBodyPtr());
    else
        return Tree(0);
}
//-----------------------------------------------------------------------------

// retrieve the tree where the change is actually taking place
NodeRef NodeChange::getAffectedNode() const
{
    if (isChange())
    {
        TreeHolder aTree = m_pImpl->getAffectedTree();
        NodeOffset nOffset = m_pImpl->getAffectedNode();

        OSL_ASSERT(aTree.isValid() && aTree->isValidNode(nOffset));

        if (aTree.isValid() && nOffset)
            return TreeImplHelper::makeNode(aTree.getBody(),nOffset );
    }
    return NodeRef();
}
//-----------------------------------------------------------------------------

NodeID NodeChange::getAffectedNodeID() const
{
    TreeHolder aTree = m_pImpl->getAffectedTree();
    NodeOffset nOffset = m_pImpl->getAffectedNode();

    OSL_ASSERT(aTree.isValid() ? aTree->isValidNode(nOffset) : 0==nOffset);

    return NodeID(aTree.getBodyPtr(),nOffset);
}
//-----------------------------------------------------------------------------

NodeID NodeChange::getChangedNodeID() const
{
    TreeHolder aTree = m_pImpl->getChangingTree();
    NodeOffset nOffset = m_pImpl->getChangingNode();

    OSL_ASSERT(aTree.isValid() ? aTree->isValidNode(nOffset) : 0==nOffset);

    return NodeID(aTree.getBodyPtr(),nOffset);
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
    return !aChange.isChange();
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
    <p>if there is an existing change for this element, they are combine using <method>NodeChange::combine</method>
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

bool NodeChanges::getChangesInfo(std::vector<NodeChangeInfo>& rInfos) const
{
    if (isEmpty()) return false;

    rInfos.clear();
    rInfos.reserve(getCount());

    for (Iterator it = begin(); it != end(); ++it)
    {
        NodeChangeInfo aInfo;
        if ( it->getChangeInfo(aInfo) )
            rInfos.push_back(aInfo);
    }

    return !rInfos.empty();
}
//-----------------------------------------------------------------------------

bool NodeChanges::getChangesInfo(std::vector<ExtendedNodeChangeInfo>& rInfos) const
{
    if (isEmpty()) return false;

    rInfos.clear();
    rInfos.reserve(getCount());

    for (Iterator it = begin(); it != end(); ++it)
    {
        ExtendedNodeChangeInfo aInfo;
        if ( it->getChangeInfo(aInfo) )
            rInfos.push_back(aInfo);
    }

    return !rInfos.empty();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

    }
}

