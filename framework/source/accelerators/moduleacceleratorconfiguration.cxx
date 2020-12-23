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

#include <accelerators/acceleratorconfiguration.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/util/XChangesNotifier.hpp>

#include <cppuhelper/implbase.hxx>

using namespace framework;

namespace {

/**
    implements a read/write access to a module
    dependent accelerator configuration.
 */
typedef ::cppu::ImplInheritanceHelper<
            XCUBasedAcceleratorConfiguration,
            css::lang::XServiceInfo > ModuleAcceleratorConfiguration_BASE;

class ModuleAcceleratorConfiguration : public ModuleAcceleratorConfiguration_BASE
{
private:
    /** identify the application module, where this accelerator
        configuration cache should work on. */
    OUString m_sModule;

public:

    /** initialize this instance and fill the internal cache.

        @param  xSMGR
                reference to a uno service manager, which is used internally.
     */
    ModuleAcceleratorConfiguration(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Sequence< css::uno::Any >& lArguments);

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.ModuleAcceleratorConfiguration";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.ui.ModuleAcceleratorConfiguration"};
    }

    /// This has to be called after when the instance is acquire()'d.
    void fillCache();

private:
    /** helper to listen for configuration changes without ownership cycle problems */
    css::uno::Reference< css::util::XChangesListener > m_xCfgListener;
};

ModuleAcceleratorConfiguration::ModuleAcceleratorConfiguration(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Sequence< css::uno::Any >& lArguments)
    : ModuleAcceleratorConfiguration_BASE(xContext)
{
    SolarMutexGuard g;

    OUString sModule;
    if (lArguments.getLength() == 1 && (lArguments[0] >>= sModule))
    {
        m_sModule = sModule;
    } else
    {
        ::comphelper::SequenceAsHashMap lArgs(lArguments);
        m_sModule = lArgs.getUnpackedValueOrDefault("ModuleIdentifier", OUString());
        // OUString sLocale = lArgs.getUnpackedValueOrDefault("Locale", OUString("x-default"));
    }

    if (m_sModule.isEmpty())
        throw css::uno::RuntimeException(
                "The module dependent accelerator configuration service was initialized with an empty module identifier!",
                static_cast< ::cppu::OWeakObject* >(this));
}

void ModuleAcceleratorConfiguration::fillCache()
{
    {
        SolarMutexGuard g;
        m_sModuleCFG = m_sModule;
    }

#if 0
    // get current office locale ... but don't cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superfluous for this small implementation .-)
    // XXX: what is this good for? it was a comphelper::Locale but unused
    LanguageTag aLanguageTag(m_sLocale);
#endif

    // May be the current app module does not have any
    // accelerator config? Handle it gracefully :-)
    try
    {
        m_sGlobalOrModules = CFG_ENTRY::MODULES;
        XCUBasedAcceleratorConfiguration::reload();

        css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(m_xCfg, css::uno::UNO_QUERY_THROW);
        m_xCfgListener = new WeakChangesListener(this);
        xBroadcaster->addChangesListener(m_xCfgListener);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ModuleAcceleratorConfiguration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    ModuleAcceleratorConfiguration *inst = new ModuleAcceleratorConfiguration(context, arguments);
    css::uno::XInterface *acquired_inst = cppu::acquire(inst);

    inst->fillCache();

    return acquired_inst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
