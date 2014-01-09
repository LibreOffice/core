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
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <acceleratorconst.h>
#include <services.h>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>

#include <cppuhelper/implbase2.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ref.hxx>

using namespace framework;

namespace {

/**
    implements a read/write access to a document
    based accelerator configuration.
 */

typedef ::cppu::ImplInheritanceHelper2<
             XMLBasedAcceleratorConfiguration,
             css::lang::XServiceInfo,
             css::lang::XInitialization > DocumentAcceleratorConfiguration_BASE;

class DocumentAcceleratorConfiguration : public DocumentAcceleratorConfiguration_BASE
{
private:

    //----------------------------------
    /** points to the root storage of the outside document,
        where we can read/save our configuration data. */
    css::uno::Reference< css::embed::XStorage > m_xDocumentRoot;

//______________________________________
// interface

public:

    /** initialize this instance and fill the internal cache.

        @param  xSMGR
                reference to an uno service manager, which is used internaly.
     */
    DocumentAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~DocumentAcceleratorConfiguration();

    // XInterface, XTypeProvider, XServiceInfo
    DECLARE_XSERVICEINFO

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
        throw(css::uno::Exception       ,
              css::uno::RuntimeException);

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

DEFINE_XSERVICEINFO_MULTISERVICE_2(DocumentAcceleratorConfiguration                   ,
                                   ::cppu::OWeakObject                                ,
                                   "com.sun.star.ui.DocumentAcceleratorConfiguration" ,
                                   OUString("com.sun.star.comp.framework.DocumentAcceleratorConfiguration"))

DEFINE_INIT_SERVICE(DocumentAcceleratorConfiguration,
                    {
                        /*Attention
                        I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                        to create a new instance of this class by our own supported service factory.
                        see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

//-----------------------------------------------
DocumentAcceleratorConfiguration::DocumentAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : DocumentAcceleratorConfiguration_BASE(xContext)
{
}

//-----------------------------------------------
DocumentAcceleratorConfiguration::~DocumentAcceleratorConfiguration()
{
    m_aPresetHandler.removeStorageListener(this);
}

//-----------------------------------------------
void SAL_CALL DocumentAcceleratorConfiguration::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
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
    // <- SAFE ----------------------------------

    impl_ts_fillCache();
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
        css::uno::XComponentContext * context,
        uno_Sequence * arguments)
{
    assert(arguments != 0);
    rtl::Reference<DocumentAcceleratorConfiguration> x(
            new DocumentAcceleratorConfiguration(context));
    css::uno::Sequence<css::uno::Any> aArgs(
            reinterpret_cast<css::uno::Any *>(arguments->elements),
            arguments->nElements);
    x->initialize(aArgs);
    x->acquire();
    return static_cast<cppu::OWeakObject *>(x.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
