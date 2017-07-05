/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pdfread.hxx"

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_save.h>
#endif

#include <vcl/bitmapaccess.hxx>

using namespace com::sun::star;

namespace
{

#if HAVE_FEATURE_PDFIUM

/// Callback class to be used with FPDF_SaveWithVersion().
struct CompatibleWriter : public FPDF_FILEWRITE
{
public:
    CompatibleWriter();
    static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite, const void* pData, unsigned long nSize);

    SvMemoryStream m_aStream;
};

CompatibleWriter::CompatibleWriter()
{
    FPDF_FILEWRITE::version = 1;
    FPDF_FILEWRITE::WriteBlock = CompatibleWriter::WriteBlockCallback;
}

int CompatibleWriter::WriteBlockCallback(FPDF_FILEWRITE* pFileWrite, const void* pData, unsigned long nSize)
{
    auto pImpl = static_cast<CompatibleWriter*>(pFileWrite);
    pImpl->m_aStream.WriteBytes(pData, nSize);
    return 1;
}

/// Convert to inch, then assume 96 DPI.
double pointToPixel(double fPoint)
{
    return fPoint / 72 * 96;
}

/// Does PDF to bitmap conversion using pdfium.
bool generatePreview(SvStream& rStream, Graphic& rGraphic)
{
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);

    // Read input into a buffer.
    SvMemoryStream aInBuffer;
    aInBuffer.WriteStream(rStream);

    // Load the buffer using pdfium.
    FPDF_DOCUMENT pPdfDocument = FPDF_LoadMemDocument(aInBuffer.GetData(), aInBuffer.GetSize(), /*password=*/nullptr);
    if (!pPdfDocument)
        return false;

    // Render the first page.
    FPDF_PAGE pPdfPage = FPDF_LoadPage(pPdfDocument, /*page_index=*/0);
    if (!pPdfPage)
        return false;

    // Returned unit is points, convert that to pixel.
    size_t nPageWidth = pointToPixel(FPDF_GetPageWidth(pPdfPage));
    size_t nPageHeight = pointToPixel(FPDF_GetPageHeight(pPdfPage));
    FPDF_BITMAP pPdfBitmap = FPDFBitmap_Create(nPageWidth, nPageHeight, /*alpha=*/1);
    if (!pPdfBitmap)
        return false;

    FPDF_DWORD nColor = FPDFPage_HasTransparency(pPdfPage) ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(pPdfBitmap, 0, 0, nPageWidth, nPageHeight, nColor);
    FPDF_RenderPageBitmap(pPdfBitmap, pPdfPage, /*start_x=*/0, /*start_y=*/0, nPageWidth, nPageHeight, /*rotate=*/0, /*flags=*/0);

    // Save the buffer as a bitmap.
    Bitmap aBitmap(Size(nPageWidth, nPageHeight), 24);
    {
        Bitmap::ScopedWriteAccess pWriteAccess(aBitmap);
        auto pPdfBuffer = static_cast<ConstScanline>(FPDFBitmap_GetBuffer(pPdfBitmap));
        for (size_t nRow = 0; nRow < nPageHeight; ++nRow)
        {
            int nStride = FPDFBitmap_GetStride(pPdfBitmap);
            ConstScanline pPdfLine = pPdfBuffer + (nStride * nRow);
            // pdfium byte order is BGRA.
            pWriteAccess->CopyScanline(nRow, pPdfLine, ScanlineFormat::N32BitTcBgra, nStride);
        }
    }
    rGraphic = aBitmap;

    FPDFBitmap_Destroy(pPdfBitmap);
    FPDF_ClosePage(pPdfPage);
    FPDF_CloseDocument(pPdfDocument);
    FPDF_DestroyLibrary();

    return true;
}

/// Decide if PDF data is old enough to be compatible.
bool isCompatible(SvStream& rInStream)
{
    // %PDF-x.y
    sal_uInt8 aFirstBytes[8];
    rInStream.Seek(STREAM_SEEK_TO_BEGIN);
    sal_uLong nRead = rInStream.ReadBytes(aFirstBytes, 8);
    if (nRead < 8)
        return false;

    if (aFirstBytes[0] != '%' || aFirstBytes[1] != 'P' || aFirstBytes[2] != 'D' || aFirstBytes[3] != 'F' || aFirstBytes[4] != '-')
        return false;

    sal_Int32 nMajor = OString(aFirstBytes[5]).toInt32();
    sal_Int32 nMinor = OString(aFirstBytes[7]).toInt32();
    if (nMajor > 1 || (nMajor == 1 && nMinor > 4))
        return false;

    return true;
}

/// Takes care of transparently downgrading the version of the PDF stream in
/// case it's too new for our PDF export.
bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream)
{
    bool bCompatible = isCompatible(rInStream);
    rInStream.Seek(STREAM_SEEK_TO_BEGIN);
    if (bCompatible)
        // Not converting.
        rOutStream.WriteStream(rInStream);
    else
    {
        // Downconvert to PDF-1.4.
        FPDF_LIBRARY_CONFIG aConfig;
        aConfig.version = 2;
        aConfig.m_pUserFontPaths = nullptr;
        aConfig.m_pIsolate = nullptr;
        aConfig.m_v8EmbedderSlot = 0;
        FPDF_InitLibraryWithConfig(&aConfig);

        // Read input into a buffer.
        SvMemoryStream aInBuffer;
        aInBuffer.WriteStream(rInStream);

        // Load the buffer using pdfium.
        FPDF_DOCUMENT pPdfDocument = FPDF_LoadMemDocument(aInBuffer.GetData(), aInBuffer.GetSize(), /*password=*/nullptr);
        if (!pPdfDocument)
            return false;

        CompatibleWriter aWriter;
        // 14 means PDF-1.4.
        if (!FPDF_SaveWithVersion(pPdfDocument, &aWriter, 0, 14))
            return false;

        FPDF_CloseDocument(pPdfDocument);
        FPDF_DestroyLibrary();

        aWriter.m_aStream.Seek(STREAM_SEEK_TO_BEGIN);
        rOutStream.WriteStream(aWriter.m_aStream);
    }

    return rOutStream.good();
}
#else
bool generatePreview(SvStream& rStream, Graphic& rGraphic)
{
    (void)rStream;
    (void)rGraphic;

    return true;
}

bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream)
{
    rInStream.Seek(STREAM_SEEK_TO_BEGIN);
    rOutStream.WriteStream(rInStream);
    return rOutStream.good();
}
#endif // HAVE_FEATURE_PDFIUM

}

namespace vcl
{

bool ImportPDF(SvStream& rStream, Graphic& rGraphic)
{
    // Get the preview of the first page.
    if (!generatePreview(rStream, rGraphic))
        return false;

    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(rStream, aMemoryStream))
        return false;

    aMemoryStream.Seek(STREAM_SEEK_TO_END);
    uno::Sequence<sal_Int8> aPdfData(aMemoryStream.Tell());
    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aMemoryStream.ReadBytes(aPdfData.getArray(), aPdfData.getLength());
    rGraphic.setPdfData(aPdfData);

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
