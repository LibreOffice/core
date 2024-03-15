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

#include <svgfecompositenode.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

namespace svgio::svgreader
{
SvgFeCompositeNode::SvgFeCompositeNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgFilterNode(SVGToken::FeComposite, rDocument, pParent)
    , maOperator(Operator::Over)
{
}

SvgFeCompositeNode::~SvgFeCompositeNode() {}

void SvgFeCompositeNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
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
        case SVGToken::Operator:
        {
            if (!aContent.isEmpty())
            {
                if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"over"))
                {
                    maOperator = Operator::Over;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"in"))
                {
                    maOperator = Operator::In;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"out"))
                {
                    maOperator = Operator::Out;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"xor"))
                {
                    maOperator = Operator::Xor;
                }
                else if (o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"atop"))
                {
                    maOperator = Operator::Atop;
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

void SvgFeCompositeNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                               const SvgFilterNode* pParent) const
{
    basegfx::B2DPolyPolygon aPolyPolygon, aPolyPolygon2;

    // Process maIn2 first
    if (const drawinglayer::primitive2d::Primitive2DContainer* pSource2
        = pParent->findGraphicSource(maIn2))
    {
        rTarget.append(*pSource2);
        const basegfx::B2DRange aRange2(
            pSource2->getB2DRange(drawinglayer::geometry::ViewInformation2D()));

        aPolyPolygon2 = basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aRange2));
    }

    if (const drawinglayer::primitive2d::Primitive2DContainer* pSource
        = pParent->findGraphicSource(maIn))
    {
        rTarget.append(*pSource);
        const basegfx::B2DRange aRange(
            pSource->getB2DRange(drawinglayer::geometry::ViewInformation2D()));

        aPolyPolygon = basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aRange));
    }

    basegfx::B2DPolyPolygon aResult;
    if (maOperator == Operator::Over)
    {
        aResult = basegfx::utils::solvePolygonOperationOr(aPolyPolygon, aPolyPolygon2);
    }
    else if (maOperator == Operator::Out)
    {
        aResult = basegfx::utils::solvePolygonOperationDiff(aPolyPolygon, aPolyPolygon2);
    }
    else if (maOperator == Operator::In)
    {
        aResult = basegfx::utils::solvePolygonOperationAnd(aPolyPolygon, aPolyPolygon2);
    }
    else if (maOperator == Operator::Xor)
    {
        aResult = basegfx::utils::solvePolygonOperationXor(aPolyPolygon, aPolyPolygon2);
    }
    else if (maOperator == Operator::Atop)
    {
        // Atop is the union of In and Out.
        // The parts of in2 graphic that do not overlap with the in graphic stay untouched.
        aResult = basegfx::utils::solvePolygonOperationDiff(aPolyPolygon2, aPolyPolygon);
        aResult.append(basegfx::utils::solvePolygonOperationAnd(aPolyPolygon, aPolyPolygon2));
    }

    rTarget = drawinglayer::primitive2d::Primitive2DContainer{
        new drawinglayer::primitive2d::MaskPrimitive2D(std::move(aResult), std::move(rTarget))
    };

    pParent->addGraphicSourceToMapper(maResult, rTarget);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
