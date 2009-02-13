/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrattributecreator.cxx,v $
 *
 * $Revision: 1.2.18.1 $
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

#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svtools/itemset.hxx>
#include <svx/xdef.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>
#include <xlinjoit.hxx>
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
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <svx/svdotext.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/attribute/sdrallattribute.hxx>
#include <svx/xbtmpit.hxx>
#include <svtools/itempool.hxx>
#include <vcl/svapp.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/svx3ditems.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <svx/rectenum.hxx>

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
        attribute::SdrLineAttribute* createNewSdrLineAttribute(const SfxItemSet& rSet)
        {
            attribute::SdrLineAttribute* pRetval(0L);
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

                    pRetval = new attribute::SdrLineAttribute(
                        XLineJointtoB2DLineJoin(eJoint),
                        (double)nWidth,
                        (double)nTransparence * 0.01,
                        aColor.getBColor(),
                        aDotDashArray,
                        fFullDotDashLen);
                }
            }

            return pRetval;
        }

        attribute::SdrLineStartEndAttribute* createNewSdrLineStartEndAttribute(const SfxItemSet& rSet, double fWidth)
        {
            attribute::SdrLineStartEndAttribute* pRetval(0L);
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
                pRetval = new attribute::SdrLineStartEndAttribute(aStartPolyPolygon, aEndPolyPolygon, fStartWidth, fEndWidth, bStartActive, bEndActive, bStartCentered, bEndCentered);
            }

            return pRetval;
        }

        attribute::SdrShadowAttribute* createNewSdrShadowAttribute(const SfxItemSet& rSet)
        {
            attribute::SdrShadowAttribute* pRetval(0L);
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

                    pRetval = new attribute::SdrShadowAttribute(aOffset, (double)nTransparence * 0.01, aColor.getBColor());
                }
            }

            return pRetval;
        }

        attribute::SdrFillAttribute* createNewSdrFillAttribute(const SfxItemSet& rSet)
        {
            attribute::SdrFillAttribute* pRetval(0L);
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
                    attribute::FillGradientAttribute* pGradient(0L);
                    attribute::FillHatchAttribute* pHatch(0L);
                    attribute::SdrFillBitmapAttribute* pBitmap(0L);

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
                            XGradient aGradient(((XFillGradientItem&)(rSet.Get(XATTR_FILLGRADIENT))).GetGradientValue());

                            const Color aStartColor(aGradient.GetStartColor());
                            const sal_uInt16 nStartIntens(aGradient.GetStartIntens());
                            basegfx::BColor aStart(aStartColor.getBColor());

                            if(nStartIntens != 100)
                            {
                                const basegfx::BColor aBlack;
                                aStart = interpolate(aBlack, aStart, (double)nStartIntens * 0.01);
                            }

                            const Color aEndColor(aGradient.GetEndColor());
                            const sal_uInt16 nEndIntens(aGradient.GetEndIntens());
                            basegfx::BColor aEnd(aEndColor.getBColor());

                            if(nEndIntens != 100)
                            {
                                const basegfx::BColor aBlack;
                                aEnd = interpolate(aBlack, aEnd, (double)nEndIntens * 0.01);
                            }

                            pGradient = new attribute::FillGradientAttribute(
                                XGradientStyleToGradientStyle(aGradient.GetGradientStyle()),
                                (double)aGradient.GetBorder() * 0.01,
                                (double)aGradient.GetXOffset() * 0.01,
                                (double)aGradient.GetYOffset() * 0.01,
                                (double)aGradient.GetAngle() * F_PI1800,
                                aStart,
                                aEnd,
                                ((const XGradientStepCountItem&)rSet.Get(XATTR_GRADIENTSTEPCOUNT)).GetValue());

                            break;
                        }
                        case XFILL_HATCH :
                        {
                            const XHatch& rHatch(((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetHatchValue());
                            const Color aColorB(rHatch.GetColor());

                            pHatch = new attribute::FillHatchAttribute(
                                XHatchStyleToHatchStyle(rHatch.GetHatchStyle()),
                                (double)rHatch.GetDistance(),
                                (double)rHatch.GetAngle() * F_PI1800,
                                aColorB.getBColor(),
                                ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue());

                            break;
                        }
                        case XFILL_BITMAP :
                        {
                            pBitmap = createNewSdrFillBitmapAttribute(rSet);
                            break;
                        }
                    }

                    pRetval = new attribute::SdrFillAttribute(
                        (double)nTransparence * 0.01,
                        aColor.getBColor(),
                        pGradient, pHatch, pBitmap);
                }
            }

            return pRetval;
        }

        attribute::SdrTextAttribute* createNewSdrTextAttribute(const SfxItemSet& rSet, const SdrText& rText)
        {
            attribute::SdrTextAttribute* pRetval(0L);
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

                if(!bInEditMode)
                {
                    const SdrFitToSizeType eFit = rTextObj.GetFitToSize();
                    const SdrTextAniKind eAniKind(rTextObj.GetTextAniKind());

                    pRetval = new attribute::SdrTextAttribute(
                        rText,
                        ((const XFormTextStyleItem&)rSet.Get(XATTR_FORMTXTSTYLE)).GetValue(),
                        rTextObj.GetTextLeftDistance(),
                        rTextObj.GetTextUpperDistance(),
                        rTextObj.GetTextRightDistance(),
                        rTextObj.GetTextLowerDistance(),
                        ((const SdrTextContourFrameItem&)rSet.Get(SDRATTR_TEXT_CONTOURFRAME)).GetValue(),
                        (SDRTEXTFIT_PROPORTIONAL == eFit || SDRTEXTFIT_ALLLINES == eFit),
                        ((const XFormTextHideFormItem&)rSet.Get(XATTR_FORMTXTHIDEFORM)).GetValue(),
                        SDRTEXTANI_BLINK == eAniKind,
                        SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind);
                }
            }

            return pRetval;
        }

        attribute::FillGradientAttribute* createNewTransparenceGradientAttribute(const SfxItemSet& rSet)
        {
            attribute::FillGradientAttribute* pRetval = 0L;
            const SfxPoolItem* pGradientItem;

            if(SFX_ITEM_SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, TRUE, &pGradientItem) && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled())
            {
                // test if float transparence is completely transparent
                const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetGradientValue();
                const sal_uInt8 nStartLuminance(rGradient.GetStartColor().GetLuminance());
                const sal_uInt8 nEndLuminance(rGradient.GetEndColor().GetLuminance());
                const bool bCompletelyTransparent(0xff == nStartLuminance == nEndLuminance);

                if(!bCompletelyTransparent)
                {
                    const double fStartLum(nStartLuminance / 255.0);
                    const double fEndLum(nEndLuminance / 255.0);

                    pRetval = new attribute::FillGradientAttribute(
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

            return pRetval;
        }

        attribute::SdrFillBitmapAttribute* createNewSdrFillBitmapAttribute(const SfxItemSet& rSet)
        {
            attribute::SdrFillBitmapAttribute* pRetval(0L);
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
                // #i96237# need to use LogicToLogic, source is not always pixels
                aBitmap.SetPrefSize(Application::GetDefaultDevice()->LogicToLogic(
                    aBitmap.GetPrefSize(), aBitmap.GetPrefMapMode(), aDestinationMapUnit));
                aBitmap.SetPrefMapMode(aDestinationMapUnit);
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

            pRetval = new attribute::SdrFillBitmapAttribute(
                aBitmap,
                aSize,
                aOffset,
                aOffsetPosition,
                RectPointToB2DVector((RECT_POINT)((const SfxEnumItem&)(rSet.Get(XATTR_FILLBMP_POS))).GetValue()),
                ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_TILE))).GetValue(),
                ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_STRETCH))).GetValue(),
                ((const SfxBoolItem&) (rSet.Get(XATTR_FILLBMP_SIZELOG))).GetValue());

            return pRetval;
        }

        attribute::SdrShadowTextAttribute* createNewSdrShadowTextAttribute(const SfxItemSet& rSet, const SdrText& rText, bool bSuppressText)
        {
            attribute::SdrShadowTextAttribute* pRetval(0L);
            attribute::SdrShadowAttribute* pShadow(0L);
            attribute::SdrTextAttribute* pText(0L);

            // #i98072# added option to suppress text
            // look for text first
            if(!bSuppressText)
            {
                pText = createNewSdrTextAttribute(rSet, rText);
            }

            // try shadow
            pShadow = createNewSdrShadowAttribute(rSet);

            if(pShadow && !pShadow->isVisible())
            {
                delete pShadow;
                pShadow = 0L;
            }

            if(pText || pShadow)
            {
                pRetval = new attribute::SdrShadowTextAttribute(pShadow, pText);
            }

            return pRetval;
        }

        attribute::SdrLineShadowTextAttribute* createNewSdrLineShadowTextAttribute(const SfxItemSet& rSet, const SdrText& rText)
        {
            attribute::SdrLineShadowTextAttribute* pRetval(0L);
            attribute::SdrLineAttribute* pLine(0L);
            attribute::SdrLineStartEndAttribute* pLineStartEnd(0L);
            attribute::SdrShadowAttribute* pShadow(0L);
            attribute::SdrTextAttribute* pText(0L);
            bool bFontworkHideContour(false);

            // look for text first
            pText = createNewSdrTextAttribute(rSet, rText);

            // when object has text and text is fontwork and hide contour is set for fontwork, force
            // line and fill style to empty
            if(pText && pText->isFontwork() && pText->isHideContour())
            {
                bFontworkHideContour = true;
            }

            // try line style
            if(!bFontworkHideContour)
            {
                pLine = createNewSdrLineAttribute(rSet);

                if(pLine && !pLine->isVisible())
                {
                    delete pLine;
                    pLine = 0L;
                }

                if(pLine)
                {
                    // try LineStartEnd
                    pLineStartEnd = createNewSdrLineStartEndAttribute(rSet, pLine->getWidth());

                    if(pLineStartEnd && !pLineStartEnd->isVisible())
                    {
                        delete pLineStartEnd;
                        pLineStartEnd = 0L;
                    }
                }
            }

            // try shadow
            if(pLine || pText)
            {
                pShadow = createNewSdrShadowAttribute(rSet);

                if(pShadow && !pShadow->isVisible())
                {
                    delete pShadow;
                    pShadow = 0L;
                }
            }

            if(pLine || pText)
            {
                pRetval = new attribute::SdrLineShadowTextAttribute(pLine, pLineStartEnd, pShadow, pText);
            }

            return pRetval;
        }

        attribute::SdrLineFillShadowTextAttribute* createNewSdrLineFillShadowTextAttribute(const SfxItemSet& rSet, const SdrText& rText)
        {
            attribute::SdrLineFillShadowTextAttribute* pRetval(0L);
            attribute::SdrLineAttribute* pLine(0L);
            attribute::SdrFillAttribute* pFill(0L);
            attribute::SdrLineStartEndAttribute* pLineStartEnd(0L);
            attribute::SdrShadowAttribute* pShadow(0L);
            attribute::FillGradientAttribute* pFillFloatTransGradient(0L);
            attribute::SdrTextAttribute* pText(0L);
            bool bFontworkHideContour(false);

            // look for text first
            pText = createNewSdrTextAttribute(rSet, rText);

            // when object has text and text is fontwork and hide contour is set for fontwork, force
            // line and fill style to empty
            if(pText && pText->isFontwork() && pText->isHideContour())
            {
                bFontworkHideContour = true;
            }

            // try line style
            if(!bFontworkHideContour)
            {
                pLine = createNewSdrLineAttribute(rSet);

                if(pLine && !pLine->isVisible())
                {
                    delete pLine;
                    pLine = 0L;
                }

                if(pLine)
                {
                    // try LineStartEnd
                    pLineStartEnd = createNewSdrLineStartEndAttribute(rSet, pLine->getWidth());

                    if(pLineStartEnd && !pLineStartEnd->isVisible())
                    {
                        delete pLineStartEnd;
                        pLineStartEnd = 0L;
                    }
                }
            }

            // try fill style
            if(!bFontworkHideContour)
            {
                pFill = createNewSdrFillAttribute(rSet);

                if(pFill && !pFill->isVisible())
                {
                    delete pFill;
                    pFill = 0L;
                }

                if(pFill)
                {
                    // try fillfloattransparence
                    pFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            // try shadow
            if(pLine || pFill || pText)
            {
                pShadow = createNewSdrShadowAttribute(rSet);

                if(pShadow && !pShadow->isVisible())
                {
                    delete pShadow;
                    pShadow = 0L;
                }
            }

            if(pLine || pFill || pText)
            {
                pRetval = new attribute::SdrLineFillShadowTextAttribute(pLine, pFill, pLineStartEnd, pShadow, pFillFloatTransGradient, pText);
            }

            return pRetval;
        }

        attribute::SdrLineFillShadowAttribute* createNewSdrLineFillShadowAttribute(const SfxItemSet& rSet, bool bSuppressFill)
        {
            attribute::SdrLineFillShadowAttribute* pRetval(0L);
            attribute::SdrLineAttribute* pLine(0L);
            attribute::SdrFillAttribute* pFill(0L);
            attribute::SdrLineStartEndAttribute* pLineStartEnd(0L);
            attribute::SdrShadowAttribute* pShadow(0L);
            attribute::FillGradientAttribute* pFillFloatTransGradient(0L);

            // try line style
            pLine = createNewSdrLineAttribute(rSet);

            if(pLine && !pLine->isVisible())
            {
                delete pLine;
                pLine = 0L;
            }

            if(pLine)
            {
                // try LineStartEnd
                pLineStartEnd = createNewSdrLineStartEndAttribute(rSet, pLine->getWidth());

                if(pLineStartEnd && !pLineStartEnd->isVisible())
                {
                    delete pLineStartEnd;
                    pLineStartEnd = 0L;
                }
            }

            // try fill style
            pFill = bSuppressFill ? 0 : createNewSdrFillAttribute(rSet);

            if(pFill && !pFill->isVisible())
            {
                delete pFill;
                pFill = 0L;
            }

            if(pFill)
            {
                // try fillfloattransparence
                pFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
            }

            // try shadow
            if(pLine || pFill)
            {
                pShadow = createNewSdrShadowAttribute(rSet);

                if(pShadow && !pShadow->isVisible())
                {
                    delete pShadow;
                    pShadow = 0L;
                }
            }

            if(pLine || pFill)
            {
                pRetval = new attribute::SdrLineFillShadowAttribute(pLine, pFill, pLineStartEnd, pShadow, pFillFloatTransGradient);
            }

            return pRetval;
        }

        attribute::SdrSceneAttribute* createNewSdrSceneAttribute(const SfxItemSet& rSet)
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

            return new attribute::SdrSceneAttribute(fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting);
        }

        attribute::SdrLightingAttribute* createNewSdrLightingAttribute(const SfxItemSet& rSet)
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

            return new attribute::SdrLightingAttribute(aAmbientLight, aLightVector);
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

        attribute::SdrFillTextAttribute* createNewSdrFillTextAttribute(const SfxItemSet& rSet, const SdrText* pSdrText)
        {
            attribute::SdrFillTextAttribute* pRetval(0L);
            attribute::SdrFillAttribute* pFill(0L);
            attribute::FillGradientAttribute* pFillFloatTransGradient(0L);
            attribute::SdrTextAttribute* pText(0L);
            bool bFontworkHideContour(false);

            // look for text first
            if(pSdrText)
            {
                pText = createNewSdrTextAttribute(rSet, *pSdrText);
            }

            // when object has text and text is fontwork and hide contour is set for fontwork, force
            // fill style to empty
            if(pText && pText->isFontwork() && pText->isHideContour())
            {
                bFontworkHideContour = true;
            }

            // try fill style
            if(!bFontworkHideContour)
            {
                pFill = createNewSdrFillAttribute(rSet);

                if(pFill && !pFill->isVisible())
                {
                    delete pFill;
                    pFill = 0L;
                }

                if(pFill)
                {
                    // try fillfloattransparence
                    pFillFloatTransGradient = createNewTransparenceGradientAttribute(rSet);
                }
            }

            if(pFill || pText)
            {
                pRetval = new attribute::SdrFillTextAttribute(pFill, pFillFloatTransGradient, pText);
            }

            return pRetval;
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
