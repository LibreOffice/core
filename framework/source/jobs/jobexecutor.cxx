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

#include <jobs/job.hxx>
#include <jobs/joburl.hxx>
#include <jobs/configaccess.hxx>
#include <classes/converter.hxx>
#include <general.h>
#include <stdtypes.h>

#include <helper/mischelper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventListener.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configpaths.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>

using namespace framework;

namespace {

typedef cppu::WeakComponentImplHelper<
          css::lang::XServiceInfo
        , css::task::XJobExecutor
        , css::container::XContainerListener // => lang.XEventListener
        , css::document::XEventListener >
    Base;

/**
    @short  implements a job executor, which can be triggered from any code
    @descr  It uses the given trigger event to locate any registered job service
            inside the configuration and execute it. Of course it controls the
            lifetime of such jobs too.
 */
class JobExecutor : private cppu::BaseMutex, public Base
{
private:

    /** reference to the uno service manager */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /** cached list of all registered event names of cfg for call optimization. */
    std::vector<OUString> m_lEvents;

    /** we listen at the configuration for changes at the event list. */
    ConfigAccess m_aConfig;

    /** helper to allow us listen to the configuration without a cyclic dependency */
    css::uno::Reference<css::container::XContainerListener> m_xConfigListener;

    virtual void SAL_CALL disposing() final override;

public:

    explicit JobExecutor(const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~JobExecutor() override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.framework.JobExecutor");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.task.JobExecutor"};
    }

    // task.XJobExecutor
    virtual void SAL_CALL trigger( const OUString& sEvent ) override;

    /// Initialization function after having acquire()'d.
    void initListeners();

    // document.XEventListener
    virtual void SAL_CALL notifyEvent( const css::document::EventObject& aEvent ) override;

    // container.XContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& aEvent ) override;
    virtual void SAL_CALL elementRemoved ( const css::container::ContainerEvent& aEvent ) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& aEvent ) override;

    // lang.XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;
};

/**
    @short      standard ctor
    @descr      It initialize this new instance.

    @param      xContext
                    reference to the uno service manager
 */
