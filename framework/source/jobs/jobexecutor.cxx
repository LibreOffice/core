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

#include <jobs/jobexecutor.hxx>
#include <jobs/job.hxx>
#include <jobs/joburl.hxx>

#include <classes/converter.hxx>
#include <threadhelp/transactionguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <general.h>
#include <services.h>

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <unotools/configpaths.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

#include <rtl/logfile.hxx>

namespace framework{

DEFINE_XINTERFACE_6( JobExecutor                                                               ,
                     OWeakObject                                                               ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider                                ),
                     DIRECT_INTERFACE(css::lang::XServiceInfo                                 ),
                     DIRECT_INTERFACE(css::task::XJobExecutor                                 ),
                     DIRECT_INTERFACE(css::container::XContainerListener                      ),
                     DIRECT_INTERFACE(css::document::XEventListener                           ),
                     DERIVED_INTERFACE(css::lang::XEventListener,css::document::XEventListener)
                   )

DEFINE_XTYPEPROVIDER_6( JobExecutor                       ,
                        css::lang::XTypeProvider          ,
                        css::lang::XServiceInfo           ,
                        css::task::XJobExecutor           ,
                        css::container::XContainerListener,
                        css::document::XEventListener     ,
                        css::lang::XEventListener
                      )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE( JobExecutor                   ,
                                        ::cppu::OWeakObject           ,
                                        DECLARE_ASCII("com.sun.star.task.JobExecutor"),
                                        IMPLEMENTATIONNAME_JOBEXECUTOR
                                      )

DEFINE_INIT_SERVICE( JobExecutor,
                     {
                         m_xModuleManager = css::frame::ModuleManager::create( comphelper::getComponentContext(m_xSMGR) );

                         /*Attention
                             I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                             to create a new instance of this class by our own supported service factory.
                             see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                         */
                        // read the list of all currently registered events inside configuration.
                        // e.g. "/org.openoffice.Office.Jobs/Events/<event name>"
                        // We need it later to check if an incoming event request can be executed successfully
                        // or must be rejected. It's an optimization! Of course we must implement updating of this
                        // list too ... Be listener at the configuration.

                        m_aConfig.open(ConfigAccess::E_READONLY);
                        if (m_aConfig.getMode() == ConfigAccess::E_READONLY)
                        {
                            css::uno::Reference< css::container::XNameAccess > xRegistry(m_aConfig.cfg(), css::uno::UNO_QUERY);
                            if (xRegistry.is())
                                m_lEvents = Converter::convert_seqOUString2OUStringList(xRegistry->getElementNames());

                            css::uno::Reference< css::container::XContainer > xNotifier(m_aConfig.cfg(), css::uno::UNO_QUERY);
                            if (xNotifier.is())
                            {
                                m_xConfigListener = new WeakContainerListener(this);
                                xNotifier->addContainerListener(m_xConfigListener);
                            }

                            // don't close cfg here!
                            // It will be done inside disposing ...
                        }
                     }
                   )

//________________________________

/**
    @short      standard ctor
    @descr      It initialize this new instance.

    @param      xSMGR
                    reference to the uno service manager
 */
