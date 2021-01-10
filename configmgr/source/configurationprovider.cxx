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

#include <cassert>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/XFlushListener.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <cppu/unotype.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <sal/types.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <i18nlangtag/languagetag.hxx>

#include "components.hxx"
#include "configurationprovider.hxx"
#include "lock.hxx"
#include "defaultprovider.hxx"
#include "rootaccess.hxx"

namespace configmgr::configuration_provider {

namespace {

constexpr OUStringLiteral accessServiceName =
    u"com.sun.star.configuration.ConfigurationAccess";
constexpr OUStringLiteral updateAccessServiceName =
    u"com.sun.star.configuration.ConfigurationUpdateAccess";

void badNodePath() {
    throw css::uno::Exception(
        ("com.sun.star.configuration.ConfigurationProvider expects a single,"
         " non-empty, string nodepath argument"),
        nullptr);
}

typedef
    cppu::WeakComponentImplHelper<
        css::lang::XServiceInfo, css::lang::XMultiServiceFactory,
        css::util::XRefreshable, css::util::XFlushable,
        css::lang::XLocalizable >
    ServiceBase;

class Service:
    private cppu::BaseMutex, public ServiceBase
{
public:
    explicit Service(
        const css::uno::Reference< css::uno::XComponentContext >& context):
        ServiceBase(m_aMutex), context_(context), default_(true),
        lock_( lock() )
    {
        assert(context.is());
    }

    Service(
        const css::uno::Reference< css::uno::XComponentContext >& context,
        OUString const & locale):
        ServiceBase(m_aMutex), context_(context), locale_(locale),
        default_(false),
        lock_( lock() )
    {
        assert(context.is());
    }

private:
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    virtual ~Service() override {}

    virtual void SAL_CALL disposing() override { flushModifications(); }

    virtual OUString SAL_CALL getImplementationName() override
    {
        return default_
            ? default_provider::getImplementationName()
            : "com.sun.star.comp.configuration.ConfigurationProvider";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override
    {
        return default_
            ? default_provider::getSupportedServiceNames()
            : css::uno::Sequence<OUString> { "com.sun.star.configuration.ConfigurationProvider" };
    }

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        OUString const & aServiceSpecifier) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
    createInstanceWithArguments(
        OUString const & ServiceSpecifier,
        css::uno::Sequence< css::uno::Any > const & Arguments) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getAvailableServiceNames() override;

    virtual void SAL_CALL refresh() override;

    virtual void SAL_CALL addRefreshListener(
        css::uno::Reference< css::util::XRefreshListener > const & l) override;

    virtual void SAL_CALL removeRefreshListener(
        css::uno::Reference< css::util::XRefreshListener > const & l) override;

    virtual void SAL_CALL flush() override;

    virtual void SAL_CALL addFlushListener(
        css::uno::Reference< css::util::XFlushListener > const & l) override;

    virtual void SAL_CALL removeFlushListener(
        css::uno::Reference< css::util::XFlushListener > const & l) override;

    virtual void SAL_CALL setLocale(css::lang::Locale const & eLocale) override;

    virtual css::lang::Locale SAL_CALL getLocale() override;

    void flushModifications() const;

    css::uno::Reference< css::uno::XComponentContext > context_;
    OUString locale_;
    bool default_;
    std::shared_ptr<osl::Mutex> lock_;
};

css::uno::Reference< css::uno::XInterface > Service::createInstance(
    OUString const & aServiceSpecifier)
{
    return createInstanceWithArguments(
        aServiceSpecifier, css::uno::Sequence< css::uno::Any >());
}

css::uno::Reference< css::uno::XInterface >
Service::createInstanceWithArguments(
    OUString const & ServiceSpecifier,
    css::uno::Sequence< css::uno::Any > const & Arguments)
{
    OUString nodepath;
    OUString locale;
    for (sal_Int32 i = 0; i < Arguments.getLength(); ++i) {
        css::beans::NamedValue v1;
        css::beans::PropertyValue v2;
        OUString name;
        css::uno::Any value;
        if (Arguments[i] >>= v1) {
            name = v1.Name;
            value = v1.Value;
        } else if (Arguments[i] >>= v2) {
            name = v2.Name;
            value = v2.Value;
        } else if (Arguments.getLength() == 1 && (Arguments[i] >>= nodepath)) {
            // For backwards compatibility, allow a single string argument that
            // denotes nodepath.
            if (nodepath.isEmpty()) {
                badNodePath();
            }
            break;
        } else {
            throw css::uno::Exception(
                ("com.sun.star.configuration.ConfigurationProvider expects"
                 " NamedValue or PropertyValue arguments"),
                nullptr);
        }
        // For backwards compatibility, allow "nodepath" and "Locale" in any
        // case:
        if (name.equalsIgnoreAsciiCase("nodepath")) {
            if (!nodepath.isEmpty() || !(value >>= nodepath) ||
                nodepath.isEmpty())
            {
                badNodePath();
            }
        } else if (name.equalsIgnoreAsciiCase("locale")) {
            if (!locale.isEmpty() || !(value >>= locale) ||
                locale.isEmpty())
            {
                throw css::uno::Exception(
                    ("com.sun.star.configuration.ConfigurationProvider expects"
                     " at most one, non-empty, string Locale argument"),
                    nullptr);
            }
        }
    }
    if (nodepath.isEmpty()) {
        badNodePath();
    }
    // For backwards compatibility, allow a nodepath that misses the leading
    // slash:
    if (nodepath[0] != '/') {
        nodepath = "/" + nodepath;
    }
    if (locale.isEmpty()) {
        //TODO: should the Access use the dynamically changing locale_ instead?
        locale = locale_;
        if (locale.isEmpty()) {
            locale = "en-US";
        }
    }
    bool update;
    if (ServiceSpecifier == accessServiceName) {
        update = false;
    } else if (ServiceSpecifier == updateAccessServiceName) {
        update = true;
    } else {
        throw css::uno::Exception(
            ("com.sun.star.configuration.ConfigurationProvider does not support"
             " service " + ServiceSpecifier),
            static_cast< cppu::OWeakObject * >(this));
    }
    osl::MutexGuard guard(*lock_);
    Components & components = Components::getSingleton(context_);
    rtl::Reference root(
        new RootAccess(components, nodepath, locale, update));
    if (root->isValue()) {
        throw css::uno::Exception(
            ("com.sun.star.configuration.ConfigurationProvider: there is a leaf"
             " value at nodepath " + nodepath),
            static_cast< cppu::OWeakObject * >(this));
    }
    components.addRootAccess(root);
    return static_cast< cppu::OWeakObject * >(root.get());
}

css::uno::Sequence< OUString > Service::getAvailableServiceNames()
{
    return { accessServiceName, updateAccessServiceName };
}

void Service::refresh() {
    //TODO
    cppu::OInterfaceContainerHelper * cont = rBHelper.getContainer(
        cppu::UnoType< css::util::XRefreshListener >::get());
    if (cont != nullptr) {
        css::lang::EventObject ev(static_cast< cppu::OWeakObject * >(this));
        cont->notifyEach(&css::util::XRefreshListener::refreshed, ev);
    }
}

void Service::addRefreshListener(
    css::uno::Reference< css::util::XRefreshListener > const & l)
{
    rBHelper.addListener(
        cppu::UnoType< css::util::XRefreshListener >::get(), l);
}

void Service::removeRefreshListener(
    css::uno::Reference< css::util::XRefreshListener > const & l)
{
    rBHelper.removeListener(
        cppu::UnoType< css::util::XRefreshListener >::get(), l);
}

void Service::flush() {
    flushModifications();
    cppu::OInterfaceContainerHelper * cont = rBHelper.getContainer(
        cppu::UnoType< css::util::XFlushListener >::get());
    if (cont != nullptr) {
        css::lang::EventObject ev(static_cast< cppu::OWeakObject * >(this));
        cont->notifyEach(&css::util::XFlushListener::flushed, ev);
    }
}

void Service::addFlushListener(
    css::uno::Reference< css::util::XFlushListener > const & l)
{
    rBHelper.addListener(cppu::UnoType< css::util::XFlushListener >::get(), l);
}

void Service::removeFlushListener(
    css::uno::Reference< css::util::XFlushListener > const & l)
{
    rBHelper.removeListener(
        cppu::UnoType< css::util::XFlushListener >::get(), l);
}

void Service::setLocale(css::lang::Locale const & eLocale)
{
    osl::MutexGuard guard(*lock_);
    locale_ = LanguageTag::convertToBcp47( eLocale, false);
}

css::lang::Locale Service::getLocale() {
    osl::MutexGuard guard(*lock_);
    css::lang::Locale loc;
    if (! locale_.isEmpty()) {
        loc = LanguageTag::convertToLocale( locale_, false);
    }
    return loc;
}

void Service::flushModifications() const {
    Components * components;
    {
        osl::MutexGuard guard(*lock_);
        components = &Components::getSingleton(context_);
    }
    components->flushModifications();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_configuration_ConfigurationProvider_get_implementation(
    css::uno::XComponentContext* Context, css::uno::Sequence<css::uno::Any> const& Arguments)
{
    if (!Arguments.hasElements()) {
        auto p = css::configuration::theDefaultProvider::get(Context);
        p->acquire();
        return p.get();
    } else {
        OUString locale;
        for (sal_Int32 i = 0; i < Arguments.getLength(); ++i) {
            css::beans::NamedValue v1;
            css::beans::PropertyValue v2;
            OUString name;
            css::uno::Any value;
            if (Arguments[i] >>= v1) {
                name = v1.Name;
                value = v1.Value;
            } else if (Arguments[i] >>= v2) {
                name = v2.Name;
                value = v2.Value;
            } else {
                throw css::uno::Exception(
                    ("com.sun.star.configuration.ConfigurationProvider factory"
                     " expects NamedValue or PropertyValue arguments"),
                    nullptr);
            }
            // For backwards compatibility, allow "Locale" and (ignored)
            // "EnableAsync" in any case:
            if (name.equalsIgnoreAsciiCase("locale")) {
                if (!locale.isEmpty() || !(value >>= locale) ||
                    locale.isEmpty())
                {
                    throw css::uno::Exception(
                        ("com.sun.star.configuration.ConfigurationProvider"
                         " factory expects at most one, non-empty, string"
                         " Locale argument"),
                        nullptr);
                }
            } else if (!name.equalsIgnoreAsciiCase("enableasync")) {
                throw css::uno::Exception(
                    ("com.sun.star.configuration.ConfigurationProvider factory:"
                     " unknown argument " + name),
                    nullptr);
            }
        }
        return cppu::acquire(static_cast< cppu::OWeakObject * >(new Service(Context, locale)));
    }
}

}

css::uno::Reference< css::uno::XInterface > createDefault(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
