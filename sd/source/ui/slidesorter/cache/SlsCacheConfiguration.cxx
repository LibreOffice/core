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
#include <rtl/instance.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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

std::shared_ptr<CacheConfiguration> CacheConfiguration::Instance()
{
    SolarMutexGuard aSolarGuard;
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    if (rInstancePtr.get() == nullptr)
    {
        // Maybe somebody else kept a previously created instance alive.
        if ( ! mpWeakInstance.expired())
            rInstancePtr = std::shared_ptr<CacheConfiguration>(mpWeakInstance);
        if (rInstancePtr.get() == nullptr)
        {
            // We have to create a new instance.
            rInstancePtr.reset(new CacheConfiguration());
            mpWeakInstance = rInstancePtr;
            // Prepare to release this instance in the near future.
            rInstancePtr->m_ReleaseTimer.SetInvokeHandler(
                LINK(rInstancePtr.get(),CacheConfiguration,TimerCallback));
            rInstancePtr->m_ReleaseTimer.SetTimeout(5000 /* 5s */);
            rInstancePtr->m_ReleaseTimer.SetDebugName("sd::CacheConfiguration maReleaseTimer");
            rInstancePtr->m_ReleaseTimer.Start();
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
        Sequence<Any> aCreationArguments(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", makeAny(sPathToImpressConfigurationRoot)},
            {"depth", makeAny(sal_Int32(-1))}
        }));

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

    if (mxCacheNode != nullptr)
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

IMPL_STATIC_LINK_NOARG(CacheConfiguration, TimerCallback, Timer *, void)
{
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    // Release our reference to the instance.
    rInstancePtr.reset();
    // note: if there are no other references to the instance, m_ReleaseTimer
    // will be deleted now
}

void CacheConfiguration::Shutdown()
{
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    rInstancePtr.reset();
    assert(mpWeakInstance.expired()); // ensure m_ReleaseTimer is destroyed
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
