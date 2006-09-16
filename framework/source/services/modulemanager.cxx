/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: modulemanager.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:11:01 $
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

#include "services/modulemanager.hxx"
#include "services/frame.hxx"

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

//_______________________________________________
// other includes

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework
{

/*-----------------------------------------------
    04.12.2003 09:32
-----------------------------------------------*/
DEFINE_XINTERFACE_5(ModuleManager                                   ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                    DIRECT_INTERFACE(css::container::XNameAccess   ),
                    DIRECT_INTERFACE(css::container::XElementAccess),
                    DIRECT_INTERFACE(css::frame::XModuleManager   ))

/*-----------------------------------------------
    04.12.2003 09:32
-----------------------------------------------*/
DEFINE_XTYPEPROVIDER_5(ModuleManager                 ,
                       css::lang::XTypeProvider      ,
                       css::lang::XServiceInfo       ,
                       css::container::XNameAccess   ,
                       css::container::XElementAccess,
                       css::frame::XModuleManager   )

/*-----------------------------------------------
    04.12.2003 09:35
-----------------------------------------------*/
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE(ModuleManager                   ,
                                 ::cppu::OWeakObject             ,
                                 SERVICENAME_MODULEMANAGER       ,
                                 IMPLEMENTATIONNAME_MODULEMANAGER)

/*-----------------------------------------------
    04.12.2003 09:35
-----------------------------------------------*/
DEFINE_INIT_SERVICE(
                    ModuleManager,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

/*-----------------------------------------------
    04.12.2003 09:30
-----------------------------------------------*/
ModuleManager::ModuleManager(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase(     )
    , m_xSMGR       (xSMGR)
{
}

/*-----------------------------------------------
    10.12.2003 11:59
-----------------------------------------------*/
ModuleManager::~ModuleManager()
{
    if (m_xCFG.is())
        m_xCFG.clear();
}

/*-----------------------------------------------
    10.12.2003 11:02
-----------------------------------------------*/
::rtl::OUString SAL_CALL ModuleManager::identify(const css::uno::Reference< css::uno::XInterface >& xModule)
    throw(css::lang::IllegalArgumentException,
          css::frame::UnknownModuleException,
          css::uno::RuntimeException         )
{
    // valid parameter?
    css::uno::Reference< css::frame::XFrame >      xFrame     (xModule, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XController > xController(xModule, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XModel >      xModel     (xModule, css::uno::UNO_QUERY);

    if (
        (!xFrame.is()     ) &&
        (!xController.is()) &&
        (!xModel.is()     )
       )
    {
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("Not a XFrame, XController nor a XModel reference."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);
    }

    // frame empty?
    // controller/model?
    // all needed interfaces available?
    css::uno::Reference< css::lang::XServiceInfo > xInfo;
    if (xFrame.is())
        xController = xFrame->getController();
    if (xController.is())
        xModel = xController->getModel();

    if (xModel.is())
        xInfo = css::uno::Reference< css::lang::XServiceInfo >(xModel, css::uno::UNO_QUERY);
    else if (xController.is())
        xInfo = css::uno::Reference< css::lang::XServiceInfo >(xController, css::uno::UNO_QUERY);
    else if (xFrame.is()) // needed for detection of special modules (like e.g. help)!
        xInfo = css::uno::Reference< css::lang::XServiceInfo >(xFrame, css::uno::UNO_QUERY);

    if (!xInfo.is())
        throw css::frame::UnknownModuleException(
                ::rtl::OUString::createFromAscii("Cant classify given module."),
                static_cast< ::cppu::OWeakObject* >(this));

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    // TODO
    // Please migrate to new method impl_identifyNew() in case every office module
    // supports it right service name and can be asked directly ...

    // throws an UnknownModuleException in case module has no configuration!
    ::rtl::OUString sIdentifier = impl_identify(xInfo);
    aReadLock.unlock();
    // <- SAFE ----------------------------------
    return sIdentifier;
}

/*-----------------------------------------------
    10.12.2003 12:05
-----------------------------------------------*/
css::uno::Any SAL_CALL ModuleManager::getByName(const ::rtl::OUString& sName)
    throw(css::container::NoSuchElementException,
          css::lang::WrappedTargetException     ,
          css::uno::RuntimeException            )
{
    // get access to the element
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_openConfig();
    css::uno::Any aElement = xCFG->getByName(sName);

    css::uno::Reference< css::container::XNameAccess > xElement;
    aElement >>= xElement;
    if (!xElement.is())
    {
        throw css::container::NoSuchElementException(
                ::rtl::OUString::createFromAscii("The module configuration seems to be corrupted."),
                static_cast< css::container::XNameAccess* >(this));
    }

    // convert it to seq< PropertyValue >!
    const css::uno::Sequence< ::rtl::OUString > lProps = xElement->getElementNames();
    const ::rtl::OUString*                      pProps = lProps.getConstArray();
          sal_Int32                             c      = lProps.getLength();

    css::uno::Sequence< css::beans::PropertyValue > lElement(c);
    css::beans::PropertyValue*                      pElement = lElement.getArray();

    for (sal_Int32 i=0; i<c; ++i)
    {
        pElement[i].Name  = pProps[i];
        pElement[i].Value = xElement->getByName(pProps[i]);
    }

    /*  TODO
        a) add some implicit properties for readonly/mandatory ...
        b) catch exception during property access ...
           But how they should be handled?
     */

    return css::uno::makeAny(lElement);
}

/*-----------------------------------------------
    10.12.2003 11:58
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > SAL_CALL ModuleManager::getElementNames()
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_openConfig();
    return xCFG->getElementNames();
}

/*-----------------------------------------------
    10.12.2003 11:57
-----------------------------------------------*/
sal_Bool SAL_CALL ModuleManager::hasByName(const ::rtl::OUString& sName)
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_openConfig();
    return xCFG->hasByName(sName);
}

/*-----------------------------------------------
    10.12.2003 11:35
-----------------------------------------------*/
css::uno::Type SAL_CALL ModuleManager::getElementType()
    throw(css::uno::RuntimeException)
{
    return ::getCppuType((const css::uno::Sequence< css::beans::PropertyValue >*)0);
}

/*-----------------------------------------------
    10.12.2003 11:56
-----------------------------------------------*/
sal_Bool SAL_CALL ModuleManager::hasElements()
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_openConfig();
    return xCFG->hasElements();
}

/*-----------------------------------------------
    14.12.2003 09:45
-----------------------------------------------*/
css::uno::Reference< css::container::XNameAccess > ModuleManager::implts_openConfig()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    if (m_xCFG.is())
        return m_xCFG;

    css::uno::Reference< css::uno::XInterface > xCFG;
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            m_xSMGR->createInstance(SERVICENAME_CFGPROVIDER), css::uno::UNO_QUERY);

        if (!xConfigProvider.is())
            throw css::uno::RuntimeException(
                    ::rtl::OUString::createFromAscii("Could not locate configuration service."),
                    static_cast< css::container::XNameAccess* >(this));

        // set root path
        css::uno::Sequence< css::uno::Any > lParams(1);
        css::beans::PropertyValue           aParam;
        aParam.Name    = ::rtl::OUString::createFromAscii("nodepath");
        aParam.Value <<= ::rtl::OUString::createFromAscii("/org.openoffice.Setup/Office/Factories");
        lParams[0]   <<= aParam;

        // open it
        xCFG = xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS, lParams);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { xCFG.clear(); }

    m_xCFG = css::uno::Reference< css::container::XNameAccess >(xCFG, css::uno::UNO_QUERY);
    if (!m_xCFG.is())
    {
        throw css::uno::RuntimeException(
                ::rtl::OUString::createFromAscii("Could not open configuration package /org.openoffice.Setup/Office/Factories."),
                static_cast< css::container::XNameAccess* >(this));
    }

    return m_xCFG;
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    30.01.2004 07:54
-----------------------------------------------*/
::rtl::OUString ModuleManager::impl_identify(const css::uno::Reference< css::lang::XServiceInfo >& xModule)
    throw(css::frame::UnknownModuleException)
{
    const css::uno::Sequence< ::rtl::OUString > lKnownModules = getElementNames();
    const ::rtl::OUString*                      pKnownModules = lKnownModules.getConstArray();
          sal_Int32                             c             = lKnownModules.getLength();

    // detect modules in a generic way ...
    for (sal_Int32 m=0; m<c; ++m)
    {
        if (xModule->supportsService(pKnownModules[m]))
            return pKnownModules[m];
    }

    throw css::frame::UnknownModuleException(
            ::rtl::OUString::createFromAscii("Cant classify given module."),
            static_cast< ::cppu::OWeakObject* >(this));
}

} // namespace framework
