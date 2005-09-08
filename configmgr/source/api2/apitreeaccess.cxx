/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apitreeaccess.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:09:07 $
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

configuration::Tree TreeElement::getTree(data::Accessor const& _aAccessor) const
{
    return configuration::Tree(_aAccessor, this->getTreeRef());
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

osl::Mutex& TreeElement::getDataLock() const
{
    return getApiTree().getDataLock();
}
//-----------------------------------------------------------------------------

osl::Mutex& TreeElement::getApiLock()
{
    return getApiTree().getApiLock();
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

memory::Segment const* RootElement::getSourceData()
{
    return getApiTree().getSourceData();
}
//-----------------------------------------------------------------------------

Committer UpdateRootElement::getCommitter()
{
    return Committer(getRootTree());
}
//-----------------------------------------------------------------------------

TreeReadGuardImpl::TreeReadGuardImpl(TreeElement& rTree)
: m_aViewLock(rTree.getDataLock())
, m_rTree(rTree)
{
    rTree.checkAlive();
}
//-----------------------------------------------------------------------------

TreeReadGuardImpl::~TreeReadGuardImpl() throw ()
{
}
//-----------------------------------------------------------------------------

GuardedRootElement::GuardedRootElement(RootElement& rTree)
: m_aDataAccess(rTree.getSourceData())
, m_aImpl(rTree)
{
}
//-----------------------------------------------------------------------------

configuration::Tree GuardedRootElement::getTree() const
{
    return this->get().getTree(m_aDataAccess);
}

//-----------------------------------------------------------------------------
    }
}

