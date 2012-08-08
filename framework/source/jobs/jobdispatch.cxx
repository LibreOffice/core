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

#include <jobs/jobdispatch.hxx>
#include <jobs/joburl.hxx>
#include <jobs/job.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/resetableguard.hxx>
#include <classes/converter.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

DEFINE_XINTERFACE_6( JobDispatch                                     ,
                     OWeakObject                                     ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                     DIRECT_INTERFACE(css::frame::XDispatchProvider ),
                     DIRECT_INTERFACE(css::lang::XInitialization ),
                     DIRECT_INTERFACE(css::lang::XServiceInfo),
                     DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                     DIRECT_INTERFACE(css::frame::XDispatch         )
                   )

DEFINE_XTYPEPROVIDER_6( JobDispatch                   ,
                        css::lang::XTypeProvider      ,
                        css::frame::XDispatchProvider ,
                        css::frame::XNotifyingDispatch,
                        css::lang::XInitialization,
                        css::lang::XServiceInfo,
                        css::frame::XDispatch
                      )

DEFINE_XSERVICEINFO_MULTISERVICE( JobDispatch                   ,
                                  ::cppu::OWeakObject           ,
                                  SERVICENAME_PROTOCOLHANDLER   ,
                                  IMPLEMENTATIONNAME_JOBDISPATCH
                                )

DEFINE_INIT_SERVICE( JobDispatch,
                     {
                         /*Attention
                             I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                             to create a new instance of this class by our own supported service factory.
                             see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                         */
                     }
                   )

