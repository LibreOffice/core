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

#include <svx/sdr/overlay/overlayselection.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHairlinePrimitive2D.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <officecfg/Office/Common.hxx>
#include <o3tl/sorted_vector.hxx>
#include <map>

namespace
{
struct B2DPointCompare
{
    bool operator()(const basegfx::B2DPoint& lhs, const basegfx::B2DPoint& rhs) const
    {
        return std::make_pair(lhs.getX(), lhs.getY()) < std::make_pair(rhs.getX(), rhs.getY());
    }
};

struct B2DPointCompareYThenX
{
    bool operator()(const basegfx::B2DPoint& lhs, const basegfx::B2DPoint& rhs) const
    {
        return std::make_pair(lhs.getY(), lhs.getX()) < std::make_pair(rhs.getY(), rhs.getX());
    }
};

}

namespace sdr::overlay
{

    // Combine rectangles geometrically to a single OR'ed polygon.
    // Algorithm is from
    //     "Uniqueness of orthogonal connect-the-dots" Joseph O'Rourke 1988
    // The basic algorithm is:
    //   Sort points by lowest x, lowest y
    //   Go through each column and create edges between the vertices 2i and 2i + 1 in that column
    //   Sort points by lowest y, lowest x
    //   Go through each row and create edges between the vertices 2i and 2i + 1 in that row.
    //
    static basegfx::B2DPolyPolygon impCombineRectanglesToPolyPolygon(const std::vector< basegfx::B2DRange >& rRectangles)
    {
        o3tl::sorted_vector<basegfx::B2DPoint, B2DPointCompare> sort_x;
        for (auto const & rRect : rRectangles)
        {
            auto checkPoint = [&sort_x](double x, double y)
            {
                basegfx::B2DPoint pt(x, y);
                auto it = sort_x.find(pt);
                if (it != sort_x.end()) // Shared vertice, remove it.
                    sort_x.erase(it);
                else
                    sort_x.insert(pt);
            };
            checkPoint(rRect.getMinX(), rRect.getMinY());
            checkPoint(rRect.getMinX(), rRect.getMaxY());
            checkPoint(rRect.getMaxX(), rRect.getMinY());
            checkPoint(rRect.getMaxX(), rRect.getMaxY());
        }


        o3tl::sorted_vector<basegfx::B2DPoint, B2DPointCompareYThenX> sort_y;
        for (auto const & i : sort_x)
            sort_y.insert(i);

        std::map<basegfx::B2DPoint, basegfx::B2DPoint, B2DPointCompare> edges_h;
        std::map<basegfx::B2DPoint, basegfx::B2DPoint, B2DPointCompare> edges_v;

        size_t i = 0;
        while (i < sort_x.size())
        {
            auto curr_y = sort_y[i].getY();
            while (i < sort_x.size() && sort_y[i].getY() == curr_y)
            {
                edges_h[sort_y[i]] = sort_y[i + 1];
                edges_h[sort_y[i + 1]] = sort_y[i];
                i += 2;
            }
        }
        i = 0;
        while (i < sort_x.size())
        {
            auto curr_x = sort_x[i].getX();
            while (i < sort_x.size() && sort_x[i].getX() == curr_x)
            {
                edges_v[sort_x[i]] = sort_x[i + 1];
                edges_v[sort_x[i + 1]] = sort_x[i];
                i += 2;
            }
        }

        // Get all the polygons.
        basegfx::B2DPolyPolygon aPolyPolygon;
        std::vector<std::tuple<basegfx::B2DPoint, bool>> tmpPolygon;
        while (!edges_h.empty())
        {
            tmpPolygon.clear();
            // We can start with any point.
            basegfx::B2DPoint pt = edges_h.begin()->first;
            edges_h.erase(edges_h.begin());
            tmpPolygon.push_back({pt, false});
            for (;;)
            {
                auto [curr, e] = tmpPolygon.back();
                if (!e)
                {
                    auto it = edges_v.find(curr);
                    auto next_vertex = it->second;
                    edges_v.erase(it);
                    tmpPolygon.push_back({next_vertex, true});
                }
                else
                {
                    auto it = edges_h.find(curr);
                    auto next_vertex = it->second;
                    edges_h.erase(it);
                    tmpPolygon.push_back({next_vertex, false});
                }
                if (tmpPolygon.back() == tmpPolygon.front())
                {
                    // Closed polygon
                    break;
                }
            }
            for (auto const & pair : tmpPolygon)
            {
                auto const & vertex = std::get<0>(pair);
                edges_h.erase(vertex);
                edges_v.erase(vertex);
            }
            basegfx::B2DPolygon aPoly;
            aPoly.reserve(tmpPolygon.size());
            for (auto const & pair : tmpPolygon)
                aPoly.append(std::get<0>(pair));
            aPolyPolygon.append(std::move(aPoly));
        }

        return aPolyPolygon;
    }

