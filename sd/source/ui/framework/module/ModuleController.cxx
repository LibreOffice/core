/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModuleController.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-04-08 14:29:27 $
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

#include "precompiled_sd.hxx"

#include "framework/ModuleController.hxx"

#include "tools/ConfigurationAccess.hxx"
#include <comphelper/processfactory.hxx>
#include <comphelper/stl_types.hxx>
#include <boost/bind.hpp>
#include <hash_map>

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
    : public ::std::hash_map<
    rtl::OUString,
    rtl::OUString,
    ::comphelper::UStringHash,
    ::comphelper::UStringEqual>
{
public:
    ResourceToFactoryMap (void) {}
};


class ModuleController::LoadedFactoryContainer
    : public ::std::hash_map<
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
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.ModuleController"));
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
}




void ModuleController::LoadFactories (const Reference<XComponentContext>& rxContext)
{
    try
    {
        ConfigurationAccess aConfiguration (
            rxContext,
            OUString::createFromAscii("/org.openoffice.Office.Impress/"),
            ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode(
                OUString::createFromAscii("MultiPaneGUI/Framework/ResourceFactories")),
            UNO_QUERY);
        ::std::vector<rtl::OUString> aProperties (snFactoryPropertyCount);
        aProperties[0] = OUString::createFromAscii("ServiceName");
        aProperties[1] = OUString::createFromAscii("ResourceList");
        ConfigurationAccess::ForAll(
            xFactories,
            aProperties,
            ::boost::bind(&ModuleController::ProcessFactory, this, _2));
    }
    catch (Exception&)
    {
        OSL_TRACE("ERROR in ModuleController::LoadFactories");
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
        OUString::createFromAscii("URL"),
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
            OUString::createFromAscii("/org.openoffice.Office.Impress/"),
            tools::ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode(
                OUString::createFromAscii("MultiPaneGUI/Framework/StartupServices")),
            UNO_QUERY);
        ::std::vector<rtl::OUString> aProperties (snStartupPropertyCount);
        aProperties[0] = OUString::createFromAscii("ServiceName");
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
