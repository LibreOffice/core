/*************************************************************************
 *
 *  $RCSfile: jobexecutor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:17:12 $
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

#ifndef __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
#include <jobs/configaccess.hxx>
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

DEFINE_XINTERFACE_3( JobExecutor                               ,
                     OWeakObject                               ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider),
                     DIRECT_INTERFACE(css::lang::XServiceInfo ),
                     DIRECT_INTERFACE(css::task::XJobExecutor )
                   )

DEFINE_XTYPEPROVIDER_3( JobExecutor             ,
                        css::lang::XTypeProvider,
                        css::lang::XServiceInfo ,
                        css::task::XJobExecutor
                      )

DEFINE_XSERVICEINFO_MULTISERVICE( JobExecutor                   ,
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
    : ThreadHelpBase(&Application::GetSolarMutex())
    , m_xSMGR       (xSMGR                        )
{
}

JobExecutor::~JobExecutor()
{
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
    // get list of all enabled jobs
    // The called static helper methods read it from the configuration and
    // filter disabled jobs using it's time stamp values.
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
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

        try
        {
            pJob->execute(css::uno::Sequence< css::beans::NamedValue >());
        }
        #ifdef ENABLE_WARNINGS
        catch(const css::uno::Exception& exAny) {LOG_EXCEPTION("JobExecutor::trigger()", "catched on execute job", exAny.Message)}
        #else
        catch(const css::uno::Exception&) {}
        #endif // ENABLE_WARNINGS
    }
}

} // namespace framework
