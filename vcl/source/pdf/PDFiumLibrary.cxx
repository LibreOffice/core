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

#include <cassert>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <fpdf_annot.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_save.h>
#include <fpdf_signature.h>
#include <fpdf_formfill.h>

#include <osl/endian.h>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>

#include <bitmapwriteaccess.hxx>

static_assert(static_cast<int>(vcl::pdf::PDFPageObjectType::Unknown) == FPDF_PAGEOBJ_UNKNOWN,
              "PDFPageObjectType::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFPageObjectType::Text) == FPDF_PAGEOBJ_TEXT,
              "PDFPageObjectType::Text value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFPageObjectType::Path) == FPDF_PAGEOBJ_PATH,
              "PDFPageObjectType::Path value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFPageObjectType::Image) == FPDF_PAGEOBJ_IMAGE,
              "PDFPageObjectType::Image value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFPageObjectType::Shading) == FPDF_PAGEOBJ_SHADING,
              "PDFPageObjectType::Shading value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFPageObjectType::Form) == FPDF_PAGEOBJ_FORM,
              "PDFPageObjectType::Form value mismatch");

namespace
{
/// Callback class to be used with FPDF_SaveWithVersion().
struct CompatibleWriter : public FPDF_FILEWRITE
{
    CompatibleWriter(SvMemoryStream& rStream)
        : m_rStream(rStream)
    {
    }

    SvMemoryStream& m_rStream;
};

int CompatibleWriterCallback(FPDF_FILEWRITE* pFileWrite, const void* pData, unsigned long nSize)
{
    auto pImpl = static_cast<CompatibleWriter*>(pFileWrite);
    pImpl->m_rStream.WriteBytes(pData, nSize);
    return 1;
}
}

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

std::unique_ptr<PDFiumBitmap> PDFium::createBitmap(int nWidth, int nHeight, int nAlpha)
{
    std::unique_ptr<PDFiumBitmap> pPDFiumBitmap;
    FPDF_BITMAP pPdfBitmap = FPDFBitmap_Create(nWidth, nHeight, nAlpha);
    if (!pPdfBitmap)
    {
        maLastError = "Failed to create bitmap";
    }
    else
    {
        pPDFiumBitmap = std::make_unique<PDFiumBitmap>(pPdfBitmap);
    }
    return pPDFiumBitmap;
}

PDFiumSignature::PDFiumSignature(FPDF_SIGNATURE pSignature)
    : mpSignature(pSignature)
{
}

PDFiumDocument::PDFiumDocument(FPDF_DOCUMENT pPdfDocument)
    : mpPdfDocument(pPdfDocument)
    , m_aFormCallbacks()
{
    m_aFormCallbacks.version = 1;
    m_pFormHandle = std::make_unique<PDFiumFormHandle>(
        FPDFDOC_InitFormFillEnvironment(pPdfDocument, &m_aFormCallbacks));
}

PDFiumDocument::~PDFiumDocument()
{
    m_pFormHandle.reset();
    if (mpPdfDocument)
        FPDF_CloseDocument(mpPdfDocument);
}

FPDF_FORMHANDLE PDFiumDocument::getFormHandlePointer() { return m_pFormHandle->getPointer(); }

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

std::unique_ptr<PDFiumSignature> PDFiumDocument::getSignature(int nIndex)
{
    std::unique_ptr<PDFiumSignature> pPDFiumSignature;
    FPDF_SIGNATURE pSignature = FPDF_GetSignatureObject(mpPdfDocument, nIndex);
    if (pSignature)
    {
        pPDFiumSignature = std::make_unique<PDFiumSignature>(pSignature);
    }
    return pPDFiumSignature;
}

