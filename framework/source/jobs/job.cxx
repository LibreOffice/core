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
#include <general.h>
#include <services.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/task/XAsyncJob.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

/**
    @short      standard ctor
    @descr      It initialize this new instance. But it set some generic parameters here only.
                Specialized information (e.g. the alias or service name ofthis job) will be set
                later using the method setJobData().

    @param      xContext
                reference to the uno service manager

    @param      xFrame
                reference to the frame, in which environment we run
                (May be null!)
*/
Job::Job( /*IN*/ const css::uno::Reference< css::uno::XComponentContext >& xContext  ,
          /*IN*/ const css::uno::Reference< css::frame::XFrame >&              xFrame )
    : m_aJobCfg            (xContext                     )
    , m_xContext           (xContext                     )
    , m_xFrame             (xFrame                       )
    , m_bListenOnDesktop   (false                    )
    , m_bListenOnFrame     (false                    )
    , m_bListenOnModel     (false                    )
    , m_bPendingCloseFrame (false                    )
    , m_bPendingCloseModel (false                    )
    , m_eRunState          (E_NEW                        )
{
}

/**
    @short      standard ctor
    @descr      It initialize this new instance. But it set some generic parameters here only.
                Specialized information (e.g. the alias or service name ofthis job) will be set
                later using the method setJobData().

    @param      xContext
                reference to the uno service manager

    @param      xModel
                reference to the model, in which environment we run
                (May be null!)
*/
Job::Job( /*IN*/ const css::uno::Reference< css::uno::XComponentContext >& xContext  ,
          /*IN*/ const css::uno::Reference< css::frame::XModel >&              xModel )
    : m_aJobCfg            (xContext                     )
    , m_xContext           (xContext                     )
    , m_xModel             (xModel                       )
    , m_bListenOnDesktop   (false                    )
    , m_bListenOnFrame     (false                    )
    , m_bListenOnModel     (false                    )
    , m_bPendingCloseFrame (false                    )
    , m_bPendingCloseModel (false                    )
    , m_eRunState          (E_NEW                        )
{
}

/**
    @short  superflous!
    @descr  Releasing of memory and reference must be done inside die() call.
            Otherwhise it's a bug.
*/
Job::~Job()
{
}

/**
    @short  set (or delete) a listener for sending dispatch result events
    @descr  Because this object is used in a wrapped mode ... the original listener
            for such events can't be registered here directly. Because the
            listener expect to get the original object given as source of the event.
            That's why we get this source here too, to fake(!) it at sending time!

    @param  xListener
                the original listener for dispatch result events

    @param  xSourceFake
                our user, which got the registration request for this listener
*/
void Job::setDispatchResultFake( /*IN*/ const css::uno::Reference< css::frame::XDispatchResultListener >& xListener   ,
                                 /*IN*/ const css::uno::Reference< css::uno::XInterface >&                xSourceFake )
{
    SolarMutexGuard g;

    // reject dangerous calls
    if (m_eRunState != E_NEW)
    {
        SAL_INFO("fwk", "Job::setJobData(): job may still running or already finished");
        return;
    }

    m_xResultListener   = xListener;
    m_xResultSourceFake = xSourceFake;
}

void Job::setJobData( const JobData& aData )
{
    SolarMutexGuard g;

    // reject dangerous calls
    if (m_eRunState != E_NEW)
    {
        SAL_INFO("fwk", "Job::setJobData(): job may still running or already finished");
        return;
    }

    m_aJobCfg = aData;
}

