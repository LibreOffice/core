/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "kitclipboard.hxx"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <comphelper/kit.hxx>
#include <comphelper/sequence.hxx>
#include <tools/json_writer.hxx>
#include <tools/lazydelete.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/kit/helper.hxx>
#include <COKit/COKitEnums.h>
#include <sal/log.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <desktop/kitclipboard.hxx>

using namespace css;
using namespace css::uno;
using namespace cpo::uno;

/* static */ osl::Mutex KitClipboardFactory::gMutex;
static tools::DeleteOnDeinit<std::unordered_map<int, rtl::Reference<KitClipboard>>>& getClipboards()
{
    static tools::DeleteOnDeinit<std::unordered_map<int, rtl::Reference<KitClipboard>>>
        gClipboards{};
    return gClipboards;
}

rtl::Reference<KitClipboard> KitClipboardFactory::getClipboardForCurView()
{
    int nViewId = KitHelper::getCurrentView(); // currently active.

    osl::MutexGuard aGuard(gMutex);

    auto& gClipboards = getClipboards();
    auto it = gClipboards.get()->find(nViewId);
    if (it != gClipboards.get()->end())
    {
        SAL_INFO("kit", "Got clip: " << it->second.get() << " from " << nViewId);
        return it->second;
    }
    rtl::Reference<KitClipboard> xClip(new KitClipboard());
    xClip->setViewId(nViewId);
    (*gClipboards.get())[nViewId] = xClip;
    SAL_INFO("kit", "Created clip: " << xClip.get() << " for viewId " << nViewId);
    return xClip;
}

void KitClipboardFactory::releaseClipboardForView(int nViewId)
{
    osl::MutexGuard aGuard(gMutex);

    auto& gClipboards = getClipboards();
    if (nViewId < 0) // clear all
    {
        gClipboards.get()->clear();
        SAL_INFO("kit", "Released all clipboards on doc destroy\n");
    }
    else if (gClipboards.get())
    {
        auto it = gClipboards.get()->find(nViewId);
        if (it != gClipboards.get()->end())
        {
            SAL_INFO("kit", "Releasing clip: " << it->second.get() << " for destroyed " << nViewId);
            gClipboards.get()->erase(it);
        }
    }
}

uno::Reference<uno::XInterface>
    SAL_CALL KitClipboardFactory::createInstanceWithArguments(const Sequence<Any>& /* rArgs */)
{
    return { static_cast<cppu::OWeakObject*>(getClipboardForCurView().get()) };
}

void clearAllKitClipboardsContents()
{
    for (const auto & pair : *getClipboards().get())
        pair.second->setContents(nullptr, nullptr);
}

KitClipboard::KitClipboard()
    : cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                    css::lang::XServiceInfo>(m_aMutex)
{
    // Encourage 'paste' menu items to always show up.
    uno::Reference<datatransfer::XTransferable> xTransferable(new KitTransferable());
    setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());
}

KitClipboard::~KitClipboard() { setProvider(nullptr); }

void KitClipboard::setProvider(const COKitClipboardProvider* pProvider)
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_oProvider && m_oProvider->release)
        m_oProvider->release(m_oProvider->pUserData);
    if (pProvider)
    {
        // Copy only what the caller actually filled in (its nSize), leaving any
        // fields a future engine adds zeroed. Keeps an older app's smaller struct
        // safe to read.
        COKitClipboardProvider aCopy{};
        std::memcpy(&aCopy, pProvider, std::min(pProvider->nSize, sizeof(aCopy)));
        m_oProvider = aCopy;
    }
    else
        m_oProvider.reset();
}

Sequence<OUString> KitClipboard::getSupportedServiceNames_static()
{
    Sequence<OUString> aRet{ u"com.sun.star.datatransfer.clipboard.KitClipboard"_ustr };
    return aRet;
}

OUString KitClipboard::getImplementationName()
{
    return u"com.sun.star.datatransfer.KitClipboard"_ustr;
}

