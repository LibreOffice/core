    /*************************************************************************
 *
 *  $RCSfile: jobdata.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:16:48 $
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

#ifndef __FRAMEWORK_JOBS_JOBDATA_HXX_
#include <jobs/jobdata.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_CONVERTER_HXX_
#include <classes/converter.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#include <classes/wildcard.hxx>
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

#ifndef _COM_SUN_STAR_BEANS_XMULTIHIERARCHICALPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
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
//  exported const

const sal_Char* JobData::JOBCFG_ROOT              = "/org.openoffice.Office.Jobs/Jobs/"   ;
const sal_Char* JobData::JOBCFG_PROP_SERVICE      = "Service"                             ;
const sal_Char* JobData::JOBCFG_PROP_ARGUMENTS    = "Arguments"                           ;

const sal_Char* JobData::EVENTCFG_ROOT            = "/org.openoffice.Office.Jobs/Events/" ;
const sal_Char* JobData::EVENTCFG_PATH_JOBLIST    = "/JobList"                            ;
const sal_Char* JobData::EVENTCFG_PROP_ADMINTIME  = "AdminTime"                           ;
const sal_Char* JobData::EVENTCFG_PROP_USERTIME   = "UserTime"                            ;

const sal_Char* JobData::PROPSET_CONFIG           = "Config"                              ;
const sal_Char* JobData::PROPSET_OWNCONFIG        = "JobConfig"                           ;
const sal_Char* JobData::PROPSET_ENVIRONMENT      = "Environment"                         ;
const sal_Char* JobData::PROPSET_DYNAMICDATA      = "DynamicData"                         ;

const sal_Char* JobData::PROP_ALIAS               = "Alias"                               ;
const sal_Char* JobData::PROP_EVENTNAME           = "EventName"                           ;
const sal_Char* JobData::PROP_ENVTYPE             = "EnvType"                             ;
const sal_Char* JobData::PROP_FRAME               = "Frame"                               ;
const sal_Char* JobData::PROP_SERVICE             = "Service"                             ;

//________________________________
//  non exported definitions

//________________________________
//  declarations

//________________________________
/**
    @short      standard ctor
    @descr      It initialize this new instance.
                But for real working it's neccessary to call setAlias() or setService() later.
                Because we need the job data ...

    @param      xSMGR
                    reference to the uno service manager
*/
JobData::JobData( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(&Application::GetSolarMutex())
    , m_xSMGR       (xSMGR                        )
{
    // share code for member initialization with defaults!
    impl_reset();
}

//________________________________
/**
    @short  copy ctor
    @descr  Sometimes such job data container must be moved from one using place
            to another one. Then a copy ctor and copy operator must be available.

    @param  rCopy
                the original instance, from which we must copy all data
*/
JobData::JobData( const JobData& rCopy )
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    // use the copy operator to share the same code
    *this = rCopy;
}

//________________________________
/**
    @short  operator for coping JobData instances
    @descr  Sometimes such job data container must be moved from one using place
            to another one. Then a copy ctor and copy operator must be available.

    @param  rCopy
                the original instance, from which we must copy all data
*/
void JobData::operator=( const JobData& rCopy )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    // Please don't copy the uno service manager reference.
    // That can change the uno context, which isn't a good idea!
    m_eMode                = rCopy.m_eMode               ;
    m_eEnvironment         = rCopy.m_eEnvironment        ;
    m_sAlias               = rCopy.m_sAlias              ;
    m_sService             = rCopy.m_sService            ;
    m_sEvent               = rCopy.m_sEvent              ;
    m_lArguments           = rCopy.m_lArguments          ;
    m_aLastExecutionResult = rCopy.m_aLastExecutionResult;
    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  let this instance die
    @descr  There is no chance any longer to work. We have to
            release all used ressources and free used memory.
*/
JobData::~JobData()
{
    impl_reset();
}

