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
#include <vcl/bitmapex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <memory>
#include <sal/log.hxx>

#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>

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

    for (basegfx::B2DPolygon const & rPolygon : rB2DPolyPolygon)
    {
        rWriter.startElement("polygon");
        for (sal_uInt32 i = 0; i <rPolygon.count(); ++i)
        {
            basegfx::B2DPoint const & rPoint = rPolygon.getB2DPoint(i);

            rWriter.startElement("point");
            rWriter.attribute("x", OUString::number(rPoint.getX()));
            rWriter.attribute("y", OUString::number(rPoint.getY()));
            rWriter.endElement();
        }
        rWriter.endElement();
    }

    rWriter.endElement();
}

} // end anonymous namespace

Primitive2dXmlDump::Primitive2dXmlDump() :
    maFilter(constMaxActionType, false)
{}

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

void Primitive2dXmlDump::decomposeAndWrite(
    const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence,
    ::tools::XmlWriter& rWriter)
{
    for (size_t i = 0; i < rPrimitive2DSequence.size(); i++)
    {
        drawinglayer::primitive2d::Primitive2DReference xPrimitive2DReference = rPrimitive2DSequence[i];
        const BasePrimitive2D* pBasePrimitive = dynamic_cast<const BasePrimitive2D* >(xPrimitive2DReference.get());
        if (!pBasePrimitive)
            continue;
        sal_uInt32 nId = pBasePrimitive->getPrimitive2DID();
        if (nId < maFilter.size() && maFilter[nId])
            continue;

        OUString sCurrentElementTag = drawinglayer::primitive2d::idToString(nId);

        switch (nId)
        {
            case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
            {
                const BitmapPrimitive2D& rBitmapPrimitive2D
                    = dynamic_cast<const BitmapPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("bitmap");

                const BitmapEx aBitmapEx(VCLUnoHelper::GetBitmap(rBitmapPrimitive2D.getXBitmap()));
                const Size& rSizePixel(aBitmapEx.GetSizePixel());

                rWriter.attribute("height", rSizePixel.getHeight());
                rWriter.attribute("width", rSizePixel.getWidth());
                rWriter.attribute("checksum", OString(std::to_string(aBitmapEx.GetChecksum())));

                for (tools::Long y = 0; y < rSizePixel.getHeight(); y++)
                {
                    rWriter.startElement("data");
                    OUString aBitmapData = "";
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
                const HiddenGeometryPrimitive2D& rHiddenGeometryPrimitive2D = dynamic_cast<const HiddenGeometryPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("hiddengeometry");
                decomposeAndWrite(rHiddenGeometryPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
            {
                const TransformPrimitive2D& rTransformPrimitive2D = dynamic_cast<const TransformPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("transform");

                basegfx::B2DHomMatrix const & rMatrix = rTransformPrimitive2D.getTransformation();
                rWriter.attributeDouble("xy11", rMatrix.get(0,0));
                rWriter.attributeDouble("xy12", rMatrix.get(0,1));
                rWriter.attributeDouble("xy13", rMatrix.get(0,2));
                rWriter.attributeDouble("xy21", rMatrix.get(1,0));
                rWriter.attributeDouble("xy22", rMatrix.get(1,1));
                rWriter.attributeDouble("xy23", rMatrix.get(1,2));
                rWriter.attributeDouble("xy31", rMatrix.get(2,0));
                rWriter.attributeDouble("xy32", rMatrix.get(2,1));
                rWriter.attributeDouble("xy33", rMatrix.get(2,2));

                decomposeAndWrite(rTransformPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
            {
                const PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D = dynamic_cast<const PolyPolygonColorPrimitive2D&>(*pBasePrimitive);

                rWriter.startElement("polypolygoncolor");
                rWriter.attribute("color", convertColorToString(rPolyPolygonColorPrimitive2D.getBColor()));

                const basegfx::B2DPolyPolygon& aB2DPolyPolygon(rPolyPolygonColorPrimitive2D.getB2DPolyPolygon());
                writePolyPolygon(rWriter, aB2DPolyPolygon);

                rWriter.endElement();
            }
            break;
            case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
            {
                const PointArrayPrimitive2D& rPointArrayPrimitive2D = dynamic_cast<const PointArrayPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("pointarray");

                rWriter.attribute("color", convertColorToString(rPointArrayPrimitive2D.getRGBColor()));

                const std::vector< basegfx::B2DPoint > aPositions = rPointArrayPrimitive2D.getPositions();
                for (std::vector<basegfx::B2DPoint>::const_iterator iter = aPositions.begin(); iter != aPositions.end(); ++iter)
                {
                    rWriter.startElement("point");
                    rWriter.attribute("x", OUString::number(iter->getX()));
                    rWriter.attribute("y", OUString::number(iter->getY()));
                    rWriter.endElement();
                }

                rWriter.endElement();
            }
            break;
            case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
            {
                const PolygonStrokePrimitive2D& rPolygonStrokePrimitive2D = dynamic_cast<const PolygonStrokePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polygonstroke");

                rWriter.startElement("polygon");
                rWriter.content(basegfx::utils::exportToSvgPoints(rPolygonStrokePrimitive2D.getB2DPolygon()));
                rWriter.endElement();

                rWriter.startElement("line");
                const drawinglayer::attribute::LineAttribute& aLineAttribute = rPolygonStrokePrimitive2D.getLineAttribute();
                rWriter.attribute("color", convertColorToString(aLineAttribute.getColor()));
                rWriter.attribute("width", aLineAttribute.getWidth());

                rWriter.endElement();

                rWriter.startElement("stroke");
                const drawinglayer::attribute::StrokeAttribute& aStrokeAttribute = rPolygonStrokePrimitive2D.getStrokeAttribute();
                rWriter.attribute("fulldotdashlen", aStrokeAttribute.getFullDotDashLen());
                //rWriter.attribute("dotdasharray", aStrokeAttribute.getDotDashArray());
                rWriter.endElement();

                rWriter.endElement();
            }
            break;
            case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
            {
                const PolyPolygonStrokePrimitive2D& rPolyPolygonStrokePrimitive2D = dynamic_cast<const PolyPolygonStrokePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polypolygonstroke");

                rWriter.startElement("line");
                const drawinglayer::attribute::LineAttribute& aLineAttribute = rPolyPolygonStrokePrimitive2D.getLineAttribute();
                rWriter.attribute("color", convertColorToString(aLineAttribute.getColor()));
                rWriter.attribute("width", aLineAttribute.getWidth());
                //rWriter.attribute("linejoin", aLineAttribute.getLineJoin());
                //rWriter.attribute("linecap", aLineAttribute.getLineCap());
                rWriter.endElement();

                //getStrokeAttribute()

                writePolyPolygon(rWriter, rPolyPolygonStrokePrimitive2D.getB2DPolyPolygon());

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
            {
                const PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D = dynamic_cast<const PolygonHairlinePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("polygonhairline");

                rWriter.attribute("color", convertColorToString(rPolygonHairlinePrimitive2D.getBColor()));

                rWriter.startElement("polygon");
                rWriter.content(basegfx::utils::exportToSvgPoints(rPolygonHairlinePrimitive2D.getB2DPolygon()));
                rWriter.endElement();


                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
            {
                const TextSimplePortionPrimitive2D& rTextSimplePortionPrimitive2D = dynamic_cast<const TextSimplePortionPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("textsimpleportion");

                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                if(rTextSimplePortionPrimitive2D.getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    rWriter.attribute("width", aScale.getX());
                    rWriter.attribute("height", aScale.getY());
                }
                rWriter.attribute("x", aTranslate.getX());
                rWriter.attribute("y", aTranslate.getY());
                OUString aText = rTextSimplePortionPrimitive2D.getText();
                // TODO share code with sax_fastparser::FastSaxSerializer::write().
                rWriter.attribute("text", aText.replaceAll("", "&#9;"));
                rWriter.attribute("fontcolor", convertColorToString(rTextSimplePortionPrimitive2D.getFontColor()));

                const drawinglayer::attribute::FontAttribute& aFontAttribute = rTextSimplePortionPrimitive2D.getFontAttribute();
                rWriter.attribute("familyname", aFontAttribute.getFamilyName());
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
            {
                const GroupPrimitive2D& rGroupPrimitive2D = dynamic_cast<const GroupPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("group");
                decomposeAndWrite(rGroupPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
            {
                const MaskPrimitive2D& rMaskPrimitive2D = dynamic_cast<const MaskPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("mask");
                writePolyPolygon(rWriter, rMaskPrimitive2D.getMask());
                decomposeAndWrite(rMaskPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
            {
                const UnifiedTransparencePrimitive2D& rUnifiedTransparencePrimitive2D = dynamic_cast<const UnifiedTransparencePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("unifiedtransparence");
                rWriter.attribute("transparence", OString::number(rUnifiedTransparencePrimitive2D.getTransparence()));
                decomposeAndWrite(rUnifiedTransparencePrimitive2D.getChildren(), rWriter);

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
            {
                const ObjectInfoPrimitive2D& rObjectInfoPrimitive2D = dynamic_cast<const ObjectInfoPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("objectinfo");

                decomposeAndWrite(rObjectInfoPrimitive2D.getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
            {
                const SvgRadialGradientPrimitive2D& rSvgRadialGradientPrimitive2D = dynamic_cast<const SvgRadialGradientPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("svgradialgradient");
                basegfx::B2DPoint aFocusAttribute = rSvgRadialGradientPrimitive2D.getFocal();

                rWriter.attribute("radius", OString::number(rSvgRadialGradientPrimitive2D.getRadius()));
                rWriter.attribute("focusx", aFocusAttribute.getX());
                rWriter.attribute("focusy", aFocusAttribute.getY());

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
            {
                const SvgLinearGradientPrimitive2D& rSvgLinearGradientPrimitive2D = dynamic_cast<const SvgLinearGradientPrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("svglineargradient");
                basegfx::B2DPoint aStartAttribute = rSvgLinearGradientPrimitive2D.getStart();
                basegfx::B2DPoint aEndAttribute = rSvgLinearGradientPrimitive2D.getEnd();

                rWriter.attribute("startx", aStartAttribute.getX());
                rWriter.attribute("starty", aStartAttribute.getY());
                rWriter.attribute("endx", aEndAttribute.getX());
                rWriter.attribute("endy", aEndAttribute.getY());
                //rWriter.attribute("spreadmethod", (int)rSvgLinearGradientPrimitive2D.getSpreadMethod());
                rWriter.attributeDouble("opacity", rSvgLinearGradientPrimitive2D.getGradientEntries().front().getOpacity());

                rWriter.startElement("transform");
                basegfx::B2DHomMatrix const & rMatrix = rSvgLinearGradientPrimitive2D.getGradientTransform();
                rWriter.attributeDouble("xy11", rMatrix.get(0,0));
                rWriter.attributeDouble("xy12", rMatrix.get(0,1));
                rWriter.attributeDouble("xy13", rMatrix.get(0,2));
                rWriter.attributeDouble("xy21", rMatrix.get(1,0));
                rWriter.attributeDouble("xy22", rMatrix.get(1,1));
                rWriter.attributeDouble("xy23", rMatrix.get(1,2));
                rWriter.attributeDouble("xy31", rMatrix.get(2,0));
                rWriter.attributeDouble("xy32", rMatrix.get(2,1));
                rWriter.attributeDouble("xy33", rMatrix.get(2,2));
                rWriter.endElement();

                writePolyPolygon(rWriter, rSvgLinearGradientPrimitive2D.getPolyPolygon());

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
            {
                const MetafilePrimitive2D& rMetafilePrimitive2D = dynamic_cast<const MetafilePrimitive2D&>(*pBasePrimitive);
                rWriter.startElement("metafile");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                // since the graphic is not rendered in a document, we do not need a concrete view information
                rMetafilePrimitive2D.get2DDecomposition(aPrimitiveContainer, drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer,rWriter);
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
                rWriter.startElement("shadow");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
            {
                // ModifiedColorPrimitive2D.
                rWriter.startElement("modifiedColor");
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                                   drawinglayer::geometry::ViewInformation2D());
                decomposeAndWrite(aPrimitiveContainer, rWriter);
                rWriter.endElement();
                break;
            }

            default:
            {
                OString aName("unhandled");
                switch (nId)
                {
                    case PRIMITIVE2D_ID_RANGE_SVX | 14: // PRIMITIVE2D_ID_SDRCELLPRIMITIVE2D
                    {
                        aName = "sdrCell";
                        break;
                    }
                }
                rWriter.startElement(aName);
                rWriter.attribute("id", OUStringToOString(sCurrentElementTag, RTL_TEXTENCODING_UTF8));
                rWriter.attribute("idNumber", nId);

                auto pBufferedDecomposition
                    = dynamic_cast<const BufferedDecompositionPrimitive2D*>(pBasePrimitive);
                if (pBufferedDecomposition)
                {
                    rWriter.attribute(
                        "transparenceForShadow",
                        OString::number(pBufferedDecomposition->getTransparenceForShadow()));
                }

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
