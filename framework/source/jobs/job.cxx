/*************************************************************************
 *
 *  $RCSfile: job.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:16:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//________________________________
//  my own includes

#ifndef __FRAMEWORK_JOBS_JOB_HXX_
#include <jobs/job.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//________________________________
//  interface includes

#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XASYNCJOB_HPP_
#include <com/sun/star/task/XAsyncJob.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif

//________________________________
//  includes of other projects

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//________________________________
//  namespace

namespace framework{

//________________________________
//  non exported const

//________________________________
//  non exported definitions

//________________________________
//  declarations

DEFINE_XINTERFACE_4( Job                                             ,
                     OWeakObject                                     ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                     DIRECT_INTERFACE(css::task::XJobListener       ),
                     DIRECT_INTERFACE(css::frame::XTerminateListener),
                     DIRECT_INTERFACE(css::util::XCloseListener     )
                   )

DEFINE_XTYPEPROVIDER_4( Job                           ,
                        css::lang::XTypeProvider      ,
                        css::task::XJobListener       ,
                        css::frame::XTerminateListener,
                        css::util::XCloseListener
                      )

//________________________________
/**
    @short      standard ctor
    @descr      It initialize this new instance. But it set some generic parameters here only.
                Specialized informations (e.g. the alias or service name ofthis job) will be set
                later using the method setJobData().

    @param      xSMGR
                    reference to the uno service manager

    @param      xFrame
                    reference to the frame, in which environment we run
                    (May be null!)
*/
Job::Job( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
          /*IN*/ const css::uno::Reference< css::frame::XFrame >&              xFrame )
    : ThreadHelpBase       (&Application::GetSolarMutex())
    , ::cppu::OWeakObject  (                             )
    , m_xSMGR              (xSMGR                        )
    , m_xFrame             (xFrame                       )
    , m_bCloseListening    (sal_False                    )
    , m_bPendingCloseFrame (sal_False                    )
    , m_bRuns              (sal_False                    )
    , m_aJobCfg            (xSMGR                        )
{
}

//________________________________
/**
    @short  superflous!
    @descr  Releasing of memory and reference must be done inside die() call.
            Otherwhise it's a bug.
*/
Job::~Job()
{
}

//________________________________
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
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_xResultListener   = xListener  ;
    m_xResultSourceFake = xSourceFake;
    aWriteLock.unlock();
    /* } SAFE */
}

void Job::setJobData( const JobData& aData )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // reject dangerous calls
    if (m_bRuns)
    {
        LOG_WARNING("Job::setJobData()", "job still running")
        return;
    }

    m_aJobCfg = aData;
    aWriteLock.unlock();
    /* } SAFE */
}

JobData Job::getJobData()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aJobCfg;
    /* } SAFE */
}

JobData& Job::getJobDataRef()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aJobCfg;
    /* } SAFE */
}

//________________________________
/**
    @short  runs the job
    @descr  It doesn't matter, if the job is an asynchronous or
            synchronous one. This method returns only if it was finished
            or cancelled.

    @param  lDynamicArgs
                optional arguments for job execution
                In case the represented job is a configured one (which uses static
                arguments too) all informations will be merged!
*/
void Job::execute( /*IN*/ const css::uno::Sequence< css::beans::NamedValue >& lDynamicArgs )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // reject dangerous calls
    if (m_bRuns)
    {
        LOG_WARNING("Job::execute()", "job still running")
        return;
    }
    m_bRuns = sal_True;

    css::uno::Reference< css::task::XAsyncJob >  xAJob;
    css::uno::Reference< css::task::XJob >       xSJob;

    // create the job
    // We must check for the supported interface on demand!
    // But we preferr the synchronous one ...
    m_xJob = m_xSMGR->createInstance(m_aJobCfg.getService());
    xSJob  = css::uno::Reference< css::task::XJob >(m_xJob, css::uno::UNO_QUERY);
    if (!xSJob.is())
        xAJob = css::uno::Reference< css::task::XAsyncJob >(m_xJob, css::uno::UNO_QUERY);

    // execute it asynchron (and wait for finishing it!)
    if (xAJob.is())
    {
        // wait for finishing this job - so this method
        // does the same for synchronous and asynchronous jobs!
        m_aAsyncWait.reset();

        css::uno::Reference< css::task::XJobListener > xJobListener(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        xAJob->executeAsync(impl_generateJobArgs(lDynamicArgs), xJobListener);

        aWriteLock.unlock();
        /* } SAFE */

        m_aAsyncWait.wait();

        aWriteLock.lock();
        /* SAFE { */
        // Note: Result handling was already done inside the callback!
    }
    // execute it synchron
    else if (xSJob.is())
    {
        css::uno::Any aResult = xSJob->execute(impl_generateJobArgs(lDynamicArgs));
        impl_reactForJobResult(aResult);
    }

    // If we got a close request from our frame ...
    // but we disagreed wit that ...
    // but got the ownership ...
    // we have to disable further working with this frame ...
    // and to disable ourself!
    if (m_bPendingCloseFrame)
        die();
    else
        // Don't forget that! Otherwhise nobody can use us any longer.
        m_bRuns = sal_False;

    aWriteLock.unlock();
    /* SAFE { */
}