/**
    @short  runs the job
    @descr  It doesn't matter, if the job is an asynchronous or
            synchronous one. This method returns only if it was finished
            or cancelled.

    @param  lDynamicArgs
                optional arguments for job execution
                In case the represented job is a configured one (which uses static
                arguments too) all information will be merged!
*/
void Job::execute( /*IN*/ const css::uno::Sequence< css::beans::NamedValue >& lDynamicArgs )
{
    /* SAFE { */
    SolarMutexResettableGuard aWriteLock;

    // reject dangerous calls
    if (m_eRunState != E_NEW)
    {
        SAL_INFO("fwk", "Job::execute(): job may still running or already finished");
        return;
    }

    // create the environment and mark this job as running ...
    m_eRunState = E_RUNNING;
    impl_startListening();

    css::uno::Reference< css::task::XAsyncJob >  xAJob;
    css::uno::Reference< css::task::XJob >       xSJob;
    css::uno::Sequence< css::beans::NamedValue > lJobArgs = impl_generateJobArgs(lDynamicArgs);

    // It's necessary to hold us self alive!
    // Otherwhise we might die by ref count ...
    css::uno::Reference< css::task::XJobListener > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    try
    {
        // create the job
        // We must check for the supported interface on demand!
        // But we preferr the synchronous one ...
        m_xJob = m_xContext->getServiceManager()->createInstanceWithContext(m_aJobCfg.getService(), m_xContext);
        xSJob.set(m_xJob, css::uno::UNO_QUERY);
        if (!xSJob.is())
            xAJob.set(m_xJob, css::uno::UNO_QUERY);

        // execute it asynchron
        if (xAJob.is())
        {
            m_aAsyncWait.reset();
            aWriteLock.clear();
            /* } SAFE */
            xAJob->executeAsync(lJobArgs, xThis);
            // wait for finishing this job - so this method
            // does the same for synchronous and asynchronous jobs!
            m_aAsyncWait.wait();
            aWriteLock.reset();
            /* SAFE { */
            // Note: Result handling was already done inside the callback!
        }
        // execute it synchron
        else if (xSJob.is())
        {
            aWriteLock.clear();
            /* } SAFE */
            css::uno::Any aResult = xSJob->execute(lJobArgs);
            aWriteLock.reset();
            /* SAFE { */
            impl_reactForJobResult(aResult);
        }
    }
    #if OSL_DEBUG_LEVEL > 0
    catch(const css::uno::Exception& ex)
    {
        SAL_INFO("fwk", "Job::execute(): Got exception during job execution. Original Message was: \"" << ex.Message << "\"");
    }
    #else
    catch(const css::uno::Exception&)
        {}
    #endif

    // deinitialize the environment and mark this job as finished ...
    // but don't overwrite any information about STOPPED or might DISPOSED jobs!
    impl_stopListening();
    if (m_eRunState == E_RUNNING)
        m_eRunState = E_STOPPED_OR_FINISHED;

    // If we got a close request from our frame or model ...
    // but we disagreed wit that by throwing a veto exception...
    // and got the ownership ...
    // we have to close the resource frame or model now -
    // and to disable ourself!
    if (m_bPendingCloseFrame)
    {
        m_bPendingCloseFrame = false;
        css::uno::Reference< css::util::XCloseable > xClose(m_xFrame, css::uno::UNO_QUERY);
        if (xClose.is())
        {
            try
            {
                xClose->close(true);
            }
            catch(const css::util::CloseVetoException&) {}
        }
    }

    if (m_bPendingCloseModel)
    {
        m_bPendingCloseModel = false;
        css::uno::Reference< css::util::XCloseable > xClose(m_xModel, css::uno::UNO_QUERY);
        if (xClose.is())
        {
            try
            {
                xClose->close(true);
            }
            catch(const css::util::CloseVetoException&) {}
        }
    }

    aWriteLock.clear();
    /* SAFE { */

    // release this instance ...
    die();
}

/**
    @short  kill this job
    @descr  It doesn't matter if this request is called from inside or
            from outside. We release our internal structures and stop
            every activity. After doing so - this instance will not be
            useable any longer! Of course we try to handle further requests
            carefully. Maybe someone else holds a reference to us ...
*/
void Job::die()
{
    SolarMutexGuard g;

    impl_stopListening();

    if (m_eRunState != E_DISPOSED)
    {
        try
        {
            css::uno::Reference< css::lang::XComponent > xDispose(m_xJob, css::uno::UNO_QUERY);
            if (xDispose.is())
            {
                xDispose->dispose();
                m_eRunState = E_DISPOSED;
            }
        }
        catch(const css::lang::DisposedException&)
        {
            m_eRunState = E_DISPOSED;
        }
    }

    m_xJob.clear();
    m_xFrame.clear();
    m_xModel.clear();
    m_xDesktop.clear();
    m_xResultListener.clear();
    m_xResultSourceFake.clear();
    m_bPendingCloseFrame = false;
    m_bPendingCloseModel = false;
}

