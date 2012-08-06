/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/svdotext.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DReference createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient)
        {
            // prepare fully scaled polygon
            basegfx::B2DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
            aScaledPolyPolygon.transform(rObjectTransform);
            BasePrimitive2D* pNewFillPrimitive = 0;

            if(!rFill.getGradient().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonGradientPrimitive2D(aScaledPolyPolygon, rFill.getGradient());
            }
            else if(!rFill.getHatch().isDefault())
            {
                pNewFillPrimitive = new PolyPolygonHatchPrimitive2D(aScaledPolyPolygon, rFill.getColor(), rFill.getHatch());
            }
            else if(!rFill.getBitmap().isDefault())
            {
                const basegfx::B2DRange aRange(basegfx::tools::getRange(aScaledPolyPolygon));
                pNewFillPrimitive = new PolyPolygonBitmapPrimitive2D(aScaledPolyPolygon, rFill.getBitmap().getFillBitmapAttribute(aRange));
            }
            else
            {
                pNewFillPrimitive = new PolyPolygonColorPrimitive2D(aScaledPolyPolygon, rFill.getColor());
            }

            if(0.0 != rFill.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitive
                const Primitive2DReference xRefA(pNewFillPrimitive);
                const Primitive2DSequence aContent(&xRefA, 1L);
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(aContent, rFill.getTransparence()));
            }
            else if(!rFillGradient.isDefault())
            {
                // create sequence with created fill primitive
                const Primitive2DReference xRefA(pNewFillPrimitive);
                const Primitive2DSequence aContent(&xRefA, 1L);

                // create FillGradientPrimitive2D for transparence and add to new sequence
                // fillGradientPrimitive is enough here (compared to PolyPolygonGradientPrimitive2D) since float transparence will be masked anyways
                const basegfx::B2DRange aRange(basegfx::tools::getRange(aScaledPolyPolygon));
                const Primitive2DReference xRefB(new FillGradientPrimitive2D(aRange, rFillGradient));
                const Primitive2DSequence aAlpha(&xRefB, 1L);

                // create TransparencePrimitive2D using alpha and content
                return Primitive2DReference(new TransparencePrimitive2D(aContent, aAlpha));
            }
            else
            {
                // add to decomposition
                return Primitive2DReference(pNewFillPrimitive);
            }
        }

        Primitive2DReference createPolygonLinePrimitive(
            const basegfx::B2DPolygon& rUnitPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine,
            const attribute::SdrLineStartEndAttribute& rStroke)
        {
            // prepare fully scaled polygon
            basegfx::B2DPolygon aScaledPolygon(rUnitPolygon);
            aScaledPolygon.transform(rObjectTransform);

            // create line and stroke attribute
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin());
            const attribute::StrokeAttribute aStrokeAttribute(rLine.getDotDashArray(), rLine.getFullDotDashLen());
            BasePrimitive2D* pNewLinePrimitive = 0L;

            if(!rUnitPolygon.isClosed() && !rStroke.isDefault())
            {
                attribute::LineStartEndAttribute aStart(rStroke.getStartWidth(), rStroke.getStartPolyPolygon(), rStroke.isStartCentered());
                attribute::LineStartEndAttribute aEnd(rStroke.getEndWidth(), rStroke.getEndPolyPolygon(), rStroke.isEndCentered());

                // create data
                pNewLinePrimitive = new PolygonStrokeArrowPrimitive2D(aScaledPolygon, aLineAttribute, aStrokeAttribute, aStart, aEnd);
            }
            else
            {
                // create data
                pNewLinePrimitive = new PolygonStrokePrimitive2D(aScaledPolygon, aLineAttribute, aStrokeAttribute);
            }

            if(0.0 != rLine.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitive
                const Primitive2DReference xRefA(pNewLinePrimitive);
                const Primitive2DSequence aContent(&xRefA, 1L);
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(aContent, rLine.getTransparence()));
            }
            else
            {
                // add to decomposition
                return Primitive2DReference(pNewLinePrimitive);
            }
        }

        Primitive2DReference createTextPrimitive(
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrTextAttribute& rText,
            const attribute::SdrLineAttribute& rStroke,
            bool bCellText,
            bool bWordWrap,
            bool bClipOnBounds)
        {
            basegfx::B2DHomMatrix aAnchorTransform(rObjectTransform);
            SdrTextPrimitive2D* pNew = 0;

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
                    aScaledUnitPolyPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(
                        fabs(aScale.getX()), fabs(aScale.getY())));

                    // grow the polygon. To shrink, use negative value (half width)
                    aScaledUnitPolyPolygon = basegfx::tools::growInNormalDirection(aScaledUnitPolyPolygon, -(rStroke.getWidth() * 0.5));

                    // scale back to unit polygon
                    aScaledUnitPolyPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(
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
                // to it's size and position. Decompose to allow working with single values.
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                // extract mirroring
                const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
                const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));
                aScale = basegfx::absolute(aScale);

                // Get the real size, since polygon ountline and scale
                // from the object transformation may vary (e.g. ellipse segments)
                basegfx::B2DHomMatrix aJustScaleTransform;
                aJustScaleTransform.set(0, 0, aScale.getX());
                aJustScaleTransform.set(1, 1, aScale.getY());
                basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                aScaledUnitPolyPolygon.transform(aJustScaleTransform);
                const basegfx::B2DRange aSnapRange(basegfx::tools::getRange(aScaledUnitPolyPolygon));

                // create a range describing the wanted text position and size (aTextAnchorRange). This
                // means to use the text distance values here
                const basegfx::B2DPoint aTopLeft(aSnapRange.getMinX() + rText.getTextLeftDistance(), aSnapRange.getMinY() + rText.getTextUpperDistance());
                const basegfx::B2DPoint aBottomRight(aSnapRange.getMaxX() - rText.getTextRightDistance(), aSnapRange.getMaxY() - rText.getTextLowerDistance());
                basegfx::B2DRange aTextAnchorRange;
                aTextAnchorRange.expand(aTopLeft);
                aTextAnchorRange.expand(aBottomRight);

                // now create a transformation from this basic range (aTextAnchorRange)
                aAnchorTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aTextAnchorRange.getWidth(), aTextAnchorRange.getHeight(),
                    aTextAnchorRange.getMinX(), aTextAnchorRange.getMinY());

                // apply mirroring
                aAnchorTransform.scale(bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0);

                // apply object's other transforms
                aAnchorTransform = basegfx::tools::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                    * aAnchorTransform;

                if(rText.isFitToSize())
                {
                    // streched text in range
                    pNew = new SdrStretchTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aAnchorTransform,
                        rText.isFixedCellHeight());
                }
                else if(rText.isAutoFit())
                {
                    // isotrophically scaled text in range
                    pNew = new SdrAutoFitTextPrimitive2D(&rText.getSdrText(), rText.getOutlinerParaObject(), aAnchorTransform, bWordWrap);
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
                        bWordWrap,
                        bClipOnBounds);
                }
            }

            OSL_ENSURE(pNew != 0, "createTextPrimitive: no text primitive created (!)");

            if(rText.isBlink())
            {
                // prepare animation and primitive list
                drawinglayer::animation::AnimationEntryList aAnimationList;
                rText.getBlinkTextTiming(aAnimationList);

                if(0.0 != aAnimationList.getDuration())
                {
                    // create content sequence
                    const Primitive2DReference xRefA(pNew);
                    const Primitive2DSequence aContent(&xRefA, 1L);

                    // create and add animated switch primitive
                    return Primitive2DReference(new AnimatedBlinkPrimitive2D(aAnimationList, aContent, true));
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
                    const bool bHorizontal(SDRTEXTANI_LEFT == eDirection || SDRTEXTANI_RIGHT == eDirection);

                    // decompose to get separated values for the scroll box
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    aAnchorTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    // build transform from scaled only to full AnchorTransform and inverse
                    const basegfx::B2DHomMatrix aSRT(basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
                        fShearX, fRotate, aTranslate));
                    basegfx::B2DHomMatrix aISRT(aSRT);
                    aISRT.invert();

                    // bring the primitive back to scaled only and get scaled range, create new clone for this
                    SdrTextPrimitive2D* pNew2 = pNew->createTransformedClone(aISRT);
                    OSL_ENSURE(pNew2, "createTextPrimitive: Could not create transformed clone of text primitive (!)");
                    delete pNew;
                    pNew = pNew2;

                    // create neutral geometry::ViewInformation2D for local range and decompose calls. This is okay
                    // since the decompose is view-independent
                    const uno::Sequence< beans::PropertyValue > xViewParameters;
                    geometry::ViewInformation2D aViewInformation2D(xViewParameters);

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
                        // create a new Primitive2DSequence containing the animated text in it's scaled only state.
                        // use the decomposition to force to simple text primitives, those will no longer
                        // need the outliner for formatting (alternatively it is also possible to just add
                        // pNew to aNewPrimitiveSequence)
                        Primitive2DSequence aAnimSequence(pNew->get2DDecomposition(aViewInformation2D));
                        delete pNew;

                        // create a new animatedInterpolatePrimitive and add it
                        std::vector< basegfx::B2DHomMatrix > aMatrixStack;
                        aMatrixStack.push_back(aLeft);
                        aMatrixStack.push_back(aRight);
                        const Primitive2DReference xRefA(new AnimatedInterpolatePrimitive2D(aMatrixStack, aAnimationList, aAnimSequence, true));
                        const Primitive2DSequence aContent(&xRefA, 1L);

                        // scrolling needs an encapsulating clipping primitive
                        const basegfx::B2DRange aClipRange(aClipTopLeft, aClipBottomRight);
                        basegfx::B2DPolygon aClipPolygon(basegfx::tools::createPolygonFromRect(aClipRange));
                        aClipPolygon.transform(aSRT);
                        return Primitive2DReference(new MaskPrimitive2D(basegfx::B2DPolyPolygon(aClipPolygon), aContent));
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
                const Primitive2DReference xRefA(pNew);
                const Primitive2DSequence aContent(&xRefA, 1L);

                // create and add TextHierarchyEditPrimitive2D primitive
                return Primitive2DReference(new TextHierarchyEditPrimitive2D(aContent));
            }
            else
            {
                // add to decomposition
                return Primitive2DReference(pNew);
            }
        }

        Primitive2DSequence createEmbeddedShadowPrimitive(
            const Primitive2DSequence& rContent,
            const attribute::SdrShadowAttribute& rShadow)
        {
            if(rContent.hasElements())
            {
                Primitive2DSequence aRetval(2);
                basegfx::B2DHomMatrix aShadowOffset;

                // prepare shadow offset
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                // create shadow primitive and add content
                aRetval[0] = Primitive2DReference(
                    new ShadowPrimitive2D(
                        aShadowOffset,
                        rShadow.getColor(),
                        rContent));

                if(0.0 != rShadow.getTransparence())
                {
                    // create SimpleTransparencePrimitive2D
                    const Primitive2DSequence aTempContent(&aRetval[0], 1);

                    aRetval[0] = Primitive2DReference(
                        new UnifiedTransparencePrimitive2D(
                            aTempContent,
                            rShadow.getTransparence()));
                }

                aRetval[1] = Primitive2DReference(new GroupPrimitive2D(rContent));
                return aRetval;
            }
            else
            {
                return rContent;
            }
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
