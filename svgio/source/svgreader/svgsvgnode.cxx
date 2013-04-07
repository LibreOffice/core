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

        void SvgSvgNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DSequence aSequence;

            // decompose childs
            SvgNode::decomposeSvgNode(aSequence, bReferenced);

            if(aSequence.hasElements())
            {
                if(getParent())
                {
                    if(getViewBox())
                    {
                        // Svg defines that with no width or no height the viewBox content is empty,
                        // so both need to exist
                        if(!basegfx::fTools::equalZero(getViewBox()->getWidth()) && !basegfx::fTools::equalZero(getViewBox()->getHeight()))
                        {
                            // create target range homing x,y, width and height as given
                            const double fX(getX().isSet() ? getX().solve(*this, xcoordinate) : 0.0);
                            const double fY(getY().isSet() ? getY().solve(*this, ycoordinate) : 0.0);
                            const double fW(getWidth().isSet() ? getWidth().solve(*this, xcoordinate) : getViewBox()->getWidth());
                            const double fH(getHeight().isSet() ? getHeight().solve(*this, ycoordinate) : getViewBox()->getHeight());
                            const basegfx::B2DRange aTarget(fX, fY, fX + fW, fY + fH);

                            if(aTarget.equal(*getViewBox()))
                            {
                                // no mapping needed, append
                                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, aSequence);
                            }
                            else
                            {
                                // create mapping
                                const SvgAspectRatio& rRatio = getSvgAspectRatio();

                                if(rRatio.isSet())
                                {
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
                                else
                                {
                                    // choose default mapping
                                    const basegfx::B2DHomMatrix aEmbeddingTransform(
                                        rRatio.createLinearMapping(
                                            aTarget, *getViewBox()));

                                    // embed in transformation
                                    const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                        new drawinglayer::primitive2d::TransformPrimitive2D(
                                            aEmbeddingTransform,
                                            aSequence));

                                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(rTarget, xTransform);
                                }
                            }
                        }
                    }
                    else
                    {
                        // check if we have a size
                        const double fW(getWidth().isSet() ? getWidth().solve(*this, xcoordinate) : 0.0);
                        const double fH(getHeight().isSet() ? getHeight().solve(*this, ycoordinate) : 0.0);

                        // Svg defines that a negative value is an error and that 0.0 disables rendering
                        if(basegfx::fTools::more(fW, 0.0) && basegfx::fTools::more(fH, 0.0))
                        {
                            // check if we have a x,y position
                            const double fX(getX().isSet() ? getX().solve(*this, xcoordinate) : 0.0);
                            const double fY(getY().isSet() ? getY().solve(*this, ycoordinate) : 0.0);

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
                else
                {
                    // Outermost SVG element; create target range homing width and height as given.
                    // SVG defines that x,y has no meanig for the outermost SVG element. Use a fallback
                    // width and height of din A 4 (21 x 29,7 cm)
                    double fW(getWidth().isSet() ? getWidth().solve(*this, xcoordinate) : (210.0 * 3.543307));
                    double fH(getHeight().isSet() ? getHeight().solve(*this, ycoordinate) : (297.0 * 3.543307));

                    // Svg defines that a negative value is an error and that 0.0 disables rendering
                    if(basegfx::fTools::more(fW, 0.0) && basegfx::fTools::more(fH, 0.0))
                    {
                        const basegfx::B2DRange aSvgCanvasRange(0.0, 0.0, fW, fH);

                        if(getViewBox())
                        {
                            if(!basegfx::fTools::equalZero(getViewBox()->getWidth()) && !basegfx::fTools::equalZero(getViewBox()->getHeight()))
                            {
                                // create mapping
                                const SvgAspectRatio& rRatio = getSvgAspectRatio();
                                basegfx::B2DHomMatrix aViewBoxMapping;

                                if(rRatio.isSet())
                                {
                                    // let mapping be created from SvgAspectRatio
                                    aViewBoxMapping = rRatio.createMapping(aSvgCanvasRange, *getViewBox());

                                    // no need to check ratio here for slice, the outermost Svg will
                                    // be clipped anyways (see below)
                                }
                                else
                                {
                                    // choose default mapping
                                    aViewBoxMapping = rRatio.createLinearMapping(aSvgCanvasRange, *getViewBox());
                                }

                                // scale content to viewBox definitions
                                const drawinglayer::primitive2d::Primitive2DReference xTransform(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        aViewBoxMapping,
                                        aSequence));

                                aSequence = drawinglayer::primitive2d::Primitive2DSequence(&xTransform, 1);
                            }
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
                            // where 1 mm == 3.543307 px to get from Svg coordinates to
                            // drawinglayer ones
                            const double fScaleTo100thmm(100.0 / 3.543307);
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
            else
            {
                return SvgNode::getCurrentViewPort();
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
