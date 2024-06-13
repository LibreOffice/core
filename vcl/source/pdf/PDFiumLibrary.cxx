/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/filter/PDFiumLibrary.hxx>

#include <cassert>

#include <sal/log.hxx>
#include <fpdf_doc.h>
#include <fpdf_annot.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_save.h>
#include <fpdf_signature.h>
#include <fpdf_formfill.h>

#include <osl/endian.h>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <tools/UnitConversion.hxx>
#include <o3tl/string_view.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>

using namespace com::sun::star;

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

static_assert(static_cast<int>(vcl::pdf::PDFSegmentType::Unknown) == FPDF_SEGMENT_UNKNOWN,
              "PDFSegmentType::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFSegmentType::Lineto) == FPDF_SEGMENT_LINETO,
              "PDFSegmentType::Lineto value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFSegmentType::Bezierto) == FPDF_SEGMENT_BEZIERTO,
              "PDFSegmentType::Bezierto value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFSegmentType::Moveto) == FPDF_SEGMENT_MOVETO,
              "PDFSegmentType::Moveto value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFBitmapType::Unknown) == FPDFBitmap_Unknown,
              "PDFBitmapType::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFBitmapType::Gray) == FPDFBitmap_Gray,
              "PDFBitmapType::Gray value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFBitmapType::BGR) == FPDFBitmap_BGR,
              "PDFBitmapType::BGR value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFBitmapType::BGRx) == FPDFBitmap_BGRx,
              "PDFBitmapType::BGRx value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFBitmapType::BGRA) == FPDFBitmap_BGRA,
              "PDFBitmapType::BGRA value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Unknown) == FPDF_OBJECT_UNKNOWN,
              "PDFObjectType::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Boolean) == FPDF_OBJECT_BOOLEAN,
              "PDFObjectType::Boolean value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Number) == FPDF_OBJECT_NUMBER,
              "PDFObjectType::Number value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::String) == FPDF_OBJECT_STRING,
              "PDFObjectType::String value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Name) == FPDF_OBJECT_NAME,
              "PDFObjectType::Name value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Array) == FPDF_OBJECT_ARRAY,
              "PDFObjectType::Array value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Dictionary) == FPDF_OBJECT_DICTIONARY,
              "PDFObjectType::Dictionary value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Stream) == FPDF_OBJECT_STREAM,
              "PDFObjectType::Stream value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Nullobj) == FPDF_OBJECT_NULLOBJ,
              "PDFObjectType::Nullobj value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFObjectType::Reference) == FPDF_OBJECT_REFERENCE,
              "PDFObjectType::Reference value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::Unknown) == FPDF_TEXTRENDERMODE_UNKNOWN,
              "PDFTextRenderMode::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::Fill) == FPDF_TEXTRENDERMODE_FILL,
              "PDFTextRenderMode::Fill value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::Stroke) == FPDF_TEXTRENDERMODE_STROKE,
              "PDFTextRenderMode::Stroke value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::FillStroke)
                  == FPDF_TEXTRENDERMODE_FILL_STROKE,
              "PDFTextRenderMode::FillStroke value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::Invisible)
                  == FPDF_TEXTRENDERMODE_INVISIBLE,
              "PDFTextRenderMode::Invisible value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::FillClip)
                  == FPDF_TEXTRENDERMODE_FILL_CLIP,
              "PDFTextRenderMode::FillClip value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::StrokeClip)
                  == FPDF_TEXTRENDERMODE_STROKE_CLIP,
              "PDFTextRenderMode::StrokeClip value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::FillStrokeClip)
                  == FPDF_TEXTRENDERMODE_FILL_STROKE_CLIP,
              "PDFTextRenderMode::FillStrokeClip value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFTextRenderMode::Clip) == FPDF_TEXTRENDERMODE_CLIP,
              "PDFTextRenderMode::Clip value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFFillMode::None) == FPDF_FILLMODE_NONE,
              "PDFFillMode::None value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFillMode::Alternate) == FPDF_FILLMODE_ALTERNATE,
              "PDFFillMode::Alternate value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFillMode::Winding) == FPDF_FILLMODE_WINDING,
              "PDFFillMode::Winding value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFFindFlags::MatchCase) == FPDF_MATCHCASE,
              "PDFFindFlags::MatchCase value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFindFlags::MatchWholeWord) == FPDF_MATCHWHOLEWORD,
              "PDFFindFlags::MatchWholeWord value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFindFlags::Consecutive) == FPDF_CONSECUTIVE,
              "PDFFindFlags::Consecutive value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFErrorType::Success) == FPDF_ERR_SUCCESS,
              "PDFErrorType::Success value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFErrorType::Unknown) == FPDF_ERR_UNKNOWN,
              "PDFErrorType::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFErrorType::File) == FPDF_ERR_FILE,
              "PDFErrorType::File value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFErrorType::Format) == FPDF_ERR_FORMAT,
              "PDFErrorType::Format value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFErrorType::Password) == FPDF_ERR_PASSWORD,
              "PDFErrorType::Password value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFErrorType::Security) == FPDF_ERR_SECURITY,
              "PDFErrorType::Security value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFErrorType::Page) == FPDF_ERR_PAGE,
              "PDFErrorType::Page value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::Unknown) == FPDF_FORMFIELD_UNKNOWN,
              "PDFFormFieldType::Unknown value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::PushButton) == FPDF_FORMFIELD_PUSHBUTTON,
              "PDFFormFieldType::PushButton value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::CheckBox) == FPDF_FORMFIELD_CHECKBOX,
              "PDFFormFieldType::CheckBox value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::RadioButton)
                  == FPDF_FORMFIELD_RADIOBUTTON,
              "PDFFormFieldType::RadioButton value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::ComboBox) == FPDF_FORMFIELD_COMBOBOX,
              "PDFFormFieldType::ComboBox value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::ListBox) == FPDF_FORMFIELD_LISTBOX,
              "PDFFormFieldType::ListBox value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::TextField) == FPDF_FORMFIELD_TEXTFIELD,
              "PDFFormFieldType::TextField value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFFormFieldType::Signature) == FPDF_FORMFIELD_SIGNATURE,
              "PDFFormFieldType::Signature value mismatch");

static_assert(static_cast<int>(vcl::pdf::PDFAnnotAActionType::KeyStroke)
                  == FPDF_ANNOT_AACTION_KEY_STROKE,
              "PDFAnnotAActionType::KeyStroke) value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFAnnotAActionType::Format) == FPDF_ANNOT_AACTION_FORMAT,
              "PDFAnnotAActionType::Format) value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFAnnotAActionType::Validate)
                  == FPDF_ANNOT_AACTION_VALIDATE,
              "PDFAnnotAActionType::Validate) value mismatch");
static_assert(static_cast<int>(vcl::pdf::PDFAnnotAActionType::Calculate)
                  == FPDF_ANNOT_AACTION_CALCULATE,
              "PDFAnnotAActionType::Calculate) value mismatch");

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
namespace
{
class PDFiumBitmapImpl final : public PDFiumBitmap
{
private:
    FPDF_BITMAP mpBitmap;

