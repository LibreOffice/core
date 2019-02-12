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

#ifndef INCLUDED_SVX_SOURCE_TABLE_TABLEHANDLES_HXX
#define INCLUDED_SVX_SOURCE_TABLE_TABLEHANDLES_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

#include <svx/svdhdl.hxx>


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

    virtual PointerStyle GetPointer() const override;

    void SetEdge( sal_Int32 nEdge, sal_Int32 nStart, sal_Int32 nEnd, TableEdgeState nState );

    bool IsHorizontalEdge() const { return mbHorizontal; }

    basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;
    void getPolyPolygon(basegfx::B2DPolyPolygon& rVisible, basegfx::B2DPolyPolygon& rInvisible, const SdrDragStat* pDrag) const;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject() override;

private:
    bool const mbHorizontal;
    sal_Int32 mnMin, mnMax;
    TableEdgeVector maEdges;
};

class TableBorderHdl : public SdrHdl
{
public:
    TableBorderHdl(
        const tools::Rectangle& rRect,
        bool bAnimate);

    virtual PointerStyle GetPointer() const override;
    bool getAnimate() const { return mbAnimate; }

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject() override;

private:
    tools::Rectangle const maRectangle;

    bool const            mbAnimate : 1;
};

} // end of namespace table
} // end of namespace sdr

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
