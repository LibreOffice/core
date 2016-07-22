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

#include "sal/config.h"

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implbase.hxx>
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/current_context.hxx"

namespace {

OUString SAL_CALL getDefaultImplementationName() {
    return OUString(
            "com.sun.star.comp.configuration.backend.DesktopBackend");
}

css::uno::Sequence< OUString > SAL_CALL getDefaultSupportedServiceNames() {
    OUString name(
            "com.sun.star.configuration.backend.DesktopBackend");
    return css::uno::Sequence< OUString >(&name, 1);
}

class Default:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::beans::XPropertySet >
{
public:
    Default() {}
    Default(const Default&) = delete;
    Default& operator=(const Default&) = delete;

private:
    virtual ~Default() {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return getDefaultImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return ServiceName == getSupportedServiceNames()[0]; }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override
    { return getDefaultSupportedServiceNames(); }

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
    getPropertySetInfo() throw (css::uno::RuntimeException, std::exception) override
    { return css::uno::Reference< css::beans::XPropertySetInfo >(); }

    virtual void SAL_CALL setPropertyValue(
        OUString const &, css::uno::Any const &)
        throw (
            css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
        OUString const & PropertyName)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XPropertyChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const &,
        css::uno::Reference< css::beans::XVetoableChangeListener > const &)
        throw (
            css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
    {}
};

void Default::setPropertyValue(OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    throw css::lang::IllegalArgumentException(
        OUString("setPropertyValue not supported"),
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any Default::getPropertyValue(OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    if ( PropertyName == "EnableATToolSupport" ||
         PropertyName == "ExternalMailer" ||
         PropertyName == "SourceViewFontHeight" ||
         PropertyName == "SourceViewFontName" ||
         PropertyName == "TemplatePathVariable" ||
         PropertyName == "WorkPathVariable" ||
         PropertyName == "ooInetFTPProxyName" ||
         PropertyName == "ooInetFTPProxyPort" ||
         PropertyName == "ooInetHTTPProxyName" ||
         PropertyName == "ooInetHTTPProxyPort" ||
         PropertyName == "ooInetHTTPSProxyName" ||
         PropertyName == "ooInetHTTPSProxyPort" ||
         PropertyName == "ooInetNoProxy" ||
         PropertyName == "ooInetProxyType" ||
         PropertyName == "givenname" ||
         PropertyName == "sn" )
    {
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    }
    throw css::beans::UnknownPropertyException(
        PropertyName, static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::uno::XInterface > createBackend(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    OUString const & name)
{
    try {
        return css::uno::Reference< css::lang::XMultiComponentFactory >(
            context->getServiceManager(), css::uno::UNO_SET_THROW)->
            createInstanceWithContext(name, context);
    } catch (css::uno::RuntimeException &) {
        // Assuming these exceptions are real errors:
        throw;
    } catch (const css::uno::Exception & e) {
        // Assuming these exceptions indicate that the service is not installed:
        OSL_TRACE(
            "createInstance(%s) failed with %s",
            OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr(),
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        return css::uno::Reference< css::uno::XInterface >();
    }
}

css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    OUString desktop;
    css::uno::Reference< css::uno::XCurrentContext > current(
        css::uno::getCurrentContext());
    if (current.is()) {
        current->getValueByName("system.desktop-environment") >>= desktop;
    }

    // Fall back to the default if the specific backend is not available:
    css::uno::Reference< css::uno::XInterface > backend;
    if ( desktop == "KDE" ) {
        backend = createBackend(
            context,
            "com.sun.star.configuration.backend.KDEBackend");
    } else if ( desktop == "KDE4" ) {
        backend = createBackend(
            context,
            "com.sun.star.configuration.backend.KDE4Backend");
    }
    return backend.is()
        ? backend : static_cast< cppu::OWeakObject * >(new Default);
}

static cppu::ImplementationEntry const services[] = {
    { &createInstance, &getDefaultImplementationName,
      &getDefaultSupportedServiceNames, &cppu::createSingleComponentFactory, nullptr,
      0 },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL desktopbe1_component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
