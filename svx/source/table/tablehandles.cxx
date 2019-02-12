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


#include "tablehandles.hxx"

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/hatch.hxx>
#include <vcl/ptrstyle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <sdr/overlay/overlayrectangle.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>

namespace sdr { namespace table {


class OverlayTableEdge : public sdr::overlay::OverlayObject
{
protected:
    basegfx::B2DPolyPolygon const maPolyPolygon;
    bool const                    mbVisible;

    // geometry creation for OverlayObject
    virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

public:
    OverlayTableEdge( const basegfx::B2DPolyPolygon& rPolyPolygon, bool bVisible );
};


TableEdgeHdl::TableEdgeHdl( const Point& rPnt, bool bHorizontal, sal_Int32 nMin, sal_Int32 nMax, sal_Int32 nEdges )
: SdrHdl( rPnt, SdrHdlKind::User )
, mbHorizontal( bHorizontal )
, mnMin( nMin )
, mnMax( nMax )
, maEdges(nEdges)
{
}

void TableEdgeHdl::SetEdge( sal_Int32 nEdge, sal_Int32 nStart, sal_Int32 nEnd, TableEdgeState eState )
{
    if( (nEdge >= 0) && (nEdge <= sal::static_int_cast<sal_Int32>(maEdges.size())) )
    {
        maEdges[nEdge].mnStart = nStart;
        maEdges[nEdge].mnEnd = nEnd;
        maEdges[nEdge].meState = eState;
    }
    else
    {
        OSL_FAIL( "sdr::table::TableEdgeHdl::SetEdge(), invalid edge!" );
    }
}

PointerStyle TableEdgeHdl::GetPointer() const
{
    if( mbHorizontal )
        return PointerStyle::VSplit;
    else
        return PointerStyle::HSplit;
}

sal_Int32 TableEdgeHdl::GetValidDragOffset( const SdrDragStat& rDrag ) const
{
    return std::min( std::max( static_cast<sal_Int32>(mbHorizontal ? rDrag.GetDY() : rDrag.GetDX()), mnMin ), mnMax );
}

basegfx::B2DPolyPolygon TableEdgeHdl::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aVisible;
    basegfx::B2DPolyPolygon aInvisible;

    // create and return visible and non-visible parts for drag
    getPolyPolygon(aVisible, aInvisible, &rDrag);
    aVisible.append(aInvisible);

    return aVisible;
}

void TableEdgeHdl::getPolyPolygon(basegfx::B2DPolyPolygon& rVisible, basegfx::B2DPolyPolygon& rInvisible, const SdrDragStat* pDrag) const
{
    // changed method to create visible and invisible partial polygons in one run in
    // separate PolyPolygons; both kinds are used
    basegfx::B2DPoint aOffset(aPos.X(), aPos.Y());
    rVisible.clear();
    rInvisible.clear();

    if( pDrag )
    {
        int n = mbHorizontal ? 1 : 0;
        aOffset[n] = aOffset[n] + GetValidDragOffset( *pDrag );
    }

    basegfx::B2DPoint aStart(aOffset), aEnd(aOffset);
    int nPos = mbHorizontal ? 0 : 1;

    for( const TableEdge& aEdge : maEdges )
    {
        aStart[nPos] = aOffset[nPos] + aEdge.mnStart;
        aEnd[nPos] = aOffset[nPos] + aEdge.mnEnd;

        basegfx::B2DPolygon aPolygon;
        aPolygon.append( aStart );
        aPolygon.append( aEnd );

        if(aEdge.meState == Visible)
        {
            rVisible.append(aPolygon);
        }
        else
        {
            rInvisible.append(aPolygon);
        }
    }
}

void TableEdgeHdl::CreateB2dIAObject()
{
    GetRidOfIAObject();

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->areMarkHandlesHidden())
    {
        SdrMarkView* pView = pHdlList->GetView();
        SdrPageView* pPageView = pView->GetSdrPageView();

        if(pPageView)
        {
            basegfx::B2DPolyPolygon aVisible;
            basegfx::B2DPolyPolygon aInvisible;

            // get visible and invisible parts
            getPolyPolygon(aVisible, aInvisible, nullptr);

            if(aVisible.count() || aInvisible.count())
            {
                for(sal_uInt32 nWindow = 0; nWindow < pPageView->PageWindowCount(); nWindow++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(nWindow);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            if(aVisible.count())
                            {
                                // create overlay object for visible parts
                                std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject(new OverlayTableEdge(aVisible, true));

                                // OVERLAYMANAGER
                                insertNewlyCreatedOverlayObjectForSdrHdl(
                                    std::move(pOverlayObject),
                                    rPageWindow.GetObjectContact(),
                                    *xManager);
                            }

                            if(aInvisible.count())
                            {
                                // also create overlay object for invisible parts to allow
                                // a standard HitTest using the primitives from that overlay object
                                // (see OverlayTableEdge implementation)
                                std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject(new OverlayTableEdge(aInvisible, false));

                                // OVERLAYMANAGER
                                insertNewlyCreatedOverlayObjectForSdrHdl(
                                    std::move(pOverlayObject),
                                    rPageWindow.GetObjectContact(),
                                    *xManager);
                            }
                        }
                    }
                }
            }
        }
    }
}


