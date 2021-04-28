/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <svgrectnode.hxx>
#include <svgsvgnode.hxx>
#include <svgpathnode.hxx>
#include <svggradientnode.hxx>

#include <svgvisitor.hxx>

namespace svgio::svgreader
{
SvgDrawVisitor::SvgDrawVisitor()
    : mpDrawRoot(std::make_shared<gfx::DrawRoot>())
    , mpCurrent(mpDrawRoot)
{
}

void SvgDrawVisitor::visit(svgio::svgreader::SvgNode const& rNode)
{
    switch (rNode.getType())
    {
        case svgio::svgreader::SVGToken::Svg:
        {
            auto const& rSvgNode = static_cast<svgio::svgreader::SvgSvgNode const&>(rNode);

            basegfx::B2DRange aRange = rSvgNode.getCurrentViewPort();

            static_cast<gfx::DrawRoot*>(mpCurrent.get())->maRectangle = aRange;
        }
        break;
        case svgio::svgreader::SVGToken::Rect:
        {
            auto const& rRectNode = static_cast<svgio::svgreader::SvgRectNode const&>(rNode);

            double x = rRectNode.getX().getNumber();
            double y = rRectNode.getY().getNumber();
            double w = rRectNode.getWidth().getNumber();
            double h = rRectNode.getHeight().getNumber();

            auto pRectangle
                = std::make_shared<gfx::DrawRectangle>(basegfx::B2DRange(x, y, x + w, y + h));
            pRectangle->mnRx = rRectNode.getRx().getNumber();
            pRectangle->mnRy = rRectNode.getRy().getNumber();

            pRectangle->mnStrokeWidth
                = rRectNode.getSvgStyleAttributes()->getStrokeWidth().getNumber();

            pRectangle->mnOpacity = rRectNode.getSvgStyleAttributes()->getOpacity().getNumber();

            const basegfx::BColor* pFillColor = rRectNode.getSvgStyleAttributes()->getFill();
            const SvgGradientNode* pFillGradient
                = rRectNode.getSvgStyleAttributes()->getSvgGradientNodeFill();
            if (pFillColor)
            {
                pRectangle->mpFillColor = std::make_shared<basegfx::BColor>(*pFillColor);
            }
            else if (pFillGradient)
            {
                drawinglayer::primitive2d::SvgGradientEntryVector aSvgGradientEntryVector;
                pFillGradient->collectGradientEntries(aSvgGradientEntryVector);
                if (!aSvgGradientEntryVector.empty())
                {
                    auto aGradientInfo = std::make_shared<gfx::LinearGradientInfo>();

                    aGradientInfo->x1 = pFillGradient->getX1().getNumber();
                    aGradientInfo->y1 = pFillGradient->getY1().getNumber();
                    aGradientInfo->x2 = pFillGradient->getX2().getNumber();
                    aGradientInfo->y2 = pFillGradient->getY2().getNumber();

                    const basegfx::B2DHomMatrix* pGradientTransform
                        = pFillGradient->getGradientTransform();
                    if (pGradientTransform)
                    {
                        aGradientInfo->maMatrix = *pGradientTransform;
                    }

                    pRectangle->mpFillGradient = aGradientInfo;

                    for (auto const& rEntry : aSvgGradientEntryVector)
                    {
                        gfx::GradientStop aStop;
                        aStop.maColor = rEntry.getColor();
                        aStop.mfOffset = rEntry.getOffset();
                        aStop.mfOpacity = rEntry.getOpacity();
                        pRectangle->mpFillGradient->maGradientStops.push_back(aStop);
                    }
                }
            }

            const basegfx::BColor* pStrokeColor = rRectNode.getSvgStyleAttributes()->getStroke();
            if (pStrokeColor)
                pRectangle->mpStrokeColor = std::make_shared<basegfx::BColor>(*pStrokeColor);

            mpCurrent->maChildren.push_back(pRectangle);
        }
        break;
        case svgio::svgreader::SVGToken::Path:
        {
            auto const& rPathNode = static_cast<svgio::svgreader::SvgPathNode const&>(rNode);

            auto pPath = rPathNode.getPath();
            if (pPath)
            {
                auto pDrawPath = std::make_shared<gfx::DrawPath>(*pPath);

                pDrawPath->mnStrokeWidth
                    = rPathNode.getSvgStyleAttributes()->getStrokeWidth().getNumber();

                pDrawPath->mnOpacity = rPathNode.getSvgStyleAttributes()->getOpacity().getNumber();

                const basegfx::BColor* pFillColor = rPathNode.getSvgStyleAttributes()->getFill();
                if (pFillColor)
                    pDrawPath->mpFillColor = std::make_shared<basegfx::BColor>(*pFillColor);

                const basegfx::BColor* pStrokeColor
                    = rPathNode.getSvgStyleAttributes()->getStroke();
                if (pStrokeColor)
                    pDrawPath->mpStrokeColor = std::make_shared<basegfx::BColor>(*pStrokeColor);

                mpCurrent->maChildren.push_back(pDrawPath);
            }
        }
        break;

        default:
            break;
    }
    goToChildren(rNode);
}

void SvgDrawVisitor::goToChildren(svgio::svgreader::SvgNode const& rNode)
{
    for (auto& rChild : rNode.getChildren())
    {
        rChild->accept(*this);
    }
}
} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