//________________________________
/**
    @short      initalize this instance as a job with configuration
    @descr      They given alias can be used to adress some configuraton data.
                We read it and fill our internal structures. Of course old informations
                will be lost doing so.

    @param      sAlias
                    the alias name of this job, used to locate job properties inside cfg
*/
void JobData::setAlias( const ::rtl::OUString& sAlias )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    // delete all old informations! Otherwhise we mix it with the new one ...
    impl_reset();

    // take over the new informations
    m_sAlias   = sAlias;
    m_eMode    = E_ALIAS;

    // try to open the configuration set of this job directly and get a property access to it
    // We open it readonly here
    ::rtl::OUString sKey;
    sKey  = ::rtl::OUString::createFromAscii(JOBCFG_ROOT);
    sKey += ::utl::wrapConfigurationElementName(m_sAlias);

    ConfigAccess aConfig(m_xSMGR, sKey);
    aConfig.open(ConfigAccess::E_READONLY);
    if (aConfig.getMode()==ConfigAccess::E_CLOSED)
    {
        impl_reset();
        return;
    }

    css::uno::Reference< css::beans::XPropertySet > xJobProperties(aConfig.cfg(), css::uno::UNO_QUERY);
    if (xJobProperties.is())
    {
        css::uno::Any aValue;

        // read uno implementation name
        aValue   = xJobProperties->getPropertyValue(::rtl::OUString::createFromAscii(JOBCFG_PROP_SERVICE));
        aValue >>= m_sService;

        // read whole argument list
        aValue = xJobProperties->getPropertyValue(::rtl::OUString::createFromAscii(JOBCFG_PROP_ARGUMENTS));
        css::uno::Reference< css::container::XNameAccess > xArgumentList;
        if (
            (aValue >>= xArgumentList)  &&
            (xArgumentList.is()      )
           )
        {
            css::uno::Sequence< ::rtl::OUString > lArgumentNames = xArgumentList->getElementNames();
            sal_Int32                             nCount         = lArgumentNames.getLength();
            m_lArguments.realloc(nCount);
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                m_lArguments[i].Name  = lArgumentNames[i];
                m_lArguments[i].Value = xArgumentList->getByName(m_lArguments[i].Name);
            }
        }
    }

    aConfig.close();
    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      initalize this instance as a job without configuration
    @descr      This job has no configuration data. We have to forget all old informations
                and set only some of them new, so this instance can work.

    @param      sService
                    the uno service name of this "non configured" job
