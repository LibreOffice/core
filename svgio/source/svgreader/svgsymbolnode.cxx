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

#include <svgsymbolnode.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

namespace svgio::svgreader
{
        SvgSymbolNode::SvgSymbolNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Symbol, rDocument, pParent),
            maSvgStyleAttributes(*this)
        {
        }

        SvgSymbolNode::~SvgSymbolNode()
        {
        }

        const SvgStyleAttributes* SvgSymbolNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgSymbolNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGToken::ViewBox:
                {
                    const basegfx::B2DRange aRange(readViewBox(aContent, *this));

                    if(!aRange.isEmpty())
                    {
                        setViewBox(&aRange);
                    }
                    break;
                }
                case SVGToken::PreserveAspectRatio:
                {
                    maSvgAspectRatio = readSvgAspectRatio(aContent);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgSymbolNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
        {
            // decompose children
            SvgNode::decomposeSvgNode(rTarget, bReferenced);

            if (rTarget.empty())
                return;

            if(getViewBox())
            {
                // create mapping
                // #i122610 SVG 1.1 defines in section 5.1.2 that if the attribute preserveAspectRatio is not specified,
                // then the effect is as if a value of 'xMidYMid meet' were specified.
                SvgAspectRatio aRatioDefault(SvgAlign::xMidYMid, true);
                const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                const double fX(maX.solve(*this, NumberType::xcoordinate));
                const double fY(maY.solve(*this, NumberType::ycoordinate));
                const double fWidth(maWidth.solve(*this, NumberType::xcoordinate));
                const double fHeight(maHeight.solve(*this, NumberType::ycoordinate));
                const basegfx::B2DRange aRange(fX, fY, fX + fWidth, fY + fHeight);

                // let mapping be created from SvgAspectRatio
                const basegfx::B2DHomMatrix aEmbeddingTransform(
                    rRatio.createMapping(aRange, *getViewBox()));

                // prepare embedding in transformation
                // create embedding group element with transformation
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        aEmbeddingTransform,
                        drawinglayer::primitive2d::Primitive2DContainer(rTarget)));

                rTarget.push_back(xRef);
            }
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