    PDFiumBitmapImpl(const PDFiumBitmapImpl&) = delete;
    PDFiumBitmapImpl& operator=(const PDFiumBitmapImpl&) = delete;

public:
    PDFiumBitmapImpl(FPDF_BITMAP pBitmap);
    ~PDFiumBitmapImpl() override;
    FPDF_BITMAP getPointer() { return mpBitmap; }

    void fillRect(int left, int top, int width, int height, sal_uInt32 nColor) override;
    void renderPageBitmap(PDFiumDocument* pDoc, PDFiumPage* pPage, int nStartX, int nStartY,
                          int nSizeX, int nSizeY) override;
    ConstScanline getBuffer() override;
    int getStride() override;
    int getWidth() override;
    int getHeight() override;
    PDFBitmapType getFormat() override;
    BitmapEx createBitmapFromBuffer() override;
};

class PDFiumPathSegmentImpl final : public PDFiumPathSegment
{
private:
    FPDF_PATHSEGMENT mpPathSegment;

    PDFiumPathSegmentImpl(const PDFiumPathSegmentImpl&) = delete;
    PDFiumPathSegmentImpl& operator=(const PDFiumPathSegmentImpl&) = delete;

public:
    PDFiumPathSegmentImpl(FPDF_PATHSEGMENT pPathSegment);

    basegfx::B2DPoint getPoint() const override;
    bool isClosed() const override;
    PDFSegmentType getType() const override;
};

class PDFiumAnnotationImpl final : public PDFiumAnnotation
{
private:
    FPDF_ANNOTATION mpAnnotation;

    PDFiumAnnotationImpl(const PDFiumAnnotationImpl&) = delete;
    PDFiumAnnotationImpl& operator=(const PDFiumAnnotationImpl&) = delete;

public:
    PDFiumAnnotationImpl(FPDF_ANNOTATION pAnnotation);
    ~PDFiumAnnotationImpl();
    FPDF_ANNOTATION getPointer() { return mpAnnotation; }

    PDFAnnotationSubType getSubType() override;
    basegfx::B2DRectangle getRectangle() override;
    bool hasKey(OString const& rKey) override;
    PDFObjectType getValueType(OString const& rKey) override;
    OUString getString(OString const& rKey) override;
    std::unique_ptr<PDFiumAnnotation> getLinked(OString const& rKey) override;
    int getObjectCount() override;
    std::unique_ptr<PDFiumPageObject> getObject(int nIndex) override;
    std::vector<std::vector<basegfx::B2DPoint>> getInkStrokes() override;
    std::vector<basegfx::B2DPoint> getVertices() override;
    Color getColor() override;
    Color getInteriorColor() override;
    float getBorderWidth() override;
    basegfx::B2DSize getBorderCornerRadius() override;
    size_t getAttachmentPointsCount() override;
    std::vector<basegfx::B2DPoint> getAttachmentPoints(size_t nIndex) override;
    std::vector<basegfx::B2DPoint> getLineGeometry() override;
    PDFFormFieldType getFormFieldType(PDFiumDocument* pDoc) override;
    float getFontSize(PDFiumDocument* pDoc) override;
    OUString getFormFieldAlternateName(PDFiumDocument* pDoc) override;
    int getFormFieldFlags(PDFiumDocument* pDoc) override;
    OUString getFormAdditionalActionJavaScript(PDFiumDocument* pDoc,
                                               PDFAnnotAActionType eEvent) override;
    OUString getFormFieldValue(PDFiumDocument* pDoc) override;
};

class PDFiumPageObjectImpl final : public PDFiumPageObject
{
private:
    FPDF_PAGEOBJECT mpPageObject;

    PDFiumPageObjectImpl(const PDFiumPageObjectImpl&) = delete;
    PDFiumPageObjectImpl& operator=(const PDFiumPageObjectImpl&) = delete;

public:
    PDFiumPageObjectImpl(FPDF_PAGEOBJECT pPageObject);

    PDFPageObjectType getType() override;
    OUString getText(std::unique_ptr<PDFiumTextPage> const& pTextPage) override;

    int getFormObjectCount() override;
    std::unique_ptr<PDFiumPageObject> getFormObject(int nIndex) override;

    basegfx::B2DHomMatrix getMatrix() override;
    basegfx::B2DRectangle getBounds() override;
    double getFontSize() override;
    OUString getFontName() override;
    PDFTextRenderMode getTextRenderMode() override;
    Color getFillColor() override;
    Color getStrokeColor() override;
    double getStrokeWidth() override;
    // Path
    int getPathSegmentCount() override;
    std::unique_ptr<PDFiumPathSegment> getPathSegment(int index) override;
    Size getImageSize(PDFiumPage& rPage) override;
    std::unique_ptr<PDFiumBitmap> getImageBitmap() override;
    bool getDrawMode(PDFFillMode& eFillMode, bool& bStroke) override;
};

class PDFiumSearchHandleImpl final : public PDFiumSearchHandle
{
private:
    FPDF_SCHHANDLE mpSearchHandle;

    PDFiumSearchHandleImpl(const PDFiumSearchHandleImpl&) = delete;
    PDFiumSearchHandleImpl& operator=(const PDFiumSearchHandleImpl&) = delete;

public:
    PDFiumSearchHandleImpl(FPDF_SCHHANDLE pSearchHandle);
    ~PDFiumSearchHandleImpl();

    bool findNext() override;
    bool findPrev() override;
    int getSearchResultIndex() override;
    int getSearchCount() override;
};

class PDFiumTextPageImpl final : public PDFiumTextPage
{
private:
    FPDF_TEXTPAGE mpTextPage;

    PDFiumTextPageImpl(const PDFiumTextPageImpl&) = delete;
    PDFiumTextPageImpl& operator=(const PDFiumTextPageImpl&) = delete;

public:
    PDFiumTextPageImpl(FPDF_TEXTPAGE pTextPage);
    ~PDFiumTextPageImpl();

    FPDF_TEXTPAGE getPointer() { return mpTextPage; }

    int countChars() override;
    unsigned int getUnicode(int index) override;
    std::unique_ptr<PDFiumSearchHandle> findStart(const OUString& rFindWhat, PDFFindFlags nFlags,
                                                  sal_Int32 nStartIndex) override;

    /// Returned rect is no longer upside down and is in mm100.
    basegfx::B2DRectangle getCharBox(int nIndex, double fPageHeight) override;
};

class PDFiumSignatureImpl final : public PDFiumSignature
{
private:
    FPDF_SIGNATURE mpSignature;
    PDFiumSignatureImpl(const PDFiumSignatureImpl&) = delete;
    PDFiumSignatureImpl& operator=(const PDFiumSignatureImpl&) = delete;

public:
    PDFiumSignatureImpl(FPDF_SIGNATURE pSignature);