OverlayTableEdge::OverlayTableEdge( const basegfx::B2DPolyPolygon& rPolyPolygon, bool bVisible )
:   OverlayObject(COL_GRAY)
,   maPolyPolygon( rPolyPolygon )
,   mbVisible(bVisible)
{
}

drawinglayer::primitive2d::Primitive2DContainer OverlayTableEdge::createOverlayObjectPrimitive2DSequence()
{
    drawinglayer::primitive2d::Primitive2DContainer aRetval;

    if(maPolyPolygon.count())
    {
        // Discussed with CL. Currently i will leave the transparence out since this
        // a little bit expensive. We may check the look with drag polygons later
        const drawinglayer::primitive2d::Primitive2DReference aReference(
            new drawinglayer::primitive2d::PolyPolygonHairlinePrimitive2D(
                maPolyPolygon,
                getBaseColor().getBColor()));

        if(mbVisible)
        {
            // visible, just return as sequence
            aRetval = drawinglayer::primitive2d::Primitive2DContainer { aReference };
        }
        else
        {
            // embed in HiddenGeometryPrimitive2D to support HitTest of this invisible
            // overlay object
            const drawinglayer::primitive2d::Primitive2DContainer aSequence { aReference };
            const drawinglayer::primitive2d::Primitive2DReference aNewReference(
                new drawinglayer::primitive2d::HiddenGeometryPrimitive2D(aSequence));
            aRetval = drawinglayer::primitive2d::Primitive2DContainer { aNewReference };
        }
    }

    return aRetval;
}


TableBorderHdl::TableBorderHdl(
    const tools::Rectangle& rRect,
    bool bAnimate)
:   SdrHdl(rRect.TopLeft(), SdrHdlKind::Move),
    maRectangle(rRect),
    mbAnimate(bAnimate)
{
}

PointerStyle TableBorderHdl::GetPointer() const
{
    return PointerStyle::Move;
}

// create marker for this kind
void TableBorderHdl::CreateB2dIAObject()
{
    GetRidOfIAObject();

    if (pHdlList && pHdlList->GetView() && !pHdlList->GetView()->areMarkHandlesHidden())
    {
        SdrMarkView* pView = pHdlList->GetView();
        SdrPageView* pPageView = pView->GetSdrPageView();

        if (!pPageView)
            return;

        for(sal_uInt32 nWindow = 0; nWindow < pPageView->PageWindowCount(); nWindow++)
        {
            const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(nWindow);

            if (rPageWindow.GetPaintWindow().OutputToWindow())
            {
                const rtl::Reference<sdr::overlay::OverlayManager>& xManager = rPageWindow.GetOverlayManager();

                if (xManager.is())
                {
                    const basegfx::B2DRange aRange(vcl::unotools::b2DRectangleFromRectangle(maRectangle));
                    const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                    const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
                    const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
                    // make animation dependent from text edit active, because for tables
                    // this handle is also used when text edit *is* active for it. This
                    // interferes too much concerning repaint stuff (at least as long as
                    // text edit is not yet on the overlay)
                    const bool bAnimate = getAnimate();

                    OutputDevice& rOutDev = rPageWindow.GetPaintWindow().GetOutputDevice();
                    float fScaleFactor = rOutDev.GetDPIScaleFactor();
                    double fWidth = fScaleFactor * 6.0;

                    std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject(
                        new sdr::overlay::OverlayRectangle(aRange.getMinimum(), aRange.getMaximum(),
                                                           aHilightColor, fTransparence,
                                                           fWidth, 0.0, 0.0, bAnimate));

                    // OVERLAYMANAGER
                    insertNewlyCreatedOverlayObjectForSdrHdl(
                        std::move(pOverlayObject),
                        rPageWindow.GetObjectContact(),
                        *xManager);
                }
            }
        }
    }
}


} // end of namespace table
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
