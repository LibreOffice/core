/*************************************************************************
 *
 *  $RCSfile: apitreeaccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 12:22:55 $
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

#include "apitreeaccess.hxx"

#include "apitreeimplobj.hxx"
#include "configset.hxx"
#include "confignotifier.hxx"
#include "committer.hxx"
#include "apinodeaccess.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------

        // self-locked methods for dispose handling
void NodeElement::checkAlive() const
{
    getApiTree().checkAlive();
}
//-----------------------------------------------------------------------------

configuration::Tree TreeElement::getTree() const
{
    return getApiTree().getTree();
}
//-----------------------------------------------------------------------------
        // self-locked methods for dispose handling
bool TreeElement::disposeTree(bool bForce)
{
    return getApiTree().disposeTree(bForce);
}
//-----------------------------------------------------------------------------
Factory& TreeElement::getFactory()
{
    return getApiTree().getFactory();
}
//-----------------------------------------------------------------------------

Notifier TreeElement::getNotifier()
{
    return getApiTree().getNotifier();
}
//-----------------------------------------------------------------------------

ISynchronizedData const* TreeElement::getDataLock() const
{
    return getApiTree().getDataLock();
}
//-----------------------------------------------------------------------------

osl::Mutex& TreeElement::getApiLock()
{
    return getApiTree().getApiLock();
}
//-----------------------------------------------------------------------------

configuration::ElementTree SetElement::getElementTree() const
{
    return configuration::ElementTree::extract(getTree());
}
//-----------------------------------------------------------------------------

configuration::SetElementInfo SetElement::getTemplateInfo() const
{
    configuration::ElementTree aTree = configuration::ElementTree::extract(getTree());
    OSL_ENSURE(aTree.isValid(), "This really must be a set element");
    return configuration::SetElementInfo(aTree.getTemplate());
}
//-----------------------------------------------------------------------------

void SetElement::haveNewParent(NodeSetInfoAccess* pNewParent)
{
    ApiTreeImpl* pNewParentImpl = pNewParent ? &pNewParent->getApiTree() : 0;

    this->getApiTree().haveNewParent( pNewParentImpl );
}
//-----------------------------------------------------------------------------
// configuration::RootTree  RootElement::getRootTree() const;

//-----------------------------------------------------------------------------

//void UpdateRootElement::commit() const;
//-----------------------------------------------------------------------------

ISynchronizedData * UpdateRootElement::getDataLock()
{
    return getApiTree().getDataLock();
}
//-----------------------------------------------------------------------------

ISynchronizedData * UpdateRootElement::getProviderLock()
{
    return getApiTree().getProviderLock();
}
//-----------------------------------------------------------------------------

Committer UpdateRootElement::getCommitter()
{
    return Committer(getApiTree());
}
//-----------------------------------------------------------------------------

TreeReadGuardImpl::TreeReadGuardImpl(TreeElement& rTree) throw()
: m_aLock(rTree.getDataLock())
, m_rTree(rTree)
{
    rTree.checkAlive();
}
//-----------------------------------------------------------------------------

TreeReadGuardImpl::~TreeReadGuardImpl() throw ()
{
}
//-----------------------------------------------------------------------------

    }
}