*/
void JobData::setService( const ::rtl::OUString& sService )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // delete all old informations! Otherwhise we mix it with the new one ...
    impl_reset();
    // take over the new informations
    m_sService = sService;
    m_eMode    = E_SERVICE;

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      initialize this instance with new job values.
    @descr      It reads automaticly all properties of the specified
                job (using it's alias name) and "register it" for the
                given event. This registration will not be validated against
                the underlying configuration! (That must be done from outside.
                Because the caller must have the configuration already open to
                get the values for sEvent and sAlias! And doing so it can perform
                only, if the time stanp values are readed outside too.
                Further it make no sense to initialize and start a disabled job.
                So this initialization method will be called for enabled jobs only.)

    @param      sEvent
                    the triggered event, for which this job should be started

    @param      sAlias
                    mark the required job inside event registration list
*/
void JobData::setEvent( const ::rtl::OUString& sEvent ,
                        const ::rtl::OUString& sAlias )
{
    // share code to read all job properties!
    setAlias(sAlias);

    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // take over the new informations - which differ against set on of method setAlias()!
    m_sEvent = sEvent;
    m_eMode  = E_EVENT;

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      set the new job specific arguments
    @descr      If a job finish his work, it can give us a new list of arguments (which
                will not interpreted by us). We write it back to the configuration only
                (if this job has it's own configuration!).
                So a job can have persistent data without implementing anything
                or define own config areas for that.

    @param      lArguments
                    list of arguments, which should be set for this job
 */
void JobData::setJobConfig( const css::uno::Sequence< css::beans::NamedValue >& lArguments )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // actualize member
    m_lArguments = lArguments;

    // actualize the configuration ... if possible!
    if (m_eMode==E_ALIAS)
    {
        // It doesn't matter if this config object was already opened before.
        // It doesn nothing here then ... or it change the mode automaticly, if
        // it was opened using another one before.
        ::rtl::OUString sKey;
        sKey  = ::rtl::OUString::createFromAscii(JOBCFG_ROOT);
        sKey += ::utl::wrapConfigurationElementName(m_sAlias);

        ConfigAccess aConfig(m_xSMGR, sKey);
        aConfig.open(ConfigAccess::E_READWRITE);
        if (aConfig.getMode()==ConfigAccess::E_CLOSED)
            return;

        css::uno::Reference< css::beans::XMultiHierarchicalPropertySet > xArgumentList(aConfig.cfg(), css::uno::UNO_QUERY);
        if (xArgumentList.is())
        {
            sal_Int32                             nCount = m_lArguments.getLength();
            css::uno::Sequence< ::rtl::OUString > lNames (nCount);
            css::uno::Sequence< css::uno::Any >   lValues(nCount);

            for (sal_Int32 i=0; i<nCount; ++i)
            {
                lNames [i] = m_lArguments[i].Name ;
                lValues[i] = m_lArguments[i].Value;
            }

            xArgumentList->setHierarchicalPropertyValues(lNames, lValues);
        }
        aConfig.close();
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      set a new excution result
    @descr      Every executed job can have returned a result.
                We set it here, so our user can use it may be later.
                But the outside code can use it too, to analyze it and
                adopt the configuration of this job too. Because the
                result uses a protocol, which allow that. And we provide
                right functionality to save it.

    @param      aResult
                    the result of last execution
 */
void JobData::setResult( const JobResult& aResult )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // overwrite the last saved result
    m_aLastExecutionResult = aResult;

    // Don't use his informations to actualize
    // e.g. the arguments of this job. It must be done
    // from outside! Here we save this information only.

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short  set a new environment descriptor for this job
    @descr  It must(!) be done everytime this container is initialized
            with new job datas e.g.: setAlias()/setEvent()/setService() ...
            Otherwhise the environment will be unknown!
 */
void JobData::setEnvironment( EEnvironment eEnvironment )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_eEnvironment = eEnvironment;
    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      these functions provides access to our internal members
    @descr      These member represent any information about the job
                and can be used from outside to e.g. start a job.
 */
JobData::EMode JobData::getMode() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_eMode;
    /* } SAFE */
}

//________________________________

JobData::EEnvironment JobData::getEnvironment() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_eEnvironment;
    /* } SAFE */
}

//________________________________

::rtl::OUString JobData::getEnvironmentDescriptor() const
{
    ::rtl::OUString sDescriptor;
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    switch(m_eEnvironment)
    {
        case E_EXECUTION :
            sDescriptor = ::rtl::OUString::createFromAscii("EXECUTOR");
            break;

        case E_DISPATCH :
            sDescriptor = ::rtl::OUString::createFromAscii("DISPATCH");
            break;
    }
    /* } SAFE */
    return sDescriptor;
}

//________________________________

::rtl::OUString JobData::getAlias() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_sAlias;
    /* } SAFE */
}

//________________________________

::rtl::OUString JobData::getService() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_sService;
    /* } SAFE */
}

//________________________________

::rtl::OUString JobData::getEvent() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_sEvent;
    /* } SAFE */
}

//________________________________

css::uno::Sequence< css::beans::NamedValue > JobData::getJobConfig() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_lArguments;
    /* } SAFE */
}

//________________________________

