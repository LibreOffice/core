/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinotifierimpl.cxx,v $
 * $Revision: 1.8.20.5 $
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

#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"
#include "noderef.hxx"
#include "valueref.hxx"
#include "confignotifier.hxx"
#include "configexcept.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------------
        namespace lang = css::lang;
        namespace util = css::util;
        namespace beans = css::beans;

// Generic Notifier Support Implementations
//-----------------------------------------------------------------------------------

/// add a Listener to the notifier of a NodeAccess
template <class Listener>
inline
void genericAddListener(NodeAccess& rNode, const uno::Reference< Listener >& xListener )
{
    GuardedNotifier aGuardedNotifier( rNode );  // guard the notifier
    aGuardedNotifier->add(rNode.getNodeRef(), xListener);
}

/// remove a Listener from the notifier of a NodeAccess
template <class Listener>
inline
void genericRemoveListener(NodeAccess& rNode, const uno::Reference< Listener >& xListener )
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
bool genericAddChildListener(NodeGroupInfoAccess& rNode, const uno::Reference< Listener >& xListener, const rtl::OUString& sName )
{
    if (sName.getLength() != 0)
    {
        GuardedNodeData<NodeAccess>     aGuardedNode( rNode );      // guard access to children
        GuardedNotifier             aGuardedNotifier( rNode );  // guard the notifier

        rtl::Reference< configuration::Tree > aTree( aGuardedNode.getTree() );
        configuration::NodeRef      aNode( aGuardedNode.getNode() );

        rtl::OUString        aChildName = configuration::validateChildName(sName,aTree,aNode);

        if (!aTree->hasChild(aNode,aChildName)) return false;

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
bool genericRemoveChildListener(NodeGroupInfoAccess& rNode, const uno::Reference< Listener >& xListener, const rtl::OUString& sName )
{
    if (sName.getLength() != 0)
    {
        GuardedNodeData<NodeAccess>     aGuardedNode( rNode );      // guard access to children
        GuardedNotifier             aGuardedNotifier( rNode );  // guard the notifier

        rtl::Reference< configuration::Tree > aTree( aGuardedNode.getTree() );
        configuration::NodeRef      aNode( aGuardedNode.getNode() );

        rtl::OUString aChildName = configuration::validateChildName(sName,aTree,aNode);

        if (!aTree->hasChild(aNode,aChildName)) return false;

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
void implAddListener(NodeAccess& rNode, const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    genericAddListener(rNode,xListener);
}

void implRemoveListener(NodeAccess& rNode, const uno::Reference< css::lang::XEventListener >& xListener )
    throw(uno::RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XContainer
//-----------------------------------------------------------------------------------
void implAddListener(NodeAccess& rNode, const uno::Reference< css::container::XContainerListener >& xListener )
    throw(uno::RuntimeException)
{
    genericAddListener(rNode,xListener);
}

void implRemoveListener(NodeAccess& rNode, const uno::Reference< css::container::XContainerListener >& xListener )
    throw(uno::RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XChangesNotifier
//-----------------------------------------------------------------------------------

void implAddListener(NodeAccess& rNode, const uno::Reference< css::util::XChangesListener >& xListener )
    throw(uno::RuntimeException)
{
    genericAddListener(rNode,xListener);
}

void implRemoveListener(NodeAccess& rNode, const uno::Reference< css::util::XChangesListener >& xListener )
    throw(uno::RuntimeException)
{
    genericRemoveListener(rNode,xListener);
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------

void implAddListener( NodeAccess& rNode, const uno::Reference< beans::XPropertiesChangeListener >& xListener, const uno::Sequence< rtl::OUString >& aPropertyNames )
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

void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XVetoableChangeListener >& xListener, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        if (!genericAddChildListener(rNode,xListener,sPropertyName))
        {
            throw css::beans::UnknownPropertyException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw css::beans::UnknownPropertyException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}


void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XVetoableChangeListener >& xListener, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        if (!genericRemoveChildListener(rNode,xListener,sPropertyName))
        {
            throw css::beans::UnknownPropertyException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw css::beans::UnknownPropertyException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
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

void implAddListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XPropertyChangeListener >& xListener, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        if (!genericAddChildListener(rNode,xListener,sPropertyName))
        {
            throw css::beans::UnknownPropertyException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw css::beans::UnknownPropertyException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot add listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

void implRemoveListener( NodeGroupInfoAccess& rNode, const uno::Reference< beans::XPropertyChangeListener >& xListener, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        if (!genericRemoveChildListener(rNode,xListener,sPropertyName))
        {
            throw css::beans::UnknownPropertyException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found !")),
                    rNode.getUnoInstance() );
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        throw css::beans::UnknownPropertyException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: cannot remove listener - node not found:")) += ex.message(),
                rNode.getUnoInstance() );
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


