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

#include <svgio/svgreader/svgsvgnode.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

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
                // initial fill is black
                maSvgStyleAttributes.setFill(SvgPaint(basegfx::BColor(0.0, 0.0, 0.0), true, true));
            }
        }

        SvgSvgNode::~SvgSvgNode()
        {
            if(mpViewBox) delete mpViewBox;
        }

        const SvgStyleAttributes* SvgSvgNode::getSvgStyleAttributes() const
        {
            return &maSvgStyleAttributes;
        }

        void SvgSvgNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
            // enclosing svg might have relative width, need to cumulate them till they are
            // resolved somewhere up in the node tree
            double fPercentage(1.0);
            for(const SvgNode* pParent = getParent(); pParent && !bHasFound; pParent = pParent->getParent())
            {
                // dynamic_cast results Null-pointer for not SvgSvgNode and so skips them in if condition
                pParentSvgSvgNode = dynamic_cast< const SvgSvgNode* >(pParent);
                if (pParentSvgSvgNode)
                {
                    if (pParentSvgSvgNode->getViewBox())
                    {
                        // viewbox values are already in 'user unit'.
                        fWidth = pParentSvgSvgNode->getViewBox()->getWidth() * fPercentage;
                        bHasFound = true;
                    }
                    else
                    {
                        // take absolute value or cummulate percentage
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
                        } // not set => width=100% => factor 1, no need for else
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
            // enclosing svg might have relative width and height, need to cumulate them till they are
            // resolved somewhere up in the node tree
            double fPercentage(1.0);
            for(const SvgNode* pParent = getParent(); pParent && !bHasFound; pParent = pParent->getParent())
            {
                // dynamic_cast results Null-pointer for not SvgSvgNode and so skips them in if condition
                pParentSvgSvgNode = dynamic_cast< const SvgSvgNode* >(pParent);
                if (pParentSvgSvgNode)
                {
                    if (pParentSvgSvgNode->getViewBox())
                    {
                        // viewbox values are already in 'user unit'.
                        fHeight = pParentSvgSvgNode->getViewBox()->getHeight() * fPercentage;
                        bHasFound = true;
                    }
                    else
                    {
                        // take absolute value or cummulate percentage
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
                        } // not set => height=100% => factor 1, no need for else
                    }
                }
            }
        }

