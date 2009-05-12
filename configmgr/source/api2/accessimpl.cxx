/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessimpl.cxx,v $
 * $Revision: 1.18 $
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
#include <stdio.h>

#include "accessimpl.hxx"
#include "apinodeaccess.hxx"
#include "valueref.hxx"
#include "anynoderef.hxx"
#include "noderef.hxx"
#include "configset.hxx"
#include "confignotifier.hxx"
#include "propertyinfohelper.hxx"
#include "treeiterators.hxx"
#include "attributes.hxx"
#include "apitypes.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <osl/diagnose.h>
#include <rtl/logfile.hxx>

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------------
        namespace lang = css::lang;
        namespace util = css::util;

//-----------------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------------

// Interface methods
//-----------------------------------------------------------------------------------


// XHierarchicalName
//------------------------------------------------------------------------------------------------------------------
rtl::OUString implGetHierarchicalName( NodeAccess& rNode ) throw(uno::RuntimeException)
{
    // RTL_LOGFILE_CONTEXT(aLog, "Configmgr::API::implGetHierarchicalName()");

    rtl::OUString sRet;
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree());

        configuration::AbsolutePath const aFullPath  = aTree->getAbsolutePath(lock.getNode());
        sRet = aFullPath.toString();
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return sRet;
}

//------------------------------------------------------------------------------------------------------------------
rtl::OUString implComposeHierarchicalName(NodeGroupInfoAccess& rNode, const rtl::OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException)
{
    rtl::OUString sRet;
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );
        configuration::NodeRef aNode( lock.getNode() );
        rtl::Reference< configuration::Tree > aTree( lock.getTree() );

        configuration::RelativePath const aAddedPath = configuration::validateRelativePath(sRelativeName, aTree, aNode);

        // TODO: add (relative) name validation based on node type - may then need provider lock
        configuration::AbsolutePath const aFullPath = aTree->getAbsolutePath(aNode).compose(aAddedPath);

        sRet = aFullPath.toString();
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(1);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }


    return sRet;
}

//------------------------------------------------------------------------------------------------------------------
rtl::OUString implComposeHierarchicalName(NodeSetInfoAccess& rNode, const rtl::OUString& sElementName )
    throw(css::lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException)
{
    rtl::OUString sRet;
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );
        configuration::NodeRef aNode( lock.getNode() );
        rtl::Reference< configuration::Tree > aTree( lock.getTree() );

        configuration::Path::Component const aAddedName = configuration::validateElementPathComponent(sElementName, aTree, aNode);

        // TODO: add (relative) name validation based on node type - may then need provider lock
        configuration::AbsolutePath const aFullPath = aTree->getAbsolutePath(aNode).compose(aAddedName);

        sRet = aFullPath.toString();
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(1);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }


    return sRet;
}

//------------------------------------------------------------------------------------------------------------------

// XElementAccess, base class of XNameAccess (and XHierarchicalNameAccess ? )
//-----------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
// for group nodes
uno::Type implGetElementType(NodeGroupInfoAccess& rNode) throw(uno::RuntimeException)
{
    rNode.checkAlive();
    // group nodes have a mix of types
    // TODO(?): Discover single common type
    return ::getCppuType( static_cast< uno::Any const*>(0) );
}

// for set nodes
uno::Type implGetElementType(NodeSetInfoAccess& rNode) throw(uno::RuntimeException)
{
    uno::Type aRet;
    try
    {
        GuardedNodeData<NodeSetInfoAccess> lock( rNode );

        aRet = rNode.getElementInfo()->getInstanceType();
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
// for group nodes
sal_Bool implHasElements(NodeGroupInfoAccess& rNode) throw(uno::RuntimeException)
{
    //  rNode.checkAlive();
//  return true;    // group nodes always have children

// Better: cater for the case where we are reaching the depth limit
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode ); // no provider lock needed

        rtl::Reference< configuration::Tree > aThisTree( lock.getTree() );
        configuration::NodeRef aThisNode( lock.getNode() );
        OSL_ASSERT( !aThisTree->hasElements(aThisNode) );
        return aThisTree->hasChildren(aThisNode);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    // unreachable, but still there to make compiler happy
    OSL_ASSERT(!"Unreachable code");
    return false;
}

// for set nodes
sal_Bool implHasElements(NodeSetInfoAccess& rNode) throw(uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode ); // provider lock needed

        rtl::Reference< configuration::Tree > aThisTree( lock.getTree() );
        configuration::NodeRef aThisNode( lock.getNode() );
        OSL_ASSERT( !aThisTree->hasChildren(aThisNode) );
        return aThisTree->hasElements(aThisNode);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    // unreachable, but still there to make compiler happy
    OSL_ASSERT(!"Unreachable code");
    return false;
}