//________________________________
/**
    @short  kill this job
    @descr  It doesn't matter if this request is called from inside or
            from outside. We release our internal structures and stop
            avary activity. After doing so - this instance will not be
            useable any longer! Of course we try to handle further requests
            carefully. May somehwere else hold a reference to us ...
*/
void Job::die()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    impl_stopListening();

    m_xJob               = css::uno::Reference< css::uno::XInterface >();
    m_xFrame             = css::uno::Reference< css::frame::XFrame >();
    m_xDesktop           = css::uno::Reference< css::frame::XDesktop >();
    m_xResultListener    = css::uno::Reference< css::frame::XDispatchResultListener >();
    m_xResultSourceFake  = css::uno::Reference< css::uno::XInterface >();
    m_bPendingCloseFrame = sal_False;
    m_bRuns              = sal_False;

    // neccessary to finish all blocked operations, which wait for that!
    m_aAsyncWait.set();

    aWriteLock.unlock();
    /* SAFE { */
}

//________________________________
/**
    @short  generates list of arguments for job execute
    @descr  There exist a set of informations, which can be needed by a job.
                a) it's static configuration data   (Equals for all jobs.    )
                b) it's specific configuration data (Different for every job.)
                c) some environment values          (e.g. the frame, for which this job was started)
                d) any other dynamic data           (e.g. parameters of a dispatch() request)
            We collect all these informations and generate one list which include all others.

    @param  lDynamicArgs
                list of dynamic arguments (given by a corresponding dispatch() call)
                Can be empty too.

    @return A list which includes all mentioned sub lists.
*/
css::uno::Sequence< css::beans::NamedValue > Job::impl_generateJobArgs( /*IN*/ const css::uno::Sequence< css::beans::NamedValue >& lDynamicArgs )
{
    css::uno::Sequence< css::beans::NamedValue > lAllArgs;

    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // the real structure of the returned list depends from the environment of this job!
    JobData::EMode eMode = m_aJobCfg.getMode();

    // Create list of environment variables. This list must be part of the
    // returned structure everytimes ... but some of its members are opetional!
    css::uno::Sequence< css::beans::NamedValue > lEnvArgs(1);
    lEnvArgs[0].Name    = ::rtl::OUString::createFromAscii(JobData::PROP_ENVTYPE);
    lEnvArgs[0].Value <<= m_aJobCfg.getEnvironmentDescriptor();

    if (m_xFrame.is())
    {
        sal_Int32 c = lEnvArgs.getLength();
        lEnvArgs.realloc(c+1);
        lEnvArgs[c].Name    = ::rtl::OUString::createFromAscii(JobData::PROP_FRAME);
        lEnvArgs[c].Value <<= m_xFrame;
    }
    if (eMode==JobData::E_EVENT)
    {
        sal_Int32 c = lEnvArgs.getLength();
        lEnvArgs.realloc(c+1);
        lEnvArgs[c].Name    = ::rtl::OUString::createFromAscii(JobData::PROP_EVENTNAME);
        lEnvArgs[c].Value <<= m_aJobCfg.getEvent();
    }

    // get the configuration data from the job data container ... if possible
    // Means: if this job has any configuration data. Note: only realy
    // filled lists will be set to the return structure at the end of this method.
    css::uno::Sequence< css::beans::NamedValue > lConfigArgs   ;
    css::uno::Sequence< css::beans::NamedValue > lJobConfigArgs;
    if (eMode==JobData::E_ALIAS || eMode==JobData::E_EVENT)
    {
        lConfigArgs    = m_aJobCfg.getConfig();
        lJobConfigArgs = m_aJobCfg.getJobConfig();
    }

    aReadLock.unlock();
    /* } SAFE */

    // Add all valid (not empty) lists to the return list
    if (lConfigArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name    = ::rtl::OUString::createFromAscii(JobData::PROPSET_CONFIG);
        lAllArgs[nLength].Value <<= lConfigArgs;
    }
    if (lJobConfigArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name    = ::rtl::OUString::createFromAscii(JobData::PROPSET_OWNCONFIG);
        lAllArgs[nLength].Value <<= lJobConfigArgs;
    }
    if (lEnvArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name    = ::rtl::OUString::createFromAscii(JobData::PROPSET_ENVIRONMENT);
        lAllArgs[nLength].Value <<= lEnvArgs;
    }
    if (lDynamicArgs.getLength()>0)
    {
        sal_Int32 nLength = lAllArgs.getLength();
        lAllArgs.realloc(nLength+1);
        lAllArgs[nLength].Name    = ::rtl::OUString::createFromAscii(JobData::PROPSET_DYNAMICDATA);
        lAllArgs[nLength].Value <<= lDynamicArgs;
    }

    return lAllArgs;
}

