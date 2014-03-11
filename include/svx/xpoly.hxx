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
#ifndef INCLUDED_SVX_XPOLY_HXX
#define INCLUDED_SVX_XPOLY_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svx/svxdllapi.h>

class Point;
class Rectangle;
class SvStream;
class Polygon;
class PolyPolygon;
class OutputDevice;

#define XPOLYPOLY_APPEND     0xFFFF
#define XPOLY_APPEND         0xFFFF

#define XPOLY_MAXPOINTS      0xFFF0 /* Auch fuer die 32-Bitter etwas Luft lassen */

// Punktstile im XPolygon:
// NORMAL : Anfangs- oder Endpunkt einer Kurve oder Linie
// SMOOTH : Glatter Uebergang zwischen Kurven
// SYMMTR : glatter und symmetrischer Uebergang zwischen Kurven
// CONTROL: Kontrollpunkt einer Bezierkurve

enum XPolyFlags { XPOLY_NORMAL, XPOLY_SMOOTH, XPOLY_CONTROL, XPOLY_SYMMTR };

// Klasse XPolygon; hat neben dem Point-Array noch ein Array mit Flags,
// die Informationen ueber den jeweiligen Punkt enthalten

class ImpXPolygon;

class SVX_DLLPUBLIC XPolygon
{
protected:
    ImpXPolygon*    pImpXPolygon;

    // ImpXPolygon-ReferenceCount pruefen und ggf. abkoppeln
    void    CheckReference();

    // Hilfsfunktionen fuer Bezierkonvertierung
    void    SubdivideBezier(sal_uInt16 nPos, bool bCalcFirst, double fT);
    void    GenBezArc(const Point& rCenter, long nRx, long nRy,
                      long nXHdl, long nYHdl, sal_uInt16 nStart, sal_uInt16 nEnd,
                      sal_uInt16 nQuad, sal_uInt16 nFirst);
    bool    CheckAngles(sal_uInt16& nStart, sal_uInt16 nEnd, sal_uInt16& nA1, sal_uInt16& nA2);

public:
    XPolygon( sal_uInt16 nSize=16, sal_uInt16 nResize=16 );
    XPolygon( const XPolygon& rXPoly );
    XPolygon( const Polygon& rPoly );
    XPolygon( const Rectangle& rRect, long nRx = 0, long nRy = 0 );
    XPolygon( const Point& rCenter, long nRx, long nRy,
              sal_uInt16 nStartAngle = 0, sal_uInt16 nEndAngle = 3600,
              bool bClose = true );

    ~XPolygon();

    sal_uInt16      GetSize() const;

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
    bool            operator==( const XPolygon& rXPoly ) const;
    bool            operator!=( const XPolygon& rXPoly ) const;

    XPolyFlags  GetFlags( sal_uInt16 nPos ) const;
    void        SetFlags( sal_uInt16 nPos, XPolyFlags eFlags );
    bool        IsControl(sal_uInt16 nPos) const;
    bool        IsSmooth(sal_uInt16 nPos) const;

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

// Klasse XPolyPolygon; wie PolyPolygon, nur statt aus Polygonen aus
// XPolygonen bestehend

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
    bool            operator==( const XPolyPolygon& rXPolyPoly ) const;
    bool            operator!=( const XPolyPolygon& rXPolyPoly ) const;

    // Transformationen
    void Distort(const Rectangle& rRefRect, const XPolygon& rDistortedRect);

    // #116512# convert to basegfx::B2DPolyPolygon and return
    basegfx::B2DPolyPolygon getB2DPolyPolygon() const;

    // #116512# constructor to convert from basegfx::B2DPolyPolygon
     // #i76339# made explicit
     explicit XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);
};

#endif // INCLUDED_SVX_XPOLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
