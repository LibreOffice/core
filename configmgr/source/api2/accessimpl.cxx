/*************************************************************************
 *
 *  $RCSfile: accessimpl.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:16:15 $
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
#include <stdio.h>

#include "accessimpl.hxx"

#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#include "apinodeaccess.hxx"
#endif
#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#include "valueref.hxx"
#endif
#ifndef CONFIGMGR_CONFIGANYNODE_HXX_
#include "anynoderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGSET_HXX_
#include "configset.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#include "confignotifier.hxx"
#endif
#ifndef CONFIGMGR_API_PROPERTYINFOIMPL_HXX_
#include "propertyinfohelper.hxx"
#endif
#ifndef CONFIGMGR_TREEITERATORS_HXX_
#include "treeiterators.hxx"
#endif
#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif
#ifndef CONFIGMGR_API_APITYPES_HXX_
#include "apitypes.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVETOEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyVetoException.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), RTL_TEXTENCODING_ASCII_US).getStr())

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------------
        namespace lang = css::lang;
        namespace util = css::util;

        using uno::RuntimeException;
        using uno::Reference;
        using uno::Any;
        using uno::Sequence;
        using lang::NoSupportException;

        using configuration::AnyNodeRef;
        using configuration::NodeRef;
        using configuration::ValueRef;

        using configuration::Tree;
        using configuration::Name;
        using configuration::AbsolutePath;
        using configuration::RelativePath;
        using node::Attributes;

//-----------------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------------

// Interface methods
//-----------------------------------------------------------------------------------


// XHierarchicalName
//------------------------------------------------------------------------------------------------------------------
OUString implGetHierarchicalName( NodeAccess& rNode ) throw(RuntimeException)
{
    // RTL_LOGFILE_CONTEXT(aLog, "Configmgr::API::implGetHierarchicalName()");

    OUString sRet;
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree aTree( lock.getTree());

        AbsolutePath const aFullPath  = aTree.getAbsolutePath(lock.getNode());
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
OUString implComposeHierarchicalName(NodeGroupInfoAccess& rNode, const OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, NoSupportException, RuntimeException)
{
    using configuration::validateRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails there
    OUString sRet;
    try
    {
        GuardedNodeDataAccess lock( rNode );
        NodeRef aNode( lock.getNode() );
        Tree aTree( lock.getTree() );

        RelativePath const aAddedPath = validateRelativePath(sRelativeName, aTree, aNode);

        // TODO: add (relative) name validation based on node type - may then need provider lock
        AbsolutePath const aFullPath = aTree.getAbsolutePath(aNode).compose(aAddedPath);

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
OUString implComposeHierarchicalName(NodeSetInfoAccess& rNode, const OUString& sElementName )
    throw(css::lang::IllegalArgumentException, NoSupportException, RuntimeException)
{
    using configuration::validateElementPathComponent; // should actually be found by "Koenig" lookup, but MSVC6 fails there
    using configuration::Path::Component;
    OUString sRet;
    try
    {
        GuardedNodeDataAccess lock( rNode );
        NodeRef aNode( lock.getNode() );
        Tree aTree( lock.getTree() );

        /*Path::*/Component const aAddedName = validateElementPathComponent(sElementName, aTree, aNode);

        // TODO: add (relative) name validation based on node type - may then need provider lock
        AbsolutePath const aFullPath = aTree.getAbsolutePath(aNode).compose(aAddedName);

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
uno::Type implGetElementType(NodeGroupInfoAccess& rNode) throw(RuntimeException)
{
    rNode.checkAlive();
    // group nodes have a mix of types
    // TODO(?): Discover single common type
    return ::getCppuType( static_cast< uno::Any const*>(0) );
}

