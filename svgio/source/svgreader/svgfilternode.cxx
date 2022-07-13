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

#include <svgfilternode.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

namespace svgio::svgreader
{
SvgFilterNode::SvgFilterNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgNode(SVGToken::Filter, rDocument, pParent)
    , maSvgStyleAttributes(*this)
    , maX(SvgNumber(-10.0, SvgUnit::percent, true))
    , maY(SvgNumber(-10.0, SvgUnit::percent, true))
    , maWidth(SvgNumber(120.0, SvgUnit::percent, true))
    , maHeight(SvgNumber(120.0, SvgUnit::percent, true))
    , maFilterUnits(SvgUnits::objectBoundingBox)
{
}

SvgFilterNode::~SvgFilterNode() {}

const SvgStyleAttributes* SvgFilterNode::getSvgStyleAttributes() const
{
    return &maSvgStyleAttributes;
}

void SvgFilterNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken,
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
        case SVGToken::FilterUnits:
        {
            if (!aContent.isEmpty())
            {
                if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent),
                                                commonStrings::aStrUserSpaceOnUse))
                {
                    setFilterUnits(SvgUnits::userSpaceOnUse);
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent),
                                                     commonStrings::aStrObjectBoundingBox))
                {
                    setFilterUnits(SvgUnits::objectBoundingBox);
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

void SvgFilterNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                                     bool bReferenced) const
{
    drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

    // decompose children
    SvgNode::decomposeSvgNode(aNewTarget, bReferenced);

    if (aNewTarget.empty())
        return;

    // append to current target
    rTarget.append(aNewTarget);
}

void SvgFilterNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                          const std::optional<basegfx::B2DHomMatrix>& pTransform) const
{
    if (rTarget.empty() || Display::None == getDisplay())
        return;

    drawinglayer::primitive2d::Primitive2DContainer aFilterTarget;

    // get mask definition as primitives
    decomposeSvgNode(aFilterTarget, true);

    if (!aFilterTarget.empty())
    {
        // get size range and create path
        const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

        if (!(pStyle && getWidth().isSet() && getHeight().isSet()))
            return;

        const double fWidth(getWidth().solve(*this, NumberType::xcoordinate));
        const double fHeight(getHeight().solve(*this, NumberType::ycoordinate));

        if (fWidth <= 0.0 || fHeight <= 0.0)
            return;

        const double fX(getX().isSet() ? getX().solve(*this, NumberType::xcoordinate) : 0.0);
        const double fY(getY().isSet() ? getY().solve(*this, NumberType::ycoordinate) : 0.0);
        const basegfx::B2DRange aRange(fX, fY, fX + fWidth, fY + fHeight);
        basegfx::B2DPolygon aPath;

        aPath = basegfx::utils::createPolygonFromRect(aRange);

        drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

        pStyle->add_path(basegfx::B2DPolyPolygon(aPath), aNewTarget, nullptr);

        if (!aNewTarget.empty())
        {
            pStyle->add_postProcess(rTarget, std::move(aNewTarget), pTransform);
        }
    }
    else
    {
        // An empty clipping path will completely clip away the element that had
        // the clip-path property applied. (Svg spec)
        rTarget.clear();
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
