/*************************************************************************
 *
 *  $RCSfile: apinotifierimpl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"

#include "noderef.hxx"
#include "confignotifier.hxx"
#include "configexcept.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------------
        namespace lang = css::lang;
        namespace util = css::util;
        namespace beans = css::beans;

        using uno::RuntimeException;
        using uno::Reference;

        using configuration::NodeRef;
        using configuration::Tree;

//-----------------------------------------------------------------------------------
// Helpers for associating listeners with specific child nodes listener
//-----------------------------------------------------------------------------------

namespace internal
{
    using configuration::Name;
    using configuration::NodeVisitor;

    /// helper for adding a Listener to all children of a node
    template <class Listener>
    class AddListener :  public NodeVisitor
    {
        Reference<Listener> m_xListener;
        Notifier    m_aNotifier;
    public:
        AddListener(Notifier const& aNotifier, Reference<Listener> const& xListener)
            : m_xListener(xListener)
            , m_aNotifier(aNotifier)
        {}

        virtual Result handle(NodeRef const& aNode); // NodeVisitor
    };

    /// helper for adding a Listener to a named child of a node
    template <class Listener>
    class AddListenerByName :  public NodeVisitor
    {
        Reference<Listener> m_xListener;
        Notifier    m_aNotifier;
        Name        m_aName;
    public:
        AddListenerByName(Notifier const& aNotifier, Reference<Listener> const& xListener, Name const& aName)
            : m_xListener(xListener)
            , m_aNotifier(aNotifier)
            , m_aName(aName)
        {}

        virtual Result handle(NodeRef const& aNode); // NodeVisitor
    };

    /// helper for removing a Listener from all children of a node
    template <class Listener>
    class RemoveListener :  public NodeVisitor
    {
        Reference<Listener> m_xListener;
        Notifier    m_aNotifier;
    public:
        RemoveListener(Notifier const& aNotifier, Reference<Listener> const& xListener)
            : m_xListener(xListener)
            , m_aNotifier(aNotifier)
        {}

        virtual Result handle(NodeRef const& aNode); // NodeVisitor
    };

    /// helper for removing a Listener from all children of a node
    template <class Listener>
    class RemoveListenerByName :  public NodeVisitor
    {
        Reference<Listener> m_xListener;
        Notifier    m_aNotifier;
        Name        m_aName;
    public:
        RemoveListenerByName(Notifier const& aNotifier, Reference<Listener> const& xListener, Name const& aName)
            : m_xListener(xListener)
            , m_aNotifier(aNotifier)
            , m_aName(aName)
        {}

        virtual Result handle(NodeRef const& aNode); // NodeVisitor
    };

    /// add a Listener to a node
    template <class Listener>
    NodeVisitor::Result AddListener<Listener>::handle(NodeRef const& aNode)
    {
        m_aNotifier.add(aNode, m_xListener);
        return CONTINUE;
    }
    /// add a Listener to a named node
    template <class Listener>
    NodeVisitor::Result AddListenerByName<Listener>::handle(NodeRef const& aNode)
    {
        if (aNode.getName() == m_aName)
        {
            m_aNotifier.add(aNode, m_xListener);
            return DONE;
        }
        else
            return CONTINUE;
    }
    /// remove a Listener from a node
    template <class Listener>
    NodeVisitor::Result RemoveListener<Listener>::handle(NodeRef const& aNode)
    {
        m_aNotifier.remove(aNode, m_xListener);
        return CONTINUE;
    }
    /// remove a Listener from a named node
    template <class Listener>
    NodeVisitor::Result RemoveListenerByName<Listener>::handle(NodeRef const& aNode)
    {
        if (aNode.getName() == m_aName)
        {
            m_aNotifier.remove(aNode, m_xListener);
            return DONE;
        }
        else
            return CONTINUE;
    }
}

// Generic Notifier Support Implementations
//-----------------------------------------------------------------------------------

/// add a Listener to the notifier of a NodeAccess
template <class Listener>
inline
void genericAddListener(NodeAccess& rNode, const Reference< Listener >& xListener )
{
    GuardedNotifier aGuardedNotifier( rNode );  // guard the notifier
    aGuardedNotifier->add(rNode.getNode(), xListener);
}

/// remove a Listener from the notifier of a NodeAccess
template <class Listener>
inline
void genericRemoveListener(NodeAccess& rNode, const Reference< Listener >& xListener )
{
    GuardedNotifier aGuardedNotifier( rNode );  // guard the notifier
    aGuardedNotifier->remove(rNode.getNode(), xListener);
}


/** add a Listener for all or some children of a NodeAccess to its notifier
    <p> If the name given is empty, the listener will be added to all children of the node</p>
    @returns
        <TRUE/>  if the node was found, or the name was empty
        <FALSE/> if the named node wasn't found
*/
template <class Listener>
inline
bool genericAddChildListener(NodeGroupInfoAccess& rNode, const Reference< Listener >& xListener, const OUString& sName )
{
    using configuration::NodeVisitor;
    using configuration::validateNodeName;

    GuardedNodeAccess           aGuardedNode( rNode );      // guard access to children
    GuardedNotifier             aGuardedNotifier( rNode );  // guard the notifier

    Tree        aTree( aGuardedNode->getTree() );
    NodeRef     aNode( aGuardedNode->getNode() );

    if (sName.getLength() != 0)
    {
        internal::AddListenerByName<Listener> aAdder( *aGuardedNotifier, xListener, validateNodeName(sName,aTree,aNode));
        NodeVisitor::Result eFound = aTree.dispatchToChildren( aNode, aAdder );

        return (eFound == NodeVisitor::DONE); // ok if NodeRef was found
    }
    else
    {
        internal::AddListener<Listener> aAdder( *aGuardedNotifier,xListener);
        aTree.dispatchToChildren( aNode, aAdder );

        return true; // always ok, as we addreess no specific NodeRef
    }
}

