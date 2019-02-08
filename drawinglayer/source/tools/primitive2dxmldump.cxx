/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/tools/primitive2dxmldump.hxx>

#include <vcl/metaact.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

#include <memory>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace drawinglayer::primitive2d;

namespace drawinglayer::tools
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

xmlDocPtr Primitive2dXmlDump::dumpAndParse(
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

    xmlDocPtr pDoc = xmlParseDoc(reinterpret_cast<xmlChar*>(pBuffer.get()));

    return pDoc;
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
        if (maFilter[nId])
            continue;

        OUString sCurrentElementTag = drawinglayer::primitive2d::idToString(nId);

        switch (nId)
        {
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
                    rWriter.attribute("height", aScale.getY());
                }
                rWriter.attribute("x", aTranslate.getX());
                rWriter.attribute("y", aTranslate.getY());
                rWriter.attribute("text", rTextSimplePortionPrimitive2D.getText());
                rWriter.attribute("fontcolor", convertColorToString(rTextSimplePortionPrimitive2D.getFontColor()));

                const drawinglayer::attribute::FontAttribute& aFontAttribute = rTextSimplePortionPrimitive2D.getFontAttribute();
                rWriter.attribute("familyname", aFontAttribute.getFamilyName());
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
                rWriter.attribute("x", aFocusAttribute.getX());
                rWriter.attribute("y", aFocusAttribute.getY());

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

            default:
            {
                rWriter.element(OUStringToOString(sCurrentElementTag, RTL_TEXTENCODING_UTF8));
            }
            break;
        }

    }
}

} // end namespace drawinglayer::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
