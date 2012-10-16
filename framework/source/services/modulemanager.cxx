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


#include "services/modulemanager.hxx"
#include "services/frame.hxx"

#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <services.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/enumhelper.hxx>

#include <rtl/logfile.hxx>

namespace framework
{

static const char CFGPATH_FACTORIES[] = "/org.openoffice.Setup/Office/Factories";
static const char MODULEPROP_IDENTIFIER[] = "ooSetupFactoryModuleIdentifier";

rtl::OUString ModuleManager::impl_getStaticImplementationName() {
    return IMPLEMENTATIONNAME_MODULEMANAGER;
}

css::uno::Reference< css::lang::XSingleServiceFactory >
ModuleManager::impl_createFactory(
    css::uno::Reference< css::lang::XMultiServiceFactory > const & manager)
{
    return cppu::createSingleFactory(
        manager, impl_getStaticImplementationName(), &impl_createInstance,
        impl_getSupportedServiceNames());
}

css::uno::Sequence< rtl::OUString >
ModuleManager::impl_getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = DECLARE_ASCII("com.sun.star.frame.ModuleManager");
    return s;
}

css::uno::Reference< css::uno::XInterface > ModuleManager::impl_createInstance(
    css::uno::Reference< css::lang::XMultiServiceFactory > const & manager)
{
    return static_cast< cppu::OWeakObject * >(new ModuleManager(manager));
}

ModuleManager::ModuleManager(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase(     )
    , m_xSMGR       (xSMGR)
{
}

ModuleManager::~ModuleManager()
{
    if (m_xCFG.is())
        m_xCFG.clear();
}

rtl::OUString ModuleManager::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

sal_Bool ModuleManager::supportsService(rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > s(getSupportedServiceNames());
    for (sal_Int32 i = 0; i != s.getLength(); ++i) {
        if (s[i] == ServiceName) {
            return true;
        }
    }
    return false;
}

css::uno::Sequence< rtl::OUString > ModuleManager::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return impl_getSupportedServiceNames();
}

