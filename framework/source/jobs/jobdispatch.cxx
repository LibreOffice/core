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

#include <jobs/configaccess.hxx>
#include <jobs/joburl.hxx>
#include <jobs/job.hxx>
#include <classes/converter.hxx>

#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <utility>
#include <vcl/svapp.hxx>

using namespace framework;

namespace {

/**
    @short  implements a dispatch object for jobs
    @descr  Such dispatch object will be used by the generic dispatch mechanism if
            a URL "vnd.sun.star.job:alias=<name>" occurs.
            Then an instance of this class will be created and used.
            This new instance will be called within his method
            dispatch() or dispatchWithNotification() for executing the
            real job. We do it, control the life cycle of this internal
            wrapped job and inform any interested listener if it finish.
 */
class JobDispatch : public  ::cppu::WeakImplHelper<
                            css::lang::XServiceInfo
                          , css::lang::XInitialization
                          , css::frame::XDispatchProvider
                          , css::frame::XNotifyingDispatch >      // => XDispatch
{
private:

    /** reference to the uno service manager */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /** reference to the frame, inside which this dispatch is used */
    css::uno::Reference< css::frame::XFrame > m_xFrame;

    /** name of module (writer, impress etc.) the frame is for */
    OUString m_sModuleIdentifier;

// native interface methods

public:

    explicit JobDispatch(css::uno::Reference< css::uno::XComponentContext >  xContext);
    virtual ~JobDispatch() override;

    void impl_dispatchEvent  ( const OUString&                                            sEvent    ,
                               const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                               const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
    void impl_dispatchService( const OUString&                                            sService  ,
                               const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                               const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
    void impl_dispatchAlias  ( const OUString&                                            sAlias    ,
                               const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                               const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );

public:
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.framework.jobs.JobDispatch"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.frame.ProtocolHandler"_ustr};
    }

    // Xinitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) override;

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL             ,
                                                                                                         const OUString&                                      sTargetFrameName ,
                                                                                                               sal_Int32                                             nSearchFlags     ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor      ) override;

    // XNotifyingDispatch
    virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                                    const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) override;

    // XDispatch
    virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                const css::uno::Sequence< css::beans::PropertyValue >&    lArgs     ) override;
    virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                const css::util::URL&                                     aURL      ) override;
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) override;
};

/**
    @short      standard ctor
    @descr      It initialize this new instance.

    @param      xContext
                    reference to the uno service manager
*/
JobDispatch::JobDispatch( /*IN*/ css::uno::Reference< css::uno::XComponentContext >  xContext )
    : m_xContext    (std::move(xContext                        ))
{
}

/**
    @short  let this instance die
    @descr  We have to release all used resources and free used memory.
*/
JobDispatch::~JobDispatch()
{
    // release all used resources
    m_xContext.clear();
    m_xFrame.clear();
}

/**
    @short  implementation of XInitialization
    @descr  A protocol handler can provide this functionality, if it wish to get additional information
            about the context it runs. In this case the frame reference would be given by the outside code.

    @param  lArguments
                the list of initialization arguments
                First parameter should be the frame reference we need.
*/
void SAL_CALL JobDispatch::initialize( const css::uno::Sequence< css::uno::Any >& lArguments )
{
    SolarMutexGuard g;

    for (int a=0; a<lArguments.getLength(); ++a)
    {
        if (a==0)
        {
            lArguments[a] >>= m_xFrame;

            css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
                css::frame::ModuleManager::create(m_xContext);
            try
            {
                m_sModuleIdentifier = xModuleManager->identify( m_xFrame );
            }
            catch( const css::uno::Exception& )
            {}
        }
    }
}

/**
    @short  implementation of XDispatchProvider::queryDispatches()
    @descr  Every protocol handler will be asked for his agreement, if a URL was queried
            for which this handler is registered. It's the chance for this handler to validate
            the given URL and return a dispatch object (may be itself) or not.

    @param  aURL
                the queried URL, which should be checked

    @param  sTargetFrameName
                describes the target frame, in which context this handler will be used
                Is mostly set to "", "_self", "_blank", "_default" or a non special one
                using SELF/CREATE as search flags.

    @param  nSearchFlags
                Can be SELF or CREATE only and are set only if sTargetFrameName isn't a special target
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL JobDispatch::queryDispatch( /*IN*/ const css::util::URL&  aURL             ,
                                                                                  /*IN*/ const OUString& /*sTargetFrameName*/ ,
                                                                                  /*IN*/       sal_Int32        /*nSearchFlags*/     )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    JobURL aAnalyzedURL(aURL.Complete);
    if (aAnalyzedURL.isValid())
        xDispatch = this;

    return xDispatch;
}