        // check if wanted type OverlayType::Transparent or OverlayType::Solid
        // is possible. If not, fallback to invert mode (classic mode)
        static OverlayType impCheckPossibleOverlayType(OverlayType aOverlayType)
        {
            if(OverlayType::Invert != aOverlayType)
            {
                if(!officecfg::Office::Common::Drawinglayer::TransparentSelection::get())
                {
                    // not possible when switched off by user
                    return OverlayType::Invert;
                }
                else if (const OutputDevice* pOut = Application::GetDefaultDevice())
                {

                    if(pOut->GetSettings().GetStyleSettings().GetHighContrastMode())
                    {
                        // not possible when in high contrast mode
                        return  OverlayType::Invert;
                    }

                    if(!pOut->SupportsOperation(OutDevSupportType::TransparentRect))
                    {
                        // not possible when no fast transparence paint is supported on the system
                        return OverlayType::Invert;
                    }
                }
            }

            return aOverlayType;
        }

        drawinglayer::primitive2d::Primitive2DContainer OverlaySelection::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DContainer aRetval;
            const sal_uInt32 nCount(maRanges.size());

            if(!nCount)
                return aRetval;

            // create range primitives
            const bool bInvert(OverlayType::Invert == maLastOverlayType);
            basegfx::BColor aRGBColor(getBaseColor().getBColor());
            if(bInvert)
            {
                // force color to white for invert to get a full invert
                aRGBColor = basegfx::BColor(1.0, 1.0, 1.0);
            }

            basegfx::B2DPolyPolygon aPolyPolygon;
            aPolyPolygon.reserve(nCount);
            for(sal_uInt32 a(0);a < nCount; a++)
                aPolyPolygon.append(basegfx::utils::createPolygonFromRect(maRanges[a]));
            aRetval.append(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    std::move(aPolyPolygon),
                    aRGBColor));

            if(bInvert)
            {
                // embed all in invert primitive
                aRetval = drawinglayer::primitive2d::Primitive2DContainer {
                        new drawinglayer::primitive2d::InvertPrimitive2D(
                            std::move(aRetval))
                };
            }
            else if(OverlayType::Transparent == maLastOverlayType)
            {
                // embed all rectangles in transparent paint
                const double fTransparence(mnLastTransparence / 100.0);
                drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparence(
                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                        std::move(aRetval),
                        fTransparence));

                if(mbBorder)
                {
                    drawinglayer::primitive2d::Primitive2DReference aSelectionOutline(
                        new drawinglayer::primitive2d::PolyPolygonHairlinePrimitive2D(
                            impCombineRectanglesToPolyPolygon(maRanges),
                            aRGBColor));

                    // add both to result
                    aRetval = drawinglayer::primitive2d::Primitive2DContainer { aUnifiedTransparence, aSelectionOutline };
                }
                else
                {
                    // just add transparent part
                    aRetval = drawinglayer::primitive2d::Primitive2DContainer { aUnifiedTransparence };
                }
            }

            return aRetval;
        }

        OverlaySelection::OverlaySelection(
            OverlayType eType,
            const Color& rColor,
            std::vector< basegfx::B2DRange >&& rRanges,
            bool bBorder)
        :   OverlayObject(rColor),
            meOverlayType(eType),
            maRanges(std::move(rRanges)),
            maLastOverlayType(eType),
            mnLastTransparence(0),
            mbBorder(bBorder)
        {
            // no AA for selection overlays
            allowAntiAliase(false);
        }

        OverlaySelection::~OverlaySelection()
        {
            if(getOverlayManager())
            {
                getOverlayManager()->remove(*this);
            }
        }

        drawinglayer::primitive2d::Primitive2DContainer OverlaySelection::getOverlayObjectPrimitive2DSequence() const
        {
            // get current values
            const OverlayType aNewOverlayType(impCheckPossibleOverlayType(meOverlayType));
            const sal_uInt16 nNewTransparence(SvtOptionsDrawinglayer::GetTransparentSelectionPercent());

            if(!getPrimitive2DSequence().empty())
            {
                if(aNewOverlayType != maLastOverlayType
                    || nNewTransparence != mnLastTransparence)
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< OverlaySelection* >(this)->resetPrimitive2DSequence();
                }
            }

            if(getPrimitive2DSequence().empty())
            {
                // remember new values
                const_cast< OverlaySelection* >(this)->maLastOverlayType = aNewOverlayType;
                const_cast< OverlaySelection* >(this)->mnLastTransparence = nNewTransparence;
            }

            // call base implementation
            return OverlayObject::getOverlayObjectPrimitive2DSequence();
        }

        void OverlaySelection::setRanges(std::vector< basegfx::B2DRange >&& rNew)
        {
            if(rNew != maRanges)
            {
                maRanges = std::move(rNew);
                objectChange();
            }
        }
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
