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

#include <vcl/gdimetafiletools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graphictools.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>

// helpers

namespace
{
    bool handleGeometricContent(
        const basegfx::B2DPolyPolygon& rClip,
        const basegfx::B2DPolyPolygon& rSource,
        GDIMetaFile& rTarget,
        bool bStroke)
    {
        if(rSource.count() && rClip.count())
        {
            const basegfx::B2DPolyPolygon aResult(
                basegfx::utils::clipPolyPolygonOnPolyPolygon(
                    rSource,
                    rClip,
                    true, // inside
                    bStroke));

            if(aResult.count())
            {
                if(aResult == rSource)
                {
                    // not clipped, but inside. Add original
                    return false;
                }
                else
                {
                    // add clipped geometry
                    if(bStroke)
                    {
                        for(auto const& rB2DPolygon : aResult)
                        {
                            rTarget.AddAction(
                                new MetaPolyLineAction(
                                        tools::Polygon(rB2DPolygon)));
                        }
                    }
                    else
                    {
                        rTarget.AddAction(
                            new MetaPolyPolygonAction(
                                tools::PolyPolygon(aResult)));
                    }
                }
            }
        }

        return true;
    }

    bool handleGradientContent(
        const basegfx::B2DPolyPolygon& rClip,
        const basegfx::B2DPolyPolygon& rSource,
        const Gradient& rGradient,
        GDIMetaFile& rTarget)
    {
        if(rSource.count() && rClip.count())
        {
            const basegfx::B2DPolyPolygon aResult(
                basegfx::utils::clipPolyPolygonOnPolyPolygon(
                    rSource,
                    rClip,
                    true, // inside
                    false)); // stroke

            if(aResult.count())
            {
                if(aResult == rSource)
                {
                    // not clipped, but inside. Add original
                    return false;
                }
                else
                {
                    // add clipped geometry
                    rTarget.AddAction(
                        new MetaGradientExAction(
                            tools::PolyPolygon(aResult),
                            rGradient));
                }
            }
        }

        return true;
    }

    bool handleBitmapContent(
        const basegfx::B2DPolyPolygon& rClip,
        const Point& rPoint,
        const Size& rSize,
        const BitmapEx& rBitmapEx,
        GDIMetaFile& rTarget)
    {
        if(!rSize.Width() || !rSize.Height() || rBitmapEx.IsEmpty())
        {
            // bitmap or size is empty
            return true;
        }

        const basegfx::B2DRange aLogicBitmapRange(
            rPoint.X(), rPoint.Y(),
            rPoint.X() + rSize.Width(), rPoint.Y() + rSize.Height());
        const basegfx::B2DPolyPolygon aClipOfBitmap(
            basegfx::utils::clipPolyPolygonOnRange(
                rClip,
                aLogicBitmapRange,
                true,
                false)); // stroke

        if(!aClipOfBitmap.count())
        {
            // outside clip region
            return true;
        }

        // inside or overlapping. Use area to find out if it is completely
        // covering (inside) or overlapping
        const double fClipArea(basegfx::utils::getArea(aClipOfBitmap));
        const double fBitmapArea(
            aLogicBitmapRange.getWidth() * aLogicBitmapRange.getWidth() +
            aLogicBitmapRange.getHeight() * aLogicBitmapRange.getHeight());
        const double fFactor(fClipArea / fBitmapArea);

        if(basegfx::fTools::more(fFactor, 1.0 - 0.001))
        {
            // completely covering (with 0.1% tolerance)
            return false;
        }

        // needs clipping (with 0.1% tolerance). Prepare VirtualDevice
        // in pixel mode for alpha channel painting (black is transparent,
        // white to paint 100% opacity)
        const Size aSizePixel(rBitmapEx.GetSizePixel());
        ScopedVclPtrInstance< VirtualDevice > aVDev;

        aVDev->SetOutputSizePixel(aSizePixel);
        aVDev->EnableMapMode(false);
        aVDev->SetFillColor( COL_WHITE);
        aVDev->SetLineColor();

        if(rBitmapEx.IsAlpha())
        {
            // use given alpha channel
            aVDev->DrawBitmap(Point(0, 0), rBitmapEx.GetAlpha().GetBitmap());
        }
        else
        {
            // reset alpha channel
            aVDev->SetBackground(Wallpaper(COL_BLACK));
            aVDev->Erase();
        }

        // transform polygon from clipping to pixel coordinates
        basegfx::B2DPolyPolygon aPixelPoly(aClipOfBitmap);
        basegfx::B2DHomMatrix aTransform;

        aTransform.translate(-aLogicBitmapRange.getMinX(), -aLogicBitmapRange.getMinY());
        aTransform.scale(
            static_cast< double >(aSizePixel.Width()) / aLogicBitmapRange.getWidth(),
            static_cast< double >(aSizePixel.Height()) / aLogicBitmapRange.getHeight());
        aPixelPoly.transform(aTransform);

        // to fill the non-covered parts, use the Xor fill rule of
        // tools::PolyPolygon painting. Start with an all-covering polygon and
        // add the clip polygon one
        basegfx::B2DPolyPolygon aInvertPixelPoly;

        aInvertPixelPoly.append(
            basegfx::utils::createPolygonFromRect(
                basegfx::B2DRange(
                    0.0, 0.0,
                    aSizePixel.Width(), aSizePixel.Height())));
        aInvertPixelPoly.append(aPixelPoly);

        // paint as alpha
        aVDev->DrawPolyPolygon(aInvertPixelPoly);

        // get created alpha mask and set defaults
        AlphaMask aAlpha(
            aVDev->GetBitmap(
                Point(0, 0),
                aSizePixel));

        aAlpha.SetPrefSize(rBitmapEx.GetPrefSize());
        aAlpha.SetPrefMapMode(rBitmapEx.GetPrefMapMode());

        // add new action replacing the old one
        rTarget.AddAction(
            new MetaBmpExScaleAction(
                Point(
                    basegfx::fround<tools::Long>(aLogicBitmapRange.getMinX()),
                    basegfx::fround<tools::Long>(aLogicBitmapRange.getMinY())),
                Size(
                    basegfx::fround<tools::Long>(aLogicBitmapRange.getWidth()),
                    basegfx::fround<tools::Long>(aLogicBitmapRange.getHeight())),
                BitmapEx(rBitmapEx.GetBitmap(), aAlpha)));

        return true;
    }

