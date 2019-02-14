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

#include "debugtrace.hxx"
#include "ChangeRequestQueueProcessor.hxx"
#include "ConfigurationTracer.hxx"

#include <framework/ConfigurationController.hxx>
#include "ConfigurationUpdater.hxx"

#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace {

#if DEBUG_SD_CONFIGURATION_TRACE

void TraceRequest (const Reference<XConfigurationChangeRequest>& rxRequest)
{
    Reference<container::XNamed> xNamed (rxRequest, UNO_QUERY);
    if (xNamed.is())
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    " << xNamed->getName());
}

#endif

} // end of anonymous namespace

namespace sd { namespace framework {

ChangeRequestQueueProcessor::ChangeRequestQueueProcessor (
    const std::shared_ptr<ConfigurationUpdater>& rpConfigurationUpdater)
    : maMutex(),
      maQueue(),
      mnUserEventId(nullptr),
      mxConfiguration(),
      mpConfigurationUpdater(rpConfigurationUpdater)
{
}

ChangeRequestQueueProcessor::~ChangeRequestQueueProcessor()
{
    if (mnUserEventId != nullptr)
        Application::RemoveUserEvent(mnUserEventId);
}

void ChangeRequestQueueProcessor::SetConfiguration (
    const Reference<XConfiguration>& rxConfiguration)
{
    ::osl::MutexGuard aGuard (maMutex);

    mxConfiguration = rxConfiguration;
    StartProcessing();
}

void ChangeRequestQueueProcessor::AddRequest (
    const Reference<XConfigurationChangeRequest>& rxRequest)
{
    ::osl::MutexGuard aGuard (maMutex);

#if DEBUG_SD_CONFIGURATION_TRACE
    if (maQueue.empty())
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": Adding requests to empty queue");
        ConfigurationTracer::TraceConfiguration(
            mxConfiguration, "current configuration of queue processor");
    }
    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": Adding request");
    TraceRequest(rxRequest);
#endif

    maQueue.push_back(rxRequest);
    StartProcessing();
}

void ChangeRequestQueueProcessor::StartProcessing()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mnUserEventId == nullptr
        && mxConfiguration.is()
        && ! maQueue.empty())
    {
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ChangeRequestQueueProcessor scheduling processing");
        mnUserEventId = Application::PostUserEvent(
            LINK(this,ChangeRequestQueueProcessor,ProcessEvent));
    }
}

IMPL_LINK_NOARG(ChangeRequestQueueProcessor, ProcessEvent, void*, void)
{
    ::osl::MutexGuard aGuard (maMutex);

    mnUserEventId = nullptr;

    ProcessOneEvent();

    if ( ! maQueue.empty())
    {
        // Schedule the processing of the next event.
        StartProcessing();
    }
}

void ChangeRequestQueueProcessor::ProcessOneEvent()
{
    ::osl::MutexGuard aGuard (maMutex);

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ProcessOneEvent");

    if (!mxConfiguration.is() || maQueue.empty())
        return;

    // Get and remove the first entry from the queue.
    Reference<XConfigurationChangeRequest> xRequest (maQueue.front());
    maQueue.pop_front();

    // Execute the change request.
    if (xRequest.is())
    {
#if DEBUG_SD_CONFIGURATION_TRACE
        TraceRequest(xRequest);
#endif
        xRequest->execute(mxConfiguration);
    }

    if (!maQueue.empty())
        return;

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": All requests are processed");
    // The queue is empty so tell the ConfigurationManager to update
    // its state.
    if (mpConfigurationUpdater != nullptr)
    {
#if DEBUG_SD_CONFIGURATION_TRACE
        ConfigurationTracer::TraceConfiguration (
            mxConfiguration, "updating to configuration");
#endif
        mpConfigurationUpdater->RequestUpdate(mxConfiguration);
    }
}

bool ChangeRequestQueueProcessor::IsEmpty() const
{
    return maQueue.empty();
}

void ChangeRequestQueueProcessor::ProcessUntilEmpty()
{
    while ( ! IsEmpty())
        ProcessOneEvent();
}

void ChangeRequestQueueProcessor::Clear()
{
    ::osl::MutexGuard aGuard (maMutex);
    maQueue.clear();
}

} } // end of namespace sd::framework::configuration

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
