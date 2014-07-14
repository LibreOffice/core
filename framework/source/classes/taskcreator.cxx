/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <classes/taskcreator.hxx>
#include "services/taskcreatorsrv.hxx"
#include <threadhelp/readguard.hxx>
#include <loadenv/targethelper.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <comphelper/configurationhelper.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  includes of my own project
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-****************************************************************************************************//**
    @short      initialize instance with necessary informations
    @descr      We need a valid uno service manager to create or instanciate new services.
                All other informations to create frames or tasks come in on right interface methods.

    @param      xSMGR
                    points to the valid uno service manager

    @modified   16.05.2002 09:25, as96863
*//*-*****************************************************************************************************/
TaskCreator::TaskCreator( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(       )
    , m_xSMGR       ( xSMGR )
{
}

/*-****************************************************************************************************//**
    @short      deinitialize instance
    @descr      We should release all used ressource which are not needed any longer.

    @modified   16.05.2002 09:33, as96863
*//*-*****************************************************************************************************/
TaskCreator::~TaskCreator()
{
    m_xSMGR.clear();
}

/*-****************************************************************************************************//**
    TODO document me
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > TaskCreator::createTask( const ::rtl::OUString& sName    ,
                                                                         sal_Bool         bVisible )
{
    static ::rtl::OUString PACKAGE = ::rtl::OUString::createFromAscii("org.openoffice.Office.TabBrowse");
    static ::rtl::OUString RELPATH = ::rtl::OUString::createFromAscii("TaskCreatorService"             );
    static ::rtl::OUString KEY     = ::rtl::OUString::createFromAscii("ImplementationName"             );

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::lang::XSingleServiceFactory > xCreator;
    ::rtl::OUString sCreator = IMPLEMENTATIONNAME_FWK_TASKCREATOR;

    try
    {
        if (
            ( TargetHelper::matchSpecialTarget(sName, TargetHelper::E_BLANK  ) ) ||
            ( TargetHelper::matchSpecialTarget(sName, TargetHelper::E_DEFAULT) )
           )
        {
            ::comphelper::ConfigurationHelper::readDirectKey(xSMGR, PACKAGE, RELPATH, KEY, ::comphelper::ConfigurationHelper::E_READONLY) >>= sCreator;
        }

        xCreator = css::uno::Reference< css::lang::XSingleServiceFactory >(
                    xSMGR->createInstance(sCreator), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::Exception&)
    {}

    // no catch here ... without an task creator service we can't open ANY document window within the office.
    // Thats IMHO not a good idea. Then we should accept the stacktrace showing us the real problem.
    // BTW: The used fallback creator service (IMPLEMENTATIONNAME_FWK_TASKCREATOR) is implemented in the same
    // library then these class here ... Why we should not be able to create it ?
    if ( ! xCreator.is())
        xCreator = css::uno::Reference< css::lang::XSingleServiceFactory >(
                    xSMGR->createInstance(IMPLEMENTATIONNAME_FWK_TASKCREATOR), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lArgs(5);
    css::beans::NamedValue              aArg    ;

    aArg.Name    = TaskCreatorService::ARGUMENT_PARENTFRAME;
    aArg.Value <<= css::uno::Reference< css::frame::XFrame >(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
    lArgs[0]   <<= aArg;

    aArg.Name    = TaskCreatorService::ARGUMENT_CREATETOPWINDOW;
    aArg.Value <<= sal_True;
    lArgs[1]   <<= aArg;

    aArg.Name    = TaskCreatorService::ARGUMENT_MAKEVISIBLE;
    aArg.Value <<= bVisible;
    lArgs[2]   <<= aArg;

    aArg.Name    = TaskCreatorService::ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE;
    aArg.Value <<= sal_True;
    lArgs[3]   <<= aArg;

    aArg.Name    = TaskCreatorService::ARGUMENT_FRAMENAME;
    aArg.Value <<= sName;
    lArgs[4]   <<= aArg;

    css::uno::Reference< css::frame::XFrame > xTask(xCreator->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
    return xTask;
}

} // namespace framework
