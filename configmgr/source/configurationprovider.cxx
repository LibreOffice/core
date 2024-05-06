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
#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <sal/types.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <i18nlangtag/languagetag.hxx>
#include <utility>

#include "components.hxx"
#include "configurationprovider.hxx"
#include "lock.hxx"
#include "defaultprovider.hxx"
#include "rootaccess.hxx"

namespace configmgr::configuration_provider {

namespace {

constexpr OUString accessServiceName =
    u"com.sun.star.configuration.ConfigurationAccess"_ustr;
constexpr OUString updateAccessServiceName =
    u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr;

void badNodePath() {
    throw css::uno::Exception(
        (u"com.sun.star.configuration.ConfigurationProvider expects a single,"
         " non-empty, string nodepath argument"_ustr),
        nullptr);
}

typedef
    comphelper::WeakComponentImplHelper<
        css::lang::XServiceInfo, css::lang::XMultiServiceFactory,
        css::util::XRefreshable, css::util::XFlushable,
        css::lang::XLocalizable >
    ServiceBase;

class Service : public ServiceBase
{
public:
    explicit Service(
        const css::uno::Reference< css::uno::XComponentContext >& context):
        context_(context), default_(true),
        lock_( lock() )
    {
        assert(context.is());
    }

    Service(
        const css::uno::Reference< css::uno::XComponentContext >& context,
        OUString locale):
        context_(context), locale_(std::move(locale)),
        default_(false),
        lock_( lock() )
    {
        assert(context.is());
    }

private:
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    virtual ~Service() override {}

    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return default_
            ? default_provider::getImplementationName()
            : u"com.sun.star.comp.configuration.ConfigurationProvider"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override
    {
        return default_
            ? default_provider::getSupportedServiceNames()
            : css::uno::Sequence<OUString> { u"com.sun.star.configuration.ConfigurationProvider"_ustr };
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
    comphelper::OInterfaceContainerHelper4<css::util::XRefreshListener> maRefreshListeners;
    comphelper::OInterfaceContainerHelper4<css::util::XFlushListener> maFlushListeners;
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
                (u"com.sun.star.configuration.ConfigurationProvider expects"
                 " NamedValue or PropertyValue arguments"_ustr),
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
                    (u"com.sun.star.configuration.ConfigurationProvider expects"
                     " at most one, non-empty, string Locale argument"_ustr),
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
            getXWeak());
    }
    osl::MutexGuard guard(*lock_);
    Components & components = Components::getSingleton(context_);
    rtl::Reference root(
        new RootAccess(components, nodepath, locale, update));
    if (root->isValue()) {
        throw css::uno::Exception(
            ("com.sun.star.configuration.ConfigurationProvider: there is a leaf"
             " value at nodepath " + nodepath),
            getXWeak());
    }
    components.addRootAccess(root);
    return root->getXWeak();
}

css::uno::Sequence< OUString > Service::getAvailableServiceNames()
{
    return { accessServiceName, updateAccessServiceName };
}

void Service::refresh() {
    //TODO
    std::unique_lock g(m_aMutex);
    if (maRefreshListeners.getLength(g)) {
        css::lang::EventObject ev(getXWeak());
        maRefreshListeners.notifyEach(g, &css::util::XRefreshListener::refreshed, ev);
    }
}

void Service::addRefreshListener(
    css::uno::Reference< css::util::XRefreshListener > const & l)
{
    std::unique_lock g(m_aMutex);
    maRefreshListeners.addInterface(g, l);
}

void Service::removeRefreshListener(
    css::uno::Reference< css::util::XRefreshListener > const & l)
{
    std::unique_lock g(m_aMutex);
    maRefreshListeners.removeInterface(g, l);
}

void Service::flush() {
    flushModifications();
    std::unique_lock g(m_aMutex);
    if (maFlushListeners.getLength(g)) {
        css::lang::EventObject ev(getXWeak());
        maFlushListeners.notifyEach(g, &css::util::XFlushListener::flushed, ev);
    }
}

void Service::addFlushListener(
    css::uno::Reference< css::util::XFlushListener > const & l)
{
    std::unique_lock g(m_aMutex);
    maFlushListeners.addInterface(g, l);
}

void Service::removeFlushListener(
    css::uno::Reference< css::util::XFlushListener > const & l)
{
    std::unique_lock g(m_aMutex);
    maFlushListeners.removeInterface(g, l);
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

void Service::disposing(std::unique_lock<std::mutex>& rGuard) {
    rGuard.unlock(); // just in case we call back into Service during dispose()
    flushModifications();
    rGuard.lock();
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
                    (u"com.sun.star.configuration.ConfigurationProvider factory"
                     " expects NamedValue or PropertyValue arguments"_ustr),
                    nullptr);
            }
            // For backwards compatibility, allow "Locale" and (ignored)
            // "EnableAsync" in any case:
            if (name.equalsIgnoreAsciiCase("locale")) {
                if (!locale.isEmpty() || !(value >>= locale) ||
                    locale.isEmpty())
                {
                    throw css::uno::Exception(
                        (u"com.sun.star.configuration.ConfigurationProvider"
                         " factory expects at most one, non-empty, string"
                         " Locale argument"_ustr),
                        nullptr);
                }
            } else if (!name.equalsIgnoreAsciiCase("enableasync")) {
                throw css::uno::Exception(
                    ("com.sun.star.configuration.ConfigurationProvider factory:"
                     " unknown argument " + name),
                    nullptr);
            }
        }
        return cppu::acquire(new Service(Context, locale));
    }
}

}

css::uno::Reference< css::uno::XInterface > createDefault(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return getXWeak(new Service(context));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
