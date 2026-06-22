/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <optional>
#include <vector>

#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <COKit/COKitTypes.h>

/// A clipboard implementation for COKit.
class KitClipboard final
    : public cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                           css::lang::XServiceInfo>
{
    osl::Mutex m_aMutex;
    css::uno::Reference<css::datatransfer::XTransferable> m_xTransferable;
    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> m_aListeners;
    int m_nViewId = -1;
    /**
     * When set, the app does the raw platform clipboard input and output for this
     * view: copies advertise through it and external pastes read through it.
     */
    std::optional<COKitClipboardProvider> m_oProvider;

public:
    KitClipboard();
    ~KitClipboard();

    void setViewId(int nViewId) { m_nViewId = nViewId; }

    /**
     * Install (or, with nullptr, remove) the app's platform clipboard backend.
     */
    void setProvider(const COKitClipboardProvider* pProvider);

    /// get an XInterface easily.
    css::uno::Reference<css::uno::XInterface> getXI()
    {
        return { static_cast<cppu::OWeakObject*>(this) };
    }

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    bool SAL_CALL supportsService(const OUString& ServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
    static css::uno::Sequence<OUString> getSupportedServiceNames_static();

    // XClipboard
    css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;
    void SAL_CALL setContents(
        const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable,
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
        override;
    OUString SAL_CALL getName() override { return u"CLIPBOARD"_ustr; }

    // XClipboardEx
    sal_Int8 SAL_CALL getRenderingCapabilities() override { return 0; }

    // XClipboardNotifier
    void SAL_CALL addClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;
    void SAL_CALL removeClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;
};

/// Represents the contents of KitClipboard.
class KitTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    css::uno::Sequence<css::datatransfer::DataFlavor> m_aFlavors;
    std::vector<css::uno::Any> m_aContent;

public:
    /**
     * Fill a DataFlavor from a wire MIME type, picking the UNO DataType (string
     * vs byte sequence) the engine expects for it.
     */
    static void initFlavourFromMime(css::datatransfer::DataFlavor& rFlavor, OUString aMimeType);

    KitTransferable();
    KitTransferable(size_t nInCount, const char** pInMimeTypes, const size_t* pInSizes,
                    const char** pInStreams);
    KitTransferable(const OUString& sMimeType, const css::uno::Sequence<sal_Int8>& aSequence);

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;

    bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * Clipboard contents that live on the platform clipboard. It advertises the
 * flavors the platform offers (asked of the provider once, no bytes) and fetches
 * the bytes for a single flavor only when the engine actually pulls it, so a
 * paste renders only the one format the engine chose. The provider's pUserData
 * is owned by the KitClipboard that handed it over, not by this object.
 */
class KitProviderTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    COKitClipboardProvider m_aProvider;
    css::uno::Sequence<css::datatransfer::DataFlavor> m_aFlavors;
    mutable std::map<OUString, css::uno::Any> m_aCache;

public:
    explicit KitProviderTransferable(const COKitClipboardProvider& rProvider);

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

/// Theoretically to hook into the (horrible) vcl dtranscomp.cxx code.
class KitClipboardFactory : public ::cppu::WeakComponentImplHelper<css::lang::XSingleServiceFactory>
{
    static osl::Mutex gMutex;

public:
    KitClipboardFactory()
        : cppu::WeakComponentImplHelper<css::lang::XSingleServiceFactory>(gMutex)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL createInstance() override
    {
        return createInstanceWithArguments(css::uno::Sequence<css::uno::Any>());
    }
    css::uno::Reference<css::uno::XInterface> SAL_CALL
    createInstanceWithArguments(const css::uno::Sequence<css::uno::Any>& /* rArgs */) override;

    /// Fetch clipboard from the global pool.
    static rtl::Reference<KitClipboard> getClipboardForCurView();

    /// Release a clipboard before its document dies, nViewId of -1 clears all.
    static void releaseClipboardForView(int nViewId);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
