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

#include <svgpolynode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace svgio::svgreader
{
        SvgPolyNode::SvgPolyNode(
            SVGToken aType,
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(aType, rDocument, pParent),
            maSvgStyleAttributes(*this)
        {
        }

        SvgPolyNode::~SvgPolyNode()
        {
        }

        const SvgStyleAttributes* SvgPolyNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle(maSvgStyleAttributes);
        }

        void SvgPolyNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(aSVGToken, aContent);

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
                case SVGToken::Points:
                {
                    basegfx::B2DPolygon aPath;

                    if(basegfx::utils::importFromSvgPoints(aPath, aContent))
                    {
                        if(aPath.count())
                        {
                            if(getType() == SVGToken::Polygon)
                            {
                                aPath.setClosed(true);
                            }

                            setPolygon(aPath);
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

        void SvgPolyNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && mpPolygon)
            {
                drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                pStyle->add_path(basegfx::B2DPolyPolygon(*mpPolygon), aNewTarget, nullptr);

                if(!aNewTarget.empty())
                {
                    pStyle->add_postProcess(rTarget, std::move(aNewTarget), getTransform());
                }
            }
        }
} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
