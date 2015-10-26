/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <lokclipboard.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

uno::Reference<datatransfer::XTransferable> SAL_CALL LOKClipboard::getContents()
throw (uno::RuntimeException, std::exception)
{
    return m_xTransferable;
}

void SAL_CALL LOKClipboard::setContents(const uno::Reference<datatransfer::XTransferable>& xTransferable,
                                        const uno::Reference<datatransfer::clipboard::XClipboardOwner>& /*xClipboardOwner*/)
throw (uno::RuntimeException, std::exception)
{
    m_xTransferable = xTransferable;
}

OUString SAL_CALL LOKClipboard::getName() throw (uno::RuntimeException, std::exception)
{
    return OUString();
}

LOKTransferable::LOKTransferable(const char* pMimeType, const char* pData, size_t nSize)
    : m_aMimeType(pMimeType),
      m_aSequence(reinterpret_cast<const sal_Int8*>(pData), nSize)
{
}

uno::Any SAL_CALL LOKTransferable::getTransferData(const datatransfer::DataFlavor& rFlavor)
throw(datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if (rFlavor.DataType == cppu::UnoType<OUString>::get())
    {
        sal_Char* pText = reinterpret_cast<sal_Char*>(m_aSequence.getArray());
        aRet <<= OUString(pText, m_aSequence.getLength(), RTL_TEXTENCODING_UTF8);
    }
    else
        aRet <<= m_aSequence;
    return aRet;
}

std::vector<datatransfer::DataFlavor> LOKTransferable::getTransferDataFlavorsAsVector()
{
    std::vector<datatransfer::DataFlavor> aRet;
    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(m_aMimeType.getStr());
    aFlavor.DataType = cppu::UnoType< uno::Sequence<sal_Int8> >::get();

    sal_Int32 nIndex(0);
    if (m_aMimeType.getToken(0, ';', nIndex) == "text/plain")
    {
        if (m_aMimeType.getToken(0, ';', nIndex) != "charset=utf-16")
            aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
    }
    aRet.push_back(aFlavor);

    return aRet;
}

uno::Sequence<datatransfer::DataFlavor> SAL_CALL LOKTransferable::getTransferDataFlavors()
throw(uno::RuntimeException, std::exception)
{
    return comphelper::containerToSequence(getTransferDataFlavorsAsVector());
}

sal_Bool SAL_CALL LOKTransferable::isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor)
throw(uno::RuntimeException, std::exception)
{
    const std::vector<datatransfer::DataFlavor> aFlavors = getTransferDataFlavorsAsVector();
    return std::find_if(aFlavors.begin(), aFlavors.end(), [&rFlavor](const datatransfer::DataFlavor& i)
    {
        return i.MimeType == rFlavor.MimeType && i.DataType == rFlavor.DataType;
    }) != aFlavors.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