JobExecutor::JobExecutor( /*IN*/ const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : Base                (m_aMutex)
    , m_xContext          (xContext                                                        )
    , m_aConfig           (xContext, "/org.openoffice.Office.Jobs/Events")
{
}

void JobExecutor::initListeners()
{
    if (utl::ConfigManager::IsFuzzing())
        return;

    // read the list of all currently registered events inside configuration.
    // e.g. "/org.openoffice.Office.Jobs/Events/<event name>"
    // We need it later to check if an incoming event request can be executed successfully
    // or must be rejected. It's an optimization! Of course we must implement updating of this
    // list too ... Be listener at the configuration.

    m_aConfig.open(ConfigAccess::E_READONLY);
    if (m_aConfig.getMode() == ConfigAccess::E_READONLY)
    {
        css::uno::Reference< css::container::XNameAccess > xRegistry(
                m_aConfig.cfg(), css::uno::UNO_QUERY);
        if (xRegistry.is())
            m_lEvents = Converter::convert_seqOUString2OUStringList(
                    xRegistry->getElementNames());

        css::uno::Reference< css::container::XContainer > xNotifier(
                m_aConfig.cfg(), css::uno::UNO_QUERY);
        if (xNotifier.is())
        {
            m_xConfigListener = new WeakContainerListener(this);
            xNotifier->addContainerListener(m_xConfigListener);
        }

        // don't close cfg here!
        // It will be done inside disposing ...
    }
}

JobExecutor::~JobExecutor()
{
    disposing();
}

void JobExecutor::disposing() {
    css::uno::Reference<css::container::XContainer> notifier;
    css::uno::Reference<css::container::XContainerListener> listener;
    {
        osl::MutexGuard g(rBHelper.rMutex);
        if (m_aConfig.getMode() != ConfigAccess::E_CLOSED) {
            notifier.set(m_aConfig.cfg(), css::uno::UNO_QUERY);
            listener = m_xConfigListener;
            m_aConfig.close();
        }
        m_xConfigListener.clear();
    }
    if (notifier.is()) {
        notifier->removeContainerListener(listener);
    }
}

/**
    @short  implementation of XJobExecutor interface
    @descr  We use the given event to locate any registered job inside our configuration
            and execute it. Further we control the lifetime of it and suppress
            shutdown of the office till all jobs was finished.

    @param  sEvent
                is used to locate registered jobs
 */
void SAL_CALL JobExecutor::trigger( const OUString& sEvent )
{
    SAL_INFO( "fwk", "JobExecutor::trigger()");

    std::vector< OUString > lJobs;

    /* SAFE */ {
    osl::MutexGuard g(rBHelper.rMutex);

    // Optimization!
    // Check if the given event name exist inside configuration and reject wrong requests.
    // This optimization suppress using of the cfg api for getting event and job descriptions ...
    if (std::find(m_lEvents.begin(), m_lEvents.end(), sEvent) == m_lEvents.end())
        return;

    // get list of all enabled jobs
    // The called static helper methods read it from the configuration and
    // filter disabled jobs using it's time stamp values.
    lJobs = JobData::getEnabledJobsForEvent(m_xContext, sEvent);
    } /* SAFE */

    // step over all enabled jobs and execute it
    size_t c = lJobs.size();
    for (size_t j=0; j<c; ++j)
    {
        rtl::Reference<Job> pJob;

        /* SAFE */
        {
            SolarMutexGuard g2;

            JobData aCfg(m_xContext);
            aCfg.setEvent(sEvent, lJobs[j]);
            aCfg.setEnvironment(JobData::E_EXECUTION);

            /*Attention!
                Jobs implements interfaces and dies by ref count!
                And freeing of such uno object is done by uno itself.
                So we have to use dynamic memory everytimes.
             */
            pJob = new Job(m_xContext, css::uno::Reference< css::frame::XFrame >());
            pJob->setJobData(aCfg);
        } /* SAFE */

        pJob->execute(css::uno::Sequence< css::beans::NamedValue >());
    }
}

void SAL_CALL JobExecutor::notifyEvent( const css::document::EventObject& aEvent )
{
    OUString EVENT_ON_DOCUMENT_OPENED("onDocumentOpened");   // Job UI  event : OnNew    or OnLoad
    OUString EVENT_ON_DOCUMENT_ADDED("onDocumentAdded");     // Job API event : OnCreate or OnLoadFinished

    OUString aModuleIdentifier;
    ::std::vector< JobData::TJob2DocEventBinding > lJobs;

    /* SAFE */ {
    osl::MutexGuard g(rBHelper.rMutex);

    // Optimization!
    // Check if the given event name exist inside configuration and reject wrong requests.
    // This optimization suppress using of the cfg api for getting event and job descriptions.
    // see using of m_lEvents.find() below ...

    // retrieve event context from event source
    try
    {
        aModuleIdentifier = css::frame::ModuleManager::create( m_xContext )->identify( aEvent.Source );
    }
    catch( const css::uno::Exception& )
    {}

    // Special feature: If the events "OnNew" or "OnLoad" occurs - we generate our own event "onDocumentOpened".
    if (
        (aEvent.EventName == "OnNew") ||
        (aEvent.EventName == "OnLoad")
       )
    {
        if (std::find(m_lEvents.begin(), m_lEvents.end(), EVENT_ON_DOCUMENT_OPENED) != m_lEvents.end())
            JobData::appendEnabledJobsForEvent(m_xContext, EVENT_ON_DOCUMENT_OPENED, lJobs);
    }

    // Special feature: If the events "OnCreate" or "OnLoadFinished" occurs - we generate our own event "onDocumentAdded".
    if (
        (aEvent.EventName == "OnCreate") ||
        (aEvent.EventName == "OnLoadFinished")
       )
    {
        if (std::find(m_lEvents.begin(), m_lEvents.end(), EVENT_ON_DOCUMENT_ADDED) != m_lEvents.end())
            JobData::appendEnabledJobsForEvent(m_xContext, EVENT_ON_DOCUMENT_ADDED, lJobs);
    }

    // Add all jobs for "real" notified event too .-)
    if (std::find(m_lEvents.begin(), m_lEvents.end(), aEvent.EventName) != m_lEvents.end())
        JobData::appendEnabledJobsForEvent(m_xContext, aEvent.EventName, lJobs);
    } /* SAFE */

    // step over all enabled jobs and execute it
    for (auto const& lJob : lJobs)
    {
        rtl::Reference<Job> pJob;

        /* SAFE */ {
        SolarMutexGuard g2;

        const JobData::TJob2DocEventBinding& rBinding = lJob;

        JobData aCfg(m_xContext);
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
        pJob = new Job(m_xContext, xModel);
        pJob->setJobData(aCfg);
        } /* SAFE */

        pJob->execute(css::uno::Sequence< css::beans::NamedValue >());
    }
}

void SAL_CALL JobExecutor::elementInserted( const css::container::ContainerEvent& aEvent )
{
    OUString sValue;
    if (aEvent.Accessor >>= sValue)
    {
        OUString sEvent = ::utl::extractFirstFromConfigurationPath(sValue);
        if (!sEvent.isEmpty())
        {
            std::vector<OUString>::iterator pEvent = std::find(m_lEvents.begin(), m_lEvents.end(), sEvent);
            if (pEvent == m_lEvents.end())
                m_lEvents.push_back(sEvent);
        }
    }
}

void SAL_CALL JobExecutor::elementRemoved ( const css::container::ContainerEvent& aEvent )
{
    OUString sValue;
    if (aEvent.Accessor >>= sValue)
    {
        OUString sEvent = ::utl::extractFirstFromConfigurationPath(sValue);
        if (!sEvent.isEmpty())
        {
            std::vector<OUString>::iterator pEvent = std::find(m_lEvents.begin(), m_lEvents.end(), sEvent);
            if (pEvent != m_lEvents.end())
                m_lEvents.erase(pEvent);
        }
    }
}

void SAL_CALL JobExecutor::elementReplaced( const css::container::ContainerEvent& )
{
    // I'm not interested on changed items :-)
}

/** @short  the used cfg changes notifier wish to be released in its reference.

    @descr  We close our internal used configuration instance to
            free this reference.

    @attention  For the special feature "bind global document event broadcaster to job execution"
                this job executor instance was registered from outside code as
                css.document.XEventListener. So it can be, that this disposing call comes from
                the global event broadcaster service. But we don't hold any reference to this service
                which can or must be released. Because this broadcaster itself is an one instance service
                too, we can ignore this request. On the other side we must release our internal CFG
                reference ... SOLUTION => check the given event source and react only, if it's our internal
                hold configuration object!
 */
void SAL_CALL JobExecutor::disposing( const css::lang::EventObject& aEvent )
{
    /* SAFE { */
    osl::MutexGuard g(rBHelper.rMutex);
    css::uno::Reference< css::uno::XInterface > xCFG(m_aConfig.cfg(), css::uno::UNO_QUERY);
    if (
        (xCFG                == aEvent.Source        ) &&
        (m_aConfig.getMode() != ConfigAccess::E_CLOSED)
       )
    {
        m_aConfig.close();
    }
    /* } SAFE */
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(
            static_cast<cppu::OWeakObject *>(new JobExecutor(context)))
    {
        // 2nd phase initialization needed
        static_cast<JobExecutor *>(static_cast<cppu::OWeakObject *>
                (instance.get()))->initListeners();
    }

    rtl::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_JobExecutor_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
