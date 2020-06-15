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
#include <fpdf_edit.h>

namespace vcl::pdf
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

std::unique_ptr<PDFiumDocument> PDFium::openDocument(const void* pData, int nSize)
{
    maLastError = OUString();
    std::unique_ptr<PDFiumDocument> pPDFiumDocument;

    FPDF_DOCUMENT pDocument = FPDF_LoadMemDocument(pData, nSize, /*password=*/nullptr);

    if (!pDocument)
    {
        switch (FPDF_GetLastError())
        {
            case FPDF_ERR_SUCCESS:
                maLastError = "Success";
                break;
            case FPDF_ERR_UNKNOWN:
                maLastError = "Unknown error";
                break;
            case FPDF_ERR_FILE:
                maLastError = "File not found";
                break;
            case FPDF_ERR_FORMAT:
                maLastError = "Input is not a PDF format";
                break;
            case FPDF_ERR_PASSWORD:
                maLastError = "Incorrect password or password is required";
                break;
            case FPDF_ERR_SECURITY:
                maLastError = "Security error";
                break;
            case FPDF_ERR_PAGE:
                maLastError = "Content error";
                break;
            default:
                break;
        }
    }
    else
    {
        pPDFiumDocument = std::make_unique<PDFiumDocument>(pDocument);
    }

    return pPDFiumDocument;
}

PDFiumDocument::PDFiumDocument(FPDF_DOCUMENT pPdfDocument)
    : mpPdfDocument(pPdfDocument)
{
}

PDFiumDocument::~PDFiumDocument()
{
    if (mpPdfDocument)
        FPDF_CloseDocument(mpPdfDocument);
}

std::unique_ptr<PDFiumPage> PDFiumDocument::openPage(int nIndex)
{
    std::unique_ptr<PDFiumPage> pPDFiumPage;
    FPDF_PAGE pPage = FPDF_LoadPage(mpPdfDocument, nIndex);
    if (pPage)
    {
        pPDFiumPage = std::make_unique<PDFiumPage>(pPage);
    }
    return pPDFiumPage;
}

basegfx::B2DSize PDFiumDocument::getPageSize(int nIndex)
{
    basegfx::B2DSize aSize;
    FS_SIZEF aPDFSize;
    if (FPDF_GetPageSizeByIndexF(mpPdfDocument, nIndex, &aPDFSize))
    {
        aSize = basegfx::B2DSize(aPDFSize.width, aPDFSize.height);
    }
    return aSize;
}

int PDFiumDocument::getPageCount() { return FPDF_GetPageCount(mpPdfDocument); }

} // end vcl::pdf

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
