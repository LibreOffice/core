/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>

#include <rtl/string.hxx>
#include <tools/json_writer.hxx>

#include <cmath>
#include <optional>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/captiontailprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/font.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitivehelper2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonAlphaGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/BitmapAlphaPrimitive2D.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/primitive2d/ReflectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <primitive2d/textlineprimitive2d.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/utils/bgradient.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/cvtgrf.hxx>
#include <comphelper/base64.hxx>
#include <tools/stream.hxx>

#include <unordered_map>

using namespace drawinglayer::primitive2d;

namespace
{
// TODO: Move to a tool class or maybe BColor itself and reuse elsewhere
OString colorToHex(const basegfx::BColor& rColor)
{
    return "#" + Color(rColor).AsRGBHexString().toUtf8();
}

// the text a form control shows: a date, a time or an amount is formatted from the model's value
// by the control's window only, so a control gets one for the moment, out of sight, as the
// control does itself to draw
OUString shownTextOf(const cpo::uno::Reference<css::awt::XControlModel>& rxModel)
{
    const cpo::uno::Reference<css::beans::XPropertySet> xModel(rxModel, cpo::uno::UNO_QUERY);
    OUString aControlName;
    if (!xModel.is() || !(xModel->getPropertyValue(u"DefaultControl"_ustr) >>= aControlName)
        || aControlName.isEmpty())
        return OUString();

    SolarMutexGuard aGuard;
    const cpo::uno::Reference<cpo::uno::XComponentContext>& xContext(
        comphelper::getProcessComponentContext());
    const cpo::uno::Reference<css::awt::XControl> xControl(
        xContext->getServiceManager()->createInstanceWithContext(aControlName, xContext),
        cpo::uno::UNO_QUERY);
    const OutputDevice* pDevice(Application::GetDefaultDevice());
    vcl::Window* pParent(pDevice ? pDevice->GetOwnerWindow() : nullptr);
    if (!xControl.is() || !pParent)
        return OUString();

    OUString aText;
    try
    {
        xControl->setModel(rxModel);
        if (const cpo::uno::Reference<css::awt::XWindow> xWindow(xControl, cpo::uno::UNO_QUERY);
            xWindow.is())
            xWindow->setVisible(false);
        xControl->createPeer(nullptr, pParent->GetComponentInterface());

        // a date or a time the form has not taken from its default yet shows that default, as
        // the office draws it; it goes to the window only, the control would set the model
        const auto aDefaultOf
            = [&xModel](const OUString& rName, const OUString& rDefault) -> cpo::uno::Any {
            const cpo::uno::Reference<css::beans::XPropertySetInfo> xInfo(
                xModel->getPropertySetInfo());
            if (!xInfo->hasPropertyByName(rName) || !xInfo->hasPropertyByName(rDefault)
                || xModel->getPropertyValue(rName).hasValue())
                return cpo::uno::Any();
            return xModel->getPropertyValue(rDefault);
        };
        css::util::Date aDate;
        if (const cpo::uno::Reference<css::awt::XDateField> xDate(xControl->getPeer(),
                                                                  cpo::uno::UNO_QUERY);
            xDate.is() && (aDefaultOf(u"Date"_ustr, u"DefaultDate"_ustr) >>= aDate))
            xDate->setDate(aDate);
        css::util::Time aTime;
        if (const cpo::uno::Reference<css::awt::XTimeField> xTime(xControl->getPeer(),
                                                                  cpo::uno::UNO_QUERY);
            xTime.is() && (aDefaultOf(u"Time"_ustr, u"DefaultTime"_ustr) >>= aTime))
            xTime->setTime(aTime);

        // the window, as the control answers from a text property where its model has one,
        // empty in a date or a time field
        if (const cpo::uno::Reference<css::awt::XTextComponent> xText(xControl->getPeer(),
                                                                      cpo::uno::UNO_QUERY);
            xText.is())
            aText = xText->getText();
    }
    catch (const cpo::uno::Exception&)
    {
    }
    xControl->dispose();
    return aText;
}

// The color of text. An automatic text color has no fixed value, it is chosen for contrast
// against what lies under the text, so it is written as "auto".
OString textColorToJson(const basegfx::BColor& rColor)
{
    if (rColor.isAutomatic())
        return "auto"_ostr;

    return colorToHex(rColor);
}

// The highlight behind a run of text, as the pixel processors paint the fill color of a text:
// from the ascent above the baseline to the descent below it, over the width of the text, turned
// and sheared with the text.
basegfx::B2DPolygon textHighlightPolygon(const TextSimplePortionPrimitive2D& rPrimitive)
{
    TextLayouterDevice aTextLayouter;
    rPrimitive.createTextLayouter(aTextLayouter);
    const basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose aDecTrans(
        rPrimitive.getTextTransform());

    double fAscent = aTextLayouter.getFontAscent();
    double fDescent = aTextLayouter.getFontDescent();

    // An emphasis mark above or below the text takes a quarter of the text height more.
    if (rPrimitive.getPrimitive2DID() == PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D)
    {
        const auto& rDecorated = static_cast<const TextDecoratedPortionPrimitive2D&>(rPrimitive);
        if (rDecorated.getTextEmphasisMark() != TEXT_FONT_EMPHASIS_MARK_NONE)
        {
            if (rDecorated.getEmphasisMarkAbove())
                fAscent += aTextLayouter.getTextHeight() * 0.25;
            if (rDecorated.getEmphasisMarkBelow())
                fDescent += aTextLayouter.getTextHeight() * 0.25;
        }
    }

    const std::vector<double>& rDXArray = rPrimitive.getDXArray();
    double fWidth = rDXArray.empty() ? aTextLayouter.getTextWidth(rPrimitive.getText(),
                                                                  rPrimitive.getTextPosition(),
                                                                  rPrimitive.getTextLength())
                                     : rDXArray.back();

    // A superscript or a subscript is highlighted at the height of the full size font around the
    // line, and its trailing spaces are not.
    const sal_Int32 nProportionalFontSize = rPrimitive.getProportionalFontSize();
    if (nProportionalFontSize != 100 && nProportionalFontSize > 0)
    {
        const double fScale = 100.0 / nProportionalFontSize;
        const double fEscapementOffset
            = rPrimitive.getEscapement() / -100.0 * aDecTrans.getScale().getY() * fScale;
        fAscent = fEscapementOffset + fAscent * fScale;
        fDescent = fDescent * fScale - fEscapementOffset;

        if (!rDXArray.empty())
        {
            const OUString& rText = rPrimitive.getText();
            const sal_Int32 nFirst = rPrimitive.getTextPosition();
            sal_Int32 nLast = nFirst + rPrimitive.getTextLength() - 1;
            while (nLast >= nFirst && rText[nLast] == ' ')
                nLast--;
            const sal_Int32 nTrimmedLength = nLast - nFirst + 1;
            if (nTrimmedLength <= 0)
                fWidth = 0.0;
            else if (nTrimmedLength < rPrimitive.getTextLength())
                fWidth = rDXArray[nTrimmedLength - 1];
        }
    }

    // The highlight of a line reaches no higher than the tallest ascent of the line.
    const double fMaxAscentFraction = rPrimitive.getFillColorMaxAscentFraction();
    if (fMaxAscentFraction > 0.0 && nProportionalFontSize == 100)
    {
        const double fMaxAscent = fMaxAscentFraction * aDecTrans.getScale().getY();
        if (fAscent > fMaxAscent)
        {
            fDescent *= fMaxAscent / fAscent;
            fAscent = fMaxAscent;
        }
    }

    basegfx::B2DPolygon aRectangle(
        basegfx::utils::createPolygonFromRect(basegfx::B2DRange(0.0, -fAscent, fWidth, fDescent)));
    aRectangle.transform(basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
        aDecTrans.getShearX(), aDecTrans.getRotate(), aDecTrans.getTranslate()));
    return aRectangle;
}

// A stroke with no line join fills each edge as an area of its own, with the caps only at the
// ends of an open line. Canvas has no such stroke.
bool hasNoLineJoin(const drawinglayer::attribute::LineAttribute& rLineAttribute)
{
    return rLineAttribute.getLineJoin() == basegfx::B2DLineJoin::NONE
           && !basegfx::fTools::equalZero(rLineAttribute.getWidth());
}

// A line join is drawn only where two edges meet, so a stroke of single edges looks the same
// with any line join.
bool hasCorner(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
    {
        if (rPolygon.count() > 2 || (rPolygon.isClosed() && rPolygon.count() > 1))
            return true;
    }
    return false;
}
} // end anonymous namespace