/**
    @short  generates list of arguments for job execute
    @descr  There exist a set of information, which can be needed by a job.
                a) it's static configuration data   (Equals for all jobs.    )
                b) it's specific configuration data (Different for every job.)
                c) some environment values          (e.g. the frame, for which this job was started)
                d) any other dynamic data           (e.g. parameters of a dispatch() request)
            We collect all these information and generate one list which include all others.

    @param  lDynamicArgs
                list of dynamic arguments (given by a corresponding dispatch() call)
                Can be empty too.

    @return A list which includes all mentioned sub lists.
*/
css::uno::Sequence< css::beans::NamedValue > Job::impl_generateJobArgs( /*IN*/ const css::uno::Sequence< css::beans::NamedValue >& lDynamicArgs )
{
    css::uno::Sequence< css::beans::NamedValue > lAllArgs;

    /* SAFE { */
    SolarMutexClearableGuard aReadLock;

    // the real structure of the returned list depends from the environment of this job!
    JobData::EMode eMode = m_aJobCfg.getMode();

    // Create list of environment variables. This list must be part of the
    // returned structure every time... but some of its members are opetional!
    css::uno::Sequence< css::beans::NamedValue > lEnvArgs(1);
    lEnvArgs[0].Name = "EnvType";
    lEnvArgs[0].Value <<= m_aJobCfg.getEnvironmentDescriptor();

    if (m_xFrame.is())
    {
        sal_Int32 c = lEnvArgs.getLength();
        lEnvArgs.realloc(c+1);
        lEnvArgs[c].Name = "Frame";
        lEnvArgs[c].Value <<= m_xFrame;
    }
    if (m_xModel.is())
    {
        sal_Int32 c = lEnvArgs.getLength();
        lEnvArgs.realloc(c+1);
        lEnvArgs[c].Name = "Model";
        lEnvArgs[c].Value <<= m_xModel;
    }
    if (eMode==JobData::E_EVENT)
    {
        sal_Int32 c = lEnvArgs.getLength();
        lEnvArgs.realloc(c+1);
        lEnvArgs[c].Name = "EventName";
        lEnvArgs[c].Value <<= m_aJobCfg.getEvent();
    }

    // get the configuration data from the job data container ... if possible
    // Means: if this job has any configuration data. Note: only really
    // filled lists will be set to the return structure at the end of this method.
    css::uno::Sequence< css::beans::NamedValue > lConfigArgs;
    std::vector< css::beans::NamedValue > lJobConfigArgs;
    if (eMode==JobData::E_ALIAS || eMode==JobData::E_EVENT)
    {
        lConfigArgs    = m_aJobCfg.getConfig();
        lJobConfigArgs = m_aJobCfg.getJobConfig();
    }

    aReadLock.clear();
    /* } SAFE */

    // Add all valid (not empty) lists to the return list
    if (lConfigArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name = "Config";
        lAllArgs[nLength].Value <<= lConfigArgs;
    }
    if (!lJobConfigArgs.empty())
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name = "JobConfig";
        lAllArgs[nLength].Value <<= comphelper::containerToSequence(lJobConfigArgs);
    }
    if (lEnvArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name = "Environment";
        lAllArgs[nLength].Value <<= lEnvArgs;
    }
    if (lDynamicArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name = "DynamicData";
        lAllArgs[nLength].Value <<= lDynamicArgs;
    }

    return lAllArgs;
}

