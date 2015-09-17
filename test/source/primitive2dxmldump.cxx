/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/primitive2dxmldump.hxx>
#include <test/xmltesttools.hxx>

#include <vcl/metaact.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include <memory>

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

#include <drawinglayer/attribute/lineattribute.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace drawinglayer::primitive2d;

namespace
{

const size_t constMaxActionType = 513;

OUString convertColorToString(const basegfx::BColor& rColor)
{
    OUString aRGBString = Color(rColor).AsRGBHexString();
    return "#" + aRGBString;
}

} // anonymous namespace

Primitive2dXmlDump::Primitive2dXmlDump() :
    maFilter(constMaxActionType, false)
{}

Primitive2dXmlDump::~Primitive2dXmlDump()
{}

void Primitive2dXmlDump::filterActionType(const sal_uInt16 nActionType, bool bShouldFilter)
{
    maFilter[nActionType] = bShouldFilter;
}

void Primitive2dXmlDump::filterAllActionTypes()
{
    maFilter.assign(constMaxActionType, true);
}

xmlDocPtr Primitive2dXmlDump::dumpAndParse(
    const drawinglayer::primitive2d::Primitive2DSequence& rPrimitive2DSequence,
    const OUString& rTempStreamName)
{
    std::unique_ptr<SvStream> pStream;

    if (rTempStreamName.isEmpty())
        pStream.reset(new SvMemoryStream());
    else
        pStream.reset(new SvFileStream(rTempStreamName, STREAM_STD_READWRITE | StreamMode::TRUNC));

    XmlWriter aWriter(pStream.get());
    aWriter.startDocument();
    aWriter.startElement("primitive2D");

    decomposeAndWrite(rPrimitive2DSequence, aWriter);

    aWriter.endElement();
    aWriter.endDocument();

    pStream->Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocPtr pDoc = XmlTestTools::parseXmlStream(pStream.get());

    return pDoc;
}

void Primitive2dXmlDump::decomposeAndWrite(
    const drawinglayer::primitive2d::Primitive2DSequence& rPrimitive2DSequence,
    XmlWriter& rWriter)
{
    for (int i = 0; i < rPrimitive2DSequence.getLength(); i++)
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
                const HiddenGeometryPrimitive2D* pHiddenGeometryPrimitive2D = dynamic_cast<const HiddenGeometryPrimitive2D*>(pBasePrimitive);
                rWriter.startElement("hiddengeometry");
                decomposeAndWrite(pHiddenGeometryPrimitive2D->getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
            {
                const TransformPrimitive2D* pTransformPrimitive2D = dynamic_cast<const TransformPrimitive2D*>(pBasePrimitive);
                rWriter.startElement("transform");
                //pTransformPrimitive2D->getTransformation()
                decomposeAndWrite(pTransformPrimitive2D->getChildren(), rWriter);
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
            {
                const PolyPolygonColorPrimitive2D* pPolyPolygonColorPrimitive2D = dynamic_cast<const PolyPolygonColorPrimitive2D*>(pBasePrimitive);

                rWriter.startElement("polypolygoncolor");
                rWriter.attribute("color", convertColorToString(pPolyPolygonColorPrimitive2D->getBColor()));
                rWriter.startElement("polypolygon");
                rWriter.content(basegfx::tools::exportToSvgD(pPolyPolygonColorPrimitive2D->getB2DPolyPolygon(), true, true, false));
                rWriter.endElement();
                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
            {
                const PolyPolygonStrokePrimitive2D* pPolyPolygonStrokePrimitive2D = dynamic_cast<const PolyPolygonStrokePrimitive2D*>(pBasePrimitive);
                rWriter.startElement("polypolygonstroke");

                rWriter.startElement("line");
                drawinglayer::attribute::LineAttribute aLineAttribute = pPolyPolygonStrokePrimitive2D->getLineAttribute();
                rWriter.attribute("color", convertColorToString(aLineAttribute.getColor()));
                rWriter.attribute("width", aLineAttribute.getWidth());
                //rWriter.attribute("linejoin", aLineAttribute.getLineJoin());
                //rWriter.attribute("linecap", aLineAttribute.getLineCap());
                rWriter.endElement();

                //getStrokeAttribute()

                rWriter.startElement("polypolygon");
                rWriter.content(basegfx::tools::exportToSvgD(pPolyPolygonStrokePrimitive2D->getB2DPolyPolygon(), true, true, false));
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
            {
                const PolygonHairlinePrimitive2D* pPolygonHairlinePrimitive2D = dynamic_cast<const PolygonHairlinePrimitive2D*>(pBasePrimitive);
                rWriter.startElement("polygonhairline");

                rWriter.attribute("color", convertColorToString(pPolygonHairlinePrimitive2D->getBColor()));

                rWriter.startElement("polygon");
                rWriter.content(basegfx::tools::exportToSvgPoints(pPolygonHairlinePrimitive2D->getB2DPolygon()));
                rWriter.endElement();


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
