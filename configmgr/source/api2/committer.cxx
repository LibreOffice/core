/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: committer.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:05:12 $
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
#include <stdio.h>
#include "committer.hxx"

#ifndef CONFIGMGR_API_TREEIMPLOBJECTS_HXX_
#include "apitreeimplobj.hxx"
#endif
#ifndef CONFIGMGR_ROOTTREE_HXX_
#include "roottree.hxx"
#endif
#ifndef CONFIGMGR_API_PROVIDERIMPL2_HXX_
#include "confproviderimpl2.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------
        using configuration::Tree;
        using configuration::CommitHelper;
//-----------------------------------------------------------------------------
namespace
{
    //-------------------------------------------------------------------------
    struct NotifyDisabler
    {
        ApiRootTreeImpl& m_rTree;
        bool m_bOldState;

        NotifyDisabler(ApiRootTreeImpl& rTree)
        : m_rTree(rTree)
        , m_bOldState(rTree .enableNotification(false) )
        {
        }

        ~NotifyDisabler()
        {
            m_rTree.enableNotification(m_bOldState);
        }
    };
    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------
// class Committer
//-----------------------------------------------------------------------------

Committer::Committer(ApiRootTreeImpl& rTree)
: m_rTree(rTree)
{}
//-----------------------------------------------------------------------------

ITreeManager* Committer::getUpdateProvider()
{
    return &m_rTree.getApiTree().getProvider().getProviderImpl();
}

//-----------------------------------------------------------------------------
void Committer::commit()
{
    ApiTreeImpl& rApiTree = m_rTree.getApiTree();

    OSL_PRECOND(!m_rTree.getLocation().isRoot(),"INTERNAL ERROR: Empty location used.");
    OSL_PRECOND(m_rTree.getOptions().isValid(),"INTERNAL ERROR: Invalid Options used.");

    if (!m_rTree.getOptions().isValid()) return;

    RequestOptions aOptions = m_rTree.getOptions()->getRequestOptions();

    ITreeManager* pUpdateProvider = getUpdateProvider();
    OSL_ASSERT(pUpdateProvider);

    Tree aTree( rApiTree.getTree());
    if (!aTree.hasChanges()) return;

    TreeChangeList  aChangeList(aOptions,
                    aTree.getRootPath(),
                    aTree.getAttributes(aTree.getRootNode()));

    // now do the commit
    CommitHelper    aHelper(rApiTree.getTree());
    if (aHelper.prepareCommit(aChangeList))
    try
    {
        pUpdateProvider->updateTree(aChangeList);

        aHelper.finishCommit(aChangeList);

        NotifyDisabler  aDisableNotify(m_rTree);        // do not notify self
        pUpdateProvider->saveAndNotifyUpdate(aChangeList);
    }
    catch(...)
    {
        // should be a special clean-up routine, but for now we just need a consistent state
        try
        {
            aHelper.failedCommit(aChangeList);
        }
        catch(configuration::Exception&)
        {
            OSL_ENSURE(false, "Cleanup really should not throw");
        }
        throw;
    }
}
//-----------------------------------------------------------------------------
    }
}

