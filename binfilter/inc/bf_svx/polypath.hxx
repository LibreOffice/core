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

#ifndef _POLYPATH_HXX
#define _POLYPATH_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace binfilter {

class XPolygon; // Dummy zum Uebersetzen

enum PathKind {PATH_POLYGON,
               PATH_POLYLINE,
               PATH_BEZIER,
               PATH_BEZIERLINE};

class PolyPath {
protected:
    // ...
public:
    PolyPath();
    PolyPath(const PolyPath& rPath);
    ~PolyPath();
    void Clear();

          USHORT    GetPathCount()        const;
          PathKind  GetPathKind(USHORT i) const;
    const Polygon&  GetPolygon (USHORT i) const;
    const XPolygon& GetXPolygon(USHORT i) const;
          Polygon&  GetPolygon (USHORT i);
          XPolygon& GetXPolygon(USHORT i);
    const Polygon&  operator[] (USHORT i); // eigentlich identisch mit Get

    void InsertPolyPath  (const PolyPath&   , USHORT nPos=POLYPOLY_APPEND);
    void InsertPolygon   (const Polygon&    , USHORT nPos=POLYPOLY_APPEND);
    void InsertPolygon   (const PolyPolygon&, USHORT nPos=POLYPOLY_APPEND);
    void InsertPolyLine  (const Polygon&    , USHORT nPos=POLYPOLY_APPEND);
    void InsertBezier    (const XPolygon&   , USHORT nPos=POLYPOLY_APPEND);
    void InsertBezierLine(const XPolygon&   , USHORT nPos=POLYPOLY_APPEND);

    BOOL operator==(const PolyPath& rPath);
    void operator!=(const PolyPath& rPath);
    void operator= (const PolyPath& rPath);
    void operator+=(const Point&);
    void operator-=(const Point&);

    Rectangle GetBoundRect() const;
    BOOL IsInside(const Point&) const;
    BOOL IsTouch(const Rectangle&) const;
};

// ExtOutputDevice::DrawPolyPath(const PolyPath&)
// Alle Polygone und Beziers werden zu einem einzigen PolyPolygon gemacht das
// dann mit DrawPolyPolygon() ausgegeben wird. Alle PolyLines und BezierLines
// werden dagegen mit DrawPolyLine() (oder DrawPolyPolyLine()?) gamalt.

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_POLYPATH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
