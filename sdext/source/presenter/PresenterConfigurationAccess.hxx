/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterConfigurationAccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:56:52 $
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

#ifndef SDEXT_PRESENTER_CONFIGURATION_ACCESS_HXX
#define SDEXT_PRESENTER_CONFIGURATION_ACCESS_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vector>
#include <boost/function.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** This class gives access to the configuration.  Create an object of this
    class for one node of the configuration.  This will be the root node.
    Its children (on all levels) are then accessible through this new object.
*/
class PresenterConfigurationAccess
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
    PresenterConfigurationAccess(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::OUString& rsRootName,
        WriteMode eMode);

    /** Return a configuration node below the root of the called object.
        @param rsPathToNode
            The relative path from the root (as given the constructor) to the node.
    */
    css::uno::Any GetConfigurationNode (
        const ::rtl::OUString& rsPathToNode);

    /** Return a configuration node below the given node.
        @param rxNode
            The node that acts as root to the given relative path.
        @param rsPathToNode
            The relative path from the given node to the requested node.
            When this string is empty then rxNode is returned.
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

    css::uno::Any GetValue (const rtl::OUString& sKey);

    typedef ::boost::function<void(
        const ::rtl::OUString&,
        const ::std::vector<css::uno::Any>&) > ItemProcessor;

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
        const ItemProcessor& rProcessor);

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
    static void PresenterConfigurationAccess::FillList(
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const ::rtl::OUString& rsArgument,
        ::std::vector<rtl::OUString>& rList);

    typedef ::boost::function<bool(
        const ::rtl::OUString&,
        const css::uno::Reference<css::container::XNameAccess>&)> Predicate;

    static css::uno::Reference<css::container::XNameAccess> Find (
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const Predicate& rPredicate);

private:
    css::uno::Reference<css::uno::XInterface> mxRoot;
};

} } // end of namespace sdext::tools

#endif
