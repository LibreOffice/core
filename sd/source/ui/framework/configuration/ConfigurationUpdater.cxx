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

#include "precompiled_sd.hxx"

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
using ::rtl::OUString;
using ::std::vector;

#undef VERBOSE
//#define VERBOSE 2

namespace {
static const sal_Int32 snShortTimeout (100);
static const sal_Int32 snNormalTimeout (1000);
static const sal_Int32 snLongTimeout (10000);
static const sal_Int32 snShortTimeoutCountThreshold (1);
static const sal_Int32 snNormalTimeoutCountThreshold (5);
}

namespace sd { namespace framework {


//===== ConfigurationUpdaterLock ==============================================

class ConfigurationUpdaterLock
{
public:
    ConfigurationUpdaterLock (ConfigurationUpdater& rUpdater)
        : mrUpdater(rUpdater) { mrUpdater.LockUpdates(); }
    ~ConfigurationUpdaterLock(void) { mrUpdater.UnlockUpdates(); }
private:
    ConfigurationUpdater& mrUpdater;
};




//===== ConfigurationUpdater ==================================================

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
    // Prepare the timer that is started when after an update the current
    // and the requested configuration differ.  With the timer we try
    // updates until the two configurations are the same.
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

    // Find out whether we really can update the configuration.
    if (IsUpdatePossible())
    {
#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("UpdateConfiguration start");
#endif

        // Call UpdateConfiguration while that is possible and while someone
        // set mbUpdatePending to true in the middle of it.
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
#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("scheduling update for later");
#endif
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
#if defined VERBOSE && VERBOSE>=1
        OSL_TRACE("UpdateConfiguration update");
#endif
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
#if defined VERBOSE && VERBOSE>=2
            OSL_TRACE("ConfigurationUpdater::UpdateConfiguration(");
            ConfigurationTracer::TraceConfiguration(
                mxRequestedConfiguration, "requested configuration");
            ConfigurationTracer::TraceConfiguration(
                mxCurrentConfiguration, "current configuration");
#endif
            // Notify the begining of the update.
            ConfigurationChangeEvent aEvent;
            aEvent.Type = FrameworkHelper::msConfigurationUpdateStartEvent;
            aEvent.Configuration = mxRequestedConfiguration;
            mpBroadcaster->NotifyListeners(aEvent);

            // Do the actual update.  All exceptions are caught and ignored,
            // so that the the end of the update is notified always.
            try
            {
                if (mnLockCount == 0)
                    UpdateCore(aClassifier);
            }
            catch(const RuntimeException&)
            {
            }

            // Notify the end of the update.
            aEvent.Type = FrameworkHelper::msConfigurationUpdateEndEvent;
            mpBroadcaster->NotifyListeners(aEvent);

            CheckUpdateSuccess();
        }
        else
        {
#if defined VERBOSE && VERBOSE>0
            OSL_TRACE("nothing to do");
#if defined VERBOSE && VERBOSE>=2
            ConfigurationTracer::TraceConfiguration(
                mxRequestedConfiguration, "requested configuration");
            ConfigurationTracer::TraceConfiguration(
                mxCurrentConfiguration, "current configuration");
#endif
#endif
        }
    }
    catch(const RuntimeException &)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

#if defined VERBOSE && VERBOSE>0
    OSL_TRACE("ConfigurationUpdater::UpdateConfiguration)");
    OSL_TRACE("UpdateConfiguration end");
#endif
}




void ConfigurationUpdater::CleanRequestedConfiguration (void)
{
    if (mxControllerManager.is())
    {
        // Request the deactivation of pure anchors that have no child.
        vector<Reference<XResourceId> > aResourcesToDeactivate;
        CheckPureAnchors(mxRequestedConfiguration, aResourcesToDeactivate);
        if (aResourcesToDeactivate.size() > 0)
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
    // When the two configurations differ then start the timer to call
    // another update later.
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
        // Update was successfull.  Reset the failed update count.
        mnFailedUpdateCount = 0;
    }
}




void ConfigurationUpdater::UpdateCore (const ConfigurationClassifier& rClassifier)
{
    try
    {
#if defined VERBOSE && VERBOSE>=2
        rClassifier.TraceResourceIdVector(
            "requested but not current resources:", rClassifier.GetC1minusC2());
        rClassifier.TraceResourceIdVector(
            "current but not requested resources:", rClassifier.GetC2minusC1());
        rClassifier.TraceResourceIdVector(
            "requested and current resources:", rClassifier.GetC1andC2());
#endif

        // Updating of the sub controllers is done in two steps.  In the
        // first the sub controllers typically shut down resources that are
        // not requested anymore.  In the second the sub controllers
        // typically set up resources that have been newly requested.
        mpResourceManager->DeactivateResources(rClassifier.GetC2minusC1(), mxCurrentConfiguration);
        mpResourceManager->ActivateResources(rClassifier.GetC1minusC2(), mxCurrentConfiguration);

#if defined VERBOSE && VERBOSE>=2
        OSL_TRACE("ConfigurationController::UpdateConfiguration)");
        ConfigurationTracer::TraceConfiguration(
            mxRequestedConfiguration, "requested configuration");
        ConfigurationTracer::TraceConfiguration(
            mxCurrentConfiguration, "current configuration");
#endif

        // Deactivate pure anchors that have no child.
        vector<Reference<XResourceId> > aResourcesToDeactivate;
        CheckPureAnchors(mxCurrentConfiguration, aResourcesToDeactivate);
        if (aResourcesToDeactivate.size() > 0)
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

    // Get a list of all resources in the configuration.
    Sequence<Reference<XResourceId> > aResources(
        rxConfiguration->getResources(
            NULL, OUString(), AnchorBindingMode_INDIRECT));
    sal_Int32 nCount (aResources.getLength());

    // Prepare the list of pure anchors that have to be deactivated.
    rResourcesToDeactivate.clear();

    // Iterate over the list in reverse order because when there is a chain
    // of pure anchors with only the last one having no child then the whole
    // list has to be deactivated.
    sal_Int32 nIndex (nCount-1);
    while (nIndex >= 0)
    {
        const Reference<XResourceId> xResourceId (aResources[nIndex]);
        const Reference<XResource> xResource (
            mpResourceManager->GetResource(xResourceId).mxResource);
        bool bDeactiveCurrentResource (false);

        // Skip all resources that are no pure anchors.
        if (xResource.is() && xResource->isAnchorOnly())
        {
            // When xResource is not an anchor of the the next resource in
            // the list then it is the anchor of no resource at all.
            if (nIndex == nCount-1)
            {
                // No following anchors, deactivate this one, then remove it
                // from the list.
                bDeactiveCurrentResource = true;
            }
            else
            {
                const Reference<XResourceId> xPrevResourceId (aResources[nIndex+1]);
                if ( ! xPrevResourceId.is()
                    || ! xPrevResourceId->isBoundTo(xResourceId, AnchorBindingMode_DIRECT))
                {
                    // The previous resource (id) does not exist or is not bound to
                    // the current anchor.
                    bDeactiveCurrentResource = true;
                }
            }
        }

        if (bDeactiveCurrentResource)
        {
#if defined VERBOSE && VERBOSE>=2
            OSL_TRACE("deactiving pure anchor %s because it has no children",
                OUStringToOString(
                    FrameworkHelper::ResourceIdToString(xResourceId),
                    RTL_TEXTENCODING_UTF8).getStr());
#endif
            // Erase element from current configuration.
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




IMPL_LINK(ConfigurationUpdater, TimeoutHandler, Timer*, EMPTYARG)
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


} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
