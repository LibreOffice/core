/*************************************************************************
 *
 *  $RCSfile: accessimpl.cxx,v $
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

#include "accessimpl.hxx"

#include "apinodeaccess.hxx"
#include "noderef.hxx"
#include "configset.hxx"
#include "confignotifier.hxx"
#include "encodename.hxx"
#include "propertyinfohelper.hxx"

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
        namespace util = css::util;

        using uno::RuntimeException;
        using uno::Reference;
        using uno::Any;
        using uno::Sequence;
        using lang::NoSupportException;

        using configuration::NodeRef;
        using configuration::NodeInfo;
        using configuration::Tree;
        using configuration::Name;
        using configuration::Path;
        using configuration::AbsolutePath;
        using configuration::RelativePath;

//-----------------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------------

// Interface methods
//-----------------------------------------------------------------------------------


// XHierarchicalName
//------------------------------------------------------------------------------------------------------------------
OUString implGetHierarchicalName( NodeAccess& rNode ) throw(RuntimeException)
{
    OUString sRet;
    try
    {
        GuardedNodeAccess impl( rNode );

        Tree aTree( impl->getTree());

        RelativePath const aLocalPath = aTree.getLocalPath(impl->getNode());
        AbsolutePath const aFullPath  = aTree.getContextPath().compose(aLocalPath);
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
OUString implComposeHierarchicalName(NodeAccess& rNode, const OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, NoSupportException, RuntimeException)
{
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails there
    OUString sRet;
    try
    {
        GuardedNodeAccess impl( rNode );
        NodeRef aNode( impl->getNode() );
        Tree aTree( impl->getTree() );

        RelativePath const aAddedPath = reduceRelativePath(sRelativeName, aTree, aNode);

        // TODO: add (relative) name validation based on node type - may then need provider lock
        RelativePath const aLocalPath = aTree.getLocalPath(aNode).compose(aAddedPath);
        AbsolutePath const aFullPath = aTree.getContextPath().compose(aLocalPath);

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
        GuardedNode<NodeSetInfoAccess> impl( rNode );

        aRet = impl->getElementInfo().getElementType();
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
        GuardedNodeAccess impl( rNode ); // no provider lock needed

        NodeRef aThisNode( impl->getNode() );
        OSL_ASSERT( !aThisNode.hasChildren() );
        return impl->getTree().hasChildren(aThisNode);
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
        GuardedNodeDataAccess impl( rNode ); // provider lock needed

        NodeRef aThisNode( impl->getNode() );
        OSL_ASSERT( !impl->getTree().hasChildren(aThisNode) );
        return aThisNode.hasChildren();
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
    using configuration::NodeVisitor;
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

        bool complete() const { return aSearchPath.end() == pSearchComponent; }

        bool search(NodeRef& aNode, Tree& aTree);

        OUString getBestMatch() const { return aSearchPath.toString(); }

    private:
        bool findMatch(NodeRef& aNode, Tree& aTree);
        virtual Result handle(NodeRef const&); // NodeVisitor
    };
//..................................................................................................................
    bool SearchExactName::findMatch(NodeRef& aNode, Tree& aTree)
    {
        OSL_ASSERT( aNode.isValid() );
        OSL_ASSERT( !complete() );

        using configuration::hasChildNode;
        using configuration::findChildNode;

        // exact match ?
        if (!hasChildNode(aTree,aNode,*pSearchComponent))
        {
            if (aTree.dispatchToChildren(aNode,*this) == CONTINUE) // not found there
                return false;
        }
        OSL_ASSERT(hasChildNode(aTree,aNode,*pSearchComponent));
        return findChildNode(aTree,aNode,*pSearchComponent);
    }
    //..................................................................................................................
    NodeVisitor::Result SearchExactName::handle(NodeRef const& aNode)
    {
        OSL_ASSERT( aNode.isValid() );
        OSL_ASSERT( !complete() );

        // find inexact match (the first one, but the order is unspecified)
        // TODO: Add support for node-type-specific element names
        Name aNodeName = aNode.getName();
        if (aNodeName.toString().equalsIgnoreCase(pSearchComponent->toString()))
        {
            *pSearchComponent = aNodeName;
            return DONE; // for this level
        }
        else
            return CONTINUE;
    }
//..................................................................................................................
    bool SearchExactName::search(NodeRef& rNode, Tree& rTree)
    {
        if (!rNode.isValid()) return false;

        while (!complete())
            if (! findMatch(rNode, rTree))
                break;

        return complete();
    }

} // namespace internal

//..................................................................................................................
OUString implGetExactName(NodeAccess& rNode, const OUString& rApproximateName ) throw(RuntimeException)
{
    try
    {
        using internal::SearchExactName;
        using configuration::reduceRelativePath;

        GuardedNodeDataAccess impl( rNode );

        Tree aTree(impl->getTree());
        NodeRef aNode(impl->getNode());

        // TODO: (?) handle absolute pathes' prefix part properly:
        RelativePath aApproximatePath = configuration::reduceRelativePath(rApproximateName,aTree,aNode);

        SearchExactName aSearch(aApproximatePath);

        aSearch.search(aNode, aTree);

        return aSearch.getBestMatch();
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

// XNameAccess
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
beans::Property implGetAsProperty(NodeAccess& rNode)
    throw(uno::RuntimeException)
{
    using beans::Property;
    try
    {
        GuardedNodeDataAccess impl( rNode );

        NodeRef aNode( impl->getNode());

        NodeInfo    aNodeInfo = aNode.getInfo();
        uno::Type   aApiType = aNode.getUnoType();

        return helperMakeProperty( aNodeInfo,aApiType );
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

// XNameAccess
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// TODO: optimization - cache the node found (for subsequent getByName)
// TODO: optimization - less locking for group nodes
//-----------------------------------------------------------------------------------
sal_Bool implHasByName(NodeAccess& rNode, const OUString& sName ) throw(RuntimeException)
{
    using namespace com::sun::star::container;
    using configuration::hasChildNode;
    try
    {
        GuardedNodeDataAccess impl( rNode );

        Name aChildName = configuration::makeName(sName);

        return hasChildNode(impl->getTree(), impl->getNode(), aChildName);
    }
#ifdef _DEBUG
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
    using configuration::findChildNode;
    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree aTree( impl->getTree() );
        NodeRef aNode( impl->getNode() );

        Name aChildName = configuration::validateNodeName(sName,aTree,aNode);

        if (!findChildNode(aTree,aNode, aChildName))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Child Element \"") );
            sMessage += sName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return configapi::makeElement( impl->getFactory(), aTree, aNode );
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
namespace internal
{
    class CollectNames :  public NodeVisitor
    {
    public:
        CollectNames() : aList() {}

        virtual Result handle(NodeRef const& aNode); // NodeVisitor

        typedef std::vector<OUString> NameList;
        NameList const& list() const { return aList; }

    private:
        void add(Name const& aName)
        {
            aList.push_back(aName.toString());
        }
        NameList aList;
    };

    NodeVisitor::Result CollectNames::handle(NodeRef const& aNode)
    {
        add(aNode.getName());
        return CONTINUE;
    }
}

//-----------------------------------------------------------------------------------
// TODO: optimization - less locking for group nodes
//-----------------------------------------------------------------------------------
Sequence< OUString > implGetElementNames( NodeAccess& rNode ) throw( RuntimeException)
{
    internal::CollectNames aCollect;

    try
    {
        GuardedNodeDataAccess impl( rNode );

        impl->getTree().dispatchToChildren(impl->getNode(), aCollect);
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
/*
// if NULL is returned, aPath may have a Partial path appended
NodeRef findDescendant(RelativePath const& aRelativePath, TreePath& aPath)
{
    checkInitialized();

    ConfigurationName aName( aRelativePath );
    if (!normalizeNestedPath(aName))
        return 0;

    ConfigurationName::Iterator it = aName.begin();
    ConfigurationName::Iterator const stop  = aName.end();

    for(TreeRef pTree = m_aSubtree; pTree != 0; )
    {
        NodeRef pFound = pTree->getChild(*it);
        aPath.push_back(pTree);

        if (++it == stop || !pFound)
            return pFound;

        pTree = pFound->asISubtree();
    }

    return 0;
}
*/

