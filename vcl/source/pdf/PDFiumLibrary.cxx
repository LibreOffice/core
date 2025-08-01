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
#include <fpdf_attachment.h>
#include <fpdf_structtree.h>

#include <osl/endian.h>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <tools/UnitConversion.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/ustrbuf.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <functional>

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

static_assert(int(vcl::pdf::PDFAnnotationSubType::Unknown) == FPDF_ANNOT_UNKNOWN);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Text) == FPDF_ANNOT_TEXT);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Link) == FPDF_ANNOT_LINK);
static_assert(int(vcl::pdf::PDFAnnotationSubType::FreeText) == FPDF_ANNOT_FREETEXT);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Line) == FPDF_ANNOT_LINE);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Square) == FPDF_ANNOT_SQUARE);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Circle) == FPDF_ANNOT_CIRCLE);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Polygon) == FPDF_ANNOT_POLYGON);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Polyline) == FPDF_ANNOT_POLYLINE);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Highlight) == FPDF_ANNOT_HIGHLIGHT);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Underline) == FPDF_ANNOT_UNDERLINE);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Squiggly) == FPDF_ANNOT_SQUIGGLY);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Strikeout) == FPDF_ANNOT_STRIKEOUT);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Stamp) == FPDF_ANNOT_STAMP);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Caret) == FPDF_ANNOT_CARET);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Ink) == FPDF_ANNOT_INK);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Popup) == FPDF_ANNOT_POPUP);
static_assert(int(vcl::pdf::PDFAnnotationSubType::FileAttachment) == FPDF_ANNOT_FILEATTACHMENT);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Sound) == FPDF_ANNOT_SOUND);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Movie) == FPDF_ANNOT_MOVIE);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Widget) == FPDF_ANNOT_WIDGET);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Screen) == FPDF_ANNOT_SCREEN);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Printermark) == FPDF_ANNOT_PRINTERMARK);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Trapnet) == FPDF_ANNOT_TRAPNET);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Watermark) == FPDF_ANNOT_WATERMARK);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Threed) == FPDF_ANNOT_THREED);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Richmedia) == FPDF_ANNOT_RICHMEDIA);
static_assert(int(vcl::pdf::PDFAnnotationSubType::XFAWidget) == FPDF_ANNOT_XFAWIDGET);
static_assert(int(vcl::pdf::PDFAnnotationSubType::Redact) == FPDF_ANNOT_REDACT);

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

OUString getUnicodeString(std::function<int(FPDF_WCHAR*, unsigned long)> aPDFiumFunctionCall)
{
    OUString sReturnText;

    int nBytes = aPDFiumFunctionCall(nullptr, 0);
    if (nBytes == 0)
        return sReturnText;
    assert(nBytes % 2 == 0);
    nBytes /= 2;

    std::vector<sal_Unicode> pText(nBytes, 0);

    int nActualBytes = aPDFiumFunctionCall(reinterpret_cast<FPDF_WCHAR*>(pText.data()), nBytes * 2);
    assert(nActualBytes % 2 == 0);
    nActualBytes /= 2;
    if (nActualBytes > 1)
    {
#ifdef OSL_BIGENDIAN
        for (int i = 0; i != nActualBytes; ++i)
        {
            pText[i] = OSL_SWAPWORD(pText[i]);
        }
#endif
        sReturnText = OUString(pText.data());
    }

    return sReturnText;
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
    Bitmap createBitmapFromBuffer() override;
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
    Color getFontColor(PDFiumDocument* pDoc) override;
    OUString getFormFieldAlternateName(PDFiumDocument* pDoc) override;
    int getFormFieldFlags(PDFiumDocument* pDoc) override;
    OUString getFormAdditionalActionJavaScript(PDFiumDocument* pDoc,
                                               PDFAnnotAActionType eEvent) override;
    OUString getFormFieldValue(PDFiumDocument* pDoc) override;
    int getOptionCount(PDFiumDocument* pDoc) override;
};

class PDFiumStructureElementImpl final : public PDFiumStructureElement
{
private:
    FPDF_STRUCTELEMENT mpStructureElement;