css::uno::Sequence< css::beans::NamedValue > JobData::getConfig() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Sequence< css::beans::NamedValue > lConfig;
    if (m_eMode==E_ALIAS)
    {
        lConfig.realloc(2);
        sal_Int32 i = 0;

        lConfig[i].Name = ::rtl::OUString::createFromAscii(PROP_ALIAS);
        lConfig[i].Value <<= m_sAlias;
        ++i;

        lConfig[i].Name = ::rtl::OUString::createFromAscii(PROP_SERVICE);
        lConfig[i].Value <<= m_sService;
        ++i;
    }
    aReadLock.unlock();
    /* } SAFE */
    return lConfig;
}

//________________________________

JobResult JobData::getResult() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aLastExecutionResult;
    /* } SAFE */
}

//________________________________
/**
    @short  return information, if this job is part of the global configuration package
            org.openoffice.Office.Jobs
    @descr  Because jobs can be executed by the dispatch framework using an uno service name
            directly - an executed job must not have any configuration realy. Such jobs
            must provide the right interfaces only! But after finishing jobs can return
            some informations (e.g. for updating her configuration ...). We must know
            if such request is valid or not then.

    @return TRUE if the represented job is part of the underlying configuration package.
 */
sal_Bool JobData::hasConfig() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return (m_eMode==E_ALIAS || m_eMode==E_EVENT);
    /* } SAFE */
}

//________________________________
/**
    @short      mark a job as non startable for further requests
    @descr      We don't remove the configuration entry! We set a timestamp value only.
                And there exist two of them: one for an administrator ... and one for the
                current user. We change it for the user layer only. So this JobDispatch can't be
                started any more ... till the administrator change his timestamp.
                That can be usefull for post setup scenarios, which must run one time only.

                Note: This method don't do anything, if ths represented job doesn't have a configuration!
 */
void JobData::disableJob()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    // No configuration - not used from EXECUTOR and not triggered from an event => no chance!
    if (m_eMode!=E_EVENT)
        return;

    // actualize the configuration
    // It doesn't matter if this config object was already opened before.
    // It doesn nothing here then ... or it change the mode automaticly, if
    // it was opened using another one before.
    ::rtl::OUStringBuffer sKey(256);
    sKey.appendAscii(JobData::EVENTCFG_ROOT                       );
    sKey.append     (::utl::wrapConfigurationElementName(m_sEvent));
    sKey.appendAscii(JobData::EVENTCFG_PATH_JOBLIST               );
    sKey.appendAscii("/"                                          );
    sKey.append     (::utl::wrapConfigurationElementName(m_sAlias));

    ConfigAccess aConfig(m_xSMGR, sKey.makeStringAndClear());
    aConfig.open(ConfigAccess::E_READWRITE);
    if (aConfig.getMode()==ConfigAccess::E_CLOSED)
        return;

    css::uno::Reference< css::beans::XPropertySet > xPropSet(aConfig.cfg(), css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        // Convert and write the user timestamp to the configuration.
        css::uno::Any aValue;
        aValue <<= Converter::convert_DateTime2ISO8601(DateTime());
        xPropSet->setPropertyValue(::rtl::OUString::createFromAscii(EVENTCFG_PROP_USERTIME), aValue);
    }

    aConfig.close();

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
 */
sal_Bool isEnabled( const ::rtl::OUString& sAdminTime ,
                    const ::rtl::OUString& sUserTime  )
{
    /*Attention!
        To prevent interpreting of TriGraphs inside next const string value,
        we have to encode all '?' signs. Otherwhise e.g. "??-" will be translated
        to "~" ...
     */
    static ::rtl::OUString PATTERN_ISO8601 = ::rtl::OUString::createFromAscii("\?\?\?\?-\?\?-\?\?*\0");

    sal_Bool bValidAdmin = Wildcard::match(sAdminTime, PATTERN_ISO8601);
    sal_Bool bValidUser  = Wildcard::match(sUserTime , PATTERN_ISO8601);

    // We check for "isEnabled()" here only.
    // Note further: ISO8601 formated strings can be compared as strings directly!
    return (
            (!bValidAdmin && !bValidUser                         ) ||
            ( bValidAdmin &&  bValidUser && sAdminTime>=sUserTime)
           );
}

