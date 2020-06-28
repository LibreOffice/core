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
#include <fpdf_annot.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>

namespace vcl::pdf
{
OUString convertPdfDateToISO8601(OUString const& rInput)
{
    if (rInput.getLength() < 6)
        return OUString();

    OUString prefix = rInput.copy(0, 2);
    if (prefix != "D:")
        return OUString();

    OUString sYear = rInput.copy(2, 4);

    OUString sMonth("01");
    if (rInput.getLength() >= 8)
        sMonth = rInput.copy(6, 2);

    OUString sDay("01");
    if (rInput.getLength() >= 10)
        sDay = rInput.copy(8, 2);

    OUString sHours("00");
    if (rInput.getLength() >= 12)
        sHours = rInput.copy(10, 2);

    OUString sMinutes("00");
    if (rInput.getLength() >= 14)
        sMinutes = rInput.copy(12, 2);

    OUString sSeconds("00");
    if (rInput.getLength() >= 16)
        sSeconds = rInput.copy(14, 2);

    OUString sTimeZoneMark("Z");
    if (rInput.getLength() >= 17)
        sTimeZoneMark = rInput.copy(16, 1);

    OUString sTimeZoneHours("00");
    OUString sTimeZoneMinutes("00");
    if ((sTimeZoneMark == "+" || sTimeZoneMark == "-") && rInput.getLength() >= 22)
    {
        OUString sTimeZoneSeparator = rInput.copy(19, 1);
        if (sTimeZoneSeparator == "'")
        {
            sTimeZoneHours = rInput.copy(17, 2);
            sTimeZoneMinutes = rInput.copy(20, 2);
        }
    }

    OUString sTimeZoneString;
    if (sTimeZoneMark == "+" || sTimeZoneString == "-")
        sTimeZoneString = sTimeZoneMark + sTimeZoneHours + ":" + sTimeZoneMinutes;
    else if (sTimeZoneMark == "Z")
        sTimeZoneString = sTimeZoneMark;

    return sYear + "-" + sMonth + "-" + sDay + "T" + sHours + ":" + sMinutes + ":" + sSeconds
           + sTimeZoneString;
}

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

int PDFiumPage::getObjectCount() { return FPDFPage_CountObjects(mpPage); }

std::unique_ptr<PDFiumPageObject> PDFiumPage::getObject(int nIndex)
{
    std::unique_ptr<PDFiumPageObject> pPDFiumPageObject;
    FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(mpPage, nIndex);
    if (pPageObject)
    {
        pPDFiumPageObject = std::make_unique<PDFiumPageObject>(pPageObject);
    }
    return pPDFiumPageObject;
}

int PDFiumPage::getAnnotationCount() { return FPDFPage_GetAnnotCount(mpPage); }

int PDFiumPage::getAnnotationIndex(std::unique_ptr<PDFiumAnnotation> const& rAnnotation)
{
    return FPDFPage_GetAnnotIndex(mpPage, rAnnotation->getPointer());
}

std::unique_ptr<PDFiumAnnotation> PDFiumPage::getAnnotation(int nIndex)
{
    std::unique_ptr<PDFiumAnnotation> pPDFiumAnnotation;
    FPDF_ANNOTATION pAnnotation = FPDFPage_GetAnnot(mpPage, nIndex);
    if (pAnnotation)
    {
        pPDFiumAnnotation = std::make_unique<PDFiumAnnotation>(pAnnotation);
    }
    return pPDFiumAnnotation;
}

std::unique_ptr<PDFiumTextPage> PDFiumPage::getTextPage()
{
    std::unique_ptr<PDFiumTextPage> pPDFiumTextPage;
    FPDF_TEXTPAGE pTextPage = FPDFText_LoadPage(mpPage);
    if (pTextPage)
    {
        pPDFiumTextPage = std::make_unique<PDFiumTextPage>(pTextPage);
    }
    return pPDFiumTextPage;
}

PDFiumPageObject::PDFiumPageObject(FPDF_PAGEOBJECT pPageObject)
    : mpPageObject(pPageObject)
{
}

PDFiumPageObject::~PDFiumPageObject() {}

OUString PDFiumPageObject::getText(std::unique_ptr<PDFiumTextPage> const& pTextPage)
{
    OUString sReturnText;

    const int nBytes = FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(), nullptr, 0);

    std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nBytes]);

    const int nActualBytes
        = FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(), pText.get(), nBytes);
    if (nActualBytes > 2)
        sReturnText = OUString(pText.get());

    return sReturnText;
}

int PDFiumPageObject::getType() { return FPDFPageObj_GetType(mpPageObject); }

PDFiumAnnotation::PDFiumAnnotation(FPDF_ANNOTATION pAnnotation)
    : mpAnnotation(pAnnotation)
{
}

PDFiumAnnotation::~PDFiumAnnotation()
{
    if (mpAnnotation)
        FPDFPage_CloseAnnot(mpAnnotation);
}

int PDFiumAnnotation::getSubType() { return FPDFAnnot_GetSubtype(mpAnnotation); }

basegfx::B2DRectangle PDFiumAnnotation::getRectangle()
{
    basegfx::B2DRectangle aB2DRectangle;
    FS_RECTF aRect;
    if (FPDFAnnot_GetRect(mpAnnotation, &aRect))
    {
        aB2DRectangle = basegfx::B2DRectangle(aRect.left, aRect.top, aRect.right, aRect.bottom);
    }
    return aB2DRectangle;
}

bool PDFiumAnnotation::hasKey(OString const& rKey)
{
    return FPDFAnnot_HasKey(mpAnnotation, rKey.getStr());
}

OUString PDFiumAnnotation::getString(OString const& rKey)
{
    OUString rString;
    unsigned long nSize = FPDFAnnot_GetStringValue(mpAnnotation, rKey.getStr(), nullptr, 0);
    if (nSize > 2)
    {
        std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nSize]);
        unsigned long nStringSize = FPDFAnnot_GetStringValue(
            mpAnnotation, rKey.getStr(), reinterpret_cast<FPDF_WCHAR*>(pText.get()), nSize);
        if (nStringSize > 0)
            rString = OUString(pText.get());
    }
    return rString;
}

std::unique_ptr<PDFiumAnnotation> PDFiumAnnotation::getLinked(OString const& rKey)
{
    std::unique_ptr<PDFiumAnnotation> pPDFiumAnnotation;
    FPDF_ANNOTATION pAnnotation = FPDFAnnot_GetLinkedAnnot(mpAnnotation, rKey.getStr());
    if (pAnnotation)
    {
        pPDFiumAnnotation = std::make_unique<PDFiumAnnotation>(pAnnotation);
    }
    return pPDFiumAnnotation;
}

PDFiumTextPage::PDFiumTextPage(FPDF_TEXTPAGE pTextPage)
    : mpTextPage(pTextPage)
{
}

PDFiumTextPage::~PDFiumTextPage()
{
    if (mpTextPage)
        FPDFText_ClosePage(mpTextPage);
}

} // end vcl::pdf

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