// XExactName
//-----------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------
namespace internal
{
    struct SearchExactName : private configuration::NodeVisitor
    {
    protected:
        // warning: order dependency
        configuration::RelativePath aSearchPath;
        std::vector<configuration::Path::Component>::reverse_iterator pSearchComponent;
    public:
        explicit
        SearchExactName(const configuration::RelativePath& aLookFor)
        : aSearchPath(aLookFor)
        , pSearchComponent(aSearchPath.begin_mutate())
        {}

        bool complete() { return aSearchPath.end_mutate() == pSearchComponent; }

        bool search(configuration::NodeRef const& aNode, rtl::Reference< configuration::Tree > const& aTree);

        configuration::RelativePath const& getBestMatch() const { return aSearchPath; }

    private:
        bool findMatch(configuration::NodeRef& aNode, rtl::Reference< configuration::Tree > & aTree);
        virtual Result handle(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const&); // NodeVisitor
        virtual Result handle(rtl::Reference< configuration::Tree > const& aTree, configuration::ValueRef const&); // NodeVisitor
    };
//..................................................................................................................
    bool SearchExactName::findMatch(configuration::NodeRef& aNode, rtl::Reference< configuration::Tree > & aTree)
    {
        OSL_ASSERT( !complete() );

        if ( !aNode.isValid() ) return false;

        // exact match ?
        if (!configuration::hasChildOrElement(aTree,aNode,*pSearchComponent))
        {
            if (aTree->dispatchToChildren(aNode,*this) == CONTINUE) // not found there
                return false;
        }
        OSL_ASSERT(configuration::hasChildOrElement(aTree,aNode,*pSearchComponent));

        if (! configuration::findInnerChildOrAvailableElement(aTree,aNode,pSearchComponent->getName()) )
            aNode = configuration::NodeRef(); // will stop recursion (value or unloaded element found)
        ++pSearchComponent;

        return true;
    }
    //..................................................................................................................
    // helper
    static configuration::Path::Component getExtendedNodeName(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode, rtl::OUString const& aSimpleNodeName)
    {
        OSL_PRECOND( !configuration::isEmpty(aTree.get()), "ERROR: Configuration: Tree operation requires valid tree" );
        OSL_PRECOND( !aNode.isValid() || aTree->isValidNode(aNode.getOffset()), "ERROR: Configuration: NodeRef does not match tree" );

        if (aTree->isRootNode(aNode))
            return aTree->getExtendedRootName();

        else
            return configuration::Path::wrapSimpleName(aSimpleNodeName);
    }
    //..................................................................................................................
    configuration::NodeVisitor::Result SearchExactName::handle(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode)
    {
        OSL_ASSERT( aNode.isValid() );
        OSL_ASSERT( !complete() );

        // find inexact match (the first one, but the order is unspecified)
        // TODO: Add support for node-type-specific element names
        rtl::OUString aNodeName = aTree->getSimpleNodeName(aNode.getOffset());
        rtl::OUString aSearchName = pSearchComponent->getName();
        if (aNodeName.equalsIgnoreAsciiCase(aSearchName))
        {
            *pSearchComponent = getExtendedNodeName(aTree,aNode,aNodeName);
            return DONE; // for this level
        }
        else
            return CONTINUE;
    }
    //..................................................................................................................
    configuration::NodeVisitor::Result SearchExactName::handle(rtl::Reference< configuration::Tree > const&, configuration::ValueRef const& aNode)
    {
        OSL_ASSERT( aNode.isValid() );
        OSL_ASSERT( !complete() );

        // find inexact match (the first one, but the order is unspecified)
        // TODO: Add support for node-type-specific element names
        rtl::OUString aNodeName = aNode.m_sNodeName;
        OSL_ASSERT( configuration::isSimpleName(aNodeName) );

        // value refs are group members and thus have to have simple names
        if (aNodeName.equalsIgnoreAsciiCase(pSearchComponent->getName()))
        {
            *pSearchComponent = configuration::Path::wrapSimpleName(aNodeName);
            return DONE; // for this level
        }
        else
            return CONTINUE;
    }
//..................................................................................................................
    bool SearchExactName::search(configuration::NodeRef const & aNode, rtl::Reference< configuration::Tree > const & aTree)
    {
        if (!aNode.isValid()) return false;

        rtl::Reference< configuration::Tree > aSearchTree(aTree);
        configuration::NodeRef aSearchNode(aNode);

        while (!complete())
            if (! findMatch(aSearchNode, aSearchTree))
                break;

        return complete();
    }

} // namespace internal

