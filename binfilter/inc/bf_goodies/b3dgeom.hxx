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

#ifndef _B3D_B3DGEOM_HXX
#define _B3D_B3DGEOM_HXX

#ifndef _B3D_B3DENTITY_HXX
#include "b3dentty.hxx"
#endif

#ifndef _B3D_VOLUM_HXX
#include "b3dvolum.hxx"
#endif

namespace binfilter {

/*************************************************************************
|*
|* Defines fuer die Erzeugung von Default-Normalen und -Texturkoordinaten
|*
\************************************************************************/

#define	B3D_CREATE_DEFAULT_X			(0x0001)
#define	B3D_CREATE_DEFAULT_Y			(0x0002)
#define	B3D_CREATE_DEFAULT_Z			(0x0004)
#define	B3D_CREATE_DEFAULT_ALL			(B3D_CREATE_DEFAULT_X|B3D_CREATE_DEFAULT_Y|B3D_CREATE_DEFAULT_Z)


/*************************************************************************
|*
|* Index-Klasse fuer Geometrie-Bucket der B3dGeometry-Klasse
|*
\************************************************************************/

#define	B3D_INDEX_MODE_FILLED					(0)
#define	B3D_INDEX_MODE_LINE						(1)
#define	B3D_INDEX_MODE_POINT					(2)
#define	B3D_INDEX_MODE_UNUSED					(3)

class GeometryIndexValue
{
private:
    unsigned					nIndex		: 30;
    unsigned					nMode		:  2;

public:
    GeometryIndexValue() : nIndex(0), nMode(B3D_INDEX_MODE_FILLED) {}
    GeometryIndexValue(UINT32 nInd) : nIndex(nInd), nMode(B3D_INDEX_MODE_FILLED) {}
    GeometryIndexValue(UINT32 nInd, UINT8 nMod) : nIndex(nInd), nMode(nMod) {}

    UINT32 GetIndex() { return ((UINT32)nIndex); }
    void SetIndex(UINT32 nNew) { nIndex = nNew; }

    UINT8 GetMode() { return ((UINT8)nMode); }
    void SetMode(UINT8 nNew) { nMode = nNew; }

protected:
};

/*************************************************************************
|*
|* Bucket fuer Indices
|*
\************************************************************************/

SV_DECL_VARARR(GeometryIndexValueBucketMemArr, char*, 32, 32)
class GeometryIndexValueBucket {
private:
GeometryIndexValueBucketMemArr	aMemArray;
    UINT32			nMask;
    UINT32			nCount;
    INT16			nFreeMemArray;
    INT16			nActMemArray;
    UINT16			nFreeEntry;
    UINT16			nShift;
    UINT16			nBlockShift;
    UINT16			nEntriesPerArray;
    UINT16			nSlotSize;
    UINT16			nNext;
    UINT16			nMemArray;
public:
    GeometryIndexValueBucket(UINT16 TheSize);
    /* Zu verwendende Groesse der Speicherarrays setzen */
    /* Bitte NUR verwenden, falls sich der Leerkonstruktor */
    /* nicht vermeiden laesst! Nicht nachtraeglich anwenden!  */
    void InitializeSize(UINT16 TheSize);
    /* Destruktor */
    ~GeometryIndexValueBucket();
    /* Anhaengen und kopieren */
    BOOL Append(GeometryIndexValue& rVec)
        { if(CareForSpace()) return ImplAppend(rVec); return FALSE; }
    /* leeren und Speicher freigeben */
    void Empty();
    /* leeren aber Speicher behalten */
    void Erase();
    GeometryIndexValue& operator[] (UINT32 nPos);
    UINT32 Count() { return nCount; }
    UINT32 GetNumAllocated() { return aMemArray.Count() * nEntriesPerArray; }
    void operator=(const GeometryIndexValueBucket&);
    UINT16 GetBlockShift() { return nBlockShift; }
    UINT16 GetSlotSize() { return nSlotSize; }
private:
    BOOL CareForSpace()
        { if(nFreeEntry == nEntriesPerArray)
        return ImplCareForSpace(); return TRUE; }
    BOOL ImplCareForSpace();
    /* Anhaengen und kopieren */
    BOOL ImplAppend(GeometryIndexValue& rVec);
};

/*************************************************************************
|*
|* Geometrie eines 3D-Objektes
|*
\************************************************************************/
class B3dComplexPolygon;
class Matrix4D;

class B3dGeometry
{
private:
    // Tool zum triangulieren komplexer Polygone
    // Wird nur temporaer erzeugt
    B3dComplexPolygon*			pComplexPolygon;

    // Bucket fuer grundsaetzliche Geometrie
    B3dEntityBucket				aEntityBucket;

    // Indices der Polygonendpunkte
    GeometryIndexValueBucket	aIndexBucket;

    // Ausgangszustand der Variablen herstellen
    void Reset();

    // Freien Eintrag zum fuellen holen
    B3dEntity& GetFreeEntity();

    // Hint-Variable
    unsigned					bHintIsComplex		: 1;
    unsigned					bOutline			: 1;

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

    void AddEdge(const Vector3D& rPoint);
    void AddEdge(const Vector3D& rPoint,
        const Vector3D& rNormal);
    void AddEdge(const Vector3D& rPoint,
        const Vector3D& rNormal,
        const Vector3D& rTexture);

    // Inhalte loeschen
    void Erase();

    // Copy-Operator
    void operator=(const B3dGeometry& rObj);

    // Zugriff auf beide Buckets um die Geometrie zu lesen
    B3dEntityBucket& GetEntityBucket() { return aEntityBucket; }

    // Hittest auf Geometrie
    sal_Bool CheckHit(const Vector3D &rFront, const Vector3D &rBack, sal_uInt16 nTol);

    // BoundVolume liefern
    B3dVolume GetBoundVolume() const;

    // Mittelpunkt liefern
    Vector3D GetCenter() const;

    // Standard - Normalen generieren
    void CreateDefaultNormalsSphere();

    // Standard - Texturkoordinaten generieren
    void CreateDefaultTexture(UINT16 nCreateWhat=B3D_CREATE_DEFAULT_ALL,
        BOOL bUseSphere=TRUE);

    // Normalen invertieren
    void InvertNormals();
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
    sal_Bool CheckSinglePolygonHit(UINT32 nLow, UINT32 nHigh, const Vector3D& rFront, const Vector3D& rBack, Vector3D& rCut) const;

    // Schnittpunkt Polygon/Vektor bestimmen
    sal_Bool GetCutPoint(UINT32 nLow, Vector3D& rCut, const Vector3D& rFront, const Vector3D& rBack) const;

    // Test auf drin/draussen fuer einzelnes Polygon
    sal_Bool IsInside(UINT32 nLow, UINT32 nHigh, const Vector3D& rPnt) const;

    // Normale ermitteln fuer einzelnes Polygon
    Vector3D CalcNormal(UINT32 nLow, UINT32 nHigh) const;
};
}//end of namespace binfilter

#endif          // _B3D_B3DGEOM_HXX
