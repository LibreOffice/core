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

#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>
#include <svx/xdef.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlinjoit.hxx>
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
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <svx/svdotext.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
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

//////////////////////////////////////////////////////////////////////////////

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
                    return attribute::GRADIENTSTYLE_RECT; // XGRAD_RECT
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
                    return attribute::HATCHSTYLE_TRIPLE; // XHATCH_TRIPLE
                }
            }
        }

        basegfx::B2DLineJoin XLineJointtoB2DLineJoin(XLineJoint eLineJoint)
        {
            switch(eLineJoint)
            {
                case XLINEJOINT_MIDDLE :
                {
                    return basegfx::B2DLINEJOIN_MIDDLE;
                }
                case XLINEJOINT_BEVEL :
                {
                    return basegfx::B2DLINEJOIN_BEVEL;
                }
                case XLINEJOINT_MITER :
                {
                    return basegfx::B2DLINEJOIN_MITER;
                }
                case XLINEJOINT_ROUND :
                {
                    return basegfx::B2DLINEJOIN_ROUND;
                }
                default :
                {
                    return basegfx::B2DLINEJOIN_NONE; // XLINEJOINT_NONE
                }
            }
        }

        basegfx::B2DVector RectPointToB2DVector(RECT_POINT eRectPoint)
        {
            basegfx::B2DVector aRetval(0.0, 0.0);

            // position changes X
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

            // position changes Y
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
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

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
                    const XLineJoint eJoint(((const XLineJointItem&)(rSet.Get(XATTR_LINEJOINT))).GetValue());
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
                        XLineJointtoB2DLineJoin(eJoint),
                        (double)nWidth,
                        (double)nTransparence * 0.01,
                        aColor.getBColor(),
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
                        // shadow does not really have an own transparence, but the application
                        // sets the shadow transparence equal to the object transparence for
                        // convenience. This is not useful for primitive creation, so take
                        // this as no shadow transparence
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
                    const Color aColor(((const XFillColorItem&)(rSet.Get(XATTR_FILLCOLOR))).GetColorValue());
                    attribute::FillGradientAttribute aGradient;
                    attribute::FillHatchAttribute aHatch;
                    attribute::SdrFillBitmapAttribute aBitmap;

                    switch(eStyle)
                    {
                        case XFILL_NONE : // for warnings
                        case XFILL_SOLID :
                        {
                            // nothing to do, color is defined
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
                                ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue());

                            break;
                        }
                        case XFILL_BITMAP :
                        {
                            aBitmap = createNewSdrFillBitmapAttribute(rSet);
                            break;
                        }
                    }

                    return attribute::SdrFillAttribute(
                        (double)nTransparence * 0.01,
                        aColor.getBColor(),
                        aGradient,
                        aHatch,
                        aBitmap);
                }
            }

            return attribute::SdrFillAttribute();
        }

        // #i101508# Support handing over given text-to-border distances
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
                // added TextEdit text suppression
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
                        // #i100537#
                        // GetEditOutlinerParaObject() returning no object does not mean that
                        // text edit mode is not active. Do not reset the flag here
                        // bInEditMode = false;
                    }
                }

                const SdrTextAniKind eAniKind(rTextObj.GetTextAniKind());

                // #i107346#
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

            if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, sal_True, &pGradientItem)
                && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled())
            {
                // test if float transparence is completely transparent
                const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetGradientValue();
                const sal_uInt8 nStartLuminance(rGradient.GetStartColor().GetLuminance());
                const sal_uInt8 nEndLuminance(rGradient.GetEndColor().GetLuminance());
                const bool bCompletelyTransparent(0xff == nStartLuminance && 0xff == nEndLuminance);

                if(!bCompletelyTransparent)
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

        attribute::SdrFillBitmapAttribute createNewSdrFillBitmapAttribute(const SfxItemSet& rSet)
        {
            Bitmap aBitmap((((const XFillBitmapItem&)(rSet.Get(XATTR_FILLBITMAP))).GetBitmapValue()).GetBitmap());

            // make sure it's not empty, use default instead
            if(aBitmap.IsEmpty())
            {
                aBitmap = Bitmap(Size(4,4), 8);
            }

            // if there is no logical size, create a size from pixel size and set MapMode accordingly
            if(0L == aBitmap.GetPrefSize().Width() || 0L == aBitmap.GetPrefSize().Height())
            {
                aBitmap.SetPrefSize(aBitmap.GetSizePixel());
                aBitmap.SetPrefMapMode(MAP_PIXEL);
            }

            // convert size and MapMode to destination logical size and MapMode. The created
            // bitmap must have a valid logical size (PrefSize)
            const MapUnit aDestinationMapUnit((MapUnit)rSet.GetPool()->GetMetric(0));

            if(aBitmap.GetPrefMapMode() != aDestinationMapUnit)
            {
                // #i100360# for MAP_PIXEL, LogicToLogic will not work properly,
                // so fallback to Application::GetDefaultDevice()
                if(MAP_PIXEL == aBitmap.GetPrefMapMode().GetMapUnit())
                {
                    aBitmap.SetPrefSize(Application::GetDefaultDevice()->PixelToLogic(
                        aBitmap.GetPrefSize(), aDestinationMapUnit));
                }
                else
                {
                    aBitmap.SetPrefSize(OutputDevice::LogicToLogic(
                        aBitmap.GetPrefSize(), aBitmap.GetPrefMapMode(), aDestinationMapUnit));
                }
            }

            // get size
            const basegfx::B2DVector aSize(
                (double)((const SfxMetricItem&)(rSet.Get(XATTR_FILLBMP_SIZEX))).GetValue(),
                (double)((const SfxMetricItem&)(rSet.Get(XATTR_FILLBMP_SIZEY))).GetValue());
            const basegfx::B2DVector aOffset(
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_TILEOFFSETX))).GetValue(),
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_TILEOFFSETY))).GetValue());
            const basegfx::B2DVector aOffsetPosition(
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_POSOFFSETX))).GetValue(),
                (double)((const SfxUInt16Item&) (rSet.Get(XATTR_FILLBMP_POSOFFSETY))).GetValue());

            return attribute::SdrFillBitmapAttribute(
                aBitmap,
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

            // #i98072# added option to suppress text
            // look for text first
            if(!bSuppressText && pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText);
            }

            // try shadow
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

            // look for text first
            if(pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText);

                // when object has text and text is fontwork and hide contour is set for fontwork, force
                // line and fill style to empty
                if(!aText.isDefault()
                    && !aText.getSdrFormTextAttribute().isDefault()
                    && aText.isHideContour())
                {
                    bFontworkHideContour = true;
                }
            }

            // try line style
            if(!bFontworkHideContour)
            {
                aLine = createNewSdrLineAttribute(rSet);

                if(!aLine.isDefault())
                {
                    // try LineStartEnd
                    aLineStartEnd = createNewSdrLineStartEndAttribute(rSet, aLine.getWidth());
                }
            }

            if(!aLine.isDefault() || !aText.isDefault())
            {
                // try shadow
                const attribute::SdrShadowAttribute aShadow(createNewSdrShadowAttribute(rSet));

                return attribute::SdrLineShadowTextAttribute(aLine, aLineStartEnd, aShadow, aText);
            }

            return attribute::SdrLineShadowTextAttribute();
        }

        attribute::SdrLineFillShadowTextAttribute createNewSdrLineFillShadowTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText)
        {
            attribute::SdrLineAttribute aLine;
            attribute::SdrFillAttribute aFill;
            attribute::SdrLineStartEndAttribute aLineStartEnd;
            attribute::SdrShadowAttribute aShadow;
            attribute::FillGradientAttribute aFillFloatTransGradient;
            attribute::SdrTextAttribute aText;
            bool bFontworkHideContour(false);

            // look for text first
            if(pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText);

                // when object has text and text is fontwork and hide contour is set for fontwork, force
                // line and fill style to empty
                if(!aText.getSdrFormTextAttribute().isDefault() && aText.isHideContour())
                {
                    bFontworkHideContour = true;
                }
            }

            if(!bFontworkHideContour)
            {
                // try line style
                aLine = createNewSdrLineAttribute(rSet);

                if(!aLine.isDefault())
                {
                    // try LineStartEnd
                    aLineStartEnd = createNewSdrLineStartEndAttribute(rSet, aLine.getWidth());
                }

                // try fill style
                aFill = createNewSdrFillAttribute(rSet);

                if(!aFill.isDefault())
                {
                    // try fillfloattransparence
                    aFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            if(!aLine.isDefault() || !aFill.isDefault() || !aText.isDefault())
            {
                // try shadow
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

            // try line style
            const attribute::SdrLineAttribute aLine(createNewSdrLineAttribute(rSet));

            if(!aLine.isDefault())
            {
                // try LineStartEnd
                aLineStartEnd = createNewSdrLineStartEndAttribute(rSet, aLine.getWidth());
            }

            // try fill style
            if(!bSuppressFill)
            {
                aFill = createNewSdrFillAttribute(rSet);

                if(!aFill.isDefault())
                {
                    // try fillfloattransparence
                    aFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            if(!aLine.isDefault() || !aFill.isDefault())
            {
                // try shadow
                aShadow = createNewSdrShadowAttribute(rSet);

                return attribute::SdrLineFillShadowAttribute3D(
                    aLine, aFill, aLineStartEnd, aShadow, aFillFloatTransGradient);
            }

            return attribute::SdrLineFillShadowAttribute3D();
        }

        attribute::SdrSceneAttribute createNewSdrSceneAttribute(const SfxItemSet& rSet)
        {
            // get perspective
            ::com::sun::star::drawing::ProjectionMode aProjectionMode(::com::sun::star::drawing::ProjectionMode_PARALLEL);
            const sal_uInt16 nProjectionValue(((const Svx3DPerspectiveItem&)rSet.Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue());

            if(1L == nProjectionValue)
            {
                aProjectionMode = ::com::sun::star::drawing::ProjectionMode_PERSPECTIVE;
            }

            // get distance
            const double fDistance(((const Svx3DDistanceItem&)rSet.Get(SDRATTR_3DSCENE_DISTANCE)).GetValue());

            // get shadow slant
            const double fShadowSlant(F_PI180 * ((const Svx3DShadowSlantItem&)rSet.Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue());

            // get shade mode
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

            // get two sided lighting
            const bool bTwoSidedLighting(((const Svx3DTwoSidedLightingItem&)rSet.Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue());

            return attribute::SdrSceneAttribute(fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting);
        }

        attribute::SdrLightingAttribute createNewSdrLightingAttribute(const SfxItemSet& rSet)
        {
            // extract lights from given SfxItemSet (from scene)
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

            // get ambient color
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

        // #i101508# Support handing over given text-to-border distances
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

            // look for text first
            if(pText)
            {
                aText = createNewSdrTextAttribute(rSet, *pText, pLeft, pUpper, pRight, pLower);

                // when object has text and text is fontwork and hide contour is set for fontwork, force
                // fill style to empty
                if(!aText.getSdrFormTextAttribute().isDefault() && aText.isHideContour())
                {
                    bFontworkHideContour = true;
                }
            }

            if(!bFontworkHideContour)
            {
                // try fill style
                aFill = createNewSdrFillAttribute(rSet);

                if(!aFill.isDefault())
                {
                    // try fillfloattransparence
                    aFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            if(!aFill.isDefault() || !aText.isDefault())
            {
                return attribute::SdrFillTextAttribute(aFill, aFillFloatTransGradient, aText);
            }

            return attribute::SdrFillTextAttribute();
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
