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

namespace svgio::svgreader
{
SvgFilterNode::SvgFilterNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgNode(SVGToken::Filter, rDocument, pParent)
{
}

SvgFilterNode::~SvgFilterNode() {}

void SvgFilterNode::apply(drawinglayer::primitive2d::Primitive2DContainer& rTarget) const
{
    if (rTarget.empty())
        return;

    const auto& rChildren = getChildren();
    const sal_uInt32 nCount(rChildren.size());

    // apply children's filters
    for (sal_uInt32 a(0); a < nCount; a++)
    {
        SvgNode* pCandidate = rChildren[a].get();
        if (pCandidate->getType() == SVGToken::FeGaussianBlur)
        {
            const SvgFeGaussianBlurNode& rFeGaussianBlurNode
                = dynamic_cast<const SvgFeGaussianBlurNode&>(*pCandidate);
            rFeGaussianBlurNode.apply(rTarget);
        }
        else if (pCandidate->getType() == SVGToken::FeColorMatrix)
        {
            const SvgFeColorMatrixNode& rFeColorMatrixNode
                = dynamic_cast<const SvgFeColorMatrixNode&>(*pCandidate);
            rFeColorMatrixNode.apply(rTarget);
        }
        else if (pCandidate->getType() == SVGToken::FeOffset)
        {
            const SvgFeOffsetNode& rFeOffsetNode
                = dynamic_cast<const SvgFeOffsetNode&>(*pCandidate);
            rFeOffsetNode.apply(rTarget);
        }
        else if (pCandidate->getType() == SVGToken::FeFlood)
        {
            const SvgFeFloodNode& rFeFloodNode = dynamic_cast<const SvgFeFloodNode&>(*pCandidate);
            rFeFloodNode.apply(rTarget);
        }
        else if (pCandidate->getType() == SVGToken::FeDropShadow)
        {
            const SvgFeDropShadowNode& rFeDropShadowNode
                = dynamic_cast<const SvgFeDropShadowNode&>(*pCandidate);
            rFeDropShadowNode.apply(rTarget);
        }
        else if (pCandidate->getType() == SVGToken::FeImage)
        {
            const SvgFeImageNode& rFeImageNode = dynamic_cast<const SvgFeImageNode&>(*pCandidate);
            rFeImageNode.apply(rTarget);
        }
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
