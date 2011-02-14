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

#include <svx/svdotext.hxx>
#include <svx/xpoly.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/vector/b2enums.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class ImpPathForDragAndCreate;

//************************************************************
//   Hilfsklasse SdrPathObjGeoData
//
// fuer Undo/Redo
//
//************************************************************

class SdrPathObjGeoData : public SdrTextObjGeoData
{
public:
    basegfx::B2DPolyPolygon maPathPolygon;
    SdrObjKind                  meKind;

    SdrPathObjGeoData();
    virtual ~SdrPathObjGeoData();
};

//************************************************************
//   SdrPathObj
//************************************************************

class SVX_DLLPUBLIC SdrPathObj : public SdrTextObj
{
private:
    friend class ImpPathForDragAndCreate;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    basegfx::B2DPolyPolygon maPathPolygon;
    SdrObjKind                  meKind;

    // for isolation of old Drag/Create code
    ImpPathForDragAndCreate*    mpDAC;

protected:
    // Hilfsfunktion fuer GET/SET/INS/etc. PNT
    void ImpSetClosed(sal_Bool bClose);
    void ImpForceKind();
    void ImpForceLineWink();
    ImpPathForDragAndCreate& impGetDAC() const;
    void impDeleteDAC() const;

public:
    static sal_Bool ImpFindPolyPnt(const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum);
    virtual void SetRectsDirty(sal_Bool bNotMyself = sal_False);

public:
    TYPEINFO();
    SdrPathObj(SdrObjKind eNewKind);
    SdrPathObj(SdrObjKind eNewKind, const basegfx::B2DPolyPolygon& rPathPoly);
    virtual ~SdrPathObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual void operator=(const SdrObject& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual void RecalcSnapRect();
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    virtual FASTBOOL BegCreate(SdrDragStat& rStat);
    virtual FASTBOOL MovCreate(SdrDragStat& rStat);
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual FASTBOOL BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    Pointer GetCreatePointer() const;

    // during drag or create, allow accessing the so-far created/modified polyPolygon
    basegfx::B2DPolyPolygon getObjectPolyPolygon(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getDragPolyPolygon(const SdrDragStat& rDrag) const;

    virtual void NbcMove(const Size& aSize);
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact);
    virtual void NbcRotate(const Point& rRefPnt, long nAngle, double fSin, double fCos);
    virtual void NbcMirror(const Point& rRefPnt1, const Point& rRefPnt2);
    virtual void NbcShear(const Point& rRefPnt, long nAngle, double fTan, FASTBOOL bVShear);

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual Point GetSnapPoint(sal_uInt32 i) const;

    virtual sal_Bool IsPolyObj() const;
    virtual sal_uInt32 GetPointCount() const;
    virtual Point GetPoint(sal_uInt32 nHdlNum) const;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 nHdlNum);

    // Punkt einfuegen
    sal_uInt32 NbcInsPointOld(const Point& rPos, sal_Bool bNewObj, sal_Bool bHideHim);
    sal_uInt32 NbcInsPoint(sal_uInt32 i, const Point& rPos, sal_Bool bNewObj, sal_Bool bHideHim);

    // An diesem Punkt auftrennen
    SdrObject* RipPoint(sal_uInt32 nHdlNum, sal_uInt32& rNewPt0Index);

protected:
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

public:
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

    // Bezierpolygon holen/setzen
    const basegfx::B2DPolyPolygon& GetPathPoly() const { return maPathPolygon; }
    void SetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly);
    void NbcSetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly);

    // Spezialfunktionen fuer Bezierpolygon-Bearbeitung
    sal_Bool IsClosed() const { return meKind==OBJ_POLY || meKind==OBJ_PATHPOLY || meKind==OBJ_PATHFILL || meKind==OBJ_FREEFILL || meKind==OBJ_SPLNFILL; }
    sal_Bool IsLine() const { return meKind==OBJ_PLIN || meKind==OBJ_PATHPLIN || meKind==OBJ_PATHLINE || meKind==OBJ_FREELINE || meKind==OBJ_SPLNLINE || meKind==OBJ_LINE; }
    sal_Bool IsFreeHand() const { return meKind==OBJ_FREELINE || meKind==OBJ_FREEFILL; }
    sal_Bool IsBezier() const { return meKind==OBJ_PATHLINE || meKind==OBJ_PATHFILL; }
    sal_Bool IsSpline() const { return meKind==OBJ_SPLNLINE || meKind==OBJ_SPLNFILL; }

    // Pfad schliessen bzw. oeffnen; im letzteren Fall den Endpunkt um
    // "nOpenDistance" verschieben
    void ToggleClosed(); // long nOpenDistance);

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
    virtual sal_Bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOPATH_HXX

