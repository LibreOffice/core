/*************************************************************************
 *
 *  $RCSfile: confignotifier.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:11:32 $
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

#include "confignotifier.hxx"

#include "notifierimpl.hxx"
#include "broadcaster.hxx"

#include "noderef.hxx"
#include "nodechange.hxx"

#include "apinodeaccess.hxx"
#include "apitreeaccess.hxx"
#include "apitreeimplobj.hxx"

#include <set>
#include <functional>

namespace configmgr
{
    namespace configapi
    {
// ---------------------------------------------------------------------------------------------------

        using configuration::Tree;
        using configuration::NodeID;

// ---------------------------------------------------------------------------------------------------
// class Notifier (-Impl)
// ---------------------------------------------------------------------------------------------------

Notifier::Notifier(NotifierHolder const& aImpl,ApiTreeImpl const* pTree)
: m_aImpl(aImpl)
, m_pTree(pTree)
{
    OSL_ENSURE(aImpl.isValid(),"Invalid initialization of a Notifier: No impl");
    OSL_ENSURE(pTree,"Invalid initialization of a Notifier: No tree");
}
// ---------------------------------------------------------------------------------------------------

Notifier::Notifier(Notifier const& aOther)
: m_aImpl(aOther.m_aImpl)
, m_pTree(aOther.m_pTree)
{
}
// ---------------------------------------------------------------------------------------------------

Notifier::~Notifier()
{
}
// ---------------------------------------------------------------------------------------------------

osl::Mutex&  Notifier::getMutex() const
{
    return m_aImpl->mutex();
}
// ---------------------------------------------------------------------------------------------------

Broadcaster Notifier::makeBroadcaster(NodeChange const& aChange, bool bLocal) const
{
    return Broadcaster(*this,aChange,bLocal);
}
// ---------------------------------------------------------------------------------------------------

Broadcaster Notifier::makeBroadcaster(NodeChanges const& aChanges, bool bLocal) const
{
    OSL_ENSURE(!aChanges.isEmpty(),"Creating broadcaster for no changes");
    return Broadcaster(*this,aChanges,bLocal);
}
// ---------------------------------------------------------------------------------------------------

NotifierImpl::NotifierImpl(Tree const& aTree)
: m_aMutex()
, m_aListeners(m_aMutex, aTree.getContainedNodeCount(), KeyToIndex(aTree))
{
}
// ---------------------------------------------------------------------------------------------------

NotifierImpl::~NotifierImpl()
{
}

// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForAll(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addForAll( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForOne(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addNamed( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForAll(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addForAll( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForOne(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addNamed( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener, uno::Sequence<OUString> const& aNames) const
{
    if (xListener.is())
    {
        if (aNames.getLength() > 0)
            m_aImpl->add( NodeID(m_pTree->getTree(),aNode), xListener, aNames);
        else
            m_aImpl->add( NodeID(m_pTree->getTree(),aNode), xListener );
    }
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForAll(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeForAll( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForOne(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeNamed( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForAll(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeForAll( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForOne(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeNamed( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_pTree->getTree(),aNode), xListener );
}

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(NotifierImpl& rNotifierImpl) throw()
: m_aLock(rNotifierImpl.mutex())
{
}
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(Notifier const& rNotifier) throw()
: m_aLock(rNotifier.getMutex())
{
}
// ---------------------------------------------------------------------------------------------------
DisposeGuardImpl::~DisposeGuardImpl() throw ()
{
}
// ---------------------------------------------------------------------------------------------------
GuardedNotifier::GuardedNotifier(Notifier const& rNotifier) throw()
: m_aNotifier(rNotifier)
, m_aImpl(rNotifier)
{
}
// ---------------------------------------------------------------------------------------------------
GuardedNotifier::GuardedNotifier(NodeAccess& rNode) throw()
: m_aNotifier(rNode.getNotifier())
, m_aImpl(m_aNotifier)
{
}
// ---------------------------------------------------------------------------------------------------
GuardedNotifier::GuardedNotifier(TreeElement& rTree) throw()
: m_aNotifier(rTree.getNotifier())
, m_aImpl(m_aNotifier)
{
}
// ---------------------------------------------------------------------------------------------------

DisposeGuard::DisposeGuard(NodeAccess& rNode) throw(css::lang::DisposedException)
: m_aImpl(rNode.getNotifier())
{
    rNode.checkAlive();
}
// ---------------------------------------------------------------------------------------------------

DisposeGuard::DisposeGuard(TreeElement& rTree) throw(css::lang::DisposedException)
: m_aImpl(rTree.getNotifier())
{
    rTree.checkAlive();
}
// ---------------------------------------------------------------------------------------------------
    }
}