    PDFiumStructureElementImpl(const PDFiumStructureElementImpl&) = delete;
    PDFiumStructureElementImpl& operator=(const PDFiumStructureElementImpl&) = delete;

public:
    PDFiumStructureElementImpl(FPDF_STRUCTELEMENT pStructureElement);

    OUString getAltText() override;
    OUString getActualText() override;
    OUString getID() override;
    OUString getLang() override;
    OUString getTitle() override;
    OUString getType() override;
    OUString getObjectType() override;

    int getNumberOfChildren() override;
    int getChildMarkedContentID(int nIndex) override;
    std::unique_ptr<PDFiumStructureElement> getChild(int nIndex) override;
    std::unique_ptr<PDFiumStructureElement> getParent() override;
};

class PDFiumStructureTreeImpl final : public PDFiumStructureTree
{
private:
    FPDF_STRUCTTREE mpStructureTree;

    PDFiumStructureTreeImpl(const PDFiumStructureTreeImpl&) = delete;
    PDFiumStructureTreeImpl& operator=(const PDFiumStructureTreeImpl&) = delete;

public:
    PDFiumStructureTreeImpl(FPDF_STRUCTTREE pStructureTree);
    ~PDFiumStructureTreeImpl();

    int getNumberOfChildren() override;
    std::unique_ptr<PDFiumStructureElement> getChild(int nIndex) override;
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

    std::unique_ptr<PDFiumStructureTree> getStructureTree() override;

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

class PDFiumAttachmentImpl final : public PDFiumAttachment
{
private:
    FPDF_ATTACHMENT mpAttachment;
    PDFiumAttachmentImpl(const PDFiumSignatureImpl&) = delete;
    PDFiumAttachmentImpl& operator=(const PDFiumSignatureImpl&) = delete;

public:
    PDFiumAttachmentImpl(FPDF_ATTACHMENT pAttachment)
        : mpAttachment(pAttachment)
    {
    }

    OUString getName() override;
    bool getFile(std::vector<unsigned char>& rOutBuffer) override;
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
    int getAttachmentCount() override;
    int getFileVersion() override;
    bool saveWithVersion(SvMemoryStream& rStream, int nFileVersion, bool bRemoveSecurity) override;

    std::unique_ptr<PDFiumPage> openPage(int nIndex) override;
    std::unique_ptr<PDFiumSignature> getSignature(int nIndex) override;
    std::unique_ptr<PDFiumAttachment> getAttachment(int nIndex) override;
    std::vector<unsigned int> getTrailerEnds() override;
    OUString getBookmarks() override;
};

class PDFiumImpl : public PDFium
{
private:
    PDFiumImpl(const PDFiumImpl&) = delete;
    PDFiumImpl& operator=(const PDFiumImpl&) = delete;

    OUString maLastError;
    void setLastError(OUString const& rErrorString);

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

void PDFiumImpl::setLastError(OUString const& rErrorString)
{
    if (!rErrorString.isEmpty())
    {
        // Report what error was set (useful in test failures)
        SAL_WARN("vcl.filter", "PDFiumImpl Error: '" << rErrorString << "' Error number: "
                                                     << sal_Int32(getLastErrorCode()));
    }
    maLastError = rErrorString;
}

std::unique_ptr<PDFiumDocument> PDFiumImpl::openDocument(const void* pData, int nSize,
                                                         const OString& rPassword)
{
    setLastError(u""_ustr);
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
                setLastError(u"Success"_ustr);
                break;
            case FPDF_ERR_UNKNOWN:
                setLastError(u"Unknown error"_ustr);
                break;
            case FPDF_ERR_FILE:
                setLastError(u"File not found"_ustr);
                break;
            case FPDF_ERR_FORMAT:
                setLastError(u"Input is not a PDF format"_ustr);
                break;
            case FPDF_ERR_PASSWORD:
                setLastError(u"Incorrect password or password is required"_ustr);
                break;
            case FPDF_ERR_SECURITY:
                setLastError(u"Security error"_ustr);
                break;
            case FPDF_ERR_PAGE:
                setLastError(u"Content error"_ustr);
                break;
            default:
                setLastError(u"Unknown error number"_ustr);
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
        setLastError(u"Failed to create bitmap"_ustr);
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

OUString PDFiumAttachmentImpl::getName()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDFAttachment_GetName(mpAttachment, buffer, length);
    });
}

