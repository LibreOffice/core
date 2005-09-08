/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apinotifierimpl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:08:08 $
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

#include "apinotifierimpl.hxx"

#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#include "apinodeaccess.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#include "valueref.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#include "confignotifier.hxx"
#endif
#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif

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
        using configuration::ValueRef;
        using configuration::Tree;


// Generic Notifier Support Implementations
//-----------------------------------------------------------------------------------

/// add a Listener to the notifier of a NodeAccess
template <class Listener>
inline
void genericAddListener(NodeAccess& rNode, const Reference< Listener >& xListener )
{
    GuardedNotifier aGuardedNotifier( rNode );  // guard the notifier
    aGuardedNotifier->add(rNode.getNodeRef(), xListener);
}

/// remove a Listener from the notifier of a NodeAccess
template <class Listener>
inline
void genericRemoveListener(NodeAccess& rNode, const Reference< Listener >& xListener )
{
    GuardedNotifier aGuardedNotifier( rNode );  // guard the notifier
    aGuardedNotifier->remove(rNode.getNodeRef(), xListener);
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
    using configuration::validateChildName;

    if (sName.getLength() != 0)
    {
        GuardedNodeDataAccess       aGuardedNode( rNode );      // guard access to children
        GuardedNotifier             aGuardedNotifier( rNode );  // guard the notifier

        Tree        aTree( aGuardedNode.getTree() );
        NodeRef     aNode( aGuardedNode.getNode() );

        Name        aChildName = validateChildName(sName,aTree,aNode);

        if (!aTree.hasChild(aNode,aChildName)) return false;

        aGuardedNotifier->addForOne(aNode, xListener, aChildName);
    }
    else
    {
        GuardedNotifier  aGuardedNotifier( rNode ); // guard the notifier

        aGuardedNotifier->addForAll(rNode.getNodeRef(), xListener);

        // always ok, as we addreess no specific NodeRef
    }
    return true;
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
    using configuration::validateChildName;

    if (sName.getLength() != 0)
    {
        GuardedNodeDataAccess       aGuardedNode( rNode );      // guard access to children
        GuardedNotifier             aGuardedNotifier( rNode );  // guard the notifier

        Tree        aTree( aGuardedNode.getTree() );
        NodeRef     aNode( aGuardedNode.getNode() );

        Name        aChildName = validateChildName(sName,aTree,aNode);

        if (!aTree.hasChild(aNode,aChildName)) return false;

        aGuardedNotifier->removeForOne(aNode, xListener, aChildName);
    }
    else
    {
        GuardedNotifier  aGuardedNotifier( rNode ); // guard the notifier

        aGuardedNotifier->removeForAll(rNode.getNodeRef(), xListener);

        // always ok, as we addreess no specific NodeRef
    }
    return true;
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
    impl->add(rNode.getNodeRef(), xListener, aPropertyNames);
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


