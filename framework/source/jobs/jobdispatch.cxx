    /*************************************************************************
 *
 *  $RCSfile: jobdispatch.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-10-11 13:41:14 $
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

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
#include <classes/converter.hxx>
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XMULTIHIERARCHICALPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//________________________________
//  includes of other projects

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif

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

#define JOB_ROOT                    "/org.openoffice.Office.Jobs/Jobs/"
#define JOB_PROPERTY_ADMINTIME      "AdminTime"
#define JOB_PROPERTY_USERTIME       "UserTime"
#define JOB_PROPERTY_SERVICE        "Service"
#define JOB_PROPERTY_ASYNC          "Async"
#define JOB_PROPERTY_ARGUMENTS      "Arguments"

//________________________________
//  non exported definitions

//________________________________
//  declarations

DEFINE_XINTERFACE_5( JobDispatch                                      ,
                     OWeakObject                                      ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider       ),
                     DIRECT_INTERFACE(css::frame::XNotifyingDispatch ),
                     DIRECT_INTERFACE(css::frame::XDispatch          ),
                     DIRECT_INTERFACE(css::task::XJobListener        ),
                     DIRECT_INTERFACE(css::frame::XTerminateListener )
                   )

DEFINE_XTYPEPROVIDER_5( JobDispatch                   ,
                        css::lang::XTypeProvider      ,
                        css::frame::XNotifyingDispatch,
                        css::frame::XDispatch         ,
                        css::task::XJobListener       ,
                        css::frame::XTerminateListener
                      )

//________________________________
/**
    @short      standard ctor
    @descr      It initialize this new instance and read all neccessary data
                driect from the configuration.

    @param      xSMGR
                    reference to the uno service manager

    @param      sAlias
                    the alias name of this job, used to locate job properties inside cfg
*/
JobDispatch::JobDispatch( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                          /*IN*/ const ::rtl::OUString&                                        sAlias )
    : ThreadHelpBase(&Application::GetSolarMutex()              )
    , OWeakObject   (                                           )
    , m_xSMGR       (xSMGR                                      )
    , m_aConfig     (xSMGR,
                     DECLARE_ASCII(JOB_ROOT)+
                     ::utl::wrapConfigurationElementName(sAlias))
    , m_aJobData    (sAlias                                     )
{
    // For calling of the methods open()/close() the config access object must not
    // be locked from outside! Only for using cfg()...
    m_aConfig.open(ConfigAccess::E_READONLY);
    impl_readConfig();
    m_aConfig.close();

    // start listening
    css::uno::Reference< css::frame::XDesktop >           xDesktop(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XTerminateListener > xThis   (static_cast< ::cppu::OWeakObject* >(this)   , css::uno::UNO_QUERY);
    if (xDesktop.is())
        xDesktop->addTerminateListener(xThis);
}

//________________________________
/**
    @short  let this instance die
    @descr  There is no chance any longer to work. We have to
            release all used ressources and free used memory.
*/
JobDispatch::~JobDispatch()
{
    // Can be called without any check about open configuration!
    // It does the right things automaticly.
    m_aConfig.close();

    // release all used ressources
    m_xJob  = css::uno::Reference< css::uno::XInterface >();
    m_xSMGR = css::uno::Reference< css::lang::XMultiServiceFactory >();
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
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    m_lDispatchQueue.push( DispatchParameter(aURL, lArgs, xListener, this) );
    if (!m_xJob.is())
        impl_startJob();

    aWriteLock.unlock();
    /* } SAFE */
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
void SAL_CALL JobDispatch::dispatch( const css::util::URL&                                  aURL  ,
                              const css::uno::Sequence< css::beans::PropertyValue >& lArgs ) throw(css::uno::RuntimeException)
{
    dispatchWithNotification(aURL, lArgs, css::uno::Reference< css::frame::XDispatchResultListener >());
}

//________________________________
/**
    @short  not supported
*/
void SAL_CALL JobDispatch::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                       const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException)
{
    LOG_WARNING("Job::addStatusListener()", "Not supported! There is no status available.")
}

//________________________________
/**
    @short  not supported
*/
void SAL_CALL JobDispatch::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                          const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException)
{
    LOG_WARNING("Job::removeStatusListener()", "Not supported! There is no status available.")
}

//________________________________
/**
    @short  react for the notification of an asynchronous job
    @descr  We are not interested on the given job reference. Because we already know it.
            But we must react for the given result value. To share the same code for
            synchronous and asynchronous job, we call an impl method to do so.
            But we don't do it here. This async job was started in our method doIt() ...
            and there we yield till our member m_bAsyncFinished ws set to "true". And
            thats it, what we are doing here. Of course we set the getted result here as
            member too. So doIt() can call impl_finishJob() with the result.

    @param  xJob
                describe the source of this event
                We are not interested on that. Because we already know this job and hold
                it alive by using a reference to it as member.

    @param  aResult
                this any wrap a sequence< NamedValue >, which describes the results of this executed job
 */
