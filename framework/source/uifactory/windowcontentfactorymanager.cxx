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

#include <uifactory/configurationaccessfactorymanager.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace framework;

namespace {

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XServiceInfo,
    css::lang::XSingleComponentFactory > WindowContentFactoryManager_BASE;

class WindowContentFactoryManager : private cppu::BaseMutex,
                                    public WindowContentFactoryManager_BASE
{
public:
    explicit WindowContentFactoryManager( const css::uno::Reference< css::uno::XComponentContext>& rxContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.framework.WindowContentFactoryManager");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.ui.WindowContentFactoryManager"};
    }

    // XSingleComponentFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext( const css::uno::Reference< css::uno::XComponentContext >& Context ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) override;

private:
    virtual void SAL_CALL disposing() override;

    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    bool                                               m_bConfigRead;
    rtl::Reference<ConfigurationAccess_FactoryManager> m_pConfigAccess;
};

WindowContentFactoryManager::WindowContentFactoryManager( const uno::Reference< uno::XComponentContext >& rxContext ) :
    WindowContentFactoryManager_BASE(m_aMutex),
    m_xContext( rxContext ),
    m_bConfigRead( false ),
    m_pConfigAccess(
        new ConfigurationAccess_FactoryManager(
            m_xContext,
            "/org.openoffice.Office.UI.WindowContentFactories/Registered/ContentFactories"))
{}

void SAL_CALL WindowContentFactoryManager::disposing()
{
    m_pConfigAccess.clear();
}

// XSingleComponentFactory
uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithContext(
    const uno::Reference< uno::XComponentContext >& /*xContext*/ )
{
    uno::Reference< uno::XInterface > xWindow;
    return xWindow;
}

uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithArgumentsAndContext(
    const uno::Sequence< uno::Any >& Arguments, const uno::Reference< uno::XComponentContext >& Context )
{
    uno::Reference< uno::XInterface > xWindow;
    uno::Reference< frame::XFrame >   xFrame;
    OUString                   aResourceURL;

    for (sal_Int32 i=0; i < Arguments.getLength(); i++ )
    {
        beans::PropertyValue aPropValue;
        if ( Arguments[i] >>= aPropValue )
        {
            if ( aPropValue.Name == "Frame" )
                aPropValue.Value >>= xFrame;
            else if ( aPropValue.Name == "ResourceURL" )
                aPropValue.Value >>= aResourceURL;
        }
    }

    // Determine the module identifier
    OUString aType;
    OUString aName;
    OUString aModuleId;
    uno::Reference< frame::XModuleManager2 > xModuleManager =
        frame::ModuleManager::create( m_xContext );
    try
    {
        if ( xFrame.is() && xModuleManager.is() )
            aModuleId = xModuleManager->identify( uno::Reference< uno::XInterface >( xFrame, uno::UNO_QUERY ) );
    }
    catch ( const frame::UnknownModuleException& )
    {
    }

    RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );
    if ( !aType.isEmpty() &&
         !aName.isEmpty() &&
         !aModuleId.isEmpty() )
    {
        OUString                   aImplementationName;
        uno::Reference< uno::XInterface > xHolder( static_cast<cppu::OWeakObject*>(this), uno::UNO_QUERY );

        // Determine the implementation name of the window content factory dependent on the
        // module identifier, user interface element type and name
        { // SAFE
        osl::MutexGuard g(rBHelper.rMutex);
        if (rBHelper.bDisposed) {
            throw css::lang::DisposedException(
                "disposed", static_cast<OWeakObject *>(this));
        }
        if ( !m_bConfigRead )
        {
            m_bConfigRead = true;
            m_pConfigAccess->readConfigurationData();
        }
        aImplementationName = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
        } // SAFE

        if ( !aImplementationName.isEmpty() )
        {
            uno::Reference< lang::XMultiServiceFactory > xServiceManager( Context->getServiceManager(), uno::UNO_QUERY );
            if ( xServiceManager.is() )
            {
                uno::Reference< lang::XSingleComponentFactory > xFactory(
                    xServiceManager->createInstance( aImplementationName ), uno::UNO_QUERY );
                if ( xFactory.is() )
                {
                    // Be careful: We call external code. Therefore here we have to catch all exceptions
                    try
                    {
                        xWindow = xFactory->createInstanceWithArgumentsAndContext( Arguments, Context );
                    }
                    catch ( uno::Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
    }

    // UNSAFE
    if ( !xWindow.is())
    {
        // Fallback: Use internal factory code to create a toolkit dialog as a content window
        xWindow = createInstanceWithContext(Context);
    }

    return xWindow;
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new WindowContentFactoryManager(context)))
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
com_sun_star_comp_framework_WindowContentFactoryManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
