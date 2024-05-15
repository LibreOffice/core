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

#include <memory>
#include <sal/config.h>
#include <osl/diagnose.h>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "iahndl.hxx"
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace {

class UUIInteractionHandler:
    public cppu::WeakImplHelper<css::lang::XServiceInfo,
                                css::lang::XInitialization,
                                css::task::XInteractionHandler2,
                                css::beans::XPropertySet>
{
private:
    UUIInteractionHelper m_pImpl;

public:
    explicit UUIInteractionHandler(css::uno::Reference< css::uno::XComponentContext > const & rxContext);

    UUIInteractionHandler(const UUIInteractionHandler&) = delete;
    UUIInteractionHandler& operator=(const UUIInteractionHandler&) = delete;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual void SAL_CALL
    initialize(
        css::uno::Sequence< css::uno::Any > const & rArguments) override;

    virtual void SAL_CALL
    handle(css::uno::Reference< css::task::XInteractionRequest > const & rRequest) override;

    virtual sal_Bool SAL_CALL
        handleInteractionRequest(
            const css::uno::Reference< css::task::XInteractionRequest >& Request
        ) override;

    virtual void SAL_CALL
        addPropertyChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XPropertyChangeListener >& /*xListener*/ ) override
    {
        throw css::uno::RuntimeException(
            u"UUIInteractionHandler addPropertyChangeListener is not supported"_ustr);
    }

    virtual void SAL_CALL
        removePropertyChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XPropertyChangeListener >& /*xListener*/ ) override
    {
        throw css::uno::RuntimeException(
            u"UUIInteractionHandler removePropertyChangeListener is not supported"_ustr);
    }

    virtual void SAL_CALL
        addVetoableChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XVetoableChangeListener >& /*xListener*/ ) override
    {
        throw css::uno::RuntimeException(
            u"UUIInteractionHandler addVetoableChangeListener is not supported"_ustr);
    }

    virtual void SAL_CALL
        removeVetoableChangeListener( const OUString& /*aPropertyName*/, const css::uno::Reference< css::beans::XVetoableChangeListener >& /*xListener*/ ) override
    {
        throw css::uno::RuntimeException(
            u"UUIInteractionHandler removeVetoableChangeListener is not supported"_ustr);
    }

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override
    {
        return nullptr;
    }

    virtual void SAL_CALL setPropertyValue(const OUString& rPropertyName, const css::uno::Any& rValue) override
    {
        if (rPropertyName == "ParentWindow")
        {
            css::uno::Reference<css::awt::XWindow> xWindow;
            rValue >>= xWindow;
            m_pImpl.SetParentWindow(xWindow);
            return;
        }
        throw css::beans::UnknownPropertyException(rPropertyName);
    }

    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& rPropertyName) override
    {
        if (rPropertyName == "ParentWindow")
        {
            return uno::Any(m_pImpl.GetParentWindow());
        }
        throw css::beans::UnknownPropertyException(rPropertyName);
    }
};

UUIInteractionHandler::UUIInteractionHandler(
    uno::Reference< uno::XComponentContext > const & rxContext)
        : m_pImpl(rxContext)
{
}

OUString SAL_CALL UUIInteractionHandler::getImplementationName()
{
    return u"com.sun.star.comp.uui.UUIInteractionHandler"_ustr;
}

sal_Bool SAL_CALL
UUIInteractionHandler::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
UUIInteractionHandler::getSupportedServiceNames()
{
    return { u"com.sun.star.task.InteractionHandler"_ustr,
 // added to indicate support for configuration.backend.MergeRecoveryRequest
             u"com.sun.star.configuration.backend.InteractionHandler"_ustr,
    // for backwards compatibility
             u"com.sun.star.uui.InteractionHandler"_ustr };
}

void SAL_CALL
UUIInteractionHandler::initialize(
    uno::Sequence< uno::Any > const & rArguments)
{
    // The old-style InteractionHandler service supported a sequence of
    // PropertyValue, while the new-style service now uses constructors to pass
    // in Parent and Context values; for backwards compatibility, keep support
    // for a PropertyValue sequence, too:
    uno::Reference< awt::XWindow > xWindow;
    OUString aContext;
    if (!((rArguments.getLength() == 1 && (rArguments[0] >>= xWindow)) ||
          (rArguments.getLength() == 2 && (rArguments[0] >>= xWindow) &&
           (rArguments[1] >>= aContext))))
    {
        ::comphelper::NamedValueCollection aProperties( rArguments );
        if ( aProperties.has( u"Parent"_ustr ) )
        {
            OSL_VERIFY( aProperties.get( u"Parent"_ustr ) >>= xWindow );
        }
        if ( aProperties.has( u"Context"_ustr ) )
        {
            OSL_VERIFY( aProperties.get( u"Context"_ustr ) >>= aContext );
        }
    }

    m_pImpl.SetParentWindow(xWindow);
    m_pImpl.setContext(aContext);
}

void SAL_CALL
UUIInteractionHandler::handle(
    uno::Reference< task::XInteractionRequest > const & rRequest)
{
    try
    {
        m_pImpl.handleRequest(rRequest);
    }
    catch (uno::RuntimeException const & ex)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException( ex.Message,
                *this, anyEx );
    }
}

sal_Bool SAL_CALL UUIInteractionHandler::handleInteractionRequest(
    const uno::Reference< task::XInteractionRequest >& Request )
{
    try
    {
        return m_pImpl.handleRequest( Request );
    }
    catch (uno::RuntimeException const & ex)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException( ex.Message,
                *this, anyEx );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_uui_UUIInteractionHandler_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UUIInteractionHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
