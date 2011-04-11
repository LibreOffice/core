/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SDEXT_PRESENTER_CONFIGURATION_ACCESS_HXX
#define SDEXT_PRESENTER_CONFIGURATION_ACCESS_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vector>
#include <boost/function.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** This class gives access to the configuration.  Create an object of this
    class for one node of the configuration.  This will be the root node.
    From this one you can use this class in two ways.

    <p>In a stateless mode (with exception of the root node) you can use static
    methods for obtaining child nodes, get values from properties at leaf
    nodes and iterate over children of inner nodes.</p>

    <p>In a stateful mode use non-static methods like GoToChild() to
    navigate to children.</p>

    <p>Note to call CommitChanges() after making changes to
    PresenterConfigurationAccess object that was opened in READ_WRITE mode.</p>
*/
class PresenterConfigurationAccess
{
public:
    enum WriteMode { READ_WRITE, READ_ONLY };
    typedef ::boost::function<bool(
        const ::rtl::OUString&,
        const css::uno::Reference<css::beans::XPropertySet>&)> Predicate;
    static const ::rtl::OUString msPresenterScreenRootName;

    /** Create a new object to access the configuration entries below the
        given root.
        @param rsRootName
            Name of the root.  You can use msPresenterScreenRootName to
            access the configuration of the presenter screen.
        @param eMode
            This flag specifies whether to give read-write or read-only
            access.
    */
    PresenterConfigurationAccess(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::OUString& rsRootName,
        WriteMode eMode);

    ~PresenterConfigurationAccess (void);

    /** Return a configuration node below the root of the called object.
        @param rsPathToNode
            The relative path from the root (as given the constructor) to the node.
    */
    css::uno::Any GetConfigurationNode (
        const ::rtl::OUString& rsPathToNode);
    css::uno::Reference<css::beans::XPropertySet> GetNodeProperties (
        const ::rtl::OUString& rsPathToNode);

    /** Return <TRUE/> when opening the configuration (via creating a new
        PresenterConfigurationAccess object) or previous calls to
        GoToChild() left the called PresenterConfigurationAccess object in a
        valid state.
    */
    bool IsValid (void) const;

    /** Move the focused node to the (possibly indirect) child specified by the given path.
    */
    bool GoToChild (const ::rtl::OUString& rsPathToNode);

    /** Move the focused node to the first direct child that fulfills the the given predicate.
    */
    bool GoToChild (const Predicate& rPredicate);

    /** Modify the property child of the currently focused node.  Keep in
        mind to call CommitChanges() to write the change back to the
        configuration.
    */
    bool SetProperty (const ::rtl::OUString& rsPropertyName, const css::uno::Any& rValue);

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

    static css::uno::Reference<css::beans::XPropertySet> GetNodeProperties (
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
    typedef ::boost::function<void(
        const ::rtl::OUString&,
        const ::css::uno::Reference<css::beans::XPropertySet>&) > PropertySetProcessor;

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
    static void ForAll (
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const PropertySetProcessor& rProcessor);

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

    static css::uno::Any Find (
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const Predicate& rPredicate);

    static bool IsStringPropertyEqual (
        const ::rtl::OUString& rsValue,
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Reference<css::beans::XPropertySet>& rxNode);

    /** This method wraps a call to getPropertyValue() and returns an empty
        Any instead of throwing an exception when the property does not
        exist.
    */
    static css::uno::Any GetProperty (
        const css::uno::Reference<css::beans::XPropertySet>& rxProperties,
        const ::rtl::OUString& rsKey);

private:
    css::uno::Reference<css::uno::XInterface> mxRoot;
    css::uno::Any maNode;
};

} } // end of namespace sdext::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