    std::vector<int> getByteRange() override;
    int getDocMDPPermission() override;
    std::vector<unsigned char> getContents() override;
    OString getSubFilter() override;
    OUString getReason() override;
    css::util::DateTime getTime() override;
};

class PDFiumPageImpl final : public PDFiumPage
{
private:
    FPDF_PAGE mpPage;

private:
    PDFiumPageImpl(const PDFiumPageImpl&) = delete;
    PDFiumPageImpl& operator=(const PDFiumPageImpl&) = delete;

public:
    PDFiumPageImpl(FPDF_PAGE pPage)
        : mpPage(pPage)
    {
    }

    ~PDFiumPageImpl() override
    {
        if (mpPage)
            FPDF_ClosePage(mpPage);
    }

    FPDF_PAGE getPointer() { return mpPage; }

    int getObjectCount() override;
    std::unique_ptr<PDFiumPageObject> getObject(int nIndex) override;

    int getAnnotationCount() override;
    int getAnnotationIndex(std::unique_ptr<PDFiumAnnotation> const& rAnnotation) override;

    std::unique_ptr<PDFiumAnnotation> getAnnotation(int nIndex) override;

    std::unique_ptr<PDFiumTextPage> getTextPage() override;

    BitmapChecksum getChecksum(int nMDPPerm) override;

    double getWidth() override;
    double getHeight() override;

    bool hasTransparency() override;

    bool hasLinks() override;

    void onAfterLoadPage(PDFiumDocument* pDoc) override;
};

/// Wrapper around FPDF_FORMHANDLE.
class PDFiumFormHandle final
{
private:
    FPDF_FORMHANDLE mpHandle;

    PDFiumFormHandle(const PDFiumFormHandle&) = delete;
    PDFiumFormHandle& operator=(const PDFiumFormHandle&) = delete;

public:
    PDFiumFormHandle(FPDF_FORMHANDLE pHandle);
    ~PDFiumFormHandle();
    FPDF_FORMHANDLE getPointer();
};

class PDFiumDocumentImpl : public PDFiumDocument
{
private:
    FPDF_DOCUMENT mpPdfDocument;
    FPDF_FORMFILLINFO m_aFormCallbacks;
    std::unique_ptr<PDFiumFormHandle> m_pFormHandle;

private:
    PDFiumDocumentImpl(const PDFiumDocumentImpl&) = delete;
    PDFiumDocumentImpl& operator=(const PDFiumDocumentImpl&) = delete;

public:
    PDFiumDocumentImpl(FPDF_DOCUMENT pPdfDocument);
    ~PDFiumDocumentImpl() override;
    FPDF_FORMHANDLE getFormHandlePointer();

    // Page size in points
    basegfx::B2DSize getPageSize(int nIndex) override;
    int getPageCount() override;
    int getSignatureCount() override;
    int getFileVersion() override;
    bool saveWithVersion(SvMemoryStream& rStream, int nFileVersion) override;

    std::unique_ptr<PDFiumPage> openPage(int nIndex) override;
    std::unique_ptr<PDFiumSignature> getSignature(int nIndex) override;
    std::vector<unsigned int> getTrailerEnds() override;
};

class PDFiumImpl : public PDFium
{
private:
    PDFiumImpl(const PDFiumImpl&) = delete;
    PDFiumImpl& operator=(const PDFiumImpl&) = delete;

    OUString maLastError;

public:
    PDFiumImpl();
    ~PDFiumImpl() override;

    const OUString& getLastError() const override { return maLastError; }

    std::unique_ptr<PDFiumDocument> openDocument(const void* pData, int nSize,
                                                 const OString& rPassword) override;
    PDFErrorType getLastErrorCode() override;
    /// @brief creates bitmap, can reduce size if needed, check nWidth and nHeight
    std::unique_ptr<PDFiumBitmap> createBitmap(int& nWidth, int& nHeight, int nAlpha) override;
};
}

PDFiumImpl::PDFiumImpl()
{
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);
}

PDFiumImpl::~PDFiumImpl() { FPDF_DestroyLibrary(); }

std::unique_ptr<PDFiumDocument> PDFiumImpl::openDocument(const void* pData, int nSize,
                                                         const OString& rPassword)
{
    maLastError = OUString();
    std::unique_ptr<PDFiumDocument> pPDFiumDocument;

    FPDF_BYTESTRING pPassword = nullptr;
    if (!rPassword.isEmpty())
    {
        pPassword = rPassword.getStr();
    }
    FPDF_DOCUMENT pDocument = FPDF_LoadMemDocument(pData, nSize, pPassword);

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
        pPDFiumDocument = std::make_unique<PDFiumDocumentImpl>(pDocument);
    }

    return pPDFiumDocument;
}

PDFErrorType PDFiumImpl::getLastErrorCode()
{
    return static_cast<PDFErrorType>(FPDF_GetLastError());
}

std::unique_ptr<PDFiumBitmap> PDFiumImpl::createBitmap(int& nWidth, int& nHeight, int nAlpha)
{
    std::unique_ptr<PDFiumBitmap> pPDFiumBitmap;

    FPDF_BITMAP pPdfBitmap = FPDFBitmap_Create(nWidth, nHeight, nAlpha);
    if (!pPdfBitmap)
    {
        int nOriginal = nHeight;
        // PDFium cannot create big bitmaps, max 2^14 x 2^14 x 4 bytes per pixel
        if (nHeight > 16384)
            nHeight = 16384;

        if (nWidth > 16384)
        {
            nWidth = 16384.0 / nOriginal * nWidth;
        }

        if (nWidth * nHeight > 16384 * 16384)
        {
            nOriginal = nWidth;
            nHeight = 16384.0 / nOriginal * nHeight;
        }

        pPdfBitmap = FPDFBitmap_Create(nWidth, nHeight, nAlpha);
    }

    if (!pPdfBitmap)
    {
        maLastError = "Failed to create bitmap";
        SAL_WARN("vcl.filter", "PDFiumImpl: " << getLastError());
    }
    else
    {
        pPDFiumBitmap = std::make_unique<PDFiumBitmapImpl>(pPdfBitmap);
    }
    return pPDFiumBitmap;
}

PDFiumSignatureImpl::PDFiumSignatureImpl(FPDF_SIGNATURE pSignature)
    : mpSignature(pSignature)
{
}

std::vector<int> PDFiumSignatureImpl::getByteRange()
{
    int nByteRangeLen = FPDFSignatureObj_GetByteRange(mpSignature, nullptr, 0);
    std::vector<int> aByteRange(nByteRangeLen);
    if (nByteRangeLen <= 0)
    {
        return aByteRange;
    }

    FPDFSignatureObj_GetByteRange(mpSignature, aByteRange.data(), aByteRange.size());
    return aByteRange;
}

int PDFiumSignatureImpl::getDocMDPPermission()
{
    return FPDFSignatureObj_GetDocMDPPermission(mpSignature);
}

