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

#include <svgrectnode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgRectNode::SvgRectNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenRect, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maX(0),
            maY(0),
            maWidth(0),
            maHeight(0),
            maRx(0),
            maRy(0)
        {
        }

        SvgRectNode::~SvgRectNode()
        {
        }

        const SvgStyleAttributes* SvgRectNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("rect", maSvgStyleAttributes);
        }

        void SvgRectNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenRx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maRx = aNum;
                        }
                    }
                    break;
                }
                case SVGTokenRy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            maRy = aNum;
                        }
                    }
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
                default:
                {
                    break;
                }
            }
        }

        void SvgRectNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            // get size range and create path
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(!(pStyle && getWidth().isSet() && getHeight().isSet()))
                return;

            const double fWidth(getWidth().solve(*this, xcoordinate));
            const double fHeight(getHeight().solve(*this, ycoordinate));

            if(!(fWidth > 0.0 && fHeight > 0.0))
                return;

            const double fX(getX().isSet() ? getX().solve(*this, xcoordinate) : 0.0);
            const double fY(getY().isSet() ? getY().solve(*this, ycoordinate) : 0.0);
            const basegfx::B2DRange aRange(fX, fY, fX + fWidth, fY + fHeight);
            basegfx::B2DPolygon aPath;

            if(getRx().isSet() || getRy().isSet())
            {
                double frX(getRx().isSet() ? getRx().solve(*this, xcoordinate) : 0.0);
                double frY(getRy().isSet() ? getRy().solve(*this, ycoordinate) : 0.0);

                frX = std::max(0.0, frX);
                frY = std::max(0.0, frY);

                if(0.0 == frY && frX > 0.0)
                {
                    frY = frX;
                }
                else if(0.0 == frX && frY > 0.0)
                {
                    frX = frY;
                }

                frX /= fWidth;
                frY /= fHeight;

                frX = std::min(0.5, frX);
                frY = std::min(0.5, frY);

                aPath = basegfx::utils::createPolygonFromRect(aRange, frX * 2.0, frY * 2.0);
            }
            else
            {
                aPath = basegfx::utils::createPolygonFromRect(aRange);
            }

            drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

            pStyle->add_path(basegfx::B2DPolyPolygon(aPath), aNewTarget, nullptr);

            if(!aNewTarget.empty())
            {
                pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
