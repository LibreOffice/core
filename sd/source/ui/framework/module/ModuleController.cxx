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

#include "precompiled_sd.hxx"

#include "framework/ModuleController.hxx"

#include "tools/ConfigurationAccess.hxx"
#include <comphelper/processfactory.hxx>
#include <comphelper/stl_types.hxx>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;
using ::sd::tools::ConfigurationAccess;

#undef VERBOSE
//#define VERBOSE 2

namespace sd { namespace framework {

static const sal_uInt32 snFactoryPropertyCount (2);
static const sal_uInt32 snStartupPropertyCount (1);




class ModuleController::ResourceToFactoryMap
    : public ::boost::unordered_map<
    rtl::OUString,
    rtl::OUString,
    ::comphelper::UStringHash,
    ::comphelper::UStringEqual>
{
public:
    ResourceToFactoryMap (void) {}
};


class ModuleController::LoadedFactoryContainer
    : public ::boost::unordered_map<
    rtl::OUString,
    WeakReference<XInterface>,
    ::comphelper::UStringHash,
    ::comphelper::UStringEqual>
{
public:
    LoadedFactoryContainer (void) {}
};





Reference<XInterface> SAL_CALL ModuleController_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(ModuleController::CreateInstance(rxContext), UNO_QUERY);
}




::rtl::OUString ModuleController_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.framework.module.ModuleController"));
}




Sequence<rtl::OUString> SAL_CALL ModuleController_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.framework.ModuleController"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== ModuleController ======================================================

Reference<XModuleController> ModuleController::CreateInstance (
    const Reference<XComponentContext>& rxContext)
{
    return new ModuleController(rxContext);
}




ModuleController::ModuleController (const Reference<XComponentContext>& rxContext) throw()
    : ModuleControllerInterfaceBase(MutexOwner::maMutex),
      mxController(),
      mpResourceToFactoryMap(new ResourceToFactoryMap()),
      mpLoadedFactories(new LoadedFactoryContainer())
{
    (void)rxContext;
    LoadFactories(rxContext);
}




ModuleController::~ModuleController (void) throw()
{
}




void SAL_CALL ModuleController::disposing (void)
{
    // Break the cyclic reference back to DrawController object
    mpLoadedFactories.reset();
    mpResourceToFactoryMap.reset();
    mxController.clear();
}




