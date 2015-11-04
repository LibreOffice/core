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

#include "SlsCacheConfiguration.hxx"
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace slidesorter { namespace cache {

namespace
{
    typedef std::shared_ptr<CacheConfiguration> CacheConfigSharedPtr;
    class theInstance :
        public rtl::Static<CacheConfigSharedPtr, theInstance> {};
}

std::weak_ptr<CacheConfiguration> CacheConfiguration::mpWeakInstance;
Timer CacheConfiguration::maReleaseTimer;

std::shared_ptr<CacheConfiguration> CacheConfiguration::Instance()
{
    SolarMutexGuard aSolarGuard;
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    if (rInstancePtr.get() == NULL)
    {
        // Maybe somebody else kept a previously created instance alive.
        if ( ! mpWeakInstance.expired())
            rInstancePtr = std::shared_ptr<CacheConfiguration>(mpWeakInstance);
        if (rInstancePtr.get() == NULL)
        {
            // We have to create a new instance.
            rInstancePtr.reset(new CacheConfiguration());
            mpWeakInstance = rInstancePtr;
            // Prepare to release this instance in the near future.
            maReleaseTimer.SetTimeoutHdl(
                LINK(rInstancePtr.get(),CacheConfiguration,TimerCallback));
            maReleaseTimer.SetTimeout(5000 /* 5s */);
            maReleaseTimer.Start();
        }
    }
    return rInstancePtr;
}

CacheConfiguration::CacheConfiguration()
{
    // Get the cache size from configuration.
    const OUString sPathToImpressConfigurationRoot("/org.openoffice.Office.Impress/");
    const OUString sPathToNode("MultiPaneGUI/SlideSorter/PreviewCache");

    try
    {
        // Obtain access to the configuration.
        Reference<lang::XMultiServiceFactory> xProvider =
            configuration::theDefaultProvider::get( ::comphelper::getProcessComponentContext() );

        // Obtain access to Impress configuration.
        Sequence<Any> aCreationArguments(3);
        aCreationArguments[0] = makeAny(beans::PropertyValue(
            "nodepath",
            0,
            makeAny(sPathToImpressConfigurationRoot),
            beans::PropertyState_DIRECT_VALUE));
        aCreationArguments[1] = makeAny(beans::PropertyValue(
            "depth",
            0,
            makeAny((sal_Int32)-1),
            beans::PropertyState_DIRECT_VALUE));
        aCreationArguments[2] = makeAny(beans::PropertyValue(
            "lazywrite",
            0,
            makeAny(true),
            beans::PropertyState_DIRECT_VALUE));

        Reference<XInterface> xRoot (xProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess",
            aCreationArguments));
        if ( ! xRoot.is())
            return;
        Reference<container::XHierarchicalNameAccess> xHierarchy (xRoot, UNO_QUERY);
        if ( ! xHierarchy.is())
            return;

        // Get the node for the slide sorter preview cache.
        mxCacheNode.set( xHierarchy->getByHierarchicalName(sPathToNode), UNO_QUERY);
    }
    catch (RuntimeException &)
    {
    }
    catch (Exception &)
    {
    }
}

Any CacheConfiguration::GetValue (const OUString& rName)
{
    Any aResult;

    if (mxCacheNode != NULL)
    {
        try
        {
            aResult = mxCacheNode->getByName(rName);
        }
        catch (Exception &)
        {
        }
    }

    return aResult;
}

IMPL_STATIC_LINK_NOARG_TYPED(CacheConfiguration, TimerCallback, Timer *, void)
{
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    // Release our reference to the instance.
    rInstancePtr.reset();
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