// ToDo: Consider attribute overflow in method decomposeSvgNode
        void SvgSvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DSequence aSequence;

            // decompose childs
            SvgNode::decomposeSvgNode(aSequence, bReferenced);

            if(aSequence.hasElements())
            {
                if(getParent())
                {
                    // #i122594# if width/height is not given, it's 100% (see 5.1.2 The 'svg' element in SVG1.1 spec).
                    // If it is relative, the question is to what. The previous implementatin assumed relative to the
                    // local ViewBox which is implied by (4.2 Basic data types):
                    //
                    // "Note that the non-property <length> definition also allows a percentage unit identifier.
                    // The meaning of a percentage length value depends on the attribute for which the percentage
                    // length value has been specified. Two common cases are: (a) when a percentage length value
                    // represents a percentage of the viewport width or height (refer to the section that discusses
                    // units in general), and (b) when a percentage length value represents a percentage of the
                    // bounding box width or height on a given object (refer to the section that describes object
                    // bounding box units)."

                    // Comparisons with commom browsers show, that it's mostly interpreted relative to the viewport
                    // of the parent, and so does the new implementation.

                    // Extract known viewport data
                    // bXXXIsAbsolute tracks whether relative values could be resolved to absolute values

                    // If width or height is not provided, the default 100% is used, see SVG 1.1 section 5.1.2
                    // value 0.0 here is only to initialize variable
                    bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);

                    bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);

                    // If x or y not provided, then default 0.0 is used, see SVG 1.1 Section 5.1.2
                    bool bXIsAbsolute((getX().isSet() && Unit_percent != getX().getUnit()) || !getX().isSet());
                    double fX( bXIsAbsolute && getX().isSet() ? getX().solveNonPercentage(*this) : 0.0);

                    bool bYIsAbsolute((getY().isSet() && Unit_percent != getY().getUnit()) || !getY().isSet());
                    double fY( bYIsAbsolute && getY().isSet() ? getY().solveNonPercentage(*this) : 0.0);

                    if ( !bXIsAbsolute || !bWidthIsAbsolute)
                    {
                        // get width of enclosing svg and resolve percentage in x and width;
                        double fWReference(0.0);
                        bool bHasFoundWidth(false);
                        seekReferenceWidth(fWReference, bHasFoundWidth);
                        if (!bHasFoundWidth)
                        {
                            // Even outermost svg has not all information to resolve relative values,
                            // I use content itself as fallback to set missing values for viewport
                            // Any better idea for such ill structures svg documents?
                            const basegfx::B2DRange aChildRange(
                                        drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                            aSequence,
                                        drawinglayer::geometry::ViewInformation2D()));
                            fWReference = aChildRange.getWidth();
                        }
                        // referenced values are already in 'user unit'
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
                        // get height of enclosing svg and resolve percentage in y and height
                        double fHReference(0.0);
                        bool bHasFoundHeight(false);
                        seekReferenceHeight(fHReference, bHasFoundHeight);
                        if (!bHasFoundHeight)
                        {
                            // Even outermost svg has not all information to resolve relative values,
                            // I use content itself as fallback to set missing values for viewport
                            // Any better idea for such ill structures svg documents?
                            const basegfx::B2DRange aChildRange(
                                        drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                            aSequence,
                                        drawinglayer::geometry::ViewInformation2D()));
                            fHReference = aChildRange.getHeight();
                        }

                        // referenced values are already in 'user unit'
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
                        // SVG 1.1 defines in section 7.7 that a negative value for width or height
                        // in viewBox is an error and that 0.0 disables rendering
                        if(basegfx::fTools::more(getViewBox()->getWidth(),0.0) && basegfx::fTools::more(getViewBox()->getHeight(),0.0))
                        {
                            // create target range homing x,y, width and height as calculated above
                            const basegfx::B2DRange aTarget(fX, fY, fX + fW, fY + fH);

                            if(aTarget.equal(*getViewBox()))
                            {
                                // no mapping needed, append
                                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aSequence);
                            }
                            else
                            {
                                // create mapping
                                // #i122610 SVG 1.1 defines in section 5.1.2 that if the attribute perserveAspectRatio is not specified,
                                // then the effect is as if a value of 'xMidYMid meet' were specified.
                                SvgAspectRatio aRatioDefault(Align_xMidYMid,false,true);
                                const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                                // let mapping be created from SvgAspectRatio
                                const basegfx::B2DHomMatrix aEmbeddingTransform(
                                    rRatio.createMapping(aTarget, *getViewBox()));

                                // prepare embedding in transformation
                                const drawinglayer::primitive2d::Primitive2DReference xRef(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aEmbeddingTransform,
                                        aSequence));

                                if(rRatio.isMeetOrSlice())
                                {
                                    // embed in transformation
                                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xRef);
                                }
                                else
                                {
                                    // need to embed in MaskPrimitive2D, too
                                    const drawinglayer::primitive2d::Primitive2DReference xMask(
                                        new drawinglayer::primitive2d::MaskPrimitive2D(
                                            basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aTarget)),
                                            drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1)));

                                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xMask);
                                }
                            }
                        }
                    }
                    else // no viewBox attribute
                    {
                        // Svg defines that a negative value is an error and that 0.0 disables rendering
                        if(basegfx::fTools::more(fW, 0.0) && basegfx::fTools::more(fH, 0.0))
                        {
                            if(!basegfx::fTools::equalZero(fX) || !basegfx::fTools::equalZero(fY))
                            {
                                // embed in transform
                                const drawinglayer::primitive2d::Primitive2DReference xRef(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        basegfx::tools::createTranslateB2DHomMatrix(fX, fY),
                                        aSequence));

                                aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
                            }

                            // embed in MaskPrimitive2D to clip
                            const drawinglayer::primitive2d::Primitive2DReference xMask(
                                new drawinglayer::primitive2d::MaskPrimitive2D(
                                    basegfx::B2DPolyPolygon(
                                        basegfx::tools::createPolygonFromRect(
                                            basegfx::B2DRange(fX, fY, fX + fW, fY + fH))),
                                    aSequence));

                            // append
                            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xMask);
                        }
                    }
                }
                else // Outermost SVG element
                {
                    double fW = 0.0; // effective value depends on viewBox
                    double fH = 0.0;

                    // Svg defines that a negative value is an error and that 0.0 disables rendering
                    // isPositive() not usable because it allows 0.0 in contrast to mathematical definition of 'positive'
                    const bool bWidthInvalid(getWidth().isSet() && basegfx::fTools::lessOrEqual(getWidth().getNumber(), 0.0));
                    const bool bHeightInvalid(getHeight().isSet() && basegfx::fTools::lessOrEqual(getHeight().getNumber(), 0.0));
                    if(!bWidthInvalid && !bHeightInvalid)
                    {
                        basegfx::B2DRange aSvgCanvasRange; // effective value depends on viewBox
                        if(getViewBox())
                        {
                            // SVG 1.1 defines in section 7.7 that a negative value for width or height
                            // in viewBox is an error and that 0.0 disables rendering
                            const double fViewBoxWidth = getViewBox()->getWidth();
                            const double fViewBoxHeight = getViewBox()->getHeight();
                            if(basegfx::fTools::more(fViewBoxWidth,0.0) && basegfx::fTools::more(fViewBoxHeight,0.0))
                            {
                                // The intrinsic aspect ratio of the svg element is given by absolute values of both width and height
                                // or if one or both of them is relative by the width and height of the viewBox
                                // see SVG 1.1 section 7.12
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
                                // SVG 1.1 defines in section 5.1.2 that x,y has no meanig for the outermost SVG element.
                                aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);

                                // create mapping
                                // SVG 1.1 defines in section 5.1.2 that if the attribute perserveAspectRatio is not specified,
                                // then the effect is as if a value of 'xMidYMid meet' were specified.
                                SvgAspectRatio aRatioDefault(Align_xMidYMid,false,true);
                                const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                                basegfx::B2DHomMatrix aViewBoxMapping;
                                aViewBoxMapping = rRatio.createMapping(aSvgCanvasRange, *getViewBox());
                                // no need to check ratio here for slice, the outermost Svg will
                                // be clipped anyways (see below)

                                // scale content to viewBox definitions
                                const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aViewBoxMapping,
                                        aSequence));

                                aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xTransform, 1);
                            }
                        }
                        else // no viewbox
                        {
                           // There exists no parent to resolve relative width or height.
                           // Use child size as fallback.
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
                            // SVG 1.1 defines in section 5.1.2 that x,y has no meanig for the outermost SVG element.
                            aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);
                        }

                        // to be completely correct in Svg sense it is necessary to clip
                        // the whole content to the given canvas. I choose here to do this
                        // initially despite I found various examples of Svg files out there
                        // which have no correct values for this clipping. It's correct
                        // due to the Svg spec.
                        bool bDoCorrectCanvasClipping(true);

                        if(bDoCorrectCanvasClipping)
                        {
                            // different from Svg we have the possibility with primitives to get
                            // a correct bounding box for the geometry. Get it for evtl. taking action
                            const basegfx::B2DRange aContentRange(
                                drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                    aSequence,
                                    drawinglayer::geometry::ViewInformation2D()));

                            if(aSvgCanvasRange.isInside(aContentRange))
                            {
                                // no clip needed, but an invisible HiddenGeometryPrimitive2D
                                // to allow getting the full Svg range using the primitive mechanisms.
                                // This is needed since e.g. an SdrObject using this as graphic will
                                // create a mapping transformation to exactly map the content to it's
                                // real life size
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
                                // Clip is necessary. This will make Svg images evtl. smaller
                                // than wanted from Svg (the free space which may be around it is
                                // conform to the Svg spec), but avoids an expensive and unneccessary
                                // clip. Keep the full Svg range here to get the correct mappings
                                // to objects using this. Optimizations can be done in the processors
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
                                // not inside, no overlap. Empty Svg
                                aSequence.realloc(0);
                            }
                        }

                        if(aSequence.hasElements())
                        {
                            // embed in transform primitive to scale to 1/100th mm
                            // where 1 inch == 25.4 mm to get from Svg coordinates (px) to
                            // drawinglayer coordinates
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

                            // append to result
                            drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aSequence);
                        }
                    }
                }
            }
        }

        const basegfx::B2DRange* SvgSvgNode::getCurrentViewPort() const
        {
            if(getViewBox())
            {
                return getViewBox();
            }
            else // viewport should be given by x, y, width, and height
            {
                // Extract known viewport data
                // bXXXIsAbsolute tracks whether relative values could be resolved to absolute values
                if (getParent())
                    {
                    // If width or height is not provided, the default 100% is used, see SVG 1.1 section 5.1.2
                    // value 0.0 here is only to initialize variable
                    bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);
                    bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);

                    // If x or y not provided, then default 0.0 is used, see SVG 1.1 Section 5.1.2
                    bool bXIsAbsolute((getX().isSet() && Unit_percent != getX().getUnit()) || !getX().isSet());
                    double fX( bXIsAbsolute && getX().isSet() ? getX().solveNonPercentage(*this) : 0.0);

                    bool bYIsAbsolute((getY().isSet() && Unit_percent != getY().getUnit()) || !getY().isSet());
                    double fY( bYIsAbsolute && getY().isSet() ? getY().solveNonPercentage(*this) : 0.0);

                    if (bXIsAbsolute && bYIsAbsolute && bWidthIsAbsolute && bHeightIsAbsolute)
                    {
                        return &basegfx::B2DRange(fX, fY, fX+fW, fY+fH);
                    }
                    else // try to resolve relative values
                    {
                        if (!bXIsAbsolute || !bWidthIsAbsolute)
                        {
                            // get width of enclosing svg and resolve percentage in x and width
                            double fWReference(0.0);
                            bool bHasFoundWidth(false);
                            seekReferenceWidth(fWReference, bHasFoundWidth);
                            // referenced values are already in 'user unit'
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
                            // get height of enclosing svg and resolve percentage in y and height
                            double fHReference(0.0);
                            bool bHasFoundHeight(false);
                            seekReferenceHeight(fHReference, bHasFoundHeight);
                            // referenced values are already in 'user unit'
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
                            return &basegfx::B2DRange(fX, fY, fX+fW, fY+fH);
                        }
                        else // relative values could not be resolved, there exists no fallback
                        {
                            return SvgNode::getCurrentViewPort();
                        }
                    }
                }
                else //outermost svg
                {
                    // If width or height is not provided, the default would be 100%, see SVG 1.1 section 5.1.2
                    // But here it cannot be resolved and no fallback exists.
                    // SVG 1.1 defines in section 5.1.2 that x,y has no meanig for the outermost SVG element.
                    bool bWidthIsAbsolute(getWidth().isSet() && Unit_percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);
                    bool bHeightIsAbsolute(getHeight().isSet() && Unit_percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);
                    if (bWidthIsAbsolute && bHeightIsAbsolute)
                    {
                        return &basegfx::B2DRange(0.0, 0.0, fW, fH);
                    }
                    else // no fallback exists
                    {
                            return SvgNode::getCurrentViewPort();
                    }
                }
// ToDo: Is it possible to decompose and use the bounding box of the childs, if even the
//       outermost svg has no information to resolve percentage? Is it worth, how expensive is it?

            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