/**
    @short  analyze the given job result and change the job configuration
    @descr  Note: Some results can be handled only, if this job has a valid configuration!
            For "not configured jobs" (means pure services) they can be ignored.
            But these cases are handled by our JobData member. We can call it every time.
            It does the right things automatically. E.g. if the job has no configuration ...
            it does nothing during setJobConfig()!

    @param  aResult
                the job result for analyzing
*/
void Job::impl_reactForJobResult( /*IN*/ const css::uno::Any& aResult )
{
    SolarMutexGuard g;

    // analyze the result set ...
    JobResult aAnalyzedResult(aResult);

    // some of the following operations will be supported for different environments
    // or different type of jobs only.
    JobData::EEnvironment eEnvironment = m_aJobCfg.getEnvironment();

    // write back the job specific configuration data ...
    // If the environment allow it and if this job has a configuration!
    if (
        (m_aJobCfg.hasConfig()                            ) &&
        (aAnalyzedResult.existPart(JobResult::E_ARGUMENTS))
       )
    {
        m_aJobCfg.setJobConfig(aAnalyzedResult.getArguments());
    }

    // disable a job for further executions.
    // Note: this option is available inside the environment EXECUTOR only
    if (
//        (eEnvironment == JobData::E_EXECUTION              ) &&
        (m_aJobCfg.hasConfig()                             ) &&
        (aAnalyzedResult.existPart(JobResult::E_DEACTIVATE))
       )
    {
        m_aJobCfg.disableJob();
    }

    // notify any interested listener with the may given result state.
    // Note: this option is available inside the environment DISPATCH only
    if (
        (eEnvironment == JobData::E_DISPATCH                   ) &&
        (m_xResultListener.is()                                ) &&
        (aAnalyzedResult.existPart(JobResult::E_DISPATCHRESULT))
       )
    {
        m_aJobCfg.setResult(aAnalyzedResult);
        // Attention: Because the listener expect that the original object send this event ...
        // and we nor the job are the right ones ...
        // our user has set itself before. So we can fake this source address!
        css::frame::DispatchResultEvent aEvent        = aAnalyzedResult.getDispatchResult();
                                        aEvent.Source = m_xResultSourceFake;
        m_xResultListener->dispatchFinished(aEvent);
    }
}

