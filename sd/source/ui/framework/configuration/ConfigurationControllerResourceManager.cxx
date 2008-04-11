/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigurationControllerResourceManager.cxx,v $
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

#include "ConfigurationControllerResourceManager.hxx"
#include "ConfigurationControllerBroadcaster.hxx"
#include "ResourceFactoryManager.hxx"
#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <algorithm>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#undef VERBOSE
//#define VERBOSE 1

namespace sd { namespace framework {

//===== ConfigurationControllerResourceManager ================================

ConfigurationControllerResourceManager::ConfigurationControllerResourceManager (
    const ::boost::shared_ptr<ResourceFactoryManager>& rpResourceFactoryContainer,
    const ::boost::shared_ptr<ConfigurationControllerBroadcaster>& rpBroadcaster)
    : maResourceMap(ResourceComparator()),
      mpResourceFactoryContainer(rpResourceFactoryContainer),
      mpBroadcaster(rpBroadcaster)
{
}




ConfigurationControllerResourceManager::~ConfigurationControllerResourceManager (void)
{
}




ConfigurationControllerResourceManager::ResourceDescriptor
    ConfigurationControllerResourceManager::GetResource (
        const Reference<XResourceId>& rxResourceId)
{
    ::osl::MutexGuard aGuard (maMutex);
    ResourceMap::const_iterator iResource (maResourceMap.find(rxResourceId));
    if (iResource != maResourceMap.end())
        return iResource->second;
    else
        return ResourceDescriptor();
}




void ConfigurationControllerResourceManager::ActivateResources (
    const ::std::vector<Reference<XResourceId> >& rResources,
    const Reference<XConfiguration>& rxConfiguration)
{
    ::osl::MutexGuard aGuard (maMutex);
    // Iterate in normal order over the resources that are to be
    // activated so that resources on which others depend are activated
    // beforet the depending resources are activated.
    ::std::for_each(
        rResources.begin(),
        rResources.end(),
        ::boost::bind(&ConfigurationControllerResourceManager::ActivateResource,
            this, _1, rxConfiguration));
}




void ConfigurationControllerResourceManager::DeactivateResources (
    const ::std::vector<Reference<XResourceId> >& rResources,
    const Reference<XConfiguration>& rxConfiguration)
{
    ::osl::MutexGuard aGuard (maMutex);
    // Iterate in reverese order over the resources that are to be
    // deactivated so that resources on which others depend are deactivated
    // only when the depending resources have already been deactivated.
    ::std::for_each(
        rResources.rbegin(),
        rResources.rend(),
        ::boost::bind(&ConfigurationControllerResourceManager::DeactivateResource,
            this, _1, rxConfiguration));
}




/* In this method we do following steps.
    1. Get the factory with which the resource will be created.
    2. Create the resource.
    3. Add the resource to the URL->Object map of the configuration
    controller.
    4. Add the resource id to the current configuration.
    5. Notify listeners.
*/
void ConfigurationControllerResourceManager::ActivateResource (
    const Reference<XResourceId>& rxResourceId,
    const Reference<XConfiguration>& rxConfiguration)
{
   if ( ! rxResourceId.is())
   {
       OSL_ASSERT(rxResourceId.is());
       return;
   }

#if defined VERBOSE && VERBOSE>=1
    OSL_TRACE("activating resource %s", OUStringToOString(
        FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif

    // 1. Get the factory.
    const OUString sResourceURL (rxResourceId->getResourceURL());
    Reference<XResourceFactory> xFactory (mpResourceFactoryContainer->GetFactory(sResourceURL));
    if ( ! xFactory.is())
    {
#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("    no factory found fo %s",
            OUStringToOString(sResourceURL, RTL_TEXTENCODING_UTF8).getStr());
#endif
        return;
    }

    try
    {
        // 2. Create the resource.
        Reference<XResource> xResource;
        try
        {
            xResource = xFactory->createResource(rxResourceId);
        }
        catch (lang::DisposedException&)
        {
            // The factory is disposed and can be removed from the list
            // of registered factories.
            mpResourceFactoryContainer->RemoveFactoryForReference(xFactory);
        }
        catch(Exception&)
        {
            OSL_ENSURE(false, "caught exception while activating resource");
        }

        if (xResource.is())
        {
#if defined VERBOSE && VERBOSE>=1
            OSL_TRACE("    successfully created");
#endif
            // 3. Add resource to URL->Object map.
            AddResource(xResource, xFactory);

            // 4. Add resource id to current configuration.
            rxConfiguration->addResource(rxResourceId);

            // 5. Notify the new resource to listeners of the ConfigurationController.
            mpBroadcaster->NotifyListeners(
                FrameworkHelper::msResourceActivationEvent,
                rxResourceId,
                xResource);
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}




/* In this method we do following steps.
    1. Remove the resource from the URL->Object map of the configuration
    controller.
    2. Notify listeners.
    3. Remove the resource id from the current configuration.
    4. Release the resource.
*/
void ConfigurationControllerResourceManager::DeactivateResource (
    const Reference<XResourceId>& rxResourceId,
    const Reference<XConfiguration>& rxConfiguration)
{
    if ( ! rxResourceId.is())
        return;

    bool bSuccess (false);
    try
    {
        // 1. Remove resource from URL->Object map.
        ResourceDescriptor aDescriptor (RemoveResource(rxResourceId));

        if (aDescriptor.mxResource.is() && aDescriptor.mxResourceFactory.is())
        {
            // 2.  Notifiy listeners that the resource is being deactivated.
            mpBroadcaster->NotifyListeners(
                FrameworkHelper::msResourceDeactivationEvent,
                rxResourceId,
                aDescriptor.mxResource);

            // 3. Remove resource id from current configuration.
            rxConfiguration->removeResource(rxResourceId);

            // 4. Release the resource.
            try
            {
                aDescriptor.mxResourceFactory->releaseResource(aDescriptor.mxResource);
            }
            catch (lang::DisposedException& rException)
            {
                if ( ! rException.Context.is()
                    || rException.Context == aDescriptor.mxResourceFactory)
                {
                    // The factory is disposed and can be removed from the
                    // list of registered factories.
                    mpResourceFactoryContainer->RemoveFactoryForReference(
                        aDescriptor.mxResourceFactory);
                }
            }

            bSuccess = true;
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }

#if defined VERBOSE && VERBOSE>=1
    if (bSuccess)
        OSL_TRACE("successfully deactivated %s", OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
    else
        OSL_TRACE("activating resource %s failed", OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif
}




void ConfigurationControllerResourceManager::AddResource (
    const Reference<XResource>& rxResource,
    const Reference<XResourceFactory>& rxFactory)
{
    if ( ! rxResource.is())
    {
        OSL_ASSERT(rxResource.is());
        return;
    }

    // Add the resource to the resource container.
    ResourceDescriptor aDescriptor;
    aDescriptor.mxResource = rxResource;
    aDescriptor.mxResourceFactory = rxFactory;
    maResourceMap[rxResource->getResourceId()] = aDescriptor;

#if defined VERBOSE && VERBOSE>=2
    OSL_TRACE("ConfigurationControllerResourceManager::AddResource(): added %s -> %x",
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResource->getResourceId()),
            RTL_TEXTENCODING_UTF8).getStr(),
        rxResource.get());
#endif
}




ConfigurationControllerResourceManager::ResourceDescriptor
    ConfigurationControllerResourceManager::RemoveResource (
        const Reference<XResourceId>& rxResourceId)
{
    ResourceDescriptor aDescriptor;

    ResourceMap::iterator iResource (maResourceMap.find(rxResourceId));
    if (iResource != maResourceMap.end())
    {
#if defined VERBOSE && VERBOSE>=2
        OSL_TRACE("ConfigurationControllerResourceManager::RemoveResource(): removing %s -> %x",
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId),
                RTL_TEXTENCODING_UTF8).getStr(),
            *iResource);
#endif

        aDescriptor = iResource->second;
        maResourceMap.erase(rxResourceId);
    }

    return aDescriptor;
}




//===== ConfigurationControllerResourceManager::ResourceComparator ============

bool ConfigurationControllerResourceManager::ResourceComparator::operator() (
    const Reference<XResourceId>& rxId1,
    const Reference<XResourceId>& rxId2) const
{
    if (rxId1.is() && rxId2.is())
        return rxId1->compareTo(rxId2)<0;
    else if (rxId1.is())
        return true;
    else if (rxId2.is())
        return false;
    else
        return false;
}




} } // end of namespace sd::framework

