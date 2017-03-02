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
#ifdef WNT
#include <prewin.h>
#endif
#include <fpdfview.h>
#include <fpdf_edit.h>
#ifdef WNT
#include <postwin.h>
#endif
#endif

#include <vcl/bitmapaccess.hxx>

using namespace com::sun::star;

namespace
{

#if HAVE_FEATURE_PDFIUM

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
#else
bool generatePreview(SvStream& rStream, Graphic& rGraphic)
{
    (void)rStream;
    (void)rGraphic;

    return true;
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
    rStream.Seek(STREAM_SEEK_TO_END);
    uno::Sequence<sal_Int8> aPdfData(rStream.Tell());
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ReadBytes(aPdfData.getArray(), aPdfData.getLength());
    rGraphic.setPdfData(aPdfData);

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
