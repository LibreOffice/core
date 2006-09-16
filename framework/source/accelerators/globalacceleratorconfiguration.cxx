/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: globalacceleratorconfiguration.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:41:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_ACCELERATORS_GLOBALACCELERATORCONFIGURATION_HXX_
#include <accelerators/globalacceleratorconfiguration.hxx>
#endif

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORCONST_H_
#include <acceleratorconst.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COMPHELPER_LOCALE_HXX
#include <comphelper/locale.hxx>
#endif

//_______________________________________________
// const

namespace framework
{

//-----------------------------------------------
// XInterface, XTypeProvider, XServiceInfo
DEFINE_XINTERFACE_1(GlobalAcceleratorConfiguration           ,
                    AcceleratorConfiguration                 ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo))
DEFINE_XTYPEPROVIDER_1_WITH_BASECLASS(GlobalAcceleratorConfiguration,
                                      AcceleratorConfiguration      ,
                                      css::lang::XServiceInfo       )

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
    : AcceleratorConfiguration(xSMGR)
{
}

//-----------------------------------------------
GlobalAcceleratorConfiguration::~GlobalAcceleratorConfiguration()
{
    m_aPresetHandler.removeStorageListener(this);
}

//-----------------------------------------------
void GlobalAcceleratorConfiguration::impl_ts_fillCache()
{
    // get current office locale ... but dont cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superflous for this small implementation .-)
    ::comphelper::Locale aLocale = impl_ts_getLocale();

    // May be there exists no accelerator config? Handle it gracefully :-)
    try
    {
        // Note: The used preset class is threadsafe by itself ... and live if we live!
        // We do not need any mutex here.

        // open the folder, where the configuration exists
        m_aPresetHandler.connectToResource(
            PresetHandler::E_GLOBAL,
            PresetHandler::RESOURCETYPE_ACCELERATOR(),
            ::rtl::OUString(),
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
