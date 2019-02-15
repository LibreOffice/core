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

#include <svgclippathnode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace svgio
{
    namespace svgreader
    {
        SvgClipPathNode::SvgClipPathNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenClipPathNode, rDocument, pParent),
            maSvgStyleAttributes(*this),
            maClipPathUnits(userSpaceOnUse)
        {
        }

        SvgClipPathNode::~SvgClipPathNode()
        {
        }

        const SvgStyleAttributes* SvgClipPathNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgClipPathNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                case SVGTokenClipPathUnits:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse))
                        {
                            setClipPathUnits(userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox))
                        {
                            setClipPathUnits(objectBoundingBox);
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

        void SvgClipPathNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
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

                rTarget.push_back(xRef);
            }
            else
            {
                // append to current target
                rTarget.append(aNewTarget);
            }
        }

        void SvgClipPathNode::apply(
            drawinglayer::primitive2d::Primitive2DContainer& rContent,
            const basegfx::B2DHomMatrix* pTransform) const
        {
            if(rContent.empty() || Display_none == getDisplay())
                return;

            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            drawinglayer::primitive2d::Primitive2DContainer aClipTarget;
            basegfx::B2DPolyPolygon aClipPolyPolygon;

            // get clipPath definition as primitives
            decomposeSvgNode(aClipTarget, true);

            if(!aClipTarget.empty())
            {
                // extract filled polygons as base for a mask PolyPolygon
                drawinglayer::processor2d::ContourExtractor2D aExtractor(aViewInformation2D, true);

                aExtractor.process(aClipTarget);

                const basegfx::B2DPolyPolygonVector& rResult(aExtractor.getExtractedContour());
                const sal_uInt32 nSize(rResult.size());

                if(nSize > 1)
                {
                    // merge to single clipPolyPolygon
                    aClipPolyPolygon = basegfx::utils::mergeToSinglePolyPolygon(rResult);
                }
                else
                {
                    aClipPolyPolygon = rResult[0];
                }
            }

            if(aClipPolyPolygon.count())
            {
                if(objectBoundingBox == getClipPathUnits())
                {
                    // clip is object-relative, transform using content transformation
                    const basegfx::B2DRange aContentRange(rContent.getB2DRange(aViewInformation2D));

                    aClipPolyPolygon.transform(
                        basegfx::utils::createScaleTranslateB2DHomMatrix(
                            aContentRange.getRange(),
                            aContentRange.getMinimum()));
                }
                else // userSpaceOnUse
                {
                    // #i124852#
                    if(pTransform)
                    {
                        aClipPolyPolygon.transform(*pTransform);
                    }
                }

                // #i124313# try to avoid creating an embedding to a MaskPrimitive2D if
                // possible; MaskPrimitive2D processing is potentially expensive
                bool bCreateEmbedding(true);
                bool bAddContent(true);

                if(basegfx::utils::isRectangle(aClipPolyPolygon))
                {
                    // ClipRegion is a rectangle, thus it is not expensive to tell
                    // if the content is completely inside or outside of it; get ranges
                    const basegfx::B2DRange aClipRange(aClipPolyPolygon.getB2DRange());
                    const basegfx::B2DRange aContentRange(
                        rContent.getB2DRange(
                            aViewInformation2D));

                    if(aClipRange.isInside(aContentRange))
                    {
                        // completely contained, no need to clip at all, so no need for embedding
                        bCreateEmbedding = false;
                    }
                    else if(aClipRange.overlaps(aContentRange))
                    {
                        // overlap; embedding needed. ClipRegion can be minimized by using
                        // the intersection of the ClipRange and the ContentRange. Minimizing
                        // the ClipRegion potentially enhances further processing since
                        // usually clip operations are expensive.
                        basegfx::B2DRange aCommonRange(aContentRange);

                        aCommonRange.intersect(aClipRange);
                        aClipPolyPolygon = basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aCommonRange));
                    }
                    else
                    {
                        // not inside and no overlap -> completely outside
                        // no need for embedding, no need for content at all
                        bCreateEmbedding = false;
                        bAddContent = false;
                    }
                }
                else
                {
                    // ClipRegion is not a simple rectangle, it would be possible but expensive to
                    // tell if the content needs clipping or not. It is also dependent of
                    // the content's decomposition. To do this, a processor would be needed that
                    // is capable if processing the given sequence of primitives and decide
                    // if all is inside or all is outside. Such a ClipProcessor could be written,
                    // but for now just create the embedding
                }

                if(bCreateEmbedding)
                {
                    // redefine target. Use MaskPrimitive2D with created clip
                    // geometry. Using the automatically set mbIsClipPathContent at
                    // SvgStyleAttributes the clip definition is without fill, stroke,
                    // and strokeWidth and forced to black
                    const drawinglayer::primitive2d::Primitive2DReference xEmbedTransparence(
                        new drawinglayer::primitive2d::MaskPrimitive2D(
                            aClipPolyPolygon,
                            rContent));

                    rContent = drawinglayer::primitive2d::Primitive2DContainer { xEmbedTransparence };
                }
                else
                {
                    if(!bAddContent)
                    {
                        rContent.clear();
                    }
                }
            }
            else
            {
                // An empty clipping path will completely clip away the element that had
                // the clip-path property applied. (Svg spec)
                rContent.clear();
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
