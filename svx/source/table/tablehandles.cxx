/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablehandles.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:04:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "tablehandles.hxx"

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/hatch.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <svx/sdr/overlay/overlayobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpagewindow.hxx>
#include <sdrpaintwindow.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>


namespace sdr { namespace table {

class OverlayTableEdge : public sdr::overlay::OverlayObject
{
protected:
    basegfx::B2DPolyPolygon maPolyPolygon;

    // Draw geometry
    virtual void drawGeometry(OutputDevice& rOutputDevice);

    // Create the BaseRange. This method needs to calculate maBaseRange.
    virtual void createBaseRange(OutputDevice& rOutputDevice);

    virtual void transform(const basegfx::B2DHomMatrix& rMatrix);

public:
    OverlayTableEdge( const basegfx::B2DPolyPolygon& rPolyPolygon );
    virtual ~OverlayTableEdge();
};

// --------------------------------------------------------------------

class OverlayTableBorder : public sdr::overlay::OverlayObject
{
protected:
    basegfx::B2DPolyPolygon maPolyPolygon;

    // Draw geometry
    virtual void drawGeometry(OutputDevice& rOutputDevice);

    // Create the BaseRange. This method needs to calculate maBaseRange.
    virtual void createBaseRange(OutputDevice& rOutputDevice);

    virtual void transform(const basegfx::B2DHomMatrix& rMatrix);

    virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;

public:
    OverlayTableBorder( const basegfx::B2DPolyPolygon& rPolyPolygon );
    virtual ~OverlayTableBorder();
};

// --------------------------------------------------------------------

TableEdgeHdl::TableEdgeHdl( const Point& rPnt, bool bHorizontal, sal_Int32 nMin, sal_Int32 nMax, sal_Int32 nEdges )
: SdrHdl( rPnt, HDL_USER )
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
        OSL_ENSURE( false, "sdr::table::TableEdgeHdl::SetEdge(), invalid edge!" );
    }
}

Pointer TableEdgeHdl::GetPointer() const
{
    if( mbHorizontal )
        return POINTER_VSPLIT;
    else
        return POINTER_HSPLIT;
}

sal_Int32 TableEdgeHdl::GetValidDragOffset( const SdrDragStat& rDrag ) const
{
    return std::min( std::max( mbHorizontal ? rDrag.GetDY() : rDrag.GetDX(), mnMin ), mnMax );
}

basegfx::B2DPolyPolygon TableEdgeHdl::TakeDragPoly( const SdrDragStat* pDrag /*= 0*/ ) const
{
    return GetPolyPolygon(false, pDrag);
}

bool TableEdgeHdl::IsHdlHit(const Point& rPnt) const
{
    if( GetPointNum() != 0 )
    {
        double fTol = 0.0;
        if( pHdlList )
        {
            SdrMarkView* pView = pHdlList->GetView();
            if( pView )
            {
                OutputDevice* pOutDev = pView->GetFirstOutputDevice();
                if( pOutDev )
                {
                    fTol = static_cast<double>(pOutDev->PixelToLogic(Size(3, 3)).Width());
                }
            }
        }

        basegfx::B2DPoint aPosition(rPnt.X(), rPnt.Y());
        if( basegfx::tools::isInEpsilonRange( maVisiblePolygon, aPosition, fTol ) )
            return sal_True;
    }
    return false;
}


basegfx::B2DPolyPolygon TableEdgeHdl::GetPolyPolygon( bool bOnlyVisible, const SdrDragStat* pDrag /*= 0*/ ) const
{
    basegfx::B2DPolyPolygon aRetValue;

    basegfx::B2DPoint aOffset(aPos.X(), aPos.Y());

    if( pDrag )
    {
        int n = mbHorizontal ? 1 : 0;
        aOffset[n] = aOffset[n] + GetValidDragOffset( *pDrag );
    }

    basegfx::B2DPoint aStart(aOffset), aEnd(aOffset);

    int nPos = mbHorizontal ? 0 : 1;

    // base line hit, check for edges
    TableEdgeVector::const_iterator aIter( maEdges.begin() );
    while( aIter != maEdges.end() )
    {
        TableEdge aEdge(*aIter++);

        if( aEdge.meState == Visible || ( aEdge.meState == Invisible && !bOnlyVisible ) )
        {
            aStart[nPos] = aOffset[nPos] + aEdge.mnStart;
            aEnd[nPos] = aOffset[nPos] + aEdge.mnEnd;

            basegfx::B2DPolygon aPolygon;
            aPolygon.append( aStart );
            aPolygon.append( aEnd );

            aRetValue.append( aPolygon );
        }
    }

    return aRetValue;
}

