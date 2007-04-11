/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dgeom.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:32:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _B3D_B3DGEOM_HXX
#define _B3D_B3DGEOM_HXX

#ifndef _B3D_B3DENTITY_HXX
#include <goodies/b3dentty.hxx>
#endif

//#ifndef _B3D_VOLUM_HXX
//#include "b3dvolum.hxx"
//#endif

#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
#endif

#include <vector>

/*************************************************************************
|*
|* Defines fuer die Erzeugung von Default-Normalen und -Texturkoordinaten
|*
\************************************************************************/

#define B3D_CREATE_DEFAULT_X            (0x0001)
#define B3D_CREATE_DEFAULT_Y            (0x0002)
#define B3D_CREATE_DEFAULT_ALL          (B3D_CREATE_DEFAULT_X|B3D_CREATE_DEFAULT_Y)

/*************************************************************************
|*
|* Index-Klasse fuer Geometrie-Bucket der B3dGeometry-Klasse
|*
\************************************************************************/

#define B3D_INDEX_MODE_FILLED                   (0)
#define B3D_INDEX_MODE_LINE                     (1)
#define B3D_INDEX_MODE_POINT                    (2)
#define B3D_INDEX_MODE_UNUSED                   (3)

class GeometryIndexValue
{
private:
    unsigned                    nIndex      : 30;
    unsigned                    nMode       :  2;

public:
    GeometryIndexValue() : nIndex(0), nMode(B3D_INDEX_MODE_FILLED) {}
    GeometryIndexValue(UINT32 nInd) : nIndex(nInd), nMode(B3D_INDEX_MODE_FILLED) {}
    GeometryIndexValue(UINT32 nInd, UINT8 nMod) : nIndex(nInd), nMode(nMod) {}

    UINT32 GetIndex() const { return ((UINT32)nIndex); }
    void SetIndex(UINT32 nNew) { nIndex = nNew; }

    UINT8 GetMode() const { return ((UINT8)nMode); }
    void SetMode(UINT8 nNew) { nMode = nNew; }

protected:
};

/*************************************************************************
|*
|* Bucket fuer Indices
|*
\************************************************************************/

BASE3D_DECL_BUCKET(GeometryIndexValue, Bucket)

/*************************************************************************
|*
|* Geometrie eines 3D-Objektes
|*
\************************************************************************/
class B3dComplexPolygon;

class B3dGeometry
{
private:
    // Tool zum triangulieren komplexer Polygone
    // Wird nur temporaer erzeugt
    B3dComplexPolygon*          pComplexPolygon;

    // Bucket fuer grundsaetzliche Geometrie
    B3dEntityBucket             aEntityBucket;

    // Indices der Polygonendpunkte
    GeometryIndexValueBucket    aIndexBucket;

    // Ausgangszustand der Variablen herstellen
    void Reset();

    // Freien Eintrag zum fuellen holen
    B3dEntity& GetFreeEntity();

    // Hint-Variable
    unsigned                    bHintIsComplex      : 1;
    unsigned                    bOutline            : 1;

public:
    // Konstruktor, Destruktor
    B3dGeometry();

    // #92030# add access to bOutline hint flag
    sal_Bool IsOutline() const { return (sal_Bool)bOutline; }

    // Geometrieerzeugung
    void StartDescription();
    void EndDescription();

    void StartObject(BOOL bHintIsComplex=TRUE, BOOL bOutl=FALSE);
    void EndObject();

    void AddEdge(const basegfx::B3DPoint& rPoint);
    void AddEdge(const basegfx::B3DPoint& rPoint, const basegfx::B3DVector& rNormal);
    void AddEdge(const basegfx::B3DPoint& rPoint, const basegfx::B3DVector& rNormal, const basegfx::B2DPoint& rTexture);

    // Inhalte loeschen
    void Erase();
    // Inhalte loeschen und Speicher freigeben
    void Empty();

    // Copy-Operator
    void operator=(const B3dGeometry& rObj);

    // Zugriff auf beide Buckets um die Geometrie zu lesen
    const B3dEntityBucket& GetEntityBucket() const { return aEntityBucket; }
    const GeometryIndexValueBucket& GetIndexBucket() const { return aIndexBucket; }

    // Eine beliebige Transformation auf die Geometrie anwenden
    void Transform(const basegfx::B3DHomMatrix& rMat);

    // Hittest auf Geometrie
    sal_Bool CheckHit(const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack, sal_uInt16 nTol);

    // BoundVolume liefern
    basegfx::B3DRange GetBoundVolume() const;

    // Mittelpunkt liefern
    basegfx::B3DPoint GetCenter() const;

    // Standard - Normalen generieren
    void CreateDefaultNormalsSphere();
    void RemoveNormals();

    // Standard - Texturkoordinaten generieren
    void CreateDefaultTexture(sal_uInt16 nCreateWhat=B3D_CREATE_DEFAULT_ALL,
        BOOL bUseSphere=TRUE);
    void RemoveTexture();

    // Default-Geometrien erstellen
    void CreateCube(const basegfx::B3DRange& rVolume);
    void CreateSphere(const basegfx::B3DRange& rVolume, double nX, double nY);

    // Normalen invertieren
    void InvertNormals();

    // #110988# get all cuts of the geometry with the given vector defined by the two positions
    void GetAllCuts(::std::vector< basegfx::B3DPoint >& rVector, const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack) const;

protected:
    // Callbacks bei komplexen Primitiven
    friend class B3dComplexPolygon;
    void StartComplexPrimitive();
    void EndComplexPrimitive();
    void AddComplexVertex(B3dEntity& rNew, BOOL bIsVisible);

    // PolygonStart und -Ende aufzeichnen
    void StartPolygon();
    void EndPolygon();

    // Hittest fuer einzelnes Polygon, -1: Kein Hit, sonst die Tiefe
    sal_Bool CheckSinglePolygonHit(sal_uInt32 nLow, sal_uInt32 nHigh, const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack, basegfx::B3DPoint& rCut) const;

    // Schnittpunkt Polygon/Vektor bestimmen
    sal_Bool GetCutPoint(sal_uInt32 nLow, basegfx::B3DPoint& rCut, const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack) const;

    // Test auf drin/draussen fuer einzelnes Polygon
    sal_Bool IsInside(sal_uInt32 nLow, sal_uInt32 nHigh, const basegfx::B3DPoint& rPnt) const;

    // Normale ermitteln fuer einzelnes Polygon
    basegfx::B3DVector CalcNormal(sal_uInt32 nLow, sal_uInt32 nHigh) const;
};

#endif          // _B3D_B3DGEOM_HXX
