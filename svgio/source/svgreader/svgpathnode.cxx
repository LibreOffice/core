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

#include <svgpathnode.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace svgio::svgreader
{
        SvgPathNode::SvgPathNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Path, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maPathLength()
        {
        }

        SvgPathNode::~SvgPathNode()
        {
        }

        const SvgStyleAttributes* SvgPathNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle("path", maSvgStyleAttributes);
        }

        void SvgPathNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent, false);

            // parse own
            switch(aSVGToken)
            {
                case SVGToken::Style:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGToken::D:
                {
                    basegfx::B2DPolyPolygon aPath;

                    if(basegfx::utils::importFromSvgD(aPath, aContent, false, &maHelpPointIndices))
                    {
                        if(aPath.count())
                        {
                            setPath(&aPath);
                        }
                    }
                    break;
                }
                case SVGToken::Transform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                case SVGToken::PathLength:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maPathLength = aNum;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgPathNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool /*bReferenced*/) const
        {
            // fill and/or stroke needed, also a path
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && getPath())
            {
                drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

                pStyle->add_path(*getPath(), aNewTarget, &maHelpPointIndices);

                if(!aNewTarget.empty())
                {
                    pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
                }
            }
        }
} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