// for set nodes
uno::Type implGetElementType(NodeSetInfoAccess& rNode) throw(RuntimeException)
{
    uno::Type aRet;
    try
    {
        GuardedNodeData<NodeSetInfoAccess> lock( rNode );

        aRet = rNode.getElementInfo(lock.getDataAccessor()).getElementType();
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
sal_Bool implHasElements(NodeGroupInfoAccess& rNode) throw(RuntimeException)
{
    //  rNode.checkAlive();
//  return true;    // group nodes always have children

// Better: cater for the case where we are reaching the depth limit
    try
    {
        GuardedNodeDataAccess lock( rNode ); // no provider lock needed

        Tree    aThisTree( lock.getTree() );
        NodeRef aThisNode( lock.getNode() );
        OSL_ASSERT( !aThisTree.hasElements(aThisNode) );
        return aThisTree.hasChildren(aThisNode);
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
sal_Bool implHasElements(NodeSetInfoAccess& rNode) throw(RuntimeException)
{
    try
    {
        GuardedNodeDataAccess lock( rNode ); // provider lock needed

        Tree    aThisTree( lock.getTree() );
        NodeRef aThisNode( lock.getNode() );
        OSL_ASSERT( !aThisTree.hasChildren(aThisNode) );
        return aThisTree.hasElements(aThisNode);
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
    using namespace configuration;
    struct SearchExactName : private NodeVisitor
    {
    protected:
        // warning: order dependency
        RelativePath aSearchPath;
        RelativePath::MutatingIterator pSearchComponent;
    public:
        explicit
        SearchExactName(const RelativePath& aLookFor)
        : aSearchPath(aLookFor)
        , pSearchComponent(aSearchPath.begin_mutate())
        {}

        bool complete() { return aSearchPath.end_mutate() == pSearchComponent; }

        bool search(NodeRef const& aNode, Tree const& aTree);

        RelativePath const& getBestMatch() const { return aSearchPath; }

    private:
        bool findMatch(NodeRef& aNode, Tree& aTree);
        virtual Result handle(Tree const& aTree, NodeRef const&); // NodeVisitor
        virtual Result handle(Tree const& aTree, ValueRef const&); // NodeVisitor
    };
//..................................................................................................................
    bool SearchExactName::findMatch(NodeRef& aNode, Tree& aTree)
    {
        OSL_ASSERT( !complete() );

        if ( !aNode.isValid() ) return false;

        using configuration::hasChildOrElement;
        using configuration::findInnerChildOrAvailableElement;

        // exact match ?
        if (!hasChildOrElement(aTree,aNode,*pSearchComponent))
        {
            if (aTree.dispatchToChildren(aNode,*this) == CONTINUE) // not found there
                return false;
        }
        OSL_ASSERT(hasChildOrElement(aTree,aNode,*pSearchComponent));

        if (! findInnerChildOrAvailableElement(aTree,aNode,pSearchComponent->getName()) )
            aNode = NodeRef(); // will stop recursion (value or unloaded element found)
        ++pSearchComponent;

        return true;
    }
    //..................................................................................................................
    // helper
    static Path::Component getExtendedNodeName(Tree const& aTree, NodeRef const& aNode, Name const& aSimpleNodeName)
    {
        OSL_PRECOND( !aTree.isEmpty(), "ERROR: Configuration: Tree operation requires valid tree" );
        OSL_PRECOND( !aNode.isValid() || aTree.isValidNode(aNode), "ERROR: Configuration: NodeRef does not match tree" );

        if (aTree.isRootNode(aNode))
            return aTree.getRootName();

        else
            return Path::wrapSimpleName(aSimpleNodeName);
    }
    //..................................................................................................................
    NodeVisitor::Result SearchExactName::handle(Tree const& aTree, NodeRef const& aNode)
    {
        OSL_ASSERT( aNode.isValid() );
        OSL_ASSERT( !complete() );

        // find inexact match (the first one, but the order is unspecified)
        // TODO: Add support for node-type-specific element names
        Name aNodeName = aTree.getName(aNode);
        Name aSearchName = pSearchComponent->getName();
        if (aNodeName.toString().equalsIgnoreAsciiCase(aSearchName.toString()))
        {
            *pSearchComponent = getExtendedNodeName(aTree,aNode,aNodeName);
            return DONE; // for this level
        }
        else
            return CONTINUE;
    }
    //..................................................................................................................
    NodeVisitor::Result SearchExactName::handle(Tree const& aTree, ValueRef const& aNode)
    {
        OSL_ASSERT( aNode.isValid() );
        OSL_ASSERT( !complete() );

        // find inexact match (the first one, but the order is unspecified)
        // TODO: Add support for node-type-specific element names
        Name aNodeName = aTree.getName(aNode);
        OSL_ASSERT( isSimpleName(aNodeName) );

        // value refs are group members and thus have to have simple names
        if (aNodeName.toString().equalsIgnoreAsciiCase(pSearchComponent->getName().toString()))
        {
            *pSearchComponent = Path::wrapSimpleName(aNodeName);
            return DONE; // for this level
        }
        else
            return CONTINUE;
    }
//..................................................................................................................
    bool SearchExactName::search(NodeRef const & aNode, Tree const & aTree)
    {
        if (!aNode.isValid()) return false;

        Tree    aSearchTree(aTree);
        NodeRef aSearchNode(aNode);

        while (!complete())
            if (! findMatch(aSearchNode, aSearchTree))
                break;

        return complete();
    }

} // namespace internal

//..................................................................................................................
OUString implGetExactName(NodeGroupInfoAccess& rNode, const OUString& rApproximateName ) throw(RuntimeException)
{
    // here we try to support both tree-fragment-local pathes and simple names (the latter ones are just an instance of the first)
    try
    {
        using internal::SearchExactName;
        using configuration::validateRelativePath;

        GuardedNodeDataAccess lock( rNode );

        Tree aTree(lock.getTree());
        NodeRef aNode(lock.getNode());

        RelativePath aApproximatePath = validateRelativePath(rApproximateName,aTree,aNode);

        SearchExactName aSearch(aApproximatePath);

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
OUString implGetExactName(NodeSetInfoAccess& rNode, const OUString& rApproximateName ) throw(RuntimeException)
{

    // here we can support only local names
    try
    {
        using internal::SearchExactName;
        using configuration::validateElementPathComponent;
        using configuration::Path::Component;

        GuardedNodeDataAccess lock( rNode );

        Tree aTree(lock.getTree());
        NodeRef aNode(lock.getNode());

        /*Path::*/Component aApproximateName = validateElementPathComponent(rApproximateName,aTree,aNode);

        SearchExactName aSearch(aApproximateName);

        aSearch.search(aNode, aTree);

        OSL_ENSURE( aSearch.getBestMatch().getDepth() == 1,
                    "Search for exact names changed number of path components !?");

        return aSearch.getBestMatch().getLocalName().getName().toString();
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

    using beans::Property;
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree    aTree( lock.getTree());
        NodeRef aNode( lock.getNode());

        Name        aName       = aTree.getName(aNode);
        Attributes  aAttributes = aTree.getAttributes(aNode);
        uno::Type   aApiType    = getUnoInterfaceType();

        return helperMakeProperty( aName,aAttributes,aApiType, aTree.hasNodeDefault(aNode) );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return Property();
}
// XPropertySetInfo
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
uno::Sequence< css::beans::Property > implGetProperties( NodeAccess& rNode ) throw (uno::RuntimeException)
{
    CollectPropertyInfo aCollect;

    try
    {
        GuardedNodeDataAccess lock( rNode );

        lock.getTree().dispatchToChildren(lock.getNode(), aCollect);
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
css::beans::Property implGetPropertyByName( NodeAccess& rNode, const OUString& aName )
    throw (css::beans::UnknownPropertyException, uno::RuntimeException)
{
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = configuration::validateChildOrElementName(aName,aTree,aNode);

        AnyNodeRef aChildNode = configuration::getChildOrElement(aTree,aNode, aChildName);

        if (!aChildNode.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Property not found by implementation");

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property. Property '") );
            sMessage += aName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw css::beans::UnknownPropertyException( sMessage, xContext );
        }

        Attributes  aChildAttributes = aTree.getAttributes(aChildNode);
        uno::Type   aApiType = aChildNode.isNode() ? getUnoInterfaceType() : aTree.getUnoType(aChildNode.toValue());

        return helperMakeProperty( aChildName,aChildAttributes,aApiType, aTree.hasNodeDefault(aChildNode) );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
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
sal_Bool implHasPropertyByName( NodeAccess& rNode, const OUString& Name ) throw (uno::RuntimeException)
{
    return implHasByName(rNode, Name);
}

// XNameAccess
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// TODO: optimization - cache the node found (for subsequent getByName)
// TODO: optimization - less locking for group nodes
//-----------------------------------------------------------------------------------
sal_Bool implHasByName(NodeAccess& rNode, const OUString& sName ) throw(RuntimeException)
{
    using namespace com::sun::star::container;
    using configuration::hasChildOrElement;
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Name aChildName = configuration::makeName(sName, Name::NoValidate());

        return hasChildOrElement(lock.getTree(), lock.getNode(), aChildName);
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
Any implGetByName(NodeAccess& rNode, const OUString& sName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, RuntimeException)
{
    using namespace com::sun::star::container;
    using configuration::getChildOrElement;
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree aTree( lock.getTree() );
        NodeRef aNode( lock.getNode() );

        Name aChildName = configuration::validateChildOrElementName(sName,aTree,aNode);

        AnyNodeRef aChildNode = getChildOrElement(aTree,aNode, aChildName);
        if (!aChildNode.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Child Element '") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return configapi::makeElement( rNode.getFactory(), aTree, aChildNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
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
// TODO: optimization - less locking for group nodes
//-----------------------------------------------------------------------------------
Sequence< OUString > implGetElementNames( NodeAccess& rNode ) throw( RuntimeException)
{
    CollectNodeNames aCollect;

    try
    {
        GuardedNodeDataAccess lock( rNode );

        lock.getTree().dispatchToChildren(lock.getNode(), aCollect);
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
sal_Bool implHasByHierarchicalName(NodeAccess& rNode, const OUString& sHierarchicalName ) throw(RuntimeException)
{
    using namespace com::sun::star::container;
    using configuration::validateAndReducePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::getDeepDescendant; // should actually be found by "Koenig" lookup, but MSVC6 fails
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree aTree( lock.getTree() );
        NodeRef aNode( lock.getNode() );

        RelativePath aRelPath = validateAndReducePath( sHierarchicalName, aTree, aNode );

        return getDeepDescendant(aTree, aNode, aRelPath).isValid();
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
Any implGetByHierarchicalName(NodeAccess& rNode, const OUString& sHierarchicalName )
    throw(css::container::NoSuchElementException, RuntimeException)
{
    // rtl::OUString aTmpStr(implGetHierarchicalName(rNode));
    // RTL_LOGFILE_CONTEXT_TRACE2(aLog, "Node: %s HierachicalName: %s",RTL_LOGFILE_OU2A(aTmpStr), RTL_LOGFILE_OU2A(sHierarchicalName));

    using namespace com::sun::star::container;
    using configuration::validateAndReducePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::getDeepDescendant; // should actually be found by "Koenig" lookup, but MSVC6 fails
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree aTree( lock.getTree() );
        NodeRef aNode( lock.getNode() );

        RelativePath aRelPath = validateAndReducePath( sHierarchicalName, aTree, aNode );

        AnyNodeRef aNestedNode = getDeepDescendant( aTree, aNode, aRelPath );
        if (!aNestedNode.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Descendant Element '") );
            sMessage += aRelPath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in Node ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        OSL_ASSERT(aNode.isValid());
        return configapi::makeElement( rNode.getFactory(), aTree, aNestedNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw NoSuchElementException( e.message(), xContext );
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

// XPropertyWithState
//---------------------------------------------------------------------
css::beans::PropertyState implGetStateAsProperty(NodeAccess& rNode)
    throw (uno::RuntimeException)
{
    using namespace css::beans;

    PropertyState aRet = PropertyState_AMBIGUOUS_VALUE;
    try
    {
        GuardedNodeDataAccess lock( rNode );

        if ( lock.getTree().isNodeDefault( lock.getNode() ) )
            aRet = PropertyState_DEFAULT_VALUE;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
    return aRet;
}

void implSetToDefaultAsProperty(NodeAccess& rNode)
    throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    GuardedNodeAccess lock( rNode );

    OUString const sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot set Property object to default: Object or View is read-only"));
    beans::PropertyVetoException aVeto(sMessage, rNode.getUnoInstance());

    OUString const sWrapMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Operation failed: "));
    throw lang::WrappedTargetException(sWrapMessage + sMessage, rNode.getUnoInstance(), uno::makeAny(aVeto));
}

Reference< uno::XInterface > implGetDefaultAsProperty(NodeAccess& rNode)
    throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    // not really supported

    /* possible, but nor really useful:
    GuardedNodeAccess lock( rNode );
    if (implGetStateAsProperty(rNode) == PropertyState_DEFAULT_VALUE)
        return rNode.getUnoInstance();
    */

    return Reference< uno::XInterface >();
}


// set-specific Interfaces
//-----------------------------------------------------------------------------------


// XTemplateContainer
//-----------------------------------------------------------------------------------
OUString SAL_CALL implGetElementTemplateName(NodeSetInfoAccess& rNode)
    throw(uno::RuntimeException)
{
    GuardedNodeData<NodeSetInfoAccess> lock(rNode);
    return rNode.getElementInfo(lock.getDataAccessor()).getTemplateInfo().getTemplatePathString();
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


