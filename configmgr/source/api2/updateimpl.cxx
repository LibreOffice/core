/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updateimpl.cxx,v $
 * $Revision: 1.15.20.8 $
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

#include "updateimpl.hxx"
#include "accessimpl.hxx"
#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"
#include "noderef.hxx"
#include "valueref.hxx"
#include "nodechange.hxx"
#include "configset.hxx"
#include "configgroup.hxx"
#include "confignotifier.hxx"
#include "broadcaster.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------------
        namespace lang = css::lang;
        namespace util = css::util;
        namespace container = css::container;

// Interface methods
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Update methods
//-----------------------------------------------------------------------------------

// XNameReplace
//-----------------------------------------------------------------------------------
void implReplaceByName(NodeGroupAccess& rNode, const rtl::OUString& sName, const uno::Any& rElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeGroupAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildName(sName,aTree,aNode);

        configuration::ValueRef aChildValue( aTree->getChildValue(aNode, aChildName) );

        if (!aChildValue.isValid())
        {
            if (aTree->hasChildNode(aNode, aChildName))
            {
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value. Node '") );
                sMessage += sName;
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.")  );

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw lang::IllegalArgumentException( sMessage, xContext, 2 );
            }
            else
            {
                OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing child node not found by implementation");
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value. Value '") );
                sMessage += sName;
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
                sMessage += aTree->getAbsolutePath(aNode).toString();

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw container::NoSuchElementException( sMessage, xContext );
            }
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateSetValue(aChildValue, rElement);

        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));
//          lock.clearForBroadcast();

            aSender.queryConstraints(aChange);

            aTree->integrate(aChange, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw container::NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    catch (css::beans::PropertyVetoException& ex)
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value. Change was Vetoed: ") );
        throw lang::WrappedTargetException( sMessage += ex.Message, rNode.getUnoInstance(), uno::makeAny(ex) );
    }
}
//-----------------------------------------------------------------------------------

void implReplaceByName(NodeTreeSetAccess& rNode, const rtl::OUString& sName, const uno::Any& rElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeTreeSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateElementName(sName,aTree,aNode);

        rtl::Reference< configuration::ElementTree > aElement( aTree->getElement(aNode,aChildName) );

        if (!aElement.is())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element. Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw container::NoSuchElementException( sMessage, xContext );
        }

        rtl::Reference< configuration::ElementTree > aElementTree = configapi::extractElementTree(rNode.getFactory(), rElement, rNode.getElementInfo());
        if (!aElementTree.is())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Replacing object was not created from this set's template") );

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw lang::IllegalArgumentException( sMessage, xContext, 2 );
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateReplaceElement( aElement, aElementTree );

        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

            //aSender.queryConstraints(aChange); - N/A: no external constraints on set children possible

            aTree->integrate(aChange, aNode, true);
            attachSetElement(rNode, aElementTree);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw container::NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}
//-----------------------------------------------------------------------------------

void implReplaceByName(NodeValueSetAccess& rNode, const rtl::OUString& sName, const uno::Any& rElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeValueSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateElementName(sName,aTree,aNode);

        rtl::Reference< configuration::ElementTree > aElement( aTree->getElement(aNode,aChildName) );

        if (!aElement.is())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element. Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw container::NoSuchElementException( sMessage, xContext );
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateReplaceElement( aElement, rElement );

        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

            //aSender.queryConstraints(aChange); - N/A: no external constraints on set children possible

            aTree->integrate(aChange, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw container::NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XNameContainer
//-----------------------------------------------------------------------------------
void implInsertByName(NodeTreeSetAccess& rNode, const rtl::OUString& sName, const uno::Any& rElement)
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeTreeSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateElementName(sName,aTree,aNode);

        if( aTree->hasElement(aNode,aChildName) )
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set. Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is already present in Set ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw container::ElementExistException( sMessage, xContext );
        }
        OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

        rtl::Reference< configuration::ElementTree > aElementTree = configapi::extractElementTree(rNode.getFactory(), rElement, rNode.getElementInfo());
        if (!aElementTree.is())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set: ") );
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Inserted object was not created from this set's template") );

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw lang::IllegalArgumentException( sMessage, xContext, 2 );
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateInsertElement(aChildName, aElementTree);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Adding a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree->integrate(aChange, aNode, true);
        attachSetElement(rNode, aElementTree);

        lock.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument( 1 );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}

