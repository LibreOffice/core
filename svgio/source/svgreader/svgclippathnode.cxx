/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svgio/svgreader/svgclippathnode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>



namespace svgio
{
    namespace svgreader
    {
        SvgClipPathNode::SvgClipPathNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenClipPathNode, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpaTransform(0),
            maClipPathUnits(userSpaceOnUse)
        {
        }

        SvgClipPathNode::~SvgClipPathNode()
        {
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgClipPathNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgClipPathNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent);

            
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    maSvgStyleAttributes.readStyle(aContent);
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
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse, 0))
                        {
                            setClipPathUnits(userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox, 0))
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

        void SvgClipPathNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

            
            SvgNode::decomposeSvgNode(aNewTarget, bReferenced);

            if(aNewTarget.hasElements())
            {
                if(getTransform())
                {
                    
                    const drawinglayer::primitive2d::Primitive2DReference xRef(
                        new drawinglayer::primitive2d::TransformPrimitive2D(
                            *getTransform(),
                            aNewTarget));

                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xRef);
                }
                else
                {
                    
                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aNewTarget);
                }
            }
        }

        void SvgClipPathNode::apply(drawinglayer::primitive2d::Primitive2DSequence& rContent) const
        {
            if(rContent.hasElements() && Display_none != getDisplay())
            {
                const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                drawinglayer::primitive2d::Primitive2DSequence aClipTarget;
                basegfx::B2DPolyPolygon aClipPolyPolygon;

                
                decomposeSvgNode(aClipTarget, true);

                if(aClipTarget.hasElements())
                {
                    
                    drawinglayer::processor2d::ContourExtractor2D aExtractor(aViewInformation2D, true);

                    aExtractor.process(aClipTarget);

                    const basegfx::B2DPolyPolygonVector& rResult(aExtractor.getExtractedContour());
                    const sal_uInt32 nSize(rResult.size());

                    if(nSize > 1)
                    {
                        
                        aClipPolyPolygon = basegfx::tools::mergeToSinglePolyPolygon(rResult);
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
                        
                        const basegfx::B2DRange aContentRange(
                            drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                rContent,
                                aViewInformation2D));

                        aClipPolyPolygon.transform(
                            basegfx::tools::createScaleTranslateB2DHomMatrix(
                                aContentRange.getRange(),
                                aContentRange.getMinimum()));
                    }

                    
                    
                    
                    
                    const drawinglayer::primitive2d::Primitive2DReference xEmbedTransparence(
                        new drawinglayer::primitive2d::MaskPrimitive2D(
                            aClipPolyPolygon,
                            rContent));

                    rContent = drawinglayer::primitive2d::Primitive2DSequence(&xEmbedTransparence, 1);
                }
                else
                {
                    
                    // the ‘clip-path’ property applied. (Svg spec)
                    rContent.realloc(0);
                }
            }
        }

    } 
} 




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
