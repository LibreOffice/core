/*************************************************************************
 *
 *  $RCSfile: propsetaccessimpl.cxx,v $
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

#include "propsetaccessimpl.hxx"

#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"
#include "noderef.hxx"
#include "nodechange.hxx"
#include "configgroup.hxx"
#include "confignotifier.hxx"
#include "broadcaster.hxx"

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
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
        namespace beans = css::beans;

        using uno::RuntimeException;
        using uno::Reference;
        using uno::Any;
        using uno::Sequence;

        using lang::IllegalArgumentException;
        using lang::WrappedTargetException;
        using beans::UnknownPropertyException;
        using beans::PropertyVetoException;

        using configuration::NodeRef;
        using configuration::Tree;
        using configuration::NodeChange;
        using configuration::NodeChanges;

        using configuration::Name;
        using configuration::Path;
        using configuration::AbsolutePath;
        using configuration::RelativePath;
        using configuration::validateNodeName;

// Interface methods
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// getting Property Metadata
//-----------------------------------------------------------------------------------

// XPropertySet & XMultiPropertySet
//-----------------------------------------------------------------------------------
Reference< beans::XPropertySetInfo > implGetPropertySetInfo( NodeGroupInfoAccess& rNode )
    throw(RuntimeException)
{
    // TODO: Implement
    return 0;
}

// XHierarchicalPropertySet & XHierarchicalMultiPropertySet
//-----------------------------------------------------------------------------------
Reference< beans::XHierarchicalPropertySetInfo > implGetHierarchicalPropertySetInfo( NodeGroupInfoAccess& rNode )
    throw(RuntimeException)
{
    // TODO: Implement
    return 0;
}

//-----------------------------------------------------------------------------------
// setting values - may all throw (PropertyVeto)Exceptions on read-only property sets
//-----------------------------------------------------------------------------------

// XPropertySet
//-----------------------------------------------------------------------------------
void implSetPropertyValue( NodeGroupAccess& rNode, const OUString& sPropertyName, const Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
              lang::WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sPropertyName,aTree,aNode);

        NodeRef aChild( aTree.getChild(aNode, aChildName) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Property not found by node");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Property \"") );
            sMessage += sPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }

        NodeChange aChange = impl->getNodeUpdater().validateSetValue( aChild, aValue );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, true);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------
void implSetPropertyValues( NodeGroupAccess& rNode, const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
              lang::WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aValues.getLength(); i < count; ++i)
        {
            Name aChildName = configuration::makeName( aPropertyNames[i] ); // not validated

            NodeRef aChild( aTree.getChild(aNode, aChildName) );

            if (!aChild.isValid())
            {
                OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Property not found by node");
                OSL_TRACE("Configuration: MultiPropertySet: trying to set unknown property - ignored");
                continue;
            }

            NodeChange aChange = impl->getNodeUpdater().validateSetValue( aChild, aValues[i] );
            if (aChange.isChange())
            {
                aChanges.add(aChange);
            }
        }

        if (!aChanges.test().isEmpty())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChanges.compact(),true));

            aSender.queryConstraints(aChanges);

            aTree.integrate(aChanges, aNode, true);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChanges);
        }
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XHierarchicalPropertySet
//-----------------------------------------------------------------------------------
void implSetHierarchicalPropertyValue( NodeGroupAccess& rNode, const OUString& aPropertyName, const Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, RuntimeException)
{
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::findDescendantNode; // should actually be found by "Koenig" lookup, but MSVC6 fails

    try
    {
        GuardedGroupUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        RelativePath const aRelPath = reduceRelativePath( aPropertyName, aTree, aNode );

        Tree aNestedTree( aTree );
        NodeRef aNestedNode( aNode );
        RelativePath aResolvePath( aRelPath );

        if (!findDescendantNode( aNestedTree, aNestedNode, aResolvePath ))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Nested Property \"") );
            sMessage += aResolvePath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aNestedTree.getLocalPath(aNestedNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        NodeChange aChange = impl->getNodeUpdater().validateSetDeepValue( aNestedTree, aNestedNode, aRelPath, aValue );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,false));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, false);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XMultiHierarchicalPropertySet
//-----------------------------------------------------------------------------------
void implSetHierarchicalPropertyValues( NodeGroupAccess& rNode, const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, RuntimeException)
{
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::findDescendantNode; // should actually be found by "Koenig" lookup, but MSVC6 fails

    try
    {
        GuardedGroupUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aValues.getLength(); i < count; ++i)
        try
        {
            RelativePath aRelPath = reduceRelativePath( aPropertyNames[i], aTree, aNode );

            Tree aNestedTree( aTree );
            NodeRef aNestedNode( aNode );
            RelativePath aResolvePath( aRelPath );

            if (!findDescendantNode( aNestedTree, aNestedNode, aResolvePath ))
            {
                OSL_TRACE("Configuration: MultiHierarchicalPropertySet: trying to set unknown property - ignored");
                continue;
            }
            OSL_ASSERT(aNestedNode.isValid());

            NodeChange aChange = impl->getNodeUpdater().validateSetDeepValue( aNestedTree, aNestedNode, aRelPath, aValues[i] );
            if (aChange.isChange())
            {
                aChanges.add(aChange);
            }
        }
        catch (configuration::InvalidName& )
        {
            OSL_TRACE("Configuration: MultiHierarchicalPropertySet: trying to set property <invalid path> - ignored");
            continue;
        }

        if (!aChanges.test().isEmpty())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChanges.compact(),false));

            aSender.queryConstraints(aChanges);

            aTree.integrate(aChanges, aNode, false);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChanges);
        }
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
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
// getting values
//-----------------------------------------------------------------------------------

// XPropertySet
//-----------------------------------------------------------------------------------
Any implGetPropertyValue( NodeGroupInfoAccess& rNode,const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(aPropertyName,aTree,aNode);

        NodeRef aChild( aTree.getChild(aNode, aChildName) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Property not found by node");
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Property \"") );
            sMessage += aPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }

        return configapi::makeElement( impl->getFactory(), aTree, aChild );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't read Property: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return Any();
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------
Sequence< Any > implGetPropertyValues( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<Any> aRet(count);

    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            Name aChildName = configuration::makeName( aPropertyNames[i] ); // not validated

            NodeRef aChild( aTree.getChild(aNode, aChildName) );

            Any aValue;
            if (aChild.isValid())
            {
                aValue = configapi::makeElement( impl->getFactory(), aTree, aChild );
            }
            else
            {
                OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Property not found by node");
                OSL_TRACE("Configuration: MultiPropertySet: trying to get unknown property - returning void");
            }
            aRet[i] = aValue;
        }
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aRet;
}

// XHierarchicalPropertySet
//-----------------------------------------------------------------------------------
Any implGetHierarchicalPropertyValue( NodeGroupInfoAccess& rNode, const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::findDescendantNode; // should actually be found by "Koenig" lookup, but MSVC6 fails
    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree aTree( impl->getTree() );
        NodeRef aNode( impl->getNode() );

        RelativePath aRelPath = reduceRelativePath( aPropertyName, aTree, aNode );

        if (!findDescendantNode( aTree, aNode, aRelPath ))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Descendant Element \"") );
            sMessage += aRelPath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( impl->getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }

        OSL_ASSERT(aNode.isValid());
        return configapi::makeElement( impl->getFactory(), aTree, aNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return Any();
}

// XMultiHierarchicalPropertySet
//-----------------------------------------------------------------------------------
Sequence< Any > implGetHierarchicalPropertyValues( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(RuntimeException)
{
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::findDescendantNode; // should actually be found by "Koenig" lookup, but MSVC6 fails

    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<Any> aRet(count);

    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        try
        {
            RelativePath aRelPath = reduceRelativePath( aPropertyNames[i], aTree, aNode );

            Any aValue;
            Tree aNestedTree( aTree );
            NodeRef aNestedNode( aNode );

            if (findDescendantNode( aNestedTree, aNestedNode, aRelPath ))
            {
                OSL_ASSERT(aNestedNode.isValid());
                aValue = configapi::makeElement( impl->getFactory(), aNestedTree, aNestedNode );
            }
            else
            {
                OSL_TRACE("Configuration: MultiPropertySet: trying to get unknown property - returning void");
            }
            aRet[i] = aValue;
        }
        catch (configuration::InvalidName& )
        {
            OSL_TRACE("Configuration: MultiPropertySet: trying to get property from unknown path - returning void");
            OSL_ASSERT(!aRet[i].hasValue());
        }
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aRet;
}

//------------------------------------------------------------------------------------------------------------------
// SPECIAL: XMultiPropertySet::firePropertiesChangeEvent
//------------------------------------------------------------------------------------------------------------------

void implFirePropertiesChangeEvent( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames, const Reference< beans::XPropertiesChangeListener >& xListener )
    throw(RuntimeException)
{
    OSL_ENSHURE(xListener.is(), "ERROR: requesting to fire Events to a NULL listener.");
    if (!xListener.is())
    {
        return; // should this be an exception ??
    }

    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<beans::PropertyChangeEvent> aEvents(count);

    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );
        configapi::Factory& rFactory = impl->getFactory();

        sal_Int32 nFire = 0;

        for(sal_Int32 i = 0; i < count; ++i)
        {
            Name aChildName = configuration::makeName( aPropertyNames[i] ); // not validated

            NodeRef aChild( aTree.getChild(aNode, aChildName) );

            if (aChild.isValid())
            {
                aEvents[nFire].Source = impl->getUnoInstance();
                aEvents[nFire].PropertyName = aChildName.toString();
                aEvents[nFire].PropertyHandle = -1;

                aEvents[nFire].NewValue = aEvents[nFire].OldValue = configapi::makeElement( rFactory, aTree, aChild );
                nFire++;
            }
            else
            {
                OSL_ENSURE(!configuration::hasChildNode(aTree,aNode,aChildName),"ERROR: Configuration: Property not found by node");
                OSL_TRACE("Configuration: MultiPropertySet: request to fire unknown property - skipping");
            }
        }

        if (nFire < count) aEvents.realloc(nFire);;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    xListener->propertiesChange(aEvents);
}

//------------------------------------------------------------------------------------------------------------------
// XPropertyState
//------------------------------------------------------------------------------------------------------------------
beans::PropertyState implGetPropertyState( NodeAccess& rNode, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    try
    {
        using configuration::findChildNode;

        GuardedNodeDataAccess impl( rNode );

        Tree aTree( impl->getTree() );
        NodeRef aNode( impl->getNode() );

        Name aChildName = validateNodeName(sPropertyName,aTree,aNode);

        if (!findChildNode(aTree,aNode,aChildName))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Property \"") );
            sMessage += sPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        //aTree.ensureDefaults();
        return aTree.isNodeDefault(aNode) ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't read Property: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return beans::PropertyState_AMBIGUOUS_VALUE;
}

//-----------------------------------------------------------------------------------
Sequence< beans::PropertyState > implGetPropertyStates( NodeAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<beans::PropertyState> aRet(count);

    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            using configuration::findChildNode;

            Name aChildName = validateNodeName(aPropertyNames[i],aTree,aNode);

            Tree aChildTree( aTree);
            NodeRef aChildNode( aNode );

            if (!findChildNode(aChildTree, aChildNode, aChildName))
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Property \"") );
                sMessage += aPropertyNames[i];
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
                sMessage += aTree.getLocalPath(aNode).toString();

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aChildNode.isValid());

            aRet[i] = aChildTree.isNodeDefault(aChildNode) ? beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aRet;
}

//-----------------------------------------------------------------------------------
void implSetPropertyToDefault( NodeGroupAccess& rNode, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    try
    {
        using configuration::findChildNode;

        GuardedGroupUpdateAccess impl( rNode );

        Tree const aTree( impl->getTree() );
        NodeRef const aNode( impl->getNode() );

        Name aChildName = validateNodeName(sPropertyName,aTree,aNode);

        Tree aChildTree( aTree);
        NodeRef aChildNode( aNode );

        if (!findChildNode(aChildTree, aChildNode, aChildName))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Property \"") );
            sMessage += sPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aChildNode.isValid());

        aChildTree.ensureDefaults();

        NodeChange aChange = impl->getNodeUpdater().validateSetDefault( aChildNode );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(impl->getNotifier().makeBroadcaster(aChange,true));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, true);

            impl.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't update Property: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

//-----------------------------------------------------------------------------------
Any implGetPropertyDefault( NodeAccess& rNode, const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    try
    {
        using configuration::findChildNode;

        GuardedNodeDataAccess impl( rNode );

        Tree aTree( impl->getTree() );
        NodeRef aNode( impl->getNode() );

        Name aChildName = validateNodeName(aPropertyName,aTree,aNode);

        if (!findChildNode(aTree, aNode, aChildName))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Property \"") );
            sMessage += aPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        aTree.ensureDefaults();
        return aTree.getNodeDefault(aNode);
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("ConfigurationUpdate can't read Property: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return Any();
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


