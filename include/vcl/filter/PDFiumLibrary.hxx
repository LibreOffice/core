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

#include <vcl/dllapi.h>

#include <memory>
#include <rtl/instance.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <rtl/ustring.hxx>

#include <fpdf_doc.h>

namespace vcl::pdf
{
constexpr char constDictionaryKeyTitle[] = "T";
constexpr char constDictionaryKeyContents[] = "Contents";
constexpr char constDictionaryKeyPopup[] = "Popup";
constexpr char constDictionaryKeyModificationDate[] = "M";
class PDFiumDocument;

class VCL_DLLPUBLIC PDFium final
{
private:
    PDFium(const PDFium&) = delete;
    PDFium& operator=(const PDFium&) = delete;

    OUString maLastError;

public:
    PDFium();
    ~PDFium();

    OUString getLastError() { return maLastError; }

    std::unique_ptr<PDFiumDocument> openDocument(const void* pData, int nSize);
};

class VCL_DLLPUBLIC PDFiumAnnotation final
{
private:
    FPDF_ANNOTATION mpAnnotation;

    PDFiumAnnotation(const PDFiumAnnotation&) = delete;
    PDFiumAnnotation& operator=(const PDFiumAnnotation&) = delete;

public:
    PDFiumAnnotation(FPDF_ANNOTATION pAnnotation);
    ~PDFiumAnnotation();
    FPDF_ANNOTATION getPointer() { return mpAnnotation; }

    int getSubType();
    basegfx::B2DRectangle getRectangle();
    bool hasKey(OString const& rKey);
    OUString getString(OString const& rKey);
    std::unique_ptr<PDFiumAnnotation> getLinked(OString const& rKey);
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

    FPDF_PAGE getPointer() { return mpPage; }

    int getAnnotationCount();
    int getAnnotationIndex(std::unique_ptr<PDFiumAnnotation> const& rAnnotation);

    std::unique_ptr<PDFiumAnnotation> getAnnotation(int nIndex);
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

    // Page size in points
    basegfx::B2DSize getPageSize(int nIndex);
    int getPageCount();

    std::unique_ptr<PDFiumPage> openPage(int nIndex);
};

struct PDFiumLibrary final : public rtl::StaticWithInit<std::shared_ptr<PDFium>, PDFiumLibrary>
{
    std::shared_ptr<PDFium> operator()() { return std::make_shared<PDFium>(); }
};

// Tools

VCL_DLLPUBLIC OUString convertPdfDateToISO8601(OUString const& rInput);

} // namespace vcl::pdf

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
