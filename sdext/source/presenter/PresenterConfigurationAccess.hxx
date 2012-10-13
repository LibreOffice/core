/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
