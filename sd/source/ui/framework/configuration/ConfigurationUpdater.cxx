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


#include "ConfigurationUpdater.hxx"
#include "ConfigurationTracer.hxx"
#include "ConfigurationClassifier.hxx"
#include "ConfigurationControllerBroadcaster.hxx"
#include "framework/Configuration.hxx"
#include "framework/FrameworkHelper.hxx"

#include <comphelper/scopeguard.hxx>
#include <tools/diagnose_ex.h>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;
using ::std::vector;

namespace {
static const sal_Int32 snShortTimeout (100);
static const sal_Int32 snNormalTimeout (1000);
static const sal_Int32 snLongTimeout (10000);
static const sal_Int32 snShortTimeoutCountThreshold (1);
static const sal_Int32 snNormalTimeoutCountThreshold (5);
}

namespace sd { namespace framework {




class ConfigurationUpdaterLock
{
public:
    ConfigurationUpdaterLock (ConfigurationUpdater& rUpdater)
        : mrUpdater(rUpdater) { mrUpdater.LockUpdates(); }
    ~ConfigurationUpdaterLock(void) { mrUpdater.UnlockUpdates(); }
private:
    ConfigurationUpdater& mrUpdater;
};






ConfigurationUpdater::ConfigurationUpdater (
    const ::boost::shared_ptr<ConfigurationControllerBroadcaster>& rpBroadcaster,
    const ::boost::shared_ptr<ConfigurationControllerResourceManager>& rpResourceManager,
    const Reference<XControllerManager>& rxControllerManager)
    : mxControllerManager(),
      mpBroadcaster(rpBroadcaster),
      mxCurrentConfiguration(Reference<XConfiguration>(new Configuration(NULL, false))),
      mxRequestedConfiguration(),
      mbUpdatePending(false),
      mbUpdateBeingProcessed(false),
      mnLockCount(0),
      maUpdateTimer(),
      mnFailedUpdateCount(0),
      mpResourceManager(rpResourceManager)
{
    
    
    
    maUpdateTimer.SetTimeout(snNormalTimeout);
    maUpdateTimer.SetTimeoutHdl(LINK(this,ConfigurationUpdater,TimeoutHandler));
    SetControllerManager(rxControllerManager);
}




ConfigurationUpdater::~ConfigurationUpdater (void)
{
    maUpdateTimer.Stop();
}




void ConfigurationUpdater::SetControllerManager(
    const Reference<XControllerManager>& rxControllerManager)
{
    mxControllerManager = rxControllerManager;
}




void ConfigurationUpdater::RequestUpdate (
    const Reference<XConfiguration>& rxRequestedConfiguration)
{
    mxRequestedConfiguration = rxRequestedConfiguration;

    
    if (IsUpdatePossible())
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": UpdateConfiguration start");

        
        
        do
        {
            UpdateConfiguration();

            if (mbUpdatePending && IsUpdatePossible())
                continue;
        }
        while (false);
    }
    else
    {
        mbUpdatePending = true;
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": scheduling update for later");
    }
}




Reference<XConfiguration> ConfigurationUpdater::GetCurrentConfiguration (void) const
{
    return mxCurrentConfiguration;
}




bool ConfigurationUpdater::IsUpdatePossible (void)
{
    return ! mbUpdateBeingProcessed
        && mxControllerManager.is()
        && mnLockCount==0
        && mxRequestedConfiguration.is()
        && mxCurrentConfiguration.is();
}




void ConfigurationUpdater::UpdateConfiguration (void)
{
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": UpdateConfiguration update");
    SetUpdateBeingProcessed(true);
    comphelper::ScopeGuard aScopeGuard (
        ::boost::bind(&ConfigurationUpdater::SetUpdateBeingProcessed, this, false));

    try
    {
        mbUpdatePending = false;

        CleanRequestedConfiguration();
        ConfigurationClassifier aClassifier(mxRequestedConfiguration, mxCurrentConfiguration);
        if (aClassifier.Partition())
        {
#if OSL_DEBUG_LEVEL >= 2
            SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationUpdater::UpdateConfiguration(");
            ConfigurationTracer::TraceConfiguration(
                mxRequestedConfiguration, "requested configuration");
            ConfigurationTracer::TraceConfiguration(
                mxCurrentConfiguration, "current configuration");
#endif
            
            ConfigurationChangeEvent aEvent;
            aEvent.Type = FrameworkHelper::msConfigurationUpdateStartEvent;
            aEvent.Configuration = mxRequestedConfiguration;
            mpBroadcaster->NotifyListeners(aEvent);

            
            
            try
            {
                if (mnLockCount == 0)
                    UpdateCore(aClassifier);
            }
            catch(const RuntimeException&)
            {
            }

            
            aEvent.Type = FrameworkHelper::msConfigurationUpdateEndEvent;
            mpBroadcaster->NotifyListeners(aEvent);

            CheckUpdateSuccess();
        }
        else
        {
            SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": nothing to do");
#if OSL_DEBUG_LEVEL >= 2
            ConfigurationTracer::TraceConfiguration(
                mxRequestedConfiguration, "requested configuration");
            ConfigurationTracer::TraceConfiguration(
                mxCurrentConfiguration, "current configuration");
#endif
        }
    }
    catch(const RuntimeException &)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationUpdater::UpdateConfiguration)");
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": UpdateConfiguration end");
}




