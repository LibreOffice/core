/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SlsCacheConfiguration.hxx"
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#ifndef _COM_SUN_STAR_CONTAINER_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace slidesorter { namespace cache {

::boost::shared_ptr<CacheConfiguration> CacheConfiguration::mpInstance;
::boost::weak_ptr<CacheConfiguration> CacheConfiguration::mpWeakInstance;
Timer CacheConfiguration::maReleaseTimer;



::boost::shared_ptr<CacheConfiguration> CacheConfiguration::Instance (void)
{
    ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
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
        do
        {
            // Obtain access to the configuration.
            Reference<lang::XMultiServiceFactory> xProvider (
                ::comphelper::getProcessServiceFactory()->createInstance(
                    sConfigurationProviderServiceName),
                UNO_QUERY);
            if ( ! xProvider.is())
                break;

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
                break;
            Reference<container::XHierarchicalNameAccess> xHierarchy (xRoot, UNO_QUERY);
            if ( ! xHierarchy.is())
                break;

            // Get the node for the slide sorter preview cache.
            mxCacheNode = Reference<container::XNameAccess>(
                xHierarchy->getByHierarchicalName(sPathToNode),
                UNO_QUERY);
        }
        while (false);
    }
    catch (RuntimeException aException)
    {
        (void)aException;
    }
    catch (Exception aException)
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
        catch (Exception aException)
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
