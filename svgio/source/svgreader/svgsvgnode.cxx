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

#include <svgsvgnode.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svgdocument.hxx>

namespace svgio::svgreader
{
        SvgSvgNode::SvgSvgNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Svg, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mbStyleAttributesInitialized(false) // #i125258#
        {
        }

        // #i125258#
        void SvgSvgNode::initializeStyleAttributes()
        {
            if(mbStyleAttributesInitialized)
                return;

            // #i125258# determine if initial values need to be initialized with hard values
            // for the case that this is the outmost SVG statement and it has no parent
            // stale (CssStyle for svg may be defined)
            bool bSetInitialValues(true);

            if(getParent())
            {
                // #i125258# no initial values when it's a SVG element embedded in SVG
                bSetInitialValues = false;
            }

            if(bSetInitialValues)
            {
                const SvgStyleAttributes* pStyles = getSvgStyleAttributes();

                if(pStyles && pStyles->getParentStyle())
                {
                    // SVG has a parent style (probably CssStyle), check if fill is set there anywhere
                    // already. If yes, do not set the default fill (black)
                    bool bFillSet(false);
                    const SvgStyleAttributes* pParentStyle = pStyles->getParentStyle();

                    while(pParentStyle && !bFillSet)
                    {
                        bFillSet = pParentStyle->isFillSet();
                        pParentStyle = pParentStyle->getParentStyle();
                    }

                    if(bFillSet)
                    {
                        // #125258# no initial values when SVG has a parent style at which a fill
                        // is already set
                        bSetInitialValues = false;
                    }
                }
            }

            if(bSetInitialValues)
            {
                // #i125258# only set if not yet initialized (SvgSvgNode::parseAttribute is already done,
                // just setting may revert an already set valid value)
                if(!maSvgStyleAttributes.isFillSet())
                {
                    // #i125258# initial fill is black (see SVG1.1 spec)
                    maSvgStyleAttributes.setFill(SvgPaint(basegfx::BColor(0.0, 0.0, 0.0), true, true));
                }
            }

            mbStyleAttributesInitialized = true;
        }

        SvgSvgNode::~SvgSvgNode()
        {
        }

        const SvgStyleAttributes* SvgSvgNode::getSvgStyleAttributes() const
        {
            // #i125258# svg node can have CssStyles, too, so check for it here
            return checkForCssStyle("svg", maSvgStyleAttributes);
        }