//________________________________
/**
    @short      standard ctor
    @descr      It initialize this new instance.

    @param      xSMGR
                    reference to the uno service manager
*/
JobDispatch::JobDispatch( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(&Application::GetSolarMutex())
    , OWeakObject   (                             )
    , m_xSMGR       (xSMGR                        )
{
}

//________________________________
/**
    @short  let this instance die
    @descr  We have to release all used resources and free used memory.
*/
JobDispatch::~JobDispatch()
{
    // release all used resources
    m_xSMGR  = css::uno::Reference< css::lang::XMultiServiceFactory >();
    m_xFrame = css::uno::Reference< css::frame::XFrame >();
}

//________________________________
/**
    @short  implementation of XInitalization
    @descr  A protocol handler can provide this functionality, if it wish to get additional informations
            about the context it runs. In this case the frame reference would be given by the outside code.

    @param  lArguments
                the list of initialization arguments
                First parameter should be the frame reference we need.
*/
void SAL_CALL JobDispatch::initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) throw(css::uno::Exception       ,
                                                                                                     css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    for (int a=0; a<lArguments.getLength(); ++a)
    {
        if (a==0)
        {
            lArguments[a] >>= m_xFrame;

            css::uno::Reference< css::frame::XModuleManager > xModuleManager(
                m_xSMGR->createInstance(
                    SERVICENAME_MODULEMANAGER ),
                css::uno::UNO_QUERY_THROW );
            try
            {
                m_sModuleIdentifier = xModuleManager->identify( m_xFrame );
            }
            catch( const css::uno::Exception& )
            {}
        }
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  implementation of XDispatchProvider::queryDispatches()
    @descr  Every protocol handler will be asked for his agreement, if an URL was queried
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
                                                                                  /*IN*/ const ::rtl::OUString& /*sTargetFrameName*/ ,
                                                                                  /*IN*/       sal_Int32        /*nSearchFlags*/     ) throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    JobURL aAnalyzedURL(aURL.Complete);
    if (aAnalyzedURL.isValid())
        xDispatch = css::uno::Reference< css::frame::XDispatch >( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    return xDispatch;
}

//________________________________
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
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL JobDispatch::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw(css::uno::RuntimeException)
{
    // don't pack resulting list!
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatches(nCount);

    for (sal_Int32 i=0; i<nCount; ++i)
        lDispatches[i] = queryDispatch( lDescriptor[i].FeatureURL  ,
                                        lDescriptor[i].FrameName   ,
                                        lDescriptor[i].SearchFlags );
    return lDispatches;
}

//________________________________
/**
    @short  implementation of XNotifyingDispatch::dispatchWithNotification()
    @descr  It creates the job service implementation and call execute on it.
            Further it starts the life time control of it. (important for async job)
            For synchonrous job we react for the returned result directly ... for asynchronous
            ones we do it later inside our callback method. But we use the same impl method
            doing that to share the code. (see impl_finishJob())

            If a job is already running, (it can only occure for asynchronous jobs)
            don't start the same job a second time. Queue in the given dispatch parameter
            and return immediatly. If the current running job call us back, we will start this
            new dispatch request.
            If no job is running - queue the parameter too! But then start the new job immediatly.
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
                                                     /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException)
{
    JobURL aAnalyzedURL(aURL.Complete);
    if (aAnalyzedURL.isValid())
    {
        ::rtl::OUString sRequest;
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

//________________________________
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
void JobDispatch::impl_dispatchEvent( /*IN*/ const ::rtl::OUString&                                            sEvent    ,
                                      /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                      /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // get list of all enabled jobs
    // The called static helper methods read it from the configuration and
    // filter disabled jobs using it's time stamp values.
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Sequence< ::rtl::OUString > lJobs = JobData::getEnabledJobsForEvent(m_xSMGR, sEvent);
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDispatchResultListener > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // no jobs ... no execution
    // But a may given listener will know something ...
    // I think this operaton was finished successfully.
    // It's not realy an error, if no registered jobs could be located.
    // Step over all found jobs and execute it
    int nExecutedJobs=0;
    for (int j=0; j<lJobs.getLength(); ++j)
    {
        /* SAFE { */
        aReadLock.lock();

        JobData aCfg(m_xSMGR);
        aCfg.setEvent(sEvent, lJobs[j]);
        aCfg.setEnvironment(JobData::E_DISPATCH);
        const bool bIsEnabled=aCfg.hasCorrectContext(m_sModuleIdentifier);

        /*Attention!
            Jobs implements interfaces and dies by ref count!
            And freeing of such uno object is done by uno itself.
            So we have to use dynamic memory everytimes.
         */
        Job* pJob = new Job(m_xSMGR, m_xFrame);
        css::uno::Reference< css::uno::XInterface > xJob(static_cast< ::cppu::OWeakObject* >(pJob), css::uno::UNO_QUERY);
        pJob->setJobData(aCfg);

        aReadLock.unlock();
        /* } SAFE */

        if (!bIsEnabled)
            continue;

        // Special mode for listener.
        // We dont notify it directly here. We delegate that
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

//________________________________
/**
    @short  dispatch a service
    @descr  We use the given name only to create and if possible to initialize
            it as an uno service. It can be usefully for creating (caching?)
            of e.g. one instance services.

    @param  sService
                the uno implementation or service name of the job, which should be instanciated

    @param  lArgs
                optional arguments for this request
                Currently not used!

    @param  xListener
                an interested listener for possible results of this operation
*/
void JobDispatch::impl_dispatchService( /*IN*/ const ::rtl::OUString&                                            sService  ,
                                        /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                        /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    JobData aCfg(m_xSMGR);
    aCfg.setService(sService);
    aCfg.setEnvironment(JobData::E_DISPATCH);

    /*Attention!
        Jobs implements interfaces and dies by ref count!
        And freeing of such uno object is done by uno itself.
        So we have to use dynamic memory everytimes.
     */
    Job* pJob = new Job(m_xSMGR, m_xFrame);
    css::uno::Reference< css::uno::XInterface > xJob(static_cast< ::cppu::OWeakObject* >(pJob), css::uno::UNO_QUERY);
    pJob->setJobData(aCfg);

    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDispatchResultListener > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // Special mode for listener.
    // We dont notify it directly here. We delegate that
    // to the job implementation. But we must set ourself there too.
    // Because this job must fake the source address of the event.
    // Otherwise the listener may ignore it.
    if (xListener.is())
        pJob->setDispatchResultFake(xListener, xThis);
    pJob->execute(Converter::convert_seqPropVal2seqNamedVal(lArgs));
}

//________________________________
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
void JobDispatch::impl_dispatchAlias( /*IN*/ const ::rtl::OUString&                                            sAlias    ,
                                      /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >&            lArgs     ,
                                      /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    JobData aCfg(m_xSMGR);
    aCfg.setAlias(sAlias);
    aCfg.setEnvironment(JobData::E_DISPATCH);

    /*Attention!
        Jobs implements interfaces and dies by ref count!
        And freeing of such uno object is done by uno itself.
        So we have to use dynamic memory everytimes.
     */
    Job* pJob = new Job(m_xSMGR, m_xFrame);
    css::uno::Reference< css::uno::XInterface > xJob(static_cast< ::cppu::OWeakObject* >(pJob), css::uno::UNO_QUERY);
    pJob->setJobData(aCfg);

    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDispatchResultListener > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // Special mode for listener.
    // We dont notify it directly here. We delegate that
    // to the job implementation. But we must set ourself there too.
    // Because this job must fake the source address of the event.
    // Otherwise the listener may ignore it.
    if (xListener.is())
        pJob->setDispatchResultFake(xListener, xThis);
    pJob->execute(Converter::convert_seqPropVal2seqNamedVal(lArgs));
}

//________________________________
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
                                     /*IN*/ const css::uno::Sequence< css::beans::PropertyValue >& lArgs ) throw(css::uno::RuntimeException)
{
    dispatchWithNotification(aURL, lArgs, css::uno::Reference< css::frame::XDispatchResultListener >());
}

//________________________________
/**
    @short  not supported
*/
void SAL_CALL JobDispatch::addStatusListener( /*IN*/ const css::uno::Reference< css::frame::XStatusListener >&,
                                              /*IN*/ const css::util::URL&                                      ) throw(css::uno::RuntimeException)
{
}

//________________________________
/**
    @short  not supported
*/
void SAL_CALL JobDispatch::removeStatusListener( /*IN*/ const css::uno::Reference< css::frame::XStatusListener >&,
                                                 /*IN*/ const css::util::URL&                                          ) throw(css::uno::RuntimeException)
{
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