::rtl::OUString SAL_CALL ModuleManager::identify(const css::uno::Reference< css::uno::XInterface >& xModule)
    throw(css::lang::IllegalArgumentException,
          css::frame::UnknownModuleException,
          css::uno::RuntimeException         )
{
    // valid parameter?
    css::uno::Reference< css::frame::XFrame >      xFrame     (xModule, css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >       xWindow    (xModule, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XController > xController(xModule, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XModel >      xModel     (xModule, css::uno::UNO_QUERY);

    if (
        (!xFrame.is()     ) &&
        (!xWindow.is()    ) &&
        (!xController.is()) &&
        (!xModel.is()     )
       )
    {
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString("Given module is not a frame nor a window, controller or model."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);
    }

    if (xFrame.is())
    {
        xController = xFrame->getController();
        xWindow     = xFrame->getComponentWindow();
    }
    if (xController.is())
        xModel = xController->getModel();

    // modules are implemented by the deepest component in hierarchy ...
    // Means: model -> controller -> window
    // No fallbacks to higher components are allowed !
    // Note : A frame provides access to module components only ... but it's not a module by himself.

    ::rtl::OUString sModule;
    if (xModel.is())
        sModule = implts_identify(xModel);
    else if (xController.is())
        sModule = implts_identify(xController);
    else if (xWindow.is())
        sModule = implts_identify(xWindow);

    if (sModule.isEmpty())
        throw css::frame::UnknownModuleException(
                ::rtl::OUString("Cant find suitable module for the given component."),
                static_cast< ::cppu::OWeakObject* >(this));

    return sModule;
}

void SAL_CALL ModuleManager::replaceByName(const ::rtl::OUString& sName ,
                                           const css::uno::Any&   aValue)
    throw (css::lang::IllegalArgumentException   ,
           css::container::NoSuchElementException,
           css::lang::WrappedTargetException     ,
           css::uno::RuntimeException            )
{
    ::comphelper::SequenceAsHashMap lProps(aValue);
    if (lProps.empty() )
    {
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString("No properties given to replace part of module."),
                static_cast< cppu::OWeakObject * >(this),
                2);
    }

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // get access to the element
    // Note: Dont use impl_getConfig() method here. Because it creates a readonly access only, further
    // it cache it as a member of this module manager instance. If we change some props there ... but dont
    // flush changes (because an error occurred) we will read them later. If we use a different config access
    // we can close it without a flush ... and our read data wont be affected .-)
    css::uno::Reference< css::uno::XInterface >         xCfg      = ::comphelper::ConfigurationHelper::openConfig(
                                                                        xSMGR,
                                                                        rtl::OUString(CFGPATH_FACTORIES),
                                                                        ::comphelper::ConfigurationHelper::E_STANDARD);
    css::uno::Reference< css::container::XNameAccess >  xModules (xCfg, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameReplace > xModule  ;

    xModules->getByName(sName) >>= xModule;
    if (!xModule.is())
    {
        throw css::uno::RuntimeException(
                ::rtl::OUString("Was not able to get write access to the requested module entry inside configuration."),
                static_cast< cppu::OWeakObject * >(this));
    }

    ::comphelper::SequenceAsHashMap::const_iterator pProp;
    for (  pProp  = lProps.begin();
           pProp != lProps.end()  ;
         ++pProp                  )
    {
        const ::rtl::OUString& sPropName  = pProp->first;
        const css::uno::Any&   aPropValue = pProp->second;

        // let "NoSuchElementException" out ! We support the same API ...
        // and without a flush() at the end all changed data before will be ignored !
        xModule->replaceByName(sPropName, aPropValue);
    }

    ::comphelper::ConfigurationHelper::flush(xCfg);
}

css::uno::Any SAL_CALL ModuleManager::getByName(const ::rtl::OUString& sName)
    throw(css::container::NoSuchElementException,
          css::lang::WrappedTargetException     ,
          css::uno::RuntimeException            )
{
    // get access to the element
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_getConfig();
    css::uno::Reference< css::container::XNameAccess > xModule;
    xCFG->getByName(sName) >>= xModule;
    if (!xModule.is())
    {
        throw css::uno::RuntimeException(
                ::rtl::OUString("Was not able to get write access to the requested module entry inside configuration."),
                static_cast< cppu::OWeakObject * >(this));
    }

    // convert it to seq< PropertyValue >
    const css::uno::Sequence< ::rtl::OUString > lPropNames = xModule->getElementNames();
          ::comphelper::SequenceAsHashMap       lProps     ;
          sal_Int32                             c          = lPropNames.getLength();
          sal_Int32                             i          = 0;

    lProps[rtl::OUString(MODULEPROP_IDENTIFIER)] <<= sName;
    for (i=0; i<c; ++i)
    {
        const ::rtl::OUString& sPropName         = lPropNames[i];
                               lProps[sPropName] = xModule->getByName(sPropName);
    }

    return css::uno::makeAny(lProps.getAsConstPropertyValueList());
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL ModuleManager::getElementNames()
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_getConfig();
    return xCFG->getElementNames();
}

sal_Bool SAL_CALL ModuleManager::hasByName(const ::rtl::OUString& sName)
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_getConfig();
    return xCFG->hasByName(sName);
}

css::uno::Type SAL_CALL ModuleManager::getElementType()
    throw(css::uno::RuntimeException)
{
    return ::getCppuType((const css::uno::Sequence< css::beans::PropertyValue >*)0);
}

sal_Bool SAL_CALL ModuleManager::hasElements()
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNameAccess > xCFG = implts_getConfig();
    return xCFG->hasElements();
}

css::uno::Reference< css::container::XEnumeration > SAL_CALL ModuleManager::createSubSetEnumerationByQuery(const ::rtl::OUString&)
    throw(css::uno::RuntimeException)
{
    return css::uno::Reference< css::container::XEnumeration >();
}

css::uno::Reference< css::container::XEnumeration > SAL_CALL ModuleManager::createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
    throw(css::uno::RuntimeException)
{
    ::comphelper::SequenceAsHashMap                 lSearchProps (lProperties);
    css::uno::Sequence< ::rtl::OUString >           lModules     = getElementNames();
    sal_Int32                                       c            = lModules.getLength();
    sal_Int32                                       i            = 0;
    ::comphelper::SequenceAsVector< css::uno::Any > lResult      ;

    for (i=0; i<c; ++i)
    {
        try
        {
            const ::rtl::OUString&                sModule      = lModules[i];
                  ::comphelper::SequenceAsHashMap lModuleProps = getByName(sModule);

            if (lModuleProps.match(lSearchProps))
                lResult.push_back(css::uno::makeAny(lModuleProps.getAsConstPropertyValueList()));
        }
        catch(const css::uno::Exception&)
        {
        }
    }

    ::comphelper::OAnyEnumeration*                      pEnum = new ::comphelper::OAnyEnumeration(lResult.getAsConstList());
    css::uno::Reference< css::container::XEnumeration > xEnum(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY_THROW);
    return xEnum;
}

css::uno::Reference< css::container::XNameAccess > ModuleManager::implts_getConfig()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    if (m_xCFG.is())
        return m_xCFG;
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::uno::XInterface > xCfg;
    try
    {
        xCfg = ::comphelper::ConfigurationHelper::openConfig(
                    xSMGR,
                    rtl::OUString(CFGPATH_FACTORIES),
                    ::comphelper::ConfigurationHelper::E_READONLY);
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        xCfg.clear();
    }

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_xCFG = css::uno::Reference< css::container::XNameAccess >(xCfg, css::uno::UNO_QUERY_THROW);
    return m_xCFG;
    // <- SAFE ----------------------------------
}

::rtl::OUString ModuleManager::implts_identify(const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    // Search for an optional (!) interface XModule first.
    // Its used to overrule an existing service name. Used e.g. by our database form designer
    // which uses a writer module internaly.
    css::uno::Reference< css::frame::XModule > xModule(xComponent, css::uno::UNO_QUERY);
    if (xModule.is())
        return xModule->getIdentifier();

    // detect modules in a generic way ...
    // comparing service names with configured entries ...
    css::uno::Reference< css::lang::XServiceInfo > xInfo(xComponent, css::uno::UNO_QUERY);
    if (!xInfo.is())
        return ::rtl::OUString();

    const css::uno::Sequence< ::rtl::OUString > lKnownModules = getElementNames();
    const ::rtl::OUString*                      pKnownModules = lKnownModules.getConstArray();
          sal_Int32                             c             = lKnownModules.getLength();
          sal_Int32                             i             = 0;

    for (i=0; i<c; ++i)
    {
        if (xInfo->supportsService(pKnownModules[i]))
            return pKnownModules[i];
    }

    return ::rtl::OUString();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
