/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdf/pdfcompat.hxx>

#include <o3tl/string_view.hxx>
#include <tools/solar.h>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdf/pwdinteract.hxx>
#include <sal/log.hxx>

namespace vcl::pdf
{
/// Decide if PDF data is old enough to be compatible.
bool isCompatible(SvStream& rInStream)
{
    // %PDF-x.y
    sal_uInt8 aFirstBytes[8];
    rInStream.Seek(STREAM_SEEK_TO_BEGIN);
    sal_uLong nRead = rInStream.ReadBytes(aFirstBytes, 8);
    if (nRead < 8)
        return false;

    if (aFirstBytes[0] != '%' || aFirstBytes[1] != 'P' || aFirstBytes[2] != 'D'
        || aFirstBytes[3] != 'F' || aFirstBytes[4] != '-')
        return false;

    sal_Int32 nMajor = o3tl::toInt32(std::string_view(reinterpret_cast<char*>(&aFirstBytes[5]), 1));
    sal_Int32 nMinor = o3tl::toInt32(std::string_view(reinterpret_cast<char*>(&aFirstBytes[7]), 1));
    return !(nMajor > 1 || (nMajor == 1 && nMinor > 6));
}

/// Converts to highest supported format version (1.6).
/// Usually used to deal with missing referenced objects in source
/// pdf stream.
/// The conversion takes place if either the stream is encrypted, or 'bForce' is true
bool convertToHighestSupported(
    SvStream& rInStream, SvStream& rOutStream,
    const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler, bool bForce,
    bool& bEncrypted)
{
    sal_uInt64 nPos = STREAM_SEEK_TO_BEGIN;
    sal_uInt64 nSize = STREAM_SEEK_TO_END;
    rInStream.Seek(nPos);
    bEncrypted = false;
    // Convert to PDF-1.6.
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return false;

    // Read input into a buffer.
    SvMemoryStream aInBuffer;
    aInBuffer.WriteStream(rInStream, nSize);

    SvMemoryStream aSaved;
    bool bAgain = false;
    OUString aPassword;
    do
    {
        // Load the buffer using pdfium.
        OString aIsoPwd = OUStringToOString(aPassword, RTL_TEXTENCODING_ISO_8859_1);

        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPdfium->openDocument(aInBuffer.GetData(), aInBuffer.GetSize(), aIsoPwd);
        auto nPdfiumErr = pPdfium->getLastErrorCode();
        if (!pPdfDocument && nPdfiumErr != vcl::pdf::PDFErrorType::Password)
        {
            SAL_WARN("vcl.filter",
                     "convertToHighestSupported pdfium err: " << pPdfium->getLastError());
            return false;
        }

        if (!pPdfDocument && nPdfiumErr == vcl::pdf::PDFErrorType::Password)
        {
            if (!xInteractionHandler || !xInteractionHandler.is())
            {
                SAL_WARN("vcl.filter", "convertToHighestSupported no Int handler for pass");
                return false;
            }

            // We don't have a filename for the GUI here
            bEncrypted = true;
            bAgain = vcl::pdf::getPassword(xInteractionHandler, aPassword, !bAgain, u"PDF"_ustr);
            SAL_INFO("vcl.filter", "convertToHighestSupported pass result: " << bAgain);
            if (!bAgain)
            {
                SAL_WARN("vcl.filter", "convertToHighestSupported Failed to get pass");
                return false;
            }
            continue;
        }
        bAgain = false;

        SAL_INFO("vcl.filter", "convertToHighestSupported do save");
        // 16 means PDF-1.6.
        // true means 'remove security' - i.e. not passworded
        if (!pPdfDocument->saveWithVersion(aSaved, 16, true))
            return false;
    } while (bAgain);

    aSaved.Seek(STREAM_SEEK_TO_BEGIN);
    if (bEncrypted || bForce)
    {
        SAL_INFO("vcl.filter", "convertToHighestSupported do write");
        rOutStream.WriteStream(aSaved);
    }

    SAL_INFO("vcl.filter",
             "convertToHighestSupported exit: encrypted: " << bEncrypted << " force: " << bForce);
    return rOutStream.good();
}

/// Takes care of transparently downgrading the version of the PDF stream in
/// case it's too new for our PDF export.
bool getCompatibleStream(
    SvStream& rInStream, SvStream& rOutStream,
    const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler,
    bool& bEncrypted)
{
    bool bCompatible = isCompatible(rInStream);

    // This will convert if either the file is encrypted, or !bCompatible
    convertToHighestSupported(rInStream, rOutStream, xInteractionHandler, !bCompatible, bEncrypted);
    rInStream.Seek(STREAM_SEEK_TO_BEGIN);
    if (bCompatible && !bEncrypted)
        // Just pass the original through
        rOutStream.WriteStream(rInStream, STREAM_SEEK_TO_END);

    return rOutStream.good();
}

BinaryDataContainer createBinaryDataContainer(
    SvStream& rStream,
    const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler)
{
    bool bEncrypted;
    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(rStream, aMemoryStream, xInteractionHandler, bEncrypted))
        return {};

    const sal_uInt64 nStreamLength = aMemoryStream.TellEnd();

    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    BinaryDataContainer aPdfData(aMemoryStream, nStreamLength);
    if (aMemoryStream.GetError())
        return {};

    return aPdfData;
}

} // end vcl::filter::ipdf namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
