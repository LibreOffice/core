/*************************************************************************
 *
 *  $RCSfile: confignotifier.cxx,v $
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

#include "confignotifier.hxx"

#include "notifierimpl.hxx"
#include "noderef.hxx"
#include "nodechange.hxx"
#include "apinodeaccess.hxx"
#include "apitreeaccess.hxx"

namespace configmgr
{
    namespace configapi
    {
// ---------------------------------------------------------------------------------------------------

        using configuration::Tree;
        using configuration::NodeID;

// ---------------------------------------------------------------------------------------------------

Broadcaster::Broadcaster(NotifierHolder const& rImpl,Tree const& aBaseTree)
: m_aTargets(rImpl)
, m_aBaseTree(aBaseTree)
{
}
// ---------------------------------------------------------------------------------------------------

Broadcaster::Broadcaster(Broadcaster const& aOther)
: m_aTargets(aOther.m_aTargets)
, m_aBaseTree(aOther.m_aBaseTree)
{
}
// ---------------------------------------------------------------------------------------------------

Broadcaster::~Broadcaster()
{
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::queryConstraints(NodeChange const& aChange) throw(com::sun::star::beans::PropertyVetoException)
{
    OSL_ENSURE(aChange.isChange(),"Constraint query without a change !");
//  Tree aTree = m_;
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::queryConstraints(NodeChanges const& aChanges) throw(com::sun::star::beans::PropertyVetoException)
{
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(NodeChange const& aChange) throw()
{
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(NodeChanges const& aChanges) throw()
{
}
// ---------------------------------------------------------------------------------------------------

Notifier::Notifier(NotifierHolder const& aImpl,Tree const& aTree)
: m_aImpl(aImpl)
, m_aTree(aTree)
{
}
// ---------------------------------------------------------------------------------------------------

Notifier::Notifier(Notifier const& aOther)
: m_aImpl(aOther.m_aImpl)
, m_aTree(aOther.m_aTree)
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
    if (bLocal)
    {
        OSL_ENSURE(!m_aTree.isEmpty(),"Unexpected NULL Tree");
        OSL_ENSURE(m_aTree.isValidNode(aChange.getAffectedNode()),"Node does not match tree");
        return Broadcaster(m_aImpl,m_aTree);
    }
    else
    {
        Tree aTree = aChange.getAffectedTree();
        OSL_ENSURE(!aTree.isEmpty(),"Unexpected NULL Tree");
        OSL_ENSURE(m_aTree.isValidNode(aChange.getAffectedNode()),"Node does not match nested tree");
        return Broadcaster(m_aImpl,aTree);
    }
}
// ---------------------------------------------------------------------------------------------------

Broadcaster Notifier::makeBroadcaster(NodeChanges const& aChanges, bool bLocal) const
{
    OSL_ENSURE(!aChanges.isEmpty(),"Creating broadcaster for no changes");
    if (bLocal)
    {
        OSL_ENSURE(!m_aTree.isEmpty(),"Unexpected NULL Tree");
        return Broadcaster(m_aImpl,m_aTree);
    }
    else
    {
        return Broadcaster(m_aImpl, Tree(0));
    }
}
// ---------------------------------------------------------------------------------------------------

NotifierImpl::NotifierImpl()
: m_aMutex()
, m_aListeners(m_aMutex)
{
}
// ---------------------------------------------------------------------------------------------------

NotifierImpl::~NotifierImpl()
{
}

// ---------------------------------------------------------------------------------------------------
/*void NotifierImpl::disposeNode(Key const& aNode, css::lang::EventObject const& aEvt)
{
    osl::ClearableMutexGuard aGuard(m_aListeners.mutex());

    if (m_aListeners.isAlive()) // has not started disposing
    {
        if (ListenerContainer* pNodeListeners = m_aListeners.getContainer(aNode))
        {
            // make a copy and clear the container
            uno::Sequence<uno::Reference<uno::XInterface> > aElements( pNodeListeners->getElements() );
            pNodeListeners->clear();

            // now release the guard
            aGuard.clear();
            for (sal_Int32 i = 0; i<aElements.getLength(); ++i)
            {
                uno::Reference<css::lang::XEventListener> xListener(aElements[i],uno::UNO_QUERY);
                if (xListener.is())
                    xListener->disposing(aEvt);
            }
        }
    }
}
*/
// ---------------------------------------------------------------------------------------------------
bool NotifierImpl::disposeNodeHelper(Key const& aNode, css::lang::EventObject const& aEvt) const
{
    using configuration::NodeIDList;
    using configuration::getAllChildrenHelper;
    using com::sun::star::lang::XEventListener;
    using com::sun::star::beans::XPropertyChangeListener;
    using com::sun::star::beans::XVetoableChangeListener;

    osl::ClearableMutexGuard aGuard( mutex() );

    if (m_aListeners.isAlive())
    {
        // collect a list of all relevant listeners
        typedef std::vector< uno::Reference< XEventListener> > EventListeners;
        EventListeners aNotifyListeners;

        // collect the child listeners for properties
        NodeIDList aChildNodes;
        getAllChildrenHelper(aNode, aChildNodes);

        for (NodeIDList::const_iterator it = aChildNodes.begin(); it !=aChildNodes.end(); ++it)
        {
            if (ListenerContainer* pSubContainer = m_aListeners.getContainer( *it ) )
            {
                ListenerContainerIterator<XPropertyChangeListener> itProps(*pSubContainer);
                while (itProps.hasMoreElements())
                {
                    uno::Reference< XPropertyChangeListener > xListener = itProps.next();
                    aNotifyListeners.push_back( xListener.get() );
                    pSubContainer->removeInterface( xListener );
                }

                ListenerContainerIterator<XVetoableChangeListener> itVetos(*pSubContainer);
                while (itVetos.hasMoreElements())
                {
                    uno::Reference< XVetoableChangeListener > xListener = itVetos.next();
                    aNotifyListeners.push_back( xListener.get() );
                    pSubContainer->removeInterface( xListener );
                }
            }
        }
        // collect the direct listeners
        if (ListenerContainer* pThisContainer = m_aListeners.getContainer( aNode ) )
        {
            ListenerContainerIterator<XEventListener> itAllEvents(*pThisContainer);
            while (itAllEvents.hasMoreElements())
            {
                uno::Reference< XEventListener > xListener = itAllEvents.next();

                // skip propertylisteners
                if (uno::Reference< XPropertyChangeListener >::query(xListener).is()) continue;
                if (uno::Reference< XVetoableChangeListener >::query(xListener).is()) continue;

                aNotifyListeners.push_back( xListener );
                pThisContainer->removeInterface( xListener );
            }
        }

        // now do the dispose notifications
        aGuard.clear();

        for(EventListeners::iterator itNotify = aNotifyListeners.begin(); itNotify != aNotifyListeners.end(); ++it)
        {
            OSL_ASSERT(itNotify->is());
            (*itNotify)->disposing(aEvt);
        }
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->add( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::add(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener, uno::Sequence<OUString> const& aNames) const
{
    if (xListener.is())
    {
        if (aNames.getLength() > 0)
            m_aImpl->add( NodeID(m_aTree,aNode), xListener, aNames);
        else
            m_aImpl->add( NodeID(m_aTree,aNode), xListener );
    }
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_aTree,aNode), xListener );
}
// ---------------------------------------------------------------------------------------------------

void Notifier::remove(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const
{
    if (xListener.is())
        m_aImpl->remove( NodeID(m_aTree,aNode), xListener );
}

// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(NotifierImpl& rNotifierImpl) throw()
: m_aLock(rNotifierImpl.mutex())
, m_rNotifierImpl(rNotifierImpl)
{
}
// ---------------------------------------------------------------------------------------------------

DisposeGuardImpl::DisposeGuardImpl(Notifier const& rNotifier) throw()
: m_aLock(rNotifier.getMutex())
, m_rNotifierImpl(rNotifier.getImpl())
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