Sequence<OUString> KitClipboard::getSupportedServiceNames()
{
    return getSupportedServiceNames_static();
}

bool KitClipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Reference<css::datatransfer::XTransferable> KitClipboard::getContents()
{
    // With a provider whose platform clipboard is no longer ours, paste from the
    // platform: the engine picks one format from the flavor list and pulls only
    // that one. When the platform still holds our own copy, fall through to the
    // in-memory transferable for full fidelity.
    if (m_oProvider && m_oProvider->getMimeTypes && m_oProvider->getDataForMimeType)
    {
        const bool bOurs = m_oProvider->ownsClipboard
                           && m_oProvider->ownsClipboard(m_oProvider->pUserData) != 0;
        if (!bOurs)
            return new KitProviderTransferable(*m_oProvider);
    }
    return m_xTransferable;
}

void KitClipboard::setContents(
    const Reference<css::datatransfer::XTransferable>& xTrans,
    const Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    Reference<datatransfer::clipboard::XClipboardOwner> xOldOwner(m_aOwner);
    Reference<datatransfer::XTransferable> xOldContents(m_xTransferable);
    m_xTransferable = xTrans;
    m_aOwner = xClipboardOwner;

    std::vector<Reference<datatransfer::clipboard::XClipboardListener>> aListeners(m_aListeners);
    datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = m_xTransferable;
    SAL_INFO("kit", "Clip: " << this << " set contents to " << m_xTransferable);

    aGuard.clear();

    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership(this, xOldContents);
    for (auto const& listener : aListeners)
    {
        listener->changedContents(aEv);
    }

    // Emit here rather than from SfxClipboardChangeListener: the listener can
    // attach to the pre-Kit default clipboard before the per-view Kit clipboard
    // lands on the window frame, and then miss every real copy.
    if (!xTrans.is() || !comphelper::COKit::isActive() || m_nViewId < 0)
        return;

    std::vector<OString> aMimeTypes;
    try
    {
        const auto aFlavors = xTrans->getTransferDataFlavors();
        aMimeTypes.reserve(aFlavors.getLength());
        for (const auto& rFlavor : aFlavors)
        {
            OString aMime = OUStringToOString(rFlavor.MimeType, RTL_TEXTENCODING_UTF8);
            // Match doc_getClipboard() behaviour: advertise UTF-8 not UTF-16
            if (aMime.startsWith("text/plain"))
                aMime = "text/plain;charset=utf-8"_ostr;
            aMimeTypes.push_back(aMime);
        }
    }
    catch (const uno::Exception&)
    {
        return;
    }

    if (aMimeTypes.empty())
        return;

    // With a provider, advertise straight onto the platform clipboard; otherwise
    // tell the view's client (the browser) which formats are now available.
    if (m_oProvider && m_oProvider->advertiseToPlatform)
    {
        std::vector<const char*> aPtrs;
        aPtrs.reserve(aMimeTypes.size() + 1);
        for (const auto& rMime : aMimeTypes)
            aPtrs.push_back(rMime.getStr());
        aPtrs.push_back(nullptr);
        m_oProvider->advertiseToPlatform(m_oProvider->pUserData, aPtrs.data());
        return;
    }

    tools::JsonWriter aWriter;
    {
        auto aArr = aWriter.startArray("mimeTypes");
        for (const auto& rMime : aMimeTypes)
            aWriter.putSimpleValue(OUString::fromUtf8(rMime));
    }
    KitHelper::notifyView(m_nViewId, KIT_CALLBACK_CLIPBOARD_MIMETYPES,
                          aWriter.finishAndGetAsOString());
}

void KitClipboard::addClipboardListener(
    const Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    m_aListeners.push_back(listener);
}