std::vector<unsigned char> PDFiumSignatureImpl::getContents()
{
    int nContentsLen = FPDFSignatureObj_GetContents(mpSignature, nullptr, 0);
    std::vector<unsigned char> aContents(nContentsLen);
    if (aContents.empty())
    {
        return aContents;
    }

    FPDFSignatureObj_GetContents(mpSignature, aContents.data(), aContents.size());
    return aContents;
}

OString PDFiumSignatureImpl::getSubFilter()
{
    int nSubFilterLen = FPDFSignatureObj_GetSubFilter(mpSignature, nullptr, 0);
    std::vector<char> aSubFilterBuf(nSubFilterLen);
    FPDFSignatureObj_GetSubFilter(mpSignature, aSubFilterBuf.data(), aSubFilterBuf.size());
    // Buffer is NUL-terminated.
    OString aSubFilter(aSubFilterBuf.data(), aSubFilterBuf.size() - 1);
    return aSubFilter;
}

OUString PDFiumSignatureImpl::getReason()
{
    int nReasonLen = FPDFSignatureObj_GetReason(mpSignature, nullptr, 0);
    OUString aRet;
    if (nReasonLen > 0)
    {
        std::vector<char16_t> aReasonBuf(nReasonLen);
        FPDFSignatureObj_GetReason(mpSignature, aReasonBuf.data(), aReasonBuf.size());
        aRet = OUString(aReasonBuf.data(), aReasonBuf.size() - 1);
    }

    return aRet;
}

util::DateTime PDFiumSignatureImpl::getTime()
{
    util::DateTime aRet;
    int nTimeLen = FPDFSignatureObj_GetTime(mpSignature, nullptr, 0);
    if (nTimeLen <= 0)
    {
        return aRet;
    }

    // Example: "D:20161027100104".
    std::vector<char> aTimeBuf(nTimeLen);
    FPDFSignatureObj_GetTime(mpSignature, aTimeBuf.data(), aTimeBuf.size());
    OString aM(aTimeBuf.data(), aTimeBuf.size() - 1);
    if (aM.startsWith("D:") && aM.getLength() >= 16)
    {
        aRet.Year = o3tl::toInt32(aM.subView(2, 4));
        aRet.Month = o3tl::toInt32(aM.subView(6, 2));
        aRet.Day = o3tl::toInt32(aM.subView(8, 2));
        aRet.Hours = o3tl::toInt32(aM.subView(10, 2));
        aRet.Minutes = o3tl::toInt32(aM.subView(12, 2));
        aRet.Seconds = o3tl::toInt32(aM.subView(14, 2));
    }
    return aRet;
}

PDFiumDocumentImpl::PDFiumDocumentImpl(FPDF_DOCUMENT pPdfDocument)
    : mpPdfDocument(pPdfDocument)
    , m_aFormCallbacks()
{
    m_aFormCallbacks.version = 1;
    m_pFormHandle = std::make_unique<PDFiumFormHandle>(
        FPDFDOC_InitFormFillEnvironment(pPdfDocument, &m_aFormCallbacks));
}

PDFiumDocumentImpl::~PDFiumDocumentImpl()
{
    m_pFormHandle.reset();
    if (mpPdfDocument)
        FPDF_CloseDocument(mpPdfDocument);
}

FPDF_FORMHANDLE PDFiumDocumentImpl::getFormHandlePointer() { return m_pFormHandle->getPointer(); }

std::unique_ptr<PDFiumPage> PDFiumDocumentImpl::openPage(int nIndex)
{
    std::unique_ptr<PDFiumPage> pPDFiumPage;
    FPDF_PAGE pPage = FPDF_LoadPage(mpPdfDocument, nIndex);
    if (pPage)
    {
        pPDFiumPage = std::make_unique<PDFiumPageImpl>(pPage);
    }
    return pPDFiumPage;
}

std::unique_ptr<PDFiumSignature> PDFiumDocumentImpl::getSignature(int nIndex)
{
    std::unique_ptr<PDFiumSignature> pPDFiumSignature;
    FPDF_SIGNATURE pSignature = FPDF_GetSignatureObject(mpPdfDocument, nIndex);
    if (pSignature)
    {
        pPDFiumSignature = std::make_unique<PDFiumSignatureImpl>(pSignature);
    }
    return pPDFiumSignature;
}

std::vector<unsigned int> PDFiumDocumentImpl::getTrailerEnds()
{
    int nNumTrailers = FPDF_GetTrailerEnds(mpPdfDocument, nullptr, 0);
    std::vector<unsigned int> aTrailerEnds(nNumTrailers);
    FPDF_GetTrailerEnds(mpPdfDocument, aTrailerEnds.data(), aTrailerEnds.size());
    return aTrailerEnds;
}

basegfx::B2DSize PDFiumDocumentImpl::getPageSize(int nIndex)
{
    basegfx::B2DSize aSize;
    FS_SIZEF aPDFSize;
    if (FPDF_GetPageSizeByIndexF(mpPdfDocument, nIndex, &aPDFSize))
    {
        aSize = basegfx::B2DSize(aPDFSize.width, aPDFSize.height);
    }
    return aSize;
}

int PDFiumDocumentImpl::getPageCount() { return FPDF_GetPageCount(mpPdfDocument); }

int PDFiumDocumentImpl::getSignatureCount() { return FPDF_GetSignatureCount(mpPdfDocument); }

int PDFiumDocumentImpl::getFileVersion()
{
    int nFileVersion = 0;
    FPDF_GetFileVersion(mpPdfDocument, &nFileVersion);
    return nFileVersion;
}

bool PDFiumDocumentImpl::saveWithVersion(SvMemoryStream& rStream, int nFileVersion)
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

int PDFiumPageImpl::getObjectCount() { return FPDFPage_CountObjects(mpPage); }

std::unique_ptr<PDFiumPageObject> PDFiumPageImpl::getObject(int nIndex)
{
    std::unique_ptr<PDFiumPageObject> pPDFiumPageObject;
    FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(mpPage, nIndex);
    if (pPageObject)
    {
        pPDFiumPageObject = std::make_unique<PDFiumPageObjectImpl>(pPageObject);
    }
    return pPDFiumPageObject;
}

int PDFiumPageImpl::getAnnotationCount() { return FPDFPage_GetAnnotCount(mpPage); }

int PDFiumPageImpl::getAnnotationIndex(std::unique_ptr<PDFiumAnnotation> const& rAnnotation)
{
    auto pAnnotation = static_cast<PDFiumAnnotationImpl*>(rAnnotation.get());
    return FPDFPage_GetAnnotIndex(mpPage, pAnnotation->getPointer());
}

std::unique_ptr<PDFiumAnnotation> PDFiumPageImpl::getAnnotation(int nIndex)
{
    std::unique_ptr<PDFiumAnnotation> pPDFiumAnnotation;
    FPDF_ANNOTATION pAnnotation = FPDFPage_GetAnnot(mpPage, nIndex);
    if (pAnnotation)
    {
        pPDFiumAnnotation = std::make_unique<PDFiumAnnotationImpl>(pAnnotation);
    }
    return pPDFiumAnnotation;
}

