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

#ifndef _POLY3D_HXX
#define _POLY3D_HXX

#include <tools/solar.h>

#include <bf_svx/volume3d.hxx>

#include <bf_goodies/base3d.hxx>
class SvStream;
class Polygon;
class PolyPolygon;
namespace binfilter {
class Vector3D;
class Matrix4D;


class XPolygon;
class XPolyPolygon;

#define POLY3D_APPEND		0xFFFF
#define POLYPOLY3D_APPEND	0xFFFF

#define POLY3D_MAXPOINTS	0x7FFF

/*************************************************************************
|*
|* 3D-Polygon-Implementierung
|*
\************************************************************************/

#if _SOLAR__PRIVATE
class ImpPolygon3D
{
public:
    Vector3D*					pPointAry;
    Vector3D*					pOldPointAry;

    UINT16						nSize;
    UINT16						nResize;
    UINT16						nPoints;
    UINT16  					nRefCount;

    unsigned					bDeleteOldAry : 1;
    unsigned					bClosed : 1;

    ImpPolygon3D(UINT16 nInitSize = 4, UINT16 nPolyResize = 4);
    ImpPolygon3D(const ImpPolygon3D& rImpPoly3D);
    ~ImpPolygon3D();

    void CheckPointDelete();
    void Resize(UINT16 nNewSize, BOOL bDeletePoints = TRUE);
    void Remove(UINT16 nPos, UINT16 nCount);
};
#endif

/************************************************************************/

// Degree Flags
#define	DEGREE_FLAG_X			(0x0001)
#define	DEGREE_FLAG_Y			(0x0002)
#define	DEGREE_FLAG_Z			(0x0004)
#define	DEGREE_FLAG_ALL			(DEGREE_FLAG_X|DEGREE_FLAG_Y|DEGREE_FLAG_Z)

/************************************************************************/

// Cut Flags for FindCut()
#define	CUTFLAG_LINE			(0x0001)
#define	CUTFLAG_START1			(0x0002)
#define	CUTFLAG_START2			(0x0004)
#define	CUTFLAG_END1			(0x0008)
#define	CUTFLAG_END2			(0x0010)

#define	CUTFLAG_ALL			(CUTFLAG_LINE|CUTFLAG_START1|CUTFLAG_START2|CUTFLAG_END1|CUTFLAG_END2)
#define	CUTFLAG_DEFAULT		(CUTFLAG_LINE|CUTFLAG_START2|CUTFLAG_END2)

/************************************************************************/

class ImpPolyPolygon3D;
class ImpPolygon3D;
class PolyPolygon3D;
class E3dTriangle;
class E3dTriangleList;

/************************************************************************/

class Polygon3D
{
protected:
    ImpPolygon3D*				pImpPolygon3D;

    // ImpPolygon3D-ReferenceCount pruefen und ggf. abkoppeln
    void CheckReference();

    // Hilfsfunktionen fuer Triangulation
    UINT16 GetHighestEdge() const;

public:
    Polygon3D(UINT16 nSize = 4, UINT16 nResize = 4);
    Polygon3D(const Polygon3D& rPoly3D);
    Polygon3D(const Polygon& rPoly, double fScale = 1.0);
    // es wird keine Konvertierung des XPolygon durchgefuehrt,
    // sondern es werden nur die Punktkoordinaten uebernommen!
    Polygon3D(const XPolygon& rXPoly, double fScale = 1.0);
    ~Polygon3D();

    // Korrigiert das closed-Flag und entfernt den doppelten Punkt
    void CheckClosed();


    void SetPointCount(UINT16 nPoints);
    UINT16 GetPointCount() const;


    const Vector3D&	operator[](UINT16 nPos) const;
    Vector3D& operator[](UINT16 nPos);
    Polygon3D& operator= (const Polygon3D& rPoly3D);
    BOOL operator==(const Polygon3D& rPoly3D) const;

    // Drehrichtung feststellen (fuer Polygone in der XY-Ebene) bzw. umkehren
    BOOL IsClockwise(const Vector3D &rNormal) const;
    BOOL IsClockwise() const { Vector3D aNrm(0.0, 0.0, 1.0); return IsClockwise(aNrm); }
    Vector3D GetNormal() const;
    Vector3D GetMiddle() const;
    void FlipDirection();
    BOOL IsClosed() const;
    void SetClosed(BOOL bNew);

    // aufeinanderfolgende doppelte Punkte sowie gleiche Anfangs- und
    // Endpunkte entfernen
    void RemoveDoublePoints();

    // Ueberlappen sich das aktuelle und das angegebene Polygon ?

    // Existiert ein Schnitt zwischen den Polys?

    void Transform(const Matrix4D& rTfMatrix);

    BOOL IsInside(const Vector3D& rPnt, BOOL bWithBorder=FALSE) const;

