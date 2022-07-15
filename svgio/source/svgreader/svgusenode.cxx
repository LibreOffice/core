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

#include <svgusenode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svgdocument.hxx>

namespace svgio::svgreader
{
        SvgUseNode::SvgUseNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Use, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mbDecomposingSvgNode(false)
        {
        }

        SvgUseNode::~SvgUseNode()
        {
        }

        const SvgStyleAttributes* SvgUseNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("use", maSvgStyleAttributes);
        }

        void SvgUseNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGToken::Style:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGToken::Transform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(aMatrix);
                    }
                    break;
                }
                case SVGToken::X:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maX = aNum;
                    }
                    break;
                }
                case SVGToken::Y:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maY = aNum;
                    }
                    break;
                }
                case SVGToken::Width:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maWidth = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::Height:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maHeight = aNum;
                        }
                    }
                    break;
                }
                case SVGToken::Href:
                case SVGToken::XlinkHref:
                {
                    readLocalLink(aContent, maXLink);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgUseNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            // try to access link to content
            const SvgNode* pXLink = getDocument().findSvgNodeById(maXLink);

            if (!pXLink || Display::None == pXLink->getDisplay() || mbDecomposingSvgNode)
                return;

            // decompose children
            drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

            // todo: in case mpXLink is a SVGToken::Svg or SVGToken::Symbol the
            // SVG docs want the getWidth() and getHeight() from this node
            // to be valid for the subtree.
            mbDecomposingSvgNode = true;
            const_cast< SvgNode* >(pXLink)->setAlternativeParent(this);
            pXLink->decomposeSvgNode(aNewTarget, true);
            const_cast< SvgNode* >(pXLink)->setAlternativeParent();
            mbDecomposingSvgNode = false;

            if(aNewTarget.empty())
                return;

            basegfx::B2DHomMatrix aTransform;

            if(getX().isSet() || getY().isSet())
            {
                aTransform.translate(
                    getX().solve(*this, NumberType::xcoordinate),
                    getY().solve(*this, NumberType::ycoordinate));
            }

            if(getTransform())
            {
                aTransform = *getTransform() * aTransform;
            }

            if(!aTransform.isIdentity())
            {
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aTransform,
                        std::move(aNewTarget)));

                rTarget.push_back(xRef);
            }
            else
            {
                rTarget.append(aNewTarget);
            }
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