std::unique_ptr<PDFiumTextPage> PDFiumPageImpl::getTextPage()
{
    std::unique_ptr<PDFiumTextPage> pPDFiumTextPage;
    FPDF_TEXTPAGE pTextPage = FPDFText_LoadPage(mpPage);
    if (pTextPage)
    {
        pPDFiumTextPage = std::make_unique<PDFiumTextPageImpl>(pTextPage);
    }
    return pPDFiumTextPage;
}

bool PDFiumPageImpl::hasLinks()
{
    // This could be a full iterator, but at the moment we just determine if the list is empty or
    // not.
    int nStartPos = 0;
    FPDF_LINK pLinkAnnot = nullptr;
    return FPDFLink_Enumerate(mpPage, &nStartPos, &pLinkAnnot);
}

void PDFiumPageImpl::onAfterLoadPage(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    FORM_OnAfterLoadPage(mpPage, pDocImpl->getFormHandlePointer());
}

PDFiumPageObjectImpl::PDFiumPageObjectImpl(FPDF_PAGEOBJECT pPageObject)
    : mpPageObject(pPageObject)
{
}

OUString PDFiumPageObjectImpl::getText(std::unique_ptr<PDFiumTextPage> const& rTextPage)
{
    OUString sReturnText;

    auto pTextPage = static_cast<PDFiumTextPageImpl*>(rTextPage.get());
    int nBytes = FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(), nullptr, 0);
    assert(nBytes % 2 == 0);
    nBytes /= 2;

    std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nBytes]);

    int nActualBytes = FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(),
                                           reinterpret_cast<FPDF_WCHAR*>(pText.get()), nBytes * 2);
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

PDFPageObjectType PDFiumPageObjectImpl::getType()
{
    return static_cast<PDFPageObjectType>(FPDFPageObj_GetType(mpPageObject));
}

int PDFiumPageObjectImpl::getFormObjectCount() { return FPDFFormObj_CountObjects(mpPageObject); }

std::unique_ptr<PDFiumPageObject> PDFiumPageObjectImpl::getFormObject(int nIndex)
{
    std::unique_ptr<PDFiumPageObject> pPDFiumFormObject;
    FPDF_PAGEOBJECT pFormObject = FPDFFormObj_GetObject(mpPageObject, nIndex);
    if (pFormObject)
    {
        pPDFiumFormObject = std::make_unique<PDFiumPageObjectImpl>(pFormObject);
    }
    return pPDFiumFormObject;
}

basegfx::B2DHomMatrix PDFiumPageObjectImpl::getMatrix()
{
    basegfx::B2DHomMatrix aB2DMatrix;
    FS_MATRIX matrix;
    if (FPDFPageObj_GetMatrix(mpPageObject, &matrix))
        aB2DMatrix = basegfx::B2DHomMatrix::abcdef(matrix.a, matrix.b, matrix.c, matrix.d, matrix.e,
                                                   matrix.f);
    return aB2DMatrix;
}

basegfx::B2DRectangle PDFiumPageObjectImpl::getBounds()
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

double PDFiumPageObjectImpl::getFontSize()
{
    float nSize{};
    FPDFTextObj_GetFontSize(mpPageObject, &nSize);
    return nSize;
}

OUString PDFiumPageObjectImpl::getFontName()
{
    OUString sFontName;
    const int nFontName = 80 + 1;
    std::unique_ptr<char[]> pFontName(new char[nFontName]); // + terminating null
    FPDF_FONT pFontObject = FPDFTextObj_GetFont(mpPageObject);
    int nFontNameChars = FPDFFont_GetFontName(pFontObject, pFontName.get(), nFontName);
    if (nFontName >= nFontNameChars)
    {
        sFontName = OUString::createFromAscii(pFontName.get());
    }
    return sFontName;
}

PDFTextRenderMode PDFiumPageObjectImpl::getTextRenderMode()
{
    return static_cast<PDFTextRenderMode>(FPDFTextObj_GetTextRenderMode(mpPageObject));
}

Color PDFiumPageObjectImpl::getFillColor()
{
    Color aColor = COL_TRANSPARENT;
    unsigned int nR, nG, nB, nA;
    if (FPDFPageObj_GetFillColor(mpPageObject, &nR, &nG, &nB, &nA))
    {
        aColor = Color(ColorAlpha, nA, nR, nG, nB);
    }
    return aColor;
}

Color PDFiumPageObjectImpl::getStrokeColor()
{
    Color aColor = COL_TRANSPARENT;
    unsigned int nR, nG, nB, nA;
    if (FPDFPageObj_GetStrokeColor(mpPageObject, &nR, &nG, &nB, &nA))
    {
        aColor = Color(ColorAlpha, nA, nR, nG, nB);
    }
    return aColor;
}

double PDFiumPageObjectImpl::getStrokeWidth()
{
    float fWidth = 1;
    FPDFPageObj_GetStrokeWidth(mpPageObject, &fWidth);
    return fWidth;
}

int PDFiumPageObjectImpl::getPathSegmentCount() { return FPDFPath_CountSegments(mpPageObject); }

std::unique_ptr<PDFiumPathSegment> PDFiumPageObjectImpl::getPathSegment(int index)
{
    std::unique_ptr<PDFiumPathSegment> pPDFiumPathSegment;
    FPDF_PATHSEGMENT pPathSegment = FPDFPath_GetPathSegment(mpPageObject, index);
    if (pPathSegment)
    {
        pPDFiumPathSegment = std::make_unique<PDFiumPathSegmentImpl>(pPathSegment);
    }
    return pPDFiumPathSegment;
}

Size PDFiumPageObjectImpl::getImageSize(PDFiumPage& rPage)
{
    FPDF_IMAGEOBJ_METADATA aMeta;
    auto& rPageImpl = static_cast<PDFiumPageImpl&>(rPage);
    FPDFImageObj_GetImageMetadata(mpPageObject, rPageImpl.getPointer(), &aMeta);
    return Size(aMeta.width, aMeta.height);
}

std::unique_ptr<PDFiumBitmap> PDFiumPageObjectImpl::getImageBitmap()
{
    std::unique_ptr<PDFiumBitmap> pPDFiumBitmap;
    FPDF_BITMAP pBitmap = FPDFImageObj_GetBitmap(mpPageObject);
    if (pBitmap)
    {
        pPDFiumBitmap = std::make_unique<PDFiumBitmapImpl>(pBitmap);
    }
    return pPDFiumBitmap;
}

bool PDFiumPageObjectImpl::getDrawMode(PDFFillMode& rFillMode, bool& rStroke)
{
    auto nFillMode = static_cast<int>(rFillMode);
    auto bStroke = static_cast<FPDF_BOOL>(rStroke);
    bool bRet = FPDFPath_GetDrawMode(mpPageObject, &nFillMode, &bStroke);
    rFillMode = static_cast<PDFFillMode>(nFillMode);
    rStroke = static_cast<bool>(bStroke);
    return bRet;
}

