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
#include <vcl/filter/PDFiumLibrary.hxx>
#include <sal/log.hxx>

namespace vcl::pdf
{
/// Decide if PDF data is old enough to be compatible.
bool isCompatible(SvStream& rInStream, sal_uInt64 nPos, sal_uInt64 nSize)
{
    if (nSize < 8)
        return false;

    // %PDF-x.y
    sal_uInt8 aFirstBytes[8];
    rInStream.Seek(nPos);
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
bool convertToHighestSupported(SvStream& rInStream, SvStream& rOutStream)
{
    sal_uInt64 nPos = STREAM_SEEK_TO_BEGIN;
    sal_uInt64 nSize = STREAM_SEEK_TO_END;
    rInStream.Seek(nPos);
    // Convert to PDF-1.6.
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return false;

    // Read input into a buffer.
    SvMemoryStream aInBuffer;
    aInBuffer.WriteStream(rInStream, nSize);

    SvMemoryStream aSaved;
    {
        // Load the buffer using pdfium.
        std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
            = pPdfium->openDocument(aInBuffer.GetData(), aInBuffer.GetSize(), OString());
        if (!pPdfDocument)
            return false;

        // 16 means PDF-1.6.
        if (!pPdfDocument->saveWithVersion(aSaved, 16))
            return false;
    }

    aSaved.Seek(STREAM_SEEK_TO_BEGIN);
    rOutStream.WriteStream(aSaved);

    return rOutStream.good();
}

/// Takes care of transparently downgrading the version of the PDF stream in
/// case it's too new for our PDF export.
bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream)
{
    sal_uInt64 nPos = STREAM_SEEK_TO_BEGIN;
    sal_uInt64 nSize = STREAM_SEEK_TO_END;
    bool bCompatible = isCompatible(rInStream, nPos, nSize);
    rInStream.Seek(nPos);
    if (bCompatible)
        // Not converting.
        rOutStream.WriteStream(rInStream, nSize);
    else
        convertToHighestSupported(rInStream, rOutStream);

    return rOutStream.good();
}

BinaryDataContainer createBinaryDataContainer(SvStream& rStream)
{
    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(rStream, aMemoryStream))
        return {};

    const sal_uInt32 nStreamLength = aMemoryStream.TellEnd();

    auto aPdfData = std::make_unique<std::vector<sal_uInt8>>(nStreamLength);

    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aMemoryStream.ReadBytes(aPdfData->data(), aPdfData->size());
    if (aMemoryStream.GetError())
        return {};

    return { std::move(aPdfData) };
}

} // end vcl::filter::ipdf namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