    void addSvtGraphicStroke(const SvtGraphicStroke& rStroke, GDIMetaFile& rTarget)
    {
        // write SvtGraphicFill
        SvMemoryStream aMemStm;
        WriteSvtGraphicStroke( aMemStm, rStroke );
        rTarget.AddAction(
            new MetaCommentAction(
                "XPATHSTROKE_SEQ_BEGIN",
                0,
                static_cast< const sal_uInt8* >(aMemStm.GetData()),
                aMemStm.TellEnd()));
    }

    void addSvtGraphicFill(const SvtGraphicFill &rFilling, GDIMetaFile& rTarget)
    {
        // write SvtGraphicFill
        SvMemoryStream aMemStm;
        WriteSvtGraphicFill( aMemStm, rFilling );
        rTarget.AddAction(
            new MetaCommentAction(
                "XPATHFILL_SEQ_BEGIN",
                0,
                static_cast< const sal_uInt8* >(aMemStm.GetData()),
                aMemStm.TellEnd()));
    }
} // end of anonymous namespace

// #i121267# Tooling to internally clip geometry against internal clip regions

void clipMetafileContentAgainstOwnRegions(GDIMetaFile& rSource)
{
    const sal_uLong nObjCount(rSource.GetActionSize());

    if(!nObjCount)
    {
        return;
    }

    // prepare target data container and push/pop stack data
    GDIMetaFile aTarget;
    bool bChanged(false);
    std::vector< basegfx::B2DPolyPolygon > aClips;
    std::vector< vcl::PushFlags > aPushFlags;
    std::vector< MapMode > aMapModes;

    // start with empty region
    aClips.emplace_back();

    // start with default MapMode (MapUnit::MapPixel)
    aMapModes.emplace_back();

    for(sal_uLong i(0); i < nObjCount; ++i)
    {
        const MetaAction* pAction(rSource.GetAction(i));
        const MetaActionType nType(pAction->GetType());
        bool bDone(false);

        // basic operation takes care of clipregion actions (four) and push/pop of these
        // to steer the currently set clip region. There *is* an active
        // clip region when (aClips.size() && aClips.back().count()), see
        // below
        switch(nType)
        {
            case MetaActionType::CLIPREGION :
            {
                const MetaClipRegionAction* pA = static_cast< const MetaClipRegionAction* >(pAction);

                if(pA->IsClipping())
                {
                    const vcl::Region& rRegion = pA->GetRegion();
                    const basegfx::B2DPolyPolygon aNewClip(rRegion.GetAsB2DPolyPolygon());

                    aClips.back() = aNewClip;
                }
                else
                {
                    aClips.back() = basegfx::B2DPolyPolygon();
                }

                break;
            }

            case MetaActionType::ISECTRECTCLIPREGION :
            {
                const MetaISectRectClipRegionAction* pA = static_cast< const MetaISectRectClipRegionAction* >(pAction);
                const tools::Rectangle& rRect = pA->GetRect();

                if(!rRect.IsEmpty() && !aClips.empty() && aClips.back().count())
                {
                    const basegfx::B2DRange aClipRange(vcl::unotools::b2DRectangleFromRectangle(rRect));

                    aClips.back() = basegfx::utils::clipPolyPolygonOnRange(
                        aClips.back(),
                        aClipRange,
                        true, // inside
                        false); // stroke
                }
                break;
            }

            case MetaActionType::ISECTREGIONCLIPREGION :
            {
                const MetaISectRegionClipRegionAction* pA = static_cast< const MetaISectRegionClipRegionAction* >(pAction);
                const vcl::Region& rRegion = pA->GetRegion();

                if(!rRegion.IsEmpty() && !aClips.empty() && aClips.back().count())
                {
                    const basegfx::B2DPolyPolygon aNewClip(rRegion.GetAsB2DPolyPolygon());

                    aClips.back() = basegfx::utils::clipPolyPolygonOnPolyPolygon(
                        aClips.back(),
                        aNewClip,
                        true,  // inside
                        false); // stroke
                }
                break;
            }

            case MetaActionType::MOVECLIPREGION :
            {
                const MetaMoveClipRegionAction* pA = static_cast< const MetaMoveClipRegionAction* >(pAction);
                const tools::Long aHorMove(pA->GetHorzMove());
                const tools::Long aVerMove(pA->GetVertMove());

                if((aHorMove || aVerMove) && !aClips.empty() && aClips.back().count())
                {
                    aClips.back().transform(
                        basegfx::utils::createTranslateB2DHomMatrix(
                            aHorMove,
                            aVerMove));
                }
                break;
            }

            case MetaActionType::PUSH :
            {
                const MetaPushAction* pA = static_cast< const MetaPushAction* >(pAction);
                const vcl::PushFlags nFlags(pA->GetFlags());

                aPushFlags.push_back(nFlags);

                if(nFlags & vcl::PushFlags::CLIPREGION)
                {
                    aClips.push_back(aClips.back());
                }

                if(nFlags & vcl::PushFlags::MAPMODE)
                {
                    aMapModes.push_back(aMapModes.back());
                }
                break;
            }

            case MetaActionType::POP :
            {

                if(!aPushFlags.empty())
                {
                    const vcl::PushFlags nFlags(aPushFlags.back());
                    aPushFlags.pop_back();

                    if(nFlags & vcl::PushFlags::CLIPREGION)
                    {
                        if(aClips.size() > 1)
                        {
                            aClips.pop_back();
                        }
                        else
                        {
                            OSL_ENSURE(false, "Wrong POP() in ClipRegions (!)");
                        }
                    }

                    if(nFlags & vcl::PushFlags::MAPMODE)
                    {
                        if(aMapModes.size() > 1)
                        {
                            aMapModes.pop_back();
                        }
                        else
                        {
                            OSL_ENSURE(false, "Wrong POP() in MapModes (!)");
                        }
                    }
                }
                else
                {
                    OSL_ENSURE(false, "Invalid pop() without push() (!)");
                }

                break;
            }

            case MetaActionType::MAPMODE :
            {
                const MetaMapModeAction* pA = static_cast< const MetaMapModeAction* >(pAction);

                aMapModes.back() = pA->GetMapMode();
                break;
            }

            default:
            {
                break;
            }
        }

        // this area contains all actions which could potentially be clipped. Since
        // this tooling is only a fallback (see comments in header), only the needed
        // actions will be implemented. Extend using the pattern for the already
        // implemented actions.
        if(!aClips.empty() && aClips.back().count())
        {
            switch(nType)
            {

                // pixel actions, just check on inside

                case MetaActionType::PIXEL :
                {
                    const MetaPixelAction* pA = static_cast< const MetaPixelAction* >(pAction);
                    const Point& rPoint = pA->GetPoint();

                    if(!basegfx::utils::isInside(
                        aClips.back(),
                        basegfx::B2DPoint(rPoint.X(), rPoint.Y())))
                    {
                        // when not inside, do not add original
                        bDone = true;
                    }
                    break;
                }

                case MetaActionType::POINT :
                {
                    const MetaPointAction* pA = static_cast< const MetaPointAction* >(pAction);
                    const Point& rPoint = pA->GetPoint();

                    if(!basegfx::utils::isInside(
                        aClips.back(),
                        basegfx::B2DPoint(rPoint.X(), rPoint.Y())))
                    {
                        // when not inside, do not add original
                        bDone = true;
                    }
                    break;
                }

                // geometry actions

                case MetaActionType::LINE :
                {
                    const MetaLineAction* pA = static_cast< const MetaLineAction* >(pAction);
                    const Point& rStart(pA->GetStartPoint());
                    const Point& rEnd(pA->GetEndPoint());
                    basegfx::B2DPolygon aLine;

                    aLine.append(basegfx::B2DPoint(rStart.X(), rStart.Y()));
                    aLine.append(basegfx::B2DPoint(rEnd.X(), rEnd.Y()));

                    bDone = handleGeometricContent(
                        aClips.back(),
                        basegfx::B2DPolyPolygon(aLine),
                        aTarget,
                        true); // stroke
                    break;
                }

                case MetaActionType::RECT :
                {
                    const MetaRectAction* pA = static_cast< const MetaRectAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(
                                basegfx::utils::createPolygonFromRect(
                                        vcl::unotools::b2DRectangleFromRectangle(rRect))),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case MetaActionType::ROUNDRECT :
                {
                    const MetaRoundRectAction* pA = static_cast< const MetaRoundRectAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const sal_uInt32 nHor(pA->GetHorzRound());
                        const sal_uInt32 nVer(pA->GetVertRound());
                        const basegfx::B2DRange aRange(vcl::unotools::b2DRectangleFromRectangle(rRect));
                        basegfx::B2DPolygon aOutline;

                        if(nHor || nVer)
                        {
                            double fRadiusX((nHor * 2.0) / (aRange.getWidth() > 0.0 ? aRange.getWidth() : 1.0));
                            double fRadiusY((nVer * 2.0) / (aRange.getHeight() > 0.0 ? aRange.getHeight() : 1.0));
                            fRadiusX = std::clamp(fRadiusX, 0.0, 1.0);
                            fRadiusY = std::clamp(fRadiusY, 0.0, 1.0);

                            aOutline = basegfx::utils::createPolygonFromRect(aRange, fRadiusX, fRadiusY);
                        }
                        else
                        {
                            aOutline = basegfx::utils::createPolygonFromRect(aRange);
                        }

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aOutline),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case MetaActionType::ELLIPSE :
                {
                    const MetaEllipseAction* pA = static_cast< const MetaEllipseAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const basegfx::B2DRange aRange(vcl::unotools::b2DRectangleFromRectangle(rRect));

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(
                                basegfx::utils::createPolygonFromEllipse(
                                    aRange.getCenter(),
                                    aRange.getWidth() * 0.5,
                                    aRange.getHeight() * 0.5)),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case MetaActionType::ARC :
                {
                    const MetaArcAction* pA = static_cast< const MetaArcAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const tools::Polygon aToolsPoly(
                                rRect,
                                pA->GetStartPoint(),
                                pA->GetEndPoint(),
                                PolyStyle::Arc);

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aToolsPoly.getB2DPolygon()),
                            aTarget,
                            true); // stroke
                    }
                    break;
                }

                case MetaActionType::PIE :
                {
                    const MetaPieAction* pA = static_cast< const MetaPieAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const tools::Polygon aToolsPoly(
                                rRect,
                                pA->GetStartPoint(),
                                pA->GetEndPoint(),
                                PolyStyle::Pie);

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aToolsPoly.getB2DPolygon()),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case MetaActionType::CHORD :
                {
                    const MetaChordAction* pA = static_cast< const MetaChordAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const tools::Polygon aToolsPoly(
                                rRect,
                                pA->GetStartPoint(),
                                pA->GetEndPoint(),
                                PolyStyle::Chord);

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aToolsPoly.getB2DPolygon()),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case MetaActionType::POLYLINE :
                {
                    const MetaPolyLineAction* pA = static_cast< const MetaPolyLineAction* >(pAction);

                    bDone = handleGeometricContent(
                        aClips.back(),
                        basegfx::B2DPolyPolygon(pA->GetPolygon().getB2DPolygon()),
                        aTarget,
                        true); // stroke
                    break;
                }

                case MetaActionType::POLYGON :
                {
                    const MetaPolygonAction* pA = static_cast< const MetaPolygonAction* >(pAction);

                    bDone = handleGeometricContent(
                        aClips.back(),
                        basegfx::B2DPolyPolygon(pA->GetPolygon().getB2DPolygon()),
                        aTarget,
                        false); // stroke
                    break;
                }

                case MetaActionType::POLYPOLYGON :
                {
                    const MetaPolyPolygonAction* pA = static_cast< const MetaPolyPolygonAction* >(pAction);
                    const tools::PolyPolygon& rPoly = pA->GetPolyPolygon();

                    bDone = handleGeometricContent(
                        aClips.back(),
                        rPoly.getB2DPolyPolygon(),
                        aTarget,
                        false); // stroke
                    break;
                }

                // bitmap actions, create BitmapEx with alpha channel derived
                // from clipping

                case MetaActionType::BMPEX :
                {
                    const MetaBmpExAction* pA = static_cast< const MetaBmpExAction* >(pAction);
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    // the logical size depends on the PrefSize of the given bitmap in
                    // combination with the current MapMode
                    Size aLogicalSize(rBitmapEx.GetPrefSize());

                    if(MapUnit::MapPixel == rBitmapEx.GetPrefMapMode().GetMapUnit())
                    {
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aLogicalSize, aMapModes.back());
                    }
                    else
                    {
                        aLogicalSize = OutputDevice::LogicToLogic(aLogicalSize, rBitmapEx.GetPrefMapMode(), aMapModes.back());
                    }

                    bDone = handleBitmapContent(
                        aClips.back(),
                        pA->GetPoint(),
                        aLogicalSize,
                        rBitmapEx,
                        aTarget);
                    break;
                }

                case MetaActionType::BMP :
                {
                    const MetaBmpAction* pA = static_cast< const MetaBmpAction* >(pAction);
                    const Bitmap& rBitmap = pA->GetBitmap();

                    // the logical size depends on the PrefSize of the given bitmap in
                    // combination with the current MapMode
                    Size aLogicalSize(rBitmap.GetPrefSize());

                    if(MapUnit::MapPixel == rBitmap.GetPrefMapMode().GetMapUnit())
                    {
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aLogicalSize, aMapModes.back());
                    }
                    else
                    {
                        aLogicalSize = OutputDevice::LogicToLogic(aLogicalSize, rBitmap.GetPrefMapMode(), aMapModes.back());
                    }

                    bDone = handleBitmapContent(
                        aClips.back(),
                        pA->GetPoint(),
                        aLogicalSize,
                        BitmapEx(rBitmap),
                        aTarget);
                    break;
                }

                case MetaActionType::BMPEXSCALE :
                {
                    const MetaBmpExScaleAction* pA = static_cast< const MetaBmpExScaleAction* >(pAction);

                    bDone = handleBitmapContent(
                        aClips.back(),
                        pA->GetPoint(),
                        pA->GetSize(),
                        pA->GetBitmapEx(),
                        aTarget);
                    break;
                }

                case MetaActionType::BMPSCALE :
                {
                    const MetaBmpScaleAction* pA = static_cast< const MetaBmpScaleAction* >(pAction);

                    bDone = handleBitmapContent(
                        aClips.back(),
                        pA->GetPoint(),
                        pA->GetSize(),
                        BitmapEx(pA->GetBitmap()),
                        aTarget);
                    break;
                }

                case MetaActionType::BMPEXSCALEPART :
                {
                    const MetaBmpExScalePartAction* pA = static_cast< const MetaBmpExScalePartAction* >(pAction);
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    if(rBitmapEx.IsEmpty())
                    {
                        // empty content
                        bDone = true;
                    }
                    else
                    {
                        BitmapEx aCroppedBitmapEx(rBitmapEx);
                        const tools::Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

                        if(aCropRectangle.IsEmpty())
                        {
                            // empty content
                            bDone = true;
                        }
                        else
                        {
                            aCroppedBitmapEx.Crop(aCropRectangle);
                            bDone = handleBitmapContent(
                                aClips.back(),
                                pA->GetDestPoint(),
                                pA->GetDestSize(),
                                aCroppedBitmapEx,
                                aTarget);
                        }
                    }
                    break;
                }

                case MetaActionType::BMPSCALEPART :
                {
                    const MetaBmpScalePartAction* pA = static_cast< const MetaBmpScalePartAction* >(pAction);
                    const Bitmap& rBitmap = pA->GetBitmap();

                    if(rBitmap.IsEmpty())
                    {
                        // empty content
                        bDone = true;
                    }
                    else
                    {
                        Bitmap aCroppedBitmap(rBitmap);
                        const tools::Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

                        if(aCropRectangle.IsEmpty())
                        {
                            // empty content
                            bDone = true;
                        }
                        else
                        {
                            aCroppedBitmap.Crop(aCropRectangle);
                            bDone = handleBitmapContent(
                                aClips.back(),
                                pA->GetDestPoint(),
                                pA->GetDestSize(),
                                BitmapEx(aCroppedBitmap),
                                aTarget);
                        }
                    }
                    break;
                }

                // need to handle all those 'hacks' which hide data in comments

                case MetaActionType::COMMENT :
                {
                    const MetaCommentAction* pA = static_cast< const MetaCommentAction* >(pAction);
                    const OString& rComment = pA->GetComment();

                    if(rComment.equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
                    {
                        // nothing to do; this just means that between here and XGRAD_SEQ_END
                        // exists a MetaActionType::GRADIENTEX mixed with Xor-tricked painting
                        // commands. This comment is used to scan over these and filter for
                        // the gradient action. It is needed to support MetaActionType::GRADIENTEX
                        // in this processor to solve usages.
                    }
                    else if(rComment.equalsIgnoreAsciiCase("XPATHFILL_SEQ_BEGIN"))
                    {
                        SvtGraphicFill aFilling;
                        tools::PolyPolygon aPath;

                        {   // read SvtGraphicFill
                            SvMemoryStream aMemStm(const_cast<sal_uInt8 *>(pA->GetData()), pA->GetDataSize(),StreamMode::READ);
                            ReadSvtGraphicFill( aMemStm, aFilling );
                        }

                        aFilling.getPath(aPath);

                        if(aPath.Count())
                        {
                            const basegfx::B2DPolyPolygon aSource(aPath.getB2DPolyPolygon());
                            const basegfx::B2DPolyPolygon aResult(
                                basegfx::utils::clipPolyPolygonOnPolyPolygon(
                                    aSource,
                                    aClips.back(),
                                    true, // inside
                                    false)); // stroke

                            if(aResult.count())
                            {
                                if(aResult != aSource)
                                {
                                    // add clipped geometry
                                    aFilling.setPath(tools::PolyPolygon(aResult));
                                    addSvtGraphicFill(aFilling, aTarget);
                                    bDone = true;
                                }
                            }
                            else
                            {
                                // exchange with empty polygon
                                aFilling.setPath(tools::PolyPolygon());
                                addSvtGraphicFill(aFilling, aTarget);
                                bDone = true;
                            }
                        }
                    }
                    else if(rComment.equalsIgnoreAsciiCase("XPATHSTROKE_SEQ_BEGIN"))
                    {
                        SvtGraphicStroke aStroke;
                        tools::Polygon aPath;

                        {   // read SvtGraphicFill
                            SvMemoryStream aMemStm(const_cast<sal_uInt8 *>(pA->GetData()), pA->GetDataSize(),StreamMode::READ);
                            ReadSvtGraphicStroke( aMemStm, aStroke );
                        }

                        aStroke.getPath(aPath);

                        if(aPath.GetSize())
                        {
                            const basegfx::B2DPolygon aSource(aPath.getB2DPolygon());
                            const basegfx::B2DPolyPolygon aResult(
                                basegfx::utils::clipPolygonOnPolyPolygon(
                                    aSource,
                                    aClips.back(),
                                    true, // inside
                                    true)); // stroke

                            if(aResult.count())
                            {
                                if(aResult.count() > 1 || aResult.getB2DPolygon(0) != aSource)
                                {
                                    // add clipped geometry
                                    for(auto const& rB2DPolygon : aResult)
                                    {
                                        aStroke.setPath(tools::Polygon(rB2DPolygon));
                                        addSvtGraphicStroke(aStroke, aTarget);
                                    }

                                    bDone = true;
                                }
                            }
                            else
                            {
                                // exchange with empty polygon
                                aStroke.setPath(tools::Polygon());
                                addSvtGraphicStroke(aStroke, aTarget);
                                bDone = true;
                            }

                        }
                    }
                    break;
                }

                // need to handle gradient fills (hopefully only unrotated ones)

                case MetaActionType::GRADIENT :
                {
                    const MetaGradientAction* pA = static_cast< const MetaGradientAction* >(pAction);
                    const tools::Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        bDone = handleGradientContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(
                                basegfx::utils::createPolygonFromRect(
                                        vcl::unotools::b2DRectangleFromRectangle(rRect))),
                            pA->GetGradient(),
                            aTarget);
                    }

                    break;
                }

                case MetaActionType::GRADIENTEX :
                {
                    const MetaGradientExAction* pA = static_cast< const MetaGradientExAction* >(pAction);
                    const tools::PolyPolygon& rPolyPoly = pA->GetPolyPolygon();

                    bDone = handleGradientContent(
                        aClips.back(),
                        rPolyPoly.getB2DPolyPolygon(),
                        pA->GetGradient(),
                        aTarget);
                    break;
                }

                // not (yet) supported actions

                // MetaActionType::NONE
                // MetaActionType::TEXT
                // MetaActionType::TEXTARRAY
                // MetaActionType::STRETCHTEXT
                // MetaActionType::TEXTRECT
                // MetaActionType::MASK
                // MetaActionType::MASKSCALE
                // MetaActionType::MASKSCALEPART
                // MetaActionType::HATCH
                // MetaActionType::WALLPAPER
                // MetaActionType::FILLCOLOR
                // MetaActionType::TEXTCOLOR
                // MetaActionType::TEXTFILLCOLOR
                // MetaActionType::TEXTALIGN
                // MetaActionType::MAPMODE
                // MetaActionType::FONT
                // MetaActionType::Transparent
                // MetaActionType::EPS
                // MetaActionType::REFPOINT
                // MetaActionType::TEXTLINECOLOR
                // MetaActionType::TEXTLINE
                // MetaActionType::FLOATTRANSPARENT
                // MetaActionType::LAYOUTMODE
                // MetaActionType::TEXTLANGUAGE
                // MetaActionType::OVERLINECOLOR

                // if an action is not handled at all, it will simply get copied to the
                // target (see below). This is the default for all non-implemented actions
                default:
                {
                    break;
                }
            }
        }

        if(bDone)
        {
            bChanged = true;
        }
        else
        {
            aTarget.AddAction(const_cast< MetaAction* >(pAction));
        }
    }

    if(bChanged)
    {
        // when changed, copy back and do not forget to set MapMode
        // and PrefSize
        aTarget.SetPrefMapMode(rSource.GetPrefMapMode());
        aTarget.SetPrefSize(rSource.GetPrefSize());
        rSource = aTarget;
    }
}

bool usesClipActions(const GDIMetaFile& rSource)
{
    const sal_uLong nObjCount(rSource.GetActionSize());

    for(sal_uLong i(0); i < nObjCount; ++i)
    {
        const MetaAction* pAction(rSource.GetAction(i));
        const MetaActionType nType(pAction->GetType());

        switch(nType)
        {
            case MetaActionType::CLIPREGION :
            case MetaActionType::ISECTRECTCLIPREGION :
            case MetaActionType::ISECTREGIONCLIPREGION :
            case MetaActionType::MOVECLIPREGION :
            {
                return true;
            }

            default: break;
        }
    }

    return false;
}

MetafileAccessor::~MetafileAccessor()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
