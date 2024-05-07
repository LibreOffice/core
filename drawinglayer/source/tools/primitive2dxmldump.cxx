/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/tools/primitive2dxmldump.hxx>

#include <rtl/string.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

#include <math.h>
#include <memory>
#include <libxml/parser.h>
#include <sal/log.hxx>

#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <primitive2d/textlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/primitive3d/Tools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/sdrextrudeprimitive3d.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>

using namespace drawinglayer::primitive2d;

namespace drawinglayer
{
namespace
{
const size_t constMaxActionType = 513;

OUString convertColorToString(const basegfx::BColor& rColor)
{
    OUString aRGBString = Color(rColor).AsRGBHexString();
    return "#" + aRGBString;
}

void writeMatrix(::tools::XmlWriter& rWriter, const basegfx::B2DHomMatrix& rMatrix)
{
    rWriter.attribute("xy11", rMatrix.get(0, 0));
    rWriter.attribute("xy12", rMatrix.get(0, 1));
    rWriter.attribute("xy13", rMatrix.get(0, 2));
    rWriter.attribute("xy21", rMatrix.get(1, 0));
    rWriter.attribute("xy22", rMatrix.get(1, 1));
    rWriter.attribute("xy23", rMatrix.get(1, 2));
    rWriter.attribute("xy31", 0);
    rWriter.attribute("xy32", 0);
    rWriter.attribute("xy33", 1);
}

void writeMatrix3D(::tools::XmlWriter& rWriter, const basegfx::B3DHomMatrix& rMatrix)
{
    rWriter.attribute("xy11", rMatrix.get(0, 0));
    rWriter.attribute("xy12", rMatrix.get(0, 1));
    rWriter.attribute("xy13", rMatrix.get(0, 2));
    rWriter.attribute("xy14", rMatrix.get(0, 3));
    rWriter.attribute("xy21", rMatrix.get(1, 0));
    rWriter.attribute("xy22", rMatrix.get(1, 1));
    rWriter.attribute("xy23", rMatrix.get(1, 2));
    rWriter.attribute("xy24", rMatrix.get(1, 3));
    rWriter.attribute("xy31", rMatrix.get(2, 0));
    rWriter.attribute("xy32", rMatrix.get(2, 1));
    rWriter.attribute("xy33", rMatrix.get(2, 2));
    rWriter.attribute("xy34", rMatrix.get(2, 3));
    rWriter.attribute("xy41", rMatrix.get(3, 0));
    rWriter.attribute("xy42", rMatrix.get(3, 1));
    rWriter.attribute("xy43", rMatrix.get(3, 2));
    rWriter.attribute("xy44", rMatrix.get(3, 3));
}

void writePolyPolygon(::tools::XmlWriter& rWriter, const basegfx::B2DPolyPolygon& rB2DPolyPolygon)
{
    rWriter.startElement("polypolygon");
    const basegfx::B2DRange aB2DRange(rB2DPolyPolygon.getB2DRange());
    rWriter.attributeDouble("height", aB2DRange.getHeight());
    rWriter.attributeDouble("width", aB2DRange.getWidth());
    rWriter.attributeDouble("minx", aB2DRange.getMinX());
    rWriter.attributeDouble("miny", aB2DRange.getMinY());
    rWriter.attributeDouble("maxx", aB2DRange.getMaxX());
    rWriter.attributeDouble("maxy", aB2DRange.getMaxY());
    rWriter.attribute("path", basegfx::utils::exportToSvgD(rB2DPolyPolygon, true, true, false));

    for (basegfx::B2DPolygon const& rPolygon : rB2DPolyPolygon)
    {
        rWriter.startElement("polygon");
        for (sal_uInt32 i = 0; i < rPolygon.count(); ++i)
        {
            basegfx::B2DPoint const& rPoint = rPolygon.getB2DPoint(i);

            rWriter.startElement("point");
            rWriter.attribute("x", OUString::number(rPoint.getX()));
            rWriter.attribute("y", OUString::number(rPoint.getY()));
            rWriter.endElement();
        }
        rWriter.endElement();
    }

    rWriter.endElement();
}

void writeStrokeAttribute(::tools::XmlWriter& rWriter,
                          const drawinglayer::attribute::StrokeAttribute& rStrokeAttribute)
{
    if (!rStrokeAttribute.getDotDashArray().empty())
    {
        rWriter.startElement("stroke");

        OUString sDotDash;
        for (double fDotDash : rStrokeAttribute.getDotDashArray())
        {
            sDotDash += OUString::number(lround(fDotDash)) + " ";
        }
        rWriter.attribute("dotDashArray", sDotDash);
        rWriter.attribute("fullDotDashLength", rStrokeAttribute.getFullDotDashLen());
        rWriter.endElement();
    }
}

void writeLineAttribute(::tools::XmlWriter& rWriter,
                        const drawinglayer::attribute::LineAttribute& rLineAttribute)
{
    rWriter.startElement("line");
    rWriter.attribute("color", convertColorToString(rLineAttribute.getColor()));
    rWriter.attribute("width", rLineAttribute.getWidth());
    switch (rLineAttribute.getLineJoin())
    {
        case basegfx::B2DLineJoin::NONE:
            rWriter.attribute("linejoin", "NONE"_ostr);
            break;
        case basegfx::B2DLineJoin::Bevel:
            rWriter.attribute("linejoin", "Bevel"_ostr);
            break;
        case basegfx::B2DLineJoin::Miter:
        {
            rWriter.attribute("linejoin", "Miter"_ostr);
            rWriter.attribute("miterangle",
                              basegfx::rad2deg(rLineAttribute.getMiterMinimumAngle()));
            break;
        }
        case basegfx::B2DLineJoin::Round:
            rWriter.attribute("linejoin", "Round"_ostr);
            break;
        default:
            rWriter.attribute("linejoin", "Unknown"_ostr);
            break;
    }
    switch (rLineAttribute.getLineCap())
    {
        case css::drawing::LineCap::LineCap_BUTT:
            rWriter.attribute("linecap", "BUTT"_ostr);
            break;
        case css::drawing::LineCap::LineCap_ROUND:
            rWriter.attribute("linecap", "ROUND"_ostr);
            break;
        case css::drawing::LineCap::LineCap_SQUARE:
            rWriter.attribute("linecap", "SQUARE"_ostr);
            break;
        default:
            rWriter.attribute("linecap", "Unknown"_ostr);
            break;
    }

    rWriter.endElement();
}

void writeSdrLineAttribute(::tools::XmlWriter& rWriter,
                           const drawinglayer::attribute::SdrLineAttribute& rLineAttribute)
{
    if (rLineAttribute.isDefault())
        return;

    rWriter.startElement("line");
    rWriter.attribute("color", convertColorToString(rLineAttribute.getColor()));
    rWriter.attribute("width", rLineAttribute.getWidth());
    rWriter.attribute("transparence", rLineAttribute.getTransparence());

    switch (rLineAttribute.getJoin())
    {
        case basegfx::B2DLineJoin::NONE:
            rWriter.attribute("linejoin", "NONE"_ostr);
            break;
        case basegfx::B2DLineJoin::Bevel:
            rWriter.attribute("linejoin", "Bevel"_ostr);
            break;
        case basegfx::B2DLineJoin::Miter:
            rWriter.attribute("linejoin", "Miter"_ostr);
            break;
        case basegfx::B2DLineJoin::Round:
            rWriter.attribute("linejoin", "Round"_ostr);
            break;
        default:
            rWriter.attribute("linejoin", "Unknown"_ostr);
            break;
    }
    switch (rLineAttribute.getCap())
    {
        case css::drawing::LineCap::LineCap_BUTT:
            rWriter.attribute("linecap", "BUTT"_ostr);
            break;
        case css::drawing::LineCap::LineCap_ROUND:
            rWriter.attribute("linecap", "ROUND"_ostr);
            break;
        case css::drawing::LineCap::LineCap_SQUARE:
            rWriter.attribute("linecap", "SQUARE"_ostr);
            break;
        default:
            rWriter.attribute("linecap", "Unknown"_ostr);
            break;
    }

    if (!rLineAttribute.getDotDashArray().empty())
    {
        OUString sDotDash;
        for (double fDotDash : rLineAttribute.getDotDashArray())
        {
            sDotDash += OUString::number(fDotDash) + " ";
        }
        rWriter.attribute("dotDashArray", sDotDash);
        rWriter.attribute("fullDotDashLength", rLineAttribute.getFullDotDashLen());
    }

    rWriter.endElement();
}

void writeSdrFillAttribute(::tools::XmlWriter& rWriter,
                           const drawinglayer::attribute::SdrFillAttribute& rFillAttribute)
{
    if (rFillAttribute.isDefault())
        return;

    rWriter.startElement("fill");
    rWriter.attribute("color", convertColorToString(rFillAttribute.getColor()));
    rWriter.attribute("transparence", rFillAttribute.getTransparence());

    auto const& rGradient = rFillAttribute.getGradient();
    if (!rGradient.isDefault())
    {
        rWriter.startElement("gradient");
        switch (rGradient.getStyle())
        {
            default: // GradientStyle_MAKE_FIXED_SIZE
            case css::awt::GradientStyle_LINEAR:
                rWriter.attribute("style", "Linear"_ostr);
                break;
            case css::awt::GradientStyle_AXIAL:
                rWriter.attribute("style", "Axial"_ostr);
                break;
            case css::awt::GradientStyle_RADIAL:
                rWriter.attribute("style", "Radial"_ostr);
                break;
            case css::awt::GradientStyle_ELLIPTICAL:
                rWriter.attribute("style", "Elliptical"_ostr);
                break;
            case css::awt::GradientStyle_SQUARE:
                rWriter.attribute("style", "Square"_ostr);
                break;
            case css::awt::GradientStyle_RECT:
                rWriter.attribute("style", "Rect"_ostr);
                break;
        }
        rWriter.attribute("border", rGradient.getBorder());
        rWriter.attribute("offsetX", rGradient.getOffsetX());
        rWriter.attribute("offsetY", rGradient.getOffsetY());
        rWriter.attribute("angle", rGradient.getAngle());
        rWriter.attribute("steps", rGradient.getSteps());

        auto const& rColorStops(rGradient.getColorStops());
        for (size_t a(0); a < rColorStops.size(); a++)
        {
            if (0 == a)
                rWriter.attribute("startColor",
                                  convertColorToString(rColorStops[a].getStopColor()));
            else if (rColorStops.size() == a + 1)
                rWriter.attribute("endColor", convertColorToString(rColorStops[a].getStopColor()));
            else
            {
                rWriter.startElement("colorStop");
                rWriter.attribute("stopOffset", rColorStops[a].getStopOffset());
                rWriter.attribute("stopColor", convertColorToString(rColorStops[a].getStopColor()));
                rWriter.endElement();
            }
        }
        rWriter.endElement();
    }

    auto const& rHatch = rFillAttribute.getHatch();
    if (!rHatch.isDefault())
    {
        rWriter.startElement("hatch");
        switch (rHatch.getStyle())
        {
            case drawinglayer::attribute::HatchStyle::Single:
                rWriter.attribute("style", "Single"_ostr);
                break;
            case drawinglayer::attribute::HatchStyle::Double:
                rWriter.attribute("style", "Double"_ostr);
                break;
            case drawinglayer::attribute::HatchStyle::Triple:
                rWriter.attribute("style", "Triple"_ostr);
                break;
        }
        rWriter.attribute("distance", rHatch.getDistance());
        rWriter.attribute("angle", rHatch.getAngle());
        rWriter.attribute("color", convertColorToString(rHatch.getColor()));
        rWriter.attribute("minimalDescreteDistance", rHatch.getMinimalDiscreteDistance());
        rWriter.attribute("isFillBackground", sal_Int32(rHatch.isFillBackground()));
        rWriter.endElement();
    }

    auto const& rGraphic = rFillAttribute.getFillGraphic();
    if (!rGraphic.isDefault())
    {
        rWriter.startElement("graphic");
        // TODO
        rWriter.endElement();
    }

    rWriter.endElement();
}

void writeShadeMode(::tools::XmlWriter& rWriter, const css::drawing::ShadeMode& rMode)
{
    switch (rMode)
    {
        case css::drawing::ShadeMode_FLAT:
            rWriter.attribute("shadeMode", "Flat"_ostr);
            break;
        case css::drawing::ShadeMode_SMOOTH:
            rWriter.attribute("shadeMode", "Smooth"_ostr);
            break;
        case css::drawing::ShadeMode_PHONG:
            rWriter.attribute("shadeMode", "Phong"_ostr);
            break;
        case css::drawing::ShadeMode_DRAFT:
            rWriter.attribute("shadeMode", "Draft"_ostr);
            break;
        default:
            rWriter.attribute("shadeMode", "Undefined"_ostr);
            break;
    }
}

void writeProjectionMode(::tools::XmlWriter& rWriter, const css::drawing::ProjectionMode& rMode)
{
    switch (rMode)
    {
        case css::drawing::ProjectionMode_PARALLEL:
            rWriter.attribute("projectionMode", "Parallel"_ostr);
            break;
        case css::drawing::ProjectionMode_PERSPECTIVE:
            rWriter.attribute("projectionMode", "Perspective"_ostr);
            break;
        default:
            rWriter.attribute("projectionMode", "Undefined"_ostr);
            break;
    }
}

void writeNormalsKind(::tools::XmlWriter& rWriter, const css::drawing::NormalsKind& rKind)
{
    switch (rKind)
    {
        case css::drawing::NormalsKind_SPECIFIC:
            rWriter.attribute("normalsKind", "Specific"_ostr);
            break;
        case css::drawing::NormalsKind_FLAT:
            rWriter.attribute("normalsKind", "Flat"_ostr);
            break;
        case css::drawing::NormalsKind_SPHERE:
            rWriter.attribute("normalsKind", "Sphere"_ostr);
            break;
        default:
            rWriter.attribute("normalsKind", "Undefined"_ostr);
            break;
    }
}

void writeTextureProjectionMode(::tools::XmlWriter& rWriter, const char* pElement,
                                const css::drawing::TextureProjectionMode& rMode)
{
    switch (rMode)
    {
        case css::drawing::TextureProjectionMode_OBJECTSPECIFIC:
            rWriter.attribute(pElement, "Specific"_ostr);
            break;
        case css::drawing::TextureProjectionMode_PARALLEL:
            rWriter.attribute(pElement, "Parallel"_ostr);
            break;
        case css::drawing::TextureProjectionMode_SPHERE:
            rWriter.attribute(pElement, "Sphere"_ostr);
            break;
        default:
            rWriter.attribute(pElement, "Undefined"_ostr);
            break;
    }
}

void writeTextureKind(::tools::XmlWriter& rWriter, const css::drawing::TextureKind2& rKind)
{
    switch (rKind)
    {
        case css::drawing::TextureKind2_LUMINANCE:
            rWriter.attribute("textureKind", "Luminance"_ostr);
            break;
        case css::drawing::TextureKind2_INTENSITY:
            rWriter.attribute("textureKind", "Intensity"_ostr);
            break;
        case css::drawing::TextureKind2_COLOR:
            rWriter.attribute("textureKind", "Color"_ostr);
            break;
        default:
            rWriter.attribute("textureKind", "Undefined"_ostr);
            break;
    }
}

void writeTextureMode(::tools::XmlWriter& rWriter, const css::drawing::TextureMode& rMode)
{
    switch (rMode)
    {
        case css::drawing::TextureMode_REPLACE:
            rWriter.attribute("textureMode", "Replace"_ostr);
            break;
        case css::drawing::TextureMode_MODULATE:
            rWriter.attribute("textureMode", "Modulate"_ostr);
            break;
        case css::drawing::TextureMode_BLEND:
            rWriter.attribute("textureMode", "Blend"_ostr);
            break;
        default:
            rWriter.attribute("textureMode", "Undefined"_ostr);
            break;
    }
}

void writeMaterialAttribute(::tools::XmlWriter& rWriter,
                            const drawinglayer::attribute::MaterialAttribute3D& rMaterial)
{
    rWriter.startElement("material");
    rWriter.attribute("color", convertColorToString(rMaterial.getColor()));
    rWriter.attribute("specular", convertColorToString(rMaterial.getSpecular()));
    rWriter.attribute("emission", convertColorToString(rMaterial.getEmission()));
    rWriter.attribute("specularIntensity", rMaterial.getSpecularIntensity());
    rWriter.endElement();
}

void writeSpreadMethod(::tools::XmlWriter& rWriter,
                       const drawinglayer::primitive2d::SpreadMethod& rSpreadMethod)
{
    switch (rSpreadMethod)
    {
        case drawinglayer::primitive2d::SpreadMethod::Pad:
            rWriter.attribute("spreadmethod", "pad"_ostr);
            break;
        case drawinglayer::primitive2d::SpreadMethod::Reflect:
            rWriter.attribute("spreadmethod", "reflect"_ostr);
            break;
        case drawinglayer::primitive2d::SpreadMethod::Repeat:
            rWriter.attribute("spreadmethod", "repeat"_ostr);
            break;
        default:
            rWriter.attribute("spreadmethod", "unknown"_ostr);
    }
}

} // end anonymous namespace

Primitive2dXmlDump::Primitive2dXmlDump()
    : maFilter(constMaxActionType, false)
{
}

Primitive2dXmlDump::~Primitive2dXmlDump() = default;

void Primitive2dXmlDump::dump(
    const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence,
    const OUString& rStreamName)
{
    std::unique_ptr<SvStream> pStream;

    if (rStreamName.isEmpty())
        pStream.reset(new SvMemoryStream());
    else
        pStream.reset(new SvFileStream(rStreamName, StreamMode::STD_READWRITE | StreamMode::TRUNC));

    ::tools::XmlWriter aWriter(pStream.get());
    aWriter.startDocument();
    aWriter.startElement("primitive2D");

    decomposeAndWrite(rPrimitive2DSequence, aWriter);

    aWriter.endElement();
    aWriter.endDocument();

    pStream->Seek(STREAM_SEEK_TO_BEGIN);
}

namespace
{
class Primitive3DXmlDump
{
public:
    void decomposeAndWrite(const drawinglayer::primitive3d::Primitive3DContainer& rSequence,
                           ::tools::XmlWriter& rWriter)
    {
        for (size_t i = 0; i < rSequence.size(); i++)
        {
            drawinglayer::primitive3d::Primitive3DReference xReference = rSequence[i];
            const auto* pBasePrimitive
                = static_cast<const drawinglayer::primitive3d::BasePrimitive3D*>(xReference.get());
            sal_uInt32 nId = pBasePrimitive->getPrimitive3DID();
            OUString sCurrentElementTag = drawinglayer::primitive3d::idToString(nId);
            switch (nId)
            {
                case PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D:
                {
                    const auto* pExtrudePrimitive3D
                        = static_cast<const drawinglayer::primitive3d::SdrExtrudePrimitive3D*>(
                            xReference.get());
                    rWriter.startElement("extrude3D");

                    rWriter.startElement("matrix3D");
                    writeMatrix3D(rWriter, pExtrudePrimitive3D->getTransform());
                    rWriter.endElement();

                    rWriter.attribute("textureSizeX", pExtrudePrimitive3D->getTextureSize().getX());
                    rWriter.attribute("textureSizeY", pExtrudePrimitive3D->getTextureSize().getY());
                    auto const& rLFSAttribute = pExtrudePrimitive3D->getSdrLFSAttribute();
                    writeSdrLineAttribute(rWriter, rLFSAttribute.getLine());
                    writeSdrFillAttribute(rWriter, rLFSAttribute.getFill());

                    rWriter.startElement("object3Dattributes");
                    {
                        auto const& r3DObjectAttributes
                            = pExtrudePrimitive3D->getSdr3DObjectAttribute();

                        writeNormalsKind(rWriter, r3DObjectAttributes.getNormalsKind());
                        writeTextureProjectionMode(rWriter, "textureProjectionX",
                                                   r3DObjectAttributes.getTextureProjectionX());
                        writeTextureProjectionMode(rWriter, "textureProjectionY",
                                                   r3DObjectAttributes.getTextureProjectionY());
                        writeTextureKind(rWriter, r3DObjectAttributes.getTextureKind());
                        writeTextureMode(rWriter, r3DObjectAttributes.getTextureMode());
                        writeMaterialAttribute(rWriter, r3DObjectAttributes.getMaterial());

                        rWriter.attribute("normalsInvert",
                                          sal_Int32(r3DObjectAttributes.getNormalsInvert()));
                        rWriter.attribute("doubleSided",
                                          sal_Int32(r3DObjectAttributes.getDoubleSided()));
                        rWriter.attribute("shadow3D", sal_Int32(r3DObjectAttributes.getShadow3D()));
                        rWriter.attribute("textureFilter",
                                          sal_Int32(r3DObjectAttributes.getTextureFilter()));
                        rWriter.attribute("reducedGeometry",
                                          sal_Int32(r3DObjectAttributes.getReducedLineGeometry()));
                    }
                    rWriter.endElement();

                    rWriter.attribute("depth", pExtrudePrimitive3D->getDepth());
                    rWriter.attribute("diagonal", pExtrudePrimitive3D->getDiagonal());
                    rWriter.attribute("backScale", pExtrudePrimitive3D->getBackScale());
                    rWriter.attribute("smoothNormals",
                                      sal_Int32(pExtrudePrimitive3D->getSmoothNormals()));
                    rWriter.attribute("smoothLids",
                                      sal_Int32(pExtrudePrimitive3D->getSmoothLids()));
                    rWriter.attribute("characterMode",
                                      sal_Int32(pExtrudePrimitive3D->getCharacterMode()));
                    rWriter.attribute("closeFront",
                                      sal_Int32(pExtrudePrimitive3D->getCloseFront()));
                    rWriter.attribute("closeBack", sal_Int32(pExtrudePrimitive3D->getCloseBack()));
                    writePolyPolygon(rWriter, pExtrudePrimitive3D->getPolyPolygon());
                    rWriter.endElement();
                }
                break;

                default:
                {
                    rWriter.startElement("unhandled");
                    rWriter.attribute("id", sCurrentElementTag);
                    rWriter.attribute("idNumber", nId);

                    drawinglayer::geometry::ViewInformation3D aViewInformation3D;
                    drawinglayer::primitive3d::Primitive3DContainer aContainer;
                    aContainer = pBasePrimitive->get3DDecomposition(aViewInformation3D);
                    decomposeAndWrite(aContainer, rWriter);
                    rWriter.endElement();
                }
                break;
            }
        }
    }
};
}
xmlDocUniquePtr Primitive2dXmlDump::dumpAndParse(
    const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence,
    const OUString& rStreamName)
{
    std::unique_ptr<SvStream> pStream;

    if (rStreamName.isEmpty())
        pStream.reset(new SvMemoryStream());
    else
        pStream.reset(new SvFileStream(rStreamName, StreamMode::STD_READWRITE | StreamMode::TRUNC));

    ::tools::XmlWriter aWriter(pStream.get());
    aWriter.startDocument();
    aWriter.startElement("primitive2D");

    decomposeAndWrite(rPrimitive2DSequence, aWriter);

    aWriter.endElement();
    aWriter.endDocument();

    pStream->Seek(STREAM_SEEK_TO_BEGIN);

    std::size_t nSize = pStream->remainingSize();
    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nSize + 1]);
    pStream->ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;
    SAL_INFO("drawinglayer", "Parsed XML: " << pBuffer.get());

    return xmlDocUniquePtr(xmlParseDoc(reinterpret_cast<xmlChar*>(pBuffer.get())));
}

