/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apitreeaccess.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:03:54 $
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
    return configuration::Tree(this->getTreeRef());
}
//-----------------------------------------------------------------------------

configuration::TreeRef  TreeElement::getTreeRef() const
{
    return getApiTree().getTree();
}
//-----------------------------------------------------------------------------
        // self-locked methods for dispose handling
bool SetElement::disposeTree(bool bForce)
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

configuration::ElementRef SetElement::getElementRef() const
{
    return configuration::ElementRef::extract(getTreeRef());
}
//-----------------------------------------------------------------------------

configuration::TemplateInfo SetElement::getTemplateInfo() const
{
    configuration::ElementRef aTree = configuration::ElementRef::extract(getTreeRef());
    OSL_ENSURE(aTree.isValid(), "This really must be a set element");
    return configuration::TemplateInfo(aTree.getTemplate());
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

bool RootElement::disposeTree()
{
    return getRootTree().disposeTree();
}
//-----------------------------------------------------------------------------

Committer UpdateRootElement::getCommitter()
{
    return Committer(getRootTree());
}
//-----------------------------------------------------------------------------

TreeReadGuardImpl::TreeReadGuardImpl(TreeElement& rTree)
: m_rTree(rTree)
{
    rTree.checkAlive();
}
//-----------------------------------------------------------------------------

TreeReadGuardImpl::~TreeReadGuardImpl() throw ()
{
}
//-----------------------------------------------------------------------------

GuardedRootElement::GuardedRootElement(RootElement& rTree)
: m_aImpl(rTree)
{
}
//-----------------------------------------------------------------------------

configuration::Tree GuardedRootElement::getTree() const
{
    return this->get().getTree();
}

//-----------------------------------------------------------------------------
    }
}