//________________________________
/**
    @short  analyze the given job result and change the job configuration
    @descr  Note: Some results can be handled only, if this job has a valid configuration!
            For "not configured jobs" (means pure services) they can be ignored.
            But these cases are handled by our JobData member. We can call it everytime.
            It does the right things automaticly. E.g. if the job has no configuration ...
            it does nothing during setJobConfig()!

    @param  aResult
                the job result for analyzing
*/
void Job::impl_reactForJobResult( /*IN*/ const css::uno::Any& aResult )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

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
        (eEnvironment == JobData::E_EXECUTION              ) &&
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

    aWriteLock.unlock();
    /* SAFE { */
}

//________________________________
/**
    @short  starts listening for office shutdown and closing of our
            given target frame (if its a valid reference)
    @descr  We will reghister ourself as terminate listener
            at the global desktop instance. That will hold us
            alive and additional we get the information, if the
            office whish to shutdown. If then an internal job
            is running we will have the chance to supress that
            by throwing a veto exception. If our internal wrapped
            job finished his work, we can release this listener
            connection.

            Further we are listener for closing of the (possible valid)
            given frame. We must be shure, that this ressource won't be gone
            if our internal job is still running.
*/
void Job::impl_startListening()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // listening for office shutdown
    if (!m_xDesktop.is())
    {
        try
        {
            m_xDesktop = css::uno::Reference< css::frame::XDesktop >(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
            css::uno::Reference< css::frame::XTerminateListener > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (m_xDesktop.is())
                m_xDesktop->addTerminateListener(xThis);
        }
        catch(css::uno::Exception&)
        {
            m_xDesktop = css::uno::Reference< css::frame::XDesktop >();
        }
    }

    // listening for frame closing
    if (m_xFrame.is() && m_bCloseListening)
    {
        try
        {
            css::uno::Reference< css::util::XCloseBroadcaster > xCloseable(m_xFrame                                 , css::uno::UNO_QUERY);
            css::uno::Reference< css::util::XCloseListener >    xThis     (static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (xCloseable.is())
            {
                xCloseable->addCloseListener(xThis);
                m_bCloseListening = sal_True;
            }
        }
        catch(css::uno::Exception&)
        {
            m_bCloseListening = sal_False;
        }
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  release listener connection for office shutdown
    @descr  see description of impl_startListening()
*/
void Job::impl_stopListening()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // listening for office shutdown
    if (m_xDesktop.is())
    {
        try
        {
            css::uno::Reference< css::frame::XTerminateListener > xThis(static_cast< ::cppu::OWeakObject* >(this)   , css::uno::UNO_QUERY);
            m_xDesktop->removeTerminateListener(xThis);
            m_xDesktop = css::uno::Reference< css::frame::XDesktop >();
        }
        catch(css::uno::Exception&)
        {
        }
    }

    // listening for frame closing
    if (m_xFrame.is() && !m_bCloseListening)
    {
        try
        {
            css::uno::Reference< css::util::XCloseBroadcaster > xCloseable(m_xFrame                                 , css::uno::UNO_QUERY);
            css::uno::Reference< css::util::XCloseListener >    xThis     (static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
            if (xCloseable.is())
            {
                xCloseable->removeCloseListener(xThis);
                m_bCloseListening = sal_False;
            }
        }
        catch(css::uno::Exception&)
        {
            m_bCloseListening = sal_True;
        }
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  callback from any asynchronous executed job

    @descr  Our execute() method waits for this callback.
            We have to react for the possible results here,
            to kill the running job and disable the blocked condition
            so execute() can be finished too.

    @param  xJob
                the job, which was running and inform us now

    @param  aResult
                it's results
*/
void SAL_CALL Job::jobFinished( /*IN*/ const css::uno::Reference< css::task::XAsyncJob >& xJob    ,
                                /*IN*/ const css::uno::Any&                               aResult ) throw(css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // It's neccessary to check this.
    // May this job was cancelled by any other reason
    // some milliseconds before. :-)
    if (m_xJob.is() && m_xJob==xJob)
    {
        // react for his results
        // (means enable/disable it for further requests
        // or save arguments or notify listener ...)
        impl_reactForJobResult(aResult);

        // Let the job die!
        m_xJob = css::uno::Reference< css::uno::XInterface >();
    }

    // And let the start method "execute()" finishing it's job.
    // But do it everytime. So any outside blocking code can finish
    // his work too.
    m_aAsyncWait.set();

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
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
void SAL_CALL Job::queryTermination( /*IN*/ const css::lang::EventObject& aEvent ) throw(css::frame::TerminationVetoException,
                                                                                         css::uno::RuntimeException          )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // if internal job is still running ...
    // throw the suitable veto exception to supress office termination
    if (m_bRuns)
    {
        css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        throw css::frame::TerminationVetoException(DECLARE_ASCII("job still in progress"), xThis);
    }

    aReadLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  inform us about office termination
    @descr  Instead of the method queryTermination(), here is no chance to disagree with that.
            We have to accept it and cancel all current processes inside.
            It can occure only, if job was not already started if queryTermination() was called here ..
            Then we had not throwed a veto exception. But now we must agree with this situation and break
            all our internal processes. Its not a good idea to mark this instance as non startable any longer
            inside queryTermination() if no job was unning too. Because that would disable this job and may
            the office does not realy shutdownm, because another listener has thrown the suitable exception.

    @param  aEvent
                describes the broadcaster and must be the desktop instance
 */
void SAL_CALL Job::notifyTermination( /*IN*/ const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    // Kill the (may still running) job
    // and stop listening for any events.
    die();
    // Do nothing else here. Our internal ressources was released ...
}

//________________________________
/**
    @short  prevent internal wrapped job against frame closing
    @descr  This event is broadcasted by the frame instance and ask for closing.
            If the internal wrapped job is still in progress, we disagree with that by throwing the
            right veto exception. If not - we agree. But then we must be aware, that another event
            notifyClosing() can follow. Then we have no chance to do the same. Then we have to
            accept that and stop our work instandly.

    @param  aEvent
                describes the broadcaster and must be the frame instance

    @param  bGetsOwnerShip
                If it's set to <TRUE> and we throw the right veto excepion, we have to close this frame later
                if our internal processes will be finished. If it's set to <FALSE/> we can ignore it.

    @throw  CloseVetoException
                if our internal wrapped job is still running.
 */
void SAL_CALL Job::queryClosing( const css::lang::EventObject& aEvent         ,
                                       sal_Bool                bGetsOwnership ) throw(css::util::CloseVetoException,
                                                                                      css::uno::RuntimeException   )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // if internal job is still running ...
    // throw the suitable veto exception to supress frame closing
    if (m_bRuns)
    {
        // save the information about the owner ship of this frame!
        // That means - we have try to close it again if our reason for this veto
        // will finish its work too.
        m_bPendingCloseFrame = bGetsOwnership;

        css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        throw css::util::CloseVetoException(DECLARE_ASCII("job still in progress"), xThis);
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  inform us about frame closing
    @descr  Instead of the method queryClosing(), here is no chance to disagree with that.
            We have to accept it and cancel all current processes inside.

    @param  aEvent
                describes the broadcaster and must be the frame instance
 */
void SAL_CALL Job::notifyClosing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    // Kill the (may still running) job
    // and stop listening for any events.
    die();
    // Do nothing else here. Our internal ressources was released ...
}

//________________________________
/**
    @short      shouldn't be called normaly
    @descr      But it doesn't matter, who called it. We have to kill our internal
                running processes hardly.

    @param      aEvent
                    describe the broadcaster
*/
void SAL_CALL Job::disposing( /*IN*/ const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    // Kill the (may still running) job
    // and stop listening for any events.
    die();
    // Do nothing else here. Our internal ressources was released ...
}

} // namespace framework
