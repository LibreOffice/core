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

#pragma once

#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>
#include <memory>

class ImpPathForDragAndCreate;

// Helper class SdrPathObjGeoData
// used for undo/redo

class SdrPathObjGeoData final : public SdrTextObjGeoData
{
public:
    basegfx::B2DPolyPolygon maPathPolygon;
    SdrObjKind meKind;

    SdrPathObjGeoData();
    virtual ~SdrPathObjGeoData() override;
};


class SVXCORE_DLLPUBLIC SdrPathObj final : public SdrTextObj
{
private:
    friend class ImpPathForDragAndCreate;

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    basegfx::B2DPolyPolygon maPathPolygon;
    SdrObjKind meKind;
    bool mbHandleScale = false;

    // for isolation of old Drag/Create code
    std::unique_ptr<ImpPathForDragAndCreate> mpDAC;

    // helper functions for GET, SET, INS etc. PNT
    void ImpSetClosed(bool bClose);
    void ImpForceKind();
    void ImpForceLineAngle();
    ImpPathForDragAndCreate& impGetDAC() const;

private:
    // protected destructor - due to final, make private
    virtual ~SdrPathObj() override;

public:
    SdrPathObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewKind);
    // Copy constructor
    SdrPathObj(SdrModel& rSdrModel, SdrPathObj const & rSource);
    SdrPathObj(
        SdrModel& rSdrModel,
        SdrObjKind eNewKind,
        const basegfx::B2DPolyPolygon& rPathPoly);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrObjKind GetObjIdentifier() const override;
    virtual void TakeUnrotatedSnapRect(tools::Rectangle& rRect) const override;
    virtual SdrPathObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual void RecalcSnapRect() override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;
    virtual void AddToPlusHdlList(SdrHdlList& rHdlList, SdrHdl& rHdl) const override;
    virtual const tools::Rectangle& GetLogicRect() const override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const override;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    PointerStyle GetCreatePointer() const override;

    // during drag or create, allow accessing the so-far created/modified polyPolygon
    basegfx::B2DPolyPolygon getObjectPolyPolygon(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getDragPolyPolygon(const SdrDragStat& rDrag) const;

    virtual void NbcMove(const Size& aSize) override;
    virtual void NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact) override;
    virtual void NbcRotate(const Point& rRefPnt, Degree100 nAngle, double fSin, double fCos) override;
    virtual void NbcMirror(const Point& rRefPnt1, const Point& rRefPnt2) override;
    virtual void NbcShear(const Point& rRefPnt, Degree100 nAngle, double fTan, bool bVShear) override;

    virtual sal_uInt32 GetSnapPointCount() const override;
    virtual Point GetSnapPoint(sal_uInt32 i) const override;

    virtual bool IsPolyObj() const override;
    virtual sal_uInt32 GetPointCount() const override;
    virtual Point GetPoint(sal_uInt32 nHdlNum) const override;
    virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 nHdlNum) override;

    // insert point
    sal_uInt32 NbcInsPointOld(const Point& rPos, bool bNewObj);
    sal_uInt32 NbcInsPoint(const Point& rPos, bool bNewObj);

    // rip at given point
    SdrObject* RipPoint(sal_uInt32 nHdlNum, sal_uInt32& rNewPt0Index);

private:
    virtual std::unique_ptr<SdrObjGeoData> NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestoreGeoData(const SdrObjGeoData& rGeo) override;

public:
    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    // Bezier-polygon getter/setter
    const basegfx::B2DPolyPolygon& GetPathPoly() const { return maPathPolygon; }
    void SetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly);
    void NbcSetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly);

    // special functions for Bezier-polygon handling
    bool IsClosed() const { return meKind==SdrObjKind::Polygon || meKind==SdrObjKind::PathPoly || meKind==SdrObjKind::PathFill || meKind==SdrObjKind::FreehandFill || meKind==SdrObjKind::SplineFill; }
    bool IsLine() const { return meKind==SdrObjKind::PolyLine || meKind==SdrObjKind::PathPolyLine || meKind==SdrObjKind::PathLine || meKind==SdrObjKind::FreehandLine || meKind==SdrObjKind::SplineLine || meKind==SdrObjKind::Line; }
    bool IsBezier() const { return meKind==SdrObjKind::PathLine || meKind==SdrObjKind::PathFill; }
    bool IsSpline() const { return meKind==SdrObjKind::SplineLine || meKind==SdrObjKind::SplineFill; }

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
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;
    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    /**
     * Scaling is ignored from the transform matrix by default, to not break compatibility. One can
     * opt in to handle scaling if matching ODF behavior is more important.
     */
    void SetHandleScale(bool bHandleScale);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
