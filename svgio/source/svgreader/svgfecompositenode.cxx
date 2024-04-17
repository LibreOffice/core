/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svgfecompositenode.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapArithmeticBlendFilter.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/BitmapTools.hxx>

namespace svgio::svgreader
{
SvgFeCompositeNode::SvgFeCompositeNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeComposite, rDocument, pParent)
    , maOperator(Operator::Over)
{
}

SvgFeCompositeNode::~SvgFeCompositeNode() {}

void SvgFeCompositeNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
{
    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Style:
        {
            readLocalCssStyle(aContent);
            break;
        }
        case SVGToken::In:
        {
            maIn = aContent.trim();
            break;
        }
        case SVGToken::In2:
        {
            maIn2 = aContent.trim();
            break;
        }
        case SVGToken::Result:
        {
            maResult = aContent.trim();
            break;
        }
        case SVGToken::Operator:
        {
            if (!aContent.isEmpty())
            {
                if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"over"))
                {
                    maOperator = Operator::Over;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"in"))
                {
                    maOperator = Operator::In;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"out"))
                {
                    maOperator = Operator::Out;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"xor"))
                {
                    maOperator = Operator::Xor;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"atop"))
                {
                    maOperator = Operator::Atop;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"arithmetic"))
                {
                    maOperator = Operator::Arithmetic;
                }
            }
            break;
        }
        case SVGToken::K1:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maK1 = aNum;
            }
            break;
        }
        case SVGToken::K2:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maK2 = aNum;
            }
            break;
        }
        case SVGToken::K3:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maK3 = aNum;
            }
            break;
        }
        case SVGToken::K4:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maK4 = aNum;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void SvgFeCompositeNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                               const SvgFilterNode* pParent) const
{
    if (maOperator != Operator::Arithmetic)
    {
        basegfx::B2DPolyPolygon aPolyPolygon, aPolyPolygon2;

        // Process maIn2 first
        if (const drawinglayer::primitive2d::Primitive2DContainer* pSource2
            = pParent->findGraphicSource(maIn2))
        {
            rTarget.append(*pSource2);
            drawinglayer::processor2d::ContourExtractor2D aExtractor(
                drawinglayer::geometry::ViewInformation2D(), true);
            aExtractor.process(*pSource2);
            const basegfx::B2DPolyPolygonVector& rResult(aExtractor.getExtractedContour());
            aPolyPolygon2 = basegfx::utils::mergeToSinglePolyPolygon(rResult);
        }

        if (const drawinglayer::primitive2d::Primitive2DContainer* pSource
            = pParent->findGraphicSource(maIn))
        {
            rTarget.append(*pSource);
            drawinglayer::processor2d::ContourExtractor2D aExtractor(
                drawinglayer::geometry::ViewInformation2D(), true);
            aExtractor.process(*pSource);
            const basegfx::B2DPolyPolygonVector& rResult(aExtractor.getExtractedContour());
            aPolyPolygon = basegfx::utils::mergeToSinglePolyPolygon(rResult);
        }

        basegfx::B2DPolyPolygon aResult;
        if (maOperator == Operator::Over)
        {
            aResult = basegfx::utils::solvePolygonOperationOr(aPolyPolygon, aPolyPolygon2);
        }
        else if (maOperator == Operator::Out)
        {
            aResult = basegfx::utils::solvePolygonOperationDiff(aPolyPolygon, aPolyPolygon2);
        }
        else if (maOperator == Operator::In)
        {
            aResult = basegfx::utils::solvePolygonOperationAnd(aPolyPolygon, aPolyPolygon2);
        }
        else if (maOperator == Operator::Xor)
        {
            aResult = basegfx::utils::solvePolygonOperationXor(aPolyPolygon, aPolyPolygon2);
        }
        else if (maOperator == Operator::Atop)
        {
            // Atop is the union of In and Out.
            // The parts of in2 graphic that do not overlap with the in graphic stay untouched.
            aResult = basegfx::utils::solvePolygonOperationDiff(aPolyPolygon2, aPolyPolygon);
            aResult.append(basegfx::utils::solvePolygonOperationAnd(aPolyPolygon, aPolyPolygon2));
        }

        rTarget = drawinglayer::primitive2d::Primitive2DContainer{
            new drawinglayer::primitive2d::MaskPrimitive2D(std::move(aResult), std::move(rTarget))
        };

        pParent->addGraphicSourceToMapper(maResult, rTarget);
    }
    else
    {
        basegfx::B2DRange aRange, aRange2;
        BitmapEx aBmpEx, aBmpEx2;

        if (const drawinglayer::primitive2d::Primitive2DContainer* pSource
            = pParent->findGraphicSource(maIn))
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            aRange = pSource->getB2DRange(aViewInformation2D);
            aBmpEx = convertToBitmapEx(pSource);
        }

        if (const drawinglayer::primitive2d::Primitive2DContainer* pSource2
            = pParent->findGraphicSource(maIn2))
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            aRange2 = pSource2->getB2DRange(aViewInformation2D);
            aBmpEx2 = convertToBitmapEx(pSource2);
        }

        basegfx::B2DRectangle aBaseRect(std::min(aRange.getMinX(), aRange2.getMinX()),
                                        std::min(aRange.getMinY(), aRange2.getMinY()),
                                        std::max(aRange.getMaxX(), aRange2.getMaxX()),
                                        std::max(aRange.getMaxY(), aRange2.getMaxY()));

        aBmpEx = vcl::bitmap::DrawBitmapInRect(aBmpEx, aRange, aBaseRect);
        aBmpEx2 = vcl::bitmap::DrawBitmapInRect(aBmpEx2, aRange2, aBaseRect);

        BitmapArithmeticBlendFilter* pArithmeticFilter
            = new BitmapArithmeticBlendFilter(aBmpEx, aBmpEx2);
        BitmapEx aResBmpEx = pArithmeticFilter->execute(maK1.getNumber(), maK2.getNumber(),
                                                        maK3.getNumber(), maK4.getNumber());

        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::BitmapPrimitive2D(
                aResBmpEx, basegfx::utils::createScaleTranslateB2DHomMatrix(
                               aBaseRect.getRange(), aBaseRect.getMinimum())));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
