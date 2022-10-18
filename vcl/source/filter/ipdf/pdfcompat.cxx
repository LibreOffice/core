/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdf/pdfcompat.hxx>

#if HAVE_FEATURE_PDFIUM
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <tools/UnitConversion.hxx>
#endif

#include <vcl/filter/PDFiumLibrary.hxx>
#include <sal/log.hxx>

namespace vcl::pdf
{
#if HAVE_FEATURE_PDFIUM

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

    sal_Int32 nMajor = OString(aFirstBytes[5]).toInt32();
    sal_Int32 nMinor = OString(aFirstBytes[7]).toInt32();
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
            = pPdfium->openDocument(aInBuffer.GetData(), aInBuffer.GetSize());
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
#else
bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream)
{
    rInStream.Seek(STREAM_SEEK_TO_BEGIN);
    rOutStream.WriteStream(rInStream, STREAM_SEEK_TO_END);
    return rOutStream.good();
}
#endif // HAVE_FEATURE_PDFIUM

VectorGraphicDataArray createVectorGraphicDataArray(SvStream& rStream)
{
    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(rStream, aMemoryStream))
        return VectorGraphicDataArray();

    const sal_uInt32 nStreamLength = aMemoryStream.TellEnd();

    VectorGraphicDataArray aPdfData(nStreamLength);

    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aMemoryStream.ReadBytes(aPdfData.begin(), nStreamLength);
    if (aMemoryStream.GetError())
        return VectorGraphicDataArray();

    return aPdfData;
}

} // end vcl::filter::ipdf namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
