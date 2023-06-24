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

#include <svgfegaussianblurnode.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
SvgFeGaussianBlurNode::SvgFeGaussianBlurNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgNode(SVGToken::FeGaussianBlur, rDocument, pParent)
    , maStdDeviation(SvgNumber(0.0))
    , maIn(In::SourceGraphic)
{
}

SvgFeGaussianBlurNode::~SvgFeGaussianBlurNode() {}

void SvgFeGaussianBlurNode::parseAttribute(const OUString& /*rTokenName*/, SVGToken aSVGToken,
                                           const OUString& aContent)
{
    // parse own
    switch (aSVGToken)
    {
        case SVGToken::StdDeviation:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                if (aNum.isPositive())
                {
                    maStdDeviation = aNum;
                }
            }
            break;
        }
        case SVGToken::In:
        {
            if (!aContent.isEmpty())
            {
                if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"SourceGraphic"))
                {
                    maIn = In::SourceGraphic;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"SourceAlpha"))
                {
                    maIn = In::SourceAlpha;
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

void SvgFeGaussianBlurNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget) const
{
    if (maStdDeviation.getNumber() != 0.0)
    {
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::SoftEdgePrimitive2D(maStdDeviation.getNumber(),
                                                               std::move(rTarget)));

        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }

    if (maIn == In::SourceAlpha)
    {
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                std::move(rTarget), std::make_shared<basegfx::BColorModifier_alpha>()));

        rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