namespace drawinglayer
{
void Primitive2dJsonProcessor::setBitmapCache(std::unordered_map<sal_Int64, Graphic>& rCache)
{
    mpBitmapCache = &rCache;
}

void Primitive2dJsonProcessor::setFontCache(
    std::unordered_map<sal_uInt64, BinaryDataContainer>& rCache,
    std::unordered_map<OUString, ResolvedFace>& rFaceByKey)
{
    mpFontCache = &rCache;
    mpFaceByKey = &rFaceByKey;
}

/// Write graphic data as base64 data URL, preferring native browser-supported formats. A graphic
/// that cannot be encoded gets an error attribute naming the reason instead of data.
void Primitive2dJsonProcessor::writeGraphicBase64(tools::JsonWriter& rWriter,
                                                  const Graphic& rGraphic)
{
    if (rGraphic.IsGfxLink())
    {
        GfxLink aLink = rGraphic.GetGfxLink();
        GfxLinkType eType = aLink.GetType();
        const char* pMime = nullptr;

        switch (eType)
        {
            case GfxLinkType::NativeJpg:
                pMime = "data:image/jpeg;base64,";
                break;
            case GfxLinkType::NativePng:
                pMime = "data:image/png;base64,";
                break;
            case GfxLinkType::NativeSvg:
                pMime = "data:image/svg+xml;base64,";
                break;
            case GfxLinkType::NativeGif:
                pMime = "data:image/gif;base64,";
                break;
            case GfxLinkType::NativeWebp:
                pMime = "data:image/webp;base64,";
                break;
            default:
                break;
        }

        if (pMime)
        {
            cpo::uno::Sequence<sal_Int8> aSequence(
                reinterpret_cast<const sal_Int8*>(aLink.GetData()), aLink.GetDataSize());
            OStringBuffer aBase64(pMime);
            comphelper::Base64::encode(aBase64, aSequence);
            rWriter.put("data", aBase64);
            return;
        }
    }

    // Fallback: convert to PNG
    // Maybe we can be smarter what to do here, probably don't need to full resolution if the image is 40MP
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, rGraphic, ConvertDataFormat::PNG) == ERRCODE_NONE)
    {
        cpo::uno::Sequence<sal_Int8> aSequence(static_cast<const sal_Int8*>(aStream.GetData()),
                                               aStream.Tell());
        OStringBuffer aBase64("data:image/png;base64,");
        comphelper::Base64::encode(aBase64, aSequence);
        rWriter.put("data", aBase64);
        return;
    }

    rWriter.put("error", "conversionfailed");
}

/// Write graphic: checksum + either cache or inline base64.
void Primitive2dJsonProcessor::writeGraphicData(const Graphic& rGraphic)
{
    sal_Int64 nChecksum = static_cast<sal_Int64>(rGraphic.GetChecksum());
    mrWriter.put("checksum", nChecksum);

    if (mpBitmapCache)
    {
        mpBitmapCache->emplace(nChecksum, rGraphic);
        return;
    }

    writeGraphicBase64(mrWriter, rGraphic);
}

/// Write bitmap data: writes the bitmap's own checksum and stores
/// the image in the cache. The cache must be set before any bitmap
/// primitive is serialised. Bitmaps are always referenced by
/// checksum on the wire and never carry inline data.
void Primitive2dJsonProcessor::writeBitmapData(const Bitmap& rBitmap)
{
    assert(mpBitmapCache && "bitmap cache must be set before writing bitmap primitives");

    sal_Int64 nChecksum = static_cast<sal_Int64>(rBitmap.GetChecksum());
    mrWriter.put("checksum", nChecksum);
    mpBitmapCache->emplace(nChecksum, Graphic(rBitmap));
}

void Primitive2dJsonProcessor::writeSvgGradientCommon(
    const drawinglayer::primitive2d::SvgGradientHelper& rGradient)
{
    writePathScaled(rGradient.getPolyPolygon());

    // The unit gradient is the x axis from 0 to 1 for a linear gradient and the unit circle for a
    // radial one. A single stop or an empty gradient has no mapping.
    if (rGradient.getCreatesContent() && !rGradient.getSingleEntry())
        writeMatrixScaled("unitToObject", rGradient.createUnitGradientToObjectTransformation());

    switch (rGradient.getSpreadMethod())
    {
        case SpreadMethod::Pad:
            mrWriter.put("spreadMethod", "pad");
            break;
        case SpreadMethod::Reflect:
            mrWriter.put("spreadMethod", "reflect");
            break;
        case SpreadMethod::Repeat:
            mrWriter.put("spreadMethod", "repeat");
            break;
    }

    {
        auto aStopsArray = mrWriter.startArray("colorStops");
        for (const auto& rEntry : rGradient.getGradientEntries())
        {
            auto aStopNode = mrWriter.startStruct();
            mrWriter.put("offset", rEntry.getOffset());
            mrWriter.put("color", colorToHex(rEntry.getColor()));
            if (rEntry.getOpacity() < 1.0)
                mrWriter.put("opacity", rEntry.getOpacity());
        }
    }
}

void Primitive2dJsonProcessor::writeGradient(
    const drawinglayer::attribute::FillGradientAttribute& rGradient, std::string_view sNodeName)
{
    auto aGradientNode = mrWriter.startNode(sNodeName);

    switch (rGradient.getStyle())
    {
        case css::awt::GradientStyle_LINEAR:
            mrWriter.put("style", "linear");
            break;
        case css::awt::GradientStyle_AXIAL:
            mrWriter.put("style", "axial");
            break;
        case css::awt::GradientStyle_RADIAL:
            mrWriter.put("style", "radial");
            break;
        case css::awt::GradientStyle_ELLIPTICAL:
            mrWriter.put("style", "elliptical");
            break;
        case css::awt::GradientStyle_SQUARE:
            mrWriter.put("style", "square");
            break;
        case css::awt::GradientStyle_RECT:
            mrWriter.put("style", "rect");
            break;
        default:
            mrWriter.put("style", "unknown");
            break;
    }

    // Two decimal places of a degree are finer than any angle the user
    // interface offers, and keep a whole number of degrees whole.
    mrWriter.put("angle", std::round(basegfx::rad2deg(rGradient.getAngle()) * 100.0) / 100.0);
    mrWriter.put("border", rGradient.getBorder());
    mrWriter.put("offsetX", rGradient.getOffsetX());
    mrWriter.put("offsetY", rGradient.getOffsetY());

    if (rGradient.getSteps() > 0)
        mrWriter.put("steps", sal_Int64(rGradient.getSteps()));

    // Color stops
    const basegfx::BColorStops& rStops = rGradient.getColorStops();
    if (!rStops.empty())
    {
        auto aStopsArray = mrWriter.startArray("colorStops");
        for (const auto& rStop : rStops)
        {
            auto aStopNode = mrWriter.startStruct();
            mrWriter.put("offset", rStop.getStopOffset());
            mrWriter.put("color", colorToHex(rStop.getStopColor()));
        }
    }
}

Primitive2dJsonProcessor::~Primitive2dJsonProcessor() = default;

void Primitive2dJsonProcessor::putPosition(std::string_view sName, double fValue, bool)
{
    mrWriter.put(sName, fValue);
}

void Primitive2dJsonProcessor::putTextOrigin(double fX, double fY)
{
    mrWriter.put("x", sal_Int64(std::llround(fX)));
    mrWriter.put("y", sal_Int64(std::llround(fY)));
}

void Primitive2dJsonProcessor::putRange(std::string_view sName, const basegfx::B2DRange& rRange,
                                        double fScale)
{
    writeRange(sName, rRange, fScale);
}

void Primitive2dJsonProcessor::putPath(std::string_view sName,
                                       const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    mrWriter.put(sName, basegfx::utils::exportToSvgD(rPolyPolygon, true, true, false));
}

void Primitive2dJsonProcessor::putMatrix(std::string_view sName,
                                         const basegfx::B2DHomMatrix& rMatrix, MatrixKind eKind)
{
    if (eKind == MatrixKind::Children)
        writeMatrixTranslationScaled(sName, rMatrix);
    else
        writeMatrixScaled(sName, rMatrix);
}

void Primitive2dJsonProcessor::beginChildren(const basegfx::B2DHomMatrix&) {}

void Primitive2dJsonProcessor::endChildren() {}

void Primitive2dJsonProcessor::writeRange(std::string_view sName, const basegfx::B2DRange& rRange,
                                          double fScale)
{
    auto aArray = mrWriter.startArray(sName);
    mrWriter.putSimpleValue(rRange.getMinX() * fScale);
    mrWriter.putSimpleValue(rRange.getMinY() * fScale);
    mrWriter.putSimpleValue(rRange.getMaxX() * fScale);
    mrWriter.putSimpleValue(rRange.getMaxY() * fScale);
}

void Primitive2dJsonProcessor::writeHatchScaled(
    const drawinglayer::attribute::FillHatchAttribute& rHatch)
{
    auto aHatchNode = mrWriter.startNode("hatch");

    switch (rHatch.getStyle())
    {
        case drawinglayer::attribute::HatchStyle::Single:
            mrWriter.put("style", "single");
            break;
        case drawinglayer::attribute::HatchStyle::Double:
            mrWriter.put("style", "double");
            break;
        case drawinglayer::attribute::HatchStyle::Triple:
            mrWriter.put("style", "triple");
            break;
    }

    mrWriter.put("distance", rHatch.getDistance() * mfScaleFactor);
    mrWriter.put("angle", rHatch.getAngle());
    mrWriter.put("color", colorToHex(rHatch.getColor()));

    if (rHatch.isFillBackground())
        mrWriter.put("fillBackground", true);
}

void Primitive2dJsonProcessor::writeFillGraphic(
    const drawinglayer::attribute::FillGraphicAttribute& rFillGraphic)
{
    auto aFillNode = mrWriter.startNode("fillGraphic");
    mrWriter.put("tiling", rFillGraphic.getTiling());
    mrWriter.put("offsetX", rFillGraphic.getOffsetX());
    mrWriter.put("offsetY", rFillGraphic.getOffsetY());

    // The graphic range is a position in the unit square of the area being
    // filled, not a length, so it goes out at a scale of one.
    writeRange("graphicRange", rFillGraphic.getGraphicRange(), 1.0);
    writeGraphicData(rFillGraphic.getGraphic());
}