/**
    @short  implementation of XDispatchProvider::queryDispatches()
    @descr  It's an optimized access for remote, so you can ask for
            multiple dispatch objects at the same time.

    @param  lDescriptor
                a list of queryDispatch() parameter

    @return A list of corresponding dispatch objects.
            NULL references are not skipped. Every result
            match to one given descriptor item.
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL JobDispatch::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
{
    // don't pack resulting list!
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatches(nCount);
    auto lDispatchesRange = asNonConstRange(lDispatches);
    for (sal_Int32 i=0; i<nCount; ++i)
        lDispatchesRange[i] = queryDispatch( lDescriptor[i].FeatureURL  ,
                                        lDescriptor[i].FrameName   ,
                                        lDescriptor[i].SearchFlags );
    return lDispatches;
}

/**
    @short  implementation of XNotifyingDispatch::dispatchWithNotification()
    @descr  It creates the job service implementation and call execute on it.
            Further it starts the life time control of it. (important for async job)
            For synchronous job we react for the returned result directly ... for asynchronous
            ones we do it later inside our callback method. But we use the same impl method
            doing that to share the code. (see impl_finishJob())

            If a job is already running, (it can only occur for asynchronous jobs)
            don't start the same job a second time. Queue in the given dispatch parameter
            and return immediately. If the current running job call us back, we will start this
            new dispatch request.
            If no job is running - queue the parameter too! But then start the new job immediately.
            We have to queue it every time - because it hold us alive by ref count!

    @param  aURL
                describe the job(s), which should be started

    @param  lArgs
                optional arguments for this request

    @param  xListener
                an interested listener for possible results of this operation
*/
void SAL_CALL JobDispatch::dispatchWithNotification( /*IN*/ const css::util::URL&                                             aURL      ,
                                                     /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                                     /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    JobURL aAnalyzedURL(aURL.Complete);
    if (aAnalyzedURL.isValid())
    {
        OUString sRequest;
        if (aAnalyzedURL.getEvent(sRequest))
            impl_dispatchEvent(sRequest, lArgs, xListener);
        else
        if (aAnalyzedURL.getService(sRequest))
            impl_dispatchService(sRequest, lArgs, xListener);
        else
        if (aAnalyzedURL.getAlias(sRequest))
            impl_dispatchAlias(sRequest, lArgs, xListener);
    }
}

/**
    @short  dispatch an event
    @descr  We search all registered jobs for this event and execute it.
            After doing so, we inform the given listener about the results.
            (There will be one notify for every executed job!)

    @param  sEvent
                the event, for which jobs can be registered

    @param  lArgs
                optional arguments for this request
                Currently not used!

    @param  xListener
                an interested listener for possible results of this operation
*/
void JobDispatch::impl_dispatchEvent( /*IN*/ const OUString&                                            sEvent    ,
                                      /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                      /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // get list of all enabled jobs
    // The called static helper methods read it from the configuration and
    // filter disabled jobs using it's time stamp values.
    std::vector< OUString > lJobs = JobData::getEnabledJobsForEvent(m_xContext, sEvent);

    css::uno::Reference< css::frame::XDispatchResultListener > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // no jobs... no execution
    // But a may given listener will know something...
    // I think this operation was finished successfully.
    // It's not really an error, if no registered jobs could be located.
    // Step over all found jobs and execute it
    int nExecutedJobs=0;
    for (const OUString & lJob : lJobs)
    {
        JobData aCfg(m_xContext);
        aCfg.setEvent(sEvent, lJob);
        aCfg.setEnvironment(JobData::E_DISPATCH);
        const bool bIsEnabled=aCfg.hasCorrectContext(m_sModuleIdentifier);

        rtl::Reference<Job> pJob = new Job(m_xContext, m_xFrame);
        pJob->setJobData(aCfg);

        if (!bIsEnabled)
            continue;

        // Special mode for listener.
        // We don't notify it directly here. We delegate that
        // to the job implementation. But we must set ourself there too.
        // Because this job must fake the source address of the event.
        // Otherwise the listener may ignore it.
        if (xListener.is())
            pJob->setDispatchResultFake(xListener, xThis);
        pJob->execute(Converter::convert_seqPropVal2seqNamedVal(lArgs));
        ++nExecutedJobs;
    }

    if (nExecutedJobs<1 && xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.Source = xThis;
        aEvent.State  = css::frame::DispatchResultState::SUCCESS;
        xListener->dispatchFinished(aEvent);
    }
}

