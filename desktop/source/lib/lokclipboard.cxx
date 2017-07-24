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
#include <vcl/graphicfilter.hxx>

using namespace com::sun::star;

uno::Reference<datatransfer::XTransferable> SAL_CALL LOKClipboard::getContents()
{
    return m_xTransferable;
}

void SAL_CALL LOKClipboard::setContents(const uno::Reference<datatransfer::XTransferable>& xTransferable,
                                        const uno::Reference<datatransfer::clipboard::XClipboardOwner>& /*xClipboardOwner*/)
{
    m_xTransferable = xTransferable;
}

OUString SAL_CALL LOKClipboard::getName()
{
    return OUString();
}

LOKTransferable::LOKTransferable(const char* pMimeType, const char* pData, std::size_t nSize)
    : m_aSequence(reinterpret_cast<const sal_Int8*>(pData), nSize)
{
    OString aMimeType(pMimeType);
    if (aMimeType.startsWith("image/png"))
    {
        AddFormat(SotClipboardFormatId::PNG);
    }
    else if (aMimeType.startsWith("image/jpeg"))
    {
        AddFormat(SotClipboardFormatId::JPEG);
    }
    else if (aMimeType.startsWith("text/html"))
    {
        AddFormat(SotClipboardFormatId::HTML);
    }
    else if (aMimeType.startsWith("text/rtf"))
    {
        AddFormat(SotClipboardFormatId::RTF);
    }
    else if (aMimeType.startsWith("text/richtext"))
    {
        AddFormat(SotClipboardFormatId::RICHTEXT);
    }
    else if (aMimeType.startsWith("text/plain"))
    {
        AddFormat(SotClipboardFormatId::STRING);
    }
}

void LOKTransferable::AddSupportedFormats()
{
}

bool LOKTransferable::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    bool bSuccess(false);
    sal_Char* pText = reinterpret_cast<sal_Char*>(m_aSequence.getArray());

    SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );

    switch( nFormat )
    {
    case SotClipboardFormatId::PNG:
    case SotClipboardFormatId::BITMAP:
    case SotClipboardFormatId::JPEG:
    {
        Graphic aGraphic;
        INetURLObject aURL(OUString(pText, m_aSequence.getLength(), RTL_TEXTENCODING_UTF8));
        if (GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL))
            bSuccess = SetBitmapEx(aGraphic.GetBitmapEx(), rFlavor);
    }
    break;

    case SotClipboardFormatId::RTF:
    case SotClipboardFormatId::RICHTEXT:
    case SotClipboardFormatId::HTML:
    case SotClipboardFormatId::STRING:
    {
        uno::Any aAny;
        aAny <<= m_aSequence;
        bSuccess = SetAny(aAny, rFlavor);
    }
    break;

    default: break;
    }

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
