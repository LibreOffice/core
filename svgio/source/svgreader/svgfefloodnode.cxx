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

#include <svgfefloodnode.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace svgio::svgreader
{
SvgFeFlood::SvgFeFlood(SvgDocument& rDocument, SvgNode* pParent)
    : SvgNode(SVGToken::FeFlood, rDocument, pParent)
    , maSvgStyleAttributes(*this)
{
}

SvgFeFlood::~SvgFeFlood() {}

const SvgStyleAttributes* SvgFeFlood::getSvgStyleAttributes() const
{
    return &maSvgStyleAttributes;
}

void SvgFeFlood::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken,
                                const OUString& aContent)
{
    // call parent
    SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

    // read style attributes
    maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent, false);

    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Style:
        {
            readLocalCssStyle(aContent);
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
        default:
        {
            break;
        }
    }
}

void SvgFeFlood::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                                  bool /*bReferenced*/) const
{
    /*TODO*/
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
