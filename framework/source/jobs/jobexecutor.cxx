/*************************************************************************
 *
 *  $RCSfile: jobexecutor.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-22 08:38:04 $
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

#ifndef __FRAMEWORK_JOBS_JOBEXECUTOR_HXX_
#include <jobs/jobexecutor.hxx>
#endif

#ifndef __FRAMEWORK_JOBS_JOB_HXX_
#include <jobs/job.hxx>
#endif

#ifndef __FRAMEWORK_JOBS_JOBURL_HXX_
#include <jobs/joburl.hxx>
#endif

#ifndef __FRAMEWORK_CLASS_CONVERTER_HXX_
#include <classes/converter.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#include <threadhelp/transactionguard.hxx>
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
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

//________________________________
//  non exported definitions

//________________________________
//  declarations

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
                                        SERVICENAME_JOBEXECUTOR       ,
                                        IMPLEMENTATIONNAME_JOBEXECUTOR
                                      )

DEFINE_INIT_SERVICE( JobExecutor,
                     {
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
                                css::uno::Reference< css::container::XContainerListener > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
                                xNotifier->addContainerListener(xThis);
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
    , m_aConfig           (xSMGR, ::rtl::OUString::createFromAscii(JobData::EVENTCFG_ROOT) )
{
    // Don't do any reference related code here! Do it inside special
    // impl_ method() ... see DEFINE_INIT_SERVICE() macro for further informations.
}

JobExecutor::~JobExecutor()
{
    LOG_ASSERT(m_aConfig.getMode() == ConfigAccess::E_CLOSED, "JobExecutor::~JobExecutor()\nConfiguration don't send dispoing() message!\n")
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
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // Optimization!
    // Check if the given event name exist inside configuration and reject wrong requests.
    // This optimization supress using of the cfg api for getting event and job descriptions ...
    if (m_lEvents.find(aEvent.EventName) == m_lEvents.end())
        return;

    // get list of all enabled jobs
    // The called static helper methods read it from the configuration and
    // filter disabled jobs using it's time stamp values.
    css::uno::Sequence< ::rtl::OUString > lJobs = JobData::getEnabledJobsForEvent(m_xSMGR, aEvent.EventName);

    // Special feature: If the events "OnNew" or "OnLoad" occures - we generate our own event "onDocumentOpened".
    if (
        (aEvent.EventName.equalsAscii("OnNew") ) ||
        (aEvent.EventName.equalsAscii("OnLoad"))
       )
    {
        css::uno::Sequence< ::rtl::OUString > lAdditionalJobs = JobData::getEnabledJobsForEvent(m_xSMGR, DECLARE_ASCII("onDocumentOpened"));
        sal_Int32 count = lAdditionalJobs.getLength();
        if (count > 0)
        {
            sal_Int32 dest   = lJobs.getLength();
            sal_Int32 source = 0;
            lJobs.realloc(dest+count);
            while(source < count)
            {
                lJobs[dest] = lAdditionalJobs[source];
                ++source;
                ++dest;
            }
        }
    }

    aReadLock.unlock();
    /* } SAFE */

    // step over all enabled jobs and execute it
    sal_Int32 c = lJobs.getLength();
    for (sal_Int32 j=0; j<c; ++j)
    {
        /* SAFE { */
        aReadLock.lock();

        JobData aCfg(m_xSMGR);
        aCfg.setEvent(aEvent.EventName, lJobs[j]);
        aCfg.setEnvironment(JobData::E_DOCUMENTEVENT);

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
        if (sEvent.getLength() > 0)
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
        if (sEvent.getLength() > 0)
        {
            OUStringList::iterator pEvent = m_lEvents.find(sEvent);
            if (pEvent != m_lEvents.end())
                m_lEvents.erase(pEvent);
        }
    }
}

void SAL_CALL JobExecutor::elementReplaced( const css::container::ContainerEvent& aEvent ) throw(css::uno::RuntimeException)
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