std::vector<unsigned int> PDFiumDocument::getTrailerEnds()
{
    int nNumTrailers = FPDF_GetTrailerEnds(mpPdfDocument, nullptr, 0);
    std::vector<unsigned int> aTrailerEnds(nNumTrailers);
    FPDF_GetTrailerEnds(mpPdfDocument, aTrailerEnds.data(), aTrailerEnds.size());
    return aTrailerEnds;
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

int PDFiumDocument::getSignatureCount() { return FPDF_GetSignatureCount(mpPdfDocument); }

int PDFiumDocument::getFileVersion()
{
    int nFileVersion = 0;
    FPDF_GetFileVersion(mpPdfDocument, &nFileVersion);
    return nFileVersion;
}

bool PDFiumDocument::saveWithVersion(SvMemoryStream& rStream, int nFileVersion)
{
    CompatibleWriter aWriter(rStream);
    aWriter.version = 1;
    aWriter.WriteBlock = &CompatibleWriterCallback;
    if (!FPDF_SaveWithVersion(mpPdfDocument, &aWriter, 0, nFileVersion))
    {
        return false;
    }

    return true;
}

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

    int nBytes = FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(), nullptr, 0);
    assert(nBytes % 2 == 0);
    nBytes /= 2;

    std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nBytes]);

    int nActualBytes
        = FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(), pText.get(), nBytes * 2);
    assert(nActualBytes % 2 == 0);
    nActualBytes /= 2;
    if (nActualBytes > 1)
    {
#if defined OSL_BIGENDIAN
        // The data returned by FPDFTextObj_GetText is documented to always be UTF-16LE:
        for (int i = 0; i != nActualBytes; ++i)
        {
            pText[i] = OSL_SWAPWORD(pText[i]);
        }
#endif
        sReturnText = OUString(pText.get());
    }

    return sReturnText;
}

PDFPageObjectType PDFiumPageObject::getType()
{
    return static_cast<PDFPageObjectType>(FPDFPageObj_GetType(mpPageObject));
}

int PDFiumPageObject::getFormObjectCount() { return FPDFFormObj_CountObjects(mpPageObject); }

std::unique_ptr<PDFiumPageObject> PDFiumPageObject::getFormObject(int nIndex)
{
    std::unique_ptr<PDFiumPageObject> pPDFiumFormObject;
    FPDF_PAGEOBJECT pFormObject = FPDFFormObj_GetObject(mpPageObject, nIndex);
    if (pFormObject)
    {
        pPDFiumFormObject = std::make_unique<PDFiumPageObject>(pFormObject);
    }
    return pPDFiumFormObject;
}

basegfx::B2DHomMatrix PDFiumPageObject::getMatrix()
{
    basegfx::B2DHomMatrix aB2DMatrix;
    FS_MATRIX matrix;
    if (FPDFFormObj_GetMatrix(mpPageObject, &matrix))
        aB2DMatrix = basegfx::B2DHomMatrix::abcdef(matrix.a, matrix.b, matrix.c, matrix.d, matrix.e,
                                                   matrix.f);
    return aB2DMatrix;
}

basegfx::B2DRectangle PDFiumPageObject::getBounds()
{
    basegfx::B2DRectangle aB2DRectangle;

    float left = 0;
    float bottom = 0;
    float right = 0;
    float top = 0;
    if (FPDFPageObj_GetBounds(mpPageObject, &left, &bottom, &right, &top))
    {
        aB2DRectangle = basegfx::B2DRectangle(left, top, right, bottom);
    }
    return aB2DRectangle;
}

double PDFiumPageObject::getFontSize() { return FPDFTextObj_GetFontSize(mpPageObject); }

OUString PDFiumPageObject::getFontName()
{
    OUString sFontName;
    const int nFontName = 80 + 1;
    std::unique_ptr<char[]> pFontName(new char[nFontName]); // + terminating null
    int nFontNameChars = FPDFTextObj_GetFontName(mpPageObject, pFontName.get(), nFontName);
    if (nFontName >= nFontNameChars)
    {
        sFontName = OUString::createFromAscii(pFontName.get());
    }
    return sFontName;
}

int PDFiumPageObject::getTextRenderMode() { return FPDFTextObj_GetTextRenderMode(mpPageObject); }

Color PDFiumPageObject::getFillColor()
{
    Color aColor = COL_TRANSPARENT;
    unsigned int nR, nG, nB, nA;
    if (FPDFPageObj_GetFillColor(mpPageObject, &nR, &nG, &nB, &nA))
    {
        aColor = Color(0xFF - nA, nR, nG, nB);
    }
    return aColor;
}

Color PDFiumPageObject::getStrokeColor()
{
    Color aColor = COL_TRANSPARENT;
    unsigned int nR, nG, nB, nA;
    if (FPDFPageObj_GetStrokeColor(mpPageObject, &nR, &nG, &nB, &nA))
    {
        aColor = Color(0xFF - nA, nR, nG, nB);
    }
    return aColor;
}

int PDFiumPageObject::getPathSegmentCount() { return FPDFPath_CountSegments(mpPageObject); }