    // different export formats
    Polygon GetPolygon() const;

    Volume3D GetPolySize() const;
    double GetPolyArea(const Vector3D& rNormal) const;

    // Laenge des Polygons liefern
    double GetLength() const;

    friend SvStream& operator>>(SvStream& rIStream, Polygon3D& rPoly3D);
    friend SvStream& operator<<(SvStream& rOStream, const Polygon3D& rPoly3D);

    // Schnitt zwischen den von den Punkten angegebenen Kanten ermitteln.
    // Dabei ist der Rueckgabewert != 0.0, wenn der Schnitt innerhalb
    // der Parameterbereiche der Kanten liegt und gibt den Wert ]0.0, 1.0]
    // innerhalb der ersten Kante an.

    // Diese Version arbeitet mit der Kante nEdge1 aus dem lokalen
    // Polygon und nEdge2 aus dem uebergebenen

    // Diese Version nimmt die Startpunkte und Vektoren (relative Angabe
    // des Endpunktes) zweier Kanten

    // test if point is on line in range ]0.0..1.0[ without
    // the points. If so, return TRUE and put the parameter
    // value in pCut (if provided)

    // Orientierung im Punkt nIndex liefern
    BOOL GetPointOrientation(UINT16 nIndex) const;

    // get position on polypos, with clipping to start/end
    Vector3D GetPosition(double fPos) const;

    // create a expanded or compresssed poly with exactly nNum Points
    Polygon3D GetExpandedPolygon(sal_uInt32 nNum);
};

/************************************************************************/

//#define	TEST_MERGE
//#ifdef TEST_MERGE
//extern OutputDevice* pThisIsTheActualOutDev;
//#endif

///////////////////////////////////////////////////////////////////////////////

class PolyPolygon3D
{
protected:
    ImpPolyPolygon3D*			pImpPolyPolygon3D;

    void CheckReference();

public:
    PolyPolygon3D(UINT16 nInitSize = 4, UINT16 nResize = 4);
    PolyPolygon3D(const Polygon3D& r3DPoly);
    PolyPolygon3D(const PolyPolygon3D& r3DPolyPoly);
    PolyPolygon3D(const PolyPolygon& rPolyPoly, double fScale = 1.0);
    PolyPolygon3D(const XPolyPolygon& rXPolyPoly, double fScale = 1.0);
    ~PolyPolygon3D();

    // Korrigiert das closed-Flag und entfernt den doppelten Punkt

    void Insert(const Polygon3D& rPoly3D, UINT16 nPos = POLYPOLY3D_APPEND);
    void Insert(const PolyPolygon3D& rPoly3D, UINT16 nPos = POLYPOLY3D_APPEND);


    const Polygon3D& GetObject(UINT16 nPos) const;

    void Clear();
    UINT16 Count() const;

    const Polygon3D& operator[](UINT16 nPos) const { return GetObject(nPos); }
    Polygon3D& operator[](UINT16 nPos);

    PolyPolygon3D& operator=(const PolyPolygon3D& rPolyPoly3D);
    BOOL operator!=(const PolyPolygon3D& rPolyPoly3D) const;

    void Transform(const Matrix4D& rTfMatrix);

    void FlipDirections();

    // Die Umlaufrichtungen ineinanderliegender Polygone anpassen
    void SetDirections(const Vector3D& rNormal);
    void SetDirections() { Vector3D aNormal = GetNormal(); SetDirections(aNormal); }

    // in allen Polygonen aufeinanderfolgende doppelte Punkte sowie
    // gleiche Anfangs- und Endpunkte entfernen
    void RemoveDoublePoints();

    // Remove all completely overlapping polygons

    // evtl. entstandene Selbstueberschneidungen in Eckpunkten
    // ohne Punktreduzierung korrigieren
    void CorrectGrownPoly(const PolyPolygon3D& rOrig);

    // Ueberlappen sich das aktuelle und das angegebene PolyPolygon ?

    friend SvStream& operator>>(SvStream& rIStream, PolyPolygon3D& rPolyPoly3D);
    friend SvStream& operator<<(SvStream& rOStream, const PolyPolygon3D& rPolyPoly3D);

    // Drehrichtung feststellen (fuer Polygone in der XY-Ebene) bzw. umkehren
    Vector3D GetNormal() const;
    Vector3D GetMiddle() const;
    BOOL IsClosed() const;

    PolyPolygon GetPolyPolygon() const;

    Volume3D GetPolySize() const;
    double GetPolyArea() const;

    // Laenge des Polygons liefern
    double GetLength() const;

    // merge the contents of the whole PolyPolygon to contain no more
    // cuts or overlaps. Makes all necessary merges between all
    // contained polygons. Preserves Holes.
};

}//end of namespace binfilter
#endif		// _POLY3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