bool PDFiumAttachmentImpl::getFile(std::vector<unsigned char>& rOutBuffer)
{
    rOutBuffer.clear();

    unsigned long nLength{};
    if (!FPDFAttachment_GetFile(mpAttachment, nullptr, 0, &nLength))
        return false;

    rOutBuffer.resize(nLength);
    unsigned long nActualLength{};
    if (!FPDFAttachment_GetFile(mpAttachment, rOutBuffer.data(), nLength, &nActualLength))
        return false;
    rOutBuffer.resize(nActualLength);
    return true;
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

std::unique_ptr<PDFiumAttachment> PDFiumDocumentImpl::getAttachment(int nIndex)
{
    std::unique_ptr<PDFiumAttachment> pPDFiumAttachment;
    FPDF_ATTACHMENT pAttachment = FPDFDoc_GetAttachment(mpPdfDocument, nIndex);
    if (pAttachment)
    {
        pPDFiumAttachment = std::make_unique<PDFiumAttachmentImpl>(pAttachment);
    }
    return pPDFiumAttachment;
}

std::vector<unsigned int> PDFiumDocumentImpl::getTrailerEnds()
{
    int nNumTrailers = FPDF_GetTrailerEnds(mpPdfDocument, nullptr, 0);
    std::vector<unsigned int> aTrailerEnds(nNumTrailers);
    FPDF_GetTrailerEnds(mpPdfDocument, aTrailerEnds.data(), aTrailerEnds.size());
    return aTrailerEnds;
}

static void lcl_getBookmarks(int nLevel, OUStringBuffer& rBuf, FPDF_DOCUMENT pDoc,
                             FPDF_BOOKMARK pBookmark)
{
    // no first child or too much levels
    if (!pBookmark || nLevel > 10)
        return;

    OUString aString;
    int nBytes = FPDFBookmark_GetTitle(pBookmark, nullptr, 0);
    assert(nBytes % 2 == 0);
    nBytes /= 2;

    std::unique_ptr<sal_Unicode[]> pText(new sal_Unicode[nBytes]);

    int nActualBytes = FPDFBookmark_GetTitle(pBookmark, pText.get(), nBytes * 2);
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
        // insert nLevel spaces before the title
        rBuf.append(OUString("          ").subView(0, nLevel));
        aString = OUString(pText.get());
    }

    rBuf.append(aString);
    rBuf.append("\n");

    // get children
    lcl_getBookmarks(nLevel + 1, rBuf, pDoc, FPDFBookmark_GetFirstChild(pDoc, pBookmark));

    // get siblings
    while (nullptr != (pBookmark = FPDFBookmark_GetNextSibling(pDoc, pBookmark)))
        lcl_getBookmarks(nLevel, rBuf, pDoc, pBookmark);
}