void ModuleController::LoadFactories (const Reference<XComponentContext>& rxContext)
{
    try
    {
        ConfigurationAccess aConfiguration (
            rxContext,
            OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Impress/")),
            ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode(
                OUString(RTL_CONSTASCII_USTRINGPARAM("MultiPaneGUI/Framework/ResourceFactories"))),
            UNO_QUERY);
        ::std::vector<rtl::OUString> aProperties (snFactoryPropertyCount);
        aProperties[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("ServiceName"));
        aProperties[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("ResourceList"));
        ConfigurationAccess::ForAll(
            xFactories,
            aProperties,
            ::boost::bind(&ModuleController::ProcessFactory, this, _2));
    }
    catch (Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




void ModuleController::ProcessFactory (const ::std::vector<Any>& rValues)
{
    OSL_ASSERT(rValues.size() == snFactoryPropertyCount);

    // Get the service name of the factory.
    rtl::OUString sServiceName;
    rValues[0] >>= sServiceName;

    // Get all resource URLs that are created by the factory.
    Reference<container::XNameAccess> xResources (rValues[1], UNO_QUERY);
    ::std::vector<rtl::OUString> aURLs;
    tools::ConfigurationAccess::FillList(
        xResources,
        OUString(RTL_CONSTASCII_USTRINGPARAM("URL")),
        aURLs);

#if defined VERBOSE && VERBOSE>0
    OSL_TRACE("ModuleController::adding factory %s",
        OUStringToOString(sServiceName, RTL_TEXTENCODING_UTF8).getStr());
#endif

    // Add the resource URLs to the map.
    ::std::vector<rtl::OUString>::const_iterator iResource;
    for (iResource=aURLs.begin(); iResource!=aURLs.end(); ++iResource)
    {
        (*mpResourceToFactoryMap)[*iResource] = sServiceName;
#if defined VERBOSE && VERBOSE>1
        OSL_TRACE("    %s",
            OUStringToOString(*iResource, RTL_TEXTENCODING_UTF8).getStr());
#endif
    }
}




void ModuleController::InstantiateStartupServices (void)
{
    try
    {
        tools::ConfigurationAccess aConfiguration (
            OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Impress/")),
            tools::ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode(
                OUString(RTL_CONSTASCII_USTRINGPARAM("MultiPaneGUI/Framework/StartupServices"))),
            UNO_QUERY);
        ::std::vector<rtl::OUString> aProperties (snStartupPropertyCount);
        aProperties[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("ServiceName"));
        tools::ConfigurationAccess::ForAll(
            xFactories,
            aProperties,
            ::boost::bind(&ModuleController::ProcessStartupService, this, _2));
    }
    catch (Exception&)
    {
        OSL_TRACE("ERROR in ModuleController::InstantiateStartupServices");
    }
}




void ModuleController::ProcessStartupService (const ::std::vector<Any>& rValues)
{
    OSL_ASSERT(rValues.size() == snStartupPropertyCount);

    try
    {
        // Get the service name of the startup service.
        rtl::OUString sServiceName;
        rValues[0] >>= sServiceName;

        // Instantiate service.
        Reference<lang::XMultiServiceFactory> xGlobalFactory (
            ::comphelper::getProcessServiceFactory(), UNO_QUERY);
        if (xGlobalFactory.is())
        {
            // Create the startup service.
            Sequence<Any> aArguments(1);
            aArguments[0] <<= mxController;
            // Note that when the new object will be destroyed at the end of
            // this scope when it does not register itself anywhere.
            // Typically it will add itself as ConfigurationChangeListener
            // at the configuration controller.
            xGlobalFactory->createInstanceWithArguments(sServiceName, aArguments);

#if defined VERBOSE && VERBOSE>0
            OSL_TRACE("ModuleController::created startup service %s",
                OUStringToOString(sServiceName, RTL_TEXTENCODING_UTF8).getStr());
#endif
        }
    }
    catch (Exception&)
    {
        OSL_TRACE("ERROR in ModuleController::ProcessStartupServices");
    }
}




//----- XModuleController -----------------------------------------------------

void SAL_CALL ModuleController::requestResource (const OUString& rsResourceURL)
    throw (RuntimeException)
{
    ResourceToFactoryMap::const_iterator iFactory (mpResourceToFactoryMap->find(rsResourceURL));
    if (iFactory != mpResourceToFactoryMap->end())
    {
        // Check that the factory has already been loaded and not been
        // destroyed in the meantime.
        Reference<XInterface> xFactory;
        LoadedFactoryContainer::const_iterator iLoadedFactory (
            mpLoadedFactories->find(iFactory->second));
        if (iLoadedFactory != mpLoadedFactories->end())
            xFactory = Reference<XInterface>(iLoadedFactory->second, UNO_QUERY);
        if ( ! xFactory.is())
        {
            // Create a new instance of the factory.
            Reference<lang::XMultiServiceFactory> xGlobalFactory (
                ::comphelper::getProcessServiceFactory(), UNO_QUERY);
            if (xGlobalFactory.is())
            {
                // Create the factory service.
                Sequence<Any> aArguments(1);
                aArguments[0] <<= mxController;
                xFactory = xGlobalFactory->createInstanceWithArguments(
                    iFactory->second,
                    aArguments);

                // Remember that this factory has been instanced.
                (*mpLoadedFactories)[iFactory->second] = xFactory;
            }
        }
    }
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL ModuleController::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            // Get the XController from the first argument.
            mxController = Reference<frame::XController>(aArguments[0], UNO_QUERY_THROW);

            InstantiateStartupServices();
        }
        catch (RuntimeException&)
        {}
    }
}


} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
