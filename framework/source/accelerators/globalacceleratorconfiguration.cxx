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

#include <accelerators/globalacceleratorconfiguration.hxx>

#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include "helper/mischelper.hxx"

#include <acceleratorconst.h>
#include <services.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

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
        m_xCfgListener = new WeakChangesListener(this);
        xBroadcaster->addChangesListener(m_xCfgListener);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
