/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "lokclipboard.hxx"
#include <unordered_map>
#include <vcl/lazydelete.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/lokhelper.hxx>
#include <sal/log.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace css;
using namespace css::uno;

/* static */ osl::Mutex LOKClipboardFactory::gMutex;
static vcl::DeleteOnDeinit<std::unordered_map<int, rtl::Reference<LOKClipboard>>>
gClipboards(new std::unordered_map<int, rtl::Reference<LOKClipboard>>);

rtl::Reference<LOKClipboard> LOKClipboardFactory::getClipboardForCurView()
{
    int nViewId = SfxLokHelper::getView(); // currently active.

    osl::MutexGuard aGuard(gMutex);

    auto it = gClipboards.get()->find(nViewId);
    if (it != gClipboards.get()->end())
    {
        SAL_INFO("lok", "Got clip: " << it->second.get() << " from " << nViewId);
        return it->second;
    }
    rtl::Reference<LOKClipboard> xClip(new LOKClipboard());
    (*gClipboards.get())[nViewId] = xClip;
    SAL_INFO("lok", "Created clip: " << xClip.get() << " for viewId " << nViewId);
    return xClip;
}

void LOKClipboardFactory::releaseClipboardForView(int nViewId)
{
    osl::MutexGuard aGuard(gMutex);

    if (nViewId < 0) // clear all
    {
        gClipboards.get()->clear();
        SAL_INFO("lok", "Released all clipboards on doc destroy\n");
    }
    else if (gClipboards.get())
    {
        auto it = gClipboards.get()->find(nViewId);
        if (it != gClipboards.get()->end())
        {
            SAL_INFO("lok", "Releasing clip: " << it->second.get() << " for destroyed " << nViewId);
            gClipboards.get()->erase(it);
        }
    }
}

uno::Reference<uno::XInterface>
    SAL_CALL LOKClipboardFactory::createInstanceWithArguments(const Sequence<Any>& /* rArgs */)
{
    return uno::Reference<uno::XInterface>(
        static_cast<cppu::OWeakObject*>(getClipboardForCurView().get()));
}

LOKClipboard::LOKClipboard()
    : cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                    css::lang::XServiceInfo>(m_aMutex)
{
    // Encourage 'paste' menu items to always show up.
    uno::Reference<datatransfer::XTransferable> xTransferable(new LOKTransferable());
    setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());
}

Sequence<OUString> LOKClipboard::getSupportedServiceNames_static()
{
    Sequence<OUString> aRet{ "com.sun.star.datatransfer.clipboard.LokClipboard" };
    return aRet;
}

OUString LOKClipboard::getImplementationName() { return "com.sun.star.datatransfer.LOKClipboard"; }

Sequence<OUString> LOKClipboard::getSupportedServiceNames()
{
    return getSupportedServiceNames_static();
}

sal_Bool LOKClipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Reference<css::datatransfer::XTransferable> LOKClipboard::getContents() { return m_xTransferable; }

void LOKClipboard::setContents(
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
    SAL_INFO("lok", "Clip: " << this << " set contents to " << m_xTransferable);

    aGuard.clear();

    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership(this, xOldContents);
    for (auto const& listener : aListeners)
    {
        listener->changedContents(aEv);
    }
}

void LOKClipboard::addClipboardListener(
    const Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    m_aListeners.push_back(listener);
}

void LOKClipboard::removeClipboardListener(
    const Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener),
                       m_aListeners.end());
}
LOKTransferable::LOKTransferable(const OUString& sMimeType,
                                 const css::uno::Sequence<sal_Int8>& aSequence)
{
    m_aContent.reserve(1);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(1);
    initFlavourFromMime(m_aFlavors[0], sMimeType);

    uno::Any aContent;
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
LOKTransferable::LOKTransferable()
{
    m_aContent.reserve(1);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(1);
    initFlavourFromMime(m_aFlavors[0], "text/plain");
    uno::Any aContent;
    aContent <<= OUString();
    m_aContent.push_back(aContent);
}

// cf. sot/source/base/exchange.cxx for these two exceptional types.
void LOKTransferable::initFlavourFromMime(css::datatransfer::DataFlavor& rFlavor,
                                          OUString aMimeType)
{
    if (aMimeType.startsWith("text/plain"))
    {
        aMimeType = "text/plain;charset=utf-16";
        rFlavor.DataType = cppu::UnoType<OUString>::get();
    }
    else if (aMimeType == "application/x-libreoffice-tsvc")
        rFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        rFlavor.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();
    rFlavor.MimeType = aMimeType;
    rFlavor.HumanPresentableName = aMimeType;
}

LOKTransferable::LOKTransferable(const size_t nInCount, const char** pInMimeTypes,
                                 const size_t* pInSizes, const char** pInStreams)
{
    m_aContent.reserve(nInCount);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(nInCount);
    for (size_t i = 0; i < nInCount; ++i)
    {
        initFlavourFromMime(m_aFlavors[i], OUString::fromUtf8(pInMimeTypes[i]));

        uno::Any aContent;
        if (m_aFlavors[i].DataType == cppu::UnoType<OUString>::get())
            aContent <<= OUString(pInStreams[i], pInSizes[i], RTL_TEXTENCODING_UTF8);
        else
            aContent <<= css::uno::Sequence<sal_Int8>(
                reinterpret_cast<const sal_Int8*>(pInStreams[i]), pInSizes[i]);
        m_aContent.push_back(aContent);
    }
}

uno::Any SAL_CALL LOKTransferable::getTransferData(const datatransfer::DataFlavor& rFlavor)
{
    assert(m_aContent.size() == static_cast<size_t>(m_aFlavors.getLength()));
    for (size_t i = 0; i < m_aContent.size(); ++i)
    {
        if (m_aFlavors[i].MimeType == rFlavor.MimeType)
        {
            if (m_aFlavors[i].DataType != rFlavor.DataType)
                SAL_WARN("lok", "Horror type mismatch!");
            return m_aContent[i];
        }
    }
    return uno::Any();
}

uno::Sequence<datatransfer::DataFlavor> SAL_CALL LOKTransferable::getTransferDataFlavors()
{
    return m_aFlavors;
}

sal_Bool SAL_CALL LOKTransferable::isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor)
{
    return std::find_if(m_aFlavors.begin(), m_aFlavors.end(),
                        [&rFlavor](const datatransfer::DataFlavor& i) {
                            return i.MimeType == rFlavor.MimeType && i.DataType == rFlavor.DataType;
                        })
           != m_aFlavors.end();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_LOKClipboard_get_implementation(css::uno::XComponentContext*,
                                        css::uno::Sequence<css::uno::Any> const& /*args*/)
{
    SolarMutexGuard aGuard;

    auto pClipboard
        = static_cast<cppu::OWeakObject*>(LOKClipboardFactory::getClipboardForCurView().get());

    pClipboard->acquire();
    return pClipboard;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