void TableEdgeHdl::CreateB2dIAObject()
{
    GetRidOfIAObject();

    maVisiblePolygon = GetPolyPolygon(false);

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->areMarkHandlesHidden())
    {
        SdrMarkView* pView = pHdlList->GetView();
        SdrPageView* pPageView = pView->GetSdrPageView();

        if(pPageView)
        {
            for(sal_uInt32 nWindow = 0; nWindow < pPageView->PageWindowCount(); nWindow++)
            {
                // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
                const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(nWindow);

                if(rPageWindow.GetPaintWindow().OutputToWindow())
                {
                    if(rPageWindow.GetOverlayManager())
                    {
                        ::sdr::overlay::OverlayObject* pOverlayObject =
                            new OverlayTableEdge( GetPolyPolygon(true) );

                        rPageWindow.GetOverlayManager()->add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

OverlayTableEdge::OverlayTableEdge( const basegfx::B2DPolyPolygon& rPolyPolygon )
:   OverlayObject(Color(COL_GRAY))
,   maPolyPolygon( rPolyPolygon )
{
}

OverlayTableEdge::~OverlayTableEdge()
{
}

void OverlayTableEdge::drawGeometry(OutputDevice& rOutputDevice)
{
    rOutputDevice.SetLineColor(getBaseColor());
    rOutputDevice.SetFillColor();

    for(sal_uInt32 a(0L); a < maPolyPolygon.count();a ++)
    {
/*
        const basegfx::B2DPolygon aPolygon = maPolyPolygon.getB2DPolygon(a);
        const basegfx::B2DPoint aStart(aPolygon.getB2DPoint(0L));
        const basegfx::B2DPoint aEnd(aPolygon.getB2DPoint(aPolygon.count() - 1L));
        const Point aStartPoint(FRound(aStart.getX()), FRound(aStart.getY()));
        const Point aEndPoint(FRound(aEnd.getX()), FRound(aEnd.getY()));
        rOutputDevice.DrawLine(aStartPoint, aEndPoint);
*/
        PolyPolygon aPolyPolygon( maPolyPolygon );
        rOutputDevice.DrawTransparent( aPolyPolygon, 50 );
    }
}

void OverlayTableEdge::transform(const basegfx::B2DHomMatrix& rMatrix)
{
    maPolyPolygon.transform( rMatrix );
}

void OverlayTableEdge::createBaseRange(OutputDevice& /*rOutputDevice*/)
{
    // reset range and expand it
    maBaseRange.reset();

    if(isHittable())
    {
        for(sal_uInt32 a(0L); a < maPolyPolygon.count();a ++)
        {
            const basegfx::B2DPolygon aPolygon = maPolyPolygon.getB2DPolygon(a);
            const basegfx::B2DPoint aStart(aPolygon.getB2DPoint(0L));
            const basegfx::B2DPoint aEnd(aPolygon.getB2DPoint(aPolygon.count() - 1L));
            maBaseRange.expand(aStart);
            maBaseRange.expand(aEnd);
        }
    }
}

// ====================================================================

TableBorderHdl::TableBorderHdl( const Rectangle& rRect )
: SdrHdl( rRect.TopLeft(), HDL_MOVE )
, maRectangle( rRect )
{

}

Pointer TableBorderHdl::GetPointer() const
{
    return POINTER_MOVE;
}

// create marker for this kind
void TableBorderHdl::CreateB2dIAObject()
{
    GetRidOfIAObject();

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->areMarkHandlesHidden())
    {
        SdrMarkView* pView = pHdlList->GetView();
        SdrPageView* pPageView = pView->GetSdrPageView();

        if(pPageView)
        {
            for(sal_uInt32 nWindow = 0; nWindow < pPageView->PageWindowCount(); nWindow++)
            {
                // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
                const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(nWindow);

                if(rPageWindow.GetPaintWindow().OutputToWindow())
                {
                    if(rPageWindow.GetOverlayManager())
                    {
                        OutputDevice& rOutDev = rPageWindow.GetPaintWindow().GetOutputDevice();

                        Size aBorderSize( rOutDev.PixelToLogic( Size( 6, 6 ) ) );
                        basegfx::B2DRectangle aRect( vcl::unotools::b2DRectangleFromRectangle( maRectangle ) );
                        basegfx::B2DPolyPolygon aPolyPolygon;
                        aPolyPolygon.append(basegfx::tools::createPolygonFromRect( aRect ) );
                        aRect.grow( aBorderSize.nA );
                        aPolyPolygon.append(basegfx::tools::createPolygonFromRect( aRect ) );

                        ::sdr::overlay::OverlayObject* pOverlayObject = new OverlayTableBorder( aPolyPolygon );

                        rPageWindow.GetOverlayManager()->add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
                    }
                }
            }
        }
    }
}

OverlayTableBorder::OverlayTableBorder( const basegfx::B2DPolyPolygon& rPolyPolygon )
: OverlayObject(Color(COL_GRAY))
, maPolyPolygon( rPolyPolygon )
{
}

OverlayTableBorder::~OverlayTableBorder()
{
}

// Hittest with logical coordinates
sal_Bool OverlayTableBorder::isHit(const basegfx::B2DPoint& rPos, double /*fTol = 0.0*/) const
{
    if(isHittable())
    {
        if( basegfx::tools::isInside( maPolyPolygon.getB2DPolygon(1), rPos, true ) )
            if( !basegfx::tools::isInside( maPolyPolygon.getB2DPolygon(0), rPos, false ) )
                return sal_True;
    }
    return sal_False;
}

// Draw geometry
void OverlayTableBorder::drawGeometry(OutputDevice& rOutputDevice)
{
    svtools::ColorConfig aColorConfig;
    Color aHatchCol( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    const Hatch aHatch( HATCH_SINGLE, aHatchCol, 3, 450 );
    PolyPolygon aPolyPolygon( maPolyPolygon );
    rOutputDevice.DrawHatch( aPolyPolygon, aHatch );
}

// Create the BaseRange. This method needs to calculate maBaseRange.
void OverlayTableBorder::createBaseRange(OutputDevice& /*rOutputDevice*/)
{
    if(isHittable())
    {
        maBaseRange = basegfx::tools::getRange(maPolyPolygon);
    }
    else
    {
        maBaseRange.reset();
    }
}

void OverlayTableBorder::transform(const basegfx::B2DHomMatrix& rMatrix)
{
    maPolyPolygon.transform( rMatrix );
}

} // end of namespace table
} // end of namespace sdr
