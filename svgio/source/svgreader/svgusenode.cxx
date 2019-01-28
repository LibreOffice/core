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

namespace svgio
{
    namespace svgreader
    {
        SvgUseNode::SvgUseNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenG, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maX(),
            maY(),
            maWidth(),
            maHeight(),
            maXLink(),
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
            maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent, false);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGTokenTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                case SVGTokenX:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maX = aNum;
                    }
                    break;
                }
                case SVGTokenY:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maY = aNum;
                    }
                    break;
                }
                case SVGTokenWidth:
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
                case SVGTokenHeight:
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
                case SVGTokenXlinkHref:
                {
                    const sal_Int32 nLen(aContent.getLength());

                    if(nLen && '#' == aContent[0])
                    {
                        maXLink = aContent.copy(1);
                    }
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

            if (!(pXLink && Display_none != pXLink->getDisplay() && !mbDecomposingSvgNode))
                return;

            // decompose children
            drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

            // todo: in case mpXLink is a SVGTokenSvg or SVGTokenSymbol the
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
                    getX().solve(*this, xcoordinate),
                    getY().solve(*this, ycoordinate));
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
                        aNewTarget));

                rTarget.push_back(xRef);
            }
            else
            {
                rTarget.append(aNewTarget);
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
