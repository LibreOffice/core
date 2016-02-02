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

#include <sal/config.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/enumhelper.hxx>

#include <boost/noncopyable.hpp>

namespace {

class ModuleManager:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::frame::XModuleManager2,
        css::container::XContainerQuery >,
    private boost::noncopyable
{
private:

    /** the global uno service manager.
        Must be used to create own needed services.
     */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /** points to the underlying configuration.
        This ModuleManager does not cache - it calls directly the
        configuration API!
      */
    css::uno::Reference< css::container::XNameAccess > m_xCFG;

public:

    ModuleManager(const css::uno::Reference< css::uno::XComponentContext >& xContext);

    virtual ~ModuleManager();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(
        OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XModuleManager
    virtual OUString SAL_CALL identify(const css::uno::Reference< css::uno::XInterface >& xModule)
        throw(css::lang::IllegalArgumentException,
              css::frame::UnknownModuleException,
              css::uno::RuntimeException, std::exception         ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName(const OUString& sName ,
                                        const css::uno::Any&   aValue)
        throw (css::lang::IllegalArgumentException   ,
               css::container::NoSuchElementException,
               css::lang::WrappedTargetException     ,
               css::uno::RuntimeException, std::exception            ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& sName)
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException     ,
              css::uno::RuntimeException, std::exception            ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasByName(const OUString& sName)
        throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasElements()
        throw(css::uno::RuntimeException, std::exception) override;

    // XContainerQuery
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery(const OUString& sQuery)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
        throw(css::uno::RuntimeException, std::exception) override;

private:

    /** @short  makes the real identification of the module.

        @descr  It checks for the optional but preferred interface
                XModule first. If this module does not exists at the
                given component it tries to use XServiceInfo instead.

                Note: This method try to locate a suitable module name.
                Nothing else. Selecting the right component and throwing suitable
                exceptions must be done outside.

        @see    identify()

        @param  xComponent
                the module for identification.

        @return The identifier of the given module.
                Can be empty if given component is not a real module !

        @threadsafe
     */
    OUString implts_identify(const css::uno::Reference< css::uno::XInterface >& xComponent);
};

ModuleManager::ModuleManager(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext(xContext)
{
    m_xCFG.set( comphelper::ConfigurationHelper::openConfig(
                m_xContext, "/org.openoffice.Setup/Office/Factories",
                comphelper::ConfigurationHelper::E_READONLY),
            css::uno::UNO_QUERY_THROW );
}

ModuleManager::~ModuleManager()
{
}

OUString ModuleManager::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.framework.ModuleManager");
}

sal_Bool ModuleManager::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > ModuleManager::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence< OUString > s { "com.sun.star.frame.ModuleManager" };
    return s;
}

OUString SAL_CALL ModuleManager::identify(const css::uno::Reference< css::uno::XInterface >& xModule)
    throw(css::lang::IllegalArgumentException,
          css::frame::UnknownModuleException,
          css::uno::RuntimeException, std::exception         )
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
                OUString("Given module is not a frame nor a window, controller or model."),
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

    OUString sModule;
    if (xModel.is())
        sModule = implts_identify(xModel);
    else if (xController.is())
        sModule = implts_identify(xController);
    else if (xWindow.is())
        sModule = implts_identify(xWindow);

    if (sModule.isEmpty())
        throw css::frame::UnknownModuleException(
                OUString("Can not find suitable module for the given component."),
                static_cast< ::cppu::OWeakObject* >(this));

    return sModule;
}

void SAL_CALL ModuleManager::replaceByName(const OUString& sName ,
                                           const css::uno::Any&   aValue)
    throw (css::lang::IllegalArgumentException   ,
           css::container::NoSuchElementException,
           css::lang::WrappedTargetException     ,
           css::uno::RuntimeException, std::exception            )
{
    ::comphelper::SequenceAsHashMap lProps(aValue);
    if (lProps.empty() )
    {
        throw css::lang::IllegalArgumentException(
                OUString("No properties given to replace part of module."),
                static_cast< cppu::OWeakObject * >(this),
                2);
    }

    // get access to the element
    // Note: Don't use impl_getConfig() method here. Because it creates a readonly access only, further
    // it cache it as a member of this module manager instance. If we change some props there ... but don't
    // flush changes (because an error occurred) we will read them later. If we use a different config access
    // we can close it without a flush... and our read data won't be affected .-)
    css::uno::Reference< css::uno::XInterface >         xCfg      = ::comphelper::ConfigurationHelper::openConfig(
                                                                        m_xContext,
                                                                        "/org.openoffice.Setup/Office/Factories",
                                                                        ::comphelper::ConfigurationHelper::E_STANDARD);
    css::uno::Reference< css::container::XNameAccess >  xModules (xCfg, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameReplace > xModule  ;

    xModules->getByName(sName) >>= xModule;
    if (!xModule.is())
    {
        throw css::uno::RuntimeException(
                OUString("Was not able to get write access to the requested module entry inside configuration."),
                static_cast< cppu::OWeakObject * >(this));
    }

    ::comphelper::SequenceAsHashMap::const_iterator pProp;
    for (  pProp  = lProps.begin();
           pProp != lProps.end()  ;
         ++pProp                  )
    {
        const OUString& sPropName  = pProp->first;
        const css::uno::Any&   aPropValue = pProp->second;

        // let "NoSuchElementException" out ! We support the same API ...
        // and without a flush() at the end all changed data before will be ignored !
        xModule->replaceByName(sPropName, aPropValue);
    }

    ::comphelper::ConfigurationHelper::flush(xCfg);
}

css::uno::Any SAL_CALL ModuleManager::getByName(const OUString& sName)
    throw(css::container::NoSuchElementException,
          css::lang::WrappedTargetException     ,
          css::uno::RuntimeException, std::exception            )
{
    // get access to the element
    css::uno::Reference< css::container::XNameAccess > xModule;
    m_xCFG->getByName(sName) >>= xModule;
    if (!xModule.is())
    {
        throw css::uno::RuntimeException(
                OUString("Was not able to get write access to the requested module entry inside configuration."),
                static_cast< cppu::OWeakObject * >(this));
    }

    // convert it to seq< PropertyValue >
    const css::uno::Sequence< OUString > lPropNames = xModule->getElementNames();
    comphelper::SequenceAsHashMap lProps;

    lProps[OUString("ooSetupFactoryModuleIdentifier")] <<= sName;
    for (sal_Int32 i = 0; i < lPropNames.getLength(); ++i)
    {
        const OUString& sPropName = lPropNames[i];
        lProps[sPropName] = xModule->getByName(sPropName);
    }

    return css::uno::makeAny(lProps.getAsConstPropertyValueList());
}

css::uno::Sequence< OUString > SAL_CALL ModuleManager::getElementNames()
    throw(css::uno::RuntimeException, std::exception)
{
    return m_xCFG->getElementNames();
}

sal_Bool SAL_CALL ModuleManager::hasByName(const OUString& sName)
    throw(css::uno::RuntimeException, std::exception)
{
    return m_xCFG->hasByName(sName);
}

css::uno::Type SAL_CALL ModuleManager::getElementType()
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get();
}

sal_Bool SAL_CALL ModuleManager::hasElements()
    throw(css::uno::RuntimeException, std::exception)
{
    return m_xCFG->hasElements();
}

css::uno::Reference< css::container::XEnumeration > SAL_CALL ModuleManager::createSubSetEnumerationByQuery(const OUString&)
    throw(css::uno::RuntimeException, std::exception)
{
    return css::uno::Reference< css::container::XEnumeration >();
}

css::uno::Reference< css::container::XEnumeration > SAL_CALL ModuleManager::createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
    throw(css::uno::RuntimeException, std::exception)
{
    ::comphelper::SequenceAsHashMap lSearchProps(lProperties);
    const css::uno::Sequence< OUString > lModules = getElementNames();
    ::std::vector< css::uno::Any > lResult;

    for (sal_Int32 i = 0; i < lModules.getLength(); ++i)
    {
        try
        {
            ::comphelper::SequenceAsHashMap lModuleProps = getByName(lModules[i]);
            if (lModuleProps.match(lSearchProps))
                lResult.push_back(css::uno::makeAny(lModuleProps.getAsConstPropertyValueList()));
        }
        catch(const css::uno::Exception&)
        {
        }
    }

    ::comphelper::OAnyEnumeration*                      pEnum =
                 new ::comphelper::OAnyEnumeration(comphelper::containerToSequence(lResult));
    css::uno::Reference< css::container::XEnumeration > xEnum(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY_THROW);
    return xEnum;
}

OUString ModuleManager::implts_identify(const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    // Search for an optional (!) interface XModule first.
    // Its used to overrule an existing service name. Used e.g. by our database form designer
    // which uses a writer module internally.
    css::uno::Reference< css::frame::XModule > xModule(xComponent, css::uno::UNO_QUERY);
    if (xModule.is())
        return xModule->getIdentifier();

    // detect modules in a generic way ...
    // comparing service names with configured entries ...
    css::uno::Reference< css::lang::XServiceInfo > xInfo(xComponent, css::uno::UNO_QUERY);
    if (!xInfo.is())
        return OUString();

    const css::uno::Sequence< OUString > lKnownModules = getElementNames();
    for (sal_Int32 i = 0; i < lKnownModules.getLength(); ++i)
    {
        if (xInfo->supportsService(lKnownModules[i]))
            return lKnownModules[i];
    }

    return OUString();
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(new ModuleManager(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ModuleManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
