    /*************************************************************************
 *
 *  $RCSfile: jobdata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:43 $
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
const sal_Char* JobData::JOBCFG_PROP_ADMINTIME    = "AdminTime"                           ;
const sal_Char* JobData::JOBCFG_PROP_ARGUMENTS    = "Arguments"                           ;
const sal_Char* JobData::JOBCFG_PROP_ASYNC        = "Async"                               ;
const sal_Char* JobData::JOBCFG_PROP_SERVICE      = "Service"                             ;
const sal_Char* JobData::JOBCFG_PROP_USERTIME     = "UserTime"                            ;

const sal_Char* JobData::EVENTCFG_ROOT            = "/org.openoffice.Office.Jobs/Events/" ;
const sal_Char* JobData::EVENTCFG_PROP_JOBLIST    =  "Joblist"                            ;

const sal_Char* JobData::PROPSET_CONFIG           = "Config"                              ;
const sal_Char* JobData::PROPSET_OWNCONFIG        = "JobConfig"                           ;
const sal_Char* JobData::PROPSET_ENVIRONMENT      = "Environment"                         ;
const sal_Char* JobData::PROPSET_DYNAMICDATA      = "DynamicData"                         ;

const sal_Char* JobData::PROP_ADMINTIME           = "AdminTime"                           ;
const sal_Char* JobData::PROP_ALIAS               = "Alias"                               ;
const sal_Char* JobData::PROP_ASYNC               = "Async"                               ;
const sal_Char* JobData::PROP_EVENT               = "Event"                               ;
const sal_Char* JobData::PROP_FRAME               = "Frame"                               ;
const sal_Char* JobData::PROP_SERVICE             = "Service"                             ;
const sal_Char* JobData::PROP_USERTIME            = "UserTime"                            ;

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
JobData::JobData( /*IN*/ const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(&Application::GetSolarMutex())
    , m_eMode       (E_UNKNOWN                    )
    , m_xSMGR       (xSMGR                        )
{
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
    m_sAlias               = rCopy.m_sAlias              ;
    m_sService             = rCopy.m_sService            ;
    m_sEvent               = rCopy.m_sEvent              ;
    m_lArguments           = rCopy.m_lArguments          ;
    m_aAdminTime           = rCopy.m_aAdminTime          ;
    m_aUserTime            = rCopy.m_aUserTime           ;
    m_bAsync               = rCopy.m_bAsync              ;
    m_bEnabled             = rCopy.m_bEnabled            ;
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
void JobData::setAlias( /*IN*/ const ::rtl::OUString& sAlias )
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
        css::uno::Any   aValue;
        ::rtl::OUString sTime ;

        // read and convert "AdminTime"
        aValue = xJobProperties->getPropertyValue(::rtl::OUString::createFromAscii(JOBCFG_PROP_ADMINTIME));
        if (aValue >>= sTime)
            m_aAdminTime = Converter::convert_String2DateTime(sTime);

        // read and convert "UserTime"
        aValue = xJobProperties->getPropertyValue(::rtl::OUString::createFromAscii(JOBCFG_PROP_USERTIME));
        if (aValue >>= sTime)
            m_aUserTime = Converter::convert_String2DateTime(sTime);

        // check active state by comparing time stamps
        m_bEnabled = (m_aAdminTime > m_aUserTime);

        // read async state
        aValue   = xJobProperties->getPropertyValue(::rtl::OUString::createFromAscii(JOBCFG_PROP_ASYNC));
        aValue >>= m_bAsync;

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
void JobData::setService( /*IN*/ const ::rtl::OUString& sService )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    // delete all old informations! Otherwhise we mix it with the new one ...
    impl_reset();

    // take over the new informations
    m_sService = sService;
    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      a job can(!) be registered for such event
    @descr      This information can be set from outside as part of this container.
                We use it to add it to the argument list of the job ... but for temp. using only.
                So the outside code can pass the extended list of arguments to the job at execution time.
                But we don't make it persistent!

    @param      sEvent
                    the possible event, for which a job can be registered
*/
void JobData::setEvent( /*IN*/ const ::rtl::OUString& sEvent )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_sEvent = sEvent;
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
void JobData::setJobConfig( /*IN*/ const css::uno::Sequence< css::beans::NamedValue >& lArguments )
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
        lConfig.realloc(6);

        lConfig[0].Name = ::rtl::OUString::createFromAscii(PROP_ADMINTIME);
        lConfig[0].Value <<= Converter::convert_DateTime2String(m_aAdminTime);

        lConfig[1].Name = ::rtl::OUString::createFromAscii(PROP_ALIAS);
        lConfig[1].Value <<= m_sAlias;

        lConfig[2].Name = ::rtl::OUString::createFromAscii(PROP_ASYNC);
        lConfig[2].Value <<= m_bAsync;

        lConfig[3].Name = ::rtl::OUString::createFromAscii(PROP_EVENT);
        lConfig[3].Value <<= m_sEvent;

        lConfig[4].Name = ::rtl::OUString::createFromAscii(PROP_SERVICE);
        lConfig[4].Value <<= m_sService;

        lConfig[5].Name = ::rtl::OUString::createFromAscii(PROP_USERTIME);
        lConfig[5].Value <<= Converter::convert_DateTime2String(m_aUserTime);
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

sal_Bool JobData::isAsync() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_bAsync;
    /* } SAFE */
}

//________________________________

sal_Bool JobData::isEnabled() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_bEnabled;
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

    // No configuration => no chance
    if (m_eMode!=E_ALIAS)
        return;

    // actualize the user time stamp
    // It should be newer then the admin value ... and
    // that disable this job.
    // (default ctor of DateTime set current date & time on instanciated object automaticly!)
    m_aUserTime = DateTime();

    // But we have to actualize this state internaly too.
    m_bEnabled = sal_False;

    // actualize the configuration
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

    css::uno::Reference< css::beans::XPropertySet > xPropSet(aConfig.cfg(), css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        // Convert and write the user timestamp to the configuration.
        css::uno::Any aValue;
        aValue <<= Converter::convert_DateTime2String(m_aUserTime);
        xPropSet->setPropertyValue(::rtl::OUString::createFromAscii(JOBCFG_PROP_USERTIME), aValue);
    }

    aConfig.close();

    aWriteLock.unlock();
    /* } SAFE */
}

//________________________________
/**
    @short      reset all internal structures
    @descr      If somehwere recycle this instance, he can switch from one
                using mode to another one. But then we have to reset all currently
                used informations. Otherwhise we mix it and they can make trouble.
 */
void JobData::impl_reset()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_eMode      = E_UNKNOWN;
    m_sAlias     = ::rtl::OUString();
    m_sService   = ::rtl::OUString();
    m_sEvent     = ::rtl::OUString();
    m_lArguments = css::uno::Sequence< css::beans::NamedValue >();
    m_aAdminTime = DateTime();
    m_aUserTime  = DateTime();
    m_bAsync     = sal_False;
    m_bEnabled   = sal_False;
    aWriteLock.unlock();
    /* } SAFE */
}

} // namespace framework