void KitClipboard::removeClipboardListener(
    const Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    std::erase(m_aListeners, listener);
}
KitTransferable::KitTransferable(const OUString& sMimeType,
                                 const css::uno::Sequence<sal_Int8>& aSequence)
{
    m_aContent.reserve(1);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(1);
    initFlavourFromMime(m_aFlavors.getArray()[0], sMimeType);

    cpo::uno::Any aContent;
    if (m_aFlavors[0].DataType == cppu::UnoType<OUString>::get())
    {
        auto pText = reinterpret_cast<const char*>(aSequence.getConstArray());
        aContent <<= OUString(pText, aSequence.getLength(), RTL_TEXTENCODING_UTF8);
    }
    else
        aContent <<= aSequence;
    m_aContent.push_back(aContent);
}

/// Use to ensure we have some dummy content on the clipboard to allow a 1st 'paste'
KitTransferable::KitTransferable()
{
    m_aContent.reserve(1);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(1);
    initFlavourFromMime(m_aFlavors.getArray()[0], u"text/plain"_ustr);
    cpo::uno::Any aContent;
    aContent <<= OUString();
    m_aContent.push_back(aContent);
}

// cf. sot/source/base/exchange.cxx for these two exceptional types.
void KitTransferable::initFlavourFromMime(css::datatransfer::DataFlavor& rFlavor,
                                          OUString aMimeType)
{
    if (aMimeType.startsWith("text/plain"))
    {
        aMimeType = u"text/plain;charset=utf-16"_ustr;
        rFlavor.DataType = cppu::UnoType<OUString>::get();
    }
    else if (aMimeType.startsWith("text/markdown"))
    {
        aMimeType = u"text/markdown"_ustr;
        rFlavor.DataType = cppu::UnoType<OUString>::get();
    }
    else if (aMimeType == "application/x-libreoffice-markdown-annotated")
        rFlavor.DataType = cppu::UnoType<OUString>::get();
    else if (aMimeType == "application/x-libreoffice-tsvc")
        rFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        rFlavor.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();
    rFlavor.MimeType = aMimeType;
    rFlavor.HumanPresentableName = aMimeType;
}

KitTransferable::KitTransferable(const size_t nInCount, const char** pInMimeTypes,
                                 const size_t* pInSizes, const char** pInStreams)
{
    m_aContent.reserve(nInCount);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(nInCount);
    auto p_aFlavors = m_aFlavors.getArray();
    for (size_t i = 0; i < nInCount; ++i)
    {
        initFlavourFromMime(p_aFlavors[i], OUString::fromUtf8(pInMimeTypes[i]));

        cpo::uno::Any aContent;
        if (m_aFlavors[i].DataType == cppu::UnoType<OUString>::get())
            aContent <<= OUString(pInStreams[i], pInSizes[i], RTL_TEXTENCODING_UTF8);
        else
            aContent <<= css::uno::Sequence<sal_Int8>(
                reinterpret_cast<const sal_Int8*>(pInStreams[i]), pInSizes[i]);
        m_aContent.push_back(aContent);
    }
}

cpo::uno::Any SAL_CALL KitTransferable::getTransferData(const datatransfer::DataFlavor& rFlavor)
{
    assert(m_aContent.size() == static_cast<size_t>(m_aFlavors.getLength()));
    for (size_t i = 0; i < m_aContent.size(); ++i)
    {
        if (m_aFlavors[i].MimeType == rFlavor.MimeType)
        {
            if (m_aFlavors[i].DataType != rFlavor.DataType)
                SAL_WARN("kit", "Horror type mismatch!");
            return m_aContent[i];
        }
    }
    return {};
}

uno::Sequence<datatransfer::DataFlavor> SAL_CALL KitTransferable::getTransferDataFlavors()
{
    return m_aFlavors;
}

bool SAL_CALL KitTransferable::isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor)
{
    return std::find_if(std::cbegin(m_aFlavors), std::cend(m_aFlavors),
                        [&rFlavor](const datatransfer::DataFlavor& i) {
                            return i.MimeType == rFlavor.MimeType && i.DataType == rFlavor.DataType;
                        })
           != std::cend(m_aFlavors);
}