std::unique_ptr<PDFiumPathSegment> PDFiumPageObject::getPathSegment(int index)
{
    std::unique_ptr<PDFiumPathSegment> pPDFiumPathSegment;
    FPDF_PATHSEGMENT pPathSegment = FPDFPath_GetPathSegment(mpPageObject, index);
    if (pPathSegment)
    {
        pPDFiumPathSegment = std::make_unique<PDFiumPathSegment>(pPathSegment);
    }
    return pPDFiumPathSegment;
}

Size PDFiumPageObject::getImageSize(PDFiumPage& rPage)
{
    FPDF_IMAGEOBJ_METADATA aMeta;
    FPDFImageObj_GetImageMetadata(mpPageObject, rPage.getPointer(), &aMeta);
    return Size(aMeta.width, aMeta.height);
}

std::unique_ptr<PDFiumBitmap> PDFiumPageObject::getImageBitmap()
{
    std::unique_ptr<PDFiumBitmap> pPDFiumBitmap;
    FPDF_BITMAP pBitmap = FPDFImageObj_GetBitmap(mpPageObject);
    if (pBitmap)
    {
        pPDFiumBitmap = std::make_unique<PDFiumBitmap>(pBitmap);
    }
    return pPDFiumBitmap;
}

BitmapChecksum PDFiumPage::getChecksum(int nMDPPerm)
{
    size_t nPageWidth = getWidth();
    size_t nPageHeight = getHeight();
    auto pPdfBitmap
        = std::make_unique<PDFiumBitmap>(FPDFBitmap_Create(nPageWidth, nPageHeight, /*alpha=*/1));
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
    FPDF_RenderPageBitmap(pPdfBitmap->getPointer(), mpPage, /*start_x=*/0, /*start_y=*/0,
                          nPageWidth, nPageHeight,
                          /*rotate=*/0, nFlags);
    Bitmap aBitmap(Size(nPageWidth, nPageHeight), 24);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        const auto pPdfBuffer
            = static_cast<ConstScanline>(FPDFBitmap_GetBuffer(pPdfBitmap->getPointer()));
        const int nStride = FPDFBitmap_GetStride(pPdfBitmap->getPointer());
        for (size_t nRow = 0; nRow < nPageHeight; ++nRow)
        {
            ConstScanline pPdfLine = pPdfBuffer + (nStride * nRow);
            pWriteAccess->CopyScanline(nRow, pPdfLine, ScanlineFormat::N32BitTcBgra, nStride);
        }
    }
    return aBitmap.GetChecksum();
}

double PDFiumPage::getWidth() { return FPDF_GetPageWidth(mpPage); }

double PDFiumPage::getHeight() { return FPDF_GetPageHeight(mpPage); }

bool PDFiumPage::hasTransparency() { return FPDFPage_HasTransparency(mpPage); }

PDFiumPathSegment::PDFiumPathSegment(FPDF_PATHSEGMENT pPathSegment)
    : mpPathSegment(pPathSegment)
{
}

PDFiumPathSegment::~PDFiumPathSegment() {}

basegfx::B2DPoint PDFiumPathSegment::getPoint() const
{
    basegfx::B2DPoint aPoint;
    float fx, fy;
    if (FPDFPathSegment_GetPoint(mpPathSegment, &fx, &fy))
        aPoint = basegfx::B2DPoint(fx, fy);
    return aPoint;
}

bool PDFiumPathSegment::isClosed() const { return FPDFPathSegment_GetClose(mpPathSegment); }

int PDFiumPathSegment::getType() const { return FPDFPathSegment_GetType(mpPathSegment); }

PDFiumFormHandle::PDFiumFormHandle(FPDF_FORMHANDLE pHandle)
    : mpHandle(pHandle)
{
}

PDFiumFormHandle::~PDFiumFormHandle() { FPDFDOC_ExitFormFillEnvironment(mpHandle); }

FPDF_FORMHANDLE PDFiumFormHandle::getPointer() { return mpHandle; }

PDFiumBitmap::PDFiumBitmap(FPDF_BITMAP pBitmap)
    : mpBitmap(pBitmap)
{
}

PDFiumBitmap::~PDFiumBitmap()
{
    if (mpBitmap)
    {
        FPDFBitmap_Destroy(mpBitmap);
    }
}

void PDFiumBitmap::fillRect(int left, int top, int width, int height, sal_uInt32 nColor)
{
    FPDFBitmap_FillRect(mpBitmap, left, top, width, height, nColor);
}

