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

#include <sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <sdr/attribute/sdrtextattribute.hxx>
#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/svdotext.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/attribute/sdrglowattribute.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
namespace
{
// See also: SdrTextObj::AdjustRectToTextDistance
basegfx::B2DRange getTextAnchorRange(const attribute::SdrTextAttribute& rText,
                                     const basegfx::B2DRange& rSnapRange)
{
    // Take vertical text orientation into account when deciding
    // which dimension is its width, and which is its height
    const OutlinerParaObject& rOutlinerParaObj = rText.getOutlinerParaObject();
    const bool bVerticalWriting(rOutlinerParaObj.IsEffectivelyVertical());
    const double fWidthForText = bVerticalWriting ? rSnapRange.getHeight() : rSnapRange.getWidth();
    // create a range describing the wanted text position and size (aTextAnchorRange). This
    // means to use the text distance values here
    // If the margin is larger than the entire width of the text area, then limit the
    // margin.
    const double fTextLeftDistance
        = std::min(static_cast<double>(rText.getTextLeftDistance()), fWidthForText);
    const double nTextRightDistance
        = std::min(static_cast<double>(rText.getTextRightDistance()), fWidthForText);
    double fDistanceForTextL, fDistanceForTextT, fDistanceForTextR, fDistanceForTextB;
    if (!bVerticalWriting)
    {
        fDistanceForTextL = fTextLeftDistance;
        fDistanceForTextT = rText.getTextUpperDistance();
        fDistanceForTextR = nTextRightDistance;
        fDistanceForTextB = rText.getTextLowerDistance();
    }
    else if (rOutlinerParaObj.IsTopToBottom())
    {
        fDistanceForTextL = rText.getTextLowerDistance();
        fDistanceForTextT = fTextLeftDistance;
        fDistanceForTextR = rText.getTextUpperDistance();
        fDistanceForTextB = nTextRightDistance;
    }
    else
    {
        fDistanceForTextL = rText.getTextUpperDistance();
        fDistanceForTextT = nTextRightDistance;
        fDistanceForTextR = rText.getTextLowerDistance();
        fDistanceForTextB = fTextLeftDistance;
    }
    const basegfx::B2DPoint aTopLeft(rSnapRange.getMinX() + fDistanceForTextL,
                                     rSnapRange.getMinY() + fDistanceForTextT);
    const basegfx::B2DPoint aBottomRight(rSnapRange.getMaxX() - fDistanceForTextR,
                                         rSnapRange.getMaxY() - fDistanceForTextB);
    basegfx::B2DRange aAnchorRange;
    aAnchorRange.expand(aTopLeft);
    aAnchorRange.expand(aBottomRight);

    // If the shape has no width, then don't attempt to break the text into multiple
    // lines, not a single character would satisfy a zero width requirement.
    // SdrTextObj::impDecomposeBlockTextPrimitive() uses the same constant to
    // effectively set no limits.
    if (!bVerticalWriting && aAnchorRange.getWidth() == 0)
    {
        aAnchorRange.expand(basegfx::B2DPoint(aTopLeft.getX() - 1000000, aTopLeft.getY()));
        aAnchorRange.expand(basegfx::B2DPoint(aBottomRight.getX() + 1000000, aBottomRight.getY()));
    }
    else if (bVerticalWriting && aAnchorRange.getHeight() == 0)
    {
        aAnchorRange.expand(basegfx::B2DPoint(aTopLeft.getX(), aTopLeft.getY() - 1000000));
        aAnchorRange.expand(basegfx::B2DPoint(aBottomRight.getX(), aBottomRight.getY() + 1000000));
    }
    return aAnchorRange;
}
};

        class TransparencePrimitive2D;

        Primitive2DReference createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient)
        {
            // when we have no given definition range, use the range of the given geometry
            // also for definition (simplest case)
            const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolyPolygon));

            return createPolyPolygonFillPrimitive(
                rPolyPolygon,
                aRange,
                rFill,
                rFillGradient);
        }

        Primitive2DReference createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::B2DRange& rDefinitionRange,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient)
        {
            if(basegfx::fTools::moreOrEqual(rFill.getTransparence(), 1.0))
            {
                return Primitive2DReference();
            }

            // prepare fully scaled polygon
            rtl::Reference<BasePrimitive2D> pNewFillPrimitive;

            if(!rFill.getGradient().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonGradientPrimitive2D(
                    rPolyPolygon,
                    rDefinitionRange,
                    rFill.getGradient());
            }
            else if(!rFill.getHatch().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonHatchPrimitive2D(
                    rPolyPolygon,
                    rDefinitionRange,
                    rFill.getColor(),
                    rFill.getHatch());
            }
            else if(!rFill.getFillGraphic().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonGraphicPrimitive2D(
                    rPolyPolygon,
                    rDefinitionRange,
                    rFill.getFillGraphic().createFillGraphicAttribute(rDefinitionRange));
            }
            else
            {
                pNewFillPrimitive = new PolyPolygonColorPrimitive2D(
                    rPolyPolygon,
                    rFill.getColor());
            }

            if(0.0 != rFill.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitive
                Primitive2DContainer aContent { pNewFillPrimitive };
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(std::move(aContent), rFill.getTransparence()));
            }
            else if(!rFillGradient.isDefault())
            {
                // create sequence with created fill primitive
                Primitive2DContainer aContent { pNewFillPrimitive };

                // create FillGradientPrimitive2D for transparence and add to new sequence
                // fillGradientPrimitive is enough here (compared to PolyPolygonGradientPrimitive2D) since float transparence will be masked anyways
                const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolyPolygon));
                Primitive2DReference xRefB(
                    new FillGradientPrimitive2D(
                        aRange,
                        rDefinitionRange,
                        rFillGradient));
                Primitive2DContainer aAlpha { xRefB };

                // create TransparencePrimitive2D using alpha and content
                return Primitive2DReference(new TransparencePrimitive2D(std::move(aContent), std::move(aAlpha)));
            }
            else
            {
                // add to decomposition
                return pNewFillPrimitive;
            }
        }

        Primitive2DReference createPolygonLinePrimitive(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::SdrLineAttribute& rLine,
            const attribute::SdrLineStartEndAttribute& rStroke)
        {
            // create line and stroke attribute
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin(), rLine.getCap());
            const attribute::StrokeAttribute aStrokeAttribute(std::vector(rLine.getDotDashArray()), rLine.getFullDotDashLen());
            rtl::Reference<BasePrimitive2D> pNewLinePrimitive;

            if(!rPolygon.isClosed() && !rStroke.isDefault())
            {
                attribute::LineStartEndAttribute aStart(rStroke.getStartWidth(), rStroke.getStartPolyPolygon(), rStroke.isStartCentered());
                attribute::LineStartEndAttribute aEnd(rStroke.getEndWidth(), rStroke.getEndPolyPolygon(), rStroke.isEndCentered());

                // create data
                pNewLinePrimitive = new PolygonStrokeArrowPrimitive2D(rPolygon, aLineAttribute, aStrokeAttribute, aStart, aEnd);
            }
            else
            {
                // create data
                pNewLinePrimitive = new PolygonStrokePrimitive2D(rPolygon, aLineAttribute, aStrokeAttribute);
            }

            if(0.0 != rLine.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitive
                Primitive2DContainer aContent { pNewLinePrimitive };
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(std::move(aContent), rLine.getTransparence()));
            }
            else
            {
                // add to decomposition
                return pNewLinePrimitive;
            }
        }

        Primitive2DReference createTextPrimitive(
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrTextAttribute& rText,
            const attribute::SdrLineAttribute& rStroke,
            bool bCellText,
            bool bWordWrap)
        {
            basegfx::B2DHomMatrix aAnchorTransform(rObjectTransform);
            rtl::Reference<SdrTextPrimitive2D> pNew;

            if(rText.isContour())
            {
                // contour text
                if(!rStroke.isDefault() && 0.0 != rStroke.getWidth())
                {
                    // take line width into account and shrink contour polygon accordingly
                    // decompose to get scale
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    // scale outline to object's size to allow growing with value relative to that size
                    // and also to keep aspect ratio
                    basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                    aScaledUnitPolyPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(
                        fabs(aScale.getX()), fabs(aScale.getY())));

                    // grow the polygon. To shrink, use negative value (half width)
                    aScaledUnitPolyPolygon = basegfx::utils::growInNormalDirection(aScaledUnitPolyPolygon, -(rStroke.getWidth() * 0.5));

                    // scale back to unit polygon
                    aScaledUnitPolyPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(
                        0.0 != aScale.getX() ? 1.0 / aScale.getX() : 1.0,
                        0.0 != aScale.getY() ? 1.0 / aScale.getY() : 1.0));

                    // create with unit polygon
                    pNew = new SdrContourTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aScaledUnitPolyPolygon,
                        rObjectTransform);
                }
                else
                {
                    // create with unit polygon
                    pNew = new SdrContourTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        rUnitPolyPolygon,
                        rObjectTransform);
                }
            }
            else if(!rText.getSdrFormTextAttribute().isDefault())
            {
                // text on path, use scaled polygon
                basegfx::B2DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
                aScaledPolyPolygon.transform(rObjectTransform);
                pNew = new SdrPathTextPrimitive2D(
                    &rText.getSdrText(),
                    rText.getOutlinerParaObject(),
                    aScaledPolyPolygon,
                    rText.getSdrFormTextAttribute());
            }
            else
            {
                // rObjectTransform is the whole SdrObject transformation from unit rectangle
                // to its size and position. Decompose to allow working with single values.
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                // extract mirroring
                const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
                const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));
                aScale = basegfx::absolute(aScale);

                // Get the real size, since polygon outline and scale
                // from the object transformation may vary (e.g. ellipse segments)
                basegfx::B2DHomMatrix aJustScaleTransform;
                aJustScaleTransform.set(0, 0, aScale.getX());
                aJustScaleTransform.set(1, 1, aScale.getY());
                basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                aScaledUnitPolyPolygon.transform(aJustScaleTransform);
                const basegfx::B2DRange aTextAnchorRange
                    = getTextAnchorRange(rText, basegfx::utils::getRange(aScaledUnitPolyPolygon));

                // now create a transformation from this basic range (aTextAnchorRange)
                // #i121494# if we have no scale use at least 1.0 to have a carrier e.g. for
                // mirror values, else these will get lost
                aAnchorTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(
                    basegfx::fTools::equalZero(aTextAnchorRange.getWidth()) ? 1.0 : aTextAnchorRange.getWidth(),
                    basegfx::fTools::equalZero(aTextAnchorRange.getHeight()) ? 1.0 : aTextAnchorRange.getHeight(),
                    aTextAnchorRange.getMinX(), aTextAnchorRange.getMinY());

                // apply mirroring
                aAnchorTransform.scale(bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0);

                // apply object's other transforms
                aAnchorTransform = basegfx::utils::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                    * aAnchorTransform;

                if(rText.isFitToSize())
                {
                    // stretched text in range
                    pNew = new SdrStretchTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aAnchorTransform,
                        rText.isFixedCellHeight());
                }
                else if(rText.isAutoFit())
                {
                    // isotropically scaled text in range
                    pNew = new SdrAutoFitTextPrimitive2D(
                                    &rText.getSdrText(),
                                    rText.getOutlinerParaObject(),
                                    aAnchorTransform,
                                    bWordWrap);
                }
                else if( rText.isChainable() && !rText.isInEditMode() )
                {
                    pNew = new SdrChainedTextPrimitive2D(
                                    &rText.getSdrText(),
                                    rText.getOutlinerParaObject(),
                                    aAnchorTransform );
                }
                else // text in range
                {
                    // build new primitive
                    pNew = new SdrBlockTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aAnchorTransform,
                        rText.getSdrTextHorzAdjust(),
                        rText.getSdrTextVertAdjust(),
                        rText.isFixedCellHeight(),
                        rText.isScroll(),
                        bCellText,
                        bWordWrap);
                }
            }

            OSL_ENSURE(pNew != nullptr, "createTextPrimitive: no text primitive created (!)");

            if(rText.isBlink())
            {
                // prepare animation and primitive list
                drawinglayer::animation::AnimationEntryList aAnimationList;
                rText.getBlinkTextTiming(aAnimationList);

                if(0.0 != aAnimationList.getDuration())
                {
                    // create content sequence
                    Primitive2DReference xRefA(pNew);
                    Primitive2DContainer aContent { xRefA };

                    // create and add animated switch primitive
                    return Primitive2DReference(new AnimatedBlinkPrimitive2D(aAnimationList, std::move(aContent)));
                }
                else
                {
                    // add to decomposition
                    return Primitive2DReference(pNew);
                }
            }

            if(rText.isScroll())
            {
                // suppress scroll when FontWork
                if(rText.getSdrFormTextAttribute().isDefault())
                {
                    // get scroll direction
                    const SdrTextAniDirection eDirection(rText.getSdrText().GetObject().GetTextAniDirection());
                    const bool bHorizontal(SdrTextAniDirection::Left == eDirection || SdrTextAniDirection::Right == eDirection);

                    // decompose to get separated values for the scroll box
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    aAnchorTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    // build transform from scaled only to full AnchorTransform and inverse
                    const basegfx::B2DHomMatrix aSRT(basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
                        fShearX, fRotate, aTranslate));
                    basegfx::B2DHomMatrix aISRT(aSRT);
                    aISRT.invert();

                    // bring the primitive back to scaled only and get scaled range, create new clone for this
                    rtl::Reference<SdrTextPrimitive2D> pNew2 = pNew->createTransformedClone(aISRT);
                    OSL_ENSURE(pNew2, "createTextPrimitive: Could not create transformed clone of text primitive (!)");
                    pNew = pNew2.get();

                    // create neutral geometry::ViewInformation2D for local range and decompose calls. This is okay
                    // since the decompose is view-independent
                    geometry::ViewInformation2D aViewInformation2D;

                    // get range
                    const basegfx::B2DRange aScaledRange(pNew->getB2DRange(aViewInformation2D));

                    // create left outside and right outside transformations. Also take care
                    // of the clip rectangle
                    basegfx::B2DHomMatrix aLeft, aRight;
                    basegfx::B2DPoint aClipTopLeft(0.0, 0.0);
                    basegfx::B2DPoint aClipBottomRight(aScale.getX(), aScale.getY());

                    if(bHorizontal)
                    {
                        aClipTopLeft.setY(aScaledRange.getMinY());
                        aClipBottomRight.setY(aScaledRange.getMaxY());
                        aLeft.translate(-aScaledRange.getMaxX(), 0.0);
                        aRight.translate(aScale.getX() - aScaledRange.getMinX(), 0.0);
                    }
                    else
                    {
                        aClipTopLeft.setX(aScaledRange.getMinX());
                        aClipBottomRight.setX(aScaledRange.getMaxX());
                        aLeft.translate(0.0, -aScaledRange.getMaxY());
                        aRight.translate(0.0, aScale.getY() - aScaledRange.getMinY());
                    }

                    aLeft *= aSRT;
                    aRight *= aSRT;

                    // prepare animation list
                    drawinglayer::animation::AnimationEntryList aAnimationList;

                    if(bHorizontal)
                    {
                        rText.getScrollTextTiming(aAnimationList, aScale.getX(), aScaledRange.getWidth());
                    }
                    else
                    {
                        rText.getScrollTextTiming(aAnimationList, aScale.getY(), aScaledRange.getHeight());
                    }

                    if(0.0 != aAnimationList.getDuration())
                    {
                        // create a new Primitive2DContainer containing the animated text in its scaled only state.
                        // use the decomposition to force to simple text primitives, those will no longer
                        // need the outliner for formatting (alternatively it is also possible to just add
                        // pNew to aNewPrimitiveSequence)
                        Primitive2DContainer aAnimSequence;
                        pNew->get2DDecomposition(aAnimSequence, aViewInformation2D);
                        pNew.clear();

                        // create a new animatedInterpolatePrimitive and add it
                        Primitive2DReference xRefA(new AnimatedInterpolatePrimitive2D({ aLeft, aRight }, aAnimationList, std::move(aAnimSequence)));
                        Primitive2DContainer aContent { xRefA };

                        // scrolling needs an encapsulating clipping primitive
                        const basegfx::B2DRange aClipRange(aClipTopLeft, aClipBottomRight);
                        basegfx::B2DPolygon aClipPolygon(basegfx::utils::createPolygonFromRect(aClipRange));
                        aClipPolygon.transform(aSRT);
                        return Primitive2DReference(new MaskPrimitive2D(basegfx::B2DPolyPolygon(aClipPolygon), std::move(aContent)));
                    }
                    else
                    {
                        // add to decomposition
                        return Primitive2DReference(pNew);
                    }
                }
            }

            if(rText.isInEditMode())
            {
                // #i97628#
                // encapsulate with TextHierarchyEditPrimitive2D to allow renderers
                // to suppress actively edited content if needed
                Primitive2DReference xRefA(pNew);
                Primitive2DContainer aContent { xRefA };

                // create and add TextHierarchyEditPrimitive2D primitive
                return Primitive2DReference(new TextHierarchyEditPrimitive2D(std::move(aContent)));
            }
            else
            {
                // add to decomposition
                return pNew;
            }
        }

        Primitive2DContainer createEmbeddedShadowPrimitive(
            Primitive2DContainer&& rContent,
            const attribute::SdrShadowAttribute& rShadow,
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const Primitive2DContainer* pContentForShadow)
        {
            if(rContent.empty())
                return std::move(rContent);

            basegfx::B2DHomMatrix aShadowOffset;

            if(rShadow.getSize().getX() != 100000)
            {
                basegfx::B2DTuple aScale;
                basegfx::B2DTuple aTranslate;
                double fRotate = 0;
                double fShearX = 0;
                rObjectMatrix.decompose(aScale, aTranslate, fRotate, fShearX);
                // Scale the shadow
                double nTranslateX = aTranslate.getX();
                double nTranslateY = aTranslate.getY();

                // The origin for scaling is the top left corner by default. A negative
                // shadow offset changes the origin.
                if (rShadow.getOffset().getX() < 0)
                    nTranslateX += aScale.getX();
                if (rShadow.getOffset().getY() < 0)
                    nTranslateY += aScale.getY();

                aShadowOffset.translate(-nTranslateX, -nTranslateY);
                aShadowOffset.scale(rShadow.getSize().getX() * 0.00001, rShadow.getSize().getY() * 0.00001);
                aShadowOffset.translate(nTranslateX, nTranslateY);
            }

            aShadowOffset.translate(rShadow.getOffset().getX(), rShadow.getOffset().getY());

            // create shadow primitive and add content
            const Primitive2DContainer& rContentForShadow
                    = pContentForShadow ? *pContentForShadow : rContent;
            int nContentWithTransparence = std::count_if(
                rContentForShadow.begin(), rContentForShadow.end(),
                [](const Primitive2DReference& xChild) {
                    auto pChild = dynamic_cast<BufferedDecompositionPrimitive2D*>(xChild.get());
                    return pChild && pChild->getTransparenceForShadow() != 0;
                });
            if (nContentWithTransparence == 0)
            {
                Primitive2DContainer aRetval(2);
                aRetval[0] = Primitive2DReference(
                    new ShadowPrimitive2D(
                        aShadowOffset,
                        rShadow.getColor(),
                        rShadow.getBlur(),
                        Primitive2DContainer(pContentForShadow ? *pContentForShadow : rContent)));

                if (0.0 != rShadow.getTransparence())
                {
                    // create SimpleTransparencePrimitive2D
                    Primitive2DContainer aTempContent{ aRetval[0] };

                    aRetval[0] = Primitive2DReference(
                        new UnifiedTransparencePrimitive2D(
                            std::move(aTempContent),
                            rShadow.getTransparence()));
                }

                aRetval[1] = Primitive2DReference(new GroupPrimitive2D(std::move(rContent)));
                return aRetval;
            }

            Primitive2DContainer aRetval;
            for (const auto& xChild : rContentForShadow)
            {
                double fChildTransparence = 0.0;
                auto pChild = dynamic_cast<BufferedDecompositionPrimitive2D*>(xChild.get());
                if (pChild)
                {
                        fChildTransparence = pChild->getTransparenceForShadow();
                        fChildTransparence /= 100;
                }
                aRetval.push_back(Primitive2DReference(
                    new ShadowPrimitive2D(aShadowOffset, rShadow.getColor(), rShadow.getBlur(),
                                            Primitive2DContainer({ xChild }))));
                if (rShadow.getTransparence() != 0.0 || fChildTransparence != 0.0)
                {
                    Primitive2DContainer aTempContent{ aRetval.back() };

                    double fChildAlpha = 1.0 - fChildTransparence;
                    double fShadowAlpha = 1.0 - rShadow.getTransparence();
                    double fTransparence = 1.0 - fChildAlpha * fShadowAlpha;
                    aRetval.back() = Primitive2DReference(new UnifiedTransparencePrimitive2D(
                            std::move(aTempContent), fTransparence));
                }
            }

            aRetval.push_back(
                    Primitive2DReference(new GroupPrimitive2D(std::move(rContent))));
            return aRetval;
        }

        Primitive2DContainer createEmbeddedGlowPrimitive(
            Primitive2DContainer&& rContent,
            const attribute::SdrGlowAttribute& rGlow)
        {
            if(rContent.empty())
                return std::move(rContent);
            Primitive2DContainer aRetval(2);
            aRetval[0] = Primitive2DReference(
                new GlowPrimitive2D(rGlow.getColor(), rGlow.getRadius(), Primitive2DContainer(rContent)));
            aRetval[1] = Primitive2DReference(new GroupPrimitive2D(Primitive2DContainer(rContent)));
            return aRetval;
        }

        Primitive2DContainer createEmbeddedSoftEdgePrimitive(Primitive2DContainer&& aContent,
                                                             sal_Int32 nRadius)
        {
            if (aContent.empty() || !nRadius)
                return std::move(aContent);
            Primitive2DContainer aRetval(1);
            aRetval[0] = Primitive2DReference(new SoftEdgePrimitive2D(nRadius, std::move(aContent)));
            return aRetval;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
