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
#ifndef _XPOLY_HXX
#define _XPOLY_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include "svx/svxdllapi.h"

class Point;
class Rectangle;
class SvStream;
class Polygon;
class PolyPolygon;
class OutputDevice;

/************************************************************************/

#define XPOLYPOLY_APPEND     0xFFFF
#define XPOLY_APPEND         0xFFFF

#define XPOLY_MAXPOINTS      0xFFF0 /* Auch fuer die 32-Bitter etwas Luft lassen */

/************************************************************************/
// Punktstile im XPolygon:
// NORMAL : Anfangs- oder Endpunkt einer Kurve oder Linie
// SMOOTH : Glatter Uebergang zwischen Kurven
// SYMMTR : glatter und symmetrischer Uebergang zwischen Kurven
// CONTROL: Kontrollpunkt einer Bezierkurve

enum XPolyFlags { XPOLY_NORMAL, XPOLY_SMOOTH, XPOLY_CONTROL, XPOLY_SYMMTR };


/*************************************************************************
|*
|* Klasse XPolygon; hat neben dem Point-Array noch ein Array mit Flags,
|* die Informationen ueber den jeweiligen Punkt enthalten
|*
\************************************************************************/

class ImpXPolygon;

class SVX_DLLPUBLIC XPolygon
{
protected:
    ImpXPolygon*    pImpXPolygon;

    // ImpXPolygon-ReferenceCount pruefen und ggf. abkoppeln
    void    CheckReference();

    // Hilfsfunktionen fuer Bezierkonvertierung
    void    SubdivideBezier(sal_uInt16 nPos, sal_Bool bCalcFirst, double fT);
    void    GenBezArc(const Point& rCenter, long nRx, long nRy,
                      long nXHdl, long nYHdl, sal_uInt16 nStart, sal_uInt16 nEnd,
                      sal_uInt16 nQuad, sal_uInt16 nFirst);
    sal_Bool    CheckAngles(sal_uInt16& nStart, sal_uInt16 nEnd, sal_uInt16& nA1, sal_uInt16& nA2);

public:
    XPolygon( sal_uInt16 nSize=16, sal_uInt16 nResize=16 );
    XPolygon( const XPolygon& rXPoly );
    XPolygon( const Rectangle& rRect, long nRx = 0, long nRy = 0 );
    XPolygon( const Point& rCenter, long nRx, long nRy,
              sal_uInt16 nStartAngle = 0, sal_uInt16 nEndAngle = 3600,
              sal_Bool bClose = sal_True );

    ~XPolygon();

    void        SetPointCount( sal_uInt16 nPoints );
    sal_uInt16      GetPointCount() const;

    void        Insert( sal_uInt16 nPos, const Point& rPt, XPolyFlags eFlags );
    void        Insert( sal_uInt16 nPos, const XPolygon& rXPoly );
    void        Remove( sal_uInt16 nPos, sal_uInt16 nCount );
    void        Move( long nHorzMove, long nVertMove );
    Rectangle   GetBoundRect() const;

    const Point&    operator[]( sal_uInt16 nPos ) const;
          Point&    operator[]( sal_uInt16 nPos );
    XPolygon&       operator=( const XPolygon& rXPoly );
    sal_Bool            operator==( const XPolygon& rXPoly ) const;
    sal_Bool            operator!=( const XPolygon& rXPoly ) const;

    XPolyFlags  GetFlags( sal_uInt16 nPos ) const;
    void        SetFlags( sal_uInt16 nPos, XPolyFlags eFlags );
    sal_Bool        IsControl(sal_uInt16 nPos) const;
    sal_Bool        IsSmooth(sal_uInt16 nPos) const;

    // Abstand zwischen zwei Punkten
    double  CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2);

    // Bezierkonvertierungen
    void CalcSmoothJoin(sal_uInt16 nCenter, sal_uInt16 nDrag, sal_uInt16 nPnt);
    void CalcTangent(sal_uInt16 nCenter, sal_uInt16 nPrev, sal_uInt16 nNext);
    void PointsToBezier(sal_uInt16 nFirst);

    // Transformationen
    void Translate(const Point& rTrans);
    void Scale(double fSx, double fSy);
    void Distort(const Rectangle& rRefRect, const XPolygon& rDistortedRect);

    // #116512# convert to basegfx::B2DPolygon and return
    basegfx::B2DPolygon getB2DPolygon() const;

    // #116512# constructor to convert from basegfx::B2DPolygon
     // #i76339# made explicit
     explicit XPolygon(const basegfx::B2DPolygon& rPolygon);
};

/*************************************************************************
|*
|* Klasse XPolyPolygon; wie PolyPolygon, nur statt aus Polygonen aus
|* XPolygonen bestehend
|*
\************************************************************************/

class ImpXPolyPolygon;

class SVX_DLLPUBLIC XPolyPolygon
{
protected:
    ImpXPolyPolygon* pImpXPolyPolygon;

    // ImpXPolyPolygon-ReferenceCount pruefen und ggf. abkoppeln
    void    CheckReference();

public:
                    XPolyPolygon( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 );
                    XPolyPolygon( const XPolyPolygon& rXPolyPoly );

                    ~XPolyPolygon();

    void            Insert( const XPolygon& rXPoly,
                            sal_uInt16 nPos = XPOLYPOLY_APPEND );
    void            Insert( const XPolyPolygon& rXPoly,
                            sal_uInt16 nPos=XPOLYPOLY_APPEND );
    XPolygon        Remove( sal_uInt16 nPos );
    const XPolygon& GetObject( sal_uInt16 nPos ) const;

    void            Clear();
    sal_uInt16          Count() const;

    Rectangle       GetBoundRect() const;

    const XPolygon& operator[]( sal_uInt16 nPos ) const
                        { return GetObject( nPos ); }
    XPolygon&       operator[]( sal_uInt16 nPos );

    XPolyPolygon&   operator=( const XPolyPolygon& rXPolyPoly );
    sal_Bool            operator==( const XPolyPolygon& rXPolyPoly ) const;
    sal_Bool            operator!=( const XPolyPolygon& rXPolyPoly ) const;

    // Transformationen
    void Distort(const Rectangle& rRefRect, const XPolygon& rDistortedRect);

    // #116512# convert to basegfx::B2DPolyPolygon and return
    basegfx::B2DPolyPolygon getB2DPolyPolygon() const;

    // #116512# constructor to convert from basegfx::B2DPolyPolygon
     // #i76339# made explicit
     explicit XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);
};

#endif      // _XPOLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
