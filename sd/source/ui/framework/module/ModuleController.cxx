/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModuleController.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:53:05 $
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


namespace sd { namespace framework {

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
    LoadFactories();
}




ModuleController::~ModuleController (void) throw()
{
}




void SAL_CALL ModuleController::disposing (void)
{
}



void ModuleController::LoadFactories (void)
{
    try
    {
        tools::ConfigurationAccess aConfiguration (
            OUString::createFromAscii("/org.openoffice.Office.Impress/"),
            tools::ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xFactories (
            aConfiguration.GetConfigurationNode(
                OUString::createFromAscii("MultiPaneGUI/Framework/ResourceFactories")),
            UNO_QUERY);
        ::std::vector<rtl::OUString> aProperties (2);
        aProperties[0] = OUString::createFromAscii("ServiceName");
        aProperties[1] = OUString::createFromAscii("ResourceList");
        tools::ConfigurationAccess::ForAll(
            xFactories,
            aProperties,
            ::boost::bind(&ModuleController::ProcessFactory, this, _1));
    }
    catch (Exception&)
    {
        OSL_TRACE("ERROR in ModuleController::LoadDependencies");
        // Ignore exception.
    }
}




void ModuleController::ProcessFactory (const ::std::vector<Any>& rValues)
{
    if (rValues.size() == 2)
    {
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

        // Add the resource URLs to the map.
        ::std::vector<rtl::OUString>::const_iterator iResource;
        for (iResource=aURLs.begin(); iResource!=aURLs.end(); ++iResource)
            (*mpResourceToFactoryMap)[*iResource] = sServiceName;
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
                xFactory = Reference<XInterface>(xGlobalFactory->createInstanceWithArguments(
                    iFactory->second,
                    aArguments));

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
            mxController = Reference<frame::XController>(aArguments[0], UNO_QUERY);
        }
        catch (RuntimeException&)
        {}
    }
}


} } // end of namespace sd::framework
