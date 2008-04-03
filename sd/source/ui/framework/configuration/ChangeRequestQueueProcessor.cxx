/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChangeRequestQueueProcessor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:27:55 $
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

#include "ChangeRequestQueueProcessor.hxx"
#include "ConfigurationTracer.hxx"

#include "framework/ConfigurationController.hxx"
#include "ConfigurationUpdater.hxx"

#include <vcl/svapp.hxx>

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

#undef VERBOSE
//#define VERBOSE 1

namespace {

#ifdef VERBOSE

void TraceRequest (const Reference<XConfigurationChangeRequest>& rxRequest)
{
    Reference<container::XNamed> xNamed (rxRequest, UNO_QUERY);
    if (xNamed.is())
        OSL_TRACE("    %s",
            ::rtl::OUStringToOString(xNamed->getName(), RTL_TEXTENCODING_UTF8).getStr());
}

#endif

} // end of anonymous namespace


namespace sd { namespace framework {

ChangeRequestQueueProcessor::ChangeRequestQueueProcessor (
    const ::rtl::Reference<ConfigurationController>& rpConfigurationController,
    const ::boost::shared_ptr<ConfigurationUpdater>& rpConfigurationUpdater)
    : maMutex(),
      maQueue(),
      mnUserEventId(0),
      mxConfiguration(),
      mpConfigurationController(rpConfigurationController),
      mpConfigurationUpdater(rpConfigurationUpdater)
{
}




ChangeRequestQueueProcessor::~ChangeRequestQueueProcessor (void)
{
    if (mnUserEventId != 0)
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

#ifdef VERBOSE
    if (maQueue.empty())
    {
        OSL_TRACE("Adding requests to empty queue");
        ConfigurationTracer::TraceConfiguration(
            mxConfiguration, "current configuration of queue processor");
    }
    OSL_TRACE("Adding request");
    TraceRequest(rxRequest);
#endif

    maQueue.push_back(rxRequest);
    StartProcessing();
}




void ChangeRequestQueueProcessor::StartProcessing (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mnUserEventId == 0
        && mxConfiguration.is()
        && ! maQueue.empty())
    {
#ifdef VERBOSE
        OSL_TRACE("ChangeRequestQueueProcessor scheduling processing");
#endif
        mnUserEventId = Application::PostUserEvent(
            LINK(this,ChangeRequestQueueProcessor,ProcessEvent));
    }
}




IMPL_LINK(ChangeRequestQueueProcessor, ProcessEvent, void*, pUnused)
{
    (void)pUnused;

    ::osl::MutexGuard aGuard (maMutex);

    mnUserEventId = 0;

    ProcessOneEvent();

    if ( ! maQueue.empty())
    {
        // Schedule the processing of the next event.
        StartProcessing();
    }

    return 0;
}




void ChangeRequestQueueProcessor::ProcessOneEvent (void)
{
    ::osl::MutexGuard aGuard (maMutex);

#ifdef VERBOSE
    OSL_TRACE("ProcessOneEvent");
#endif

    if (mxConfiguration.is()
        && ! maQueue.empty())
    {
        // Get and remove the first entry from the queue.
        Reference<XConfigurationChangeRequest> xRequest (maQueue.front());
        maQueue.pop_front();

        // Execute the change request.
        if (xRequest.is())
        {
#ifdef VERBOSE
            TraceRequest(xRequest);
#endif
            xRequest->execute(mxConfiguration);
        }

        if (maQueue.empty())
        {
#ifdef VERBOSE
            OSL_TRACE("All requests are processed");
#endif
            // The queue is empty so tell the ConfigurationManager to update
            // its state.
            if (mpConfigurationUpdater.get() != NULL)
                mpConfigurationUpdater->RequestUpdate(mxConfiguration);
        }
    }
}




bool ChangeRequestQueueProcessor::IsEmpty (void) const
{
    return maQueue.empty();
}




void ChangeRequestQueueProcessor::ProcessUntilEmpty (void)
{
    while ( ! IsEmpty())
        ProcessOneEvent();
}




void ChangeRequestQueueProcessor::Clear (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    maQueue.clear();
}


} } // end of namespace sd::framework::configuration
