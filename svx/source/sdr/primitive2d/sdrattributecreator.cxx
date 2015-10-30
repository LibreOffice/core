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
#include <sdr/attribute/sdrfilltextattribute.hxx>
#include <com/sun/star/drawing/LineCap.hpp>

using namespace com::sun::star;

namespace drawinglayer
{
    namespace
    {
        attribute::GradientStyle XGradientStyleToGradientStyle(css::awt::GradientStyle eStyle)
        {
            switch(eStyle)
            {
                case css::awt::GradientStyle_LINEAR :
                {
                    return attribute::GRADIENTSTYLE_LINEAR;
                }
                case css::awt::GradientStyle_AXIAL :
                {
                    return attribute::GRADIENTSTYLE_AXIAL;
                }
                case css::awt::GradientStyle_RADIAL :
                {
                    return attribute::GRADIENTSTYLE_RADIAL;
                }
                case css::awt::GradientStyle_ELLIPTICAL :
                {
                    return attribute::GRADIENTSTYLE_ELLIPTICAL;
                }
                case css::awt::GradientStyle_SQUARE :
                {
                    return attribute::GRADIENTSTYLE_SQUARE;
                }
                default :
                {
                    return attribute::GRADIENTSTYLE_RECT; // css::awt::GradientStyle_RECT
                }
            }
        }

        attribute::HatchStyle XHatchStyleToHatchStyle(css::drawing::HatchStyle eStyle)
        {
            switch(eStyle)
            {
                case css::drawing::HatchStyle_SINGLE :
                {
                    return attribute::HATCHSTYLE_SINGLE;
                }
                case css::drawing::HatchStyle_DOUBLE :
                {
                    return attribute::HATCHSTYLE_DOUBLE;
                }
                default :
                {
                    return attribute::HATCHSTYLE_TRIPLE; // css::drawing::HatchStyle_TRIPLE
                }
            }
        }

