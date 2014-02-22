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

#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>
#include <svx/xdef.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlinjoit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xdash.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrscit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflbckit.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/sdshcitm.hxx>
#include <svx/sdshtitm.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <svx/svdotext.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/xbtmpit.hxx>
#include <svl/itempool.hxx>
#include <vcl/svapp.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/svx3ditems.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <svx/rectenum.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/editstat.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <svx/sdr/attribute/sdrshadowtextattribute.hxx>
#include <svx/sdr/attribute/sdrlineshadowtextattribute.hxx>
#include <svx/sdr/attribute/sdrformtextattribute.hxx>
#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>
#include <svx/sdr/attribute/sdrfilltextattribute.hxx>
#include <com/sun/star/drawing/LineCap.hpp>



namespace drawinglayer
{
    namespace
    {
        attribute::GradientStyle XGradientStyleToGradientStyle(XGradientStyle eStyle)
        {
            switch(eStyle)
            {
                case XGRAD_LINEAR :
                {
                    return attribute::GRADIENTSTYLE_LINEAR;
                }
                case XGRAD_AXIAL :
                {
                    return attribute::GRADIENTSTYLE_AXIAL;
                }
                case XGRAD_RADIAL :
                {
                    return attribute::GRADIENTSTYLE_RADIAL;
                }
                case XGRAD_ELLIPTICAL :
                {
                    return attribute::GRADIENTSTYLE_ELLIPTICAL;
                }
                case XGRAD_SQUARE :
                {
                    return attribute::GRADIENTSTYLE_SQUARE;
                }
                default :
                {
                    return attribute::GRADIENTSTYLE_RECT; 
                }
            }
        }

        attribute::HatchStyle XHatchStyleToHatchStyle(XHatchStyle eStyle)
        {
            switch(eStyle)
            {
                case XHATCH_SINGLE :
                {
                    return attribute::HATCHSTYLE_SINGLE;
                }
                case XHATCH_DOUBLE :
                {
                    return attribute::HATCHSTYLE_DOUBLE;
                }
                default :
                {
                    return attribute::HATCHSTYLE_TRIPLE; 
                }
            }
        }

        basegfx::B2DLineJoin LineJointToB2DLineJoin(com::sun::star::drawing::LineJoint eLineJoint)
        {
            switch(eLineJoint)
            {
                case com::sun::star::drawing::LineJoint_MIDDLE :
                {
                    return basegfx::B2DLINEJOIN_MIDDLE;
                }
                case com::sun::star::drawing::LineJoint_BEVEL :
                {
                    return basegfx::B2DLINEJOIN_BEVEL;
                }
                case com::sun::star::drawing::LineJoint_MITER :
                {
                    return basegfx::B2DLINEJOIN_MITER;
                }
                case com::sun::star::drawing::LineJoint_ROUND :
                {
                    return basegfx::B2DLINEJOIN_ROUND;
                }
                default : 
                {
                    return basegfx::B2DLINEJOIN_NONE;
                }
            }
        }