/**
    @short  dispatch a service
    @descr  We use the given name only to create and if possible to initialize
            it as a uno service. It can be useful for creating (caching?)
            of e.g. one instance services.

    @param  sService
                the uno implementation or service name of the job, which should be instantiated

    @param  lArgs
                optional arguments for this request
                Currently not used!

    @param  xListener
                an interested listener for possible results of this operation
*/
void JobDispatch::impl_dispatchService( /*IN*/ const OUString&                                            sService  ,
                                        /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                        /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    JobData aCfg(m_xContext);
    aCfg.setService(sService);
    aCfg.setEnvironment(JobData::E_DISPATCH);

    /*Attention!
        Jobs implements interfaces and dies by ref count!
        And freeing of such uno object is done by uno itself.
        So we have to use dynamic memory everytimes.
     */
    rtl::Reference<Job> pJob = new Job(m_xContext, m_xFrame);
    pJob->setJobData(aCfg);

    css::uno::Reference< css::frame::XDispatchResultListener > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // Special mode for listener.
    // We don't notify it directly here. We delegate that
    // to the job implementation. But we must set ourself there too.
    // Because this job must fake the source address of the event.
    // Otherwise the listener may ignore it.
    if (xListener.is())
        pJob->setDispatchResultFake(xListener, xThis);
    pJob->execute(Converter::convert_seqPropVal2seqNamedVal(lArgs));
}

/**
    @short  dispatch an alias
    @descr  We use this alias to locate a job inside the configuration
            and execute it. Further we inform the given listener about the results.

    @param  sAlias
                the alias name of the configured job

    @param  lArgs
                optional arguments for this request
                Currently not used!

    @param  xListener
                an interested listener for possible results of this operation
*/
void JobDispatch::impl_dispatchAlias( /*IN*/ const OUString&                                            sAlias    ,
                                      /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                      /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    JobData aCfg(m_xContext);
    aCfg.setAlias(sAlias);
    aCfg.setEnvironment(JobData::E_DISPATCH);

    rtl::Reference<Job> pJob = new Job(m_xContext, m_xFrame);
    pJob->setJobData(aCfg);

    css::uno::Reference< css::frame::XDispatchResultListener > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // Special mode for listener.
    // We don't notify it directly here. We delegate that
    // to the job implementation. But we must set ourself there too.
    // Because this job must fake the source address of the event.
    // Otherwise the listener may ignore it.
    if (xListener.is())
        pJob->setDispatchResultFake(xListener, xThis);
    pJob->execute(Converter::convert_seqPropVal2seqNamedVal(lArgs));
}

/**
    @short  implementation of XDispatch::dispatch()
    @descr  Because the methods dispatch() and dispatchWithNotification() are different in her parameters
            only, we can forward this request to dispatchWithNotification() by using an empty listener!

    @param  aURL
                describe the job(s), which should be started

    @param  lArgs
                optional arguments for this request

    @see    dispatchWithNotification()
*/
void SAL_CALL JobDispatch::dispatch( /*IN*/ const css::util::URL&                                  aURL  ,
                                     /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >& lArgs )
{
    dispatchWithNotification(aURL, lArgs, css::uno::Reference< css::frame::XDispatchResultListener >());
}

/**
    @short  not supported
*/
void SAL_CALL JobDispatch::addStatusListener( /*IN*/ const css::uno::Reference< css::frame::XStatusListener >&,
                                              /*IN*/ const css::util::URL&                                      )
{
}

/**
    @short  not supported
*/
void SAL_CALL JobDispatch::removeStatusListener( /*IN*/ const css::uno::Reference< css::frame::XStatusListener >&,
                                                 /*IN*/ const css::util::URL&                                          )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_jobs_JobDispatch_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new JobDispatch(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
