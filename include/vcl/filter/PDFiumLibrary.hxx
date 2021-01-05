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

#include <com/sun/star/util/DateTime.hpp>

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
#include <tools/gen.hxx>
#include <vcl/checksum.hxx>
#include <vcl/Scanline.hxx>
#include <vcl/pdf/PDFAnnotationSubType.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <vcl/pdf/PDFSegmentType.hxx>
#include <vcl/pdf/PDFBitmapType.hxx>
#include <vcl/pdf/PDFObjectType.hxx>

#include <fpdf_doc.h>

class SvMemoryStream;

namespace vcl::pdf
{
inline constexpr OStringLiteral constDictionaryKeyTitle = "T";
inline constexpr OStringLiteral constDictionaryKeyContents = "Contents";
inline constexpr OStringLiteral constDictionaryKeyPopup = "Popup";
inline constexpr OStringLiteral constDictionaryKeyModificationDate = "M";
inline constexpr OStringLiteral constDictionaryKeyInteriorColor = "IC";

class PDFiumBitmap;
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

    OUString getLastError() const { return maLastError; }

    std::unique_ptr<PDFiumDocument> openDocument(const void* pData, int nSize);
    std::unique_ptr<PDFiumBitmap> createBitmap(int nWidth, int nHeight, int nAlpha);
};

class PDFiumPage;

class VCL_DLLPUBLIC PDFiumBitmap
{
public:
    virtual ~PDFiumBitmap() = default;
    virtual void fillRect(int left, int top, int width, int height, sal_uInt32 nColor) = 0;
    virtual void renderPageBitmap(PDFiumPage* pPage, int nStartX, int nStartY, int nSizeX,
                                  int nSizeY)
        = 0;
    virtual ConstScanline getBuffer() = 0;
    virtual int getStride() = 0;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual PDFBitmapType getFormat() = 0;
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
    PDFObjectType getValueType(OString const& rKey);
    OUString getString(OString const& rKey);
    std::unique_ptr<PDFiumAnnotation> getLinked(OString const& rKey);
    int getObjectCount();
    std::unique_ptr<PDFiumPageObject> getObject(int nIndex);
    std::vector<std::vector<basegfx::B2DPoint>> getInkStrokes();
    std::vector<basegfx::B2DPoint> getVertices();
    Color getColor();
    Color getInteriorColor();
    float getBorderWidth();
    basegfx::B2DSize getBorderCornerRadius();
    size_t getAttachmentPointsCount();
    std::vector<basegfx::B2DPoint> getAttachmentPoints(size_t nIndex);
    std::vector<basegfx::B2DPoint> getLineGeometry();
};

class PDFiumTextPage;

class VCL_DLLPUBLIC PDFiumPathSegment
{
public:
    virtual ~PDFiumPathSegment() = default;
    virtual basegfx::B2DPoint getPoint() const = 0;
    virtual bool isClosed() const = 0;
    virtual PDFSegmentType getType() const = 0;
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

    PDFPageObjectType getType();
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
    Size getImageSize(PDFiumPage& rPage);
    std::unique_ptr<PDFiumBitmap> getImageBitmap();
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
    BitmapChecksum getChecksum(int nMDPPerm);

    double getWidth();
    double getHeight();

    bool hasTransparency();

    bool hasLinks();
};

/// Represents one digital signature, as exposed by PDFium.
class VCL_DLLPUBLIC PDFiumSignature final
{
private:
    FPDF_SIGNATURE mpSignature;
    PDFiumSignature(const PDFiumSignature&) = delete;
    PDFiumSignature& operator=(const PDFiumSignature&) = delete;

public:
    PDFiumSignature(FPDF_SIGNATURE pSignature);

    std::vector<int> getByteRange();
    int getDocMDPPermission();
    std::vector<unsigned char> getContents();
    OString getSubFilter();
    OUString getReason();
    css::util::DateTime getTime();
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
    int getSignatureCount();
    int getFileVersion();
    bool saveWithVersion(SvMemoryStream& rStream, int nFileVersion);

    std::unique_ptr<PDFiumPage> openPage(int nIndex);
    std::unique_ptr<PDFiumSignature> getSignature(int nIndex);
    std::vector<unsigned int> getTrailerEnds();
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
