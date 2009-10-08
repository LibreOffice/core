/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confignotifier.cxx,v $
 * $Revision: 1.12 $
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
// class Notifier (-Impl)
// ---------------------------------------------------------------------------------------------------

Notifier::Notifier(vos::ORef<NotifierImpl> const& aImpl,ApiTreeImpl const* pTree)
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

Broadcaster Notifier::makeBroadcaster(configuration::NodeChange const& aChange, bool bLocal) const
{
    return Broadcaster(*this,aChange,bLocal);
}
// ---------------------------------------------------------------------------------------------------

Broadcaster Notifier::makeBroadcaster(configuration::NodeChanges const& aChanges, bool bLocal) const
{
    OSL_ENSURE(!aChanges.isEmpty(),"Creating broadcaster for no changes");
    return Broadcaster(*this,aChanges,bLocal);
}
// ---------------------------------------------------------------------------------------------------

NotifierImpl::NotifierImpl(rtl::Reference< configuration::Tree > const& aTree)
: m_aListeners(aTree->nodeCount(), SubNodeToIndex(aTree))
{
}
// ---------------------------------------------------------------------------------------------------

NotifierImpl::~NotifierImpl()
{
}

// ---------------------------------------------------------------------------------------------------

void Notifier::add(configuration::NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(configuration::NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(configuration::NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addForAll( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener, rtl::OUString const& aName) const
{
    if (xListener.is())
        m_aImpl->addNamed( configuration::SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->addForAll( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::addForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener, rtl::OUString const& aName) const
{
    if (xListener.is())
        m_aImpl->addNamed( configuration::SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener, uno::Sequence<rtl::OUString> const& aNames) const
{
    if (xListener.is())
    {
        if (aNames.getLength() > 0)
            m_aImpl->add( configuration::NodeID(m_pTree->getTree(),aNode), xListener, aNames);
        else
            m_aImpl->add( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
    }
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(configuration::NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(configuration::NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(configuration::NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeForAll( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener, rtl::OUString const& aName) const
{
    if (xListener.is())
        m_aImpl->removeNamed( configuration::SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->removeForAll( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::removeForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener, rtl::OUString const& aName) const
{
    if (xListener.is())
        m_aImpl->removeNamed( configuration::SubNodeID(m_pTree->getTree(),aNode, aName), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( configuration::NodeID(m_pTree->getTree(),aNode), xListener );
}

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(Notifier const&) throw()
{
}
// ---------------------------------------------------------------------------------------------------
DisposeGuardImpl::~DisposeGuardImpl() throw ()
{
}
// ---------------------------------------------------------------------------------------------------
GuardedNotifier::GuardedNotifier(NodeAccess& rNode) throw()
: m_aNotifier(rNode.getNotifier())
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
    }
}

