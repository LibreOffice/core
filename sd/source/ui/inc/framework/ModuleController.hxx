/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ModuleController.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SD_FRAMEWORK_MODULE_CONTROLLER_HXX
#define SD_FRAMEWORK_MODULE_CONTROLLER_HXX

#include "MutexOwner.hxx"

#include <osl/mutex.hxx>
#include <com/sun/star/drawing/framework/XModuleController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/compbase2.hxx>

#include <boost/scoped_ptr.hpp>
#include <set>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    css::drawing::framework::XModuleController,
    css::lang::XInitialization
    > ModuleControllerInterfaceBase;

} // end of anonymous namespace.



namespace sd { namespace framework {

/** The ModuleController has to tasks:

    1. It reads the
    org.openoffice.Office.Impress/MultiPaneGUI/Framework/ResourceFactories
    configuration data that maps from resource URLs to service names of
    factories that can create resources for the URLs.  When the
    configuration controller wants to create a resource for which it does
    not have a factory, it asks the ModuleController to provide one.  The
    ModuleController looks up the service name registered for the URL of the
    resource and instantiates this service.  The service is expected to
    register on its creation a factory for the resource in question.

    2. The ModuleController reads on its creation
    org.openoffice.Office.Impress/MultiPaneGUI/Framework/StartupServices
    configuration data and instantiates all listed services.  These servces
    can then register as listeners at the ConfigurationController or do
    whatever they like.
*/
class ModuleController
    : private sd::MutexOwner,
      public ModuleControllerInterfaceBase
{
public:
    static css::uno::Reference<
        css::drawing::framework::XModuleController>
        CreateInstance (
            const css::uno::Reference<css::uno::XComponentContext>&
            rxContext);

    virtual void SAL_CALL disposing (void);


    // XModuleController

    virtual void SAL_CALL requestResource(const ::rtl::OUString& rsResourceURL)
        throw (css::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

private:
    css::uno::Reference<
        css::frame::XController> mxController;

    class ResourceToFactoryMap;
    ::boost::scoped_ptr<ResourceToFactoryMap> mpResourceToFactoryMap;
    class LoadedFactoryContainer;
    ::boost::scoped_ptr<LoadedFactoryContainer> mpLoadedFactories;

    ModuleController (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        throw();
    ModuleController (void); // Not implemented.
    ModuleController (const ModuleController&); // Not implemented.
    virtual ~ModuleController (void) throw();

    /** Load a list of URL to service mappings from the
        /org.openoffice.Office.Impress/MultiPaneGUI/Framework/ResourceFactories
        configuration entry.  The mappings are stored in the
        mpResourceToFactoryMap member.
    */
    void LoadFactories (const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    /** Called for every entry in the ResourceFactories configuration entry.
    */
    void ProcessFactory (const ::std::vector<css::uno::Any>& rValues);

    /** Instantiate all startup services that are found in the
        /org.openoffice.Office.Impress/MultiPaneGUI/Framework/StartupServices
        configuration entry.  This method is called once when a new
        ModuleController object is created.
    */
    void InstantiateStartupServices (void);

    /** Called for one entry in the StartupServices configuration list this
        method instantiates the service described by the entry.  It does not
        hold references to the new object so that the object will be
        destroyed on function exit when it does not register itself
        somewhere.  It typically will register as
        XConfigurationChangeListener at the configuration controller.
    */
    void ProcessStartupService (const ::std::vector<css::uno::Any>& rValues);
};

} } // end of namespace sd::framework

#endif
