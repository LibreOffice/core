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

#include <svglinenode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgLineNode::SvgLineNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenLine, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maX1(0),
            maY1(0),
            maX2(0),
            maY2(0)
        {
        }

        SvgLineNode::~SvgLineNode()
        {
        }

        const SvgStyleAttributes* SvgLineNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("line", maSvgStyleAttributes);
        }

        void SvgLineNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenX1:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maX1 = aNum;
                    }
                    break;
                }
                case SVGTokenY1:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maY1 = aNum;
                    }
                    break;
                }
                case SVGTokenX2:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maX2 = aNum;
                    }
                    break;
                }
                case SVGTokenY2:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maY2 = aNum;
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

        void SvgLineNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(!pStyle)
                return;

            const basegfx::B2DPoint X(
                getX1().isSet() ? getX1().solve(*this, xcoordinate) : 0.0,
                getY1().isSet() ? getY1().solve(*this, ycoordinate) : 0.0);
            const basegfx::B2DPoint Y(
                getX2().isSet() ? getX2().solve(*this, xcoordinate) : 0.0,
                getY2().isSet() ? getY2().solve(*this, ycoordinate) : 0.0);

            // X and Y may be equal, do not drop them. Markers or linecaps 'round' and 'square'
            // need to be drawn for zero-length lines too.

            basegfx::B2DPolygon aPath;

            aPath.append(X);
            aPath.append(Y);

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
