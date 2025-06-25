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

#include <sal/config.h>

#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star::util { class XURLTransformer; }
namespace sd { class DrawController; }

namespace sd::framework {
class ResourceFactory;

/** Container of resource factories of the drawing framework.
*/
class ResourceFactoryManager
{
public:
    explicit ResourceFactoryManager(const rtl::Reference<::sd::DrawController>& rxManager);

    ~ResourceFactoryManager();

    /** Register a resource factory for one type of resource.
        @param rsURL
            The type of the resource that will be created by the factory.
        @param rxFactory
            The factory that will create resource objects of the specified type.
        @throws css::uno::RuntimeException
    */
    void AddFactory (
        const OUString& rsURL,
        const rtl::Reference<sd::framework::ResourceFactory>& rxFactory);

    /** Unregister the specified factory.
        @param rxFactory
            Unregister the this factory for all URLs that it has been
            registered for.
        @throws css::uno::RuntimeException
    */
    void RemoveFactoryForReference(
        const rtl::Reference<sd::framework::ResourceFactory>& rxFactory);

    /** Return a factory that can create resources specified by the given URL.
        @param rsCompleteURL
            This URL specifies the type of the resource.  It may contain arguments.
        @return
            When a factory for the specified URL has been registered by a
            previous call to AddFactory() then a reference to that factory
            is returned.  Otherwise an empty reference is returned.
        @throws css::uno::RuntimeException
    */
    rtl::Reference<sd::framework::ResourceFactory> GetFactory (
        const OUString& rsURL);

private:
    std::mutex maMutex;
    typedef std::unordered_map<
        OUString,
        rtl::Reference<sd::framework::ResourceFactory> > FactoryMap;
    FactoryMap maFactoryMap;

    typedef ::std::vector<
        ::std::pair<
            OUString,
            rtl::Reference<sd::framework::ResourceFactory> > >
        FactoryPatternList;
    FactoryPatternList maFactoryPatternList;

    rtl::Reference<::sd::DrawController> mxControllerManager;
    css::uno::Reference<css::util::XURLTransformer> mxURLTransformer;

    /** Look up the factory for the given URL.
        @param rsURLBase
            The css::tools::URL.Main part of a URL. Arguments have to be
            stripped off by the caller.
        @return
            When the factory has not yet been added then return NULL.
        @throws css::uno::RuntimeException
    */
    rtl::Reference<sd::framework::ResourceFactory> FindFactory (
        const OUString& rsURLBase);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
