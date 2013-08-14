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

#include <svgio/svgreader/svgcirclenode.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgCircleNode::SvgCircleNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenCircle, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maCx(0),
            maCy(0),
            maR(0),
            mpaTransform(0)
        {
        }

        SvgCircleNode::~SvgCircleNode()
        {
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgCircleNode::getSvgStyleAttributes() const
        {
            static OUString aClassStr(OUString::createFromAscii("circle"));
            return checkForCssStyle(aClassStr, maSvgStyleAttributes);
        }

        void SvgCircleNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    maSvgStyleAttributes.readStyle(aContent);
                    break;
                }
                case SVGTokenCx:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setCx(aNum);
                    }
                    break;
                }
                case SVGTokenCy:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setCy(aNum);
                    }
                    break;
                }
                case SVGTokenR:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setR(aNum);
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

        void SvgCircleNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool /*bReferenced*/) const
        {
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && getR().isSet())
            {
                const double fR(getR().solve(*this, length));

                if(fR > 0.0)
                {
                    const basegfx::B2DPolygon aPath(
                        basegfx::tools::createPolygonFromCircle(
                            basegfx::B2DPoint(
                                getCx().isSet() ? getCx().solve(*this, xcoordinate) : 0.0,
                                getCy().isSet() ? getCy().solve(*this, ycoordinate) : 0.0),
                            fR));

                    drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                    pStyle->add_path(basegfx::B2DPolyPolygon(aPath), aNewTarget);

                    if(aNewTarget.hasElements())
                    {
                        pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
                    }
                }
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
