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

#include <uifactory/factoryconfiguration.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::container;
using namespace css::frame;
using namespace framework;

namespace {

typedef ::cppu::WeakComponentImplHelper2<
    css::lang::XServiceInfo,
    css::frame::XUIControllerFactory > UIControllerFactory_BASE;

class UIControllerFactory : private osl::Mutex,
                            public UIControllerFactory_BASE
{
public:
    virtual ~UIControllerFactory();

    
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException) = 0;
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (css::uno::RuntimeException) = 0;
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException) = 0;

    
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext( const OUString& aServiceSpecifier, const css::uno::Reference< css::uno::XComponentContext >& Context ) throw (css::uno::Exception, css::uno::RuntimeException);
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) throw (css::uno::Exception, css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() throw (css::uno::RuntimeException);

    
    virtual sal_Bool SAL_CALL hasController( const OUString& aCommandURL, const OUString& aModuleName ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL registerController( const OUString& aCommandURL, const OUString& aModuleName, const OUString& aControllerImplementationName ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL deregisterController( const OUString& aCommandURL, const OUString& aModuleName ) throw (css::uno::RuntimeException);

protected:
    UIControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext, const rtl::OUString &rUINode  );
    sal_Bool                                                                         m_bConfigRead;
    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    ConfigurationAccess_ControllerFactory*                                           m_pConfigAccess;

private:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;
};

UIControllerFactory::UIControllerFactory(
    const Reference< XComponentContext >& xContext,
    const rtl::OUString &rConfigurationNode )
    : UIControllerFactory_BASE(*static_cast<osl::Mutex *>(this))
    , m_bConfigRead( sal_False )
    , m_xContext( xContext )
    , m_pConfigAccess()
{
    m_pConfigAccess = new ConfigurationAccess_ControllerFactory(m_xContext,
            "/org.openoffice.Office.UI.Controller/Registered/" + rConfigurationNode);
    m_pConfigAccess->acquire();
}

UIControllerFactory::~UIControllerFactory()
{
    disposing();
}

void SAL_CALL UIControllerFactory::disposing()
{
    osl::MutexGuard g(rBHelper.rMutex);
    if (m_pConfigAccess)
    {
        
        m_pConfigAccess->release();
        m_pConfigAccess = 0;
    }
}


Reference< XInterface > SAL_CALL UIControllerFactory::createInstanceWithContext(
    const OUString& aServiceSpecifier,
    const Reference< XComponentContext >& )
throw (Exception, RuntimeException)
{
    
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( aServiceSpecifier, OUString() );
    if ( !aServiceName.isEmpty() )
        return m_xContext->getServiceManager()->createInstanceWithContext( aServiceName, m_xContext );
    else
        return Reference< XInterface >();
    
}

Reference< XInterface > SAL_CALL UIControllerFactory::createInstanceWithArgumentsAndContext(
    const OUString&                  ServiceSpecifier,
    const Sequence< Any >&                  Arguments,
    const Reference< XComponentContext >& )
throw (Exception, RuntimeException)
{
    const OUString aPropModuleName( "ModuleIdentifier" );
    const OUString aPropValueName( "Value" );

    OUString   aPropName;
    PropertyValue   aPropValue;

    
    
    
    for ( int i = 0; i < Arguments.getLength(); i++ )
    {
        if (( Arguments[i] >>= aPropValue ) && ( aPropValue.Name.equals( aPropModuleName )))
        {
            aPropValue.Value >>= aPropName;
            break;
        }
    }

    Sequence< Any > aNewArgs( Arguments );

    sal_Int32 nAppendIndex = aNewArgs.getLength();
    bool bHasValue = m_pConfigAccess->hasValue();
    aNewArgs.realloc( aNewArgs.getLength() + (bHasValue ? 2 : 1) );

    
    
    aPropValue.Name     = "CommandURL";
    aPropValue.Value  <<= ServiceSpecifier;
    aNewArgs[nAppendIndex] <<= aPropValue;

    if ( bHasValue )
    {
        
        
        OUString aValue = m_pConfigAccess->getValueFromCommandModule( ServiceSpecifier, aPropName );
        aPropValue.Name = aPropValueName;
        aPropValue.Value <<= aValue;
        aNewArgs[nAppendIndex+1] <<= aPropValue;
    }

    {
        OUString aServiceName;
        { 
        osl::MutexGuard g(rBHelper.rMutex);

        if ( !m_bConfigRead )
        {
            m_bConfigRead = sal_True;
            m_pConfigAccess->readConfigurationData();
        }

        aServiceName = m_pConfigAccess->getServiceFromCommandModule( ServiceSpecifier, aPropName );
        } 

        if ( !aServiceName.isEmpty() )
            return m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aServiceName, aNewArgs, m_xContext );
        else
            return Reference< XInterface >();
    }
}

