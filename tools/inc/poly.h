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
#ifndef INCLUDED_TOOLS_INC_POLY_H
#define INCLUDED_TOOLS_INC_POLY_H

#include <sal/types.h>
#include <memory>
#include <tools/poly.hxx>

class SAL_WARN_UNUSED ImplPolygon
{
public:
    std::unique_ptr<Point[]> mxPointAry;
    std::unique_ptr<PolyFlags[]> mxFlagAry;
    sal_uInt16      mnPoints;

public:
                    ImplPolygon() : mnPoints(0) {}
                    ImplPolygon( sal_uInt16 nInitSize );
                    ImplPolygon( sal_uInt16 nPoints, const Point* pPtAry, const PolyFlags* pInitFlags );
                    ImplPolygon( const ImplPolygon& rImplPoly );
                    ImplPolygon( const tools::Rectangle& rRect );
                    ImplPolygon( const tools::Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound);
                    ImplPolygon( const Point& rCenter, tools::Long nRadX, tools::Long nRadY );
                    ImplPolygon( const tools::Rectangle& rBound, const Point& rStart, const Point& rEnd,
                                    PolyStyle eStyle );
                    ImplPolygon( const Point& rBezPt1, const Point& rCtrlPt1, const Point& rBezPt2,
                                    const Point& rCtrlPt2, sal_uInt16 nPoints );
                    ImplPolygon(const basegfx::B2DPolygon& rPolygon);

    bool            operator==( const ImplPolygon& rCandidate ) const;

    void            ImplInitSize(sal_uInt16 nInitSize, bool bFlags = false);
    void            ImplSetSize( sal_uInt16 nSize, bool bResize = true );
    void            ImplCreateFlagArray();
    bool            ImplSplit( sal_uInt16 nPos, sal_uInt16 nSpace, ImplPolygon const * pInitPoly = nullptr );
};

#define MAX_POLYGONS        SAL_MAX_UINT16

struct ImplPolyPolygon
{
    std::vector<tools::Polygon> mvPolyAry;

    ImplPolyPolygon( sal_uInt16 nInitSize )
    {
       if ( !nInitSize )
            nInitSize = 1;
       mvPolyAry.reserve(nInitSize);
    }

    ImplPolyPolygon( const tools::Polygon& rPoly )
    {
        if ( rPoly.GetSize() )
            mvPolyAry.push_back(rPoly);
        else
            mvPolyAry.reserve(16);
    }

    ImplPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);

    bool operator==(ImplPolyPolygon const & other) const
    {
        return mvPolyAry == other.mvPolyAry;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