void Primitive2dJsonProcessor::writeLineAttributeScaled(
    const drawinglayer::attribute::LineAttribute& rLineAttribute)
{
    auto aLineNode = mrWriter.startNode("line");
    mrWriter.put("color", colorToHex(rLineAttribute.getColor()));
    mrWriter.put("width", rLineAttribute.getWidth() * mfScaleFactor);

    switch (rLineAttribute.getLineJoin())
    {
        case basegfx::B2DLineJoin::NONE:
            mrWriter.put("linejoin", "none");
            break;
        case basegfx::B2DLineJoin::Bevel:
            mrWriter.put("linejoin", "bevel");
            break;
        case basegfx::B2DLineJoin::Miter:
            mrWriter.put("linejoin", "miter");
            break;
        case basegfx::B2DLineJoin::Round:
            mrWriter.put("linejoin", "round");
            break;
        default:
            mrWriter.put("linejoin", "unknown");
            break;
    }

    switch (rLineAttribute.getLineCap())
    {
        case css::drawing::LineCap::LineCap_BUTT:
            mrWriter.put("linecap", "butt");
            break;
        case css::drawing::LineCap::LineCap_ROUND:
            mrWriter.put("linecap", "round");
            break;
        case css::drawing::LineCap::LineCap_SQUARE:
            mrWriter.put("linecap", "square");
            break;
        default:
            mrWriter.put("linecap", "unknown");
            break;
    }
}

void Primitive2dJsonProcessor::writeStrokeAttributeScaled(
    const drawinglayer::attribute::StrokeAttribute& rStroke)
{
    if (rStroke.isDefault())
        return;

    auto aStrokeNode = mrWriter.startNode("stroke");
    mrWriter.put("fullDotDashLen", rStroke.getFullDotDashLen() * mfScaleFactor);

    const auto& rDashArray = rStroke.getDotDashArray();
    if (!rDashArray.empty())
    {
        auto aDashArrayNode = mrWriter.startArray("dotDashArray");
        for (double fValue : rDashArray)
            mrWriter.putSimpleValue(fValue * mfScaleFactor);
    }
}

void Primitive2dJsonProcessor::writeColorModifier(const basegfx::BColorModifier& rModifier)
{
    mrWriter.put("modifier", rModifier.getModifierName());

    switch (rModifier.getBColorModifierType())
    {
        case basegfx::BColorModifierType::BCMType_replace:
        {
            const auto& rReplace = static_cast<const basegfx::BColorModifier_replace&>(rModifier);
            mrWriter.put("color", colorToHex(rReplace.getBColor()));
        }
        break;

        case basegfx::BColorModifierType::BCMType_interpolate:
        {
            const auto& rInterpolate
                = static_cast<const basegfx::BColorModifier_interpolate&>(rModifier);
            mrWriter.put("color", colorToHex(rInterpolate.getBColor()));
            mrWriter.put("value", rInterpolate.getValue());
        }
        break;

        case basegfx::BColorModifierType::BCMType_saturate:
        case basegfx::BColorModifierType::BCMType_hueRotate:
        {
            // Both are a 3x3 matrix over red, green and blue, row by row.
            const basegfx::B3DHomMatrix& rMatrix
                = rModifier.getBColorModifierType() == basegfx::BColorModifierType::BCMType_saturate
                      ? static_cast<const basegfx::BColorModifier_saturate&>(rModifier).getMatrix()
                      : static_cast<const basegfx::BColorModifier_hueRotate&>(rModifier)
                            .getMatrix();
            auto aArray = mrWriter.startArray("matrix");
            for (sal_uInt16 nRow = 0; nRow < 3; ++nRow)
                for (sal_uInt16 nColumn = 0; nColumn < 3; ++nColumn)
                    mrWriter.putSimpleValue(rMatrix.get(nRow, nColumn));
        }
        break;

        case basegfx::BColorModifierType::BCMType_matrix:
        {
            // A 4x5 matrix over red, green, blue, alpha and one, row by row.
            const auto& rMatrix = static_cast<const basegfx::BColorModifier_matrix&>(rModifier);
            auto aArray = mrWriter.startArray("matrix");
            for (double fValue : rMatrix.getVector())
                mrWriter.putSimpleValue(fValue);
        }
        break;

        case basegfx::BColorModifierType::BCMType_black_and_white:
            mrWriter.put(
                "value",
                static_cast<const basegfx::BColorModifier_black_and_white&>(rModifier).getValue());
            break;

        case basegfx::BColorModifierType::BCMType_gamma:
            mrWriter.put("value",
                         static_cast<const basegfx::BColorModifier_gamma&>(rModifier).getValue());
            break;

        case basegfx::BColorModifierType::BCMType_RGBLuminanceContrast:
        {
            const auto& rAdjust
                = static_cast<const basegfx::BColorModifier_RGBLuminanceContrast&>(rModifier);
            mrWriter.put("red", rAdjust.getRed());
            mrWriter.put("green", rAdjust.getGreen());
            mrWriter.put("blue", rAdjust.getBlue());
            mrWriter.put("luminance", rAdjust.getLuminance());
            mrWriter.put("contrast", rAdjust.getContrast());
        }
        break;

        default:
            break;
    }
}

void Primitive2dJsonProcessor::writeGraphicColorModifiers(const GraphicAttr& rAttributes)
{
    if (!rAttributes.IsAdjusted() && !rAttributes.IsSpecialDrawMode())
        return;

    // A stand-in child nests like the decomposition, draw mode innermost. The list goes out
    // innermost first, the order the colors change in.
    const Primitive2DContainer aEmbedded(create2DColorModifierEmbeddingsAsNeeded(
        Primitive2DContainer{ new GroupPrimitive2D(Primitive2DContainer()) }, rAttributes));

    std::vector<const basegfx::BColorModifier*> aModifiers;
    const BasePrimitive2D* pCurrent = aEmbedded.empty() ? nullptr : aEmbedded[0].get();
    while (auto pModified = dynamic_cast<const ModifiedColorPrimitive2D*>(pCurrent))
    {
        aModifiers.push_back(pModified->getColorModifier().get());
        const Primitive2DContainer& rChildren = pModified->getChildren();
        pCurrent = rChildren.empty() ? nullptr : rChildren[0].get();
    }

    if (aModifiers.empty())
        return;

    auto aModifierArray = mrWriter.startArray("colorModifiers");
    for (auto it = aModifiers.rbegin(); it != aModifiers.rend(); ++it)
    {
        auto aModifierNode = mrWriter.startStruct();
        writeColorModifier(**it);
    }
}

void Primitive2dJsonProcessor::writeBoundsScaled(const BasePrimitive2D& rPrimitive)
{
    const basegfx::B2DRange aRange(rPrimitive.getB2DRange(maViewInformation2D));
    if (!aRange.isEmpty())
        putRange("bounds", aRange, mfScaleFactor);
}

sal_Int32 TextStyleTable::idOf(const OString& rFields)
{
    auto [aIterator, bInserted] = maIds.try_emplace(rFields, sal_Int32(maFields.size()));
    if (bInserted)
        maFields.push_back(rFields);
    return aIterator->second;
}

void Primitive2dJsonProcessor::writeTextStyleFields(tools::JsonWriter& rWriter,
                                                    const TextSimplePortionPrimitive2D& rPrimitive,
                                                    double fFontSize)
{
    rWriter.put("fontSize", fFontSize);
    rWriter.put("fontcolor", textColorToJson(rPrimitive.getFontColor()));

    if (rPrimitive.getTextFillColor() != COL_TRANSPARENT)
        rWriter.put("fillcolor", colorToHex(rPrimitive.getTextFillColor().getBColor()));

    if (rPrimitive.getLetterSpacing() != 0)
        rWriter.put("letterSpacing",
                    static_cast<double>(rPrimitive.getLetterSpacing()) * mfScaleFactor);

    // Font attributes
    const drawinglayer::attribute::FontAttribute& rFontAttr = rPrimitive.getFontAttribute();
    rWriter.put("familyname", rFontAttr.getFamilyName());

    if (!rFontAttr.getStyleName().isEmpty())
        rWriter.put("stylename", rFontAttr.getStyleName());

    rWriter.put("weight", rFontAttr.getWeight());

    if (rFontAttr.getItalic())
        rWriter.put("italic", true);
    if (rFontAttr.getOutline())
        rWriter.put("outline", true);
    if (rFontAttr.getRTL())
        rWriter.put("rtl", true);
    if (rFontAttr.getMonospaced())
        rWriter.put("monospaced", true);

    // The exact face the engine resolved for this portion, named by a
    // content-hash id. The font command serves the bytes on demand.
    if (mpFontCache && mpFaceByKey)
    {
        const OUString aFontKey = rFontAttr.getFamilyName() + u"|"_ustr + rFontAttr.getStyleName()
                                  + u"|"_ustr + OUString::number(rFontAttr.getWeight()) + u"|"_ustr
                                  + OUString::number(rFontAttr.getItalic() ? 1 : 0) + u"|"_ustr
                                  + rPrimitive.getLocale().Language;
        auto aFound = mpFaceByKey->find(aFontKey);
        ResolvedFace aFace = aFound != mpFaceByKey->end() ? aFound->second : ResolvedFace();
        if (aFound == mpFaceByKey->end())
        {
            // The height is arbitrary, the font file bytes do not depend
            // on it. A fresh device keeps the file the face was loaded
            // from, which the shared layout device drops.
            constexpr double fFontExtractHeight = 2000.0;
            ScopedVclPtrInstance<VirtualDevice> pFontDevice;
            const vcl::Font aFont = getVclFontFromFontAttribute(
                rFontAttr, fFontExtractHeight, fFontExtractHeight, 0.0, rPrimitive.getLocale());
            pFontDevice->SetFont(aFont);
            const BinaryDataContainer aFontData = pFontDevice->GetCurrentFontRawData();
            if (!aFontData.isEmpty())
            {
                // The id is the content hash of the font bytes, so identical
                // faces share one cache entry.
                aFace.mnFontId = aFontData.calculateHash();
                mpFontCache->try_emplace(aFace.mnFontId, aFontData);
            }

            // With no bold or italic cut the resolved face is neither and the
            // drawing adds it. The metric describes the face that was picked.
            const FontMetric aMetric = pFontDevice->GetFontMetric();
            aFace.mbSyntheticBold
                = rFontAttr.getWeight() >= WEIGHT_SEMIBOLD && aMetric.GetWeight() < WEIGHT_SEMIBOLD;
            aFace.mbSyntheticItalic = rFontAttr.getItalic() && aMetric.GetItalic() == ITALIC_NONE;

            mpFaceByKey->emplace(aFontKey, aFace);
        }
        if (aFace.mnFontId != 0)
        {
            rWriter.put("fontId", OString::number(aFace.mnFontId, 16));
            if (aFace.mbSyntheticBold)
                rWriter.put("syntheticBold", true);
            if (aFace.mbSyntheticItalic)
                rWriter.put("syntheticItalic", true);
        }
    }
}