//________________________________
/**
 */
css::uno::Sequence< ::rtl::OUString > JobData::getEnabledJobsForEvent( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                                                                       const ::rtl::OUString&                                        sEvent )
{
    // these static values may perform following loop for reading time stamp values ...
    static ::rtl::OUString ADMINTIME = ::rtl::OUString::createFromAscii(JobData::EVENTCFG_PROP_ADMINTIME);
    static ::rtl::OUString USERTIME  = ::rtl::OUString::createFromAscii(JobData::EVENTCFG_PROP_USERTIME );

    // generate the full qualified path to the job list, which is registered for the given event.
    // e.g. "/org.openoffice.Office.Jobs/Events/<event name>/JobList"
    ::rtl::OUStringBuffer sCfgEntry(256);
    sCfgEntry.appendAscii(JobData::EVENTCFG_ROOT                     );
    sCfgEntry.append     (::utl::wrapConfigurationElementName(sEvent));
    sCfgEntry.appendAscii(JobData::EVENTCFG_PATH_JOBLIST             );

    // create a config access
    ConfigAccess aConfig(xSMGR,sCfgEntry.makeStringAndClear());
    aConfig.open(ConfigAccess::E_READONLY);
    if (aConfig.getMode()==ConfigAccess::E_CLOSED)
        return css::uno::Sequence< ::rtl::OUString >();

    css::uno::Reference< css::container::XNameAccess > xEventRegistry(aConfig.cfg(), css::uno::UNO_QUERY);
    if (!xEventRegistry.is())
        return css::uno::Sequence< ::rtl::OUString >();

    // get all alias names of jobs, which are part of this job list
    // But Some of them can be disabled by it's time stamp values.
    // We create an additional job name list iwth the same size, then the original list ...
    // step over all job entries ... check her time stamps ... and put only job names to the
    // destination list, which represent an enabled job.
    css::uno::Sequence< ::rtl::OUString > lAllJobs = xEventRegistry->getElementNames();
    sal_Int32 c = lAllJobs.getLength();
    css::uno::Sequence< ::rtl::OUString > lEnabledJobs(c);
    sal_Int32 d = 0;
    for (sal_Int32 s=0; s<c; ++s)
    {
        css::uno::Any aNode = xEventRegistry->getByName(lAllJobs[s]);
        css::uno::Reference< css::beans::XPropertySet > xJob;
        if (
            !(aNode >>= xJob) ||
            !(xJob.is()     )
           )
        {
           continue;
        }

        aNode = xJob->getPropertyValue(ADMINTIME);
        ::rtl::OUString sAdminTime;
        aNode >>= sAdminTime;

        aNode = xJob->getPropertyValue(USERTIME);
        ::rtl::OUString sUserTime;
        aNode >>= sUserTime;

        if (!isEnabled(sAdminTime, sUserTime))
            continue;

        lEnabledJobs[d] = lAllJobs[s];
        ++d;
    }
    lEnabledJobs.realloc(d);

    aConfig.close();

    return lEnabledJobs;
}

//________________________________
/**
    @short      reset all internal structures
    @descr      If somehwere recycle this instance, he can switch from one
                using mode to another one. But then we have to reset all currently
                used informations. Otherwhise we mix it and they can make trouble.

                But note: that does not set defaults for internal used members, which
                does not relate to any job property! e.g. the reference to the global
                uno service manager. Such informations are used for internal processes only
                and are neccessary for our work.
 */
void JobData::impl_reset()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_eMode        = E_UNKNOWN_MODE;
    m_eEnvironment = E_UNKNOWN_ENVIRONMENT;
    m_sAlias       = ::rtl::OUString();
    m_sService     = ::rtl::OUString();
    m_sEvent       = ::rtl::OUString();
    m_lArguments   = css::uno::Sequence< css::beans::NamedValue >();
    aWriteLock.unlock();
    /* } SAFE */
}

} // namespace framework
