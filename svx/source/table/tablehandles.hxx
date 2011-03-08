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

#ifndef _SVX_TABLEHANDLES_HXX
#define _SVX_TABLEHANDLES_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

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

    sal_Int32 GetValidDragOffset( const SdrDragStat& rDrag ) const;

    virtual Pointer GetPointer() const;

    void SetEdge( sal_Int32 nEdge, sal_Int32 nStart, sal_Int32 nEnd, TableEdgeState nState );

    bool IsHorizontalEdge() const { return mbHorizontal; }

    basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;
    void getPolyPolygon(basegfx::B2DPolyPolygon& rVisible, basegfx::B2DPolyPolygon& rInvisible, const SdrDragStat* pDrag) const;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject();

private:
    bool mbHorizontal;
    sal_Int32 mnMin, mnMax;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