void Primitive2dJsonProcessor::writeTextPortionScaled(
    const TextSimplePortionPrimitive2D& rPrimitive)
{
    // Extract font size from the text transform matrix scale
    const basegfx::B2DHomMatrix& rMatrix = rPrimitive.getTextTransform();
    const double fFontScaleY = std::hypot(rMatrix.get(1, 0), rMatrix.get(1, 1)) * mfScaleFactor;
    const OUString& rText = rPrimitive.getText();
    if (mpTextStyles)
    {
        tools::JsonWriter aStyleWriter;
        writeTextStyleFields(aStyleWriter, rPrimitive, fFontScaleY);
        const sal_Int32 nStyle = mpTextStyles->idOf(aStyleWriter.finishAndGetAsOString());
        maUsedTextStyles.insert(nStyle);
        mrWriter.put("style", nStyle);

        // A text that stands upright at the size of its font is placed by its origin alone.
        if (rMatrix.get(1, 0) == 0.0 && rMatrix.get(0, 1) == 0.0
            && rMatrix.get(0, 0) == rMatrix.get(1, 1))
        {
            putTextOrigin(rMatrix.get(0, 2) * mfScaleFactor, rMatrix.get(1, 2) * mfScaleFactor);
        }
        else
            putMatrix("matrix", rMatrix, MatrixKind::Glyphs);

        mrWriter.put("text", rText);
        if (rPrimitive.getTextPosition() != 0)
            mrWriter.put("textPosition", rPrimitive.getTextPosition());
        if (rPrimitive.getTextLength() != rText.getLength() - rPrimitive.getTextPosition())
            mrWriter.put("textLength", rPrimitive.getTextLength());
    }
    else
    {
        putMatrix("matrix", rMatrix, MatrixKind::Glyphs);
        mrWriter.put("text", rText);
        mrWriter.put("textPosition", rPrimitive.getTextPosition());
        mrWriter.put("textLength", rPrimitive.getTextLength());
        writeTextStyleFields(mrWriter, rPrimitive, fFontScaleY);
    }

    if (!rPrimitive.getTextFillColor().IsTransparent())
    {
        auto aHighlightNode = mrWriter.startNode("highlight");
        writePathScaled(basegfx::B2DPolyPolygon(textHighlightPolygon(rPrimitive)));
    }

    // The backdrop is the fill under the text, which an automatic text color is chosen against.
    if (moTextSurface)
    {
        mrWriter.put("backdrop", colorToHex(moTextSurface->getBColor()));
        if (mfTextSurfaceTransparency != 0.0)
            mrWriter.put("backdropTransparency", mfTextSurfaceTransparency);
    }

    // How far each glyph advances from the one before it, in whole twips. A whole twip is finer
    // than a screen pixel, so rounding to it loses nothing a reader can see. The drawing layer
    // measures from the start of the run instead, where the numbers reach four or five digits by
    // the end of a line and a step stays at two or three, and the steps of one face repeat,
    // which compresses better. Each value is the difference of the rounded distances, so adding
    // them up gives the measured distances back in whole twips.
    if (!rPrimitive.getDXArray().empty())
    {
        auto aAdvanceArray = mrWriter.startArray("advances");
        sal_Int64 nPreviousDistance = 0;
        for (double fValue : rPrimitive.getDXArray())
        {
            const sal_Int64 nDistance = std::llround(fValue * mfScaleFactor);
            mrWriter.putSimpleValue(nDistance - nPreviousDistance);
            nPreviousDistance = nDistance;
        }
    }

    // The zero-based index of the paragraph this portion belongs to.
    if (rPrimitive.getParagraph() >= 0)
        mrWriter.put("paragraph", sal_Int64(rPrimitive.getParagraph()));

    // The paragraph alignment of this portion.
    const char* pAlign = nullptr;
    switch (rPrimitive.getAlign())
    {
        case drawinglayer::primitive2d::TextPortionAlign::Left:
            pAlign = "left";
            break;
        case drawinglayer::primitive2d::TextPortionAlign::Right:
            pAlign = "right";
            break;
        case drawinglayer::primitive2d::TextPortionAlign::Center:
            pAlign = "center";
            break;
        case drawinglayer::primitive2d::TextPortionAlign::Block:
            pAlign = "justify";
            break;
        case drawinglayer::primitive2d::TextPortionAlign::Unknown:
            break;
    }
    if (pAlign)
        mrWriter.put("align", pAlign);

    // The height and ascent of the line this portion sits on.
    if (rPrimitive.getLineHeight() > 0.0)
        mrWriter.put("lineHeight", rPrimitive.getLineHeight() * mfScaleFactor);
    if (rPrimitive.getLineAscent() > 0.0)
        mrWriter.put("lineAscent", rPrimitive.getLineAscent() * mfScaleFactor);

    // The size of the area the text is laid out within. Lines wrap at the
    // width.
    if (rPrimitive.getTextAreaWidth() > 0.0)
        mrWriter.put("textAreaWidth", rPrimitive.getTextAreaWidth() * mfScaleFactor);
    if (rPrimitive.getTextAreaHeight() > 0.0)
        mrWriter.put("textAreaHeight", rPrimitive.getTextAreaHeight() * mfScaleFactor);

    // Where this portion's line starts in the text area before alignment, and
    // the width it wraps at.
    if (rPrimitive.getLineWidth() > 0.0)
    {
        mrWriter.put("lineIndent", rPrimitive.getLineIndent() * mfScaleFactor);
        mrWriter.put("lineWidth", rPrimitive.getLineWidth() * mfScaleFactor);
    }
}

// Write matrix as 6 elements. Scale all components (unit conversion).
void Primitive2dJsonProcessor::writeMatrixScaled(std::string_view sName,
                                                 const basegfx::B2DHomMatrix& rMatrix)
{
    auto aArray = mrWriter.startArray(sName);
    mrWriter.putSimpleValue(rMatrix.get(0, 0) * mfScaleFactor);
    mrWriter.putSimpleValue(rMatrix.get(1, 0) * mfScaleFactor);
    mrWriter.putSimpleValue(rMatrix.get(0, 1) * mfScaleFactor);
    mrWriter.putSimpleValue(rMatrix.get(1, 1) * mfScaleFactor);
    mrWriter.putSimpleValue(rMatrix.get(0, 2) * mfScaleFactor);
    mrWriter.putSimpleValue(rMatrix.get(1, 2) * mfScaleFactor);
}

// Write matrix with scale applied only to translation (e, f).
// Use for TransformPrimitive2D / ShadowPrimitive2D where the matrix maps
// child-space -> parent-space and both spaces are in the same units.
// The scale/rotation components (a, b, c, d) are dimensionless ratios.
// Only the translation needs converted.
void Primitive2dJsonProcessor::writeMatrixTranslationScaled(std::string_view sName,
                                                            const basegfx::B2DHomMatrix& rMatrix)
{
    auto aArray = mrWriter.startArray(sName);
    mrWriter.putSimpleValue(rMatrix.get(0, 0));
    mrWriter.putSimpleValue(rMatrix.get(1, 0));
    mrWriter.putSimpleValue(rMatrix.get(0, 1));
    mrWriter.putSimpleValue(rMatrix.get(1, 1));
    mrWriter.putSimpleValue(rMatrix.get(0, 2) * mfScaleFactor);
    mrWriter.putSimpleValue(rMatrix.get(1, 2) * mfScaleFactor);
}

basegfx::B2DPolyPolygon
Primitive2dJsonProcessor::scalePolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    if (mfScaleFactor == 1.0)
        return rPolyPolygon;

    basegfx::B2DHomMatrix aScaleMatrix;
    aScaleMatrix.scale(mfScaleFactor, mfScaleFactor);
    basegfx::B2DPolyPolygon aScaled(rPolyPolygon);
    aScaled.transform(aScaleMatrix);
    return aScaled;
}

void Primitive2dJsonProcessor::writePathScaled(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    const basegfx::B2DPolyPolygon aScaled = scalePolyPolygon(rPolyPolygon);
    putPath("path", aScaled);

    // The polygon carries the scale factor already.
    putRange("bounds", aScaled.getB2DRange(), 1.0);
}

void Primitive2dJsonProcessor::decomposeAndWrite(const Primitive2DContainer& rPrimitive2DSequence)
{
    for (const auto& rPrimitive : rPrimitive2DSequence)
    {
        const BasePrimitive2D* pBasePrimitive = rPrimitive.get();
        if (!pBasePrimitive)
            continue;

        auto aNode = mrWriter.startStruct();
        processPrimitive(*pBasePrimitive);
    }
}