KitProviderTransferable::KitProviderTransferable(const COKitClipboardProvider& rProvider)
    : m_aProvider(rProvider)
{
    std::vector<datatransfer::DataFlavor> aFlavors;
    char** ppMimeTypes = m_aProvider.getMimeTypes ? m_aProvider.getMimeTypes(m_aProvider.pUserData)
                                                  : nullptr;
    if (ppMimeTypes)
    {
        for (size_t i = 0; ppMimeTypes[i]; ++i)
        {
            datatransfer::DataFlavor aFlavor;
            KitTransferable::initFlavourFromMime(aFlavor, OUString::fromUtf8(ppMimeTypes[i]));
            free(ppMimeTypes[i]);

            // Several platform types can map to one flavor (the plain-text
            // variants); keep the first.
            const bool bSeen = std::any_of(aFlavors.begin(), aFlavors.end(),
                                           [&aFlavor](const datatransfer::DataFlavor& r)
                                           { return r.MimeType == aFlavor.MimeType; });
            if (!bSeen)
                aFlavors.push_back(aFlavor);
        }
        free(ppMimeTypes);
    }
    m_aFlavors = comphelper::containerToSequence(aFlavors);
}

cpo::uno::Any SAL_CALL KitProviderTransferable::getTransferData(const datatransfer::DataFlavor& rFlavor)
{
    auto itCache = m_aCache.find(rFlavor.MimeType);
    if (itCache != m_aCache.end())
        return itCache->second;

    if (!m_aProvider.getDataForMimeType)
        return {};

    // The provider speaks the wire MIME; the engine flavor for text carries the
    // UTF-16 variant, so ask for UTF-8 back.
    const bool bText = rFlavor.DataType == cppu::UnoType<OUString>::get();
    OString aWireMime = OUStringToOString(rFlavor.MimeType, RTL_TEXTENCODING_UTF8);
    if (aWireMime.startsWith("text/plain"))
        aWireMime = "text/plain;charset=utf-8"_ostr;

    // Reading the platform clipboard can re-enter the source document (its own
    // advertise pulls a format through getClipboard, which makes its view
    // current). Restore the current view afterwards so the running paste stays
    // on its own view.
    const int nSavedView = KitHelper::getCurrentView();

    char* pData = nullptr;
    size_t nSize = 0;
    const int nOk
        = m_aProvider.getDataForMimeType(m_aProvider.pUserData, aWireMime.getStr(), &pData, &nSize);

    if (nSavedView >= 0 && KitHelper::getCurrentView() != nSavedView)
        KitHelper::setView(nSavedView);

    cpo::uno::Any aRet;
    if (nOk && pData)
    {
        if (bText)
            aRet <<= OUString(pData, nSize, RTL_TEXTENCODING_UTF8);
        else
            aRet <<= uno::Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(pData), nSize);
    }
    free(pData);

    m_aCache.emplace(rFlavor.MimeType, aRet);
    return aRet;
}

uno::Sequence<datatransfer::DataFlavor> SAL_CALL KitProviderTransferable::getTransferDataFlavors()
{
    return m_aFlavors;
}

bool SAL_CALL
KitProviderTransferable::isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor)
{
    return std::any_of(std::cbegin(m_aFlavors), std::cend(m_aFlavors),
                       [&rFlavor](const datatransfer::DataFlavor& i)
                       { return i.MimeType == rFlavor.MimeType && i.DataType == rFlavor.DataType; });
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_KitClipboard_get_implementation(css::uno::XComponentContext*,
                                        css::uno::Sequence<cpo::uno::Any> const& /*args*/)
{
    SolarMutexGuard aGuard;

    cppu::OWeakObject* pClipboard = KitClipboardFactory::getClipboardForCurView().get();

    pClipboard->acquire();
    return pClipboard;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
