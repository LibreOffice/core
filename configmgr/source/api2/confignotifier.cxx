/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confignotifier.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:05:32 $
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

#include "confignotifier.hxx"

#include "notifierimpl.hxx"
#include "broadcaster.hxx"

#include "noderef.hxx"
#include "valueref.hxx"
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
        using configuration::SubNodeID;

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

NotifierImpl::NotifierImpl(configuration::TreeRef const& aTree)
: m_aListeners(aTree.getContainedInnerNodeCount(), SubNodeToIndex(aTree))
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

void Notifier::addForOne(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener, Name const& aName) const
{
    if (xListener.is())
        m_aImpl->addNamed( SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForAll(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addForAll( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForOne(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener, Name const& aName) const
{
    if (xListener.is())
        m_aImpl->addNamed( SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
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

void Notifier::removeForOne(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener, Name const& aName) const
{
    if (xListener.is())
        m_aImpl->removeNamed( SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForAll(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeForAll( NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForOne(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener, Name const& aName) const
{
    if (xListener.is())
        m_aImpl->removeNamed( SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_pTree->getTree(),aNode), xListener );
}

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(NotifierImpl&) throw()
{
}
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(Notifier const&) throw()
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

