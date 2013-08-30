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

    // brightness - used in EnhancedCustomShapes2d.cxx for DARKEN[LESS] and LIGHTEN[LESS] segments implementation
    double mdBrightness;

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
    double GetBrightness() { return mdBrightness; }

public:
    TYPEINFO();
    SdrPathObj(SdrObjKind eNewKind);
    SdrPathObj(SdrObjKind eNewKind, const basegfx::B2DPolyPolygon& rPathPoly, double dBrightness = 1.0);
    virtual ~SdrPathObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
    virtual SdrPathObj* Clone() const;
    SdrPathObj& operator=(const SdrPathObj& rObj);

    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;
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
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
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
    virtual void NbcShear(const Point& rRefPnt, long nAngle, double fTan, bool bVShear);

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
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
