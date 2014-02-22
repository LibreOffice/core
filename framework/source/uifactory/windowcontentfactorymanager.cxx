/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/config.h>

#include <uifactory/configurationaccessfactorymanager.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include "com/sun/star/frame/XModuleManager2.hpp"
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace framework;

namespace {

typedef ::cppu::WeakComponentImplHelper2<
    com::sun::star::lang::XServiceInfo,
    com::sun::star::lang::XSingleComponentFactory > WindowContentFactoryManager_BASE;

class WindowContentFactoryManager : private osl::Mutex,
                                    public WindowContentFactoryManager_BASE
{
public:
    WindowContentFactoryManager( const css::uno::Reference< css::uno::XComponentContext>& rxContext );
    virtual ~WindowContentFactoryManager();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.WindowContentFactoryManager");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = OUString("com.sun.star.ui.WindowContentFactoryManager");
        return aSeq;
    }

    
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext( const css::uno::Reference< css::uno::XComponentContext >& Context ) throw (css::uno::Exception, css::uno::RuntimeException);
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) throw (css::uno::Exception, css::uno::RuntimeException);

private:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    sal_Bool                                               m_bConfigRead;
    ConfigurationAccess_FactoryManager*                    m_pConfigAccess;
};

WindowContentFactoryManager::WindowContentFactoryManager( const uno::Reference< uno::XComponentContext >& rxContext ) :
    WindowContentFactoryManager_BASE(*static_cast<osl::Mutex *>(this)),
    m_xContext( rxContext ),
    m_bConfigRead( sal_False )
{
    m_pConfigAccess = new ConfigurationAccess_FactoryManager( m_xContext,
            "/org.openoffice.Office.UI.WindowContentFactories/Registered/ContentFactories" );
    m_pConfigAccess->acquire();
}

WindowContentFactoryManager::~WindowContentFactoryManager()
{
    disposing();
}

void SAL_CALL WindowContentFactoryManager::disposing()
{
    osl::MutexGuard g(rBHelper.rMutex);

    if (m_pConfigAccess)
    {
        
        m_pConfigAccess->release();
        m_pConfigAccess = 0;
    }
}


uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithContext(
    const uno::Reference< uno::XComponentContext >& /*xContext*/ )
throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xWindow;
    return xWindow;
}

uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithArgumentsAndContext(
    const uno::Sequence< uno::Any >& Arguments, const uno::Reference< uno::XComponentContext >& Context )
throw (uno::Exception, uno::RuntimeException)
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

        
        
        { 
        osl::MutexGuard g(rBHelper.rMutex);
        if ( !m_bConfigRead )
        {
            m_bConfigRead = sal_True;
            m_pConfigAccess->readConfigurationData();
        }
        aImplementationName = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
        } 

        if ( !aImplementationName.isEmpty() )
        {
            uno::Reference< lang::XMultiServiceFactory > xServiceManager( Context->getServiceManager(), uno::UNO_QUERY );
            if ( xServiceManager.is() )
            {
                uno::Reference< lang::XSingleComponentFactory > xFactory(
                    xServiceManager->createInstance( aImplementationName ), uno::UNO_QUERY );
                if ( xFactory.is() )
                {
                    
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

    
    if ( !xWindow.is())
    {
        
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
