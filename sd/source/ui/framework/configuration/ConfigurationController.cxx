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

#include "framework/ConfigurationController.hxx"
#include "framework/Configuration.hxx"
#include "framework/FrameworkHelper.hxx"
#include "ConfigurationUpdater.hxx"
#include "ConfigurationControllerBroadcaster.hxx"
#include "ConfigurationTracer.hxx"
#include "GenericConfigurationChangeRequest.hxx"
#include "ResourceFactoryManager.hxx"
#include "UpdateRequest.hxx"
#include "ChangeRequestQueueProcessor.hxx"
#include "ConfigurationClassifier.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

namespace sd { namespace framework {

Reference<XInterface> SAL_CALL ConfigurationController_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    (void)rxContext;
    return static_cast<XWeak*>(new ConfigurationController());
}




OUString ConfigurationController_getImplementationName (void) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.framework.configuration.ConfigurationController");
}




Sequence<OUString> SAL_CALL ConfigurationController_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName("com.sun.star.drawing.framework.ConfigurationController");
    return Sequence<OUString>(&sServiceName, 1);
}






class ConfigurationController::Implementation
{
public:
    Implementation (
        ConfigurationController& rController,
        const Reference<frame::XController>& rxController);
    ~Implementation (void);

    Reference<XControllerManager> mxControllerManager;

    /** The Broadcaster class implements storing and calling of listeners.
    */
    ::boost::shared_ptr<ConfigurationControllerBroadcaster> mpBroadcaster;

    /** The requested configuration which is modified (asynchronously) by
        calls to requestResourceActivation() and
        requestResourceDeactivation().  The mpConfigurationUpdater makes the
        current configuration reflect the content of this one.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration> mxRequestedConfiguration;

    ViewShellBase* mpBase;

    ::boost::shared_ptr<ResourceFactoryManager> mpResourceFactoryContainer;

    ::boost::shared_ptr<ConfigurationControllerResourceManager> mpResourceManager;

    ::boost::shared_ptr<ConfigurationUpdater> mpConfigurationUpdater;

    /** The queue processor ownes the queue of configuration change request
        objects and processes the objects.
    */
    ::boost::scoped_ptr<ChangeRequestQueueProcessor> mpQueueProcessor;

    ::boost::shared_ptr<ConfigurationUpdaterLock> mpConfigurationUpdaterLock;

    sal_Int32 mnLockCount;
};






ConfigurationController::Lock::Lock (const Reference<XConfigurationController>& rxController)
    : mxController(rxController)
{
    OSL_ASSERT(mxController.is());

    if (mxController.is())
        mxController->lock();
}




ConfigurationController::Lock::~Lock (void)
{
    if (mxController.is())
        mxController->unlock();
}






ConfigurationController::ConfigurationController (void) throw()
    : ConfigurationControllerInterfaceBase(MutexOwner::maMutex)
    , mpImplementation()
    , mbIsDisposed(false)
{
}

ConfigurationController::~ConfigurationController (void) throw()
{
}


void SAL_CALL ConfigurationController::disposing (void)
{
    if (mpImplementation.get() == NULL)
        return;

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationController::disposing");
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":     requesting empty configuration");
    
    
    mpImplementation->mpQueueProcessor->Clear();
    restoreConfiguration(new Configuration(this,false));
    mpImplementation->mpQueueProcessor->ProcessUntilEmpty();
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":     all requests processed");

    
    
    mbIsDisposed = true;

    
    lang::EventObject aEvent;
    aEvent.Source = uno::Reference<uno::XInterface>((cppu::OWeakObject*)this);

    {
        const SolarMutexGuard aSolarGuard;
        mpImplementation->mpBroadcaster->DisposeAndClear();
    }

    mpImplementation->mpQueueProcessor.reset();
    mpImplementation->mxRequestedConfiguration = NULL;
    mpImplementation.reset();
}




void ConfigurationController::ProcessEvent (void)
{
    if (mpImplementation.get() != NULL)
    {
        OSL_ASSERT(mpImplementation->mpQueueProcessor.get()!=NULL);

        mpImplementation->mpQueueProcessor->ProcessOneEvent();
    }
}




void ConfigurationController::RequestSynchronousUpdate (void)
{
    if (mpImplementation.get() == NULL)
        return;
    if (mpImplementation->mpQueueProcessor.get() == 0)
        return;
    mpImplementation->mpQueueProcessor->ProcessUntilEmpty();
}






void SAL_CALL ConfigurationController::addConfigurationChangeListener (
    const Reference<XConfigurationChangeListener>& rxListener,
    const OUString& rsEventType,
    const Any& rUserData)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed();
    OSL_ASSERT(mpImplementation.get()!=NULL);
    mpImplementation->mpBroadcaster->AddListener(rxListener, rsEventType, rUserData);
}




