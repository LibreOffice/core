/*************************************************************************
 *
 *  $RCSfile: updateimpl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-16 18:15:43 $
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
#include "accessimpl.hxx"

#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"

#include "noderef.hxx"
#include "nodechange.hxx"
#include "configset.hxx"
#include "configgroup.hxx"

#include "confignotifier.hxx"
#include "broadcaster.hxx"
#include "encodename.hxx"

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
        using configuration::ElementTree;
        using configuration::Tree;
        using configuration::Name;
        using configuration::Path;
        using configuration::AbsolutePath;
        using configuration::RelativePath;
        using configuration::validateNodeName;

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
        GuardedGroupUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        NodeRef aChild( aTree.getChild(aNode, aChildName) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Group child not found by tree");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Child Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = impl->getNodeUpdater().validateSetValue(aChild, rElement);

        if (aChange.test().isChange())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));
//          impl.clearForBroadcast();

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, true);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: ") );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: ") );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: Constraint violation (veto): ") );
        throw WrappedTargetException( sMessage += ex.Message, rNode.getUnoInstance(), uno::makeAny(ex) );
    }
}
//-----------------------------------------------------------------------------------

void implReplaceByName(NodeTreeSetAccess& rNode, const OUString& sName, const Any& rElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedSetUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        Tree aChildTree( aTree );
        NodeRef aChild( aNode.getChild(aChildName, aChildTree) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Set child not found by node");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in Set NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        ElementTree aElementTree = configapi::extractElementTree(impl->getFactory(), rElement, impl->getElementInfo() );
        if (!aElementTree.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Replacing Element was not created from this set's template") );

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw IllegalArgumentException( sMessage, xContext, 2 );
        }

        NodeChange aChange = rNode.getNodeUpdater().validateReplaceElement( aChildTree, aChild, aElementTree );

        if (aChange.test().isChange())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

            //aSender.queryConstraints(aChange); - N/A: no external constraints on set children possible

            aTree.integrate(aChange, aNode, true);
            attachSetElement(rNode, aElementTree);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate: can't update NodeRef: ") );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: ") );
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
        GuardedSetUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        Tree aChildTree( aTree );
        NodeRef aChild( aNode.getChild(aChildName, aChildTree) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Set child not found by node");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in Set NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = rNode.getNodeUpdater().validateReplaceElement( aChildTree, aChild, rElement );

        if (aChange.test().isChange())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

            //aSender.queryConstraints(aChange); - N/A: no external constraints on set children possible

            aTree.integrate(aChange, aNode, true);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate: can't update NodeRef: ") );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: ") );
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
        GuardedSetUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        if( aNode.hasChild(aChildName) )
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" already present in Set NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw ElementExistException( sMessage, xContext );
        }
        OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Set child not found by node");


        ElementTree aElementTree = configapi::extractElementTree(impl->getFactory(), rElement, impl->getElementInfo() );
        if (!aElementTree.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Inserted Element was not created from this set's template") );

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw IllegalArgumentException( sMessage, xContext, 2 );
        }

        NodeChange aChange = rNode.getNodeUpdater().validateInsertElement(aChildName, aElementTree);
        OSL_ENSHURE(aChange.test().isChange(), "ERROR: Adding a node validated as empty change");

        Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);
        attachSetElement(rNode, aElementTree);

        impl.clearForBroadcast();
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't insert NodeRef: ") );
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
        GuardedSetUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        if( aNode.hasChild(aChildName) )
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" already present in Set NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw ElementExistException( sMessage, xContext );
        }
        OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Set child not found by node");

        NodeChange aChange = rNode.getNodeUpdater().validateInsertElement(aChildName, rElement);
        OSL_ENSHURE(aChange.test().isChange(), "ERROR: Adding a node validated as empty change");

        Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);

        impl.clearForBroadcast();
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't insert NodeRef: ") );
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
        GuardedSetUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        Tree aChildTree( aTree );
        NodeRef aChild( aNode.getChild(aChildName, aChildTree) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Set child not found by node");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in Set NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = rNode.getNodeUpdater().validateRemoveElement(aChildTree, aChild);
        OSL_ENSHURE(aChange.test().isChange(), "ERROR: Removing a node validated as empty change");

        Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);
        detachSetElement(impl->getFactory(), ElementTree::extract(aChildTree));

        impl.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate: can't update NodeRef: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: ") );
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
        GuardedSetUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sName,aTree,aNode);

        Tree aChildTree( aTree );
        NodeRef aChild( aNode.getChild(aChildName, aChildTree) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Set child not found by node");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in Set NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        NodeChange aChange = rNode.getNodeUpdater().validateRemoveElement(aChildTree, aChild);
        OSL_ENSHURE(aChange.test().isChange(), "ERROR: Removing a node validated as empty change");

        Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

        //aSender.queryConstraints(); - N/A: no external constraints on set children possible

        aTree.integrate(aChange, aNode, true);

        impl.clearForBroadcast();
        aSender.notifyListeners(aChange);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate: can't update NodeRef: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update NodeRef: ") );
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

// XSingleServiceFactory
//-----------------------------------------------------------------------------------
Reference< uno::XInterface > implCreateElement(NodeTreeSetAccess& rNode )
    throw(Exception, RuntimeException)
{
    Reference< uno::XInterface > xRet;
    try
    {
        GuardedNode<NodeSetAccess> impl( rNode ); // no provider lock needed ? => if template lock is separate - OK

        ElementTree aNewElement( rNode.getElementFactory().instantiateTemplate(impl->getElementInfo().getTemplate()) );

        Any aAny = configapi::makeElement( impl->getFactory(), aNewElement );
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


