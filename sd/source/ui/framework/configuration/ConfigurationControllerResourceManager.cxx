/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "ConfigurationControllerResourceManager.hxx"
#include "ConfigurationControllerBroadcaster.hxx"
#include "ResourceFactoryManager.hxx"
#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <tools/diagnose_ex.h>
#include <algorithm>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {



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

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": activating resource " << OUStringToOString(
        FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());

    
    const OUString sResourceURL (rxResourceId->getResourceURL());
    Reference<XResourceFactory> xFactory (mpResourceFactoryContainer->GetFactory(sResourceURL));
    if ( ! xFactory.is())
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    no factory found for " <<
            OUStringToOString(sResourceURL, RTL_TEXTENCODING_UTF8).getStr());
        return;
    }

    try
    {
        
        Reference<XResource> xResource;
        try
        {
            xResource = xFactory->createResource(rxResourceId);
        }
        catch (lang::DisposedException&)
        {
            
            
            mpResourceFactoryContainer->RemoveFactoryForReference(xFactory);
        }
        catch (Exception& e)
        {
            (void)e;
        }

        if (xResource.is())
        {
            SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    successfully created");
            
            AddResource(xResource, xFactory);

            
            rxConfiguration->addResource(rxResourceId);

            
            mpBroadcaster->NotifyListeners(
                FrameworkHelper::msResourceActivationEvent,
                rxResourceId,
                xResource);
        }
        else
        {
            SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    resource creation failed");
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




/* In this method we do following steps.
    1. Remove the resource from the URL->Object map of the configuration
    controller.
    2. Notify listeners that deactivation has started.
    3. Remove the resource id from the current configuration.
    4. Release the resource.
    5. Notify listeners about that deactivation is completed.
*/
void ConfigurationControllerResourceManager::DeactivateResource (
    const Reference<XResourceId>& rxResourceId,
    const Reference<XConfiguration>& rxConfiguration)
{
    if ( ! rxResourceId.is())
        return;

#if OSL_DEBUG_LEVEL >= 1
    bool bSuccess (false);
#endif
    try
    {
        
        ResourceDescriptor aDescriptor (RemoveResource(rxResourceId));

        if (aDescriptor.mxResource.is() && aDescriptor.mxResourceFactory.is())
        {
            
            mpBroadcaster->NotifyListeners(
                FrameworkHelper::msResourceDeactivationEvent,
                rxResourceId,
                aDescriptor.mxResource);

            
            rxConfiguration->removeResource(rxResourceId);

            
            try
            {
                aDescriptor.mxResourceFactory->releaseResource(aDescriptor.mxResource);
            }
            catch (const lang::DisposedException& rException)
            {
                if ( ! rException.Context.is()
                    || rException.Context == aDescriptor.mxResourceFactory)
                {
                    
                    
                    mpResourceFactoryContainer->RemoveFactoryForReference(
                        aDescriptor.mxResourceFactory);
                }
            }

#if OSL_DEBUG_LEVEL >= 1
            bSuccess = true;
#endif
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    
    mpBroadcaster->NotifyListeners(
        FrameworkHelper::msResourceDeactivationEndEvent,
        rxResourceId,
        NULL);

#if OSL_DEBUG_LEVEL >= 1
    if (bSuccess)
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": successfully deactivated " << OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
    else
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": activating resource " << OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr()
            << " failed");
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

    
    ResourceDescriptor aDescriptor;
    aDescriptor.mxResource = rxResource;
    aDescriptor.mxResourceFactory = rxFactory;
    maResourceMap[rxResource->getResourceId()] = aDescriptor;

#if OSL_DEBUG_LEVEL >= 2
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationControllerResourceManager::AddResource(): added " <<
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResource->getResourceId()),
            RTL_TEXTENCODING_UTF8).getStr() << " -> " << rxResource.get());
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
#if OSL_DEBUG_LEVEL >= 2
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationControllerResourceManager::RemoveResource(): removing " <<
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId),
                RTL_TEXTENCODING_UTF8).getStr() <<
                " -> " << &iResource);
#endif

        aDescriptor = iResource->second;
        maResourceMap.erase(rxResourceId);
    }

    return aDescriptor;
}






bool ConfigurationControllerResourceManager::ResourceComparator::operator() (
    const Reference<XResourceId>& rxId1,
    const Reference<XResourceId>& rxId2) const
{
    if (rxId1.is() && rxId2.is())
        return rxId1->compareTo(rxId2)<0;
    else if (rxId1.is())
        return true;
    else
        return false;
}




} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