//-----------------------------------------------------------------------------------

void implInsertByName(NodeValueSetAccess& rNode, const rtl::OUString& sName, const uno::Any& rElement)
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeValueSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateElementName(sName,aTree,aNode);

        if( aTree->hasElement(aNode,aChildName) )
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set. Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is already present in Set ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw container::ElementExistException( sMessage, xContext );
        }
        OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

        configuration::NodeChange aChange = lock.getNodeUpdater().validateInsertElement(aChildName, rElement);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Adding a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree->integrate(aChange, aNode, true);

        lock.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument( 1 );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}

//-----------------------------------------------------------------------------------
void implRemoveByName(NodeTreeSetAccess& rNode, const rtl::OUString& sName )
    throw(css::container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeTreeSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateElementName(sName,aTree,aNode);

        rtl::Reference< configuration::ElementTree > aElement( aTree->getElement(aNode,aChildName) );

        if (!aElement.is())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element. Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw container::NoSuchElementException( sMessage, xContext );
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateRemoveElement(aElement);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Removing a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree->integrate(aChange, aNode, true);
        detachSetElement(rNode.getFactory(), aElement);

        lock.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw container::NoSuchElementException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}
//-----------------------------------------------------------------------------------
void implRemoveByName(NodeValueSetAccess& rNode, const rtl::OUString& sName )
    throw(css::container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeValueSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateElementName(sName,aTree,aNode);

        rtl::Reference< configuration::ElementTree > aElement = aTree->getElement(aNode,aChildName);
        if (!aElement.is())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element. Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw container::NoSuchElementException( sMessage, xContext );
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateRemoveElement(aElement);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Removing a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree->integrate(aChange, aNode, true);

        lock.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw container::NoSuchElementException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}
//-----------------------------------------------------------------------------------

// XPropertyWithState
//-----------------------------------------------------------------------------------

void implSetToDefaultAsProperty(NodeSetAccess& rNode)
    throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeSetAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::SetDefaulter aDefaulter = lock.getNodeDefaulter();

        configuration::NodeChange aChange = aDefaulter.validateSetToDefaultState();

        const bool bLocal = true;

        if (aChange.test().isChange() )
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,bLocal));

            aSender.queryConstraints(aChange);

            aTree->integrate(aChange, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );

        throw lang::WrappedTargetException( sMessage += e.message(), xContext, uno::Any());
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    catch (lang::WrappedTargetException& )  { throw;}
    catch (uno::RuntimeException& )         { throw;}
    catch (uno::Exception& e)
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );

        throw lang::WrappedTargetException( sMessage += e.Message, xContext, uno::makeAny(e));
    }
}
//-----------------------------------------------------------------------------------

// XSingleServiceFactory
//-----------------------------------------------------------------------------------
uno::Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode )
    throw(uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xRet;
    try
    {
        GuardedNodeData<NodeSetAccess> lock( rNode ); // no provider lock needed ? => if template lock is separate - OK

        rtl::Reference< configuration::ElementTree > aNewElement( rNode.getElementFactory().instantiateTemplate(rNode.getElementInfo()) );

        uno::Any aAny = configapi::makeElement( rNode.getFactory(), aNewElement );
        if (!(aAny >>= xRet)) // no parent available
        {
            OSL_ASSERT(!xRet.is()); // make sure we return NULL
            OSL_ENSURE(!aAny.hasValue(), "configmgr: BasicSetElement::getParent: could not extract parent - node is not an object");
        }
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return xRet;
}
//-----------------------------------------------------------------------------------

uno::Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode, const uno::Sequence< uno::Any >& aArguments )
    throw(uno::Exception, uno::RuntimeException)
{
    { (void)aArguments; }
    OSL_ENSURE(aArguments.getLength() == 0, "ConfigurationContainer: createInstance: Arguments not supported - ignoring ...");
    return implCreateElement(rNode);
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


