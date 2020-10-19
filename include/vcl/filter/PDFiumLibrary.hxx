/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM

#include <memory>
#include <rtl/instance.hxx>
#include <vcl/dllapi.h>
#include <vcl/checksum.hxx>

#include <fpdf_doc.h>

namespace vcl::pdf
{
class PDFiumDocument;

class VCL_DLLPUBLIC PDFium final
{
private:
    PDFium(const PDFium&) = delete;
    PDFium& operator=(const PDFium&) = delete;

public:
    PDFium();
    ~PDFium();
};

class VCL_DLLPUBLIC PDFiumPage final
{
private:
    FPDF_PAGE mpPage;

private:
    PDFiumPage(const PDFiumPage&) = delete;
    PDFiumPage& operator=(const PDFiumPage&) = delete;

public:
    PDFiumPage(FPDF_PAGE pPage)
        : mpPage(pPage)
    {
    }

    ~PDFiumPage()
    {
        if (mpPage)
            FPDF_ClosePage(mpPage);
    }

    /// Get bitmap checksum of the page, without annotations/commenting.
    BitmapChecksum getChecksum(int nMDPPerm);
};

class VCL_DLLPUBLIC PDFiumDocument final
{
private:
    FPDF_DOCUMENT mpPdfDocument;

private:
    PDFiumDocument(const PDFiumDocument&) = delete;
    PDFiumDocument& operator=(const PDFiumDocument&) = delete;

public:
    PDFiumDocument(FPDF_DOCUMENT pPdfDocument);
    ~PDFiumDocument();

    int getPageCount();

    std::unique_ptr<PDFiumPage> openPage(int nIndex);
};

struct PDFiumLibrary : public rtl::StaticWithInit<std::shared_ptr<PDFium>, PDFiumLibrary>
{
    std::shared_ptr<PDFium> operator()() { return std::make_shared<PDFium>(); }
};

} // namespace vcl::pdf

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