void PDFiumBitmap::renderPageBitmap(PDFiumDocument* pDoc, PDFiumPage* pPage, int nStartX,
                                    int nStartY, int nSizeX, int nSizeY)
{
    FPDF_RenderPageBitmap(mpBitmap, pPage->getPointer(), nStartX, nStartY, nSizeX, nSizeY,
                          /*rotate=*/0, /*flags=*/0);

    // Render widget annotations for FormFields.
    FPDF_FFLDraw(pDoc->getFormHandlePointer(), mpBitmap, pPage->getPointer(), nStartX, nStartY,
                 nSizeX, nSizeY, /*rotate=*/0, /*flags=*/0);
}

ConstScanline PDFiumBitmap::getBuffer()
{
    return static_cast<ConstScanline>(FPDFBitmap_GetBuffer(mpBitmap));
}

int PDFiumBitmap::getStride() { return FPDFBitmap_GetStride(mpBitmap); }

PDFiumAnnotation::PDFiumAnnotation(FPDF_ANNOTATION pAnnotation)
    : mpAnnotation(pAnnotation)
{
}

PDFiumAnnotation::~PDFiumAnnotation()
{
    if (mpAnnotation)
        FPDFPage_CloseAnnot(mpAnnotation);
}

PDFAnnotationSubType PDFiumAnnotation::getSubType()
{
    return PDFAnnotationSubType(FPDFAnnot_GetSubtype(mpAnnotation));
}

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

Color PDFiumAnnotation::getColor()
{
    Color aColor = COL_TRANSPARENT;
    unsigned int nR, nG, nB, nA;
    if (FPDFAnnot_GetColor(mpAnnotation, FPDFANNOT_COLORTYPE_Color, &nR, &nG, &nB, &nA))
    {
        aColor = Color(0xFF - nA, nR, nG, nB);
    }
    return aColor;
}

Color PDFiumAnnotation::getInteriorColor()
{
    Color aColor = COL_TRANSPARENT;
    unsigned int nR, nG, nB, nA;
    if (FPDFAnnot_GetColor(mpAnnotation, FPDFANNOT_COLORTYPE_InteriorColor, &nR, &nG, &nB, &nA))
    {
        aColor = Color(0xFF - nA, nR, nG, nB);
    }
    return aColor;
}

size_t PDFiumAnnotation::getAttachmentPointsCount()
{
    return FPDFAnnot_CountAttachmentPoints(mpAnnotation);
}

std::vector<basegfx::B2DPoint> PDFiumAnnotation::getAttachmentPoints(size_t nIndex)
{
    std::vector<basegfx::B2DPoint> aQuads;

    FS_QUADPOINTSF aQuadpoints;
    if (FPDFAnnot_GetAttachmentPoints(mpAnnotation, nIndex, &aQuadpoints))
    {
        aQuads.emplace_back(aQuadpoints.x1, aQuadpoints.y1);
        aQuads.emplace_back(aQuadpoints.x2, aQuadpoints.y2);
        aQuads.emplace_back(aQuadpoints.x3, aQuadpoints.y3);
        aQuads.emplace_back(aQuadpoints.x4, aQuadpoints.y4);
    }
    return aQuads;
}

std::vector<basegfx::B2DPoint> PDFiumAnnotation::getLineGeometry()
{
    std::vector<basegfx::B2DPoint> aLine;
    FS_POINTF aStart;
    FS_POINTF aEnd;
    if (FPDFAnnot_GetLine(mpAnnotation, &aStart, &aEnd))
    {
        aLine.emplace_back(aStart.x, aStart.y);
        aLine.emplace_back(aEnd.x, aEnd.y);
    }
    return aLine;
}

namespace
{
bool getBorderProperties(FPDF_ANNOTATION mpAnnotation, float& rHorizontalCornerRadius,
                         float& rVerticalCornerRadius, float& rBorderWidth)
{
    float fHoriRadius = 0.0f;
    float fVertRadius = 0.0f;
    float fWidth = 0.0f;

    if (!FPDFAnnot_GetBorder(mpAnnotation, &fHoriRadius, &fVertRadius, &fWidth))
        return false;

    rHorizontalCornerRadius = fHoriRadius;
    rVerticalCornerRadius = fVertRadius;
    rBorderWidth = fWidth;
    return true;
}
}

float PDFiumAnnotation::getBorderWidth()
{
    float fHorizontalCornerRadius;
    float fVerticalCornerRadius;
    float fBorderWidth;

    if (!getBorderProperties(mpAnnotation, fHorizontalCornerRadius, fVerticalCornerRadius,
                             fBorderWidth))
        return 0.0f;
    return fBorderWidth;
}

