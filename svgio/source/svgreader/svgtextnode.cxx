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

#include <svgtextnode.hxx>
#include <svgcharacternode.hxx>
#include <svgstyleattributes.hxx>
#include <svgtrefnode.hxx>
#include <svgtextpathnode.hxx>
#include <svgtspannode.hxx>
#include <osl/diagnose.h>

namespace svgio::svgreader
{
        SvgTextNode::SvgTextNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgTspanNode(SVGToken::Text, rDocument, pParent)
        {
        }

        SvgTextNode::~SvgTextNode()
        {
        }

        void SvgTextNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgTspanNode::parseAttribute(aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGToken::Transform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(aMatrix);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgTextNode::addTextPrimitives(
            const SvgNode& rCandidate,
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            drawinglayer::primitive2d::Primitive2DContainer&& rSource)
        {
            if(rSource.empty())
                return;

            const SvgStyleAttributes* pAttributes = rCandidate.getSvgStyleAttributes();

            if(pAttributes)
            {
                // add text with taking all Fill/Stroke attributes into account
                pAttributes->add_text(rTarget, std::move(rSource));
            }
            else
            {
                // should not happen, every subnode from SvgTextNode will at least
                // return the attributes from SvgTextNode. Nonetheless, add text
                rTarget.append(std::move(rSource));
            }
        }

        void SvgTextNode::DecomposeChild(const SvgNode& rCandidate, drawinglayer::primitive2d::Primitive2DContainer& rTarget, SvgTextPosition& rSvgTextPosition) const
        {
            switch(rCandidate.getType())
            {
                case SVGToken::Character:
                {
                    // direct SvgTextPathNode derivates, decompose them
                    const SvgCharacterNode& rSvgCharacterNode = static_cast< const SvgCharacterNode& >(rCandidate);
                    rSvgCharacterNode.decomposeText(rTarget, rSvgTextPosition);
                    break;
                }
                case SVGToken::TextPath:
                {
                    // direct TextPath decompose
                    const SvgTextPathNode& rSvgTextPathNode = static_cast< const SvgTextPathNode& >(rCandidate);
                    const auto& rChildren = rSvgTextPathNode.getChildren();
                    const sal_uInt32 nCount(rChildren.size());

                    if(nCount && rSvgTextPathNode.isValid())
                    {
                        // remember original TextStart to later detect hor/ver offsets
                        const basegfx::B2DPoint aTextStart(rSvgTextPosition.getPosition());
                        drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                        // decompose to regular TextPrimitives
                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            DecomposeChild(*rChildren[a], aNewTarget, rSvgTextPosition);
                        }

                        if(!aNewTarget.empty())
                        {
                            const drawinglayer::primitive2d::Primitive2DContainer aPathContent(aNewTarget);
                            aNewTarget.clear();

                            // dismantle TextPrimitives and map them on curve/path
                            rSvgTextPathNode.decomposePathNode(aPathContent, aNewTarget, aTextStart);
                        }

                        if(!aNewTarget.empty())
                        {
                            addTextPrimitives(rCandidate, rTarget, std::move(aNewTarget));
                        }
                    }

                    break;
                }
                case SVGToken::Tspan:
                {
                    // Tspan may have children, call recursively
                    const SvgTspanNode& rSvgTspanNode = static_cast< const SvgTspanNode& >(rCandidate);
                    const auto& rChildren = rSvgTspanNode.getChildren();
                    const sal_uInt32 nCount(rChildren.size());

                    if(nCount)
                    {
                        SvgTextPosition aSvgTextPosition(&rSvgTextPosition, rSvgTspanNode);
                        drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            DecomposeChild(*rChildren[a], aNewTarget, aSvgTextPosition);
                        }

                        rSvgTextPosition.setPosition(aSvgTextPosition.getPosition());

                        if(!aNewTarget.empty())
                        {
                            addTextPrimitives(rCandidate, rTarget, std::move(aNewTarget));
                        }
                    }
                    break;
                }
                case SVGToken::Tref:
                {
                    const SvgTrefNode& rSvgTrefNode = static_cast< const SvgTrefNode& >(rCandidate);
                    const SvgTextNode* pRefText = rSvgTrefNode.getReferencedSvgTextNode();

                    if(pRefText)
                    {
                        const auto& rChildren = pRefText->getChildren();
                        const sal_uInt32 nCount(rChildren.size());
                        drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                        if(nCount)
                        {
                            for(sal_uInt32 a(0); a < nCount; a++)
                            {
                                const SvgNode& rChildCandidate = *rChildren[a];
                                const_cast< SvgNode& >(rChildCandidate).setAlternativeParent(this);

                                DecomposeChild(rChildCandidate, aNewTarget, rSvgTextPosition);
                                const_cast< SvgNode& >(rChildCandidate).setAlternativeParent();
                            }

                            if(!aNewTarget.empty())
                            {
                                addTextPrimitives(rCandidate, rTarget, std::move(aNewTarget));
                            }
                        }
                    }

                    break;
                }
                default:
                {
                    OSL_ENSURE(false, "Unexpected node in text token (!)");
                    break;
                }
            }
        }

        void SvgTextNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced`*/) const
        {
            // text has a group of child nodes, allowed are SVGToken::Character, SVGToken::Tspan,
            // SVGToken::Tref and SVGToken::TextPath. These increase a given current text position
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(!pStyle || getChildren().empty())
                return;

            const double fOpacity(pStyle->getOpacity().getNumber());

            if(fOpacity <= 0.0)
                return;

            SvgTextPosition aSvgTextPosition(nullptr, *this);
            drawinglayer::primitive2d::Primitive2DContainer aNewTarget;
            const auto& rChildren = getChildren();
            const sal_uInt32 nCount(rChildren.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const SvgNode& rCandidate = *rChildren[a];

                DecomposeChild(rCandidate, aNewTarget, aSvgTextPosition);
            }

            if(!aNewTarget.empty())
            {
                drawinglayer::primitive2d::Primitive2DContainer aNewTarget2;

                addTextPrimitives(*this, aNewTarget2, std::move(aNewTarget));
                aNewTarget = std::move(aNewTarget2);
            }

            if(!aNewTarget.empty())
            {
                pStyle->add_postProcess(rTarget, std::move(aNewTarget), getTransform());
            }
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
