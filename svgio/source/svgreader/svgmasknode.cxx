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

#include <svgmasknode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

namespace svgio::svgreader
{
        SvgMaskNode::SvgMaskNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Mask, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maX(SvgNumber(-10.0, SvgUnit::percent, true)),
            maY(SvgNumber(-10.0, SvgUnit::percent, true)),
            maWidth(SvgNumber(120.0, SvgUnit::percent, true)),
            maHeight(SvgNumber(120.0, SvgUnit::percent, true)),
            maMaskUnits(SvgUnits::objectBoundingBox),
            maMaskContentUnits(SvgUnits::userSpaceOnUse)
        {
        }

        SvgMaskNode::~SvgMaskNode()
        {
        }

        const SvgStyleAttributes* SvgMaskNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgMaskNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGToken::Transform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                case SVGToken::MaskUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse))
                        {
                            setMaskUnits(SvgUnits::userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox))
                        {
                            setMaskUnits(SvgUnits::objectBoundingBox);
                        }
                    }
                    break;
                }
                case SVGToken::MaskContentUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse))
                        {
                            setMaskContentUnits(SvgUnits::userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox))
                        {
                            setMaskContentUnits(SvgUnits::objectBoundingBox);
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgMaskNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DContainer aNewTarget;

            // decompose children
            SvgNode::decomposeSvgNode(aNewTarget, bReferenced);

            if(aNewTarget.empty())
                return;

            if(getTransform())
            {
                // create embedding group element with transformation
                const drawinglayer::primitive2d::Primitive2DReference xRef(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        *getTransform(),
                        aNewTarget));

                aNewTarget = drawinglayer::primitive2d::Primitive2DContainer { xRef };
            }

            // append to current target
            rTarget.append(aNewTarget);
        }

        void SvgMaskNode::apply(
            drawinglayer::primitive2d::Primitive2DContainer& rTarget,
            const basegfx::B2DHomMatrix* pTransform) const
        {
            if(rTarget.empty() || Display_none == getDisplay())
                return;

            drawinglayer::primitive2d::Primitive2DContainer aMaskTarget;

            // get mask definition as primitives
            decomposeSvgNode(aMaskTarget, true);

            if(!aMaskTarget.empty())
            {
                // get range of content to be masked
                const basegfx::B2DRange aContentRange(
                        rTarget.getB2DRange(
                            drawinglayer::geometry::ViewInformation2D()));
                const double fContentWidth(aContentRange.getWidth());
                const double fContentHeight(aContentRange.getHeight());

                if(fContentWidth > 0.0 && fContentHeight > 0.0)
                {
                    // create OffscreenBufferRange
                    basegfx::B2DRange aOffscreenBufferRange;

                    if (SvgUnits::objectBoundingBox == maMaskUnits)
                    {
                        // fractions or percentages of the bounding box of the element to which the mask is applied
                        const double fX(SvgUnit::percent == getX().getUnit() ? getX().getNumber() * 0.01 : getX().getNumber());
                        const double fY(SvgUnit::percent == getY().getUnit() ? getY().getNumber() * 0.01 : getY().getNumber());
                        const double fW(SvgUnit::percent == getWidth().getUnit() ? getWidth().getNumber() * 0.01 : getWidth().getNumber());
                        const double fH(SvgUnit::percent == getHeight().getUnit() ? getHeight().getNumber() * 0.01 : getHeight().getNumber());

                        aOffscreenBufferRange = basegfx::B2DRange(
                            aContentRange.getMinX() + (fX * fContentWidth),
                            aContentRange.getMinY() + (fY * fContentHeight),
                            aContentRange.getMinX() + ((fX + fW) * fContentWidth),
                            aContentRange.getMinY() + ((fY + fH) * fContentHeight));
                    }
                    else
                    {
                        const double fX(getX().isSet() ? getX().solve(*this, NumberType::xcoordinate) : 0.0);
                        const double fY(getY().isSet() ? getY().solve(*this, NumberType::ycoordinate) : 0.0);

                        aOffscreenBufferRange = basegfx::B2DRange(
                            fX,
                            fY,
                            fX + (getWidth().isSet() ? getWidth().solve(*this, NumberType::xcoordinate) : 0.0),
                            fY + (getHeight().isSet() ? getHeight().solve(*this, NumberType::ycoordinate) : 0.0));
                    }

                    if (SvgUnits::objectBoundingBox == maMaskContentUnits)
                    {
                        // mask is object-relative, embed in content transformation
                        const drawinglayer::primitive2d::Primitive2DReference xTransform(
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                basegfx::utils::createScaleTranslateB2DHomMatrix(
                                    aContentRange.getRange(),
                                    aContentRange.getMinimum()),
                                aMaskTarget));

                        aMaskTarget = drawinglayer::primitive2d::Primitive2DContainer { xTransform };
                    }
                    else // userSpaceOnUse
                    {
                        // #i124852#
                        if(pTransform)
                        {
                            const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                new drawinglayer::primitive2d::TransformPrimitive2D(
                                    *pTransform,
                                    aMaskTarget));

                            aMaskTarget = drawinglayer::primitive2d::Primitive2DContainer { xTransform };
                        }
                    }

                    // embed content to a ModifiedColorPrimitive2D since the definitions
                    // how content is used as alpha is special for Svg
                    {
                        const drawinglayer::primitive2d::Primitive2DReference xInverseMask(
                            new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                                aMaskTarget,
                                std::make_shared<basegfx::BColorModifier_luminance_to_alpha>()));

                        aMaskTarget = drawinglayer::primitive2d::Primitive2DContainer { xInverseMask };
                    }

                    // prepare new content
                    drawinglayer::primitive2d::Primitive2DReference xNewContent(
                        new drawinglayer::primitive2d::TransparencePrimitive2D(
                            rTarget,
                            aMaskTarget));

                    // output up to now is defined by aContentRange and mask is oriented
                    // relative to it. It is possible that aOffscreenBufferRange defines
                    // a smaller area. In that case, embed to a mask primitive
                    if(!aOffscreenBufferRange.isInside(aContentRange))
                    {
                        xNewContent = new drawinglayer::primitive2d::MaskPrimitive2D(
                            basegfx::B2DPolyPolygon(
                                basegfx::utils::createPolygonFromRect(
                                    aOffscreenBufferRange)),
                            drawinglayer::primitive2d::Primitive2DContainer { xNewContent });
                    }

                    // redefine target. Use TransparencePrimitive2D with created mask
                    // geometry
                    rTarget = drawinglayer::primitive2d::Primitive2DContainer { xNewContent };
                }
                else
                {
                    // content is geometrically empty
                    rTarget.clear();
                }
            }
            else
            {
                // An empty clipping path will completely clip away the element that had
                // the clip-path property applied. (Svg spec)
                rTarget.clear();
            }
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