basegfx::B2DSize PDFiumAnnotation::getBorderCornerRadius()
{
    float fHorizontalCornerRadius;
    float fVerticalCornerRadius;
    float fBorderWidth;

    if (!getBorderProperties(mpAnnotation, fHorizontalCornerRadius, fVerticalCornerRadius,
                             fBorderWidth))
        return basegfx::B2DSize(0.0, 0.0);
    return basegfx::B2DSize(fHorizontalCornerRadius, fVerticalCornerRadius);
}

bool PDFiumAnnotation::hasKey(OString const& rKey)
{
    return FPDFAnnot_HasKey(mpAnnotation, rKey.getStr());
}

int PDFiumAnnotation::getValueType(OString const& rKey)
{
    return FPDFAnnot_GetValueType(mpAnnotation, rKey.getStr());
}

OUString PDFiumAnnotation::getString(OString const& rKey)
{
    OUString rString;
    unsigned long nSize = FPDFAnnot_GetStringValue(mpAnnotation, rKey.getStr(), nullptr, 0);
    assert(nSize % 2 == 0);
    nSize /= 2;
    if (nSize > 1)
    {
        std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nSize]);
        unsigned long nStringSize = FPDFAnnot_GetStringValue(
            mpAnnotation, rKey.getStr(), reinterpret_cast<FPDF_WCHAR*>(pText.get()), nSize * 2);
        assert(nStringSize % 2 == 0);
        nStringSize /= 2;
        if (nStringSize > 0)
        {
#if defined OSL_BIGENDIAN
            // The data returned by FPDFAnnot_GetStringValue is documented to always be UTF-16LE:
            for (unsigned long i = 0; i != nStringSize; ++i)
            {
                pText[i] = OSL_SWAPWORD(pText[i]);
            }
#endif
            rString = OUString(pText.get());
        }
    }
    return rString;
}

std::vector<std::vector<basegfx::B2DPoint>> PDFiumAnnotation::getInkStrokes()
{
    std::vector<std::vector<basegfx::B2DPoint>> aB2DPointList;
    int nInkStrokes = FPDFAnnot_GetInkStrokeCount(mpAnnotation);
    for (int i = 0; i < nInkStrokes; i++)
    {
        std::vector<basegfx::B2DPoint> aB2DPoints;
        int nPoints = FPDFAnnot_GetInkStrokePointCount(mpAnnotation, i);
        if (nPoints)
        {
            std::vector<FS_POINTF> aPoints(nPoints);
            if (FPDFAnnot_GetInkStrokePoints(mpAnnotation, i, aPoints.data()))
            {
                for (auto const& rPoint : aPoints)
                {
                    aB2DPoints.emplace_back(rPoint.x, rPoint.y);
                }
                aB2DPointList.push_back(aB2DPoints);
            }
        }
    }
    return aB2DPointList;
}

std::vector<basegfx::B2DPoint> PDFiumAnnotation::getVertices()
{
    std::vector<basegfx::B2DPoint> aB2DPoints;
    int nPoints = FPDFAnnot_GetVerticesCount(mpAnnotation);
    if (nPoints)
    {
        std::vector<FS_POINTF> aPoints(nPoints);
        if (FPDFAnnot_GetVertices(mpAnnotation, aPoints.data()))
        {
            for (auto const& rPoint : aPoints)
                aB2DPoints.emplace_back(rPoint.x, rPoint.y);
        }
    }
    return aB2DPoints;
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

int PDFiumAnnotation::getObjectCount() { return FPDFAnnot_GetObjectCount(mpAnnotation); }

std::unique_ptr<PDFiumPageObject> PDFiumAnnotation::getObject(int nIndex)
{
    std::unique_ptr<PDFiumPageObject> pPDFiumPageObject;
    FPDF_PAGEOBJECT pPageObject = FPDFAnnot_GetObject(mpAnnotation, nIndex);
    if (pPageObject)
    {
        pPDFiumPageObject = std::make_unique<PDFiumPageObject>(pPageObject);
    }
    return pPDFiumPageObject;
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

int PDFiumTextPage::countChars() { return FPDFText_CountChars(mpTextPage); }

unsigned int PDFiumTextPage::getUnicode(int index)
{
    return FPDFText_GetUnicode(mpTextPage, index);
}

} // end vcl::pdf

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