OUString PDFiumDocumentImpl::getBookmarks()
{
    OUStringBuffer aBuf;
    FPDF_BOOKMARK pBookmark = FPDFBookmark_GetFirstChild(mpPdfDocument, nullptr);
    lcl_getBookmarks(0, aBuf, mpPdfDocument, pBookmark);
    return aBuf.makeStringAndClear();
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

int PDFiumDocumentImpl::getAttachmentCount() { return FPDFDoc_GetAttachmentCount(mpPdfDocument); }

int PDFiumDocumentImpl::getFileVersion()
{
    int nFileVersion = 0;
    FPDF_GetFileVersion(mpPdfDocument, &nFileVersion);
    return nFileVersion;
}

bool PDFiumDocumentImpl::saveWithVersion(SvMemoryStream& rStream, int nFileVersion,
                                         bool bRemoveSecurity)
{
    CompatibleWriter aWriter(rStream);
    aWriter.version = 1;
    aWriter.WriteBlock = &CompatibleWriterCallback;
    if (!FPDF_SaveWithVersion(mpPdfDocument, &aWriter, bRemoveSecurity ? FPDF_REMOVE_SECURITY : 0,
                              nFileVersion))
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

std::unique_ptr<PDFiumStructureTree> PDFiumPageImpl::getStructureTree()
{
    std::unique_ptr<PDFiumStructureTree> pPDFiumStructureTree;
    FPDF_STRUCTTREE pStructTree = FPDF_StructTree_GetForPage(mpPage);
    if (pStructTree)
    {
        pPDFiumStructureTree = std::make_unique<PDFiumStructureTreeImpl>(pStructTree);
    }
    return pPDFiumStructureTree;
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
    auto pTextPage = static_cast<PDFiumTextPageImpl*>(rTextPage.get());
    return getUnicodeString([this, pTextPage](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDFTextObj_GetText(mpPageObject, pTextPage->getPointer(), buffer, length);
    });
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
    OUString sFamilyName;
    const int nFamilyName = 80 + 1;
    std::unique_ptr<char[]> pFamilyName(new char[nFamilyName]); // + terminating null
    FPDF_FONT pFontObject = FPDFTextObj_GetFont(mpPageObject);
    int nFamilyNameChars = FPDFFont_GetFamilyName(pFontObject, pFamilyName.get(), nFamilyName);
    if (nFamilyName >= nFamilyNameChars)
    {
        sFamilyName = OUString::createFromAscii(pFamilyName.get());
    }
    return sFamilyName;
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

Bitmap PDFiumBitmapImpl::createBitmapFromBuffer()
{
    Bitmap aBitmap;

    const vcl::pdf::PDFBitmapType eFormat = getFormat();
    if (eFormat == vcl::pdf::PDFBitmapType::Unknown)
        return aBitmap;

    const int nWidth = getWidth();
    const int nHeight = getHeight();
    const int nStride = getStride();

    switch (eFormat)
    {
        case vcl::pdf::PDFBitmapType::BGR:
        {
            aBitmap = Bitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
            ReadRawDIB(aBitmap, getBuffer(), ScanlineFormat::N24BitTcBgr, nHeight, nStride);
        }
        break;

        case vcl::pdf::PDFBitmapType::BGRx:
        {
            aBitmap = Bitmap(Size(nWidth, nHeight), vcl::PixelFormat::N24_BPP);
            ReadRawDIB(aBitmap, getBuffer(), ScanlineFormat::N32BitTcRgbx, nHeight, nStride);
        }
        break;

        case vcl::pdf::PDFBitmapType::BGRA:
        {
            aBitmap = Bitmap(Size(nWidth, nHeight), vcl::PixelFormat::N32_BPP);
            {
                BitmapScopedWriteAccess pWriteAccess(aBitmap);
                ConstScanline pBuffer = getBuffer();
                for (int nRow = 0; nRow < nHeight; ++nRow)
                {
                    ConstScanline pLine = pBuffer + (nStride * nRow);
                    pWriteAccess->CopyScanline(nRow, pLine, ScanlineFormat::N32BitTcBgra, nStride);
                }
            }
        }
        break;

        default:
            break;
    }

    return aBitmap;
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

Color PDFiumAnnotationImpl::getFontColor(PDFiumDocument* pDoc)
{
    auto pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    unsigned int nR, nG, nB;
    if (!FPDFAnnot_GetFontColor(pDocImpl->getFormHandlePointer(), mpAnnotation, &nR, &nG, &nB))
    {
        return Color();
    }

    return Color(nR, nG, nB);
}

OUString PDFiumAnnotationImpl::getFormFieldAlternateName(PDFiumDocument* pDoc)
{
    auto* pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    return getUnicodeString([this, pDocImpl](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDFAnnot_GetFormFieldAlternateName(pDocImpl->getFormHandlePointer(), mpAnnotation,
                                                   buffer, length);
    });
}

OUString PDFiumAnnotationImpl::getFormFieldValue(PDFiumDocument* pDoc)
{
    auto* pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    return getUnicodeString([this, pDocImpl](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDFAnnot_GetFormFieldValue(pDocImpl->getFormHandlePointer(), mpAnnotation, buffer,
                                           length);
    });
}
int PDFiumAnnotationImpl::getOptionCount(PDFiumDocument* pDoc)
{
    auto* pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    return FPDFAnnot_GetOptionCount(pDocImpl->getFormHandlePointer(), mpAnnotation);
}

OUString PDFiumAnnotationImpl::getFormAdditionalActionJavaScript(PDFiumDocument* pDoc,
                                                                 PDFAnnotAActionType eEvent)
{
    auto* pDocImpl = static_cast<PDFiumDocumentImpl*>(pDoc);
    return getUnicodeString([this, pDocImpl, eEvent](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDFAnnot_GetFormAdditionalActionJavaScript(pDocImpl->getFormHandlePointer(),
                                                           mpAnnotation, static_cast<int>(eEvent),
                                                           buffer, length);
    });
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
    return getUnicodeString([this, rKey](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDFAnnot_GetStringValue(mpAnnotation, rKey.getStr(), buffer, length);
    });
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

PDFiumStructureElementImpl::PDFiumStructureElementImpl(FPDF_STRUCTELEMENT pStructureElement)
    : mpStructureElement(pStructureElement)
{
}

OUString PDFiumStructureElementImpl::getAltText()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetAltText(mpStructureElement, buffer, length);
    });
}

OUString PDFiumStructureElementImpl::getActualText()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetActualText(mpStructureElement, buffer, length);
    });
}

