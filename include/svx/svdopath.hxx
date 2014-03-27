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

#ifndef INCLUDED_SVX_SVDOPATH_HXX
#define INCLUDED_SVX_SVDOPATH_HXX

#include <svx/svdotext.hxx>
#include <svx/xpoly.hxx>
#include <svx/svxdllapi.h>
#include <basegfx/vector/b2enums.hxx>

class ImpPathForDragAndCreate;

// Helper class SdrPathObjGeoData
// used for undo/redo

class SdrPathObjGeoData : public SdrTextObjGeoData
{
public:
    basegfx::B2DPolyPolygon maPathPolygon;
    SdrObjKind                  meKind;

    SdrPathObjGeoData();
    virtual ~SdrPathObjGeoData();
};


class SVX_DLLPUBLIC SdrPathObj : public SdrTextObj
{
private:
    friend class ImpPathForDragAndCreate;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

    basegfx::B2DPolyPolygon maPathPolygon;
    SdrObjKind                  meKind;

    // for isolation of old Drag/Create code
    ImpPathForDragAndCreate*    mpDAC;

    // brightness - used in EnhancedCustomShapes2d.cxx for DARKEN[LESS] and LIGHTEN[LESS] segments implementation
    double mdBrightness;

protected:
    // helper functions for GET, SET, INS etc. PNT
    void ImpSetClosed(bool bClose);
    void ImpForceKind();
    void ImpForceLineWink();
    ImpPathForDragAndCreate& impGetDAC() const;
    void impDeleteDAC() const;

public:
    static bool ImpFindPolyPnt(const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum);
    virtual void SetRectsDirty(bool bNotMyself = false) SAL_OVERRIDE;
    double GetBrightness() { return mdBrightness; }

public:
    TYPEINFO_OVERRIDE();
    SdrPathObj(SdrObjKind eNewKind);
    SdrPathObj(SdrObjKind eNewKind, const basegfx::B2DPolyPolygon& rPathPoly, double dBrightness = 1.0);
    virtual ~SdrPathObj();

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const SAL_OVERRIDE;
    virtual SdrPathObj* Clone() const SAL_OVERRIDE;
    SdrPathObj& operator=(const SdrPathObj& rObj);

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;
    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual sal_uInt32 GetHdlCount() const SAL_OVERRIDE;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const SAL_OVERRIDE;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const SAL_OVERRIDE;
    virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlNum) const SAL_OVERRIDE;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const SAL_OVERRIDE;

    // special drag methods
    virtual bool hasSpecialDrag() const SAL_OVERRIDE;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) SAL_OVERRIDE;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    virtual bool BegCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool MovCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;
    virtual bool BckCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual void BrkCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    Pointer GetCreatePointer() const SAL_OVERRIDE;

    // during drag or create, allow accessing the so-far created/modified polyPolygon
    basegfx::B2DPolyPolygon getObjectPolyPolygon(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getDragPolyPolygon(const SdrDragStat& rDrag) const;

    virtual void NbcMove(const Size& aSize) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact) SAL_OVERRIDE;
    virtual void NbcRotate(const Point& rRefPnt, long nAngle, double fSin, double fCos) SAL_OVERRIDE;
    virtual void NbcMirror(const Point& rRefPnt1, const Point& rRefPnt2) SAL_OVERRIDE;
    virtual void NbcShear(const Point& rRefPnt, long nAngle, double fTan, bool bVShear) SAL_OVERRIDE;

    virtual sal_uInt32 GetSnapPointCount() const SAL_OVERRIDE;
    virtual Point GetSnapPoint(sal_uInt32 i) const SAL_OVERRIDE;

    virtual bool IsPolyObj() const SAL_OVERRIDE;
    virtual sal_uInt32 GetPointCount() const SAL_OVERRIDE;
    virtual Point GetPoint(sal_uInt32 nHdlNum) const SAL_OVERRIDE;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 nHdlNum) SAL_OVERRIDE;

    // insert point
    sal_uInt32 NbcInsPointOld(const Point& rPos, bool bNewObj, bool bHideHim);
    sal_uInt32 NbcInsPoint(sal_uInt32 i, const Point& rPos, bool bNewObj, bool bHideHim);

    // rip at given point
    SdrObject* RipPoint(sal_uInt32 nHdlNum, sal_uInt32& rNewPt0Index);

protected:
    virtual SdrObjGeoData* NewGeoData() const SAL_OVERRIDE;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const SAL_OVERRIDE;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

public:
    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const SAL_OVERRIDE;

    // Bezier-polygon getter/setter
    const basegfx::B2DPolyPolygon& GetPathPoly() const { return maPathPolygon; }
    void SetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly);
    void NbcSetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly);

    // special functions for Bezier-polygon handling
    bool IsClosed() const { return meKind==OBJ_POLY || meKind==OBJ_PATHPOLY || meKind==OBJ_PATHFILL || meKind==OBJ_FREEFILL || meKind==OBJ_SPLNFILL; }
    bool IsLine() const { return meKind==OBJ_PLIN || meKind==OBJ_PATHPLIN || meKind==OBJ_PATHLINE || meKind==OBJ_FREELINE || meKind==OBJ_SPLNLINE || meKind==OBJ_LINE; }
    bool IsFreeHand() const { return meKind==OBJ_FREELINE || meKind==OBJ_FREEFILL; }
    bool IsBezier() const { return meKind==OBJ_PATHLINE || meKind==OBJ_PATHFILL; }
    bool IsSpline() const { return meKind==OBJ_SPLNLINE || meKind==OBJ_SPLNFILL; }

    // close/open path
    // if opening, move end point by "nOpenDistance"
    void ToggleClosed(); // long nOpenDistance);

    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
    //
    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const SAL_OVERRIDE;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_SVDOPATH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