void SAL_CALL ConfigurationController::removeConfigurationChangeListener (
    const Reference<XConfigurationChangeListener>& rxListener)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed();
    mpImplementation->mpBroadcaster->RemoveListener(rxListener);
}




void SAL_CALL ConfigurationController::notifyEvent (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mpImplementation->mpBroadcaster->NotifyListeners(rEvent);
}



void SAL_CALL ConfigurationController::lock()
    throw (RuntimeException, std::exception)
{
    OSL_ASSERT(mpImplementation.get()!=NULL);
    OSL_ASSERT(mpImplementation->mpConfigurationUpdater.get()!=NULL);

    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();


    ++mpImplementation->mnLockCount;
    if (mpImplementation->mpConfigurationUpdaterLock.get()==NULL)
        mpImplementation->mpConfigurationUpdaterLock
            = mpImplementation->mpConfigurationUpdater->GetLock();
}




void SAL_CALL ConfigurationController::unlock (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    
    
    if (rBHelper.bDisposed)
        ThrowIfDisposed();

    OSL_ASSERT(mpImplementation->mnLockCount>0);
    --mpImplementation->mnLockCount;
    if (mpImplementation->mnLockCount == 0)
        mpImplementation->mpConfigurationUpdaterLock.reset();
}




void SAL_CALL ConfigurationController::requestResourceActivation (
    const Reference<XResourceId>& rxResourceId,
    ResourceActivationMode eMode)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
       ThrowIfDisposed();

    
    
    
    
    
    if (rBHelper.bInDispose)
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationController::requestResourceActivation(): ignoring " <<
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
        return;
    }

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationController::requestResourceActivation() " <<
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());

    if (rxResourceId.is())
    {
        if (eMode == ResourceActivationMode_REPLACE)
        {
            
            
            Sequence<Reference<XResourceId> > aResourceList (
                mpImplementation->mxRequestedConfiguration->getResources(
                    rxResourceId->getAnchor(),
                    rxResourceId->getResourceTypePrefix(),
                    AnchorBindingMode_DIRECT));

            for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
            {
                
                
                
                if (rxResourceId->compareTo(aResourceList[nIndex]) == 0)
                    continue;

                
                
                requestResourceDeactivation(aResourceList[nIndex]);
            }
        }

        Reference<XConfigurationChangeRequest> xRequest(
            new GenericConfigurationChangeRequest(
                rxResourceId,
                GenericConfigurationChangeRequest::Activation));
        postChangeRequest(xRequest);
    }
}




void SAL_CALL ConfigurationController::requestResourceDeactivation (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationController::requestResourceDeactivation() " <<
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());

    if (rxResourceId.is())
    {
        
        
        const Sequence<Reference<XResourceId> > aLinkedResources (
            mpImplementation->mxRequestedConfiguration->getResources(
                rxResourceId,
                OUString(),
                AnchorBindingMode_DIRECT));
        const sal_Int32 nCount (aLinkedResources.getLength());
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            
            
            
            requestResourceDeactivation(aLinkedResources[nIndex]);
        }

        
        Reference<XConfigurationChangeRequest> xRequest(
            new GenericConfigurationChangeRequest(
                rxResourceId,
                GenericConfigurationChangeRequest::Deactivation));
        postChangeRequest(xRequest);
    }
}




Reference<XResource> SAL_CALL ConfigurationController::getResource (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    ConfigurationControllerResourceManager::ResourceDescriptor aDescriptor (
        mpImplementation->mpResourceManager->GetResource(rxResourceId));
    return aDescriptor.mxResource;
}




void SAL_CALL ConfigurationController::update (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (mpImplementation->mpQueueProcessor->IsEmpty())
    {
        
        
        mpImplementation->mpQueueProcessor->AddRequest(new UpdateRequest());
    }
    else
    {
        
        
    }
}




sal_Bool SAL_CALL ConfigurationController::hasPendingRequests (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    return ! mpImplementation->mpQueueProcessor->IsEmpty();
}





void SAL_CALL ConfigurationController::postChangeRequest (
    const Reference<XConfigurationChangeRequest>& rxRequest)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    mpImplementation->mpQueueProcessor->AddRequest(rxRequest);
}




Reference<XConfiguration> SAL_CALL ConfigurationController::getRequestedConfiguration (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (mpImplementation->mxRequestedConfiguration.is())
        return Reference<XConfiguration>(
            mpImplementation->mxRequestedConfiguration->createClone(), UNO_QUERY);
    else
        return Reference<XConfiguration>();
}




Reference<XConfiguration> SAL_CALL ConfigurationController::getCurrentConfiguration (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    Reference<XConfiguration> xCurrentConfiguration(
        mpImplementation->mpConfigurationUpdater->GetCurrentConfiguration());
    if (xCurrentConfiguration.is())
        return Reference<XConfiguration>(xCurrentConfiguration->createClone(), UNO_QUERY);
    else
        return Reference<XConfiguration>();
}




