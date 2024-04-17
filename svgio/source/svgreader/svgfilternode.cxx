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
#include <svgfecolormatrixnode.hxx>
#include <svgfedropshadownode.hxx>
#include <svgfefloodnode.hxx>
#include <svgfeimagenode.hxx>
#include <svgfegaussianblurnode.hxx>
#include <svgfeoffsetnode.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/converters.hxx>

namespace svgio::svgreader
{
SvgFilterNode::SvgFilterNode(SVGToken aType, SvgDocument& rDocument, SvgNode* pParent)
    : SvgNode(aType, rDocument, pParent)
{
}

SvgFilterNode::~SvgFilterNode() {}

void SvgFilterNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                          const SvgFilterNode* /*pParent*/) const
{
    if (rTarget.empty())
        return;

    const auto& rChildren = getChildren();
    const sal_uInt32 nCount(rChildren.size());

    addGraphicSourceToMapper("SourceGraphic", rTarget);

    // TODO: For now, map SourceAlpha, BackgroundImage,
    // BackgroundAlpha, FillPaint and StrokePaint to rTarget
    // so at least something is displayed
    addGraphicSourceToMapper("SourceAlpha", rTarget);
    addGraphicSourceToMapper("BackgroundImage", rTarget);
    addGraphicSourceToMapper("BackgroundAlpha", rTarget);
    addGraphicSourceToMapper("FillPaint", rTarget);
    addGraphicSourceToMapper("StrokePaint", rTarget);

    // apply children's filters
    for (sal_uInt32 a(0); a < nCount; a++)
    {
        SvgFilterNode* pFilterNode = dynamic_cast<SvgFilterNode*>(rChildren[a].get());
        if (pFilterNode)
        {
            pFilterNode->apply(rTarget, this);
        }
    }
}

void SvgFilterNode::addGraphicSourceToMapper(
    const OUString& rStr, drawinglayer::primitive2d::Primitive2DContainer pGraphicSource) const
{
    if (!rStr.isEmpty())
    {
        const_cast<SvgFilterNode*>(this)->maIdGraphicSourceMapperList.emplace(rStr, pGraphicSource);
    }
}

const drawinglayer::primitive2d::Primitive2DContainer*
SvgFilterNode::findGraphicSource(const OUString& rStr) const
{
    if (rStr.isEmpty())
        return nullptr;

    const IdGraphicSourceMapper::const_iterator aResult(maIdGraphicSourceMapperList.find(rStr));
    if (aResult == maIdGraphicSourceMapperList.end())
    {
        return nullptr;
    }
    else
    {
        return &aResult->second;
    }
}

BitmapEx
SvgFilterNode::convertToBitmapEx(const drawinglayer::primitive2d::Primitive2DContainer* pSeq)
{
    drawinglayer::primitive2d::Primitive2DContainer aSequence(*pSeq);

    const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    basegfx::B2DRange aRange = aSequence.getB2DRange(aViewInformation2D);
    basegfx::B2DHomMatrix aEmbedding(
        basegfx::utils::createTranslateB2DHomMatrix(-aRange.getMinX(), -aRange.getMinY()));
    aEmbedding.scale(aRange.getWidth(), aRange.getHeight());
    const drawinglayer::primitive2d::Primitive2DReference xEmbedRef(
        new drawinglayer::primitive2d::TransformPrimitive2D(aEmbedding, std::move(aSequence)));
    drawinglayer::primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };
    return drawinglayer::convertToBitmapEx(std::move(xEmbedSeq), aViewInformation2D,
                                           aRange.getWidth(), aRange.getHeight(), 500000);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
