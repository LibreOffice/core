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

#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svgfeoffsetnode.hxx>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{
SvgFeOffsetNode::SvgFeOffsetNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeOffset, rDocument, pParent)
    , maDx(SvgNumber(0.0))
    , maDy(SvgNumber(0.0))
{
}

SvgFeOffsetNode::~SvgFeOffsetNode() {}

void SvgFeOffsetNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
{
    // call parent
    SvgFilterNode::parseAttribute(aSVGToken, aContent);

    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Dx:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                if (aNum.isPositive())
                {
                    maDx = aNum;
                }
            }
            break;
        }
        case SVGToken::Dy:
        {
            SvgNumber aNum;

            if (readSingleNumber(aContent, aNum))
            {
                if (aNum.isPositive())
                {
                    maDy = aNum;
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

void SvgFeOffsetNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget) const
{
    basegfx::B2DHomMatrix aTransform;

    if (maDx.isSet() || maDy.isSet())
    {
        aTransform.translate(maDx.solve(*this, NumberType::xcoordinate),
                             maDy.solve(*this, NumberType::ycoordinate));
    }

    const drawinglayer::primitive2d::Primitive2DReference xRef(
        new drawinglayer::primitive2d::TransformPrimitive2D(aTransform, std::move(rTarget)));

    rTarget = drawinglayer::primitive2d::Primitive2DContainer{ xRef };
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
