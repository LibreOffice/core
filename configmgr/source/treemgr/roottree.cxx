/*************************************************************************
 *
 *  $RCSfile: roottree.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:47 $
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
#include <stdio.h>

#include "roottree.hxx"
#include "roottreeimpl.hxx"

#ifndef CONFIGMGR_VIEWACCESS_HXX_
#include "viewaccess.hxx"
#endif

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif

#ifndef CONFIGMGR_VIEWBEHAVIORFACTORY_HXX_
#include "viewfactory.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#include "nodechangeinfo.hxx"
#endif

#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
// factory methods
//-----------------------------------------------------------------------------

RootTree createReadOnlyTree(    AbsolutePath const& aRootPath,
                                memory::Segment const* _pDataSegment,
                                data::NodeAccess const& _aCacheNode,
                                TreeDepth nDepth,
                                TemplateProvider const& aTemplateProvider)
{
    return RootTree( _aCacheNode.accessor(),
                     new RootTreeImpl(  view::createReadOnlyStrategy(_pDataSegment),
                                        aRootPath, _aCacheNode, nDepth,
                                        aTemplateProvider
                                    ));
}
//-----------------------------------------------------------------------------

RootTree createUpdatableTree(   AbsolutePath const& aRootPath,
                                memory::Segment const* _pDataSegment,
                                data::NodeAccess const& _aCacheNode,
                                TreeDepth nDepth,
                                TemplateProvider const& aTemplateProvider)
{
    return RootTree( _aCacheNode.accessor(),
                     new RootTreeImpl(  view::createDeferredChangeStrategy(_pDataSegment),
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
bool CommitHelper::prepareCommit(data::Accessor const& _aAccessor, TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"ERROR: CommitHelper: Cannot commit without a tree");
    if (m_pTree == NULL)
        return false;

    OSL_ENSURE(m_pData.get() == NULL,"ERROR: CommitHelper: Need to reset before reusing");
    m_pData.reset( new Data() );

    // get and check the changes
    std::auto_ptr<SubtreeChange> pTreeChange(view::ViewTreeAccess(_aAccessor,*m_pTree).preCommitChanges(m_pData->m_aRemovedElements));
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

void CommitHelper::finishCommit(data::Accessor const& _aAccessor, TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"INTERNAL ERROR: Nothing to finish without a tree");

    // find the name and path of the change
    AbsolutePath aPath = m_pTree->getRootPath();

    OSL_ENSURE( rChangeList.getRootNodePath().toString() == aPath.toString(), "ERROR: FinishCommit cannot handle rebased changes trees");
    if ( !matches(rChangeList.getRootNodePath(), aPath) )
        throw configuration::Exception("INTERNAL ERROR: FinishCommit cannot handle rebased changes trees");

    view::ViewTreeAccess(_aAccessor,*m_pTree).finishCommit(rChangeList.root);
}
//-----------------------------------------------------------------------------

void CommitHelper::revertCommit(data::Accessor const& _aAccessor, TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"INTERNAL ERROR: Nothing to finish without a tree");

    AbsolutePath aPath = m_pTree->getRootPath();

    OSL_ENSURE( rChangeList.getRootNodePath().toString() == aPath.toString(), "ERROR: FinishCommit cannot handle rebased changes trees");
    if ( !matches(rChangeList.getRootNodePath(), aPath) )
        throw configuration::Exception("INTERNAL ERROR: FinishCommit cannot handle rebased changes trees");

    view::ViewTreeAccess(_aAccessor,*m_pTree).revertCommit(rChangeList.root);
}
//-----------------------------------------------------------------------------

void CommitHelper::failedCommit(data::Accessor const& _aAccessor, TreeChangeList& rChangeList)
{
    OSL_ENSURE(m_pTree,"INTERNAL ERROR: Nothing to finish without a tree");

    AbsolutePath aPath = m_pTree->getRootPath();

    OSL_ENSURE( rChangeList.getRootNodePath().toString() == aPath.toString(), "ERROR: FinishCommit cannot handle rebased changes trees");
    if ( !matches(rChangeList.getRootNodePath(), aPath) )
        throw configuration::Exception("INTERNAL ERROR: FinishCommit cannot handle rebased changes trees");

    view::ViewTreeAccess(_aAccessor,*m_pTree).recoverFailedCommit(rChangeList.root);
}
//-----------------------------------------------------------------------------

    }
}

