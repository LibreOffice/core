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
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgPolyNode::SvgPolyNode(
            SvgDocument& rDocument,
            SvgNode* pParent,
            bool bIsPolyline)
        :   SvgNode(SVGTokenPolygon, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpPolygon(nullptr),
            mpaTransform(nullptr),
            mbIsPolyline(bIsPolyline)
        {
        }

        SvgPolyNode::~SvgPolyNode()
        {
            delete mpaTransform;
            delete mpPolygon;
        }

        const SvgStyleAttributes* SvgPolyNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle(mbIsPolyline? OUString("polyline") : OUString("polygon"), maSvgStyleAttributes);
        }

        void SvgPolyNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenPoints:
                {
                    basegfx::B2DPolygon aPath;

                    if(basegfx::utils::importFromSvgPoints(aPath, aContent))
                    {
                        if(aPath.count())
                        {
                            if(!mbIsPolyline)
                            {
                                aPath.setClosed(true);
                            }

                            setPolygon(&aPath);
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

        void SvgPolyNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && mpPolygon)
            {
                drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                pStyle->add_path(basegfx::B2DPolyPolygon(*mpPolygon), aNewTarget, nullptr);

                if(!aNewTarget.empty())
                {
                    pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
                }
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
