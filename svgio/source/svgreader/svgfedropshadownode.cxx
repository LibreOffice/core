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

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svgfedropshadownode.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
SvgFeDropShadowNode::SvgFeDropShadowNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeDropShadow, rDocument, pParent)
    , maDx(0.0)
    , maDy(0.0)
    , maStdDeviation(0.0)
    , maFloodColor(SvgPaint())
    , maFloodOpacity(1.0)
{
}

SvgFeDropShadowNode::~SvgFeDropShadowNode() {}

void SvgFeDropShadowNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
{
    // call parent
    SvgFilterNode::parseAttribute(aSVGToken, aContent);

    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Style:
        {
            readLocalCssStyle(aContent);
            break;
        }
        case SVGToken::Dx:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maDx = aNum;
            }
            break;
        }
        case SVGToken::Dy:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maDy = aNum;
            }
            break;
        }
        case SVGToken::StdDeviation:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                maStdDeviation = aNum;
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

void SvgFeDropShadowNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget) const
{
    basegfx::B2DHomMatrix aTransform;
    if (maDx.isSet() || maDy.isSet())
    {
        aTransform.translate(maDx.solve(*this, NumberType::xcoordinate),
                             maDy.solve(*this, NumberType::ycoordinate));
    }

    drawinglayer::primitive2d::Primitive2DContainer aTempTarget;

    // Create the shadow
    aTempTarget.append(new drawinglayer::primitive2d::ShadowPrimitive2D(
        aTransform, maFloodColor.getBColor(), maStdDeviation.getNumber(),
        drawinglayer::primitive2d::Primitive2DContainer(rTarget)));

    const double fOpacity(maFloodOpacity.solve(*this));
    if (basegfx::fTools::less(fOpacity, 1.0))
    {
        // Apply transparence to the shadow
        aTempTarget.append(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            std::move(aTempTarget), 1.0 - fOpacity));
    }

    // Append the original target
    aTempTarget.append(rTarget);

    rTarget = aTempTarget;
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
