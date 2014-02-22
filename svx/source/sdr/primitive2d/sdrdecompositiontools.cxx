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
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
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



using namespace com::sun::star;



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
            else if(!rFill.getFillGraphic().isDefault())
            {
                const basegfx::B2DRange aRange(basegfx::tools::getRange(aScaledPolyPolygon));
                pNewFillPrimitive = new PolyPolygonGraphicPrimitive2D(aScaledPolyPolygon, rFill.getFillGraphic().createFillGraphicAttribute(aRange));
            }
            else
            {
                pNewFillPrimitive = new PolyPolygonColorPrimitive2D(aScaledPolyPolygon, rFill.getColor());
            }

            if(0.0 != rFill.getTransparence())
            {
                
                const Primitive2DReference xRefA(pNewFillPrimitive);
                const Primitive2DSequence aContent(&xRefA, 1L);
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(aContent, rFill.getTransparence()));
            }
            else if(!rFillGradient.isDefault())
            {
                
                const Primitive2DReference xRefA(pNewFillPrimitive);
                const Primitive2DSequence aContent(&xRefA, 1L);

                
                
                const basegfx::B2DRange aRange(basegfx::tools::getRange(aScaledPolyPolygon));
                const Primitive2DReference xRefB(new FillGradientPrimitive2D(aRange, rFillGradient));
                const Primitive2DSequence aAlpha(&xRefB, 1L);

                
                return Primitive2DReference(new TransparencePrimitive2D(aContent, aAlpha));
            }
            else
            {
                
                return Primitive2DReference(pNewFillPrimitive);
            }
        }

        Primitive2DReference createPolygonLinePrimitive(
            const basegfx::B2DPolygon& rUnitPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine,
            const attribute::SdrLineStartEndAttribute& rStroke)
        {
            
            basegfx::B2DPolygon aScaledPolygon(rUnitPolygon);
            aScaledPolygon.transform(rObjectTransform);

            
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin(), rLine.getCap());
            const attribute::StrokeAttribute aStrokeAttribute(rLine.getDotDashArray(), rLine.getFullDotDashLen());
            BasePrimitive2D* pNewLinePrimitive = 0L;

            if(!rUnitPolygon.isClosed() && !rStroke.isDefault())
            {
                attribute::LineStartEndAttribute aStart(rStroke.getStartWidth(), rStroke.getStartPolyPolygon(), rStroke.isStartCentered());
                attribute::LineStartEndAttribute aEnd(rStroke.getEndWidth(), rStroke.getEndPolyPolygon(), rStroke.isEndCentered());

                
                pNewLinePrimitive = new PolygonStrokeArrowPrimitive2D(aScaledPolygon, aLineAttribute, aStrokeAttribute, aStart, aEnd);
            }
            else
            {
                
                pNewLinePrimitive = new PolygonStrokePrimitive2D(aScaledPolygon, aLineAttribute, aStrokeAttribute);
            }

            if(0.0 != rLine.getTransparence())
            {
                
                const Primitive2DReference xRefA(pNewLinePrimitive);
                const Primitive2DSequence aContent(&xRefA, 1L);
                return Primitive2DReference(new UnifiedTransparencePrimitive2D(aContent, rLine.getTransparence()));
            }
            else
            {
                
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
                
                if(!rStroke.isDefault() && 0.0 != rStroke.getWidth())
                {
                    
                    
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    
                    
                    basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                    aScaledUnitPolyPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(
                        fabs(aScale.getX()), fabs(aScale.getY())));

                    
                    aScaledUnitPolyPolygon = basegfx::tools::growInNormalDirection(aScaledUnitPolyPolygon, -(rStroke.getWidth() * 0.5));

                    
                    aScaledUnitPolyPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(
                        0.0 != aScale.getX() ? 1.0 / aScale.getX() : 1.0,
                        0.0 != aScale.getY() ? 1.0 / aScale.getY() : 1.0));

                    
                    pNew = new SdrContourTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aScaledUnitPolyPolygon,
                        rObjectTransform);
                }
                else
                {
                    
                    pNew = new SdrContourTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        rUnitPolyPolygon,
                        rObjectTransform);
                }
            }
            else if(!rText.getSdrFormTextAttribute().isDefault())
            {
                
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
                
                
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                rObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                
                const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
                const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));
                aScale = basegfx::absolute(aScale);

                
                
                basegfx::B2DHomMatrix aJustScaleTransform;
                aJustScaleTransform.set(0, 0, aScale.getX());
                aJustScaleTransform.set(1, 1, aScale.getY());
                basegfx::B2DPolyPolygon aScaledUnitPolyPolygon(rUnitPolyPolygon);
                aScaledUnitPolyPolygon.transform(aJustScaleTransform);
                const basegfx::B2DRange aSnapRange(basegfx::tools::getRange(aScaledUnitPolyPolygon));

                
                
                const basegfx::B2DPoint aTopLeft(aSnapRange.getMinX() + rText.getTextLeftDistance(), aSnapRange.getMinY() + rText.getTextUpperDistance());
                const basegfx::B2DPoint aBottomRight(aSnapRange.getMaxX() - rText.getTextRightDistance(), aSnapRange.getMaxY() - rText.getTextLowerDistance());
                basegfx::B2DRange aTextAnchorRange;
                aTextAnchorRange.expand(aTopLeft);
                aTextAnchorRange.expand(aBottomRight);

                
                
                
                aAnchorTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(
                    basegfx::fTools::equalZero(aTextAnchorRange.getWidth()) ? 1.0 : aTextAnchorRange.getWidth(),
                    basegfx::fTools::equalZero(aTextAnchorRange.getHeight()) ? 1.0 : aTextAnchorRange.getHeight(),
                    aTextAnchorRange.getMinX(), aTextAnchorRange.getMinY());

                
                aAnchorTransform.scale(bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0);

                
                aAnchorTransform = basegfx::tools::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                    * aAnchorTransform;

                if(rText.isFitToSize())
                {
                    
                    pNew = new SdrStretchTextPrimitive2D(
                        &rText.getSdrText(),
                        rText.getOutlinerParaObject(),
                        aAnchorTransform,
                        rText.isFixedCellHeight());
                }
                else if(rText.isAutoFit())
                {
                    
                    pNew = new SdrAutoFitTextPrimitive2D(&rText.getSdrText(), rText.getOutlinerParaObject(), aAnchorTransform, bWordWrap);
                }
                else 
                {
                    
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
                
                drawinglayer::animation::AnimationEntryList aAnimationList;
                rText.getBlinkTextTiming(aAnimationList);

                if(0.0 != aAnimationList.getDuration())
                {
                    
                    const Primitive2DReference xRefA(pNew);
                    const Primitive2DSequence aContent(&xRefA, 1L);

                    
                    return Primitive2DReference(new AnimatedBlinkPrimitive2D(aAnimationList, aContent, true));
                }
                else
                {
                    
                    return Primitive2DReference(pNew);
                }
            }

            if(rText.isScroll())
            {
                
                if(rText.getSdrFormTextAttribute().isDefault())
                {
                    
                    const SdrTextAniDirection eDirection(rText.getSdrText().GetObject().GetTextAniDirection());
                    const bool bHorizontal(SDRTEXTANI_LEFT == eDirection || SDRTEXTANI_RIGHT == eDirection);

                    
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    aAnchorTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                    
                    const basegfx::B2DHomMatrix aSRT(basegfx::tools::createShearXRotateTranslateB2DHomMatrix(
                        fShearX, fRotate, aTranslate));
                    basegfx::B2DHomMatrix aISRT(aSRT);
                    aISRT.invert();

                    
                    SdrTextPrimitive2D* pNew2 = pNew->createTransformedClone(aISRT);
                    OSL_ENSURE(pNew2, "createTextPrimitive: Could not create transformed clone of text primitive (!)");
                    delete pNew;
                    pNew = pNew2;

                    
                    
                    const uno::Sequence< beans::PropertyValue > xViewParameters;
                    geometry::ViewInformation2D aViewInformation2D(xViewParameters);

                    
                    const basegfx::B2DRange aScaledRange(pNew->getB2DRange(aViewInformation2D));

                    
                    
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
                        
                        
                        
                        
                        Primitive2DSequence aAnimSequence(pNew->get2DDecomposition(aViewInformation2D));
                        delete pNew;

                        
                        std::vector< basegfx::B2DHomMatrix > aMatrixStack;
                        aMatrixStack.push_back(aLeft);
                        aMatrixStack.push_back(aRight);
                        const Primitive2DReference xRefA(new AnimatedInterpolatePrimitive2D(aMatrixStack, aAnimationList, aAnimSequence, true));
                        const Primitive2DSequence aContent(&xRefA, 1L);

                        
                        const basegfx::B2DRange aClipRange(aClipTopLeft, aClipBottomRight);
                        basegfx::B2DPolygon aClipPolygon(basegfx::tools::createPolygonFromRect(aClipRange));
                        aClipPolygon.transform(aSRT);
                        return Primitive2DReference(new MaskPrimitive2D(basegfx::B2DPolyPolygon(aClipPolygon), aContent));
                    }
                    else
                    {
                        
                        return Primitive2DReference(pNew);
                    }
                }
            }

            if(rText.isInEditMode())
            {
                
                
                
                const Primitive2DReference xRefA(pNew);
                const Primitive2DSequence aContent(&xRefA, 1L);

                
                return Primitive2DReference(new TextHierarchyEditPrimitive2D(aContent));
            }
            else
            {
                
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

                
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                
                aRetval[0] = Primitive2DReference(
                    new ShadowPrimitive2D(
                        aShadowOffset,
                        rShadow.getColor(),
                        rContent));

                if(0.0 != rShadow.getTransparence())
                {
                    
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
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
