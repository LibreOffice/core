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
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
SvgFeGaussianBlurNode::SvgFeGaussianBlurNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeGaussianBlur, rDocument, pParent)
    , maStdDeviation(SvgNumber(0.0))
{
}

SvgFeGaussianBlurNode::~SvgFeGaussianBlurNode() {}

void SvgFeGaussianBlurNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
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
            maIn = aContent.trim();
            break;
        }
        case SVGToken::Result:
        {
            maResult = aContent.trim();
            break;
        }
        default:
        {
            break;
        }
    }
}

void SvgFeGaussianBlurNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                                  const SvgFilterNode* pParent) const
{
    if (const drawinglayer::primitive2d::Primitive2DContainer* rSource
        = pParent->findGraphicSource(maIn))
    {
        rTarget = *rSource;
    }

    const drawinglayer::primitive2d::Primitive2DReference xRef(
        new drawinglayer::primitive2d::SoftEdgePrimitive2D(maStdDeviation.getNumber(),
                                                           std::move(rTarget)));

    rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };

    pParent->addGraphicSourceToMapper(maResult, rTarget);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
