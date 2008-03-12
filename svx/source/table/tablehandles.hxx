/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablehandles.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:04:33 $
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

#ifndef _SVX_TABLEHANDLES_HXX
#define _SVX_TABLEHANDLES_HXX

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#include <svx/svdhdl.hxx>

// --------------------------------------------------------------------

namespace sdr { namespace table {

enum TableEdgeState { Empty, Invisible, Visible };

struct TableEdge
{
    sal_Int32 mnStart;
    sal_Int32 mnEnd;
    TableEdgeState meState;

    TableEdge() : mnStart(0), mnEnd(0), meState(Empty) {}
};

typedef std::vector< TableEdge > TableEdgeVector;

class TableEdgeHdl : public SdrHdl
{
public:
    TableEdgeHdl( const Point& rPnt, bool bHorizontal, sal_Int32 nMin, sal_Int32 nMax, sal_Int32 nEdges );

    virtual bool IsHdlHit(const Point& rPnt) const;

    sal_Int32 GetValidDragOffset( const SdrDragStat& rDrag ) const;

    virtual Pointer GetPointer() const;

    void SetEdge( sal_Int32 nEdge, sal_Int32 nStart, sal_Int32 nEnd, TableEdgeState nState );

    bool IsHorizontalEdge() const { return mbHorizontal; }

    basegfx::B2DPolyPolygon TakeDragPoly( const SdrDragStat* pDrag = 0 ) const;
    basegfx::B2DPolyPolygon GetPolyPolygon( bool bOnlyVisible, const SdrDragStat* pDrag = 0 ) const;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject();

private:
    bool mbHorizontal;
    sal_Int32 mnMin, mnMax;
    basegfx::B2DPolyPolygon maVisiblePolygon;
    TableEdgeVector maEdges;
};

class TableBorderHdl : public SdrHdl
{
public:
    TableBorderHdl( const Rectangle& rRect );

    virtual Pointer GetPointer() const;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject();

private:
    Rectangle maRectangle;
};

} // end of namespace table
} // end of namespace sdr

#endif