OUString Primitive2dXmlDump::idToString(sal_uInt32 nId)
{
    return drawinglayer::primitive2d::idToString(nId);
}

void Primitive2dXmlDump::decomposeAndWrite(
    const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence,
    ::tools::XmlWriter& rWriter)
{
    for (auto const& i : rPrimitive2DSequence)
    {
        const BasePrimitive2D* pBasePrimitive = i.get();
        sal_uInt32 nId = pBasePrimitive->getPrimitive2DID();
        if (nId < maFilter.size() && maFilter[nId])
            continue;

        // handled by subclass
        if (decomposeAndWrite(*pBasePrimitive, rWriter))
            continue;

        OUString sCurrentElementTag = drawinglayer::primitive2d::idToString(nId);

        switch (nId)
        {
            case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
            {
                const BitmapPrimitive2D& rBitmapPrimitive2D
                    = dynamic_cast<const BitmapPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("bitmap");
                writeMatrix(rWriter, rBitmapPrimitive2D.getTransform());

                const BitmapEx aBitmapEx(rBitmapPrimitive2D.getBitmap());
                const Size& rSizePixel(aBitmapEx.GetSizePixel());

                rWriter.attribute("height", rSizePixel.getHeight());
                rWriter.attribute("width", rSizePixel.getWidth());
                rWriter.attribute("checksum", OString(std::to_string(aBitmapEx.GetChecksum())));

                for (tools::Long y = 0; y < rSizePixel.getHeight(); y++)
                {
                    rWriter.startElement("data");
                    OUString aBitmapData = u""_ustr;
                    for (tools::Long x = 0; x < rSizePixel.getHeight(); x++)
                    {
                        if (x != 0)
                            aBitmapData = aBitmapData + ",";
                        aBitmapData = aBitmapData + aBitmapEx.GetPixelColor(x, y).AsRGBHexString();
                    }
                    rWriter.attribute("row", aBitmapData);
                    rWriter.endElement();
                }
                rWriter.endElement();
            }
            break;
            case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D:
            {
                const HiddenGeometryPrimitive2D& rHiddenGeometryPrimitive2D
                    = dynamic_cast<const HiddenGeometryPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("hiddengeometry");
                decomposeAndWrite(rHiddenGeometryPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
            {
                const TransformPrimitive2D& rTransformPrimitive2D
                    = dynamic_cast<const TransformPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("transform");
                writeMatrix(rWriter, rTransformPrimitive2D.getTransformation());
                decomposeAndWrite(rTransformPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
            {
                const PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D
                    = dynamic_cast<const PolyPolygonColorPrimitive2D&>(*pBasePrimitive);

                rWriter.startElement("polypolygoncolor");
                rWriter.attribute("color",
                                  convertColorToString(rPolyPolygonColorPrimitive2D.getBColor()));

                const basegfx::B2DPolyPolygon& aB2DPolyPolygon(
                    rPolyPolygonColorPrimitive2D.getB2DPolyPolygon());
                writePolyPolygon(rWriter, aB2DPolyPolygon);

                rWriter.endElement();
            }
            break;
            case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
            {
                const PointArrayPrimitive2D& rPointArrayPrimitive2D
                    = dynamic_cast<const PointArrayPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("pointarray");

                rWriter.attribute("color",
                                  convertColorToString(rPointArrayPrimitive2D.getRGBColor()));

                const std::vector<basegfx::B2DPoint> aPositions
                    = rPointArrayPrimitive2D.getPositions();
                for (std::vector<basegfx::B2DPoint>::const_iterator iter = aPositions.begin();
                     iter != aPositions.end(); ++iter)
                {
                    rWriter.startElement("point");
                    rWriter.attribute("x", OUString::number(iter->getX()));
                    rWriter.attribute("y", OUString::number(iter->getY()));
                    rWriter.endElement();
                }

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D:
            {
                const PolygonStrokeArrowPrimitive2D& rPolygonStrokeArrowPrimitive2D
                    = dynamic_cast<const PolygonStrokeArrowPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polygonstrokearrow");

                rWriter.startElement("polygon");
                rWriter.content(basegfx::utils::exportToSvgPoints(
                    rPolygonStrokeArrowPrimitive2D.getB2DPolygon()));
                rWriter.endElement();

                if (rPolygonStrokeArrowPrimitive2D.getStart().getB2DPolyPolygon().count())
                {
                    rWriter.startElement("linestartattribute");
                    rWriter.attribute("width",
                                      rPolygonStrokeArrowPrimitive2D.getStart().getWidth());
                    rWriter.attribute("centered",
                                      static_cast<sal_Int32>(
                                          rPolygonStrokeArrowPrimitive2D.getStart().isCentered()));
                    writePolyPolygon(rWriter,
                                     rPolygonStrokeArrowPrimitive2D.getStart().getB2DPolyPolygon());
                    rWriter.endElement();
                }

                if (rPolygonStrokeArrowPrimitive2D.getEnd().getB2DPolyPolygon().count())
                {
                    rWriter.startElement("lineendattribute");
                    rWriter.attribute("width", rPolygonStrokeArrowPrimitive2D.getEnd().getWidth());
                    rWriter.attribute("centered",
                                      static_cast<sal_Int32>(
                                          rPolygonStrokeArrowPrimitive2D.getEnd().isCentered()));
                    writePolyPolygon(rWriter,
                                     rPolygonStrokeArrowPrimitive2D.getEnd().getB2DPolyPolygon());
                    rWriter.endElement();
                }

                writeLineAttribute(rWriter, rPolygonStrokeArrowPrimitive2D.getLineAttribute());
                writeStrokeAttribute(rWriter, rPolygonStrokeArrowPrimitive2D.getStrokeAttribute());
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
            {
                const PolygonStrokePrimitive2D& rPolygonStrokePrimitive2D
                    = dynamic_cast<const PolygonStrokePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polygonstroke");

                rWriter.startElement("polygon");
                rWriter.content(
                    basegfx::utils::exportToSvgPoints(rPolygonStrokePrimitive2D.getB2DPolygon()));
                rWriter.endElement();

                writeLineAttribute(rWriter, rPolygonStrokePrimitive2D.getLineAttribute());
                writeStrokeAttribute(rWriter, rPolygonStrokePrimitive2D.getStrokeAttribute());
                rWriter.endElement();
            }
            break;
            case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
            {
                const PolyPolygonStrokePrimitive2D& rPolyPolygonStrokePrimitive2D
                    = dynamic_cast<const PolyPolygonStrokePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polypolygonstroke");

                writeLineAttribute(rWriter, rPolyPolygonStrokePrimitive2D.getLineAttribute());
                writeStrokeAttribute(rWriter, rPolyPolygonStrokePrimitive2D.getStrokeAttribute());
                writePolyPolygon(rWriter, rPolyPolygonStrokePrimitive2D.getB2DPolyPolygon());

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
            {
                const PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D
                    = dynamic_cast<const PolygonHairlinePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polygonhairline");

                rWriter.attribute("color",
                                  convertColorToString(rPolygonHairlinePrimitive2D.getBColor()));

                rWriter.startElement("polygon");
                rWriter.content(
                    basegfx::utils::exportToSvgPoints(rPolygonHairlinePrimitive2D.getB2DPolygon()));
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
            {
                const TextDecoratedPortionPrimitive2D& rTextDecoratedPortionPrimitive2D
                    = dynamic_cast<const TextDecoratedPortionPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("textdecoratedportion");
                writeMatrix(rWriter, rTextDecoratedPortionPrimitive2D.getTextTransform());

                rWriter.attribute("text", rTextDecoratedPortionPrimitive2D.getText());
                rWriter.attribute(
                    "fontcolor",
                    convertColorToString(rTextDecoratedPortionPrimitive2D.getFontColor()));

                const drawinglayer::attribute::FontAttribute& aFontAttribute
                    = rTextDecoratedPortionPrimitive2D.getFontAttribute();
                rWriter.attribute("familyname", aFontAttribute.getFamilyName());
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D:
            {
                const TextLinePrimitive2D& rTextLinePrimitive2D
                    = dynamic_cast<const TextLinePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("textline");
                writeMatrix(rWriter, rTextLinePrimitive2D.getObjectTransformation());

                rWriter.attribute("width", rTextLinePrimitive2D.getWidth());
                rWriter.attribute("offset", rTextLinePrimitive2D.getOffset());
                rWriter.attribute("height", rTextLinePrimitive2D.getHeight());
                rWriter.attribute("color",
                                  convertColorToString(rTextLinePrimitive2D.getLineColor()));
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
            {
                const TextSimplePortionPrimitive2D& rTextSimplePortionPrimitive2D
                    = dynamic_cast<const TextSimplePortionPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("textsimpleportion");

                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                if (rTextSimplePortionPrimitive2D.getTextTransform().decompose(aScale, aTranslate,
                                                                               fRotate, fShearX))
                {
                    rWriter.attribute("width", aScale.getX());
                    rWriter.attribute("height", aScale.getY());
                }
                rWriter.attribute("x", aTranslate.getX());
                rWriter.attribute("y", aTranslate.getY());
                OUString aText = rTextSimplePortionPrimitive2D.getText();
                // TODO share code with sax_fastparser::FastSaxSerializer::write().
                rWriter.attribute("text", aText.replaceAll("", "&#9;"));
                rWriter.attribute("fontcolor", convertColorToString(
                                                   rTextSimplePortionPrimitive2D.getFontColor()));

                const drawinglayer::attribute::FontAttribute& aFontAttribute
                    = rTextSimplePortionPrimitive2D.getFontAttribute();
                rWriter.attribute("familyname", aFontAttribute.getFamilyName());
                const std::vector<double> aDx = rTextSimplePortionPrimitive2D.getDXArray();
                if (aDx.size())
                {
                    for (size_t iDx = 0; iDx < aDx.size(); ++iDx)
                    {
                        OString sName = "dx" + OString::number(iDx);
                        rWriter.attribute(sName, OString::number(aDx[iDx]));
                    }
                }
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
            {
                const GroupPrimitive2D& rGroupPrimitive2D
                    = dynamic_cast<const GroupPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("group");
                decomposeAndWrite(rGroupPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
            {
                const MaskPrimitive2D& rMaskPrimitive2D
                    = dynamic_cast<const MaskPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("mask");
                writePolyPolygon(rWriter, rMaskPrimitive2D.getMask());
                decomposeAndWrite(rMaskPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
            {
                const UnifiedTransparencePrimitive2D& rUnifiedTransparencePrimitive2D
                    = dynamic_cast<const UnifiedTransparencePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("unifiedtransparence");
                rWriter.attribute(
                    "transparence",
                    std::lround(100 * rUnifiedTransparencePrimitive2D.getTransparence()));
                decomposeAndWrite(rUnifiedTransparencePrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
            {
                const ObjectInfoPrimitive2D& rObjectInfoPrimitive2D
                    = dynamic_cast<const ObjectInfoPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("objectinfo");

                decomposeAndWrite(rObjectInfoPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D:
            {
                const StructureTagPrimitive2D& rStructureTagPrimitive2D
                    = dynamic_cast<const StructureTagPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("structuretag");
                rWriter.attribute("structureelement",
                                  rStructureTagPrimitive2D.getStructureElement());

                decomposeAndWrite(rStructureTagPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
            {
                const SvgRadialGradientPrimitive2D& rSvgRadialGradientPrimitive2D
                    = dynamic_cast<const SvgRadialGradientPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("svgradialgradient");
                if (rSvgRadialGradientPrimitive2D.isFocalSet())
                {
                    basegfx::B2DPoint aFocalAttribute = rSvgRadialGradientPrimitive2D.getFocal();
                    rWriter.attribute("focalx", aFocalAttribute.getX());
                    rWriter.attribute("focaly", aFocalAttribute.getY());
                }

                basegfx::B2DPoint aStartPoint = rSvgRadialGradientPrimitive2D.getStart();
                rWriter.attribute("startx", aStartPoint.getX());
                rWriter.attribute("starty", aStartPoint.getY());
                rWriter.attribute("radius",
                                  OString::number(rSvgRadialGradientPrimitive2D.getRadius()));
                writeSpreadMethod(rWriter, rSvgRadialGradientPrimitive2D.getSpreadMethod());
                rWriter.attributeDouble(
                    "opacity",
                    rSvgRadialGradientPrimitive2D.getGradientEntries().front().getOpacity());

                rWriter.startElement("transform");
                writeMatrix(rWriter, rSvgRadialGradientPrimitive2D.getGradientTransform());
                rWriter.endElement();

                writePolyPolygon(rWriter, rSvgRadialGradientPrimitive2D.getPolyPolygon());
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
            {
                const SvgLinearGradientPrimitive2D& rSvgLinearGradientPrimitive2D
                    = dynamic_cast<const SvgLinearGradientPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("svglineargradient");
                basegfx::B2DPoint aStartAttribute = rSvgLinearGradientPrimitive2D.getStart();
                basegfx::B2DPoint aEndAttribute = rSvgLinearGradientPrimitive2D.getEnd();

                rWriter.attribute("startx", aStartAttribute.getX());
                rWriter.attribute("starty", aStartAttribute.getY());
                rWriter.attribute("endx", aEndAttribute.getX());
                rWriter.attribute("endy", aEndAttribute.getY());
                writeSpreadMethod(rWriter, rSvgLinearGradientPrimitive2D.getSpreadMethod());
                rWriter.attributeDouble(
                    "opacity",
                    rSvgLinearGradientPrimitive2D.getGradientEntries().front().getOpacity());

                rWriter.startElement("transform");
                writeMatrix(rWriter, rSvgLinearGradientPrimitive2D.getGradientTransform());
                rWriter.endElement();

                writePolyPolygon(rWriter, rSvgLinearGradientPrimitive2D.getPolyPolygon());

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
            {
                const MetafilePrimitive2D& rMetafilePrimitive2D
                    = dynamic_cast<const MetafilePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("metafile");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                // since the graphic is not rendered in a document, we do not need a concrete view information
                rMetafilePrimitive2D.get2DDecomposition(
                    aPrimitiveContainer, drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
            }

            break;

            case PRIMITIVE2D_ID_SDRRECTANGLEPRIMITIVE2D:
            {
                // SdrRectanglePrimitive2D is private to us.
                rWriter.startElement("sdrrectangle");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_SDRBLOCKTEXTPRIMITIVE2D:
            {
                // SdrBlockTextPrimitive2D is private to us.
                rWriter.startElement("sdrblocktext");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
            {
                // TextHierarchyBlockPrimitive2D.
                rWriter.startElement("texthierarchyblock");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
            {
                // TextHierarchyParagraphPrimitive2D.
                rWriter.startElement("texthierarchyparagraph");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
            {
                // TextHierarchyLinePrimitive2D.
                rWriter.startElement("texthierarchyline");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
            {
                // ShadowPrimitive2D.
                const ShadowPrimitive2D& rShadowPrimitive2D
                    = dynamic_cast<const ShadowPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("shadow");
                rWriter.attribute("color",
                                  convertColorToString(rShadowPrimitive2D.getShadowColor()));
                rWriter.attributeDouble("blur", rShadowPrimitive2D.getShadowBlur());

                rWriter.startElement("transform");
                writeMatrix(rWriter, rShadowPrimitive2D.getShadowTransform());
                rWriter.endElement();

                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
            {
                // ModifiedColorPrimitive2D.
                const ModifiedColorPrimitive2D& rModifiedColorPrimitive2D
                    = dynamic_cast<const ModifiedColorPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("modifiedColor");
                const basegfx::BColorModifierSharedPtr& aColorModifier
                    = rModifiedColorPrimitive2D.getColorModifier();
                rWriter.attribute("modifier", aColorModifier->getModifierName());

                decomposeAndWrite(rModifiedColorPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
                break;
            }
            case PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D:
            {
                // SoftEdgePrimitive2D.
                const SoftEdgePrimitive2D& rSoftEdgePrimitive2D
                    = dynamic_cast<const SoftEdgePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("softedge");
                rWriter.attribute("radius", OUString::number(rSoftEdgePrimitive2D.getRadius()));

                decomposeAndWrite(rSoftEdgePrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_SCENEPRIMITIVE2D:
            {
                const auto& rScenePrimitive2D
                    = dynamic_cast<const drawinglayer::primitive2d::ScenePrimitive2D&>(
                        *pBasePrimitive);
                rWriter.startElement("scene");

                auto const& rSceneAttribute = rScenePrimitive2D.getSdrSceneAttribute();

                rWriter.attribute("shadowSlant", rSceneAttribute.getShadowSlant());
                rWriter.attribute("isTwoSidedLighting",
                                  sal_Int32(rSceneAttribute.getTwoSidedLighting()));
                writeShadeMode(rWriter, rSceneAttribute.getShadeMode());
                writeProjectionMode(rWriter, rSceneAttribute.getProjectionMode());

                auto const& rLightingAttribute = rScenePrimitive2D.getSdrLightingAttribute();
                rWriter.attribute("ambientLightColor",
                                  convertColorToString(rLightingAttribute.getAmbientLightColor()));
                rWriter.startElement("lights");
                for (auto const& rLight : rLightingAttribute.getLightVector())
                {
                    rWriter.startElement("light");
                    rWriter.attribute("color", convertColorToString(rLight.getColor()));
                    rWriter.attribute("directionVectorX", rLight.getDirection().getX());
                    rWriter.attribute("directionVectorY", rLight.getDirection().getY());
                    rWriter.attribute("specular", sal_Int32(rLight.getSpecular()));
                    rWriter.endElement();
                }
                rWriter.endElement();

                Primitive3DXmlDump aPrimitive3DXmlDump;
                aPrimitive3DXmlDump.decomposeAndWrite(rScenePrimitive2D.getChildren3D(), rWriter);

                rWriter.endElement();
                break;
            }

            default:
            {
                rWriter.startElement("unhandled");
                rWriter.attribute("id", sCurrentElementTag);
                rWriter.attribute("idNumber", nId);

                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
            }
            break;
        }
    }
}

} // end namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
