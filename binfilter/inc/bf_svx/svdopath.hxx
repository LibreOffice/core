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

#ifndef _SVDOPATH_HXX
#define _SVDOPATH_HXX

#ifndef _SVDOTEXT_HXX
#include <bf_svx/svdotext.hxx>
#endif

#ifndef _XPOLY_HXX
#include <bf_svx/xpoly.hxx>
#endif
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class XPolyPolygon;

//************************************************************
//   Defines
//************************************************************

enum SdrPathType {SDRPATH_NONE,SDRPATH_LINE,SDRPATH_CURVE};

//************************************************************
//   Hilfsklasse SdrPathObjGeoData
//
// fuer Undo/Redo
//
//************************************************************

class SdrPathObjGeoData : public SdrTextObjGeoData
{
    friend class				SdrPolyEditView;

public:
    XPolyPolygon				aPathPolygon;
    SdrObjKind					eKind;

    SdrPathObjGeoData();
    virtual ~SdrPathObjGeoData();
};

//************************************************************
//   SdrPathObj
//************************************************************

class SdrPathObj : public SdrTextObj
{
    friend class				ImpSdrPathDragData;
    friend class				SdrPolyEditView;

protected:
    XPolyPolygon				aPathPolygon;
    SdrObjKind					eKind;

    // Hilfsvariable fuer Create
    USHORT						bCreating; // nActPoly;

protected:
    // Hilfsfunktion fuer GET/SET/INS/etc. PNT
    FASTBOOL FindPolyPnt(USHORT nAbsPnt, USHORT& rPolyNum, USHORT& rPointNum, FASTBOOL bAllPoints=TRUE) const;
    void ImpForceKind();
    void ImpForceLineWink();

    // fuer friend class SdrPolyEditView auf einigen Compilern:
    void SetRectsDirty(FASTBOOL bNotMyself=FALSE) { SdrTextObj::SetRectsDirty(bNotMyself); }

public:
    TYPEINFO();
    SdrPathObj(SdrObjKind eNewKind);
    SdrPathObj(SdrObjKind eNewKind, const XPolyPolygon& rPathPoly);
    SdrPathObj(const Point& rPt1, const Point& rPt2);
    virtual ~SdrPathObj();

    virtual UINT16 GetObjIdentifier() const;
    virtual void RecalcBoundRect();
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual FASTBOOL Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual void operator=(const SdrObject& rObj);

    virtual void TakeXorPoly(XPolyPolygon& rXPolyPoly, FASTBOOL bDetail) const;
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void TakeContour(XPolyPolygon& rPoly) const;





    virtual void NbcMove(const Size& aSize);
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact);
    virtual void NbcRotate(const Point& rRefPnt, long nAngle, double fSin, double fCos);
    virtual void NbcShear(const Point& rRefPnt, long nAngle, double fTan, FASTBOOL bVShear);


    virtual FASTBOOL IsPolyObj() const;
    virtual USHORT GetPointCount() const;
    virtual const Point& GetPoint(USHORT nHdlNum) const;
    virtual void NbcSetPoint(const Point& rPnt, USHORT nHdlNum);

    // Punkt einfuegen

    // Punkt loeschen

    // An diesem Punkt auftrennen

    // Objekt schliessen

protected:
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;

public:

    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);

    // Bezierpolygon holen/setzen
    const XPolyPolygon& GetPathPoly() const { return aPathPolygon; }
    void SetPathPoly(const XPolyPolygon& rPathPoly);
    void NbcSetPathPoly(const XPolyPolygon& rPathPoly);

    // Man stecke die Handlenummer rein und bekomme die Polygon- und Punktnummer
    // des zugehoerigen Punkts im XPolyPolygon.
    FASTBOOL TakePolyIdxForHdlNum(USHORT nHdlNum, USHORT& rPolyNum, USHORT& rPointNum) const { return FindPolyPnt(nHdlNum,rPolyNum,rPointNum,FALSE); }

    // Spezialfunktionen fuer Bezierpolygon-Bearbeitung
    static FASTBOOL IsClosed(SdrObjKind eKind) { return eKind==OBJ_POLY || eKind==OBJ_PATHPOLY || eKind==OBJ_PATHFILL || eKind==OBJ_FREEFILL || eKind==OBJ_SPLNFILL; }
    static FASTBOOL IsLine(SdrObjKind eKind) { return eKind==OBJ_PLIN || eKind==OBJ_PATHPLIN || eKind==OBJ_PATHLINE || eKind==OBJ_FREELINE || eKind==OBJ_SPLNLINE || eKind==OBJ_LINE; }
    static FASTBOOL IsFreeHand(SdrObjKind eKind) { return eKind==OBJ_FREELINE || eKind==OBJ_FREEFILL; }
    static FASTBOOL IsBezier(SdrObjKind eKind) { return eKind==OBJ_PATHLINE || eKind==OBJ_PATHFILL; }
    static FASTBOOL IsSpline(SdrObjKind eKind) { return eKind==OBJ_SPLNLINE || eKind==OBJ_SPLNFILL; }
    FASTBOOL IsClosed() const { return eKind==OBJ_POLY || eKind==OBJ_PATHPOLY || eKind==OBJ_PATHFILL || eKind==OBJ_FREEFILL || eKind==OBJ_SPLNFILL; }
    FASTBOOL IsLine() const { return eKind==OBJ_PLIN || eKind==OBJ_PATHPLIN || eKind==OBJ_PATHLINE || eKind==OBJ_FREELINE || eKind==OBJ_SPLNLINE || eKind==OBJ_LINE; }
    FASTBOOL IsFreeHand() const { return eKind==OBJ_FREELINE || eKind==OBJ_FREEFILL; }
    FASTBOOL IsBezier() const { return eKind==OBJ_PATHLINE || eKind==OBJ_PATHFILL; }
    FASTBOOL IsSpline() const { return eKind==OBJ_SPLNLINE || eKind==OBJ_SPLNFILL; }

    // Pfad schliessen bzw. oeffnen; im letzteren Fall den Endpunkt um
    // "nOpenDistance" verschieben

    // Pfadabschnitt in Kurve bzw. Linie umwandeln. Toggle!

    // Flag fuer Uebergang zwischen zwei Pfadsegmenten setzen

    // Welche Umwandlung eines Pfadabschnitts ist moeglich?

    // Feststellen von welcher Art das Segment ist

    // eType=SDRPATH_NONE bedeutet Toggle

    // Alle Segmente konvertieren, eType=SDRPATH_NONE bedeutet Toggle

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // transformation interface for StarOfficeAPI. This implements support for 
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear 
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual BOOL TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix. 
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has 
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOPATH_HXX