//-----------------------------------------------------------------------------------
// TO DO: optimization - cache the node found for subsequent getByHierarchicalName()
//-----------------------------------------------------------------------------------
sal_Bool implHasByHierarchicalName(NodeAccess& rNode, const OUString& sHierarchicalName ) throw(RuntimeException)
{
    using namespace com::sun::star::container;
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::findDescendantNode; // should actually be found by "Koenig" lookup, but MSVC6 fails
    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree aTree( impl->getTree() );
        NodeRef aNode( impl->getNode() );

        RelativePath aRelPath = reduceRelativePath( sHierarchicalName, aTree, aNode );

        return findDescendantNode(aTree, aNode, aRelPath);
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
    using namespace com::sun::star::container;
    using configuration::reduceRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::findDescendantNode; // should actually be found by "Koenig" lookup, but MSVC6 fails
    try
    {
        GuardedNodeDataAccess impl( rNode );

        Tree aTree( impl->getTree() );
        NodeRef aNode( impl->getNode() );

        RelativePath aRelPath = reduceRelativePath( sHierarchicalName, aTree, aNode );

        if (!findDescendantNode( aTree, aNode, aRelPath ))
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr: BasicAccess: Descendant Element \"") );
            sMessage += aRelPath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("\" not found in NodeRef ")  );
            sMessage += aTree.getLocalPath(aNode).toString();

            Reference<uno::XInterface> xContext( impl->getUnoInstance() );
            throw NoSuchElementException( sMessage, xContext );
        }

        OSL_ASSERT(aNode.isValid());
        return configapi::makeElement( impl->getFactory(), aTree, aNode );
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

// set-specific Interfaces
//-----------------------------------------------------------------------------------


// XTemplateContainer
//-----------------------------------------------------------------------------------
OUString SAL_CALL implGetElementTemplateName(NodeSetInfoAccess& rNode)
    throw(uno::RuntimeException)
{
    GuardedNode<NodeSetInfoAccess> impl(rNode);
    return impl->getElementInfo().getTemplatePath().toString();
}

// XStringEscape
//-----------------------------------------------------------------------------------
OUString SAL_CALL implEscapeString(NodeAccess& rNode, const OUString& aString)
    throw(css::lang::IllegalArgumentException, RuntimeException)
{
    OUString sRet;
    try
    {
        sRet = escaped_name::escapeString(aString,0,1);
    }
    catch (css::lang::IllegalArgumentException& ex)
    {
        ex.Context = rNode.getUnoInstance();
        throw;
    }
    catch (uno::Exception& ex)
    {
        throw RuntimeException(ex.Message, rNode.getUnoInstance());
    }
    return sRet;
}

OUString SAL_CALL implUnescapeString(NodeAccess& rNode, const OUString& aEscapedString)
    throw(css::lang::IllegalArgumentException, RuntimeException)
{
    OUString sRet;
    try
    {
        sRet = escaped_name::unescapeString(aEscapedString,0,1);
    }
    catch (css::lang::IllegalArgumentException& ex)
    {
        ex.Context = rNode.getUnoInstance();
        throw;
    }
    catch (uno::Exception& ex)
    {
        throw RuntimeException(ex.Message, rNode.getUnoInstance());
    }
    return sRet;
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