OString Primitive2dJsonProcessor::dumpAsJson(const Primitive2DContainer& rPrimitive2DSequence)
{
    tools::JsonWriter aWriter;
    Primitive2dJsonProcessor aProcessor(aWriter);
    {
        auto aArray = aWriter.startArray("primitives");
        aProcessor.decomposeAndWrite(rPrimitive2DSequence);
    }
    return aWriter.finishAndGetAsOString();
}

bool Primitive2dJsonProcessor::writeFormControl(const ControlPrimitive2D& rControl)
{
    const cpo::uno::Reference<css::beans::XPropertySet> xModel(rControl.getControlModel(),
                                                               cpo::uno::UNO_QUERY);
    const cpo::uno::Reference<css::beans::XPropertySetInfo> xInfo(
        xModel.is() ? xModel->getPropertySetInfo() : nullptr);
    if (!xInfo.is())
        return false;

    // a value the model of that kind does not have stays empty
    const auto aValueOf = [&xModel, &xInfo](const OUString& rName) {
        return xInfo->hasPropertyByName(rName) ? xModel->getPropertyValue(rName) : cpo::uno::Any();
    };

    sal_Int16 nClassId(0);
    aValueOf(u"ClassId"_ustr) >>= nClassId;
    // a formatted field is a text field of the forms that keeps a number format
    const char* pKind(nullptr);
    switch (nClassId)
    {
        case css::form::FormComponentType::COMMANDBUTTON:
            pKind = "pushButton";
            break;
        case css::form::FormComponentType::CHECKBOX:
            pKind = "checkBox";
            break;
        case css::form::FormComponentType::RADIOBUTTON:
            pKind = "optionButton";
            break;
        case css::form::FormComponentType::FIXEDTEXT:
            pKind = "label";
            break;
        case css::form::FormComponentType::GROUPBOX:
            pKind = "groupBox";
            break;
        case css::form::FormComponentType::TEXTFIELD:
            pKind = xInfo->hasPropertyByName(u"FormatKey"_ustr) ? "formattedField" : "textBox";
            break;
        case css::form::FormComponentType::NUMERICFIELD:
            pKind = "numericField";
            break;
        case css::form::FormComponentType::CURRENCYFIELD:
            pKind = "currencyField";
            break;
        case css::form::FormComponentType::DATEFIELD:
            pKind = "dateField";
            break;
        case css::form::FormComponentType::TIMEFIELD:
            pKind = "timeField";
            break;
        case css::form::FormComponentType::PATTERNFIELD:
            pKind = "patternField";
            break;
        case css::form::FormComponentType::FILECONTROL:
            pKind = "fileSelection";
            break;
        case css::form::FormComponentType::COMBOBOX:
            pKind = "comboBox";
            break;
        case css::form::FormComponentType::LISTBOX:
            pKind = "listBox";
            break;
        case css::form::FormComponentType::IMAGEBUTTON:
            pKind = "imageButton";
            break;
        case css::form::FormComponentType::IMAGECONTROL:
            pKind = "imageControl";
            break;
        case css::form::FormComponentType::SCROLLBAR:
            pKind = "scrollBar";
            break;
        case css::form::FormComponentType::SPINBUTTON:
            pKind = "spinButton";
            break;
        case css::form::FormComponentType::GRIDCONTROL:
            pKind = "tableControl";
            break;
        case css::form::FormComponentType::NAVIGATIONBAR:
            pKind = "navigationBar";
            break;
        default:
            return false;
    }

    mrWriter.put("type", "formControl");
    mrWriter.put("kind", pKind);
    writeMatrixScaled("matrix", rControl.getTransform());

    OUString aLabel;
    if (aValueOf(u"Label"_ustr) >>= aLabel)
        mrWriter.put("label", aLabel);
    // 0 not checked, 1 checked, 2 neither
    sal_Int16 nState(0);
    if (aValueOf(u"State"_ustr) >>= nState)
        mrWriter.put("state", sal_Int32(nState));
    bool bEnabled(true);
    aValueOf(u"Enabled"_ustr) >>= bEnabled;
    if (!bEnabled)
        mrWriter.put("disabled", true);
    bool bDefault(false);
    aValueOf(u"DefaultButton"_ustr) >>= bDefault;
    if (bDefault)
        mrWriter.put("defaultButton", true);
    // 0 left, 1 centred, 2 right
    sal_Int16 nAlign(0);
    if (aValueOf(u"Align"_ustr) >>= nAlign)
        mrWriter.put("align", sal_Int32(nAlign));

    sal_Int32 nColor(0);
    if (aValueOf(u"TextColor"_ustr) >>= nColor)
        mrWriter.put("textColor", colorToHex(::Color(ColorTransparency, nColor).getBColor()));
    if (aValueOf(u"BackgroundColor"_ustr) >>= nColor)
        mrWriter.put("background", colorToHex(::Color(ColorTransparency, nColor).getBColor()));

    css::awt::FontDescriptor aFont;
    if (aValueOf(u"FontDescriptor"_ustr) >>= aFont)
    {
        auto aFontNode = mrWriter.startNode("font");
        if (!aFont.Name.isEmpty())
            mrWriter.put("name", aFont.Name);
        // in points
        if (aFont.Height > 0)
            mrWriter.put("height", sal_Int32(aFont.Height));
        if (aFont.Weight > css::awt::FontWeight::NORMAL)
            mrWriter.put("bold", true);
        // an unset slant is upright
        if (aFont.Slant != css::awt::FontSlant_NONE
            && aFont.Slant != css::awt::FontSlant_DONTKNOW)
            mrWriter.put("italic", true);
    }

    // the text as the control shows it, so a date or an amount comes formatted as the office
    // formats it
    if (const cpo::uno::Reference<css::awt::XTextComponent> xText(rControl.getXControl(),
                                                                  cpo::uno::UNO_QUERY);
        xText.is())
        mrWriter.put("text", shownTextOf(rControl.getControlModel()));

    bool bFlag(false);
    if ((aValueOf(u"Dropdown"_ustr) >>= bFlag) && bFlag)
        mrWriter.put("dropdown", true);
    if ((aValueOf(u"Spin"_ustr) >>= bFlag) && bFlag)
        mrWriter.put("spin", true);
    if ((aValueOf(u"MultiLine"_ustr) >>= bFlag) && bFlag)
        mrWriter.put("multiLine", true);
    // a list of which several entries can be selected at once
    if ((aValueOf(u"MultiSelection"_ustr) >>= bFlag) && bFlag)
        mrWriter.put("multiSelection", true);
    // 0 none, 1 3D, 2 flat
    sal_Int16 nBorder(1);
    if ((aValueOf(u"Border"_ustr) >>= nBorder) && nBorder == 0)
        mrWriter.put("border", sal_Int32(0));

    cpo::uno::Sequence<OUString> aItems;
    if ((aValueOf(u"StringItemList"_ustr) >>= aItems) && aItems.hasElements())
    {
        auto aArray = mrWriter.startArray("items");
        for (const OUString& rItem : aItems)
            mrWriter.putSimpleValue(rItem);
    }
    cpo::uno::Sequence<sal_Int16> aSelected;
    if ((aValueOf(u"SelectedItems"_ustr) >>= aSelected) && aSelected.hasElements())
    {
        auto aArray = mrWriter.startArray("selected");
        for (sal_Int16 nItem : aSelected)
            mrWriter.putSimpleValue(sal_Int32(nItem));
    }

    // a scroll bar and a spin button: which way they run and where their value stands
    sal_Int32 nOrientation(0);
    if ((aValueOf(u"Orientation"_ustr) >>= nOrientation)
        && nOrientation == css::awt::ScrollBarOrientation::VERTICAL)
        mrWriter.put("vertical", true);
    sal_Int32 nValue(0);
    const bool bScroll(nClassId == css::form::FormComponentType::SCROLLBAR);
    if (bScroll || nClassId == css::form::FormComponentType::SPINBUTTON)
    {
        const OUString aPrefix(bScroll ? u"Scroll"_ustr : u"Spin"_ustr);
        // the value it holds now, or where it holds none the one it starts with
        if ((aValueOf(aPrefix + u"Value"_ustr) >>= nValue)
            || (aValueOf(u"Default"_ustr + aPrefix + u"Value"_ustr) >>= nValue))
            mrWriter.put("value", nValue);
        if (aValueOf(aPrefix + u"ValueMin"_ustr) >>= nValue)
            mrWriter.put("min", nValue);
        if (aValueOf(aPrefix + u"ValueMax"_ustr) >>= nValue)
            mrWriter.put("max", nValue);
        if (aValueOf(u"VisibleSize"_ustr) >>= nValue)
            mrWriter.put("visible", nValue);
        // how far one step of an arrow and one of a click on the track go
        if (aValueOf(bScroll ? u"LineIncrement"_ustr : u"SpinIncrement"_ustr) >>= nValue)
            mrWriter.put("step", nValue);
        if (bScroll && (aValueOf(u"BlockIncrement"_ustr) >>= nValue))
            mrWriter.put("pageStep", nValue);
    }

    // the labels of the columns of a table control
    if (const cpo::uno::Reference<css::container::XIndexAccess> xColumns(xModel,
                                                                         cpo::uno::UNO_QUERY);
        xColumns.is() && nClassId == css::form::FormComponentType::GRIDCONTROL)
    {
        auto aArray = mrWriter.startArray("columns");
        for (sal_Int32 nColumn = 0; nColumn < xColumns->getCount(); ++nColumn)
        {
            const cpo::uno::Reference<css::beans::XPropertySet> xColumn(
                xColumns->getByIndex(nColumn), cpo::uno::UNO_QUERY);
            OUString aColumnLabel;
            if (xColumn.is())
                xColumn->getPropertyValue(u"Label"_ustr) >>= aColumnLabel;
            mrWriter.putSimpleValue(aColumnLabel);
        }
    }
    return true;
}