//..................................................................................................................
rtl::OUString implGetExactName(NodeGroupInfoAccess& rNode, const rtl::OUString& rApproximateName ) throw(uno::RuntimeException)
{
    // here we try to support both tree-fragment-local pathes and simple names (the latter ones are just an instance of the first)
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree(lock.getTree());
        configuration::NodeRef aNode(lock.getNode());

        configuration::RelativePath aApproximatePath = configuration::validateRelativePath(rApproximateName,aTree,aNode);

        internal::SearchExactName aSearch(aApproximatePath);

        aSearch.search(aNode, aTree);

        OSL_ENSURE( aSearch.getBestMatch().getDepth() == aApproximatePath.getDepth(),
                    "Search for exact names changed number of path components !?");

        return aSearch.getBestMatch().toString();
    }
    catch (configuration::InvalidName& )
    {
        OSL_TRACE("WARNING: Configuration::getExactName: query uses locally invalid Path");
        return rApproximateName;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    // unreachable, but still there to make compiler happy
    OSL_ASSERT(!"Unreachable code");
    return rApproximateName;
}

//..................................................................................................................
rtl::OUString implGetExactName(NodeSetInfoAccess& rNode, const rtl::OUString& rApproximateName ) throw(uno::RuntimeException)
{

    // here we can support only local names
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree(lock.getTree());
        configuration::NodeRef aNode(lock.getNode());

        configuration::Path::Component aApproximateName = configuration::validateElementPathComponent(rApproximateName,aTree,aNode);

        internal::SearchExactName aSearch(aApproximateName);

        aSearch.search(aNode, aTree);

        OSL_ENSURE( aSearch.getBestMatch().getDepth() == 1,
                    "Search for exact names changed number of path components !?");

        return aSearch.getBestMatch().getLocalName().getName();
    }
    catch (configuration::InvalidName& )
    {
        OSL_TRACE("WARNING: Configuration::getExactName: query uses locally invalid Path");
        return rApproximateName;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    // unreachable, but still there to make compiler happy
    OSL_ASSERT(!"Unreachable code");
    return rApproximateName;
}

// XProperty
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
beans::Property implGetAsProperty(NodeAccess& rNode)
    throw(uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree());
        configuration::NodeRef aNode( lock.getNode());

        rtl::OUString aName       = aTree->getSimpleNodeName(aNode.getOffset());
        node::Attributes    aAttributes = aTree->getAttributes(aNode);
        uno::Type   aApiType    = getUnoInterfaceType();

        return helperMakeProperty( aName,aAttributes,aApiType, aTree->hasNodeDefault(aNode) );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return beans::Property();
}
// XPropertySetInfo
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
uno::Sequence< css::beans::Property > implGetProperties( NodeAccess& rNode ) throw (uno::RuntimeException)
{
    CollectPropertyInfo aCollect;

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        lock.getTree()->dispatchToChildren(lock.getNode(), aCollect);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return makeSequence( aCollect.list() );
}

//-----------------------------------------------------------------------------------
css::beans::Property implGetPropertyByName( NodeAccess& rNode, const rtl::OUString& aName )
    throw (css::beans::UnknownPropertyException, uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildOrElementName(aName,aTree,aNode);

        configuration::AnyNodeRef aChildNode = configuration::getChildOrElement(aTree,aNode, aChildName);

        if (!aChildNode.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Property not found by implementation");

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property. Property '") );
            sMessage += aName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw css::beans::UnknownPropertyException( sMessage, xContext );
        }

        node::Attributes    aChildAttributes = aTree->getAttributes(aChildNode);
        uno::Type   aApiType = aChildNode.isNode() ? getUnoInterfaceType() : aTree->getUnoType(aChildNode.toValue());

        return helperMakeProperty( aChildName,aChildAttributes,aApiType, aTree->hasNodeDefault(aChildNode) );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw css::beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return css::beans::Property();
}


//-----------------------------------------------------------------------------------
sal_Bool implHasPropertyByName( NodeAccess& rNode, const rtl::OUString& name ) throw (uno::RuntimeException)
{
    return implHasByName(rNode, name);
}

// XNameAccess
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// TODO: optimization - cache the node found (for subsequent getByName)
// TODO: optimization - less locking for group nodes
//-----------------------------------------------------------------------------------
sal_Bool implHasByName(NodeAccess& rNode, const rtl::OUString& sName ) throw(uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::OUString aChildName(sName);

        return configuration::hasChildOrElement(lock.getTree(), lock.getNode(), aChildName);
    }
