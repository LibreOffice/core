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

#define MAX_64KPOINTS       ((((USHORT)0xFFFF)-32)/sizeof(Point))

// -------------------
// - ImplPolygonData -
// -------------------

class ImplPolygonData
{
public:
    Point*          mpPointAry;
    BYTE*           mpFlagAry;
    USHORT          mnPoints;
    ULONG           mnRefCount;
};

// ---------------
// - ImplPolygon -
// ---------------

class ImplPolygon  : public ImplPolygonData
{
public:
                    ImplPolygon( USHORT nInitSize, BOOL bFlags = FALSE );
                    ImplPolygon( USHORT nPoints, const Point* pPtAry, const BYTE* pInitFlags = NULL );
                    ImplPolygon( const ImplPolygon& rImplPoly );
                    ~ImplPolygon();

    void            ImplSetSize( USHORT nSize, BOOL bResize = TRUE );
    void            ImplCreateFlagArray();
    void            ImplSplit( USHORT nPos, USHORT nSpace, ImplPolygon* pInitPoly = NULL );
    void            ImplRemove( USHORT nPos, USHORT nCount );
};

// -------------------
// - ImplPolyPolygon -
// -------------------

#define MAX_POLYGONS        ((USHORT)0x3FF0)

class Polygon;
typedef Polygon* SVPPOLYGON;

class ImplPolyPolygon
{
public:
    SVPPOLYGON*     mpPolyAry;
    ULONG           mnRefCount;
    USHORT          mnCount;
    USHORT          mnSize;
    USHORT          mnResize;

                    ImplPolyPolygon( USHORT nInitSize, USHORT nResize )
                        { mpPolyAry = NULL; mnCount = 0; mnRefCount = 1;
                          mnSize = nInitSize; mnResize = nResize; }
                    ImplPolyPolygon( USHORT nInitSize );
                    ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly );
                    ~ImplPolyPolygon();
};

#endif // _SV_POLY_H
