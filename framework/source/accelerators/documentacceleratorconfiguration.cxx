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
#include <accelerators/presethandler.hxx>

#include <xml/acceleratorconfigurationreader.hxx>
#include <xml/acceleratorconfigurationwriter.hxx>
#include <xml/saxnamespacefilter.hxx>

#include <acceleratorconst.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>

using namespace framework;

#define RESOURCETYPE_ACCELERATOR "accelerator"

namespace {

/**
    implements a read/write access to a document
    based accelerator configuration.
 */

typedef ::cppu::ImplInheritanceHelper<
             XMLBasedAcceleratorConfiguration,
             css::lang::XServiceInfo> DocumentAcceleratorConfiguration_BASE;

class DocumentAcceleratorConfiguration : public DocumentAcceleratorConfiguration_BASE
{
private:

    /** points to the root storage of the outside document,
        where we can read/save our configuration data. */
    css::uno::Reference< css::embed::XStorage > m_xDocumentRoot;

public:

    /** initialize this instance and fill the internal cache.

        @param  xSMGR
                reference to an uno service manager, which is used internally.
     */
    DocumentAcceleratorConfiguration(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Sequence< css::uno::Any >& lArguments);

    virtual ~DocumentAcceleratorConfiguration();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.DocumentAcceleratorConfiguration");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.ui.DocumentAcceleratorConfiguration" };
        return aSeq;
    }

    // XUIConfigurationStorage
    virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasStorage()
        throw(css::uno::RuntimeException, std::exception) override;

    /** read all data into the cache. */
    void fillCache();

private:

    /** forget all currently cached data AND(!)
        forget all currently used storages. */
    void clearCache();
};

DocumentAcceleratorConfiguration::DocumentAcceleratorConfiguration(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Sequence< css::uno::Any >& lArguments)
    : DocumentAcceleratorConfiguration_BASE(xContext)
{
    {
        SolarMutexGuard g;
        css::uno::Reference<css::embed::XStorage> xRoot;
        if (lArguments.getLength() == 1 && (lArguments[0] >>= xRoot))
        {
            m_xDocumentRoot = xRoot;
        }
        else
        {
            ::comphelper::SequenceAsHashMap lArgs(lArguments);
            m_xDocumentRoot = lArgs.getUnpackedValueOrDefault(
                "DocumentRoot",
                css::uno::Reference< css::embed::XStorage >());
        }
    }
}

DocumentAcceleratorConfiguration::~DocumentAcceleratorConfiguration()
{
    m_aPresetHandler.removeStorageListener(this);
}

void SAL_CALL DocumentAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
    throw(css::uno::RuntimeException, std::exception)
{
    // Attention! xStorage must be accepted too, if it's NULL !

    bool bForgetOldStorages;
    {
        SolarMutexGuard g;
        bForgetOldStorages = m_xDocumentRoot.is();
        m_xDocumentRoot = xStorage;
    }

    if (bForgetOldStorages)
        clearCache();

    if (xStorage.is())
        fillCache();
}

sal_Bool SAL_CALL DocumentAcceleratorConfiguration::hasStorage()
    throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;
    return m_xDocumentRoot.is();
}

void DocumentAcceleratorConfiguration::fillCache()
{
    css::uno::Reference< css::embed::XStorage > xDocumentRoot;
    {
        SolarMutexGuard g;
        xDocumentRoot = m_xDocumentRoot;
    }

    // Sometimes we must live without a document root.
    // E.g. if the document is readonly ...
    if (!xDocumentRoot.is())
        return;

    // get current office locale ... but don't cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superflous for this small implementation .-)
    LanguageTag aLanguageTag( impl_ts_getLocale());

    // May be the current document does not contain any
    // accelerator config? Handle it gracefully :-)
    try
    {
        // Note: The used preset class is threadsafe by itself ... and live if we live!
        // We do not need any mutex here.

        // open the folder, where the configuration exists
        m_aPresetHandler.connectToResource(
            PresetHandler::E_DOCUMENT,
            RESOURCETYPE_ACCELERATOR,
            OUString(),
            xDocumentRoot,
            aLanguageTag);

        DocumentAcceleratorConfiguration::reload();
        m_aPresetHandler.addStorageListener(this);
    }
    catch(const css::uno::Exception&)
    {}
}

void DocumentAcceleratorConfiguration::clearCache()
{
    m_aPresetHandler.forgetCachedStorages();
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_DocumentAcceleratorConfiguration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    DocumentAcceleratorConfiguration *inst = new DocumentAcceleratorConfiguration(context, arguments);
    css::uno::XInterface *acquired_inst = cppu::acquire(inst);

    inst->fillCache();

    return acquired_inst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