        void SvgSvgNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
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
                case SVGToken::Version:
                {
                    SvgNumber aNum;

                    if(readSingleNumber(aContent, aNum))
                    {
                        maVersion = aNum;
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
            const SvgSvgNode* pParentSvgSvgNode = nullptr;
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
                        // take absolute value or cumulate percentage
                        if (pParentSvgSvgNode->getWidth().isSet())
                        {
                            if (SvgUnit::percent == pParentSvgSvgNode->getWidth().getUnit())
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
            const SvgSvgNode* pParentSvgSvgNode = nullptr;
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
                        // take absolute value or cumulate percentage
                        if (pParentSvgSvgNode->getHeight().isSet())
                        {
                            if (SvgUnit::percent == pParentSvgSvgNode->getHeight().getUnit())
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
        void SvgSvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DContainer aSequence;

            // #i125258# check now if we need to init some style settings locally. Do not do this
            // in the constructor, there is not yet information e.g. about existing CssStyles.
            // Here all nodes are read and interpreted
            const_cast< SvgSvgNode* >(this)->initializeStyleAttributes();

            // decompose children
            SvgNode::decomposeSvgNode(aSequence, bReferenced);

            if(!aSequence.empty())
            {
                if(getParent())
                {
                    // #i122594# if width/height is not given, it's 100% (see 5.1.2 The 'svg' element in SVG1.1 spec).
                    // If it is relative, the question is to what. The previous implementation assumed relative to the
                    // local ViewBox which is implied by (4.2 Basic data types):

                    // "Note that the non-property <length> definition also allows a percentage unit identifier.
                    // The meaning of a percentage length value depends on the attribute for which the percentage
                    // length value has been specified. Two common cases are: (a) when a percentage length value
                    // represents a percentage of the viewport width or height (refer to the section that discusses
                    // units in general), and (b) when a percentage length value represents a percentage of the
                    // bounding box width or height on a given object (refer to the section that describes object
                    // bounding box units)."

                    // Comparisons with common browsers show that it's mostly interpreted relative to the viewport
                    // of the parent, and so does the new implementation.

                    // Extract known viewport data
                    // bXXXIsAbsolute tracks whether relative values could be resolved to absolute values

                    // If width or height is not provided, the default 100% is used, see SVG 1.1 section 5.1.2
                    // value 0.0 here is only to initialize variable
                    bool bWidthIsAbsolute(getWidth().isSet() && SvgUnit::percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);

                    bool bHeightIsAbsolute(getHeight().isSet() && SvgUnit::percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);

                    // If x or y not provided, then default 0.0 is used, see SVG 1.1 Section 5.1.2
                    bool bXIsAbsolute((getX().isSet() && SvgUnit::percent != getX().getUnit()) || !getX().isSet());
                    double fX( bXIsAbsolute && getX().isSet() ? getX().solveNonPercentage(*this) : 0.0);

                    bool bYIsAbsolute((getY().isSet() && SvgUnit::percent != getY().getUnit()) || !getY().isSet());
                    double fY( bYIsAbsolute && getY().isSet() ? getY().solveNonPercentage(*this) : 0.0);

                    if ( !bXIsAbsolute || !bWidthIsAbsolute)
                    {
                        // get width of enclosing svg and resolve percentage in x and width;
                        double fWReference(0.0);
                        bool bHasFoundWidth(false);
                        seekReferenceWidth(fWReference, bHasFoundWidth);
                        if (!bHasFoundWidth)
                        {
                            if (getViewBox())
                            {
                                fWReference = getViewBox()->getWidth();
                            }
                            else
                            {
                                // Even outermost svg has not all information to resolve relative values,
                                // I use content itself as fallback to set missing values for viewport
                                // Any better idea for such ill structured svg documents?
                                const basegfx::B2DRange aChildRange(
                                            aSequence.getB2DRange(
                                                drawinglayer::geometry::ViewInformation2D()));
                                fWReference = aChildRange.getWidth();
                            }
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
                            if (getViewBox())
                            {
                                fHReference = getViewBox()->getHeight();
                            }
                            else
                            {
                            // Even outermost svg has not all information to resolve relative values,
                                // I use content itself as fallback to set missing values for viewport
                                // Any better idea for such ill structured svg documents?
                                const basegfx::B2DRange aChildRange(
                                        aSequence.getB2DRange(
                                            drawinglayer::geometry::ViewInformation2D()));
                                fHReference = aChildRange.getHeight();
                            }
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
                                rTarget.append(aSequence);
                            }
                            else
                            {
                                // create mapping
                                // #i122610 SVG 1.1 defines in section 5.1.2 that if the attribute preserveAspectRatio is not specified,
                                // then the effect is as if a value of 'xMidYMid meet' were specified.
                                SvgAspectRatio aRatioDefault(SvgAlign::xMidYMid,true);
                                const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                                // let mapping be created from SvgAspectRatio
                                const basegfx::B2DHomMatrix aEmbeddingTransform(
                                    rRatio.createMapping(aTarget, *getViewBox()));

                                // prepare embedding in transformation
                                const drawinglayer::primitive2d::Primitive2DReference xRef(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aEmbeddingTransform,
                                        drawinglayer::primitive2d::Primitive2DContainer(aSequence)));

                                if(rRatio.isMeetOrSlice())
                                {
                                    // embed in transformation
                                    rTarget.push_back(xRef);
                                }
                                else
                                {
                                    // need to embed in MaskPrimitive2D, too
                                    const drawinglayer::primitive2d::Primitive2DReference xMask(
                                        new drawinglayer::primitive2d::MaskPrimitive2D(
                                            basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aTarget)),
                                            drawinglayer::primitive2d::Primitive2DContainer { xRef }));

                                    rTarget.push_back(xMask);
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
                                        basegfx::utils::createTranslateB2DHomMatrix(fX, fY),
                                        std::move(aSequence)));

                                aSequence = drawinglayer::primitive2d::Primitive2DContainer { xRef, };
                            }

                            // embed in MaskPrimitive2D to clip
                            const drawinglayer::primitive2d::Primitive2DReference xMask(
                                new drawinglayer::primitive2d::MaskPrimitive2D(
                                    basegfx::B2DPolyPolygon(
                                        basegfx::utils::createPolygonFromRect(
                                            basegfx::B2DRange(fX, fY, fX + fW, fY + fH))),
                                    drawinglayer::primitive2d::Primitive2DContainer(aSequence)));

                            // append
                            rTarget.push_back(xMask);
                        }
                    }
                }
                else // Outermost SVG element
                {
                    // Svg defines that a negative value is an error and that 0.0 disables rendering
                    // isPositive() not usable because it allows 0.0 in contrast to mathematical definition of 'positive'
                    const bool bWidthInvalid(getWidth().isSet() && basegfx::fTools::lessOrEqual(getWidth().getNumber(), 0.0));
                    const bool bHeightInvalid(getHeight().isSet() && basegfx::fTools::lessOrEqual(getHeight().getNumber(), 0.0));
                    if(!bWidthInvalid && !bHeightInvalid)
                    {
                        basegfx::B2DRange aSvgCanvasRange; // viewport
                        double fW = 0.0; // dummy values
                        double fH = 0.0;
                        if (const basegfx::B2DRange* pBox = getViewBox())
                        {
                            // SVG 1.1 defines in section 7.7 that a negative value for width or height
                            // in viewBox is an error and that 0.0 disables rendering
                            const double fViewBoxWidth = pBox->getWidth();
                            const double fViewBoxHeight = pBox->getHeight();
                            if(basegfx::fTools::more(fViewBoxWidth,0.0) && basegfx::fTools::more(fViewBoxHeight,0.0))
                            {
                                // The intrinsic aspect ratio of the svg element is given by absolute values of svg width and svg height
                                // or by the width and height of the viewBox, if svg width or svg height is relative.
                                // see SVG 1.1 section 7.12
                                bool bNeedsMapping(true);
                                const bool bWidthIsAbsolute(getWidth().isSet() && SvgUnit::percent != getWidth().getUnit());
                                const bool bHeightIsAbsolute(getHeight().isSet() && SvgUnit::percent != getHeight().getUnit());
                                const double fViewBoxRatio(fViewBoxWidth/fViewBoxHeight);
                                if(bWidthIsAbsolute && bHeightIsAbsolute)
                                {
                                    fW = getWidth().solveNonPercentage(*this);
                                    fH = getHeight().solveNonPercentage(*this);
                                    aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);
                                }
                                else if (bWidthIsAbsolute)
                                {
                                    fW = getWidth().solveNonPercentage(*this);
                                    fH = fW / fViewBoxRatio ;
                                    aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);
                                }
                                else if (bHeightIsAbsolute)
                                {
                                    fH = getHeight().solveNonPercentage(*this);
                                    fW = fH * fViewBoxRatio ;
                                    aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);
                                }
                                else
                                {
                                    // There exists no parent to resolve relative width or height.
                                    // Use child size as fallback and expand to aspect ratio given
                                    // by the viewBox. No mapping.
                                    // We get viewport >= content, therefore no clipping.
                                    bNeedsMapping = false;

                                    const double fChildWidth(pBox->getWidth());
                                    const double fChildHeight(pBox->getHeight());
                                    const double fLeft(pBox->getMinX());
                                    const double fTop(pBox->getMinY());
                                    if ( fChildWidth / fViewBoxWidth > fChildHeight / fViewBoxHeight )
                                    {  // expand y
                                        fW = fChildWidth;
                                        fH = fChildWidth / fViewBoxRatio;
                                    }
                                    else
                                    {  // expand x
                                        fH = fChildHeight;
                                        fW = fChildHeight * fViewBoxRatio;
                                    }
                                    aSvgCanvasRange = basegfx::B2DRange(fLeft, fTop, fLeft + fW, fTop + fH);
                                }

                                if (bNeedsMapping)
                                {
                                    // create mapping
                                    // SVG 1.1 defines in section 5.1.2 that if the attribute preserveAspectRatio is not specified,
                                    // then the effect is as if a value of 'xMidYMid meet' were specified.
                                    SvgAspectRatio aRatioDefault(SvgAlign::xMidYMid, true);
                                    const SvgAspectRatio& rRatio = getSvgAspectRatio().isSet()? getSvgAspectRatio() : aRatioDefault;

                                    basegfx::B2DHomMatrix aViewBoxMapping = rRatio.createMapping(aSvgCanvasRange, *pBox);
                                    // no need to check ratio here for slice, the outermost Svg will
                                    // be clipped anyways (see below)

                                    // scale content to viewBox definitions
                                    const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                        new drawinglayer::primitive2d::TransformPrimitive2D(
                                            aViewBoxMapping,
                                            std::move(aSequence)));

                                    aSequence = drawinglayer::primitive2d::Primitive2DContainer { xTransform };
                                }
                            }
                        }
                        else // no viewbox => no mapping
                        {
                            const bool bWidthIsAbsolute(getWidth().isSet() && SvgUnit::percent != getWidth().getUnit());
                            const bool bHeightIsAbsolute(getHeight().isSet() && SvgUnit::percent != getHeight().getUnit());
                            if (bWidthIsAbsolute && bHeightIsAbsolute)
                            {
                                fW =getWidth().solveNonPercentage(*this);
                                fH =getHeight().solveNonPercentage(*this);
                                aSvgCanvasRange = basegfx::B2DRange(0.0, 0.0, fW, fH);
                            }
                            else
                            {
                                // There exists no parent to resolve relative width or height.
                                // Use child size as fallback. We get viewport >= content, therefore no clipping.
                                const basegfx::B2DRange aChildRange(
                                     aSequence.getB2DRange(
                                         drawinglayer::geometry::ViewInformation2D()));
                                const double fChildWidth(aChildRange.getWidth());
                                const double fChildHeight(aChildRange.getHeight());
                                const double fChildLeft(aChildRange.getMinX());
                                const double fChildTop(aChildRange.getMinY());
                                fW = bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : fChildWidth;
                                fH = bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : fChildHeight;
                                const double fLeft(bWidthIsAbsolute ? 0.0 : fChildLeft);
                                const double fTop(bHeightIsAbsolute ? 0.0 : fChildTop);
                                aSvgCanvasRange = basegfx::B2DRange(fLeft, fTop, fLeft+fW, fTop+fH);
                            }

                        }

                        // to be completely correct in Svg sense it is necessary to clip
                        // the whole content to the given canvas. I choose here to do this
                        // initially despite I found various examples of Svg files out there
                        // which have no correct values for this clipping. It's correct
                        // due to the Svg spec.

                        // different from Svg we have the possibility with primitives to get
                        // a correct bounding box for the geometry. Get it for evtl. taking action
                        const basegfx::B2DRange aContentRange(
                            aSequence.getB2DRange(
                                drawinglayer::geometry::ViewInformation2D()));

                        if(aSvgCanvasRange.isInside(aContentRange))
                        {
                            // no clip needed, but an invisible HiddenGeometryPrimitive2D
                            // to allow getting the full Svg range using the primitive mechanisms.
                            // This is needed since e.g. an SdrObject using this as graphic will
                            // create a mapping transformation to exactly map the content to its
                            // real life size
                            const drawinglayer::primitive2d::Primitive2DReference xLine(
                                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                                    basegfx::utils::createPolygonFromRect(
                                        aSvgCanvasRange),
                                    basegfx::BColor(0.0, 0.0, 0.0)));
                            const drawinglayer::primitive2d::Primitive2DReference xHidden(
                                new drawinglayer::primitive2d::HiddenGeometryPrimitive2D(
                                    drawinglayer::primitive2d::Primitive2DContainer { xLine }));

                            aSequence.push_back(xHidden);
                        }
                        else if(aSvgCanvasRange.overlaps(aContentRange))
                        {
                            // Clip is necessary. This will make Svg images evtl. smaller
                            // than wanted from Svg (the free space which may be around it is
                            // conform to the Svg spec), but avoids an expensive and unnecessary
                            // clip. Keep the full Svg range here to get the correct mappings
                            // to objects using this. Optimizations can be done in the processors
                            const drawinglayer::primitive2d::Primitive2DReference xMask(
                                new drawinglayer::primitive2d::MaskPrimitive2D(
                                    basegfx::B2DPolyPolygon(
                                        basegfx::utils::createPolygonFromRect(
                                            aSvgCanvasRange)),
                                    std::move(aSequence)));

                            aSequence = drawinglayer::primitive2d::Primitive2DContainer { xMask };
                        }
                        else
                        {
                            // not inside, no overlap. Empty Svg
                            aSequence.clear();
                        }

                        if(!aSequence.empty())
                        {
                            // Another correction:
                            // If no Width/Height is set (usually done in
                            // <svg ... width="215.9mm" height="279.4mm" >) which
                            // is the case for own-Impress-exports, assume that
                            // the Units are already 100ThMM.
                            // Maybe only for own-Impress-exports, thus may need to be
                            // &&ed with getDocument().findSvgNodeById("ooo:meta_slides"),
                            // but does not need to be.
                            bool bEmbedInFinalTransformPxTo100ThMM(true);

                            if(getDocument().findSvgNodeById("ooo:meta_slides")
                                && !getWidth().isSet()
                                && !getHeight().isSet())
                            {
                                bEmbedInFinalTransformPxTo100ThMM = false;
                            }

                            if(bEmbedInFinalTransformPxTo100ThMM)
                            {
                                // embed in transform primitive to scale to 1/100th mm
                                // to get from Svg coordinates (px) to drawinglayer coordinates
                                constexpr double fScaleTo100thmm(o3tl::convert(1.0, o3tl::Length::px, o3tl::Length::mm100));
                                const basegfx::B2DHomMatrix aTransform(
                                    basegfx::utils::createScaleB2DHomMatrix(
                                        fScaleTo100thmm,
                                        fScaleTo100thmm));

                                const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aTransform,
                                        std::move(aSequence)));

                                aSequence = drawinglayer::primitive2d::Primitive2DContainer { xTransform };
                            }

                            // append to result
                            rTarget.append(aSequence);
                        }
                    }
                }
            }

            if(!(aSequence.empty() && !getParent() && getViewBox()))
                return;

            // tdf#118232 No geometry, Outermost SVG element and we have a ViewBox.
            // Create a HiddenGeometry Primitive containing an expanded
            // hairline geometry to have the size contained
            const drawinglayer::primitive2d::Primitive2DReference xLine(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    basegfx::utils::createPolygonFromRect(
                        *getViewBox()),
                    basegfx::BColor(0.0, 0.0, 0.0)));
            const drawinglayer::primitive2d::Primitive2DReference xHidden(
                new drawinglayer::primitive2d::HiddenGeometryPrimitive2D(
                    drawinglayer::primitive2d::Primitive2DContainer { xLine }));

            rTarget.push_back(xHidden);
        }

        basegfx::B2DRange SvgSvgNode::getCurrentViewPort() const
        {
            if(getViewBox())
            {
                return *(getViewBox());
            }
            else // viewport should be given by x, y, width, and height
            {
                // Extract known viewport data
                // bXXXIsAbsolute tracks whether relative values could be resolved to absolute values
                if (getParent())
                {
                    // If width or height is not provided, the default 100% is used, see SVG 1.1 section 5.1.2
                    // value 0.0 here is only to initialize variable
                    bool bWidthIsAbsolute(getWidth().isSet() && SvgUnit::percent != getWidth().getUnit());
                    double fW( bWidthIsAbsolute ? getWidth().solveNonPercentage(*this) : 0.0);
                    bool bHeightIsAbsolute(getHeight().isSet() && SvgUnit::percent != getHeight().getUnit());
                    double fH( bHeightIsAbsolute ? getHeight().solveNonPercentage(*this) : 0.0);

                    // If x or y not provided, then default 0.0 is used, see SVG 1.1 Section 5.1.2
                    bool bXIsAbsolute((getX().isSet() && SvgUnit::percent != getX().getUnit()) || !getX().isSet());
                    double fX( bXIsAbsolute && getX().isSet() ? getX().solveNonPercentage(*this) : 0.0);

                    bool bYIsAbsolute((getY().isSet() && SvgUnit::percent != getY().getUnit()) || !getY().isSet());
                    double fY( bYIsAbsolute && getY().isSet() ? getY().solveNonPercentage(*this) : 0.0);

                    if (bXIsAbsolute && bYIsAbsolute && bWidthIsAbsolute && bHeightIsAbsolute)
                    {
                        return basegfx::B2DRange(fX, fY, fX+fW, fY+fH);
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
                            return basegfx::B2DRange(fX, fY, fX+fW, fY+fH);
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
                    // SVG 1.1 defines in section 5.1.2 that x,y has no meaning for the outermost SVG element.
                    bool bWidthIsAbsolute(getWidth().isSet() && SvgUnit::percent != getWidth().getUnit());
                    bool bHeightIsAbsolute(getHeight().isSet() && SvgUnit::percent != getHeight().getUnit());
                    if (bWidthIsAbsolute && bHeightIsAbsolute)
                    {
                        double fW( getWidth().solveNonPercentage(*this) );
                        double fH( getHeight().solveNonPercentage(*this) );
                        return basegfx::B2DRange(0.0, 0.0, fW, fH);
                    }
                    else // no fallback exists
                    {
                            return SvgNode::getCurrentViewPort();
                    }
                }
// TODO: Is it possible to decompose and use the bounding box of the children, if even the
//       outermost svg has no information to resolve percentage? Is it worth, how expensive is it?

            }
        }

} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
