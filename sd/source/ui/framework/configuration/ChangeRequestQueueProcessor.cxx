/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChangeRequestQueueProcessor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:44:15 $
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

#include "framework/ConfigurationController.hxx"
#include "ConfigurationUpdater.hxx"

#include <vcl/svapp.hxx>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATION_HPP_
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_CONFIGURATIONCHANGEEVENT_HPP_
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#endif

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




void TraceConfiguration (const Reference<XConfiguration>& rxConfiguration)
{
    OSL_TRACE("Configuration at %p {", rxConfiguration.get());
    Sequence<ResourceId> aResourceList (rxConfiguration->getResources(ResourceId()));
    for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
    {
        OSL_TRACE("    %s",
            ::rtl::OUStringToOString(aResourceList[nIndex].ResourceURL, RTL_TEXTENCODING_UTF8).getStr());
        Sequence<ResourceId> aRelationList (
            rxConfiguration->getResources(aResourceList[nIndex]));
        for (sal_Int32 nIndex=0; nIndex<aRelationList.getLength(); ++nIndex)
        {
            OSL_TRACE("        %s",
                ::rtl::OUStringToOString(aRelationList[nIndex].ResourceURL, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    OSL_TRACE("}");
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
    mxConfiguration = rxConfiguration;
    StartProcessing();
}




void ChangeRequestQueueProcessor::AddRequest (
    const Reference<XConfigurationChangeRequest>& rxRequest)
{
#ifdef VERBOSE
    if (maQueue.empty())
    {
        OSL_TRACE("Adding requests to empty queue");
        TraceConfiguration(mxConfiguration);
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
            LINK(this,ChangeRequestQueueProcessor,ProcessOneEvent));
    }
}




IMPL_LINK(ChangeRequestQueueProcessor, ProcessOneEvent, void*, pUnused)
{
    (void)pUnused;

    ::osl::MutexGuard aGuard (maMutex);

    mnUserEventId = 0;

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
        else
        {
            // Schedule the processing of the next event.
            StartProcessing();
        }
    }

    return 0;
}




bool ChangeRequestQueueProcessor::IsEmpty (void) const
{
    return maQueue.empty();
}



} } // end of namespace sd::framework::configuration
