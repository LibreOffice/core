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

#include <svgellipsenode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace svgio::svgreader
{
        SvgEllipseNode::SvgEllipseNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Ellipse, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maCx(0),
            maCy(0),
            maRx(0),
            maRy(0)
        {
        }

        SvgEllipseNode::~SvgEllipseNode()
        {
        }

        const SvgStyleAttributes* SvgEllipseNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("ellipse", maSvgStyleAttributes);
        }

        void SvgEllipseNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGToken::Cx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maCx = aNum;
                    }
                    break;
                }
                case SVGToken::Cy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maCy = aNum;
                    }
                    break;
                }
                case SVGToken::Rx:
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
                case SVGToken::Ry:
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

        void SvgEllipseNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(!(pStyle && getRx().isSet() && getRy().isSet()))
                return;

            const double fRx(getRx().solve(*this, NumberType::xcoordinate));
            const double fRy(getRy().solve(*this, NumberType::ycoordinate));

            if(fRx <= 0.0 || fRy <= 0.0)
                return;

            const basegfx::B2DPolygon aPath(
                basegfx::utils::createPolygonFromEllipse(
                    basegfx::B2DPoint(
                        getCx().isSet() ? getCx().solve(*this, NumberType::xcoordinate) : 0.0,
                        getCy().isSet() ? getCy().solve(*this, NumberType::ycoordinate) : 0.0),
                    fRx, fRy));

            drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

            pStyle->add_path(basegfx::B2DPolyPolygon(aPath), aNewTarget, nullptr);

            if(!aNewTarget.empty())
            {
                pStyle->add_postProcess(rTarget, std::move(aNewTarget), getTransform());
            }
        }
} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
