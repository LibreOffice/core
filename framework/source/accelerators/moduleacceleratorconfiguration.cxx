/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: moduleacceleratorconfiguration.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <accelerators/moduleacceleratorconfiguration.hxx>

//_______________________________________________
// own includes
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#ifndef __FRAMEWORK_ACCELERATORCONST_H_
#include <acceleratorconst.h>
#endif
#include <services.h>

//_______________________________________________
// interface includes
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

//_______________________________________________
// other includes

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX
#include <comphelper/sequenceashashmap.hxx>
#endif
#include <vcl/svapp.hxx>

//_______________________________________________
// const

namespace framework
{

//-----------------------------------------------
// XInterface, XTypeProvider, XServiceInfo
DEFINE_XINTERFACE_2(ModuleAcceleratorConfiguration              ,
                    AcceleratorConfiguration                    ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo)   ,
                    DIRECT_INTERFACE(css::lang::XInitialization))

DEFINE_XTYPEPROVIDER_2_WITH_BASECLASS(ModuleAcceleratorConfiguration,
                                      AcceleratorConfiguration      ,
                                      css::lang::XServiceInfo       ,
                                      css::lang::XInitialization    )

DEFINE_XSERVICEINFO_MULTISERVICE(ModuleAcceleratorConfiguration                   ,
                                 ::cppu::OWeakObject                              ,
                                 SERVICENAME_MODULEACCELERATORCONFIGURATION       ,
                                 IMPLEMENTATIONNAME_MODULEACCELERATORCONFIGURATION)

DEFINE_INIT_SERVICE(ModuleAcceleratorConfiguration,
                    {
                        /*Attention
                        I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                        to create a new instance of this class by our own supported service factory.
                        see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//-----------------------------------------------
ModuleAcceleratorConfiguration::ModuleAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR)
    : AcceleratorConfiguration(xSMGR)
{
}

//-----------------------------------------------
ModuleAcceleratorConfiguration::~ModuleAcceleratorConfiguration()
{
    m_aPresetHandler.removeStorageListener(this);
}

//-----------------------------------------------
void SAL_CALL ModuleAcceleratorConfiguration::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    ::comphelper::SequenceAsHashMap lArgs(lArguments);
    m_sModule = lArgs.getUnpackedValueOrDefault(::rtl::OUString::createFromAscii("ModuleIdentifier"), ::rtl::OUString());

    if (!m_sModule.getLength())
        throw css::uno::RuntimeException(
                ::rtl::OUString::createFromAscii("The module dependend accelerator configuration service was initialized with an empty module identifier!"),
                static_cast< ::cppu::OWeakObject* >(this));

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    impl_ts_fillCache();
}

//-----------------------------------------------
void ModuleAcceleratorConfiguration::impl_ts_fillCache()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    ::rtl::OUString sModule = m_sModule;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // get current office locale ... but dont cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superflous for this small implementation .-)
    ::comphelper::Locale aLocale = impl_ts_getLocale();

    // May be the current app module does not have any
    // accelerator config? Handle it gracefully :-)
    try
    {
        // Note: The used preset class is threadsafe by itself ... and live if we live!
        // We do not need any mutex here.

        // open the folder, where the configuration exists
        m_aPresetHandler.connectToResource(
            PresetHandler::E_MODULES,
            PresetHandler::RESOURCETYPE_ACCELERATOR(),
            sModule,
            css::uno::Reference< css::embed::XStorage >(),
            aLocale);

        // check if the user already has a current configuration
        // if not - se the default preset as new current one.
        // means: copy "share/default.xml" => "user/current.xml"
        if (!m_aPresetHandler.existsTarget(PresetHandler::TARGET_CURRENT()))
            m_aPresetHandler.copyPresetToTarget(PresetHandler::PRESET_DEFAULT(), PresetHandler::TARGET_CURRENT());

        AcceleratorConfiguration::reload();
        m_aPresetHandler.addStorageListener(this);
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}
}

} // namespace framework

