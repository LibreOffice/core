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
#include <accelerators/istoragelistener.hxx>
#include <accelerators/presethandler.hxx>

#include <xml/acceleratorconfigurationreader.hxx>
#include <xml/acceleratorconfigurationwriter.hxx>
#include <xml/saxnamespacefilter.hxx>

#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <acceleratorconst.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ref.hxx>

using namespace framework;

namespace {

/**
    implements a read/write access to a document
    based accelerator configuration.
 */

typedef ::cppu::ImplInheritanceHelper1<
             XMLBasedAcceleratorConfiguration,
             css::lang::XServiceInfo> DocumentAcceleratorConfiguration_BASE;

class DocumentAcceleratorConfiguration : public DocumentAcceleratorConfiguration_BASE
{
private:

    //----------------------------------
    /** points to the root storage of the outside document,
        where we can read/save our configuration data. */
    css::uno::Reference< css::embed::XStorage > m_xDocumentRoot;

public:

    /** initialize this instance and fill the internal cache.

        @param  xSMGR
                reference to an uno service manager, which is used internaly.
     */
    DocumentAcceleratorConfiguration(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Sequence< css::uno::Any >& lArguments);

    virtual ~DocumentAcceleratorConfiguration();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.DocumentAcceleratorConfiguration");
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
        aSeq[0] = OUString("com.sun.star.ui.DocumentAcceleratorConfiguration");
        return aSeq;
    }

    // XUIConfigurationStorage
    virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasStorage()
        throw(css::uno::RuntimeException);

private:

    /** read all data into the cache. */
    void impl_ts_fillCache();

    /** forget all currently cached data AND(!)
        forget all currently used storages. */
    void impl_ts_clearCache();
};

//-----------------------------------------------
DocumentAcceleratorConfiguration::DocumentAcceleratorConfiguration(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& lArguments)
    : DocumentAcceleratorConfiguration_BASE(xContext)
{
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference<css::embed::XStorage> xRoot;
    if (lArguments.getLength() == 1 && (lArguments[0] >>= xRoot))
    {
        m_xDocumentRoot = xRoot;
    }
    else
    {
        ::comphelper::SequenceAsHashMap lArgs(lArguments);
        m_xDocumentRoot = lArgs.getUnpackedValueOrDefault(
                            OUString("DocumentRoot"),
                            css::uno::Reference< css::embed::XStorage >());
    }

    aWriteLock.unlock();

    impl_ts_fillCache();
}

//-----------------------------------------------
DocumentAcceleratorConfiguration::~DocumentAcceleratorConfiguration()
{
    m_aPresetHandler.removeStorageListener(this);
}

//-----------------------------------------------
void SAL_CALL DocumentAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
    throw(css::uno::RuntimeException)
{
    // Attention! xStorage must be accepted too, if it's NULL !

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    sal_Bool bForgetOldStorages = m_xDocumentRoot.is();
    m_xDocumentRoot = xStorage;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    if (bForgetOldStorages)
        impl_ts_clearCache();

    if (xStorage.is())
        impl_ts_fillCache();
}

//-----------------------------------------------
sal_Bool SAL_CALL DocumentAcceleratorConfiguration::hasStorage()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    return m_xDocumentRoot.is();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void DocumentAcceleratorConfiguration::impl_ts_fillCache()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xDocumentRoot = m_xDocumentRoot;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // Sometimes we must live without a document root.
    // E.g. if the document is readonly ...
    if (!xDocumentRoot.is())
        return;

    // get current office locale ... but dont cache it.
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
            PresetHandler::RESOURCETYPE_ACCELERATOR(),
            OUString(),
            xDocumentRoot,
            aLanguageTag);

        DocumentAcceleratorConfiguration::reload();
        m_aPresetHandler.addStorageListener(this);
    }
    catch(const css::uno::Exception&)
    {}
}

//-----------------------------------------------
void DocumentAcceleratorConfiguration::impl_ts_clearCache()
{
    m_aPresetHandler.forgetCachedStorages();
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_DocumentAcceleratorConfiguration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new DocumentAcceleratorConfiguration(context, arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