/** The given configuration is restored by generating the appropriate set of
    activation and deactivation requests.
*/
void SAL_CALL ConfigurationController::restoreConfiguration (
    const Reference<XConfiguration>& rxNewConfiguration)
    throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    
    
    
    ::boost::shared_ptr<ConfigurationUpdaterLock> pLock (
        mpImplementation->mpConfigurationUpdater->GetLock());

    
    Reference<XConfiguration> xCurrentConfiguration (mpImplementation->mxRequestedConfiguration);
#if OSL_DEBUG_LEVEL >=1
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationController::restoreConfiguration(");
    ConfigurationTracer::TraceConfiguration(rxNewConfiguration, "requested configuration");
    ConfigurationTracer::TraceConfiguration(xCurrentConfiguration, "current configuration");
#endif
    ConfigurationClassifier aClassifier (rxNewConfiguration, xCurrentConfiguration);
    aClassifier.Partition();
#if OSL_DEBUG_LEVEL >=2
    aClassifier.TraceResourceIdVector(
        "requested but not current resources:\n", aClassifier.GetC1minusC2());
    aClassifier.TraceResourceIdVector(
        "current but not requested resources:\n", aClassifier.GetC2minusC1());
    aClassifier.TraceResourceIdVector(
        "requested and current resources:\n", aClassifier.GetC1andC2());
#endif

    ConfigurationClassifier::ResourceIdVector::const_iterator iResource;

    
    
    const ConfigurationClassifier::ResourceIdVector& rResourcesToDeactivate (
        aClassifier.GetC2minusC1());
    for (iResource=rResourcesToDeactivate.begin();
         iResource!=rResourcesToDeactivate.end();
         ++iResource)
    {
        requestResourceDeactivation(*iResource);
    }

    
    
    const ConfigurationClassifier::ResourceIdVector& rResourcesToActivate (
        aClassifier.GetC1minusC2());
    for (iResource=rResourcesToActivate.begin();
         iResource!=rResourcesToActivate.end();
         ++iResource)
    {
        requestResourceActivation(*iResource, ResourceActivationMode_ADD);
    }

    pLock.reset();
}






void SAL_CALL ConfigurationController::addResourceFactory(
    const OUString& sResourceURL,
    const Reference<XResourceFactory>& rxResourceFactory)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    mpImplementation->mpResourceFactoryContainer->AddFactory(sResourceURL, rxResourceFactory);
}




void SAL_CALL ConfigurationController::removeResourceFactoryForURL(
    const OUString& sResourceURL)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    mpImplementation->mpResourceFactoryContainer->RemoveFactoryForURL(sResourceURL);
}




void SAL_CALL ConfigurationController::removeResourceFactoryForReference(
    const Reference<XResourceFactory>& rxResourceFactory)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();
    mpImplementation->mpResourceFactoryContainer->RemoveFactoryForReference(rxResourceFactory);
}




Reference<XResourceFactory> SAL_CALL ConfigurationController::getResourceFactory (
    const OUString& sResourceURL)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    return mpImplementation->mpResourceFactoryContainer->GetFactory(sResourceURL);
}






void SAL_CALL ConfigurationController::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    if (aArguments.getLength() == 1)
    {
        const SolarMutexGuard aSolarGuard;

        mpImplementation.reset(new Implementation(
            *this,
            Reference<frame::XController>(aArguments[0], UNO_QUERY_THROW)));
    }
}






void ConfigurationController::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (mbIsDisposed)
    {
        throw lang::DisposedException ("ConfigurationController object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }

    if (mpImplementation.get() == NULL)
    {
        OSL_ASSERT(mpImplementation.get() != NULL);
        throw RuntimeException("ConfigurationController not initialized",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}






ConfigurationController::Implementation::Implementation (
    ConfigurationController& rController,
    const Reference<frame::XController>& rxController)
    : mxControllerManager(rxController, UNO_QUERY_THROW),
      mpBroadcaster(new ConfigurationControllerBroadcaster(&rController)),
      mxRequestedConfiguration(new Configuration(&rController, true)),
      mpBase(NULL),
      mpResourceFactoryContainer(new ResourceFactoryManager(mxControllerManager)),
      mpResourceManager(
          new ConfigurationControllerResourceManager(mpResourceFactoryContainer,mpBroadcaster)),
      mpConfigurationUpdater(
          new ConfigurationUpdater(mpBroadcaster, mpResourceManager,mxControllerManager)),
      mpQueueProcessor(new ChangeRequestQueueProcessor(&rController,mpConfigurationUpdater)),
      mpConfigurationUpdaterLock(),
      mnLockCount(0)
{
    mpQueueProcessor->SetConfiguration(mxRequestedConfiguration);
}




ConfigurationController::Implementation::~Implementation (void)
{
}




} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