void Primitive2dJsonProcessor::processPrimitive(const BasePrimitive2D& rBasePrimitive)
{
    sal_uInt32 nId = rBasePrimitive.getPrimitive2DID();

    // a form control of a kind the client draws itself goes as its model's values
    if (nId == PRIMITIVE2D_ID_CONTROLPRIMITIVE2D
        && writeFormControl(static_cast<const ControlPrimitive2D&>(rBasePrimitive)))
        return;

    // The areas the drawing layer fills for the primitive, as a group.
    const auto writeDecompositionAsGroup = [this, &rBasePrimitive]() {
        mrWriter.put("type", "group");
        Primitive2DContainer aContainer;
        rBasePrimitive.get2DDecomposition(aContainer, maViewInformation2D);
        auto aChildArray = mrWriter.startArray("children");
        decomposeAndWrite(aContainer);
    };

    switch (nId)
    {
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
        {
            auto const& rPrimitive = static_cast<const PolygonHairlinePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polygonHairline");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            const basegfx::B2DPolyPolygon aScaled
                = scalePolyPolygon(basegfx::B2DPolyPolygon(rPrimitive.getB2DPolygon()));
            putPath("path", aScaled);
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonColorPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonColor");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            writePathScaled(rPrimitive.getB2DPolyPolygon());
        }
        break;

        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const BitmapPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "bitmap");
            putMatrix("matrix", rPrimitive.getTransform(), MatrixKind::Image);

            const Bitmap& rBitmap = rPrimitive.getBitmap();
            const Size aSizePixel(rBitmap.GetSizePixel());
            mrWriter.put("width", aSizePixel.getWidth());
            mrWriter.put("height", aSizePixel.getHeight());
            writeBitmapData(rBitmap);
        }
        break;

        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PointArrayPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "pointArray");
            mrWriter.put("color", colorToHex(rPrimitive.getRGBColor()));
            {
                auto aPointsArray = mrWriter.startArray("points");
                for (const auto& rPoint : rPrimitive.getPositions())
                {
                    auto aPointNode = mrWriter.startStruct();
                    mrWriter.put("x", rPoint.getX() * mfScaleFactor);
                    mrWriter.put("y", rPoint.getY() * mfScaleFactor);
                }
            }
        }
        break;

        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const TransformPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "transform");
            putMatrix("matrix", rPrimitive.getTransformation(), MatrixKind::Children);
            {
                auto aChildArray = mrWriter.startArray("children");
                const basegfx::B2DHomMatrix aEnclosing(maEnclosingTransformation);
                maEnclosingTransformation = aEnclosing * rPrimitive.getTransformation();
                beginChildren(rPrimitive.getTransformation());
                decomposeAndWrite(rPrimitive.getChildren());
                endChildren();
                maEnclosingTransformation = aEnclosing;
            }
        }
        break;

        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const MaskPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "mask");
            const basegfx::B2DPolyPolygon aScaled = scalePolyPolygon(rPrimitive.getMask());
            putPath("clip", aScaled);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const GroupPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "group");
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const InvertPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "invert");
            writeBoundsScaled(rPrimitive);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ObjectInfoPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "objectInfo");
            mrWriter.put("name", rPrimitive.getName());
            mrWriter.put("title", rPrimitive.getTitle());
            mrWriter.put("desc", rPrimitive.getDesc());
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_CAPTIONTAILPRIMITIVE2D:
        {
            // the tail of a caption travels named, so a reader can lay it out itself while the
            // caption changes
            const auto& rPrimitive = static_cast<const CaptionTailPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "captionTail");
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const HiddenGeometryPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "hiddenGeometry");

            // What this holds is there to be hit and to give the object its range, not to be seen,
            // and its own decomposition is empty for that reason. The children are written out
            // directly instead.
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const TextSimplePortionPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "textSimplePortion");
            writeTextPortionScaled(rPrimitive);
        }
        break;

        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const TextDecoratedPortionPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "textDecoratedPortion");
            writeTextPortionScaled(rPrimitive);

            // Decoration properties
            if (rPrimitive.getFontUnderline() != TEXT_LINE_NONE)
            {
                mrWriter.put("underline", sal_Int32(rPrimitive.getFontUnderline()));
                // An automatic line color is the color of the text, so the line takes the
                // color the text gets.
                if (!rPrimitive.getTextlineColor().isAutomatic())
                    mrWriter.put("underlineColor", colorToHex(rPrimitive.getTextlineColor()));
                if (rPrimitive.getUnderlineAbove())
                    mrWriter.put("underlineAbove", true);
            }

            if (rPrimitive.getFontOverline() != TEXT_LINE_NONE)
            {
                mrWriter.put("overline", sal_Int32(rPrimitive.getFontOverline()));
                if (!rPrimitive.getOverlineColor().isAutomatic())
                    mrWriter.put("overlineColor", colorToHex(rPrimitive.getOverlineColor()));
            }

            if (rPrimitive.getTextStrikeout() != TEXT_STRIKEOUT_NONE)
                mrWriter.put("strikeout", sal_Int32(rPrimitive.getTextStrikeout()));

            if (rPrimitive.getTextEmphasisMark() != TEXT_FONT_EMPHASIS_MARK_NONE)
            {
                mrWriter.put("emphasisMark", sal_Int32(rPrimitive.getTextEmphasisMark()));
                if (rPrimitive.getEmphasisMarkAbove())
                    mrWriter.put("emphasisMarkAbove", true);
                if (rPrimitive.getEmphasisMarkBelow())
                    mrWriter.put("emphasisMarkBelow", true);
            }

            if (rPrimitive.getTextRelief() != TEXT_RELIEF_NONE)
                mrWriter.put("relief", sal_Int32(rPrimitive.getTextRelief()));

            if (rPrimitive.getShadow())
                mrWriter.put("shadow", true);

            if (rPrimitive.getWordLineMode())
                mrWriter.put("wordLineMode", true);

            // The lines, strikeout and emphasis marks as the drawing layer builds them, without
            // the text, one set per word in word line mode. Those in an automatic color take the
            // text color, so they are a list of their own.
            Primitive2DContainer aDecorations;
            Primitive2DContainer aDecorationsInTextColor;
            const bool bAutomaticFontColor = rPrimitive.getFontColor().isAutomatic();
            const auto appendDecorations = [this, &aDecorations, &aDecorationsInTextColor,
                                            bAutomaticFontColor](
                                               const TextDecoratedPortionPrimitive2D& rPortion) {
                const basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose aDecTrans(
                    rPortion.getTextTransform());
                const Primitive2DContainer& rGeometry
                    = rPortion.getOrCreateDecorationGeometryContent(
                        aDecTrans, rPortion.getText(), rPortion.getTextPosition(),
                        rPortion.getTextLength(), rPortion.getDXArray());
                for (const Primitive2DReference& rDecoration : rGeometry)
                {
                    // A line and a strikeout travel as the strokes and the text they draw. The
                    // strikeout with a character is text, which names an automatic color itself.
                    bool bInTextColor = false;
                    bool bDecompose = false;
                    switch (rDecoration->getPrimitive2DID())
                    {
                        case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D:
                            bInTextColor = static_cast<const TextLinePrimitive2D&>(*rDecoration)
                                               .getLineColor()
                                               .isAutomatic();
                            bDecompose = true;
                            break;
                        case PRIMITIVE2D_ID_TEXTGEOMETRYSTRIKEOUTPRIMITIVE2D:
                            bInTextColor = bAutomaticFontColor;
                            bDecompose = true;
                            break;
                        case PRIMITIVE2D_ID_TEXTCHARACTERSTRIKEOUTPRIMITIVE2D:
                            bDecompose = true;
                            break;
                        case PRIMITIVE2D_ID_TEXTHIERARCHYEMPHASISMARKPRIMITIVE2D:
                            bInTextColor = bAutomaticFontColor;
                            break;
                        default:
                            break;
                    }
                    Primitive2DContainer& rTarget
                        = bInTextColor ? aDecorationsInTextColor : aDecorations;
                    if (bDecompose)
                        rDecoration->get2DDecomposition(rTarget, maViewInformation2D);
                    else
                        rTarget.push_back(rDecoration);
                }
            };
            const Primitive2DContainer& rWords = rPrimitive.getOrCreateBrokenUpText();
            if (rWords.empty())
                appendDecorations(rPrimitive);
            for (const Primitive2DReference& rWord : rWords)
            {
                if (rWord->getPrimitive2DID() == PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D)
                    appendDecorations(static_cast<const TextDecoratedPortionPrimitive2D&>(*rWord));
            }
            if (!aDecorations.empty())
            {
                auto aDecorationArray = mrWriter.startArray("decorations");
                decomposeAndWrite(aDecorations);
            }
            if (!aDecorationsInTextColor.empty())
            {
                auto aDecorationArray = mrWriter.startArray("decorationsInTextColor");
                decomposeAndWrite(aDecorationsInTextColor);
            }
        }
        break;

        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PolygonStrokePrimitive2D&>(rBasePrimitive);
            if (hasNoLineJoin(rPrimitive.getLineAttribute())
                && hasCorner(basegfx::B2DPolyPolygon(rPrimitive.getB2DPolygon())))
            {
                writeDecompositionAsGroup();
                break;
            }
            mrWriter.put("type", "polygonStroke");
            const basegfx::B2DPolyPolygon aScaled
                = scalePolyPolygon(basegfx::B2DPolyPolygon(rPrimitive.getB2DPolygon()));
            putPath("path", aScaled);
            writeLineAttributeScaled(rPrimitive.getLineAttribute());
            writeStrokeAttributeScaled(rPrimitive.getStrokeAttribute());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonStrokePrimitive2D&>(rBasePrimitive);
            if (hasNoLineJoin(rPrimitive.getLineAttribute())
                && hasCorner(rPrimitive.getB2DPolyPolygon()))
            {
                writeDecompositionAsGroup();
                break;
            }
            mrWriter.put("type", "polyPolygonStroke");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeLineAttributeScaled(rPrimitive.getLineAttribute());
            writeStrokeAttributeScaled(rPrimitive.getStrokeAttribute());
        }
        break;

        case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D:
        {
            /*
                A line with an arrow at one end or at both. Where the arrow sits, the line is
                drawn short of the end and the arrow is a polygon of its own, placed and turned
                along the line, which is work of the kind a reader should not have to do. So
                what travels is what it draws: the line as a line and each arrow as a polygon.
            */
            mrWriter.put("type", "polygonStrokeArrow");

            Primitive2DContainer aContainer;
            rBasePrimitive.get2DDecomposition(aContainer, maViewInformation2D);
            if (!aContainer.empty())
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(aContainer);
            }
        }
        break;

        case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FillGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "fillGradient");
            putRange("outputRange", rPrimitive.getOutputRange(), mfScaleFactor);
            putRange("definitionRange", rPrimitive.getDefinitionRange(), mfScaleFactor);
            writeGradient(rPrimitive.getFillGradient());

            if (rPrimitive.hasAlphaGradient())
                writeGradient(rPrimitive.getAlphaGradient(), "alphaGradient");

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonGradient");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            putRange("definitionRange", rPrimitive.getDefinitionRange(), mfScaleFactor);
            writeGradient(rPrimitive.getFillGradient());

            if (rPrimitive.hasAlphaGradient())
                writeGradient(rPrimitive.getAlphaGradient(), "alphaGradient");

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FillHatchPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "fillHatch");
            putRange("outputRange", rPrimitive.getOutputRange(), mfScaleFactor);
            putRange("definitionRange", rPrimitive.getDefinitionRange(), mfScaleFactor);
            mrWriter.put("backgroundColor", colorToHex(rPrimitive.getBColor()));
            writeHatchScaled(rPrimitive.getFillHatch());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonHatchPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonHatch");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            putRange("definitionRange", rPrimitive.getDefinitionRange(), mfScaleFactor);
            mrWriter.put("backgroundColor", colorToHex(rPrimitive.getBackgroundColor()));
            writeHatchScaled(rPrimitive.getFillHatch());
        }
        break;

        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FillGraphicPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "fillGraphic");
            putMatrix("matrix", rPrimitive.getTransformation(), MatrixKind::Image);
            writeFillGraphic(rPrimitive.getFillGraphic());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonGraphicPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonGraphic");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            putRange("definitionRange", rPrimitive.getDefinitionRange(), mfScaleFactor);
            writeFillGraphic(rPrimitive.getFillGraphic());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const GraphicPrimitive2D&>(rBasePrimitive);
            const Graphic& rGraphic = rPrimitive.getGraphicObject().GetGraphic();

            // An SVG image travels as its own data and the browser draws it like a bitmap, with
            // the same crop, alpha, rotation, mirror and color changes.
            const auto& pVectorData = rGraphic.getVectorGraphicData();
            const bool bSvg = pVectorData && pVectorData->getType() == VectorGraphicDataType::Svg;

            // Non-SVG vector graphics should be decomposed into primitives.
            // The decomposition already applies the graphic's transform matrix, so
            // children are in the same coordinate space as all other primitives.
            if (!bSvg && (rGraphic.GetType() == GraphicType::GdiMetafile || pVectorData))
            {
                mrWriter.put("type", "graphic");
                mrWriter.put("vector", true);

                Primitive2DContainer aContainer;
                rPrimitive.get2DDecomposition(aContainer, maViewInformation2D);
                if (!aContainer.empty())
                {
                    auto aChildArray = mrWriter.startArray("children");
                    decomposeAndWrite(aContainer);
                }
                break;
            }

            // Bitmap or SVG graphic: encode as image data
            mrWriter.put("type", "graphic");
            putMatrix("matrix", rPrimitive.getTransform(), MatrixKind::Image);

            const GraphicAttr& rAttributes = rPrimitive.getGraphicAttr();
            if (rAttributes.IsCropped())
            {
                // With a unit object scale, calculateCropScaling returns the reciprocal of the
                // preferred size left after cropping, so each crop distance becomes a fraction
                // of the frame.
                const basegfx::B2DVector aCropScale(
                    rPrimitive.getGraphicObject().calculateCropScaling(
                        1.0, 1.0, rAttributes.GetLeftCrop(), rAttributes.GetTopCrop(),
                        rAttributes.GetRightCrop(), rAttributes.GetBottomCrop()));
                const double fLeft(rAttributes.GetLeftCrop() * aCropScale.getX());
                const double fTop(rAttributes.GetTopCrop() * aCropScale.getY());
                const double fRight(rAttributes.GetRightCrop() * aCropScale.getX());
                const double fBottom(rAttributes.GetBottomCrop() * aCropScale.getY());

                // The rectangle the whole image maps onto, in the frame's unit square.
                auto aImageRectNode = mrWriter.startNode("imageRect");
                mrWriter.put("x", -fLeft);
                mrWriter.put("y", -fTop);
                mrWriter.put("width", 1.0 + fLeft + fRight);
                mrWriter.put("height", 1.0 + fTop + fBottom);
            }

            if (rAttributes.IsTransparent())
                mrWriter.put("alpha", sal_Int32(rAttributes.GetAlpha()));

            if (rAttributes.GetRotation() != 0_deg10)
                mrWriter.put("rotation", rAttributes.GetRotation().get());

            if (rAttributes.IsMirrored())
                mrWriter.put("mirror", sal_Int32(rAttributes.GetMirrorFlags()));

            writeGraphicColorModifiers(rAttributes);

            if (!bSvg)
            {
                const Size aSizePixel(rGraphic.GetSizePixel());
                mrWriter.put("width", sal_Int64(aSizePixel.getWidth()));
                mrWriter.put("height", sal_Int64(aSizePixel.getHeight()));
            }
            writeGraphicData(rGraphic);
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONRGBAPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PolyPolygonRGBAPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonRGBA");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            writePathScaled(rPrimitive.getB2DPolyPolygon());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONALPHAGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonAlphaGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonAlphaGradient");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeGradient(rPrimitive.getAlphaGradient(), "alphaGradient");
        }
        break;

        case PRIMITIVE2D_ID_BITMAPALPHAPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const BitmapAlphaPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "bitmapAlpha");
            putMatrix("matrix", rPrimitive.getTransform(), MatrixKind::Image);

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());

            const Bitmap& rBitmap = rPrimitive.getBitmap();
            const Size aSizePixel(rBitmap.GetSizePixel());
            mrWriter.put("width", aSizePixel.getWidth());
            mrWriter.put("height", aSizePixel.getHeight());
            writeBitmapData(rBitmap);
        }
        break;

        case PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PatternFillPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "patternFill");
            writePathScaled(rPrimitive.getMask());

            // The reference range is relative to the mask's bounds, which act as
            // the unit square, so it goes out at a scale of one.
            putRange("referenceRange", rPrimitive.getReferenceRange(), 1.0);

            const Primitive2DContainer& rChildren = rPrimitive.getChildren();
            if (!rChildren.empty())
            {
                double fSavedScale = mfScaleFactor;
                mfScaleFactor = 1.0;
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rChildren);
                mfScaleFactor = fSavedScale;
            }
        }
        break;

        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const UnifiedTransparencePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "unifiedTransparence");
            mrWriter.put("transparence", rPrimitive.getTransparence());
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const TransparencePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "transparence");
            writeBoundsScaled(rPrimitive);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
            {
                auto aTranspArray = mrWriter.startArray("transparence");
                decomposeAndWrite(rPrimitive.getTransparence());
            }
        }
        break;

        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ModifiedColorPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "modifiedColor");
            writeColorModifier(*rPrimitive.getColorModifier());
            writeBoundsScaled(rPrimitive);

            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ShadowPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "shadow");
            mrWriter.put("color", colorToHex(rPrimitive.getShadowColor()));
            mrWriter.put("blur", rPrimitive.getShadowBlur() * mfScaleFactor);
            writeMatrixTranslationScaled("matrix", rPrimitive.getShadowTransform());
            writeBoundsScaled(rPrimitive);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_GLOWPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const GlowPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "glow");
            const Color aGlowColor(rPrimitive.getGlowColor());
            mrWriter.put("color", colorToHex(aGlowColor.getBColor()));
            if (aGlowColor.IsTransparent())
                mrWriter.put("transparency", 1.0 - aGlowColor.GetAlpha() / 255.0);
            mrWriter.put("radius", rPrimitive.getGlowRadius() * mfScaleFactor);
            writeBoundsScaled(rPrimitive);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const SoftEdgePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "softEdge");
            mrWriter.put("radius", rPrimitive.getRadius() * mfScaleFactor);
            writeBoundsScaled(rPrimitive);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_REFLECTIONPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ReflectionPrimitive2D&>(rBasePrimitive);
            const drawinglayer::attribute::ReflectionAttribute& rReflection
                = rPrimitive.getReflection();
            mrWriter.put("type", "reflection");

            // An invisible reflection, or one of empty content, has no fields.
            const basegfx::B2DRange aContentRange(
                rPrimitive.getChildren().getB2DRange(maViewInformation2D));
            if (rReflection.isDefault() || aContentRange.isEmpty())
                break;

            writeMatrixTranslationScaled("matrix",
                                         rPrimitive.getMirrorTransformation(aContentRange));
            writeRange("range", rPrimitive.getReflectionRange(aContentRange), mfScaleFactor);
            // The transparency is the start one down to the start position, the end one from the
            // end position on, and linear between them. Positions are fractions of the height.
            mrWriter.put("startTransparency", rReflection.getStartTransparency());
            mrWriter.put("startPosition", rReflection.getStartPosition());
            mrWriter.put("endTransparency", rReflection.getEndTransparency());
            mrWriter.put("endPosition", rReflection.getEndPosition());
            if (rReflection.getBlurRadius() > 0)
                mrWriter.put("blur", rReflection.getBlurRadius() * mfScaleFactor);
            writeBoundsScaled(rPrimitive);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const SvgLinearGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "svgLinearGradient");
            putPosition("startX", rPrimitive.getStart().getX() * mfScaleFactor, false);
            putPosition("startY", rPrimitive.getStart().getY() * mfScaleFactor, true);
            putPosition("endX", rPrimitive.getEnd().getX() * mfScaleFactor, false);
            putPosition("endY", rPrimitive.getEnd().getY() * mfScaleFactor, true);
            writeSvgGradientCommon(rPrimitive);
        }
        break;

        case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const SvgRadialGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "svgRadialGradient");
            mrWriter.put("centerX", rPrimitive.getStart().getX() * mfScaleFactor);
            mrWriter.put("centerY", rPrimitive.getStart().getY() * mfScaleFactor);
            mrWriter.put("radius", rPrimitive.getRadius() * mfScaleFactor);

            if (rPrimitive.isFocalSet())
            {
                mrWriter.put("focalX", rPrimitive.getFocal().getX() * mfScaleFactor);
                mrWriter.put("focalY", rPrimitive.getFocal().getY() * mfScaleFactor);

                // The circle of radius r in the unit gradient is centered at this step times
                // (reach - r).
                const basegfx::B2DVector aFocalStep(rPrimitive.getFocal() - rPrimitive.getStart());
                auto aStepArray = mrWriter.startArray("focalStep");
                mrWriter.putSimpleValue(aFocalStep.getX());
                mrWriter.putSimpleValue(aFocalStep.getY());
            }

            writeSvgGradientCommon(rPrimitive);
        }
        break;

        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const BackgroundColorPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "backgroundcolor");
            // An automatic background color is the document background of the view, which is
            // no fixed color, so the primitive then names none.
            if (!rPrimitive.getBColor().isAutomatic())
                mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            if (rPrimitive.getTransparency() > 0.0)
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FilledRectanglePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "filledRectangle");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            putRange("bounds", rPrimitive.getB2DRange(), mfScaleFactor);
        }
        break;

        case PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const LineRectanglePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "lineRectangle");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            putRange("bounds", rPrimitive.getB2DRange(), mfScaleFactor);
        }
        break;

        case PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const SingleLinePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "singleLine");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            putPosition("startX", rPrimitive.getStart().getX() * mfScaleFactor, false);
            putPosition("startY", rPrimitive.getStart().getY() * mfScaleFactor, true);
            putPosition("endX", rPrimitive.getEnd().getX() * mfScaleFactor, false);
            putPosition("endY", rPrimitive.getEnd().getY() * mfScaleFactor, true);
        }
        break;

        case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D:
        {
            // The wave under a misspelled word is sized in device pixels, so what travels is the
            // line it follows and the font height.
            const auto& rPrimitive = static_cast<const WrongSpellPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "wrongSpell");
            mrWriter.put("color", colorToHex(rPrimitive.getColor()));

            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            rPrimitive.getTransformation().decompose(aScale, aTranslate, fRotate, fShearX);
            mrWriter.put("fontHeight", aScale.getY() * mfScaleFactor);

            // The line runs 3% of the font height below the baseline.
            const double fDistance = basegfx::fTools::equalZero(aScale.getY()) ? 0.0 : 0.03;
            const basegfx::B2DPoint aStart(rPrimitive.getTransformation()
                                           * basegfx::B2DPoint(rPrimitive.getStart(), fDistance));
            const basegfx::B2DPoint aStop(rPrimitive.getTransformation()
                                          * basegfx::B2DPoint(rPrimitive.getStop(), fDistance));
            putPosition("startX", aStart.getX() * mfScaleFactor, false);
            putPosition("startY", aStart.getY() * mfScaleFactor, true);
            putPosition("endX", aStop.getX() * mfScaleFactor, false);
            putPosition("endY", aStop.getY() * mfScaleFactor, true);
        }
        break;

        default:
        {
            const char* pTypeName = nullptr;
            switch (nId)
            {
                // Text hierarchy (structural text grouping)
                case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D:
                    pTypeName = "textHierarchyField";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
                    pTypeName = "textHierarchyLine";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
                    pTypeName = "textHierarchyParagraph";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
                    pTypeName = "textHierarchyBlock";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D:
                    pTypeName = "textHierarchyBullet";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYEMPHASISMARKPRIMITIVE2D:
                    pTypeName = "textHierarchyEmphasisMark";
                    break;
                // Other drawinglayer primitives
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D:
                    pTypeName = "pagePreview";
                    break;
                case PRIMITIVE2D_ID_EXCLUSIVEEDITVIEWPRIMITIVE2D:
                    pTypeName = "exclusiveEditView";
                    break;
                default:
                    break;
            }

            if (pTypeName)
            {
                mrWriter.put("type", pTypeName);
                // A field says what kind it is. A hyperlink is a property of the text and not
                // of the geometry, so a reader that holds the geometry alone learns it here.
                if (nId == PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D)
                {
                    const auto& rField
                        = static_cast<const primitive2d::TextHierarchyFieldPrimitive2D&>(
                            rBasePrimitive);
                    switch (rField.getType())
                    {
                        case primitive2d::FIELD_TYPE_URL:
                            mrWriter.put("fieldType", "url");
                            break;
                        case primitive2d::FIELD_TYPE_PAGE:
                            mrWriter.put("fieldType", "page");
                            break;
                        default:
                            break;
                    }
                }
                // A paragraph can be an animation target of its own, so it carries the bounds
                // of the rectangle it covers.
                if (nId == PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D)
                {
                    // The bounds are in the twips of the part, so a paragraph in a table
                    // cell takes the transformations above it.
                    basegfx::B2DRange aRange(rBasePrimitive.getB2DRange(maViewInformation2D));
                    aRange.transform(maEnclosingTransformation);
                    if (!aRange.isEmpty())
                        writeRange("bounds", aRange, mfScaleFactor);
                }
            }
            else if (maExternalPrimitiveWriter
                     && maExternalPrimitiveWriter(rBasePrimitive, mrWriter, mfScaleFactor))
            {
                // The module that defines the primitive wrote its type and fields.
            }
            else
            {
                // Encode module and local ID so we know the origin
                // Modules: 0=drawinglayer, 1=svx, 2=sd, 3=sw, 4=sc
                static constexpr auto aModuleNames
                    = std::to_array<std::string_view>({ "drawinglayer", "svx", "sd", "sw", "sc" });
                sal_uInt32 nModule = nId >> 16;
                sal_uInt32 nLocalId = nId & 0xFFFF;
                std::string_view aModule
                    = nModule < aModuleNames.size() ? aModuleNames[nModule] : "other";
                mrWriter.put("type", rtl::Concat2View(OString::Concat(aModule) + ":"
                                                      + OString::number(sal_Int32(nLocalId))));
            }

            // A page preview shows another page, and a field inside it reads against that page
            // rather than the one being drawn, so its content is decomposed with the shown page
            // as the visualized one, the way the drawing processors do it.
            std::optional<geometry::ViewInformation2D> oOuterViewInformation;
            if (nId == PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D)
            {
                const auto& rPreview
                    = static_cast<const primitive2d::PagePreviewPrimitive2D&>(rBasePrimitive);
                oOuterViewInformation = maViewInformation2D;
                maViewInformation2D.setVisualizedPage(rPreview.getXDrawPage());

                // The objects of the shown page sit on the fill of that page.
                if (const std::optional<Color>& oSurface = rPreview.getPageSurface())
                {
                    mrWriter.put("backdrop", colorToHex(oSurface->getBColor()));
                    if (rPreview.getPageSurfaceTransparency() != 0.0)
                        mrWriter.put("backdropTransparency", rPreview.getPageSurfaceTransparency());
                }
            }

            // The text of a block sits on the surface the block names, and each text portion
            // inside it is written with that surface.
            const std::optional<Color> oOuterTextSurface = moTextSurface;
            const double fOuterTextSurfaceTransparency = mfTextSurfaceTransparency;
            if (nId == PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D)
            {
                const auto& rBlock
                    = static_cast<const TextHierarchyBlockPrimitive2D&>(rBasePrimitive);
                moTextSurface = rBlock.getTextSurface();
                mfTextSurfaceTransparency = rBlock.getTextSurfaceTransparency();
            }
            else if (mbChildTextSurfaceSet)
            {
                // The external writer named the surface under the text of its primitive.
                moTextSurface = moChildTextSurface;
                mfTextSurfaceTransparency = mfChildTextSurfaceTransparency;
                mbChildTextSurfaceSet = false;
            }

            Primitive2DContainer aContainer;
            rBasePrimitive.get2DDecomposition(aContainer, maViewInformation2D);
            if (!aContainer.empty())
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(aContainer);
            }

            if (oOuterViewInformation)
                maViewInformation2D = *oOuterViewInformation;
            moTextSurface = oOuterTextSurface;
            mfTextSurfaceTransparency = fOuterTextSurfaceTransparency;
        }
        break;
    }
}

} // end namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