Sequence< OUString > SAL_CALL UIControllerFactory::getAvailableServiceNames()
throw (RuntimeException)
{
    return Sequence< OUString >();
}


sal_Bool SAL_CALL UIControllerFactory::hasController(
    const OUString& aCommandURL,
    const OUString& aModuleName )
throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    return ( !m_pConfigAccess->getServiceFromCommandModule( aCommandURL, aModuleName ).isEmpty() );
}

void SAL_CALL UIControllerFactory::registerController(
    const OUString& aCommandURL,
    const OUString& aModuleName,
    const OUString& aControllerImplementationName )
throw (RuntimeException)
{
    
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addServiceToCommandModule( aCommandURL, aModuleName, aControllerImplementationName );
    
}

void SAL_CALL UIControllerFactory::deregisterController(
    const OUString& aCommandURL,
    const OUString& aModuleName )
throw (RuntimeException)
{
    
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeServiceFromCommandModule( aCommandURL, aModuleName );
    
}

class PopupMenuControllerFactory :  public UIControllerFactory
{
public:
    PopupMenuControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.PopupMenuControllerFactory");
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
        aSeq[0] = OUString("com.sun.star.frame.PopupMenuControllerFactory");
        return aSeq;
    }

};

PopupMenuControllerFactory::PopupMenuControllerFactory( const Reference< XComponentContext >& xContext ) :
    UIControllerFactory(
        xContext,
        OUString( "PopupMenu") )
{
}

struct PopupMenuControllerFactoryInstance {
    explicit PopupMenuControllerFactoryInstance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new PopupMenuControllerFactory(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct PopupMenuControllerFactorySingleton:
    public rtl::StaticWithArg<
        PopupMenuControllerFactoryInstance,
        css::uno::Reference<css::uno::XComponentContext>,
        PopupMenuControllerFactorySingleton>
{};

class ToolbarControllerFactory :  public UIControllerFactory
{
public:
    ToolbarControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.ToolBarControllerFactory");
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
        aSeq[0] = OUString("com.sun.star.frame.ToolbarControllerFactory");
        return aSeq;
    }

};

ToolbarControllerFactory::ToolbarControllerFactory( const Reference< XComponentContext >& xContext ) :
    UIControllerFactory(
        xContext,
        OUString( "ToolBar" ))
{
}

struct ToolbarControllerFactoryInstance {
    explicit ToolbarControllerFactoryInstance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new ToolbarControllerFactory(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct ToolbarControllerFactorySingleton:
    public rtl::StaticWithArg<
        ToolbarControllerFactoryInstance,
        css::uno::Reference<css::uno::XComponentContext>,
        ToolbarControllerFactorySingleton>
{};

class StatusbarControllerFactory :  public UIControllerFactory
{
public:
    StatusbarControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.StatusBarControllerFactory");
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
        aSeq[0] = OUString("com.sun.star.frame.StatusbarControllerFactory");
        return aSeq;
    }

};

StatusbarControllerFactory::StatusbarControllerFactory( const Reference< XComponentContext >& xContext ) :
    UIControllerFactory(
        xContext,
        OUString( "StatusBar" ) )
{
}

struct StatusbarControllerFactoryInstance {
    explicit StatusbarControllerFactoryInstance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new StatusbarControllerFactory(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct StatusbarControllerFactorySingleton:
    public rtl::StaticWithArg<
        StatusbarControllerFactoryInstance,
        css::uno::Reference<css::uno::XComponentContext>,
        StatusbarControllerFactorySingleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_PopupMenuControllerFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
            PopupMenuControllerFactorySingleton::get(context).instance.get()));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ToolBarControllerFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
            ToolbarControllerFactorySingleton::get(context).instance.get()));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_StatusBarControllerFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
            StatusbarControllerFactorySingleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