        basegfx::B2DLineJoin LineJointToB2DLineJoin(css::drawing::LineJoint eLineJoint)
        {
            switch(eLineJoint)
            {
                case css::drawing::LineJoint_MIDDLE :
                {
                    return basegfx::B2DLineJoin::Middle;
                }
                case css::drawing::LineJoint_BEVEL :
                {
                    return basegfx::B2DLineJoin::Bevel;
                }
                case css::drawing::LineJoint_MITER :
                {
                    return basegfx::B2DLineJoin::Miter;
                }
                case css::drawing::LineJoint_ROUND :
                {
                    return basegfx::B2DLineJoin::Round;
                }
                default : // css::drawing::LineJoint_NONE
                {
                    return basegfx::B2DLineJoin::NONE;
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



namespace drawinglayer
{
    namespace primitive2d
    {
        attribute::SdrLineAttribute createNewSdrLineAttribute(const SfxItemSet& rSet)
        {
            const css::drawing::LineStyle eStyle(static_cast<const XLineStyleItem&>(rSet.Get(XATTR_LINESTYLE)).GetValue());

            if(drawing::LineStyle_NONE != eStyle)
            {
                sal_uInt16 nTransparence(static_cast<const XLineTransparenceItem&>(rSet.Get(XATTR_LINETRANSPARENCE)).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(100 != nTransparence)
                {
                    const sal_uInt32 nWidth(static_cast<const XLineWidthItem&>(rSet.Get(XATTR_LINEWIDTH)).GetValue());
                    const Color aColor(static_cast<const XLineColorItem&>(rSet.Get(XATTR_LINECOLOR)).GetColorValue());
                    const css::drawing::LineJoint eJoint(static_cast<const XLineJointItem&>(rSet.Get(XATTR_LINEJOINT)).GetValue());
                    const css::drawing::LineCap eCap(static_cast<const XLineCapItem&>(rSet.Get(XATTR_LINECAP)).GetValue());
                    ::std::vector< double > aDotDashArray;
                    double fFullDotDashLen(0.0);

                    if(drawing::LineStyle_DASH == eStyle)
                    {
                        const XDash& rDash = static_cast<const XLineDashItem&>(rSet.Get(XATTR_LINEDASH)).GetDashValue();

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
            const sal_Int32 nTempStartWidth(static_cast<const XLineStartWidthItem&>(rSet.Get(XATTR_LINESTARTWIDTH)).GetValue());
            const sal_Int32 nTempEndWidth(static_cast<const XLineEndWidthItem&>(rSet.Get(XATTR_LINEENDWIDTH)).GetValue());
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
                    aStartPolyPolygon = basegfx::B2DPolyPolygon(static_cast<const XLineStartItem&>(rSet.Get(XATTR_LINESTART)).GetLineStartValue());

                    if(aStartPolyPolygon.count() && aStartPolyPolygon.getB2DPolygon(0L).count())
                    {
                        bStartActive = true;
                        bStartCentered = static_cast<const XLineStartCenterItem&>(rSet.Get(XATTR_LINESTARTCENTER)).GetValue();
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
                    aEndPolyPolygon = basegfx::B2DPolyPolygon(static_cast<const XLineEndItem&>(rSet.Get(XATTR_LINEEND)).GetLineEndValue());

                    if(aEndPolyPolygon.count() && aEndPolyPolygon.getB2DPolygon(0L).count())
                    {
                        bEndActive = true;
                        bEndCentered = static_cast<const XLineEndCenterItem&>(rSet.Get(XATTR_LINEENDCENTER)).GetValue();
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
            const bool bShadow(static_cast<const SdrOnOffItem&>(rSet.Get(SDRATTR_SHADOW)).GetValue());

            if(bShadow)
            {
                sal_uInt16 nTransparence(static_cast<const SdrPercentItem&>(rSet.Get(SDRATTR_SHADOWTRANSPARENCE)).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(nTransparence)
                {
                    sal_uInt16 nFillTransparence(static_cast<const XFillTransparenceItem&>(rSet.Get(XATTR_FILLTRANSPARENCE)).GetValue());

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
                        (double)static_cast<const SdrMetricItem&>(rSet.Get(SDRATTR_SHADOWXDIST)).GetValue(),
                        (double)static_cast<const SdrMetricItem&>(rSet.Get(SDRATTR_SHADOWYDIST)).GetValue());
                    const Color aColor(static_cast<const XColorItem&>(rSet.Get(SDRATTR_SHADOWCOLOR)).GetColorValue());

                    return attribute::SdrShadowAttribute(aOffset, (double)nTransparence * 0.01, aColor.getBColor());
                }
            }

            return attribute::SdrShadowAttribute();
        }

        attribute::SdrFillAttribute createNewSdrFillAttribute(const SfxItemSet& rSet)
        {
            const drawing::FillStyle eStyle(static_cast<const XFillStyleItem&>(rSet.Get(XATTR_FILLSTYLE)).GetValue());

            sal_uInt16 nTransparence(static_cast<const XFillTransparenceItem&>(rSet.Get(XATTR_FILLTRANSPARENCE)).GetValue());

            if(nTransparence > 100)
            {
                nTransparence = 100;
            }

            if(drawing::FillStyle_NONE != eStyle)
            {
                if(100 != nTransparence)
                {
                    // need to check XFillFloatTransparence, object fill may still be completely transparent
                    const SfxPoolItem* pGradientItem;

                    if(SfxItemState::SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, true, &pGradientItem)
                        && static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->IsEnabled())
                    {
                        const XGradient& rGradient = static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->GetGradientValue();
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
                    const Color aColor(static_cast<const XFillColorItem&>(rSet.Get(XATTR_FILLCOLOR)).GetColorValue());
                    attribute::FillGradientAttribute aGradient;
                    attribute::FillHatchAttribute aHatch;
                    attribute::SdrFillGraphicAttribute aFillGraphic;

                    switch(eStyle)
                    {
                        default:
                        {
                            // nothing to do, color is defined
                            break;
                        }
                        case drawing::FillStyle_GRADIENT :
                        {
                            XGradient aXGradient(static_cast<const XFillGradientItem&>(rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue());

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
                                static_cast<const XGradientStepCountItem&>(rSet.Get(XATTR_GRADIENTSTEPCOUNT)).GetValue());

                            break;
                        }
                        case drawing::FillStyle_HATCH :
                        {
                            const XHatch& rHatch(static_cast<const XFillHatchItem&>(rSet.Get(XATTR_FILLHATCH)).GetHatchValue());
                            const Color aColorB(rHatch.GetColor());

                            aHatch = attribute::FillHatchAttribute(
                                XHatchStyleToHatchStyle(rHatch.GetHatchStyle()),
                                (double)rHatch.GetDistance(),
                                (double)rHatch.GetAngle() * F_PI1800,
                                aColorB.getBColor(),
                                3, // same default as VCL, a minimum of three discrete units (pixels) offset
                                static_cast<const XFillBackgroundItem&>(rSet.Get(XATTR_FILLBACKGROUND)).GetValue());

                            break;
                        }
                        case drawing::FillStyle_BITMAP :
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

            if(nTransparence == 100)
            {
                attribute::FillGradientAttribute aGradient;
                attribute::FillHatchAttribute aHatch;
                attribute::SdrFillGraphicAttribute aFillGraphic;
                    return attribute::SdrFillAttribute(
                        1,
                        basegfx::BColor( 0, 0, 0 ),
                        aGradient,
                        aHatch,
                        aFillGraphic);
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

            // Save chaining attributes
            bool bToBeChained = rTextObj.IsToBeChained();
            bool bChainable = rTextObj.IsChainable();


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
                const bool bWrongSpell(rDrawTextOutliner.GetControlWord() & EEControlBits::ONLINESPELLING);

                return attribute::SdrTextAttribute(
                    rText,
                    aOutlinerParaObject,
                    static_cast<const XFormTextStyleItem&>(rSet.Get(XATTR_FORMTXTSTYLE)).GetValue(),
                    pLeft ? *pLeft : rTextObj.GetTextLeftDistance(),
                    pUpper ? *pUpper : rTextObj.GetTextUpperDistance(),
                    pRight ? *pRight : rTextObj.GetTextRightDistance(),
                    pLower ? *pLower : rTextObj.GetTextLowerDistance(),
                    rTextObj.GetTextHorizontalAdjust(rSet),
                    rTextObj.GetTextVerticalAdjust(rSet),
                    static_cast<const SdrOnOffItem&>(rSet.Get(SDRATTR_TEXT_CONTOURFRAME)).GetValue(),
                    rTextObj.IsFitToSize(),
                    rTextObj.IsAutoFit(),
                    static_cast<const XFormTextHideFormItem&>(rSet.Get(XATTR_FORMTXTHIDEFORM)).GetValue(),
                    SDRTEXTANI_BLINK == eAniKind,
                    SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind,
                    bInEditMode,
                    static_cast<const SdrTextFixedCellHeightItem&>(rSet.Get(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue(),
                    bWrongSpell,
                    bToBeChained,
                    bChainable);
            }

            return attribute::SdrTextAttribute();
        }

        attribute::FillGradientAttribute createNewTransparenceGradientAttribute(const SfxItemSet& rSet)
        {
            const SfxPoolItem* pGradientItem;

            if(SfxItemState::SET == rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, true, &pGradientItem)
                && static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->IsEnabled())
            {
                // test if float transparence is completely transparent
                const XGradient& rGradient = static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->GetGradientValue();
                const sal_uInt8 nStartLuminance(rGradient.GetStartColor().GetLuminance());
                const sal_uInt8 nEndLuminance(rGradient.GetEndColor().GetLuminance());
                const bool bCompletelyTransparent(0xff == nStartLuminance && 0xff == nEndLuminance);
                const bool bNotTransparent(0x00 == nStartLuminance && 0x00 == nEndLuminance);

                // create nothing when completely transparent: This case is already checked for the
                // normal fill attributes, XFILL_NONE will be used.
                // create nothing when not transparent: use normal fill, no need t create a FillGradientAttribute.
                // Both cases are optimizations, always creating FillGradientAttribute will work, too
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
            Graphic aGraphic(static_cast<const XFillBitmapItem&>(rSet.Get(XATTR_FILLBITMAP)).GetGraphicObject().GetGraphic());

            if(!(GRAPHIC_BITMAP == aGraphic.GetType() || GRAPHIC_GDIMETAFILE == aGraphic.GetType()))
            {
                // no content if not bitmap or metafile
                OSL_ENSURE(false, "No fill graphic in SfxItemSet (!)");
                return attribute::SdrFillGraphicAttribute();
            }

            Size aPrefSize(aGraphic.GetPrefSize());

            if(!aPrefSize.Width() || !aPrefSize.Height())
            {
                // if there is no logical size, create a size from pixel size and set MapMode accordingly
                if(GRAPHIC_BITMAP == aGraphic.GetType())
                {
                    aGraphic.SetPrefSize(aGraphic.GetBitmapEx().GetSizePixel());
                    aGraphic.SetPrefMapMode(MAP_PIXEL);
                }
            }

            if(!aPrefSize.Width() || !aPrefSize.Height())
            {
                // no content if no size
                OSL_ENSURE(false, "Graphic has no size in SfxItemSet (!)");
                return attribute::SdrFillGraphicAttribute();
            }

            // convert size and MapMode to destination logical size and MapMode
            const MapUnit aDestinationMapUnit((MapUnit)rSet.GetPool()->GetMetric(0));
            basegfx::B2DVector aGraphicLogicSize(aGraphic.GetPrefSize().Width(), aGraphic.GetPrefSize().Height());

            if(aGraphic.GetPrefMapMode() != aDestinationMapUnit)
            {
                // #i100360# for MAP_PIXEL, LogicToLogic will not work properly,
                // so fallback to Application::GetDefaultDevice()
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

                // #i124002# do not set new size using SetPrefSize at the graphic, this will lead to problems.
                // Instead, adapt the GraphicLogicSize which will be used for further decompositions
                aGraphicLogicSize = basegfx::B2DVector(aNewSize.Width(), aNewSize.Height());
            }

            // get size
            const basegfx::B2DVector aSize(
                (double) static_cast<const SfxMetricItem&>(rSet.Get(XATTR_FILLBMP_SIZEX)).GetValue(),
                (double) static_cast<const SfxMetricItem&>(rSet.Get(XATTR_FILLBMP_SIZEY)).GetValue());
            const basegfx::B2DVector aOffset(
                (double) static_cast<const SfxUInt16Item&>(rSet.Get(XATTR_FILLBMP_TILEOFFSETX)).GetValue(),
                (double) static_cast<const SfxUInt16Item&>(rSet.Get(XATTR_FILLBMP_TILEOFFSETY)).GetValue());
            const basegfx::B2DVector aOffsetPosition(
                (double) static_cast<const SfxUInt16Item&>(rSet.Get(XATTR_FILLBMP_POSOFFSETX)).GetValue(),
                (double) static_cast<const SfxUInt16Item&>(rSet.Get(XATTR_FILLBMP_POSOFFSETY)).GetValue());

            return attribute::SdrFillGraphicAttribute(
                aGraphic,
                aGraphicLogicSize,
                aSize,
                aOffset,
                aOffsetPosition,
                RectPointToB2DVector(
                    (RECT_POINT)static_cast<const SfxEnumItem&>(rSet.Get(XATTR_FILLBMP_POS)).GetValue()),
                    static_cast<const SfxBoolItem&>(rSet.Get(XATTR_FILLBMP_TILE)).GetValue(),
                    static_cast<const SfxBoolItem&>(rSet.Get(XATTR_FILLBMP_STRETCH)).GetValue(),
                    static_cast<const SfxBoolItem&>(rSet.Get(XATTR_FILLBMP_SIZELOG)).GetValue());
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

            // bHasContent is used from OLE and graphic objects. Normally a possible shadow
            // depends on line, fill or text to be set, but for these objects it is possible
            // to have none of these, but still content which needs to have a shadow (if set),
            // so shadow needs to be tried
            if(bHasContent || !aLine.isDefault() || !aFill.isDefault() || !aText.isDefault())
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
            css::drawing::ProjectionMode aProjectionMode(css::drawing::ProjectionMode_PARALLEL);
            const sal_uInt16 nProjectionValue(static_cast<const Svx3DPerspectiveItem&>(rSet.Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue());

            if(1L == nProjectionValue)
            {
                aProjectionMode = css::drawing::ProjectionMode_PERSPECTIVE;
            }

            // get distance
            const double fDistance(static_cast<const SfxUInt32Item&>(rSet.Get(SDRATTR_3DSCENE_DISTANCE)).GetValue());

            // get shadow slant
            const double fShadowSlant(F_PI180 * static_cast<const SfxUInt16Item&>(rSet.Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue());

            // get shade mode
            css::drawing::ShadeMode aShadeMode(css::drawing::ShadeMode_FLAT);
            const sal_uInt16 nShadeValue(static_cast<const Svx3DShadeModeItem&>(rSet.Get(SDRATTR_3DSCENE_SHADE_MODE)).GetValue());

            if(1L == nShadeValue)
            {
                aShadeMode = css::drawing::ShadeMode_PHONG;
            }
            else if(2L == nShadeValue)
            {
                aShadeMode = css::drawing::ShadeMode_SMOOTH;
            }
            else if(3L == nShadeValue)
            {
                aShadeMode = css::drawing::ShadeMode_DRAFT;
            }

            // get two sided lighting
            const bool bTwoSidedLighting(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue());

            return attribute::SdrSceneAttribute(fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting);
        }

        attribute::SdrLightingAttribute createNewSdrLightingAttribute(const SfxItemSet& rSet)
        {
            // extract lights from given SfxItemSet (from scene)
            ::std::vector< attribute::Sdr3DLightAttribute > aLightVector;

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, true));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            if(static_cast<const SfxBoolItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue())
            {
                const basegfx::BColor aColor(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue().getBColor());
                const basegfx::B3DVector aDirection(static_cast<const SvxB3DVectorItem&>(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue());
                aLightVector.push_back(attribute::Sdr3DLightAttribute(aColor, aDirection, false));
            }

            // get ambient color
            const Color aAmbientValue(static_cast<const SvxColorItem&>(rSet.Get(SDRATTR_3DSCENE_AMBIENTCOLOR)).GetValue());
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
