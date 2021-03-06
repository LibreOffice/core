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

#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>

#include <vector>
#include <functional>

namespace com::sun::star::container { class XHierarchicalNameAccess; }
namespace com::sun::star::container { class XNameAccess; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::uno { class XComponentContext; }

namespace sd::tools {

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
        const OUString& rsRootName,
        const WriteMode eMode);

    ConfigurationAccess(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const OUString& rsRootName,
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
        const OUString& rsPathToNode);

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
        const OUString& rsPathToNode);

    /** Write any changes that have been made back to the configuration.
        This call is ignored when the called ConfigurationAccess object was
        not create with read-write mode.
    */
    void CommitChanges();

    /** This functor is typically called for every item in a set.  Its two
        parameters are the name of key item (often of no further interest)
        and the value of the item.
    */
    typedef ::std::function<void (
        const OUString&,
        const std::vector<css::uno::Any>&) > Functor;

    /** Execute a functor for all elements of the given container.
        @param rxContainer
            The container is a XNameAccess to a list of the configuration.
            This can be a node returned by GetConfigurationNode().
        @param rArguments
            The functor is called with arguments that are children of each
            element of the container.  The set of children is specified in this
            list.
        @param rFunctor
            The functor to be executed for some or all of the elements in
            the given container.
    */
    static void ForAll (
        const css::uno::Reference<css::container::XNameAccess>& rxContainer,
        const ::std::vector<OUString>& rArguments,
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
        const OUString& rsArgument,
        ::std::vector<OUString>& rList);

private:
    css::uno::Reference<css::uno::XInterface> mxRoot;

    void Initialize (
        const css::uno::Reference<css::lang::XMultiServiceFactory>& rxProvider,
        const OUString& rsRootName,
        const WriteMode eMode);
};

} // end of namespace sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
