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

#ifndef _POLY_H
#define _POLY_H

#include <tools/gen.hxx>

#define MAX_64KPOINTS       ((((sal_uInt16)0xFFFF)-32)/sizeof(Point))

// -------------------
// - ImplPolygonData -
// -------------------

class ImplPolygonData
{
public:
    Point*          mpPointAry;
    sal_uInt8*           mpFlagAry;
    sal_uInt16          mnPoints;
    sal_uIntPtr           mnRefCount;
};

// ---------------
// - ImplPolygon -
// ---------------

class ImplPolygon  : public ImplPolygonData
{
public:
                    ImplPolygon( sal_uInt16 nInitSize, sal_Bool bFlags = sal_False );
                    ImplPolygon( sal_uInt16 nPoints, const Point* pPtAry, const sal_uInt8* pInitFlags = NULL );
                    ImplPolygon( const ImplPolygon& rImplPoly );
                    ~ImplPolygon();

    void            ImplSetSize( sal_uInt16 nSize, sal_Bool bResize = sal_True );
    void            ImplCreateFlagArray();
    void            ImplSplit( sal_uInt16 nPos, sal_uInt16 nSpace, ImplPolygon* pInitPoly = NULL );
    void            ImplRemove( sal_uInt16 nPos, sal_uInt16 nCount );
};

// -------------------
// - ImplPolyPolygon -
// -------------------

#define MAX_POLYGONS        ((sal_uInt16)0x3FF0)

class Polygon;
typedef Polygon* SVPPOLYGON;

class ImplPolyPolygon
{
public:
    SVPPOLYGON*     mpPolyAry;
    sal_uIntPtr           mnRefCount;
    sal_uInt16          mnCount;
    sal_uInt16          mnSize;
    sal_uInt16          mnResize;

                    ImplPolyPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize )
                        { mpPolyAry = NULL; mnCount = 0; mnRefCount = 1;
                          mnSize = nInitSize; mnResize = nResize; }
                    ImplPolyPolygon( sal_uInt16 nInitSize );
                    ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly );
                    ~ImplPolyPolygon();
};

#endif // _SV_POLY_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
