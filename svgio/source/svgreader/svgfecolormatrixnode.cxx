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
    : SvgNode(SVGToken::FeColorMatrix, rDocument, pParent)
    , maType(ColorType::None)
    , maValues(1.0)
{
}

SvgFeColorMatrixNode::~SvgFeColorMatrixNode() {}

void SvgFeColorMatrixNode::parseAttribute(const OUString& /*rTokenName*/, SVGToken aSVGToken,
                                          const OUString& aContent)
{
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
            }
            break;
        }
        case SVGToken::Values:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maValues = aNum;
            }
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
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                std::move(rTarget),
                std::make_shared<basegfx::BColorModifier_saturate>(maValues.getNumber())));
        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
