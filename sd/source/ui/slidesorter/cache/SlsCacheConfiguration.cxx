/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    typedef ::boost::shared_ptr<CacheConfiguration> CacheConfigSharedPtr;
    class theInstance :
        public rtl::Static<CacheConfigSharedPtr, theInstance> {};
}

::boost::weak_ptr<CacheConfiguration> CacheConfiguration::mpWeakInstance;
Timer CacheConfiguration::maReleaseTimer;

::boost::shared_ptr<CacheConfiguration> CacheConfiguration::Instance (void)
{
    SolarMutexGuard aSolarGuard;
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    if (rInstancePtr.get() == NULL)
    {
        // Maybe somebody else kept a previously created instance alive.
        if ( ! mpWeakInstance.expired())
            rInstancePtr = ::boost::shared_ptr<CacheConfiguration>(mpWeakInstance);
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




CacheConfiguration::CacheConfiguration (void)
{
    // Get the cache size from configuration.
    const ::rtl::OUString sPathToImpressConfigurationRoot("/org.openoffice.Office.Impress/");
    const ::rtl::OUString sPathToNode("MultiPaneGUI/SlideSorter/PreviewCache");

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
        mxCacheNode = Reference<container::XNameAccess>(
            xHierarchy->getByHierarchicalName(sPathToNode),
            UNO_QUERY);
    }
    catch (RuntimeException &)
    {
    }
    catch (Exception &)
    {
    }
}




Any CacheConfiguration::GetValue (const ::rtl::OUString& rName)
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




IMPL_LINK_NOARG(CacheConfiguration, TimerCallback)
{
    CacheConfigSharedPtr &rInstancePtr = theInstance::get();
    // Release out reference to the instance.
    rInstancePtr.reset();
    return 0;
}


} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