JobExecutor::JobExecutor( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase      (&Application::GetSolarMutex()                                   )
    , ::cppu::OWeakObject (                                                                )
    , m_xSMGR             (xSMGR                                                           )
    , m_xModuleManager    (                                                                )
    , m_aConfig           (xSMGR, ::rtl::OUString::createFromAscii(JobData::EVENTCFG_ROOT) )
{
    // Don't do any reference related code here! Do it inside special
    // impl_ method() ... see DEFINE_INIT_SERVICE() macro for further informations.
}

JobExecutor::~JobExecutor()
{
    css::uno::Reference< css::container::XContainer > xNotifier(m_aConfig.cfg(), css::uno::UNO_QUERY);
    if (xNotifier.is())
        xNotifier->removeContainerListener(m_xConfigListener);
}

//________________________________

/**
    @short  implementation of XJobExecutor interface
    @descr  We use the given event to locate any registered job inside our configuration
            and execute it. Further we control the lifetime of it and supress
            shutdown of the office till all jobs was finished.

    @param  sEvent
                is used to locate registered jobs
 */
void SAL_CALL JobExecutor::trigger( const ::rtl::OUString& sEvent ) throw(css::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT(aLog, "fwk (as96863) JobExecutor::trigger()");

    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // Optimization!
    // Check if the given event name exist inside configuration and reject wrong requests.
    // This optimization supress using of the cfg api for getting event and job descriptions ...
    if (m_lEvents.find(sEvent) == m_lEvents.end())
        return;

    // get list of all enabled jobs
    // The called static helper methods read it from the configuration and
    // filter disabled jobs using it's time stamp values.
    css::uno::Sequence< ::rtl::OUString > lJobs = JobData::getEnabledJobsForEvent(m_xSMGR, sEvent);

    aReadLock.unlock();
    /* } SAFE */

    // step over all enabled jobs and execute it
    sal_Int32 c = lJobs.getLength();
    for (sal_Int32 j=0; j<c; ++j)
    {
        /* SAFE { */
        aReadLock.lock();

        JobData aCfg(m_xSMGR);
        aCfg.setEvent(sEvent, lJobs[j]);
        aCfg.setEnvironment(JobData::E_EXECUTION);

        /*Attention!
            Jobs implements interfaces and dies by ref count!
            And freeing of such uno object is done by uno itself.
            So we have to use dynamic memory everytimes.
         */
        Job* pJob = new Job(m_xSMGR, css::uno::Reference< css::frame::XFrame >());
        css::uno::Reference< css::uno::XInterface > xJob(static_cast< ::cppu::OWeakObject* >(pJob), css::uno::UNO_QUERY);
        pJob->setJobData(aCfg);

        aReadLock.unlock();
        /* } SAFE */

        pJob->execute(css::uno::Sequence< css::beans::NamedValue >());
    }
}

//________________________________

void SAL_CALL JobExecutor::notifyEvent( const css::document::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    const char EVENT_ON_NEW[] = "OnNew";                            // Doc UI  event
    const char EVENT_ON_LOAD[] = "OnLoad";                          // Doc UI  event
    const char EVENT_ON_CREATE[] = "OnCreate";                      // Doc API event
    const char EVENT_ON_LOAD_FINISHED[] = "OnLoadFinished";         // Doc API event
    ::rtl::OUString EVENT_ON_DOCUMENT_OPENED("onDocumentOpened");   // Job UI  event : OnNew    or OnLoad
    ::rtl::OUString EVENT_ON_DOCUMENT_ADDED("onDocumentAdded");     // Job API event : OnCreate or OnLoadFinished

    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    ::comphelper::SequenceAsVector< JobData::TJob2DocEventBinding > lJobs;

    // Optimization!
    // Check if the given event name exist inside configuration and reject wrong requests.
    // This optimization supress using of the cfg api for getting event and job descriptions.
    // see using of m_lEvents.find() below ...

    // retrieve event context from event source
    rtl::OUString aModuleIdentifier;
    try
    {
        aModuleIdentifier = m_xModuleManager->identify( aEvent.Source );
    }
    catch( const css::uno::Exception& )
    {}

    // Special feature: If the events "OnNew" or "OnLoad" occures - we generate our own event "onDocumentOpened".
    if (
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_NEW))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_LOAD)))
       )
    {
        if (m_lEvents.find(EVENT_ON_DOCUMENT_OPENED) != m_lEvents.end())
            JobData::appendEnabledJobsForEvent(m_xSMGR, EVENT_ON_DOCUMENT_OPENED, lJobs);
    }

    // Special feature: If the events "OnCreate" or "OnLoadFinished" occures - we generate our own event "onDocumentAdded".
    if (
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_CREATE))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_LOAD_FINISHED)))
       )
    {
        if (m_lEvents.find(EVENT_ON_DOCUMENT_ADDED) != m_lEvents.end())
            JobData::appendEnabledJobsForEvent(m_xSMGR, EVENT_ON_DOCUMENT_ADDED, lJobs);
    }

    // Add all jobs for "real" notified event too .-)
    if (m_lEvents.find(aEvent.EventName) != m_lEvents.end())
        JobData::appendEnabledJobsForEvent(m_xSMGR, aEvent.EventName, lJobs);

    aReadLock.unlock();
    /* } SAFE */

    // step over all enabled jobs and execute it
    ::comphelper::SequenceAsVector< JobData::TJob2DocEventBinding >::const_iterator pIt;
    for (  pIt  = lJobs.begin();
           pIt != lJobs.end()  ;
         ++pIt                 )
    {
        /* SAFE { */
        aReadLock.lock();

        const JobData::TJob2DocEventBinding& rBinding = *pIt;

        JobData aCfg(m_xSMGR);
        aCfg.setEvent(rBinding.m_sDocEvent, rBinding.m_sJobName);
        aCfg.setEnvironment(JobData::E_DOCUMENTEVENT);

        if (!aCfg.hasCorrectContext(aModuleIdentifier))
            continue;

        /*Attention!
            Jobs implements interfaces and dies by ref count!
            And freeing of such uno object is done by uno itself.
            So we have to use dynamic memory everytimes.
         */
        css::uno::Reference< css::frame::XModel > xModel(aEvent.Source, css::uno::UNO_QUERY);
        Job* pJob = new Job(m_xSMGR, xModel);
        css::uno::Reference< css::uno::XInterface > xJob(static_cast< ::cppu::OWeakObject* >(pJob), css::uno::UNO_QUERY);
        pJob->setJobData(aCfg);

        aReadLock.unlock();
        /* } SAFE */

        pJob->execute(css::uno::Sequence< css::beans::NamedValue >());
    }
}