BitmapChecksum PDFiumPageImpl::getChecksum(int nMDPPerm)
{
    int nPageWidth = getWidth();
    int nPageHeight = getHeight();
    std::unique_ptr<PDFiumBitmap> pPdfBitmap
        = PDFiumLibrary::get()->createBitmap(nPageWidth, nPageHeight, /*nAlpha=*/1);
    if (!pPdfBitmap)
        return 0;

    PDFiumBitmapImpl* pBitmapImpl = static_cast<PDFiumBitmapImpl*>(pPdfBitmap.get());

    int nFlags = 0;
    if (nMDPPerm != 3)
    {
        // Annotations/commenting should affect the checksum, signature verification wants this.
        nFlags = FPDF_ANNOT;
    }
    FPDF_RenderPageBitmap(pBitmapImpl->getPointer(), mpPage, /*start_x=*/0, /*start_y=*/0,
                          nPageWidth, nPageHeight,
                          /*rotate=*/0, nFlags);
    Bitmap aBitmap(Size(nPageWidth, nPageHeight), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        const auto pPdfBuffer
            = static_cast<ConstScanline>(FPDFBitmap_GetBuffer(pBitmapImpl->getPointer()));
        const int nStride = FPDFBitmap_GetStride(pBitmapImpl->getPointer());
        for (int nRow = 0; nRow < nPageHeight; ++nRow)
        {
            ConstScanline pPdfLine = pPdfBuffer + (nStride * nRow);
            pWriteAccess->CopyScanline(nRow, pPdfLine, ScanlineFormat::N32BitTcBgra, nStride);
        }
    }
    return aBitmap.GetChecksum();
}

double PDFiumPageImpl::getWidth() { return FPDF_GetPageWidth(mpPage); }

double PDFiumPageImpl::getHeight() { return FPDF_GetPageHeight(mpPage); }

bool PDFiumPageImpl::hasTransparency() { return FPDFPage_HasTransparency(mpPage); }

PDFiumPathSegmentImpl::PDFiumPathSegmentImpl(FPDF_PATHSEGMENT pPathSegment)
    : mpPathSegment(pPathSegment)
{
}

basegfx::B2DPoint PDFiumPathSegmentImpl::getPoint() const
{
    basegfx::B2DPoint aPoint;
    float fx, fy;
    if (FPDFPathSegment_GetPoint(mpPathSegment, &fx, &fy))
        aPoint = basegfx::B2DPoint(fx, fy);
    return aPoint;
}

bool PDFiumPathSegmentImpl::isClosed() const { return FPDFPathSegment_GetClose(mpPathSegment); }

PDFSegmentType PDFiumPathSegmentImpl::getType() const
{
    return static_cast<PDFSegmentType>(FPDFPathSegment_GetType(mpPathSegment));
}

PDFiumFormHandle::PDFiumFormHandle(FPDF_FORMHANDLE pHandle)
    : mpHandle(pHandle)
{
}

PDFiumFormHandle::~PDFiumFormHandle() { FPDFDOC_ExitFormFillEnvironment(mpHandle); }

FPDF_FORMHANDLE PDFiumFormHandle::getPointer() { return mpHandle; }

PDFiumBitmapImpl::PDFiumBitmapImpl(FPDF_BITMAP pBitmap)
    : mpBitmap(pBitmap)
{
}

PDFiumBitmapImpl::~PDFiumBitmapImpl()
{
    if (mpBitmap)
    {
        FPDFBitmap_Destroy(mpBitmap);
    }
}

void PDFiumBitmapImpl::fillRect(int left, int top, int width, int height, sal_uInt32 nColor)
{
    FPDFBitmap_FillRect(mpBitmap, left, top, width, height, nColor);
}

void PDFiumBitmapImpl::renderPageBitmap(PDFiumDocument* pDoc, PDFiumPage* pPage, int nStartX,
                                        int nStartY, int nSizeX, int nSizeY)
{
    auto pPageImpl = static_cast<PDFiumPageImpl*>(pPage);
    FPDF_RenderPageBitmap(mpBitmap, pPageImpl->getPointer(), nStartX, nStartY, nSizeX, nSizeY,
                          /*rotate=*/0, /*flags=*/0);

    // Render widget annotations for FormFields.
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    FPDF_FFLDraw(pDocImpl->getFormHandlePointer(), mpBitmap, pPageImpl->getPointer(), nStartX,
                 nStartY, nSizeX, nSizeY, /*rotate=*/0, /*flags=*/0);
}

ConstScanline PDFiumBitmapImpl::getBuffer()
{
    return static_cast<ConstScanline>(FPDFBitmap_GetBuffer(mpBitmap));
}

int PDFiumBitmapImpl::getStride() { return FPDFBitmap_GetStride(mpBitmap); }

int PDFiumBitmapImpl::getWidth() { return FPDFBitmap_GetWidth(mpBitmap); }

int PDFiumBitmapImpl::getHeight() { return FPDFBitmap_GetHeight(mpBitmap); }

PDFBitmapType PDFiumBitmapImpl::getFormat()
{
    return static_cast<PDFBitmapType>(FPDFBitmap_GetFormat(mpBitmap));
}

