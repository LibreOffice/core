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
#include <vcl/pdf/PDFTextRenderMode.hxx>
#include <vcl/pdf/PDFFillMode.hxx>
#include <vcl/pdf/PDFFindFlags.hxx>

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

    const OUString& getLastError() const { return maLastError; }

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

class VCL_DLLPUBLIC PDFiumAnnotation
{
public:
    virtual ~PDFiumAnnotation() = default;
    virtual PDFAnnotationSubType getSubType() = 0;
    virtual basegfx::B2DRectangle getRectangle() = 0;
    virtual bool hasKey(OString const& rKey) = 0;
    virtual PDFObjectType getValueType(OString const& rKey) = 0;
    virtual OUString getString(OString const& rKey) = 0;
    virtual std::unique_ptr<PDFiumAnnotation> getLinked(OString const& rKey) = 0;
    virtual int getObjectCount() = 0;
    virtual std::unique_ptr<PDFiumPageObject> getObject(int nIndex) = 0;
    virtual std::vector<std::vector<basegfx::B2DPoint>> getInkStrokes() = 0;
    virtual std::vector<basegfx::B2DPoint> getVertices() = 0;
    virtual Color getColor() = 0;
    virtual Color getInteriorColor() = 0;
    virtual float getBorderWidth() = 0;
    virtual basegfx::B2DSize getBorderCornerRadius() = 0;
    virtual size_t getAttachmentPointsCount() = 0;
    virtual std::vector<basegfx::B2DPoint> getAttachmentPoints(size_t nIndex) = 0;
    virtual std::vector<basegfx::B2DPoint> getLineGeometry() = 0;
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

class VCL_DLLPUBLIC PDFiumPageObject
{
public:
    virtual ~PDFiumPageObject() = default;

    virtual PDFPageObjectType getType() = 0;
    virtual OUString getText(std::unique_ptr<PDFiumTextPage> const& pTextPage) = 0;

    virtual int getFormObjectCount() = 0;
    virtual std::unique_ptr<PDFiumPageObject> getFormObject(int nIndex) = 0;

    virtual basegfx::B2DHomMatrix getMatrix() = 0;
    virtual basegfx::B2DRectangle getBounds() = 0;
    virtual double getFontSize() = 0;
    virtual OUString getFontName() = 0;
    virtual PDFTextRenderMode getTextRenderMode() = 0;
    virtual Color getFillColor() = 0;
    virtual Color getStrokeColor() = 0;
    virtual double getStrokeWidth() = 0;
    // Path
    virtual int getPathSegmentCount() = 0;
    virtual std::unique_ptr<PDFiumPathSegment> getPathSegment(int index) = 0;
    virtual Size getImageSize(PDFiumPage& rPage) = 0;
    virtual std::unique_ptr<PDFiumBitmap> getImageBitmap() = 0;
    virtual bool getDrawMode(PDFFillMode& eFillMode, bool& bStroke) = 0;
};

class VCL_DLLPUBLIC PDFiumSearchHandle final
{
private:
    FPDF_SCHHANDLE mpSearchHandle;

    PDFiumSearchHandle(const PDFiumSearchHandle&) = delete;
    PDFiumSearchHandle& operator=(const PDFiumSearchHandle&) = delete;

public:
    PDFiumSearchHandle(FPDF_SCHHANDLE pSearchHandle);
    ~PDFiumSearchHandle();

    FPDF_SCHHANDLE getPointer() { return mpSearchHandle; }
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
    std::unique_ptr<PDFiumSearchHandle> findStart(const OUString& rFindWhat, PDFFindFlags nFlags,
                                                  sal_Int32 nStartIndex);
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
    FPDF_DOCUMENT getPointer() { return mpPdfDocument; }

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
