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

#include "ResourceFactoryManager.hxx"
#include <tools/wldcrd.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

#undef VERBOSE
//#define VERBOSE 1

namespace sd::framework {

ResourceFactoryManager::ResourceFactoryManager (const Reference<XControllerManager>& rxManager)
    : maMutex(),
      maFactoryMap(),
      maFactoryPatternList(),
      mxControllerManager(rxManager),
      mxURLTransformer()
{
    // Create the URL transformer.
    Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());
    mxURLTransformer = util::URLTransformer::create(xContext);
}

ResourceFactoryManager::~ResourceFactoryManager()
{
    for (auto& rXInterfaceResource : maFactoryMap)
    {
        Reference<lang::XComponent> xComponent (rXInterfaceResource.second, UNO_QUERY);
        rXInterfaceResource.second = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    Reference<lang::XComponent> xComponent (mxURLTransformer, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

void ResourceFactoryManager::AddFactory (
    const OUString& rsURL,
    const Reference<XResourceFactory>& rxFactory)
{
    if ( ! rxFactory.is())
        throw lang::IllegalArgumentException();
    if (rsURL.isEmpty())
        throw lang::IllegalArgumentException();

    std::lock_guard aGuard (maMutex);

    if (rsURL.indexOf('*') >= 0 || rsURL.indexOf('?') >= 0)
    {
        // The URL is a URL pattern not a single URL.
        maFactoryPatternList.emplace_back(rsURL, rxFactory);

#if defined VERBOSE && VERBOSE>=1
        SAL_INFO("sd","ResourceFactoryManager::AddFactory pattern " << rsURL << std::hex << rxFactory.get());
#endif
    }
    else
    {
        maFactoryMap[rsURL] = rxFactory;

#if defined VERBOSE && VERBOSE>=1
        SAL_INFO("sd", "ResourceFactoryManager::AddFactory fixed " << rsURL << " 0x" << std::hex << rxFactory.get());
#endif
    }
}

void ResourceFactoryManager::RemoveFactoryForURL (
    const OUString& rsURL)
{
    if (rsURL.isEmpty())
        throw lang::IllegalArgumentException();

    std::lock_guard aGuard (maMutex);

    FactoryMap::iterator iFactory (maFactoryMap.find(rsURL));
    if (iFactory != maFactoryMap.end())
    {
        maFactoryMap.erase(iFactory);
    }
    else
    {
        // The URL may be a pattern.  Look that up.
        auto iPattern = std::find_if(maFactoryPatternList.begin(), maFactoryPatternList.end(),
            [&rsURL](const FactoryPatternList::value_type& rPattern) { return rPattern.first == rsURL; });
        if (iPattern != maFactoryPatternList.end())
        {
            // Found the pattern.  Remove it.
            maFactoryPatternList.erase(iPattern);
        }
    }
}

void ResourceFactoryManager::RemoveFactoryForReference(
    const Reference<XResourceFactory>& rxFactory)
{
    std::lock_guard aGuard (maMutex);

    // Collect a list with all keys that map to the given factory.
    ::std::vector<OUString> aKeys;
    for (const auto& rFactory : maFactoryMap)
        if (rFactory.second == rxFactory)
            aKeys.push_back(rFactory.first);

    // Remove the entries whose keys we just have collected.
    for (const auto& rKey : aKeys)
        maFactoryMap.erase(rKey);

    // Remove the pattern entries whose factories are identical to the given
    // factory.
    maFactoryPatternList.erase(
        std::remove_if(
            maFactoryPatternList.begin(),
            maFactoryPatternList.end(),
            [&] (FactoryPatternList::value_type const& it) { return it.second == rxFactory; }),
        maFactoryPatternList.end());
}

Reference<XResourceFactory> ResourceFactoryManager::GetFactory (
    const OUString& rsCompleteURL)
{
    OUString sURLBase (rsCompleteURL);
    if (mxURLTransformer.is())
    {
        util::URL aURL;
        aURL.Complete = rsCompleteURL;
        if (mxURLTransformer->parseStrict(aURL))
            sURLBase = aURL.Main;
    }

    Reference<XResourceFactory> xFactory = FindFactory(sURLBase);

    if ( ! xFactory.is() && mxControllerManager.is())
    {
        Reference<XModuleController> xModuleController(mxControllerManager->getModuleController());
        if (xModuleController.is())
        {
            // Ask the module controller to provide a factory of the
            // requested view type.  Note that this can (and should) cause
            // intermediate calls to AddFactory().
            xModuleController->requestResource(sURLBase);

            xFactory = FindFactory(sURLBase);
        }
    }

    return xFactory;
}

Reference<XResourceFactory> ResourceFactoryManager::FindFactory (const OUString& rsURLBase)
{
    std::lock_guard aGuard (maMutex);
    FactoryMap::const_iterator iFactory (maFactoryMap.find(rsURLBase));
    if (iFactory != maFactoryMap.end())
        return iFactory->second;
    else
    {
        // Check the URL patterns.
        auto iPattern = std::find_if(maFactoryPatternList.begin(), maFactoryPatternList.end(),
            [&rsURLBase](const FactoryPatternList::value_type& rPattern) {
                WildCard aWildCard (rPattern.first);
                return aWildCard.Matches(rsURLBase);
            });
        if (iPattern != maFactoryPatternList.end())
            return iPattern->second;
    }
    return nullptr;
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
