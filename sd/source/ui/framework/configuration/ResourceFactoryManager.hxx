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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_RESOURCEFACTORYMANAGER_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_RESOURCEFACTORYMANAGER_HXX

#include <sal/config.h>

#include <unordered_map>
#include <utility>
#include <vector>

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XModuleController.hpp>
#include <com/sun/star/drawing/framework/XResourceFactoryManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <osl/mutex.hxx>

namespace sd { namespace framework {

/** Container of resource factories of the drawing framework.
*/
class ResourceFactoryManager
{
public:
    explicit ResourceFactoryManager (
        const css::uno::Reference<css::drawing::framework::XControllerManager>& rxManager);

    ~ResourceFactoryManager();

    /** Register a resource factory for one type of resource.
        @param rsURL
            The type of the resource that will be created by the factory.
        @param rxFactory
            The factory that will create resource objects of the specified type.
    */
    void AddFactory (
        const OUString& rsURL,
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxFactory)
        throw (css::uno::RuntimeException);

    /** Unregister the specified factory.
        @param rsURL
            Unregister only the factory for this URL.  When the same factory
            is registered for other URLs then these remain registered.
    */
    void RemoveFactoryForURL(
        const OUString& rsURL)
        throw (css::uno::RuntimeException);

    /** Unregister the specified factory.
        @param rxFactory
            Unregister the this factory for all URLs that it has been
            registered for.
    */
    void RemoveFactoryForReference(
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxFactory)
        throw (css::uno::RuntimeException);

    /** Return a factory that can create resources specified by the given URL.
        @param rsCompleteURL
            This URL specifies the type of the resource.  It may contain arguments.
        @return
            When a factory for the specified URL has been registered by a
            previous call to AddFactory() then a reference to that factory
            is returned.  Otherwise an empty reference is returned.
    */
    css::uno::Reference<css::drawing::framework::XResourceFactory> GetFactory (
        const OUString& rsURL)
        throw (css::uno::RuntimeException);

private:
    ::osl::Mutex maMutex;
    typedef std::unordered_map<
        OUString,
        css::uno::Reference<css::drawing::framework::XResourceFactory>,
        OUStringHash> FactoryMap;
    FactoryMap maFactoryMap;

    typedef ::std::vector<
        ::std::pair<
            OUString,
            css::uno::Reference<css::drawing::framework::XResourceFactory> > >
        FactoryPatternList;
    FactoryPatternList maFactoryPatternList;

    css::uno::Reference<css::drawing::framework::XControllerManager> mxControllerManager;
    css::uno::Reference<css::util::XURLTransformer> mxURLTransformer;

    /** Look up the factory for the given URL.
        @param rsURLBase
            The css::tools::URL.Main part of a URL. Arguments have to be
            stripped off by the caller.
        @return
            When the factory has not yet been added then return NULL.
    */
    css::uno::Reference<css::drawing::framework::XResourceFactory> FindFactory (
        const OUString& rsURLBase)
        throw (css::uno::RuntimeException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
