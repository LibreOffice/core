/*************************************************************************
 *
 *  $RCSfile: updateimpl.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:38 $
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

#include "updateimpl.hxx"

#ifndef CONFIGMGR_API_BASEACCESSIMPL_HXX_
#include "accessimpl.hxx"
#endif
#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#include "apinodeaccess.hxx"
#endif
#ifndef CONFIGMGR_API_NODEUPDATE_HXX_
#include "apinodeupdate.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#include "valueref.hxx"
#endif

#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGSET_HXX_
#include "configset.hxx"
#endif
#ifndef CONFIGMGR_CONFIGGROUP_HXX_
#include "configgroup.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#include "confignotifier.hxx"
#endif
#ifndef CONFIGMGR_API_BROADCASTER_HXX_
#include "broadcaster.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVETOEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyVetoException.hpp>
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
        namespace container = css::container;

        using uno::Reference;
        using uno::Any;
        using uno::Sequence;
        using uno::Exception;
        using uno::RuntimeException;
        using lang::NoSupportException;
        using lang::IllegalArgumentException;
        using lang::WrappedTargetException;
        using container::NoSuchElementException;
        using container::ElementExistException;

        using configuration::NodeRef;
        using configuration::NodeChange;
        using configuration::Tree;
        using configuration::ElementRef;
        using configuration::ElementTree;
        using configuration::Tree;
        using configuration::Name;
        using configuration::AbsolutePath;
        using configuration::RelativePath;
        using configuration::validateChildName;
        using configuration::validateElementName;

// Interface methods
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Update methods
//-----------------------------------------------------------------------------------

// XNameReplace
//-----------------------------------------------------------------------------------
void implReplaceByName(NodeGroupAccess& rNode, const OUString& sName, const Any& rElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateChildName(sName,aTree,aNode);

        ValueRef aChildValue( aTree.getChildValue(aNode, aChildName) );

        if (!aChildValue.isValid())
        {
            if (aTree.hasChildNode(aNode, aChildName))
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value. Node '") );
                sMessage += sName;
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.")  );

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw IllegalArgumentException( sMessage, xContext, 2 );
            }
            else
            {
                OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing child node not found by implementation");
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value. Value '") );
                sMessage += sName;
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
                sMessage += aTree.getAbsolutePath(aNode).toString();

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw NoSuchElementException( sMessage, xContext );
            }
        }

        NodeChange aChange = lock.getNodeUpdater().validateSetValue(aChildValue, rElement);

        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));
//          lock.clearForBroadcast();

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
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
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    catch (css::beans::PropertyVetoException& ex)
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Value. Change was Vetoed: ") );
        throw WrappedTargetException( sMessage += ex.Message, rNode.getUnoInstance(), uno::makeAny(ex) );
    }
}
//-----------------------------------------------------------------------------------

void implReplaceByName(NodeTreeSetAccess& rNode, const OUString& sName, const Any& rElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedTreeSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateElementName(sName,aTree,aNode);

        ElementRef aElement( aTree.getElement(aNode,aChildName) );

        if (!aElement.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element. Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        ElementTree aElementTree = configapi::extractElementTree(rNode.getFactory(), rElement, rNode.getElementInfo(lock.getDataAccessor()) );
        if (!aElementTree.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("Replacing object was not created from this set's template") );

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw IllegalArgumentException( sMessage, xContext, 2 );
        }

        NodeChange aChange = lock.getNodeUpdater().validateReplaceElement( aElement, aElementTree );

        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

            //aSender.queryConstraints(aChange); - N/A: no external constraints on set children possible

            aTree.integrate(aChange, aNode, true);
            attachSetElement(rNode, aElementTree);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}
//-----------------------------------------------------------------------------------

void implReplaceByName(NodeValueSetAccess& rNode, const OUString& sName, const Any& rElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedValueSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateElementName(sName,aTree,aNode);

        ElementRef aElement( aTree.getElement(aNode,aChildName) );

        if (!aElement.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element. Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = lock.getNodeUpdater().validateReplaceElement( aElement, rElement );

        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

            //aSender.queryConstraints(aChange); - N/A: no external constraints on set children possible

            aTree.integrate(aChange, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
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
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot replace Set Element: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
void implInsertByName(NodeTreeSetAccess& rNode, const OUString& sName, const Any& rElement)
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedTreeSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateElementName(sName,aTree,aNode);

        if( aTree.hasElement(aNode,aChildName) )
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set. Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is already present in Set ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw ElementExistException( sMessage, xContext );
        }
        OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

        ElementTree aElementTree = configapi::extractElementTree(rNode.getFactory(), rElement, rNode.getElementInfo(lock.getDataAccessor()) );
        if (!aElementTree.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set: ") );
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("Inserted object was not created from this set's template") );

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw IllegalArgumentException( sMessage, xContext, 2 );
        }

        NodeChange aChange = lock.getNodeUpdater().validateInsertElement(aChildName, aElementTree);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Adding a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);
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
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}

//-----------------------------------------------------------------------------------

void implInsertByName(NodeValueSetAccess& rNode, const OUString& sName, const Any& rElement)
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedValueSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateElementName(sName,aTree,aNode);

        if( aTree.hasElement(aNode,aChildName) )
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set. Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is already present in Set ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw ElementExistException( sMessage, xContext );
        }
        OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

        NodeChange aChange = lock.getNodeUpdater().validateInsertElement(aChildName, rElement);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Adding a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);

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
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot insert into Set: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}

//-----------------------------------------------------------------------------------
void implRemoveByName(NodeTreeSetAccess& rNode, const OUString& sName )
    throw(css::container::NoSuchElementException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedTreeSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateElementName(sName,aTree,aNode);

        ElementRef aElement( aTree.getElement(aNode,aChildName) );

        if (!aElement.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element. Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = lock.getNodeUpdater().validateRemoveElement(aElement);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Removing a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);
        detachSetElement(rNode.getFactory(), aElement);

        lock.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

}
//-----------------------------------------------------------------------------------
void implRemoveByName(NodeValueSetAccess& rNode, const OUString& sName )
    throw(css::container::NoSuchElementException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedValueSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateElementName(sName,aTree,aNode);

        ElementRef aElement = aTree.getElement(aNode,aChildName);
        if (!aElement.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Set element not found by implementation");

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element. Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Set ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = lock.getNodeUpdater().validateRemoveElement(aElement);

        aChange.test(); // make sure old values are set up correctly
        OSL_ENSURE(aChange.isChange(), "ERROR: Removing a node validated as empty change");

        Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);

        lock.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot remove Set Element: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
        GuardedSetUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        configuration::SetDefaulter aDefaulter = lock.getNodeDefaulter();

        NodeChange aChange = aDefaulter.validateSetToDefaultState();

        const bool bLocal = true;

        if (aChange.test().isChange() )
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,bLocal));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );

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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );

        throw lang::WrappedTargetException( sMessage += e.Message, xContext, uno::makeAny(e));
    }
}
//-----------------------------------------------------------------------------------

// XSingleServiceFactory
//-----------------------------------------------------------------------------------
Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode )
    throw(Exception, RuntimeException)
{
    Reference< uno::XInterface > xRet;
    try
    {
        GuardedNodeData<NodeSetAccess> lock( rNode ); // no provider lock needed ? => if template lock is separate - OK

        data::Accessor aDataAccess = lock.getDataAccessor();
        ElementTree aNewElement( rNode.getElementFactory(aDataAccess).instantiateTemplate(rNode.getElementInfo(aDataAccess).getTemplate()) );

        Any aAny = configapi::makeElement( rNode.getFactory(), aNewElement );
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

Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode, const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
    OSL_ENSURE(aArguments.getLength() == 0, "ConfigurationContainer: createInstance: Arguments not supported - ignoring ...");
    return implCreateElement(rNode);
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


