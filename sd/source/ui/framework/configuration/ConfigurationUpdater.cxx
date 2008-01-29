/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationUpdater.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:19:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "ConfigurationUpdater.hxx"
#include "ConfigurationTracer.hxx"
#include "ConfigurationClassifier.hxx"
#include "framework/Configuration.hxx"
#include "framework/FrameworkHelper.hxx"

#include <comphelper/scopeguard.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

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
    const Reference<XConfigurationControllerBroadcaster>& rxBroadcaster)
    : mxControllerManager(),
      mxBroadcaster(rxBroadcaster),
      mxCurrentConfiguration(Reference<XConfiguration>(new Configuration(NULL, false))),
      mxRequestedConfiguration(),
      mbUpdatePending(false),
      mbUpdateBeingProcessed(false),
      mnLockCount(0),
      maUpdateTimer(),
      mnFailedUpdateCount(0)
{
    // Prepare the timer that is started when after an update the current
    // and the requested configuration differ.  With the timer we try
    // updates until the two configurations are the same.
    maUpdateTimer.SetTimeout(snNormalTimeout);
    maUpdateTimer.SetTimeoutHdl(LINK(this,ConfigurationUpdater,TimeoutHandler));
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
#if defined VERBOSE && VERBOSE>=1
    OSL_TRACE("UpdateConfiguration start");
#endif

    mxRequestedConfiguration = rxRequestedConfiguration;

    // Find out whether we really can update the configuration.
    if (IsUpdatePossible())
    {
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
        ::boost::bind(
            &ConfigurationUpdater::SetUpdateBeingProcessed,
            this,
            false));

    try
    {
        mbUpdatePending = false;

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
            if (mxBroadcaster.is())
                mxBroadcaster->notifyEvent(aEvent);

            // Do the actual update.  All exceptions are caught and ignored,
            // so that the the end of the update is notified always.
            try
            {
                if (mnLockCount == 0)
                    UpdateCore(aClassifier);
            }
            catch(RuntimeException)
            {
            }

            // Notify the end of the update.
            aEvent.Type = FrameworkHelper::msConfigurationUpdateEndEvent;
            if (mxBroadcaster.is())
                mxBroadcaster->notifyEvent(aEvent);

            CheckUpdateSuccess();
        }
        else
        {
#if defined VERBOSE && VERBOSE>0
            OSL_TRACE("nothing to do");
#endif
        }
    }
    catch (RuntimeException e)
    {
        OSL_TRACE("caught exception while updating the current configuration");
        DBG_ASSERT(false, "caught exception while updating the current configuration");
    }

#if defined VERBOSE && VERBOSE>0
    OSL_TRACE("ConfigurationUpdater::UpdateConfiguration)");
    OSL_TRACE("UpdateConfiguration end");
#endif
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
#if defined VERBOSE && VERBOSE>=2
    rClassifier.TraceResourceIdVector(
        "requested but not current resources:", rClassifier.GetC1minusC2());
    rClassifier.TraceResourceIdVector(
        "current but not requested resources:", rClassifier.GetC2minusC1());
    rClassifier.TraceResourceIdVector(
        "requested and current resources:", rClassifier.GetC1andC2());
#endif

    // Updating of the sub controllers is done in two steps.  In the
    // first the sub controllers typically shut down resources that
    // are not requested anymore.  In the second the sub controllers
    // typically set up resources that have been newly requested.
    UpdateStart(rClassifier.GetC2minusC1());
    UpdateEnd(rClassifier.GetC1minusC2());

#if defined VERBOSE && VERBOSE>=2
    OSL_TRACE("ConfigurationController::UpdateConfiguration)");
    ConfigurationTracer::TraceConfiguration(
        mxRequestedConfiguration, "requested configuration");
    ConfigurationTracer::TraceConfiguration(
        mxCurrentConfiguration, "current configuration");
#endif
}




void ConfigurationUpdater::UpdateStart (
    const ::std::vector<Reference<XResourceId> >& rResources)
{
    // Convert vector into sequence.
    sal_Int32 nCount (rResources.size());
    Sequence<Reference<XResourceId> > aResourcesToDeactivate (nCount);
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        aResourcesToDeactivate[nIndex] = rResources[nIndex];

    // Call all resource controllers known to the controller manager and
    // tell them to start the configuration update.
    Sequence<Reference<XResourceController> > aResourceControllers(
        mxControllerManager->getResourceControllers());
    for (sal_Int32 nIndex=0; nIndex<aResourceControllers.getLength(); ++nIndex)
    {
        if (aResourceControllers[nIndex].is())
        {
            try
            {
                aResourceControllers[nIndex]->updateStart(
                    mxRequestedConfiguration,
                    mxCurrentConfiguration,
                    aResourcesToDeactivate);
            }
            catch(RuntimeException)
            {
            }
        }
    }
}





void ConfigurationUpdater::UpdateEnd (
    const ::std::vector<Reference<XResourceId> >& rResources)
{
    // Convert vector into sequence.
    sal_Int32 nCount (rResources.size());
    Sequence<Reference<XResourceId> > aResourcesToActivate (nCount);
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        aResourcesToActivate[nIndex] = rResources[nIndex];

    // Call all resource controllers known to the controller manager and
    // tell them to finish the configuration update.
    Sequence<Reference<XResourceController> > aResourceControllers(
        mxControllerManager->getResourceControllers());
    for (sal_Int32 nIndex=aResourceControllers.getLength()-1; nIndex>=0; --nIndex)
    {
        if (aResourceControllers[nIndex].is())
        {
            try
            {
                aResourceControllers[nIndex]->updateEnd(
                    mxRequestedConfiguration,
                    mxCurrentConfiguration,
                    aResourcesToActivate);
            }
            catch(RuntimeException)
            {
            }
        }
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