BitmapEx PDFiumBitmapImpl::createBitmapFromBuffer()
{
    BitmapEx aBitmapEx;

    const vcl::pdf::PDFBitmapType eFormat = getFormat();
    if (eFormat == vcl::pdf::PDFBitmapType::Unknown)
        return aBitmapEx;

    const int nWidth = getWidth();
    const int nHeight = getHeight();
    const int nStride = getStride();

    switch (eFormat)
    {
        case vcl::pdf::PDFBitmapType::BGR:
        {
            aBitmapEx = BitmapEx(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
            ReadRawDIB(aBitmapEx, getBuffer(), ScanlineFormat::N24BitTcBgr, nHeight, nStride);
        }
        break;

        case vcl::pdf::PDFBitmapType::BGRx:
        {
            aBitmapEx = BitmapEx(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
            ReadRawDIB(aBitmapEx, getBuffer(), ScanlineFormat::N32BitTcRgba, nHeight, nStride);
        }
        break;

        case vcl::pdf::PDFBitmapType::BGRA:
        {
            Bitmap aBitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
            AlphaMask aMask(Size(nWidth, nHeight));
            {
                BitmapScopedWriteAccess pWriteAccess(aBitmap);
                BitmapScopedWriteAccess pMaskAccess(aMask);
                ConstScanline pBuffer = getBuffer();
                std::vector<sal_uInt8> aScanlineAlpha(nWidth);
                for (int nRow = 0; nRow < nHeight; ++nRow)
                {
                    ConstScanline pLine = pBuffer + (nStride * nRow);
                    pWriteAccess->CopyScanline(nRow, pLine, ScanlineFormat::N32BitTcBgra, nStride);
                    for (int nCol = 0; nCol < nWidth; ++nCol)
                    {
                        aScanlineAlpha[nCol] = pLine[3];
                        pLine += 4;
                    }
                    pMaskAccess->CopyScanline(nRow, aScanlineAlpha.data(), ScanlineFormat::N8BitPal,
                                              nWidth);
                }
            }
            aBitmapEx = BitmapEx(aBitmap, aMask);
        }
        break;

        default:
            break;
    }

    return aBitmapEx;
}

PDFiumAnnotationImpl::PDFiumAnnotationImpl(FPDF_ANNOTATION pAnnotation)
    : mpAnnotation(pAnnotation)
{
}

PDFiumAnnotationImpl::~PDFiumAnnotationImpl()
{
    if (mpAnnotation)
        FPDFPage_CloseAnnot(mpAnnotation);
}

PDFAnnotationSubType PDFiumAnnotationImpl::getSubType()
{
    return PDFAnnotationSubType(FPDFAnnot_GetSubtype(mpAnnotation));
}

basegfx::B2DRectangle PDFiumAnnotationImpl::getRectangle()
{
    basegfx::B2DRectangle aB2DRectangle;
    FS_RECTF aRect;
    if (FPDFAnnot_GetRect(mpAnnotation, &aRect))
    {
        aB2DRectangle = basegfx::B2DRectangle(aRect.left, aRect.top, aRect.right, aRect.bottom);
    }
    return aB2DRectangle;
}

Color PDFiumAnnotationImpl::getColor()
{
    unsigned int nR, nG, nB, nA;
    if (FPDFAnnot_GetColor(mpAnnotation, FPDFANNOT_COLORTYPE_Color, &nR, &nG, &nB, &nA))
    {
        return Color(ColorAlpha, nA, nR, nG, nB);
    }
    // FPDFAnnot_GetColor can return false if there is an appearance stream
    // So we search for a color with getStrokeColor
    for (int i = 0; i < getObjectCount(); ++i)
    {
        if (getObject(i)->getType() == PDFPageObjectType::Path)
            return getObject(i)->getStrokeColor();
    }
    return COL_TRANSPARENT;
}

Color PDFiumAnnotationImpl::getInteriorColor()
{
    unsigned int nR, nG, nB, nA;
    if (FPDFAnnot_GetColor(mpAnnotation, FPDFANNOT_COLORTYPE_InteriorColor, &nR, &nG, &nB, &nA))
    {
        return Color(ColorAlpha, nA, nR, nG, nB);
    }
    // FPDFAnnot_GetColor can return false if there is an appearance stream
    // So we search for a color with getFillColor
    for (int i = 0; i < getObjectCount(); ++i)
    {
        if (getObject(i)->getType() == PDFPageObjectType::Path)
            return getObject(i)->getFillColor();
    }
    return COL_TRANSPARENT;
}

size_t PDFiumAnnotationImpl::getAttachmentPointsCount()
{
    return FPDFAnnot_CountAttachmentPoints(mpAnnotation);
}

std::vector<basegfx::B2DPoint> PDFiumAnnotationImpl::getAttachmentPoints(size_t nIndex)
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

std::vector<basegfx::B2DPoint> PDFiumAnnotationImpl::getLineGeometry()
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

PDFFormFieldType PDFiumAnnotationImpl::getFormFieldType(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    return PDFFormFieldType(
        FPDFAnnot_GetFormFieldType(pDocImpl->getFormHandlePointer(), mpAnnotation));
}

int PDFiumAnnotationImpl::getFormFieldFlags(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    return FPDFAnnot_GetFormFieldFlags(pDocImpl->getFormHandlePointer(), mpAnnotation);
}

float PDFiumAnnotationImpl::getFontSize(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    float fRet{};
    if (!FPDFAnnot_GetFontSize(pDocImpl->getFormHandlePointer(), mpAnnotation, &fRet))
    {
        return 0.0f;
    }

    return fRet;
}

OUString PDFiumAnnotationImpl::getFormFieldAlternateName(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    OUString aString;
    unsigned long nSize = FPDFAnnot_GetFormFieldAlternateName(pDocImpl->getFormHandlePointer(),
                                                              mpAnnotation, nullptr, 0);
    assert(nSize % 2 == 0);
    nSize /= 2;
    if (nSize > 1)
    {
        std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nSize]);
        unsigned long nStringSize = FPDFAnnot_GetFormFieldAlternateName(
            pDocImpl->getFormHandlePointer(), mpAnnotation,
            reinterpret_cast<FPDF_WCHAR*>(pText.get()), nSize * 2);
        assert(nStringSize % 2 == 0);
        nStringSize /= 2;
        if (nStringSize > 0)
        {
#if defined OSL_BIGENDIAN
            for (unsigned long i = 0; i != nStringSize; ++i)
            {
                pText[i] = OSL_SWAPWORD(pText[i]);
            }
#endif
            aString = OUString(pText.get());
        }
    }
    return aString;
}

OUString PDFiumAnnotationImpl::getFormFieldValue(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    OUString aString;
    unsigned long nSize
        = FPDFAnnot_GetFormFieldValue(pDocImpl->getFormHandlePointer(), mpAnnotation, nullptr, 0);
    assert(nSize % 2 == 0);
    nSize /= 2;
    if (nSize > 1)
    {
        std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nSize]);
        unsigned long nStringSize
            = FPDFAnnot_GetFormFieldValue(pDocImpl->getFormHandlePointer(), mpAnnotation,
                                          reinterpret_cast<FPDF_WCHAR*>(pText.get()), nSize * 2);
        assert(nStringSize % 2 == 0);
        nStringSize /= 2;
        if (nStringSize > 0)
        {
#if defined OSL_BIGENDIAN
            for (unsigned long i = 0; i != nStringSize; ++i)
            {
                pText[i] = OSL_SWAPWORD(pText[i]);
            }
#endif
            aString = OUString(pText.get());
        }
    }
    return aString;
}

OUString PDFiumAnnotationImpl::getFormAdditionalActionJavaScript(PDFiumDocument* pDoc,
                                                                 PDFAnnotAActionType eEvent)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    OUString aString;
    unsigned long nSize = FPDFAnnot_GetFormAdditionalActionJavaScript(
        pDocImpl->getFormHandlePointer(), mpAnnotation, static_cast<int>(eEvent), nullptr, 0);
    assert(nSize % 2 == 0);
    nSize /= 2;
    if (nSize > 1)
    {
        std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nSize]);
        unsigned long nStringSize = FPDFAnnot_GetFormAdditionalActionJavaScript(
            pDocImpl->getFormHandlePointer(), mpAnnotation, static_cast<int>(eEvent),
            reinterpret_cast<FPDF_WCHAR*>(pText.get()), nSize * 2);
        assert(nStringSize % 2 == 0);
        nStringSize /= 2;
        if (nStringSize > 0)
        {
#if defined OSL_BIGENDIAN
            for (unsigned long i = 0; i != nStringSize; ++i)
            {
                pText[i] = OSL_SWAPWORD(pText[i]);
            }
#endif
            aString = OUString(pText.get());
        }
    }
    return aString;
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

