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

#include <sdr/primitive2d/sdrattributecreator.hxx>
#include <svl/itemset.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/xdef.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmpit.hxx>
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
#include <svx/xfilluseslidebackgrounditem.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xgrscit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xflboxy.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/RectangleAlignmentItem.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <svx/svdotext.hxx>
#include <sdr/attribute/sdrtextattribute.hxx>
#include <svx/xbtmpit.hxx>
#include <svl/itempool.hxx>
#include <vcl/svapp.hxx>
#include <vcl/GraphicLoader.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <svx/svx3ditems.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <drawinglayer/attribute/sdrallattribute3d.hxx>
#include <svx/rectenum.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xflbmsli.hxx>
#include <editeng/editstat.hxx>
#include <osl/diagnose.h>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <sdr/attribute/sdreffectstextattribute.hxx>
#include <sdr/attribute/sdrlineeffectstextattribute.hxx>
#include <sdr/attribute/sdrformtextattribute.hxx>
#include <sdr/attribute/sdrlinefilleffectstextattribute.hxx>
#include <drawinglayer/attribute/sdrglowattribute.hxx>
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
        attribute::HatchStyle XHatchStyleToHatchStyle(css::drawing::HatchStyle eStyle)
        {
            switch(eStyle)
            {
                case css::drawing::HatchStyle_SINGLE :
                {
                    return attribute::HatchStyle::Single;
                }
                case css::drawing::HatchStyle_DOUBLE :
                {
                    return attribute::HatchStyle::Double;
                }
                default :
                {
                    return attribute::HatchStyle::Triple; // css::drawing::HatchStyle_TRIPLE
                }
            }
        }

        basegfx::B2DLineJoin LineJointToB2DLineJoin(css::drawing::LineJoint eLineJoint)
        {
            switch(eLineJoint)
            {
                case css::drawing::LineJoint_BEVEL :
                {
                    return basegfx::B2DLineJoin::Bevel;
                }
                case css::drawing::LineJoint_MIDDLE :
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

        basegfx::B2DVector RectPointToB2DVector(RectPoint eRectPoint)
        {
            basegfx::B2DVector aRetval(0.0, 0.0);

            // position changes X
            switch(eRectPoint)
            {
                case RectPoint::LT: case RectPoint::LM: case RectPoint::LB:
                {
                    aRetval.setX(-1.0);
                    break;
                }

                case RectPoint::RT: case RectPoint::RM: case RectPoint::RB:
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
                case RectPoint::LT: case RectPoint::MT: case RectPoint::RT:
                {
                    aRetval.setY(-1.0);
                    break;
                }

                case RectPoint::LB: case RectPoint::MB: case RectPoint::RB:
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

        attribute::SdrGlowAttribute createNewSdrGlowAttribute(const SfxItemSet& rSet)
        {
            sal_Int32 nRadius = rSet.Get(SDRATTR_GLOW_RADIUS).GetValue();
            if (!nRadius)
                return attribute::SdrGlowAttribute();
            Color aColor(rSet.Get(SDRATTR_GLOW_COLOR).GetColorValue());
            sal_uInt16 nTransparency(rSet.Get(SDRATTR_GLOW_TRANSPARENCY).GetValue());
            if (nTransparency)
                aColor.SetAlpha(255 - std::round(nTransparency / 100.0 * 255.0));

            attribute::SdrGlowAttribute glowAttr{ nRadius, aColor };
            return glowAttr;
        }

        sal_Int32 getSoftEdgeRadius(const SfxItemSet& rSet)
        {
            return rSet.Get(SDRATTR_SOFTEDGE_RADIUS).GetValue();
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer


namespace drawinglayer::primitive2d
{
        attribute::SdrLineAttribute createNewSdrLineAttribute(const SfxItemSet& rSet)
        {
            const css::drawing::LineStyle eStyle(rSet.Get(XATTR_LINESTYLE).GetValue());

            if(drawing::LineStyle_NONE != eStyle)
            {
                sal_uInt16 nTransparence(rSet.Get(XATTR_LINETRANSPARENCE).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(100 != nTransparence)
                {
                    const sal_uInt32 nWidth(rSet.Get(XATTR_LINEWIDTH).GetValue());
                    const Color aColor(rSet.Get(XATTR_LINECOLOR).GetColorValue());
                    const css::drawing::LineJoint eJoint(rSet.Get(XATTR_LINEJOINT).GetValue());
                    const css::drawing::LineCap eCap(rSet.Get(XATTR_LINECAP).GetValue());
                    ::std::vector< double > aDotDashArray;
                    double fFullDotDashLen(0.0);

                    if(drawing::LineStyle_DASH == eStyle)
                    {
                        const XDash& rDash = rSet.Get(XATTR_LINEDASH).GetDashValue();

                        if(rDash.GetDots() || rDash.GetDashes())
                        {
                            fFullDotDashLen = rDash.CreateDotDashArray(aDotDashArray, static_cast<double>(nWidth));
                        }
                    }

                    return attribute::SdrLineAttribute(
                        LineJointToB2DLineJoin(eJoint),
                        static_cast<double>(nWidth),
                        static_cast<double>(nTransparence) * 0.01,
                        aColor.getBColor(),
                        eCap,
                        std::move(aDotDashArray),
                        fFullDotDashLen);
                }
            }

            return attribute::SdrLineAttribute();
        }

        attribute::SdrLineStartEndAttribute createNewSdrLineStartEndAttribute(
            const SfxItemSet& rSet,
            double fWidth)
        {
            const sal_Int32 nTempStartWidth(rSet.Get(XATTR_LINESTARTWIDTH).GetValue());
            const sal_Int32 nTempEndWidth(rSet.Get(XATTR_LINEENDWIDTH).GetValue());
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
                if(nTempStartWidth < 0)
                {
                    fStartWidth = (static_cast<double>(-nTempStartWidth) * fWidth) * 0.01;
                }
                else
                {
                    fStartWidth = static_cast<double>(nTempStartWidth);
                }

                if(0.0 != fStartWidth)
                {
                    aStartPolyPolygon = rSet.Get(XATTR_LINESTART).GetLineStartValue();

                    if(aStartPolyPolygon.count() && aStartPolyPolygon.getB2DPolygon(0).count())
                    {
                        bStartActive = true;
                        bStartCentered = rSet.Get(XATTR_LINESTARTCENTER).GetValue();
                    }
                }
            }

            if(nTempEndWidth)
            {
                if(nTempEndWidth < 0)
                {
                    fEndWidth = (static_cast<double>(-nTempEndWidth) * fWidth) * 0.01;
                }
                else
                {
                    fEndWidth = static_cast<double>(nTempEndWidth);
                }

                if(0.0 != fEndWidth)
                {
                    aEndPolyPolygon = rSet.Get(XATTR_LINEEND).GetLineEndValue();

                    if(aEndPolyPolygon.count() && aEndPolyPolygon.getB2DPolygon(0).count())
                    {
                        bEndActive = true;
                        bEndCentered = rSet.Get(XATTR_LINEENDCENTER).GetValue();
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
            const bool bShadow(rSet.Get(SDRATTR_SHADOW).GetValue());

            if(bShadow)
            {
                sal_uInt16 nTransparence(rSet.Get(SDRATTR_SHADOWTRANSPARENCE).GetValue());

                if(nTransparence > 100)
                {
                    nTransparence = 100;
                }

                if(nTransparence)
                {
                    sal_uInt16 nFillTransparence(rSet.Get(XATTR_FILLTRANSPARENCE).GetValue());

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
                        static_cast<double>(rSet.Get(SDRATTR_SHADOWXDIST).GetValue()),
                        static_cast<double>(rSet.Get(SDRATTR_SHADOWYDIST).GetValue()));

                    const basegfx::B2DVector aSize(
                        static_cast<double>(rSet.Get(SDRATTR_SHADOWSIZEX).GetValue()),
                        static_cast<double>(rSet.Get(SDRATTR_SHADOWSIZEY).GetValue()));

                    const Color aColor(rSet.Get(SDRATTR_SHADOWCOLOR).GetColorValue());

                    sal_Int32 nBlur(rSet.Get(SDRATTR_SHADOWBLUR).GetValue());

                    model::RectangleAlignment eAlignment{rSet.Get(SDRATTR_SHADOWALIGNMENT).GetValue()};

                    return attribute::SdrShadowAttribute(aOffset, aSize, static_cast<double>(nTransparence) * 0.01, nBlur, eAlignment, aColor.getBColor());
                }
            }

            return attribute::SdrShadowAttribute();
        }

        attribute::SdrFillAttribute createNewSdrFillAttribute(const SfxItemSet& rSet)
        {
            const drawing::FillStyle eStyle(rSet.Get(XATTR_FILLSTYLE).GetValue());

            sal_uInt16 nTransparence(rSet.Get(XATTR_FILLTRANSPARENCE).GetValue());

            if(nTransparence > 100)
            {
                nTransparence = 100;
            }

            if(drawing::FillStyle_NONE == eStyle)
            {
                XFillUseSlideBackgroundItem aBckItem(rSet.Get(XATTR_FILLUSESLIDEBACKGROUND));
                const bool bSlideBackgroundFill(aBckItem.GetValue());

                if(bSlideBackgroundFill)
                {
                    // we have SlideBackgroundFill mode, create a
                    // SdrFillAttribute accordingly
                    return attribute::SdrFillAttribute(true);
                }
            }

            if(drawing::FillStyle_NONE != eStyle)
            {
                if(100 != nTransparence)
                {
                    // need to check XFillFloatTransparence, object fill may still be completely transparent
                    const XFillFloatTransparenceItem* pGradientItem;

                    if((pGradientItem = rSet.GetItemIfSet(XATTR_FILLFLOATTRANSPARENCE, true))
                        && pGradientItem->IsEnabled())
                    {
                        const XGradient& rGradient = pGradientItem->GetGradientValue();
                        basegfx::BColor aSingleColor;
                        const bool bSingleColor(basegfx::utils::isSingleColor(rGradient.GetColorStops(), aSingleColor));
                        const bool bCompletelyTransparent(bSingleColor && basegfx::fTools::equal(aSingleColor.luminance(), 1.0));

                        if(bCompletelyTransparent)
                        {
                            nTransparence = 100;
                        }
                    }
                }

                if(100 != nTransparence)
                {
                    const Color aColor(rSet.Get(XATTR_FILLCOLOR).GetColorValue());
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
                            XGradient aXGradient(rSet.Get(XATTR_FILLGRADIENT).GetGradientValue());
                            basegfx::ColorStops aColorStops(aXGradient.GetColorStops());

                            // test code here, can/will be removed later
                            static const char* pUseGradientSteps(std::getenv("MCGR_TEST"));
                            static int nUseGradientSteps(pUseGradientSteps ? std::atoi(pUseGradientSteps) : 0);

                            switch(nUseGradientSteps)
                            {
                                case 1:
                                {
                                    // just test a nice valid gradient
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.25, COL_LIGHTGREEN.getBColor()); // green@25%
                                    aColorStops.emplace_back(0.50, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(0.75, COL_LIGHTMAGENTA.getBColor()); // pink@75%
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 2:
                                {
                                    // single added in-between, no change of start/end
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.5, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 3:
                                {
                                    // check additional StartColor, the second one has to win
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.0, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 4:
                                {
                                    // check additional EndColor, the first one has to win
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.0, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 5:
                                {
                                    // check invalid color (too low index), has to be ignored
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(-1.0, COL_YELLOW.getBColor()); // yellow@50%
                                    break;
                                }

                                case 6:
                                {
                                    // check invalid color (too high index), has to be ignored
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(2.0, COL_YELLOW.getBColor()); // yellow@50%
                                    break;
                                }

                                case 7:
                                {
                                    // check in-between single-color section
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.3, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(0.7, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 8:
                                {
                                    // check in-between single-color sections
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.2, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(0.4, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.6, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(0.8, COL_YELLOW.getBColor()); // yellow@50%
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 9:
                                {
                                    // check single-color start area
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.6, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 10:
                                {
                                    // check single-color end area
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.4, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(1.0, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 11:
                                {
                                    // check case without direct Start/EndColor
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.4, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.6, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 12:
                                {
                                    // check case without colors at all
                                    aColorStops.clear();
                                    break;
                                }

                                case 13:
                                {
                                    // check case with single stop
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    break;
                                }

                                case 14:
                                {
                                    // check case with single-double stop
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    break;
                                }

                                case 15:
                                {
                                    // check case with single stop diff colors
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.5, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 16:
                                {
                                    // check case with gradient, hard change, gradient
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.0, COL_LIGHTGREEN.getBColor()); // green
                                    aColorStops.emplace_back(0.2, COL_LIGHTGREEN.getBColor()); // green
                                    aColorStops.emplace_back(0.2, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.5, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(0.8, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.8, COL_LIGHTGREEN.getBColor()); // green
                                    aColorStops.emplace_back(1.0, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                case 17:
                                {
                                    // check case with single stop < 0.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-0.5, COL_LIGHTRED.getBColor()); // red
                                    break;
                                }

                                case 18:
                                {
                                    // check case with single stop > 1.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(1.5, COL_LIGHTRED.getBColor()); // red
                                    break;
                                }

                                case 19:
                                {
                                    // check case with stops overlapping 0.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.5, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 20:
                                {
                                    // check case with stops overlapping 1.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.5, COL_LIGHTBLUE.getBColor()); // blue
                                    break;
                                }

                                case 21:
                                {
                                    // check case with multiple stops < 0.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(-0.4, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(-0.3, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                case 22:
                                {
                                    // check case with multiple stops > 1.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(1.3, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.4, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(1.5, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                case 23:
                                {
                                    // check case with stops overlapping 0.0 and 1.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.5, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(0.5, COL_LIGHTGREEN.getBColor()); // green
                                    aColorStops.emplace_back(1.5, COL_LIGHTRED.getBColor()); // red
                                    break;
                                }

                                case 24:
                                {
                                    // check case with stops overlapping 0.0 and 1.0 and multiple entries
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-0.5, COL_LIGHTGREEN.getBColor()); // green
                                    aColorStops.emplace_back(-0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(0.4, COL_LIGHTBLUE.getBColor()); // blue
                                    aColorStops.emplace_back(0.5, COL_YELLOW.getBColor()); // yellow
                                    aColorStops.emplace_back(0.6, COL_LIGHTGREEN.getBColor()); // green
                                    aColorStops.emplace_back(1.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.5, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                case 25:
                                {
                                    // check case with just two stops overlapping 0.0 and 1.0
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-0.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(1.5, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                case 26:
                                {
                                    // check case with just two stops overlapping 0.0 and 1.0 faaaar out
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-5.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(5.5, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                case 27:
                                {
                                    // check case with just two stops overlapping 0.0 and 1.0 faaaar out but closer to one
                                    aColorStops.clear();
                                    aColorStops.emplace_back(-1.5, COL_LIGHTRED.getBColor()); // red
                                    aColorStops.emplace_back(5.5, COL_LIGHTGREEN.getBColor()); // green
                                    break;
                                }

                                default:
                                {
                                    break;
                                }
                            }

                            if (aXGradient.GetStartIntens() != 100 || aXGradient.GetEndIntens() != 100)
                            {
                                // Need to do the (old, crazy) blend against black for a
                                // used intensity, but now for all ColorStops relative to their
                                // offsets, where 0 means black and 100 means original color
                                basegfx::utils::blendColorStopsToIntensity(
                                    aColorStops,
                                    aXGradient.GetStartIntens() * 0.01,
                                    aXGradient.GetEndIntens() * 0.01,
                                    basegfx::BColor()); // COL_BLACK
                            }

                            aGradient = attribute::FillGradientAttribute(
                                aXGradient.GetGradientStyle(),
                                static_cast<double>(aXGradient.GetBorder()) * 0.01,
                                static_cast<double>(aXGradient.GetXOffset()) * 0.01,
                                static_cast<double>(aXGradient.GetYOffset()) * 0.01,
                                toRadians(aXGradient.GetAngle()),
                                aColorStops,
                                rSet.Get(XATTR_GRADIENTSTEPCOUNT).GetValue());

                            break;
                        }
                        case drawing::FillStyle_HATCH :
                        {
                            const XHatch& rHatch(rSet.Get(XATTR_FILLHATCH).GetHatchValue());
                            const Color aColorB(rHatch.GetColor());

                            aHatch = attribute::FillHatchAttribute(
                                XHatchStyleToHatchStyle(rHatch.GetHatchStyle()),
                                static_cast<double>(rHatch.GetDistance()),
                                toRadians(rHatch.GetAngle()),
                                aColorB.getBColor(),
                                3, // same default as VCL, a minimum of three discrete units (pixels) offset
                                rSet.Get(XATTR_FILLBACKGROUND).GetValue());

                            break;
                        }
                        case drawing::FillStyle_BITMAP :
                        {
                            aFillGraphic = createNewSdrFillGraphicAttribute(rSet);
                            break;
                        }
                    }

                    return attribute::SdrFillAttribute(
                        static_cast<double>(nTransparence) * 0.01,
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
            bool bChainable = rTextObj.IsChainable();


            if(rText.GetOutlinerParaObject())
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
                    std::optional<OutlinerParaObject> pTempObj = rTextObj.CreateEditOutlinerParaObject();

                    if(pTempObj)
                    {
                        aOutlinerParaObject = *pTempObj;
                    }
                    else
                    {
                        // #i100537#
                        // CreateEditOutlinerParaObject() returning no object does not mean that
                        // text edit mode is not active. Do not reset the flag here
                        // bInEditMode = false;
                    }
                }

                const SdrTextAniKind eAniKind(rTextObj.GetTextAniKind());

                // #i107346#
                const SdrOutliner& rDrawTextOutliner(rText.GetObject().getSdrModelFromSdrObject().GetDrawOutliner(&rTextObj));
                const bool bWrongSpell(rDrawTextOutliner.GetControlWord() & EEControlBits::ONLINESPELLING);

                return attribute::SdrTextAttribute(
                    rText,
                    aOutlinerParaObject,
                    rSet.Get(XATTR_FORMTXTSTYLE).GetValue(),
                    pLeft ? *pLeft : rTextObj.GetTextLeftDistance(),
                    pUpper ? *pUpper : rTextObj.GetTextUpperDistance(),
                    pRight ? *pRight : rTextObj.GetTextRightDistance(),
                    pLower ? *pLower : rTextObj.GetTextLowerDistance(),
                    rTextObj.GetTextHorizontalAdjust(rSet),
                    rTextObj.GetTextVerticalAdjust(rSet),
                    rSet.Get(SDRATTR_TEXT_CONTOURFRAME).GetValue(),
                    rTextObj.IsFitToSize(),
                    rTextObj.IsAutoFit(),
                    rSet.Get(XATTR_FORMTXTHIDEFORM).GetValue(),
                    SdrTextAniKind::Blink == eAniKind,
                    SdrTextAniKind::Scroll == eAniKind || SdrTextAniKind::Alternate == eAniKind || SdrTextAniKind::Slide == eAniKind,
                    bInEditMode,
                    rSet.Get(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue(),
                    bWrongSpell,
                    bChainable);
            }

            return attribute::SdrTextAttribute();
        }

        attribute::FillGradientAttribute createNewTransparenceGradientAttribute(const SfxItemSet& rSet)
        {
            const XFillFloatTransparenceItem* pGradientItem;

            if((pGradientItem = rSet.GetItemIfSet(XATTR_FILLFLOATTRANSPARENCE))
                && pGradientItem->IsEnabled())
            {
                // test if float transparency is completely transparent
                const XGradient& rGradient(pGradientItem->GetGradientValue());
                basegfx::BColor aSingleColor;
                const bool bSingleColor(basegfx::utils::isSingleColor(rGradient.GetColorStops(), aSingleColor));
                const bool bCompletelyTransparent(bSingleColor && basegfx::fTools::equal(aSingleColor.luminance(), 1.0));
                const bool bNotTransparent(bSingleColor && basegfx::fTools::equalZero(aSingleColor.luminance()));

                // create nothing when completely transparent: This case is already checked for the
                // normal fill attributes, XFILL_NONE will be used.
                // create nothing when not transparent: use normal fill, no need t create a FillGradientAttribute.
                // Both cases are optimizations, always creating FillGradientAttribute will work, too
                if (!bNotTransparent && !bCompletelyTransparent)
                {
                    basegfx::ColorStops aColorStops(rGradient.GetColorStops());

                    if (rGradient.GetStartIntens() != 100 || rGradient.GetEndIntens() != 100)
                    {
                        // this may also be set for transparence, so need to take care of it
                        basegfx::utils::blendColorStopsToIntensity(
                            aColorStops,
                            rGradient.GetStartIntens() * 0.01,
                            rGradient.GetEndIntens() * 0.01,
                            basegfx::BColor()); // COL_BLACK
                    }

                    return attribute::FillGradientAttribute(
                        rGradient.GetGradientStyle(),
                        static_cast<double>(rGradient.GetBorder()) * 0.01,
                        static_cast<double>(rGradient.GetXOffset()) * 0.01,
                        static_cast<double>(rGradient.GetYOffset()) * 0.01,
                        toRadians(rGradient.GetAngle()),
                        aColorStops,
                        // oops - the gradientStepCount was missing here. If we want to use
                        // a combination of gradient & transparencyGradient to represent
                        // imported gradients of formats which do originally support transparency
                        // in gradients, then the gradient has to be exactly defined the same,
                        // including the (evtl. used) gradientStepCount
                        rSet.Get(XATTR_GRADIENTSTEPCOUNT).GetValue());
                }
            }

            return attribute::FillGradientAttribute();
        }

        attribute::SdrFillGraphicAttribute createNewSdrFillGraphicAttribute(const SfxItemSet& rSet)
        {
            Graphic aGraphic(rSet.Get(XATTR_FILLBITMAP).GetGraphicObject().GetGraphic());

            OUString aOriginURL = aGraphic.getOriginURL();
            if (aGraphic.GetType() == GraphicType::Default && !aOriginURL.isEmpty())
            {
                aGraphic = vcl::graphic::loadFromURL(aGraphic.getOriginURL());
                aGraphic.setOriginURL(aOriginURL);
            }

            if(GraphicType::Bitmap != aGraphic.GetType() && GraphicType::GdiMetafile != aGraphic.GetType())
            {
                // no content if not bitmap or metafile
                OSL_ENSURE(false, "No fill graphic in SfxItemSet (!)");
                return attribute::SdrFillGraphicAttribute();
            }

            Size aPrefSize(aGraphic.GetPrefSize());

            if(!aPrefSize.Width() || !aPrefSize.Height())
            {
                // if there is no logical size, create a size from pixel size and set MapMode accordingly
                if(GraphicType::Bitmap == aGraphic.GetType())
                {
                    aGraphic.SetPrefSize(aGraphic.GetBitmapEx().GetSizePixel());
                    aGraphic.SetPrefMapMode(MapMode(MapUnit::MapPixel));
                    aPrefSize = aGraphic.GetPrefSize();
                }
            }

            if(!aPrefSize.Width() || !aPrefSize.Height())
            {
                // no content if no size
                OSL_ENSURE(false, "Graphic has no size in SfxItemSet (!)");
                return attribute::SdrFillGraphicAttribute();
            }

            // convert size and MapMode to destination logical size and MapMode
            const MapUnit aDestinationMapUnit(rSet.GetPool()->GetMetric(0));
            basegfx::B2DVector aGraphicLogicSize(aGraphic.GetPrefSize().Width(), aGraphic.GetPrefSize().Height());

            if (aGraphic.GetPrefMapMode().GetMapUnit() != aDestinationMapUnit)
            {
                // #i100360# for MapUnit::MapPixel, LogicToLogic will not work properly,
                // so fallback to Application::GetDefaultDevice()
                Size aNewSize(0, 0);

                if(MapUnit::MapPixel == aGraphic.GetPrefMapMode().GetMapUnit())
                {
                    aNewSize = Application::GetDefaultDevice()->PixelToLogic(
                        aGraphic.GetPrefSize(),
                        MapMode(aDestinationMapUnit));
                }
                else
                {
                    aNewSize = OutputDevice::LogicToLogic(
                        aGraphic.GetPrefSize(),
                        aGraphic.GetPrefMapMode(),
                        MapMode(aDestinationMapUnit));
                }

                // #i124002# do not set new size using SetPrefSize at the graphic, this will lead to problems.
                // Instead, adapt the GraphicLogicSize which will be used for further decompositions
                aGraphicLogicSize = basegfx::B2DVector(aNewSize.Width(), aNewSize.Height());
            }

            // get size
            const basegfx::B2DVector aSize(
                static_cast<double>(rSet.Get(XATTR_FILLBMP_SIZEX).GetValue()),
                static_cast<double>(rSet.Get(XATTR_FILLBMP_SIZEY).GetValue()));
            const basegfx::B2DVector aOffset(
                static_cast<double>(rSet.Get(XATTR_FILLBMP_TILEOFFSETX).GetValue()),
                static_cast<double>(rSet.Get(XATTR_FILLBMP_TILEOFFSETY).GetValue()));
            const basegfx::B2DVector aOffsetPosition(
                static_cast<double>(rSet.Get(XATTR_FILLBMP_POSOFFSETX).GetValue()),
                static_cast<double>(rSet.Get(XATTR_FILLBMP_POSOFFSETY).GetValue()));

            return attribute::SdrFillGraphicAttribute(
                aGraphic,
                aGraphicLogicSize,
                aSize,
                aOffset,
                aOffsetPosition,
                RectPointToB2DVector(rSet.GetItem<XFillBmpPosItem>(XATTR_FILLBMP_POS)->GetValue()),
                rSet.Get(XATTR_FILLBMP_TILE).GetValue(),
                rSet.Get(XATTR_FILLBMP_STRETCH).GetValue(),
                rSet.Get(XATTR_FILLBMP_SIZELOG).GetValue());
        }

        attribute::SdrEffectsTextAttribute createNewSdrEffectsTextAttribute(
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
            const attribute::SdrGlowAttribute aGlow(createNewSdrGlowAttribute(rSet));
            const sal_Int32 nSoftEdgeRadius(getSoftEdgeRadius(rSet));

            return attribute::SdrEffectsTextAttribute(aShadow, aText, aGlow, nSoftEdgeRadius);
        }

        attribute::SdrLineEffectsTextAttribute createNewSdrLineEffectsTextAttribute(
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
                const attribute::SdrGlowAttribute aGlow = createNewSdrGlowAttribute(rSet);
                const sal_Int32 nSoftEdgeRadius(getSoftEdgeRadius(rSet));

                return attribute::SdrLineEffectsTextAttribute(aLine, aLineStartEnd, aShadow, aText,
                                                              aGlow, nSoftEdgeRadius);
            }

            return attribute::SdrLineEffectsTextAttribute();
        }

        attribute::SdrLineFillEffectsTextAttribute createNewSdrLineFillEffectsTextAttribute(
            const SfxItemSet& rSet,
            const SdrText* pText,
            bool bHasContent)
        {
            attribute::SdrLineAttribute aLine;
            attribute::SdrFillAttribute aFill;
            attribute::SdrLineStartEndAttribute aLineStartEnd;
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
                const attribute::SdrShadowAttribute aShadow = createNewSdrShadowAttribute(rSet);

                // glow
                const attribute::SdrGlowAttribute aGlow = createNewSdrGlowAttribute(rSet);

                const sal_Int32 nSoftEdgeRadius(getSoftEdgeRadius(rSet));

                return attribute::SdrLineFillEffectsTextAttribute(aLine, aFill, aLineStartEnd,
                                                                  aShadow, aFillFloatTransGradient,
                                                                  aText, aGlow, nSoftEdgeRadius);
            }

            return attribute::SdrLineFillEffectsTextAttribute();
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
            const sal_uInt16 nProjectionValue(rSet.Get(SDRATTR_3DSCENE_PERSPECTIVE).GetValue());

            if(1 == nProjectionValue)
            {
                aProjectionMode = css::drawing::ProjectionMode_PERSPECTIVE;
            }

            // get distance
            const double fDistance(rSet.Get(SDRATTR_3DSCENE_DISTANCE).GetValue());

            // get shadow slant
            const double fShadowSlant(
                basegfx::deg2rad(rSet.Get(SDRATTR_3DSCENE_SHADOW_SLANT).GetValue()));

            // get shade mode
            css::drawing::ShadeMode aShadeMode(css::drawing::ShadeMode_FLAT);
            const sal_uInt16 nShadeValue(rSet.Get(SDRATTR_3DSCENE_SHADE_MODE).GetValue());

            if(1 == nShadeValue)
            {
                aShadeMode = css::drawing::ShadeMode_PHONG;
            }
            else if(2 == nShadeValue)
            {
                aShadeMode = css::drawing::ShadeMode_SMOOTH;
            }
            else if(3 == nShadeValue)
            {
                aShadeMode = css::drawing::ShadeMode_DRAFT;
            }

            // get two sided lighting
            const bool bTwoSidedLighting(rSet.Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING).GetValue());

            return attribute::SdrSceneAttribute(fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting);
        }

        attribute::SdrLightingAttribute createNewSdrLightingAttribute(const SfxItemSet& rSet)
        {
            // extract lights from given SfxItemSet (from scene)
            ::std::vector< attribute::Sdr3DLightAttribute > aLightVector;

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_1).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_1).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1).GetValue());
                aLightVector.emplace_back(aColor, aDirection, true);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_2).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_2).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_3).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_3).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_4).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_4).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_5).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_5).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_6).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_6).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_7).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_7).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            if(rSet.Get(SDRATTR_3DSCENE_LIGHTON_8).GetValue())
            {
                const basegfx::BColor aColor(rSet.Get(SDRATTR_3DSCENE_LIGHTCOLOR_8).GetValue().getBColor());
                const basegfx::B3DVector aDirection(rSet.Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8).GetValue());
                aLightVector.emplace_back(aColor, aDirection, false);
            }

            // get ambient color
            const Color aAmbientValue(rSet.Get(SDRATTR_3DSCENE_AMBIENTCOLOR).GetValue());
            const basegfx::BColor aAmbientLight(aAmbientValue.getBColor());

            return attribute::SdrLightingAttribute(aAmbientLight, std::move(aLightVector));
        }

        void calculateRelativeCornerRadius(sal_Int32 nRadius, const basegfx::B2DRange& rObjectRange, double& rfCornerRadiusX, double& rfCornerRadiusY)
        {
            rfCornerRadiusX = rfCornerRadiusY = static_cast<double>(nRadius);

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

            if(0.0 == rfCornerRadiusY)
                return;

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

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
