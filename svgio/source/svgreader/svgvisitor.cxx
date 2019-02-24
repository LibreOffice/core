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
#include <sal/log.hxx>

#include <svgdocumenthandler.hxx>
#include <svgrectnode.hxx>
#include <svgsvgnode.hxx>
#include <svggnode.hxx>
#include <svgpathnode.hxx>

#include <svgvisitor.hxx>

namespace svgio
{
namespace svgreader
{
SvgDrawVisitor::SvgDrawVisitor()
    : mpDrawRoot(std::make_shared<DrawRoot>())
    , mpCurrent(mpDrawRoot)
{
}

void SvgDrawVisitor::visit(svgio::svgreader::SvgNode const& rNode)
{
    switch (rNode.getType())
    {
        case svgio::svgreader::SVGTokenSvg:
        {
            auto const& rSvgNode = static_cast<svgio::svgreader::SvgSvgNode const&>(rNode);

            double x = rSvgNode.getX().getNumber();
            double y = rSvgNode.getY().getNumber();
            double w = rSvgNode.getWidth().getNumber();
            double h = rSvgNode.getHeight().getNumber();

            static_cast<DrawRoot*>(mpCurrent.get())->maRectangle
                = basegfx::B2DRange(x, y, x + w, y + h);
        }
        break;
        case svgio::svgreader::SVGTokenG:
        {
            auto const& rGNode = static_cast<svgio::svgreader::SvgGNode const&>(rNode);

            if (rGNode.getTransform() != nullptr)
            {
                basegfx::B2DHomMatrix rMatrix = *rGNode.getTransform();

                printf("G [%f %f %f - %f %f %f - %f %f %f]\n", rMatrix.get(0, 0), rMatrix.get(0, 1),
                       rMatrix.get(0, 2), rMatrix.get(1, 0), rMatrix.get(1, 1), rMatrix.get(1, 2),
                       rMatrix.get(2, 0), rMatrix.get(2, 1), rMatrix.get(2, 2));
            }
        }
        break;
        case svgio::svgreader::SVGTokenRect:
        {
            auto const& rRectNode = static_cast<svgio::svgreader::SvgRectNode const&>(rNode);

            double x = rRectNode.getX().getNumber();
            double y = rRectNode.getY().getNumber();
            double w = rRectNode.getWidth().getNumber();
            double h = rRectNode.getHeight().getNumber();

            auto pRectangle
                = std::make_shared<DrawRectangle>(basegfx::B2DRange(x, y, x + w, y + h));
            mpCurrent->maChildren.push_back(pRectangle);
        }
        break;
        case svgio::svgreader::SVGTokenPath:
        {
            auto const& rPathNode = static_cast<svgio::svgreader::SvgPathNode const&>(rNode);
            auto pPath = rPathNode.getPath();
            if (pPath)
            {
                auto pDrawPath = std::make_shared<DrawPath>(*pPath);
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
}
} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
