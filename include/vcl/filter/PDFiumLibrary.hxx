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
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <vcl/checksum.hxx>
#include <vcl/pdf/PDFAnnotationSubType.hxx>

#include <fpdf_doc.h>

namespace vcl::pdf
{
constexpr char constDictionaryKeyTitle[] = "T";
constexpr char constDictionaryKeyContents[] = "Contents";
constexpr char constDictionaryKeyPopup[] = "Popup";
constexpr char constDictionaryKeyModificationDate[] = "M";

class PDFiumDocument;
class PDFiumPageObject;

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

    PDFAnnotationSubType getSubType();
    basegfx::B2DRectangle getRectangle();
    bool hasKey(OString const& rKey);
    int getValueType(OString const& rKey);
    OUString getString(OString const& rKey);
    std::unique_ptr<PDFiumAnnotation> getLinked(OString const& rKey);
    int getObjectCount();
    std::unique_ptr<PDFiumPageObject> getObject(int nIndex);
};

class PDFiumTextPage;

class VCL_DLLPUBLIC PDFiumPathSegment final
{
private:
    FPDF_PATHSEGMENT mpPathSegment;

    PDFiumPathSegment(const PDFiumPathSegment&) = delete;
    PDFiumPathSegment& operator=(const PDFiumPathSegment&) = delete;

public:
    PDFiumPathSegment(FPDF_PATHSEGMENT pPathSegment);
    ~PDFiumPathSegment();

    FPDF_PATHSEGMENT getPointer() { return mpPathSegment; }
    basegfx::B2DPoint getPoint();
    bool isClosed();
    int getType();
};

class VCL_DLLPUBLIC PDFiumPageObject final
{
private:
    FPDF_PAGEOBJECT mpPageObject;

    PDFiumPageObject(const PDFiumPageObject&) = delete;
    PDFiumPageObject& operator=(const PDFiumPageObject&) = delete;

public:
    PDFiumPageObject(FPDF_PAGEOBJECT pPageObject);
    ~PDFiumPageObject();

    FPDF_PAGEOBJECT getPointer() { return mpPageObject; }

    int getType();
    OUString getText(std::unique_ptr<PDFiumTextPage> const& pTextPage);

    int getFormObjectCount();
    std::unique_ptr<PDFiumPageObject> getFormObject(int nIndex);

    basegfx::B2DHomMatrix getMatrix();
    basegfx::B2DRectangle getBounds();
    double getFontSize();
    OUString getFontName();
    int getTextRenderMode();
    Color getFillColor();
    Color getStrokeColor();
    // Path
    int getPathSegmentCount();
    std::unique_ptr<PDFiumPathSegment> getPathSegment(int index);
};

class VCL_DLLPUBLIC PDFiumTextPage final
{
private:
    FPDF_TEXTPAGE mpTextPage;

    PDFiumTextPage(const PDFiumTextPage&) = delete;
    PDFiumTextPage& operator=(const PDFiumTextPage&) = delete;

public:
    PDFiumTextPage(FPDF_TEXTPAGE pTextPage);
    ~PDFiumTextPage();

    FPDF_TEXTPAGE getPointer() { return mpTextPage; }

    int countChars();
    unsigned int getUnicode(int index);
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

    int getObjectCount();
    std::unique_ptr<PDFiumPageObject> getObject(int nIndex);

    int getAnnotationCount();
    int getAnnotationIndex(std::unique_ptr<PDFiumAnnotation> const& rAnnotation);

    std::unique_ptr<PDFiumAnnotation> getAnnotation(int nIndex);

    std::unique_ptr<PDFiumTextPage> getTextPage();

    /// Get bitmap checksum of the page, without annotations/commenting.
    BitmapChecksum getChecksum();

    double getWidth();
    double getHeight();
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

    FPDF_DOCUMENT getPointer() { return mpPdfDocument; }
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
