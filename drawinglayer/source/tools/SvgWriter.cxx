/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sal/log.hxx>

#include <drawinglayer/tools/SvgWriter.hxx>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>

#include <primitive2d/textlineprimitive2d.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace svg
{
SvgWriter::SvgWriter(SvStream& rStream, SvgVersion eSvgVersion)
    : mrStream(rStream)
    , maWriter(&mrStream)
    , meSvgVersion(eSvgVersion)
{
}

bool SvgWriter::write(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence)
{
    maWriter.startDocument();
    maWriter.startElement("svg");
    OString aVersion = meSvgVersion == SvgVersion::v1_1 ? "1.1" : "2";
    maWriter.attribute("version", aVersion);
    maWriter.attribute("xmlns", "http://www.w3.org/2000/svg");

    drawinglayer::primitive2d::VisitorParameters aParameters; // default

    auto aRange = rPrimitive2DSequence.getB2DRange(aParameters);

    maWriter.attribute("width", aRange.getWidth());
    maWriter.attribute("height", aRange.getHeight());

    decomposeAndWrite(rPrimitive2DSequence);

    maWriter.endElement();
    maWriter.endDocument();

    return true;
}

void SvgWriter::decomposeAndWrite(
    const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence)
{
    drawinglayer::primitive2d::VisitorParameters aVisitorParameters;

    for (size_t i = 0; i < rPrimitive2DSequence.size(); i++)
    {
        auto xPrimitive2DReference = rPrimitive2DSequence[i];

        const auto* pBasePrimitive
            = dynamic_cast<const drawinglayer::primitive2d::BasePrimitive2D*>(
                xPrimitive2DReference.get());

        if (!pBasePrimitive)
            continue;

        sal_uInt32 nId = pBasePrimitive->getPrimitive2DID();

        switch (nId)
        {
            case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
            {
            }
            break;
            case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
            {
                ;
            }
            break;
            case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
            {
            }
            break;
            case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
            {
            }
            break;
            case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
            {
            }
            break;

            case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
            {
            }
            break;

            default:
            {
            }
            break;
        }
    }
}

} // end namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
