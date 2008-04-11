/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: roottree.cxx,v $
 * $Revision: 1.20 $
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

#include "roottree.hxx"
#include "roottreeimpl.hxx"
#include "viewaccess.hxx"
#include "nodeaccess.hxx"
#include "viewfactory.hxx"
#include "noderef.hxx"
#include "nodechangeinfo.hxx"
#include "treechangelist.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// factory methods
//-----------------------------------------------------------------------------

RootTree createReadOnlyTree(    AbsolutePath const& aRootPath,
                                data::NodeAccess const& _aCacheNode,
                                TreeDepth nDepth,
                TemplateProvider const& aTemplateProvider)
{
    return RootTree( new RootTreeImpl(view::createReadOnlyStrategy(),
                      aRootPath, _aCacheNode, nDepth,
                      aTemplateProvider
                      ));
}
//-----------------------------------------------------------------------------

RootTree createUpdatableTree(   AbsolutePath const& aRootPath,
                                data::NodeAccess const& _aCacheNode,
                                TreeDepth nDepth,
                                TemplateProvider const& aTemplateProvider)
{
    return RootTree(new RootTreeImpl(view::createDeferredChangeStrategy(),
                      aRootPath, _aCacheNode, nDepth,
                      aTemplateProvider
                      ));
}

//-----------------------------------------------------------------------------
// update on notify method
//-----------------------------------------------------------------------------
bool adjustToChanges(   NodeChangesInformation& rLocalChanges,
                        Tree const& aBaseTree, NodeRef const& aBaseNode,
                        SubtreeChange const& aExternalChange)
{
    OSL_PRECOND( !aBaseTree.isEmpty(), "ERROR: Configuration: Tree operation requires a valid Tree");
    OSL_PRECOND(  aBaseTree.isValidNode(aBaseNode), "ERROR: Configuration: NodeRef does not match Tree");

    if (!aBaseTree.isEmpty())
    {
        OSL_ENSURE(rLocalChanges.empty(), "Should pass empty container to adjustToChanges(...)");

        aBaseTree.getView().adjustToChanges(rLocalChanges, aBaseNode, aExternalChange);

        return !rLocalChanges.empty();
    }
    else
        return false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class CommitHelper
//-----------------------------------------------------------------------------
struct CommitHelper::Data
{
    ElementList m_aRemovedElements; // filled to keep the elements alive 'till after notification
};

//-----------------------------------------------------------------------------
CommitHelper::CommitHelper(TreeRef const& aTree)
: m_pData(  )
, m_pTree( TreeImplHelper::impl(aTree) )
{
    OSL_ENSURE(m_pTree, "INTERNAL ERROR: Unexpected NULL tree in commit helper");
}
//-----------------------------------------------------------------------------
CommitHelper::~CommitHelper()
{
}

//-----------------------------------------------------------------------------
void CommitHelper::reset()
{
    m_pData.reset();
}

//-----------------------------------------------------------------------------
bool CommitHelper::prepareCommit(TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"ERROR: CommitHelper: Cannot commit without a tree");
    if (m_pTree == NULL)
        return false;

    OSL_ENSURE(m_pData.get() == NULL,"ERROR: CommitHelper: Need to reset before reusing");
    m_pData.reset( new Data() );

    // get and check the changes
    std::auto_ptr<SubtreeChange> pTreeChange(view::ViewTreeAccess(*m_pTree).preCommitChanges(m_pData->m_aRemovedElements));
    if (pTreeChange.get() == NULL)
        return false;

    // find the name and path of the change
    OSL_ENSURE(m_pTree->getSimpleRootName().toString() == pTreeChange->getNodeName(), "ERROR in Commit: Change Name Mismatch");

    // now fill the TreeChangeList
    rChangeList.setRootPath( m_pTree->getRootPath() );
    rChangeList.root.swap( *pTreeChange );

    return true;
}
//-----------------------------------------------------------------------------

void CommitHelper::finishCommit(TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"INTERNAL ERROR: Nothing to finish without a tree");

    // find the name and path of the change
    AbsolutePath aPath = m_pTree->getRootPath();

    OSL_ENSURE( rChangeList.getRootNodePath().toString() == aPath.toString(), "ERROR: FinishCommit cannot handle rebased changes trees");
    if ( !matches(rChangeList.getRootNodePath(), aPath) )
        throw configuration::Exception("INTERNAL ERROR: FinishCommit cannot handle rebased changes trees");

    view::ViewTreeAccess(*m_pTree).finishCommit(rChangeList.root);
}
//-----------------------------------------------------------------------------

void CommitHelper::revertCommit(TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"INTERNAL ERROR: Nothing to finish without a tree");

    AbsolutePath aPath = m_pTree->getRootPath();

    OSL_ENSURE( rChangeList.getRootNodePath().toString() == aPath.toString(), "ERROR: FinishCommit cannot handle rebased changes trees");
    if ( !matches(rChangeList.getRootNodePath(), aPath) )
        throw configuration::Exception("INTERNAL ERROR: FinishCommit cannot handle rebased changes trees");

    view::ViewTreeAccess(*m_pTree).revertCommit(rChangeList.root);
}
//-----------------------------------------------------------------------------

void CommitHelper::failedCommit(TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"INTERNAL ERROR: Nothing to finish without a tree");

    AbsolutePath aPath = m_pTree->getRootPath();

    OSL_ENSURE( rChangeList.getRootNodePath().toString() == aPath.toString(), "ERROR: FinishCommit cannot handle rebased changes trees");
    if ( !matches(rChangeList.getRootNodePath(), aPath) )
        throw configuration::Exception("INTERNAL ERROR: FinishCommit cannot handle rebased changes trees");

    view::ViewTreeAccess(*m_pTree).recoverFailedCommit(rChangeList.root);
}
//-----------------------------------------------------------------------------

    }
}