float PDFiumAnnotationImpl::getBorderWidth()
{
    float fHorizontalCornerRadius;
    float fVerticalCornerRadius;
    float fBorderWidth;

    if (!getBorderProperties(mpAnnotation, fHorizontalCornerRadius, fVerticalCornerRadius,
                             fBorderWidth))
        return 0.0f;
    return fBorderWidth;
}

basegfx::B2DSize PDFiumAnnotationImpl::getBorderCornerRadius()
{
    float fHorizontalCornerRadius;
    float fVerticalCornerRadius;
    float fBorderWidth;

    if (!getBorderProperties(mpAnnotation, fHorizontalCornerRadius, fVerticalCornerRadius,
                             fBorderWidth))
        return basegfx::B2DSize(0.0, 0.0);
    return basegfx::B2DSize(fHorizontalCornerRadius, fVerticalCornerRadius);
}

bool PDFiumAnnotationImpl::hasKey(OString const& rKey)
{
    return FPDFAnnot_HasKey(mpAnnotation, rKey.getStr());
}

PDFObjectType PDFiumAnnotationImpl::getValueType(OString const& rKey)
{
    return static_cast<PDFObjectType>(FPDFAnnot_GetValueType(mpAnnotation, rKey.getStr()));
}

OUString PDFiumAnnotationImpl::getString(OString const& rKey)
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

std::vector<std::vector<basegfx::B2DPoint>> PDFiumAnnotationImpl::getInkStrokes()
{
    std::vector<std::vector<basegfx::B2DPoint>> aB2DPointList;
    int nInkStrokes = FPDFAnnot_GetInkListCount(mpAnnotation);
    for (int i = 0; i < nInkStrokes; i++)
    {
        std::vector<basegfx::B2DPoint> aB2DPoints;
        int nPoints = FPDFAnnot_GetInkListPath(mpAnnotation, i, nullptr, 0);
        if (nPoints)
        {
            std::vector<FS_POINTF> aPoints(nPoints);
            if (FPDFAnnot_GetInkListPath(mpAnnotation, i, aPoints.data(), aPoints.size()))
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

std::vector<basegfx::B2DPoint> PDFiumAnnotationImpl::getVertices()
{
    std::vector<basegfx::B2DPoint> aB2DPoints;
    int nPoints = FPDFAnnot_GetVertices(mpAnnotation, nullptr, 0);
    if (nPoints)
    {
        std::vector<FS_POINTF> aPoints(nPoints);
        if (FPDFAnnot_GetVertices(mpAnnotation, aPoints.data(), aPoints.size()))
        {
            for (auto const& rPoint : aPoints)
                aB2DPoints.emplace_back(rPoint.x, rPoint.y);
        }
    }
    return aB2DPoints;
}

std::unique_ptr<PDFiumAnnotation> PDFiumAnnotationImpl::getLinked(OString const& rKey)
{
    std::unique_ptr<PDFiumAnnotation> pPDFiumAnnotation;
    FPDF_ANNOTATION pAnnotation = FPDFAnnot_GetLinkedAnnot(mpAnnotation, rKey.getStr());
    if (pAnnotation)
    {
        pPDFiumAnnotation = std::make_unique<PDFiumAnnotationImpl>(pAnnotation);
    }
    return pPDFiumAnnotation;
}

int PDFiumAnnotationImpl::getObjectCount() { return FPDFAnnot_GetObjectCount(mpAnnotation); }

std::unique_ptr<PDFiumPageObject> PDFiumAnnotationImpl::getObject(int nIndex)
{
    std::unique_ptr<PDFiumPageObject> pPDFiumPageObject;
    FPDF_PAGEOBJECT pPageObject = FPDFAnnot_GetObject(mpAnnotation, nIndex);
    if (pPageObject)
    {
        pPDFiumPageObject = std::make_unique<PDFiumPageObjectImpl>(pPageObject);
    }
    return pPDFiumPageObject;
}

PDFiumTextPageImpl::PDFiumTextPageImpl(FPDF_TEXTPAGE pTextPage)
    : mpTextPage(pTextPage)
{
}

PDFiumTextPageImpl::~PDFiumTextPageImpl()
{
    if (mpTextPage)
        FPDFText_ClosePage(mpTextPage);
}

int PDFiumTextPageImpl::countChars() { return FPDFText_CountChars(mpTextPage); }

basegfx::B2DRectangle PDFiumTextPageImpl::getCharBox(int nIndex, double fPageHeight)
{
    double left = 0.0;
    double right = 0.0;
    double bottom = 0.0;
    double top = 0.0;

    if (FPDFText_GetCharBox(mpTextPage, nIndex, &left, &right, &bottom, &top))
    {
        left = convertPointToMm100(left);
        right = convertPointToMm100(right);
        top = fPageHeight - convertPointToMm100(top);
        bottom = fPageHeight - convertPointToMm100(bottom);

        return basegfx::B2DRectangle(left, bottom, right, top);
    }

    return basegfx::B2DRectangle();
}

unsigned int PDFiumTextPageImpl::getUnicode(int index)
{
    return FPDFText_GetUnicode(mpTextPage, index);
}

std::unique_ptr<PDFiumSearchHandle>
PDFiumTextPageImpl::findStart(const OUString& rFindWhat, PDFFindFlags nFlags, sal_Int32 nStartIndex)
{
    FPDF_WIDESTRING pFindWhat = reinterpret_cast<FPDF_WIDESTRING>(rFindWhat.getStr());
    return std::make_unique<vcl::pdf::PDFiumSearchHandleImpl>(
        FPDFText_FindStart(mpTextPage, pFindWhat, static_cast<sal_uInt32>(nFlags), nStartIndex));
}

PDFiumSearchHandleImpl::PDFiumSearchHandleImpl(FPDF_SCHHANDLE pSearchHandle)
    : mpSearchHandle(pSearchHandle)
{
}

PDFiumSearchHandleImpl::~PDFiumSearchHandleImpl()
{
    if (mpSearchHandle)
        FPDFText_FindClose(mpSearchHandle);
}

bool PDFiumSearchHandleImpl::findNext() { return FPDFText_FindNext(mpSearchHandle); }

bool PDFiumSearchHandleImpl::findPrev() { return FPDFText_FindPrev(mpSearchHandle); }

int PDFiumSearchHandleImpl::getSearchResultIndex()
{
    return FPDFText_GetSchResultIndex(mpSearchHandle);
}

int PDFiumSearchHandleImpl::getSearchCount() { return FPDFText_GetSchCount(mpSearchHandle); }

std::shared_ptr<PDFium>& PDFiumLibrary::get()
{
    static std::shared_ptr<PDFium> pInstance = std::make_shared<PDFiumImpl>();
    return pInstance;
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
