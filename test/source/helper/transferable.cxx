/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/helper/transferable.hxx>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>

using namespace css;

namespace apitest
{
namespace helper
{
namespace transferable
{
OString OOO_DLLPUBLIC_TEST getTextSelection(
    const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable, OString mimeType)
{
    if (!xTransferable.is())
        return OString();

    // Take care of UTF-8 text here.
    bool bConvert = false;
    sal_Int32 nIndex = 0;
    if (mimeType.getToken(0, ';', nIndex) == "text/plain")
    {
        if (mimeType.getToken(0, ';', nIndex) == "charset=utf-8")
        {
            mimeType = "text/plain;charset=utf-16";
            bConvert = true;
        }
    }

    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(mimeType.getStr());
    if (mimeType == "text/plain;charset=utf-16")
        aFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        aFlavor.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();

    if (!xTransferable.is() || !xTransferable->isDataFlavorSupported(aFlavor))
        return OString();

    uno::Any aAny;
    try
    {
        aAny = xTransferable->getTransferData(aFlavor);
    }
    catch (const css::datatransfer::UnsupportedFlavorException&)
    {
        return OString();
    }
    catch (const css::uno::Exception&)
    {
        return OString();
    }

    OString aRet;
    if (aFlavor.DataType == cppu::UnoType<OUString>::get())
    {
        OUString aString;
        aAny >>= aString;
        if (bConvert)
            aRet = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
        else
            aRet = OString(reinterpret_cast<const sal_Char*>(aString.getStr()),
                           aString.getLength() * sizeof(sal_Unicode));
    }
    else
    {
        uno::Sequence<sal_Int8> aSequence;
        aAny >>= aSequence;
        aRet = OString(reinterpret_cast<sal_Char*>(aSequence.getArray()), aSequence.getLength());
    }
    return aRet;
}

} // namespace transferable
} // namespace helper
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
