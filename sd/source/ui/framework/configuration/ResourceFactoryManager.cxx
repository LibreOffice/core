/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResourceFactoryManager.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "precompiled_sd.hxx"

#include "ResourceFactoryManager.hxx"
#include <tools/wldcrd.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <boost/bind.hpp>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sd { namespace framework {

ResourceFactoryManager::ResourceFactoryManager (const Reference<XControllerManager>& rxManager)
    : maMutex(),
      maFactoryMap(),
      maFactoryPatternList(),
      mxControllerManager(rxManager),
      mxURLTransformer()
{
    // Create the URL transformer.
    Reference<lang::XMultiServiceFactory> xServiceManager (
        ::comphelper::getProcessServiceFactory());
    mxURLTransformer = Reference<util::XURLTransformer>(
        xServiceManager->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
        UNO_QUERY);
}




ResourceFactoryManager::~ResourceFactoryManager (void)
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
    if (rsURL.getLength() == 0)
        throw lang::IllegalArgumentException();

    ::osl::MutexGuard aGuard (maMutex);

    if (rsURL.indexOf('*') >= 0 || rsURL.indexOf('?') >= 0)
    {
        // The URL is a URL pattern not an single URL.
        maFactoryPatternList.push_back(FactoryPatternList::value_type(rsURL, rxFactory));
    }
    else
    {
        maFactoryMap[rsURL] = rxFactory;
    }
}




void ResourceFactoryManager::RemoveFactoryForURL (
    const OUString& rsURL)
    throw (RuntimeException)
{
    if (rsURL.getLength() == 0)
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
    return NULL;
}

} } // end of namespace sd::framework
