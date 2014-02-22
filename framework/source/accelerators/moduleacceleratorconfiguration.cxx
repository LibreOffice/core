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

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/presethandler.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include "helper/mischelper.hxx"

#include <acceleratorconst.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/util/XChangesNotifier.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <rtl/logfile.h>

using namespace framework;

namespace {

/**
    implements a read/write access to a module
    dependend accelerator configuration.
 */
typedef ::cppu::ImplInheritanceHelper1<
            XCUBasedAcceleratorConfiguration,
            css::lang::XServiceInfo > ModuleAcceleratorConfiguration_BASE;

class ModuleAcceleratorConfiguration : public ModuleAcceleratorConfiguration_BASE
{
private:
    /** identify the application module, where this accelerator
        configuration cache should work on. */
    OUString m_sModule;
    OUString m_sLocale;

public:

    /** initialize this instance and fill the internal cache.

        @param  xSMGR
                reference to an uno service manager, which is used internally.
     */
    ModuleAcceleratorConfiguration(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& lArguments);

    /** TODO */
    virtual ~ModuleAcceleratorConfiguration();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.ModuleAcceleratorConfiguration");
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
        aSeq[0] = OUString("com.sun.star.ui.ModuleAcceleratorConfiguration");
        return aSeq;
    }

    
    virtual  void SAL_CALL dispose() throw (css::uno::RuntimeException);

    
    void SAL_CALL fillCache();

private:
    /** helper to listen for configuration changes without ownership cycle problems */
    css::uno::Reference< css::util::XChangesListener > m_xCfgListener;
};

ModuleAcceleratorConfiguration::ModuleAcceleratorConfiguration(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& lArguments)
    : ModuleAcceleratorConfiguration_BASE(xContext)
{
    WriteGuard aWriteLock(m_aLock);

    OUString sModule;
    if (lArguments.getLength() == 1 && (lArguments[0] >>= sModule))
    {
        m_sModule = sModule;
    } else
    {
        ::comphelper::SequenceAsHashMap lArgs(lArguments);
        m_sModule = lArgs.getUnpackedValueOrDefault("ModuleIdentifier", OUString());
        m_sLocale = lArgs.getUnpackedValueOrDefault("Locale", OUString("x-default"));
    }

    if (m_sModule.isEmpty())
        throw css::uno::RuntimeException(
                OUString("The module dependend accelerator configuration service was initialized with an empty module identifier!"),
                static_cast< ::cppu::OWeakObject* >(this));

    aWriteLock.unlock();
}

ModuleAcceleratorConfiguration::~ModuleAcceleratorConfiguration()
{
}

void ModuleAcceleratorConfiguration::fillCache()
{
    
    ReadGuard aReadLock(m_aLock);
    m_sModuleCFG = m_sModule;
    aReadLock.unlock();
    

#if 0
    
    
    
    
    LanguageTag aLanguageTag(m_sLocale);
#endif

    
    
    try
    {
        m_sGlobalOrModules = CFG_ENTRY_MODULES;
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


//

//
void SAL_CALL ModuleAcceleratorConfiguration::dispose()
    throw(css::uno::RuntimeException)
{
    try
    {
        css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(m_xCfg, css::uno::UNO_QUERY_THROW);
        if ( xBroadcaster.is() )
            xBroadcaster->removeChangesListener(static_cast< css::util::XChangesListener* >(this));
    }
    catch(const css::uno::RuntimeException&)
    { throw; }
    catch(const css::uno::Exception&)
    {}
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
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
