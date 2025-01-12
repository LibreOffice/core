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

        void SvgSymbolNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
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
            drawinglayer::primitive2d::Primitive2DContainer aContent;
            SvgNode::decomposeSvgNode(aContent, bReferenced);

            // no geometry provided, done
            if (aContent.empty())
                return;

            // if no ViewBox append aContent without embedding
            if(nullptr == getViewBox())
            {
                rTarget.append(aContent);
                return;
            }

            // prepare range of imported geometry
            // tdf#164434 CAUTION: There *are* svg files which do not define
            // all needed data, e.g. x/y/w/h might be missing in any combination.
            // As fallback, use size of imported geometry. Do this separate for
            // position and size to do the best if only one value pair is provided
            basegfx::B2DRange aImportedRange;
            basegfx::B2DPoint aPosition;
            basegfx::B2DVector aSize;

            // evaluate position
            if (maX.isSet() && maY.isSet())
            {
                // use values from imported svg if both provided
                aPosition = basegfx::B2DPoint(
                    maX.solve(*this, NumberType::xcoordinate),
                    maY.solve(*this, NumberType::ycoordinate));
            }
            else
            {
                // fallback to imported geometry
                drawinglayer::geometry::ViewInformation2D aViewInfo;
                aImportedRange = aContent.getB2DRange(aViewInfo);
                aPosition = aImportedRange.getMinimum();
            }

            // evaluate size
            if (maWidth.isSet() && maHeight.isSet())
            {
                // use values from imported svg if both provided
                aSize = basegfx::B2DVector(
                    maWidth.solve(*this, NumberType::xcoordinate),
                    maHeight.solve(*this, NumberType::ycoordinate));
            }
            else
            {
                // fallback to imported geometry
                if (aImportedRange.isEmpty())
                {
                    drawinglayer::geometry::ViewInformation2D aViewInfo;
                    aImportedRange = aContent.getB2DRange(aViewInfo);
                }

                aSize = aImportedRange.getRange();
            }

            // tdf#164434 no size, no geometry, done
            if (0.0 == aSize.getX() || 0.0 == aSize.getY())
                return;

            // create mapping using SvgAspectRatio
            const SvgAspectRatio& rRatio = getSvgAspectRatio();
            const basegfx::B2DHomMatrix aEmbeddingTransform(
                rRatio.createMapping(basegfx::B2DRange(aPosition, aPosition + aSize), *getViewBox()));

            // prepare embedding in transformation
            // create embedding group element with transformation
            const drawinglayer::primitive2d::Primitive2DReference xRef(
                new drawinglayer::primitive2d::TransformPrimitive2D(
                    aEmbeddingTransform,
                    drawinglayer::primitive2d::Primitive2DContainer(std::move(aContent))));

            // add embedded geometry to result
            rTarget.push_back(xRef);
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