        basegfx::B2DVector RectPointToB2DVector(RECT_POINT eRectPoint)
        {
            basegfx::B2DVector aRetval(0.0, 0.0);

            
            switch(eRectPoint)
            {
                case RP_LT: case RP_LM: case RP_LB:
                {
                    aRetval.setX(-1.0);
                    break;
                }

                case RP_RT: case RP_RM: case RP_RB:
                {
                    aRetval.setX(1.0);
                    break;
                }

                default :
                {
                    break;
                }
            }

            
            switch(eRectPoint)
            {
                case RP_LT: case RP_MT: case RP_RT:
                {
                    aRetval.setY(-1.0);
                    break;
                }

                case RP_LB: case RP_MB: case RP_RB:
                {
                    aRetval.setY(1.0);
                    break;
                }

                default :
                {
                    break;
                }
            }

            return aRetval;
        }
    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        attribute::SdrLineAttribute createNewSdrLineAttribute(const SfxItemSet& rSet)
        {
            const XLineStyle eStyle(((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue());

            if(XLINE_NONE != eStyle)
            {
                sal_uInt16 nTransparence(((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(100 != nTransparence)
                {
                    const sal_uInt32 nWidth(((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue());
                    const Color aColor(((const XLineColorItem&)(rSet.Get(XATTR_LINECOLOR))).GetColorValue());
                    const com::sun::star::drawing::LineJoint eJoint(((const XLineJointItem&)(rSet.Get(XATTR_LINEJOINT))).GetValue());
                    const com::sun::star::drawing::LineCap eCap(((const XLineCapItem&)(rSet.Get(XATTR_LINECAP))).GetValue());
                    ::std::vector< double > aDotDashArray;
                    double fFullDotDashLen(0.0);

                    if(XLINE_DASH == eStyle)
                    {
                        const XDash& rDash = ((const XLineDashItem&)(rSet.Get(XATTR_LINEDASH))).GetDashValue();

                        if(rDash.GetDots() || rDash.GetDashes())
                        {
                            fFullDotDashLen = rDash.CreateDotDashArray(aDotDashArray, (double)nWidth);
                        }
                    }

                    return attribute::SdrLineAttribute(
                        LineJointToB2DLineJoin(eJoint),
                        (double)nWidth,
                        (double)nTransparence * 0.01,
                        aColor.getBColor(),
                        eCap,
                        aDotDashArray,
                        fFullDotDashLen);
                }
            }

            return attribute::SdrLineAttribute();
        }

        attribute::SdrLineStartEndAttribute createNewSdrLineStartEndAttribute(
            const SfxItemSet& rSet,
            double fWidth)
        {
            const sal_Int32 nTempStartWidth(((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue());
            const sal_Int32 nTempEndWidth(((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue());
            basegfx::B2DPolyPolygon aStartPolyPolygon;
            basegfx::B2DPolyPolygon aEndPolyPolygon;
            double fStartWidth(0.0);
            double fEndWidth(0.0);
            bool bStartActive(false);
            bool bEndActive(false);
            bool bStartCentered(true);
            bool bEndCentered(true);

            if(nTempStartWidth)
            {
                if(nTempStartWidth < 0L)
                {
                    fStartWidth = ((double)(-nTempStartWidth) * fWidth) * 0.01;
                }
                else
                {
                    fStartWidth = (double)nTempStartWidth;
                }

                if(0.0 != fStartWidth)
                {
                    aStartPolyPolygon = basegfx::B2DPolyPolygon(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetLineStartValue());

                    if(aStartPolyPolygon.count() && aStartPolyPolygon.getB2DPolygon(0L).count())
                    {
                        bStartActive = true;
                        bStartCentered = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
                    }
                }
            }

            if(nTempEndWidth)
            {
                if(nTempEndWidth < 0L)
                {
                    fEndWidth = ((double)(-nTempEndWidth) * fWidth) * 0.01;
                }
                else
                {
                    fEndWidth = (double)nTempEndWidth;
                }

                if(0.0 != fEndWidth)
                {
                    aEndPolyPolygon = basegfx::B2DPolyPolygon(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetLineEndValue());

                    if(aEndPolyPolygon.count() && aEndPolyPolygon.getB2DPolygon(0L).count())
                    {
                        bEndActive = true;
                        bEndCentered = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
                    }
                }
            }

            if(bStartActive || bEndActive)
            {
                return attribute::SdrLineStartEndAttribute(
                    aStartPolyPolygon, aEndPolyPolygon, fStartWidth, fEndWidth,
                    bStartActive, bEndActive, bStartCentered, bEndCentered);
            }

            return attribute::SdrLineStartEndAttribute();
        }

        attribute::SdrShadowAttribute createNewSdrShadowAttribute(const SfxItemSet& rSet)
        {
            const bool bShadow(((SdrShadowItem&)rSet.Get(SDRATTR_SHADOW)).GetValue());

            if(bShadow)
            {
                sal_uInt16 nTransparence(((SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(nTransparence)
                {
                    sal_uInt16 nFillTransparence(((const XFillTransparenceItem&)(rSet.Get(XATTR_FILLTRANSPARENCE))).GetValue());

                    if(nFillTransparence > 100)
                    {
                        nFillTransparence = 100;
                    }

                    if(nTransparence == nFillTransparence)
                    {
                        
                        
                        
                        
                        nTransparence = 0;
                    }
                }

                if(100 != nTransparence)
                {
                    const basegfx::B2DVector aOffset(
                        (double)((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue(),
                        (double)((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue());
                    const Color aColor(((SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR))).GetColorValue());

                    return attribute::SdrShadowAttribute(aOffset, (double)nTransparence * 0.01, aColor.getBColor());
                }
            }

            return attribute::SdrShadowAttribute();
        }

        attribute::SdrFillAttribute createNewSdrFillAttribute(const SfxItemSet& rSet)
        {
            const XFillStyle eStyle(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue());

            if(XFILL_NONE != eStyle)
            {
                sal_uInt16 nTransparence(((const XFillTransparenceItem&)(rSet.Get(XATTR_FILLTRANSPARENCE))).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(100 != nTransparence)
                {
                    
                    const SfxPoolItem* pGradientItem;

                    if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, true, &pGradientItem)
                        && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled())
                    {
                        const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetGradientValue();
                        const sal_uInt8 nStartLuminance(rGradient.GetStartColor().GetLuminance());
                        const sal_uInt8 nEndLuminance(rGradient.GetEndColor().GetLuminance());
                        const bool bCompletelyTransparent(0xff == nStartLuminance && 0xff == nEndLuminance);

                        if(bCompletelyTransparent)
                        {
                            nTransparence = 100;
                        }
                    }
                }

                if(100 != nTransparence)
                {
                    const Color aColor(((const XFillColorItem&)(rSet.Get(XATTR_FILLCOLOR))).GetColorValue());
                    attribute::FillGradientAttribute aGradient;
                    attribute::FillHatchAttribute aHatch;
                    attribute::SdrFillGraphicAttribute aFillGraphic;

                    switch(eStyle)
                    {
                        case XFILL_NONE : 
                        case XFILL_SOLID :
                        {
                            
                            break;
                        }
                        case XFILL_GRADIENT :
                        {
                            XGradient aXGradient(((XFillGradientItem&)(rSet.Get(XATTR_FILLGRADIENT))).GetGradientValue());

                            const Color aStartColor(aXGradient.GetStartColor());
                            const sal_uInt16 nStartIntens(aXGradient.GetStartIntens());
                            basegfx::BColor aStart(aStartColor.getBColor());

                            if(nStartIntens != 100)
                            {
                                const basegfx::BColor aBlack;
                                aStart = interpolate(aBlack, aStart, (double)nStartIntens * 0.01);
                            }

                            const Color aEndColor(aXGradient.GetEndColor());
                            const sal_uInt16 nEndIntens(aXGradient.GetEndIntens());
                            basegfx::BColor aEnd(aEndColor.getBColor());

                            if(nEndIntens != 100)
                            {
                                const basegfx::BColor aBlack;
                                aEnd = interpolate(aBlack, aEnd, (double)nEndIntens * 0.01);
                            }

                            aGradient = attribute::FillGradientAttribute(
                                XGradientStyleToGradientStyle(aXGradient.GetGradientStyle()),
                                (double)aXGradient.GetBorder() * 0.01,
                                (double)aXGradient.GetXOffset() * 0.01,
                                (double)aXGradient.GetYOffset() * 0.01,
                                (double)aXGradient.GetAngle() * F_PI1800,
                                aStart,
                                aEnd,
                                ((const XGradientStepCountItem&)rSet.Get(XATTR_GRADIENTSTEPCOUNT)).GetValue());

                            break;
                        }
                        case XFILL_HATCH :
                        {
                            const XHatch& rHatch(((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetHatchValue());
                            const Color aColorB(rHatch.GetColor());

                            aHatch = attribute::FillHatchAttribute(
                                XHatchStyleToHatchStyle(rHatch.GetHatchStyle()),
                                (double)rHatch.GetDistance(),
                                (double)rHatch.GetAngle() * F_PI1800,
                                aColorB.getBColor(),
                                3, 
                                ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue());

                            break;
                        }
                        case XFILL_BITMAP :
                        {
                            aFillGraphic = createNewSdrFillGraphicAttribute(rSet);
                            break;
                        }
                    }

                    return attribute::SdrFillAttribute(
                        (double)nTransparence * 0.01,
                        aColor.getBColor(),
                        aGradient,
                        aHatch,
                        aFillGraphic);
                }
            }

            return attribute::SdrFillAttribute();
        }

        
        attribute::SdrTextAttribute createNewSdrTextAttribute(
            const SfxItemSet& rSet,
            const SdrText& rText,
            const sal_Int32* pLeft,
            const sal_Int32* pUpper,
            const sal_Int32* pRight,
            const sal_Int32* pLower)
        {
            const SdrTextObj& rTextObj = rText.GetObject();

            if(rText.GetOutlinerParaObject() && rText.GetModel())
            {
                
                bool bInEditMode(false);

                if(rText.GetObject().getTextCount() > 1)
                {
                    bInEditMode = rTextObj.IsInEditMode() && rText.GetObject().getActiveText() == &rText;
                }
                else
                {
                    bInEditMode = rTextObj.IsInEditMode();
                }

                OutlinerParaObject aOutlinerParaObject(*rText.GetOutlinerParaObject());

                if(bInEditMode)
                {
                    OutlinerParaObject* pTempObj = rTextObj.GetEditOutlinerParaObject();

                    if(pTempObj)
                    {
                        aOutlinerParaObject = *pTempObj;
                        delete pTempObj;
                    }
                    else
                    {
                        
                        
                        
                        
                    }
                }

                const SdrTextAniKind eAniKind(rTextObj.GetTextAniKind());

                
                const SdrOutliner& rDrawTextOutliner = rText.GetModel()->GetDrawOutliner(&rTextObj);
                const bool bWrongSpell(rDrawTextOutliner.GetControlWord() & EE_CNTRL_ONLINESPELLING);

                return attribute::SdrTextAttribute(
                    rText,
                    aOutlinerParaObject,
                    ((const XFormTextStyleItem&)rSet.Get(XATTR_FORMTXTSTYLE)).GetValue(),
                    pLeft ? *pLeft : rTextObj.GetTextLeftDistance(),
                    pUpper ? *pUpper : rTextObj.GetTextUpperDistance(),
                    pRight ? *pRight : rTextObj.GetTextRightDistance(),
                    pLower ? *pLower : rTextObj.GetTextLowerDistance(),
                    rTextObj.GetTextHorizontalAdjust(rSet),
                    rTextObj.GetTextVerticalAdjust(rSet),
                    ((const SdrTextContourFrameItem&)rSet.Get(SDRATTR_TEXT_CONTOURFRAME)).GetValue(),
                    rTextObj.IsFitToSize(),
                    rTextObj.IsAutoFit(),
                    ((const XFormTextHideFormItem&)rSet.Get(XATTR_FORMTXTHIDEFORM)).GetValue(),
                    SDRTEXTANI_BLINK == eAniKind,
                    SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind,
                    bInEditMode,
                    ((const SdrTextFixedCellHeightItem&)rSet.Get(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue(),
                    bWrongSpell);
            }

            return attribute::SdrTextAttribute();
        }

        attribute::FillGradientAttribute createNewTransparenceGradientAttribute(const SfxItemSet& rSet)
        {
            const SfxPoolItem* pGradientItem;

            if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, true, &pGradientItem)
                && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled())
            {
                
                const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetGradientValue();
                const sal_uInt8 nStartLuminance(rGradient.GetStartColor().GetLuminance());
                const sal_uInt8 nEndLuminance(rGradient.GetEndColor().GetLuminance());
                const bool bCompletelyTransparent(0xff == nStartLuminance && 0xff == nEndLuminance);
                const bool bNotTransparent(0x00 == nStartLuminance && 0x00 == nEndLuminance);

                
                
                
                
                if(!bNotTransparent && !bCompletelyTransparent)
                {
                    const double fStartLum(nStartLuminance / 255.0);
                    const double fEndLum(nEndLuminance / 255.0);

                    return attribute::FillGradientAttribute(
                        XGradientStyleToGradientStyle(rGradient.GetGradientStyle()),
                        (double)rGradient.GetBorder() * 0.01,
                        (double)rGradient.GetXOffset() * 0.01,
                        (double)rGradient.GetYOffset() * 0.01,
                        (double)rGradient.GetAngle() * F_PI1800,
                        basegfx::BColor(fStartLum, fStartLum, fStartLum),
                        basegfx::BColor(fEndLum, fEndLum, fEndLum),
                        0);
                }
            }

            return attribute::FillGradientAttribute();
        }

        attribute::SdrFillGraphicAttribute createNewSdrFillGraphicAttribute(const SfxItemSet& rSet)
        {
            Graphic aGraphic(((const XFillBitmapItem&)(rSet.Get(XATTR_FILLBITMAP))).GetGraphicObject().GetGraphic());

            if(!(GRAPHIC_BITMAP == aGraphic.GetType() || GRAPHIC_GDIMETAFILE == aGraphic.GetType()))
            {
                
                OSL_ENSURE(false, "No fill graphic in SfxItemSet (!)");
                return attribute::SdrFillGraphicAttribute();
            }

            Size aPrefSize(aGraphic.GetPrefSize());

            if(!aPrefSize.Width() || !aPrefSize.Height())
            {
                
                if(GRAPHIC_BITMAP == aGraphic.GetType())
                {
                    aGraphic.SetPrefSize(aGraphic.GetBitmapEx().GetSizePixel());
                    aGraphic.SetPrefMapMode(MAP_PIXEL);
                }
            }

            if(!aPrefSize.Width() || !aPrefSize.Height())
            {
                
                OSL_ENSURE(false, "Graphic has no size in SfxItemSet (!)");
                return attribute::SdrFillGraphicAttribute();
            }

            
            const MapUnit aDestinationMapUnit((MapUnit)rSet.GetPool()->GetMetric(0));
            basegfx::B2DVector aGraphicLogicSize(aGraphic.GetPrefSize().Width(), aGraphic.GetPrefSize().Height());

            if(aGraphic.GetPrefMapMode() != aDestinationMapUnit)
            {
                
                
                Size aNewSize(0, 0);

                if(MAP_PIXEL == aGraphic.GetPrefMapMode().GetMapUnit())
                {
                    aNewSize = Application::GetDefaultDevice()->PixelToLogic(
                        aGraphic.GetPrefSize(),
                        aDestinationMapUnit);
                }
                else
                {
                    aNewSize = OutputDevice::LogicToLogic(
                        aGraphic.GetPrefSize(),
                        aGraphic.GetPrefMapMode(),
                        aDestinationMapUnit);
                }

                
                
                aGraphicLogicSize = basegfx::B2DVector(aNewSize.Width(), aNewSize.Height());
            }

            
            const basegfx::B2DVector aSize(
                (double)((const SfxMetricItem&)(rSet.Get(XATTR_FILLBMP_SIZEX))).GetValue(),
                (double)((const SfxMetricItem&)(rSet.Get(XATTR_FILLBMP_SIZEY))).GetValue());
            const basegfx::B2DVector aOffset(
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_TILEOFFSETX))).GetValue(),
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_TILEOFFSETY))).GetValue());
            const basegfx::B2DVector aOffsetPosition(
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_POSOFFSETX))).GetValue(),
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_POSOFFSETY))).GetValue());

            return attribute::SdrFillGraphicAttribute(
                aGraphic,
                aGraphicLogicSize,
                aSize,
                aOffset,
                aOffsetPosition,
                RectPointToB2DVector((RECT_POINT)((const SfxEnumItem&)(rSet.Get(XATTR_FILLBMP_POS))).GetValue()),
                ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_TILE))).GetValue(),
                ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_STRETCH))).GetValue(),
                ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_SIZELOG))).GetValue());
        }

        attribute::SdrShadowTextAttribute createNewSdrShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText,
            bool bSuppressText)
        {
            attribute::SdrTextAttribute aText;

            
            
            if(!bSuppressText && pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText);
            }

            
            const attribute::SdrShadowAttribute aShadow(createNewSdrShadowAttribute(rSet));

            return attribute::SdrShadowTextAttribute(aShadow, aText);
        }

        attribute::SdrLineShadowTextAttribute createNewSdrLineShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText)
        {
            attribute::SdrLineAttribute aLine;
            attribute::SdrLineStartEndAttribute aLineStartEnd;
            attribute::SdrTextAttribute aText;
            bool bFontworkHideContour(false);

            
            if(pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText);

                
                
                if(!aText.isDefault()
                    && !aText.getSdrFormTextAttribute().isDefault()
                    && aText.isHideContour())
                {
                    bFontworkHideContour = true;
                }
            }

            
            if(!bFontworkHideContour)
            {
                aLine = createNewSdrLineAttribute(rSet);

                if(!aLine.isDefault())
                {
                    
                    aLineStartEnd = createNewSdrLineStartEndAttribute(rSet, aLine.getWidth());
                }
            }

            if(!aLine.isDefault() || !aText.isDefault())
            {
                
                const attribute::SdrShadowAttribute aShadow(createNewSdrShadowAttribute(rSet));

                return attribute::SdrLineShadowTextAttribute(aLine, aLineStartEnd, aShadow, aText);
            }

            return attribute::SdrLineShadowTextAttribute();
        }

        attribute::SdrLineFillShadowTextAttribute createNewSdrLineFillShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText,
            bool bHasContent)
        {
            attribute::SdrLineAttribute aLine;
            attribute::SdrFillAttribute aFill;
            attribute::SdrLineStartEndAttribute aLineStartEnd;
            attribute::SdrShadowAttribute aShadow;
            attribute::FillGradientAttribute aFillFloatTransGradient;
            attribute::SdrTextAttribute aText;
            bool bFontworkHideContour(false);

            
            if(pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText);

                
                
                if(!aText.getSdrFormTextAttribute().isDefault() && aText.isHideContour())
                {
                    bFontworkHideContour = true;
                }
            }

            if(!bFontworkHideContour)
            {
                
                aLine = createNewSdrLineAttribute(rSet);

                if(!aLine.isDefault())
                {
                    
                    aLineStartEnd = createNewSdrLineStartEndAttribute(rSet, aLine.getWidth());
                }

                
                aFill = createNewSdrFillAttribute(rSet);

                if(!aFill.isDefault())
                {
                    
                    aFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            
            
            
            
            if(bHasContent || !aLine.isDefault() || !aFill.isDefault() || !aText.isDefault())
            {
                
                aShadow = createNewSdrShadowAttribute(rSet);

                return attribute::SdrLineFillShadowTextAttribute(
                    aLine, aFill, aLineStartEnd, aShadow, aFillFloatTransGradient, aText);
            }

            return attribute::SdrLineFillShadowTextAttribute();
        }

        attribute::SdrLineFillShadowAttribute3D createNewSdrLineFillShadowAttribute(const SfxItemSet& rSet, bool bSuppressFill)
        {
            attribute::SdrFillAttribute aFill;
            attribute::SdrLineStartEndAttribute aLineStartEnd;
            attribute::SdrShadowAttribute aShadow;
            attribute::FillGradientAttribute aFillFloatTransGradient;

            
            const attribute::SdrLineAttribute aLine(createNewSdrLineAttribute(rSet));

            if(!aLine.isDefault())
            {
                
                aLineStartEnd = createNewSdrLineStartEndAttribute(rSet, aLine.getWidth());
            }

            
            if(!bSuppressFill)
            {
                aFill = createNewSdrFillAttribute(rSet);

                if(!aFill.isDefault())
                {
                    
                    aFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            if(!aLine.isDefault() || !aFill.isDefault())
            {
                
                aShadow = createNewSdrShadowAttribute(rSet);

                return attribute::SdrLineFillShadowAttribute3D(
                    aLine, aFill, aLineStartEnd, aShadow, aFillFloatTransGradient);
            }

            return attribute::SdrLineFillShadowAttribute3D();
        }

        attribute::SdrSceneAttribute createNewSdrSceneAttribute(const SfxItemSet& rSet)
        {
            
            ::com::sun::star::drawing::ProjectionMode aProjectionMode(::com::sun::star::drawing::ProjectionMode_PARALLEL);
            const sal_uInt16 nProjectionValue(((const Svx3DPerspectiveItem&)rSet.Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue());

            if(1L == nProjectionValue)
            {
                aProjectionMode = ::com::sun::star::drawing::ProjectionMode_PERSPECTIVE;
            }

            
            const double fDistance(((const Svx3DDistanceItem&)rSet.Get(SDRATTR_3DSCENE_DISTANCE)).GetValue());

            
            const double fShadowSlant(F_PI180 * ((const Svx3DShadowSlantItem&)rSet.Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue());

            
            ::com::sun::star::drawing::ShadeMode aShadeMode(::com::sun::star::drawing::ShadeMode_FLAT);
            const sal_uInt16 nShadeValue(((const Svx3DShadeModeItem&)rSet.Get(SDRATTR_3DSCENE_SHADE_MODE)).GetValue());

            if(1L == nShadeValue)
            {
                aShadeMode = ::com::sun::star::drawing::ShadeMode_PHONG;
            }
            else if(2L == nShadeValue)
            {
                aShadeMode = ::com::sun::star::drawing::ShadeMode_SMOOTH;
            }
            else if(3L == nShadeValue)
            {
                aShadeMode = ::com::sun::star::drawing::ShadeMode_DRAFT;
            }

            
            const bool bTwoSidedLighting(((const Svx3DTwoSidedLightingItem&)rSet.Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue());

            return attribute::SdrSceneAttribute(fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting);
        }

        attribute::SdrLightingAttribute createNewSdrLightingAttribute(const SfxItemSet& rSet)
        {
            
            ::std::vector< attribute::Sdr3DLightAttribute > aLightVector;

            if(((const Svx3DLightOnOff1Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor1Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection1Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, true));
            }

            if(((const Svx3DLightOnOff2Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor2Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection2Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(((const Svx3DLightOnOff3Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor3Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection3Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(((const Svx3DLightOnOff4Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor4Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection4Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(((const Svx3DLightOnOff5Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor5Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection5Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(((const Svx3DLightOnOff6Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor6Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection6Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(((const Svx3DLightOnOff7Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor7Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection7Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(((const Svx3DLightOnOff8Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue())
            {
                const basegfx::BColor aColor(((const Svx3DLightcolor8Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(((const Svx3DLightDirection8Item&)rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            
            const Color aAmbientValue(((const Svx3DAmbientcolorItem&)rSet.Get(SDRATTR_3DSCENE_AMBIENTCOLOR)).GetValue());
            const basegfx::BColor aAmbientLight(aAmbientValue.getBColor());

            return attribute::SdrLightingAttribute(aAmbientLight, aLightVector);
        }

        void calculateRelativeCornerRadius(sal_Int32 nRadius, const basegfx::B2DRange& rObjectRange, double& rfCornerRadiusX, double& rfCornerRadiusY)
        {
            rfCornerRadiusX = rfCornerRadiusY = (double)nRadius;

            if(0.0 != rfCornerRadiusX)
            {
                const double fHalfObjectWidth(rObjectRange.getWidth() * 0.5);

                if(0.0 != fHalfObjectWidth)
                {
                    if(rfCornerRadiusX < 0.0)
                    {
                        rfCornerRadiusX = 0.0;
                    }

                    if(rfCornerRadiusX > fHalfObjectWidth)
                    {
                        rfCornerRadiusX = fHalfObjectWidth;
                    }

                    rfCornerRadiusX /= fHalfObjectWidth;
                }
                else
                {
                    rfCornerRadiusX = 0.0;
                }
            }

            if(0.0 != rfCornerRadiusY)
            {
                const double fHalfObjectHeight(rObjectRange.getHeight() * 0.5);

                if(0.0 != fHalfObjectHeight)
                {
                    if(rfCornerRadiusY < 0.0)
                    {
                        rfCornerRadiusY = 0.0;
                    }

                    if(rfCornerRadiusY > fHalfObjectHeight)
                    {
                        rfCornerRadiusY = fHalfObjectHeight;
                    }

                    rfCornerRadiusY /= fHalfObjectHeight;
                }
                else
                {
                    rfCornerRadiusY = 0.0;
                }
            }
        }

        
        attribute::SdrFillTextAttribute createNewSdrFillTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText,
            const sal_Int32* pLeft,
            const sal_Int32* pUpper,
            const sal_Int32* pRight,
            const sal_Int32* pLower)
        {
            attribute::SdrFillAttribute aFill;
            attribute::FillGradientAttribute aFillFloatTransGradient;
            attribute::SdrTextAttribute aText;
            bool bFontworkHideContour(false);

            
            if(pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText, pLeft, pUpper, pRight, pLower);

                
                
                if(!aText.getSdrFormTextAttribute().isDefault() && aText.isHideContour())
                {
                    bFontworkHideContour = true;
                }
            }

            if(!bFontworkHideContour)
            {
                
                aFill = createNewSdrFillAttribute(rSet);

                if(!aFill.isDefault())
                {
                    
                    aFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            if(!aFill.isDefault() || !aText.isDefault())
            {
                return attribute::SdrFillTextAttribute(aFill, aFillFloatTransGradient, aText);
            }

            return attribute::SdrFillTextAttribute();
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
