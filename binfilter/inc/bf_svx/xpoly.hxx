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

class Point;
class Rectangle;
class SvStream;
class Polygon;
class PolyPolygon;
class OutputDevice;
namespace binfilter {

/************************************************************************/

#define XPOLYPOLY_APPEND     0xFFFF
#define XPOLY_APPEND         0xFFFF

#ifdef WIN // Windows 16 Bit
#define XPOLY_MAXPOINTS      8160   /* =0xFF00/sizeof(Point), also mit etwas Platz! */
#else
#define XPOLY_MAXPOINTS      0xFFF0 /* Auch fuer die 32-Bitter etwas Luft lassen */
#endif

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

class XPolygon
{
protected:
    ImpXPolygon*    pImpXPolygon;

    // ImpXPolygon-ReferenceCount pruefen und ggf. abkoppeln
    void    CheckReference();

    // Hilfsfunktionen fuer Bezierkonvertierung
    void    SubdivideBezier(USHORT nPos, BOOL bCalcFirst, double fT);
    void    GenBezArc(const Point& rCenter, long nRx, long nRy,
                      long nXHdl, long nYHdl, USHORT nStart, USHORT nEnd,
                      USHORT nQuad, USHORT nFirst);
    BOOL    CheckAngles(USHORT& nStart, USHORT nEnd, USHORT& nA1, USHORT& nA2);

public:
    XPolygon( USHORT nSize=16, USHORT nResize=16 );
    XPolygon( const XPolygon& rXPoly );
    XPolygon( const Polygon& rPoly );
    XPolygon( const Rectangle& rRect, long nRx = 0, long nRy = 0 );
    XPolygon( const Point& rCenter, long nRx, long nRy,
              USHORT nStartAngle = 0, USHORT nEndAngle = 3600,
              BOOL bClose = TRUE );

    ~XPolygon();

    void        SetSize( USHORT nSize );

    void        SetPointCount( USHORT nPoints );
    USHORT      GetPointCount() const;

    void        Insert( USHORT nPos, const Point& rPt, XPolyFlags eFlags );
    void        Remove( USHORT nPos, USHORT nCount );
    void        Move( long nHorzMove, long nVertMove );
    Rectangle   GetBoundRect(OutputDevice* pOut = NULL) const;

    const Point&    operator[]( USHORT nPos ) const;
          Point&    operator[]( USHORT nPos );
    XPolygon&       operator=( const XPolygon& rXPoly );
    BOOL            operator==( const XPolygon& rXPoly ) const;
    BOOL            operator!=( const XPolygon& rXPoly ) const;

    XPolyFlags  GetFlags( USHORT nPos ) const;
    void        SetFlags( USHORT nPos, XPolyFlags eFlags );
    BOOL        IsControl(USHORT nPos) const;

    // Abstand zwischen zwei Punkten

    // Bezierkonvertierungen

    // Transformationen
    void Translate(const Point& rTrans);
    void Rotate(const Point& rCenter, double fSin, double fCos);
    void Rotate(const Point& rCenter, USHORT nAngle);
    void Scale(double fSx, double fSy);
    void SlantX(long nYRef, double fSin, double fCos);

    friend SvStream& operator>>( SvStream& rIStream, XPolygon& rXPoly );
    friend SvStream& operator<<( SvStream& rOStream, const XPolygon& rXPoly );
};

/*************************************************************************
|*
|* Klasse XPolyPolygon; wie PolyPolygon, nur statt aus Polygonen aus
|* XPolygonen bestehend
|*
\************************************************************************/

class ImpXPolyPolygon;

class XPolyPolygon
{
protected:
    ImpXPolyPolygon* pImpXPolyPolygon;

    // ImpXPolyPolygon-ReferenceCount pruefen und ggf. abkoppeln
    void    CheckReference();

public:
                    XPolyPolygon( USHORT nInitSize = 16, USHORT nResize = 16 );
                    XPolyPolygon( const XPolygon& rXPoly );
                    XPolyPolygon( const XPolyPolygon& rXPolyPoly );
                    XPolyPolygon( const PolyPolygon& rPolyPoly);

                    ~XPolyPolygon();

    void            Insert( const XPolygon& rXPoly,
                            USHORT nPos = XPOLYPOLY_APPEND );
    const XPolygon& GetObject( USHORT nPos ) const;

    void            Clear();
    USHORT          Count() const;

    void            Move( long nHorzMove, long nVertMove );
    Rectangle       GetBoundRect(OutputDevice* pOut = NULL) const;

    const XPolygon& operator[]( USHORT nPos ) const
                        { return GetObject( nPos ); }
    XPolygon&       operator[]( USHORT nPos );

    XPolyPolygon&   operator=( const XPolyPolygon& rXPolyPoly );

    // Transformationen
    void Translate(const Point& rTrans);
    void Rotate(const Point& rCenter, double fSin, double fCos);
    void Scale(double fSx, double fSy);
    void SlantX(long nYRef, double fSin, double fCos);

    friend SvStream& operator>>( SvStream& rIStream, XPolyPolygon& rXPolyPoly );
    friend SvStream& operator<<( SvStream& rOStream, const XPolyPolygon& rXPolyPoly );
};

}//end of namespace binfilter
#endif      // _XPOLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
