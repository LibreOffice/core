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

#include <string_view>

#include <uifactory/factoryconfiguration.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>

#include <rtl/ref.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::container;
using namespace css::frame;
using namespace framework;

namespace {

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XServiceInfo,
    css::frame::XUIControllerFactory > UIControllerFactory_BASE;

class UIControllerFactory : private cppu::BaseMutex,
                            public UIControllerFactory_BASE
{
public:
    virtual ~UIControllerFactory() override;

    // XMultiComponentFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext( const OUString& aServiceSpecifier, const css::uno::Reference< css::uno::XComponentContext >& Context ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    // XUIControllerRegistration
    virtual sal_Bool SAL_CALL hasController( const OUString& aCommandURL, const OUString& aModuleName ) override;
    virtual void SAL_CALL registerController( const OUString& aCommandURL, const OUString& aModuleName, const OUString& aControllerImplementationName ) override;
    virtual void SAL_CALL deregisterController( const OUString& aCommandURL, const OUString& aModuleName ) override;

protected:
    UIControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext, std::u16string_view rUINode  );
    bool                                                    m_bConfigRead;
    css::uno::Reference< css::uno::XComponentContext >       m_xContext;
    rtl::Reference<ConfigurationAccess_ControllerFactory>    m_pConfigAccess;

private:
    virtual void SAL_CALL disposing() final override;
};

UIControllerFactory::UIControllerFactory(
    const Reference< XComponentContext >& xContext,
    std::u16string_view rConfigurationNode )
    : UIControllerFactory_BASE(m_aMutex)
    , m_bConfigRead( false )
    , m_xContext( xContext )
    , m_pConfigAccess()
{
    m_pConfigAccess = new ConfigurationAccess_ControllerFactory(m_xContext,
            OUString::Concat("/org.openoffice.Office.UI.Controller/Registered/")
                + rConfigurationNode);
}

UIControllerFactory::~UIControllerFactory()
{
    disposing();
}

void SAL_CALL UIControllerFactory::disposing()
{
    osl::MutexGuard g(rBHelper.rMutex);
    m_pConfigAccess.clear();
}

// XMultiComponentFactory
Reference< XInterface > SAL_CALL UIControllerFactory::createInstanceWithContext(
    const OUString& aServiceSpecifier,
    const Reference< XComponentContext >& )
{
    // SAFE
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( aServiceSpecifier, std::u16string_view() );
    if ( !aServiceName.isEmpty() )
        return m_xContext->getServiceManager()->createInstanceWithContext( aServiceName, m_xContext );
    else
        return Reference< XInterface >();
    // SAFE
}

Reference< XInterface > SAL_CALL UIControllerFactory::createInstanceWithArgumentsAndContext(
    const OUString&                  ServiceSpecifier,
    const Sequence< Any >&                  Arguments,
    const Reference< XComponentContext >& )
{
    const OUString aPropModuleName( "ModuleIdentifier" );

    OUString   aPropName;
    PropertyValue   aPropValue;

    // Retrieve the optional module name from the Arguments sequence. It is used as a part of
    // the hash map key to support different controller implementation for the same URL but different
    // module!!
    for ( Any const & arg : Arguments )
    {
        if (( arg >>= aPropValue ) && ( aPropValue.Name == aPropModuleName ))
        {
            aPropValue.Value >>= aPropName;
            break;
        }
    }

    Sequence< Any > aNewArgs( Arguments );

    sal_Int32 nAppendIndex = aNewArgs.getLength();
    aNewArgs.realloc( aNewArgs.getLength() + 2 );

    // Append the command URL to the Arguments sequence so that one controller can be
    // used for more than one command URL.
    aPropValue.Name     = "CommandURL";
    aPropValue.Value  <<= ServiceSpecifier;
    aNewArgs[nAppendIndex] <<= aPropValue;

    // Append the optional value argument. It's an empty string if no additional info
    // is provided to the controller.
    OUString aValue = m_pConfigAccess->getValueFromCommandModule( ServiceSpecifier, aPropName );
    aPropValue.Name = "Value";
    aPropValue.Value <<= aValue;
    aNewArgs[nAppendIndex+1] <<= aPropValue;

    {
        OUString aServiceName;
        { // SAFE
        osl::MutexGuard g(rBHelper.rMutex);

        if ( !m_bConfigRead )
        {
            m_bConfigRead = true;
            m_pConfigAccess->readConfigurationData();
        }

        aServiceName = m_pConfigAccess->getServiceFromCommandModule( ServiceSpecifier, aPropName );
        } // SAFE

        if ( !aServiceName.isEmpty() )
            return m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aServiceName, aNewArgs, m_xContext );
        else
            return Reference< XInterface >();
    }
}

Sequence< OUString > SAL_CALL UIControllerFactory::getAvailableServiceNames()
{
    return Sequence< OUString >();
}

// XUIControllerRegistration
sal_Bool SAL_CALL UIControllerFactory::hasController(
    const OUString& aCommandURL,
    const OUString& aModuleName )
{
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    return ( !m_pConfigAccess->getServiceFromCommandModule( aCommandURL, aModuleName ).isEmpty() );
}

void SAL_CALL UIControllerFactory::registerController(
    const OUString& aCommandURL,
    const OUString& aModuleName,
    const OUString& aControllerImplementationName )
{
    // SAFE
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addServiceToCommandModule( aCommandURL, aModuleName, aControllerImplementationName );
    // SAFE
}

void SAL_CALL UIControllerFactory::deregisterController(
    const OUString& aCommandURL,
    const OUString& aModuleName )
{
    // SAFE
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeServiceFromCommandModule( aCommandURL, aModuleName );
    // SAFE
}

class PopupMenuControllerFactory :  public UIControllerFactory
{
public:
    explicit PopupMenuControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.PopupMenuControllerFactory";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.PopupMenuControllerFactory"};
    }

};

PopupMenuControllerFactory::PopupMenuControllerFactory( const Reference< XComponentContext >& xContext ) :
    UIControllerFactory( xContext, u"PopupMenu" )
{
}

class ToolbarControllerFactory :  public UIControllerFactory
{
public:
    explicit ToolbarControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.ToolBarControllerFactory";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.ToolbarControllerFactory"};
    }

};

ToolbarControllerFactory::ToolbarControllerFactory( const Reference< XComponentContext >& xContext ) :
    UIControllerFactory( xContext, u"ToolBar" )
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
    explicit StatusbarControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.StatusBarControllerFactory";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.StatusbarControllerFactory"};
    }

};

StatusbarControllerFactory::StatusbarControllerFactory( const Reference< XComponentContext >& xContext ) :
    UIControllerFactory( xContext, u"StatusBar" )
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_PopupMenuControllerFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PopupMenuControllerFactory(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ToolBarControllerFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
            ToolbarControllerFactorySingleton::get(context).instance.get()));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_StatusBarControllerFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
            StatusbarControllerFactorySingleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
