/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svgfefloodnode.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
SvgFeFloodNode::SvgFeFloodNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeFlood, rDocument, pParent)
    , maX(0.0)
    , maY(0.0)
    , maWidth(0.0)
    , maHeight(0.0)
    , maFloodColor(SvgPaint())
    , maFloodOpacity(1.0)
{
}

SvgFeFloodNode::~SvgFeFloodNode() {}

void SvgFeFloodNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
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
        case SVGToken::Result:
        {
            maResult = aContent.trim();
            break;
        }
        case SVGToken::X:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maX = aNum;
            }
            break;
        }
        case SVGToken::Y:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maY = aNum;
            }
            break;
        }
        case SVGToken::Width:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                if (aNum.isPositive())
                {
                    maWidth = aNum;
                }
            }
            break;
        }
        case SVGToken::Height:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                if (aNum.isPositive())
                {
                    maHeight = aNum;
                }
            }
            break;
        }
        case SVGToken::FloodColor:
        {
            SvgPaint aSvgPaint;
            OUString aURL;
            SvgNumber aOpacity;

            if (readSvgPaint(aContent, aSvgPaint, aURL, aOpacity))
            {
                maFloodColor = aSvgPaint;
            }
            break;
        }
        case SVGToken::FloodOpacity:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maFloodOpacity = SvgNumber(std::clamp(aNum.getNumber(), 0.0, 1.0), aNum.getUnit(),
                                           aNum.isSet());
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

void SvgFeFloodNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                           const SvgFilterNode* pParent) const
{
    const double fWidth(maWidth.solve(*this, NumberType::xcoordinate));
    const double fHeight(maHeight.solve(*this, NumberType::ycoordinate));

    if (fWidth <= 0.0 || fHeight <= 0.0)
        return;

    if (const drawinglayer::primitive2d::Primitive2DContainer* rSource
        = pParent->findGraphicSource(maIn))
    {
        rTarget = *rSource;
    }

    const double fX(maX.solve(*this, NumberType::xcoordinate));
    const double fY(maY.solve(*this, NumberType::ycoordinate));
    const basegfx::B2DRange aRange(fX, fY, fX + fWidth, fY + fHeight);

    drawinglayer::primitive2d::Primitive2DReference xRef(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
            basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aRange)),
            maFloodColor.getBColor()));

    rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };

    const double fOpacity(maFloodOpacity.solve(*this));

    if (basegfx::fTools::less(fOpacity, 1.0))
    {
        xRef = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(std::move(rTarget),
                                                                             1.0 - fOpacity);

        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }

    pParent->addGraphicSourceToMapper(maResult, rTarget);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
