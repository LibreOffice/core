/*************************************************************************
 *
 *  $RCSfile: committer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-13 12:14:15 $
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
#include "committer.hxx"

#include "apitreeimplobj.hxx"
#include "roottree.hxx"
#include "cmtreemodel.hxx"
#include "confproviderimpl2.hxx"

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

ITreeProvider2* Committer::getUpdateProvider()
{
    return &m_rTree.getApiTree().getProvider().getProviderImpl();
}

//-----------------------------------------------------------------------------
void Committer::commit()
{
    ApiTreeImpl& rApiTree = m_rTree.getApiTree();
    OClearableWriteSynchronized aProviderGuard(rApiTree.getProviderLock());
    OClearableWriteSynchronized aLocalGuard(rApiTree.getDataLock());

    Tree aTree(rApiTree.getTree());
    if (!aTree.hasChanges()) return;


    TreeChangeList  aChangeList(aTree.getContextPath().toString(),aTree.getRootNode().getName().toString());

    ITreeProvider2* pUpdateProvider = getUpdateProvider();
    OSL_ASSERT(pUpdateProvider);

    CommitHelper    aHelper(aTree);
    if (aHelper.prepareCommit(aChangeList))
    try
    {

        pUpdateProvider->updateTree(aChangeList);
        aHelper.finishCommit(aChangeList);

        aLocalGuard.clear();        // done locally
        aProviderGuard.downgrade(); // keep a read lock for notification

        NotifyDisabler  aDisableNotify(m_rTree);    // do not notify self
        pUpdateProvider->notifyUpdate(aChangeList);

    }
    catch(...)
    {
        // should be a special clean-up routine, but for now we just need a consistent state
        try
        {
            aHelper.finishCommit(aChangeList);
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

