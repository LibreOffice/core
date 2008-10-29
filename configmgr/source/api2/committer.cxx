/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: committer.cxx,v $
 * $Revision: 1.18 $
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
#include "committer.hxx"
#include "apitreeimplobj.hxx"
#include "providerimpl.hxx"
#include "roottree.hxx"
#include "treechangelist.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
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

OProviderImpl * Committer::getUpdateProvider()
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

    OProviderImpl * pUpdateProvider = getUpdateProvider();
    OSL_ASSERT(pUpdateProvider);

    rtl::Reference< configuration::Tree > aTree( rApiTree.getTree());
    if (!aTree->hasChanges()) return;

    TreeChangeList  aChangeList(aOptions,
                    aTree->getRootPath(),
                    aTree->getAttributes(aTree->getRootNode()));

    // now do the commit
    configuration::CommitHelper aHelper(rApiTree.getTree());
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