OUString PDFiumStructureElementImpl::getID()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetID(mpStructureElement, buffer, length);
    });
}

OUString PDFiumStructureElementImpl::getLang()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetLang(mpStructureElement, buffer, length);
    });
}

OUString PDFiumStructureElementImpl::getType()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetType(mpStructureElement, buffer, length);
    });
}

OUString PDFiumStructureElementImpl::getObjectType()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetObjType(mpStructureElement, buffer, length);
    });
}

int PDFiumStructureElementImpl::getChildMarkedContentID(int nIndex)
{
    return FPDF_StructElement_GetChildMarkedContentID(mpStructureElement, nIndex);
}

OUString PDFiumStructureElementImpl::getTitle()
{
    return getUnicodeString([this](FPDF_WCHAR* buffer, unsigned long length) {
        return FPDF_StructElement_GetTitle(mpStructureElement, buffer, length);
    });
}

int PDFiumStructureElementImpl::getNumberOfChildren()
{
    return FPDF_StructElement_CountChildren(mpStructureElement);
}

std::unique_ptr<PDFiumStructureElement> PDFiumStructureElementImpl::getChild(int nIndex)
{
    std::unique_ptr<PDFiumStructureElement> pPDFiumStructureElement;
    FPDF_STRUCTELEMENT pElement = FPDF_StructElement_GetChildAtIndex(mpStructureElement, nIndex);
    if (pElement)
    {
        pPDFiumStructureElement = std::make_unique<PDFiumStructureElementImpl>(pElement);
    }
    return pPDFiumStructureElement;
}

std::unique_ptr<PDFiumStructureElement> PDFiumStructureElementImpl::getParent()
{
    std::unique_ptr<PDFiumStructureElement> pPDFiumStructureElement;
    FPDF_STRUCTELEMENT pElement = FPDF_StructElement_GetParent(mpStructureElement);
    if (pElement)
    {
        pPDFiumStructureElement = std::make_unique<PDFiumStructureElementImpl>(pElement);
    }
    return pPDFiumStructureElement;
}

PDFiumStructureTreeImpl::PDFiumStructureTreeImpl(FPDF_STRUCTTREE pStructureTree)
    : mpStructureTree(pStructureTree)
{
}

PDFiumStructureTreeImpl::~PDFiumStructureTreeImpl()
{
    if (mpStructureTree)
        FPDF_StructTree_Close(mpStructureTree);
}

int PDFiumStructureTreeImpl::getNumberOfChildren()
{
    return FPDF_StructTree_CountChildren(mpStructureTree);
}

std::unique_ptr<PDFiumStructureElement> PDFiumStructureTreeImpl::getChild(int nIndex)
{
    std::unique_ptr<PDFiumStructureElement> pPDFiumStructureElement;
    FPDF_STRUCTELEMENT pElement = FPDF_StructTree_GetChildAtIndex(mpStructureTree, nIndex);
    if (pElement)
    {
        pPDFiumStructureElement = std::make_unique<PDFiumStructureElementImpl>(pElement);
    }
    return pPDFiumStructureElement;
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
