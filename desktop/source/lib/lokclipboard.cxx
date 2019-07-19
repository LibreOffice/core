/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "lokclipboard.hxx"
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

uno::Reference<datatransfer::XTransferable> SAL_CALL LOKClipboard::getContents()
{
    return m_xTransferable;
}

void SAL_CALL LOKClipboard::setContents(
    const uno::Reference<datatransfer::XTransferable>& xTransferable,
    const uno::Reference<datatransfer::clipboard::XClipboardOwner>& /*xClipboardOwner*/)
{
    m_xTransferable = xTransferable;
}

OUString SAL_CALL LOKClipboard::getName() { return OUString(); }

LOKTransferable::LOKTransferable(const char* pMimeType, const char* pData, std::size_t nSize)
    : m_aMimeType(OUString::fromUtf8(pMimeType))
    , m_aSequence(reinterpret_cast<const sal_Int8*>(pData), nSize)
{
}

LOKTransferable::LOKTransferable(OUString sMimeType, const css::uno::Sequence<sal_Int8>& aSequence)
    : m_aMimeType(std::move(sMimeType))
    , m_aSequence(aSequence)
{
}

LOKTransferable::LOKTransferable(const size_t nInCount, const char** pInMimeTypes,
                                 const size_t* pInSizes, const char** pInStreams)
{
    m_aContent.reserve(nInCount);
    m_aFlavors = css::uno::Sequence<css::datatransfer::DataFlavor>(nInCount);
    for (size_t i = 0; i < nInCount; ++i)
    {
        OUString aMimeType = OUString::fromUtf8(pInMimeTypes[i]);

        // cf. sot/source/base/exchange.cxx for these two exceptional types.
        if (aMimeType.startsWith("text/plain"))
        {
            aMimeType = "text/plain;charset=utf-16";
            m_aFlavors[i].DataType = cppu::UnoType<OUString>::get();
        }
        else if (aMimeType == "application/x-libreoffice-tsvc")
            m_aFlavors[i].DataType = cppu::UnoType<OUString>::get();
        else
            m_aFlavors[i].DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();
        m_aFlavors[i].MimeType = aMimeType;
        m_aFlavors[i].HumanPresentableName = aMimeType;

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
    assert(m_aContent.size() == m_aFlavors.getLength());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
