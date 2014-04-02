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
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graphictools.hxx>

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
                basegfx::tools::clipPolyPolygonOnPolyPolygon(
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
                        for(sal_uInt32 a(0); a < aResult.count(); a++)
                        {
                            rTarget.AddAction(
                                new MetaPolyLineAction(
                                    Polygon(aResult.getB2DPolygon(a))));
                        }
                    }
                    else
                    {
                        rTarget.AddAction(
                            new MetaPolyPolygonAction(
                                PolyPolygon(aResult)));
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
                basegfx::tools::clipPolyPolygonOnPolyPolygon(
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
                            PolyPolygon(aResult),
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
            basegfx::tools::clipPolyPolygonOnRange(
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
        const double fClipArea(basegfx::tools::getArea(aClipOfBitmap));
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
        VirtualDevice aVDev;

        aVDev.SetOutputSizePixel(aSizePixel);
        aVDev.EnableMapMode(false);
        aVDev.SetFillColor(COL_WHITE);
        aVDev.SetLineColor();

        if(rBitmapEx.IsTransparent())
        {
            // use given alpha channel
            aVDev.DrawBitmap(Point(0, 0), rBitmapEx.GetAlpha().GetBitmap());
        }
        else
        {
            // reset alpha channel
            aVDev.SetBackground(Wallpaper(Color(COL_BLACK)));
            aVDev.Erase();
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
        // PolyPolygon painting. Start with a all-covering polygon and
        // add the clip polygon one
        basegfx::B2DPolyPolygon aInvertPixelPoly;

        aInvertPixelPoly.append(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRange(
                    0.0, 0.0,
                    aSizePixel.Width(), aSizePixel.Height())));
        aInvertPixelPoly.append(aPixelPoly);

        // paint as alpha
        aVDev.DrawPolyPolygon(aInvertPixelPoly);

        // get created alpha mask and set defaults
        AlphaMask aAlpha(
            aVDev.GetBitmap(
                Point(0, 0),
                aSizePixel));

        aAlpha.SetPrefSize(rBitmapEx.GetPrefSize());
        aAlpha.SetPrefMapMode(rBitmapEx.GetPrefMapMode());

        // add new action replacing the old one
        rTarget.AddAction(
            new MetaBmpExScaleAction(
                Point(
                    basegfx::fround(aLogicBitmapRange.getMinX()),
                    basegfx::fround(aLogicBitmapRange.getMinY())),
                Size(
                    basegfx::fround(aLogicBitmapRange.getWidth()),
                    basegfx::fround(aLogicBitmapRange.getHeight())),
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
                aMemStm.Seek(STREAM_SEEK_TO_END)));
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
                aMemStm.Seek(STREAM_SEEK_TO_END)));
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
    std::vector< sal_uInt16 > aPushFlags;
    std::vector< MapMode > aMapModes;

    // start with empty region
    aClips.push_back(basegfx::B2DPolyPolygon());

    // start with default MapMode (MAP_PIXEL)
    aMapModes.push_back(MapMode());

    for(sal_uLong i(0); i < nObjCount; ++i)
    {
        const MetaAction* pAction(rSource.GetAction(i));
        const sal_uInt16 nType(pAction->GetType());
        bool bDone(false);

        // basic operation takes care of clipregion actions (four) and push/pop of these
        // to steer the currently set clip region. There *is* an active
        // clip region when (aClips.size() && aClips.back().count()), see
        // below
        switch(nType)
        {
            case META_CLIPREGION_ACTION :
            {
                const MetaClipRegionAction* pA = static_cast< const MetaClipRegionAction* >(pAction);

                if(pA->IsClipping())
                {
                    const Region& rRegion = pA->GetRegion();
                    const basegfx::B2DPolyPolygon aNewClip(rRegion.GetAsB2DPolyPolygon());

                    aClips.back() = aNewClip;
                }
                else
                {
                    aClips.back() = basegfx::B2DPolyPolygon();
                }

                break;
            }

            case META_ISECTRECTCLIPREGION_ACTION :
            {
                const MetaISectRectClipRegionAction* pA = static_cast< const MetaISectRectClipRegionAction* >(pAction);
                const Rectangle& rRect = pA->GetRect();

                if(!rRect.IsEmpty() && aClips.size() && aClips.back().count())
                {
                    const basegfx::B2DRange aClipRange(
                        rRect.Left(), rRect.Top(),
                        rRect.Right(), rRect.Bottom());

                    aClips.back() = basegfx::tools::clipPolyPolygonOnRange(
                        aClips.back(),
                        aClipRange,
                        true, // inside
                        false); // stroke
                }
                break;
            }

            case META_ISECTREGIONCLIPREGION_ACTION :
            {
                const MetaISectRegionClipRegionAction* pA = static_cast< const MetaISectRegionClipRegionAction* >(pAction);
                const Region& rRegion = pA->GetRegion();

                if(!rRegion.IsEmpty() && aClips.size() && aClips.back().count())
                {
                    const basegfx::B2DPolyPolygon aNewClip(rRegion.GetAsB2DPolyPolygon());

                    aClips.back() = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                        aClips.back(),
                        aNewClip,
                        true,  // inside
                        false); // stroke
                }
                break;
            }

            case META_MOVECLIPREGION_ACTION :
            {
                const MetaMoveClipRegionAction* pA = static_cast< const MetaMoveClipRegionAction* >(pAction);
                const long aHorMove(pA->GetHorzMove());
                const long aVerMove(pA->GetVertMove());

                if((aHorMove || aVerMove) && aClips.size() && aClips.back().count())
                {
                    aClips.back().transform(
                        basegfx::tools::createTranslateB2DHomMatrix(
                            aHorMove,
                            aVerMove));
                }
                break;
            }

            case META_PUSH_ACTION :
            {
                const MetaPushAction* pA = static_cast< const MetaPushAction* >(pAction);
                const sal_uInt16 nFlags(pA->GetFlags());

                aPushFlags.push_back(nFlags);

                if(nFlags & PUSH_CLIPREGION)
                {
                    aClips.push_back(aClips.back());
                }

                if(nFlags & PUSH_MAPMODE)
                {
                    aMapModes.push_back(aMapModes.back());
                }
                break;
            }

            case META_POP_ACTION :
            {

                if(aPushFlags.size())
                {
                    const sal_uInt16 nFlags(aPushFlags.back());
                    aPushFlags.pop_back();

                    if(nFlags & PUSH_CLIPREGION)
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

                    if(nFlags & PUSH_MAPMODE)
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

            case META_MAPMODE_ACTION :
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
        if(aClips.size() && aClips.back().count())
        {
            switch(nType)
            {

                // pixel actions, just check on inside

                case META_PIXEL_ACTION :
                {
                    const MetaPixelAction* pA = static_cast< const MetaPixelAction* >(pAction);
                    const Point& rPoint = pA->GetPoint();

                    if(!basegfx::tools::isInside(
                        aClips.back(),
                        basegfx::B2DPoint(rPoint.X(), rPoint.Y())))
                    {
                        // when not inside, do not add original
                        bDone = true;
                    }
                    break;
                }

                case META_POINT_ACTION :
                {
                    const MetaPointAction* pA = static_cast< const MetaPointAction* >(pAction);
                    const Point& rPoint = pA->GetPoint();

                    if(!basegfx::tools::isInside(
                        aClips.back(),
                        basegfx::B2DPoint(rPoint.X(), rPoint.Y())))
                    {
                        // when not inside, do not add original
                        bDone = true;
                    }
                    break;
                }

                // geometry actions

                case META_LINE_ACTION :
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

                case META_RECT_ACTION :
                {
                    const MetaRectAction* pA = static_cast< const MetaRectAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(
                                basegfx::tools::createPolygonFromRect(
                                    basegfx::B2DRange(
                                        rRect.Left(), rRect.Top(),
                                        rRect.Right(), rRect.Bottom()))),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case META_ROUNDRECT_ACTION :
                {
                    const MetaRoundRectAction* pA = static_cast< const MetaRoundRectAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const sal_uInt32 nHor(pA->GetHorzRound());
                        const sal_uInt32 nVer(pA->GetVertRound());
                        const basegfx::B2DRange aRange(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom());
                        basegfx::B2DPolygon aOutline;

                        if(nHor || nVer)
                        {
                            double fRadiusX((nHor * 2.0) / (aRange.getWidth() > 0.0 ? aRange.getWidth() : 1.0));
                            double fRadiusY((nVer * 2.0) / (aRange.getHeight() > 0.0 ? aRange.getHeight() : 1.0));
                            fRadiusX = std::max(0.0, std::min(1.0, fRadiusX));
                            fRadiusY = std::max(0.0, std::min(1.0, fRadiusY));

                            aOutline = basegfx::tools::createPolygonFromRect(aRange, fRadiusX, fRadiusY);
                        }
                        else
                        {
                            aOutline = basegfx::tools::createPolygonFromRect(aRange);
                        }

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aOutline),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case META_ELLIPSE_ACTION :
                {
                    const MetaEllipseAction* pA = static_cast< const MetaEllipseAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const basegfx::B2DRange aRange(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom());

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(
                                basegfx::tools::createPolygonFromEllipse(
                                    aRange.getCenter(),
                                    aRange.getWidth() * 0.5,
                                    aRange.getHeight() * 0.5)),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case META_ARC_ACTION :
                {
                    const MetaArcAction* pA = static_cast< const MetaArcAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const Polygon aToolsPoly(
                            rRect,
                            pA->GetStartPoint(),
                            pA->GetEndPoint(),
                            POLY_ARC);

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aToolsPoly.getB2DPolygon()),
                            aTarget,
                            true); // stroke
                    }
                    break;
                }

                case META_PIE_ACTION :
                {
                    const MetaPieAction* pA = static_cast< const MetaPieAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const Polygon aToolsPoly(
                            rRect,
                            pA->GetStartPoint(),
                            pA->GetEndPoint(),
                            POLY_PIE);

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aToolsPoly.getB2DPolygon()),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case META_CHORD_ACTION :
                {
                    const MetaChordAction* pA = static_cast< const MetaChordAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        const Polygon aToolsPoly(
                            rRect,
                            pA->GetStartPoint(),
                            pA->GetEndPoint(),
                            POLY_CHORD);

                        bDone = handleGeometricContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(aToolsPoly.getB2DPolygon()),
                            aTarget,
                            false); // stroke
                    }
                    break;
                }

                case META_POLYLINE_ACTION :
                {
                    const MetaPolyLineAction* pA = static_cast< const MetaPolyLineAction* >(pAction);

                    bDone = handleGeometricContent(
                        aClips.back(),
                        basegfx::B2DPolyPolygon(pA->GetPolygon().getB2DPolygon()),
                        aTarget,
                        true); // stroke
                    break;
                }

                case META_POLYGON_ACTION :
                {
                    const MetaPolygonAction* pA = static_cast< const MetaPolygonAction* >(pAction);

                    bDone = handleGeometricContent(
                        aClips.back(),
                        basegfx::B2DPolyPolygon(pA->GetPolygon().getB2DPolygon()),
                        aTarget,
                        false); // stroke
                    break;
                }

                case META_POLYPOLYGON_ACTION :
                {
                    const MetaPolyPolygonAction* pA = static_cast< const MetaPolyPolygonAction* >(pAction);
                    const PolyPolygon& rPoly = pA->GetPolyPolygon();

                    bDone = handleGeometricContent(
                        aClips.back(),
                        rPoly.getB2DPolyPolygon(),
                        aTarget,
                        false); // stroke
                    break;
                }

                // bitmap actions, create BitmapEx with alpha channel derived
                // from clipping

                case META_BMPEX_ACTION :
                {
                    const MetaBmpExAction* pA = static_cast< const MetaBmpExAction* >(pAction);
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    // the logical size depends on the PrefSize of the given bitmap in
                    // combination with the current MapMode
                    Size aLogicalSize(rBitmapEx.GetPrefSize());

                    if(MAP_PIXEL == rBitmapEx.GetPrefMapMode().GetMapUnit())
                    {
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aLogicalSize, aMapModes.back().GetMapUnit());
                    }
                    else
                    {
                        aLogicalSize = OutputDevice::LogicToLogic(aLogicalSize, rBitmapEx.GetPrefMapMode(), aMapModes.back().GetMapUnit());
                    }

                    bDone = handleBitmapContent(
                        aClips.back(),
                        pA->GetPoint(),
                        aLogicalSize,
                        rBitmapEx,
                        aTarget);
                    break;
                }

                case META_BMP_ACTION :
                {
                    const MetaBmpAction* pA = static_cast< const MetaBmpAction* >(pAction);
                    const Bitmap& rBitmap = pA->GetBitmap();

                    // the logical size depends on the PrefSize of the given bitmap in
                    // combination with the current MapMode
                    Size aLogicalSize(rBitmap.GetPrefSize());

                    if(MAP_PIXEL == rBitmap.GetPrefMapMode().GetMapUnit())
                    {
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic(aLogicalSize, aMapModes.back().GetMapUnit());
                    }
                    else
                    {
                        aLogicalSize = OutputDevice::LogicToLogic(aLogicalSize, rBitmap.GetPrefMapMode(), aMapModes.back().GetMapUnit());
                    }

                    bDone = handleBitmapContent(
                        aClips.back(),
                        pA->GetPoint(),
                        aLogicalSize,
                        BitmapEx(rBitmap),
                        aTarget);
                    break;
                }

                case META_BMPEXSCALE_ACTION :
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

                case META_BMPSCALE_ACTION :
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

                case META_BMPEXSCALEPART_ACTION :
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
                        const Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

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

                case META_BMPSCALEPART_ACTION :
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
                        const Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

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

                case META_COMMENT_ACTION :
                {
                    const MetaCommentAction* pA = static_cast< const MetaCommentAction* >(pAction);
                    const OString& rComment = pA->GetComment();

                    if(rComment.equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
                    {
                        // nothing to do; this just means that between here and XGRAD_SEQ_END
                        // exists a META_GRADIENTEX_ACTION mixed with Xor-tricked painiting
                        // commands. This comment is used to scan over these and filter for
                        // the gradient action. It is needed to support META_GRADIENTEX_ACTION
                        // in this processor to solve usages.
                    }
                    else if(rComment.equalsIgnoreAsciiCase("XPATHFILL_SEQ_BEGIN"))
                    {
                        SvtGraphicFill aFilling;
                        PolyPolygon aPath;

                        {   // read SvtGraphicFill
                            SvMemoryStream aMemStm((void*)pA->GetData(), pA->GetDataSize(),STREAM_READ);
                            ReadSvtGraphicFill( aMemStm, aFilling );
                        }

                        aFilling.getPath(aPath);

                        if(aPath.Count())
                        {
                            const basegfx::B2DPolyPolygon aSource(aPath.getB2DPolyPolygon());
                            const basegfx::B2DPolyPolygon aResult(
                                basegfx::tools::clipPolyPolygonOnPolyPolygon(
                                    aSource,
                                    aClips.back(),
                                    true, // inside
                                    false)); // stroke

                            if(aResult.count())
                            {
                                if(aResult != aSource)
                                {
                                    // add clipped geometry
                                    aFilling.setPath(PolyPolygon(aResult));
                                    addSvtGraphicFill(aFilling, aTarget);
                                    bDone = true;
                                }
                            }
                            else
                            {
                                // exchange with empty polygon
                                aFilling.setPath(PolyPolygon());
                                addSvtGraphicFill(aFilling, aTarget);
                                bDone = true;
                            }
                        }
                    }
                    else if(rComment.equalsIgnoreAsciiCase("XPATHSTROKE_SEQ_BEGIN"))
                    {
                        SvtGraphicStroke aStroke;
                        Polygon aPath;

                        {   // read SvtGraphicFill
                            SvMemoryStream aMemStm((void*)pA->GetData(), pA->GetDataSize(),STREAM_READ);
                            ReadSvtGraphicStroke( aMemStm, aStroke );
                        }

                        aStroke.getPath(aPath);

                        if(aPath.GetSize())
                        {
                            const basegfx::B2DPolygon aSource(aPath.getB2DPolygon());
                            const basegfx::B2DPolyPolygon aResult(
                                basegfx::tools::clipPolygonOnPolyPolygon(
                                    aSource,
                                    aClips.back(),
                                    true, // inside
                                    true)); // stroke

                            if(aResult.count())
                            {
                                if(aResult.count() > 1 || aResult.getB2DPolygon(0) != aSource)
                                {
                                    // add clipped geometry
                                    for(sal_uInt32 a(0); a < aResult.count(); a++)
                                    {
                                        aStroke.setPath(Polygon(aResult.getB2DPolygon(a)));
                                        addSvtGraphicStroke(aStroke, aTarget);
                                    }

                                    bDone = true;
                                }
                            }
                            else
                            {
                                // exchange with empty polygon
                                aStroke.setPath(Polygon());
                                addSvtGraphicStroke(aStroke, aTarget);
                                bDone = true;
                            }

                        }
                    }
                    break;
                }

                // need to handle gradient fills (hopefully only unroated ones)

                case META_GRADIENT_ACTION :
                {
                    const MetaGradientAction* pA = static_cast< const MetaGradientAction* >(pAction);
                    const Rectangle& rRect = pA->GetRect();

                    if(rRect.IsEmpty())
                    {
                        bDone = true;
                    }
                    else
                    {
                        bDone = handleGradientContent(
                            aClips.back(),
                            basegfx::B2DPolyPolygon(
                                basegfx::tools::createPolygonFromRect(
                                    basegfx::B2DRange(
                                        rRect.Left(), rRect.Top(),
                                        rRect.Right(), rRect.Bottom()))),
                            pA->GetGradient(),
                            aTarget);
                    }

                    break;
                }

                case META_GRADIENTEX_ACTION :
                {
                    const MetaGradientExAction* pA = static_cast< const MetaGradientExAction* >(pAction);
                    const PolyPolygon& rPolyPoly = pA->GetPolyPolygon();

                    bDone = handleGradientContent(
                        aClips.back(),
                        rPolyPoly.getB2DPolyPolygon(),
                        pA->GetGradient(),
                        aTarget);
                    break;
                }

                // not (yet) supported actions

                // META_NULL_ACTION
                // META_TEXT_ACTION
                // META_TEXTARRAY_ACTION
                // META_STRETCHTEXT_ACTION
                // META_TEXTRECT_ACTION
                // META_MASK_ACTION
                // META_MASKSCALE_ACTION
                // META_MASKSCALEPART_ACTION
                // META_HATCH_ACTION
                // META_WALLPAPER_ACTION
                // META_FILLCOLOR_ACTION
                // META_TEXTCOLOR_ACTION
                // META_TEXTFILLCOLOR_ACTION
                // META_TEXTALIGN_ACTION
                // META_MAPMODE_ACTION
                // META_FONT_ACTION
                // META_TRANSPARENT_ACTION
                // META_EPS_ACTION
                // META_REFPOINT_ACTION
                // META_TEXTLINECOLOR_ACTION
                // META_TEXTLINE_ACTION
                // META_FLOATTRANSPARENT_ACTION
                // META_LAYOUTMODE_ACTION
                // META_TEXTLANGUAGE_ACTION
                // META_OVERLINECOLOR_ACTION

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
            const_cast< MetaAction* >(pAction)->Duplicate();
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

bool VCL_DLLPUBLIC usesClipActions(const GDIMetaFile& rSource)
{
    const sal_uLong nObjCount(rSource.GetActionSize());

    for(sal_uLong i(0); i < nObjCount; ++i)
    {
        const MetaAction* pAction(rSource.GetAction(i));
        const sal_uInt16 nType(pAction->GetType());

        switch(nType)
        {
            case META_CLIPREGION_ACTION :
            case META_ISECTRECTCLIPREGION_ACTION :
            case META_ISECTREGIONCLIPREGION_ACTION :
            case META_MOVECLIPREGION_ACTION :
            {
                return true;
            }

            default: break;
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