/**
    @short  starts listening for office shutdown and closing of our
            given target frame (if it's a valid reference)
    @descr  We will register ourself as terminate listener
            at the global desktop instance. That will hold us
            alive and additional we get the information, if the
            office wish to shutdown. If then an internal job
            is running we will have the chance to suppress that
            by throwing a veto exception. If our internal wrapped
            job finished his work, we can release this listener
            connection.

            Further we are listener for closing of the (possible valid)
            given frame. We must be sure, that this resource won't be gone
            if our internal job is still running.
*/
void Job::impl_startListening()
{
    SolarMutexGuard g;

    // listening for office shutdown
    if (!m_xDesktop.is() && !m_bListenOnDesktop)
    {
        try
        {
            m_xDesktop = css::frame::Desktop::create( m_xContext );
            css::uno::Reference< css::frame::XTerminateListener > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            m_xDesktop->addTerminateListener(xThis);
            m_bListenOnDesktop = true;
        }
        catch(const css::uno::Exception&)
        {
            m_xDesktop.clear();
        }
    }

    // listening for frame closing
    if (m_xFrame.is() && !m_bListenOnFrame)
    {
        try
        {
            css::uno::Reference< css::util::XCloseBroadcaster > xCloseable(m_xFrame                                 , css::uno::UNO_QUERY);
            css::uno::Reference< css::util::XCloseListener >    xThis     (static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (xCloseable.is())
            {
                xCloseable->addCloseListener(xThis);
                m_bListenOnFrame = true;
            }
        }
        catch(const css::uno::Exception&)
        {
            m_bListenOnFrame = false;
        }
    }

    // listening for model closing
    if (m_xModel.is() && !m_bListenOnModel)
    {
        try
        {
            css::uno::Reference< css::util::XCloseBroadcaster > xCloseable(m_xModel                                 , css::uno::UNO_QUERY);
            css::uno::Reference< css::util::XCloseListener >    xThis     (static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (xCloseable.is())
            {
                xCloseable->addCloseListener(xThis);
                m_bListenOnModel = true;
            }
        }
        catch(const css::uno::Exception&)
        {
            m_bListenOnModel = false;
        }
    }
}

/**
    @short  release listener connection for office shutdown
    @descr  see description of impl_startListening()
*/
void Job::impl_stopListening()
{
    SolarMutexGuard g;

    // stop listening for office shutdown
    if (m_xDesktop.is() && m_bListenOnDesktop)
    {
        try
        {
            css::uno::Reference< css::frame::XTerminateListener > xThis(static_cast< ::cppu::OWeakObject* >(this)   , css::uno::UNO_QUERY);
            m_xDesktop->removeTerminateListener(xThis);
            m_xDesktop.clear();
            m_bListenOnDesktop = false;
        }
        catch(const css::uno::Exception&)
        {
        }
    }

    // stop listening for frame closing
    if (m_xFrame.is() && m_bListenOnFrame)
    {
        try
        {
            css::uno::Reference< css::util::XCloseBroadcaster > xCloseable(m_xFrame                                 , css::uno::UNO_QUERY);
            css::uno::Reference< css::util::XCloseListener >    xThis     (static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (xCloseable.is())
            {
                xCloseable->removeCloseListener(xThis);
                m_bListenOnFrame = false;
            }
        }
        catch(const css::uno::Exception&)
        {
        }
    }

    // stop listening for model closing
    if (m_xModel.is() && m_bListenOnModel)
    {
        try
        {
            css::uno::Reference< css::util::XCloseBroadcaster > xCloseable(m_xModel                                 , css::uno::UNO_QUERY);
            css::uno::Reference< css::util::XCloseListener >    xThis     (static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (xCloseable.is())
            {
                xCloseable->removeCloseListener(xThis);
                m_bListenOnModel = false;
            }
        }
        catch(const css::uno::Exception&)
        {
        }
    }
}

/**
    @short  callback from any asynchronous executed job

    @descr  Our execute() method waits for this callback.
            We have to react for the possible results here,
            to kill the running job and disable the blocked condition
            so execute() can be finished too.

    @param  xJob
                the job, which was running and inform us now

    @param  aResult
                its results
*/
void SAL_CALL Job::jobFinished( /*IN*/ const css::uno::Reference< css::task::XAsyncJob >& xJob    ,
                                /*IN*/ const css::uno::Any&                               aResult )
{
    SolarMutexGuard g;

    // It's necessary to check this.
    // May this job was cancelled by any other reason
    // some milliseconds before. :-)
    if (m_xJob.is() && m_xJob==xJob)
    {
        // react for his results
        // (means enable/disable it for further requests
        // or save arguments or notify listener ...)
        impl_reactForJobResult(aResult);

        // Let the job die!
        m_xJob.clear();
    }

    // And let the start method "execute()" finishing it's job.
    // But do it every time. So any outside blocking code can finish
    // his work too.
    m_aAsyncWait.set();
}

/**
    @short  prevent internal wrapped job against office termination
    @descr  This event is broadcasted by the desktop instance and ask for an office termination.
            If the internal wrapped job is still in progress, we disagree with that by throwing the
            right veto exception. If not - we agree. But then we must be aware, that another event
            notifyTermination() can follow. Then we have no chance to do the same. Then we have to
            accept that and stop our work instandly.

    @param  aEvent
                describes the broadcaster and must be the desktop instance

    @throw  TerminateVetoException
                if our internal wrapped job is still running.
 */
void SAL_CALL Job::queryTermination( /*IN*/ const css::lang::EventObject& )
{
    SolarMutexGuard g;

    // Otherwhise try to close() it
    css::uno::Reference< css::util::XCloseable > xClose(m_xJob, css::uno::UNO_QUERY);
    if (xClose.is())
    {
        try
        {
            xClose->close(false);
            m_eRunState = E_STOPPED_OR_FINISHED;
        }
        catch(const css::util::CloseVetoException&) {}
    }

    if (m_eRunState != E_STOPPED_OR_FINISHED)
    {
        css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        throw css::frame::TerminationVetoException("job still in progress", xThis);
    }
}

/**
    @short  inform us about office termination
    @descr  Instead of the method queryTermination(), here is no chance to disagree with that.
            We have to accept it and cancel all current processes inside.
            It can occur only, if job was not already started if queryTermination() was called here ..
            Then we had not throwed a veto exception. But now we must agree with this situation and break
            all our internal processes. It's not a good idea to mark this instance as non startable any longer
            inside queryTermination() if no job was running too. Because that would disable this job and may
            the office does not really shutdownm, because another listener has thrown the suitable exception.

    @param  aEvent
                describes the broadcaster and must be the desktop instance
 */
void SAL_CALL Job::notifyTermination( /*IN*/ const css::lang::EventObject& )
{
    die();
    // Do nothing else here. Our internal resources was released ...
}

/**
    @short  prevent internal wrapped job against frame closing
    @descr  This event is broadcasted by the frame instance and ask for closing.
            If the internal wrapped job is still in progress, we disagree with that by throwing the
            right veto exception. If not - we agree. But then we must be aware, that another event
            notifyClosing() can follow. Then we have no chance to do the same. Then we have to
            accept that and stop our work instandly.

    @param  aEvent
                describes the broadcaster and must be the frame instance

    @param  bGetsOwnership
                If it's set to <sal_True> and we throw the right veto excepion, we have to close this frame later
                if our internal processes will be finished. If it's set to <FALSE/> we can ignore it.

    @throw  CloseVetoException
                if our internal wrapped job is still running.
 */
void SAL_CALL Job::queryClosing( const css::lang::EventObject& aEvent         ,
                                       sal_Bool                bGetsOwnership )
{
    SolarMutexGuard g;

    // do nothing, if no internal job is still running ...
    // The frame or model can be closed then successfully.
    if (m_eRunState != E_RUNNING)
        return;

    // try close() first at the job.
    // The job can agree or disagree with this request.
    css::uno::Reference< css::util::XCloseable > xClose(m_xJob, css::uno::UNO_QUERY);
    if (xClose.is())
    {
        xClose->close(bGetsOwnership);
        // Here we can say: "this job was stopped successfully". Because
        // no veto exception was thrown!
        m_eRunState = E_STOPPED_OR_FINISHED;
        return;
    }

    // try dispose() then
    // Here the job has no chance for a veto.
    // But we must be aware of an "already disposed exception"...
    try
    {
        css::uno::Reference< css::lang::XComponent > xDispose(m_xJob, css::uno::UNO_QUERY);
        if (xDispose.is())
        {
            xDispose->dispose();
            m_eRunState = E_DISPOSED;
        }
    }
    catch(const css::lang::DisposedException&)
    {
        // the job was already disposed by any other mechanism !?
        // But it's not interesting for us. For us this job is stopped now.
        m_eRunState = E_DISPOSED;
    }

    if (m_eRunState != E_DISPOSED)
    {
        // analyze event source - to find out, which resource called queryClosing() at this
        // job wrapper. We must bind a "pending close" request to this resource.
        // Closing of the corresponding resource will be done if our internal job finish it's work.
        m_bPendingCloseFrame = (m_xFrame.is() && aEvent.Source == m_xFrame);
        m_bPendingCloseModel = (m_xModel.is() && aEvent.Source == m_xModel);

        // throw suitable veto exception - because the internal job could not be cancelled.
        css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        throw css::util::CloseVetoException("job still in progress", xThis);
    }

    // No veto ...
    // But don't call die() here or free our internal member.
    // This must be done inside notifyClosing() only. Otherwhise the
    // might stopped job has no chance to return its results or
    // call us back. We must give him the chance to finish it's work successfully.
}

/**
    @short  inform us about frame closing
    @descr  Instead of the method queryClosing(), here is no chance to disagree with that.
            We have to accept it and cancel all current processes inside.

    @param  aEvent
            describes the broadcaster and must be the frame or model instance we know
 */
void SAL_CALL Job::notifyClosing( const css::lang::EventObject& )
{
    die();
    // Do nothing else here. Our internal resources was released ...
}

/**
    @short      shouldn't be called normally
    @descr      But it doesn't matter, who called it. We have to kill our internal
                running processes hardly.

    @param      aEvent
                describe the broadcaster
*/
void SAL_CALL Job::disposing( const css::lang::EventObject& aEvent )
{
    /* SAFE { */
    SolarMutexClearableGuard aWriteLock;

    if (m_xDesktop.is() && aEvent.Source == m_xDesktop)
    {
        m_xDesktop.clear();
        m_bListenOnDesktop = false;
    }
    else if (m_xFrame.is() && aEvent.Source == m_xFrame)
    {
        m_xFrame.clear();
        m_bListenOnFrame = false;
    }
    else if (m_xModel.is() && aEvent.Source == m_xModel)
    {
        m_xModel.clear();
        m_bListenOnModel = false;
    }

    aWriteLock.clear();
    /* } SAFE */

    die();
    // Do nothing else here. Our internal resources was released ...
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