#if OSL_DEBUG_LEVEL > 0
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OSL_ENSURE(false,"configmgr: BasicAccess::hasByName: Unexpected exception <InvalidName>");
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
#endif
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    // unreachable, but still there to make compiler happy
    OSL_ASSERT(!"Unreachable code");
    return false;
}

//-----------------------------------------------------------------------------------
uno::Any implGetByName(NodeAccess& rNode, const rtl::OUString& sName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree() );
        configuration::NodeRef aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildOrElementName(sName,aTree,aNode);

        configuration::AnyNodeRef aChildNode = configuration::getChildOrElement(aTree,aNode, aChildName);
        if (!aChildNode.isValid())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Child Element '") );
            sMessage += sName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw com::sun::star::container::NoSuchElementException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return configapi::makeElement( rNode.getFactory(), aTree, aChildNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw com::sun::star::container::NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return uno::Any();
}

//-----------------------------------------------------------------------------------
// TODO: optimization - less locking for group nodes
//-----------------------------------------------------------------------------------
uno::Sequence< rtl::OUString > implGetElementNames( NodeAccess& rNode ) throw( uno::RuntimeException)
{
    CollectNodeNames aCollect;

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        lock.getTree()->dispatchToChildren(lock.getNode(), aCollect);
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return makeSequence( aCollect.list() );
}

// XHierarchicalNameAccess
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// TO DO: optimization - cache the node found for subsequent getByHierarchicalName()
//-----------------------------------------------------------------------------------
sal_Bool implHasByHierarchicalName(NodeAccess& rNode, const rtl::OUString& sHierarchicalName ) throw(uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree() );
        configuration::NodeRef aNode( lock.getNode() );

        configuration::RelativePath aRelPath = configuration::validateAndReducePath( sHierarchicalName, aTree, aNode );

        return configuration::getDeepDescendant(aTree, aNode, aRelPath).isValid();
    }
    catch (configuration::InvalidName& )
    {
        OSL_TRACE("WARNING: Configuration::hasByHierarchicalName: query uses locally invalid Path");
        return false;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return false;
}

//-----------------------------------------------------------------------------------
uno::Any implGetByHierarchicalName(NodeAccess& rNode, const rtl::OUString& sHierarchicalName )
    throw(css::container::NoSuchElementException, uno::RuntimeException)
{
    // rtl::OUString aTmpStr(implGetHierarchicalName(rNode));
    // RTL_LOGFILE_CONTEXT_TRACE2(aLog, "Node: %s HierachicalName: %s",RTL_LOGFILE_OU2A(aTmpStr), RTL_LOGFILE_OU2A(sHierarchicalName));

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree() );
        configuration::NodeRef aNode( lock.getNode() );

        configuration::RelativePath aRelPath = configuration::validateAndReducePath( sHierarchicalName, aTree, aNode );

        configuration::AnyNodeRef aNestedNode = configuration::getDeepDescendant( aTree, aNode, aRelPath );
        if (!aNestedNode.isValid())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Descendant Element '") );
            sMessage += aRelPath.toString();
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Node ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw com::sun::star::container::NoSuchElementException( sMessage, xContext );
        }

        OSL_ASSERT(aNode.isValid());
        return configapi::makeElement( rNode.getFactory(), aTree, aNestedNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw com::sun::star::container::NoSuchElementException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return uno::Any();
}

// XPropertyWithState
//---------------------------------------------------------------------
css::beans::PropertyState implGetStateAsProperty(NodeAccess& rNode)
    throw (uno::RuntimeException)
{
    css::beans::PropertyState aRet = css::beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        if ( lock.getTree()->isNodeDefault( lock.getNode() ) )
            aRet = css::beans::PropertyState_DEFAULT_VALUE;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    return aRet;
}

uno::Reference< uno::XInterface > implGetDefaultAsProperty(NodeAccess& )
    throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    // not really supported

    /* possible, but nor really useful:
    GuardedNodeAccess lock( rNode );
    if (implGetStateAsProperty(rNode) == PropertyState_DEFAULT_VALUE)
        return rNode.getUnoInstance();
    */

    return uno::Reference< uno::XInterface >();
}


// set-specific Interfaces
//-----------------------------------------------------------------------------------


// XTemplateContainer
//-----------------------------------------------------------------------------------
rtl::OUString SAL_CALL implGetElementTemplateName(NodeSetInfoAccess& rNode)
    throw(uno::RuntimeException)
{
    GuardedNodeData<NodeSetInfoAccess> lock(rNode);
    return rNode.getElementInfo()->getPathString();
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


