/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <framework/ModuleController.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <tools/ConfigurationAccess.hxx>
#include <comphelper/processfactory.hxx>
#include <unordered_map>

#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <facreg.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::tools::ConfigurationAccess;

namespace sd { namespace framework {

static const sal_uInt32 snFactoryPropertyCount (2);
static const sal_uInt32 snStartupPropertyCount (1);

class ModuleController::ResourceToFactoryMap
    : public std::unordered_map<
    OUString,
    OUString>
{
public:
    ResourceToFactoryMap() {}
};

class ModuleController::LoadedFactoryContainer
    : public std::unordered_map<
    OUString,
    WeakReference<XInterface>>
{
public:
    LoadedFactoryContainer() {}
};

//===== ModuleController ======================================================
Reference<XModuleController> ModuleController::CreateInstance (
    const Reference<XComponentContext>& rxContext)
{
    return new ModuleController(rxContext);
}

ModuleController::ModuleController (const Reference<XComponentContext>& rxContext)
    : ModuleControllerInterfaceBase(MutexOwner::maMutex),
      mxController(),
      mpResourceToFactoryMap(new ResourceToFactoryMap()),
      mpLoadedFactories(new LoadedFactoryContainer())
{
    /** Load a list of URL to service mappings from the
        /org.openoffice.Office.Impress/MultiPaneGUI/Framework/ResourceFactories
        configuration entry.  The mappings are stored in the
        mpResourceToFactoryMap member.
    */
    try
    {
        ConfigurationAccess aConfiguration (
            rxContext,
            "/org.openoffice.Office.Impress/",
            ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode("MultiPaneGUI/Framework/ResourceFactories"),
            UNO_QUERY);
        ::std::vector<OUString> aProperties (snFactoryPropertyCount);
        aProperties[0] = "ServiceName";
        aProperties[1] = "ResourceList";
        ConfigurationAccess::ForAll(
            xFactories,
            aProperties,
            [this] (OUString const&, ::std::vector<Any> const& xs) {
                return this->ProcessFactory(xs);
            } );
    }
    catch (Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

ModuleController::~ModuleController() throw()
{
}

void SAL_CALL ModuleController::disposing()
{
    // Break the cyclic reference back to DrawController object
    mpLoadedFactories.reset();
    mpResourceToFactoryMap.reset();
    mxController.clear();
}

void ModuleController::ProcessFactory (const ::std::vector<Any>& rValues)
{
    OSL_ASSERT(rValues.size() == snFactoryPropertyCount);

    // Get the service name of the factory.
    OUString sServiceName;
    rValues[0] >>= sServiceName;

    // Get all resource URLs that are created by the factory.
    Reference<container::XNameAccess> xResources (rValues[1], UNO_QUERY);
    ::std::vector<OUString> aURLs;
    tools::ConfigurationAccess::FillList(
        xResources,
        "URL",
        aURLs);

    SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ModuleController::adding factory " << sServiceName);

    // Add the resource URLs to the map.
    for (const auto& rResource : aURLs)
    {
        (*mpResourceToFactoryMap)[rResource] = sServiceName;
        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ":    " << rResource);
    }
}

void ModuleController::InstantiateStartupServices()
{
    try
    {
        tools::ConfigurationAccess aConfiguration (
            "/org.openoffice.Office.Impress/",
            tools::ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode("MultiPaneGUI/Framework/StartupServices"),
            UNO_QUERY);
        ::std::vector<OUString> aProperties (snStartupPropertyCount);
        aProperties[0] = "ServiceName";
        tools::ConfigurationAccess::ForAll(
            xFactories,
            aProperties,
            [this] (OUString const&, ::std::vector<Any> const& xs) {
                return this->ProcessStartupService(xs);
            } );
    }
    catch (Exception&)
    {
        SAL_WARN("sd.fwk", "ERROR in ModuleController::InstantiateStartupServices");
    }
}

void ModuleController::ProcessStartupService (const ::std::vector<Any>& rValues)
{
    OSL_ASSERT(rValues.size() == snStartupPropertyCount);

    try
    {
        // Get the service name of the startup service.
        OUString sServiceName;
        rValues[0] >>= sServiceName;

        // Instantiate service.
        Reference<uno::XComponentContext> xContext =
            ::comphelper::getProcessComponentContext();

        // Create the startup service.
        Sequence<Any> aArguments(1);
        aArguments[0] <<= mxController;
        // Note that when the new object will be destroyed at the end of
        // this scope when it does not register itself anywhere.
        // Typically it will add itself as ConfigurationChangeListener
        // at the configuration controller.
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(sServiceName, aArguments, xContext);

        SAL_INFO("sd.fwk", OSL_THIS_FUNC << ": ModuleController::created startup service " << sServiceName);
    }
    catch (Exception&)
    {
        SAL_WARN("sd.fwk", "ERROR in ModuleController::ProcessStartupServices");
    }
}

//----- XModuleController -----------------------------------------------------

void SAL_CALL ModuleController::requestResource (const OUString& rsResourceURL)
{
    ResourceToFactoryMap::const_iterator iFactory (mpResourceToFactoryMap->find(rsResourceURL));
    if (iFactory == mpResourceToFactoryMap->end())
        return;

    // Check that the factory has already been loaded and not been
    // destroyed in the meantime.
    Reference<XInterface> xFactory;
    LoadedFactoryContainer::const_iterator iLoadedFactory (
        mpLoadedFactories->find(iFactory->second));
    if (iLoadedFactory != mpLoadedFactories->end())
        xFactory.set(iLoadedFactory->second, UNO_QUERY);
    if (  xFactory.is())
        return;

    // Create a new instance of the factory.
    Reference<uno::XComponentContext> xContext =
        ::comphelper::getProcessComponentContext();

    // Create the factory service.
    Sequence<Any> aArguments(1);
    aArguments[0] <<= mxController;
    try
    {
        xFactory = xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            iFactory->second,
            aArguments,
            xContext);
    }
    catch (const Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sd.fwk", "caught exception while creating factory. " << exceptionToString(ex));
    }

    // Remember that this factory has been instanced.
    (*mpLoadedFactories)[iFactory->second] = xFactory;
}

//----- XInitialization -------------------------------------------------------

void SAL_CALL ModuleController::initialize (const Sequence<Any>& aArguments)
{
    if (aArguments.getLength() > 0)
    {
        try
        {
            // Get the XController from the first argument.
            mxController.set(aArguments[0], UNO_QUERY_THROW);

            InstantiateStartupServices();
        }
        catch (RuntimeException&)
        {}
    }
}

} } // end of namespace sd::framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_framework_module_ModuleController_get_implementation(
        css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    css::uno::Reference< css::uno::XInterface > xModCont ( sd::framework::ModuleController::CreateInstance(context) );
    xModCont->acquire();
    return xModCont.get();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
