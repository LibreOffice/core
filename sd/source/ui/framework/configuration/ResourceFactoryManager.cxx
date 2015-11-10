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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <boost/bind.hpp>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

#undef VERBOSE
//#define VERBOSE 1

namespace sd { namespace framework {

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
    Reference<lang::XComponent> xComponent (mxURLTransformer, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

void ResourceFactoryManager::AddFactory (
    const OUString& rsURL,
    const Reference<XResourceFactory>& rxFactory)
        throw (RuntimeException)
{
    if ( ! rxFactory.is())
        throw lang::IllegalArgumentException();
    if (rsURL.isEmpty())
        throw lang::IllegalArgumentException();

    ::osl::MutexGuard aGuard (maMutex);

    if (rsURL.indexOf('*') >= 0 || rsURL.indexOf('?') >= 0)
    {
        // The URL is a URL pattern not an single URL.
        maFactoryPatternList.push_back(FactoryPatternList::value_type(rsURL, rxFactory));

#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("ResourceFactoryManager::AddFactory pattern %s %x\n",
            OUStringToOString(rsURL, RTL_TEXTENCODING_UTF8).getStr(),
            rxFactory.get());
#endif
    }
    else
    {
        maFactoryMap[rsURL] = rxFactory;

#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("ResourceFactoryManager::AddFactory fixed %s %x\n",
            OUStringToOString(rsURL, RTL_TEXTENCODING_UTF8).getStr(),
            rxFactory.get());
#endif
    }
}

void ResourceFactoryManager::RemoveFactoryForURL (
    const OUString& rsURL)
    throw (RuntimeException)
{
    if (rsURL.isEmpty())
        throw lang::IllegalArgumentException();

    ::osl::MutexGuard aGuard (maMutex);

    FactoryMap::iterator iFactory (maFactoryMap.find(rsURL));
    if (iFactory != maFactoryMap.end())
    {
        maFactoryMap.erase(iFactory);
    }
    else
    {
        // The URL may be a pattern.  Look that up.
        FactoryPatternList::iterator iPattern;
        for (iPattern=maFactoryPatternList.begin();
             iPattern!=maFactoryPatternList.end();
             ++iPattern)
        {
            if (iPattern->first == rsURL)
            {
                // Found the pattern.  Remove it.
                maFactoryPatternList.erase(iPattern);
                break;
            }
        }
    }
}

void ResourceFactoryManager::RemoveFactoryForReference(
    const Reference<XResourceFactory>& rxFactory)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Collect a list with all keys that map to the given factory.
    ::std::vector<OUString> aKeys;
    FactoryMap::const_iterator iFactory;
    for (iFactory=maFactoryMap.begin(); iFactory!=maFactoryMap.end(); ++iFactory)
        if (iFactory->second == rxFactory)
            aKeys.push_back(iFactory->first);

    // Remove the entries whose keys we just have collected.
    ::std::vector<OUString>::const_iterator iKey;
    for (iKey=aKeys.begin(); iKey!=aKeys.end();  ++iKey)
        maFactoryMap.erase(maFactoryMap.find(*iKey));

    // Remove the pattern entries whose factories are identical to the given
    // factory.
    FactoryPatternList::iterator iNewEnd (
        std::remove_if(
            maFactoryPatternList.begin(),
            maFactoryPatternList.end(),
            ::boost::bind(
                std::equal_to<Reference<XResourceFactory> >(),
                ::boost::bind(&FactoryPatternList::value_type::second, _1),
                rxFactory)));
    if (iNewEnd != maFactoryPatternList.end())
        maFactoryPatternList.erase(iNewEnd, maFactoryPatternList.end());
}

Reference<XResourceFactory> ResourceFactoryManager::GetFactory (
    const OUString& rsCompleteURL)
    throw (RuntimeException)
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
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    FactoryMap::const_iterator iFactory (maFactoryMap.find(rsURLBase));
    if (iFactory != maFactoryMap.end())
        return iFactory->second;
    else
    {
        // Check the URL patterns.
        FactoryPatternList::const_iterator iPattern;
        for (iPattern=maFactoryPatternList.begin();
             iPattern!=maFactoryPatternList.end();
             ++iPattern)
        {
            WildCard aWildCard (iPattern->first);
            if (aWildCard.Matches(rsURLBase))
                return iPattern->second;
        }
    }
    return nullptr;
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
