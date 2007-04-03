
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModuleController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:11:35 $
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

#ifndef SD_FRAMEWORK_MODULE_CONTROLLER_HXX
#define SD_FRAMEWORK_MODULE_CONTROLLER_HXX

#include "MutexOwner.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XMODULECONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XModuleController.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#include <boost/scoped_ptr.hpp>
#include <set>


namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    ::com::sun::star::drawing::framework::XModuleController,
    ::com::sun::star::lang::XInitialization
    > ModuleControllerInterfaceBase;

} // end of anonymous namespace.



namespace sd { namespace framework {

/** At the moment the ModuleController reads the
    org.openoffice.Office.Impress/MultiPaneGUI/Framework/ResourceFactories
    configuration data that maps from resource URLs to service names of
    factories that can create resources for the URLs.  When a resource
    controller, for example the PaneController, wants to create a pane for
    which it does not have a factory, it asks the ModuleController to
    provide one.  The ModuleController looks up the service name registere
    for the resource URL, in this case a pane URL and instantiates the
    service.  The service is expected to register on its creation a factory
    for the resource in question.

    This simple concept may be extended in the future.
*/
class ModuleController
    : private sd::MutexOwner,
      public ModuleControllerInterfaceBase
{
public:
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XModuleController>
        CreateInstance (
            const ::com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>&
            rxContext);

    virtual void SAL_CALL disposing (void);


    // XModuleController

    virtual void SAL_CALL requestResource(const ::rtl::OUString& rsResourceURL)
        throw (::com::sun::star::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
        com::sun::star::frame::XController> mxController;

    class ResourceToFactoryMap;
    ::boost::scoped_ptr<ResourceToFactoryMap> mpResourceToFactoryMap;
    class LoadedFactoryContainer;
    ::boost::scoped_ptr<LoadedFactoryContainer> mpLoadedFactories;

    ModuleController (
        const ::com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& rxContext)
        throw();
    ModuleController (void); // Not implemented.
    ModuleController (const ModuleController&); // Not implemented.
    virtual ~ModuleController (void) throw();
    void LoadFactories (void);
    void ProcessFactory (const ::std::vector<com::sun::star::uno::Any>& rValues);
};

} } // end of namespace sd::framework

#endif
