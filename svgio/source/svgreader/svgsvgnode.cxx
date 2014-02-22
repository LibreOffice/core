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

#include <svgio/svgreader/svgsvgnode.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>



namespace svgio
{
    namespace svgreader
    {
        SvgSvgNode::SvgSvgNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenSvg, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpViewBox(0),
            maSvgAspectRatio(),
            maX(),
            maY(),
            maWidth(),
            maHeight(),
            maVersion()
        {
            if(!getParent())
            {
                
                maSvgStyleAttributes.setFill(SvgPaint(basegfx::BColor(0.0, 0.0, 0.0), true, true));
            }
        }

        SvgSvgNode::~SvgSvgNode()
        {
            if(mpViewBox) delete mpViewBox;
        }

        const SvgStyleAttributes* SvgSvgNode::getSvgStyleAttributes() const
        {
            return checkForCssStyle(OUString("svg"), maSvgStyleAttributes);
        }

        void SvgSvgNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGTokenViewBox:
                {
                    const basegfx::B2DRange aRange(readViewBox(aContent, *this));

                    if(!aRange.isEmpty())
                    {
                        setViewBox(&aRange);
                    }
                    break;
                }
                case SVGTokenPreserveAspectRatio:
                {
                    setSvgAspectRatio(readSvgAspectRatio(aContent));
                    break;
                }
                case SVGTokenX:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setX(aNum);
                    }
                    break;
                }
                case SVGTokenY:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setY(aNum);
                    }
                    break;
                }
                case SVGTokenWidth:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setWidth(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenHeight:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        if(aNum.isPositive())
                        {
                            setHeight(aNum);
                        }
                    }
                    break;
                }
                case SVGTokenVersion:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        setVersion(aNum);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgSvgNode::seekReferenceWidth(double& fWidth, bool& bHasFound) const
        {
            if (!getParent() || bHasFound)
            {
                return;
            }
            const SvgSvgNode* pParentSvgSvgNode = 0;
            
            
            double fPercentage(1.0);
            for(const SvgNode* pParent = getParent(); pParent && !bHasFound; pParent = pParent->getParent())
            {
                
                pParentSvgSvgNode = dynamic_cast< const SvgSvgNode* >(pParent);
                if (pParentSvgSvgNode)
                {
                    if (pParentSvgSvgNode->getViewBox())
                    {
                        
                        fWidth = pParentSvgSvgNode->getViewBox()->getWidth() * fPercentage;
                        bHasFound = true;
                    }
                    else
                    {
                        
                        if (pParentSvgSvgNode->getWidth().isSet())
                        {
                            if (Unit_percent == pParentSvgSvgNode->getWidth().getUnit())
                            {
                                fPercentage *= pParentSvgSvgNode->getWidth().getNumber() * 0.01;
                            }
                            else
                            {
                                fWidth = pParentSvgSvgNode->getWidth().solveNonPercentage(*pParentSvgSvgNode) * fPercentage;
                                bHasFound = true;
                            }
                        } 
                    }
                }
            }
        }

        void SvgSvgNode::seekReferenceHeight(double& fHeight, bool& bHasFound) const
        {
            if (!getParent() || bHasFound)
            {
                return;
            }
            const SvgSvgNode* pParentSvgSvgNode = 0;
            
            
            double fPercentage(1.0);
            for(const SvgNode* pParent = getParent(); pParent && !bHasFound; pParent = pParent->getParent())
            {
                
                pParentSvgSvgNode = dynamic_cast< const SvgSvgNode* >(pParent);
                if (pParentSvgSvgNode)
                {
                    if (pParentSvgSvgNode->getViewBox())
                    {
                        
                        fHeight = pParentSvgSvgNode->getViewBox()->getHeight() * fPercentage;
                        bHasFound = true;
                    }
                    else
                    {
                        
                        if (pParentSvgSvgNode->getHeight().isSet())
                        {
                            if (Unit_percent == pParentSvgSvgNode->getHeight().getUnit())
                            {
                                fPercentage *= pParentSvgSvgNode->getHeight().getNumber() * 0.01;
                            }
                            else
                            {
                                fHeight = pParentSvgSvgNode->getHeight().solveNonPercentage(*pParentSvgSvgNode) * fPercentage;
                                bHasFound = true;
                            }
                        } 
                    }
                }
            }
        }


        void SvgSvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DSequence aSequence;

            
            SvgNode::decomposeSvgNode(aSequence, bReferenced);

            if(aSequence.hasElements())
            {
                if(getParent())
                {
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    

                    
                    

                    
                    

                    
                    
                    bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);

                    bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);

                    
                    bool bXIsAbsolute((getX().isSet() && Unit_percent != getX().getUnit()) || !getX().isSet());
                    double fX( bXIsAbsolute && getX().isSet() ? getX().solveNonPercentage(*this) : 0.0);

                    bool bYIsAbsolute((getY().isSet() && Unit_percent != getY().getUnit()) || !getY().isSet());
                    double fY( bYIsAbsolute && getY().isSet() ? getY().solveNonPercentage(*this) : 0.0);

                    if ( !bXIsAbsolute || !bWidthIsAbsolute)
                    {
                        
                        double fWReference(0.0);
                        bool bHasFoundWidth(false);
                        seekReferenceWidth(fWReference, bHasFoundWidth);
                        if (!bHasFoundWidth)
                        {
                            
                            
                            
                            const basegfx::B2DRange aChildRange(
                                        drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                            aSequence,
                                        drawinglayer::geometry::ViewInformation2D()));
                            fWReference = aChildRange.getWidth();
                        }
                        
                        if (!bXIsAbsolute)
                        {
                            fX = getX().getNumber() * 0.01 * fWReference;
                        }
                        if (!bWidthIsAbsolute)
                        {
                            fW = (getWidth().isSet() ? getWidth().getNumber() *0.01 : 1.0) * fWReference;
                        }
                    }

                    if ( !bYIsAbsolute || !bHeightIsAbsolute)
                    {
                        
                        double fHReference(0.0);
                        bool bHasFoundHeight(false);
                        seekReferenceHeight(fHReference, bHasFoundHeight);
                        if (!bHasFoundHeight)
                        {
                            
                            
                            
                            const basegfx::B2DRange aChildRange(
                                        drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                            aSequence,
                                        drawinglayer::geometry::ViewInformation2D()));
                            fHReference = aChildRange.getHeight();
                        }

                        
                        if (!bYIsAbsolute)
                        {
                            fY = getY().getNumber() * 0.01 * fHReference;
                        }
                        if (!bHeightIsAbsolute)
                        {
                            fH = (getHeight().isSet() ? getHeight().getNumber() *0.01 : 1.0) * fHReference;
                        }
                    }

                    if(getViewBox())
                    {
                        
                        
                        if(basegfx::fTools::more(getViewBox()->getWidth(),0.0) && basegfx::fTools::more(getViewBox()->getHeight(),0.0))
                        {
                            
                            const basegfx::B2DRange aTarget(fX, fY, fX + fW, fY + fH);

                            if(aTarget.equal(*getViewBox()))
                            {
                                
                                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aSequence);
                            }
                            else
                            {
                                
                                
                                
                                SvgAspectRatio aRatioDefault(Align_xMidYMid,false,true);
                                const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                                
                                const basegfx::B2DHomMatrix aEmbeddingTransform(
                                    rRatio.createMapping(aTarget, *getViewBox()));

                                
                                const drawinglayer::primitive2d::Primitive2DReference xRef(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aEmbeddingTransform,
                                        aSequence));

                                if(rRatio.isMeetOrSlice())
                                {
                                    
                                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xRef);
                                }
                                else
                                {
                                    
                                    const drawinglayer::primitive2d::Primitive2DReference xMask(
                                        new drawinglayer::primitive2d::MaskPrimitive2D(
                                            basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aTarget)),
                                            drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1)));

                                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xMask);
                                }
                            }
                        }
                    }
                    else 
                    {
                        
                        if(basegfx::fTools::more(fW, 0.0) && basegfx::fTools::more(fH, 0.0))
                        {
                            if(!basegfx::fTools::equalZero(fX) || !basegfx::fTools::equalZero(fY))
                            {
                                
                                const drawinglayer::primitive2d::Primitive2DReference xRef(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        basegfx::tools::createTranslateB2DHomMatrix(fX, fY),
                                        aSequence));

                                aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                            }

                            
                            const drawinglayer::primitive2d::Primitive2DReference xMask(
                                new drawinglayer::primitive2d::MaskPrimitive2D(
                                    basegfx::B2DPolyPolygon(
                                        basegfx::tools::createPolygonFromRect(
                                            basegfx::B2DRange(fX, fY, fX + fW, fY + fH))),
                                    aSequence));

                            
                            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xMask);
                        }
                    }
                }
                else 
                {
                    double fW = 0.0; 
                    double fH = 0.0;

                    
                    
                    const bool bWidthInvalid(getWidth().isSet() && basegfx::fTools::lessOrEqual(getWidth().getNumber(), 0.0));
                    const bool bHeightInvalid(getHeight().isSet() && basegfx::fTools::lessOrEqual(getHeight().getNumber(), 0.0));
                    if(!bWidthInvalid && !bHeightInvalid)
                    {
                        basegfx::B2DRange aSvgCanvasRange; 
                        if(getViewBox())
                        {
                            
                            
                            const double fViewBoxWidth = getViewBox()->getWidth();
                            const double fViewBoxHeight = getViewBox()->getHeight();
                            if(basegfx::fTools::more(fViewBoxWidth,0.0) && basegfx::fTools::more(fViewBoxHeight,0.0))
                            {
                                
                                
                                
                                const bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                                const bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                                if(bWidthIsAbsolute && bHeightIsAbsolute)
                                {
                                    fW =getWidth().solveNonPercentage(*this);
                                    fH =getHeight().solveNonPercentage(*this);
                                }
                                else if (bWidthIsAbsolute)
                                {
                                    fW = getWidth().solveNonPercentage(*this);
                                    fH = fW * fViewBoxWidth / fViewBoxHeight ;
                                }
                                else if (bHeightIsAbsolute)
                                {
                                    fH = getHeight().solveNonPercentage(*this);
                                    fW = fH * fViewBoxWidth / fViewBoxHeight ;
                                }
                                else
                                {
                                    fW = fViewBoxWidth;
                                    fH = fViewBoxHeight;
                                }
                                
                                aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);

                                
                                
                                
                                SvgAspectRatio aRatioDefault(Align_xMidYMid,false,true);
                                const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                                basegfx::B2DHomMatrix aViewBoxMapping;
                                aViewBoxMapping = rRatio.createMapping(aSvgCanvasRange, *getViewBox());
                                
                                

                                
                                const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aViewBoxMapping,
                                        aSequence));

                                aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xTransform, 1);
                            }
                        }
                        else 
                        {
                           
                           
                            const bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                            const bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                            if (bWidthIsAbsolute && bHeightIsAbsolute)
                            {
                                fW =getWidth().solveNonPercentage(*this);
                                fH =getHeight().solveNonPercentage(*this);

                            }
                            else
                            {
                                const basegfx::B2DRange aChildRange(
                                    drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                        aSequence,
                                     drawinglayer::geometry::ViewInformation2D()));
                                const double fChildWidth(aChildRange.getWidth());
                                const double fChildHeight(aChildRange.getHeight());
                                fW = bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : fChildWidth;
                                fH = bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : fChildHeight;
                            }
                            
                            aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);
                        }

                        
                        
                        
                        
                        
                        bool bDoCorrectCanvasClipping(true);

                        if(bDoCorrectCanvasClipping)
                        {
                            
                            
                            const basegfx::B2DRange aContentRange(
                                drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                    aSequence,
                                    drawinglayer::geometry::ViewInformation2D()));

                            if(aSvgCanvasRange.isInside(aContentRange))
                            {
                                
                                
                                
                                
                                
                                const drawinglayer::primitive2d::Primitive2DReference xLine(
                                    new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                                        basegfx::tools::createPolygonFromRect(
                                            aSvgCanvasRange),
                                        basegfx::BColor(0.0, 0.0, 0.0)));
                                const drawinglayer::primitive2d::Primitive2DReference xHidden(
                                    new drawinglayer::primitive2d::HiddenGeometryPrimitive2D(
                                        drawinglayer::primitive2d::Primitive2DSequence(&xLine, 1)));

                                drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aSequence, xHidden);
                            }
                            else if(aSvgCanvasRange.overlaps(aContentRange))
                            {
                                
                                
                                
                                
                                
                                const drawinglayer::primitive2d::Primitive2DReference xMask(
                                    new drawinglayer::primitive2d::MaskPrimitive2D(
                                        basegfx::B2DPolyPolygon(
                                            basegfx::tools::createPolygonFromRect(
                                                aSvgCanvasRange)),
                                        aSequence));

                                aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xMask, 1);
                            }
                            else
                            {
                                
                                aSequence.realloc(0);
                            }
                        }

                        if(aSequence.hasElements())
                        {
                            
                            
                            
                            const double fScaleTo100thmm(25.4 * 100.0 / F_SVG_PIXEL_PER_INCH);
                            const basegfx::B2DHomMatrix aTransform(
                                basegfx::tools::createScaleB2DHomMatrix(
                                    fScaleTo100thmm,
                                    fScaleTo100thmm));

                            const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                new drawinglayer::primitive2d::TransformPrimitive2D(
                                    aTransform,
                                    aSequence));

                            aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xTransform, 1);

                            
                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aSequence);
                        }
                    }
                }
            }
        }

        const basegfx::B2DRange SvgSvgNode::getCurrentViewPort() const
        {
            if(getViewBox())
            {
                return *(getViewBox());
            }
            else 
            {
                
                
                if (getParent())
                    {
                    
                    
                    bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);
                    bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);

                    
                    bool bXIsAbsolute((getX().isSet() && Unit_percent != getX().getUnit()) || !getX().isSet());
                    double fX( bXIsAbsolute && getX().isSet() ? getX().solveNonPercentage(*this) : 0.0);

                    bool bYIsAbsolute((getY().isSet() && Unit_percent != getY().getUnit()) || !getY().isSet());
                    double fY( bYIsAbsolute && getY().isSet() ? getY().solveNonPercentage(*this) : 0.0);

                    if (bXIsAbsolute && bYIsAbsolute && bWidthIsAbsolute && bHeightIsAbsolute)
                    {
                        return basegfx::B2DRange(fX, fY, fX+fW, fY+fH);
                    }
                    else 
                    {
                        if (!bXIsAbsolute || !bWidthIsAbsolute)
                        {
                            
                            double fWReference(0.0);
                            bool bHasFoundWidth(false);
                            seekReferenceWidth(fWReference, bHasFoundWidth);
                            
                            if (!bXIsAbsolute && bHasFoundWidth)
                            {
                                fX = getX().getNumber() * 0.01 * fWReference;
                                bXIsAbsolute = true;
                            }
                            if (!bWidthIsAbsolute && bHasFoundWidth)
                            {
                                fW = (getWidth().isSet() ? getWidth().getNumber() *0.01 : 1.0) * fWReference;
                                bWidthIsAbsolute = true;
                            }
                        }
                        if (!bYIsAbsolute || !bHeightIsAbsolute)
                        {
                            
                            double fHReference(0.0);
                            bool bHasFoundHeight(false);
                            seekReferenceHeight(fHReference, bHasFoundHeight);
                            
                            if (!bYIsAbsolute && bHasFoundHeight)
                            {
                                fY = getY().getNumber() * 0.01 * fHReference;
                                bYIsAbsolute = true;
                            }
                            if (!bHeightIsAbsolute && bHasFoundHeight)
                            {
                                fH = (getHeight().isSet() ? getHeight().getNumber() *0.01 : 1.0) * fHReference;
                                bHeightIsAbsolute = true;
                            }
                        }

                        if (bXIsAbsolute && bYIsAbsolute && bWidthIsAbsolute && bHeightIsAbsolute)
                        {
                            return basegfx::B2DRange(fX, fY, fX+fW, fY+fH);
                        }
                        else 
                        {
                            return SvgNode::getCurrentViewPort();
                        }
                    }
                }
                else 
                {
                    
                    
                    
                    bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);
                    bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);
                    if (bWidthIsAbsolute && bHeightIsAbsolute)
                    {
                        return basegfx::B2DRange(0.0, 0.0, fW, fH);
                    }
                    else 
                    {
                            return SvgNode::getCurrentViewPort();
                    }
                }



            }
        }

    } 
} 




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
