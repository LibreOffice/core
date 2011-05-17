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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SlsCacheConfiguration.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace slidesorter { namespace cache {

::boost::shared_ptr<CacheConfiguration> CacheConfiguration::mpInstance;
::boost::weak_ptr<CacheConfiguration> CacheConfiguration::mpWeakInstance;
Timer CacheConfiguration::maReleaseTimer;



::boost::shared_ptr<CacheConfiguration> CacheConfiguration::Instance (void)
{
    SolarMutexGuard aSolarGuard;
    if (mpInstance.get() == NULL)
    {
        // Maybe somebody else kept a previously created instance alive.
        if ( ! mpWeakInstance.expired())
            mpInstance = ::boost::shared_ptr<CacheConfiguration>(mpWeakInstance);
        if (mpInstance.get() == NULL)
        {
            // We have to create a new instance.
            mpInstance.reset(new CacheConfiguration());
            mpWeakInstance = mpInstance;
            // Prepare to release this instance in the near future.
            maReleaseTimer.SetTimeoutHdl(
                LINK(mpInstance.get(),CacheConfiguration,TimerCallback));
            maReleaseTimer.SetTimeout(5000 /* 5s */);
            maReleaseTimer.Start();
        }
    }
    return mpInstance;
}




CacheConfiguration::CacheConfiguration (void)
{
    // Get the cache size from configuration.
    const ::rtl::OUString sConfigurationProviderServiceName(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.ConfigurationProvider"));
    const ::rtl::OUString sPathToImpressConfigurationRoot(
        RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Impress/"));
    const ::rtl::OUString sPathToNode(
        RTL_CONSTASCII_USTRINGPARAM(
            "MultiPaneGUI/SlideSorter/PreviewCache"));

    try
    {
        // Obtain access to the configuration.
        Reference<lang::XMultiServiceFactory> xProvider (
            ::comphelper::getProcessServiceFactory()->createInstance(
                sConfigurationProviderServiceName),
            UNO_QUERY);
        if ( ! xProvider.is())
            return;

        // Obtain access to Impress configuration.
        Sequence<Any> aCreationArguments(3);
        aCreationArguments[0] = makeAny(beans::PropertyValue(
            ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("nodepath")),
            0,
            makeAny(sPathToImpressConfigurationRoot),
            beans::PropertyState_DIRECT_VALUE));
        aCreationArguments[1] = makeAny(beans::PropertyValue(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("depth")),
            0,
            makeAny((sal_Int32)-1),
            beans::PropertyState_DIRECT_VALUE));
        aCreationArguments[2] = makeAny(beans::PropertyValue(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("lazywrite")),
            0,
            makeAny(true),
            beans::PropertyState_DIRECT_VALUE));
        ::rtl::OUString sAccessService (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.ConfigurationAccess")));
        Reference<XInterface> xRoot (xProvider->createInstanceWithArguments(
            sAccessService, aCreationArguments));
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
    catch (RuntimeException &aException)
    {
        (void)aException;
    }
    catch (Exception &aException)
    {
        (void)aException;
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
        catch (Exception &aException)
        {
            (void)aException;
        }
    }

    return aResult;
}




IMPL_LINK(CacheConfiguration,TimerCallback, Timer*,EMPTYARG)
{
    // Release out reference to the instance.
    mpInstance.reset();
    return 0;
}


} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