//________________________________

void SAL_CALL JobExecutor::elementInserted( const css::container::ContainerEvent& aEvent ) throw(css::uno::RuntimeException)
{
    ::rtl::OUString sValue;
    if (aEvent.Accessor >>= sValue)
    {
        ::rtl::OUString sEvent = ::utl::extractFirstFromConfigurationPath(sValue);
        if (!sEvent.isEmpty())
        {
            OUStringList::iterator pEvent = m_lEvents.find(sEvent);
            if (pEvent == m_lEvents.end())
                m_lEvents.push_back(sEvent);
        }
    }
}

void SAL_CALL JobExecutor::elementRemoved ( const css::container::ContainerEvent& aEvent ) throw(css::uno::RuntimeException)
{
    ::rtl::OUString sValue;
    if (aEvent.Accessor >>= sValue)
    {
        ::rtl::OUString sEvent = ::utl::extractFirstFromConfigurationPath(sValue);
        if (!sEvent.isEmpty())
        {
            OUStringList::iterator pEvent = m_lEvents.find(sEvent);
            if (pEvent != m_lEvents.end())
                m_lEvents.erase(pEvent);
        }
    }
}

void SAL_CALL JobExecutor::elementReplaced( const css::container::ContainerEvent& ) throw(css::uno::RuntimeException)
{
    // I'm not interested on changed items :-)
}

//________________________________

/** @short  the used cfg changes notifier wish to be released in its reference.

    @descr  We close our internal used configuration instance to
            free this reference.

    @attention  For the special feature "bind global document event broadcaster to job execution"
                this job executor instance was registered from outside code as
                css.document.XEventListener. So it can be, that this disposing call comes from
                the global event broadcaster service. But we don't hold any reference to this service
                which can or must be released. Because this broadcaster itself is an one instance service
                too, we can ignore this request. On the other side we must relase our internal CFG
                reference ... SOLUTION => check the given event source and react only, if it's our internal
                hold configuration object!
 */
void SAL_CALL JobExecutor::disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XInterface > xCFG(m_aConfig.cfg(), css::uno::UNO_QUERY);
    if (
        (xCFG                == aEvent.Source        ) &&
        (m_aConfig.getMode() != ConfigAccess::E_CLOSED)
       )
    {
        m_aConfig.close();
    }
    aReadLock.unlock();
    /* } SAFE */
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