void ConfigurationUpdater::CleanRequestedConfiguration (void)
{
    if (mxControllerManager.is())
    {
        
        vector<Reference<XResourceId> > aResourcesToDeactivate;
        CheckPureAnchors(mxRequestedConfiguration, aResourcesToDeactivate);
        if (!aResourcesToDeactivate.empty())
        {
            Reference<XConfigurationController> xCC (
                mxControllerManager->getConfigurationController());
            vector<Reference<XResourceId> >::iterator iId;
            for (iId=aResourcesToDeactivate.begin(); iId!=aResourcesToDeactivate.end(); ++iId)
                if (iId->is())
                    xCC->requestResourceDeactivation(*iId);
        }
    }
}




void ConfigurationUpdater::CheckUpdateSuccess (void)
{
    
    
    if ( ! AreConfigurationsEquivalent(mxCurrentConfiguration, mxRequestedConfiguration))
    {
        if (mnFailedUpdateCount <= snShortTimeoutCountThreshold)
            maUpdateTimer.SetTimeout(snShortTimeout);
        else if (mnFailedUpdateCount < snNormalTimeoutCountThreshold)
            maUpdateTimer.SetTimeout(snNormalTimeout);
        else
            maUpdateTimer.SetTimeout(snLongTimeout);
        ++mnFailedUpdateCount;
        maUpdateTimer.Start();
    }
    else
    {
        
        mnFailedUpdateCount = 0;
    }
}




void ConfigurationUpdater::UpdateCore (const ConfigurationClassifier& rClassifier)
{
    try
    {
#if OSL_DEBUG_LEVEL >= 2
        rClassifier.TraceResourceIdVector(
            "requested but not current resources:", rClassifier.GetC1minusC2());
        rClassifier.TraceResourceIdVector(
            "current but not requested resources:", rClassifier.GetC2minusC1());
        rClassifier.TraceResourceIdVector(
            "requested and current resources:", rClassifier.GetC1andC2());
#endif

        
        
        
        
        mpResourceManager->DeactivateResources(rClassifier.GetC2minusC1(), mxCurrentConfiguration);
        mpResourceManager->ActivateResources(rClassifier.GetC1minusC2(), mxCurrentConfiguration);

#if OSL_DEBUG_LEVEL >= 2
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ConfigurationController::UpdateConfiguration)");
        ConfigurationTracer::TraceConfiguration(
            mxRequestedConfiguration, "requested configuration");
        ConfigurationTracer::TraceConfiguration(
            mxCurrentConfiguration, "current configuration");
#endif

        
        vector<Reference<XResourceId> > aResourcesToDeactivate;
        CheckPureAnchors(mxCurrentConfiguration, aResourcesToDeactivate);
        if (!aResourcesToDeactivate.empty())
            mpResourceManager->DeactivateResources(aResourcesToDeactivate, mxCurrentConfiguration);
    }
    catch(const RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




void ConfigurationUpdater::CheckPureAnchors (
    const Reference<XConfiguration>& rxConfiguration,
    vector<Reference<XResourceId> >& rResourcesToDeactivate)
{
    if ( ! rxConfiguration.is())
        return;

    
    Sequence<Reference<XResourceId> > aResources(
        rxConfiguration->getResources(
            NULL, OUString(), AnchorBindingMode_INDIRECT));
    sal_Int32 nCount (aResources.getLength());

    
    rResourcesToDeactivate.clear();

    
    
    
    sal_Int32 nIndex (nCount-1);
    while (nIndex >= 0)
    {
        const Reference<XResourceId> xResourceId (aResources[nIndex]);
        const Reference<XResource> xResource (
            mpResourceManager->GetResource(xResourceId).mxResource);
        bool bDeactiveCurrentResource (false);

        
        if (xResource.is() && xResource->isAnchorOnly())
        {
            
            
            if (nIndex == nCount-1)
            {
                
                
                bDeactiveCurrentResource = true;
            }
            else
            {
                const Reference<XResourceId> xPrevResourceId (aResources[nIndex+1]);
                if ( ! xPrevResourceId.is()
                    || ! xPrevResourceId->isBoundTo(xResourceId, AnchorBindingMode_DIRECT))
                {
                    
                    
                    bDeactiveCurrentResource = true;
                }
            }
        }

        if (bDeactiveCurrentResource)
        {
            SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": deactiving pure anchor " <<
                OUStringToOString(
                    FrameworkHelper::ResourceIdToString(xResourceId),
                    RTL_TEXTENCODING_UTF8).getStr() << "because it has no children");
            
            for (sal_Int32 nI=nIndex; nI<nCount-2; ++nI)
                aResources[nI] = aResources[nI+1];
            nCount -= 1;

            rResourcesToDeactivate.push_back(xResourceId);
        }
        nIndex -= 1;
    }
}




void ConfigurationUpdater::LockUpdates (void)
{
    ++mnLockCount;
}




void ConfigurationUpdater::UnlockUpdates (void)
{
    --mnLockCount;
    if (mnLockCount == 0 && mbUpdatePending)
    {
        RequestUpdate(mxRequestedConfiguration);
    }
}




::boost::shared_ptr<ConfigurationUpdaterLock> ConfigurationUpdater::GetLock (void)
{
    return ::boost::shared_ptr<ConfigurationUpdaterLock>(new ConfigurationUpdaterLock(*this));
}




void ConfigurationUpdater::SetUpdateBeingProcessed (bool bValue)
{
    mbUpdateBeingProcessed = bValue;
}




IMPL_LINK_NOARG(ConfigurationUpdater, TimeoutHandler)
{
    OSL_TRACE("configuration update timer");
    if ( ! mbUpdateBeingProcessed
        && mxCurrentConfiguration.is()
        && mxRequestedConfiguration.is())
    {
        if ( ! AreConfigurationsEquivalent(mxCurrentConfiguration, mxRequestedConfiguration))
        {
            OSL_TRACE("configurations differ, requesting update");
            RequestUpdate(mxRequestedConfiguration);
        }
    }
    return 0;
}


} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
