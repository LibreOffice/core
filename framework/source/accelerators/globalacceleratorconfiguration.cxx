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
#include <accelerators/globalacceleratorconfiguration.hxx>

//_______________________________________________
// own includes
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#include <acceleratorconst.h>
#include <services.h>

//_______________________________________________
// interface includes
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

//_______________________________________________
// other includes
#include <vcl/svapp.hxx>
#include <comphelper/locale.hxx>
#include <comphelper/configurationhelper.hxx>

//_______________________________________________
// const

namespace framework
{

//-----------------------------------------------
// XInterface, XTypeProvider, XServiceInfo
DEFINE_XINTERFACE_2(GlobalAcceleratorConfiguration           ,
                    XCUBasedAcceleratorConfiguration                 ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo),
                    DIRECT_INTERFACE(css::lang::XInitialization))
DEFINE_XTYPEPROVIDER_2_WITH_BASECLASS(GlobalAcceleratorConfiguration,
                                      XCUBasedAcceleratorConfiguration      ,
                                      css::lang::XServiceInfo       ,
                                      css::lang::XInitialization)

DEFINE_XSERVICEINFO_MULTISERVICE(GlobalAcceleratorConfiguration                   ,
                                 ::cppu::OWeakObject                              ,
                                 SERVICENAME_GLOBALACCELERATORCONFIGURATION       ,
                                 IMPLEMENTATIONNAME_GLOBALACCELERATORCONFIGURATION)

DEFINE_INIT_SERVICE(GlobalAcceleratorConfiguration,
                    {
                        /*Attention
                        I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                        to create a new instance of this class by our own supported service factory.
                        see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                        impl_ts_fillCache();
                    }
                   )

//-----------------------------------------------
GlobalAcceleratorConfiguration::GlobalAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR)
    : XCUBasedAcceleratorConfiguration(xSMGR)
{
}

//-----------------------------------------------
GlobalAcceleratorConfiguration::~GlobalAcceleratorConfiguration()
{
}

void SAL_CALL GlobalAcceleratorConfiguration::initialize(const css::uno::Sequence< css::uno::Any >& /*lArguments*/)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
}

//-----------------------------------------------
void GlobalAcceleratorConfiguration::impl_ts_fillCache()
{
    // get current office locale ... but dont cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superflous for this small implementation .-)
    ::comphelper::Locale aLocale = ::comphelper::Locale(m_sLocale);

    // May be there exists no accelerator config? Handle it gracefully :-)
    try
    {
        m_sGlobalOrModules = CFG_ENTRY_GLOBAL;
        XCUBasedAcceleratorConfiguration::reload();

        css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(m_xCfg, css::uno::UNO_QUERY_THROW);
        xBroadcaster->addChangesListener(static_cast< css::util::XChangesListener* >(this));
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}
}

} // namespace framework
