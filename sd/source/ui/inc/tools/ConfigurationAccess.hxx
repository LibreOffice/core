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
namespace com::sun::star::lang { class XMultiServiceFactory; }

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

private:
    css::uno::Reference<css::uno::XInterface> mxRoot;

    void Initialize (
        const css::uno::Reference<css::lang::XMultiServiceFactory>& rxProvider,
        const OUString& rsRootName,
        const WriteMode eMode);
};

} // end of namespace sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
