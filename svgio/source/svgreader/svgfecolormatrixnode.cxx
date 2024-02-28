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

#include <svgfecolormatrixnode.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
SvgFeColorMatrixNode::SvgFeColorMatrixNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeColorMatrix, rDocument, pParent)
    , maType(ColorType::Matrix)
{
}

SvgFeColorMatrixNode::~SvgFeColorMatrixNode() {}

void SvgFeColorMatrixNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
{
    // call parent
    SvgFilterNode::parseAttribute(aSVGToken, aContent);

    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Type:
        {
            if (!aContent.isEmpty())
            {
                if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"luminanceToAlpha"))
                {
                    maType = ColorType::LuminanceToAlpha;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"saturate"))
                {
                    maType = ColorType::Saturate;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"hueRotate"))
                {
                    maType = ColorType::HueRotate;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"matrix"))
                {
                    maType = ColorType::Matrix;
                }
            }
            break;
        }
        case SVGToken::Values:
        {
            maValuesContent = aContent;
            break;
        }
        default:
        {
            break;
        }
    }
}

void SvgFeColorMatrixNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget) const
{
    if (maType == ColorType::LuminanceToAlpha)
    {
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                std::move(rTarget),
                std::make_shared<basegfx::BColorModifier_luminance_to_alpha>()));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
    else if (maType == ColorType::Saturate)
    {
        SvgNumber aNum(1.0);
        (void)readSingleNumber(maValuesContent, aNum);

        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                std::move(rTarget),
                std::make_shared<basegfx::BColorModifier_saturate>(aNum.getNumber())));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
    else if (maType == ColorType::HueRotate)
    {
        SvgNumber aNum(0.0);
        (void)readSingleNumber(maValuesContent, aNum);
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                std::move(rTarget), std::make_shared<basegfx::BColorModifier_hueRotate>(
                                        basegfx::deg2rad(aNum.getNumber()))));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
    else if (maType == ColorType::Matrix)
    {
        std::vector<double> aVector = readFilterMatrix(maValuesContent, *this);

        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                std::move(rTarget), std::make_shared<basegfx::BColorModifier_matrix>(aVector)));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
