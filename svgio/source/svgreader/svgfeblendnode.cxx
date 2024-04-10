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

#include <svgfeblendnode.hxx>
#include <o3tl/string_view.hxx>

#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <vcl/bitmapex.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/BitmapScreenBlendFilter.hxx>
#include <vcl/BitmapTools.hxx>

namespace svgio::svgreader
{
SvgFeBlendNode::SvgFeBlendNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeBlend, rDocument, pParent)
    , maMode(Mode::Normal)
{
}

SvgFeBlendNode::~SvgFeBlendNode() {}

void SvgFeBlendNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
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
        case SVGToken::Mode:
        {
            if (!aContent.isEmpty())
            {
                if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"normal"))
                {
                    maMode = Mode::Normal;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"screen"))
                {
                    maMode = Mode::Screen;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void SvgFeBlendNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                           const SvgFilterNode* pParent) const
{
    if (maMode == Mode::Normal)
    {
        if (const drawinglayer::primitive2d::Primitive2DContainer* rSource2
            = pParent->findGraphicSource(maIn2))
        {
            rTarget = *rSource2;
        }

        if (const drawinglayer::primitive2d::Primitive2DContainer* rSource
            = pParent->findGraphicSource(maIn))
        {
            rTarget.append(*rSource);
        }
    }
    else if (maMode == Mode::Screen)
    {
        basegfx::B2DRange aRange, aRange2;
        BitmapEx aBmpEx, aBmpEx2;

        if (const drawinglayer::primitive2d::Primitive2DContainer* pSource
            = pParent->findGraphicSource(maIn))
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            aRange = pSource->getB2DRange(aViewInformation2D);
            basegfx::B2DHomMatrix aEmbedding(
                basegfx::utils::createTranslateB2DHomMatrix(-aRange.getMinX(), -aRange.getMinY()));
            aEmbedding.scale(aRange.getWidth(), aRange.getHeight());
            const drawinglayer::primitive2d::Primitive2DReference xEmbedRef(
                new drawinglayer::primitive2d::TransformPrimitive2D(
                    aEmbedding, drawinglayer::primitive2d::Primitive2DContainer(*pSource)));
            drawinglayer::primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };
            aBmpEx = drawinglayer::convertToBitmapEx(std::move(xEmbedSeq), aViewInformation2D,
                                                     aRange.getWidth(), aRange.getHeight(), 500000);
        }

        if (const drawinglayer::primitive2d::Primitive2DContainer* pSource2
            = pParent->findGraphicSource(maIn2))
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            aRange2 = pSource2->getB2DRange(aViewInformation2D);
            basegfx::B2DHomMatrix aEmbedding(basegfx::utils::createTranslateB2DHomMatrix(
                -aRange2.getMinX(), -aRange2.getMinY()));
            aEmbedding.scale(aRange2.getWidth(), aRange2.getHeight());
            const drawinglayer::primitive2d::Primitive2DReference xEmbedRef(
                new drawinglayer::primitive2d::TransformPrimitive2D(
                    aEmbedding, drawinglayer::primitive2d::Primitive2DContainer(*pSource2)));
            drawinglayer::primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };
            aBmpEx2
                = drawinglayer::convertToBitmapEx(std::move(xEmbedSeq), aViewInformation2D,
                                                  aRange2.getWidth(), aRange2.getHeight(), 500000);
        }

        basegfx::B2DRectangle aBaseRect(std::min(aRange.getMinX(), aRange2.getMinX()),
                                        std::min(aRange.getMinY(), aRange2.getMinY()),
                                        std::max(aRange.getMaxX(), aRange2.getMaxX()),
                                        std::max(aRange.getMaxY(), aRange2.getMaxY()));

        aBmpEx = vcl::bitmap::DrawBitmapInRect(aBmpEx, aRange, aBaseRect);
        aBmpEx2 = vcl::bitmap::DrawBitmapInRect(aBmpEx2, aRange2, aBaseRect);

        BitmapScreenBlendFilter* pScreenBlendFilter = new BitmapScreenBlendFilter(aBmpEx, aBmpEx2);
        BitmapEx aResBmpEx = pScreenBlendFilter->execute();

        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::BitmapPrimitive2D(
                aResBmpEx, basegfx::utils::createScaleTranslateB2DHomMatrix(
                               aBaseRect.getRange(), aBaseRect.getMinimum())));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }

    pParent->addGraphicSourceToMapper(maResult, rTarget);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