void SAL_CALL JobDispatch::jobFinished( /*IN*/ const css::uno::Reference< css::task::XAsyncJob >& xJob    ,
                                        /*IN*/ const css::uno::Any&                               aResult ) throw(css::uno::RuntimeException)
{
    // finish this job
    // It will start a new one, if some more items exist in our dipsatch queue ...
    impl_finishJob(aResult);
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
 */
void SAL_CALL JobDispatch::queryTermination( const css::lang::EventObject& aEvent ) throw(css::frame::TerminationVetoException,
                                                                                   css::uno::RuntimeException          )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // if some outstanding jobs exist ...
    // throw the suitable veto excepotion to supress
    // office terminate
    if (!m_lDispatchQueue.empty() || m_xJob.is())
    {
        css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        throw css::frame::TerminationVetoException(DECLARE_ASCII("job still in progress"), xThis);
    }
    // Otherwhise let do noting here!
    // React in method notifyTermination().

    aReadLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  inform us about office termination
    @descr  Instead of the method queryTermination(), here is no chance to disagree with that.
            We have to accept it and cancel all current processes inside.

            But normaly it can't occure - because we throw suitable terminate veto exception inside
            queryTermination()!

    @param  aEvent
                describes the broadcaster and must be the desktop instance
 */
void SAL_CALL JobDispatch::notifyTermination( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // remove listener connections
    css::uno::Reference< css::frame::XDesktop >           xDesktop(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XTerminateListener > xThis   (static_cast< ::cppu::OWeakObject* >(this)   , css::uno::UNO_QUERY);
    if (xDesktop.is())
        xDesktop->removeTerminateListener(xThis);

    aReadLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      shouldn't be called normaly

    @param      aEvent
                    describe the broadcaster
*/
void SAL_CALL JobDispatch::disposing( /*IN*/ const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException)
{
    LOG_WARNING("Job::disposing()", "should normaly not be called")
}

//________________________________
/**
    @short  start next job, which stands in queue
    @descr  Every dispatch request will be queued.
            Then this method can be called from different places
            to start the next outstanding request.
*/
void JobDispatch::impl_startJob()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::task::XAsyncJob >  xAJob;
    css::uno::Reference< css::task::XJob >       xSJob;

    // get the queued parameter
    // But don't remove it from the queue.
    // That will be done by the method impl_finishJob().
    // Every queued request hold a reference to this JobDispatch object.
    // If queue is empty ... we will die!
    DispatchParameter aParamSet = m_lDispatchQueue.front();

    // create the job
    m_xJob = m_xSMGR->createInstance(m_aJobData.m_sService);
    if (m_aJobData.m_bAsync)
        xAJob = css::uno::Reference< css::task::XAsyncJob >(m_xJob, css::uno::UNO_QUERY);
    else
        xSJob = css::uno::Reference< css::task::XJob >(m_xJob, css::uno::UNO_QUERY);

    // execute it asynchron
    if (xAJob.is())
    {
        css::uno::Reference< css::task::XJobListener > xJobListener(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
        xAJob->executeAsync(m_aJobData.m_lArguments, xJobListener);
        // don't react here directly.
        // Do it in callback method.
    }
    // execute it synchron
    else if (xSJob.is())
    {
        css::uno::Any aResult = xSJob->execute(m_aJobData.m_lArguments);
        impl_finishJob(aResult);
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  react for finishing any job, which was started here
    @descr  It doesn't matter, if it was an asynchronous or a synchronoues one.
            We have to forget this job. But before we must react for his returned result.
            It can use a special protocol mode to force some further actions.
            <p>
            Following protocol items can be used:
            <p>
            <ul>
                <li>
                    "SaveArguments" [sequence< css.beans.NamedValue >]
                    <br>
                    The returned list of (for this generic implementation unknown!)
                    properties, will be written directly to the configuration and replace
                    any old values there. There will no check for changes and we doesn't
                    support any mege feature here. They are written only. The job has
                    to modify this list.
                </li>
                <li>
                    "SendDispatchResult" [css.frame.DispatchResultEvent]
                    <br>
                    The given event is send to all current registered listener.
                    But it's not guaranteed. In case no listener are available or
                    this job isn't part of the dispatch environment (because it was used
                    by the css..task.XJobExecutor->trigger() implementation) this option
                    will be ignored.
                </li>
                <li>
                    "Deactivate" [boolean]
                    <br>
                    The job whish to be disabled. But note: There is no way, to enable it later
                    again by using this implementation. It can be done by using the configuration
                    only. (Means to register this job again.)
                    If a job knows, that there exist some status or result listener, it must use
                    the options "SendDispatchStatus" and "SendDispatchResult" (see before) too, to
                    inform it about the deactivation of this service.
                </li>
            </ul>

    @param  aResult
                this any wrap a sequence< NamedValue >, which contains the mentioned protocol
 */
void JobDispatch::impl_finishJob( /*IN*/ const css::uno::Any& aResult )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // Analyze the results of the finished job and react in the right way.
    css::uno::Sequence< css::beans::NamedValue > lProtocol;
    if (aResult >>= lProtocol)
    {
        //  try to find items of protocol to analyze it later
        // Otherwise it's not easy to decide if a property was found before another one ...
        // (use invalid index as "non existing" mark)
        sal_Int32 nPDeactivate          = -1;
        sal_Int32 nPSaveArguments       = -1;
        sal_Int32 nPSendDispatchResult  = -1;

        sal_Int32 nCount = lProtocol.getLength();
        for( sal_Int32 i=0; i<nCount; ++i )
        {
            if (lProtocol[i].Name.equalsIgnoreAsciiCaseAsciiL("Deactivate",10))
                nPDeactivate = i;
            else
            if (lProtocol[i].Name.equalsIgnoreAsciiCaseAsciiL("SaveArguments",13))
                nPSaveArguments = i;
            else
            if (lProtocol[i].Name.equalsIgnoreAsciiCaseAsciiL("SendDispatchResult",18))
                nPSendDispatchResult = i;
        }

        /* CFG SAFE { */
        // It's neccessary to lock the config access from outside!
        ReadGuard aCfgLock(m_aConfig.m_aLock);
        m_aConfig.open(ConfigAccess::E_READWRITE);

        // a) react for "SaveArguments"
        // Do it before all other ones. May they will fail ...
        if (nPSaveArguments != -1)
        {
            // We don't analyze the list of arguments or support a merge
            // with old values. We save the complete list .. that's it.
            if (lProtocol[nPSaveArguments].Value >>= m_aJobData.m_lArguments)
                impl_saveArguments();
        }

        // b) react for "SendDispatchResult"
        if (nPSendDispatchResult != -1)
        {
            css::frame::DispatchResultEvent aResult;
            if (lProtocol[nPSendDispatchResult].Value >>= aResult)
                impl_sendDispatchResult(aResult);
        }

        // c) react for "Deactivate"
        if (nPDeactivate != -1)
        {
            sal_Bool bDeactivate = sal_False;
            if ((lProtocol[nPDeactivate].Value >>= bDeactivate) && bDeactivate)
                impl_deactivateJob();
        }

        m_aConfig.close();
        aCfgLock.unlock();
        /* } CFG SAFE */
    }

    // let the job die and forget his queued parameter.
    // It hold a reference to us ... if it will be the last item - we can die.
    // So it seams to be better to hold us self alive till this method ends.
    css::uno::Reference< css::uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    m_xJob = css::uno::Reference< css::uno::XInterface >();
    m_lDispatchQueue.pop();

    // Search for next one in queue. If some one exist -
    // start it. But don't delete it from the queue.
    // It will be done by the next impl_finishJob()!
    // (startJob() uses the queue directly ...)
    if (!m_lDispatchQueue.empty())
        impl_startJob();

    // Otherwhise do nothing. We can(!) die by ref count now.

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  read the configuration of this job
    @descr  An instance of this class will be initialized with the alias name of the real
            job only. Then the configuration package has to be opened to read all other
            configured properties. After doing that all member of an object of this
            class has to be filled with valid values.

    @attention  The member m_xConfig must be valid, if this method is called!
 */
void JobDispatch::impl_readConfig()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // try to open the configuration set of this job directly and get a property access to it
    // We open it readonly here

    /* CFG SAFE { */
    // Note: We have to lock the used configuration access from outside to synchronize it
    // with other threads!
    ReadGuard aCfgLock(m_aConfig.m_aLock);
    m_aConfig.open(ConfigAccess::E_READONLY);

    css::uno::Reference< css::beans::XPropertySet > xJobProperties(m_aConfig.cfg(), css::uno::UNO_QUERY);
    if (xJobProperties.is())
    {
        css::uno::Any   aValue;
        ::rtl::OUString sTime ;

        // read and convert "AdminTime"
        aValue = xJobProperties->getPropertyValue(DECLARE_ASCII(JOB_PROPERTY_ADMINTIME));
        if (aValue >>= sTime)
            m_aJobData.m_aAdminTime = Converter::convert_String2DateTime(sTime);

        // read and convert "UserTime"
        aValue = xJobProperties->getPropertyValue(DECLARE_ASCII(JOB_PROPERTY_USERTIME));
        if (aValue >>= sTime)
            m_aJobData.m_aUserTime = Converter::convert_String2DateTime(sTime);

        // check active state by comparing time stamps
        m_aJobData.m_bActive = (m_aJobData.m_aAdminTime > m_aJobData.m_aUserTime);

        // read async state
        aValue   = xJobProperties->getPropertyValue(DECLARE_ASCII(JOB_PROPERTY_ASYNC));
        aValue >>= m_aJobData.m_bAsync;

        // read uno implementation name
        aValue   = xJobProperties->getPropertyValue(DECLARE_ASCII(JOB_PROPERTY_SERVICE));
        aValue >>= m_aJobData.m_sService;

        // read whole argument list
        aValue = xJobProperties->getPropertyValue(DECLARE_ASCII(JOB_PROPERTY_ARGUMENTS));
        css::uno::Reference< css::container::XNameAccess > xArgumentList;
        if (
            (aValue >>= xArgumentList)  &&
            (xArgumentList.is()      )
           )
        {
            css::uno::Sequence< ::rtl::OUString > lArgumentNames = xArgumentList->getElementNames();
            sal_Int32                             nCount         = lArgumentNames.getLength();
            m_aJobData.m_lArguments.realloc(nCount);
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                m_aJobData.m_lArguments[i].Name  = lArgumentNames[i];
                m_aJobData.m_lArguments[i].Value = xArgumentList->getByName(m_aJobData.m_lArguments[i].Name);
            }
        }
    }

    aCfgLock.unlock();
    /* } CFG SAFE */

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  write the new JobDispatch specific arguments back to the configuration
    @descr  If a JobDispatch finish his work, it can give us a new list of arguments (which
            will not interpreted by us). We write it back to the configuration only.
            So a JobDispatch can have persistent data without implementing anything
            or define own config areas for that.
            The arguments must be set to our member m_lArguments before.

    @attention  The member m_xConfig must be valid, if this method is called!
 */
void JobDispatch::impl_saveArguments()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // Note: The used member m_aConfig must be valid opened and locked by the oustide code!
    css::uno::Reference< css::beans::XMultiHierarchicalPropertySet > xArgumentList(m_aConfig.cfg(), css::uno::UNO_QUERY);
    if (xArgumentList.is())
    {
        sal_Int32                             nCount = m_aJobData.m_lArguments.getLength();
        css::uno::Sequence< ::rtl::OUString > lNames (nCount);
        css::uno::Sequence< css::uno::Any >   lValues(nCount);

        for (sal_Int32 i=0; i<nCount; ++i)
        {
            lNames [i] = m_aJobData.m_lArguments[i].Name ;
            lValues[i] = m_aJobData.m_lArguments[i].Value;
        }

        xArgumentList->setHierarchicalPropertyValues(lNames, lValues);
    }

    aReadLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  mark a JobDispatch as non startable for further requests
    @descr  We don't remove the configuration entry! We set a timestamp value only.
            And there exist two of them: one for an administrator ... and one for the
            current user. We change it for the user layer only. So this JobDispatch can't be
            started any more ... till the administrator changehis timestamp.
            That can be usefull for post setup scenarios, which must run one time only.

    @attention  The member m_xConfig must be valid, if this method is called!
 */
void JobDispatch::impl_deactivateJob()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // Note: The used member m_aConfig must be valid opened and locked by the oustide code!
    css::uno::Reference< css::beans::XPropertySet > xPropSet(m_aConfig.cfg(), css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        // Convert and write the user timestamp to the configuration.
        // default ctor of DateTime set current date & time on instanciated object automaticly!
        m_aJobData.m_aUserTime = DateTime();
        css::uno::Any aValue;
        aValue <<= Converter::convert_DateTime2String(m_aJobData.m_aUserTime);
        xPropSet->setPropertyValue(DECLARE_ASCII(JOB_PROPERTY_USERTIME), aValue);
        // It's important to mark this JobDispatch instance in memory as deactivated too!
        m_aJobData.m_bActive = sal_False;
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  notifies possible registered XDispatchResultListener
    @descr  If a job finish his work, it can give us a valid struct from type
            DispatchResultEvent. In case we have some suitable listener registered,
            they will be informed.

    @param  aResult
                the given result of the finished JobDispatch
 */
void JobDispatch::impl_sendDispatchResult( /*IN*/ const css::frame::DispatchResultEvent& aResult )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    DispatchParameter aParamSet = m_lDispatchQueue.front();
    if (aParamSet.m_xListener.is())
        aParamSet.m_xListener->dispatchFinished(aResult);

    aReadLock.unlock();
    /* } SAFE */
}

} // namespace framework
