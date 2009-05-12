/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigurationAccess.hxx,v $
 * $Revision: 1.5 $
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

#ifndef SD_TOOLS_CONFIGURATION_ACCESS_HXX
#define SD_TOOLS_CONFIGURATION_ACCESS_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vector>
#include <boost/function.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace tools {

/** This class gives access to the configuration.  Create an object of this
    class for one node of the configuration.  This will be the root node.
    Its children are then accessible through the new ConfigurationAccess
    object.
*/
class ConfigurationAccess
{
public:
    enum WriteMode { READ_WRITE, READ_ONLY };

    /** Create a new object to access the configuration entries below the
        given root.
        @param rsRootName
            Name of the root.
        @param eMode
            This flag specifies whether to give read-write or read-only
            access.
    */
    ConfigurationAccess(
        const ::rtl::OUString& rsRootName,
        const WriteMode eMode);

    ConfigurationAccess(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::OUString& rsRootName,
        const WriteMode eMode);

    /** Return a configuration node below the root of the called object.
        @param rsPathToNode
            The relative path from the root (as given the constructor) to
            the node.
        @return
            The type of the returned node varies with the requested node.
            It is empty when the node was not found.
    */
    css::uno::Any GetConfigurationNode (
        const ::rtl::OUString& rsPathToNode);

    /** Return a configuration node below the given node.
        @param rxNode
            The node that acts as root to the given relative path.
        @param rsPathToNode
            The relative path from the given node to the requested node.
        @return
            The type of the returned node varies with the requested node.
            It is empty when the node was not found.
    */
    static css::uno::Any GetConfigurationNode (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
        const ::rtl::OUString& rsPathToNode);

    /** Write any changes that have been made back to the configuration.
        This call is ignored when the called ConfigurationAccess object was
        not create with read-write mode.
    */
    void CommitChanges (void);

    /** This functor is typically called for every item in a set.  Its two
        parameters are the name of key item (often of no further interest)
        and the value of the item.
    */
    typedef ::boost::function<void(
        const ::rtl::OUString&,
        const std::vector<css::uno::Any>&) > Functor;

    /** Execute a functor for all elements of the given container.
        @param rxContainer
            The container is a XNameAccess to a list of the configuration.
            This can be a node returned by GetConfigurationNode().
        @param rArguments
            The functor is called with arguments that are children of each
            element of the container.  The set of children is specified  this
            list.
        @param rFunctor
            The functor to be executed for some or all of the elements in
            the given container.
    */
    static void ForAll (
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const ::std::vector<rtl::OUString>& rArguments,
        const Functor& rFunctor);

    /** Fill a list with the string contents of all sub-elements in the given container.
        @param rxContainer
            The container is a XNameAccess to a list of the configuration.
            This can be a node returned by GetConfigurationNode().
        @param rsArgument
            This specifies which string children of the elements in the
            container are to be inserted into the list.  The specified child
            has to be of type string.
        @param rList
            The list to be filled.
    */
    static void FillList(
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const ::rtl::OUString& rsArgument,
        ::std::vector<rtl::OUString>& rList);

private:
    css::uno::Reference<css::uno::XInterface> mxRoot;

    void Initialize (
        const css::uno::Reference<css::lang::XMultiServiceFactory>& rxProvider,
        const ::rtl::OUString& rsRootName,
        const WriteMode eMode);
};

} } // end of namespace sd::tools

#endif
