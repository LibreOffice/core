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
#include <accelerators/keymapping.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>

using namespace framework;

namespace {

/**
    implements a read/write access to the global
    accelerator configuration.
 */
typedef ::cppu::ImplInheritanceHelper<
             XCUBasedAcceleratorConfiguration,
             css::lang::XServiceInfo > GlobalAcceleratorConfiguration_BASE;
class GlobalAcceleratorConfiguration : public GlobalAcceleratorConfiguration_BASE
{
public:

    /** initialize this instance and fill the internal cache.

        @param  xSMGR
                reference to a uno service manager, which is used internally.
     */
    explicit GlobalAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext);

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.GlobalAcceleratorConfiguration";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.ui.GlobalAcceleratorConfiguration"};
    }

    /// This has to be called after when the instance is acquire()'d.
    void fillCache();

private:

    /** helper to listen for configuration changes without ownership cycle problems */
    css::uno::Reference< css::util::XChangesListener > m_xCfgListener;
};

GlobalAcceleratorConfiguration::GlobalAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : GlobalAcceleratorConfiguration_BASE(xContext)
{
    // force keyboard string registration.
    KeyMapping::get();
}

void GlobalAcceleratorConfiguration::fillCache()
{
    /** read all data into the cache. */

#if 0
    // get current office locale ... but don't cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superfluous for this small implementation .-)
    // XXX: what is this good for? it was a comphelper::Locale but unused
    LanguageTag aLanguageTag(m_sLocale);
#endif

    // May be there exists no accelerator config? Handle it gracefully :-)
    try
    {
        m_sGlobalOrModules = CFG_ENTRY_GLOBAL;
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
com_sun_star_comp_framework_GlobalAcceleratorConfiguration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    rtl::Reference<GlobalAcceleratorConfiguration> xGAC = new GlobalAcceleratorConfiguration(context);
    xGAC->fillCache();
    return cppu::acquire(xGAC.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
