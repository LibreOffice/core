/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM

#include <vcl/filter/PDFiumLibrary.hxx>
#include <fpdf_doc.h>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>

namespace vcl
{
namespace pdf
{
PDFium::PDFium()
{
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);
}

PDFium::~PDFium() { FPDF_DestroyLibrary(); }

BitmapChecksum PDFiumPage::getChecksum(int nMDPPerm)
{
    size_t nPageWidth = FPDF_GetPageWidth(mpPage);
    size_t nPageHeight = FPDF_GetPageHeight(mpPage);
    FPDF_BITMAP pPdfBitmap = FPDFBitmap_Create(nPageWidth, nPageHeight, /*alpha=*/1);
    if (!pPdfBitmap)
    {
        return 0;
    }

    int nFlags = 0;
    if (nMDPPerm != 3)
    {
        // Annotations/commenting should affect the checksum, signature verification wants this.
        nFlags = FPDF_ANNOT;
    }
    FPDF_RenderPageBitmap(pPdfBitmap, mpPage, /*start_x=*/0, /*start_y=*/0, nPageWidth, nPageHeight,
                          /*rotate=*/0, nFlags);
    Bitmap aBitmap(Size(nPageWidth, nPageHeight), 24);
    {
        Bitmap::ScopedWriteAccess pWriteAccess(aBitmap);
        const auto pPdfBuffer = static_cast<const sal_uInt8*>(FPDFBitmap_GetBuffer(pPdfBitmap));
        const int nStride = FPDFBitmap_GetStride(pPdfBitmap);
        for (size_t nRow = 0; nRow < nPageHeight; ++nRow)
        {
            const sal_uInt8* pPdfLine = pPdfBuffer + (nStride * nRow);
            pWriteAccess->CopyScanline(nRow, pPdfLine, ScanlineFormat::N32BitTcBgra, nStride);
        }
    }
    return aBitmap.GetChecksum();
}

} // end pdf
} // end vcl

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