/** remove a Listener from all or some children of a NodeAccess to its notifier
    <p> If the name given is empty, the listener will be removed from any children of the node</p>
    @returns
        <TRUE/>  if the node was found, or the name was empty
        <FALSE/> if the named node wasn't found
*/
template <class Listener>
inline
bool genericRemoveChildListener(NodeGroupInfoAccess& rNode, const Reference< Listener >& xListener, const OUString& sName )
{
    using configuration::NodeVisitor;
    using configuration::validateNodeName;

    GuardedNodeAccess           aGuardedNode( rNode );      // guard access to children
    GuardedNotifier             aGuardedNotifier( rNode );  // guard the notifier

    Tree        aTree( aGuardedNode->getTree() );
    NodeRef     aNode( aGuardedNode->getNode() );

    if (sName.getLength() != 0)
    {
        internal::RemoveListenerByName<Listener> aRemover( *aGuardedNotifier, xListener, validateNodeName(sName,aTree,aNode) );
        NodeVisitor::Result eFound = aTree.dispatchToChildren( aNode, aRemover );

        return (eFound == NodeVisitor::DONE); // ok if NodeRef was found
    }
    else
    {
        internal::RemoveListener<Listener> aRemover( *aGuardedNotifier, xListener) ;
        aTree.dispatchToChildren( aNode, aRemover );

        return true; // always ok, as we addreess no specific NodeRef
    }
}



// XComponent
//-----------------------------------------------------------------------------------
void implAddListener(NodeAccess& rNode, const Reference< css::lang::XEventListener >& xListener )
    throw(RuntimeException)
{
    genericAddListener(rNode,xListener);
}

void implRemoveListener(NodeAccess& rNode, const Reference< css::lang::XEventListener >& xListener )
    throw(RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XContainer
//-----------------------------------------------------------------------------------
void implAddListener(NodeAccess& rNode, const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    genericAddListener(rNode,xListener);
}

void implRemoveListener(NodeAccess& rNode, const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XChangesNotifier
//-----------------------------------------------------------------------------------

void implAddListener(NodeAccess& rNode, const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    genericAddListener(rNode,xListener);
}

void implRemoveListener(NodeAccess& rNode, const Reference< css::util::XChangesListener >& xListener )
    throw(RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------

void implAddListener( NodeAccess& rNode, const uno::Reference< beans::XPropertiesChangeListener >& xListener, const uno::Sequence< OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{

    GuardedNotifier impl( rNode );

    // TODO: is an exception for unknown names allowed/needed ?
    impl->add(rNode.getNode(), xListener, aPropertyNames);
}


void implRemoveListener( NodeAccess& rNode, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XPropertySet (manages listeners associated with named child node)
//-----------------------------------------------------------------------------------

// XPropertySet - VetoableChangeListeners
//-----------------------------------------------------------------------------------

void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XVetoableChangeListener >& xListener, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    using namespace css::beans;
    try
    {
        if (!genericAddChildListener(rNode,xListener,sPropertyName))
        {
            throw UnknownPropertyException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        throw css::lang::WrappedTargetException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: adding a listener failed: ")) += ex.message(),
                rNode.getUnoInstance(),
                ex.getAnyUnoException());
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}


void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XVetoableChangeListener >& xListener, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    using namespace css::beans;
    try
    {
        if (!genericRemoveChildListener(rNode,xListener,sPropertyName))
        {
            throw UnknownPropertyException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        throw css::lang::WrappedTargetException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: removing a listener failed: ")) += ex.message(),
                rNode.getUnoInstance(),
                ex.getAnyUnoException());
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XPropertySet - PropertyChangeListeners
//-----------------------------------------------------------------------------------

void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XPropertyChangeListener >& xListener, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    using namespace css::beans;
    try
    {
        if (!genericAddChildListener(rNode,xListener,sPropertyName))
        {
            throw UnknownPropertyException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        throw css::lang::WrappedTargetException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: adding a listener failed: ")) += ex.message(),
                rNode.getUnoInstance(),
                ex.getAnyUnoException());
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XPropertyChangeListener >& xListener, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    using namespace css::beans;
    try
    {
        if (!genericRemoveChildListener(rNode,xListener,sPropertyName))
        {
            throw UnknownPropertyException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw UnknownPropertyException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        throw css::lang::WrappedTargetException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: removing a listener failed: ")) += ex.message(),
                rNode.getUnoInstance(),
                ex.getAnyUnoException());
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


